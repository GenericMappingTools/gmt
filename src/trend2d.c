/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"trend2d"
#define THIS_MODULE_MODERN_NAME	"trend2d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Fit [weighted] [robust] polynomial for z = f(x,y) to xyz[w] data"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefhiqsw" GMT_OPT("H")

#define TREND2D_N_OUTPUT_CHOICES 6

struct TREND2D_CTRL {
	unsigned int n_outputs;
	bool weighted_output;
	struct TREND2D_C {	/* -C<condition_#> */
		bool active;
		double value;
	} C;
	struct TREND2D_F {	/* -F<xymrw>|p */
		bool active;
		bool report;
		char col[TREND2D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} F;
	struct TREND2D_I {	/* -I[<confidence>] */
		bool active;
		double value;
	} I;
	struct TREND2D_N {	/* -N<n_model>[r] */
		bool active;
		bool robust;
		unsigned int value;
	} N;
	struct TREND2D_W {	/* -W[+s] */
		bool active;
		unsigned int mode;
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

GMT_LOCAL int trend2d_read_data (struct GMT_CTRL *GMT, struct TREND2D_DATA **data, uint64_t *n_data, double *xmin, double *xmax, double *ymin, double *ymax, unsigned int weighted_input, double **work) {
	uint64_t i;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	double *in = NULL;
	struct GMT_RECORD *In = NULL;

	*data = gmt_M_memory (GMT, NULL, n_alloc, struct TREND2D_DATA);

	i = 0;
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (GMT->parent, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}
		if (In == NULL) {	/* Crazy safety valve but it should never get here*/
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Input pointer is NULL where it should not be, aborting\n");
			return (GMT_PTR_IS_NULL);
		}
		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		(*data)[i].x = in[GMT_X];
		(*data)[i].y = in[GMT_Y];
		(*data)[i].z = in[GMT_Z];
		if (weighted_input == 2)	/* Got sigma, set weight = 1/s^2 */
			(*data)[i].w = 1.0 / (in[3] * in[3]);
		else if (weighted_input == 1)	/* Got weight  */
			(*data)[i].w = in[3];
		else	/* Default is unit weight */
			(*data)[i].w = 1.0;

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
			*data = gmt_M_memory (GMT, *data, n_alloc, struct TREND2D_DATA);
		}
	} while (true);
	*data = gmt_M_memory (GMT, *data, i, struct TREND2D_DATA);
	*work = gmt_M_memory (GMT, NULL, i, double);
	*n_data = i;
	return (0);
}

GMT_LOCAL void trend2d_allocate_the_memory (struct GMT_CTRL *GMT, uint64_t np, double **gtg, double **v, double **gtd, double **lambda, double **workb, double **workz, double **c_model, double **o_model, double **w_model) {
	*gtg = gmt_M_memory (GMT, NULL, np*np, double);
	*v = gmt_M_memory (GMT, NULL, np*np, double);
	*gtd = gmt_M_memory (GMT, NULL, np, double);
	*lambda = gmt_M_memory (GMT, NULL, np, double);
	*workb = gmt_M_memory (GMT, NULL, np, double);
	*workz = gmt_M_memory (GMT, NULL, np, double);
	*c_model = gmt_M_memory (GMT, NULL, np, double);
	*o_model = gmt_M_memory (GMT, NULL, np, double);
	*w_model = gmt_M_memory (GMT, NULL, np, double);
}

GMT_LOCAL void trend2d_write_output_trend (struct GMT_CTRL *GMT, struct TREND2D_DATA *data, uint64_t n_data, char *output_choice, unsigned int n_outputs) {
	uint64_t i;
	unsigned int j;
	static double out[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	struct GMT_RECORD Out;

	Out.data = out;	Out.text = NULL;

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
		GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, &Out);	/* Write this to output */
	}
}

GMT_LOCAL void trend2d_free_the_memory (struct GMT_CTRL *GMT, double *gtg, double *v, double *gtd, double *lambda, double *workb, double *workz, double *c_model, double *o_model, double *w_model, struct TREND2D_DATA *data, double *work) {
	gmt_M_free (GMT, work);
	gmt_M_free (GMT, data);
	gmt_M_free (GMT, w_model);
	gmt_M_free (GMT, o_model);
	gmt_M_free (GMT, c_model);
	gmt_M_free (GMT, workz);
	gmt_M_free (GMT, workb);
	gmt_M_free (GMT, lambda);
	gmt_M_free (GMT, gtd);
	gmt_M_free (GMT, v);
	gmt_M_free (GMT, gtg);
}

GMT_LOCAL void trend2d_transform_x (struct TREND2D_DATA *data, uint64_t n_data, double xmin, double xmax, double ymin, double ymax) {
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

GMT_LOCAL void trend2d_untransform_x (struct TREND2D_DATA *data, uint64_t n_data, double xmin, double xmax, double ymin, double ymax) {
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

GMT_LOCAL double trend2d_get_chisq (struct TREND2D_DATA *data, uint64_t n_data, unsigned int n_model) {
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

GMT_LOCAL void trend2d_recompute_weights (struct GMT_CTRL *GMT, struct TREND2D_DATA *data, uint64_t n_data, double *work, double *scale) {
	uint64_t i;
	double k, ksq, rr;

	/* First find median { fabs(data[].r) },
	   estimate scale from this,
	   and compute chisq based on this.  */

	for (i = 0; i < n_data; i++) work[i] = fabs(data[i].r);
	gmt_sort_array (GMT, work, n_data, GMT_DOUBLE);

	if (n_data%2)
		*scale = MAD_NORMALIZE * work[n_data/2];
	else
		*scale = MAD_NORMALIZE * (work[n_data/2 - 1] + work[n_data/2]) / 2;

	k = 1.5 * (*scale);	/*  Huber[1964] weight; 95% efficient for Normal data  */
	ksq = k * k;

	for (i = 0; i < n_data; i++) {
		rr = fabs(data[i].r);
		data[i].w = (rr <= k) ? 1.0 : (2*k/rr) - (ksq/(rr*rr) );	/* This is really w-squared  */
	}
}

GMT_LOCAL void trend2d_load_g_row (double x, double y, unsigned int n, double *gr) {
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

GMT_LOCAL void trend2d_calc_m_and_r (struct TREND2D_DATA *data, uint64_t n_data, double *model, unsigned int n_model, double *grow) {
	/*	model[n_model] holds solved coefficients of m_type model.
		grow[n_model] is a vector for a row of G matrix.  */

	uint64_t i;
	unsigned int j;
	for (i = 0; i < n_data; i++) {
		trend2d_load_g_row (data[i].x, data[i].y, n_model, grow);
		data[i].m = 0.0;
		for (j = 0; j < n_model; j++) data[i].m += model[j]*grow[j];
		data[i].r = data[i].z - data[i].m;
	}
}

GMT_LOCAL void trend2d_move_model_a_to_b (double *model_a, double *model_b, unsigned int n_model, double *chisq_a, double *chisq_b) {
	unsigned int i;
	for (i = 0; i < n_model; i++) model_b[i] = model_a[i];
	*chisq_b = *chisq_a;
}

GMT_LOCAL void trend2d_load_gtg_and_gtd (struct GMT_CTRL *GMT, struct TREND2D_DATA *data, uint64_t n_data, double *gtg, double *gtd, double *grow, unsigned int n_model, unsigned int mp) {
	/* mp is row dimension of gtg  */

	uint64_t i;
	unsigned int j, k;
	double wz;
	gmt_M_unused(GMT);

	/* First zero the contents for summing */

	for (j = 0; j < n_model; j++) {
		for (k = 0; k < n_model; k++) gtg[j + k*mp] = 0.0;
		gtd[j] = 0.0;
	}

	/* Sum over all data  */
	for (i = 0; i < n_data; i++) {
		trend2d_load_g_row (data[i].x, data[i].y, n_model, grow);
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

GMT_LOCAL void trend2d_solve_system (struct GMT_CTRL *GMT, double *gtg, double *gtd, double *model, unsigned int n_model, unsigned int mp, double *lambda, double *v, double *b, double *z, double c_no, unsigned int *ir) {

	unsigned int i, j, k, rank = 0, nrots;
	double c_test, temp_inverse_ij;

	if (n_model == 1) {
		model[0] = gtd[0] / gtg[0];
		*ir = 1;
	}
	else {
		if (gmt_jacobi (GMT, gtg, n_model, mp, lambda, v, b, z, &nrots)) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Matrix Solver Convergence Failure.\n");
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

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TREND2D_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct TREND2D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.value = 1.0e06;		/* Condition number for matrix solution  */
	C->I.value = 0.51;		/* Confidence interval for significance test  */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct TREND2D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -F<xyzmrw>|p -N<n_model>[+r] [-C<condition_#>] [-I[<confidence>]] "
		"[%s] [-W[+s|w]] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_q_OPT, GMT_s_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, -2, "Note: Input must provide (x,y,z[,w]) records; see -W for weights.");
	GMT_Usage (API, 1, "\n-F<xyzmrw>|p");
	GMT_Usage (API, -2, "Choose at least 1, up to 6, any order, of xyzmrw for output to standard output.:");
	GMT_Usage (API, 3, "x: The x-coordinate.");
	GMT_Usage (API, 3, "y: The y-coordinate.");
	GMT_Usage (API, 3, "z: The z-value.");
	GMT_Usage (API, 3, "m: The model prediction.");
	GMT_Usage (API, 3, "r: The residual (z-m).");
	GMT_Usage (API, 3, "w: The weight (determined iteratively if robust fit used).");
	GMT_Usage (API, -2, "Alternatively, use -Fp by itself to report the model coefficients only.");
	GMT_Usage (API, 1, "\n-N<n_model>[+r]");
	GMT_Usage (API, -2, "Fit a [robust] model with <n_model> terms, with <n_model> in [1,10].  Model parameters order is given as follows: "
		"z = m1 + m2*x + m3*y + m4*x*y + m5*x^2 + m6*y^2 + m7*x^3 + m8*x^2*y + m9*x*y^2 + m10*y^3.");
	GMT_Usage (API, 3, "+r Require a robust solution. E.g., robust planar = -N3+r.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C<condition_#>");
	GMT_Usage (API, -2, "Truncate eigenvalue spectrum so matrix has <condition_#> [Default = 1.0e06].");
	GMT_Usage (API, 1, "\n-I[<confidence>]");
	GMT_Usage (API, -2, "Iteratively Increase # model parameters, to a max of <n_model> so long as the "
		"reduction in variance is significant at the <confidence> level [0.51].");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W[+s|w]");
	GMT_Usage (API, -2, "Weighted input given, weights in 4th column [Default is unweighted]. Select modifier:");
	GMT_Usage (API, 3, "+s Read standard deviations and compute weights as 1/s^2.");
	GMT_Usage (API, 3, "+w Read weights directly [Default].");
	GMT_Option (API, "bi");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Default is 3 (or 4 if -W is set) columns.");
	GMT_Option (API, "bo,d,e,f,h,i,q,s,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct TREND2D_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'C':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				for (j = 0; opt->arg[j]; j++) {
					if (j < TREND2D_N_OUTPUT_CHOICES)
						Ctrl->F.col[j] = opt->arg[j];
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Too many output columns selected: Choose from -Fxyzmrw\n");
						n_errors++;
					}
				}
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (opt->arg[0]) Ctrl->I.value = atof (opt->arg);
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				if (strchr (opt->arg, 'r')) Ctrl->N.robust = true;
				j = (opt->arg[0] == 'r') ? 1 : 0;	/* GMT4 backwardness */
				Ctrl->N.value = (opt->arg[j]) ? atoi (&opt->arg[j]) : 0;
				break;
			case 'W':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'W', "sw", GMT_MSG_ERROR)) n_errors++;
				Ctrl->W.mode = (strstr (opt->arg, "+s")) ? 2 : 1;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.value <= 1.0, "Option -C: Condition number must be larger than unity\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.value < 0.0 || Ctrl->I.value > 1.0, "Option -C: Give 0 < confidence level < 1.0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.value <= 0 || Ctrl->N.value > 10, "Option -N: Must request 1-10 parameters\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);

	for (j = Ctrl->n_outputs = 0; j < TREND2D_N_OUTPUT_CHOICES && Ctrl->F.col[j]; j++) {
		if (!strchr ("xyzmrwp", Ctrl->F.col[j])) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -F: Unrecognized output choice %c\n", Ctrl->F.col[j]);
			n_errors++;
		}
		else if (Ctrl->F.col[j] == 'w')
			Ctrl->weighted_output = true;
		else if (Ctrl->F.col[j] == 'p')
			Ctrl->F.report = true;

		Ctrl->n_outputs++;
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->n_outputs == 0, "Option -F: Must specify at least one output column\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->n_outputs > TREND2D_N_OUTPUT_CHOICES, "Option -F: Too many output columns specified (%d)\n", Ctrl->n_outputs);
	n_errors += gmt_M_check_condition (GMT, Ctrl->n_outputs > 1 && Ctrl->F.report,
					"Option -Fp: When selecting model parameters, it must be the only output\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_trend2d (void *V_API, int mode, void *args) {
	unsigned int i, n_model, rank, np;
	int error = 0;
	bool significant;

	uint64_t n_data;

	double *gtg = NULL, *v = NULL, *gtd = NULL, *lambda = NULL, *workb = NULL;
	double *workz = NULL, *c_model = NULL, *o_model = NULL, *w_model = NULL, *work = NULL;
	double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0, c_chisq, o_chisq = 0.0, w_chisq, scale = 1.0, prob;

	char format[GMT_BUFSIZ];

	struct TREND2D_DATA *data = NULL;
	struct TREND2D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the trend2d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	sprintf (format, "%s%s", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator);

	GMT->common.R.wesn[XLO] = 0;	GMT->common.R.wesn[XHI] = 360.0;	/* For -L not to cause trouble in GMT->current.io.input */
	np = Ctrl->N.value;	/* Row dimension for matrices gtg and v  */
	trend2d_allocate_the_memory (GMT, np, &gtg, &v, &gtd, &lambda, &workb, &workz, &c_model, &o_model, &w_model);

	if ((error = GMT_Set_Columns (API, GMT_IN, 3 + Ctrl->W.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (error);
	}
	i = (Ctrl->F.report) ? np : Ctrl->n_outputs;
	if ((error = GMT_Set_Columns (API, GMT_OUT, i, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (API->error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (API->error);
	}
	if ((error = trend2d_read_data (GMT,&data, &n_data, &xmin, &xmax, &ymin, &ymax, Ctrl->W.mode, &work)) != 0) {
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (error);
	}
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (API->error);
	}

	if (xmin == xmax || ymin == ymax) {
		GMT_Report (API, GMT_MSG_ERROR, "Maximum and minimum input values are the same.\n");
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (GMT_RUNTIME_ERROR);
	}
	if (n_data == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "Could not read any data.\n");
		trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);
		Return (GMT_RUNTIME_ERROR);
	}
	if (n_data < (uint64_t)Ctrl->N.value) {
		GMT_Report (API, GMT_MSG_ERROR, "Ill-posed problem; n_data < n_model_max.\n");
	}

	trend2d_transform_x (data, n_data, xmin, xmax, ymin, ymax);	/* Set domain to [-1, 1] or [-pi, pi]  */

	GMT_Report (API, GMT_MSG_INFORMATION, "Read %" PRIu64 " data with X values from %.8g to %.8g\n", n_data, xmin, xmax);
	GMT_Report (API, GMT_MSG_INFORMATION, "N_model%sRank%sChi_Squared%sSignificance\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);

	sprintf (format, "%%d%s%%d%s%s%s%s\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);

	if (Ctrl->I.active) {
		n_model = 1;

		/* Fit first model  */
		trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
		trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
		c_chisq = trend2d_get_chisq (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.robust) {
			do {
				trend2d_recompute_weights (GMT, data, n_data, work, &scale);
				trend2d_move_model_a_to_b (c_model, w_model, n_model, &c_chisq, &w_chisq);
				trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
				trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
				c_chisq = trend2d_get_chisq (data, n_data, n_model);
				significant = gmt_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				trend2d_move_model_a_to_b (w_model, c_model, n_model, &w_chisq, &c_chisq);
				trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) trend2d_recompute_weights (GMT, data, n_data, work, &scale);
			}
		}
		/* First [robust] model has been found  */

		significant = true;
		while (n_model < Ctrl->N.value && significant) {
			trend2d_move_model_a_to_b (c_model, o_model, n_model, &c_chisq, &o_chisq);
			n_model++;

			/* Fit next model  */
			trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
			trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
			trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
			c_chisq = trend2d_get_chisq (data, n_data, n_model);
			GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, 1.0);
			if (Ctrl->N.robust) {
				do {
					trend2d_recompute_weights (GMT, data, n_data, work, &scale);
					trend2d_move_model_a_to_b (c_model, w_model, n_model, &c_chisq, &w_chisq);
					trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
					trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
					trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
					c_chisq = trend2d_get_chisq (data, n_data, n_model);
					significant = gmt_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
					GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, prob);
				} while (significant);
				/* Go back to previous model only if w_chisq < c_chisq  */
				if (w_chisq < c_chisq) {
					trend2d_move_model_a_to_b (w_model, c_model, n_model, &w_chisq, &c_chisq);
					trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
					if (Ctrl->weighted_output && n_model == Ctrl->N.value) trend2d_recompute_weights (GMT, data, n_data, work, &scale);
				}
			}
			/* Next [robust] model has been found  */
			significant = gmt_sig_f (GMT, c_chisq, n_data-n_model, o_chisq, n_data-n_model-1, Ctrl->I.value, &prob);
		}

		if (!significant) {	/* Go back to previous [robust] model, stored in o_model  */
			n_model--;
			rank--;
			trend2d_move_model_a_to_b (o_model, c_model, n_model, &o_chisq, &c_chisq);
			trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
			if (Ctrl->N.robust && Ctrl->weighted_output) trend2d_recompute_weights (GMT, data, n_data, work, &scale);
		}
	}
	else {
		n_model = Ctrl->N.value;
		trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
		trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
		c_chisq = trend2d_get_chisq (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.robust) {
			do {
				trend2d_recompute_weights (GMT, data, n_data, work, &scale);
				trend2d_move_model_a_to_b (c_model, w_model, n_model, &c_chisq, &w_chisq);
				trend2d_load_gtg_and_gtd (GMT,data, n_data, gtg, gtd, workb, n_model, np);
				trend2d_solve_system (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
				c_chisq = trend2d_get_chisq (data, n_data, n_model);
				significant = gmt_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				trend2d_move_model_a_to_b (w_model, c_model, n_model, &w_chisq, &c_chisq);
				trend2d_calc_m_and_r (data, n_data, c_model, n_model, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) trend2d_recompute_weights (GMT, data, n_data, work, &scale);
			}
		}
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (format, "Final model stats: N model parameters %%d.  Rank %%d.  Chi-Squared: %s\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, n_model, rank, c_chisq);
		GMT_Report (API, GMT_MSG_INFORMATION, "Model Coefficients: ");
		sprintf (format, "%s%s", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator);
		for (i = 0; i < n_model; i++) {
			GMT_Message (API, GMT_TIME_NONE, format, c_model[i]);
		}
		GMT_Message (API, GMT_TIME_NONE, "\n");
	}

	trend2d_untransform_x (data, n_data, xmin, xmax, ymin, ymax);

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	if (Ctrl->F.report) {
		struct GMT_RECORD Rec;
		Rec.data = c_model;	Rec.text = NULL;
		GMT_Put_Record (API, GMT_WRITE_DATA, &Rec);

	}
	else
		trend2d_write_output_trend (GMT,data, n_data, Ctrl->F.col, Ctrl->n_outputs);
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	trend2d_free_the_memory (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);

	Return (GMT_NOERROR);
}
