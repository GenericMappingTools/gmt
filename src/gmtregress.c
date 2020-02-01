/*
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: gmtregress fits y = a + bx linear regression to x,y[,w] data
 *	using a variety of misfit norms and regression modes.
 *
 * Author:	Paul Wessel
 * Date:	5-JAN-2015
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtregress"
#define THIS_MODULE_MODERN_NAME	"gmtregress"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Linear regression of 1-D data sets"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioqs"

/* Various integer constants for flags and modes */

enum GMT_enum_regress {
	GMTREGRESS_N_FARGS	= 7,
	GMTREGRESS_X		= 0,
	GMTREGRESS_Y		= 1,
	GMTREGRESS_XY		= 2,
	GMTREGRESS_RMA		= 3,
	GMTREGRESS_NORM_L1	= 0,
	GMTREGRESS_NORM_L2	= 1,
	GMTREGRESS_NORM_LMS	= 2,
	GMTREGRESS_NORM_RLS	= 3,
	GMTREGRESS_ANGLE	= 0,
	GMTREGRESS_MISFT	= 1,
	GMTREGRESS_SLOPE	= 2,
	GMTREGRESS_ICEPT	= 3,
	GMTREGRESS_SIGSL	= 4,
	GMTREGRESS_SIGIC	= 5,
	GMTREGRESS_XMEAN	= 6,
	GMTREGRESS_YMEAN	= 7,
	GMTREGRESS_R		= 8,
	GMTREGRESS_CORR		= 9,
	GMTREGRESS_NPAR		= 10,
	GMTREGRESS_NPAR_MAIN	= 4,
	GMTREGRESS_OUTPUT_GOOD  = 1,
	GMTREGRESS_OUTPUT_BAD   = 2};

#define GMTREGRESS_FARGS	"xymrczw"	/* Default -F setting */
#define GMTREGRESS_ZSCORE_LIMIT	2.5		/* z-scores higher than this are outliers */

/* Control structure for gmtregress */

struct GMTREGRESS_CTRL {
	struct Out {	/* ->[<outfile>] */
		bool active;
		char *file;
	} Out;
	struct A {	/* 	-A<min>/<max>/<inc> */
		bool active;
		double min, max, inc;
	} A;
	struct C {	/* 	-C<confidence> */
		bool active;
		double value;
	} C;
	struct E {	/* 	-Ex|y|o|r */
		bool active;
		unsigned int mode;
	} E;
	struct F {	/* 	-Fxymrcsw */
		bool active;
		bool band;	/* True if c was given */
		bool param;	/* True if only -Fp was given */
		unsigned int n_cols;
		char col[GMTREGRESS_N_FARGS];	/* Character codes for desired output in the right order */
	} F;
	struct N {	/* 	-N1|2|r|w */
		bool active;
		unsigned int mode;
	} N;
	struct S {	/* 	-S[r] */
		bool active;
		unsigned int mode;
	} S;
	struct T {	/* 	-T[<min>/<max>/]<inc>[+n] */
		bool active;
		bool no_eval;
		struct GMT_ARRAY T;
	} T;
	struct W {	/* 	-W[s]x|y|r */
		bool active;
		unsigned int type;	/* 0 for weights, 1 if sigmas */
		unsigned int n_weights;	/* 1-3 if any weights are selected */
		unsigned int col[3];	/* Column numbers >=2 if weights are present */
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTREGRESS_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTREGRESS_CTRL);
	C->A.min = -90.0;	C->A.max = 90.0;	C->A.inc = 1.0;
	C->C.value = 0.95;
	C->E.mode = GMTREGRESS_Y;
	C->N.mode = GMTREGRESS_NORM_L2;

	return ((void *)C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTREGRESS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A[<min>/<max>/<inc>]] [-C<level>] [-Ex|y|o|r] [-F<flags>] [-N1|2|r|w]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-S[r]] [-T[<min>/<max>/]<inc>[+n] [%s] [-W[w][x][y][r]] [%s]\n", GMT_V_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Examine E as function of line slope; give angle range and increment [-90/+90/1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Option -F is not required as no model will be returned; instead we return\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   records of (angle, E, slope, intercept) for all angles specified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Select level (in %%) to use in confidence band calculations (see -Fc) [95].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Regression type. Select how misfit should be measured:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : Horizontally from data point to regression line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : Vertically from data point to regression line [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     o : Orthogonally from data point to regression line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : Use Reduced Major Axis area misfit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Append desired output columns in any order; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : The x observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : The y observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m : The estimated model values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : The estimated residuals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c : The confidence limits (to add/subtract from m).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     z : The standardized residuals (z-scores).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w : The outlier flags (1 or 0), or Reweighted Least Squares weights (for -Nw).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t         A value of 0 identifies an outlier.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Note: Cannot use y|r|z|w with -T. With -T, x means locations implied by -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is %s].\n", GMTREGRESS_FARGS);
	GMT_Message (API, GMT_TIME_NONE, "\t     Alternatively, choose -Fp to output only the model parameters:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     N meanX meanY angle misfit slope icept sigma_slope sigma_icept\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Append desired misfit norm; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 : L-1 measure (mean absolute residuals).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2 : L-2 measure (mean squared residuals) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : LMS robust measure (median of squared residuals).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w : RLS Reweighted L-2 measure (r followed by 2 after excluding outliers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Skip records identified as outliers on output. Append r to reverse mode and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   only output the outlier records. Cannot be used with -T [output all records].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Evaluate model at the equidistant points implied by the arguments.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only -T<inc>[+n] is given we reset <min> and <max> to the extreme x-values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for each segment.  Append +n if <inc> is the number of t-values to produce instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For absolute time data, append a valid time unit (%s) to the increment.\n", GMT_TIME_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with output times in the first column, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -T0 to bypass model evaluation entirely.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses locations of input data to evaluate the model].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Supply individual 1-sigma uncertainties for data points [no weights].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append x for sigma_x, y for sigma_y, and r for x-y correlation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   We then expect 1-3 extra columns with these data in the given order.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Given a sigma, the weight will be computed via weight = 1/sigma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ww if weights are precomputed and not given as 1-sigma values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Except for -N1 we square the weights when computing misfits.\n");
	GMT_Option (API, "V,a,bi,bo,d,e,g,h,i,o,q,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTREGRESS_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtregress and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j, k, n, col, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Explore E vs slope */
				Ctrl->A.active = true;
				if (opt->arg[0]) {
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->A.min, &Ctrl->A.max, &Ctrl->A.inc);
					n_errors += gmt_M_check_condition (GMT, n < 2, "Option -A: Must specify min/max/inc\n");
				}
				break;
			case 'C':	/* Set confidence level in %, convert to fraction */
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg) * 0.01;
				break;
			case 'E':	/* Select regression type */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'x': Ctrl->E.mode = GMTREGRESS_X;   break; /* Regress on x */
					case 'y': Ctrl->E.mode = GMTREGRESS_Y;   break; /* Regress on y */
					case 'o': Ctrl->E.mode = GMTREGRESS_XY;  break; /* Orthogonal Regression*/
					case 'r': Ctrl->E.mode = GMTREGRESS_RMA; break; /* RMA Regression*/
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -E: Unrecognized type %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'F':	/* Select output columns */
				Ctrl->F.active = true;
				if (!strcmp (opt->arg, "p")) {	/* Just want to return model parameters */
					Ctrl->F.param = true;
					break;
				}
				for (j = 0, k = 0; opt->arg[j]; j++, k++) {
					if (k < GMTREGRESS_N_FARGS) {
						Ctrl->F.col[k] = opt->arg[j];
						if (!strchr (GMTREGRESS_FARGS, opt->arg[j])) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -F: Choose from -F%s\n", GMTREGRESS_FARGS);
							n_errors++;
						}
						Ctrl->F.n_cols++;
						if (opt->arg[j] == 'c') Ctrl->F.band = true;
					}
					else {
						n_errors++;
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -F: Too many output columns selected\n");
					}
				}
				break;
			case 'N':	/* Select Norm for misfit calculations */
				Ctrl->N.active = true;
				switch (opt->arg[0]) {
					case '1': Ctrl->N.mode = GMTREGRESS_NORM_L1;	break;
					case '2': Ctrl->N.mode = GMTREGRESS_NORM_L2;	break;
					case 'r': Ctrl->N.mode = GMTREGRESS_NORM_LMS;	break;
					case 'w': Ctrl->N.mode = GMTREGRESS_NORM_RLS;	break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -N: Unrecognized norm %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'S':	/* Restrict output records */
				Ctrl->S.active = true;
				Ctrl->S.mode = (opt->arg[0] == 'r') ? GMTREGRESS_OUTPUT_BAD : GMTREGRESS_OUTPUT_GOOD;
				break;
			case 'T':	/* Output lattice or length */
				Ctrl->T.active = true;
				if (!strcmp (opt->arg, "0"))	/* -T0 means no model evaluation */
					Ctrl->T.no_eval = true;
				else
					n_errors += gmt_parse_array (GMT, 'T', opt->arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_DIST, 0);
				break;
			case 'W':	/* Weights or not */
				Ctrl->W.active = true;
				if (opt->arg[0] == 'w') Ctrl->W.type = 1;	/* Got weights; determine their column position */
				for (k = Ctrl->W.type, col = GMT_Z; opt->arg[k]; k++) {
					if (opt->arg[k] == 'x') Ctrl->W.col[GMT_X] = col++;
					else if (opt->arg[k] == 'y') Ctrl->W.col[GMT_Y] = col++;
					else if (opt->arg[k] == 'r') Ctrl->W.col[GMT_Z] = col++;
					else {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -W: Specify -W[w][x][y][r]\n");
						n_errors++;
					}
					Ctrl->W.n_weights++;
				}
				if (Ctrl->W.n_weights > 3) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -W: Gave more than 3 uncertainty types\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->T.active, "Option -S: Cannot simultaneously specify -T.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_XY && Ctrl->W.n_weights == 1, "Option -Eo: Needs errors in both x,y or neither.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_RMA && Ctrl->W.n_weights == 1, "Option -Er: Needs errors in both x,y or neither.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_X && Ctrl->W.col[GMT_Y] > 0, "Option -Ex: Cannot specify errors in y.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_X && Ctrl->W.col[GMT_Z] > 0, "Option -Ex: Cannot specify x-y correlations\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_Y && Ctrl->W.col[GMT_X] > 0, "Option -Ey: Cannot specify errors in x.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_Y && Ctrl->W.col[GMT_Z] > 0, "Option -Ey: Cannot specify x-y correlations\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->E.mode == GMTREGRESS_Y || Ctrl->E.mode == GMTREGRESS_X) && Ctrl->W.n_weights == 2,
	                                   "Option -Ex|y: Cannot specify errors in both x and y.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.value < 0.0 || Ctrl->C.value >= 1.0, "Option -C: Level must be in 0-100%% range.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->A.active, "Option -A: Cannot simultaneously specify -T.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->F.active, "Option -A: Cannot simultaneously specify -F.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->C.active, "Option -A: Cannot simultaneously specify -C.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->F.param, "Option -Fp: Cannot simultaneously specify -C.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->F.param, "Option -Fp: Cannot simultaneously specify -T.\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2,
	                                   "Binary input data (-bi) must have at least 2 columns.\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Some of the inspiration for this module came from the following two papers:
 *
 * Hartmann, C., P. Venkeerberghen, J. Smeyers-Verbeke, and D. L. Massart (1997),
 *   Robust orthogonal regression for the outlier detection when comparing two
 *   serious of measurement results, Analytica Chimica Acta, 344, 17–28.
 * York, D., N. M. Evensen, M. L. Martínez, and J. De Basebe Delgado (2004),
 *   Unified equations for the slope, intercept, and standard errors of the
 *   best straight line, Am. J. Phys., 72(3), 367–375.
 *
 * as well as the standard book on regular and robust regression:
 *
 * Draper, N. R., and H. Smith (1998), Applied regression analysis,
 *   3rd ed., 736 pp., John Wiley and Sons, New York.
 *
 * Rousseeuw, P. J., and A. M. Leroy (1987), Robust regression and outlier
 *   detection, 329 pp., John Wiley and Sons, New York.
 */

GMT_LOCAL double model (double x, double *par) {
	/* Evaluate the model given the parameters in par */
	return (par[GMTREGRESS_SLOPE] * x + par[GMTREGRESS_ICEPT]);
}

GMT_LOCAL double gmt_sum (double *x, uint64_t n) {
	/* Return sum of values in the array x */
	uint64_t k;
	double S = 0.0;
	for (k = 0; k < n; k++) S += x[k];
	return (S);
}

GMT_LOCAL double icept_basic (struct GMT_CTRL *GMT, double *e, uint64_t n, unsigned int norm) {
	/* Return the proper "average" intercept given the chosen norm */
	unsigned int GMT_n_multiples = 0;
	double intercept = 0.0, *ee = NULL;

	if (norm != GMTREGRESS_NORM_L2) {	/* Need temporary space for scaled residuals */
		ee = gmt_M_memory (GMT, NULL, n, double);
		gmt_M_memcpy (ee, e, n, double);
		gmt_sort_array (GMT, ee, n, GMT_DOUBLE);
	}
	switch (norm) {
		case GMTREGRESS_NORM_L1:	/* Return median */
		 	intercept = (n%2) ? ee[n/2] : 0.5 * (e[(n-1)/2] + ee[n/2]);
			break;
		case GMTREGRESS_NORM_L2:	/* Return mean */
			intercept = gmt_sum (e, n) / n;
			break;
		case GMTREGRESS_NORM_LMS:	/* Return mode */
			gmt_mode (GMT, ee, n, n/2, 0, -1, &GMT_n_multiples, &intercept);
			break;
	}
	if (norm != GMTREGRESS_NORM_L2) gmt_M_free (GMT, ee);

	return (intercept);
}

GMT_LOCAL double icept_weighted (struct GMT_CTRL *GMT, double *e, double *W, uint64_t n, unsigned int norm) {
	/* Return the proper "weighted average" intercept given chosen norm */
	double intercept = 0.0;
	struct GMT_OBSERVATION *ee = NULL;

	if (norm != GMTREGRESS_NORM_L2) {	/* Need temporary space for scaled residuals */
		uint64_t k;
		ee = gmt_M_memory (GMT, NULL, n, struct GMT_OBSERVATION);
		for (k = 0; k < n; k++) {
			ee[k].weight = (gmt_grdfloat)W[k];
			ee[k].value  = (gmt_grdfloat)e[k];
		}
	}
	switch (norm) {
		case GMTREGRESS_NORM_L1:	/* Return weighted median */
			intercept = gmt_median_weighted (GMT, ee, n);
			break;
		case GMTREGRESS_NORM_L2:	/* Return weighted mean */
			intercept = gmt_mean_weighted (GMT, e, W, n);
			break;
		case GMTREGRESS_NORM_LMS:	/* Return weighted mode */
			intercept = gmt_mode_weighted (GMT, ee, n);
			break;
	}
	if (norm != GMTREGRESS_NORM_L2) gmt_M_free (GMT, ee);

	return (intercept);
}

GMT_LOCAL double intercept (struct GMT_CTRL *GMT, double *e, double *W, uint64_t n, bool weighted, unsigned int norm) {
	/* Return the weighted or unweighted intercept given chosen norm */
	double a = (weighted) ? icept_weighted (GMT, e, W, n, norm) : icept_basic (GMT, e, n, norm);
	return (a);
}

GMT_LOCAL double get_scale_factor (unsigned int regression, double slope) {
	/* Scale that turns a y-misfit into another misfit measures given regression slope */
	double f = 1.0;	/* To please gcc */
	slope = fabs (slope);
	switch (regression) {
		case GMTREGRESS_X:   f = 1.0 / slope; break;
		case GMTREGRESS_Y:   f = 1.0; break;
		case GMTREGRESS_XY:  f = sqrt (1.0 / (1.0 + slope * slope)); break;
		case GMTREGRESS_RMA: f = sqrt (1.0 / slope); break;
	}
	return (f);
}

GMT_LOCAL double L1_misfit (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, unsigned int regression, double slope) {
	/* Compute L1 misfit from y-residuals ey and weights W for regression x|y|o|r.
	 * Since W contains squared weights and we use a linear sum we take sqrt(W) below */
	uint64_t k;
	double f, E = 0.0;
	gmt_M_unused(GMT);
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) E += fabs (sqrt (W[k]) * ey[k]);
	return (f * E / (n-2));
}

GMT_LOCAL double L2_misfit (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, unsigned int regression, double slope) {
	/* Compute L2 misfit from y-residuals ey and weights W for regression x|y|o|r */
	uint64_t k;
	double f, E = 0.0;
	gmt_M_unused(GMT);
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) E += W[k] * ey[k] * ey[k];	/* Basically a chi-squared sum */
	return (f * f * E / (n-2));	/* f^2 since E was computed from squared misfits */
}

GMT_LOCAL double LMS_misfit (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, unsigned int regression, double slope) {
	/* Compute LMS misfit from y-residuals ey and weights W for regression x|y|o|r */
	uint64_t k;
	double f, E, *ee = gmt_M_memory (GMT, NULL, n, double);
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) ee[k] = W[k] * ey[k] * ey[k];
 	gmt_sort_array (GMT, ee, n, GMT_DOUBLE);
	E = (n%2) ? ee[n/2] : 0.5 * (ee[(n-1)/2] + ee[n/2]);
	gmt_M_free (GMT, ee);
	return (f * f * E);	/* f^2 since E was computed from squared misfits */
}

GMT_LOCAL double L1_scale (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, double *par) {
	/* L1 regression scale estimate is weighted median absolute residual */
	uint64_t k;
	double MAD;
	struct GMT_OBSERVATION *ee = NULL;
	gmt_M_unused(GMT); gmt_M_unused(par);
	/* Compute the (weighted) MAD */
	ee = gmt_M_memory (GMT, NULL, n, struct GMT_OBSERVATION);
	for (k = 0; k < n; k++) {
		ee[k].weight = (gmt_grdfloat)W[k];
		ee[k].value  = (gmt_grdfloat)fabs (ey[k]);
	}
	MAD = gmt_median_weighted (GMT, ee, n);
	gmt_M_free (GMT, ee);
	return (MAD);
}

GMT_LOCAL double L2_scale (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, double *par) {
	/* LS scale estimate as weighted average residual */
	double W_sum, scale;
	gmt_M_unused(GMT); gmt_M_unused(ey);
	W_sum = gmt_sum (W, n);
	scale = sqrt ((n-2)*par[GMTREGRESS_MISFT] / W_sum);	/* Undo the previous (n-2) division */
	return (scale);
}

GMT_LOCAL double LMS_scale (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, double *par) {
	/* LMS scale estimate as per Rousseuuw & Leroy [1987] */
	double scale;
	gmt_M_unused(GMT); gmt_M_unused(ey); gmt_M_unused(W);
	scale = MAD_NORMALIZE * (1.0 + 5.0 / (n - 2.0)) * sqrt (par[GMTREGRESS_MISFT]);
	return (scale);
}

GMT_LOCAL void eval_product (double *x, double *y, double *xy, uint64_t n) {
	/* Compute new array xy[i] = x[i] * y[i] */
	uint64_t k;
	for (k = 0; k < n; k++) xy[k] = x[k] * y[k];
}

GMT_LOCAL double eval_sumprod2 (double *x, double *y, uint64_t n) {
	/* Sum up the product of x * y */
	uint64_t k;
	double sum = 0.0;
	for (k = 0; k < n; k++) sum += x[k] * y[k];
	return (sum);
}

GMT_LOCAL double eval_sumprod3 (double *x, double *y, double *z, uint64_t n) {
	/* Sum up the product of x * y * z */
	uint64_t k;
	double sum = 0.0;
	for (k = 0; k < n; k++) sum += x[k] * y[k] * z[k];
	return (sum);
}

GMT_LOCAL void eval_add (double *x, double c, double *out, uint64_t n) {
	/* Compute array out[i] = x[i] + c */
	uint64_t k;
	for (k = 0; k < n; k++) out[k] = x[k] + c;
}

GMT_LOCAL void ones (double *x, uint64_t n) {
	/* Set a unitary vector */
	uint64_t k;
	for (k = 0; k < n; k++) x[k] = 1.0;
}

GMT_LOCAL void get_correlation (struct GMT_CTRL *GMT, double *X, double *Y, double *w[], uint64_t n, double *par) {
	/* standard r = s_xy / (s_x * s_y), using the weighted expressions for these terms.
	 */

	uint64_t k;
	double sx = 0.0, sy = 0.0, sxy = 0.0, wx = 1.0, wy = 1.0, swx = 0.0, swy = 0.0, swxy = 0.0;
	gmt_M_unused(GMT);

	for (k = 0; k < n; k++) {
		if (w[GMT_X]) wx = w[GMT_X][k];	/* Was given x-weights */
		if (w[GMT_Y]) wy = w[GMT_Y][k];	/* Was given y-weights */
		sx  += wx * pow (X[k] - par[GMTREGRESS_XMEAN], 2.0);
		sy  += wy * pow (Y[k] - par[GMTREGRESS_YMEAN], 2.0);
		sxy += wx * wy * (X[k] - par[GMTREGRESS_XMEAN]) * (Y[k] - par[GMTREGRESS_YMEAN]);
		swx += wx;	swy += wy;	swxy += wx * wy;
	}
	par[GMTREGRESS_CORR] = (sxy / swxy) / sqrt ((sx / swx) * (sy / swy));
}

GMT_LOCAL void get_coeffR (struct GMT_CTRL *GMT, double *X, double *Y, double *w[], uint64_t n, unsigned int regression, double *par) {
	/* Compute coefficient of determination, R ( = r^2 for LSY Pearsonian correlation).
	 * Currently only set up to do standard y on x (weights on y) only.
	 * Compute both coefficient of determination (R) and the correlation coefficient (r).
	 *   R = 1 - SSR/SST, with
	 *   SSR is the sum of squared residuals: sum (y_i - y(x_i))^2
	 *   and SST is the sum of total variance of the model : sum (y_i - mean(y))^2
	 *   We augment these with weights w_i as well, if given.
	 */

	uint64_t k;
	double SSR = 0.0, SST = 0.0, y_hat, ww = 1.0, f;
	gmt_M_unused(GMT);

	f = get_scale_factor (regression, par[GMTREGRESS_SLOPE]);
	f *= f;	/* Since working on squared misfits */
	for (k = 0; k < n; k++) {
		if (w[GMT_Y]) ww = w[GMT_Y][k];	/* Was given weights */
		y_hat = par[GMTREGRESS_SLOPE] * X[k] + par[GMTREGRESS_ICEPT];
		SSR += ww * pow (Y[k] - y_hat, 2.0);
		SST += ww * pow (Y[k] - par[GMTREGRESS_YMEAN], 2.0);
	}
	par[GMTREGRESS_R] = 1.0 - f * SSR / SST;
}

GMT_LOCAL double gmt_demeaning (struct GMT_CTRL *GMT, double *X, double *Y, double *w[], uint64_t n, double *par, double *U, double *V, double *W, double *alpha, double *beta) {
	/* Compute weighted X and Y means, return these via par, and calculate residuals U and V and weights W
	 * (and alpha, beta if orthogonal).  If orthogonal regression we expect a preliminary estimate of the
	 * slope to be present in par[GMTREGRESS_SLOPE].  Return weight sum S.  This function carries out many of
	 * the steps in York et al [2004]. */
	double S;
	gmt_M_unused(GMT);

	if (w && w[GMT_X] && w[GMT_Y]) {	/* Orthogonal regression with x/y weights [and optionally x-y correlations] requested */
		double corr_i = 0.0, alpha_i, w_xy;
		uint64_t i;
		/*  Compute Wi from w(X_i), w(Y_i), r_i and best estimate of slope placed in par[GMTREGRESS_SLOPE]. */
		for (i = 0; i < n; i++) {
			w_xy = w[GMT_X][i] * w[GMT_Y][i];
			alpha_i = sqrt (w_xy);
			if (w[GMT_Z]) corr_i = w[GMT_Z][i];
			W[i] = (w_xy > 0.0) ? w_xy / (w[GMT_X][i] + par[GMTREGRESS_SLOPE] * par[GMTREGRESS_SLOPE] * w[GMT_Y][i] - 2 * par[GMTREGRESS_SLOPE] * corr_i * alpha_i) : 0.0;
			if (alpha) alpha[i] = alpha_i;
		}
		/*  Step 4: Compute weighted X_mean, Y_mean, then U, V, and beta */
		S = gmt_sum (W, n);					/* Get sum of weights */
		par[GMTREGRESS_XMEAN] = eval_sumprod2 (W, X, n) / S;	/* Compute weighted X_mean */
		par[GMTREGRESS_YMEAN] = eval_sumprod2 (W, Y, n) / S;	/* Compute weighted Y_mean */
		eval_add (X, -par[GMTREGRESS_XMEAN], U, n);		/* Compute U */
		eval_add (Y, -par[GMTREGRESS_YMEAN], V, n);		/* Compute V */
		if (beta && alpha) {	/* Compute beta (as alpha above) which is needed for weighted orthogonal regression */
			for (i = 0; i < n; i++) {
				if (w[GMT_Z]) corr_i = w[GMT_Z][i];
				beta[i] = W[i] * (U[i] / w[GMT_Y][i] + par[GMTREGRESS_SLOPE] * V[i] / w[GMT_X][i] - (par[GMTREGRESS_SLOPE] * U[i] + V[i]) * corr_i / alpha[i]);
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Computed single weights from separate x- and y-weights %s\n",
			(w[GMT_Z]) ? " and x-y correlations" : "only");
	}
	else if (w && (w[GMT_X] || w[GMT_Y])) {	/* Not orthogonal regression, and have weights in x or y */
		double *pW = (w[GMT_X]) ? w[GMT_X] : w[GMT_Y];	/* Shorthand for the (squared) weights */
		gmt_M_memcpy (W, pW, n, double);			/* Duplicate the chosen weight array to W */
		S = gmt_sum (W, n);					/* Get sum of weights */
		par[GMTREGRESS_XMEAN] = eval_sumprod2 (W, X, n) / S;	/* Compute weighted X_mean */
		par[GMTREGRESS_YMEAN] = eval_sumprod2 (W, Y, n) / S;	/* Compute weighted Y_mean */
		eval_add (X, -par[GMTREGRESS_XMEAN], U, n);		/* Compute U */
		eval_add (Y, -par[GMTREGRESS_YMEAN], V, n);		/* Compute V */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Computed weights from given %c-weights\n", (w[GMT_X]) ? 'x' : 'y');
	}
	else {	/* No weights, create unit array */
		ones (W, n);				/* Unit weights */
		par[GMTREGRESS_XMEAN] = gmt_sum (X, n) / n;	/* Compute X_mean */
		par[GMTREGRESS_YMEAN] = gmt_sum (Y, n) / n;	/* Compute X_mean */
		eval_add (X, -par[GMTREGRESS_XMEAN], U, n);	/* Compute U */
		eval_add (Y, -par[GMTREGRESS_YMEAN], V, n);	/* Compute V */
		S = (double)n;					/* Trivial sum of weights */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Computed unit weights in the absence of actual weights\n");
	}
	return (S);	/* Returning the weight sum */
}

GMT_LOCAL double LSy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par) {
	/* Basic LS y-regression on x, only uses w[GMT_Y] weights if not NULL */
	uint64_t k;
	double *Q = gmt_M_memory (GMT, NULL, n, double), *W = gmt_M_memory (GMT, NULL, n, double);
	double *U = gmt_M_memory (GMT, NULL, n, double), *V = gmt_M_memory (GMT, NULL, n, double);
	double S, S_xx, S_xy, D, scale;

	gmt_M_memset (par, GMTREGRESS_NPAR, double);	/* Reset all regression parameters */
	S = gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);	/* alpha, beta not used here so passing NULL */

	/* Because we operate on U and V, the terms S_x = S_y == 0 and are thus ignored in the equations below */
	if (w && w[GMT_Y]) {	/* Weighted regression */
		double *P = gmt_M_memory (GMT, NULL, n, double);
		eval_product (U, W, P, n);	/* Form P[i] = W[i] * U[i] */
		eval_product (P, U, Q, n);	/* Form Q[i] = P[i] * U[i] = W[i] * U[i] * U[i] */
		S_xx = gmt_sum (Q, n);		/* The weighted sum of U^2 */
		eval_product (P, V, Q, n);	/* Form Q[i] = P[i] * V[i] = W[i] * U[i] * V[i] */
		S_xy = gmt_sum (Q, n);		/* The weighted sum of U*V */
		gmt_M_free (GMT, P);
	}
	else {	/* No weights supplied */
		eval_product (U, U, Q, n);	/* Form Q[i] = U[i] * U[i] */
		S_xx = gmt_sum (Q, n);		/* The sum of U^2 */
		eval_product (U, V, Q, n);	/* Form Q[i] = U[i] * V[i] */
		S_xy = gmt_sum (Q, n);		/* The sum of U*V */
	}
	D = 1.0 / (S * S_xx);
	par[GMTREGRESS_SLOPE] = (S * S_xy) * D;
 	par[GMTREGRESS_ICEPT] = par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN];
 	par[GMTREGRESS_SIGSL] = sqrt (S * D);
 	par[GMTREGRESS_SIGIC] = sqrt (S_xx * D);
	for (k = 0; k < n; k++)	/* Here we recycle Q to hold y-residual e */
		Q[k] = y[k] - model (x[k], par);
	par[GMTREGRESS_MISFT] = L2_misfit (GMT, Q, W, n, GMTREGRESS_Y, 0.0);
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	scale = L2_scale (GMT, NULL, W, n, par);
	gmt_M_free (GMT, Q);
	gmt_M_free (GMT, U);
	gmt_M_free (GMT, V);
	gmt_M_free (GMT, W);
	return (scale);
}

GMT_LOCAL double LSxy_regress1D_basic (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *par) {
	/* Basic LS xy orthogonal regression, with no data errors. See York [1966] */
	uint64_t k;
	unsigned int p;
	double *u = gmt_M_memory (GMT, NULL, n, double), *v = gmt_M_memory (GMT, NULL, n, double);
	double *W = gmt_M_memory (GMT, NULL, n, double), *Q = gmt_M_memory (GMT, NULL, n, double);
	double mean_x, mean_y, sig_x, sig_y, sum_u2, sum_v2, sum_uv, part1, part2, r, scale, a[2], b[2], E[2];

	mean_x = gmt_mean_and_std (GMT, x, n, &sig_x);
	mean_y = gmt_mean_and_std (GMT, y, n, &sig_y);
	/* Normalize the data */
	eval_add (x, -mean_x, u, n);	/* Get reduced x-coordinates u */
	eval_add (y, -mean_y, v, n);	/* Get reduced y-coordinates v */
	eval_product (u, u, Q, n);	/* Compute Q[i] = u[i] * u[i] */
	sum_u2 = gmt_sum (Q, n);	/* Get sum of u*u */
	eval_product (v, v, Q, n);	/* Compute Q[i] = v[i] * v[i] */
	sum_v2 = gmt_sum (Q, n);	/* Get sum of v*v */
	eval_product (u, v, Q, n);	/* Compute Q[i] = u[i] * v[i] */
	sum_uv = gmt_sum (Q, n);	/* Get sum of u*v */
	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);	/* Done with these arrays */
	part1 = sum_v2 - sum_u2;
	part2 = sqrt (pow (sum_u2 - sum_v2, 2.0) + 4.0 * sum_uv * sum_uv);
	b[0] = (part1 + part2) / (2.0 * sum_uv);
	b[1] = (part1 - part2) / (2.0 * sum_uv);
	r = sum_uv / sqrt (sum_u2 * sum_v2);
	ones (W, n);			/* Unit weights */
	for (p = 0; p < 2; p++) {	/* Compute E from vertical y-residuals for both solutions to the slope */
		a[p] = mean_y - b[p] * mean_x;	/* Trial intercept */
		for (k = 0; k < n; k++) Q[k] = y[k] - b[p] * x[k] - a[p];
		E[p] = L2_misfit (GMT, Q, W, n, GMTREGRESS_XY, b[p]);
	}
	p = (E[0] < E[1]) ? 0 : 1;	/* Determine the solution with the smallest misfit and copy to par array: */
	par[GMTREGRESS_SLOPE] = b[p];
	par[GMTREGRESS_ICEPT] = a[p];
	par[GMTREGRESS_SIGSL] = par[GMTREGRESS_SLOPE] * sqrt ((1.0 - r * r) / n) / r;
	par[GMTREGRESS_SIGIC] = sqrt (pow (sig_y - sig_x * par[GMTREGRESS_SLOPE], 2.0) / n + (1.0 - r) * par[GMTREGRESS_SLOPE] * (2.0 * sig_x * sig_y + (mean_x * par[GMTREGRESS_SLOPE] * (1.0 + r) / (r * r))));
	par[GMTREGRESS_MISFT] = E[p];
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	par[GMTREGRESS_XMEAN] = mean_x;
	par[GMTREGRESS_YMEAN] = mean_y;
	scale = L2_scale (GMT, NULL, W, n, par);
	gmt_M_free (GMT, Q);
	gmt_M_free (GMT, W);

	return (scale);
}

GMT_LOCAL double LSRMA_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par) {
	/* Basic LS RMA orthogonal regression with no weights [Reference?] */
	uint64_t k;
	double sx, sy, scale;
	double *U = gmt_M_memory (GMT, NULL, n, double), *V = gmt_M_memory (GMT, NULL, n, double), *W = gmt_M_memory (GMT, NULL, n, double);
	gmt_M_memset (par, GMTREGRESS_NPAR, double);
	(void)gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);
	sx = gmt_std_weighted (GMT, U, w[GMT_X], 0.0, n);
	sy = gmt_std_weighted (GMT, V, w[GMT_Y], 0.0, n);
	par[GMTREGRESS_SLOPE] = sy / sx;
	par[GMTREGRESS_ICEPT] = par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN];
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	for (k = 0; k < n; k++)	/* Here we recycle U as y-residual e */
		U[k] = y[k] - model (x[k], par);
	par[GMTREGRESS_MISFT] = L2_misfit (GMT, U, W, n, GMTREGRESS_RMA, par[GMTREGRESS_SLOPE]);
	scale = L2_scale (GMT, NULL, W, n, par);
	gmt_M_free (GMT, U);
	gmt_M_free (GMT, V);
	gmt_M_free (GMT, W);
	return (scale);
}

GMT_LOCAL void regress1D_sub (struct GMT_CTRL *GMT, double *x, double *y, double *W, double *e, uint64_t n, unsigned int regression, unsigned int norm, bool weighted, double angle, double *par) {
	/* Solve the linear regression problem for a given slope angle and chosen misfit and norm to give a unique intercept */
	/* x, y here are actually the reduced coordinates U, V */
	uint64_t k;
	double a, b, E;
	double (*misfit) (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, unsigned int regression, double slope);
	switch (norm) {	/* Set misfit function pointer */
		case GMTREGRESS_NORM_L1:  misfit = L1_misfit;  break;
		case GMTREGRESS_NORM_L2:  misfit = L2_misfit;  break;
		case GMTREGRESS_NORM_LMS: misfit = LMS_misfit; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Misfit norm not specified? - set to L2\n");
			misfit = L2_misfit;
			break;
	}
	if (gmt_M_is_zero (fabs (angle) - 90.0)) {	/* Vertical line is a special case since slope is infinity */
		b = GMT->session.d_NaN;				/* Slope is undefined */
		a = intercept (GMT, x, W, n, weighted, norm);	/* Determine best x-intercept */
		for (k = 0; k < n; k++) e[k] = x[k] - a;	/* Final x-residuals */
		/* For GMTREGRESS_Y|GMTREGRESS_RMA a vertical line gives Inf misfit; the others are measured horizontally so always finite.
		 * We obtain E by passing e as ex but giving the mode Gas MTREGRESS_Y instead and pass 0 as slope. */
		E = (regression == GMTREGRESS_Y || regression == GMTREGRESS_RMA) ? DBL_MAX : misfit (GMT, e, W, n, GMTREGRESS_Y, 0.0);
	}
	else if (gmt_M_is_zero (angle)) {	/* Horizontal line is also a special case since X and RMA regressions give infinite misfits */
		b = 0.0;	/* Slope is straightforward */
		a = intercept (GMT, y, W, n, weighted, norm);	/* Determine best y-intercept */
		/* For GMTREGRESS_X|GMTREGRESS_RMA a horizontal line gives Inf misfit; the others are measured vertically so always finite.
		 * We obtain E by passing e as ey but giving mode GMTREGRESS_Y instead and pass 0 as slope. */
		for (k = 0; k < n; k++) e[k] = y[k] - a;	/* Final y-residuals */
		E = (regression == GMTREGRESS_X || regression == GMTREGRESS_RMA) ? DBL_MAX : misfit (GMT, e, W, n, GMTREGRESS_Y, 0.0);
	}
	else {	/* Neither vertical|horizontal, we can measure any misfit and need to pass the slope b */
		b = tand (angle);				/* Regression slope */
		for (k = 0; k < n; k++) e[k] = y[k] - b * x[k];	/* The y-residuals after removing sloping trend */
		a = intercept (GMT, e, W, n, weighted, norm);	/* Determine best y-intercept */
		for (k = 0; k < n; k++) e[k] -= a;		/* Final y-residuals */
		E = misfit (GMT, e, W, n, regression, b);	/* The representative misfit */
	}
	if (gmt_M_is_dnan (E)) E = DBL_MAX;	/* If anything goes crazy, set E to huge, but this should not happen */
	/* Update the new best solution; we do not change entries for U and W as well as the sigmas for slope and intercept */
	par[GMTREGRESS_ANGLE] = angle;	par[GMTREGRESS_MISFT] = E; par[GMTREGRESS_SLOPE] = b;	par[GMTREGRESS_ICEPT] = a;
}

#define N_ANGLE_SELECTIONS	90	/* Fixed number of slope angles to try between min/max slope limits */

GMT_LOCAL double regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, unsigned int regression, unsigned int norm, double *par) {
	/* Solve the linear regression problem for chosen misfit and norm by an iterative approach */
	uint64_t k;
	unsigned int n_iter = 0;
	bool done = false, weighted = false;
	char buffer[GMT_LEN256] = {""};
	double a_min = -90.0, a_max = 90.0, angle, r_a, d_a, f, last_E = DBL_MAX, scale, tpar[GMTREGRESS_NPAR];
	double *U = gmt_M_memory (GMT, NULL, n, double), *V = gmt_M_memory (GMT, NULL, n, double);
	double *W = gmt_M_memory (GMT, NULL, n, double), *e = gmt_M_memory (GMT, NULL, n, double);
	double (*scl_func) (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, double *par);

	switch (norm) {	/* Set regression residual scale function pointer */
		case GMTREGRESS_NORM_L1:  scl_func = L1_scale;  break;
		case GMTREGRESS_NORM_L2:  scl_func = L2_scale;  break;
		case GMTREGRESS_NORM_LMS: scl_func = LMS_scale; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Misfit norm not specified? - set to L2\n");
			scl_func = L2_scale;
			break;
	}

	gmt_M_memset (par,  GMTREGRESS_NPAR, double);	/* Reset all regression parameters */
	gmt_M_memset (tpar, GMTREGRESS_NPAR, double);	/* Reset all test regression parameters */
	if (regression != GMTREGRESS_XY) (void)gmt_demeaning (GMT, x, y, w, n, tpar, U, V, W, NULL, NULL);	/* Do this once except for orthogonal */
	par[GMTREGRESS_MISFT] = DBL_MAX;	/* Initially we have no fit */
	weighted = (regression == GMTREGRESS_X) ? (w && w[GMT_X]) : (w && w[GMT_Y]);	/* true if weights were provided */
	while (!done) {	/* Keep iterating and zooming in on smaller angle-ranges until misfit is very small */
		r_a = a_max - a_min;	/* Range of angles */
		d_a = r_a / (double)N_ANGLE_SELECTIONS;	/* Angle increment */
		for (k = 0; k <= N_ANGLE_SELECTIONS; k++) {	/* Try all slopes in current angle range */
			angle = a_min + d_a * k;		/* This is the current slope angle */
			if (regression == GMTREGRESS_XY) {	/* Since W depends on slope we must recompute W each time in this loop */
				tpar[GMTREGRESS_SLOPE] = tand (angle);
				(void)gmt_demeaning (GMT, x, y, w, n, tpar, U, V, W, NULL, NULL);
			}
			regress1D_sub (GMT, U, V, W, e, n, regression, norm, weighted, angle, tpar);	/* Solve for best intercept given this slope */
			if (tpar[GMTREGRESS_MISFT] < par[GMTREGRESS_MISFT])
				gmt_M_memcpy (par, tpar, GMTREGRESS_NPAR, double);	/* Update best fit so far without stepping on the means and sigmas */
		}
		if (par[GMTREGRESS_MISFT] <= last_E && (f = (last_E - par[GMTREGRESS_MISFT])/par[GMTREGRESS_MISFT]) < GMT_CONV15_LIMIT)
			done = true;	/* Change is tiny so we are done */
		else {	/* Gradually zoom in on the angles with smallest misfit but allow some slack */
			a_min = MAX (-90.0, par[GMTREGRESS_ANGLE] - 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			a_max = MIN (+90.0, par[GMTREGRESS_ANGLE] + 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			last_E = par[GMTREGRESS_MISFT];
		}
		/* Adjust intercept from U,V -> (x,y) */
		n_iter++;
		if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) {
			double a = par[GMTREGRESS_ICEPT] + (par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN]);
			snprintf (buffer, GMT_LEN256, "Robust iteration %u: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: --N/A-- sig_icept: --N/A--",
				n_iter, n, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN], par[GMTREGRESS_ANGLE], par[GMTREGRESS_MISFT], par[GMTREGRESS_SLOPE], a);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s\n", buffer);
		}
	}
	par[GMTREGRESS_ICEPT] += (par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN]);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Robust regression algorithm convergence required %d iterations\n", n_iter);
	scale = scl_func (GMT, e, W, n, par);	/* Get final regression-scale for residuals */

	gmt_M_free (GMT, U);
	gmt_M_free (GMT, V);
	gmt_M_free (GMT, W);
	gmt_M_free (GMT, e);

	return (scale);
}

#define GMTREGRESS_MAX_YORK_ITERATIONS	1000	/* Gotta have a stopper in case of bad data? */

GMT_LOCAL double LSxy_regress1D_york (struct GMT_CTRL *GMT, double *X, double *Y, double *w[], uint64_t n, double *par) {
	/* Solution to general LS orthogonal regression with weights, per York et al. [2004] */
	uint64_t i;
	unsigned int n_iter = 0;
	double *W = NULL, *U = NULL, *V = NULL, *x = NULL, *u = NULL, *alpha = NULL, *beta = NULL;
	double b, b_old, a, W_sum, x_mean, sigma_a, sigma_b, misfit, scale;
	char buffer[GMT_LEN256] = {""};

	/* Allocate temporary vectors */
	W = gmt_M_memory (GMT, NULL, n, double);
	U = gmt_M_memory (GMT, NULL, n, double);
	V = gmt_M_memory (GMT, NULL, n, double);
	x = gmt_M_memory (GMT, NULL, n, double);
	u = gmt_M_memory (GMT, NULL, n, double);
	alpha = gmt_M_memory (GMT, NULL, n, double);
	beta  = gmt_M_memory (GMT, NULL, n, double);
	/* Step 1: Get initial slope from basic LS y on x with no weights (and ignore scale on return) */
	(void)LSy_regress1D (GMT, X, Y, NULL, n, par);
	b = par[GMTREGRESS_SLOPE];	/* This is our initial slope value */
	gmt_M_memset (par, GMTREGRESS_NPAR, double);	/* Reset all regression parameters */
	/* Step 2: Weights w(X_i) and w(Y_i) are already set in the main program */
	do {	/* Keep iterating until converged [Step 6 in York et al, 2004 recipe] */
		b_old = b;	/* Previous best slope */
		/*  Step 3+4: Compute single weights Wi from w(X_i), w(Y_i), r_i and weighted X_mean, Y_mean, then U, V, alpha, and beta */
		par[GMTREGRESS_SLOPE] = b_old;	/* Pass in previous best-fitting slope needed to update W */
		W_sum = gmt_demeaning (GMT, X, Y, w, n, par, U, V, W, alpha, beta);	/* Sets the above variables */
		/*  Step 5: Compute an improved estimate of the slope b */
		b = eval_sumprod3 (W, beta, V, n) / eval_sumprod3 (W, beta, U, n);
		/* Step 7: Calculate the corresponding intercept a (which is zero in U-V coordinates so we convert to X-Y) */
		a = par[GMTREGRESS_YMEAN] - b * par[GMTREGRESS_XMEAN];
		/* Step 8: Compute the adjusted points x (x,y) are the orthogonal projection of (X,Y) onto the regression line */
		eval_add (beta, par[GMTREGRESS_XMEAN], x, n);	/* Compute x (we don't actually need y so we don't do that here) */
		/* Step 9: Compute u */
		x_mean = eval_sumprod2 (W, x, n) / W_sum;	/* Compute x_mean */
		eval_add (x, -x_mean, u, n);			/* Compute u */
		/* Step 10: Compute sigma_b and sigma_a */
		sigma_b = 1.0 / eval_sumprod3 (W, u, u, n);	/* Actually sigma_b^2 since we need that quantity first */
		sigma_a = sqrt (1.0 / W_sum + x_mean * x_mean * sigma_b);
		sigma_b = sqrt (sigma_b);			/* Now it is sigma_b */
		/* Estimate weighted residuals (recycling V for holding the y-residuals) */
		for (i = 0; i < n; i++) V[i] = Y[i] - (a + b * X[i]);
		misfit = L2_misfit (GMT, V, W, n, GMTREGRESS_XY, 0.0);	/* Get misfit from residuals */
		n_iter++;
		snprintf (buffer, GMT_LEN256, "York iteration %u: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: %g sig_icept: %g",
			n_iter, n, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN], atand (b), misfit, b, a, sigma_b, sigma_a);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s\n", buffer);
	} while (fabs (b - b_old) > GMT_CONV15_LIMIT && n_iter < GMTREGRESS_MAX_YORK_ITERATIONS);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "York orthogonal algorithm convergence required %d iterations\n", n_iter);
	/* Pass out the final result via the par array */
	par[GMTREGRESS_SLOPE] = b;
	par[GMTREGRESS_ICEPT] = a;
	par[GMTREGRESS_SIGSL] = sigma_b;
	par[GMTREGRESS_SIGIC] = sigma_a;
	par[GMTREGRESS_MISFT] = misfit;
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	scale = L2_scale (GMT, NULL, W, n, par);	/* Get the regression residual scale */

	/* Free temporary arrays */
	gmt_M_free (GMT, W);
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, u);
	gmt_M_free (GMT, U);
	gmt_M_free (GMT, V);
	gmt_M_free (GMT, alpha);
	gmt_M_free (GMT, beta);

	return (scale);
}

GMT_LOCAL double LSxy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par) {
	/* Front to calling LSxy_regress1D_york or LSxy_regress1D_basic, depending on weights */
	double scale;
	if (w && w[GMT_X] && w[GMT_Y])	/* Have weights in x and y [and possibly correlation coefficients as well] */
		scale = LSxy_regress1D_york (GMT, x, y, w, n, par);
	else	/* Simpler case with no weights */
		scale = LSxy_regress1D_basic (GMT, x, y, n, par);
	return (scale);
}

GMT_LOCAL double *do_regression (struct GMT_CTRL *GMT, double *x_in, double *y_in, double *w[], uint64_t n, unsigned int regression, unsigned int in_norm, double *par, unsigned int mode) {
	/* Solves for the best regression of (x_in, y_in) given the current settings.
	 * mode is only 1 when called to do RLS after the initial LMS regression returns. */

	uint64_t k;
	unsigned int norm = in_norm;
	bool flipped, reweighted_ls = false;
	double scale = 1.0, *x = NULL, *y = NULL, *z = NULL, *ww[3] = {NULL, NULL, NULL};

	if (in_norm == GMTREGRESS_NORM_RLS) {	/* Reweighted Least Squares means first LMS, then remove outliers, then L2 for final result */
		norm = GMTREGRESS_NORM_LMS;
		reweighted_ls = true;
	}
	if (regression == GMTREGRESS_X) {	/* Do x on y regression by flipping the input, then transpose par before output */
		x = y_in;	y = x_in;
		regression = GMTREGRESS_Y;
		flipped = true;
		ww[GMT_X] = w[GMT_Y];	/* Must also flip the order of the weights (w[GMT_Z] not used) */
		ww[GMT_Y] = w[GMT_X];
	}
	else {	/* Normal situation: x is x and y is y */
		x = x_in;	y = y_in;
		flipped = false;
		ww[GMT_X] = w[GMT_X];
		ww[GMT_Y] = w[GMT_Y];
	}

	switch (regression) {	/* Different actions depending on what kind of regression we seek */
		case GMTREGRESS_Y:	/* Vertical misfit measure */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					scale = regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 regression y on x has an analytic solution */
					scale = LSy_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
		case GMTREGRESS_XY:	/* Orthogonal regression */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					scale = regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 orthogonal regression has an analytic (iterative if weighted) solution */
					LSxy_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
		case GMTREGRESS_RMA:	/* RMA regression */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					scale = regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 RMA regression has analytic solution */
					scale = LSRMA_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
	}
	assert (scale != 0.0);	/* This would be failure */
	if (flipped) {	/* Must transpose back the results */
		/* We solved x = a' + b' * y but we wanted y = a + b * x.
		 * Basic algebra shows a = -a' / b' and b = 1/b'.
		 * Likewise, da must be da' / b' which db remains the same.
		 * Also swap the locations of x/y means, but leave misfit and sigmas as is. */
		par[GMTREGRESS_ICEPT] = -par[GMTREGRESS_ICEPT] / par[GMTREGRESS_SLOPE];
		par[GMTREGRESS_SLOPE] = 1.0 / par[GMTREGRESS_SLOPE];
		par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
		gmt_M_double_swap (par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN]);
		par[GMTREGRESS_SIGIC] = par[GMTREGRESS_SIGIC] / par[GMTREGRESS_SLOPE];
	}
	if (mode == 0) {	/* Normal mode is to compute normalized residuals (aka z-scores) */
		double e_k;
		z = gmt_M_memory (GMT, NULL, n, double);	/* Array with z-scores */
		for (k = 0; k < n; k++) {
			e_k = y[k] - model (x[k], par);	/* Get y-residual */
			z[k] = e_k / scale;		/* Compute z-scores */
		}
	}
	if (reweighted_ls) {	/* Must identify outliers, give those points zero weight and redo the regression, but pass back the initial RLS z-scores */
		unsigned int col, first_col;
		bool made[2] = {false, false};
		double w_k, *www[3] = {NULL, NULL, NULL};
		/* If there are no weights then we must make unitary weights so we can change some weights to zero */
		www[GMT_Z] = ww[GMT_Z];	/* Pass correlations as is, present or not */
		first_col = (regression == GMTREGRESS_Y) ? GMT_Y : GMT_X;	/* Y-regression has errors in y, ortho may have x,y, weights for x-regression was flipped to y-regression */
		for (col = first_col; col <= GMT_Y; col++) {	/* Loop over possible error columns */
 			if (ww[col])	/* Have existing weights so we use those */
				www[col] = ww[col];
			else {	/* Must make unitary weights so we have something to change below */
				www[col] = gmt_M_memory (GMT, NULL, n, double);
				ones (www[col], n);
				made[col] = true;	/* So we know to free these arrays later */
			}
		}
		for (k = 0; k < n; k++) {	/* Modify weights based on z-score threshold (correlations are not modified) */
			w_k = (mode == 0 && fabs (z[k]) < GMTREGRESS_ZSCORE_LIMIT) ? 1.0 : 0.0;
			if (www[GMT_X]) www[GMT_X][k] *= w_k;
			if (www[GMT_Y]) www[GMT_Y][k] *= w_k;
		}
		(void) do_regression (GMT, x_in, y_in, www, n, regression, GMTREGRESS_NORM_L2, par, 1);
		for (col = first_col; col <= GMT_Y; col++)	/* Free any arrays we allocated */
			if (made[col]) gmt_M_free (GMT, www[col]);
	}
	get_correlation (GMT, x_in, y_in, w, n, par);	/* Evaluate r */
	get_coeffR (GMT, x_in, y_in, w, n, regression, par);	/* Evaluate R */

	return (z);	/* Return those z-scores, calling unit must free this array when done */
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtregress (void *V_API, int mode, void *args) {
	uint64_t k, seg, tbl, col = 0, row, n_try = 0, n_t, n_alloc = 0, n_columns = GMTREGRESS_N_FARGS;

	int error = 0;

	unsigned geometry = GMT_IS_NONE;

	double *x = NULL, *U = NULL, *V = NULL, *W = NULL, *e = NULL, *w[3] = {NULL, NULL, NULL};
	double t_scale = 0.0, par[GMTREGRESS_NPAR], out[9];

	char buffer[GMT_LEN256];

	struct GMT_DATASET *Din = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sa = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMTREGRESS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 1, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtregress main code ----------------------------*/

	if (Ctrl->A.active) {	/* Explore E vs slope only; no best-fit solution is returned */
		n_try = lrint ((Ctrl->A.max - Ctrl->A.min) / Ctrl->A.inc) + 1;	/* Number of angles to explore */
		n_columns = GMTREGRESS_NPAR_MAIN;	/* Hardwired to return angle, misfit, slope, intercept */
		/* Allocate fixed temp space hold the result of the experiment */
		Sa = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, n_try, n_columns, NULL, NULL);
	}
	else {	/* Work up best regression solution per input segment */
		if (Ctrl->F.param)
			n_columns = 9;
		else {
			if (!Ctrl->F.active) {	/* Default output includes all possible columns */
				char *s = GMTREGRESS_FARGS;
				Ctrl->F.n_cols = GMTREGRESS_N_FARGS;
				for (col = 0; col < n_columns; col++) Ctrl->F.col[col] = s[col];
				Ctrl->F.band = true;
			}
			n_columns = Ctrl->F.n_cols;
		}
	}

	/* Allocate memory and read in all the files; each file can have many records */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		Return (API->error);	/* Establishes data files or stdin */
	}
	if ((error = GMT_Set_Columns (API, GMT_IN, 2 + Ctrl->W.n_weights, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < (2 + Ctrl->W.n_weights)) {
		GMT_Report (API, GMT_MSG_ERROR, "Dataset only has %" PRIu64 " columns but %d are required by your settings!\n", Din->n_columns, 2 + Ctrl->W.n_weights);
		Return (GMT_RUNTIME_ERROR);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, geometry) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_columns, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}

	if (Ctrl->T.active) {	/* Evaluate solution for equidistant spacing instead of at the input locations */
		unsigned int bad = 0;	/* Must check for conflict between -T and -F settings */
		for (col = 0; col < n_columns; col++) if (!Ctrl->T.no_eval && strchr ("yrzw", (int)Ctrl->F.col[col])) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -F: Cannot include %c when -T is in effect\n", (int)Ctrl->F.col[col]);
			bad++;
		}
		if (bad) Return (GMT_RUNTIME_ERROR);
		if (Ctrl->T.T.set == 3 && gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL))
			Return (GMT_RUNTIME_ERROR);
		geometry = GMT_IS_LINE;
	}

	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* To write segment headers regardless of input */
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	/* Process all tables and their segments */
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current data segment */

			/* Make sure we have enough memory allocated for weight arrays and such */
			if (S->n_rows > n_alloc) {	/* Need more memory (first time we have nothing, then only allocate more when larger segments are encountered) */
				n_alloc = S->n_rows;	/* New allocation limit */
				for (k = GMT_X; k <= GMT_Z; k++)	/* Allocate a temporary array for each weight column in the input */
					if (Ctrl->W.col[k]) w[k] = gmt_M_memory (GMT, w[k], n_alloc, double);
				if (Ctrl->A.active) {	/* Additional arrays are needed for the slope-scanning experiment */
					U = gmt_M_memory (GMT, U, n_alloc, double);
					V = gmt_M_memory (GMT, V, n_alloc, double);
					W = gmt_M_memory (GMT, W, n_alloc, double);
					e = gmt_M_memory (GMT, e, n_alloc, double);
				}
			}

			/* Deal with input columns containing sigmas and correlations, if present, and convert to weights (unless -Ww) */
			for (k = GMT_X; k <= GMT_Z; k++) {	/* Check in turn for sigma-x, sigma-y, r columns */
				if (!Ctrl->W.col[k]) continue;	/* No such column was provided */
				if (Ctrl->W.type || k == GMT_Z)	/* Got weights or dealing with the correlation column, so just copy the contents */
					gmt_M_memcpy (w[k], S->data[Ctrl->W.col[k]], S->n_rows, double);
				else	/* Got sigma; convert to weights = 1/sigma */
					for (row = 0; row < S->n_rows; row++) w[k][row] = 1.0 / S->data[Ctrl->W.col[k]][row];
				/* Square the weights here, except for correlations (if present) */
				if (k < GMT_Z) for (row = 0; row < S->n_rows; row++) w[k][row] *= w[k][row];
				col++;	/* Go to next potential input column */
			}

			if (Ctrl->A.active) {	/* Explore E vs slope only - no final best regression is returned */
				uint64_t min_row = 0;
				double angle, min_E = DBL_MAX;
				bool weighted = (Ctrl->E.mode == GMTREGRESS_X) ? (w[GMT_X]) : (w[GMT_Y]);	/* true if these pointers are not NULL */

				/* Determine x/y means, compute reduced coordinates U,V and return proper weights W once, unless orthogonal regression was selected */
				if (Ctrl->E.mode != GMTREGRESS_XY) (void)gmt_demeaning (GMT, S->data[GMT_X], S->data[GMT_Y], w, S->n_rows, par, U, V, W, NULL, NULL);
				for (row = 0; row < n_try; row++) {	/* For each new slope candidate */
					angle = Ctrl->A.min + row * Ctrl->A.inc;	/* Current slope in degrees */
					if (Ctrl->E.mode == GMTREGRESS_XY) {	/* Since W depends on slope when doing orthogonal regression we must recompute W for each slope */
						par[GMTREGRESS_SLOPE] = tand (angle);
						(void)gmt_demeaning (GMT, S->data[GMT_X], S->data[GMT_Y], w, S->n_rows, par, U, V, W, NULL, NULL);
					}
					regress1D_sub (GMT, U, V, W, e, S->n_rows, Ctrl->E.mode, Ctrl->N.mode, weighted, angle, par);	/* Solve for best intercept given this slope */
					if (par[GMTREGRESS_MISFT] < min_E) {	/* Update best fit so far */
						min_E = par[GMTREGRESS_MISFT];
						min_row = row;
					}
					for (k = 0; k < GMTREGRESS_NPAR_MAIN; k++) Sa->data[k][row] = par[k];	/* Save the results into this row */
				}
				/* Make segment header with the findings for the overall best regression */
				snprintf (buffer, GMT_LEN256, "Best regression: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: --N/A-- sig_icept: --N/A--", S->n_rows, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN], Sa->data[0][min_row],
					Sa->data[1][min_row], Sa->data[2][min_row], Sa->data[3][min_row]);
				GMT_Report (API, GMT_MSG_INFORMATION, "%s\n", buffer);	/* Report results if verbose */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);	/* Also include result in segment header */
				for (row = 0; row < n_try; row++) {	/* Write the saved results of the experiment */
					for (k = 0; k < GMTREGRESS_NPAR_MAIN; k++) out[k] = Sa->data[k][row];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this record to output */
				}
			}
			else {	/* Here we are solving for the best regression */
				bool outlier = false;
				double *z_score = do_regression (GMT, S->data[GMT_X], S->data[GMT_Y], w, S->n_rows, Ctrl->E.mode, Ctrl->N.mode, par, 0);	/* The heavy work happens here */
				if (Ctrl->F.param) {	/* Just print the model parameters */
					out[0] = (double)S->n_rows;
					out[1] = par[GMTREGRESS_XMEAN];
					out[2] = par[GMTREGRESS_YMEAN];
					out[3] = par[GMTREGRESS_ANGLE];
					out[4] = par[GMTREGRESS_MISFT];
					out[5] = par[GMTREGRESS_SLOPE];
					out[6] = par[GMTREGRESS_ICEPT];
					out[7] = par[GMTREGRESS_SIGSL];
					out[8] = par[GMTREGRESS_SIGIC];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this record to output */
				}
				else {
					/* Make segment header with the findings for best regression */
					snprintf (buffer, GMT_LEN256, "Best regression: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: %g sig_icept: %g corr: %g R: %g", S->n_rows, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN],
						par[GMTREGRESS_ANGLE], par[GMTREGRESS_MISFT], par[GMTREGRESS_SLOPE], par[GMTREGRESS_ICEPT], par[GMTREGRESS_SIGSL], par[GMTREGRESS_SIGIC], par[GMTREGRESS_CORR], par[GMTREGRESS_R]);
					GMT_Report (API, GMT_MSG_INFORMATION, "%s\n", buffer);	/* Report results if verbose */
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);	/* Also include in segment header */

					if (Ctrl->F.band)	/* For confidence band we need the student-T scale given the significance level and degrees of freedom */
						t_scale = fabs (gmt_tcrit (GMT, 0.5 * (1.0 - Ctrl->C.value), S->n_rows - 2.0));

					if (Ctrl->T.no_eval)	/* No model evaluation */
						n_t = 0;
					else if (Ctrl->T.active) {	/* Evaluate the model at the chosen equidistant output points */
						if (Ctrl->T.T.set == 1) {	/* Must update the output t array to be nultiples of inc, possibly starting and ending outside range */
							double min, max;
							gmt_M_free (GMT, Ctrl->T.T.array);
							min = floor (S->min[GMT_X] / Ctrl->T.T.inc) * Ctrl->T.T.inc;
							max = ceil  (S->max[GMT_X] / Ctrl->T.T.inc) * Ctrl->T.T.inc ;
							if (gmt_create_array (GMT, 'T', &(Ctrl->T.T), &min, &max))
								Return (GMT_RUNTIME_ERROR);
						}
						x = Ctrl->T.T.array;	/* Pass these coordinates as our "x" */
						n_t = Ctrl->T.T.n;
					}
					else {	/* Use the given data abscissae instead */
						x = S->data[GMT_X];
						n_t = S->n_rows;
					}

					/* 3. Evaluate the chosen output columns and write records */

					for (row = 0; row < n_t; row++) {
						if (!Ctrl->T.active) outlier = (fabs (z_score[row]) > GMTREGRESS_ZSCORE_LIMIT);	/* Gotta exceed this threshold to be a bad boy */
						if (Ctrl->S.active) {	/* Restrict the output records */
							if (Ctrl->S.mode == GMTREGRESS_OUTPUT_GOOD && outlier) continue;	/* Don't want the outliers */
							if (Ctrl->S.mode == GMTREGRESS_OUTPUT_BAD && !outlier) continue;	/* Only want the outliers */
						}
						for (col = 0; col < n_columns; col++) {	/* Loop over the chosen output columns (-F) */
							switch (Ctrl->F.col[col]) {
								case 'x':	/* Input (or equidistant) x */
									out[col] = x[row];
									break;
								case 'y':	/* Input y */
									out[col] = S->data[GMT_Y][row];
									break;
								case 'm':	/* Model prediction */
									out[col] = model (x[row], par);
									break;
								case 'r':	/* Residual */
									out[col] = S->data[GMT_Y][row] - model (x[row], par);
									break;
								case 'c':	/* Model confidence limit (add x and y uncertainties in quadrature since uncorrelated) */
									out[col] = t_scale * hypot (par[GMTREGRESS_SIGIC], par[GMTREGRESS_SIGSL] * fabs (x[row] - par[GMTREGRESS_XMEAN]));
									break;
								case 'z':	/* Standardized residuals (z-scores) */
									out[col] = z_score[row];
									break;
								case 'w':	/* RLS weights or outlier flags [0 or 1, with 0 meaning outlier] */
									out[col] = (outlier) ? 0.0 : 1.0;
									break;
							}
						}
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this record to output */
					}
				}
				gmt_M_free (GMT, z_score);	/* Done with this array */
			}
		}
	}

	gmt_M_free (GMT, Out);
	error = GMT_NOERROR;
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) 	/* Disables further data output */
		error = API->error;

	if (Ctrl->A.active) {	/* Free special arrays and segment used for -A experiment */
		gmt_free_segment (GMT, &Sa);
		gmt_M_free (GMT, U);
		gmt_M_free (GMT, V);
		gmt_M_free (GMT, W);
		gmt_M_free (GMT, e);
	}
	for (k = GMT_X; k <= GMT_Z; k++)	/* Free arrays used for weights */
		if (Ctrl->W.col[k]) gmt_M_free (GMT, w[k]);

	Return (error);
}
