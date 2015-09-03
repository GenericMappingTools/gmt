/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: greenspline grids data using Green's functions for a selected spline.
 * The data may be Cartesian or geographical, and gridding can be done
 * in a Cartesian 1-D, 2-D, 3-D space or on a sphere.  The spline may be evaluated on
 * a grid, on parts of a grid, or at specified arbitrary locations.
 * Five classes of splines (for a total of 10 splines) are implemented:
 * 1. Minimum curvature Cartesian spline [Sandwell, Geophys. Res. Lett, 1987]
 * 2. Minimum curvature Cartesian spline in tension [Wessel & Bercovici, Math., Geol., 1998]
 * 3. Regularized Cartesian spline in tension [Mitasova & Mitas, Math. Geol., 1993]
 * 4. Minimum curvature spherical spline [Parker, "Geophysical Inverse Theory", 1994]
 * 5. Minimum curvature spherical spline in tension [Wessel & Becker, Geophys. J. Int, 2008]
 *
 * Originally published as:
 *   "Wessel, P., 2009. A general-purpose Green's function-based interpolator,
 *	Computers & Geosciences, 35: 1247-1254".
 *
 * PW Update June 2013.  The numerical implementation of the Green's function found by Wessel & Becker [2008]
 * was unstable (it required the difference between k*P_v and log, and P_v is difficult to compute accurately).
 * Bob Parker (Scripps) helped develop a series solution for canceling out the two singular behaviors,
 * resulting in a more stable expression that converges reasonably rapidly.  We now use this new series
 * solution for -Sq, combined with a (new) cubic spline interpolation.  This replaces the old -SQ machinery
 * with linear interpolation which is now deprecated.
 *
 * PW Update July 2015. With help from Dong Ju Choi, San Diego Supercomputing Center, we have added Open MP
 * support in greenspline and the matrix solvers in gmt_vector.c.  Requires open MP support and use of -x.
 */

#define THIS_MODULE_NAME	"greenspline"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate using Green's functions for splines in 1-3 dimensions"
#define THIS_MODULE_KEYS	"<DI,ADi,NDi,TGi,CDo,GGO,RG+"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbdfghiors" GMT_OPT("FH") GMT_ADD_x_OPT

double GMT_geodesic_dist_cos (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE);
double GMT_great_circle_dist_cos (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC int GMT_cspline (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *c);

/* Control structure for greenspline */

struct GREENSPLINE_CTRL {
	struct A {	/* -A<gradientfile> */
		bool active;
		unsigned int mode;	/* 0 = azimuths, 1 = directions, 2 = dx,dy components, 3 = dx, dy, dz components */
		char *file;
	} A	;
	struct C {	/* -C[n|v]<cutoff>[/<file>] */
		bool active;
		unsigned int mode;
		double value;
		char *file;
	} C;
	struct D {	/* -D<distflag> */
		bool active;
		int mode;	/* Can be negative */
	} D;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy[/dz]] */
		bool active;
		double inc[3];
	} I;
	struct L {	/* -L */
		bool active;
	} L;
	struct N {	/* -N<outputnode_file> */
		bool active;
		char *file;
	} N;
	struct Q {	/* -Qdaz */
		bool active;
		double az;
		double dir[3];
	} Q;
	struct R3 {	/* -Rxmin/xmax[/ymin/ymax[/zmin/zmaz]] | -Ggridfile */
		bool active;
		bool mode;		/* true if settings came from a grid file */
		unsigned int dimension;	/* 1, 2, or 3 */
		unsigned int offset;	/* 0 or 1 */
		double range[6];	/* Min/max for each dimension */
		double inc[2];		/* xinc/yinc when -Rgridfile was given*/
	} R3;
	struct S {	/* -S<mode>[<tension][+<mod>[args]] */
		bool active;
		unsigned int mode;
		double value[4];
		double rval[2];
		char *arg;
	} S;
	struct T {	/* -T<mask_grdfile> */
		bool active;
		char *file;
	} T;
	struct W {	/* -W */
		bool active;
	} W;
};

#define SANDWELL_1987_1D		0
#define SANDWELL_1987_2D		1
#define SANDWELL_1987_3D		2
#define WESSEL_BERCOVICI_1998_1D	3
#define WESSEL_BERCOVICI_1998_2D	4
#define WESSEL_BERCOVICI_1998_3D	5
#define MITASOVA_MITAS_1993_2D		6
#define MITASOVA_MITAS_1993_3D		7
#define PARKER_1994			8
#define WESSEL_BECKER_2008		9
#define LINEAR_1D			10
#define LINEAR_2D			11

#define N_METHODS			12
#define N_PARAMS			11

#ifndef M_LOG_2
#define M_LOG_2 0.69314718055994530942
#endif
#ifndef M_GAMMA
#define M_GAMMA 0.577215664901532860606512
#endif
#ifndef M_SQRT_PI
#define M_SQRT_PI 1.772453850905516027298167483341
#endif
#ifndef M_INV_SQRT_PI
#define M_INV_SQRT_PI (1.0 / M_SQRT_PI)
#endif

#define SQ_N_NODES 		10001	/* Default number of nodes in the precalculated -Sq spline */
#define SQ_TRUNC_ERROR		1.0e-6	/* Max truncation error in Parker's simplified sum for WB'08 */

#define GREENSPLINE_TREND	1	/* Remove/Restore linear trend */
#define GREENSPLINE_NORM	2	/* Normalize residual data to 0-1 range */

enum Greenspline_index {	/* Indices for coeff array for normalization */
	GSP_MEAN_X	= 0,
	GSP_MEAN_Y	= 1,
	GSP_MEAN_Z	= 2,
	GSP_SLP_X	= 3,
	GSP_SLP_Y	= 4,
	GSP_RANGE	= 5,
	GSP_LENGTH	= 6};

struct GREENSPLINE_LOOKUP {	/* Used to spline interpolation of precalculated function */
	uint64_t n;		/* Number of values in the spline setup */
	double *y;		/* Function values */
	double *c;		/* spline  coefficients */
	double *A, *B, *C;	/* power/ratios of order l terms */
};

struct ZGRID {
	unsigned int nz;
	double z_min, z_max, z_inc;
};

#ifdef DEBUG
bool TEST = false;	/* Global variable used for undocumented testing [under -DDEBUG only; see -+ hidden option] */
#endif

void *New_greenspline_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GREENSPLINE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GREENSPLINE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->S.mode = SANDWELL_1987_2D;
	C->S.rval[0] = -1.0;	C->S.rval[1] = 1.0;
	C->S.value[3] = (double)SQ_N_NODES;	/* Default number of spline nodes */
	C->S.value[2] = SQ_TRUNC_ERROR;		/* Default truncation error for Legendre sum in -Sq */
	return (C);
}

void Free_greenspline_Ctrl (struct GMT_CTRL *GMT, struct GREENSPLINE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->A.file) free (C->A.file);
	if (C->C.file) free (C->C.file);
	if (C->G.file) free (C->G.file);
	if (C->N.file) free (C->N.file);
	if (C->T.file) free (C->T.file);
	if (C->S.arg)  free (C->S.arg);
	GMT_free (GMT, C);
}

int GMT_greenspline_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: greenspline [<table>] -G<outfile> [-A[<format>,]<gradientfile>]\n\t[-R<xmin>/<xmax[/<ymin>/<ymax>[/<zmin>/<zmax>]]]");
	GMT_Message (API, GMT_TIME_NONE, "[-I<dx>[/<dy>[/<dz>]] [-C[n|v]<cut>[/<file>]]\n\t[-D<mode>] [-L] [-N<nodes>] [-Q<az>] [-Sc|l|t|r|p|q[<pars>]] [-T<maskgrid>] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]%s[%s]\n\n",
		GMT_bi_OPT, GMT_d_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_s_OPT, GMT_x_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\tChoose one of three ways to specify where to evaluate the spline:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t1. Specify a rectangular grid domain with options -R, -I [and optionally -r].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t2. Supply a mask file via -T whose values are NaN or 0.  The spline will then\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   only be evaluated at the nodes originally set to zero.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t3. Specify a set of output locations via the -N option.\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Output data. Give name of output file.\n");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");

	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A ASCII file with surface gradients V to use in the modeling.  Specify format:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (0) For 1-D: x, slope, (1) X, Vmagnitude, Vazimuth(s), (2) X, Vazimuth(s), Vmagnitude,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (3) X, Vmagnitude, Vangle(s), (4) X, Vcomponents, or (5) X, Vunit-vector, Vmagnitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Here, X = (x, y[, z]) is the position vector, V is the gradient vector.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Solve by SVD and eliminate eigenvalues whose ratio to largest eigenvalue is less than <cut> [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append /<filename> to save the eigenvalues to this file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A negative cutoff will stop execution after saving the eigenvalues.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Cn to select only the largest <cut> eigenvalues [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Cv to select only eigenvalues needed to explain <cut> %% of data variance [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses Gauss-Jordan elimination to solve the linear system]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Distance flag determines how we calculate distances between (x,y) points:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Options 0 apples to Cartesian 1-D spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D0 x in user units, Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Options 1-3 apply to Cartesian 2-D surface spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D1 x,y in user units, Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D2 x,y in degrees, flat Earth distances in meters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D3 x,y in degrees, spherical distances in meters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Option 4 applies to 2-D spherical surface spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D4 x,y in degrees, use cosine of spherical distances in degrees.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Option 5 applies to Cartesian 3-D volume interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D5 x,y,z in user units, Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For option 3-4, use PROJ_ELLIPSOID to select geodesic or great cicle arcs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify a regular set of output locations.  Give equidistant increment for each dimension.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Requires -R for specifying the output domain.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Leave trend alone.  Do not remove least squares plane from data before spline fit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Only applies to -D0-2.  [Default removes linear trend, fits residuals, and restores trend].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N ASCII file with desired output locations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The resulting ASCII coordinates and interpolation are written to file given in -G\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or stdout if no file specified (see -bo for binary output).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Calculate the directional derivative in the <az> direction and return it instead of surface elevation.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Specify a regular set of output locations.  Give min and max coordinates for each dimension.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Requires -I for specifying equidistant increments.  For 2D-gridding a gridfile may be given;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   this then also sets -I (and perhaps -r); use those options to override the grid settings.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify which spline to use; except for c|p, append normalized <tension> between 0 and 1:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sc is minimum curvature spline (Sandwell, 1987) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sl is a linear (1-D) or bilinear (2-D) spline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -St<tension>[/<scale>] is a Cartesian spline in tension (Wessel & Bercovici, 1998).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Optionally, specify a length-scale [Default is the given output spacing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sr<tension> is a regularized spline in tension (Mitasova & Mitas, 1993).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Optionally, specify a length-scale [Default is given output spacing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sp is a spherical surface spline (Parker, 1994); automatically sets -D4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sq is a spherical surface spline in tension (Wessel & Becker, 2008); automatically sets -D4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +e<error> to change maximum error in series truncation [%g].\n", SQ_TRUNC_ERROR);
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +n<n> to change the (odd) number of precalculated nodes for spline interpolation [%d].\n", SQ_N_NODES);
	GMT_Message (API, GMT_TIME_NONE, "\t-T Mask grid file whose values are NaN or 0; its header implicitly sets -R, -I (and -r).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Expects one extra input column with data weights (e.g., w_i = 1/sigma_i).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note this will only have an effect if -C is used.\n");
	GMT_Option (API, "V,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 2-5 input columns depending on dimensionality (see -D) and weights (see -W).\n");
	GMT_Option (API, "d,g,h,i,o,r,s,x,:,.");

	return (EXIT_FAILURE);
}

int GMT_greenspline_parse (struct GMT_CTRL *GMT, struct GREENSPLINE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to greenspline and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_items;
	unsigned int n_errors = 0, dimension, k, pos = 0;
	char txt[6][GMT_LEN64], p[GMT_BUFSIZ] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			case 'R':	/* Normally processed internally but must be handled separately since it can take 1,2,3 dimensions */
				GMT->common.R.active = true;
				Ctrl->R3.dimension = 1;	/* At least */

				if (opt->arg[0] == 'g' && opt->arg[1] == '\0') {	/* Got -Rg */
					Ctrl->R3.range[0] = 0.0;	Ctrl->R3.range[1] = 360.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					Ctrl->R3.dimension = 2;
					break;
				}
				if (opt->arg[0] == 'd' && opt->arg[1] == '\0') {	/* Got -Rd */
					Ctrl->R3.range[0] = -180.0;	Ctrl->R3.range[1] = 180.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					Ctrl->R3.dimension = 2;
					break;
				}
				if (!GMT_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file, presumably a grid */
					struct GMT_GRID *G = NULL;
					if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, opt->arg, NULL)) == NULL) {	/* Get header only */
						return (API->error);
					}
					Ctrl->R3.range[0] = G->header->wesn[XLO]; Ctrl->R3.range[1] = G->header->wesn[XHI];
					Ctrl->R3.range[2] = G->header->wesn[YLO]; Ctrl->R3.range[3] = G->header->wesn[YHI];
					Ctrl->R3.inc[GMT_X] = G->header->inc[GMT_X];	Ctrl->R3.inc[GMT_Y] = G->header->inc[GMT_Y];
					Ctrl->R3.offset = G->header->registration;
					Ctrl->R3.dimension = 2;
					Ctrl->R3.mode = true;
					if (GMT_Destroy_Data (API, &G) != GMT_OK) {
						return (API->error);
					}
					break;
				}
				/* Only get here if the above cases did not trip */
				n_items = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s", txt[0], txt[1], txt[2], txt[3], txt[4], txt[5]);
				if (!(n_items == 2 || n_items == 4 || n_items == 6)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -R option: Give 2, 4, or 6 coordinates\n");
					n_errors++;
				}
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt[0], GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->R3.range[0]), txt[0]);
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt[1], GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->R3.range[1]), txt[1]);
				if (n_items > 2) {
					Ctrl->R3.dimension = 2;
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt[2], GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->R3.range[2]), txt[2]);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt[3], GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->R3.range[3]), txt[3]);
				}
				if (n_items == 6) {
					Ctrl->R3.dimension = 3;
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, txt[4], GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->R3.range[4]), txt[4]);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, txt[5], GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->R3.range[5]), txt[5]);
				}
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Gradient data */
				Ctrl->A.active = true;
				k = 0;
				if (strchr (opt->arg, ',')) {	/* Specified a particular format with -A<mode>,<file> */
					Ctrl->A.mode = (int)(opt->arg[0] - '0');
					k = 2;
				}
				Ctrl->A.file = strdup (&opt->arg[k]);
				break;
			case 'C':	/* Solve by SVD */
				Ctrl->C.active = true;
				if (opt->arg[0] == 'v') Ctrl->C.mode = 1;
				else if (opt->arg[0] == 'n') Ctrl->C.mode = 2;
				k = (Ctrl->C.mode) ? 1 : 0;
				if (strchr (opt->arg, '/')) {
					char tmp[GMT_BUFSIZ];
					sscanf (&opt->arg[k], "%lf/%s", &Ctrl->C.value, tmp);
					Ctrl->C.file = strdup (tmp);
				}
				else
					Ctrl->C.value = atof (&opt->arg[k]);
				break;
			case 'D':	/* Distance mode */
				Ctrl->D.active = true;
				Ctrl->D.mode = atoi (opt->arg);	/* Since I added 0 to be 1-D later so now this is mode -1 */
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Table or grid spacings */
				Ctrl->I.active = true;
				k = GMT_getincn (GMT, opt->arg, Ctrl->I.inc, 3);
				if (k < 1) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				if (Ctrl->I.inc[GMT_Y] == 0.0) Ctrl->I.inc[GMT_Y] = Ctrl->I.inc[GMT_X];
				if (Ctrl->I.inc[GMT_Z] == 0.0) Ctrl->I.inc[GMT_Z] = Ctrl->I.inc[GMT_X];
				break;
			case 'L':	/* Leave trend alone */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Output locations */
				if ((Ctrl->N.active = GMT_check_filearg (GMT, 'N', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->N.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'Q':	/* Directional derivative */
				Ctrl->Q.active = true;
				if (strchr (opt->arg, '/')) {	/* Got 3-D vector components */
					n_items = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->Q.dir[0], &Ctrl->Q.dir[1], &Ctrl->Q.dir[2]);
					if (n_items != 3) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Append azimuth (2-D) or x/y/z components (3-D)\n");
						n_errors++;
					}
					GMT_normalize3v (GMT, Ctrl->Q.dir);	/* Normalize to unit vector */
				}
				else if (opt->arg[0])	/* 2-D azimuth */
					Ctrl->Q.az = atof(opt->arg);
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Append azimuth (2-D) or x/y/z components (3-D)\n");
					n_errors++;
				}
				break;
			case 'S':	/* Spline selection */
				Ctrl->S.arg = strdup (opt->arg);
				switch (opt->arg[0]) {
					case 'l':	/*  Cartesian linear spline in 1-D or 2-D (bilinear) */
						Ctrl->S.mode = LINEAR_1D;
						break;
					case 'c':	/* Cartesian minimum curvature spline */
						Ctrl->S.mode = SANDWELL_1987_1D;
						break;
					case 't':	/* Cartesian minimum curvature spline in tension */
						Ctrl->S.mode = WESSEL_BERCOVICI_1998_1D;
						if (strchr (opt->arg, '/'))
							sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1]);
						else
							Ctrl->S.value[0] = atof (&opt->arg[1]);
						break;
					case 'r':	/* Regularized minimum curvature spline in tension in 2-D or 3-D */
						Ctrl->S.mode = MITASOVA_MITAS_1993_2D;
						if (strchr (opt->arg, '/'))
							sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1]);
						else
							Ctrl->S.value[0] = atof (&opt->arg[1]);
						break;
					case 'p':	/* Spherical minimum curvature spline */
						Ctrl->S.mode = PARKER_1994;
						break;
					case 'Q':	/* Spherical minimum curvature spline in tension */
						if (GMT_compat_check (GMT, 4)) {
							GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -SQ is deprecated; see -Sq syntax instead.\n");
							Ctrl->S.mode = WESSEL_BECKER_2008;
							if (strchr (opt->arg, '/')) {
								n_items = sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1], &Ctrl->S.rval[0], &Ctrl->S.rval[1]);
								if (n_items == 2) {
									Ctrl->S.rval[0] = -1.0;
									Ctrl->S.rval[1] = +1.0;
								}
							}
							else
								Ctrl->S.value[0] = atof (&opt->arg[1]);
						}
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Append c|l|t|g|p|q\n");
							n_errors++;
						}
						break;
					case 'q':	/* Spherical minimum curvature spline in tension */
						Ctrl->S.mode = WESSEL_BECKER_2008;
						Ctrl->S.value[0] = atof (&opt->arg[1]);
						if (Ctrl->S.value[0] == 0.0)	/* Switch to Parker_1994 since tension is zero */
							Ctrl->S.mode = PARKER_1994;
						if ((c = strchr (opt->arg, '+'))) {
							while (GMT_strtok (c, "+", &pos, p)) {
								switch (p[0]) {
									case 'e':	Ctrl->S.value[2] = atof (&p[1]);	break;	/* Change the truncation error limit */
									case 'n':	Ctrl->S.value[3] = atof (&p[1]);	break;	/* Change the number of nodes for the spline lookup */
									case 'l':	Ctrl->S.rval[0]  = atof (&p[1]);	break;	/* Min value for spline, undocumented for testing only */
									case 'u':	Ctrl->S.rval[1]  = atof (&p[1]);	break;	/* Max value for spline, undocumented for testing only */
									default:
										GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sq option: Unknown modifier %s\n", p);
										n_errors++;
										break;
								}
							}
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Append c|l|t|g|p|q[<params>]\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Input mask grid */
				if ((Ctrl->T.active = GMT_check_filearg (GMT, 'T', opt->arg, GMT_IN, GMT_IS_GRID))) {	/* Obtain -R -I -r from file */
					struct GMT_GRID *G = NULL;
					Ctrl->T.file = strdup (opt->arg);
					if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, opt->arg, NULL)) == NULL) {	/* Get header only */
						return (API->error);
					}
					Ctrl->R3.range[0] = G->header->wesn[XLO]; Ctrl->R3.range[1] = G->header->wesn[XHI];
					Ctrl->R3.range[2] = G->header->wesn[YLO]; Ctrl->R3.range[3] = G->header->wesn[YHI];
					Ctrl->R3.inc[GMT_X] = G->header->inc[GMT_X];	Ctrl->R3.inc[GMT_Y] = G->header->inc[GMT_Y];
					Ctrl->R3.offset = G->header->registration;
					Ctrl->R3.dimension = 2;
					Ctrl->R3.mode = true;
					if (GMT_Destroy_Data (API, &G) != GMT_OK) {
						return (API->error);
					}
					GMT->common.R.active = true;
				}
				else
					n_errors++;
				break;
			case 'W':	/* Expect data weights in last column */
				Ctrl->W.active = true;
				break;
#ifdef DEBUG
			case '+':	/* Turn on TEST mode */
				TEST = true;
				break;
#endif
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->S.mode == WESSEL_BECKER_2008) {	/* Check that nodes is an odd integer */
		double fn = rint (Ctrl->S.value[3]);
		int64_t n = lrint (fn);
		if (!doubleAlmostEqual (Ctrl->S.value[3], fn) || ((n%2) == 0)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sq option +n<N> modifier: <N> must be an odd integer\n");
			n_errors++;
		}
		if (Ctrl->S.value[2] < 0.0 || Ctrl->S.value[2] > 1.0e-4) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Sq option +e<err> modifier: <err> must be positive and < 1.0e-4\n");
			n_errors++;
		}
	}
	if (Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) Ctrl->D.mode = 4;	/* Automatically set */
	dimension = (Ctrl->D.mode == 0) ? 1 : ((Ctrl->D.mode == 5) ? 3 : 2);
	if (dimension == 2 && Ctrl->R3.mode) {	/* Set -R via a gridfile */
		/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
		if (!Ctrl->I.active) {	/* -I was not set separately; set indirectly */
			Ctrl->I.inc[GMT_X] = Ctrl->R3.inc[GMT_X];
			Ctrl->I.inc[GMT_Y] = Ctrl->R3.inc[GMT_Y];
			Ctrl->I.active = true;
		}
		/* Here, -r means toggle the grids registration */
		if (GMT->common.r.active) {
			GMT->common.r.active = !Ctrl->R3.offset;
			GMT->common.r.registration = !Ctrl->R3.offset;
		}
		else {
			GMT->common.r.active = Ctrl->R3.offset;
			GMT->common.r.registration = Ctrl->R3.offset;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->A.active && GMT_access (GMT, Ctrl->A.file, R_OK), "Syntax error -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += GMT_check_condition (GMT, !(GMT->common.R.active || Ctrl->N.active || Ctrl->T.active), "Syntax error: No output locations specified (use either [-R -I], -N, or -T)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->R3.mode && dimension != 2, "Syntax error: The -R<gridfile> or -T<gridfile> option only applies to 2-D gridding\n");
#ifdef DEBUG
	n_errors += GMT_check_condition (GMT, !TEST && Ctrl->R3.dimension != dimension, "Syntax error: The -R and -D options disagree on the dimension\n");
#else
	n_errors += GMT_check_condition (GMT, Ctrl->R3.dimension != dimension, "Syntax error: The -R and -D options disagree on the dimension\n");
#endif
	n_errors += GMT_check_binary_io (GMT, dimension + 1);
	n_errors += GMT_check_condition (GMT, Ctrl->S.value[0] < 0.0 || Ctrl->S.value[0] >= 1.0, "Syntax error -S option: Tension must be in range 0 <= t < 1\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) && Ctrl->D.mode == 3, "Syntax error -Sc|t|r option: Cannot select -D3\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == LINEAR_1D && Ctrl->D.mode > 3, "Syntax error -Sl option: Cannot select -D4 or higher\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || (dimension > 1 && Ctrl->I.inc[GMT_Y] <= 0.0) || (dimension == 3 && Ctrl->I.inc[GMT_Z] <= 0.0)), "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, dimension == 2 && !Ctrl->N.active && !(Ctrl->G.active  || Ctrl->G.file), "Syntax error -G option: Must specify output grid file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->C.value < 0.0 && !Ctrl->C.file, "Syntax error -C option: Must specify file name for eigenvalues if cut < 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->C.mode && Ctrl->C.value > 100.0, "Syntax error -Cv option: Variance explain cannot exceed 100%%\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Syntax error -T option: Must specify mask grid file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && dimension != 2, "Syntax error -T option: Only applies to 2-D gridding\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Syntax error -N option: Must specify node file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->N.file && GMT_access (GMT, Ctrl->N.file, R_OK), "Syntax error -N: Cannot read file %s!\n", Ctrl->N.file);
	n_errors += GMT_check_condition (GMT, (Ctrl->I.active + GMT->common.R.active) == 1 && dimension == 2, "Syntax error: Must specify -R, -I, [-r], -G for gridding\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#ifdef DEBUG
/* Dump a table of x, G, dGdx for test purposes [requires option -+ and compilation with -DDEBUG]  */
void dump_green (struct GMT_CTRL *GMT, double (*G) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *), double (*D) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *), double par[], double x0, double x1, int N, struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg)
{
	int i;
	double x, dx, dy, y, t, ry, rdy;
	double min_y, max_y, min_dy, max_dy;

	min_y = min_dy = DBL_MAX;
	max_y = max_dy = -DBL_MAX;

	dx = (x1 - x0) / (N - 1);
	for (i = 0; i < N; i++) {
		x = x0 + i * dx;
		t = (x0 < 0.0) ? acosd (x) : x;
		y = G (GMT, x, par, Lz);
		dy = D (GMT, x, par, Lg);
		if (y < min_y) min_y = y;
		if (y > max_y) max_y = y;
		if (dy < min_dy) min_dy = dy;
		if (dy > max_dy) max_dy = dy;
	}
	ry = max_y - min_y;
	rdy = max_dy - min_dy;
	for (i = 0; i < N; i++) {
		x = x0 + i * dx;
		t = (x0 < 0.0) ? acosd (x) : x;
		y = G (GMT, x, par, Lz);
		dy = D (GMT, x, par, Lg);
		dy = (rdy > 0.0) ? (dy - min_dy)/rdy : 1.0;
		printf ("%g\t%g\t%g\t%g\n", x, (y - min_y) / ry, dy, t);
	}
}
#endif

/* Below are all the individual Green's functions.  Note that most of them take an argument
 * that is unused except in the spline lookup version of WB_08. */

/*----------------------  ONE DIMENSION ---------------------- */
/* Basic linear spline (bilinear in 2-D) */

double spline1d_linear (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{	/* Dumb linear spline */
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	return (r);	/* Just regular spline; par not used */
}

double gradspline1d_linear (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{	/* d/dr of r is 1 */
	GMT_UNUSED(GMT); GMT_UNUSED(r); GMT_UNUSED(par); GMT_UNUSED(unused);
	return (1.0);
}

/* spline1d_sandwell computes the Green function for a 1-d spline
 * as per Sandwell [1987], G(r) = r^3.  All r must be >= 0.
 */

double spline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	if (r == 0.0) return (0.0);

	return (pow (r, 3.0));	/* Just regular spline; par not used */
}

double gradspline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	return (r);	/* Just regular spline; par not used */
}

double spline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
/* spline1d_Wessel_Bercovici computes the Green function for a 1-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = exp(-u) + u - 1,
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * All r must be >= 0. par[0] = c
 */
{
	double cx;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (exp (-cx) + cx - 1.0);
}

double gradspline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double cx;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (1.0 - exp (-cx));
}

/*----------------------  TWO DIMENSIONS ---------------------- */

double spline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	if (r == 0.0) return (0.0);

	return (r * r * (log (r) - 1.0));	/* Just regular spline; par not used */
}

double gradspline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	if (r == 0.0) return (0.0);

	return (r * (2.0 * log (r) - 1.0));	/* Just regular spline; par not used */
}

/* spline2d_Wessel_Bercovici computes the Green function for a 2-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = K(u) - log(u),
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * K is the modified Bessel function K of order zero.
 * All r must be >= 0.
 * par[0] = c
 * par[1] = 2/c
 */

double spline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double y, z, cx, t, g;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	if (r <= par[1]) {
		y = 0.25 * (t = cx * cx);
		z = t / 14.0625;
		g = (-log(0.5*cx) * (z * (3.5156229 + z * (3.0899424 + z * (1.2067492 + z * (0.2659732 +
			z * (0.360768e-1 + z * 0.45813e-2))))))) + (y * (0.42278420 + y * (0.23069756 +
			y * (0.3488590e-1 + y * (0.262698e-2 + y * (0.10750e-3 + y * 0.74e-5))))));
	}
	else {
		y = par[1] / r;
		g = (exp (-cx) / sqrt (cx)) * (1.25331414 + y * (-0.7832358e-1 + y * (0.2189568e-1 +
			y * (-0.1062446e-1 + y * (0.587872e-2 + y * (-0.251540e-2 + y * 0.53208e-3))))))
			+ log (cx) - M_LOG_2 + M_GAMMA;
	}
	return (g);
}

double gradspline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double y, z, cx, t, dgdr;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	if (r <= par[1]) {
		y = 0.25 * (t = cx * cx);
		z = t / 14.0625;
		dgdr = -((log(0.5*cx) * (cx * (0.5 + z * (0.87890594 + z * (0.51498869 + z * (0.15084934 +
			z * (0.2658733e-1 + z * (0.301532e-2 + z * 0.32411e-3)))))))) + (1.0/cx) * (y * (0.15443144 +
			y * (-0.67278579 + y * (-0.18156897 + y * (-0.1919402e-1 + y * (-0.110404e-2 + y * (-0.4686e-4))))))));
	}
	else {
		y = par[1] / r;
		dgdr = 0.5*y - ((exp (-cx) / sqrt (cx)) * (1.25331414 + y * (0.23498619 + y * (-0.3655620e-1 +
			y * (0.1504268e-1 + y * (-0.780353e-2 + y * (0.325614e-2 + y * (-0.68245e-3))))))));
	}
	return (dgdr*par[0]);
}

/* spline2d_Mitasova_Mitas computes the regularized Green function for a 2-d spline
 * in tension as per Mitasova and Mitas [1993], G(u) = log (u) + Ei(u),
 * where u = par[1] * r^2. All r must be >= 0.
 * par[0] = phi (par[0] unused by function)
 * par[1] = phi^2/4
 */

double spline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	double x, z, g, En, Ed;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	x = z = par[1] * r * r;
	if (z <= 1.0) {
		g = 0.99999193 * x;
		x *= x;
		g -= 0.24991055 * x;
		x *= x;
		g += 0.05519968 * x;
		x *= x;
		g -= 0.00976004 * x;
		x *= x;
		g += 0.00107857 * x;
	}
	else {
		g = log (x) + M_GAMMA;
		En = 0.2677737343 +  8.6347608925 * x;
		Ed = 3.9584869228 + 21.0996530827 * x;
		x *= x;
		En += 18.0590169730 * x;
		Ed += 25.6329561486 * x;
		x *= x;
		En += 8.5733287401 * x;
		Ed += 9.5733223454 * x;
		x *= x;
		En += x;
		Ed += x;
		g += (En / Ed) / (z * exp(z));
	}
	return (g);
}

double gradspline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double u, dgdr;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	u = par[1] * r * r;
	dgdr = 2.0 * (1.0 - exp (-u))/r;
	return (dgdr);
}

/*----------------------  TWO DIMENSIONS (SPHERE) ---------------------- */

/* spline2d_Parker computes the Green function for a 2-d surface spline
 * as per Parker [1994], G(x) = dilog(),
 * where x is cosine of distances. All x must be -1 <= x <= +1.
 * Parameters passed are:
 * par[0] = 6/M_PI^2 (to normalize results)
 */

double spline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *unused)
{	/* Normalized to 0-1 */
	GMT_UNUSED(unused);
	if (x == +1.0) return (1.0);
	if (x == -1.0) return (0.0);
	return (GMT_dilog (GMT, 0.5 - 0.5 * x) * par[0]);
}

double gradspline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *unused)
{	/* Normalized to 0-1 */
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	if (x == +1.0 || x == -1.0) return (0.0);
	return (log (0.5 - 0.5 * x) * sqrt ((1.0 - x) / (1.0 + x)));
}

void series_prepare (struct GMT_CTRL *GMT, double p, unsigned int L, struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg)
{	/* Precalculate Legendre series terms involving various powers/ratios of l and p */
	unsigned int l;
	double pp, t1, t2;
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Precalculate max %u terms for Legendre summation\n", L+1);
	Lz->A = GMT_memory (GMT, NULL, L+1, double);
	Lz->B = GMT_memory (GMT, NULL, L+1, double);
	Lz->C = GMT_memory (GMT, NULL, L+1, double);
	if (Lg) Lg->A = GMT_memory (GMT, NULL, L+1, double);
	pp = p * p;
	for (l = 1; l <= L; l++) {
		t1 = l + 1.0;
		t2 = 2.0 * l + 1.0;
		Lz->A[l] = t2 * pp / (l*t1*(l*t1+pp));
		Lz->B[l] = t2 / t1;
		Lz->C[l] = l / t1;
		if (Lg) Lg->A[l] = t2 * pp / (t1*(l*t1+pp));
	}
	if (Lg) Lg->B = Lz->B;	/* Lg needs the same B,C coefficients as Lz, so just share them via the link */
	if (Lg) Lg->C = Lz->C;
}

unsigned int get_max_L (struct GMT_CTRL *GMT, double p, double err)
{	/* Return max L needed given p and truncation err for Parker's simplified loop expression */
	GMT_UNUSED(GMT);
	return ((unsigned int)lrint (p / sqrt (err)  + 10.0));
}

void free_lookup (struct GMT_CTRL *GMT, struct GREENSPLINE_LOOKUP **Lptr, unsigned int mode)
{	/* Free all items allocated under the lookup structures.
	 * mode = 0 means Lz and mode = 1 means Lg; the latter has no B & C arrays */
	struct GREENSPLINE_LOOKUP *L = *Lptr;
	if (L == NULL) return;	/* Nothing to free */
	if (L->y) GMT_free (GMT, L->y);
	if (L->c) GMT_free (GMT, L->c);
	if (L->A) GMT_free (GMT, L->A);
	if (mode == 0) {	/* Only Lz has A,B,C while Lg borrows B,C */
		if (L->B) GMT_free (GMT, L->B);
		if (L->C) GMT_free (GMT, L->C);
	}
	GMT_free (GMT, L);
	*Lptr = NULL;
}

unsigned int get_L (double x, double p, double err)
{	/* Determines the truncation order L_max given x, p, and err.
	 * See ParkerNotesJan2013.pdf in gurudocs for explanations and derivation */
	unsigned int L_max;
	double s, c, pp, Lf, gam, lam;
	/*  Get all the trig functions needed; x is cos(theta), but we need */
	/* s = sin (theta/2); c=cos(theta/2); */
	/* cos(theta) = x = 1.0 - 2.0 *s^2, so */
	s = sqrt (0.5 * (1 - x));
	c = sqrt (0.5 * (1 + x));
	pp = p * p;

	/* Approximate the highest order L_max we need to sum given specified err limit */
	if (s == 0.0)
		Lf = p / sqrt (err);
	else if (c == 0.0)
		Lf = pow (pp / err, 0.333333333);
	else {
		gam = 0.00633262522 * pow (pp * s * s / err, 2.0) / c;
		if (gam <= 1.0) lam = pow (gam / (1.0 + pow (gam, 0.4)), 0.2);
		else lam = pow (gam / (1.0 + 1.0 / pow (gam, 0.285714286)), 0.142857143);
		Lf = 1.75 * lam / s;
	}

	Lf = MIN (p / sqrt (err), Lf);
	L_max = (unsigned int)lrint (Lf + 10.0);
	return (L_max);
}

double spline2d_Wessel_Becker_Revised (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *Lz)
{	/* Evaluate g = M_PI * Pv(-x)/sin (v*x) - log (1-x) via series approximation */
	unsigned int L_max, l;
	double P0, P1, P2, S;
	GMT_UNUSED(GMT);

	L_max = get_L (x, par[0], par[1]);	/* Highest order needed in sum given x, p, and err */

	/* pp = par[0] * par[0]; */
	P0 = 1.0;	/* Initialize the P0 and P1 Legendre polynomials */
	P1 = x;
	S = par[2];	/* The constant l = 0 term was computed during setup */

	/* Sum the Legendre series */
	for (l = 1; l <= L_max; l++) {
		/* All the coeffs in l have been precomputed by series_prepare.
		 * S += (2*l+1)*pp/(l*(l+1)*(l*(l+1)+pp)) * P1;
		 * P2 = x*(2*l+1)/(l+1) * P1 - l*P0/(l+1); */
		S += Lz->A[l] * P1;	/* Update sum */
		P2 = x * Lz->B[l] * P1 - Lz->C[l] * P0;	/* Get next Legendre polynomial value */
		P0 = P1;
		P1 = P2;
	}

	return (S);
}

double gradspline2d_Wessel_Becker_Revised (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *Lg)
{	/* Evaluate g = -M_PI * (v+1)*[x*Pv(-x)+Pv+1(-x)]/(sin (v*x)*sin(theta)) - 1/(1-x) via series approximation */
	unsigned int L_max, l;
	double sin_theta, P0, P1, P2, S;
	GMT_UNUSED(GMT);

	if (fabs(x) == 1.0) return 1.0;
	L_max = get_L (x, par[0], par[1]);	/* Highest order needed in sum given x, p, and err */
	sin_theta = sqrt (1.0 - x * x);
	P0 = 1.0;	/* Initialize the P0 and P1 Legendre polynomials */
	P1 = x;
	S = 0.0;	/* Initialize sum */

	/* Sum the Legendre series */
	for (l = 1; l <= L_max; l++) {
		/* All the coeffs in l have been precomputed by series_prepare.
		 * S += (2*l+1)*pp/((l+1)*(l*(l+1)+pp)) * (P1 * x - P0);
		 * P2 = x*(2*l+1)/(l+1) * P1 - l*P0/(l+1); */
		S += Lg->A[l] * (P1 * x - P0);		/* Update sum */
		P2 = x * Lg->B[l] * P1 - Lg->C[l] * P0;	/* Get next Legendre polynomial value */
		P0 = P1;
		P1 = P2;
	}
	S /= sin_theta;

	return (S);
}

/* Given the lookup tables, this is how we use these functions
 * Here, par[7]  is number of points in spline
 *	 par[8]  is spline spacing dx
 *	 par[9]  is inverse, 1/dx
 *	 par[10] is min x
 */

double csplint (double *y, double *c, double b, double h2, uint64_t klo)
{	/* Special version of GMT_csplint where x is equidistant with spacing squared h2
 	 * and b is the fractional distance relative to x[klo], so x itself is not needed here. */
	uint64_t khi;
	double a, yp;

	khi = klo + 1;
	a = 1.0 - b;	/* Fractional distance from next node */
	yp = a * y[klo] + b * y[khi] + ((a*a*a - a) * c[klo] + (b*b*b - b) * c[khi]) * h2 / 6.0;

	return (yp);
}

double spline2d_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L)
{	/* Given x, look up nearest node xx[k] <= x and do cubic spline interpolation */
	uint64_t k;
	double f, f0, df, y;
	GMT_UNUSED(GMT);

	f = (x - par[10]) * par[9];	/* Floating point index */
	f0 = floor (f);
	df = f - f0;
	k = lrint (f0);
	if (df == 0.0) return (L->y[k]);	/* Right on a node */
	y = csplint (L->y, L->c, df, par[4], k);	/* Call special cubic spline evaluator */
	return (y);
}

double spline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L)
{
	return (spline2d_lookup (GMT, x, par, L));
}

double gradspline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L)
{
	return (spline2d_lookup (GMT, x, par, L));
}

void spline2d_Wessel_Becker_splineinit (struct GMT_CTRL *GMT, double par[], double *x, struct GREENSPLINE_LOOKUP *L)
{	/* Set up cubic spline interpolation given the precomputed x,y values of the function */
	GMT_UNUSED(GMT);
	L->c = GMT_memory (GMT, NULL, 3*L->n, double);
	GMT_cspline (GMT, x, L->y, L->n, L->c);
}

void spline2d_Wessel_Becker_init (struct GMT_CTRL *GMT, double par[], struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg)
{
	uint64_t i, nx;
	double *x = NULL;
#ifdef DUMP
	FILE *fp = NULL;
	uint64_t n_out;
	double out[3];
	fp = fopen ("greenspline.b", "wb");
	n_out = (Lg) ? 3 : 2;
#endif
	nx = lrint (par[7]);
	Lz->n = nx;
	x = GMT_memory (GMT, NULL, nx, double);
	Lz->y = GMT_memory (GMT, NULL, nx, double);
	if (Lg) {
		Lg->y = GMT_memory (GMT, NULL, nx, double);
		Lg->n = nx;
	}
	for (i = 0; i < nx; i++) {
		x[i] = par[10] + i * par[8];
		Lz->y[i] = spline2d_Wessel_Becker_Revised (GMT, x[i], par, Lz);
		if (Lg) Lg->y[i] = gradspline2d_Wessel_Becker_Revised (GMT, x[i], par, Lg);
#ifdef DUMP
		out[0] = x[i];	out[1] = Lz->y[i];	if (Lg) out[2] = Lg->y[i];
		fwrite (out, sizeof (double), n_out, fp);
#endif
	}
#ifdef DUMP
	fclose (fp);
#endif
	spline2d_Wessel_Becker_splineinit (GMT, par, x, Lz);
	if (Lg) {
		if (x[0] == -1.0) Lg->y[0] = 2.0*Lg->y[1] - Lg->y[2];	/* Linear interpolation from 2 nearest nodes */
		spline2d_Wessel_Becker_splineinit (GMT, par, x, Lg);
	}
	GMT_free (GMT, x);	/* Done with x array */
}

/*----------------------  THREE DIMENSIONS ---------------------- */

/* spline3d_sandwell computes the Green function for a 3-d spline
 * as per Sandwell [1987], G(r) = r.  All r must be >= 0.
 */

double spline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	GMT_UNUSED(GMT); GMT_UNUSED(par); GMT_UNUSED(unused);
	if (r == 0.0) return (0.0);

	return (r);	/* Just regular spline; par not used */
}

double gradspline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	GMT_UNUSED(GMT); GMT_UNUSED(r); GMT_UNUSED(par); GMT_UNUSED(unused);
	return (1.0);	/* Just regular spline; par not used */
}

double spline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
/* spline1d_Wessel_Bercovici computes the Green function for a 3-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = [exp(-u) -1]/u,
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * All r must be >= 0. par[0] = c
 */
{
	double cx;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (((exp (-cx) - 1.0) / cx) + 1.0);
}

double gradspline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	double cx;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return ((1.0 - exp (-cx) * (cx + 1.0)) / (cx * r));
}

/* spline3d_Mitasova_Mitas computes the regularized Green function for a 3-d spline
 * in tension as per Mitasova and Mitas [1993], G(u) = erf (u/2)/u - 1/sqrt(pi),
 * where u = par[0] * r. All r must be >= 0. par[0] = phi
 */

double spline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	double x;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	x = par[0] * r;
	return ((erf (0.5 * x) / x) - M_INV_SQRT_PI);
}

double gradspline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused)
{
	double u, dgdr;
	GMT_UNUSED(GMT); GMT_UNUSED(unused);

	if (r == 0.0) return (0.0);

	u = par[0] * r;
	dgdr = ((u/M_SQRT_PI) * exp (-u*u) - erf (0.5 * u)) / (u * r);
	return (dgdr);
}

/* GENERAL NUMERICAL FUNCTIONS */

/* Normalization parameters are stored in the coeff array which holds up to GSP_LENGTH terms
 * coeff[GSP_MEAN_X]:	The mean x coordinate
 * coeff[GSP_MEAN_Y]:	The mean y coordinate
 * coeff[GSP_MEAN_Z]:	The mean w coordinate
 * coeff[GSP_SLP_X]:	The linear x slope
 * coeff[GSP_SLP_Y]:	The linear y slope
 * coeff[GSP_RANGE]:	The largest |range| of the detrended data
 */

double undo_normalization (double *X, double w_norm, unsigned int mode, double *coeff, unsigned int dim)
{
	if (mode & GREENSPLINE_NORM) w_norm *= coeff[GSP_RANGE];	/* Scale back up by residual data range (ir we normalized) */
	w_norm += coeff[GSP_MEAN_Z];					/* Add in mean data value plus minimum residual value (ir we normalized by range) */
	if (mode & GREENSPLINE_TREND) {					/* Restore residual trend */
		w_norm += coeff[GSP_SLP_X] * (X[GMT_X] - coeff[GSP_MEAN_X]);
		if (dim == 2) w_norm += coeff[GSP_SLP_Y] * (X[GMT_Y] - coeff[GSP_MEAN_Y]);
	}
	return (w_norm);
}

void do_normalization_1d (struct GMTAPI_CTRL *API, double **X, double *obs, uint64_t n, unsigned int mode, double *coeff)
{	/* We always remove/restore the mean observation value.  mode is a combination of bitflags that affects what we do:
	 * Bit GREENSPLINE_TREND will also remove linear trend
	 * Bit GREENSPLINE_NORM will normalize residuals by range
	 */

	uint64_t i;
	double d, min = DBL_MAX, max = -DBL_MAX;

	GMT_memset (coeff, GSP_LENGTH, double);
	for (i = 0; i < n; i++) {	/* Find mean w-value */
		coeff[GSP_MEAN_Z] += obs[i];
		if ((mode & GREENSPLINE_TREND) == 0) continue;	/* No linear trend to model */
		coeff[GSP_MEAN_X] += X[i][GMT_X];
	}
	coeff[GSP_MEAN_Z] /= n;

	if (mode & GREENSPLINE_TREND) {	/* Solve for LS linear trend using deviations from (0, 0, 0) */
		double	xx, zz, sxx, sxz;
		sxx = sxz = 0.0;
		coeff[GSP_MEAN_X] /= n;
		for (i = 0; i < n; i++) {
			xx = X[i][GMT_X] - coeff[GSP_MEAN_X];
			zz = obs[i] - coeff[GSP_MEAN_Z];
			sxx += (xx * xx);
			sxz += (xx * zz);
		}
		if (sxx != 0.0) coeff[GSP_SLP_X] = sxz/ sxx;
	}

	/* Remove linear trend (or mean) */

	for (i = 0; i < n; i++) {	/* Get residuals and find range */
		obs[i] -= coeff[GSP_MEAN_Z];	/* Always remove the mean data value */
		if (mode & GREENSPLINE_TREND) obs[i] -= (coeff[GSP_SLP_X] * (X[i][GMT_X] - coeff[GSP_MEAN_X]));
		if (obs[i] < min) min = obs[i];
		if (obs[i] > max) max = obs[i];
	}
	if (mode & GREENSPLINE_NORM) {	/* Normalize by range */
		coeff[GSP_RANGE] = MAX (fabs(min), fabs(max));	/* Determine range */
		d = (coeff[GSP_RANGE] == 0.0) ? 1.0 : 1.0 / coeff[GSP_RANGE];
		for (i = 0; i < n; i++) obs[i] *= d;	/* Normalize 0-1 */
	}

	/* Recover obs(x) = w_norm(x) * coeff[GSP_RANGE] + coeff[GSP_MEAN_Z] + coeff[GSP_SLP_X]*(x-coeff[GSP_MEAN_X]) */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "1-D Normalization coefficients: zoff = %g slope = %g xmean = %g range = %g\n", coeff[GSP_MEAN_Z], coeff[GSP_SLP_X], coeff[GSP_MEAN_X], coeff[GSP_RANGE]);
}

void do_normalization (struct GMTAPI_CTRL *API, double **X, double *obs, uint64_t n, unsigned int mode, unsigned int dim, double *coeff)
{	/* We always remove/restore the mean observation value.  mode is a combination of bitflags that affects what we do:
	 * Bit GREENSPLINE_TREND will also remove linear trend
	 * Bit GREENSPLINE_NORM will normalize residuals by range
	 */

	uint64_t i;
	double d, min = DBL_MAX, max = -DBL_MAX;

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Normalization mode: %d\n", mode);
	if (dim == 1) {	/* 1-D trend or mean only */
		do_normalization_1d (API, X, obs, n, mode, coeff);
		return;
	}
	GMT_memset (coeff, GSP_LENGTH, double);
	for (i = 0; i < n; i++) {	/* Find mean z-value */
		coeff[GSP_MEAN_Z] += obs[i];
		if ((mode & GREENSPLINE_TREND) == 0) continue;	/* Else we also sum up x and y to get their means */
		coeff[GSP_MEAN_X] += X[i][GMT_X];
		coeff[GSP_MEAN_Y] += X[i][GMT_Y];
	}
	coeff[GSP_MEAN_Z] /= n;	/* Average z value to remove/restore */

	if (mode & GREENSPLINE_TREND) {	/* Solve for LS plane using deviations from (0, 0, 0) */
		double	xx, yy, zz, sxx, sxy, sxz, syy, syz;
		sxx = sxy = sxz = syy = syz = 0.0;
		coeff[GSP_MEAN_X] /= n;	/* Mean x */
		coeff[GSP_MEAN_Y] /= n;	/* Mean y */
		for (i = 0; i < n; i++) {

			xx = X[i][GMT_X] - coeff[GSP_MEAN_X];
			yy = X[i][GMT_Y] - coeff[GSP_MEAN_Y];
			zz = obs[i] - coeff[GSP_MEAN_Z];
			/* xx,yy,zz are residuals relative to (0,0,0) */
			sxx += (xx * xx);
			sxz += (xx * zz);
			sxy += (xx * yy);
			syy += (yy * yy);
			syz += (yy * zz);
		}

		d = sxx*syy - sxy*sxy;
		if (d != 0.0) {
			coeff[GSP_SLP_X] = (sxz*syy - sxy*syz)/d;
			coeff[GSP_SLP_Y] = (sxx*syz - sxy*sxz)/d;
		}
	}

	/* Remove plane (or just mean) */

	for (i = 0; i < n; i++) {	/* Also find min/max or residuals in the process */
		obs[i] -= coeff[GSP_MEAN_Z];	/* Always remove mean data value */
		if (mode & GREENSPLINE_TREND) obs[i] -= (coeff[GSP_SLP_X] * (X[i][GMT_X] - coeff[GSP_MEAN_X]) + coeff[GSP_SLP_Y] * (X[i][GMT_Y] - coeff[GSP_MEAN_Y]));
		if (obs[i] < min) min = obs[i];
		if (obs[i] > max) max = obs[i];
	}
	if (mode & GREENSPLINE_NORM) {	/* Normalize by range */
		coeff[GSP_RANGE] = MAX (fabs(min), fabs(max));	/* Determine range */
		d = (coeff[GSP_RANGE] == 0.0) ? 1.0 : 1.0 / coeff[GSP_RANGE];
		for (i = 0; i < n; i++) obs[i] *= d;	/* Normalize 0-1 */
	}

	/* Recover obs(x,y) = w_norm(x,y) * coeff[GSP_RANGE] + coeff[GSP_MEAN_Z] + coeff[GSP_SLP_X]*(x-coeff[GSP_MEAN_X]) + coeff[GSP_SLP_Y]*(y-coeff[GSP_MEAN_Y]) */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "2-D Normalization coefficients: zoff = %g xslope = %g xmean = %g yslope = %g ymean = %g range = %g\n",
		coeff[GSP_MEAN_Z], coeff[GSP_SLP_X], coeff[GSP_MEAN_X], coeff[GSP_SLP_Y], coeff[GSP_MEAN_Y], coeff[GSP_RANGE]);
}

double get_radius (struct GMT_CTRL *GMT, double *X0, double *X1, unsigned int dim)
{
	double r = 0.0;
	/* Get distance between the two points */
	switch (dim) {
		case 1:	/* 1-D, just get x difference */
			r = fabs (X0[GMT_X] - X1[GMT_X]);
			break;
		case 2:	/* 2-D Cartesian or spherical surface in meters */
			r = GMT_distance (GMT, X0[GMT_X], X0[GMT_Y], X1[GMT_X], X1[GMT_Y]);
			break;
		case 3:	/* 3-D Cartesian */
			r = hypot (X0[GMT_X] - X1[GMT_X], X0[GMT_Y] - X1[GMT_Y]);
			r = hypot (r, X0[GMT_Z] - X1[GMT_Z]);
			break;
	}
	return (r);
}

double get_dircosine (struct GMT_CTRL *GMT, double *D, double *X0, double *X1, unsigned int dim, bool baz)
{
	/* D, the directional cosines of the observed gradient:
	 * X0: Observation point.
	 * X1: Prediction point.
	 * Compute N, the direction cosine of X1-X2, then C = D dot N.
	 */
	int ii;
	double az, C = 0.0, N[3];

	switch (dim) {
		case 1:	/* 1-D, always 1 */
			C = 1.0;
			break;
		case 2:	/* 2-D */
			az = GMT_az_backaz (GMT, X0[GMT_X], X0[GMT_Y], X1[GMT_X], X1[GMT_Y], baz);
			sincosd (az, &N[GMT_X], &N[GMT_Y]);
			for (ii = 0; ii < 2; ii++) C += D[ii] * N[ii];	/* Dot product of 2-D unit vectors */
			break;
		case 3:	/* 3-D */
			for (ii = 0; ii < 3; ii++) N[ii] = X1[ii] - X0[ii];	/* Difference vector */
			GMT_normalize3v (GMT, N);	/* Normalize to unit vector */
			C = GMT_dot3v (GMT, D, N);	/* Dot product of 3-D unit vectors */
			if (baz) C = -C;		/* The opposite direction for X0-X1 */
			break;
	}
	return (C);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_greenspline_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_greenspline (void *V_API, int mode, void *args)
{
	uint64_t col, row, n_read, p, k, i, j, seg, m, n, nm, n_ok = 0, ij, ji, ii, n_duplicates = 0, n_skip = 0;
	unsigned int dimension = 0, normalize, unit = 0, n_cols, L_Max = 0;
	size_t old_n_alloc, n_alloc;
	int error, out_ID, way, nx;
	bool delete_grid = false, check_longitude, skip;

	char *method[N_METHODS] = {"minimum curvature Cartesian spline [1-D]",
		"minimum curvature Cartesian spline [2-D]",
		"minimum curvature Cartesian spline [3-D]",
		"continuous curvature Cartesian spline in tension [1-D]",
		"continuous curvature Cartesian spline in tension [2-D]",
		"continuous curvature Cartesian spline in tension [3-D]",
		"regularized Cartesian spline in tension [2-D]",
		"regularized Cartesian spline in tension [3-D]",
		"minimum curvature spherical spline",
		"continuous curvature spherical spline in tension",
		"linear Cartesian spline [1-D]",
		"bilinear Cartesian spline [2-D]"};
	char *mem_unit[3] = {"kb", "Mb", "Gb"};

	double *obs = NULL, **D = NULL, **X = NULL, *alpha = NULL, *in = NULL;
	double mem, part, C, p_val, r, par[N_PARAMS], norm[GSP_LENGTH], az, grad, weight_i, weight_j;
	double *A = NULL, r_min, r_max;
#ifdef DEBUG
	double x0 = 0.0, x1 = 5.0;
#endif

	FILE *fp = NULL;

	double (*G) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *) = NULL;		/* Pointer to chosen Green's function */
	double (*dGdr) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *) = NULL;	/* Pointer to chosen gradient of Green's function */


	struct GMT_GRID *Grid = NULL, *Out = NULL;
	struct ZGRID Z;
	struct GREENSPLINE_LOOKUP *Lz = NULL, *Lg = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *Nin = NULL;
	struct GMT_GRID_INFO info;
	struct GREENSPLINE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_greenspline_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_greenspline_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_greenspline_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_greenspline_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_greenspline_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the greenspline main code ----------------------------*/

	GMT_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	dimension = (Ctrl->D.mode == 0) ? 1 : ((Ctrl->D.mode == 5) ? 3 : 2);
	GMT_memset (par,   N_PARAMS, double);
	GMT_memset (norm,  GSP_LENGTH, double);
	GMT_memset (&info, 1, struct GMT_GRID_INFO);
	GMT_memset (&Z,    1, struct ZGRID);

	if (Ctrl->S.mode == SANDWELL_1987_1D || Ctrl->S.mode == WESSEL_BERCOVICI_1998_1D) Ctrl->S.mode += (dimension - 1);
	if (Ctrl->S.mode == LINEAR_1D) Ctrl->S.mode += (dimension - 1);
	if (Ctrl->S.mode == MITASOVA_MITAS_1993_2D ) Ctrl->S.mode += (dimension - 2);

	way = GMT_IS_SPHERICAL (GMT) ? GMT_GREATCIRCLE : GMT_GEODESIC;
	Ctrl->D.mode--;	/* Since I added 0 to be 1-D later so now it is -1 */
	switch (Ctrl->D.mode) {	/* Set pointers to 2-D distance functions */
		case -1:	/* Cartesian 1-D x data */
			normalize = GREENSPLINE_TREND + GREENSPLINE_NORM;
			break;
		case 0:	/* Cartesian 2-D x,y data */
			GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);
			normalize = GREENSPLINE_TREND + GREENSPLINE_NORM;
			break;
		case 1:	/* 2-D lon, lat data, but scale to Cartesian flat earth km */
			GMT_set_geographic (GMT, GMT_IN);
			GMT_set_geographic (GMT, GMT_OUT);
			GMT_init_distaz (GMT, 'k', GMT_FLATEARTH, GMT_MAP_DIST);
			normalize = GREENSPLINE_TREND + GREENSPLINE_NORM;
			break;
		case 2:	/* 2-D lon, lat data, use spherical distances in km (geodesic if PROJ_ELLIPSOID is nor sphere) */
			GMT_set_geographic (GMT, GMT_IN);
			GMT_set_geographic (GMT, GMT_OUT);
			GMT_init_distaz (GMT, 'k', way, GMT_MAP_DIST);
			normalize = GREENSPLINE_NORM;
			break;
		case 3:	/* 2-D lon, lat data, and Green's function needs cosine of spherical or geodesic distance */
			GMT_set_geographic (GMT, GMT_IN);
			GMT_set_geographic (GMT, GMT_OUT);
			GMT_init_distaz (GMT, 'S', way, GMT_MAP_DIST);
			normalize = GREENSPLINE_NORM;
			break;
		case 4:	/* 3-D Cartesian x,y,z data handled separately */
			normalize = GREENSPLINE_NORM;
			break;
		default:	/* Cannot happen unless we make a bug */
			GMT_Report (API, GMT_MSG_NORMAL, "BUG since D (=%d) cannot be outside 0-5 range\n", Ctrl->D.mode+1);
			break;
	}

	if (Ctrl->D.mode <= 1 && Ctrl->L.active)
		normalize = GREENSPLINE_NORM;	/* Do not de-plane, just remove mean and normalize */
	else if (Ctrl->D.mode > 1 && Ctrl->L.active)
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: -L ignored for -D modes 2 and 3\n");

	if (Ctrl->Q.active && dimension == 2) sincosd (Ctrl->Q.az, &Ctrl->Q.dir[GMT_X], &Ctrl->Q.dir[GMT_Y]);

	/* Now we are ready to take on some input values */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	X = GMT_memory (GMT, NULL, n_alloc, double *);
	n_cols = (Ctrl->W.active) ? dimension + 1 : dimension;	/* So X[k][dimension] holds the weight if -W is active */
	for (k = 0; k < n_alloc; k++) X[k] = GMT_memory (GMT, NULL, n_cols, double);
	obs = GMT_memory (GMT, NULL, n_alloc, double);
	check_longitude = (dimension == 2 && (Ctrl->D.mode == 1 || Ctrl->D.mode == 2));

	GMT_Report (API, GMT_MSG_VERBOSE, "Read input data and check for data constraint duplicates\n");
	n = m = n_read = 0;
	r_min = DBL_MAX;	r_max = -DBL_MAX;
	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

		if (check_longitude) {
			/* Ensure geographic longitudes fit the range since the normalization function expects it */
			if (in[GMT_X] < Ctrl->R3.range[XLO] && (in[GMT_X] + 360.0) < Ctrl->R3.range[XHI]) in[GMT_X] += 360.0;
			else if (in[GMT_X] > Ctrl->R3.range[XHI] && (in[GMT_X] - 360.0) > Ctrl->R3.range[XLO]) in[GMT_X] -= 360.0;
		}

		for (k = 0; k < n_cols; k++) X[n][k] = in[k];	/* Get coordinates + optional weights (if -W) */
		/* Check for duplicates */
		skip = false;
		for (i = 0; !skip && i < n; i++) {
			r = get_radius (GMT, X[i], X[n], dimension);
			if (GMT_IS_ZERO (r)) {	/* Duplicates will give zero point separation */
				if (doubleAlmostEqualZero (in[dimension], obs[i])) {
					GMT_Report (API, GMT_MSG_NORMAL, "Data constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i);
					skip = true;
					n_skip++;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Data constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ in observation (%.12g vs %.12g)\n", n_read, i, in[dimension], obs[i]);
					n_duplicates++;
				}
			}
			else {
				if (r < r_min) r_min = r;
				if (r > r_max) r_max = r;
			}
		}
		n_read++;
		if (skip) continue;	/* Current point was a duplicate of a previous point */

		obs[n++] = in[dimension];

		if (n == n_alloc) {	/* Get more memory */
			old_n_alloc = n_alloc;
			n_alloc <<= 1;
			X = GMT_memory (GMT, X, n_alloc, double *);
			for (k = old_n_alloc; k < n_alloc; k++) X[k] = GMT_memory (GMT, X[k], n_cols, double);
			obs = GMT_memory (GMT, obs, n_alloc, double);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	for (k = n; k < n_alloc; k++) GMT_free (GMT, X[k]);	/* Remove what was not used */
	X = GMT_memory (GMT, X, n, double *);
	obs = GMT_memory (GMT, obs, n, double);
	nm = n;
	GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " unique data constraints\n", n);
	if (n_skip) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %" PRIu64 " data constraints as duplicates\n", n_skip);

	if (Ctrl->A.active) {	/* Read gradient constraints from file */
		struct GMT_DATASET *Din = NULL;
		struct GMT_DATATABLE *S = NULL;
		if (GMT->common.b.active[GMT_IN]) GMT->common.b.ncol[GMT_IN]++;	/* Must assume it is just one extra column */
		if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->A.file, NULL)) == NULL) {
			Return (API->error);
		}
		S = Din->table[0];	/* Can only be one table */
		m = S->n_records;	/* Total number of gradient constraints */
		nm += m;		/* New total of linear equations to solve */
		X = GMT_memory (GMT, X, nm, double *);
		for (k = n; k < nm; k++) X[k] = GMT_memory (GMT, NULL, n_cols, double);
		obs = GMT_memory (GMT, obs, nm, double);
		D = GMT_memory (GMT, NULL, m, double *);
		for (k = 0; k < m; k++) D[k] = GMT_memory (GMT, NULL, n_cols, double);
		n_skip = n_read = 0;
		for (seg = k = 0, p = n; seg < S->n_segments; seg++) {
			for (row = 0; row < S->segment[seg]->n_rows; row++, k++, p++) {
				for (ii = 0; ii < n_cols; ii++) X[p][ii] = S->segment[seg]->coord[ii][row];
				switch (dimension) {
					case 1:	/* 1-D */
						switch (Ctrl->A.mode) {
							case 0:	/* x, slope */
								D[k][0] = 1.0;	/* Dummy since there is no direction for 1-D spline (the gradient is in the x-y plane) */
								obs[p] = S->segment[seg]->coord[dimension][row];
								break;
							default:
								GMT_Report (API, GMT_MSG_NORMAL, "Bad gradient mode selected for 1-D data (%d) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						break;
					case 2:	/* 2-D */
						switch (Ctrl->A.mode) {
							case 1:	/* (x, y, az, gradient) */
								az = D2R * S->segment[seg]->coord[2][row];
								obs[p] = S->segment[seg]->coord[3][row];
								break;
							case 2:	/* (x, y, gradient, azimuth) */
								az = D2R * S->segment[seg]->coord[3][row];
								obs[p] = S->segment[seg]->coord[2][row];
								break;
							case 3:	/* (x, y, direction, gradient) */
								az = M_PI_2 - D2R * S->segment[seg]->coord[2][row];
								obs[p] = S->segment[seg]->coord[3][row];
								break;
							case 4:	/* (x, y, gx, gy) */
								az = atan2 (S->segment[seg]->coord[2][row], S->segment[seg]->coord[3][row]);		/* Get azimuth of gradient */
								obs[p] = hypot (S->segment[seg]->coord[3][row], S->segment[seg]->coord[3][row]);	/* Get magnitude of gradient */
								break;
							case 5:	/* (x, y, nx, ny, gradient) */
								az = atan2 (S->segment[seg]->coord[2][row], S->segment[seg]->coord[3][row]);		/* Get azimuth of gradient */
								obs[p] = S->segment[seg]->coord[4][row];	/* Magnitude of gradient */
								break;
							default:
								GMT_Report (API, GMT_MSG_NORMAL, "Bad gradient mode selected for 2-D data (%d) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						sincos (az, &D[k][GMT_X], &D[k][GMT_Y]);
						break;
					case 3:	/* 3-D */
						switch (Ctrl->A.mode) {
							case 4:	/* (x, y, z, gx, gy, gz) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = S->segment[seg]->coord[3+ii][row];	/* Get the gradient vector */
								obs[p] = GMT_mag3v (GMT, D[k]);	/* This is the gradient magnitude */
								GMT_normalize3v (GMT, D[k]);		/* These are the direction cosines of the gradient */
								break;
							case 5: /* (x, y, z, nx, ny, nz, gradient) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = S->segment[seg]->coord[3+ii][row];	/* Get the unit vector */
								obs[p] = S->segment[seg]->coord[6][row];	/* This is the gradient magnitude */
								break;
							default:
								GMT_Report (API, GMT_MSG_NORMAL, "Bad gradient mode selected for 3-D data (%d) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Bad dimension selected (%d) - aborting!\n", dimension);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
				/* Check for duplicates */
				skip = false;
				for (i = n; !skip && i < p; i++) {
					r = get_radius (GMT, X[i], X[p], dimension);
					if (GMT_IS_ZERO (r)) {	/* Duplicates will give zero point separation */
						if (doubleAlmostEqualZero (in[dimension], obs[i])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Slope constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i-n);
							skip = true;
							n_skip++;
						}
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Slope constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ in observation (%.12g vs %.12g)\n", n_read, i-n, obs[p], obs[i]);
							n_duplicates++;
						}
					}
					else {
						if (r < r_min) r_min = r;
						if (r > r_max) r_max = r;
					}
				}
				n_read++;
				if (skip) p--;	/* Current point was a duplicate of a previous point; reduce p since it will be incremented in the loop */
			}
		}
		if (GMT_Destroy_Data (API, &Din) != GMT_OK) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " unique slope constraints\n", m);
		if (n_skip) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %" PRIu64 " slope constraints as duplicates\n", n_skip);
	}

	/* Check for duplicates which would result in a singular matrix system; also update min/max radius */

	GMT_Report (API, GMT_MSG_VERBOSE, "Distance between closest constraints = %.12g]\n", r_min);
	GMT_Report (API, GMT_MSG_VERBOSE, "Distance between distant constraints = %.12g]\n", r_max);

	if (n_duplicates) {	/* These differ in observation value so need to be averaged, medianed, or whatever first */
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
		if (!Ctrl->C.active || GMT_IS_ZERO (Ctrl->C.value)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "You must reconcile duplicates before running greenspline since they will result in a singular matrix\n");
			for (p = 0; p < nm; p++) GMT_free (GMT, X[p]);
			GMT_free (GMT, X);
			GMT_free (GMT, obs);
			if (m) {
				for (p = 0; p < m; p++) GMT_free (GMT, D[p]);
				GMT_free (GMT, D);
			}
			Return (GMT_DATA_READ_ERROR);
		}
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "Expect some eigenvalues to be identically zero\n");
	}

	if (m > 0 && (normalize & GREENSPLINE_TREND)) {
		normalize = GREENSPLINE_NORM;	/* Only allow taking out data mean for mixed z/slope data */
		GMT_Report (API, GMT_MSG_NORMAL, "Only remove/restore mean z in mixed {z, grad(z)} data sets\n");
	}

	if (m == 0)
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " data points, yielding a %" PRIu64 " by %" PRIu64 " set of linear equations\n", n, nm, nm);
	else
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " data points and %" PRIu64 " gradients, yielding a %" PRIu64 " by %" PRIu64 " set of linear equations\n", n, m, nm, nm);

	if (Ctrl->T.file) {	/* Existing grid that will have zeros and NaNs, only */
		if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->T.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		if (! (Grid->header->wesn[XLO] == Ctrl->R3.range[0] && Grid->header->wesn[XHI] == Ctrl->R3.range[1] && Grid->header->wesn[YLO] == Ctrl->R3.range[2] && Grid->header->wesn[YHI] == Ctrl->R3.range[3])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The mask grid does not match your specified region\n");
			Return (EXIT_FAILURE);
		}
		if (! (Grid->header->inc[GMT_X] == Ctrl->I.inc[GMT_X] && Grid->header->inc[GMT_Y] == Ctrl->I.inc[GMT_Y])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The mask grid resolution does not match your specified grid spacing\n");
			Return (EXIT_FAILURE);
		}
		if (! (Grid->header->registration == GMT->common.r.registration)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The mask grid registration does not match your specified grid registration\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->T.file, Grid) == NULL) {	/* Get data */
			Return (API->error);
		}
		(void)GMT_set_outgrid (GMT, Ctrl->T.file, Grid, &Out);	/* true if input is a read-only array; otherwise Out is just a pointer to Grid */
		n_ok = Grid->header->nm;
		GMT_grd_loop (GMT, Grid, row, col, ij) if (GMT_is_fnan (Grid->data[ij])) n_ok--;
		Z.nz = 1;
	}
	else if (Ctrl->N.active) {	/* Read output locations from file */
		if ((Nin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			Return (API->error);
		}
		T = Nin->table[0];
	}
	else {	/* Fill in an equidistant output table or grid */
		if (dimension == 2) {	/* Need a full-fledged Grid creation since we are writing it to who knows where */
			if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->R3.range, Ctrl->I.inc, \
				GMT->common.r.registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			n_ok = Grid->header->nm;
			Z.nz = 1;	/* So that output logic will work for 1-D */
		}
		else {	/* Just a temporary internal grid created and destroyed withing greenspline */
			if ((Grid = GMT_create_grid (GMT)) == NULL) Return (API->error);
			delete_grid = true;
			Grid->header->wesn[XLO] = Ctrl->R3.range[0];	Grid->header->wesn[XHI] = Ctrl->R3.range[1];
			Grid->header->registration = GMT->common.r.registration;
			Grid->header->inc[GMT_X] = Ctrl->I.inc[GMT_X];
			Z.nz = Grid->header->ny = 1;	/* So that output logic will work for 1-D */
			if (dimension == 3) {
				Grid->header->wesn[YLO] = Ctrl->R3.range[2];	Grid->header->wesn[YHI] = Ctrl->R3.range[3];
				Grid->header->inc[GMT_Y] = Ctrl->I.inc[GMT_Y];
				GMT_RI_prepare (GMT, Grid->header);	/* Ensure -R -I consistency and set nx, ny */
				GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);
				GMT_set_grddim (GMT, Grid->header);
				/* Also set nz */
				Z.z_min = Ctrl->R3.range[4];	Z.z_max = Ctrl->R3.range[5];
				Z.z_inc = Ctrl->I.inc[GMT_Z];
				Z.nz = GMT_get_n (GMT, Z.z_min, Z.z_max, Z.z_inc, Grid->header->registration);
				n_ok = Grid->header->nm * Z.nz;
				Grid->data = GMT_memory_aligned (GMT, NULL, Grid->header->size * Z.nz, float);
			}
			else	/* Just 1-D */
				n_ok = Grid->header->nx = GMT_grd_get_nx (GMT, Grid->header);
		}
		Out = Grid;	/* Just point since we created Grid */
	}

	switch (Ctrl->S.mode) {	/* Assing pointers to Green's functions and the gradient and set up required parameters */
		case LINEAR_1D:
		case LINEAR_2D:
			G = &spline1d_linear;
			dGdr = &gradspline1d_linear;
			break;
		case SANDWELL_1987_1D:
			G = &spline1d_sandwell;
			dGdr = &gradspline1d_sandwell;
			break;
		case SANDWELL_1987_2D:
			G = &spline2d_sandwell;
			dGdr = &gradspline2d_sandwell;
			break;
		case SANDWELL_1987_3D:
			G = &spline3d_sandwell;
			dGdr = &gradspline3d_sandwell;
			break;
		case WESSEL_BERCOVICI_1998_1D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = Grid->header->inc[GMT_X];
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = &spline1d_Wessel_Bercovici;
			dGdr = &gradspline1d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_2D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = 0.5 * (Grid->header->inc[GMT_X] + Grid->header->inc[GMT_Y]);
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = &spline2d_Wessel_Bercovici;
			dGdr = &gradspline2d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_3D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = (Grid->header->inc[GMT_X] + Grid->header->inc[GMT_Y] + Z.z_inc) / 3.0;
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = &spline3d_Wessel_Bercovici;
			dGdr = &gradspline3d_Wessel_Bercovici;
			break;
		case MITASOVA_MITAS_1993_2D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_Report (API, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = &spline2d_Mitasova_Mitas;
			dGdr = &gradspline2d_Mitasova_Mitas;
			break;
		case MITASOVA_MITAS_1993_3D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_Report (API, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = &spline3d_Mitasova_Mitas;
			dGdr = &gradspline3d_Mitasova_Mitas;
			break;
		case PARKER_1994:
			par[0] = 6.0 / (M_PI*M_PI);
			G = &spline2d_Parker;
			dGdr = &gradspline2d_Parker;
#ifdef DEBUG
			if (TEST) x0 = -1.0, x1 = 1.0;
#endif
			break;
		case WESSEL_BECKER_2008:
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0]));	/* The p value */
			par[1] = Ctrl->S.value[2];	/* The truncation error */
			par[2] = -log (2.0) + (par[0]*par[0] - 1.0) / (par[0]*par[0]);	/* Precalculate the constant for the l = 0 term here */
			Lz = GMT_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
#ifdef DEBUG
			if (TEST) Lg = GMT_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
			else
#endif
			if (Ctrl->A.active) Lg = GMT_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
			L_Max = get_max_L (GMT, par[0], par[1]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "New scheme p = %g, err = %g, L_Max = %u\n", par[0], par[1], L_Max);
			series_prepare (GMT, par[0], L_Max, Lz, Lg);
			/* Set up the cubic spline lookup/interpolation */
			par[7] = Ctrl->S.value[3];
			nx = irint (par[7]);
			par[8] = (Ctrl->S.rval[1] - Ctrl->S.rval[0]) / (par[7] - 1.0);
			par[9] = 1.0 / par[8];
			par[10] = Ctrl->S.rval[0];
			par[4] = par[8] * par[8];	/* Spline spacing squared, needed by csplint */

			GMT_Report (API, GMT_MSG_VERBOSE, "Precalculate -Sq lookup table with %d items from %g to %g\n", nx, Ctrl->S.rval[0], Ctrl->S.rval[1]);
			spline2d_Wessel_Becker_init (GMT, par, Lz, Lg);
			G = &spline2d_Wessel_Becker_lookup;
			dGdr = &gradspline2d_Wessel_Becker_lookup;
#ifdef DEBUG
			if (TEST) x0 = -1.0, x1 = 1.0;
#endif
			break;
	}

#ifdef DEBUG
	if (TEST) {
		GMT_Report (API, GMT_MSG_VERBOSE, "greenspline running in TEST mode for %s\n", method[Ctrl->S.mode]);
		printf ("# %s\n#x\tG\tdG/dx\tt\n", method[Ctrl->S.mode]);
		dump_green (GMT, G, dGdr, par, x0, x1, 10001, Lz, Lg);
		GMT_free_grid (GMT, &Grid, dimension == 2);
		for (p = 0; p < nm; p++) GMT_free (GMT, X[p]);
		free_lookup (GMT, &Lz, 0);
		free_lookup (GMT, &Lg, 1);
		Return (0);
	}
#endif

	/* Remove mean (or LS plane) from data (we will add it back later) */

	do_normalization (API, X, obs, n, normalize, dimension, norm);

	/* Set up linear system Ax = z */

	mem = ((double)nm * (double)nm * (double)sizeof (double)) / 1024.0;	/* In kb */
	unit = 0;
	while (mem > 1024.0 && unit < 2) { mem /= 1024.0; unit++; }	/* Select next unit */
	GMT_Report (API, GMT_MSG_VERBOSE, "Square matrix requires %.1f %s\n", mem, mem_unit[unit]);
	A = GMT_memory (GMT, NULL, nm * nm, double);

	GMT_Report (API, GMT_MSG_VERBOSE, "Build linear system using %s\n", method[Ctrl->S.mode]);

	weight_i = weight_j = 1.0;
	for (j = 0; j < nm; j++) {	/* For each value or slope constraint */
		if (Ctrl->W.active) {
			weight_j = X[j][dimension];
			obs[j] *= weight_j;
		}
		for (i = j; i < nm; i++) {
			if (Ctrl->W.active) weight_i = X[i][dimension];
			ij = j * nm + i;
			ji = i * nm + j;
			r = get_radius (GMT, X[i], X[j], dimension);
			if (j < n) {	/* Value constraint */
				A[ij] = G (GMT, r, par, Lz);
				if (ij == ji) continue;	/* Do the diagonal terms only once */
				if (i < n) {
					A[ji] = weight_i * A[ij];
				}
				else {
					/* Get D, the directional cosine between the two points */
					/* Then get C = GMT_dot3v (GMT, D, dataD); */
					/* A[ji] = dGdr (r, par, Lg) * C; */
					C = get_dircosine (GMT, D[i-n], X[i], X[j], dimension, false);
					grad = dGdr (GMT, r, par, Lg);
					A[ji] = weight_i * grad * C;
				}
				A[ij] *= weight_j;
			}
			else if (i > n) {	/* Remaining gradient constraints */
				if (ij == ji) continue;	/* Diagonal gradient terms are zero */
				C = get_dircosine (GMT, D[j-n], X[i], X[j], dimension, true);
				grad = dGdr (GMT, r, par, Lg);
				A[ij] = weight_j * grad * C;
				C = get_dircosine (GMT, D[i-n], X[i], X[j], dimension, false);
				A[ji] = weight_i * grad * C;
			}
		}
	}

	if (Ctrl->C.active) {		/* Solve using svd decomposition */
		int n_use, error;
		double *v = NULL, *s = NULL, *b = NULL, eig_max = 0.0, limit;

		GMT_Report (API, GMT_MSG_VERBOSE, "Solve linear equations by SVD\n");
#ifndef HAVE_LAPACK
		GMT_Report (API, GMT_MSG_VERBOSE, "Note: SVD solution without LAPACK will be very very slow.\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "We strongly recommend you install LAPACK and recompile GMT.\n");
#endif
		v = GMT_memory (GMT, NULL, nm * nm, double);
		s = GMT_memory (GMT, NULL, nm, double);
		if ((error = GMT_svdcmp (GMT, A, (unsigned int)nm, (unsigned int)nm, s, v))) Return (error);

		if (Ctrl->C.file) {	/* Save the eigen-values for study */
			double *eig = GMT_memory (GMT, NULL, nm, double);
			uint64_t e_dim[4] = {1, 1, nm, 2};
			struct GMT_DATASET *E = NULL;
			GMT_memcpy (eig, s, nm, double);
			if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for saving eigenvalues\n");
				Return (API->error);
			}

			/* Sort eigenvalues into ascending order */
			GMT_sort_array (GMT, eig, nm, GMT_DOUBLE);
			eig_max = eig[nm-1];
			for (i = 0, j = nm-1; i < nm; i++, j--) {
				E->table[0]->segment[0]->coord[GMT_X][i] = i + 1.0;	/* Let 1 be x-value of the first eigenvalue */
				E->table[0]->segment[0]->coord[GMT_Y][i] = (Ctrl->C.mode == 1) ? eig[j] : eig[j] / eig_max;
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->C.file, E) != GMT_OK) {
				Return (API->error);
			}
			if (Ctrl->C.mode == 1)
				GMT_Report (API, GMT_MSG_VERBOSE, "Eigen-values saved to %s\n", Ctrl->C.file);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Eigen-value ratios s(i)/s(0) saved to %s\n", Ctrl->C.file);
			GMT_free (GMT, eig);

			if (Ctrl->C.value < 0.0) {	/* We are done */
				for (p = 0; p < nm; p++) GMT_free (GMT, X[p]);
				GMT_free (GMT, X);
				GMT_free (GMT, s);
				GMT_free (GMT, v);
				GMT_free (GMT, A);
				GMT_free (GMT, obs);
				if (dimension == 2) GMT_free_grid (GMT, &Grid, true);
				Return (EXIT_SUCCESS);
			}
		}
		b = GMT_memory (GMT, NULL, nm, double);
		GMT_memcpy (b, obs, nm, double);
		limit = Ctrl->C.value;
		n_use = GMT_solve_svd (GMT, A, (unsigned int)nm, (unsigned int)nm, v, s, b, 1U, obs, &limit, Ctrl->C.mode);
		if (n_use == -1) Return (EXIT_FAILURE);
		GMT_Report (API, GMT_MSG_VERBOSE, "[%d of %" PRIu64 " eigen-values used to explain %.1f %% of data variance]\n", n_use, nm, limit);

		GMT_free (GMT, s);
		GMT_free (GMT, v);
		GMT_free (GMT, b);
	}
	else {				/* Gauss-Jordan elimination */
		int error;
		if (GMT_IS_ZERO (r_min)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Your matrix is singular because you have duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_NORMAL, "Preprocess your data with one of the blockm* modules to eliminate them\n");

		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Solve linear equations by Gauss-Jordan elimination\n");
		if ((error = GMT_gaussjordan (GMT, A, (unsigned int)nm, obs))) {
			GMT_Report (API, GMT_MSG_NORMAL, "You probably have nearly duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_NORMAL, "Preprocess your data with one of the blockm* modules\n");
			Return (error);
		}
	}
	alpha = obs;	/* Just a different name since the obs vector now holds the alpha factors */
#if 0
	fp = fopen ("alpha.txt", "w");	/* Save alpah coefficients for debugging purposes */
	for (p = 0; p < nm; p++) fprintf (fp, "%g\n", alpha[p]);
	fclose (fp);
#endif
	GMT_free (GMT, A);

	if (Ctrl->N.file) {	/* Specified nodes only */
		double out[4];

		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active && (out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}

		GMT->common.b.ncol[GMT_OUT] = dimension + 1;
		GMT_memset (out, 4, double);
		GMT_Report (API, GMT_MSG_VERBOSE, "Evaluate spline at %" PRIu64 " given locations\n", T->n_records);
		for (seg = 0; seg < T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++) {
				for (ii = 0; ii < dimension; ii++) out[ii] = T->segment[seg]->coord[ii][row];
				out[dimension] = 0.0;
				for (p = 0; p < nm; p++) {
					r = get_radius (GMT, out, X[p], dimension);
					if (Ctrl->Q.active) {
						C = get_dircosine (GMT, Ctrl->Q.dir, out, X[p], dimension, false);
						part = dGdr (GMT, r, par, Lz) * C;
					}
					else
						part = G (GMT, r, par, Lz);
					out[dimension] += alpha[p] * part;
				}
				out[dimension] = undo_normalization (out, out[dimension], normalize, norm, dimension);
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Nin) != GMT_OK) {
			Return (API->error);
		}
		GMT_fclose (GMT, fp);
	}
	else {	/* Output on equidistance lattice */
		uint64_t nz_off, nxy;
		unsigned int layer, wmode = GMT_ADD_DEFAULT;
		double *xp = NULL, *yp = NULL, wp, V[4];
		GMT_Report (API, GMT_MSG_VERBOSE, "Evaluate spline at %" PRIu64 " equidistant output locations\n", n_ok);
		/* Precalculate coordinates */
		xp = GMT_grd_coord (GMT, Grid->header, GMT_X);
		if (dimension > 1) yp = GMT_grd_coord (GMT, Grid->header, GMT_Y);
		nxy = Grid->header->size;
		GMT->common.b.ncol[GMT_OUT] = dimension + 1;
		if (dimension != 2) {	/* Write ASCII table to named file or stdout for 1-D or 3-D */
			if (Ctrl->G.active) {
				if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET)
					Return (error);
				wmode = GMT_ADD_EXISTING;
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_OK) {	/* Establishes output */
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
				Return (API->error);
			}
			if (dimension == 1) GMT_prep_tmp_arrays (GMT, Grid->header->nx, 1);	/* Init or reallocate tmp vector since cannot write to stdout under OpenMP */

		} /* Else we are writing a grid */
		GMT_memset (V, 4, double);
		for (layer = 0, nz_off = 0; layer < Z.nz; layer++, nz_off += nxy) {	/* Might be dummy loop of 1 layer unless 3-D */
			int64_t col, row, p; /* On Windows 'for' index variables must be signed, so redefine these 3 inside this block only */
			double z_level = 0.0;
			if (dimension == 3) z_level = GMT_col_to_x (GMT, layer, Z.z_min, Z.z_max, Z.z_inc, Grid->header->xy_off, Z.nz);
#ifdef _OPENMP
#pragma omp parallel for private(V,row,col,ij,p,r,C,part,wp) shared(Z,dimension,yp,Grid,xp,X,Ctrl,GMT,alpha,Lz,norm,Out,par,nz_off,z_level,nm,normalize)
#endif
			for (row = 0; row < Grid->header->ny; row++) {	/* This would be a dummy loop for 1 row if 1-D data */
				if (dimension > 1) {
					V[GMT_Y] = yp[row];
					if (dimension == 3) V[GMT_Z] = z_level;
				}
				for (col = 0; col < Grid->header->nx; col++) {	/* This loop is always active for 1,2,3D */
					ij = GMT_IJP (Grid->header, row, col) + nz_off;
					if (dimension == 2 && GMT_is_fnan (Grid->data[ij])) continue;	/* Only do solution where mask is not NaN */
					V[GMT_X] = xp[col];
					/* Here, V holds the current output coordinates */
					for (p = 0, wp = 0.0; p < (int64_t)nm; p++) {
						r = get_radius (GMT, V, X[p], dimension);
						if (Ctrl->Q.active) {
							C = get_dircosine (GMT, Ctrl->Q.dir, V, X[p], dimension, false);
							part = dGdr (GMT, r, par, Lz) * C;
						}
						else
							part = G (GMT, r, par, Lz);
						wp += alpha[p] * part;
					}
					V[dimension] = (float)undo_normalization (V, wp, normalize, norm, dimension);
					if (dimension > 1)	/* Special 2-D grid output */
						Out->data[ij] = (float)V[dimension];
					else	/* Crude dump for now for both 1-D and 3-D */
						GMT->hidden.mem_coord[GMT_X][col] = V[dimension];
				}
			}
			/* Write output, in case of 3-D just a single slice */
			if (dimension == 1) {	/* Must dump 1-D table */
				for (col = 0; col < Grid->header->nx; col++) {
					V[GMT_X] = xp[col];
					V[dimension] = GMT->hidden.mem_coord[GMT_X][col];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, V);
				}
			}
			else if (dimension == 3) {	/* Must dump 3-D grid as ASCII slices for now */
				V[GMT_Z] = z_level;
				for (row = 0; row < Grid->header->ny; row++) {
					V[GMT_Y] = yp[row];
					for (col = 0; col < Grid->header->nx; col++) {
						V[GMT_X] = xp[col];
						ij = GMT_IJP (Grid->header, row, col) + nz_off;
						V[dimension] = Out->data[ij];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, V);
					}
				}
			}
		}
		if (dimension == 2) {	/* Write the grid */
			GMT_grd_init (GMT, Out->header, options, true);
			sprintf (Out->header->remark, "Method: %s (%s)", method[Ctrl->S.mode], Ctrl->S.arg);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Out) != GMT_OK) {
				Return (API->error);
			}
		}
		if (delete_grid) /* No longer required for 1-D and 3-D */
			GMT_free_grid (GMT, &Grid, dimension > 1);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		GMT_free (GMT, xp);
		if (dimension > 1) GMT_free (GMT, yp);
	}

	/* Clean up */

	for (p = 0; p < nm; p++) GMT_free (GMT, X[p]);
	GMT_free (GMT, X);
	GMT_free (GMT, obs);
	if (m) {
		for (p = 0; p < m; p++) GMT_free (GMT, D[p]);
		GMT_free (GMT, D);
	}
	free_lookup (GMT, &Lz, 0);
	free_lookup (GMT, &Lg, 1);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

	Return (EXIT_SUCCESS);
}
