/*
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
 * Brief synopsis: gmtregress fits y = a + bx linear regression to x,y[,w] data
 *	using a variety of misfit norms and regression modes.
 *
 * Author:	Paul Wessel
 * Date:	5-JAN-2015
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"gmtregress"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Perform linear regression of 1-D data sets"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vabdfghios"

enum GMT_enum_regress {
	GMTREGRESS_N_FARGS	= 6,
	GMTREGRESS_X		= 0,
	GMTREGRESS_Y		= 1,
	GMTREGRESS_XY		= 2,
	GMTREGRESS_RMA		= 3,
	GMTREGRESS_NORM_L1	= 0,
	GMTREGRESS_NORM_L2	= 1,
	GMTREGRESS_NORM_LMS	= 2,
	GMTREGRESS_ANGLE	= 0,
	GMTREGRESS_MISFT	= 1,
	GMTREGRESS_SLOPE	= 2,
	GMTREGRESS_ICEPT	= 3,
	GMTREGRESS_SIGSL	= 4,
	GMTREGRESS_SIGIC	= 5,
	GMTREGRESS_XMEAN	= 6,
	GMTREGRESS_YMEAN	= 7,
	GMTREGRESS_NPAR		= 8,
	GMTREGRESS_NPAR_MAIN	= 4};

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
	struct F {	/* 	-Fxymrcw */
		bool active;
		bool band;	/* True if c was given */
		unsigned int n_cols;
		char col[GMTREGRESS_N_FARGS];	/* Character codes for desired output in the right order */
	} F;
	struct N {	/* 	-N1|2|r */
		bool active;
		unsigned int mode;
	} N;
	struct T {	/* 	-T<min>/<max>/<inc> or -T<n> */
		bool active;
		bool got_n;	/* True if -T<n> was given */
		uint64_t n;
		double min, max, inc;
	} T;
	struct W {	/* 	-W[s]x|y|r */
		bool active;
		unsigned int type;	/* 0 for weights, 1 if sigmas */
		unsigned int n_weights;	/* 1-3 if weights are selected */
		unsigned int mode[3];	/* 1 for GMT_X and/or GMT_Y if weights are present */
	} W;
};

void *New_gmtregress_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTREGRESS_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTREGRESS_CTRL);
	C->A.min = -90.0;	C->A.max = 90.0;	C->A.inc = 1.0;
	C->C.value = 0.95;
	C->E.mode = GMTREGRESS_Y;
	C->N.mode = GMTREGRESS_NORM_L2;
	
	return ((void *)C);
}

void Free_gmtregress_Ctrl (struct GMT_CTRL *GMT, struct GMTREGRESS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free ((void *)C->Out.file);	
	GMT_free (GMT, C);	
}

static int GMT_gmtregress_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtregress [<table>] [-A[<min>/<max>/<inc>]] [-C<level>] [-Ex|y|o|r] [-F<flags>] [-N1|2|r]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<min>/<max>/<inc> | -T<n>] [%s] [-W[w][x][y][r]] [%s] [%s]\n", GMT_V_OPT, GMT_a_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Examine E as function of line slope; give angle range and increment [-90/+90/1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Option -F is not required as no model will be returned; instead we return.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   record of (angle, E, slope, intercept) for all angles specified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Select confidence level (%%) to use in bad calculations (see -F) [95].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Regression type: Select how misfit should be measured:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : Horizontally from data point to regression line\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : Vertically from data point to regression line [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     o : Orthogonally from data point to regression line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : Use Reduced Major Axis area misfit instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Append desired output columns in any order; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : The x observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : The y observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m : The estimated model values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : The estimated residuals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c : The confidence limits (m +/- c).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w : The reweighted Least Square weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Note: Cannot use y|r|w with -T. With -T, x instead means locations implied by -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is xymrcw].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Append desired misfit norm; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 : Use the L-1 measure (mean absolute residuals).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2 : Use L-2 measure (mean squared residuals) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : Use robust measure (median of squared residuals).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Evaluate model at the equidistant points implied by the arguments.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T<n> is given instead we reset <min> and <max> to the extreme x-values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for each segment and determine <inc> to give <n> output values per segment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -T0 to bypass model evaluation entirely.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses locations of input data to evaluate the model].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Supply individual 1-sigma uncertainties for data points [no weights].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append x for sigma_x, y for sigma_y, and r for x-y correlation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   We expect 1-3 extra columns with these data in the given order.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Given a sigma, the weight will be computed via weight = 1/sigma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ww if weights are precomputed and not given as 1-sigma values.\n");
	GMT_Option (API, "V,a,bi,bo,g,h,i,o,.");
	
	return (EXIT_FAILURE);
}

int GMT_gmtregress_parse (struct GMT_CTRL *GMT, struct GMTREGRESS_CTRL *Ctrl, struct GMT_OPTION *options)
{
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
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0 && GMT_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */
			
			case 'A':	/* Explore E vs slope */
				Ctrl->A.active = true;
				if (opt->arg[0]) {
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->A.min, &Ctrl->A.max, &Ctrl->A.inc);
					n_errors += GMT_check_condition (GMT, n < 2, "Syntax error -A option: Must specify min/max/inc\n");
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
					default: n_errors++; break;
				}
				break;
			case 'F':	/* Select output columns */
				Ctrl->F.active = true;
				for (j = 0, k = 0; opt->arg[j]; j++, k++) {
					if (k < GMTREGRESS_N_FARGS) {
						Ctrl->F.col[k] = opt->arg[j];
						if (!strchr ("xymrcw", opt->arg[j])) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F option: Choose from -Fxymrcw\n");
							n_errors++;
						}
						Ctrl->F.n_cols++;
						if (opt->arg[j] == 'c') Ctrl->F.band = true;
					}
					else {
						n_errors++;
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F option: Too many output columns selected\n");
					}
				}
				break;
			case 'N':	/* Select Norm for misfit calculations */
				Ctrl->N.active = true;
				if (opt->arg[0] == '1')
					Ctrl->N.mode = GMTREGRESS_NORM_L1;
				else if (opt->arg[0] == '2')
					Ctrl->N.mode = GMTREGRESS_NORM_L2;
				else if (opt->arg[0] == 'r')
					Ctrl->N.mode = GMTREGRESS_NORM_LMS;
				else
					n_errors++;
				break;
			case 'T':	/* Output lattice or length */
				Ctrl->T.active = true;
				if (strchr (opt->arg, '/')) {
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->T.min, &Ctrl->T.max, &Ctrl->T.inc);
					n_errors += GMT_check_condition (GMT, n < 2, "Syntax error -T option: Must specify min/max/inc\n");
				}
				else {
					Ctrl->T.got_n = true;
					Ctrl->T.n = atol (opt->arg);
				}
				break;
			case 'W':	/* Weights or not */
				Ctrl->W.active = true;
				if (opt->arg[0] == 'w') Ctrl->W.type = 1;	/* Got weights; determine their column position */
				for (k = Ctrl->W.type, col = GMT_Z; opt->arg[k]; k++) {
					if (opt->arg[k] == 'x') Ctrl->W.mode[GMT_X] = col++;
					else if (opt->arg[k] == 'y') Ctrl->W.mode[GMT_Y] = col++;
					else if (opt->arg[k] == 'r') Ctrl->W.mode[GMT_Z] = col++;
					else {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -W option: Specify -W[w][x][y][r]\n");
						n_errors++;
					}
					Ctrl->W.n_weights++;
				}
				if (Ctrl->W.n_weights > 3) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -W option: Gave more than 3 uncertainty types\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_XY && Ctrl->W.n_weights == 1, "Syntax error -Eo: Needs errors in both x,y or neither.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_RMA && Ctrl->W.n_weights == 1, "Syntax error -Er: Needs errors in both x,y or neither.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_X && Ctrl->W.mode[GMT_Y] > 0, "Syntax error -Ex: Cannot specify errors in y.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_X && Ctrl->W.mode[GMT_Z] > 0, "Syntax error -Ex: Cannot specify x-y correlations\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_Y && Ctrl->W.mode[GMT_X] > 0, "Syntax error -Ey: Cannot specify errors in x.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == GMTREGRESS_Y && Ctrl->W.mode[GMT_Z] > 0, "Syntax error -Ey: Cannot specify x-y correlations\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->E.mode == GMTREGRESS_Y || Ctrl->E.mode == GMTREGRESS_X) && Ctrl->W.n_weights == 2, "Syntax error -Ex|y: Cannot specify errors in both x and y.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.value < 0.0 || Ctrl->C.value >= 1.0, "Syntax error -C: Level must be in 0-100%% range.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->A.active, "Syntax error -A: Cannot simultaneously specify -T.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->F.active, "Syntax error -A: Cannot simultaneously specify -F.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->C.active, "Syntax error -A: Cannot simultaneously specify -C.\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 2 columns.\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

double model (double x, double *par)
{	/* Evalute the model given parameters in par */
	return (par[GMTREGRESS_SLOPE] * x + par[GMTREGRESS_ICEPT]);
}

double gmt_sum (double *x, uint64_t n)
{	/* Return sum of array x */
	uint64_t k;
	double S = 0.0;
	for (k = 0; k < n; k++) S += x[k];
	return (S);
}

double icept_basic (struct GMT_CTRL *GMT, double *e, uint64_t n, unsigned int norm)
{	/* Return the proper "average" intercept value given chosen norm */
	unsigned int GMT_n_multiples = 0;
	double X, *ee = NULL;
	
	if (norm != GMTREGRESS_NORM_L2) {	/* Need temporary space for scaled residuals */
		ee = GMT_memory (GMT, NULL, n, double);
		GMT_memcpy (ee, e, n, double);
		GMT_sort_array (GMT, ee, n, GMT_DOUBLE);
	}
	switch (norm) {
		case GMTREGRESS_NORM_L1:	/* Return median */
		 	X = (n%2) ? ee[n/2] : 0.5 * (e[(n-1)/2] + ee[n/2]);
			break;
		case GMTREGRESS_NORM_L2:	/* Return mean */
			X = gmt_sum (e, n) / n;
			break;
		case GMTREGRESS_NORM_LMS:	/* Return mode */
			GMT_mode (GMT, ee, n, n/2, 0L, -1L, &GMT_n_multiples, &X);
			break;
	}
	if (norm != GMTREGRESS_NORM_L2) GMT_free (GMT, ee);
			
	return (X);	
}

double icept_weighted (struct GMT_CTRL *GMT, double *e, double *W, uint64_t n, unsigned int norm)
{	/* Return the proper "weighted average" intercept value given chosen norm */
	double X;
	struct OBSERVATION *ee = NULL;
	
	if (norm != GMTREGRESS_NORM_L2) {	/* Need temporary space for scaled residuals */
		uint64_t k;
		ee = GMT_memory (GMT, NULL, n, struct OBSERVATION);
		for (k = 0; k < n; k++) {
			ee[k].weight = W[k];
			ee[k].value  = e[k];
		}
	}
	switch (norm) {
		case GMTREGRESS_NORM_L1:	/* Return weighted median */
			X = GMT_median_weighted (GMT, ee, n, 0.5);
			break;
		case GMTREGRESS_NORM_L2:	/* Return weighted mean */
			X = GMT_mean_weighted (GMT, e, W, n);
			break;
		case GMTREGRESS_NORM_LMS:	/* Return weighted mode */
			X = GMT_mode_weighted (GMT, ee, n);
			break;
	}
	if (norm != GMTREGRESS_NORM_L2) GMT_free (GMT, ee);
			
	return (X);	
}

double intercept (struct GMT_CTRL *GMT, double *e, double *W, uint64_t n, bool weighted, unsigned int norm)
{	/* Return the proper "average" intercept value given chosen norm */
	double a = (weighted) ? icept_weighted (GMT, e, W, n, norm) : icept_basic (GMT, e, n, norm);
	return (a);
}

double get_scale_factor (unsigned int regression, double slope)
{	/* Scale that turns y-misfit into other misfit measures given slope */
	double f;
	slope = fabs (slope);
	switch (regression) {
		case GMTREGRESS_X:   f = 1.0 / slope; break;
		case GMTREGRESS_Y:   f = 1.0; break;
		case GMTREGRESS_XY:  f = sqrt (1.0 / (1.0 + slope * slope)); break;
		case GMTREGRESS_RMA: f = sqrt (1.0 / slope); break;
	}
	return (f);
}

double L1_misfit (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, double *W, uint64_t n, unsigned int regression, double slope)
{	/* Compute L1 misfit in x|y|o|r */
	uint64_t k;
	double f, E = 0.0;
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) E += fabs (W[k] * ey[k]);
	return (f * E / (n - 2));
}

double L2_misfit (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, double *W, uint64_t n, unsigned int regression, double slope)
{	/* Compute L2 misfit in x|y|o|r */
	uint64_t k;
	double f, E = 0.0;
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) E += W[k] * ey[k] * ey[k];
	return (f * f * E / (n - 2));	/* f^2 since E was computed from squared misfits */
}

double LMS_misfit (struct GMT_CTRL *GMT, double *ey, double *W, uint64_t n, unsigned int regression, double slope)
{	/* Compute LMS misfit in x|y|o|r */
	uint64_t k;
	double f, E, *ee = GMT_memory (GMT, NULL, n, double);
	f = get_scale_factor (regression, slope);
	for (k = 0; k < n; k++) ee[k] = W[k] * ey[k] * ey[k];
 	GMT_sort_array (GMT, ee, n, GMT_DOUBLE);
	E = (n%2) ? ee[n/2] : 0.5 * (ee[(n-1)/2] + ee[n/2]);
	GMT_free (GMT, ee);
	return (1.4826 * f * f * E);	/* f^2 since E was computed from squared misfits */
}

void gmt_prod (double *x, double *y, double *xy, uint64_t n)
{	/* Compute array xy[i] = x[i] * y[i] */
	uint64_t k;
	for (k = 0; k < n; k++) xy[k] = x[k] * y[k];
}

double gmt_sumprod2 (double *x, double *y, uint64_t n)
{	/* Sum up product of x * y */
	uint64_t k;
	double sum = 0.0;
	for (k = 0; k < n; k++) sum += x[k] * y[k];
	return (sum);
}

double gmt_sumprod3 (double *x, double *y, double *z, uint64_t n)
{	/* Sum up product of x * y * z */
	uint64_t k;
	double sum = 0.0;
	for (k = 0; k < n; k++) sum += x[k] * y[k] * z[k];
	return (sum);
}

void gmt_add (double *x, double c, double *out, uint64_t n)
{	/* Compute array out[i] = x[i] + c */
	uint64_t k;
	for (k = 0; k < n; k++) out[k] = x[k] + c;
}

double gmt_demeaning (struct GMT_CTRL *GMT_UNUSED(GMT), double *X, double *Y, double *w[], uint64_t n, double *par, double *U, double *V, double *W, double *alpha, double *beta)
{
	/* Compute weighted X and Y means, return these via par, and calculate residuals U and V and weights W
	 * (and alpha, beta if orthogonal).  If orthogonal regression we expect a preliminary estimate of the
	 * slope to be present in par[GMTREGRESS_SLOPE].  Return weight sum S */
	double S;
	
	if (w && w[GMT_X] && w[GMT_Y]) {	/* Orthogonal regression with weights requested */
		double corr_i = 0.0, alpha_i, w_xy;
		uint64_t i;
		/*  Compute Wi from w(X_i), w(Y_i), r_i and best estimate of slope in par[GMTREGRESS_SLOPE].
		 * This is based on recipe in York et al [2004] */
		for (i = 0; i < n; i++) {
			w_xy = w[GMT_X][i] * w[GMT_Y][i];
			alpha_i = sqrt (w_xy);
			if (w[GMT_Z]) corr_i = w[GMT_Z][i];
			W[i] = w_xy / (w[GMT_X][i] + par[GMTREGRESS_SLOPE] * par[GMTREGRESS_SLOPE] * w[GMT_Y][i] - 2 * par[GMTREGRESS_SLOPE] * corr_i * alpha_i);
			if (alpha) alpha[i] = alpha_i;
			//fprintf (stderr, "i = %d: alpha_i = %g W[i] = %g\n", (int)i, alpha_i, W[i]);
		}
		/*  Step 4: Compute weighted X_mean, Y_mean, then U, V, and beta */
		S = gmt_sum (W, n);					/* Get sum of weights */
		par[GMTREGRESS_XMEAN] = gmt_sumprod2 (W, X, n) / S;	/* Compute X_mean */
		par[GMTREGRESS_YMEAN] = gmt_sumprod2 (W, Y, n) / S;	/* Compute Y_mean */
		gmt_add (X, -par[GMTREGRESS_XMEAN], U, n);		/* Compute U */
		gmt_add (Y, -par[GMTREGRESS_YMEAN], V, n);		/* Compute V */
		if (beta) {
			for (i = 0; i < n; i++) {	/* Compute beta */
				if (w[GMT_Z]) corr_i = w[GMT_Z][i];
				beta[i] = W[i] * (U[i] / w[GMT_Y][i] + par[GMTREGRESS_SLOPE] * V[i] / w[GMT_X][i] - (par[GMTREGRESS_SLOPE] * U[i] + V[i]) * corr_i / alpha[i]);
				//fprintf (stderr, "i = %d: U[i] = %g V[i] = %g beta_i = %g\n", (int)i, U[i], V[i], beta[i]);
			}
		}
	}
	else if (w && (w[GMT_X] || w[GMT_Y])) {	/* Not orthogonal, but have weights in x or y */
		double *pW = (w[GMT_X]) ? w[GMT_X] : w[GMT_Y];	/* Shorthand for the (squared) weights */
		GMT_memcpy (W, pW, n, double);			/* Duplicate the chosen weight array to W */
		S = gmt_sum (W, n);					/* Get sum of weights */
		par[GMTREGRESS_XMEAN] = gmt_sumprod2 (W, X, n) / S;	/* Compute X_mean */
		par[GMTREGRESS_YMEAN] = gmt_sumprod2 (W, Y, n) / S;	/* Compute Y_mean */
		gmt_add (X, -par[GMTREGRESS_XMEAN], U, n);		/* Compute U */
		gmt_add (Y, -par[GMTREGRESS_YMEAN], V, n);		/* Compute V */
	}
	else {	/* No weights, create unit array */
		uint64_t i;
		for (i = 0; i < n; i++) W[i] = 1.0;		/* Unit weights */
		par[GMTREGRESS_XMEAN] = gmt_sum (X, n) / n;	/* Compute X_mean */
		par[GMTREGRESS_YMEAN] = gmt_sum (Y, n) / n;	/* Compute X_mean */
		gmt_add (X, -par[GMTREGRESS_XMEAN], U, n);	/* Compute U */
		gmt_add (Y, -par[GMTREGRESS_YMEAN], V, n);	/* Compute V */
		S = (double)n;
	}
	return (S);
}

void LSy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par)
{	/* Basic LS y-regression, only uses w[GMT_Y] */
	uint64_t k;
	double *Q = GMT_memory (GMT, NULL, n, double), *P = GMT_memory (GMT, NULL, n, double);
	double *U = GMT_memory (GMT, NULL, n, double), *V = GMT_memory (GMT, NULL, n, double);
	double S, S_xx, S_xy, D, *W = GMT_memory (GMT, NULL, n, double);
	
	GMT_memset (par, GMTREGRESS_NPAR, double);
	S = gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);	/* alpha, beta not used */
	
	/* Because we operate on U, V the terms S_x = S_y == 0 and are thus ignored in the equation below */
	if (w && w[GMT_Y]) {	/* Weighted regression */
		gmt_prod (U, W, P, n);
		gmt_prod (P, U, Q, n);
		S_xx = gmt_sum (Q, n);
		gmt_prod (P, V, Q, n);
		S_xy = gmt_sum (Q, n);
	}
	else {	/* No weights supplied */
		gmt_prod (U, U, Q, n);
		S_xx = gmt_sum (Q, n);
		gmt_prod (U, V, Q, n);
		S_xy = gmt_sum (Q, n);
	}
	D = 1.0 / (S * S_xx);
	par[GMTREGRESS_SLOPE] = (S * S_xy) * D;
 	par[GMTREGRESS_ICEPT] = par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN];
 	par[GMTREGRESS_SIGSL] = sqrt (S * D);
 	par[GMTREGRESS_SIGIC] = sqrt (S_xx * D);
	/* Here we recycle Q to hold residual e */
	for (k = 0; k < n; k++)
		Q[k] = y[k] - model (x[k], par);
	par[GMTREGRESS_MISFT] = L2_misfit (GMT, Q, W, n, GMTREGRESS_Y, 0.0);
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	GMT_free (GMT, Q);
	GMT_free (GMT, P);
	GMT_free (GMT, U);
	GMT_free (GMT, V);
	GMT_free (GMT, W);
}

void LSxy_regress1D_basic (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *par)
{	/* Basic LS xy-regression, with no errors. See York [1966] */
	uint64_t k;
	unsigned int p;
	double *u = GMT_memory (GMT, NULL, n, double), *v = GMT_memory (GMT, NULL, n, double);
	double *W = GMT_memory (GMT, NULL, n, double), *Q = GMT_memory (GMT, NULL, n, double);
	double mean_x, mean_y, sig_x, sig_y, sum_u2, sum_v2, sum_uv, part1, part2, r, a[2], b[2], E[2];
	
	mean_x = GMT_mean_and_std (GMT, x, n, &sig_x);
	mean_y = GMT_mean_and_std (GMT, y, n, &sig_y);
	/* Normalize the data */
	gmt_add (x, -mean_x, u, n);
	gmt_add (y, -mean_y, v, n);
	gmt_prod (u, u, Q, n);
	sum_u2 = gmt_sum (Q, n);
	gmt_prod (v, v, Q, n);
	sum_v2 = gmt_sum (Q, n);
	gmt_prod (u, v, Q, n);
	sum_uv = gmt_sum (Q, n);
	GMT_free (GMT, u);	GMT_free (GMT, v);
	part1 = sum_v2 - sum_u2;
	part2 = sqrt (pow (sum_u2 - sum_v2, 2.0) + 4.0 * sum_uv * sum_uv);
	b[0] = (part1 + part2) / (2.0 * sum_uv);
	b[1] = (part1 - part2) / (2.0 * sum_uv);
	r = sum_uv / sqrt (sum_u2 * sum_v2);
	for (k = 0; k < n; k++) W[k] = 1.0;	/* Unity weights */
	for (p = 0; p < 2; p++) {	/* Compute E for both solutions */
		a[p] = mean_y - b[p] * mean_x;
		for (k = 0; k < n; k++) Q[k] = y[k] - b[p] * x[k] - a[p];
		E[p] = L2_misfit (GMT, Q, W, n, GMTREGRESS_XY, b[p]);
	}
	p = (E[0] < E[1]) ? 0 : 1;	/* Determine the solution with smallest misfit */
	par[GMTREGRESS_SLOPE] = b[p];
	par[GMTREGRESS_ICEPT] = a[p];
	par[GMTREGRESS_SIGSL] = par[GMTREGRESS_SLOPE] * sqrt ((1.0 - r * r) / n) / r;
	par[GMTREGRESS_SIGIC] = sqrt (pow (sig_y - sig_x * par[GMTREGRESS_SLOPE], 2.0) / n + (1.0 - r) * par[GMTREGRESS_SLOPE] * (2.0 * sig_x * sig_y + (mean_x * par[GMTREGRESS_SLOPE] * (1.0 + r) / (r * r))));
	par[GMTREGRESS_MISFT] = E[p];
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	GMT_free (GMT, Q);
	GMT_free (GMT, W);
}

void LSRMA_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par)
{	/* Basic LS RMA-regression with no weights [Reference?] */
	uint64_t k;
	double mx, sx, my, sy, S;
	double *U = GMT_memory (GMT, NULL, n, double), *V = GMT_memory (GMT, NULL, n, double), *W = GMT_memory (GMT, NULL, n, double);
	GMT_memset (par, GMTREGRESS_NPAR, double);
	S = gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);
	mx = GMT_mean_and_std (GMT, U, n, &sx);
	my = GMT_mean_and_std (GMT, V, n, &sy);
	par[GMTREGRESS_SLOPE] = sy / sx;
	par[GMTREGRESS_ICEPT] = par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN];
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
	/* Here we recycle U as residual e */
	for (k = 0; k < n; k++)
		U[k] = y[k] - model (x[k], par);
	par[GMTREGRESS_MISFT] = L2_misfit (GMT, U, W, n, GMTREGRESS_RMA, par[GMTREGRESS_SLOPE]);
	GMT_free (GMT, U);
	GMT_free (GMT, V);
	GMT_free (GMT, W);
}

void regress1D_sub (struct GMT_CTRL *GMT, double *x, double *y, double *W, double *e, uint64_t n, unsigned int regression, unsigned int norm, bool weighted, double angle, double *par)
{	/* Solve the linear regression for a given angle and chosen misfit and norm to give the intercept */
	/* x, y here are actually reduced coordinates U, V */
	uint64_t k;
	double a, b, E;
	double (*misfit) (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, double *W, uint64_t n, unsigned int regression, double slope);
	switch (norm) {	/* Select misfit function */
		case GMTREGRESS_NORM_L1:  misfit = L1_misfit;  break;
		case GMTREGRESS_NORM_L2:  misfit = L2_misfit;  break;
		case GMTREGRESS_NORM_LMS: misfit = LMS_misfit; break;
	}
	if (GMT_IS_ZERO (fabs (angle) - 90.0)) {	/* Vertical line is a special case */
		b = GMT->session.d_NaN;				/* Slope is undefined */
		a = intercept (GMT, x, W, n, weighted, norm);	/* Determine x-intercept */
		for (k = 0; k < n; k++) e[k] = x[k] - a;	/* Final x-residuals */
		/* For GMTREGRESS_Y|GMTREGRESS_RMA a vertical line gives Inf misfit; the others are measured horizontally.
		 * We obtain E by passing e as ex but giving mode GMTREGRESS_Y instead and pass 0 as slope. */
		E = (regression == GMTREGRESS_Y || regression == GMTREGRESS_RMA) ? DBL_MAX : misfit (GMT, e, W, n, GMTREGRESS_Y, 0.0);
	}
	else if (GMT_IS_ZERO (angle)) {	/* Horizontal line is a special case */
		b = 0.0;
		a = intercept (GMT, y, W, n, weighted, norm);	/* Determine y-intercept */
		/* For GMTREGRESS_X|GMTREGRESS_RMA a horizontal line gives Inf misfit; the others are measured vertically.
		 * We obtain E by passing e as ey but giving mode GMTREGRESS_Y instead and pass 0 as slope. */
		for (k = 0; k < n; k++) e[k] = y[k] - a;	/* Final y-residuals */
		E = (regression == GMTREGRESS_X || regression == GMTREGRESS_RMA) ? DBL_MAX : misfit (GMT, e, W, n, GMTREGRESS_Y, 0.0);
	}
	else {	/* Not vertical|horizontal, we can measure any misfit and pass the slope b */
		b = tand (angle);				/* Regression slope */
		for (k = 0; k < n; k++) e[k] = y[k] - b * x[k];	/* Residuals after removing slope */
		a = intercept (GMT, e, W, n, weighted, norm);	/* Determine intercept */
		for (k = 0; k < n; k++) e[k] -= a;		/* Final y-residuals */
		E = misfit (GMT, e, W, n, regression, b);	/* The representative misfit */
	}
	if (GMT_is_dnan (E)) E = DBL_MAX;	/* If anything goes crazy, set E to huge */
	par[GMTREGRESS_ANGLE] = angle;	par[GMTREGRESS_MISFT] = E; par[GMTREGRESS_SLOPE] = b;	par[GMTREGRESS_ICEPT] = a;
}

#define N_ANGLE_SELECTIONS	90

void regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, unsigned int regression, unsigned int norm, double *par)
{	/* Solve the linear regression for a given angle, chosen misfit and norm */
	uint64_t k;
	bool done = false, weighted = false;
	double a_min = -90.0, a_max = 90.0, angle, r_a, d_a, f, last_E = DBL_MAX, S, tpar[GMTREGRESS_NPAR];
	double *U = GMT_memory (GMT, NULL, n, double), *V = GMT_memory (GMT, NULL, n, double);
	double *W = GMT_memory (GMT, NULL, n, double), *e = GMT_memory (GMT, NULL, n, double);
	
	GMT_memset (par, GMTREGRESS_NPAR, double);
	if (regression != GMTREGRESS_XY) S = gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);	/* Do this once except for orthogonal */
	par[GMTREGRESS_MISFT] = DBL_MAX;
	weighted = (regression == GMTREGRESS_X) ? (w && w[GMT_X]) : (w && w[GMT_Y]);
	while (!done) {	/* Keep iterating an zooming in until misfit is small */
		r_a = a_max - a_min;
		d_a = r_a / (double)N_ANGLE_SELECTIONS;
		for (k = 0; k <= N_ANGLE_SELECTIONS; k++) {	/* Try all slopes in current sub-range */
			angle = a_min + d_a * k;
			if (regression == GMTREGRESS_XY) {	/* Since W depends on slope we must recompute W each time */
				par[GMTREGRESS_SLOPE] = tand (angle);
				S = gmt_demeaning (GMT, x, y, w, n, par, U, V, W, NULL, NULL);
			}
			regress1D_sub (GMT, U, V, W, e, n, regression, norm, weighted, angle, tpar);
			if (tpar[GMTREGRESS_MISFT] < par[GMTREGRESS_MISFT]) GMT_memcpy (par, tpar, GMTREGRESS_NPAR-2U, double);	/* Update best fit so far without stepping on the means*/
		}
		if (par[GMTREGRESS_MISFT] <= last_E && (f = (last_E - par[GMTREGRESS_MISFT])/par[GMTREGRESS_MISFT]) < GMT_CONV8_LIMIT)
			done = true;	/* Change is tiny so we are done */
		else {	/* Gradually zoom in on the angles with smallest misfit but allow some slack */
			a_min = MAX (-90.0, par[GMTREGRESS_ANGLE] - 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			a_max = MIN (+90.0, par[GMTREGRESS_ANGLE] + 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			last_E = par[GMTREGRESS_MISFT];
		}
	}
	/* Adjust intercept from U,V -> (x,y) */
	par[GMTREGRESS_ICEPT] += (par[GMTREGRESS_YMEAN] - par[GMTREGRESS_SLOPE] * par[GMTREGRESS_XMEAN]);
	GMT_free (GMT, U);
	GMT_free (GMT, V);
	GMT_free (GMT, W);
	GMT_free (GMT, e);
}

#define GMTREGRESS_MAX_YORK_ITERATIONS	1000	/* Gotta have a stopper in case of bad data? */

void LSxy_regress1D_york (struct GMT_CTRL *GMT, double *X, double *Y, double *w[], uint64_t n, double *par)
{
	/* Solution to general LS orthogonal regression, per York et al. [2004] */
	uint64_t i;
	unsigned int n_iter = 0;
	double *W = NULL, *U = NULL, *V = NULL, *x = NULL, *u = NULL, *alpha = NULL, *beta = NULL;
	double b, b_old, a, W_sum, x_mean, sigma_a, sigma_b, misfit;
	char buffer[GMT_BUFSIZ] = {""};
	
	/* Allocate temporary vectors */
	W = GMT_memory (GMT, NULL, n, double);
	U = GMT_memory (GMT, NULL, n, double);
	V = GMT_memory (GMT, NULL, n, double);
	x = GMT_memory (GMT, NULL, n, double);
	u = GMT_memory (GMT, NULL, n, double);
	alpha = GMT_memory (GMT, NULL, n, double);
	beta  = GMT_memory (GMT, NULL, n, double);
	/* Step 1: Get initial slope from basic LS y on x */
	LSy_regress1D (GMT, X, Y, NULL, n, par);
	b = par[GMTREGRESS_SLOPE];	/* Best slope value so far */
	GMT_memset (par, GMTREGRESS_NPAR, double);
	/* Step 2: Weights w(X_i) and w(Y_i) are already set in main */
	do {	/* Keep iterating until converged [Step 6 in York et al, 2004] */
		b_old = b;	/* Previous best slope */
		/*  Step 3: Compute Wi from w(X_i), w(Y_i), r_i and Step 4: Compute weighted X_mean, Y_mean, then U, V, alpha, beta */
		par[GMTREGRESS_SLOPE] = b_old;	/* Pass in previous best-fitting slope needed to update W */
		W_sum = gmt_demeaning (GMT, X, Y, w, n, par, U, V, W, alpha, beta);	/* Set X_mean, Y_mean, W, U, V, alpha, beta */
		/*  Step 5: Compute improved estimate of slope b */
		b = gmt_sumprod3 (W, beta, V, n) / gmt_sumprod3 (W, beta, U, n);
		/* Step 7: Calculate new a */
		a = par[GMTREGRESS_YMEAN] - b * par[GMTREGRESS_XMEAN];
		/* Step 8: Compute adjusted points x */
		gmt_add (beta, par[GMTREGRESS_XMEAN], x, n);	/* Compute x */
		/* Step 9: Compute u */
		x_mean = gmt_sumprod2 (W, x, n) / W_sum;	/* Compute x_mean */
		gmt_add (x, -x_mean, u, n);			/* Compute u */
		/* Step 10: Compute sigma_b and sigma_a */
		sigma_b = 1.0 / gmt_sumprod3 (W, u, u, n);	/* Actually sigma_b^2 */
		sigma_a = sqrt (1.0 / W_sum + x_mean * x_mean * sigma_b);
		sigma_b = sqrt (sigma_b);			/* Now it is sigma_b */
		/* Estimate weighted residuals (recycling V for this purpose) */
		for (i = 0; i < n; i++) V[i] = Y[i] - b * X[i] - a;
		misfit = L2_misfit (GMT, V, W, n, GMTREGRESS_XY, 0.0);
		n_iter++;
		sprintf (buffer, "York iteration %d: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: %g sig_icept: %g",
			n_iter, n, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN], atand (b), misfit, b, a, sigma_b, sigma_a);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "%s\n", buffer);
	} while (fabs (b - b_old) > GMT_CONV15_LIMIT && n_iter < GMTREGRESS_MAX_YORK_ITERATIONS);
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "York orthogonal algorithm convergence required %d iterations\n", n_iter);
	/* Free temporary arrays */
	GMT_free (GMT, W);
	GMT_free (GMT, x);
	GMT_free (GMT, u);
	GMT_free (GMT, U);
	GMT_free (GMT, V);
	GMT_free (GMT, alpha);
	GMT_free (GMT, beta);
	
	/* Pass out the final result via par array */
	par[GMTREGRESS_SLOPE] = b;
	par[GMTREGRESS_ICEPT] = a;
	par[GMTREGRESS_SIGSL] = sigma_b;
	par[GMTREGRESS_SIGIC] = sigma_a;
	par[GMTREGRESS_MISFT] = misfit;
	par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
}

void LSxy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w[], uint64_t n, double *par)
{
	GMT_memset (par, GMTREGRESS_NPAR, double);
	if (w && w[GMT_X] && w[GMT_Y])	/* Have weights in x and y [and possibly correlation coefficients as well] */
		LSxy_regress1D_york (GMT, x, y, w, n, par);
	else	/* Simpler case with no weights */
		LSxy_regress1D_basic (GMT, x, y, n, par);
}

void do_regression (struct GMT_CTRL *GMT, double *x_in, double *y_in, double *w[], uint64_t n, unsigned int regression, unsigned int norm, double *par)
{
	/* Solves for the best regression of (x_in, y_in) given the settings. */
	
	bool flipped;
	double *x = NULL, *y = NULL, *ww[3] = {NULL, NULL, NULL};
	
	if (regression == GMTREGRESS_X) {	/* Do x on y regression by flipping the input, then transpose par before output */
		x = y_in;	y = x_in;
		regression = GMTREGRESS_Y;
		flipped = true;
		ww[GMT_X] = w[GMT_Y];	/* Must also flip the weights */
		ww[GMT_Y] = w[GMT_X];
	}
	else {	/* Normal: x is x, y is y */
		x = x_in;	y = y_in;
		flipped = false;
		ww[GMT_X] = w[GMT_X];
		ww[GMT_Y] = w[GMT_Y];
	}
	
	switch (regression) {
		case GMTREGRESS_Y:	/* Vertical misfit measure */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 regression y on x has analytic solution */
					LSy_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
		case GMTREGRESS_XY:	/* Orthogonal regression */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 orthogonal regression has analytic solution */
					LSxy_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
		case GMTREGRESS_RMA:	/* RMA regression */
			switch (norm) {
				case GMTREGRESS_NORM_L1:	/* L1 regression */
				case GMTREGRESS_NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, ww, n, regression, norm, par);
					break;
				case GMTREGRESS_NORM_L2:	/* L2 RMA regression has analytic solution */
					LSRMA_regress1D (GMT, x, y, ww, n, par);
					break;
			}
			break;
	}
	if (flipped) {	/* Must transpose x/y results */
		/* We solved x = a' + b' * y but wanted y = a + b * x.
		 * Basic algebra shows a = -a' / b' and b = 1/b'.
		 * Also swap the means and sigmas, but leave misfit as is. */
		par[GMTREGRESS_ICEPT] = -par[GMTREGRESS_ICEPT] / par[GMTREGRESS_SLOPE];
		par[GMTREGRESS_SLOPE] = 1.0 / par[GMTREGRESS_SLOPE];
		par[GMTREGRESS_ANGLE] = atand (par[GMTREGRESS_SLOPE]);
		double_swap (par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN]);
		double_swap (par[GMTREGRESS_SIGSL], par[GMTREGRESS_SIGIC]);
	}
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtregress_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtregress (void *V_API, int mode, void *args)
{
	uint64_t k, seg, tbl, col, row, n_try, n_t, n_alloc = 0, n_columns = GMTREGRESS_N_FARGS;

	int error = 0;
	
	double *x = NULL, *U = NULL, *V = NULL, *W = NULL, *e = NULL, *w[3] = {NULL, NULL, NULL};
	double t_scale = 0.0, par[GMTREGRESS_NPAR], out[GMTREGRESS_N_FARGS], *t = NULL;
	
	char buffer[GMT_BUFSIZ], *F_default = "xymrcw";
	
	struct GMT_DATASET *Din = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sa = NULL;
	struct GMTREGRESS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtregress_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmtregress_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtregress_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtregress_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtregress_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtregress main code ----------------------------*/

	if (Ctrl->A.active) {	/* Explore E vs slope only; no best-fit is returned */
		n_try = lrint ((Ctrl->A.max - Ctrl->A.min) / Ctrl->A.inc) + 1;
		n_columns = GMTREGRESS_NPAR_MAIN;	/* Hardwired to return angle, misfit, slope, intercept */
		Sa = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
		GMT_alloc_segment (GMT, Sa, n_try, n_columns, true);	/* Reallocate fixed temp space for this experiment */
	}
	else {	/* Work up best solution per input segment */
		if (!Ctrl->F.active) {	/* Default is all columns */
			Ctrl->F.n_cols = GMTREGRESS_N_FARGS;
			for (col = 0; col < n_columns; col++) Ctrl->F.col[col] = F_default[col];
		}
		n_columns = Ctrl->F.n_cols;
	}
	
	if (Ctrl->T.active) {	/* Evaluate solution for equidistant spacing instad of input locations */
		unsigned int bad = 0;
		if (!Ctrl->T.got_n)	/* Compute number of points given fixed range and increment */
			Ctrl->T.n = GMT_get_n (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, 0);
		for (col = 0; col < n_columns; col++) if (Ctrl->T.n && strchr ("yrw", (int)Ctrl->F.col[col])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot choose -F%c when -T is in effect\n", (int)Ctrl->F.col[col]);
			bad++;
		}
		if (bad) Return (GMT_RUNTIME_ERROR);
		if (Ctrl->T.n) t = GMT_memory (GMT, NULL, Ctrl->T.n, double);	/* Space for output x-values */
	}

	/* Allocate memory and read in all the files; each file can have many lines */
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {
		Return (API->error);	/* Establishes data files or stdin */
	}
	if ((error = GMT_set_cols (GMT, GMT_IN, 2 + Ctrl->W.n_weights))) Return (error);
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_columns))) Return (error);

	GMT_set_segmentheader (GMT, GMT_OUT, true);	/* To write segment headers regardless of input */
	
	/* Process all tables and their segments */
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current data segment */
			if (S->n_rows < 3) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Segment only has < 2 points - skipped\n");
				continue;
			}

			/* Make sure we have enough memory allocated for weights and such */
			if (S->n_rows > n_alloc) {	/* Need more memory (first time we have nothing, then only allocated when larger) */
				n_alloc = S->n_rows;
				for (k = GMT_X; k <= GMT_Z; k++)	/* Do for each weight column in the data */
					if (Ctrl->W.mode[k]) w[k] = GMT_memory (GMT, w[k], n_alloc, double);
				if (Ctrl->A.active) {	/* Addidional arrays for the slope-scanning experiment */
					U = GMT_memory (GMT, U, n_alloc, double);
					V = GMT_memory (GMT, V, n_alloc, double);
					W = GMT_memory (GMT, W, n_alloc, double);
					e = GMT_memory (GMT, e, n_alloc, double);
				}
			}
			/* Deal with input columns with sigmas and correlations, if present */
			for (k = GMT_X; k <= GMT_Z; k++) {	/* Check for sigma-x, sigma-y, r columns */
				if (!Ctrl->W.mode[k]) continue;	/* No such column was selected */
				if (Ctrl->W.type || k == GMT_Z)	/* Got weights or dealing with the correlation column, so just copy */
					GMT_memcpy (w[k], S->coord[Ctrl->W.mode[k]], S->n_rows, double);
				else	/* Got sigma; convert to weights = 1/sigma */
					for (row = 0; row < S->n_rows; row++) w[k][row] = 1.0 / S->coord[Ctrl->W.mode[k]][row];
				/* Square the weights here, except for correlations (if present) */
				if (k < GMT_Z) for (row = 0; row < S->n_rows; row++) w[k][row] *= w[k][row];
				col++;	/* Go to next input column */
			}
			
			if (Ctrl->A.active) {	/* Explore E vs slope only */
				uint64_t min_row;
				double angle, min_E = DBL_MAX, W_sum;
				bool weighted = (Ctrl->E.mode == GMTREGRESS_X) ? (w && w[GMT_X]) : (w && w[GMT_Y]);
				
				/* Determine means and compute reduced coordinates U,V and return proper weights W once, unless orthogonal */
				if (Ctrl->E.mode != GMTREGRESS_XY) W_sum = gmt_demeaning (GMT, S->coord[GMT_X], S->coord[GMT_Y], w, S->n_rows, par, U, V, W, NULL, NULL);
				for (row = 0; row < n_try; row++) {	/* For each slope candidate */
					angle = Ctrl->A.min + row * Ctrl->A.inc;	/* Slope in degrees */
					if (Ctrl->E.mode == GMTREGRESS_XY) {	/* Since W depends on slope we must recompute W each time */
						par[GMTREGRESS_SLOPE] = tand (angle);
						W_sum = gmt_demeaning (GMT, S->coord[GMT_X], S->coord[GMT_Y], w, S->n_rows, par, U, V, W, NULL, NULL);
					}
					regress1D_sub (GMT, U, V, W, e, S->n_rows, Ctrl->E.mode, Ctrl->N.mode, weighted, angle, par);
					if (par[GMTREGRESS_MISFT] < min_E) {	/* Update best fit so far */
						min_E = par[GMTREGRESS_MISFT];
						min_row = row;
					}
					for (k = 0; k < GMTREGRESS_NPAR_MAIN; k++) Sa->coord[k][row] = par[k];	/* Save the results */
				}
				/* Make segment header with the findings for best regression */
				sprintf (buffer, "Best regression: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g", S->n_rows, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN], Sa->coord[0][min_row],
					Sa->coord[1][min_row], Sa->coord[2][min_row], Sa->coord[3][min_row]);
				GMT_Report (API, GMT_MSG_VERBOSE, "%s\n", buffer);	/* Report results if verbose */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);	/* Also include in segment header */
				for (row = 0; row < n_try; row++) {	/* Output the saved results of the experiment */
					for (k = 0; k < GMTREGRESS_NPAR_MAIN; k++) out[k] = Sa->coord[k][row];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this record to output */
				}
			}
			else {	/* Here we are solving for the best regression */
				do_regression (GMT, S->coord[GMT_X], S->coord[GMT_Y], w, S->n_rows, Ctrl->E.mode, Ctrl->N.mode, par);
				/* Make segment header with the findings for best regression */
				sprintf (buffer, "Best regression: N: %" PRIu64 " x0: %g y0: %g angle: %g E: %g slope: %g icept: %g sig_slope: %g sig_icept: %g", S->n_rows, par[GMTREGRESS_XMEAN], par[GMTREGRESS_YMEAN],
					par[GMTREGRESS_ANGLE], par[GMTREGRESS_MISFT], par[GMTREGRESS_SLOPE], par[GMTREGRESS_ICEPT], par[GMTREGRESS_SIGSL], par[GMTREGRESS_SIGIC]);
				GMT_Report (API, GMT_MSG_VERBOSE, "%s\n", buffer);	/* Report results if verbose */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);	/* Also include in segment header */

				if (Ctrl->F.band)	/* For confidence band given a significance level */
					t_scale = GMT_tcrit (GMT, 0.5 * (1.0 - Ctrl->C.value), S->n_rows - 2.0);

				if (Ctrl->T.active) {	/* Evaluate the model on the chosen equidistant output points */
					if (Ctrl->T.got_n) {	/* Got -T<n>, so must recompute new increment for given range */
						Ctrl->T.min = S->min[GMT_X];	Ctrl->T.max = S->max[GMT_X];
						if (Ctrl->T.n) Ctrl->T.inc = GMT_get_inc (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.n, 0);	/* Protect againts -T0 */
					}
					for (k = 0; k < Ctrl->T.n; k++)	/* Build output x coordinates */
						t[k] = GMT_col_to_x (GMT, k, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, 0, Ctrl->T.n);
					x = t;	/* Pass these coordinates as "x" */
					n_t = Ctrl->T.n;
				}
				else {	/* Use the given data abscissae instead */
					x = S->coord[GMT_X];
					n_t = S->n_rows;
				}					
				
				/* 3. Evaluate the output columns */
				for (row = 0; row < n_t; row++) {
					for (col = 0; col < n_columns; col++) {
						switch (Ctrl->F.col[col]) {
							case 'x':	/* Input (or equidistant) x */
								out[col] = x[row];
								break;
							case 'y':	/* Input y */
								out[col] = S->coord[GMT_Y][row];
								break;
							case 'm':	/* Model prediction */
								out[col] = model (x[row], par);
								break;
							case 'r':	/* Residual */
								out[col] = S->coord[GMT_Y][row] - model (x[row], par);
								break;
							case 'c':	/* Model confidence limit  */
								out[col] = t_scale * hypot (par[GMTREGRESS_SIGIC], par[GMTREGRESS_SIGSL] * fabs (x[row] - par[GMTREGRESS_XMEAN]));
								break;
							case 'w':	/* Residual weights [0 or 1 if LMS] [not implemented yet] */
								out[col] = 0.0;
								break;
						}
					}
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this record to output */
				}
			}
		}
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (Ctrl->A.active) {	/* Free special arrays and segment */
		GMT_free_segment (GMT, &Sa, GMT_ALLOCATED_BY_GMT);
		GMT_free (GMT, U);
		GMT_free (GMT, V);
		GMT_free (GMT, W);
		GMT_free (GMT, e);
	}
	for (k = GMT_X; k <= GMT_Z; k++)
		if (Ctrl->W.mode[k]) GMT_free (GMT, w[k]);
	if (Ctrl->T.n)
		GMT_free (GMT, t);
	
	Return (GMT_OK);
}
