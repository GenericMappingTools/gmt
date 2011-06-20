/*--------------------------------------------------------------------
 *	$Id: trend1d_func.c,v 1.16 2011-06-20 21:45:16 guru Exp $
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
 * Brief synopsis: Reads stdin or file of x y pairs, or weighted pairs as x,y w data.  Fit 
 * a regression model y = f(x) + e, where e are error misfits and f(x) has
 * some user-prescribed functional form.  Presently available models are
 * polynomials and Fourier series.  The user may choose the number of terms
 * in the model to fit, whether to seek iterative refinement robust w.r.t.
 * outliers, and whether to seek automatic discovery of the significant
 * number of model parameters.
 *
 * Author:	W. H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 *
 * In trend1d I chose to construct the polynomial model using Chebyshev 
 * Polynomials so that the user may easily compare the sizes of the
 * coefficients (and compare with a Fourier series as well).  Tn(x)
 * is an n-degree polynomial with n zero-crossings in [-1,1] and n+1
 * extrema, at which the value of Tn(x) is +/- 1.  It is this property
 * which makes it easy to compare the size of the coefficients.
 *
 * During model fitting the data x coordinate is normalized into the domain
 * [-1, 1] for Chebyshev Polynomial fitting, or into the domain [-pi, pi]
 * for Fourier series fitting.  Before writing out the data the coordinate
 * is rescaled to match the original input values.
 *
 * An n degree polynomial can be written with terms of the form a0 + a1*x
 * + a2*x*x + ...  But it can also be written using other polynomial 
 * basis functions, such as a0*P0 + a1*P1 + a2*P2..., the Legendre
 * polynomials, and a0*T0 + a1*T1 + a2*T2..., the Chebyshev polynomials.
 * (The domain of the x values has to be in [-1, 1] in order to use P or T.)
 * It is well known that the ordinary polynomial basis 1, x, x*x, ... gives 
 * terribly ill- conditioned matrices.  The Ps and Ts do much better.
 * This is because the ordinary basis is far from orthogonal.  The Ps
 * are orthogonal on [-1,1] and the Ts are orthogonal on [-1,1] under a
 * simple weight function.
 * Because the Ps have ordinary orthogonality on [-1,1], I expected them
 * to be the best basis for a regression model; best meaning that they 
 * would lead to the most balanced G'G (matrix of normal equations) with
 * the smallest condition number and the most nearly diagonal model
 * parameter covariance matrix ((G'G)inverse).  It turns out, however, that
 * the G'G obtained from the Ts is very similar and usually has a smaller 
 * condition number than the Ps G'G.  Both of these are vastly superior to
 * the usual polynomials 1, x, x*x.  In a test with 1000 equally spaced
 * data and 8 model parameters, the Chebyshev system had a condition # = 10.6,
 * Legendre = 14.8, and traditional = 54722.7.  For 1000 randomly spaced data
 * and 8 model parameters, the results were C = 13.1, L = 15.6, and P = 54916.6.
 * As the number of model parameters approaches the number of data, the 
 * situation still holds, although all matrices get ill-conditioned; for 8 
 * random data and 8 model parameters, C = 1.8e+05, L = 2.6e+05, P = 1.1e+08.
 * I expected the Legendre polynomials to have a covariance matrix more nearly 
 * diagonal than that of the Chebyshev polynomials, but on this criterion also
 * the Chebyshev turned out to do better.  Only as ndata -> n_model_parameters
 * does the Legendre covariance matrix do better than the Chebyshev.   So for
 * all these reasons I use Chebyshev polynomials.
 */

#include "gmt.h"

#define TREND1D_N_OUTPUT_CHOICES 5

struct TREND1D_CTRL {
	GMT_LONG n_outputs;
	GMT_LONG weighted_output;
	GMT_LONG model_parameters;
	struct C {	/* -C<condition_#> */
		GMT_LONG active;
		double value;
	} C;
	struct F {	/* -F<xymrw> */
		GMT_LONG active;
		char col[TREND1D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} F;
	struct I {	/* -I[<confidence>] */
		GMT_LONG active;
		double value;
	} I;
	struct N {	/* -N[f]<n_model>[r] */
		GMT_LONG active;
		GMT_LONG robust;
		GMT_LONG mode;
		GMT_LONG value;
	} N;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
};

#define TREND1D_POLYNOMIAL 0
#define TREND1D_FOURIER 1

struct	TREND1D_DATA {
	double	x;
	double	y;
	double	m;
	double	r;
	double	w;
};

GMT_LONG read_data_trend1d (struct GMT_CTRL *GMT, struct TREND1D_DATA **data, GMT_LONG *n_data, double *xmin, double *xmax, GMT_LONG weighted_input, double **work)
{
	GMT_LONG n_alloc = GMT_CHUNK, n_fields, i;
	double *in = NULL;

	*data = GMT_memory (GMT, NULL, n_alloc, struct TREND1D_DATA);

	i = 0;
	while ((n_fields = GMT_Get_Record (GMT->parent, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) return (GMT_RUNTIME_ERROR);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
		
		(*data)[i].x = in[GMT_X];
		(*data)[i].y = in[GMT_Y];
		(*data)[i].w = (weighted_input) ? in[GMT_Z] : 1.0;

		if (i) {
			if (*xmin > (*data)[i].x) *xmin = (*data)[i].x;
			if (*xmax < (*data)[i].x) *xmax = (*data)[i].x;
		}
		else {
			*xmin = (*data)[i].x;
			*xmax = (*data)[i].x;
		}
		i++;

		if (i == n_alloc) {
			n_alloc <<= 1;
			*data = GMT_memory (GMT, *data, n_alloc, struct TREND1D_DATA);
		}
		if (i == INT_MAX) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot process more than %d data points\n", INT_MAX);
			GMT_free (GMT, data);
			return (EXIT_FAILURE);
		}
	}

	*data = GMT_memory (GMT, *data, i, struct TREND1D_DATA);
	*work = GMT_memory (GMT, NULL, i, double);

	*n_data = i;
	return (0);
}

void allocate_the_memory_1d (struct GMT_CTRL *GMT, GMT_LONG np, double **gtg, double **v, double **gtd, double **lambda, double **workb, double **workz, double **c_model, double **o_model, double **w_model)
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

void write_output_trend1d (struct GMT_CTRL *GMT, struct TREND1D_DATA *data, GMT_LONG n_data, char *output_choice, GMT_LONG n_outputs)
{
	GMT_LONG i, j;
	double out[5];

	for (i = 0; i < n_data; i++) {
		for (j = 0; j < n_outputs; j++) {
			switch (output_choice[j]) {
				case 'x':
					out[j] = data[i].x;
					break;
				case 'y':
					out[j] = data[i].y;
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
		GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, (void *)out);	/* Write this to output */
	}
}

void free_the_memory_1d (struct GMT_CTRL *GMT, double *gtg, double *v, double *gtd, double *lambda, double *workb, double *workz, double *c_model, double *o_model, double *w_model, struct TREND1D_DATA *data, double *work)
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

void transform_x_1d (struct TREND1D_DATA *data, GMT_LONG n_data, GMT_LONG model_type, double xmin, double xmax)
{
	GMT_LONG i;
	double offset, scale;

	offset = 0.5 * (xmin + xmax);	/* Mid Range  */
	scale = 2.0 / (xmax - xmin);	/* 1 / (1/2 Range)  */

	if (model_type == TREND1D_FOURIER) scale *= M_PI;	/* Set Range to 1 period  */

	for (i = 0; i < n_data; i++) data[i].x = (data[i].x - offset) * scale;
}

void untransform_x_1d (struct TREND1D_DATA *data, GMT_LONG n_data, GMT_LONG model_type, double xmin, double xmax)
{
	GMT_LONG i;
	double offset, scale;

	offset = 0.5 * (xmin + xmax);	/* Mid Range  */
	scale = 0.5 * (xmax - xmin);	/* 1/2 Range  */

	if (model_type == TREND1D_FOURIER) scale /= M_PI;

	for (i = 0; i < n_data; i++) data[i].x = (data[i].x * scale) + offset;
}

double get_chisq_1d (struct TREND1D_DATA *data, GMT_LONG n_data, GMT_LONG n_model)
{
	GMT_LONG i, nu;
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

void recompute_weights_1d (struct GMT_CTRL *GMT, struct TREND1D_DATA *data, GMT_LONG n_data, double *work, double *scale)
{
	GMT_LONG i;
	double k, ksq, rr;

	/* First find median { fabs(data[].r) },
		estimate scale from this,
		and compute chisq based on this.  */ 

	for (i = 0; i < n_data; i++) work[i] = fabs(data[i].r);
	GMT_sort_array (GMT, (void *)work, n_data, GMT_DOUBLE_TYPE);

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

void load_g_row_1d (double x, GMT_LONG n, double *gr, GMT_LONG m)
{
	/* Current data position, appropriately normalized.  */
	/* Number of model parameters, and elements of gr[]  */
	/* Elements of row of G matrix.  */
	/* Parameter indicating model type  */

	/* Routine computes the elements gr[j] in the ith row of the
	   G matrix (Menke notation), where x is the ith datum's
	   abscissa.  */

	GMT_LONG j, k;

	if (n) {
		gr[0] = 1.0;

		switch (m) {

			case TREND1D_POLYNOMIAL:
				/* Create Chebyshev polynomials  */
				if (n > 1) gr[1] = x;
				for (j = 2; j < n; j++) gr[j] = 2 * x * gr[j-1] - gr[j-2];
				break;

			case TREND1D_FOURIER:
				for (j = 1; j < n; j++) {
					k = (j + 1)/2;
					gr[j] = (j%2) ? cos(k*x) : sin(k*x);
				}
				break;
		}
	}
}

void calc_m_and_r_1d (struct TREND1D_DATA *data, GMT_LONG n_data, double *model, GMT_LONG n_model, GMT_LONG m_type, double *grow)
{
	/* model[n_model] holds solved coefficients of m_type model.
	  grow[n_model] is a vector for a row of G matrix.  */

	GMT_LONG i, j;
	for (i = 0; i < n_data; i++) {
		load_g_row_1d (data[i].x, n_model, grow, m_type);
		data[i].m = 0.0;
		for (j = 0; j < n_model; j++) data[i].m += model[j]*grow[j];
		data[i].r = data[i].y - data[i].m;
	}
}

void move_model_a_to_b_1d (double *model_a, double *model_b, GMT_LONG n_model, double *chisq_a, double *chisq_b)
{
	GMT_LONG i;
	for (i = 0; i < n_model; i++) model_b[i] = model_a[i];
	*chisq_b = *chisq_a;
}

void load_gtg_and_gtd_1d (struct TREND1D_DATA *data, GMT_LONG n_data, double *gtg, double *gtd, double *grow, GMT_LONG n_model, GMT_LONG mp, GMT_LONG m_type)
{
   	/* mp is row dimension of gtg  */

	GMT_LONG i, j, k;
	double wy;

	/* First zero the contents for summing */

	for (j = 0; j < n_model; j++) {
		for (k = 0; k < n_model; k++) gtg[j + k*mp] = 0.0;
		gtd[j] = 0.0;
	}

	/* Sum over all data  */
	for (i = 0; i < n_data; i++) {
		load_g_row_1d (data[i].x, n_model, grow, m_type);
		if (data[i].w != 1.0) {
			wy = data[i].w * data[i].y;
			for (j = 0; j < n_model; j++) {
				for (k = 0; k < n_model; k++) gtg[j + k*mp] += (data[i].w * grow[j] * grow[k]);
				gtd[j] += (wy * grow[j]);
			}
		}
		else {
			for (j = 0; j < n_model; j++) {
				for (k = 0; k < n_model; k++) gtg[j + k*mp] += (grow[j] * grow[k]);
				gtd[j] += (data[i].y * grow[j]);
			}
		}
	}
}

void solve_system_1d (struct GMT_CTRL *GMT, double *gtg, double *gtd, double *model, GMT_LONG n_model, GMT_LONG mp, double *lambda, double *v, double *b, double *z, double c_no, GMT_LONG *ir)
{
	GMT_LONG i, j, k, rank = 0, nrots, n, m;
	double c_test, temp_inverse_ij;

	if (n_model == 1) {
		model[0] = gtd[0] / gtg[0];
		*ir = 1;
	}
	else {
		n = n_model;
		m = mp;
		if (GMT_jacobi (GMT, gtg, &n, &m, lambda, v, b, z, &nrots)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: Matrix Solver Convergence Failure.\n");
		}
		c_test = fabs (lambda[0]) / c_no;
		while (rank < n_model && lambda[rank] > 0.0 && lambda[rank] > c_test) rank++;
		for (i = 0; i < n_model; i++) {
			model[i] = 0.0;
			for (j = 0; j < n_model; j++) {
				temp_inverse_ij = 0.0;
				for (k = 0; k <  rank; k++) {
					temp_inverse_ij += (v[i + k*mp] * v[j + k*mp] / lambda[k]);
				}
				model[i] += (temp_inverse_ij * gtd[j]);
			}
		}
		*ir = rank;
	}
}

void GMT_cheb_to_pol (struct GMT_CTRL *GMT, double c[], GMT_LONG n, double a, double b)
{
	/* Convert from Chebyshev coefficients used on a t =  [-1,+1] interval
	 * to polynomial coefficients on the original x = [a b] interval.
	 * Modified from Numerical Miracles, ...eh Recipes */
	 
	 GMT_LONG j, k;
	 double sv, cnst, fac, *d, *dd;
	 
	 d  = GMT_memory (GMT, NULL, n, double);
	 dd = GMT_memory (GMT, NULL, n, double);
	 
	 /* First we generate coefficients for a polynomial in t */
	 
	 d[0] = c[n-1];
	 for (j = n - 2; j >= 1; j--) {
	 	for (k = n - j; k >= 1; k--) {
			sv = d[k];
			d[k] = 2.0 * d[k-1] - dd[k];
			dd[k] = sv;
		}
		sv = d[0];
		d[0] = -dd[0] + c[j];
		dd[0] = sv;
	}
	for (j = n - 1; j >= 1; j--) d[j] = d[j-1] - dd[j];
	/* d[0] = -dd[0] + 0.5 * c[0]; */	/* This is what Num. Rec. says, but we do not do the approx with 0.5 * c[0] */
	d[0] = -dd[0] + c[0];

	/* Next step is to undo the scaling so we can use coefficients with x */

	cnst = fac = 2.0 / (b - a);
	for (j = 1; j < n; j++) {
		d[j] *= fac;
		fac *= cnst;
	}
	cnst = 0.5 * (a + b);
	for (j = 0; j <= n - 2; j++) for (k = n - 2; k >= j; k--) d[k] -= cnst * d[k+1];

	/* Return the new coefficients via c */

	GMT_memcpy (c, d, n, double);

	GMT_free (GMT, d);
	GMT_free (GMT, dd);
}

void *New_trend1d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TREND1D_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct TREND1D_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->C.value = 1.0e06;		/* Condition number for matrix solution  */	
	C->I.value = 0.51;		/* Confidence interval for significance test  */
	C->N.mode = TREND1D_POLYNOMIAL;
	return ((void *)C);
}

void Free_trend1d_Ctrl (struct GMT_CTRL *GMT, struct TREND1D_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_trend1d_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "trend1d %s [API] - Fit a [weighted] [robust] polynomial [or Fourier] model for y = f(x) to ascii xy[w]\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: trend1d [<table>] -F<xymrw|p> -N[f]<n_model>[r] [-C<condition_#>]\n");
	GMT_message (GMT, "\t[-I[<confidence>]] [%s] [-W] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-F Choose at least 1, up to 5, any order, of xymrw for ascii output to stdout.\n");
	GMT_message (GMT, "\t   x=x, y=y, m=model, r=residual=y-m, w=weight.  w determined iteratively if robust fit used.\n");
	GMT_message (GMT, "\t   Alternatively choose -Fp to output only the model coefficients (Polynomial).\n");
	GMT_message (GMT, "\t-N Fit a Polynomial [Default] or Fourier (-Nf) model with <n_model> terms.\n");
	GMT_message (GMT, "\t   Append r for robust model. E.g., robust quadratic = -N3r.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<table> is one or more data files (in ASCII, binary, netCDF) with (x,y[,w]) data.\n");
	GMT_message (GMT, "\t   If no files are given, standard input is read.\n");
	GMT_message (GMT, "\t-C Truncate eigenvalue spectrum so matrix has <condition_#>.  [Default = 1.0e06].\n");
	GMT_message (GMT, "\t-I Iteratively Increase # model parameters, to a max of <n_model> so long as the\n");
	GMT_message (GMT, "\t   reduction in variance is significant at the <confidence> level.\n");
	GMT_message (GMT, "\t   Give -I without a number to default to 0.51 confidence level.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Weighted input given, weights in 3rd column [Default is unweighted].\n");
	GMT_explain_options (GMT, "C0");
	GMT_message (GMT, "\t   Default is 2 (or 3 if -W is set) input columns.\n");
	GMT_explain_options (GMT, "D0fhi:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_trend1d_parse (struct GMTAPI_CTRL *C, struct TREND1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to trend1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = TRUE;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				for (j = 0; opt->arg[j]; j++) {
					if (j < TREND1D_N_OUTPUT_CHOICES)
						Ctrl->F.col[j] = opt->arg[j];
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F option: Too many output columns selected: Choose from -Fxymrw|p\n");
						n_errors++;
					}
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (opt->arg[0]) Ctrl->I.value = atof (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				if (strchr (opt->arg, 'r')) Ctrl->N.robust = TRUE;
				j = (opt->arg[0] == 'r') ? 1 : 0;
				if (opt->arg[j] == 'F' || opt->arg[j] == 'f') {
					Ctrl->N.mode = TREND1D_FOURIER;
					j++;
				}
				else if (opt->arg[j] == 'P' || opt->arg[j] == 'p') {
					Ctrl->N.mode = TREND1D_POLYNOMIAL;
					j++;
				}
				if (opt->arg[j])
					Ctrl->N.value = atoi(&opt->arg[j]);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -N option: No model specified\n");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->C.value <= 1.0, "Syntax error -C option: Condition number must be larger than unity\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.value < 0.0 || Ctrl->I.value > 1.0, "Syntax error -C option: Give 0 < confidence level < 1.0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.value <= 0.0, "Syntax error -N option: A positive number of terms must be specified\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.active) ? 3 : 2);
	for (j = Ctrl->n_outputs = 0; j < TREND1D_N_OUTPUT_CHOICES && Ctrl->F.col[j]; j++) {
		if (!strchr ("xymrwp", Ctrl->F.col[j])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F option: Unrecognized output choice %c\n", Ctrl->F.col[j]);
			n_errors++;
		}
		else if (Ctrl->F.col[j] == 'w')
			Ctrl->weighted_output = TRUE;
		else if (Ctrl->F.col[j] == 'p')
			Ctrl->model_parameters = TRUE;
		Ctrl->n_outputs++;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs == 0, "Syntax error -F option: Must specify at least one output columns \n");
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs > 1 && Ctrl->model_parameters, 
					"Syntax error -F option: When selecting model parameters, it must be the only ouput\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_trend1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_trend1d (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, n_model, significant, rank, n_data, np, error = FALSE;

	double *gtg = NULL, *v = NULL, *gtd = NULL, *lambda = NULL, *workb = NULL;
	double *workz = NULL, *c_model = NULL, *o_model = NULL, *w_model = NULL, *work = NULL;
	double xmin, xmax, c_chisq, o_chisq, w_chisq, scale = 1.0, prob;

	char format[GMT_BUFSIZ];

	struct TREND1D_DATA *data = NULL;
	struct TREND1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_trend1d_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_trend1d_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_trend1d", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "his>" GMT_OPT("H"), options))) Return (error);
	Ctrl = (struct TREND1D_CTRL *) New_trend1d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_trend1d_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the trend1d main code ----------------------------*/

	np = Ctrl->N.value;	/* Row dimension for matrices gtg and v  */
	allocate_the_memory_1d(GMT, np, &gtg, &v, &gtd, &lambda, &workb, &workz, &c_model, &o_model, &w_model);

	if ((error = GMT_set_cols (GMT, GMT_IN, 2 + Ctrl->W.active))) return (error);
	if ((error = GMT_set_cols (GMT, GMT_OUT, Ctrl->n_outputs))) return (error);
	if ((error = GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) return (error);	/* Establishes data input */
	if ((error = GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) return (error);	/* Establishes data output */

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
	if ((error = read_data_trend1d (GMT, &data, &n_data, &xmin, &xmax, Ctrl->W.active, &work))) Return (error);
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (xmin == xmax) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Min and Max value of input data are the same.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not read any data.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data < Ctrl->N.value) GMT_report (GMT, GMT_MSG_FATAL, "Warning: Ill-posed problem; n_data < n_model_max.\n");

	transform_x_1d (data, n_data, Ctrl->N.mode, xmin, xmax);	/* Set domain to [-1, 1] or [-pi, pi]  */

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format,"Read %%ld data with X values from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, n_data, xmin, xmax);
		GMT_report (GMT, GMT_MSG_NORMAL, "N_model\tRank\tChi_Squared\tSignificance\n");
	}

	sprintf (format, "%%ld\t%%ld\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	if (Ctrl->I.active) {
		n_model = 1;

		/* Fit first model  */
		load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
		solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
		c_chisq = get_chisq_1d (data, n_data, n_model);
		GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.robust) {
			do {
				recompute_weights_1d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
				solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
				c_chisq = get_chisq_1d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_1d (GMT, data, n_data, work, &scale);
			}
		}
		/* First [robust] model has been found  */

		significant = TRUE;
		while (n_model < Ctrl->N.value && significant) {
			move_model_a_to_b_1d (c_model, o_model, n_model, &c_chisq, &o_chisq);
			n_model++;

			/* Fit next model  */
			load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
			solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
			calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
			c_chisq = get_chisq_1d (data, n_data, n_model);
			GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, 1.00);
			if (Ctrl->N.robust) {
				do {
					recompute_weights_1d (GMT, data, n_data, work, &scale);
					move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
					load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
					solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
					calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
					c_chisq = get_chisq_1d (data, n_data, n_model);
					significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
					GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, prob);
				} while (significant);
				/* Go back to previous model only if w_chisq < c_chisq  */
				if (w_chisq < c_chisq) {
					move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
					calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
					if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_1d (GMT, data, n_data, work, &scale);
				}
			}
			/* Next [robust] model has been found  */
			significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, o_chisq, n_data-n_model-1, Ctrl->I.value, &prob);
		}

		if (!(significant) ) {	/* Go back to previous [robust] model, stored in o_model  */
			n_model--;
			rank--;
			move_model_a_to_b_1d (o_model, c_model, n_model, &o_chisq, &c_chisq);
			calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
			if (Ctrl->N.robust && Ctrl->weighted_output) recompute_weights_1d (GMT, data, n_data, work, &scale);
		}
	}
	else {
		n_model = Ctrl->N.value;
		load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
		solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
		c_chisq = get_chisq_1d (data, n_data, n_model);
		GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, 1.00);
		if (Ctrl->N.robust) {
			do {
				recompute_weights_1d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, Ctrl->N.mode);
				solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
				c_chisq = get_chisq_1d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_1d (data, n_data, c_model, n_model, Ctrl->N.mode, workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.value) recompute_weights_1d (GMT, data, n_data, work, &scale);
			}
		}
	}

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "Final model stats: N model parameters %%ld.  Rank %%ld.  Chi-Squared: %s\n", GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, n_model, rank, c_chisq);
		GMT_report (GMT, GMT_MSG_NORMAL, "Model Coefficients  (Chebyshev):");
		sprintf (format, "\t%s", GMT->current.setting.format_float_out);
		for (i = 0; i < n_model; i++) GMT_message (GMT, format, c_model[i]);
		GMT_message (GMT, "\n");
		GMT_cheb_to_pol (GMT, c_model, n_model, xmin, xmax);
		GMT_report (GMT, GMT_MSG_NORMAL, "Model Coefficients (Polynomial):");
		for (i = 0; i < n_model; i++) GMT_message (GMT, format, c_model[i]);
		GMT_message (GMT, "\n");
	}

	untransform_x_1d (data, n_data, Ctrl->N.mode, xmin, xmax);

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

	if (!Ctrl->model_parameters)	/* Write any or all of the 'xymrw' */
		write_output_trend1d (GMT, data, n_data, Ctrl->F.col, Ctrl->n_outputs);
	else {				/* Write only the model parameters */
		GMT_cheb_to_pol (GMT, c_model, n_model, xmin, xmax);
		sprintf (format, "%s", GMT->current.setting.format_float_out);
		for (i = 0; i < n_model - 1; i++) {
			fprintf(stdout, GMT->current.setting.format_float_out, c_model[i]);	fprintf(stdout, "\t");
		}
		fprintf(stdout, GMT->current.setting.format_float_out, c_model[n_model-1]);
		fprintf(stdout, "\n");
	}

	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	free_the_memory_1d (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);

	Return (GMT_OK);
}
