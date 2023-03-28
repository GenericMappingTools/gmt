/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
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
 *
 * Note on KEYS: CD)=f means -C takes an optional output Dataset as argument via the +f modifier.
 *               AD(= means -A takes an optional input Dataset as argument which may be followed by optional modifiers.
 */

#include "gmt_dev.h"
#include "longopt/greenspline_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"greenspline"
#define THIS_MODULE_MODERN_NAME	"greenspline"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate using Green's functions for splines in 1-3 dimensions"
#define THIS_MODULE_KEYS	"<D{,AD(=,ED),ND(,TG(,CD)=f,G?},GDN"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:>Vbdefghioqrsw" GMT_OPT("FH") GMT_ADD_x_OPT

EXTERN_MSC int gmtlib_cspline (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *c);

/* Control structure for greenspline */

struct GREENSPLINE_CTRL {
	unsigned int dimension;
	struct GREENSPLINE_A {	/* -A<gradientfile> */
		bool active;
		unsigned int mode;	/* 0 = azimuths, 1 = directions, 2 = dx,dy components, 3 = dx, dy, dz components */
		char *file;
	} A	;
	struct GREENSPLINE_C {	/* -C[[n|r|v]<cutoff>[%]][+c][+f<file>][+i][+n] */
		bool active;
		bool dryrun;	/* Only report eigenvalues */
		unsigned int history;
		unsigned int mode;
		double value;
		char *file;
	} C;
	struct GREENSPLINE_D {	/* -D[+x<xname>][+yyname>][+z<zname>][+v<zname>][+s<scale>][+o<offset>][+n<invalid>][+t<title>][+r<remark>] */
		bool active;
		char *information;
	} D;
	struct GREENSPLINE_E {	/* -E[<file>] */
		bool active;
		unsigned int mode;
		char *file;
	} E;
	struct GREENSPLINE_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct GREENSPLINE_I {	/* -Idx[/dy[/dz]] */
		bool active;
		double inc[3];
	} I;
	struct GREENSPLINE_L {	/* -L */
		bool active;
		bool detrend;	/* true if OK to detrend a linear curve/plane */
		bool derange;	/* true if OK to normalized by largest absolute range */
	} L;
	struct GREENSPLINE_M {	/* -M<gfuncfile> */
		bool active;
		unsigned int mode;	/* GMT_IN or GMT_OUT */
		char *file;
	} M;
	struct GREENSPLINE_N {	/* -N<outputnode_file> */
		bool active;
		char *file;
	} N;
	struct GREENSPLINE_Q {	/* -Q[<az>|x/y/z] */
		bool active;
		double az;		/* 2-D azimuth for directional derivative */
		double dir[3];	/* 3-D vector for directional derivative */
	} Q;
	struct GREENSPLINE_R3 {	/* -Rxmin/xmax[/ymin/ymax[/zmin/zmaz]] | -Ggridfile */
		bool active;
		bool mode;		/* true if settings came from a grid file */
		unsigned int dimension;	/* 1, 2, or 3 */
		unsigned int offset;	/* 0 or 1 */
		double range[6];	/* Min/max for each dimension */
		double inc[2];		/* xinc/yinc when -Rgridfile was given*/
	} R3;
	struct GREENSPLINE_S {	/* -S<mode>[<tension][+<mod>[args]] */
		bool active;
		unsigned int mode;
		double value[4];
		double rval[2];
		char *arg;
	} S;
	struct GREENSPLINE_T {	/* -T<mask_grdfile> */
		bool active;
		char *file;
	} T;
	struct GREENSPLINE_W {	/* -W[w] */
		bool active;
		unsigned int mode;	/* 0 = got sigmas, 1 = got weights */
	} W;
	struct GREENSPLINE_Z {	/* -Z<distflag> */
		bool active;
		int mode;	/* Can be negative */
	} Z;
	struct GREENSPLINE_DEBUG {
		bool test;		/* true for -+ to output table of normalized Green's function and slope */
		bool active;	/* true for -/ undocumented debugging option to dump A | b matrix */
	} debug;
};

enum Greenspline_modes {	/* Various integer mode flags */
	SANDWELL_1987_1D		= 0,
	SANDWELL_1987_2D		= 1,
	SANDWELL_1987_3D		= 2,
	WESSEL_BERCOVICI_1998_1D	= 3,
	WESSEL_BERCOVICI_1998_2D	= 4,
	WESSEL_BERCOVICI_1998_3D	= 5,
	MITASOVA_MITAS_1993_2D		= 6,
	MITASOVA_MITAS_1993_3D		= 7,
	PARKER_1994			= 8,
	WESSEL_BECKER_2008		= 9,
	LINEAR_1D			= 10,
	LINEAR_2D			= 11,
	N_METHODS			= 12,
	N_PARAMS			= 11,
	GREENSPLINE_MEAN		= 0,		/* Remove/Restore mean value only */
	GREENSPLINE_TREND		= 1,		/* Remove/Restore linear trend */
	GREENSPLINE_NORM		= 2,		/* Normalize residual data to 0-1 range */
	SQ_N_NODES 			= 10001,	/* Default number of nodes in the precalculated -Sq spline */
	GSP_GOT_SIG			= 0,
	GSP_GOT_WEIGHTS			= 1
};

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

#define SQ_TRUNC_ERROR		1.0e-6	/* Max truncation error in Parker's simplified sum for WB'08 */

enum Greenspline_index {	/* Indices for coeff array for normalization */
	GSP_MEAN_X	= 0,
	GSP_MEAN_Y	= 1,
	GSP_MEAN_Z	= 2,
	GSP_SLP_X	= 3,
	GSP_SLP_Y	= 4,
	GSP_RANGE	= 5,
	GSP_SLP_Q	= 6,
	GSP_LENGTH	= 7};

struct GREENSPLINE_LOOKUP {	/* Used to spline interpolation of precalculated function */
	uint64_t n;		/* Number of values in the spline setup */
	double *y;		/* Function values */
	double *c;		/* spline  coefficients */
	double *A, *B, *C;	/* power/ratios of order l terms */
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GREENSPLINE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GREENSPLINE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.mode = GMT_SVD_EIGEN_RATIO_CUTOFF;
	C->L.detrend = C->L.derange = true;	/* This is the default action if permissible by the geometry */
	C->Q.az = GMT->session.d_NaN;	/* To tell if -Q was not given an argument later for dim > 1 */
	C->S.mode = SANDWELL_1987_2D;
	C->S.rval[0] = -1.0;	C->S.rval[1] = 1.0;
	C->S.value[3] = (double)SQ_N_NODES;	/* Default number of spline nodes */
	C->S.value[2] = SQ_TRUNC_ERROR;		/* Default truncation error for Legendre sum in -Sq */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GREENSPLINE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->S.arg);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -G<outfile> [-A<gradientfile>+f<format>] [-C[[n|r|v]<val>[%%]][+c][+f<file>][+i][+n]] "
		"[-D<information>] [-E[<misfitfile>]] [-I<dx>[/<dy>[/<dz>]]] [-L[t][r]] [-N<nodefile>] [-Q[<az>|<x/y/z>]] "
		"[-R<xmin>/<xmax>[/<ymin>/<ymax>[/<zmin>/<zmax>]]] [-Sc|l|t|r|p|q[<pars>]] [-T<maskgrid>] "
		"[%s] [-W[w]] [-Z<mode>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]%s[%s] [%s]\n",
		name, GMT_V_OPT,GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT,
		GMT_o_OPT, GMT_q_OPT, GMT_r_OPT, GMT_s_OPT, GMT_w_OPT, GMT_x_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Usage (API, -1, "Choose one of three ways to specify where to evaluate the spline:");
	GMT_Usage (API, 2, "%s Specify an equidistant 1-, 2-, or 3-D domain with options -R, -I [and optionally -r].", GMT_LINE_BULLET);
	GMT_Usage (API, 2, "%s Specify a set of 1- ,2-, or 3-D output locations via the -N option.", GMT_LINE_BULLET);
	GMT_Usage (API, 2, "%s Supply a mask grid file via -T whose values are NaN or 0.  The spline will then "
		"only be evaluated at the nodes originally set to zero (2-D only).", GMT_LINE_BULLET);
	GMT_Message (API, GMT_TIME_NONE, "\n  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, 1, "\n-G<outfile>");
	GMT_Usage (API, -2, "Output data. Append name of output file.");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");

	GMT_Usage (API, 1, "\n-A<gradientfile>+f<format>");
	GMT_Usage (API, -2, "ASCII file with surface gradients V to use in the modeling.  Specify format:");
	GMT_Usage (API, 3, "0: For 1-D: x, slope.");
	GMT_Usage (API, 3, "1: X, Vmagnitude, Vazimuth(s).");
	GMT_Usage (API, 3, "2: X, Vazimuth(s), Vmagnitude.");
	GMT_Usage (API, 3, "3: X, Vmagnitude, Vangle(s).");
	GMT_Usage (API, 3, "4: X, Vcomponents.");
	GMT_Usage (API, 3, "5: X, Vunit-vector, Vmagnitude.");
	GMT_Usage (API, -2, "Here, X = (x, y[, z]) is the position vector, V = (Vx, Vy[, Vz]) is the gradient vector.");
	GMT_Usage (API, 1, "\n-C[[n|r|v]<val>[%%]][+c][+f<file>][+i][+n]");
	GMT_Usage (API, -2, "Solve by SVD and control how many eigenvalues to use. Optionally append a directive and value:");
	GMT_Usage (API, 3, "n: Only use the largest <val> eigenvalues [all].");
	GMT_Usage (API, 3, "r: Eliminate eigenvalues whose ratio to largest eigenvalue is less than <val> [0].");
	GMT_Usage (API, 3, "v: Include eigenvalues needed to reach a variance explained fraction of <val> [1].");
	GMT_Usage (API, -2, "Note: For r|v you may append %% to give <val> as the percentage of total instead. Various optional modifiers are available:");
	GMT_Usage (API, 3, "+c Valid for 2-D gridding only and will create a series of intermediate "
		"grids for each eigenvalue holding the cumulative result. Requires -G with a valid filename "
		"and extension and we will insert _cum_### before the extension.");
	GMT_Usage (API, 3, "+f Save the eigenvalues to <filename>.");
	GMT_Usage (API, 3, "+i As +c but save incremental results, inserting _inc_### before the extension.");
	GMT_Usage (API, 3, "+n Stop execution after reporting the eigenvalues - no solution is computed.");
	GMT_Usage (API, -2, "Note: Without -C we use Gauss-Jordan elimination to solve the linear system.");
	gmt_grdcube_info_syntax (API->GMT, 'D');
	GMT_Usage (API, 1, "\n-E[<misfitfile>]");
	GMT_Usage (API, -2, "Evaluate solution at input locations and report misfit statistics. "
		"Append a filename to save all data with two extra columns for model and misfit. "
		"If -C+i|c are used then we instead report the history of model variance and rms misfit.");
	GMT_Usage (API, 1, "\n-I<dx>[/<dy>[/<dz>]]");
	GMT_Usage (API, -2, "Specify a regular set of output locations. Give equidistant increment for each dimension. "
		"Requires -R for specifying the output domain.");
	GMT_Usage (API, 1, "\n-L Control the trend function T(x) and normalization used. We always remove/restore the mean data value. "
		"We can (and by default will) remove/restore a linear trend and normalize/renormalize by data residual range. Change default by adding on or both of these directives:");
	GMT_Usage (API, 3, "t: Fit a least-squares line (1-D) or plane (2-D) and remove/restore this trend from data and gradients [Default is possible].");
	GMT_Usage (API, 3, "r: Normalize data and gradients by the largest absolute value of the minimum and maximum residual value [Default].");
	GMT_Usage (API, -2, "Note that giving -L with no arguments will turn off both detrending and normalization.");
	GMT_Usage (API, 1, "\n-N<nodefile>");
	GMT_Usage (API, -2, "ASCII file with desired output locations. "
		"The resulting ASCII coordinates and interpolation are written to file given in -G "
		"or standard output if no file specified (see -bo for binary output).");
	GMT_Usage (API, 1, "\n-Q[<az>|<x/y/z>]");
	GMT_Usage (API, -2, "Calculate the directional derivative and return it instead of curve or surface elevation w:");
	GMT_Usage (API, 3, "%s 1-D: We compute first-derivative dw/dx.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s 2-D: Append <az> direction for gradient of w(x,y) in that direction.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s 3-D: Append vector direction <x/y/z> for gradient of w(x,y,z) in that direction.", GMT_LINE_BULLET);
	GMT_Option (API, "R");
	if (gmt_M_showusage (API)) {
		GMT_Usage (API, -2, "-R Specify a regular set of output locations.  Give min and max coordinates for each dimension. "
			"Requires -I for specifying equidistant increments.  For 2-D gridding a gridfile may be given; "
			"this then also sets -I (and perhaps -r); use those options to override the grid settings.");
	}
	GMT_Usage (API, 1, "\n-Sc|l|t|r|p|q[<pars>]");
	GMT_Usage (API, -2, "Specify which spline to use; except for c|p, append normalized <tension> between 0 and 1:");
	GMT_Usage (API, 3, "c: Minimum curvature spline (Sandwell, 1987) [Default].");
	GMT_Usage (API, 3, "l: Linear (1-D) or bilinear (2-D) spline.");
	GMT_Usage (API, 3, "t: Cartesian spline in tension (Wessel & Bercovici, 1998). Append <tension> and "
		"optionally append /<scale> for length-scale [Default is the given output spacing].");
	GMT_Usage (API, 3, "r: Regularized spline in tension (Mitasova & Mitas, 1993). Append <tension> and "
		"optionally append /<scale> for length-scale [Default is given output spacing].");
	GMT_Usage (API, 3, "p: Spherical surface spline (Parker, 1994); automatically sets -D4.");
	GMT_Usage (API, 3, "q: Spherical surface spline in tension (Wessel & Becker, 2008); automatically sets -D4. Append <tension>. "
		"Optionally, append +e<error> to change maximum error in series truncation [%g] and "
		"+n<n> to change the (odd) number of precalculated nodes for spline interpolation [%d].", SQ_TRUNC_ERROR, SQ_N_NODES);
	GMT_Usage (API, 1, "\n-T<maskgrid>");
	GMT_Usage (API, -2, "Mask grid file whose values are NaN or 0; its header implicitly sets -R, -I (and -r) and thus -T only applies to 2-D gridding.");
	GMT_Usage (API, 1, "\n-W[w]");
	GMT_Usage (API, -2, "Expect one extra input column with data errors sigma_i. "
		"Append w to indicate this column carries weights instead "
		"[Default makes weights via w_i = 1/sigma_i^2] for the least squares solution.");
	GMT_Usage (API, -2, "Note: weights only have an effect if -C is used.");
	GMT_Usage (API, 1, "\n-Z<mode>");
	GMT_Usage (API, -2, "Distance <mode> determines how we calculate distances between (x,y) points. "
		"Mode 0 applies to Cartesian 1-D spline interpolation:");
	GMT_Usage (API, 3, "0: x in user units, Cartesian distances.");
	GMT_Usage (API, -2, "Modes 1-3 apply to Cartesian 2-D surface spline interpolation:");
	GMT_Usage (API, 3, "1: x,y in user units, Cartesian distances.");
	GMT_Usage (API, 3, "2: x,y in degrees, flat Earth distances in meters.");
	GMT_Usage (API, 3, "3: x,y in degrees, spherical distances in meters.");
	GMT_Usage (API, -2, "Mode 4 applies to 2-D spherical surface spline interpolation:");
	GMT_Usage (API, 3, "4: x,y in degrees, use cosine of spherical distances in degrees.");
	GMT_Usage (API, -2, "Mode 5 applies to Cartesian 3-D volume interpolation:");
	GMT_Usage (API, 3, "5: x,y,z in user units, Cartesian distances.");
	GMT_Usage (API, -2, "Note: For modes 3-4, use PROJ_ELLIPSOID to select geodesic or great circle arcs.");
	GMT_Option (API, "V,bi");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Default is 2-5 input columns depending on dimensionality (see -Z) and weights (see -W).");
	GMT_Option (API, "d,e,f,g,h,i,o,q,r,s,w,x,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int greenspline_pre_parser (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {
	/* Help GMT_Parse know if -R is geographic based on -Z mode and return dimension */
	unsigned int dim = 0;
	struct GMT_OPTION *opt = NULL;
	for (opt = options; opt; opt = opt->next) {	/* Look for -Z only */
		if (opt->option != 'Z' || opt->arg[0] == '\0') continue;
		switch (opt->arg[0]) {
			case '0':	dim = 1;	break;
			case '1':	dim = 2;	break;
			case '2':	case '3':	case '4':
				dim = 2;
				gmt_set_geographic (GMT, GMT_IN);
				gmt_set_geographic (GMT, GMT_OUT);
				break;
			case '5':	dim = 3;	break;
			default:	dim = 0;	break;
		}
	}
	return (dim);
}

static int parse (struct GMT_CTRL *GMT, struct GREENSPLINE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to greenspline and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_items;
	unsigned int n_errors = 0, dimension, k, pos = 0;
	char txt[6][GMT_LEN64], p[GMT_BUFSIZ] = {""}, *c = NULL, *i = NULL, *r = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;

			case 'R':	/* Normally processed internally but must be handled separately since it can take 1,2,3 dimensions */
				GMT->common.R.active[RSET] = true;
				Ctrl->R3.dimension = 1;	/* At least */
				if (GMT->current.setting.run_mode == GMT_MODERN && Ctrl->dimension == 2) {	/* Watch for multi-item -R string created by gmt_init_module that would not have been parsed */
					/* This is needed because we are not using gmt_parse_R_option in greenspline */
					if ((r = strstr (opt->arg, "+G"))) {	/* Got grid registration implicitly via history */
						switch (r[2]) {
							case 'G':	GMT->common.R.registration = GMT_GRID_NODE_REG;		break;
							default:	GMT->common.R.registration = GMT_GRID_PIXEL_REG;	break;
						}
						r[0] = '\0';	/* Chop off this modifier */
						GMT->common.R.active[GSET] = true;
					}
					if ((i = strstr (opt->arg, "+I"))) {	/* Got grid increments implicitly via history */
						Ctrl->I.active = true;
						k = gmt_getincn (GMT, &i[2], Ctrl->I.inc, 2);
						i[0] = '\0';	/* Chop off this modifier */
						GMT->common.R.active[ISET] = true;
					}
				}
				if (opt->arg[0] == 'g' && opt->arg[1] == '\0') {	/* Got -Rg */
					Ctrl->R3.range[0] = 0.0;	Ctrl->R3.range[1] = 360.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					Ctrl->R3.dimension = 2;
					break;
				}
				if (opt->arg[0] == 'g' && opt->arg[1] == '/') {	/* Got -Rg/zmin/zmax */
					Ctrl->R3.range[0] = 0.0;	Ctrl->R3.range[1] = 360.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					n_items = sscanf (&opt->arg[2], "%[^/]/%s", txt[4], txt[5]);
					if (n_items != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Rg/z0/z1: Append the z-range\n");
						n_errors++;
					}
					Ctrl->R3.dimension = 3;
					break;
				}
				if (opt->arg[0] == 'd' && opt->arg[1] == '\0') {	/* Got -Rd */
					Ctrl->R3.range[0] = -180.0;	Ctrl->R3.range[1] = 180.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					Ctrl->R3.dimension = 2;
					break;
				}
				if (opt->arg[0] == 'd' && opt->arg[1] == '/') {	/* Got -Rd/zmin/zmax */
					Ctrl->R3.range[0] = -180.0;	Ctrl->R3.range[1] = 180.0;	Ctrl->R3.range[2] = -90.0;	Ctrl->R3.range[3] = 90.0;
					n_items = sscanf (&opt->arg[2], "%[^/]/%s", txt[4], txt[5]);
					if (n_items != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Rd/z0/z1: Append the z-range\n");
						n_errors++;
					}
					Ctrl->R3.dimension = 3;
					break;
				}
				if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file, presumably a grid */
					struct GMT_GRID *G = NULL;
					if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {	/* Get header only */
						return (API->error);
					}
					Ctrl->R3.range[0] = G->header->wesn[XLO]; Ctrl->R3.range[1] = G->header->wesn[XHI];
					Ctrl->R3.range[2] = G->header->wesn[YLO]; Ctrl->R3.range[3] = G->header->wesn[YHI];
					Ctrl->R3.inc[GMT_X] = G->header->inc[GMT_X];	Ctrl->R3.inc[GMT_Y] = G->header->inc[GMT_Y];
					Ctrl->R3.offset = G->header->registration;
					Ctrl->R3.dimension = 2;
					Ctrl->R3.mode = true;
					if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
						return (API->error);
					}
					break;
				}
				/* Only get here if the above cases did not trip */
				n_items = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s", txt[0], txt[1], txt[2], txt[3], txt[4], txt[5]);
				if (!(n_items == 2 || n_items == 4 || n_items == 6)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -R: Give 2, 4, or 6 coordinates\n");
					n_errors++;
				}
				n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf_arg (GMT, txt[0], gmt_M_type (GMT, GMT_IN, GMT_X), true, &Ctrl->R3.range[0]), txt[0]);
				n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf_arg (GMT, txt[1], gmt_M_type (GMT, GMT_IN, GMT_X), true, &Ctrl->R3.range[1]), txt[1]);
				if (n_items > 2) {
					Ctrl->R3.dimension = 2;
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt[2], gmt_M_type (GMT, GMT_IN, GMT_Y), true, &Ctrl->R3.range[2]), txt[2]);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt[3], gmt_M_type (GMT, GMT_IN, GMT_Y), true, &Ctrl->R3.range[3]), txt[3]);
				}
				if (n_items == 6) {
					Ctrl->R3.dimension = 3;
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, txt[4], gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->R3.range[4]), txt[4]);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, txt[5], gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->R3.range[5]), txt[5]);
				}
				if (Ctrl->R3.dimension > 1) gmt_M_memcpy (GMT->common.R.wesn, Ctrl->R3.range, 4, double);
				if (r) r[0] = '+';	/* Restore modifier */
				if (i) i[0] = '+';	/* Restore modifier */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Gradient data: -A<gradientfile>+f<format> */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				if (strchr (opt->arg, ',')) {	/* Old syntax: Specified a particular format with -A<mode>,<file> */
					if (gmt_M_compat_check (API->GMT, 5)) {
						GMT_Report (API, GMT_MSG_COMPAT, "Option -A<format>,<gradientfile> is deprecated; use -A<gradientfile>+f<format> instead\n");
						Ctrl->A.mode = (int)(opt->arg[0] - '0');
						Ctrl->A.file = strdup (&opt->arg[2]);
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Expect -A>gradientfile>+f<format>\n");
						n_errors++;
					}
					break;
				}
				/* New syntax */
				if ((c = strstr (opt->arg, "+f")) == NULL) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Expect -A>gradientfile>+f<format>\n");
						n_errors++;
				}
				else {
					Ctrl->A.mode = (int)(c[2] - '0');
					c[0] = '\0';	/* Temporarily chop off the modifier */
					if (opt->arg[0] == 0) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: No file given\n");
						n_errors++;
					}
					else
						Ctrl->A.file = strdup (opt->arg);
					c[0] = '+';	/* Restore the modifier */
				}
				break;
			case 'C':	/* Solve by SVD */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				if (strchr (opt->arg, '/') && strstr (opt->arg, "+f") == NULL) {	/* Old-style file deprecated specification */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* OK */
						sscanf (&opt->arg[k], "%lf/%s", &Ctrl->C.value, p);
						Ctrl->C.file = strdup (p);
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Expected -C[[n|r|v]<cut>[%]][+c][+f<file>][+i][+n]\n");
						n_errors++;
					}
					break;	/* No modifiers for the deprecated syntax */
				}
				switch (opt->arg[0]) {	/* First check directives (Default is no directive [0] which is the same as -Cr) */
					case 'n': Ctrl->C.mode = GMT_SVD_EIGEN_NUMBER_CUTOFF;   k = 1; break;
					case 'r': Ctrl->C.mode = GMT_SVD_EIGEN_RATIO_CUTOFF;    k = 1; break;
					case 'v': Ctrl->C.mode = GMT_SVD_EIGEN_VARIANCE_CUTOFF; k = 1; break;
					default:	/* No directive, probably part of a number of modifier, just ignore */
						k = 0; break;
				}
				if ((c = gmt_first_modifier (GMT, &opt->arg[k], "cifmMn"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'C', c, "cifmMn", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'c': Ctrl->C.history |= GMT_SVD_CUMULATIVE; break;
							case 'i': Ctrl->C.history |= GMT_SVD_INCREMENTAL; break;
							case 'f': Ctrl->C.file = strdup (&p[1]); break;
							case 'm': Ctrl->C.history = GMT_SVD_INCREMENTAL; break;
							case 'M': Ctrl->C.history = GMT_SVD_CUMULATIVE; break;
							case 'n': Ctrl->C.dryrun = true; break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}				
				if (opt->arg[k]) {	/* See if we got any value argument */
					if (strchr (opt->arg, '%')) {	/* Got percentages of largest eigenvalues */
						Ctrl->C.value = 0.01 * atof (&opt->arg[k]);
						if (Ctrl->C.mode == GMT_SVD_EIGEN_NUMBER_CUTOFF) Ctrl->C.mode = GMT_SVD_EIGEN_PERCENT_CUTOFF;	/* else it is GMT_SVD_EIGEN_VARIANCE_CUTOFF */
						if (Ctrl->C.value < 0.0 || Ctrl->C.value > 1.0) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -C: Percentages must be in 0-100%% range!\n");
							n_errors++;
						}
					}
					else {	/* Got regular cutoff value */
						Ctrl->C.value = atof (&opt->arg[k]);
						if (Ctrl->C.mode == GMT_SVD_EIGEN_NUMBER_CUTOFF && Ctrl->C.value >= 0.0 && Ctrl->C.value < 1.0) /* Old style fraction given instead */
							Ctrl->C.mode = GMT_SVD_EIGEN_PERCENT_CUTOFF;
					}
				}
				if (Ctrl->C.value < 0.0) Ctrl->C.dryrun = true, Ctrl->C.value = 0.0;	/* Check for deprecated syntax giving negative value */
				break;
			case 'D':
				if (gmt_M_compat_check (API->GMT, 6) && strlen (opt->arg) == 1) {	/* Old -D<mode> option supported for backwards compatibility (now -Z) */
					Ctrl->Z.active = true;
					Ctrl->Z.mode = atoi (opt->arg);	/* Since I added 0 to be 1-D later so now this is mode -1 */
				}
				else {	/* Give grid or cube information */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
					Ctrl->D.information = strdup (opt->arg);
				}
				break;
			case 'E':	/* Evaluate misfit -E[<file>]*/
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (opt->arg) {
					Ctrl->E.file = strdup (opt->arg);
					Ctrl->E.mode = 1;
				}
				break;
			case 'G':	/* Output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file));
				break;
			case 'I':	/* Table or grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				k = gmt_getincn (GMT, opt->arg, Ctrl->I.inc, 3);
				if (k < 1) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				if (Ctrl->I.inc[GMT_Y] == 0.0) Ctrl->I.inc[GMT_Y] = Ctrl->I.inc[GMT_X];
				if (Ctrl->I.inc[GMT_Z] == 0.0) Ctrl->I.inc[GMT_Z] = Ctrl->I.inc[GMT_X];
				break;
			case 'L':	/* Control desired combination of detrending and normalization */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.detrend = Ctrl->L.derange = false;	/* Turn both off since -L was set, then see if one or both where given explicitly */
				for (unsigned int k = 0; opt->arg[k]; k++) {
					switch (opt->arg[k]) {
						case 't':	/* Want linear detrending */
							Ctrl->L.detrend = true;	break;
						case 'r':	/* Want range normalization */
							Ctrl->L.derange = true;	break;
						default:	/* Unrecognized directive */
							GMT_Report (API, GMT_MSG_ERROR, "Option -L: Unrecognized directive %c\n", opt->arg[k]);
							n_errors++;
							break;
					}
				}
				break;
			case 'M':	/* Read or write list of Green's function forces [NOT IMPLEMENTED YET] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->M.file))) n_errors++;
				break;
			case 'N':	/* Output locations */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				if (opt->arg[0]) Ctrl->N.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->N.file))) n_errors++;
				break;
			case 'Q':	/* Directional derivative */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				if (strchr (opt->arg, '/')) {	/* Got 3-D vector components */
					n_items = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->Q.dir[0], &Ctrl->Q.dir[1], &Ctrl->Q.dir[2]);
					if (n_items != 3) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Append x/y/z components (3-D)\n");
						n_errors++;
					}
					gmt_normalize3v (GMT, Ctrl->Q.dir);	/* Normalize to unit vector */
				}
				else if (opt->arg[0]) {	/* 2-D azimuth */
					Ctrl->Q.az = atof(opt->arg);
					if (Ctrl->Q.az < -360.0 || Ctrl->Q.az > 360.0) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Azimuth %lg is outside valid range -180 t0 +360\n", Ctrl->Q.az);
						n_errors++;						
					}
				}
				break;
			case 'S':	/* Spline selection */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->S.arg);
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
						if (gmt_M_compat_check (GMT, 4)) {
							GMT_Report (API, GMT_MSG_COMPAT, "Option -SQ is deprecated; see -Sq syntax instead.\n");
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
							GMT_Report (API, GMT_MSG_ERROR, "Option -S: Append c|l|t|g|p|q\n");
							n_errors++;
						}
						break;
					case 'q':	/* Spherical minimum curvature spline in tension */
						Ctrl->S.mode = WESSEL_BECKER_2008;
						Ctrl->S.value[0] = atof (&opt->arg[1]);
						if (Ctrl->S.value[0] == 0.0)	/* Switch to Parker_1994 since tension is zero */
							Ctrl->S.mode = PARKER_1994;
						if ((c = strchr (opt->arg, '+')) != NULL) {
							while (gmt_strtok (c, "+", &pos, p)) {
								switch (p[0]) {
									case 'e':	Ctrl->S.value[2] = atof (&p[1]);	break;	/* Change the truncation error limit */
									case 'n':	Ctrl->S.value[3] = atof (&p[1]);	break;	/* Change the number of nodes for the spline lookup */
									case 'l':	Ctrl->S.rval[0]  = atof (&p[1]);	break;	/* Min value for spline, undocumented for testing only */
									case 'u':	Ctrl->S.rval[1]  = atof (&p[1]);	break;	/* Max value for spline, undocumented for testing only */
									default:
										GMT_Report (API, GMT_MSG_ERROR, "Option -Sq: Unknown modifier %s\n", p);
										n_errors++;
										break;
								}
							}
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Append c|l|t|g|p|q[<params>]\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Input mask grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->T.file)))
					n_errors++;
				else  {	/* Obtain -R -I -r from file */
					struct GMT_GRID *G = NULL;
					if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->T.file, NULL)) == NULL) {	/* Get header only */
						return (API->error);
					}
					Ctrl->R3.range[0] = G->header->wesn[XLO]; Ctrl->R3.range[1] = G->header->wesn[XHI];
					Ctrl->R3.range[2] = G->header->wesn[YLO]; Ctrl->R3.range[3] = G->header->wesn[YHI];
					Ctrl->R3.inc[GMT_X] = G->header->inc[GMT_X];	Ctrl->R3.inc[GMT_Y] = G->header->inc[GMT_Y];
					Ctrl->R3.offset = G->header->registration;
					Ctrl->R3.dimension = 2;
					Ctrl->R3.mode = true;
					if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
						return (API->error);
					}
					GMT->common.R.active[RSET] = true;
				}
				break;
			case 'W':	/* Expect data uncertainty or weights in last column */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if (opt->arg[0] == 'w') Ctrl->W.mode = GSP_GOT_WEIGHTS;	/* Got weights instead of sigmas */
				break;
			case 'Z':	/* Distance mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.mode = atoi (opt->arg);	/* Since I added 0 to be 1-D later so now this is mode -1 */
				break;

			/* Two undocumented test options for Green's function debug output */
			case '/':	/* Dump matrices */
				Ctrl->debug.active = true;
				break;
			case '+':	/* Turn on greenspline testing mode */
				Ctrl->debug.test = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->M.active) {	/* Determine if this is for reading or writing Green's function forces */
		if (Ctrl->S.active) /* Writing, since -S was given */
			Ctrl->M.mode = GMT_OUT;
		else if (gmt_access (GMT, Ctrl->M.file, F_OK)) {
			GMT_Report (API, GMT_MSG_ERROR, "-M option given but file %s not found\n", Ctrl->M.file);
			n_errors++;
		}
		else	/* Read in previous Green's function forces */
			Ctrl->M.mode = GMT_IN;
	}

	if (Ctrl->S.mode == WESSEL_BECKER_2008) {	/* Check that nodes is an odd integer */
		double fn = rint (Ctrl->S.value[3]);
		int64_t n = lrint (fn);
		if (!doubleAlmostEqual (Ctrl->S.value[3], fn) || ((n%2) == 0)) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -Sq option +n<N> modifier: <N> must be an odd integer\n");
			n_errors++;
		}
		if (Ctrl->S.value[2] < 0.0 || Ctrl->S.value[2] > 1.0e-4) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -Sq option +e<err> modifier: <err> must be positive and < 1.0e-4\n");
			n_errors++;
		}
	}
	if (Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) Ctrl->Z.mode = 4;	/* Automatically set */
	dimension = (Ctrl->Z.mode == 0) ? 1 : ((Ctrl->Z.mode == 5) ? 3 : 2);
	if (dimension == 2 && Ctrl->R3.mode) {	/* Set -R via a gridfile */
		/* Here, -R<grdfile> was used and we will use the settings supplied by the grid file (unless overridden) */
		if (!Ctrl->I.active) {	/* -I was not set separately; set indirectly */
			Ctrl->I.inc[GMT_X] = Ctrl->R3.inc[GMT_X];
			Ctrl->I.inc[GMT_Y] = Ctrl->R3.inc[GMT_Y];
			Ctrl->I.active = true;
		}
		/* Here, -r means toggle the grids registration */
		if (GMT->common.R.active[GSET]) {
			GMT->common.R.active[GSET] = !Ctrl->R3.offset;
			GMT->common.R.registration = !Ctrl->R3.offset;
		}
		else {
			GMT->common.R.active[GSET] = Ctrl->R3.offset;
			GMT->common.R.registration = Ctrl->R3.offset;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && gmt_access (GMT, Ctrl->A.file, R_OK), "Option -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->A.mode > 5, "Option -A: format must be in 0-5 range\n");
	n_errors += gmt_M_check_condition (GMT, !(GMT->common.R.active[RSET] || Ctrl->N.active || Ctrl->T.active), "No output locations specified (use either [-R -I], -N, or -T)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->R3.mode && dimension != 2, "The -R<gridfile> or -T<gridfile> option only applies to 2-D gridding\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.history && dimension != 2, "The -C +c+i modifiers only apply to 2-D gridding\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.history && strchr (Ctrl->G.file, '%') == NULL && strchr (Ctrl->G.file, '.') == NULL, "Option -G: When -C +i|c is used your grid file must have an extension\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.history && Ctrl->E.active && Ctrl->E.mode == 0, "Option -E: When -C +i|c is used you must supply a file via -E\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->debug.test && !Ctrl->N.active && Ctrl->R3.dimension != dimension, "The -R and -Z options disagree on the dimension\n");
	n_errors += gmt_check_binary_io (GMT, dimension + 1);
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && dimension == 1 && !gmt_M_is_dnan (Ctrl->Q.az), "Option -Q: No argument expected for 1-D derivative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && dimension > 1 && gmt_M_is_dnan (Ctrl->Q.az), "Option -Q: Must append azimuth (2-D) or x/y/z components (3-D)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.value[0] < 0.0 || Ctrl->S.value[0] >= 1.0, "Option -S: Tension must be in range 0 <= t < 1\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->S.mode == PARKER_1994 || Ctrl->S.mode == WESSEL_BECKER_2008) && Ctrl->Z.mode == 3, "Option -Sc|t|r: Cannot select -Z3\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == LINEAR_1D && Ctrl->Z.mode > 3, "Option -Sl: Cannot select -Z4 or higher\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || (dimension > 1 && Ctrl->I.inc[GMT_Y] <= 0.0) || (dimension == 3 && Ctrl->I.inc[GMT_Z] <= 0.0)), "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->C.dryrun && !Ctrl->C.file, "Option -C: Must specify file name for eigenvalues if +n is set");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Option -T: Must specify mask grid file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && dimension != 2, "Option -T: Only applies to 2-D gridding\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Option -N: Must specify node file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->N.file && gmt_access (GMT, Ctrl->N.file, R_OK), "Option -N: Cannot read file %s!\n", Ctrl->N.file);
	n_errors += gmt_M_check_condition (GMT, (Ctrl->I.active + GMT->common.R.active[RSET]) == 1 && dimension == 2, "Must specify -R, -I, [-r], -G for gridding\n");
	n_errors += gmt_M_check_condition (GMT, dimension == 3 && Ctrl->G.active && API->external && strchr (Ctrl->G.file, '%'), "Option -G: Cannot contain format-specifiers when not used on the command line\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Dump a table of x, G, dGdx for test purposes [requires option -+]  */
GMT_LOCAL void greenspline_dump_green (struct GMT_CTRL *GMT, double (*G) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *), double (*D) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *), double par[], double x0, double x1, int N, struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg) {
	int i;
	double x, dx, dy, y, t, ry, rdy;
	double min_y, max_y, min_dy, max_dy;

	min_y = min_dy = DBL_MAX;
	max_y = max_dy = -DBL_MAX;

	dx = (x1 - x0) / (N - 1);
	for (i = 0; i < N; i++) {
		x = x0 + i * dx;
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

/* Below are all the individual Green's functions.  Note that most of them take an argument
 * that is unused except in the spline lookup version of WB_08. */

/*----------------------  ONE DIMENSION ---------------------- */
/* Basic linear spline (bilinear in 2-D) */

GMT_LOCAL double greenspline_spline1d_linear (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* Dumb linear spline */
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	return (fabs (r));	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_grad_spline1d_linear (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* d/dr of r is 1 */
	gmt_M_unused(GMT); gmt_M_unused(r); gmt_M_unused(par); gmt_M_unused(unused);
	return (1.0);
}

/* greenspline_spline1d_sandwell computes the Green function for a 1-d spline
 * as per Sandwell [1987], G(r) = r^3.  All r must be >= 0.
 */

GMT_LOCAL double greenspline_spline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	if (r == 0.0) return (0.0);

	return (pow (fabs (r), 3.0));	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_grad_spline1d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	return (-3.0 * fabs (r) * r);	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_spline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* greenspline_spline1d_Wessel_Bercovici computes the Green function for a 1-d spline
	 * in tension as per Wessel and Bercovici [1988], G(u) = exp(-u) + u - 1,
	 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
	 * All r must be >= 0. par[0] = c
	 */
	double cx;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * fabs (r);
	return (exp (-cx) + cx - 1.0);
}

GMT_LOCAL double greenspline_grad_spline1d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double cx;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return ((1.0 - exp (-cx)) * par[2]);	/* Dividing by p, basically */
}

/*----------------------  TWO DIMENSIONS ---------------------- */

GMT_LOCAL double greenspline_spline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	if (r == 0.0) return (0.0);

	return (r * r * (log (r) - 1.0));	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_grad_spline2d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	if (r == 0.0) return (0.0);

	return (r * (2.0 * log (r) - 1.0));	/* Just regular spline; par not used */
}

/* greenspline_spline2d_Wessel_Bercovici computes the Green function for a 2-d spline
 * in tension as per Wessel and Bercovici [1988], G(u) = K(u) - log(u),
 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
 * K is the modified Bessel function K of order zero.
 * All r must be >= 0.
 * par[0] = c
 * par[1] = 2/c
 */

GMT_LOCAL double greenspline_spline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double y, z, cx, t, g;
	gmt_M_unused(GMT); gmt_M_unused(unused);

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

GMT_LOCAL double greenspline_grad_spline2d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double y, z, cx, t, dgdr;
	gmt_M_unused(GMT); gmt_M_unused(unused);

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

/* greenspline_spline2d_Mitasova_Mitas computes the regularized Green function for a 2-d spline
 * in tension as per Mitasova and Mitas [1993], G(u) = log (u) + Ei(u),
 * where u = par[1] * r^2. All r must be >= 0.
 * par[0] = phi (par[0] unused by function)
 * par[1] = phi^2/4
 */

GMT_LOCAL double greenspline_spline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double x, z, g, En, Ed;
	gmt_M_unused(GMT); gmt_M_unused(unused);

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

GMT_LOCAL double greenspline_grad_spline2d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double u, dgdr;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	u = par[1] * r * r;
	dgdr = 2.0 * (1.0 - exp (-u))/r;
	return (dgdr);
}

/*----------------------  TWO DIMENSIONS (SPHERE) ---------------------- */

/* greenspline_spline2d_Parker computes the Green function for a 2-d surface spline
 * as per Parker [1994], G(x) = dilog(),
 * where x is cosine of distances. All x must be -1 <= x <= +1.
 * Parameters passed are:
 * par[0] = 6/M_PI^2 (to normalize results)
 */

GMT_LOCAL double greenspline_spline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* Normalized to 0-1 */
	gmt_M_unused(unused);
	if (x == +1.0) return (1.0);
	if (x == -1.0) return (0.0);
	return (gmt_dilog (GMT, 0.5 - 0.5 * x) * par[0]);
}

GMT_LOCAL double greenspline_grad_spline2d_Parker (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* Normalized to 0-1 */
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	if (x == +1.0 || x == -1.0) return (0.0);
	return (log (0.5 - 0.5 * x) * sqrt ((1.0 - x) / (1.0 + x)));
}

GMT_LOCAL void greenspline_series_prepare (struct GMT_CTRL *GMT, double p, unsigned int L, struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg) {
	/* Precalculate Legendre series terms involving various powers/ratios of l and p */
	unsigned int l;
	double pp, t1, t2;
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Precalculate max %u terms for Legendre summation\n", L+1);
	Lz->A = gmt_M_memory (GMT, NULL, L+1, double);
	Lz->B = gmt_M_memory (GMT, NULL, L+1, double);
	Lz->C = gmt_M_memory (GMT, NULL, L+1, double);
	if (Lg) Lg->A = gmt_M_memory (GMT, NULL, L+1, double);
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

GMT_LOCAL unsigned int greenspline_get_max_L (struct GMT_CTRL *GMT, double p, double err) {
	/* Return max L needed given p and truncation err for Parker's simplified loop expression */
	gmt_M_unused(GMT);
	return ((unsigned int)lrint (p / sqrt (err)  + 10.0));
}

GMT_LOCAL void greenspline_free_lookup (struct GMT_CTRL *GMT, struct GREENSPLINE_LOOKUP **Lptr, unsigned int mode) {
	/* Free all items allocated under the lookup structures.
	 * mode = 0 means Lz and mode = 1 means Lg; the latter has no B & C arrays */
	struct GREENSPLINE_LOOKUP *L = *Lptr;
	if (L == NULL) return;	/* Nothing to free */
	gmt_M_free (GMT, L->y);
	gmt_M_free (GMT, L->c);
	gmt_M_free (GMT, L->A);
	if (mode == 0) {	/* Only Lz has A,B,C while Lg borrows B,C */
		gmt_M_free (GMT, L->B);
		gmt_M_free (GMT, L->C);
	}
	gmt_M_free (GMT, L);
	*Lptr = NULL;
}

GMT_LOCAL unsigned int greenspline_get_L (double x, double p, double err) {
	/* Determines the truncation order L_max given x, p, and err.
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

GMT_LOCAL double greenspline_spline2d_Wessel_Becker_Revised (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *Lz) {
	/* Evaluate g = M_PI * Pv(-x)/sin (v*x) - log (1-x) via series approximation */
	unsigned int L_max, l;
	double P0, P1, P2, S;
	gmt_M_unused(GMT);

	L_max = greenspline_get_L (x, par[0], par[1]);	/* Highest order needed in sum given x, p, and err */

	/* pp = par[0] * par[0]; */
	P0 = 1.0;	/* Initialize the P0 and P1 Legendre polynomials */
	P1 = x;
	S = par[2];	/* The constant l = 0 term was computed during setup */

	/* Sum the Legendre series */
	for (l = 1; l <= L_max; l++) {
		/* All the coeffs in l have been precomputed by greenspline_series_prepare.
		 * S += (2*l+1)*pp/(l*(l+1)*(l*(l+1)+pp)) * P1;
		 * P2 = x*(2*l+1)/(l+1) * P1 - l*P0/(l+1); */
		S += Lz->A[l] * P1;	/* Update sum */
		P2 = x * Lz->B[l] * P1 - Lz->C[l] * P0;	/* Get next Legendre polynomial value */
		P0 = P1;
		P1 = P2;
	}

	return (S);
}

GMT_LOCAL double greenspline_grad_spline2d_Wessel_Becker_Revised (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *Lg) {
	/* Evaluate g = -M_PI * (v+1)*[x*Pv(-x)+Pv+1(-x)]/(sin (v*x)*sin(theta)) - 1/(1-x) via series approximation */
	unsigned int L_max, l;
	double sin_theta, P0, P1, P2, S;
	gmt_M_unused(GMT);

	if (fabs(x) == 1.0) return 1.0;
	L_max = greenspline_get_L (x, par[0], par[1]);	/* Highest order needed in sum given x, p, and err */
	sin_theta = sqrt (1.0 - x * x);
	P0 = 1.0;	/* Initialize the P0 and P1 Legendre polynomials */
	P1 = x;
	S = 0.0;	/* Initialize sum */

	/* Sum the Legendre series */
	for (l = 1; l <= L_max; l++) {
		/* All the coeffs in l have been precomputed by greenspline_series_prepare.
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

GMT_LOCAL double greenspline_csplint (double *y, double *c, double b, double h2, uint64_t klo) {
	/* Special version of support_greenspline_csplint where x is equidistant with spacing squared h2
 	 * and b is the fractional distance relative to x[klo], so x itself is not needed here. */
	uint64_t khi;
	double a, yp;

	khi = klo + 1;
	a = 1.0 - b;	/* Fractional distance from next node */
	yp = a * y[klo] + b * y[khi] + ((a*a*a - a) * c[klo] + (b*b*b - b) * c[khi]) * h2 / 6.0;

	return (yp);
}

GMT_LOCAL double greenspline_spline2d_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L) {
	/* Given x, look up nearest node xx[k] <= x and do cubic spline interpolation */
	uint64_t k;
	double f, f0, df, y;
	gmt_M_unused(GMT);

	f = (x - par[10]) * par[9];	/* Floating point index */
	f0 = floor (f);
	df = f - f0;
	k = lrint (f0);
	if (df == 0.0) return (L->y[k]);	/* Right on a node */
	y = greenspline_csplint (L->y, L->c, df, par[4], k);	/* Call special cubic spline evaluator */
	return (y);
}

GMT_LOCAL double greenspline_spline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L) {
	return (greenspline_spline2d_lookup (GMT, x, par, L));
}

GMT_LOCAL double greenspline_grad_spline2d_Wessel_Becker_lookup (struct GMT_CTRL *GMT, double x, double par[], struct GREENSPLINE_LOOKUP *L) {
	return (greenspline_spline2d_lookup (GMT, x, par, L));
}

GMT_LOCAL void greenspline_spline2d_Wessel_Becker_splineinit (struct GMT_CTRL *GMT, double par[], double *x, struct GREENSPLINE_LOOKUP *L) {
	/* Set up cubic spline interpolation given the precomputed x,y values of the function */
	gmt_M_unused(GMT);
	gmt_M_unused(par);
	L->c = gmt_M_memory (GMT, NULL, 3*L->n, double);
	gmtlib_cspline (GMT, x, L->y, L->n, L->c);
}

GMT_LOCAL void greenspline_spline2d_Wessel_Becker_init (struct GMT_CTRL *GMT, double par[], struct GREENSPLINE_LOOKUP *Lz, struct GREENSPLINE_LOOKUP *Lg) {
	uint64_t i, n_columns;
	double *x = NULL;
#ifdef DUMP
	FILE *fp = NULL;
	uint64_t n_out;
	double out[3];
	fp = fopen ("greenspline.b", "wb");
	n_out = (Lg) ? 3 : 2;
#endif
	n_columns = lrint (par[7]);
	Lz->n = n_columns;
	x = gmt_M_memory (GMT, NULL, n_columns, double);
	Lz->y = gmt_M_memory (GMT, NULL, n_columns, double);
	if (Lg) {
		Lg->y = gmt_M_memory (GMT, NULL, n_columns, double);
		Lg->n = n_columns;
	}
	for (i = 0; i < n_columns; i++) {
		x[i] = par[10] + i * par[8];
		Lz->y[i] = greenspline_spline2d_Wessel_Becker_Revised (GMT, x[i], par, Lz);
		if (Lg) Lg->y[i] = greenspline_grad_spline2d_Wessel_Becker_Revised (GMT, x[i], par, Lg);
#ifdef DUMP
		out[0] = x[i];	out[1] = Lz->y[i];	if (Lg) out[2] = Lg->y[i];
		fwrite (out, sizeof (double), n_out, fp);
#endif
	}
#ifdef DUMP
	fclose (fp);
#endif
	greenspline_spline2d_Wessel_Becker_splineinit (GMT, par, x, Lz);
	if (Lg) {
		if (x[0] == -1.0) Lg->y[0] = 2.0*Lg->y[1] - Lg->y[2];	/* Linear interpolation from 2 nearest nodes */
		greenspline_spline2d_Wessel_Becker_splineinit (GMT, par, x, Lg);
	}
	gmt_M_free (GMT, x);	/* Done with x array */
}

/*----------------------  THREE DIMENSIONS ---------------------- */

/* greenspline_spline3d_sandwell computes the Green function for a 3-d spline
 * as per Sandwell [1987], G(r) = r.  All r must be >= 0.
 */

GMT_LOCAL double greenspline_spline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(par); gmt_M_unused(unused);
	if (r == 0.0) return (0.0);

	return (r);	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_grad_spline3d_sandwell (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	gmt_M_unused(GMT); gmt_M_unused(r); gmt_M_unused(par); gmt_M_unused(unused);
	return (1.0);	/* Just regular spline; par not used */
}

GMT_LOCAL double greenspline_spline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	/* greenspline_spline1d_Wessel_Bercovici computes the Green function for a 3-d spline
	 * in tension as per Wessel and Bercovici [1988], G(u) = [exp(-u) -1]/u,
	 * where u = par[0] * r and par[0] = sqrt (t/(1-t)).
	 * All r must be >= 0. par[0] = c
	 */
	double cx;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return (((exp (-cx) - 1.0) / cx) + 1.0);
}

GMT_LOCAL double greenspline_grad_spline3d_Wessel_Bercovici (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double cx;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	cx = par[0] * r;
	return ((1.0 - exp (-cx) * (cx + 1.0)) / (cx * r));
}

/* greenspline_spline3d_Mitasova_Mitas computes the regularized Green function for a 3-d spline
 * in tension as per Mitasova and Mitas [1993], G(u) = erf (u/2)/u - 1/sqrt(pi),
 * where u = par[0] * r. All r must be >= 0. par[0] = phi
 */

GMT_LOCAL double greenspline_spline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double x;
	gmt_M_unused(GMT); gmt_M_unused(unused);

	if (r == 0.0) return (0.0);

	x = par[0] * r;
	return ((erf (0.5 * x) / x) - M_INV_SQRT_PI);
}

GMT_LOCAL double greenspline_grad_spline3d_Mitasova_Mitas (struct GMT_CTRL *GMT, double r, double par[], struct GREENSPLINE_LOOKUP *unused) {
	double u, dgdr;
	gmt_M_unused(GMT); gmt_M_unused(unused);

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

GMT_LOCAL double greenspline_undo_normalization (double *X, double w_norm, unsigned int mode, double *coeff, unsigned int dim) {
	/* To undo normalizations, trends and offset of solution prediction */
	if (mode & GREENSPLINE_NORM) w_norm *= coeff[GSP_RANGE];	/* Scale back up by residual data range (if we normalized) */
	w_norm += coeff[GSP_MEAN_Z];					/* Add in mean data value plus minimum residual value (if we normalized by range) */
	if (mode & GREENSPLINE_TREND) {					/* Restore residual trend */
		w_norm += coeff[GSP_SLP_X] * (X[GMT_X] - coeff[GSP_MEAN_X]);
		if (dim == 2) w_norm += coeff[GSP_SLP_Y] * (X[GMT_Y] - coeff[GSP_MEAN_Y]);
	}
	return (w_norm);
}

GMT_LOCAL double greenspline_undo_normalization_grad (double *X, double w_norm, unsigned int mode, double *coeff, unsigned int dim) {
	/* To undo normalizations and trends for solution gradient. Here, -Q was set */
	gmt_M_unused (X);
	if (mode & GREENSPLINE_NORM) w_norm *= coeff[GSP_RANGE];	/* Scale back up by residual range (if we normalized) */
	if (mode & GREENSPLINE_TREND) {
		if (dim == 1)	/* Restore residual trend removed in greenspline_do_normalization_1d since no -Q direction, really */
			w_norm += coeff[GSP_SLP_X];
		else /* Restore component of plane trend in the -Q direction only */
			w_norm += coeff[GSP_SLP_Q];
	}
	return (w_norm);
}

GMT_LOCAL void greenspline_do_normalization_1d (struct GMTAPI_CTRL *API, double **X, double *obs, uint64_t n, uint64_t m, unsigned int mode, double *coeff) {
	/* We always remove/restore the mean observation value.  mode is a combination of bitflags that affects what we do:
	 * Bit GREENSPLINE_TREND will also remove linear trend and its projected gradient along observed gradient constraints
	 * Bit GREENSPLINE_NORM will normalize residuals by range and do the same for residual gradients
	 */

	uint64_t i;
	double d, min = DBL_MAX, max = -DBL_MAX;

	for (i = 0; i < n; i++) {	/* Find mean w-value */
		coeff[GSP_MEAN_Z] += obs[i];
		if ((mode & GREENSPLINE_TREND) == 0) continue;	/* No linear trend to model */
		coeff[GSP_MEAN_X] += X[i][GMT_X];
	}
	coeff[GSP_MEAN_Z] /= n;

	if (mode & GREENSPLINE_TREND) {	/* Solve for LS linear trend using deviations from (0, 0, 0) */
		double xx, zz, sxx, sxz;
		sxx = sxz = 0.0;
		coeff[GSP_MEAN_X] /= n;
		for (i = 0; i < n; i++) {
			xx = X[i][GMT_X] - coeff[GSP_MEAN_X];
			zz = obs[i] - coeff[GSP_MEAN_Z];
			sxx += (xx * xx);
			sxz += (xx * zz);
		}
		if (sxx != 0.0) coeff[GSP_SLP_X] = sxz / sxx;
	}

	/* Remove linear trend (or mean) */

	for (i = 0; i < n; i++) {	/* Get residuals and find range */
		obs[i] -= coeff[GSP_MEAN_Z];	/* Always remove the mean data value */
		if (mode & GREENSPLINE_TREND) obs[i] -= (coeff[GSP_SLP_X] * (X[i][GMT_X] - coeff[GSP_MEAN_X]));
		if (obs[i] < min) min = obs[i];
		if (obs[i] > max) max = obs[i];
	}
	if (m && mode & GREENSPLINE_TREND) {	/* remove trend slope from slope observations */
		GMT_Report (API, GMT_MSG_INFORMATION, "1-D Normalization correction %g for slope constraints\n", coeff[GSP_SLP_X]);
		for (i = n; i < (n+m); i++)
			obs[i] -= coeff[GSP_SLP_X];
	}

	if (mode & GREENSPLINE_NORM) {	/* Normalize by range */
		coeff[GSP_RANGE] = MAX (fabs(min), fabs(max));	/* Determine range */
		d = (coeff[GSP_RANGE] == 0.0) ? 1.0 : 1.0 / coeff[GSP_RANGE];
		GMT_Report (API, GMT_MSG_INFORMATION, "1-D Normalization factor %g for data and slope constraints\n", d);
		for (i = 0; i < (n+m); i++) obs[i] *= d;	/* Normalize 0-1 plus scale any slopes */
	}

	/* Recover obs(x) = w_norm(x) * coeff[GSP_RANGE] + coeff[GSP_MEAN_Z] + coeff[GSP_SLP_X]*(x-coeff[GSP_MEAN_X]) */
	GMT_Report (API, GMT_MSG_INFORMATION, "1-D Normalization coefficients: zoff = %g slope = %g xmean = %g range = %g\n", coeff[GSP_MEAN_Z], coeff[GSP_SLP_X], coeff[GSP_MEAN_X], coeff[GSP_RANGE]);
}

GMT_LOCAL void greenspline_do_normalization (struct GMTAPI_CTRL *API, double **X, double *obs, double **N, uint64_t n, uint64_t m, unsigned int mode, unsigned int dim, double *coeff) {
	/* We always remove/restore the mean observation value.  mode is a combination of bitflags that affects what we do:
	 * Bit GREENSPLINE_TREND will also remove linear trend
	 * Bit GREENSPLINE_NORM will normalize residuals by range
	 */

	uint64_t i;
	double d, min = DBL_MAX, max = -DBL_MAX;
	char *type[4] = {"Remove mean\n", "Normalization mode: Remove %d-D linear trend\n", "Remove mean and normalize data\n", "Normalization mode: Remove %d-D linear trend and normalize data\n"};
	if (mode == 0) return;	/* Do nothing under specific debug situation */
	gmt_M_memset (coeff, GSP_LENGTH, double);
	if (mode % 2)
		GMT_Report (API, GMT_MSG_INFORMATION, type[mode], dim);
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Normalization mode: %s\n", type[mode]);
	if (dim == 1) {	/* 1-D trend or mean only is done by separate function */
		greenspline_do_normalization_1d (API, X, obs, n, m, mode, coeff);
		return;
	}
	/* Here we deal with 2-D or 3-D data corrections */
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
		coeff[GSP_MEAN_X] /= n;	/* Mean x from sum computed above */
		coeff[GSP_MEAN_Y] /= n;	/* Mean y from sum computed above */
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

		d = sxx*syy - sxy*sxy;	/* Denominator */
		if (d != 0.0) {	/* Got a valid solution with nonzero d */
			coeff[GSP_SLP_X] = (sxz*syy - sxy*syz)/d;
			coeff[GSP_SLP_Y] = (sxx*syz - sxy*sxz)/d;
		}
	}

	/* Remove plane (or just mean) */

	for (i = 0; i < n; i++) {	/* Also find min/max of residuals in the process */
		obs[i] -= coeff[GSP_MEAN_Z];	/* We always remove mean data value */
		if (mode & GREENSPLINE_TREND) /* Also remove linear planar trend */
			obs[i] -= (coeff[GSP_SLP_X] * (X[i][GMT_X] - coeff[GSP_MEAN_X]) + coeff[GSP_SLP_Y] * (X[i][GMT_Y] - coeff[GSP_MEAN_Y]));
		if (obs[i] < min) min = obs[i];
		if (obs[i] > max) max = obs[i];
	}
	if (m && mode & GREENSPLINE_TREND) {	/* Also remove planar trend slope from slope observations */
		unsigned int ii;
		double *D = NULL, v, g[3] = {coeff[GSP_SLP_X], coeff[GSP_SLP_Y], 0.0};	/* The trend slope vector */
		GMT_Report (API, GMT_MSG_INFORMATION, "%d-D Normalization correction (%g, %g) for slope constraints in observed directions\n", dim, coeff[GSP_SLP_X], coeff[GSP_SLP_Y]);
		for (i = n; i < (n+m); i++) {	/* Each slope constraint has a magnitude (in obs[i]) and direction (unit vector in N[i-n]) */
			D = N[i-n];	/* Current slope unit vector */
			for (ii = 0, v = 0.0; ii < dim; ii++) v += g[ii] * D[ii];	/* Dot product of trend unit vector g with data unit vector is component in trend direction */
			obs[i] -= v;	/* Remove projected component of trend slope onto this observed gradient */
		}
	}
	if (mode & GREENSPLINE_NORM) {	/* Normalize by data range */
		coeff[GSP_RANGE] = MAX (fabs(min), fabs(max));	/* Determine range as max abs extreme value */
		d = (coeff[GSP_RANGE] == 0.0) ? 1.0 : 1.0 / coeff[GSP_RANGE];
		for (i = 0; i < (n+m); i++) obs[i] *= d;	/* Normalize to +/- 1 plus scale any slopes as they are affected the same way since depends on delta z */
	}

	/* Recover obs(x,y) = w_norm(x,y) * coeff[GSP_RANGE] + coeff[GSP_MEAN_Z] + coeff[GSP_SLP_X]*(x-coeff[GSP_MEAN_X]) + coeff[GSP_SLP_Y]*(y-coeff[GSP_MEAN_Y]) */
	GMT_Report (API, GMT_MSG_INFORMATION, "2-D Normalization coefficients: zoff = %g xslope = %g xmean = %g yslope = %g ymean = %g data range = %g\n",
		coeff[GSP_MEAN_Z], coeff[GSP_SLP_X], coeff[GSP_MEAN_X], coeff[GSP_SLP_Y], coeff[GSP_MEAN_Y], coeff[GSP_RANGE]);
}

GMT_LOCAL double greenspline_get_radius (struct GMT_CTRL *GMT, double *X0, double *X1, unsigned int dim) {
	double r = 0.0;
	/* Get distance between the two points */
	switch (dim) {
		case 1:	/* 1-D, just get signed x difference */
			r = X0[GMT_X] - X1[GMT_X];
			break;
		case 2:	/* 2-D Cartesian or spherical surface in meters */
			r = gmt_distance (GMT, X0[GMT_X], X0[GMT_Y], X1[GMT_X], X1[GMT_Y]);
			break;
		case 3:	/* 3-D Cartesian */
			r = hypot (X0[GMT_X] - X1[GMT_X], X0[GMT_Y] - X1[GMT_Y]);
			r = hypot (r, X0[GMT_Z] - X1[GMT_Z]);
			break;
	}
	return (r);
}

GMT_LOCAL double greenspline_get_dircosine (struct GMT_CTRL *GMT, double *D, double *X0, double *X1, unsigned int dim, bool baz) {
	/* D, the directional cosines of the observed gradient:
	 * X0: Observation point.
	 * X1: Prediction point.
	 * Compute N, the direction cosine of X1-X2, then C = D dot N.
	 */
	int ii;
	double az, C = 0.0, N[3];

	switch (dim) {
		case 1:	/* 1-D (no directional cosine, just sign) */
			C = (baz) ? 1.0 : -1.0;
			break;
		case 2:	/* 2-D */
			az = gmt_az_backaz (GMT, X0[GMT_X], X0[GMT_Y], X1[GMT_X], X1[GMT_Y], baz);
			sincosd (az, &N[GMT_X], &N[GMT_Y]);
			for (ii = 0; ii < 2; ii++) C += D[ii] * N[ii];	/* Dot product of 2-D unit vectors */
			C = -C;		/* The opposite direction for X0-X1 */
			break;
		case 3:	/* 3-D */
			for (ii = 0; ii < 3; ii++) N[ii] = X1[ii] - X0[ii];	/* Difference vector */
			gmt_normalize3v (GMT, N);	/* Normalize to unit vector */
			C = gmt_dot3v (GMT, D, N);	/* Dot product of 3-D unit vectors */
			if (baz) C = -C;		/* The opposite direction for X0-X1 */
			break;
	}
	return (C);
}

GMT_LOCAL void greenspline_dump_system (double *A, double *b, uint64_t nm, char *string) {
	/* Dump an A | b system to stderr for debugging */
	uint64_t row, col, ij;
	fprintf (stderr, "\n%s\n", string);
	for (row = 0; row < nm; row++) {
		ij = row * nm;
		fprintf (stderr, "%12.6f", A[ij++]);
		for (col = 1; col < nm; col++, ij++) fprintf (stderr, "\t%12.6f", A[ij]);
		fprintf (stderr, "\t||\t%12.6f\n", b[row]);
	}
}

GMT_LOCAL void greenspline_set_filename (char *name, unsigned int k, unsigned int width, unsigned int mode, char *file) {
	/* Turn name, eigenvalue number k, precision width and mode into a filename, e.g.,
	 * ("solution.grd", 33, 3, GMT_SVD_INCREMENTAL, file) will give solution_inc_033.grd */
	unsigned int s = (unsigned int)strlen (name) - 1;
	static char *type[3] = {"", "inc", "cum"};
	while (name[s] != '.') s--;	/* Wind backwards to start of extension */
	name[s] = '\0';	/* Temporarily chop off extension */
	sprintf (file, "%s_%s_%*.*d.%s", name, type[mode], width, width, k, &name[s+1]);
	name[s] = '.';	/* Restore original name */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_greenspline (void *V_API, int mode, void *args) {
	openmp_int col, row;
	uint64_t n_read, p, k, i, j, seg, m, n, nm, n_cr = 0, n_ok = 0, ij, ji, ii, n_duplicates = 0, n_skip = 0;
	unsigned int dimension = 0, normalize, n_cols, n_layers = 1, w_col, L_Max = 0;
	int64_t *kolumn = NULL;
	size_t n_alloc;
	int error = GMT_NOERROR, out_ID, way, n_columns, n_use;
	bool delete_grid = false, check_longitude, skip, write_3D_records = false;

	char *method[N_METHODS] = {"Minimum curvature Cartesian spline [1-D]",
		"Minimum curvature Cartesian spline [2-D]",
		"Minimum curvature Cartesian spline [3-D]",
		"Continuous curvature Cartesian spline in tension [1-D]",
		"Continuous curvature Cartesian spline in tension [2-D]",
		"Continuous curvature Cartesian spline in tension [3-D]",
		"Regularized Cartesian spline in tension [2-D]",
		"Regularized Cartesian spline in tension [3-D]",
		"Minimum curvature spherical spline",
		"Continuous curvature spherical spline in tension",
		"Linear Cartesian spline [1-D]",
		"Bilinear Cartesian spline [2-D]"};

	gmt_grdfloat *data = NULL;

	double *v = NULL, *s = NULL, *b = NULL, *ssave = NULL;
	double *obs = NULL, **D = NULL, **X = NULL, *alpha = NULL, *in = NULL, *orig_obs = NULL;
	double mem, part, C, p_val, r, par[N_PARAMS], norm[GSP_LENGTH], az = 0, grad;
	double *A = NULL, *A_orig = NULL, r_min, r_max, err_sum = 0.0, var_sum = 0.0;
	double x0 = 0.0, x1 = 5.0;

	FILE *fp = NULL;

	double (*G) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *) = NULL;		/* Pointer to chosen Green's function */
	double (*dGdr) (struct GMT_CTRL *, double, double *, struct GREENSPLINE_LOOKUP *) = NULL;	/* Pointer to chosen gradient of Green's function */
	double (*finalize) (double *, double, unsigned int, double *, unsigned int);	/* Which undo norm function */

	struct GMT_GRID *Grid = NULL, *Out = NULL;
	struct GMT_GRID_HEADER *header = NULL;
	struct GMT_CUBE *Cube =  NULL;     /* Structure to hold output cube if 3-D interpolation */

	struct GREENSPLINE_LOOKUP *Lz = NULL, *Lg = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *Nin = NULL;
	struct GMT_GRID_INFO info;
	struct GMT_RECORD *In = NULL, *Rec = NULL;
	struct GREENSPLINE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	dimension = greenspline_pre_parser (GMT, options);	/* Check -Z and possibly change default to geographic data mode before -R is parsed */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	Ctrl->dimension = dimension;
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the greenspline main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	dimension = (Ctrl->Z.mode == 0) ? 1 : ((Ctrl->Z.mode == 5) ? 3 : 2);
	gmt_M_memset (par,   N_PARAMS, double);
	gmt_M_memset (norm,  GSP_LENGTH, double);
	gmt_M_memset (&info, 1, struct GMT_GRID_INFO);

	write_3D_records = (dimension == 3 && !Ctrl->G.active);	/* Just so it is only true if 3-D and no output filename was given */

	/* As many S.mode reflects 1-D after parse, here we increment S.mode if dimension is 2 or 3 */
	if (Ctrl->S.mode == SANDWELL_1987_1D || Ctrl->S.mode == WESSEL_BERCOVICI_1998_1D) Ctrl->S.mode += (dimension - 1);
	if (Ctrl->S.mode == LINEAR_1D) Ctrl->S.mode += (dimension - 1);
	if (Ctrl->S.mode == MITASOVA_MITAS_1993_2D) Ctrl->S.mode += (dimension - 2);

	way = gmt_M_is_spherical (GMT) ? GMT_GREATCIRCLE : GMT_GEODESIC;
	normalize = GREENSPLINE_MEAN;	/* Remove and restore of mean data value is always done */
	Ctrl->Z.mode--;	/* Since I added 0 to be 1-D later so now it is -1 */
	switch (Ctrl->Z.mode) {	/* Set pointers to 2-D distance functions */
		case -1:	/* Cartesian 1-D x data */
			if (Ctrl->L.detrend) normalize += GREENSPLINE_TREND;
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		case 0:	/* Cartesian 2-D x,y data */
			error = gmt_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);
			if (Ctrl->L.detrend) normalize += GREENSPLINE_TREND;
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		case 1:	/* 2-D lon, lat data, but scale to Cartesian flat Earth km */
			gmt_set_geographic (GMT, GMT_IN);
			gmt_set_geographic (GMT, GMT_OUT);
			error = gmt_init_distaz (GMT, 'k', GMT_FLATEARTH, GMT_MAP_DIST);
			if (Ctrl->L.detrend) normalize += GREENSPLINE_TREND;
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		case 2:	/* 2-D lon, lat data, use spherical distances in km (geodesic if PROJ_ELLIPSOID is nor sphere) */
			gmt_set_geographic (GMT, GMT_IN);
			gmt_set_geographic (GMT, GMT_OUT);
			error = gmt_init_distaz (GMT, 'k', way, GMT_MAP_DIST);
			if (Ctrl->L.active && Ctrl->L.detrend)	/* Spherical area so no Cartesian plane removal possible */
				GMT_Report (API, GMT_MSG_WARNING, "Cannot select -Lt for spherical surface gridding. No trend will be removed\n");
			Ctrl->L.detrend = false;	/* Spherical area so no Cartesian plane removal possible */
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		case 3:	/* 2-D lon, lat data, and Green's function needs cosine of spherical or geodesic distance */
			gmt_set_geographic (GMT, GMT_IN);
			gmt_set_geographic (GMT, GMT_OUT);
			error = gmt_init_distaz (GMT, 'S', way, GMT_MAP_DIST);
			if (Ctrl->L.active && Ctrl->L.detrend)	/* Spherical area so no Cartesian plane removal possible */
				GMT_Report (API, GMT_MSG_WARNING, "Cannot select -Lt for spherical surface gridding. No trend will be removed\n");
			Ctrl->L.detrend = false;	/* Spherical area so no Cartesian plane removal possible */
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		case 4:	/* 3-D Cartesian x,y,z data handled separately */
			if (Ctrl->L.active && Ctrl->L.detrend)	/* 3-D so no Cartesian plane removal possible */
				GMT_Report (API, GMT_MSG_WARNING, "Cannot select -Lt for 3-D gridding. No trend will be removed\n");
			Ctrl->L.detrend = false;	/* 3-D so no Cartesian plane removal possible [Maybe remove hyperplane in the future?] */
			if (Ctrl->L.derange) normalize += GREENSPLINE_NORM;
			break;
		default:	/* Cannot happen unless we make a bug */
			GMT_Report (API, GMT_MSG_ERROR, "BUG since D (=%d) cannot be outside 0-5 range\n", Ctrl->Z.mode+1);
			break;
	}
	if (error == GMT_NOT_A_VALID_TYPE) Return (error);
	
	GMT_Report (API, GMT_MSG_INFORMATION, "Mean data value will be removed and restored\n");
	if (Ctrl->L.detrend) {
		if (Ctrl->A.active)
			GMT_Report (API, GMT_MSG_INFORMATION, "Least-squares data trend will be removed and restored from data residuals and slopes\n");
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "Least-squares data trend will be removed and restored from data residuals\n");
	}
	if (Ctrl->L.derange) {
		if (Ctrl->A.active)
			GMT_Report (API, GMT_MSG_INFORMATION, "Data and slope residuals will be normalized by max (abs(wmin),abs(wmax))\n");
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "Data residuals will be normalized by max (abs(wmin),abs(wmax))\n");
	}

	if (Ctrl->Q.active && dimension == 2) sincosd (Ctrl->Q.az, &Ctrl->Q.dir[GMT_X], &Ctrl->Q.dir[GMT_Y]);

	/* Now we are ready to take on some input values */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_cols = (Ctrl->W.active) ? dimension + 1 : dimension;	/* So X[k][dimension] holds the weight if -W is active */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_cols+1, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
		Return (error);
	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	X = gmt_M_memory (GMT, NULL, n_alloc, double *);
	obs = gmt_M_memory (GMT, NULL, n_alloc, double);
	check_longitude = (dimension == 2 && (Ctrl->Z.mode == 1 || Ctrl->Z.mode == 2));
	w_col = dimension + 1;	/* Where weights would be in input, if given */

	GMT_Report (API, GMT_MSG_INFORMATION, "Read input data and check for data constraint duplicates\n");
	n = m = n_read = 0;
	r_min = DBL_MAX;	r_max = -DBL_MAX;
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				for (p = 0; p < n; p++) gmt_M_free (GMT, X[p]);
				gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;							/* Go back and read the next record */
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			for (p = 0; p < n; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (API->error);
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		if (check_longitude) {
			/* Ensure geographic longitudes fit the range since the normalization function expects it */
			if (in[GMT_X] < Ctrl->R3.range[XLO] && (in[GMT_X] + 360.0) < Ctrl->R3.range[XHI]) in[GMT_X] += 360.0;
			else if (in[GMT_X] > Ctrl->R3.range[XHI] && (in[GMT_X] - 360.0) > Ctrl->R3.range[XLO]) in[GMT_X] -= 360.0;
		}

		if (X[n] == NULL) X[n] = gmt_M_memory (GMT, NULL, n_cols, double);	/* Allocate space for this constraint */
		for (k = 0; k < dimension; k++) X[n][k] = in[k];	/* Get coordinates + optional weights (if -W) */
		/* Check for duplicates */
		skip = false;
		for (i = 0; !skip && i < n; i++) {
			r = greenspline_get_radius (GMT, X[i], X[n], dimension);
			if (gmt_M_is_zero (r)) {	/* Duplicates will give zero point separation */
				if (doubleAlmostEqualZero (in[dimension], obs[i])) {
					GMT_Report (API, GMT_MSG_WARNING,
					            "Data constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i);
					skip = true;
					n_skip++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR,
					            "Data constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ"
					            " in observation (%.12g vs %.12g)\n", n_read, i, in[dimension], obs[i]);
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

		if (Ctrl->W.active) {	/* Planning a weighted solution */
			if (Ctrl->W.mode == GSP_GOT_SIG) {	/* Got sigma, must convert to weight */
				err_sum += in[w_col] * in[w_col];	/* Sum up variance first */
				X[n][dimension] = 1.0 / in[w_col];	/* We will square later */
			}
			else	/* Got weight, use as is, no squaring later */
				X[n][dimension] = in[w_col];
		}
		var_sum += in[dimension] * in[dimension];	/* Sum up data variance */
		obs[n++] = in[dimension];

		if (n == n_alloc) {	/* Get more memory */
			n_alloc <<= 1;
			X = gmt_M_memory (GMT, X, n_alloc, double *);
			obs = gmt_M_memory (GMT, obs, n_alloc, double);
		}
	} while (true);

	if (n_skip && n < n_alloc && X[n])	/* If we end with a skip then we have allocated one too many */
		gmt_M_free (GMT, X[n]);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		for (p = 0; p < n; p++) gmt_M_free (GMT, X[p]);
		gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
		Return (API->error);
	}

	X = gmt_M_memory (GMT, X, n, double *);
	obs = gmt_M_memory (GMT, obs, n, double);
	nm = n;
	GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " unique data constraints\n", n);
	if (n_skip) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " data constraints as duplicates\n", n_skip);

	if (Ctrl->W.active && Ctrl->W.mode == GSP_GOT_SIG) {	/* Got data uncertainties */
		err_sum = sqrt (err_sum / nm);	/* Mean data uncertainty */
		GMT_Report (API, GMT_MSG_INFORMATION, "Mean data uncertainty is %g\n", err_sum);
	}

	if (Ctrl->A.active) {	/* Read gradient constraints from file */
		unsigned int n_A_cols = 0;
		struct GMT_DATASET *Din = NULL;
		struct GMT_DATASEGMENT *Slp = NULL;
		switch (dimension) {
			case 1:	/* 1-D */
				switch (Ctrl->A.mode) {
					case 0:	n_A_cols = 2; break;/* x, slope */
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Bad gradient mode selected for 1-D data (%d) - aborting!\n", Ctrl->A.mode);
						gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
				break;
			case 2:	/* 2-D */
				switch (Ctrl->A.mode) {
					case 1:	n_A_cols = 4; break; /* (x, y, az, gradient) */
					case 2:	n_A_cols = 4; break; /* (x, y, gradient, azimuth) */
					case 3:	n_A_cols = 4; break; /* (x, y, direction, gradient) */
					case 4:	n_A_cols = 4; break; /* (x, y, gx, gy) */
					case 5:	n_A_cols = 5; break; /* (x, y, nx, ny, gradient) */
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Bad gradient mode selected for 2-D data (%d) - aborting!\n", Ctrl->A.mode);
						gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
				break;
			case 3:	/* 3-D */
				switch (Ctrl->A.mode) {
					case 4:	n_A_cols = 6; break; /* (x, y, z, gx, gy, gz) */
					case 5: n_A_cols = 7; break; /* (x, y, z, nx, ny, nz, gradient) */
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Bad gradient mode selected for 3-D data (%d) - aborting!\n", Ctrl->A.mode);
						gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
				break;
		}
		/* Update the expected number of input columns */
		if ((error = GMT_Set_Columns (API, GMT_IN, n_A_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to set input data columns to %u\n", n_A_cols);
			Return (error);
		}
		if (GMT->common.b.active[GMT_IN]) GMT->common.b.ncol[GMT_IN]++;	/* Must assume it is just one extra column */
		gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from -A file */
		if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->A.file, NULL)) == NULL) {
			for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (API->error);
		}
		if (Din->n_columns < n_A_cols) {
			GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least %u are needed\n", (int)Din->n_columns, n_A_cols);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i -o were used at all) */
		m = Din->n_records;	/* Total number of gradient constraints */
		nm += m;		/* New total of linear equations to solve */
		X = gmt_M_memory (GMT, X, nm, double *);
		for (k = n; k < nm; k++) X[k] = gmt_M_memory (GMT, NULL, n_cols, double);
		obs = gmt_M_memory (GMT, obs, nm, double);
		D = gmt_M_memory (GMT, NULL, m, double *);
		kolumn = gmt_M_memory (GMT, NULL, m, int *);	/* Hold Greens' function ID for a data co-registered slope constraint */
		for (k = 0; k < m; k++) D[k] = gmt_M_memory (GMT, NULL, n_cols, double);
		for (k = 0; k < m; k++) kolumn[k] = GMT_NOTSET;	/* Flag'em as not co-registered */
		n_skip = n_read = 0;
		for (seg = k = 0, p = n; seg < Din->n_segments; seg++) {
			Slp = Din->table[0]->segment[seg];
			for (row = 0; row < (openmp_int)Slp->n_rows; row++, k++, p++) {
				for (ii = 0; ii < n_cols; ii++) X[p][ii] = Slp->data[ii][row];
				switch (dimension) {
					case 1:	/* 1-D: x, slope */
						D[k][0] = 1.0;	/* Dummy since there is no direction for 1-D spline (the gradient is in the x-y plane) */
						obs[p] = Slp->data[dimension][row];
						break;
					case 2:	/* 2-D */
						switch (Ctrl->A.mode) {
							case 1:	/* (x, y, az, gradient) */
								az = D2R * Slp->data[2][row];
								obs[p] = Slp->data[3][row];
								break;
							case 2:	/* (x, y, gradient, azimuth) */
								az = D2R * Slp->data[3][row];
								obs[p] = Slp->data[2][row];
								break;
							case 3:	/* (x, y, direction, gradient) */
								az = M_PI_2 - D2R * Slp->data[2][row];
								obs[p] = Slp->data[3][row];
								break;
							case 4:	/* (x, y, gx, gy) */
								az = atan2 (Slp->data[2][row], Slp->data[3][row]);		/* Get azimuth of gradient */
								obs[p] = hypot (Slp->data[3][row], Slp->data[3][row]);	/* Get magnitude of gradient */
								break;
							case 5:	/* (x, y, nx, ny, gradient) */
								az = atan2 (Slp->data[2][row], Slp->data[3][row]);		/* Get azimuth of gradient */
								obs[p] = Slp->data[4][row];	/* Magnitude of gradient */
								break;
						}
						sincos (az, &D[k][GMT_X], &D[k][GMT_Y]);
						break;
					case 3:	/* 3-D */
						switch (Ctrl->A.mode) {
							case 4:	/* (x, y, z, gx, gy, gz) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = Slp->data[3+ii][row];	/* Get the gradient vector */
								obs[p] = gmt_mag3v (GMT, D[k]);	/* This is the gradient magnitude */
								gmt_normalize3v (GMT, D[k]);		/* These are the direction cosines of the gradient */
								break;
							case 5: /* (x, y, z, nx, ny, nz, gradient) */
								for (ii = 0; ii < 3; ii++) D[k][ii] = Slp->data[3+ii][row];	/* Get the unit vector */
								obs[p] = Slp->data[6][row];	/* This is the gradient magnitude */
								break;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Bad dimension selected (%d) - aborting!\n", dimension);
						for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
						gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
						Return (GMT_DATA_READ_ERROR);
						break;
				}
				/* Check for duplicates as well as co-registered slopes with data constraints */
				skip = false;
				for (i = 0; !skip && i < p; i++) {
					r = greenspline_get_radius (GMT, X[i], X[p], dimension);
					if (gmt_M_is_zero (r)) {	/* Duplicates will give zero point separation */
						if (i < n) {	/* Co-registered slope and data constraints */
							GMT_Report (API, GMT_MSG_WARNING, "Slope constraint %" PRIu64 " is co-registered with data constraint %" PRIu64 "\n", n_read, i);
							kolumn[k] = i;	/* Reuse this Green's function instead of a new one for this constraint */
							n_cr++;		/* Number of co-registered constraints found */
						}
						else if (doubleAlmostEqualZero (in[dimension], obs[i])) {
							GMT_Report (API, GMT_MSG_WARNING, "Slope constraint %" PRIu64 " is identical to %" PRIu64
							            " and will be skipped\n", n_read, i-n);
							skip = true;
							n_skip++;
						}
						else {
							GMT_Report (API, GMT_MSG_ERROR, "Slope constraint %" PRIu64 " and %" PRIu64
							            " occupy the same location but differ in observation (%.12g vs %.12g)\n", n_read, i-n, obs[p], obs[i]);
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
		if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " unique slope constraints\n", m);
		if (n_skip) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " slope constraints as duplicates\n", n_skip);
	}

	/* Check for duplicates which would result in a singular matrix system; also update min/max radius */

	GMT_Report (API, GMT_MSG_INFORMATION, "Distance between the closest constraints:  %.12g]\n", r_min);
	GMT_Report (API, GMT_MSG_INFORMATION, "Distance between most distant constraints: %.12g]\n", r_max);

	if (n_duplicates || n_cr) {	/* These differ in observation value so need to be averaged, medianed, or whatever first */
		if (!Ctrl->C.active || gmt_M_is_zero (Ctrl->C.value)) {
			if (n_cr) {
				GMT_Report (API, GMT_MSG_ERROR,
			            "Found %" PRIu64 " coregistered data and slope constraints - that scenario is not yet implemented\n", n_cr);
			}
			else {
				GMT_Report (API, GMT_MSG_ERROR,
			            "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
				GMT_Report (API, GMT_MSG_ERROR,
			            "You must reconcile duplicates before running greenspline since they will result in a singular matrix\n");
			}
			for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			if (m) {
				for (p = 0; p < m; p++) gmt_M_free (GMT, D[p]);
				gmt_M_free (GMT, D);
				gmt_M_free (GMT, kolumn);
			}
			Return (GMT_DATA_READ_ERROR);
		}
		else {
			GMT_Report (API, GMT_MSG_WARNING,
			            "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
			GMT_Report (API, GMT_MSG_WARNING, "Expect some eigenvalues to be identically zero\n");
		}
	}

	if (m == 0)
		GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " data points, yielding a %" PRIu64 " by %" PRIu64 " set of linear equations\n",
		            n, nm, nm);
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " data points and %" PRIu64 " gradients, yielding a %" PRIu64 " by %"
		            PRIu64 " set of linear equations\n", n, m, nm, nm);

	if (Ctrl->T.file) {	/* Existing grid that will have zeros and NaNs, only */
		if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->T.file, NULL)) == NULL) {	/* Get header only */
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (API->error);
		}
		if (!(Grid->header->wesn[XLO] == Ctrl->R3.range[0] && Grid->header->wesn[XHI] == Ctrl->R3.range[1] &&
		      Grid->header->wesn[YLO] == Ctrl->R3.range[2] && Grid->header->wesn[YHI] == Ctrl->R3.range[3])) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid does not match your specified region\n");
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (GMT_RUNTIME_ERROR);
		}
		if (! (Grid->header->inc[GMT_X] == Ctrl->I.inc[GMT_X] && Grid->header->inc[GMT_Y] == Ctrl->I.inc[GMT_Y])) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid resolution does not match your specified grid spacing\n");
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (GMT_RUNTIME_ERROR);
		}
		if (! (Grid->header->registration == GMT->common.R.registration)) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid registration does not match your specified grid registration\n");
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->T.file, Grid) == NULL) {	/* Get data */
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (API->error);
		}
		(void)gmt_set_outgrid (GMT, Ctrl->T.file, false, 0, Grid, &Out);	/* true if input is a read-only array; otherwise Out is just a pointer to Grid */
		n_ok = Grid->header->nm;
		gmt_M_grd_loop (GMT, Grid, row, col, ij) if (gmt_M_is_fnan (Grid->data[ij])) n_ok--;
	}
	else if (Ctrl->N.active) {	/* Read output locations from file */
		gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from -N file */
		if ((Nin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (API->error);
		}
		if (Nin->n_columns < dimension) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least %u are needed\n",
			            Ctrl->N.file, (int)Nin->n_columns, dimension);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, obs);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i -o were used at all) */
		T = Nin->table[0];
	}
	else {	/* Fill in an equidistant output table, grid, or cube */
		if (dimension == 1) {	/* Dummy grid to hold the 1-D info */
			if ((Grid = gmt_create_grid (GMT)) == NULL) Return (API->error);
			delete_grid = true;
			Grid->header->wesn[XLO] = Ctrl->R3.range[XLO];	Grid->header->wesn[XHI] = Ctrl->R3.range[XHI];
			Grid->header->registration = GMT->common.R.registration;
			Grid->header->inc[GMT_X] = Ctrl->I.inc[GMT_X];
			Grid->header->n_rows = 1;	/* So that output logic will work for 1-D which only has columns */
			n_ok = Grid->header->n_columns = gmt_M_grd_get_nx (GMT, Grid->header);
			header = Grid->header;
			data = gmt_M_memory (GMT, NULL, Grid->header->n_columns, gmt_grdfloat);
		}
		else if (dimension == 2) {	/* Need a full-fledged Grid creation since we are writing it to who knows where */
			if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->R3.range, Ctrl->I.inc, \
				GMT->common.R.registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			n_ok = Grid->header->nm;
			header = Grid->header;
			data = Grid->data;	/* Pointer to the float 2-D grid */
			if (Ctrl->D.active && gmt_decode_grd_h_info (GMT, Ctrl->D.information, Grid->header)) {
				Return (GMT_PARSE_ERROR);
			}
		}
		else {	/* 3-D cube needed */
			if ((Cube = GMT_Create_Data (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, Ctrl->R3.range, Ctrl->I.inc, \
				GMT->common.R.registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			n_layers = Cube->header->n_bands;
			n_ok = Cube->header->nm * n_layers;
			header = Cube->header;
			data = Cube->data;	/* Pointer to the float 3-D cube */
			if (Ctrl->D.active && gmt_decode_cube_h_info (GMT, Ctrl->D.information, Cube)) {
				Return (GMT_PARSE_ERROR);
			}
		}
		Out = Grid;	/* Just pointer since we created Grid above (except for cube) */
	}

	switch (Ctrl->S.mode) {	/* Assign pointers to Green's functions and the gradient and set up required parameters */
		case LINEAR_1D:
		case LINEAR_2D:
			G = &greenspline_spline1d_linear;
			dGdr = &greenspline_grad_spline1d_linear;
			break;
		case SANDWELL_1987_1D:
			G = &greenspline_spline1d_sandwell;
			dGdr = &greenspline_grad_spline1d_sandwell;
			break;
		case SANDWELL_1987_2D:
			G = &greenspline_spline2d_sandwell;
			dGdr = &greenspline_grad_spline2d_sandwell;
			break;
		case SANDWELL_1987_3D:
			G = &greenspline_spline3d_sandwell;
			dGdr = &greenspline_grad_spline3d_sandwell;
			break;
		case WESSEL_BERCOVICI_1998_1D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0) Ctrl->S.value[1] = Grid->header->inc[GMT_X];
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			par[2] = 1.0 / par[0];	/* Used in grad function */
			G = &greenspline_spline1d_Wessel_Bercovici;
			dGdr = &greenspline_grad_spline1d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_2D:
			if (Ctrl->S.value[1] == 0.0 && Grid->header->inc[GMT_X] > 0.0)
				Ctrl->S.value[1] = 0.5 * (Grid->header->inc[GMT_X] + Grid->header->inc[GMT_Y]);
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = &greenspline_spline2d_Wessel_Bercovici;
			dGdr = &greenspline_grad_spline2d_Wessel_Bercovici;
			break;
		case WESSEL_BERCOVICI_1998_3D:
			if (Ctrl->S.value[1] == 0.0 && Cube->header->inc[GMT_X] > 0.0)
				Ctrl->S.value[1] = (Cube->header->inc[GMT_X] + Cube->header->inc[GMT_Y] + Cube->z_inc) / 3.0;
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			par[1] = 2.0 / par[0];
			G = &greenspline_spline3d_Wessel_Bercovici;
			dGdr = &greenspline_grad_spline3d_Wessel_Bercovici;
			break;
		case MITASOVA_MITAS_1993_2D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_Report (API, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = &greenspline_spline2d_Mitasova_Mitas;
			dGdr = &greenspline_grad_spline2d_Mitasova_Mitas;
			break;
		case MITASOVA_MITAS_1993_3D:
			/* par[0] = Ctrl->S.value[0]; */
			if (Ctrl->S.value[1] == 0.0) Ctrl->S.value[1] = 1.0;
			p_val = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0])) / Ctrl->S.value[1];
			GMT_Report (API, GMT_MSG_DEBUG, "p_val = %g\n", p_val);
			par[0] = p_val;
			par[1] = 0.25 * par[0] * par[0];
			G = &greenspline_spline3d_Mitasova_Mitas;
			dGdr = &greenspline_grad_spline3d_Mitasova_Mitas;
			break;
		case PARKER_1994:
			par[0] = 6.0 / (M_PI*M_PI);
			G = &greenspline_spline2d_Parker;
			dGdr = &greenspline_grad_spline2d_Parker;
			if (Ctrl->debug.test) x0 = -1.0, x1 = 1.0;
			break;
		case WESSEL_BECKER_2008:
			par[0] = sqrt (Ctrl->S.value[0] / (1.0 - Ctrl->S.value[0]));	/* The p value */
			par[1] = Ctrl->S.value[2];	/* The truncation error */
			par[2] = -log (2.0) + (par[0]*par[0] - 1.0) / (par[0]*par[0]);	/* Precalculate the constant for the l = 0 term here */
			Lz = gmt_M_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
			if (Ctrl->debug.test) Lg = gmt_M_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
			else if (Ctrl->A.active) Lg = gmt_M_memory (GMT, NULL, 1, struct GREENSPLINE_LOOKUP);
			L_Max = greenspline_get_max_L (GMT, par[0], par[1]);
			GMT_Report (API, GMT_MSG_DEBUG, "New scheme p = %g, err = %g, L_Max = %u\n", par[0], par[1], L_Max);
			greenspline_series_prepare (GMT, par[0], L_Max, Lz, Lg);
			/* Set up the cubic spline lookup/interpolation */
			par[7] = Ctrl->S.value[3];
			n_columns = irint (par[7]);
			par[8] = (Ctrl->S.rval[1] - Ctrl->S.rval[0]) / (par[7] - 1.0);
			par[9] = 1.0 / par[8];
			par[10] = Ctrl->S.rval[0];
			par[4] = par[8] * par[8];	/* Spline spacing squared, needed by greenspline_csplint */

			GMT_Report (API, GMT_MSG_INFORMATION, "Precalculate -Sq lookup table with %d items from %g to %g\n", n_columns, Ctrl->S.rval[0], Ctrl->S.rval[1]);
			greenspline_spline2d_Wessel_Becker_init (GMT, par, Lz, Lg);
			G = &greenspline_spline2d_Wessel_Becker_lookup;
			dGdr = &greenspline_grad_spline2d_Wessel_Becker_lookup;
			if (Ctrl->debug.test) x0 = -1.0, x1 = 1.0;
			break;
	}

	if (Ctrl->debug.test) {
		GMT_Report (API, GMT_MSG_INFORMATION, "greenspline running in test mode for %s\n", method[Ctrl->S.mode]);
		printf ("# %s\n#x\tG\tdG/dx\tt\n", method[Ctrl->S.mode]);
		greenspline_dump_green (GMT, G, dGdr, par, x0, x1, 10001, Lz, Lg);
		if (dimension == 1) {
			gmt_free_grid (GMT, &Grid, false);
			gmt_M_free (GMT, data);
		}
		for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
		greenspline_free_lookup (GMT, &Lz, 0);
		greenspline_free_lookup (GMT, &Lg, 1);
		Return (0);
	}

	if (dimension == 1) gmt_increase_abstime_format_precision (GMT, GMT_X, Ctrl->I.inc[GMT_X]);	/* In case we need more sub-second precision output */

	/* Remove mean (or LS plane) from data (we will add it back later) */

	if (Ctrl->E.active) {	/* Need to duplicate the data since SVD destroys it. */
		orig_obs = gmt_M_memory (GMT, NULL, nm, double);
		gmt_M_memcpy (orig_obs, obs, nm, double);
	}

	greenspline_do_normalization (API, X, obs, D, n, m, normalize, dimension, norm);

	if (Ctrl->Q.active && dimension == 2) {	/* Need trend slope in Q direction */
		norm[GSP_SLP_Q] = Ctrl->Q.dir[GMT_X] * norm[GSP_SLP_X] + Ctrl->Q.dir[GMT_Y] * norm[GSP_SLP_Y];
		GMT_Report (API, GMT_MSG_INFORMATION, "2-D plane slope in %g direction: %lg\n", Ctrl->Q.az, norm[GSP_SLP_Q]);
	}

	/* Set up linear system Ax = obs. To clarify, the matrix A will be
	 * of size nm by nm, where nm = n + m. Again, n is the number of
	 * value constraints and m is the number of gradient constraints.
	 * for most problems m will be 0.
	 * The loops below takes advantage of the fact that A will be symmetrical
	 * (except for terms involving gradients where A_ij = -A_ji).  So we
	 * start the loop over columns as col = row and deal with A)ij and A_ji
	 * at the same time since we can evaluate the same costly G() function
	 * [or dGdr () function)] once, if possible.
	 *
	 * Planned upgrade PW: Remove this line when this has been implemented.
	 * Note: If -A is used (m > 0) and there are slope constraints that are co-registered
	 * with data constraints then those two constraints share the same Greens function:
	 * one needs the data prediction and the other needs the gradient prediction from it.
	 * The number of such co-registered gradient constraints is n_cr and that means the
	 * number of extra Greens' functions needed for the slopes is only m - n_cr.  In the
	 * extreme case where all gradients are matched by data constraints, m = n_cr. Hence,
	 * the Ax = obs system is no longer square nm by nm but has n+m rows and only n+m-n_cr
	 * columns.  To solve that over-determined system we will form the normal equations
	 * which is done in the -W case except we have no weights (or they are all unity).  So
	 * under the -W processing below we will also enter if n_cr is nonzero, but bypass the
	 * multiplication of the (unity) weights to save time.
	 */

	mem = (double)nm * (double)nm * (double)sizeof (double);	/* In bytes */
	GMT_Report (API, GMT_MSG_INFORMATION, "Square matrix A (size %d x %d) requires %s\n", (int)nm, (int)nm, gmt_memory_use ((size_t)mem, 1));
	A = gmt_M_memory (GMT, NULL, nm * nm, double);

	GMT_Report (API, GMT_MSG_INFORMATION, "Build square linear system Ax = b using %s\n", method[Ctrl->S.mode]);

	/* First do data constraint rows */

	for (row = 0; row < (openmp_int)n; row++) {	/* For each value constraint */
		for (col = row; col < (openmp_int)nm; col++) {	/* For all points and gradient locations at and beyond */
			ij = row * nm + col;	/* Entry in this row of A */
			ji = col * nm + row;	/* Entry in row = col of A for symmetrical ji = ij point */
			r = greenspline_get_radius (GMT, X[col], X[row], dimension);
			/* Value constraint since entire row uses G */
			A[ij] = G (GMT, r, par, Lz);
			if (ij == ji)	/* Do the diagonal terms only once */
				continue;
			if (col < (openmp_int)n)	/* Place symmetrical data entry in reciprocal row */
				A[ji] = A[ij];
		}
	}

	if (m) {	/* Have to build slope constraint rows as well. Tested in 1-D and 2-D */
		for (row = n; row < (openmp_int)nm; row++) {	/* For each slope constraint [in rows n:nm-1] */
			for (col = 0; col < (openmp_int)nm; col++) {	/* We do all columns here since most are not symmetrical */
				ij = row * nm + col;
				r = greenspline_get_radius (GMT, X[col], X[row], dimension);
				if (!gmt_M_is_zero (r)) {	/* For all pairs except self-pairs */
					grad = dGdr (GMT, r, par, Lg);
					C = greenspline_get_dircosine (GMT, D[row-n], X[col], X[row], dimension, true);
					A[ij] = grad * C;
				}
			}
		}
	}

	if (Ctrl->debug.active) greenspline_dump_system (A, obs, nm, "A Matrix row || obs");	/* Dump the A | b system under debug */

	if (Ctrl->E.active && Ctrl->C.history == GMT_SVD_NO_HISTORY) {	/* Needed A to evaluate misfit later as predict = A_orig * x */
		A_orig = gmt_M_memory (GMT, NULL, nm * nm, double);
		gmt_M_memcpy (A_orig, A, nm * nm, double);
	}

	if (Ctrl->W.active) {
		/* Here we have requested an approximate fit instead of an exact interpolation.
		 * For exact interpolation the weights do not matter, but here they do.  Since
		 * we wish to solve via SVD we must convert our unweighted A*x = b linear system
		 * to the weighted W*A*x = W*b whose normal equations are [A'*S*A]*x = A'*S*b,
		 * where S = W*W, the squared weights.  This is N*x = r, where N = A'*S*A and
		 * r = A'*S*b.  Thus, we do these multiplication and store N and r in the
		 * original A and obs vectors so that the continuation of the code can work as is.
		 * Weighted solution idea credit: Leo Uieda.  Jan 14, 2018.
		 */

		double *At = NULL, *AtS = NULL, *S = NULL;	/* Need temporary work space */

		GMT_Report (API, GMT_MSG_INFORMATION, "Forming weighted normal equations A'SAx = A'Sb -> Nx = r\n");
		At = gmt_M_memory (GMT, NULL, nm * nm, double);
		AtS = gmt_M_memory (GMT, NULL, nm * nm, double);
		S = gmt_M_memory (GMT, NULL, nm, double);
		/* 1. Transpose A and set diagonal matrix with squared weights (here a vector) S */
		GMT_Report (API, GMT_MSG_INFORMATION, "Create S = W'*W diagonal matrix, A', and compute A' * S\n");
		for (row = 0; row < (openmp_int)nm; row++) {
			/* Set the diagonal using (=1/sigma^2) if given sigmas or the weights as given */
			S[row] = (Ctrl->W.mode == GSP_GOT_SIG) ? X[row][dimension] * X[row][dimension] : X[row][dimension];
			for (col = 0; col < (openmp_int)nm; col++) {
				ij = row * nm + col;
				ji = col * nm + row;
				At[ji] = A[ij];
			}
		}
		/* 2. Compute AtS = At * S.  This means scaling all terms in At columns by the corresponding S entry */
		for (row = ij = 0; row < (openmp_int)nm; row++) {
			for (col = 0; col < (openmp_int)nm; col++, ij++)
				AtS[ij] = At[ij] * S[col];
		}
		/* 3. Compute r = AtS * obs (but we recycle S to hold r) */
		GMT_Report (API, GMT_MSG_DEBUG, "Compute r = A'*S*b\n");
		gmt_matrix_matrix_mult (GMT, AtS, obs, nm, nm, 1U, S);
		/* 4. Compute N = AtS * A (but we recycle At to hold N) */
		GMT_Report (API, GMT_MSG_DEBUG, "Compute N = A'*S*A\n");
		gmt_matrix_matrix_mult (GMT, AtS, A, nm, nm, nm, At);
		/* Now free A, AtS and obs and let "A" be N and "obs" be r; these are the weighted normal equations */
		gmt_M_free (GMT, A);	gmt_M_free (GMT, AtS);	gmt_M_free (GMT, obs);
		A = At;	obs = S;
		if (Ctrl->debug.active) greenspline_dump_system (A, obs, nm, "Normal equation N row || r");
	}

	if (Ctrl->C.active) {		/* Solve using SVD */
		int error;

		GMT_Report (API, GMT_MSG_INFORMATION, "Solve linear equations by Singular Value Decomposition\n");
#ifndef HAVE_LAPACK
		GMT_Report (API, GMT_MSG_WARNING, "Note: SVD solution without LAPACK will be very, very slow.\n");
		GMT_Report (API, GMT_MSG_WARNING, "We strongly recommend you install LAPACK and recompile GMT.\n");
#endif
		v = gmt_M_memory (GMT, NULL, nm * nm, double);
		s = gmt_M_memory (GMT, NULL, nm, double);
		if ((error = gmt_svdcmp (GMT, A, (unsigned int)nm, (unsigned int)nm, s, v)) != 0) Return (error);
		if (Ctrl->C.history) {	/* Keep copy of original singular values */
			ssave = gmt_M_memory (GMT, NULL, nm, double);
			gmt_M_memcpy (ssave, s, nm, double);
		}
		if (Ctrl->C.file) {	/* Save the singular values for study */
			struct GMT_SINGULAR_VALUE *eigen = gmt_sort_svd_values (GMT, s, nm);
			uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, nm, 2};
			unsigned int col_type[3];
			struct GMT_DATASET *E = NULL;
			if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving singular values\n");
				Return (API->error);
			}
			gmt_M_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], 2, unsigned int);	/* Save previous x/y output col types */
			GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;
			for (i = 0; i < nm; i++) {
				E->table[0]->segment[0]->data[GMT_X][i] = i;	/* Let 0 be x-value of the first eigenvalue */
				E->table[0]->segment[0]->data[GMT_Y][i] = eigen[i].value;
			}
			E->table[0]->segment[0]->n_rows = nm;
			if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, "index\teigenvalue", E)) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->C.file, E) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, 2, unsigned int);	/* Restore output col types */
			GMT_Report (API, GMT_MSG_INFORMATION, "Eigenvalues saved to %s\n", Ctrl->C.file);
			gmt_M_free (GMT, eigen);

			if (Ctrl->C.dryrun) {	/* We are done */
				for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
				gmt_M_free (GMT, X);
				gmt_M_free (GMT, s);
				gmt_M_free (GMT, v);
				gmt_M_free (GMT, A);
				gmt_M_free (GMT, obs);
				if (dimension == 2) gmt_free_grid (GMT, &Grid, true);
				Return (GMT_NOERROR);
			}
		}
		b = gmt_M_memory (GMT, NULL, nm, double);
		gmt_M_memcpy (b, obs, nm, double);
		n_use = gmt_solve_svd (GMT, A, (unsigned int)nm, (unsigned int)nm, v, s, b, 1U, obs, Ctrl->C.value, Ctrl->C.mode);
		if (n_use == -1) Return (GMT_RUNTIME_ERROR);
		GMT_Report (API, GMT_MSG_INFORMATION, "[%d of %" PRIu64 " eigen-values used]\n", n_use, nm);

		if (Ctrl->C.history == GMT_SVD_NO_HISTORY) {
			gmt_M_free (GMT, s);
			gmt_M_free (GMT, v);
			gmt_M_free (GMT, b);
		}
	}
	else {				/* Gauss-Jordan elimination */
		int error;
		if (gmt_M_is_zero (r_min)) {
			GMT_Report (API, GMT_MSG_ERROR, "Your matrix is singular because you have duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_ERROR, "Preprocess your data with one of the blockm* modules to eliminate them\n");

		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Solve linear equations by Gauss-Jordan elimination\n");
		if (!Ctrl->W.active) GMT_Report (API, GMT_MSG_INFORMATION, "In the absence of weights, the solution will be exact\n");
		if ((error = gmt_gaussjordan (GMT, A, (unsigned int)nm, obs)) != 0) {
			GMT_Report (API, GMT_MSG_ERROR, "You probably have nearly duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_ERROR, "Preprocess your data with one of the blockm* modules\n");
			Return (error);
		}
	}
	alpha = obs;	/* Just a different name since the obs vector now holds the alpha factors */

	if (Ctrl->M.active && Ctrl->M.mode == GMT_OUT) {
		/* EXPERIMENTAL and not completed - need normalization information, trend etc */
		bool was = GMT->current.setting.io_header[GMT_OUT];	/* Current setting */
		uint64_t m_dim[GMT_DIM_SIZE] = {1, 1, 0, 1};	/* Do not allocate any rows */
		char header[GMT_LEN64] = {""};
		struct GMT_DATASET *M = NULL;

		if ((M = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, m_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving misfit estimates\n");
			Return (API->error);
		}
		M->table[0]->segment[0]->n_rows = nm;
		M->table[0]->segment[0]->data[GMT_X] = alpha;

		sprintf (header, "N: %" PRIu64 " S: %s G: %s", nm, (Ctrl->C.active) ? "SVD" : "G-J", Ctrl->S.arg);
		gmt_insert_tableheader (GMT, M->table[0], header);
		GMT->current.setting.io_header[GMT_OUT] = true;

		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->M.file, M) != GMT_NOERROR) {
			Return (API->error);
		}
		M->table[0]->segment[0]->data[GMT_X] = NULL;	/* Since we did not allocate that array */
		GMT->current.setting.io_header[GMT_OUT] = was;	/* Restore default */
	}

	if (Ctrl->C.history == GMT_SVD_NO_HISTORY) gmt_M_free (GMT, A);

	finalize = (Ctrl->Q.active) ? greenspline_undo_normalization_grad : greenspline_undo_normalization;

	if (Ctrl->E.active && Ctrl->C.history == GMT_SVD_NO_HISTORY) {
		double value, mean = 0.0, std = 0.0, rms = 0.0, dev, chi2, chi2_sum = 0, pvar_sum = 0.0, *predicted = NULL;
		uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, nm, dimension+3+Ctrl->W.active};
		unsigned int m = 0;
		struct GMT_DATASET *E = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		if (Ctrl->E.mode) {	/* Want to write out prediction errors */
			if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving misfit estimates\n");
				Return (API->error);
			}
			S = E->table[0]->segment[0];
			S->n_rows = nm;
		}
		predicted = gmt_M_memory (GMT, NULL, nm, double);	/* To hold predictions */
		gmt_matrix_matrix_mult (GMT, A_orig, alpha, nm, nm, 1U, predicted);	/* predicted = A * alpha are normalized predictions at data points */
		for (j = 0; j < nm; j++) {	/* For each data constraint */
			predicted[j] = finalize (X[j], predicted[j], normalize, norm, dimension);	/* undo normalization first */
			pvar_sum += predicted[j] * predicted[j];	/* Sum of predicted variance */
			dev = value = orig_obs[j] - predicted[j];	/* Deviation between observed and predicted */
			rms += dev * dev;	/* Accumulate rms sum */
			if (Ctrl->W.active) {	/* If data had uncertainties we also compute the chi2 sum */
				chi2 = pow (dev * X[j][dimension], 2.0);
				chi2_sum += chi2;
			}
			/* Use Welford (1962) algorithm to compute mean and variance */
			m++;
			dev = value - mean;
			mean += dev / m;
			std += dev * (value - mean);
			if (Ctrl->E.mode) {	/* Store assessment for each observation in misfit table */
				for (p = 0; p < dimension; p++)	/* Duplicate point coordinates */
					S->data[p][j] = X[j][p];
				S->data[p++][j] = orig_obs[j];
				S->data[p++][j] = predicted[j];
				S->data[p][j]   = value;
				if (Ctrl->W.active)
					S->data[++p][j] = chi2;
			}
		}
		rms = sqrt (rms / nm);
		std = (m > 1) ? sqrt (std / (m-1.0)) : GMT->session.d_NaN;
		if (Ctrl->W.active)
			GMT_Report (API, GMT_MSG_INFORMATION, "Misfit evaluation: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\tChi^2 = %g\n", nm, mean, std, rms, chi2_sum);
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "Misfit evaluation: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\n", nm, mean, std, rms);
		GMT_Report (API, GMT_MSG_INFORMATION, "Variance evaluation: Data = %g\tModel = %g\tExplained = %5.1lf %%\n", var_sum, pvar_sum, 100.0 * pvar_sum / var_sum);
		gmt_M_free (GMT, orig_obs);	gmt_M_free (GMT, predicted);	gmt_M_free (GMT, A_orig);
		if (Ctrl->E.mode) {	/* Want to write out prediction errors */
			char header[GMT_LEN64] = {""};
			if (gmt_M_x_is_lon (GMT, GMT_IN))
				sprintf (header, "#lon\t");
			else
				sprintf (header, "#x\t");
			if (dimension > 1) strcat (header, gmt_M_y_is_lat (GMT, GMT_IN) ? "lat\t" : "y\t");
			if (dimension > 2) strcat (header, "z\t");
			strcat (header, "obs\tpredict\tdev");
			if (Ctrl->W.active) strcat (header, "\tchi2");
			if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, E)) {
				Return (API->error);
			}
			gmt_set_tableheader (API->GMT, GMT_OUT, true);	/* So header is written */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->E.file, E) != GMT_NOERROR) {
				Return (API->error);
			}
		}
	}

	if (Ctrl->N.file || dimension == 1 || write_3D_records) Rec = gmt_new_record (GMT, NULL, NULL);

	if (Ctrl->N.file) {	/* Specified nodes only */
		unsigned int wmode = GMT_ADD_DEFAULT;
		double out[4];

		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active) {
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET)
				Return (API->error);
			wmode = GMT_ADD_EXISTING;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Establishes output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, dimension + 1, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		gmt_M_memset (out, 4, double);
		Rec->data = out;
		GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate spline at %" PRIu64 " given locations\n", T->n_records);
		for (seg = 0; seg < T->n_segments; seg++) {
			for (row = 0; row < (openmp_int)T->segment[seg]->n_rows; row++) {
				for (ii = 0; ii < dimension; ii++) out[ii] = T->segment[seg]->data[ii][row];
				if (T->segment[seg]->text) Rec->text = T->segment[seg]->text[row];
				out[dimension] = 0.0;
				for (p = 0; p < nm; p++) {
					r = greenspline_get_radius (GMT, out, X[p], dimension);
					if (!gmt_M_is_zero (r)) {	/* For all pairs except self-pairs */
						if (Ctrl->Q.active) {
							C = greenspline_get_dircosine (GMT, Ctrl->Q.dir, out, X[p], dimension, false);
							part = dGdr (GMT, r, par, Lz) * C;
						}
						else
							part = G (GMT, r, par, Lz);
						out[dimension] += alpha[p] * part;
					}
				}
				out[dimension] = finalize (out, out[dimension], normalize, norm, dimension);
				GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Nin) != GMT_NOERROR) {
			Return (API->error);
		}
		gmt_fclose (GMT, fp);
	}
	else {	/* Output on equidistant lattice */
		uint64_t nz_off, nxy;
		unsigned int layer, wmode = GMT_ADD_DEFAULT;
		double *xp = NULL, *yp = NULL, wp, V[4] = {0.0, 0.0, 0.0, 0.0};
		GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate spline at %" PRIu64 " equidistant output locations\n", n_ok);
		/* Precalculate coordinates */
		xp = gmt_grd_coord (GMT, header, GMT_X);
		if (dimension > 1) yp = gmt_grd_coord (GMT, header, GMT_Y);
		nxy = header->size;	/* Will only be used for 3-D anyway when there are layers */
		if (dimension == 1 || write_3D_records) {	/* Write ASCII table to named file or stdout for 1-D or stdout for 3-D */
			GMT->common.b.ncol[GMT_OUT] = dimension + 1;
			if (Ctrl->G.active) {
				if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET) {
					gmt_M_free (GMT, xp);
					Return (error);
				}
				wmode = GMT_ADD_EXISTING;
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Establishes output */
				gmt_M_free (GMT, xp);
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				gmt_M_free (GMT, xp);
				Return (API->error);
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, xp);
				Return (API->error);
			}

		} /* Else we are writing a grid or cube */
		if (Ctrl->C.history) {	/* 2-D only: Write out grid after adding contribution for each eigenvalue separately */
			/* Note: Because the SVD decomposition is not sorting the vectors from largest to smallest eigenvalue the
			 * gmt_solve_svd sets to zero those we don't want but we must still loop over its full length to ensure we
			 * include the eigenvalues we want. */
			unsigned int width = urint (floor (log10 ((double)n_use))) + 1;	/* Width of maximum integer needed */
			uint64_t e, p;
			gmt_grdfloat *current = NULL, *previous = NULL;
			double l2_sum_n = 0.0, l2_sum_e = 0.0, predicted;
			static char *mkind[3] = {"", "Incremental", "Cumulative"};
			char file[PATH_MAX] = {""};
			struct GMT_SINGULAR_VALUE *eigen = NULL;
			struct GMT_DATASET *E = NULL;
			struct GMT_DATASEGMENT *S = NULL;

			current  = gmt_M_memory_aligned (GMT, NULL, Out->header->size, gmt_grdfloat);
			if (Ctrl->C.history & GMT_SVD_INCREMENTAL)
				previous = gmt_M_memory_aligned (GMT, NULL, Out->header->size, gmt_grdfloat);
			gmt_grd_init (GMT, Out->header, options, true);
			if (Ctrl->E.active) {	/* Want to write out misfit as function of eigenvalue */
				uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, n_use, 4+Ctrl->W.active};
 				eigen = gmt_sort_svd_values (GMT, ssave, nm);	/* Get sorted eigenvalues */
				if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving misfit estimates per eigenvector\n");
					Return (API->error);
				}
				for (k = 0; k < nm; k++)	/* Get sum of squared eigenvalues */
					l2_sum_n += eigen[k].value * eigen[k].value;
				S = E->table[0]->segment[0];
				S->n_rows = n_use;
			}

			for (e = 0; e < (uint64_t)n_use; e++) {	/* Only loop over the first n_use eigenvalues (if restricted) */
				GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate spline for eigenvalue # %d\n", (int)e);
				gmt_M_memcpy (s, ssave, nm, double);	/* Restore original values before call */
				(void)gmt_solve_svd (GMT, A, (unsigned int)nm, (unsigned int)nm, v, s, b, 1U, obs, (double)e, GMT_SVD_EIGEN_NUMBER_CUTOFF);
				/* obs (hence alpha) now has the solution for the coefficients based on the first e eigenvalues */
				if (Ctrl->Q.active) {	/* Derivatives of solution */
#ifdef _OPENMP
#pragma omp parallel for private(row,V,col,ij,p,wp,r,C,part) shared(Grid,yp,xp,nm,GMT,Ctrl,X,G,par,Lz,alpha,Out,normalize,norm)
#endif
					for (row = 0; row < (openmp_int)Grid->header->n_rows; row++) {
						V[GMT_Y] = yp[row];
						for (col = 0; col < (openmp_int)Grid->header->n_columns; col++) {
							ij = gmt_M_ijp (Grid->header, row, col);
							if (gmt_M_is_fnan (Grid->data[ij])) continue;	/* Only do solution where mask is not NaN */
							V[GMT_X] = xp[col];
							/* Here, V holds the current output coordinates */
							for (p = 0, wp = 0.0; p < nm; p++) {
								r = greenspline_get_radius (GMT, V, X[p], 2U);
								if (!gmt_M_is_zero (r)) {	/* For all pairs except self-pairs */
									C = greenspline_get_dircosine (GMT, Ctrl->Q.dir, V, X[p], 2U, false);
									part = dGdr (GMT, r, par, Lz) * C;
									wp += alpha[p] * part;
								}
							}
							V[GMT_Z] = finalize (V, wp, normalize, norm, 2U);
							Out->data[ij] = (gmt_grdfloat)V[GMT_Z];
						}
					}
				}
				else {	/* Surface solution */
					if (Ctrl->E.active) {	/* Compute the history of model misfit */
						double dev, rms = 0.0, chi2_sum = 0.0;
						for (j = 0; j < nm; j++) {	/* For each data constraint */
							for (p = 0, wp = 0.0; p < nm; p++) {	/* Add contribution for each data constraint */
								if (!gmt_M_is_zero (r)) {	/* For all pairs except self-pairs */
									r = greenspline_get_radius (GMT, X[j], X[p], 2U);
									part = G (GMT, r, par, Lz);
									wp += alpha[p] * part;	/* Just add this scaled Green's function */
								}
							}
							predicted = finalize (X[j], wp, normalize, norm, dimension);	/* Undo normalization first */
							dev = orig_obs[j] - predicted;	/* Deviation between observed and predicted */
							dev *= dev;	/* Squared misfit */
							rms += dev;	/* Accumulate rms sum */
							if (Ctrl->W.active) {	/* If data had uncertainties we also compute the chi2 sum */
								double chi2 = dev * pow (X[j][2], 2.0);
								chi2_sum += chi2;
							}
						}
						rms = sqrt (rms / nm);
						l2_sum_e += eigen[e].value * eigen[e].value;
						if (Ctrl->W.active)
							GMT_Report (API, GMT_MSG_INFORMATION, "Cumulative data misfit for eigenvalue # %d: rms = %lg chi2 = %lg\n", (int)e, rms, chi2_sum);
						else
							GMT_Report (API, GMT_MSG_INFORMATION, "Cumulative data misfit for eigenvalue # %d: rms = %lg\n", (int)e, rms);
						S->data[0][e] = e;	/* Eigenvalue number (starting at 0) */
						S->data[1][e] = eigen[e].value;	/* Eigenvalue, from largest to smallest */
						S->data[2][e] = 100.0 * l2_sum_e / l2_sum_n;	/* Percent of model variance */
						S->data[3][e] = rms;	/* RMS misfit for this solution */
						if (Ctrl->W.active) S->data[4][e] = chi2_sum;	/* Chi^2 sum for this solution */
					}
#ifdef _OPENMP
#pragma omp parallel for private(row,V,col,ij,p,wp,r,part) shared(Grid,yp,xp,nm,GMT,X,G,par,Lz,alpha,Out,normalize,norm)
#endif
					for (row = 0; row < (openmp_int)Grid->header->n_rows; row++) {
						V[GMT_Y] = yp[row];
						for (col = 0; col < (openmp_int)Grid->header->n_columns; col++) {
							ij = gmt_M_ijp (Grid->header, row, col);
							if (gmt_M_is_fnan (Grid->data[ij])) continue;	/* Only do solution where mask is not NaN */
							V[GMT_X] = xp[col];
							/* Here, V holds the current output coordinates */
							for (p = 0, wp = 0.0; p < nm; p++) {
								r = greenspline_get_radius (GMT, V, X[p], 2U);
								part = G (GMT, r, par, Lz);
								wp += alpha[p] * part;
							}
							Out->data[ij] = (gmt_grdfloat)finalize (V, wp, normalize, norm, 2U);
						}
					}	/* End of row-loop [OpenMP] */
				}
				gmt_M_memcpy (current, Out->data, Out->header->size, gmt_grdfloat);	/* Save current solution */

				if (Ctrl->C.history & GMT_SVD_CUMULATIVE) {	/* Write out the cumulative solution first */
					if (strchr (Ctrl->G.file, '%'))	/* Gave a template, use it to write one of the two types of grids */
						sprintf (file, Ctrl->G.file, (int)e+1);
					else	/* Create the appropriate cumulative gridfile name from static file name */
						greenspline_set_filename (Ctrl->G.file, e, width, GMT_SVD_CUMULATIVE, file);
					snprintf (Out->header->remark, GMT_GRID_REMARK_LEN160, "%s (-S%s). %s contribution for eigenvalue # %d", method[Ctrl->S.mode], Ctrl->S.arg, mkind[GMT_SVD_CUMULATIVE], (int)e+1);
					if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out))
						Return (API->error);				/* Update solution for e eigenvalues only */
					if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Out) != GMT_NOERROR)
						Return (API->error);
				}
				if (Ctrl->C.history & GMT_SVD_INCREMENTAL) {	/* Want to write out incremental solution due to this eigenvalue */
					gmt_M_grd_loop (GMT, Out, row, col, ij) Out->data[ij] = current[ij] - previous[ij];	/* Incremental improvement since last time */
					gmt_M_memcpy (previous, current, Out->header->size, gmt_grdfloat);	/* Save current solution which will be previous for next eigenvalue */
					if (strchr (Ctrl->G.file, '%'))	/* Gave a template, use it to write one of the two types of grids */
						sprintf (file, Ctrl->G.file, (int)e+1);
					else	/* Create the appropriate cumulative gridfile name from static file name */
						greenspline_set_filename (Ctrl->G.file, e, width, GMT_SVD_INCREMENTAL, file);
					snprintf (Out->header->remark, GMT_GRID_REMARK_LEN160, "%s (-S%s). %s contribution for eigenvalue # %d", method[Ctrl->S.mode], Ctrl->S.arg, mkind[GMT_SVD_INCREMENTAL], (int)e+1);
					if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out))
						Return (API->error);				/* Update solution for e eigenvalues only */
					if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Out) != GMT_NOERROR)
						Return (API->error);
				}
			}
			if (Ctrl->E.active) {	/* Compute the history of model misfit as rms */
				char header[GMT_LEN64] = {""};
				sprintf (header, "# eigenno\teigenval\tvar_percent\trms");
				if (Ctrl->W.active) strcat (header, "\tchi2");
				if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, E)) {
					Return (API->error);
				}
				gmt_set_tableheader (API->GMT, GMT_OUT, true);	/* So header is written */
				for (k = 0; k < 5; k++) GMT->current.io.col_type[GMT_OUT][k] = GMT_IS_FLOAT;	/* Set plain float column types */
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->E.file, E) != GMT_NOERROR) {
					Return (API->error);
				}
				gmt_M_free (GMT, eigen);
				gmt_M_free (GMT, orig_obs);
			}

			/* Free temporary arrays */
			gmt_M_free_aligned (GMT, current);
			if (Ctrl->C.history & GMT_SVD_INCREMENTAL)
				gmt_M_free_aligned (GMT, previous);
			gmt_M_free (GMT, A);
			gmt_M_free (GMT, s);
			gmt_M_free (GMT, v);
			gmt_M_free (GMT, b);
			gmt_M_free (GMT, ssave);
		}
		else {	/* Just compute the final interpolation */
			if (dimension == 1 || write_3D_records) Rec->data = V;	/* For rec-by-rec output */
			for (layer = 0, nz_off = 0; layer < n_layers; layer++, nz_off += nxy) {	/* Might be dummy loop of 1 layer unless 3-D */
				int64_t col, row, p; /* On Windows, the 'for' index variables must be signed, so redefine these 3 inside this block only */
				double z_layer = (dimension == 3) ? Cube->z[layer] : 0.0;
				if (Ctrl->Q.active) {	/* Derivatives of solution */
#ifdef _OPENMP
#pragma omp parallel for private(row,V,ij,col,p,wp,r,C,part) shared(header,dimension,yp,z_layer,nz_off,data,xp,nm,GMT,X,Ctrl,dGdr,par,Lz,alpha,normalize,norm)
#endif
					for (row = 0; row < (openmp_int)header->n_rows; row++) {	/* This would be a dummy loop for 1 row if 1-D data */
						if (dimension > 1)  V[GMT_Y] = yp[row];
						if (dimension == 3) V[GMT_Z] = z_layer;
						ij = (dimension > 1) ? gmt_M_ijp (header, row, 0) + nz_off : 0;
						for (col = 0; col < (openmp_int)header->n_columns; col++, ij++) {	/* This loop is always active for 1-,2-,3-D */
							if (dimension == 2 && gmt_M_is_fnan (data[ij])) continue;	/* Only do solution where mask is not NaN */
							V[GMT_X] = xp[col];
							/* Here, V holds the current output coordinates */
							for (p = 0, wp = 0.0; p < (int64_t)nm; p++) {	/* Loop over Green's function components */
								r = greenspline_get_radius (GMT, V, X[p], dimension);
								if (!gmt_M_is_zero (r)) {	/* For all pairs except self-pairs */
									C = greenspline_get_dircosine (GMT, Ctrl->Q.dir, V, X[p], dimension, false);
									part = dGdr (GMT, r, par, Lz) * C;
									wp += alpha[p] * part;
								}
							}
							data[ij] = (gmt_grdfloat)finalize (V, wp, normalize, norm, dimension);
						}
					}	/* End of row-loop [OpenMP] */
				}
				else {	/* Regular surface */
#ifdef _OPENMP
#pragma omp parallel for private(row,V,ij,col,p,wp,r,part) shared(header,dimension,yp,z_layer,nz_off,data,xp,nm,GMT,X,G,par,Lz,alpha,normalize,norm)
#endif
					for (row = 0; row < (openmp_int)header->n_rows; row++) {	/* This would be a dummy loop for 1 row if 1-D data */
						if (dimension > 1)  V[GMT_Y] = yp[row];
						if (dimension == 3) V[GMT_Z] = z_layer;
						ij = (dimension > 1) ? gmt_M_ijp (header, row, 0) + nz_off : 0;
						for (col = 0; col < (openmp_int)header->n_columns; col++, ij++) {	/* This loop is always active for 1-,2-,3-D */
							if (dimension == 2 && gmt_M_is_fnan (data[ij])) continue;	/* Only do solution where mask is not NaN */
							V[GMT_X] = xp[col];
							/* Here, V holds the current output coordinates */
							for (p = 0, wp = 0.0; p < (int64_t)nm; p++) {	/* Loop over Green's function components */
								r = greenspline_get_radius (GMT, V, X[p], dimension);
								part = G (GMT, r, par, Lz);
								wp += alpha[p] * part;
							}
							data[ij] = (gmt_grdfloat)finalize (V, wp, normalize, norm, dimension);
						}
					}	/* End of row-loop [OpenMP] */
				}

				if (write_3D_records) {	/* Must dump this slice of the 3-D cube as ASCII slices as a backwards compatibility option */
					V[GMT_Z] = z_layer;
					for (row = 0; row < (openmp_int)header->n_rows; row++) {
						V[GMT_Y] = yp[row];
						for (col = 0; col < header->n_columns; col++) {
							V[GMT_X] = xp[col];
							ij = gmt_M_ijp (header, row, col) + nz_off;
							V[dimension] = data[ij];
							GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
						}
					}
				}
			}	/* End of layer loop */

			/* Time to write output */
			if (dimension == 1) {	/* Must dump 1-D records */
				for (col = 0; col < (openmp_int)header->n_columns; col++) {
					V[GMT_X] = xp[col];
					V[GMT_Y] = data[col];
					GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
				}
				gmt_M_free (GMT, data);
			}
			else if (dimension == 2) {	/* Write the 2-D grid */
				gmt_grd_init (GMT, Out->header, options, true);
				if (Ctrl->D.active && gmt_decode_grd_h_info (GMT, Ctrl->D.information, Out->header)) {
					Return (GMT_PARSE_ERROR);
				}
				snprintf (Out->header->remark, GMT_GRID_REMARK_LEN160, "%s (-S%s)", method[Ctrl->S.mode], Ctrl->S.arg);
				if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
				if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
					Return (API->error);
				}
			}
			else if (dimension == 3 && !write_3D_records) {	/* Write the 3-D cube */
				gmt_grd_init (GMT, Cube->header, options, true);
				snprintf (Cube->header->remark, GMT_GRID_REMARK_LEN160, "%s (-S%s)", method[Ctrl->S.mode], Ctrl->S.arg);
				if (GMT_Set_Comment (API, GMT_IS_CUBE, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Cube)) Return (API->error);
				if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Cube))
					Return (EXIT_FAILURE);
			}
		}
		if (delete_grid) /* No longer required for 1-D and 3-D */
			gmt_free_grid (GMT, &Grid, false);
		if (dimension == 3) GMT_Destroy_Data (API, &Cube);	/* Done with the output cube */

		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		gmt_M_free (GMT, xp);
		if (dimension > 1) gmt_M_free (GMT, yp);
	}

	/* Clean up */

	for (p = 0; p < nm; p++) gmt_M_free (GMT, X[p]);
	gmt_M_free (GMT, X);
	gmt_M_free (GMT, obs);
	if (m) {
		for (p = 0; p < m; p++) gmt_M_free (GMT, D[p]);
		gmt_M_free (GMT, D);
		gmt_M_free (GMT, kolumn);
	}
	if (Rec) gmt_M_free (GMT, Rec);
	greenspline_free_lookup (GMT, &Lz, 0);
	greenspline_free_lookup (GMT, &Lg, 1);

	Return (GMT_NOERROR);
}
