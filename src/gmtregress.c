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
 * API functions to support the gmtconvert application.
 *
 * Author:	Paul Wessel
 * Date:	10-JAN-2015
 * Version:	5 API
 *
 * Brief synopsis: gmtregress fits y = ax + b linear regression to x,y[,w] data.
 */

#define THIS_MODULE_NAME	"gmtregress"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Linear regression of 1-D data sets"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vabdfghios"

#define GMTREGRESS_N_FARGS	6
#define REG_X		0
#define REG_Y		1
#define REG_XY		2
#define REG_RMA		3
#define NORM_L1		0
#define NORM_L2		1
#define NORM_LMS	2

#define P_ANGLE		0
#define P_MISFT		1
#define P_SLOPE		2
#define P_ICEPT		3
#define P_SIGSL		4
#define P_SIGIC		5

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
	struct E {	/* 	-Ex|y|o|m */
		bool active;
		unsigned int mode;
	} E;
	struct F {	/* 	-Fxymrcw */
		bool active;
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
	struct W {	/* 	-W[s] */
		bool active;
		unsigned int mode;
	} W;
};

void *New_gmtregress_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTREGRESS_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTREGRESS_CTRL);
	C->A.min = -90.0;	C->A.max = 90.0;	C->A.inc = 1.0;
	C->C.value = 0.95;
	C->E.mode = REG_Y;
	C->N.mode = NORM_L2;
	
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
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<min>/<max>/<inc> | -T<n>] [%s] [-W[s]] [%s] [%s]\n", GMT_V_OPT, GMT_a_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Examine E as function of line slope; give angle range and increment [-90/+90/1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The -F is not required as no model is returned, just E vs angle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Select confidence level (%%) to use in calculations [95].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Select how misfit should be measured:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : Horizontally from data point to regression line\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : Vertically from data point to regression line [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     o : Orthogonally from data point to regression line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : Use Reduced Major Axis area misfit instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Append desired output columns in any order; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x : The x observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y : The y observations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m : The estimated model values.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : The estimated residuals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c : The confidence limits (m += c).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     w : The reweighted Least Square weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is xymrcw].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Append desired misfit norm; choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 : Use the L-1 measure (mean absolute residuals).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2 : Use L2 Mean Squared residuals measure [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r : Use robust Least Median of Squared residuals measure.\n");
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

	unsigned int n_errors = 0, j, k, n, n_files = 0;
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
			case 'C':	/* Set confidence level */
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg) * 0.01;
				break;
			case 'E':	/* Select E measure */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'x': Ctrl->E.mode = REG_X; break; /* Regress on x */
					case 'y': Ctrl->E.mode = REG_Y; break; /* Regress on y */
					case 'o': Ctrl->E.mode = REG_XY; break; /* Orthogonal Regression*/
					case 'r': Ctrl->E.mode = REG_RMA; break; /* RMA Regression*/
					default: n_errors++; break;
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				for (j = 0, k = 0; opt->arg[j]; j++, k++) {
					if (k < GMTREGRESS_N_FARGS) {
						Ctrl->F.col[k] = opt->arg[j];
						if (!strchr ("xymrcw", opt->arg[j])) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F option: Choose from -Fxymrcw\n");
							n_errors++;
						}
						Ctrl->F.n_cols++;
					}
					else {
						n_errors++;
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F option: Too many output columns selected\n");
					}
				}
				break;
			case 'N':	/* Select Norm */
				Ctrl->N.active = true;
				if (opt->arg[0] == '1')
					Ctrl->N.mode = NORM_L1;
				else if (opt->arg[0] == '2')
					Ctrl->N.mode = NORM_L2;
				else if (opt->arg[0] == 'r')
					Ctrl->N.mode = NORM_LMS;
				else
					n_errors++;
				break;
			case 'T':	/* Output lattice */
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
				if (opt->arg[0] == 's') Ctrl->W.mode = 1;	/* Got sigmas */
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, Ctrl->C.value < 0.0 || Ctrl->C.value >= 1.0, "Syntax error -C: Level must be in 0 < 1 range.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->A.active, "Syntax error -A: Cannot also specify -T.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->F.active, "Syntax error -A: Cannot also specify -F.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->C.active, "Syntax error -A: Cannot also specify -C.\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 2 columns.\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

double model (double x0, double *result)
{	/* Evalute the model */
	return (result[0] * x0 + result[1]);
}

double GMT_sum (double *x, uint64_t n)
{
	uint64_t k;
	double S = 0.0;
	for (k = 0; k < n; k++) S += x[k];
	return (S);
}

double icept (struct GMT_CTRL *GMT, double *e, uint64_t n, unsigned int norm)
{	/* Return the proper "average" value given chosen norm */
	unsigned int GMT_n_multiples = 0;
	double X, *ee = NULL;
	
	if (norm != NORM_L2) {	/* Need temporary space for scaled residuals */
		ee = GMT_memory (GMT, NULL, n, double);
		GMT_memcpy (ee, e, n, double);
		GMT_sort_array (GMT, ee, n, GMT_DOUBLE);
	}
	switch (norm) {
		case NORM_L1:	/* Return median */
		 	X = (n%2) ? ee[n/2] : 0.5 * (e[(n-1)/2] + ee[n/2]);
			break;
		case NORM_L2:	/* Return mean */
			X = GMT_sum (e, n) / n;
			break;
		case NORM_LMS:	/* Return mode */
			GMT_mode (GMT, ee, n, n/2, 0L, -1L, &GMT_n_multiples, &X);
			break;
	}
	if (norm != NORM_L2) GMT_free (GMT, ee);
			
	return (X);	
}

double L1_misfit (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, uint64_t n, unsigned int regression, double slope)
{	/* Compute L1 misfit in x|y|o|r (for r we take sqrt of area) */
	uint64_t k;
	double f, E = 0.0;
	slope = fabs (slope);
	switch (regression) {	/* Scale that turns abs y-misfit into other misfit measures */
		case REG_X: f = 1.0 / slope; break;
		case REG_Y: f = 1.0; break;
		case REG_XY: f = sqrt (1.0 / (1.0 + slope * slope)); break;
		case REG_RMA: f = sqrt (1.0 / slope); break;
	}
	for (k = 0; k < n; k++) E += fabs (ey[k]);
	return (f * E / n);
}

double L2_misfit (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, uint64_t n, unsigned int regression, double slope)
{	/* Compute L2 misfit in x|y|o|r */
	uint64_t k;
	double f, E = 0.0;
	slope = fabs (slope);
	switch (regression) {	/* Scale that turns squared y-misfit into other misfit measures */
		case REG_X: f = 1.0 / (slope * slope); break;
		case REG_Y: f = 1.0; break;
		case REG_XY: f = 1.0 / (1.0 + slope * slope); break;
		case REG_RMA: f = 1.0 / slope; break;
	}
	
	for (k = 0; k < n; k++) E += ey[k] * ey[k];
	return (f * E / n);
}

double LMS_misfit (struct GMT_CTRL *GMT, double *ey, uint64_t n, unsigned int regression, double slope)
{	/* Compute LMS misfit in x|y|o|r */
	uint64_t k;
	double f, E, *ee = GMT_memory (GMT, NULL, n, double);
	slope = fabs (slope);
	switch (regression) {	/* Scale that turns squared y-misfit into other misfit measures */
		case REG_X: f = 1.0 / (slope * slope); break;
		case REG_Y: f = 1.0; break;
		case REG_XY: f = 1.0 / (1.0 + slope * slope); break;
		case REG_RMA: f = 1.0 / slope; break;
	}
	for (k = 0; k < n; k++) ee[k] = ey[k] * ey[k];
 	GMT_sort_array (GMT, ee, n, GMT_DOUBLE);
	E = (n%2) ? ee[n/2] : 0.5 * (ee[(n-1)/2] + ee[n/2]);
	GMT_free (GMT, ee);
	return (f * E);
}

#if 0
void normalizer (double *x, uint64_t n, double *par)
{	/* Normalizes x so the range is from -1 to +1.
 	 * par[0] holds min value, par[1] holds max, and par[2] holds half-range
	 * Recover original = (x' + 1) * par[2] + par[0].  */
	uint64_t k;
	par[0] = DBL_MAX;	par[1] = -DBL_MAX;
	for (k = 0; k < n; k++) {	/* Find min/max */
		if (GMT_is_dnan (x[k])) continue;
		if (x[k] < par[0]) par[0] = x[k];
		if (x[k] > par[1]) par[1] = x[k];
	}
	par[2] = 0.5 * (par[1] - par[0]);
	for (k = 0; k < n; k++) x[k] = (x[k] - par[0]) / par[2] - 1.0;
}
#endif

void GMT_prod (double *x, double *y, double *xy, uint64_t n)
{
	uint64_t k;
	for (k = 0; k < n; k++) xy[k] = x[k] * y[k];
}

void GMT_add (double *x, double c, double *out, uint64_t n)
{
	uint64_t k;
	for (k = 0; k < n; k++) out[k] = x[k] + c;
}

void LSy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w, uint64_t n, double *par)
{	/* Basic LS y-regression */
	uint64_t k;
	double *Q = GMT_memory (GMT, NULL, n, double), *P = GMT_memory (GMT, NULL, n, double);
	double S, S_x, S_y, S_xx, S_xy, D;
	
	GMT_memset (par, 6, double);
	if (w) {	/* Weighted regression */
		double *w2 = GMT_memory (GMT, NULL, n, double);
		GMT_prod (w, w, w2, n);
		S = GMT_sum (w2, n);
		GMT_prod (x, w2, Q, n);
		S_x = GMT_sum (Q, n);
		GMT_prod (y, w2, Q, n);
		S_y = GMT_sum (Q, n);
		GMT_prod (x, w, P, n);
		GMT_prod (P, x, Q, n);
		S_xx = GMT_sum (Q, n);
		GMT_prod (P, y, Q, n);
		S_xy = GMT_sum (Q, n);
		GMT_free (GMT, w2);
	}
	else {	/* No weights supplied */
		S = n;
		S_x = GMT_sum (x, n);
		S_y = GMT_sum (y, n);
		GMT_prod (x, x, Q, n);
		S_xx = GMT_sum (Q, n);
		GMT_prod (x, y, Q, n);
		S_xy = GMT_sum (Q, n);
	}
	D = 1.0 / (S * S_xx - S_x * S_x);
	par[P_SLOPE] = (S * S_xy - S_x * S_y) * D;
 	par[P_ICEPT] = (S_xx * S_y - S_x * S_xy) * D;
 	par[P_SIGSL] = sqrt (S * D);
 	par[P_SIGIC] = sqrt (S_xx * D);
	for (k = 0; k < n; k++) Q[k] = y[k] - par[P_SLOPE] * x[k] - par[P_ICEPT];
	par[P_MISFT] = L2_misfit (GMT, Q, n, REG_Y, 0.0);
	par[P_ANGLE] = atan (par[P_SLOPE]) * R2D;
	GMT_free (GMT, Q);
	GMT_free (GMT, P);
}

void LSxy_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w, uint64_t n, double *par)
{	/* Basic LS xy-regression */
	uint64_t k;
	unsigned int p;
	double *u = GMT_memory (GMT, NULL, n, double), *v = GMT_memory (GMT, NULL, n, double), *Q = GMT_memory (GMT, NULL, n, double);
	double mean_x, mean_y, sig_x, sig_y, sum_u2, sum_v2, sum_uv, part1, part2, a[2], b[2], E[2], r, LSY[4];
	
	LSy_regress1D (GMT, x, y, w, n, LSY);	/* First get basic LS estimate */
	GMT_memset (par, 6, double);
	mean_x = GMT_mean_and_std (GMT, x, n, &sig_x);
	mean_y = GMT_mean_and_std (GMT, y, n, &sig_y);
	/* Normalize the data */
	GMT_add (x, -mean_x, u, n);
	GMT_add (y, -mean_y, v, n);
	GMT_prod (u, u, Q, n);
	sum_u2 = GMT_sum (Q, n);
	GMT_prod (v, v, Q, n);
	sum_v2 = GMT_sum (Q, n);
	GMT_prod (u, v, Q, n);
	sum_uv = GMT_sum (Q, n);
	GMT_free (GMT, u);	GMT_free (GMT, v);
	part1 = sum_v2 - sum_u2;
	part2 = sqrt (pow (sum_u2 - sum_v2, 2.0) + 4.0 * sum_uv * sum_uv);
	b[0] = (part1 + part2) / (2.0 * sum_uv);
	b[1] = (part1 - part2) / (2.0 * sum_uv);
	r = sum_uv / sqrt (sum_u2 * sum_v2);
	for (p = 0; p < 2; p++) {	/* Compute E for both solutions */
		a[p] = mean_y - b[p] * mean_x;
		for (k = 0; k < n; k++) Q[k] = y[k] - b[p] * x[k] - a[p];
		E[p] = L2_misfit (GMT, Q, n, REG_XY, b[p]);
	}
	p = (E[0] < E[1]) ? 0 : 1;	/* Determine the solution with smallest misfit */
	par[P_SLOPE] = b[p];
	par[P_ICEPT] = a[p];
	par[P_SIGSL] = par[P_SLOPE] * sqrt ((1.0 - r * r) / n) / r;
	par[P_SIGIC] = sqrt (pow (sig_y - sig_x * par[P_SLOPE], 2.0) / n + (1.0 - r) * par[P_SLOPE] * (2.0 * sig_x * sig_y + (mean_x * par[P_SLOPE] * (1.0 + r) / (r * r))));
	par[P_MISFT] = E[p];
	par[P_ANGLE] = atan (par[P_SLOPE]) * R2D;
	GMT_free (GMT, Q);
}

void LSRMA_regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w, uint64_t n, double *par)
{	/* Basic LS RMA-regression */
	double mx, sx, my, sy;
	GMT_memset (par, 6, double);
	mx = GMT_mean_and_std (GMT, x, n, &sx);
	my = GMT_mean_and_std (GMT, y, n, &sy);
	par[P_SLOPE] = sy / sx;
	par[P_ICEPT] = my - par[0] * mx;
	par[P_ANGLE] = atan (par[P_SLOPE]) * R2D;
}

void regress1D_sub (struct GMT_CTRL *GMT, double *x, double *y, double *w, double *e, uint64_t n, unsigned int regression, unsigned int norm, double angle, double *par)
{	/* Solve the linear regression for a given angle and chosen misfit and norm */
	uint64_t k;
	double a, b, E;
	double (*misfit) (struct GMT_CTRL *GMT_UNUSED(GMT), double *ey, uint64_t n, unsigned int regression, double slope);
	switch (norm) {
		case NORM_L1: misfit = L1_misfit; break;
		case NORM_L2: misfit = L2_misfit; break;
		case NORM_LMS: misfit = LMS_misfit; break;
	}
	if (GMT_IS_ZERO (fabs (angle) - 90.0)) {	/* Vertical line is a special case */
		b = GMT->session.d_NaN;
		a = icept (GMT, x, n, norm);				/* Determine x-intercept */
		for (k = 0; k < n; k++) e[k] = x[k] - a;		/* Final x-residuals */
		/* For REG_Y a vertical line gives Inf misfit; all others are measured horizontally.  Here
		 * we obtain this by passing e as ex but giving mode REG_Y instead. */
		E = (regression == REG_Y || regression == REG_RMA) ? GMT->session.d_NaN : misfit (GMT, e, n, REG_Y, 0.0);
	}
	else if (GMT_IS_ZERO (angle)) {	/* Horizontal line is a special case */
		b = 0.0;
		a = icept (GMT, y, n, norm);				/* Determine intercept */
		/* For REG_X a horizontal line gives Inf misfit; all others are measured vertically.  Here
		 * we obtain this by passing e as ey but giving mode REG_Y instead. */
		for (k = 0; k < n; k++) e[k] = y[k] - a;		/* Final y-residuals */
		E = (regression == REG_X || regression == REG_RMA) ? GMT->session.d_NaN : misfit (GMT, e, n, REG_Y, 0.0);
	}
	else {	/* Not vertical, we can measure any misfit */
		b = tand (angle);				/* Regression slope */
		for (k = 0; k < n; k++) e[k] = y[k] - b * x[k];	/* Residuals after removing slope */
		a = icept (GMT, e, n, norm);			/* Determine intercept */
		for (k = 0; k < n; k++) e[k] -= a;		/* Final y-residuals */
		E = misfit (GMT, e, n, regression, b);		/* The mean misfit */
	}
	par[0] = angle;	par[1] = E; par[2] = b;	par[3] = a;
}

#define N_A	90

void regress1D (struct GMT_CTRL *GMT, double *x, double *y, double *w, double *e, uint64_t n, unsigned int regression, unsigned int norm, double *par)
{	/* Solve the linear regression for a given angle and chosen misfit and norm */
	uint64_t k;
	bool done = false;
	double a_min = -90.0, a_max = 90.0, angle, r_a, d_a, tpar[6], f, last_E = DBL_MAX;
	
	par[1] = DBL_MAX;
	while (!done) {
		r_a = a_max - a_min;
		d_a = r_a / (double)N_A;
		for (k = 0; k <= N_A; k++) {
			angle = a_min + d_a * k;
			regress1D_sub (GMT, x, y, w, e, n, regression, norm, angle, tpar);
			if (tpar[P_MISFT] < par[P_MISFT]) GMT_memcpy (par, tpar, 6, double);	/* Update best fit so far */
		}
		if (par[P_MISFT] <= last_E && (f = (last_E - par[P_MISFT])/par[P_MISFT]) < GMT_CONV8_LIMIT)
			done = true;
		else {
			a_min = MAX (-90.0, par[P_ANGLE] - 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			a_max = MIN (+90.0, par[P_ANGLE] + 0.25 * r_a);	/* Get a range that is ~-/+ 25% of previous range */
			last_E = par[P_MISFT];
		}
	}
}

void do_regression (struct GMT_CTRL *GMT, double *dx, double *dy, double *w, uint64_t n, unsigned int regression, unsigned int norm, double level, double *par)
{
	/* Solves for the best regression of (dx, dy) given the settings. */
	
	bool flipped;
	double *x = NULL, *y = NULL;
	
	if (regression == REG_X) {	/* Do x on y regression by flipping the input, then recalc par before output */
		x = dy;	y = dx;
		regression = REG_Y;
		flipped = true;
	}
	else {
		x = dx;	y = dy;
		flipped = false;
	}
	double *e = GMT_memory (GMT, NULL, n, double);
	
	switch (regression) {
		case REG_Y:	/* Vertical misfit measure */
			switch (norm) {
				case NORM_L1:	/* L1 regression */
				case NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, w, e, n, regression, norm, par);
					break;
				case NORM_L2:	/* L2 regression y on x has analytic solution */
					LSy_regress1D (GMT, x, y, w, n, par);
					break;
			}
			break;
		case REG_XY:	/* Orthogonal regression */
			switch (norm) {
				case NORM_L1:	/* L1 regression */
				case NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, w, e, n, regression, norm, par);
					break;
				case NORM_L2:	/* L2 orthogonal regression has analytic solution */
					LSxy_regress1D (GMT, x, y, w, n, par);
					break;
			}
			break;
		case REG_RMA:	/* RMA regression */
			switch (norm) {
				case NORM_L1:	/* L1 regression */
				case NORM_LMS:	/* LMS regression */
					regress1D (GMT, x, y, w, e, n, regression, norm, par);
					break;
				case NORM_L2:	/* L2 RMA regression has analytic solution */
					LSRMA_regress1D (GMT, x, y, w, n, par);
					break;
			}
			break;
	}
	if (flipped) {	/* Must transpose x/y results [untested] */
		par[P_ICEPT] = -par[P_ICEPT];
		par[P_SLOPE] = 1.0 / par[P_SLOPE];
		par[P_ANGLE] = atan (par[P_SLOPE]) * R2D;
	}
	GMT_free (GMT, e);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtregress_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtregress (void *V_API, int mode, void *args)
{
	uint64_t k, seg, tbl, col, row, n_rows, n_columns = GMTREGRESS_N_FARGS;
	uint64_t n_try, n_t, n_alloc = 0;
	int error;
	
	double *x = NULL, *dx = NULL, *dy = NULL, *e = NULL, *w = NULL, *t = NULL, *dt = NULL;
	double x_min, x_max, x_mean, y_mean, par[6], out[6];
	
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
		n_rows = n_try = lrint ((Ctrl->A.max - Ctrl->A.min) / Ctrl->A.inc) + 1;
		n_columns = 4;	/* Hardwired to return angle, misfit, slope, intercept */
		e = GMT_memory (GMT, NULL, n_rows, double);
		Sa = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
		GMT_alloc_segment (GMT, Sa, n_rows, n_columns, true);	/* Reallocate fixed temp space for this experiment */
	}
	else {	/* Work up best solution */
		if (!Ctrl->F.active) {	/* Default is all columns */
			Ctrl->F.n_cols = GMTREGRESS_N_FARGS;
			for (col = 0; col < n_columns; col++) Ctrl->F.col[col] = F_default[col];
		}
		n_columns = Ctrl->F.n_cols;
	}
	
	if (Ctrl->T.active) {	/* Evaluate solution on equidistant grid */
		unsigned int bad = 0;
		for (col = 0; col < n_columns; col++) if (strchr ("yrw", (int)Ctrl->F.col[col])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot choose -F%c when -T is in effect\n", (int)Ctrl->F.col[col]);
			bad++;
		}
		if (bad) Return (GMT_RUNTIME_ERROR);
		if (!Ctrl->T.got_n)	/* Compute number of points given fixed range and increment */
			Ctrl->T.n = GMT_get_n (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, 0);
		t = GMT_memory (GMT, NULL, Ctrl->T.n, double);	/* x-values */
		dt = GMT_memory (GMT, NULL, Ctrl->T.n, double);	/* x - mean_x */
	}

	/* Now we are ready to take on some input values */
	/* Allocate memory and read in all the files; each file can have many lines */
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {
		Return (API->error);	/* Establishes data files or stdin */
	}
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

	GMT->current.io.multi_segments[GMT_OUT] = true;
	
	/* Process all tables and segments */
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			n_rows = S->n_rows;

			/* 1. Make sure we have enough memory allocated */
			if (n_rows > n_alloc) {
				n_alloc = n_rows;
				dx = GMT_memory (GMT, dx, n_alloc, double);
				dy = GMT_memory (GMT, dy, n_alloc, double);
				e  = GMT_memory (GMT, e, n_alloc, double);
				if (Ctrl->W.active) w = GMT_memory (GMT, w, n_alloc, double);
			}
			/* 2. Get a copy of the data */
			GMT_memcpy (dx, S->coord[GMT_X], n_rows, double);
			GMT_memcpy (dy, S->coord[GMT_Y], n_rows, double);
			if (Ctrl->W.active) {
				if (Ctrl->W.mode) {	/* Got sigma instead; convert to weights */
					for (row = 0; row < n_rows; row++) w[row] = 1.0 / S->coord[GMT_Z][row];
				}
				else
					GMT_memcpy (w, S->coord[GMT_Z], n_rows, double);
			}
			/* 3. Get mean (x,y) and obtain (dx,dy) instead */
			x_mean = GMT_sum (dx, n_rows) / n_rows;
			y_mean = GMT_sum (dy, n_rows) / n_rows;
			x_min = DBL_MAX;	x_max = -DBL_MAX;
			for (row = 0; row < n_rows; row++) {
				dx[row] -= x_mean;
				if (dx[row] < x_min) x_min = dx[row];
				if (dx[row] > x_max) x_max = dx[row];
				dy[row] -= y_mean;
			}
			/* Here, dx, dy and x_min/x_max are reduced coordinates */
			
			if (Ctrl->A.active) {	/* Explore E vs slope only */
				uint64_t min_row;
				double angle, min_E = DBL_MAX;
				for (row = 0; row < n_try; row++) {	/* For each slope candidate */
					angle = Ctrl->A.min + row * Ctrl->A.inc;	/* Slope in degrees */
					regress1D_sub (GMT, dx, dy, w, e, n_rows, Ctrl->E.mode, Ctrl->N.mode, angle, par);
					if (par[P_MISFT] < min_E) {	/* Update best fit so far */
						min_E = par[P_MISFT];	min_row = row;
					}
					par[P_ICEPT] += y_mean;	/* Add back mean values since intercept otherwise refers to dy */
					for (k = 0; k < 4; k++) Sa->coord[k][row] = par[k];	/* Save the results */
				}
				/* Make segment header with the findings for best regression */
				sprintf (buffer, "Best regression: x0: %g y0: %g angle: %g E: %g slope: %g icept: %g", x_mean, y_mean, Sa->coord[0][min_row],
					Sa->coord[1][min_row], Sa->coord[2][min_row], Sa->coord[3][min_row]);
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);
				for (row = 0; row < n_try; row++) {	/* Output the saved results of the experiment */
					for (k = 0; k < 4; k++) out[k] = Sa->coord[k][row];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
				}
			}
			else {	/* Here we are solving for the best regression */
				double *del_x = NULL;
				do_regression (GMT, dx, dy, w, n_rows, Ctrl->E.mode, Ctrl->N.mode, Ctrl->C.value, par);
				sprintf (buffer, "Best regression: x0: %g y0: %g angle: %g E: %g slope: %g icept: %g", x_mean, y_mean,
					par[P_ANGLE], par[P_MISFT], par[P_SLOPE], par[P_ICEPT]);
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);

				if (Ctrl->T.active) {	/* Evaluate the model at chosen equidistant lattice [Cannot specify yrw in -F] */
					if (Ctrl->T.got_n) {	/* Got n, recompute increments over given range */
						Ctrl->T.min = x_min;	Ctrl->T.max = x_max;
						Ctrl->T.inc = GMT_get_inc (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.n, 0);
					}
					for (k = 0; k < Ctrl->T.n; k++) {
						t[k] = GMT_col_to_x (GMT, k, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, 0, Ctrl->T.n);
						dt[k] = t[k] - x_mean;
					}
					x = t;
					del_x = dt;
					n_t = Ctrl->T.n;
				}
				else {	/* Use the given data point abscissae [-Fy is OK] */
					x = S->coord[GMT_X];
					del_x = dx;
					n_t = n_rows;
				}
				
				/* 3. Evaluate the output columns (Note icept is based on dy, hence y_mean is added for m) */
				for (row = 0; row < n_t; row++) {
					for (col = 0; col < n_columns; col++) {
						switch (Ctrl->F.col[col]) {
							case 'x': out[col] = x[row]; break;	/* Input (or equidistant) x */
							case 'y': out[col] = S->coord[GMT_Y][row]; break;	/* Input y */
							case 'm':		/* Model prediction */
								out[col] = par[P_SLOPE] * del_x[row] + par[P_ICEPT] + y_mean;	break;
							case 'r':		/* Data minus model */
								out[col] = dy[row] - (par[P_SLOPE] * del_x[row] + par[P_ICEPT]);	break;
							case 'c':		/* Model confidence limit */
								out[col] = par[P_SLOPE] * del_x[row] + par[P_ICEPT];	break;
							case 'w':		/* Residual weights [0 or 1 if LMS] */
								out[col] = par[P_SLOPE] * del_x[row] + par[P_ICEPT];	break;
						}
					}
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
				}
			}
			GMT_Report (API, GMT_MSG_VERBOSE, "Points in: %ld Points out: %ld\n", S->n_rows, n_rows);
		}
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (Ctrl->A.active) GMT_free_segment (GMT, &Sa, GMT_ALLOCATED_BY_GMT);
	if (Ctrl->W.active) GMT_free (GMT, w);
	if (Ctrl->T.active) {
		GMT_free (GMT, t);
		GMT_free (GMT, dt);
	}
	GMT_free (GMT, dx);
	GMT_free (GMT, dy);
	GMT_free (GMT, e);
	
	Return (GMT_OK);
}
