/*--------------------------------------------------------------------
 *	$Id: greenspline_func.c,v 1.6 2011-04-19 19:10:44 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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
 *	Computers & Geosciences, 35: 1247–1254".
 */

#include "gmt.h"

/* Control structure for greenspline */

struct GREENSPLINE_CTRL {
	struct A {	/* -A<gradientfile> */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 = azimuths, 1 = directions, 2 = dx,dy components, 3 = dx, dy, dz components */
		char *file;
	} A	;
	struct C {	/* -C<cutoff> */
		GMT_LONG active;
		double value;
		char *file;
	} C;
	struct D {	/* -D<distflag> */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct G {	/* -G<output_grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy[/dz]] */
		GMT_LONG active;
		double inc[3];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct N {	/* -N<outputnode_file> */
		GMT_LONG active;
		char *file;
	} N;
	struct Q {	/* -Qdaz */
		GMT_LONG active;
		double az;
		double dir[3];
	} Q;
	struct R3 {	/* -Rxmin/xmax[/ymin/ymax[/zmin/zmaz]] | -Ggridfile */
		GMT_LONG active;
		GMT_LONG mode;		/* TRUE if settings came from a grid file */
		GMT_LONG dimension;	/* 1, 2, or 3 */
		GMT_LONG offset;	/* 0 or 1 */
		double range[6];	/* Min/max for each dimension */
		double inc[2];		/* xinc/yinc when -Rgridfile was given*/
	} R3;
	struct S {	/* -S<mode>[/args] */
		GMT_LONG active, fast;
		GMT_LONG mode;
		double value[2];
		double rval[2];
		char *arg;
	} S;
	struct T {	/* -T<mask_grdfile> */
		GMT_LONG active;
		char *file;
	} T;
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

#define N_METHODS			10

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

#define N_X 	100001

typedef double REAL;	/* Change this to float and recompile if you only want single precision calculations */

struct ZGRID {
	GMT_LONG nz;
	double z_min, z_max, z_inc;
};

#ifdef TEST
void dump_green (PFD G, PFD D, double par[], double x0, double x1, GMT_LONG N, double *zz, double *gg);
#endif

/* Functions for complex math */

static void Cdiv (double A[], double B[], double C[])
{	/* Complex division */
	double i_denom;
	i_denom = 1.0 / (B[0]*B[0] + B[1]*B[1]);
	C[0] = (A[0]*B[0] + A[1]*B[1]) * i_denom;
	C[1] = (A[1]*B[0] - A[0]*B[1]) * i_denom;
}

static void Cmul (double A[], double B[], double C[])
{	/* Complex multiplication */
	C[0] = A[0]*B[0] - A[1]*B[1];
	C[1] = A[0]*B[1] + A[1]*B[0];
}

static void Ccot (double Z[], double cotZ[])
{	/* Complex cot(z) */
	double sx, cx, e, A[2], B[2];
	
	sincos (2.0*Z[0], &sx, &cx);
	e = exp (-2.0*Z[1]);
	A[0] = -e * sx;		A[1] = B[0] = e * cx;
	A[1] += 1.0;	B[0] -= 1.0;	B[1] = -A[0];
	Cdiv (A, B, cotZ);
}

EXTERN_MSC double GMT_geodesic_dist_cos (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE);
EXTERN_MSC double GMT_great_circle_dist_cos (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);

void *New_greenspline_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GREENSPLINE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GREENSPLINE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->S.mode = SANDWELL_1987_2D;
	C->S.rval[0] = -1.0;	C->S.rval[1] = 1.0;
	return ((void *)C);
}

void Free_greenspline_Ctrl (struct GMT_CTRL *GMT, struct GREENSPLINE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->A.file) free ((void *)C->A.file);	
	if (C->C.file) free ((void *)C->C.file);	
	if (C->G.file) free ((void *)C->G.file);	
	if (C->N.file) free ((void *)C->N.file);	
	if (C->T.file) free ((void *)C->T.file);	
	if (C->S.arg)  free ((void *)C->S.arg);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_greenspline_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "greenspline %s [API] - Interpolation using a Green's function for splines in 1-3 dimensions\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: greenspline [xyzfile] [-A[<format>,]<gradientfile>] [-R<xmin>/<xmax[/<ymin>/<ymax>[/<zmin>/<zmax>]]] [-I<dx>[/<dy>[/<dz>]] -Goutfile\n");
	GMT_message (GMT, "\t[-C<cut>[/<file>]] [-D<mode>] [%s] [-L] [-N<nodes>]\n", GMT_I_OPT);
	GMT_message (GMT, "\t[-Q<az>] [-Sc|t|r|p|q[<pars>]] [-T<maskfile>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\tgreenspline will read from standard input or specified ASCII xyz-file(s) (see -bi if binary).\n\n");
	GMT_message (GMT, "\tChoose one of three ways to specify where to evaluate the spline:\n");
	GMT_message (GMT, "\t1. Specify a rectangular grid domain with options -R, -I [and optionally -r].\n");
	GMT_message (GMT, "\t2. Supply a mask file via -T whose values are NaN or 0.  The spline will then\n");
	GMT_message (GMT, "\t   only be evaluated at the nodes originally set to zero.\n");
	GMT_message (GMT, "\t3. Specify a set of output locations via the -N option.\n\n");
	GMT_message (GMT, "\t-G Output data. Give name of output file.\n");

	GMT_message (GMT, "\n\tOPTIONS:\n");

	GMT_message (GMT, "\t-A ASCII file with surface gradients V to use in the modeling.  Specify format:\n");
	GMT_message (GMT, "\t   (0) For 1-D: x, slope, (1) X, Vmagnitude, Vazimuth(s), (2) X, Vazimuth(s), Vmagnitude,\n");
	GMT_message (GMT, "\t   (3) X, Vmagnitude, Vangle(s), (4) X, Vcomponents, or (5) X, Vunit-vector, Vmagnitude.\n");
	GMT_message (GMT, "\t   Here, X = (x, y[, z]) is the position vector, V is the gradient vector.\n");
	GMT_message (GMT, "\t-C Solve by SVD and eliminate eigenvalues whose ratio to largest eigenvalue is less than <cut>.\n");
	GMT_message (GMT, "\t   Optionally append /<filename> to save the eigenvalues to this file.\n");
	GMT_message (GMT, "\t   A negative cutoff will stop execution after saving the eigenvalues.\n");
	GMT_message (GMT, "\t   [Default uses Gauss-Jordan elimination to solve the linear system]\n");
	GMT_message (GMT, "\t-D Distance flag determines how we calculate distances between (x,y) points:\n");
	GMT_message (GMT, "\t   Options 0 apples to Cartesian 1-D spline interpolation.\n");
	GMT_message (GMT, "\t     -D0 x in user units, Cartesian distances.\n");
	GMT_message (GMT, "\t   Options 1-3 apply to Cartesian 2-D surface spline interpolation.\n");
	GMT_message (GMT, "\t     -D1 x,y in user units, Cartesian distances.\n");
	GMT_message (GMT, "\t     -D2 x,y in degrees, flat Earth distances in meters.\n");
	GMT_message (GMT, "\t     -D3 x,y in degrees, spherical distances in meters.\n");
	GMT_message (GMT, "\t   Option 4 applies to 2-D spherical surface spline interpolation.\n");
	GMT_message (GMT, "\t     -D4 x,y in degrees, use cosine of spherical distances in degrees.\n");
	GMT_message (GMT, "\t   Option 5 applies to Cartesian 3-D volume interpolation.\n");
	GMT_message (GMT, "\t     -D5 x,y,z in user units, Cartesian distances.\n");
	GMT_message (GMT, "\t   For option 3-4, use PROJ_ELLIPSOID to select geodesic or great cicle arcs.\n");
	GMT_message (GMT, "\t-I Specifies a regular set of output locations.  Give equidistant increment for each dimension.\n");
	GMT_message (GMT, "\t   Requires -R for specifying the output domain.\n");
	GMT_message (GMT, "\t-L Leave trend alone.  Do not remove least squares plane from data before spline fit.\n");
	GMT_message (GMT, "\t   Only applies to -D0-2.  [Default removes linear trend, fits residuals, and restores trend].\n");
	GMT_message (GMT, "\t-N ASCII file with desired output locations.\n");
	GMT_message (GMT, "\t   The resulting ASCII coordinates and interpolation are written to file given in -G\n");
	GMT_message (GMT, "\t   or stdout if no file specified (see -bo for binary output).\n");
	GMT_message (GMT, "\t-Q Calculate the directional derivative in the <az> direction and return it instead of surface elevation.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-R Specify a regular set of output locations.  Give min and max coordinates for each dimension.\n");
	GMT_message (GMT, "\t   Requires -I for specifying equidistant increments.  For 2D-gridding a gridfile may be given;\n");
	GMT_message (GMT, "\t   this then also sets -I (and perhaps -r); use those options to override the grid settings.\n");
	GMT_message (GMT, "\t-S Specifies which spline to use (if needed, normalized <tension> must be between 0 and 1):\n");
	GMT_message (GMT, "\t   -Sc is minimum curvature spline (Sandwell, 1987) [Default].\n");
	GMT_message (GMT, "\t   -St<tension>[/<scale>] is spline in tension (Wessel & Bercovici, 1998).\n");
	GMT_message (GMT, "\t      Optionally, specify a length-scale [Default is grid spacing].\n");
	GMT_message (GMT, "\t   -Sr<tension> is a regularized spline in tension (Mitasova & Mitas, 1993).\n");
	GMT_message (GMT, "\t      Optionally, specify a length-scale [Default is grid spacing].\n");
	GMT_message (GMT, "\t   -Sp is a spherical surface spline (Parker, 1994); automatically sets -D4.\n");
	GMT_message (GMT, "\t   -Sq is a spherical surface spline in tension (Wessel & Becker, 2008); automatically sets -D4.\n");
	GMT_message (GMT, "\t      Use -SQ to speed up calculations by using precalculated lookup tables.\n");
	GMT_message (GMT, "\t      Append /n to set the (odd) number of points in the spline [%d].\n", N_X);
	GMT_message (GMT, "\t-T Mask grid file whose values are NaN or 0; its header implicitly sets -R, -I (and -r).\n");
	GMT_explain_options (GMT, "VD0");
	GMT_message (GMT, "\t   Default is 2-4 input columns depending on dimensionality (see -D).\n");
	GMT_explain_options (GMT, "ghioF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_greenspline_parse (struct GMTAPI_CTRL *C, struct GREENSPLINE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to greenspline and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_items, k, j, dimension, error;
	char txt[6][GMT_TEXT_LEN];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			case 'R':	/* Normally processed internally but must be handled separately since it can take 1,2,3 dimensions */
				GMT->common.R.active = TRUE;
				Ctrl->R3.dimension = 1;	/* At least */

				if (opt->arg[0] == 'g' && opt->arg[1] == '\0') {	/* Got -Rg */
					Ctrl->R3.range[0] = 0.0;	Ctrl->R3.range[1] = 360.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					break;
				}
				if (opt->arg[0] == 'd' && opt->arg[1] == '\0') {	/* Got -Rd */
					Ctrl->R3.range[0] = -180.0;	Ctrl->R3.range[1] = 180.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					break;
				}
				if (!GMT_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file, presumably a grid */
					struct GMT_GRID *G = NULL;	
					if ((error = GMT_Begin_IO (C, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) return (error);				/* Enables data input and sets access mode */
					if (GMT_Get_Data (C, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&opt->arg, (void **)&G)) return (GMT_DATA_READ_ERROR);	/* Get header only */
					if ((error = GMT_End_IO (C, GMT_IN, 0))) return (error);				/* Disables further data input */
					Ctrl->R3.range[0] = G->header->wesn[XLO]; Ctrl->R3.range[1] = G->header->wesn[XHI];
					Ctrl->R3.range[2] = G->header->wesn[YLO]; Ctrl->R3.range[3] = G->header->wesn[YHI];
					Ctrl->R3.inc[GMT_X] = G->header->inc[GMT_X];	Ctrl->R3.inc[GMT_Y] = G->header->inc[GMT_Y];
					Ctrl->R3.offset = G->header->registration;
					Ctrl->R3.dimension = 2;
					Ctrl->R3.mode = 1;
					GMT_Destroy_Data (C, GMT_ALLOCATED, (void **)&G);
					break;
				}

				n_items = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s", txt[0], txt[1], txt[2], txt[3], txt[4], txt[5]);
				if (!(n_items == 2 || n_items == 4 || n_items == 6)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -R option: Give 2, 4, or 6 coordinates\n");
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
				Ctrl->A.active = TRUE;
				j = 0;
				if (strchr (opt->arg, ',')) {	/* Specified a particular format with -A<mode>,<file> */
					Ctrl->A.mode = (GMT_LONG)(opt->arg[0] - '0');
					j = 2;
				}
				Ctrl->A.file = strdup (&opt->arg[j]);
				break;
			case 'C':	/* Solve by SVD */
				Ctrl->C.active = TRUE;
				if (strchr (opt->arg, '/')) {
					char tmp[BUFSIZ];
					sscanf (opt->arg, "%lf/%s", &Ctrl->C.value, tmp);
					Ctrl->C.file = strdup (tmp);
				}
				else
					Ctrl->C.value = atof (opt->arg);
				break;
			case 'D':	/* Distance mode */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = atoi(opt->arg);	/* Since I added 0 to be 1-D later so no it is -1 */
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Table or grid spacings */
				Ctrl->I.active = TRUE;
				k = GMT_getincn (GMT, opt->arg, Ctrl->I.inc, 3);
				if (k < 1) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				if (Ctrl->I.inc[GMT_Y] == 0.0) Ctrl->I.inc[GMT_Y] = Ctrl->I.inc[GMT_X];
				if (Ctrl->I.inc[GMT_Z] == 0.0) Ctrl->I.inc[GMT_Z] = Ctrl->I.inc[GMT_X];
				break;
			case 'L':	/* Leave trend alone */
				Ctrl->L.active = TRUE;
				break;
			case 'N':	/* Output locations */
				Ctrl->N.active = TRUE;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Q':	/* Directional derivative */
				Ctrl->Q.active = TRUE;
				if (strchr (opt->arg, '/')) {	/* Got 3-D vector components */
					k = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->Q.dir[0], &Ctrl->Q.dir[1], &Ctrl->Q.dir[2]);
					if (k != 3) {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q option: Append azimuth (2-D) or x/y/z components (3-D)\n");
						n_errors++;
					}
					GMT_normalize3v (GMT, Ctrl->Q.dir);	/* Normalize to unit vector */
				}
				else if (opt->arg[0])	/* 2-D azimuth */
					Ctrl->Q.az = atof(opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q option: Append azimuth (2-D) or x/y/z components (3-D)\n");
					n_errors++;
				}
				break;
			case 'S':	/* Spline selection */
				Ctrl->S.arg = strdup (opt->arg);
				switch (opt->arg[0]) {
					case 'c':
						Ctrl->S.mode = SANDWELL_1987_1D;
						break;
					case 't':
						Ctrl->S.mode = WESSEL_BERCOVICI_1998_1D;
						if (strchr (opt->arg, '/'))
							sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1]);
						else
							Ctrl->S.value[0] = atof (&opt->arg[1]);
						break;
					case 'r':
						Ctrl->S.mode = MITASOVA_MITAS_1993_2D;
						if (strchr (opt->arg, '/'))
							sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1]);
						else
							Ctrl->S.value[0] = atof (&opt->arg[1]);
						break;
					case 'p':
						Ctrl->S.mode = PARKER_1994;
						break;
					case 'Q':
						Ctrl->S.mode = WESSEL_BECKER_2008;
						Ctrl->S.fast = TRUE; 
						if (strchr (opt->arg, '/')) {
							k = sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf", &Ctrl->S.value[0], &Ctrl->S.value[1], &Ctrl->S.rval[0], &Ctrl->S.rval[1]);
							if (k == 2) {
								Ctrl->S.rval[0] = -1.0;
								Ctrl->S.rval[1] = +1.0;
							}
						}
						else {
							Ctrl->S.value[0] = atof (&opt->arg[1]);
							Ctrl->S.value[1] = (double)N_X;
						}
						break;
					case 'q':
						Ctrl->S.mode = WESSEL_BECKER_2008;
						Ctrl->S.value[0] = atof (&opt->arg[1]);
						if (Ctrl->S.value[0] == 0.0) {
							Ctrl->S.mode = PARKER_1994;
						}
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: Append c|t|g|p|q\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Input mask grid */
				Ctrl->T.active = TRUE;
				Ctrl->T.file = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	dimension = (Ctrl->D.mode == 0) ? 1 : ((Ctrl->D.mode == 5) ? 3 : 2);
	if (dimension == 2 && Ctrl->R3.mode) {	/* Set -R via a gridfile */
		/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
		if (!Ctrl->I.active) {	/* -I was not set separately; set indirectly */
			Ctrl->I.inc[GMT_X] = Ctrl->R3.inc[GMT_X];
			Ctrl->I.inc[GMT_Y] = Ctrl->R3.inc[GMT_Y];
			Ctrl->I.active = TRUE;
		}
		/* Here, -r means toggle the grids registration */
		GMT->common.r.active = (GMT->common.r.active) ? !Ctrl->R3.offset : Ctrl->R3.offset;
	}
	
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && GMT_access (GMT, Ctrl->A.file, R_OK), "Syntax error -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += GMT_check_condition (GMT, !(GMT->common.R.active || Ctrl->N.active || Ctrl->T.active), "Syntax error: No output locations specified (use either [-R -I], -N, or -T)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->R3.mode && dimension != 2, "Syntax error: The -R<gridfile> option only applies to 2-D gridding\n");
	n_errors += GMT_check_condition (GMT, Ctrl->R3.dimension != dimension, "Syntax error: The -R and -D options disagree on the dimension\n");
	n_errors += GMT_check_binary_io (GMT, dimension + 1);
	n_errors += GMT_check_condition (GMT, Ctrl->S.value[0] < 0.0 || Ctrl->S.value[0] >= 1.0, "Syntax error -S option: Tension must be in range 0 <= t < 1\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) && Ctrl->D.mode == 3, "Syntax error -Sc|t|r option: Cannot select -D3\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || (dimension > 1 && Ctrl->I.inc[GMT_Y] <= 0.0) || (dimension == 3 && Ctrl->I.inc[GMT_Z] <= 0.0)), "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, dimension == 2 && !Ctrl->N.active && !(Ctrl->G.active  || Ctrl->G.file), "Syntax error -G option: Must specify output grid file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->C.value < 0.0 && !Ctrl->C.file, "Syntax error -C option: Must specify file name for eigenvalues if cut < 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Syntax error -T option: Must specify mask grid file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Syntax error -N option: Must specify node file name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->N.file && GMT_access (GMT, Ctrl->N.file, R_OK), "Syntax error -N: Cannot read file %s!\n", Ctrl->N.file);
	n_errors += GMT_check_condition (GMT, (Ctrl->I.active + GMT->common.R.active) == 1 && dimension == 2, "Syntax error: Must specify -R, -I, [-r], -G for gridding\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#ifdef TEST
/* Dump a table of x, G, dGdx for test purposes */
void dump_green (PFD G, PFD D, double par[], double x0, double x1, GMT_LONG N, double *zz, double *gg)
{
	GMT_LONG i;
	double x, dx, dy, y, t, ry, rdy;
	double min_y, max_y, min_dy, max_dy;
	
	min_y = min_dy = DBL_MAX;
	max_y = max_dy = -DBL_MAX;
	
	dx = (x1 - x0) / (N - 1);
	for (i = 0; i < N; i++) {
		x = x0 + i * dx;
		t = (x0 < 0.0) ? acosd (x) : x;
		y = G(x, par, zz);
		dy = D(x, par, gg);
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
		y = G(x, par, zz);
		dy = D(x, par, gg);
		dy = (rdy > 0.0) ? (dy - min_dy)/rdy : 1.0;
		printf ("%g\t%g\t%g\t%g\n", x, (y - min_y) / ry, dy, t);
	}
}
#endif

/*----------------------  ONE DIMENSION ---------------------- */
/* spline1d_sandwell computes the Green function for a 1-d spline
 * as per Sandwell [1987], G(r) = r^3.  All r must be >= 0.
 */

double spline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	if (r == 0.0) return (0.0);

	return (pow (r, 3.0));	/* Just regular spline; par not used */
}

double gradspline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	return (r);	/* Just regular spline; par not used */
}

double spline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
/* spline1d_Wessel_Bercovici computes the Green function for a 1-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = exp(-u) + u - 1,
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * All r must be >= 0. par[0] = c
 */
{
	double cx;

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (exp (-cx) + cx - 1.0);
}

double gradspline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double cx;

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (1.0 - exp (-cx));
}

/*----------------------  TWO DIMENSIONS ---------------------- */

double spline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	if (r == 0.0) return (0.0);

	return (r * r * (log (r) - 1.0));	/* Just regular spline; par not used */
}

double gradspline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
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

double spline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double y, z, cx, t, g;

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

double gradspline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double y, z, cx, t, dgdr;

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
 
double spline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double x, z, g, En, Ed;
	
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

double gradspline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double u, dgdr;
	
	if (r == 0.0) return (0.0);

	u = par[1] * r * r;
	dgdr = 2.0 * (1.0 - exp (-u))/r;
	return (dgdr);
}

/*----------------------  TWO DIMENSIONS (SPHERE) ---------------------- */

/* spline2d_Parker computes the  Green function for a 2-d surface spline
 * as per Parker [1994], G(x) = dilog(),
 * where x is cosine of distances. All x must be -1 <= x <= +1.
 * Parameters passed are:
 * par[0] = 6/M_PI^2 (to normalize results)
 */
 
double spline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], double *G)
{	/* Normalized to 0-1 */
	if (x == +1.0) return (1.0);
	if (x == -1.0) return (0.0);
	return (GMT_dilog (GMT, 0.5 - 0.5 * x) * par[0]);
}

double gradspline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], double *G)
{	/* Normalized to 0-1 */
	if (x == +1.0 || x == -1.0) return (0.0);
	return (log (0.5 - 0.5 * x) * sqrt ((1.0 - x) / (1.0 + x)));
}

/* spline2d_Wessel_Becker computes the Green function for a 2-d surface spline
 * in tension as per Wessel and Becker [2008], G(x) = M_PI * Pv(-x)/sin (v*x) - log (1-x),
 * where x is cosine of distances.  All x must be -1 <= x <= +1.
 * Parameters passed are:
 * par[0] = Real(nu)
 * par[1] = Imag(nu)
 * par[2] = G(-1)
 * par[3] = G(+1)
 * par[4] = Real {sin (nu * M_PI}
 * par[5] = Imag {sin (nu * M_PI)} == 0
 * par[6] = 1 / (par[3] - par[2])
 * par[7-9] is used by the lookup macinery
 */
 
double spline2d_Wessel_Becker (struct GMT_CTRL *GMT, double x, double par[], double *G)
{	/* g = M_PI * Pv(-x)/sin (v*x) - log (1-x) normalized to 0-1 */
	GMT_LONG n;
	double z[2], pq[4];
	
	if (x == +1.0) return (1.0);
	if (x == -1.0) return (0.0);

	GMT_PvQv (GMT, -x, par, pq, &n);	/* Get P_nu(-x) */
	Cdiv (pq, &par[4], z);		/* Get P_nu(-x) / sin (nu*M_PI) */
	pq[0] = M_PI * z[0] - log (1.0 - x);
#ifdef TEST
	pq[1] = M_PI * z[1];
	if (fabs (pq[1]) > 1.0e-6) GMT_message (GMT, "Im{G(%g)} = %g\n", x, pq[1]);
#endif
	
	return ((pq[0] - par[2]) * par[6]);	/* Normalizes to yield 0-1 */
}

double gradspline2d_Wessel_Becker (struct GMT_CTRL *GMT, double x, double par[], double *G)
{	/* g = -M_PI * (v+1)*[x*Pv(-x)+Pv+1(-x)]/(sin (v*x)*sin(theta)) - 1/(1-x), normalized to 0-1 */
	GMT_LONG n;
	double z[2], v1[2], pq[4], s;
	
	if (x == +1.0 || x == -1.0) return (0.0);

	GMT_PvQv (GMT, -x, par, pq, &n);			/* Get P_nu(-x) */
	z[0] = pq[0] * x;	z[1] = pq[1] * x;	/* Get x*P_nu(-x) */
	v1[0] = par[0] + 1.0;	v1[1] = par[1];		/* Get nu+1 */
	GMT_PvQv (GMT, -x, v1, pq, &n);			/* Get P_(nu+1)(-x) */
	z[0] += pq[0];	z[1] += pq[1];			/* Get x*P_nu(-x) + P_(nu+1)(-x) */
	Cdiv (z, &par[4], pq);				/* Get ---"--- / sin (nu*M_PI) */
	Cmul (pq, v1, z);				/* Mul by nu + 1 */
	s = M_PI / sqrt (1.0 - x*x);			/* Mul by pi/sin(theta) */
	z[0] *= s;
#ifdef TEST
	z[1] *= s;;
	if (fabs (z[1]) > 1.0e-6) GMT_message (GMT, "Im{G(%g)} = %g\n", x, z[1]);
#endif
	z[0] += sqrt ((1.0 + x)/(1.0 - x));		/* Add in last term */
	
	return (-z[0]);
}

/* Given the lookup tables, this is how we use these functions
 * Here, par[7] is number of points in spline
 *	 par[8] is spline spacing dx
 *	 par[9] is inverse, 1/dx
 *	par[10] is min x
 */

double spline2d_lookup (struct GMT_CTRL *GMT, double x, double par[], double *y)
{
	GMT_LONG k;
	double f, f0, df;
	
	f = (x - par[10]) * par[9];	/* Floating point index */
	f0 = floor (f);
	df = f - f0;
	k = (GMT_LONG)f0;
	if (df == 0.0) return (y[k]);
	return (y[k]*(1.0 - df) + y[k+1] * df);
}

double spline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], double *y)
{
	return (spline2d_lookup (GMT, x, par, y));
}

double gradspline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], double *y)
{
	return (spline2d_lookup (GMT, x, par, y));
}

void spline2d_Wessel_Becker_init (struct GMT_CTRL *GMT, double par[], double *z, double *g, GMT_LONG grad)
{
	GMT_LONG i, nx;
	double x;
#ifdef DUMP
	FILE *fp = NULL;
	size_t n_out;
	double out[3];
	fp = fopen ("greenspline.b", "wb");
	n_out = (grad) ? 3 : 2;
#endif
	nx = (GMT_LONG)irint (par[7]);
	for (i = 0; i < nx; i++) {
		x = par[10] + i * par[8];
		z[i] = spline2d_Wessel_Becker (GMT, x, par, NULL);
		if (grad) g[i] = gradspline2d_Wessel_Becker (GMT, x, par, NULL);
#ifdef DUMP
		out[0] = x;	out[1] = z[i];	if (grad) out[2] = g[i];
		fwrite ((void *)out, sizeof (double), n_out, fp);
#endif
	}
#ifdef DUMP
	fclose (fp);
#endif
}

/*----------------------  THREE DIMENSIONS ---------------------- */
		
/* spline3d_sandwell computes the Green function for a 3-d spline
 * as per Sandwell [1987], G(r) = r.  All r must be >= 0.
 */

double spline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	if (r == 0.0) return (0.0);

	return (r);	/* Just regular spline; par not used */
}

double gradspline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	return (1.0);	/* Just regular spline; par not used */
}

double spline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
/* spline1d_Wessel_Bercovici computes the Green function for a 3-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = [exp(-u) -1]/u,
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * All r must be >= 0. par[0] = c
 */
{
	double cx;

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (((exp (-cx) - 1.0) / cx) + 1.0);
}

double gradspline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double cx;

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return ((1.0 - exp (-cx) * (cx + 1.0))/ (cx * r));
}

/* spline3d_Mitasova_Mitas computes the regularized Green function for a 3-d spline
 * in tension as per Mitasova and Mitas [1993], G(u) = erf (u/2)/u - 1/sqrt(pi),
 * where u = par[0] * r. All r must be >= 0. par[0] = phi
 */
 
double spline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double x;
	
	if (r == 0.0) return (0.0);
	
	x = par[0] * r;
	return ((erf (0.5 * x) / x) - M_INV_SQRT_PI);
}

double gradspline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], double *G)
{
	double u, dgdr;
	
	if (r == 0.0) return (0.0);

	u = par[0] * r;
	dgdr = ((u/M_SQRT_PI) * exp (-u*u) - erf (0.5 * u)) / (u * r);
	return (dgdr);
}

/* GENERAL NUMERICAL FUNCTIONS */

/* Modified from similar function in Numerical Recipes */

GMT_LONG gaussj (struct GMT_CTRL *GMT, REAL *a, GMT_LONG n, GMT_LONG ndim, REAL *b, GMT_LONG m, GMT_LONG mdim)
{
	GMT_LONG i, j, k, l, ll, *ipiv = NULL, *indxc = NULL, *indxr = NULL, irow = 0, icol = 0;
	REAL big, dum, pivinv;
	
	ipiv  = GMT_memory (GMT, NULL, n, GMT_LONG);
	indxc = GMT_memory (GMT, NULL, n, GMT_LONG);
	indxr = GMT_memory (GMT, NULL, n, GMT_LONG);
	
	for (i = 0; i < n; i++) {
		big = 0.0;
		for (j = 0; j < n; j++) {
			if (ipiv[j] != 1) {
				for (k = 0; k < n; k++) {
					if (ipiv[k] == 0) {
						if ((dum = fabs (a[j*ndim+k])) >= big) {
							big = dum;
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1) {
						GMT_message (GMT, "gaussj: Singular matrix!\n");
						GMT_free (GMT, ipiv);
						GMT_free (GMT, indxc);
						GMT_free (GMT, indxr);
						return (1);
					}
				}
			}
		}
		ipiv[icol]++;
		
		if (irow != icol) {
			for (l = 0; l < n; l++) {
				dum = a[irow*ndim+l];
				a[irow*ndim+l] = a[icol*ndim+l];
				a[icol*ndim+l] = dum;
			}
			for (l = 0; l < m; l++) {
				dum = b[irow*mdim+l];
				b[irow*mdim+l] = b[icol*mdim+l];
				b[icol*mdim+l] = dum;
			}
		}
		
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol*ndim+icol] == 0.0) {
			GMT_message (GMT, "gaussj: Singular matrix!\n");
			GMT_free (GMT, ipiv);
			GMT_free (GMT, indxc);
			GMT_free (GMT, indxr);
			return (1);
		}
		pivinv = 1.0 / a[icol*ndim+icol];
		a[icol*ndim+icol] = 1.0;
		for (l = 0; l < n; l++) a[icol*ndim+l] *= pivinv;
		for (l = 0; l < m; l++)  b[icol*mdim+l] *= pivinv;
		for (ll = 0; ll < n; ll++) {
			if (ll != icol) {
				dum = a[ll*ndim+icol];
				a[ll*ndim+icol] = 0.0;
				for (l = 0; l < n; l++) a[ll*ndim+l] -= a[icol*ndim+l] * dum;
				for (l = 0; l < m; l++) b[ll*mdim+l] -= b[icol*mdim+l] * dum;
			}
		}
	}
	for (l = n-1; l >= 0; l--) {
		if (indxr[l] != indxc[l]) {
			for (k = 0; k < n; k++) {
				dum = a[k*ndim+indxr[l]];
				a[k*ndim+indxr[l]] = a[k*ndim+indxc[l]];
				a[k*ndim+indxc[l]] = dum;
			}
		}
	}
	
	GMT_free (GMT, ipiv);
	GMT_free (GMT, indxc);
	GMT_free (GMT, indxr);
	
	return (0);
}

/* Given a matrix a[0..m-1][0...n-1], this routine computes its singular
	value decomposition, A=UWVt.  The matrix U replaces a on output.
	The diagonal matrix of singular values W is output as a vector
	w[0...n-1].  The matrix V (Not V transpose) is output as
	v[0...n-1][0....n-1].  m must be greater than or equal to n; if it is
	smaller, then a should be filled up to square with zero rows.
	
	Modified from Numerical Recipes -> page 68.
*/

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

GMT_LONG svdcmp (struct GMT_CTRL *GMT, REAL *a, GMT_LONG m, GMT_LONG n, REAL *w, REAL *v)
{
	/* void svdcmp(REAL *a,int m,int n,REAL *w,REAL *v) */
	
	GMT_LONG flag,i,its,j,jj,k,l=0,nm = 0;
	REAL c,f,h,s,x,y,z;
	REAL anorm=0.0,tnorm, g=0.0,scale=0.0;
	REAL *rv1 = NULL;
	
	if (m < n) {
		GMT_message (GMT, "Error in SVCMP: m < n augment A with additional rows\n");
		return (EXIT_FAILURE);
	}
	
	/* allocate work space */
		
	rv1=(REAL *)calloc(n,sizeof(REAL));
	if (rv1 == NULL) {
		GMT_message (GMT, "Error in SVCMP: Can't allocate work space\n");
		return (EXIT_FAILURE);
	}
	
	/* do householder reduction to bidiagonal form */
		
	for (i=0;i<n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i < m) {
			for (k=i;k<m;k++) scale += fabs (a[k*n+i]);		/* a[k][i] */
			if (scale) {
				for (k=i;k<m;k++) {
					a[k*n+i] /= scale;	/* a[k][i] */
					s += a[k*n+i]*a[k*n+i];	/* a[k][i] */
				}
				f=a[i*n+i];	/* a[i][i] */
				g= -1.0*SIGN(sqrt(s),f);
				h=f*g-s;
				a[i*n+i]=f-g;	/* a[i][i] */
				if (i != n-1) {
					for (j=l;j<n;j++) {
						for (s=0.0,k=i;k<m;k++) s += a[k*n+i]*a[k*n+j];	/* a[k][i] a[k][j] */
						f=s/h;
						for (k=i;k<m;k++) a[k*n+j] += f*a[k*n+i];	/* a[k][j] a[k][i] */
					}
				}
				for (k=i;k<m;k++) a[k*n+i] *= scale;	/* a[k][i] */
			}
		}
		w[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m-1 && i != n-1) {
			for (k=l;k<n;k++) scale += fabs (a[i*n+k]);	/* a[i][k] */
			if (scale) {
				for (k=l;k<n;k++) {
					a[i*n+k] /= scale;	/* a[i][k] */
					s += a[i*n+k]*a[i*n+k];	/* a[i][k] */
				}
				f=a[i*n+l];	/* a[i][l] */
				g = -1.0*SIGN(sqrt(s),f);
				h=f*g-s;
				a[i*n+l]=f-g;	/* a[i][l] */
				for (k=l;k<n;k++) rv1[k]=a[i*n+k]/h;	/* a[i][k] */
				if (i != m-1) {
					for (j=l;j<m;j++) {
						for (s=0.0,k=l;k<n;k++) s += a[j*n+k]*a[i*n+k];	/*a[j][k] a[i][k] */
						for (k=l;k<n;k++) a[j*n+k] += s*rv1[k];	/* a[j][k] */
					}
				}
				for (k=l;k<n;k++) a[i*n+k] *= scale;	/* a[i][k] */
			}
		}
		tnorm=fabs (w[i])+fabs (rv1[i]);
		anorm=MAX(anorm,tnorm);
	}
						
	/* accumulation of right-hand transforms */
		
	for (i=n-1;i>=0;i--) {
		if (i < n-1) {
			if (g) {
				for (j=l;j<n;j++) v[j*n+i]=(a[i*n+j]/a[i*n+l])/g;	/* v[j][i] a[i][j] a[i][l] */
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<n;k++) s += a[i*n+k]*v[k*n+j];	/* a[i][k] v[k][j] */
					for (k=l;k<n;k++) v[k*n+j] += s*v[k*n+i];	/* v[k][j] v[k][i] */
				}
			}
			for (j=l;j<n;j++) v[i*n+j]=v[j*n+i]=0.0;	/* v[i][j] v[j][i] */
		}
		v[i*n+i]=1.0;	/* v[i][i] */
		g=rv1[i];
		l=i;
	}
	
	/* accumulation of left-hand transforms */
		
	for (i=n-1;i>=0;i--) {
		l=i+1;
		g=w[i];
		if (i < n-1) for (j=l;j<n;j++) a[i*n+j]=0.0;	/* a[i][j] */
		if (g) {
			g=1.0/g;
			if (i != n-1) {
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<m;k++) s += a[k*n+i]*a[k*n+j];	/* a[k][i] a[k][j] */
					f=(s/a[i*n+i])*g;	/* a[i][i] */
					for (k=i;k<m;k++) a[k*n+j] += f*a[k*n+i];	/* a[k][j] a[k][i] */
				}
			}
			for (j=i;j<m;j++) a[j*n+i] *= g;	/* a[j][i] */
		}
		else {
			for (j=i;j<m;j++) a[j*n+i]=0.0;	/* a[j][i] */
		}
		++a[i*n+i];	/* a[i][i] */
	}
	
	/* diagonalization of the bidiagonal form */
		
	for (k=n-1;k>=0;k--) {			/* loop over singular values */
		for (its=1;its<=30;its++) {	/* loop over allowed iterations */
			flag=1;
			for (l=k;l>=0;l--) {		/* test for splitting */
				nm=l-1;
				if (fabs(rv1[l])+anorm == anorm) {
					flag=0;
					break;
				}
				if (fabs (w[nm])+anorm == anorm) break;
			}
			if (flag) {
				c=0.0;			/* cancellation of rv1[l] if l > 1 */
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					if (fabs (f)+anorm != anorm) {
						g=w[i];
						h=hypot (f,g);
						w[i]=h;
						h=1.0/h;
						c=g*h;
						s=(-1.0*f*h);
						for (j=0;j<m;j++) {
							y=a[j*n+nm];	/* a[j][nm] */
							z=a[j*n+i];	/* a[j][i] */
							a[j*n+nm]=(y*c)+(z*s);	/* a[j][nm] */
							a[j*n+i]=(z*c)-(y*s);	/* a[j][i] */
						}
					}
				}
			}
			z=w[k];
			if (l == k) {		/* convergence */
				if (z < 0.0) {	/* singular value is made positive */
					w[k]= -1.0*z;
					for (j=0;j<n;j++) v[j*n+k] *= (-1.0);	/* v[j][k] */
				}
				break;
			}
			if (its == 30) {
				GMT_message (GMT, "Error in SVDCMP: No convergence in 30 iterations\n");
				return (EXIT_FAILURE);
			}
			x=w[l];		/* shift from bottom 2-by-2 minor */
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=hypot (f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			
				/* next QR transformation */
			
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=hypot(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=(x*c)+(g*s);
				g=(g*c)-(x*s);
				h=y*s;
				y=y*c;
				for (jj=0;jj<n;jj++) {
					x=v[jj*n+j];	/* v[jj][j] */
					z=v[jj*n+i];	/* v[jj][i] */
					v[jj*n+j]=(x*c)+(z*s);	/* v[jj][j] */
					v[jj*n+i]=(z*c)-(x*s);	/* v[jj][i] */
				}
				z=hypot(f,h);
				w[j]=z;		/* rotation can be arbitrary if z=0 */
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=(c*g)+(s*y);
				x=(c*y)-(s*g);
				for (jj=0;jj<m;jj++) {
					y=a[jj*n+j];	/* a[jj][j] */
					z=a[jj*n+i];	/* a[jj][i] */
					a[jj*n+j]=(y*c)+(z*s);	/* a[jj][j] */
					a[jj*n+i]=(z*c)-(y*s);	/* a[jj][i] */
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	free ((void *)rv1);
	return (GMT_NOERROR);
}

void mat_trans (REAL a[], GMT_LONG mrow, GMT_LONG ncol, REAL at[])
{
	/* Return the transpose of a */
	GMT_LONG i, j;
	for (i = 0; i < ncol; i++) for (j = 0; j < mrow; j++) at[mrow*i+j] = a[ncol*j+i];
}

void mat_mult (REAL a[], GMT_LONG mrow, GMT_LONG ncol, REAL b[], GMT_LONG kcol, REAL c[])
{
	/* Matrix multiplication a * b = c */
	
	GMT_LONG i, j, k, ij;
	
	for (i = 0; i < kcol; i++) {
		for (j = 0; j < mrow; j++) {
			ij = j * kcol + i;
			c[ij] = 0.0;
			for (k = 0; k < ncol; k++) c[ij] += a[j * ncol + k] * b[k * kcol + i];
		}
	}
}

/* Given the singular value decomposition of a matrix a[0...m-1][0...n-1]
	solve the system of equations ax=b for x.  Input the matrices 
	U[0....m-1][0...n-1],w[0...n-1], and V[0...n-1][0...n-1] determined from
	svdcmp.  Also input the matrix b[0...m-1][0....k-1] and the solution vector
	x[0....k-1][0....n-1] is output. Singular values whose ratio to the maximum
	singular value are smaller than cutoff are zeroed out. The matrix U is
	overwritten.
	
*/

GMT_LONG solve_svd (struct GMT_CTRL *GMT, REAL *u, GMT_LONG m, GMT_LONG n, REAL *v, REAL *w, REAL *b, GMT_LONG k, REAL *x, double cutoff)
{
	REAL *ut = NULL, sing_max;
	GMT_LONG i, j, n_use = 0;

	/* allocate work space */
		
	ut = (REAL *)calloc(n*m,sizeof(REAL));	/* space for the transpose */
	if (ut == NULL) {
		GMT_message (GMT, "Error in solve_svd: Can't allocate work space\n");
		return (-1);
	}
	
	/* find maximum singular value */
	
	sing_max = w[0];
	for (i = 1; i < n; i++) sing_max = MAX (sing_max, w[i]);
		
	/* loop through singular values removing small ones */
		
	for (i = 0; i < n; i++) {
		if ((w[i]/sing_max) > cutoff) {
			w[i] = 1.0 / w[i];
			n_use++;
		}
		else
			w[i] = 0.0;
	}
	
	/* multiply V by 1/w */
	
	for (i = 0; i < n; i++) for (j = 0; j < n; j++) v[j*n+i] *= w[i];
			
	/* get transpose of U */
		
	mat_trans (u, m, n, ut);
	
	/* multiply v(1/w)ut  -> this overwrites the matrix U */
		
	mat_mult (v, n, n, ut, m, u);
	
	/* multiply this result by b to get x */
		
	mat_mult (u, n, m, b, k, x);

	/* free work space */

	free ((void *)ut);
	
	return (n_use);
}

/* Normalization parameters are stored in the coeff array which holds up to 7 terms
 * coeff[0]:	The mean x coordinate
 * coeff[1]:	The mean y coordinate
 * coeff[2]:	The mean w coordinate
 * coeff[3]:	The linear x slope
 * coeff[4]:	The linear y slope
 * coeff[5]:	The offset for the detrended data (combined into coeff[2])
 * coeff[6]:	The normalizing scale for the detrended data
 */

void do_normalization_1d (double **X, double *obs, GMT_LONG n, GMT_LONG mode, double *coeff)
{	/* mode == 1 norm w; mode == 2: also remove linear trend, norm & 4: also normalize result by range */

	GMT_LONG i;
	double d;
	
	GMT_memset (coeff, 5, double);
	for (i = 0; i < n; i++) {	/* Find mean w-value */
		coeff[GMT_Z] += obs[i];
		if (mode == 1) continue;
		coeff[GMT_X] += X[i][GMT_X];
	}
	coeff[GMT_Z] /= n;

	if (mode & 2) {	/* Solve for LS linera trend using deviations from (0, 0, 0) */
		double	xx, zz, sxx, sxz;
		sxx = sxz = 0.0;
		coeff[GMT_X] /= n;
		for (i = 0; i < n; i++) {
			xx = X[i][GMT_X] - coeff[GMT_X];
			zz = obs[i] - coeff[GMT_Z];
			sxx += (xx * xx);
			sxz += (xx * zz);
		}
		if (sxx != 0.0) coeff[3] = sxz/ sxx;
	}
	
	/* Remove linear trend (or mean) */
	
	coeff[5] = DBL_MAX;	coeff[6] = -DBL_MAX;
	for (i = 0; i < n; i++) {	/* Also find min/max */
		obs[i] -= coeff[GMT_Z];
		if (mode == 2) obs[i] -= (coeff[3] * (X[i][GMT_X] - coeff[GMT_X]));
		if (obs[i] < coeff[5]) coeff[5] = obs[i];
		if (obs[i] > coeff[6]) coeff[6] = obs[i];
	}
	if (mode & 4) {	/* Normalize by range */
		coeff[6] -= coeff[5];	/* Range */
		d = (coeff[6] == 0.0) ? 1.0 : 1.0 / coeff[6];
		for (i = 0; i < n; i++) obs[i] = (obs[i] - coeff[5]) * d;	/* Normalize 0-1 */
		coeff[GMT_Z] += coeff[5];	/* Combine the two constants in one place */
	}
	
	/* Recover obs(x) = w_norm(x) * coeff[6] + coeff[GMT_Z] + coeff[3]*(x-coeff[GMT_X]) */

}

void do_normalization (double **X, double *obs, GMT_LONG n, GMT_LONG mode, double *coeff)
{	/* mode == 1 norm z; mode == 2: also remove plane, norm & 4: also normalize result by range */

	GMT_LONG i;
	double d;
	
	GMT_memset (coeff, 5, double);
	for (i = 0; i < n; i++) {	/* Find mean z-value */
		coeff[GMT_Z] += obs[i];
		if (mode == 1) continue;
		coeff[GMT_X] += X[i][GMT_X];
		coeff[GMT_Y] += X[i][GMT_Y];
	}
	coeff[GMT_Z] /= n;

	if (mode & 2) {	/* Solve for LS plane using deviations from (0, 0, 0) */
		double	xx, yy, zz, sxx, sxy, sxz, syy, syz;
		sxx = sxy = sxz = syy = syz = 0.0;
		coeff[GMT_X] /= n;
		coeff[GMT_Y] /= n;
		for (i = 0; i < n; i++) {

			xx = X[i][GMT_X] - coeff[GMT_X];
			yy = X[i][GMT_Y] - coeff[GMT_Y];
			zz = obs[i] - coeff[GMT_Z];

			sxx += (xx * xx);
			sxz += (xx * zz);
			sxy += (xx * yy);
			syy += (yy * yy);
			syz += (yy * zz);
		}

		d = sxx*syy - sxy*sxy;
		if (d != 0.0) {
			coeff[3] = (sxz*syy - sxy*syz)/d;
			coeff[4] = (sxx*syz - sxy*sxz)/d;
		}
	}
	
	/* Remove plane (or mean) */
	
	coeff[5] = DBL_MAX;	coeff[6] = -DBL_MAX;
	for (i = 0; i < n; i++) {	/* Also find min/max */
		obs[i] -= coeff[GMT_Z];
		if (mode == 2) obs[i] -= (coeff[3] * (X[i][GMT_X] - coeff[GMT_X]) + coeff[4] * (X[i][GMT_Y] - coeff[GMT_Y]));
		if (obs[i] < coeff[5]) coeff[5] = obs[i];
		if (obs[i] > coeff[6]) coeff[6] = obs[i];
	}
	if (mode & 4) {	/* Normalize by range */
		coeff[6] -= coeff[5];	/* Range */
		d = (coeff[6] == 0.0) ? 1.0 : 1.0 / coeff[6];
		for (i = 0; i < n; i++) obs[i] = (obs[i] - coeff[5]) * d;	/* Normalize 0-1 */
		coeff[GMT_Z] += coeff[5];	/* Combine the two constants in one place */
	}
	
	/* Recover obs(x,y) = w_norm(x,y) * coeff[6] + coeff[GMT_Z] + coeff[3]*(x-coeff[GMT_X]) + coeff[4]*(y-coeff[GMT_Y]) */

}

double undo_normalization (double *X, double w_norm, GMT_LONG mode, double *coeff)
{
	double w;
	w = w_norm;
	if (mode & 4) w_norm *= coeff[6];
	w += coeff[GMT_Z];
	if (mode & 2) w += coeff[3] * (X[GMT_X] - coeff[GMT_X]) + coeff[4] * (X[GMT_Y] - coeff[GMT_Y]);
	return (w);
}

double get_radius (struct GMT_CTRL *GMT, double *X0, double *X1, GMT_LONG dim)
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

double get_dircosine (struct GMT_CTRL *GMT, double *D, double *X0, double *X1, GMT_LONG dim, GMT_LONG baz)
{
	/* D, the directional cosines of the observed gradient:
	 * X0: Observation point.
	 * X1: Prediction point.
	 * Compute N, the direction cosine of X1-X2, then C = D dot N.
	 */
	GMT_LONG ii;
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

#define Return(code) {Free_greenspline_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_greenspline (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, j, k, p, ii, n, m, nm, n_fields, error, dimension = 0;
	GMT_LONG n_expected_fields, normalize = 1, unit = 0, out_ID;
	GMT_LONG old_n_alloc, n_alloc, ij, ji, nxy, n_ok = 0, way;
	
	char *method[N_METHODS] = {"minimum curvature Cartesian spline [1-D]",
		"minimum curvature Cartesian spline [2-D]",
		"minimum curvature Cartesian spline [3-D]",
		"continuous curvature Cartesian spline in tension [1-D]",
		"continuous curvature Cartesian spline in tension [2-D]",
		"continuous curvature Cartesian spline in tension [3-D]",
		"regularized Cartesian spline in tension [2-D]",
		"regularized Cartesian spline in tension [3-D]",
		"minimum curvature spherical spline",
		"continuous curvature spherical spline in tension"};
	char *mem_unit[3] = {"kb", "Mb", "Gb"};
	
	double *obs = NULL, **D = NULL, **X = NULL, *alpha = NULL, *WB_z = NULL, *WB_g = NULL, *in = NULL;
	double mem, part, C, p_val, r, par[11], norm[7], az, grad;
	REAL *A = NULL;
#ifdef TEST
	double x0 = 0.0, x1 = 5.0;
#endif
	
	FILE *fp = NULL;

	PFD G = NULL, dGdr = NULL;

	struct GMT_GRID *Grid = NULL;
	struct ZGRID Z;
	struct GMT_TABLE *T = NULL;
	struct GMT_DATASET *Nin = NULL;
	struct GMT_GRD_INFO info;
	struct GREENSPLINE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_greenspline_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_greenspline_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_greenspline", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vb:", "ghiors>" GMT_OPT("FH"), options))) Return (error);
	Ctrl = (struct GREENSPLINE_CTRL *) New_greenspline_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_greenspline_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the greenspline main code ----------------------------*/

	dimension = (Ctrl->D.mode == 0) ? 1 : ((Ctrl->D.mode == 5) ? 3 : 2);
	GMT_memset (par,   7, double);
	GMT_memset (norm,  7, double);
	GMT_memset (&info, 1, struct GMT_GRD_INFO);
	GMT_memset (&Z,    1, struct ZGRID);
	
	if (Ctrl->S.mode == SANDWELL_1987_1D || Ctrl->S.mode == WESSEL_BERCOVICI_1998_1D) Ctrl->S.mode += (dimension - 1);	
	if (Ctrl->S.mode == MITASOVA_MITAS_1993_2D ) Ctrl->S.mode += (dimension - 2);	
	if (Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) Ctrl->D.mode = 4;	/* Automatically set */

	GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_LON;
	GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_LAT;

	way = GMT_IS_SPHERICAL (GMT) ? GMT_GREATCIRCLE : GMT_GEODESIC;
	Ctrl->D.mode--;	/* Since I added 0 to be 1-D later so now it is -1 */
	switch (Ctrl->D.mode) {	/* Set pointers to 2-D distance functions */
		case -1:	/* Cartesian 1-D x data */
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_FLOAT;
			GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
			normalize = 2;
			break;
		case 0:	/* Cartesian 2-D x,y data */
			GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT_IS_FLOAT;
			GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
			normalize = 2;
			break;
		case 1:	/* 2-D lon, lat data, but scale to Cartesian flat earth km */
			GMT_init_distaz (GMT, 'k', GMT_FLATEARTH, GMT_MAP_DIST);
			normalize = 2;
			break;
		case 2:	/* 2-D lon, lat data, use spherical distances in km (geodesic if ELLIPSOID is nor sphere) */
			GMT_init_distaz (GMT, 'k', way, GMT_MAP_DIST);
			break;
		case 3:	/* 2-D lon, lat data, and Green's function needs cosine of spherical or geodesic distance */
			GMT_init_distaz (GMT, 'S', way, GMT_MAP_DIST);
			break;
		case 4:	/* 3-D Cartesian x,y,z data handled separately */
			break;
		default:	/* Cannot happen unless we make a bug */
			GMT_report (GMT, GMT_MSG_FATAL, "BUG since D (=%ld) cannot be outside 0-5 range\n", Ctrl->D.mode+1);
			break;
	}

	if (Ctrl->D.mode <= 1 && Ctrl->L.active)
		normalize = 1;	/* Do not de-plane, just remove mean and normalize */
	else if (Ctrl->D.mode > 1 && Ctrl->L.active)
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: -L ignored for -D modes 2 and 3\n");
	
	if (Ctrl->Q.active && dimension == 2) sincosd (Ctrl->Q.az, &Ctrl->Q.dir[GMT_X], &Ctrl->Q.dir[GMT_Y]);
	
	/* Now we are ready to take on some input values */

	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */

	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : dimension + 1;

	n_alloc = GMT_CHUNK;
	X = GMT_memory (GMT, NULL, n_alloc, double *);
	for (k = 0; k < n_alloc; k++) X[k] = GMT_memory (GMT, NULL, dimension, double);
	obs = GMT_memory (GMT, NULL, n_alloc, double);

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
	n = m = 0;
	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */

		for (k = 0; k < dimension; k++) X[n][k] = in[k];
		obs[n++] = in[dimension];

		if (n == n_alloc) {	/* Get more memory */
			old_n_alloc = n_alloc;
			n_alloc <<= 1;
			X = GMT_memory (GMT, X, n_alloc, double *);
			for (k = old_n_alloc; k < n_alloc; k++) X[k] = GMT_memory (GMT, (void *)X[k], dimension, double);
			obs = GMT_memory (GMT, obs, n_alloc, double);
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	for (k = n; k < (GMT_LONG)n_alloc; k++) GMT_free (GMT, X[k]);	/* Remove what was not used */
	X = GMT_memory (GMT, X, n, double *);
	obs = GMT_memory (GMT, obs, n, double);
	nm = n;

	if (Ctrl->A.active) {	/* Read gradient constraints from file */
		struct GMT_DATASET *Din = NULL;
		struct GMT_TABLE *S = NULL;
		if (GMT->common.b.active[GMT_IN]) GMT->common.b.ncol[GMT_IN]++;	/* Must assume it is just one extra column */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->A.file, (void **)&Din)) return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		S = Din->table[0];	/* Can only be one table */
		m = S->n_records;	/* Total number of gradient constraints */
		nm += m;		/* New total of linear equations to solve */
		X = GMT_memory (GMT, X, nm, double *);
		for (k = n; k < nm; k++) X[k] = GMT_memory (GMT, NULL, dimension, double);
		obs = GMT_memory (GMT, obs, nm, double);
		D = GMT_memory (GMT, NULL, m, double *);
		for (k = 0; k < m; k++) D[k] = GMT_memory (GMT, NULL, dimension, double);
		for (i = k = 0, p = n; i < S->n_segments; i++) {
			for (j = 0; j < S->segment[i]->n_rows; j++, k++, p++) {
				for (ii = 0; ii < dimension; ii++) X[p][ii] = S->segment[i]->coord[ii][j];
				switch (dimension) {
					case 1:	/* 1-D */
						switch (Ctrl->A.mode) {
							case 0:	/* x, slope */
								D[k][0] = 1.0;	/* Dummy since there is no direction for 1-D spline (the gradient is in the x-y plane) */
								obs[p] = S->segment[i]->coord[dimension][j];
								break;
							default:
								GMT_report (GMT, GMT_MSG_FATAL, "Bad gradient mode selected for 1-D data (%ld) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						break;
					case 2:	/* 2-D */
						switch (Ctrl->A.mode) {
							case 1:	/* (x, y, az, gradient) */
								az = D2R * S->segment[i]->coord[2][j];
								obs[p] = S->segment[i]->coord[3][j];
								break;
							case 2:	/* (x, y, gradient, azimuth) */
								az = D2R * S->segment[i]->coord[3][j];
								obs[p] = S->segment[i]->coord[2][j];
								break;
							case 3:	/* (x, y, direction, gradient) */
								az = M_PI_2 - D2R * S->segment[i]->coord[2][j];
								obs[p] = S->segment[i]->coord[3][j];
								break;
							case 4:	/* (x, y, gx, gy) */
								az = atan2 (S->segment[i]->coord[2][j], S->segment[i]->coord[3][j]);		/* Get azimuth of gradient */
								obs[p] = hypot (S->segment[i]->coord[3][j], S->segment[i]->coord[3][j]);	/* Get magnitude of gradient */
								break;
							case 5:	/* (x, y, nx, ny, gradient) */
								az = atan2 (S->segment[i]->coord[2][j], S->segment[i]->coord[3][j]);		/* Get azimuth of gradient */
								obs[p] = S->segment[i]->coord[4][j];	/* Magnitude of gradient */
								break;
							default:
								GMT_report (GMT, GMT_MSG_FATAL, "Bad gradient mode selected for 2-D data (%ld) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						sincos (az, &D[k][GMT_X], &D[k][GMT_Y]);
						break;
					case 3:	/* 3-D */
						switch (Ctrl->A.mode) {
							case 4:	/* (x, y, z, gx, gy, gz) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = S->segment[i]->coord[3+ii][j];	/* Get the gradient vector */
								obs[p] = GMT_mag3v (GMT, D[k]);	/* This is the gradient magnitude */
								GMT_normalize3v (GMT, D[k]);		/* These are the direction cosines of the gradient */
								break;
							case 5: /* (x, y, z, nx, ny, nz, gradient) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = S->segment[i]->coord[3+ii][j];	/* Get the unit vector */
								obs[p] = S->segment[i]->coord[6][j];	/* This is the gradient magnitude */
								break;
							default:
								GMT_report (GMT, GMT_MSG_FATAL, "Bad gradient mode selected for 3-D data (%ld) - aborting!\n", Ctrl->A.mode);
								Return (GMT_DATA_READ_ERROR);
								break;
						}
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Bad dimension selected (%ld) - aborting!\n", dimension);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
			}
		}
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Din);
	}
	
	if (m > 0 && normalize > 1) {
		normalize &= 1;	/* Only allow taking out data mean for mixed z/slope data */
		GMT_report (GMT, GMT_MSG_FATAL, "Only remove/restore mean z in mixed {z, grad(z)} data sets\n");
	}
	
	if (m == 0)
		GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld data points, yielding a %ld by %ld set of linear equations\n", n, nm, nm);
	else 
		GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld data points and %ld gradients, yielding a %ld by %ld set of linear equations\n", n, m, nm, nm);
		
	if (Ctrl->T.file) {	/* Existing grid that will have zeros and NaNs, only */
		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->T.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
		if (! (Grid->header->wesn[XLO] == Ctrl->R3.range[0] && Grid->header->wesn[XHI] == Ctrl->R3.range[1] && Grid->header->wesn[YLO] == Ctrl->R3.range[2] && Grid->header->wesn[YHI] == Ctrl->R3.range[3])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The mask grid does not match your specified region\n");
			Return (EXIT_FAILURE);
		}
		if (! (Grid->header->inc[GMT_X] == Ctrl->I.inc[GMT_X] && Grid->header->inc[GMT_Y] == Ctrl->I.inc[GMT_Y])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The mask grid resolution does not match your specified grid spacing\n");
			Return (EXIT_FAILURE);
		}
		if (! (Grid->header->registration == GMT->common.r.active)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The mask grid registration does not match your specified grid registration\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(Ctrl->T.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get data */
		nxy = n_ok = Grid->header->size;
		for (ij = 0; ij < nxy; ij++) if (GMT_is_fnan (Grid->data[ij])) n_ok--;
	}
	else if (Ctrl->N.active) {	/* Read output locations from file */
		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->N.file, (void **)&Nin)) return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		T = Nin->table[0];
	}
	else {	/* Fill in an equidistant output table or grid */
		Grid = GMT_create_grid (GMT);
		Grid->header->wesn[XLO] = Ctrl->R3.range[0];	Grid->header->wesn[XHI] = Ctrl->R3.range[1];
		Grid->header->registration = (int)GMT->common.r.active;
		Grid->header->inc[GMT_X] = Ctrl->I.inc[GMT_X];
		Z.nz = Grid->header->ny = 1;	/* So that output logic will work for lower dimensions */
		if (dimension > 1) {
			Grid->header->wesn[YLO] = Ctrl->R3.range[2];	Grid->header->wesn[YHI] = Ctrl->R3.range[3];
			Grid->header->inc[GMT_Y] = Ctrl->I.inc[GMT_Y];
			GMT_RI_prepare (GMT, Grid->header);	/* Ensure -R -I consistency and set nx, ny */
			GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);
			GMT_set_grddim (GMT, Grid->header);
			if (dimension == 3) {	/* Also set nz */
				Z.z_min = Ctrl->R3.range[4];	Z.z_max = Ctrl->R3.range[5];
				Z.z_inc = Ctrl->I.inc[GMT_Z];
				Z.nz = GMT_get_n (Z.z_min, Z.z_max, Z.z_inc, Grid->header->registration);
			}
		}
		else
			Grid->header->nx = GMT_grd_get_nx (Grid->header);
		nxy = n_ok = Grid->header->size * Z.nz;
		if (dimension == 2) Grid->data = GMT_memory (GMT, NULL, nxy, float);
	}

	switch (Ctrl->S.mode) {	/* Assing pointers to Green's functions and the gradient and set up required parameters */
		case SANDWELL_1987_1D:
			G = spline1d_sandwell;
			dGdr = gradspline1d_sandwell;
			break;
		case SANDWELL_1987_2D:
			G = spline2d_sandwell;
			dGdr = gradspline2d_sandwell;
			break;
		case SANDWELL_1987_3D:
			G = spline3d_sandwell;
			dGdr = gradspline3d_sandwell;
			break;
		case WESSEL_BERCOVICI_1998_1D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = Grid->header->inc[GMT_X];
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = spline1d_Wessel_Bercovici;
			dGdr = gradspline1d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_2D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = 0.5 * (Grid->header->inc[GMT_X] + Grid->header->inc[GMT_Y]);
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = spline2d_Wessel_Bercovici;
			dGdr = gradspline2d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_3D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = (Grid->header->inc[GMT_X] + Grid->header->inc[GMT_Y] + Z.z_inc) / 3.0;
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = spline3d_Wessel_Bercovici;
			dGdr = gradspline3d_Wessel_Bercovici;
			break;
		case MITASOVA_MITAS_1993_2D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_report (GMT, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = spline2d_Mitasova_Mitas;
			dGdr = gradspline2d_Mitasova_Mitas;
			break;
		case MITASOVA_MITAS_1993_3D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_report (GMT, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = spline3d_Mitasova_Mitas;
			dGdr = gradspline3d_Mitasova_Mitas;
			break;
		case PARKER_1994:
			par[0] = 6.0 / (M_PI*M_PI);
			G = spline2d_Parker;
			dGdr = gradspline2d_Parker;
#ifdef TEST
			x0 = -1.0;	x1 = 1.0;
#endif
			break;
		case WESSEL_BECKER_2008:
			par[0] = -0.5;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0]));
			if (p_val <= 0.5) {	/* nu is real */
				double z[2];
				par[0] += sqrt (0.25 - p_val * p_val);
				par[1] = 0.0;
				par[4] = sin (M_PI * par[0]);
				z[0] = par[0] + 1.0;	z[1] = 0.0;
				par[3] = (M_PI / tan (M_PI * par[0])) - M_LOG_2 + 2.0 * (M_GAMMA + GMT_psi (GMT, z, NULL));
			}
			else {	/* nu is complex */
				double z[2], cot_piv[2], psi[2];
				par[1] = sqrt (p_val * p_val - 0.25);
				par[4] = -cosh (M_PI * par[1]);
				z[0] = par[0] * M_PI;	z[1] = par[1] * M_PI;
				Ccot (z, cot_piv);
				cot_piv[0] *= M_PI;	cot_piv[1] *= M_PI;
				z[0] = par[0] + 1.0;	z[1] = par[1];
				(void) GMT_psi (GMT, z, psi);
				psi[0] += M_GAMMA;
				psi[0] *= 2.0;	psi[1] *= 2.0;
				z[0] = cot_piv[0] + psi[0] - M_LOG_2;	z[1] = cot_piv[1] + psi[1];
				par[3] = z[0];	/* Ignore complex parts which cancels out */
			}
			par[2] = (M_PI / par[4]) - M_LOG_2;
			par[6] = 1.0 / (par[3] - par[2]);
			if (Ctrl->S.fast) {
				GMT_LONG nx;
				par[7] = Ctrl->S.value[1];
				par[8] = (Ctrl->S.rval[1] - Ctrl->S.rval[0]) / (par[7] - 1.0);
				par[9] = 1.0 / par[8];
				par[10] = Ctrl->S.rval[0];
				nx = (GMT_LONG)irint (par[7]);
				if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "Precalculate -SQ lookup table with %ld items from %g to %g...", nx, Ctrl->S.rval[0], Ctrl->S.rval[1]);
				WB_z = GMT_memory (GMT, NULL, nx, double);
				if (Ctrl->A.active) WB_g = GMT_memory (GMT, NULL, nx, double);
				spline2d_Wessel_Becker_init (GMT, par, WB_z, WB_g, Ctrl->A.active);
				if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "done\n");
				G = spline2d_Wessel_Becker_lookup;
				dGdr = gradspline2d_Wessel_Becker_lookup;
			}
			else {
				G = spline2d_Wessel_Becker;
				dGdr = gradspline2d_Wessel_Becker;
			}
#ifdef TEST
			x0 = -1.0;	x1 = 1.0;
#endif
			break;
	}

#ifdef TEST
	printf ("# %s\n", method[Ctrl->S.mode]);
	dump_green (G, dGdr, par, x0, x1, 10001, WB_z, WB_g);
	Return (0);
#endif

	/* Remove mean (or LS plane) from data (we will add it back later) */

	do_normalization (X, obs, n, normalize, norm);
		
	/* Set up linear system Ax = z */
	
	mem = ((double)nm * (double)nm * (double)sizeof (REAL)) / 1024.0;	/* In kb */
	unit = 0;
	while (mem > 1024.0 && unit < 2) { mem /= 1024.0; unit++; }	/* Select next unit */
	GMT_report (GMT, GMT_MSG_NORMAL, "Square matrix requires %.1f %s\n", mem, mem_unit[unit]);
	A = GMT_memory (GMT, NULL, nm * nm, REAL);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Build linear system using %s\n", method[Ctrl->S.mode]);

	Ctrl->S.rval[0] = DBL_MAX;	Ctrl->S.rval[1] = -DBL_MAX;
	for (j = 0; j < nm; j++) {	/* For each value or slope constraint */
		for (i = j; i < nm; i++) {
			ij = j * nm + i;
			ji = i * nm + j;
			r = get_radius (GMT, X[i], X[j], dimension);
			if (r < Ctrl->S.rval[0]) Ctrl->S.rval[0] = r;
			if (r > Ctrl->S.rval[1]) Ctrl->S.rval[1] = r;
			if (j < n) {	/* Value constraint */
				A[ij] = G (GMT, r, par, WB_z);
				if (ij == ji) continue;	/* Do the diagonal terms only once */
				if (i < n)
					A[ji] = A[ij];
				else {
					/* Get D, the directional cosine between the two points */
					/* Then get C = GMT_dot3v (GMT, D, dataD); */
					/* A[ji] = dGdr (r, par, WB_g) * C; */
					C = get_dircosine (GMT, D[i-n], X[i], X[j], dimension, FALSE);
					grad = dGdr (GMT, r, par, WB_g);
					A[ji] = grad * C;
				}
			}
			else if (i > n) {	/* Remaining gradient constraints */
				if (ij == ji) continue;	/* Diagonal gradient terms are zero */
				C = get_dircosine (GMT, D[j-n], X[i], X[j], dimension, TRUE);
				grad = dGdr (GMT, r, par, WB_g);
				A[ij] = grad * C;
				C = get_dircosine (GMT, D[i-n], X[i], X[j], dimension, FALSE);
				A[ji] = grad * C;
			}
		}
	}

	if (Ctrl->C.active) {		/* Solve using svd decomposition */
		GMT_LONG n_use, error;
		REAL *v = NULL, *s = NULL, *b = NULL, eig_max = 0.0;
		
		GMT_report (GMT, GMT_MSG_NORMAL, "Solve linear equations by SVD ");
	
		v = GMT_memory (GMT, NULL, nm * nm, REAL);
		s = GMT_memory (GMT, NULL, nm, REAL);
		if ((error = svdcmp (GMT, A, nm, nm, s, v))) Return (error);
		if (Ctrl->C.file) {	/* Save the eigen-values for study */
			char format[GMT_LONG_TEXT];
			REAL *eig = GMT_memory (GMT, NULL, nm, REAL);
			GMT_memcpy (eig, s, nm, REAL);
			if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "Eigen-value rations s(i)/s(0) saved to %s\n", Ctrl->C.file);
			if ((fp = GMT_fopen (GMT, Ctrl->C.file, "w")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error creating file %s\n", Ctrl->C.file);
				Return (EXIT_FAILURE);
			}
			sprintf (format, "%%d\t%s\n", GMT->current.setting.format_float_out);
			/* Sort eigenvalues into ascending order */
			if (sizeof (REAL) == sizeof (double))
				GMT_sort_array ((void *)eig, nm, GMT_DOUBLE_TYPE);
			else
				GMT_sort_array ((void *)eig, nm, GMT_FLOAT_TYPE);
			eig_max = eig[nm-1];
			for (i = 0, j = nm-1; i < nm; i++, j--) fprintf (fp, format, i, eig[j] / eig_max);
			GMT_fclose (GMT, fp);
			GMT_free (GMT, eig);
			if (Ctrl->C.value < 0.0) Return (EXIT_SUCCESS);
		}
		b = GMT_memory (GMT, NULL, nm, REAL);
		if (sizeof (REAL) == sizeof (double)) {
			GMT_memcpy (b, obs, nm, REAL);
			n_use = solve_svd (GMT, A, nm, nm, v, s, b, 1, obs, Ctrl->C.value);
			if (n_use == -1) Return (EXIT_FAILURE);
		}
		else {	/* Must use temporary float array to capture result */
			REAL *z4 = NULL;
			for (i = 0; i < nm; i++) b[i] = (REAL)obs[i];
			z4 = GMT_memory (GMT, NULL, nm, REAL);
			n_use = solve_svd (GMT, A, nm, nm, v, s, b, 1, z4, Ctrl->C.value);
			if (n_use == -1) Return (EXIT_FAILURE);
			for (i = 0; i < nm; i++) obs[i] = (double)z4[i];
			GMT_free (GMT, z4);
		}
		if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "[%ld of %ld eigen-values used]\n", n_use, nm);
			
		GMT_free (GMT, s);
		GMT_free (GMT, v);
		GMT_free (GMT, b);
	}
	else {				/* Gauss-Jordan elimination */
		GMT_LONG error;
		GMT_report (GMT, GMT_MSG_NORMAL, "Solve linear equations by Gauss-Jordan elimination\n");
		if (sizeof (REAL) == sizeof (double)) {
			if ((error = gaussj (GMT, A, nm, nm, obs, 1, 1))) Return (error);
		}
		else {
			REAL *z4 = NULL;
			z4 = GMT_memory (GMT, NULL, nm, REAL);
			for (i = 0; i < nm; i++) z4[i] = (REAL)obs[i];
			if ((error = gaussj (GMT, A, nm, nm, z4, 1, 1))) Return (error);
			for (i = 0; i < nm; i++) obs[i] = (double)z4[i];
			GMT_free (GMT, z4);
		}
	}
	alpha = obs;	/* Just a different name since the obs vector now holds the alpha factors */
		
	GMT_free (GMT, A);

	if (Ctrl->N.file) {	/* Specified nodes only */
		double out[4];
	
		if (Ctrl->G.active) GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, (void **)&Ctrl->G.file, NULL, NULL, &out_ID);
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);				/* Enables data output and sets access mode */
	
		GMT->common.b.ncol[GMT_OUT] = dimension + 1;
		GMT_report (GMT, GMT_MSG_NORMAL, "Evaluate spline at %ld given locations\n", T->n_records);
		for (i = 0; i < T->n_segments; i++) {
			for (j = 0; j < T->segment[i]->n_rows; j++) {
				for (ii = 0; ii < dimension; ii++) out[ii] = T->segment[i]->coord[ii][j];
				out[dimension] = 0.0;
				for (p = 0; p < nm; p++) {
					r = get_radius (GMT, out, X[p], dimension);
					if (r < Ctrl->S.rval[0]) Ctrl->S.rval[0] = r;
					if (r > Ctrl->S.rval[1]) Ctrl->S.rval[1] = r;
					if (Ctrl->Q.active) {
						C = get_dircosine (GMT, Ctrl->Q.dir, out, X[p], dimension, FALSE);
						part = dGdr (GMT, r, par, WB_z) * C;
					}
					else
						part = G (GMT, r, par, WB_z);
					out[dimension] += alpha[p] * part;
				}
				out[dimension] = undo_normalization (out, out[dimension], normalize, norm);
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
			}
		}
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Nin);
		GMT_fclose (GMT, fp);
	}
	else {
		GMT_LONG nz_off, nxy, col, row;
		double *xp = NULL, *yp = NULL, wp, V[4];
		GMT_report (GMT, GMT_MSG_NORMAL, "Evaluate spline at %ld equidistant output locations\n", n_ok);
		/* Precalculate coordinates */
		xp = GMT_memory (GMT, NULL, Grid->header->nx, double);
		for (col = 0; col < Grid->header->nx; col++) xp[col] = GMT_grd_col_to_x (col, Grid->header);
		if (dimension > 1) {
			yp = GMT_memory (GMT, NULL, Grid->header->ny, double);
			for (row = 0; row < Grid->header->ny; row++) yp[row] = GMT_grd_row_to_y (row, Grid->header);
		}
		nxy = Grid->header->size;
		GMT->common.b.ncol[GMT_OUT] = dimension + 1;
		if (dimension != 2) {	/* Write ascii table to named file or stdout */
			if (Ctrl->G.active)
				GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, (void **)&Ctrl->G.file, NULL, (void *)Grid, &out_ID);
			else
				if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes std output */
			if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */
		}
		for (k = nz_off = 0; k < Z.nz; k++, nz_off += nxy) {
			if (dimension == 3) V[GMT_Z] = GMT_col_to_x (k, Z.z_min, Z.z_max, Z.z_inc, Grid->header->xy_off, Z.nz);
			for (row = 0; row < Grid->header->ny; row++) {
				if (dimension > 1) V[GMT_Y] = yp[row];
				for (col = 0; col < Grid->header->nx; col++) {
					ij = GMT_IJP (Grid->header, row, col) + nz_off;
					if (dimension == 2 && GMT_is_fnan (Grid->data[ij])) continue;	/* Only do solution where mask is not NaN */
					V[GMT_X] = xp[col];
					/* Here, V holds the current output coordinates */
					for (p = 0, wp = 0.0; p < nm; p++) {
						r = get_radius (GMT, V, X[p], dimension);
						if (r < Ctrl->S.rval[0]) Ctrl->S.rval[0] = r;
						if (r > Ctrl->S.rval[1]) Ctrl->S.rval[1] = r;
						if (Ctrl->Q.active) {
							C = get_dircosine (GMT, Ctrl->Q.dir, V, X[p], dimension, FALSE);
							part = dGdr (GMT, r, par, WB_z) * C;
						}
						else
							part = G (GMT, r, par, WB_z);
						wp += alpha[p] * part;
					}
					V[dimension] = (float)undo_normalization (V, wp, normalize, norm);
					if (dimension == 2)	/* Special 2-D grid output */
						Grid->data[ij] = (float)V[dimension];
					else	/* Crude dump for now for both 1-D and 3-D */
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)V);
				}
			}
		}
		if (dimension == 2) {
			GMT_grd_init (GMT, Grid->header, options, TRUE);
			sprintf (Grid->header->remark, "Method: %s (%s)", method[Ctrl->S.mode], Ctrl->S.arg);
			if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
			if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
		}
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);
			
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
	if (Ctrl->S.fast) {
		GMT_free (GMT, WB_z);
		if (Ctrl->A.active) GMT_free (GMT, WB_g);
	}
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Done [ R min/max = %g/%g]\n", Ctrl->S.rval[0], Ctrl->S.rval[1]);

	Return (EXIT_SUCCESS);
}
