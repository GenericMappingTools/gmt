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
 * Brief synopsis: Reads stdin or file of x y z triples, or weighted data, x y z w.  Fit 
 * a regression model z = f(x,y) + e, where e are error misfits and f(x,y)
 * has some user-prescribed functional form.  The user may choose the number
 * of terms in the model to fit, whether to seek iterative refinement robust
 * w.r.t. outliers, and whether to seek automatic discovery of the significant
 * number of model parameters.
 *
 * During model fitting the data x,y coordinates are normalized into the domain
 * [-1, 1] for Chebyshev Polynomial fitting.  Before writing out the data the
 * coordinates are rescaled to match the original input values.
 *
 * Author:	W. H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"trend2d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Fit a [weighted] [robust] polynomial for z = f(x,y) to xyz[w] data"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbfhis" GMT_OPT("H")

#define TREND2D_N_OUTPUT_CHOICES 6

struct TREND2D_CTRL {
	unsigned int n_outputs;
	bool weighted_output;
	struct C {	/* -C<condition_#> */
		bool active;
		double value;
	} C;
	struct F {	/* -F<xymrw> */
		bool active;
		char col[TREND2D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} F;
	struct I {	/* -I[<confidence>] */
		bool active;
		double value;
	} I;
	struct N {	/* -N<n_model>[r] */
		bool active;
		bool robust;
		unsigned int value;
	} N;
	struct W {	/* -W */
		bool active;
	} W;
};

struct TREND2D_DATA {
	double	x;
	double	y;
	double	z;
	double	m;
	double	r;
	double	w;
};

int read_data_trend2d (struct GMT_CTRL *GMT, struct TREND2D_DATA **data, uint64_t *n_data, double *xmin, double *xmax, double *ymin, double *ymax, int weighted_input, double **work)
{
	uint64_t i;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	double *in = NULL;

	*data = GMT_memory (GMT, NULL, n_alloc, struct TREND2D_DATA);

	i = 0;
	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (GMT->parent, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
	
		(*data)[i].x = in[GMT_X];
		(*data)[i].y = in[GMT_Y];
		(*data)[i].z = in[GMT_Z];
		(*data)[i].w = (weighted_input) ? in[3] : 1.0;

		if (i) {
			if (*xmin > (*data)[i].x) *xmin = (*data)[i].x;
			if (*xmax < (*data)[i].x) *xmax = (*data)[i].x;
			if (*ymin > (*data)[i].y) *ymin = (*data)[i].y;
			if (*ymax < (*data)[i].y) *ymax = (*data)[i].y;
		}
		else {
			*xmin = (*data)[i].x;
			*xmax = (*data)[i].x;
			*ymin = (*data)[i].y;
			*ymax = (*data)[i].y;
		}

		if (++i == n_alloc) {
			n_alloc <<= 1;
			*data = GMT_memory (GMT, *data, n_alloc, struct TREND2D_DATA);
		}
	} while (true);
	*data = GMT_memory (GMT, *data, i, struct TREND2D_DATA);
	*work = GMT_memory (GMT, NULL, i, double);
	*n_data = i;
	return (0);
}

void allocate_the_memory_2d (struct GMT_CTRL *GMT, uint64_t np, double **gtg, double **v, double **gtd, double **lambda, double **workb, double **workz, double **c_model, double **o_model, double **w_model)
{
	*gtg = GMT_memory (GMT, NULL, np*np, double);
	*v = GMT_memory (GMT, NULL, np*np, double);
	*gtd = GMT_memory (GMT, NULL, np, double);
	*lambda = GMT_memory (GMT, NULL, np, double);
	*workb = GMT_memory (GMT, NULL, np, double);
	*workz = GMT_memory (GMT, NULL, np, double);
	*c_model = GMT_memory (GMT, NULL, np, double);
	*o_model = GMT_memory (GMT, NULL, np, double);
	*w_model = GMT_memory (GMT, NULL, np, double);
}

void write_output_trend2d (struct GMT_CTRL *GMT, struct TREND2D_DATA *data, uint64_t n_data, char *output_choice, unsigned int n_outputs)
{
	uint64_t i;
	unsigned int j;
	double out[6];

	for (i = 0; i < n_data; i++) {
		for (j = 0; j < n_outputs; j++) {
			switch (output_choice[j]) {
				case 'x':
					out[j] = data[i].x;
					break;
				case 'y':
					out[j] = data[i].y;
					break;
				case 'z':
					out[j] = data[i].z;
					break;
				case 'm':
					out[j] = data[i].m;
					break;
				case 'r':
					out[j] = data[i].r;
					break;
				case 'w':
					out[j] = data[i].w;
					break;
			}
		}
		GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);	/* Write this to output */
	}
}

void free_the_memory_2d (struct GMT_CTRL *GMT, double *gtg, double *v, double *gtd, double *lambda, double *workb, double *workz, double *c_model, double *o_model, double *w_model, struct TREND2D_DATA *data, double *work)
{
	GMT_free (GMT, work);
	GMT_free (GMT, data);
	GMT_free (GMT, w_model);
	GMT_free (GMT, o_model);
	GMT_free (GMT, c_model);
	GMT_free (GMT, workz);
	GMT_free (GMT, workb);
	GMT_free (GMT, lambda);
	GMT_free (GMT, gtd);
	GMT_free (GMT, v);
	GMT_free (GMT, gtg);
}

void transform_x_2d (struct TREND2D_DATA *data, uint64_t n_data, double xmin, double xmax, double ymin, double ymax)
{
	uint64_t i;
	double offsetx, scalex;
	double offsety, scaley;

	offsetx = 0.5 * (xmin + xmax);	/* Mid Range  */
	offsety = 0.5 * (ymin + ymax);
	scalex  = 2.0 / (xmax - xmin);	/* 1 / (1/2 Range)  */
	scaley  = 2.0 / (ymax - ymin);

	for (i = 0; i < n_data; i++) {
		data[i].x = (data[i].x - offsetx) * scalex;
		data[i].y = (data[i].y - offsety) * scaley;
	}
}

void untransform_x_2d (struct TREND2D_DATA *data, uint64_t n_data, double xmin, double xmax, double ymin, double ymax)
{
	uint64_t i;
	double offsetx, scalex;
	double offsety, scaley;

	offsetx = 0.5 * (xmin + xmax);	/* Mid Range  */
	offsety = 0.5 * (ymin + ymax);
	scalex  = 0.5 * (xmax - xmin);	/* 1/2 Range  */
	scaley  = 0.5 * (ymax - ymin);

	for (i = 0; i < n_data; i++) {
		data[i].x = (data[i].x * scalex) + offsetx;
		data[i].y = (data[i].y * scaley) + offsety;
	}
}

double get_chisq_2d (struct TREND2D_DATA *data, uint64_t n_data, unsigned int n_model)
{
	uint64_t i, nu;
	double chi = 0.0;

	for (i = 0; i < n_data; i++) {	/* Weight is already squared  */
		if (data[i].w == 1.0)
			chi += (data[i].r * data[i].r);
		else
			chi += (data[i].r * data[i].r * data[i].w);
	}
	nu = n_data - n_model;
	if (nu > 1) return (chi/nu);
	return (chi);
}

void recompute_weights_2d (struct GMT_CTRL *GMT, struct TREND2D_DATA *data, uint64_t n_data, double *work, double *scale)
{
	uint64_t i;
	double k, ksq, rr;

	/* First find median { fabs(data[].r) },
	   estimate scale from this,
	   and compute chisq based on this.  */ 

	for (i = 0; i < n_data; i++) work[i] = fabs(data[i].r);
	GMT_sort_array (GMT, work, n_data, GMT_DOUBLE);

	if (n_data%2)
		*scale = 1.4826 * work[n_data/2];
	else
		*scale = 0.7413 * (work[n_data/2 - 1] + work[n_data/2]);

	k = 1.5 * (*scale);	/*  Huber[1964] weight; 95% efficient for Normal data  */
	ksq = k * k;

	for (i = 0; i < n_data; i++) {
		rr = fabs(data[i].r);
		data[i].w = (rr <= k) ? 1.0 : (2*k/rr) - (ksq/(rr*rr) );	/* This is really w-squared  */
	}
}

void load_g_row_2d (double x, double y, unsigned int n, double *gr)
{
	/* Current data position, appropriately normalized.  */
	/* Number of model parameters, and elements of gr[]  */
	/* Elements of row of G matrix.  */
	/* Routine computes the elements gr[j] in the ith row of the
	   G matrix (Menke notation), where x,y is the ith datum's
	   location.  */

	unsigned int j;

	j = 0;
	while (j < n) {
		switch (j) {
			case 0:
				gr[j] = 1.0;
				break;
			case 1:
				gr[j] = x;
				break;
			case 2:
				gr[j] = y;
				break;
			case 3:
				gr[j] = x*y;
				break;
			case 4:
				gr[j] = 2 * x * gr[1] - gr[0];
				break;
			case 5:
				gr[j] = 2 * y * gr[2] - gr[0];
				break;
			case 6:
				gr[j] = 2 * x * gr[4] - gr[1];
				break;
			case 7:
				gr[j] = gr[4] * gr[2];
				break;
			case 8:
				gr[j] = gr[5] * gr[1];
				break;
			case 9:
				gr[j] = 2 * y * gr[5] - gr[2];
				break;
		}
		j++;
	}
}

void calc_m_and_r_2d (struct TREND2D_DATA *data, uint64_t n_data, double *model, unsigned int n_model, double *grow)
{
	/*	model[n_model] holds solved coefficients of m_type model.
		grow[n_model] is a vector for a row of G matrix.  */

	uint64_t i;
	unsigned int j;
	for (i = 0; i < n_data; i++) {
		load_g_row_2d (data[i].x, data[i].y, n_model, grow);
		data[i].m = 0.0;
		for (j = 0; j < n_model; j++) data[i].m += model[j]*grow[j];
		data[i].r = data[i].z - data[i].m;
	}
}

void move_model_a_to_b_2d (double *model_a, double *model_b, unsigned int n_model, double *chisq_a, double *chisq_b)
{
	unsigned int i;
	for (i = 0; i < n_model; i++) model_b[i] = model_a[i];
	*chisq_b = *chisq_a;
}

void load_gtg_and_gtd_2d (struct GMT_CTRL *GMT_UNUSED(GMT), struct TREND2D_DATA *data, uint64_t n_data, double *gtg, double *gtd, double *grow, unsigned int n_model, unsigned int mp)
{	/* mp is row dimension of gtg  */

	uint64_t i;
	unsigned int j, k;
	double wz;

	/* First zero the contents for summing */

	for (j = 0; j < n_model; j++) {
		for (k = 0; k < n_model; k++) gtg[j + k*mp] = 0.0;
		gtd[j] = 0.0;
	}

	/* Sum over all data  */
	for (i = 0; i < n_data; i++) {
		load_g_row_2d (data[i].x, data[i].y, n_model, grow);
		if (data[i].w != 1.0) {
			wz = data[i].w * data[i].z;
			for (j = 0; j < n_model; j++) {
				for (k = 0; k < n_model; k++) gtg[j + k*mp] += (data[i].w * grow[j] * grow[k]);
				gtd[j] += (wz * grow[j]);
			}
		}
		else {
			for (j = 0; j < n_model; j++) {
				for (k = 0; k < n_model; k++) gtg[j + k*mp] += (grow[j] * grow[k]);
				gtd[j] += (data[i].z * grow[j]);
			}
		}
	}
}

void solve_system_2d (struct GMT_CTRL *GMT, double *gtg, double *gtd, double *model, unsigned int n_model, unsigned int mp, double *lambda, double *v, double *b, double *z, double c_no, unsigned int *ir)
{

	unsigned int i, j, k, rank = 0, nrots;
	double c_test, temp_inverse_ij;

	if (n_model == 1) {
		model[0] = gtd[0] / gtg[0];
		*ir = 1;
	}
	else {
		if (GMT_jacobi (GMT, gtg, n_model, mp, lambda, v, b, z, &nrots)) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Matrix Solver Convergence Failure.\n");
		}
		c_test = fabs (lambda[0]) / c_no;
		while (rank < n_model && lambda[rank] > 0.0 && lambda[rank] > c_test) rank++;
		for (i = 0; i < n_model; i++) {
			model[i] = 0.0;
			for (j = 0; j < n_model; j++) {
				temp_inverse_ij = 0.0;
				for (k = 0; k < rank; k++) {
					temp_inverse_ij += (v[i + k*mp] * v[j + k*mp] / lambda[k]);
				}
				model[i] += (temp_inverse_ij * gtd[j]);
			}
		}
		*ir = rank;
	}
}

void *New_trend2d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TREND2D_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct TREND2D_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.value = 1.0e06;		/* Condition number for matrix solution  */	
	C->I.value = 0.51;		/* Confidence interval for significance test  */
	return (C);
}

void Free_trend2d_Ctrl (struct GMT_CTRL *GMT, struct TREND2D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_trend2d_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: trend2d [<table>] -F<xyzmrw> -N<n_model>[r] [-C<condition_#>] [-I[<confidence>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-F Choose at least 1, up to 6, any order, of xyzmrw for ascii output to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x=x, y=y, z=z, m=model, r=residual=z-m, w=weight (determined iteratively if robust fit used).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Fit a [robust] model with <n_model> terms.  <n_model> in [1,10].  E.g., robust planar = -N3r.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Model parameters order is given as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z = m1 + m2*x + m3*y + m4*x*y + m5*x^2 + m6*y^2 + m7*x^3 + m8*x^2*y + m9*x*y^2 + m10*y^3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data files (in ASCII, binary, netCDF) with (x,y,z[,w]) data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Truncate eigenvalue spectrum so matrix has <condition_#> [Default = 1.0e06].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Iteratively Increase # model parameters, to a max of <n_model> so long as the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   reduction in variance is significant at the <confidence> level.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I without a number to default to 0.51 confidence level.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Weighted input given, weights in 4th column [Default is unweighted].\n");
	GMT_Option (API, "bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 (or 4 if -W is set) columns.\n");
	GMT_Option (API, "bo,f,h,i,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_trend2d_parse (struct GMT_CTRL *GMT, struct TREND2D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to trend2d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = true;
				for (j = 0; opt->arg[j]; j++) {
					if (j < TREND2D_N_OUTPUT_CHOICES)
						Ctrl->F.col[j] = opt->arg[j];
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Too many output columns selected: Choose from -Fxyzmrw\n");
						n_errors++;
					}
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0]) Ctrl->I.value = atof (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = true;
				if (strchr (opt->arg, 'r')) Ctrl->N.robust = true;
				j = (opt->arg[0] == 'r') ? 1 : 0;
				Ctrl->N.value = (opt->arg[j]) ? atoi (&opt->arg[j]) : 0;
				break;
			case 'W':
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->C.value <= 1.0, "Syntax error -C option: Condition number must be larger than unity\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.value < 0.0 || Ctrl->I.value > 1.0, "Syntax error -C option: Give 0 < confidence level < 1.0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.value <= 0 || Ctrl->N.value > 10, "Syntax error -N option: Must request 1-10 parameters\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);

	for (j = Ctrl->n_outputs = 0; j < TREND2D_N_OUTPUT_CHOICES && Ctrl->F.col[j]; j++) {
		if (!strchr ("xyzmrw", Ctrl->F.col[j])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Unrecognized output choice %c\n", Ctrl->F.col[j]);
			n_errors++;
		}
		else if (Ctrl->F.col[j] == 'w')
			Ctrl->weighted_output = true;

		Ctrl->n_outputs++;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs == 0, "Syntax error -F option: Must specify at least one output column\n");
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs > TREND2D_N_OUTPUT_CHOICES, "Syntax error -F option: Too many output columns specified (%d)\n", Ctrl->n_outputs);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_trend2d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_trend2d (void *V_API, int mode, void *args)
{
	unsigned int i, n_model, rank, np;
	int error = 0;
	bool significant;
	
	uint64_t n_data;

	double *gtg = NULL, *v = NULL, *gtd = NULL, *lambda = NULL, *workb = NULL;
	double *workz = NULL, *c_model = NULL, *o_model = NULL, *w_model = NULL, *work = NULL;
	double xmin, xmax, ymin, ymax, c_chisq, o_chisq = 0.0, w_chisq, scale = 1.0, prob;

	char format[GMT_BUFSIZ];

	struct TREND2D_DATA *data = NULL;
	struct TREND2D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_trend2d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_trend2d_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_trend2d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_trend2d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_trend2d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the trend2d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	sprintf (format, "%s%s", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator);

	GMT->common.R.wesn[XLO] = 0;	GMT->common.R.wesn[XHI] = 360.0;	/* For -L not to cause trouble in GMT->current.io.input */
	np = Ctrl->N.value;	/* Row dimension for matrices gtg and v  */
	allocate_the_memory_2d (GMT, np, &gtg, &v, &gtd, &lambda, &workb, &workz, &c_model, &o_model, &w_model);

	if ((error = GMT_set_cols (GMT, GMT_IN, 3 + Ctrl->W.active)) != GMT_OK) {
		Return (error);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, Ctrl->n_outputs)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if ((error = read_data_trend2d (GMT,&data, &n_data, &xmin, &xmax, &ymin, &ymax, Ctrl->W.active, &work))) Return (error);
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (xmin == xmax || ymin == ymax) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Maximum and minimum input values are the same.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not read any data.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data < (uint64_t)Ctrl->N.value) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Ill-posed problem; n_data < n_model_max.\n");
	}

	transform_x_2d (data, n_data, xmin, xmax, ymin, ymax);	/* Set domain to [-1, 1] or [-pi, pi]  */

	GMT_Report (API, GMT_MSG_VERBOSE, "Read %" PRIu64 " data with X values from %.8g to %.8g\n", n_data, xmin, xmax);
	GMT_Report (API, GMT_MSG_VERBOSE, "N_model%sRank%sChi_Squared%sSignificance\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);

	sprintf (format, "%%d%s%%d%s%s%s%s\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);

	if (Ctrl->I.active) {
		n_model = 1;

		/* Fit first model  */
		load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
		solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
		c_chisq = get_chisq_2d (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.robust) {
			do {
				recompute_weights_2d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_2d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
				solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
				c_chisq = get_chisq_2d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_2d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_2d (GMT, data, n_data, work, &scale);
			}
		}
		/* First [robust] model has been found  */

		significant = true;
		while (n_model < Ctrl->N.value && significant) {
			move_model_a_to_b_2d (c_model, o_model, n_model, &c_chisq, &o_chisq);
			n_model++;

			/* Fit next model  */
			load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
			solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
			calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
			c_chisq = get_chisq_2d (data, n_data, n_model);
			GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.0);
			if (Ctrl->N.robust) {
				do {
					recompute_weights_2d (GMT, data, n_data, work, &scale);
					move_model_a_to_b_2d (c_model, w_model, n_model, &c_chisq, &w_chisq);
					load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
					solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
					calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
					c_chisq = get_chisq_2d (data, n_data, n_model);
					significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
					GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
				} while (significant);
				/* Go back to previous model only if w_chisq < c_chisq  */
				if (w_chisq < c_chisq) {
					move_model_a_to_b_2d (w_model, c_model, n_model, &w_chisq, &c_chisq);
					calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
					if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_2d (GMT, data, n_data, work, &scale);
				}
			}
			/* Next [robust] model has been found  */
			significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, o_chisq, n_data-n_model-1, Ctrl->I.value, &prob);
		}

		if (!significant) {	/* Go back to previous [robust] model, stored in o_model  */
			n_model--;
			rank--;
			move_model_a_to_b_2d (o_model, c_model, n_model, &o_chisq, &c_chisq);
			calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
			if (Ctrl->N.robust && Ctrl->weighted_output) recompute_weights_2d (GMT, data, n_data, work, &scale);
		}
	}
	else {
		n_model = Ctrl->N.value;
		load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
		solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
		c_chisq = get_chisq_2d (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.robust) {
			do {
				recompute_weights_2d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_2d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_2d (GMT,data, n_data, gtg, gtd, workb, n_model, np);
				solve_system_2d (GMT,gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
				c_chisq = get_chisq_2d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_2d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_2d (data, n_data, c_model, n_model, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_2d (GMT, data, n_data, work, &scale);
			}
		}
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "Final model stats: N model parameters %%d.  Rank %%d.  Chi-Squared: %s\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq);
		GMT_Report (API, GMT_MSG_VERBOSE, "Model Coefficients: ");
		sprintf (format, "%s%s", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator);
		for (i = 0; i < n_model; i++) {
			GMT_Message (API, GMT_TIME_NONE, format, c_model[i]);
		}
		GMT_Message (API, GMT_TIME_NONE, "\n");
	}

	untransform_x_2d (data, n_data, xmin, xmax, ymin, ymax);

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	write_output_trend2d (GMT,data, n_data, Ctrl->F.col, Ctrl->n_outputs);
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	free_the_memory_2d (GMT,gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);

	Return (GMT_OK);
}
