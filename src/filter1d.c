/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * API functions to support the filter1d application.
 *
 * Author:	Walter H. F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis:  filter1d_func will read N columns of data from file
 * or stdin and return filtered output at user-selected positions.
 * The time variable can be in any specified column of the input.  Several
 * filters are available, with robustness as an option.
 *
 * Filters:
 * Convolutions: Boxcar, Cosine Arch, Gaussian, Median, or Mode.
 * Geospatial:   Median, Mode, Extreme values.
 * Robust:	Option replaces outliers with medians in filter.
 * Output:	At input times, or from t_start to t_stop by t_int.
 * Lack:	Option checks for data gaps in input series.
 * Symmetry:	Option checks for asymmetry in filter window.
 * Quality:	Option checks for low mean weight in window.
 *
 */

#include "gmt.h"

/* Control structure for filter1d */

struct FILTER1D_CTRL {
	struct D {	/* -D<inc> */
		GMT_LONG active;
		double inc;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct F {	/* -F<type><width>[<mode>] */
		GMT_LONG active;
		char filter;	/* Character codes for the filter */
		double width;
		GMT_LONG mode;
		char *file;	/* Character codes for the filter */
	} F;
	struct I {	/* -I<ignoreval> */
		GMT_LONG active;	/* TRUE when input values that equal value should be discarded */
		double value;
	} I;
	struct L {	/* -L<lackwidth> */
		GMT_LONG active;
		double value;
	} L;
	struct N {	/* -N<t_col> */
		GMT_LONG active;
		GMT_LONG col;
	} N;
	struct Q {	/* -Q<factor> */
		GMT_LONG active;
		double value;
	} Q;
	struct S {	/* -S<symmetry> */
		GMT_LONG active;
		double value;
	} S;
	struct T {	/* -T[<tmin/tmax/t_inc>] */
		GMT_LONG active;
		double min, max, inc;
	} T;
};

#define FILTER1D_BOXCAR		0
#define FILTER1D_COS_ARCH	1
#define FILTER1D_GAUSSIAN	2
#define FILTER1D_CUSTOM		3
#define FILTER1D_MEDIAN		4
#define FILTER1D_MODE		5
#define FILTER1D_LOWER_ALL	6
#define FILTER1D_LOWER_POS	7
#define FILTER1D_UPPER_ALL	8
#define FILTER1D_UPPER_NEG	9
#define FILTER1D_N_FILTERS	10
#define FILTER1D_CONVOLVE	3		/* If filter_type > FILTER1D_CONVOLVE then a FILTER1D_MEDIAN, FILTER1D_MODE, or EXTREME filter is selected  */

struct FILTER1D_INFO {	/* Control structure for all aspects of the filter setup */
	GMT_LONG use_ends;		/* True to start/stop at ends of series instead of 1/2 width inside  */
	GMT_LONG check_asym;		/* TRUE to test whether the data are asymmetric about the output time  */
	GMT_LONG check_lack;		/* TRUE to test for lack of data (gap) in the filter window */
	GMT_LONG check_q;		/* TRUE to test average weight or N in median */
	GMT_LONG robust;		/* Look for outliers in data when TRUE */
	GMT_LONG equidist;		/* Data is evenly sampled in t */
	GMT_LONG out_at_time;		/* TRUE when output is required at evenly spaced intervals */
	GMT_LONG f_operator;		/* TRUE if custom weights coefficients sum to zero */

	GMT_LONG *n_this_col;		/* Pointer to array of counters [one per column]  */
	GMT_LONG *n_left;		/* Pointer to array of counters [one per column]  */
	GMT_LONG *n_right;		/* Pointer to array of counters [one per column]  */
	GMT_LONG n_rows;		/* Number of rows of input  */
	GMT_LONG n_cols;		/* Number of columns of input  */
	GMT_LONG t_col;			/* Column of time abscissae (independent variable)  */
	GMT_LONG n_f_wts;		/* Number of filter weights  */
	GMT_LONG half_n_f_wts;		/* Half the number of filter weights  */
	GMT_LONG n_row_alloc;		/* Number of rows of data to allocate  */
	GMT_LONG n_work_alloc;		/* Number of rows of workspace to allocate  */
	GMT_LONG filter_type;		/* Flag indicating desired filter type  */
	GMT_LONG kind;			/* -1 skip +ve, +1 skip -ve, else use all  [for the l|L|u|U filter] */
	GMT_LONG way;			/* -1 find minimum, +1 find maximum  [for the l|L|u|U filter] */
	GMT_LONG mode_selection;
	GMT_LONG n_multiples;

	double *f_wt;			/* Pointer for array of filter coefficients  */
	double *min_loc;		/* Pointer for array of values, one per [column]  */
	double *max_loc;
	double *last_loc;
	double *this_loc;
	double *min_scl;
	double *max_scl;
	double *last_scl;
	double *this_scl;
	double **work;			/* Pointer to array of pointers to doubles for work  */
	double **data;			/* Pointer to array of pointers to doubles for data  */
	double dt;			/* Delta time resolution for filter computation  */
	double q_factor;		/* Quality level for mean weights or n in median  */
	double filter_width;		/* Full width of filter in user's units */
	double half_width;
	double t_start;			/* x-value of first output point */
	double t_stop;			/* x-value of last output point */
	double t_start_t;		/* user specified x-value of first output point if out_at_time == TRUE */
	double t_stop_t;		/* user specified x-value of last output point if out_at_time == TRUE */
	double t_int;			/* Output interval */
	double sym_coeff;		/* Symmetry coefficient  */
	double lack_width;		/* Lack of data width  */
	double extreme;			/* Extreme value [for the l|L|u|U filter] */

	struct GMT_DATASET *Fin;	/* Pointer to table with custom weight coefficients (optional) */
};

void *New_filter1d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct FILTER1D_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct FILTER1D_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_filter1d_Ctrl (struct GMT_CTRL *GMT,struct FILTER1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->F.file) free (C->F.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_filter1d_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "filter1d %s [API] - Do time domain filtering of 1-D data tables\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: filter1d [<table>] -F<type><width>[<mode>] [-D<increment>] [-E] [-I<ignore_val>]\n");
	GMT_message (GMT, "\t[-L<lack_width>] [-N<t_col>] [-Q<q_factor>] [-S<symmetry>] [-T<start>/<stop>/<int>]\n");
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-F Set filtertype.  Choose from convolution and non-convolution filters\n");
	GMT_message (GMT, "\t   and append filter <width> in same units as time column.\n");
	GMT_message (GMT, "\t   Convolution filters:\n");
	GMT_message (GMT, "\t     b: Boxcar : Weights are equal.\n");
	GMT_message (GMT, "\t     c: Cosine arch : Weights given by cosine arch.\n");
	GMT_message (GMT, "\t     g: Gaussian : Weights given by Gaussian function.\n");
	GMT_message (GMT, "\t     f<name>: Custom : Weights given in one-column file <name>.\n");
	GMT_message (GMT, "\t   Non-convolution filters:\n");
	GMT_message (GMT, "\t     m: Median : Return the median value.\n");
	GMT_message (GMT, "\t     p: Maximum likelihood probability (mode) estimator : Return the mode.\n");
	GMT_message (GMT, "\t        By default, we return the average mode if more than one is found.\n");
	GMT_message (GMT, "\t        Append - or + to the width to return the smallest or largest mode instead.\n");
	GMT_message (GMT, "\t     l: Lower : Return minimum of all points.\n");
	GMT_message (GMT, "\t     L: Lower+ : Return minimum of all positive points.\n");
	GMT_message (GMT, "\t     u: Upper : Return maximum of all points.\n");
	GMT_message (GMT, "\t     U: Upper- : Return maximum of all negative points.\n");
	GMT_message (GMT, "\t   Upper case type B, C, G, M, P, F will use robust filter versions,\n");
	GMT_message (GMT, "\t   i.e., replace outliers (2.5 L1 scale off median) with median during filtering.\n");

	GMT_message (GMT, "\n\tOPTIONS:\n");

	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-D Set fixed increment when series is NOT equidistantly sampled.\n");
	GMT_message (GMT, "\t   Then <increment> will be the abscissae resolution, i.e. all abscissae\n");
	GMT_message (GMT, "\t   will be rounded off to a multiple of <increment>.\n");
	GMT_message (GMT, "\t-E Include ends of time series in output [Default loses half_width at each end].\n");
	GMT_message (GMT, "\t-I Ignore values; If an input value == <ignore_val> it will be set to NaN.\n");
	GMT_message (GMT, "\t-L Check for lack of data condition.  If input data has a gap exceeding\n");
	GMT_message (GMT, "\t   <width> then no output will be given at that point [Default does not check Lack].\n");
	GMT_message (GMT, "\t-N Set the column that contains the independent variable (time) [0].\n");
	GMT_message (GMT, "\t   The left-most column is # 0, the right-most is # (<n_cols> - 1).\n");
	GMT_message (GMT, "\t-Q Sssess quality of output value by checking mean weight in convolution.\n");
	GMT_message (GMT, "\t   Enter <q_factor> between 0 and 1.  If mean weight < q_factor, output is\n");
	GMT_message (GMT, "\t   suppressed at this point [Default does not check quality].\n");
	GMT_message (GMT, "\t-S Check symmetry of data about window center.  Enter a factor\n");
	GMT_message (GMT, "\t   between 0 and 1.  If ( (abs(n_left - n_right)) / (n_left + n_right) ) > factor,\n");
	GMT_message (GMT, "\t   then no output will be given at this point [Default does not check Symmetry].\n");
	GMT_message (GMT, "\t-T Make evenly spaced timesteps from <start> to <stop> by <int> [Default uses input times].\n");
	GMT_explain_options (GMT, "VC0D0fghio.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_filter1d_parse (struct GMTAPI_CTRL *C, struct FILTER1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to filter1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	char c, txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Get fixed increment */
				Ctrl->D.inc = atof (opt->arg);
				Ctrl->D.active = TRUE;
				break;
			case 'E':	/* Include ends of series */
				Ctrl->E.active = TRUE;
				break;
			case 'F':	/* Filter selection  */
				Ctrl->F.active = TRUE;
				if (opt->arg[0] && strchr ("BbCcGgMmPpLlUuFf", opt->arg[0])) {	/* OK filter code */
					Ctrl->F.filter = opt->arg[0];
					Ctrl->F.width = atof (&opt->arg[1]);
					n_errors += GMT_check_condition (GMT, Ctrl->F.width <= 0.0, "Syntax error -F option: Filterwidth must be positive\n");
					switch (Ctrl->F.filter) {	/* Get some futher info from some filters */
						case 'P':
						case 'p':
							c = opt->arg[strlen(opt->arg-1)];
							if (c == '-') Ctrl->F.mode = -1;
							if (c == '+') Ctrl->F.mode = +1;
							break;
						case 'F':
						case 'f':
							Ctrl->F.width = DBL_MAX;	/* To avoid range test errors before reading coefficients */
							if (opt->arg[1] && !GMT_access (GMT, &opt->arg[1], R_OK))
								Ctrl->F.file = strdup (&opt->arg[1]);
							else {
								GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F[Ff] option: Could not find file %s.\n", &opt->arg[1]);
								++n_errors;
							}
							break;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F option: Correct syntax: -FX<width>, X one of BbCcGgMmPpFflLuU\n");
					++n_errors;
				}
				break;
			case 'I':	/* Activate the ignore option */
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Check for lack of data */
				Ctrl->L.active = TRUE;
				Ctrl->L.value = atof (opt->arg);
				break;
			case 'N':	/* Select column with independent coordinate [0] */
				Ctrl->N.active = TRUE;
#ifdef GMT_COMPAT
				if (strchr (opt->arg, '/')) { /* Gave obsolete format */
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -N<ncol>/<tcol> option is deprecated; use -N<tcol> instead.\n");
					if (sscanf (opt->arg, "%*s/%" GMT_LL "d", &Ctrl->N.col) != 1) {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -N option: Syntax is -N<tcol>\n");
						++n_errors;
					}
				}
				else if (!strchr (opt->arg, '/'))
					Ctrl->N.col = atoi (opt->arg);
#else
					Ctrl->N.col = atoi (opt->arg);
#endif
				break;
			case 'Q':	/* Assess quality of output */
				Ctrl->Q.value = atof (opt->arg);
				Ctrl->Q.active = TRUE;
				break;
			case 'S':	/* Activate symmetry test */
				Ctrl->S.active = TRUE;
				Ctrl->S.value = atof (opt->arg);
				break;
			case 'T':	/* Set output knots */
				Ctrl->T.active = TRUE;
				if (sscanf (opt->arg, "%[^/]/%[^/]/%lf", txt_a, txt_b, &Ctrl->T.inc) != 3) {
					GMT_report (GMT, GMT_MSG_FATAL, "Suntax error -T option: Syntax is -T<start>/<stop>/<inc>\n");
					++n_errors;
				}
				else {
					GMT_scanf_arg (GMT, txt_a, GMT_IS_UNKNOWN, &Ctrl->T.min);
					GMT_scanf_arg (GMT, txt_b, GMT_IS_UNKNOWN, &Ctrl->T.max);
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check arguments */

	n_errors += GMT_check_condition (GMT, !Ctrl->F.active, "Syntax error: -F is required\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->D.inc <= 0.0, "Syntax error -D: must give positive increment\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && (Ctrl->T.max - Ctrl->T.min) < Ctrl->F.width, "Syntax error -T option: Output interval < filterwidth\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && (Ctrl->L.value < 0.0 || Ctrl->L.value > Ctrl->F.width) , "Syntax error -L option: Unreasonable lack-of-data interval\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.value < 0.0 || Ctrl->S.value > 1.0) , "Syntax error -S option: Enter a factor between 0 and 1\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.col < 0, "Syntax error -N option: Time column cannot be negative.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && (Ctrl->Q.value < 0.0 || Ctrl->Q.value > 1.0), "Syntax error -Q option: Enter a factor between 0 and 1\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Various functions which will be accessed via pointers depending on chosen filter */

double boxcar_weight (double radius, double half_width)
{
	return ((radius > half_width) ? 0.0 : 1.0);
}

double cosine_weight_filter1d (double radius, double half_width)
{
	return ((radius > half_width) ? 0.0 : 1.0 + cos (radius * M_PI / half_width));
}

double gaussian_weight (double radius, double half_width)
{
	return ((radius > half_width) ? 0.0 : exp (-4.5 * radius * radius / (half_width * half_width)));
}

void allocate_data_space (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F)
{
	GMT_LONG i;

	for (i = 0; i < F->n_cols; ++i) F->data[i] = GMT_memory (GMT, F->data[i], F->n_row_alloc, double);
}

void allocate_more_work_space (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F)
{
	GMT_LONG i;

	for (i = 0; i < F->n_cols; ++i) F->work[i] = GMT_memory (GMT, F->work[i], F->n_work_alloc, double);
}

GMT_LONG set_up_filter (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F)
{
	GMT_LONG i, i1, i2, normalize = FALSE;
	double t_0, t_1, time, w_sum;
	PFD get_weight[3];		/* Selects desired weight function.  */

	t_0 = F->data[F->t_col][0];
	t_1 = F->data[F->t_col][F->n_rows-1];
	if (F->equidist) F->dt = (t_1 - t_0) / (F->n_rows - 1);

	if (F->filter_type == FILTER1D_CUSTOM) {	/* Use coefficients we read from file */
		F->n_f_wts = F->Fin->n_records;
		while (F->n_f_wts <= F->n_work_alloc) {	/* Need more memory */
			F->n_work_alloc <<= 1;
			allocate_more_work_space (GMT, F);
		}
		F->f_wt = GMT_memory (GMT, F->f_wt, F->n_f_wts, double);
		GMT_memcpy (F->f_wt, F->Fin->table[0]->segment[0]->coord[GMT_X], F->n_f_wts, double);
		for (i = 0, w_sum = 0.0; i < F->n_f_wts; ++i) w_sum += F->f_wt[i];
		F->f_operator = (GMT_IS_ZERO (w_sum));	/* If weights sum to zero it is an operator like {-1 1] or [1 -2 1] */
		F->half_n_f_wts = F->n_f_wts / 2;
		F->half_width = F->half_n_f_wts * F->dt;
		F->filter_width = 2.0 * F->half_width;
	}
	else if (F->filter_type <= FILTER1D_CONVOLVE) {
		get_weight[FILTER1D_BOXCAR] = (PFD)boxcar_weight;
		get_weight[FILTER1D_COS_ARCH] = (PFD)cosine_weight_filter1d;
		get_weight[FILTER1D_GAUSSIAN] = (PFD)gaussian_weight;
		F->half_width = 0.5 * F->filter_width;
		F->half_n_f_wts = (GMT_LONG)floor (F->half_width / F->dt);
		F->n_f_wts = 2 * F->half_n_f_wts + 1;

		F->f_wt = GMT_memory (GMT, F->f_wt, F->n_f_wts, double);
		for (i = 0; i <= F->half_n_f_wts; ++i) {
			time = i * F->dt;
			i1 = F->half_n_f_wts - i;
			i2 = F->half_n_f_wts + i;
			F->f_wt[i1] = F->f_wt[i2] = ( *get_weight[F->filter_type]) (time, F->half_width);
		}
		if (normalize) {
			w_sum = 0.0;
			for (i = 0; i < F->n_f_wts; ++i) w_sum += F->f_wt[i];
			for (i = 0; i < F->n_f_wts; ++i) F->f_wt[i] /= w_sum;
		}
	}
	else
		F->half_width = 0.5 * F->filter_width;

	/* Initialize start/stop time */

	if (F->out_at_time) {
		/* populate F->t_start and F->t_stop */
		double	t_shift;
		if (F->t_start_t < t_0) /* user defined t_start_t outside bounds */
			F->t_start = t_0;
		else
			F->t_start = F->t_start_t;
		if (F->t_stop_t > t_1) /* user defined t_stop_t outside bounds */
			F->t_stop = t_1;
		else
			F->t_stop = F->t_stop_t;

		if (!F->use_ends) {
			/* remove filter half width from bounds */
			F->t_start += F->half_width;
			F->t_stop -= F->half_width;
		}

		/* align F->t_start and F->t_stop to F->t_int */
		t_shift = F->t_int - fmod (F->t_start - F->t_start_t, F->t_int);
		if ( fabs (t_shift - F->t_int) < GMT_SMALL ) t_shift=0; /* avoid values close to F->t_int */
		F->t_start += t_shift; /* make F->t_start - F->t_start_t an integral multiple of F->t_int */
		t_shift = fmod (F->t_stop - F->t_start_t, F->t_int);
		if ( fabs (t_shift - F->t_int) < GMT_SMALL ) t_shift=0; /* avoid values close to F->t_int */
		F->t_stop -= t_shift; /* make F->t_stop - F->t_start_t an integral multiple of F->t_int */
	}
	else {
		if (F->use_ends) {
			F->t_start = t_0;
			F->t_stop = t_1;
		}
		else {
			for (i = 0; (F->data[F->t_col][i] - t_0) < F->half_width; ++i);
			F->t_start = F->data[F->t_col][i];
			for (i = F->n_rows - 1; (t_1 - F->data[F->t_col][i]) < F->half_width; --i);
			F->t_stop = F->data[F->t_col][i];
		}
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "F width: %g Resolution: %g Start: %g Stop: %g\n", F->filter_width, F->dt, F->t_start, F->t_stop);

	return (0);
}

GMT_LONG lack_check (struct FILTER1D_INFO *F, GMT_LONG i_col, GMT_LONG left, GMT_LONG right)
{
	GMT_LONG last_row, this_row, lacking = FALSE;
	double last_t;

	last_row = left;
	while (!(GMT_is_dnan (F->data[i_col][last_row])) && last_row < (right - 1)) ++last_row;

	last_t = F->data[F->t_col][last_row];
	this_row = last_row + 1;
	while (!(lacking) && this_row < (right - 1)) {
		while (!(GMT_is_dnan (F->data[i_col][this_row])) && this_row < (right - 1)) ++this_row;

		if ( (F->data[F->t_col][this_row] - last_t) > F->lack_width)
			lacking = TRUE;
		else {
			last_t = F->data[F->t_col][this_row];
			last_row = this_row;
			++this_row;
		}
	}
	return (lacking);
}

void get_robust_estimates (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F, GMT_LONG j, GMT_LONG n, GMT_LONG both)
{
	GMT_LONG i, n_smooth, sort_me = TRUE;
	double low, high, last, temp;

	if (F->filter_type > FILTER1D_MODE)
		temp = GMT_extreme (GMT, F->work[j], n, F->extreme, F->kind, F->way);
	else if (F->filter_type == FILTER1D_MODE) {
		n_smooth = n / 2;
		GMT_mode (GMT, F->work[j], n, n_smooth, sort_me, F->mode_selection, &F->n_multiples, &temp);
	}
	else {
		low = F->min_loc[j];
		high = F->max_loc[j];
		last = F->last_loc[j];

		GMT_median (GMT, F->work[j], n, low, high, last, &temp);
	}

	F->last_loc[j] = F->this_loc[j] = temp;

	if (both) {
		for (i = 0; i < n; ++i) F->work[j][i] = fabs (F->work[j][i] - F->this_loc[j]);
		low = F->min_scl[j];
		high = F->max_scl[j];
		last = F->last_scl[j];
		GMT_median (GMT, F->work[j], n, low, high, last, &temp);
		F->last_scl[j] = F->this_scl[j] = temp;
	}
}

GMT_LONG do_the_filter (struct GMTAPI_CTRL *C, struct FILTER1D_INFO *F)
{
	GMT_LONG i_row, left, right, n_l, n_r, iq, i_f_wt, i_col;
	GMT_LONG i_t_output = 0, n_in_filter, n_for_call, n_good_ones;
	GMT_LONG *good_one = NULL;	/* Pointer to array of logicals [one per column]  */
	double time, delta_time, *outval = NULL, wt, val, med, scl, small;
	double *wt_sum = NULL;		/* Pointer for array of weight sums [each column]  */
	double *data_sum = NULL;	/* Pointer for array of data * weight sums [columns]  */
	struct GMT_CTRL *GMT = C->GMT;

	outval = GMT_memory (GMT, NULL, F->n_cols, double);
	good_one = GMT_memory (GMT, NULL, F->n_cols, GMT_LONG);
	wt_sum = GMT_memory (GMT, NULL, F->n_cols, double);
	data_sum = GMT_memory (GMT, NULL, F->n_cols, double);

	if(!F->out_at_time) {	/* Position i_t_output at first output time  */
		for(i_t_output = 0; F->data[F->t_col][i_t_output] < F->t_start; ++i_t_output);
		small = (F->data[F->t_col][1] - F->data[F->t_col][0]);
	}
	else
		small = F->t_int;

	small *= GMT_CONV_LIMIT;
	time = F->t_start;
	left = right = 0;		/* Left/right end of filter window */

	iq = irint (F->q_factor);

	while (time <= (F->t_stop + small)) {
		while ((time - F->data[F->t_col][left] - small) > F->half_width) ++left;
		while (right < F->n_rows && (F->data[F->t_col][right] - time - small) <= F->half_width) ++right;
		n_in_filter = right - left;
		if ( (!(n_in_filter)) || (F->check_lack && ( (F->filter_width / n_in_filter) > F->lack_width) ) ) {
			if (F->out_at_time)
				time += F->t_int;
			else {
				++i_t_output;
				time = (i_t_output < F->n_rows) ? F->data[F->t_col][i_t_output] : F->t_stop + 1.0;
			}
			continue;
		}

		for (i_col = 0; i_col < F->n_cols; ++i_col) {
			F->n_this_col[i_col] = 0;
			wt_sum[i_col] = data_sum[i_col] = 0.0;
			if (i_col == F->t_col)
				good_one[i_col] = FALSE;
			else if (F->check_lack)
				good_one[i_col] = !(lack_check (F, i_col, left, right));
			else
				good_one[i_col] = TRUE;
			if (F->check_asym) F->n_left[i_col] = F->n_right[i_col] = 0;
		}

		if (F->robust || F->filter_type > FILTER1D_CONVOLVE) {
			if (n_in_filter > F->n_work_alloc) {
				F->n_work_alloc = n_in_filter;
				allocate_more_work_space (GMT, F);
			}
			for (i_row = left; i_row < right; ++i_row) {
				for (i_col = 0; i_col < F->n_cols; ++i_col) {
					if (!(good_one[i_col])) continue;
					if (!GMT_is_dnan (F->data[i_col][i_row])) {
						F->work[i_col][F->n_this_col[i_col]] = F->data[i_col][i_row];
						F->n_this_col[i_col]++;
						if (F->check_asym) {
							if (F->data[F->t_col][i_row] < time) F->n_left[i_col]++;
							if (F->data[F->t_col][i_row] > time) F->n_right[i_col]++;
						}
					}
				}
			}
			if (F->check_asym) {
				for (i_col = 0; i_col < F->n_cols; ++i_col) {
					if (!(good_one[i_col])) continue;
					n_l = F->n_left[i_col];
					n_r = F->n_right[i_col];
					if ((((double)GMT_abs (n_l - n_r))/(n_l + n_r)) > F->sym_coeff) good_one[i_col] = FALSE;
				}
			}
			if ((F->filter_type > FILTER1D_CONVOLVE) && F->check_q) {
				for (i_col = 0; i_col < F->n_cols; ++i_col) {
					if (F->n_this_col[i_col] < iq) good_one[i_col] = FALSE;
				}
			}

			for (i_col = 0; i_col < F->n_cols; ++i_col) {
				if (good_one[i_col]) {
					n_for_call = F->n_this_col[i_col];
					get_robust_estimates (GMT, F, i_col, n_for_call, F->robust);
				}
			}

		}	/* That's it for the robust work  */

		if (F->filter_type > FILTER1D_CONVOLVE) {

			/* Need to count how many good ones; use data_sum area  */

			n_good_ones = 0;
			for (i_col = 0; i_col < F->n_cols; ++i_col) {
				if (i_col == F->t_col)
					data_sum[i_col] = time;
				else if (good_one[i_col]) {
					data_sum[i_col] = F->this_loc[i_col];
					++n_good_ones;
				}
				else
					data_sum[i_col] = GMT->session.d_NaN;
			}
			if (n_good_ones) GMT_Put_Record (C, GMT_WRITE_DOUBLE, data_sum);
		}
		else {
			if (F->robust) for (i_col = 0; i_col < F->n_cols; ++i_col) F->n_this_col[i_col] = 0;

			for (i_row = left; i_row < right; ++i_row) {
				delta_time = time - F->data[F->t_col][i_row];
				i_f_wt = F->half_n_f_wts + (GMT_LONG)floor (0.5 + delta_time/F->dt);
				if ((i_f_wt < 0) || (i_f_wt >= F->n_f_wts)) continue;

				for(i_col = 0; i_col < F->n_cols; ++i_col) {
					if (!good_one[i_col]) continue;
					if (!GMT_is_dnan (F->data[i_col][i_row])) {
						wt = F->f_wt[i_f_wt];
						val = F->data[i_col][i_row];
						if (F->robust) {
							med = F->this_loc[i_col];
							scl = F->this_scl[i_col];
							val = ((fabs(val-med)) > (2.5 * scl)) ? med : val;
						}
						else if (F->check_asym) {	/* This wasn't already done  */
							if (F->data[F->t_col][i_row] < time) F->n_left[i_col]++;
							if (F->data[F->t_col][i_row] > time) F->n_right[i_col]++;
						}
						wt_sum[i_col] += wt;
						data_sum[i_col] += (wt * val);
						F->n_this_col[i_col]++;
					}
				}
			}
			n_good_ones = 0;
			for (i_col = 0; i_col < F->n_cols; ++i_col) {
				if (!good_one[i_col]) continue;
				if (!F->n_this_col[i_col]) {
					good_one[i_col] = FALSE;
					continue;
				}
				if (F->check_asym && !(F->robust) ) {
					n_l = F->n_left[i_col];
					n_r = F->n_right[i_col];
					if ((((double)GMT_abs (n_l - n_r))/(n_l + n_r)) > F->sym_coeff) {
						good_one[i_col] = FALSE;
						continue;
					}
				}
				if (F->check_q && ((wt_sum[i_col] / F->n_this_col[i_col]) < F->q_factor)) {
					good_one[i_col] = FALSE;
					continue;
				}
				++n_good_ones;
			}
			if (n_good_ones) {
				for (i_col = 0; i_col < F->n_cols; ++i_col) {
					if (i_col == F->t_col)
						outval[i_col] = time;
					else if (good_one[i_col])
						outval[i_col] = (F->f_operator) ? data_sum[i_col] : data_sum[i_col] / wt_sum[i_col];
					else
						outval[i_col] = GMT->session.d_NaN;
				}
				GMT_Put_Record (C, GMT_WRITE_DOUBLE, outval);
			}
		}

		/* Go to next output time */

		if (F->out_at_time)
			time += F->t_int;
		else {
			++i_t_output;
			time = (i_t_output < F->n_rows) ? F->data[F->t_col][i_t_output] : F->t_stop + 1.0;
		}
	}

	GMT_free (GMT, outval);
	GMT_free (GMT, good_one);
	GMT_free (GMT, wt_sum);
	GMT_free (GMT, data_sum);

	return (0);
}

GMT_LONG allocate_space (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F)
{
	F->n_this_col = GMT_memory (GMT, NULL, F->n_cols, GMT_LONG);
	F->data = GMT_memory (GMT, NULL, F->n_cols, double *);

	if (F->check_asym) F->n_left = GMT_memory (GMT, NULL, F->n_cols, GMT_LONG);
	if (F->check_asym) F->n_right = GMT_memory (GMT, NULL, F->n_cols, GMT_LONG);

	if (F->robust || (F->filter_type > FILTER1D_CONVOLVE) ) {	/* Then we need workspace  */
		GMT_LONG i;

		F->work = GMT_memory (GMT, NULL, F->n_cols, double *);
		for (i = 0; i < F->n_cols; ++i) F->work[i] = GMT_memory (GMT, NULL, F->n_work_alloc, double);
		F->min_loc = GMT_memory (GMT, NULL, F->n_cols, double);
		F->max_loc = GMT_memory (GMT, NULL, F->n_cols, double);
		F->last_loc = GMT_memory (GMT, NULL, F->n_cols, double);
		F->this_loc = GMT_memory (GMT, NULL, F->n_cols, double);
		F->min_scl = GMT_memory (GMT, NULL, F->n_cols, double);
		F->max_scl = GMT_memory (GMT, NULL, F->n_cols, double);
		F->this_scl = GMT_memory (GMT, NULL, F->n_cols, double);
		F->last_scl = GMT_memory (GMT, NULL, F->n_cols, double);
	}
	return (0);
}

void free_space_filter1d (struct GMT_CTRL *GMT, struct FILTER1D_INFO *F)
{
	GMT_LONG i;
	if (!F) return;
	if (F->robust || (F->filter_type > FILTER1D_CONVOLVE) ) {
		for (i = 0; i < F->n_cols; ++i)	GMT_free (GMT, F->work[i]);
		GMT_free (GMT, F->work);
	}
	for (i = 0; i < F->n_cols; ++i)	GMT_free (GMT, F->data[i]);
	GMT_free (GMT, F->data);
	GMT_free (GMT, F->n_this_col);
	if (F->check_asym) GMT_free (GMT, F->n_left);
	if (F->check_asym) GMT_free (GMT, F->n_right);
	if (F->min_loc) GMT_free (GMT, F->min_loc);
	if (F->max_loc) GMT_free (GMT, F->max_loc);
	if (F->last_loc) GMT_free (GMT, F->last_loc);
	if (F->this_loc) GMT_free (GMT, F->this_loc);
	if (F->min_scl) GMT_free (GMT, F->min_scl);
	if (F->max_scl) GMT_free (GMT, F->max_scl);
	if (F->last_scl) GMT_free (GMT, F->last_scl);
	if (F->this_scl) GMT_free (GMT, F->this_scl);
	if (F->n_f_wts) GMT_free (GMT, F->f_wt);
}

void load_parameters_filter1d (struct FILTER1D_INFO *F, struct FILTER1D_CTRL *Ctrl, GMT_LONG n_cols)
{
	F->filter_width = Ctrl->F.width;
	F->dt = Ctrl->D.inc;
	F->equidist = !Ctrl->D.active;
	F->use_ends = Ctrl->E.active;
	F->check_lack = Ctrl->L.active;
	F->lack_width = Ctrl->L.value;
	F->n_cols = n_cols;
	F->t_col = Ctrl->N.col;
	F->q_factor = Ctrl->Q.value;
	F->check_q = Ctrl->Q.active;
	F->check_asym = Ctrl->S.active;
	F->sym_coeff = Ctrl->S.value;
	F->t_start_t = F->t_start = Ctrl->T.min;
	F->t_stop_t = F->t_stop = Ctrl->T.max;
	F->t_int =Ctrl->T.inc;
	F->out_at_time = Ctrl->T.active;
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code,...) {Free_filter1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_report (GMT, GMT_MSG_FATAL, __VA_ARGS__); bailout (code);}
#define Return2(code) {Free_filter1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_filter1d (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG i, tbl, seg, row, error;

	double last_time, new_time, in;
	
	struct GMT_OPTION *options = NULL;
	struct FILTER1D_INFO F;
	struct GMT_DATASET *D = NULL;
	struct FILTER1D_CTRL *Ctrl = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_filter1d_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_filter1d_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_filter1d", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghi>" GMT_OPT("HMm"), options)) Return (API->error, "Error parsing filter1d options\n");
	Ctrl = New_filter1d_Ctrl (GMT);		/* Allocate and initialize a new control structure */
	if ((error = GMT_filter1d_parse (API, Ctrl, options))) Return (error, "Error parsing filter1d options\n");

	/*---------------------------- This is the filter1d main code ----------------------------*/

	GMT_memset (&F, 1, struct FILTER1D_INFO);	/* Init control structure to NULL */
	F.n_work_alloc = GMT_CHUNK;
	F.equidist = TRUE;

	/* Read the input data into memory */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Register data input */
		Return (API->error, "Error initializing input\n");
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) {
		Return (API->error, "Error Reading input\n");
	}
	
	load_parameters_filter1d (&F, Ctrl, D->n_columns);	/* Pass parameters from Control structure to Filter structure */

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < F.n_cols) Return (GMT_N_COLS_VARY,
		"Syntax error: Binary input data must have at least %ld fields\n", F.n_cols);

	if (strchr ("BCGMPF", Ctrl->F.filter)) {	/* First deal with robustness request */
		F.robust = TRUE;
		Ctrl->F.filter = (char)tolower ((int)Ctrl->F.filter);
	}
	switch (Ctrl->F.filter) {	/* Set filter parameters */
		case 'b':
			F.filter_type = FILTER1D_BOXCAR;
			break;
		case 'c':
			F.filter_type = FILTER1D_COS_ARCH;
			break;
		case 'g':
			F.filter_type = FILTER1D_GAUSSIAN;
			break;
		case 'm':
			F.filter_type = FILTER1D_MEDIAN;
			break;
		case 'p':
			F.filter_type = FILTER1D_MODE;
			F.mode_selection = Ctrl->F.mode;
			break;
		case 'l':
			F.filter_type = FILTER1D_LOWER_ALL;
			F.way = -1;
			F.extreme = DBL_MAX;
			break;
		case 'L':
			F.filter_type = FILTER1D_LOWER_POS;
			F.way = -1;
			F.kind = +1;
			break;
		case 'u':
			F.filter_type = FILTER1D_UPPER_ALL;
			F.way = +1;
			F.extreme = -DBL_MAX;
			break;
		case 'U':
			F.filter_type = FILTER1D_UPPER_NEG;
			F.way = +1;
			F.kind = -1;
			break;
		case 'f':
			F.filter_type = FILTER1D_CUSTOM;
			if ((error = GMT_set_cols (GMT, GMT_IN, 1))) Return (error, "Error in GMT_set_cols");
			if ((F.Fin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, Ctrl->F.file, NULL)) == NULL) {
				Return (API->error, "Error Reading input\n");
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Read %ld filter weights from file %s.\n", F.Fin->n_records, Ctrl->F.file);
			break;
	}
	if (F.filter_type > FILTER1D_CONVOLVE) F.robust = FALSE;

	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = FALSE;	/* Turn off default GMT NaN-handling */
	GMT->current.io.skip_if_NaN[F.t_col] = TRUE;			/* ... But disallow NaN in "time" column */
	GMT->common.b.ncol[GMT_OUT] = F.n_cols;
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error, "Error initializing input\n");
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error, "Error in Begin_IO\n");
	}

	allocate_space (GMT, &F);	/* Gets column-specific flags and counter space */
	for (tbl = 0; tbl < D->n_tables; ++tbl) {	/* For each input table */
		for (seg = 0; seg < D->table[tbl]->n_segments; ++seg) {	/* For each segment */
			/* Duplicate data and set up arrays and parameters needed to filter this segment */
			if (D->table[tbl]->segment[seg]->n_rows > F.n_row_alloc) {
				F.n_row_alloc = MAX (GMT_CHUNK, D->table[tbl]->segment[seg]->n_rows);
				allocate_data_space (GMT, &F);
			}

			if (F.robust || (F.filter_type == FILTER1D_MEDIAN) ) {
				for (i = 0; i < F.n_cols; ++i) {
					F.min_loc[i] = +DBL_MAX;
					F.max_loc[i] = -DBL_MAX;
				}
			}
			last_time = -DBL_MAX;

			for (row = F.n_rows = 0; row < D->table[tbl]->segment[seg]->n_rows; ++row, ++F.n_rows) {
				in = D->table[tbl]->segment[seg]->coord[F.t_col][row];
				if (GMT_is_dnan (in)) continue;	/* Skip records with time == NaN */
				new_time = in;
				if (new_time < last_time) Return (GMT_DATA_READ_ERROR, "Error! Time decreases at line # %ld\n\tUse UNIX utility sort and then try again.\n", row);
				last_time = new_time;
				for (i = 0; i < F.n_cols; ++i) {
					in = D->table[tbl]->segment[seg]->coord[i][row];
					if (Ctrl->I.active && in == Ctrl->I.value)
						F.data[i][F.n_rows] = GMT->session.d_NaN;
					else
						F.data[i][F.n_rows] = in;
					if (F.robust || (F.filter_type == FILTER1D_MEDIAN) ) {
						if (in > F.max_loc[i]) F.max_loc[i] = in;
						if (in < F.min_loc[i]) F.min_loc[i] = in;
					}
				}
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Read %ld records from table %ld, segment %ld\n", F.n_rows, tbl, seg);
			
			/* FILTER: Initialize scale parameters and last_loc based on min and max of data  */

			if (F.robust || (F.filter_type == FILTER1D_MEDIAN) ) {
				for (i = 0; i < F.n_cols; ++i) {
					F.min_scl[i] = 0.0;
					F.max_scl[i] = 0.5 * (F.max_loc[i] - F.min_loc[i]);
					F.last_scl[i] = 0.5 * F.max_scl[i];
					F.last_loc[i] = 0.5 * (F.max_loc[i] + F.min_loc[i]);
				}
			}

			if (set_up_filter (GMT, &F)) Return (GMT_RUNTIME_ERROR, "Fatal error during coefficient setup.\n");

			if (GMT->current.io.multi_segments[GMT_OUT]) GMT_Put_Record (API, GMT_WRITE_SEGHEADER, D->table[tbl]->segment[seg]->header);
			
			if (do_the_filter (API, &F)) Return (GMT_RUNTIME_ERROR, "Fatal error in filtering routine.\n");
		}
	}
	
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error, "Error in End_IO\n");
	}

	if (F.n_multiples > 0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %ld multiple modes found\n", F.n_multiples);

	free_space_filter1d (GMT, &F);

	Return2 (GMT_OK);
}
