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
 *
 * Update: Aug-8, 2015 P. Wessel: Added ability to solve for a mixed model with
 * both polynomial and Fourier parts.  Also, ability to selct just parts of a
 * polynomial model (i.e., not include every term from 0 to n), but this
 * necessitates working with powers of x and not Chebyshev, so we check and use
 * the appropriate method.
 */

#define THIS_MODULE_NAME	"trend1d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Fit a [weighted] [robust] polynomial [and/or Fourier] model for y = f(x) to xy[w] data"
#define THIS_MODULE_KEYS	"<DI,>DO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbdfhis" GMT_OPT("H")

#define TREND1D_N_OUTPUT_CHOICES 5

struct TREND1D_CTRL {
	int n_outputs;
	int weighted_output;
	int model_parameters;
	struct C {	/* -C<condition_#> */
		bool active;
		double value;
	} C;
	struct F {	/* -F<xymrw> */
		bool active;
		char col[TREND1D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} F;
	struct I {	/* -I[<confidence>] */
		bool active;
		double value;
	} I;
	struct N {	/*  -N[p|P|f|F|c|C|s|S|x|X]<list-of-terms>[,...][+l<length>][+o<origin>][+r] */
		bool active;
		struct GMT_MODEL M;
	} N;
	struct W {	/* -W */
		bool active;
	} W;
};

struct	TREND1D_DATA {
	double	x;	/* This is x and will be normalixed */
	double	t;	/* This will be in radians per x for Fourier terms */
	double	y;
	double	m;
	double	r;
	double	w;
};

int read_data_trend1d (struct GMT_CTRL *GMT, struct TREND1D_DATA **data, uint64_t *n_data, double *xmin, double *xmax, int weighted_input, double **work)
{
	uint64_t i;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	double *in = NULL;

	*data = GMT_memory (GMT, NULL, n_alloc, struct TREND1D_DATA);

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
	
		(*data)[i].x = (*data)[i].t = in[GMT_X];
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

		if (++i == n_alloc) {
			n_alloc <<= 1;
			*data = GMT_memory (GMT, *data, n_alloc, struct TREND1D_DATA);
		}
	} while (true);

	*data = GMT_memory (GMT, *data, i, struct TREND1D_DATA);
	*work = GMT_memory (GMT, NULL, i, double);

	*n_data = i;
	return (0);
}

void allocate_the_memory_1d (struct GMT_CTRL *GMT, unsigned int np, double **gtg, double **v, double **gtd, double **lambda, double **workb, double **workz, double **c_model, double **o_model, double **w_model)
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

void write_output_trend1d (struct GMT_CTRL *GMT, struct TREND1D_DATA *data, uint64_t n_data, char *output_choice, unsigned int n_outputs)
{
	uint64_t i;
	unsigned int j;
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
		GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);	/* Write this to output */
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

void transform_x_1d (struct TREND1D_DATA *data, uint64_t n_data, struct GMT_MODEL *M, double xmin, double xmax)
{
	uint64_t i;
	double offset, scale;

	offset = 0.5 * (xmin + xmax);	/* Mid Range  */
	scale = 2.0 / (xmax - xmin);	/* 1 / (1/2 Range)  */

	/* Always normalize x for Chebyshev or polynomial fit */
	if (M->type & 1) for (i = 0; i < n_data; i++) data[i].x = (data[i].x - offset) * scale;	/* Now in -1/+1 range */
	if (M->type & 2) {	/* Have Fourier model component to deal with */
		if (M->got_origin[GMT_X]) offset = M->origin[GMT_X];		/* Override the offset using given origin */
		if (M->got_period[GMT_X]) scale = 2.0 / M->period[GMT_X];	/* Override the period using given period */
		scale *= M_PI;	/* Set Range to 1 period = 2 pi */
		for (i = 0; i < n_data; i++) data[i].t = (data[i].t - offset) * scale;	/* Now in units of period */
	}
}

void untransform_x_1d (struct TREND1D_DATA *data, uint64_t n_data, struct GMT_MODEL *M, double xmin, double xmax)
{	/* Undo transformation of x, if used */
	uint64_t i;
	double offset, scale;

	if ((M->type & 1) == 0) return;	/* Nothing to do */
	offset = 0.5 * (xmin + xmax);	/* Mid Range  */
	scale = 0.5 * (xmax - xmin);	/* 1/2 Range  */

	for (i = 0; i < n_data; i++) data[i].x = (data[i].x * scale) + offset;
}

double get_chisq_1d (struct TREND1D_DATA *data, uint64_t n_data, unsigned int n_model)
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

void recompute_weights_1d (struct GMT_CTRL *GMT, struct TREND1D_DATA *data, uint64_t n_data, double *work, double *scale)
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

double chebyshev (double x, unsigned int n)
{	/* Return T_n(x) */
	double cj, cj1, cj2;
	unsigned int j;
	if (n == 0) return 1.0;
	if (n == 1) return x;
	cj1 = 1.0;	cj = x;
	for (j = 2; j <= n; j++) {
		cj2 = cj1;
		cj1 = cj;
		cj = 2.0 * x * cj1 - cj2;
	}
	return (cj);
}

double polynomial (double x, unsigned int n)
{	/* Return x^n */
	if (n == 0) return 1.0;
	if (n == 1) return x;
	return (pow (x, (double)n));
}

void load_g_row_1d (double x, double t, int n, double *gr, struct GMT_MODEL *M)
{
	/* x: Current data position, appropriately normalized.  */
	/* Number of model parameters, and elements of gr[]  */
	/* Elements of row of G matrix.  */
	/* M: structure with info for each model term  */

	/* Routine computes the elements gr[j] in the ith row of the
	   G matrix (Menke notation), where x is the ith datum's
	   abscissa.  */

	int j, k;

	for (j = 0; j < n; j++) {
		k = M->term[j].order[GMT_X];
		switch (M->term[j].kind) {
			case GMT_POLYNOMIAL:
				gr[j] = polynomial (x, k);
				break;
			case GMT_CHEBYSHEV:
				gr[j] = chebyshev (x, k);
				break;
			case GMT_COSINE:
				gr[j] = cos (k * t);
				break;
			case GMT_SINE:
				gr[j] = sin (k * t);
				break;
		}
	}
}

void calc_m_and_r_1d (struct TREND1D_DATA *data, uint64_t n_data, double *model, unsigned int n_model, struct GMT_MODEL *M, double *grow)
{
	/* model[n_model] holds solved coefficients of m_type model.
	  grow[n_model] is a vector for a row of G matrix.  */

	uint64_t i;
	unsigned int j;
	for (i = 0; i < n_data; i++) {
		load_g_row_1d (data[i].x, data[i].t, n_model, grow, M);
		data[i].m = 0.0;
		for (j = 0; j < n_model; j++) data[i].m += model[j]*grow[j];
		data[i].r = data[i].y - data[i].m;
	}
}

void move_model_a_to_b_1d (double *model_a, double *model_b, unsigned int n_model, double *chisq_a, double *chisq_b)
{
	unsigned int i;
	for (i = 0; i < n_model; i++) model_b[i] = model_a[i];
	*chisq_b = *chisq_a;
}

void load_gtg_and_gtd_1d (struct TREND1D_DATA *data, uint64_t n_data, double *gtg, double *gtd, double *grow, unsigned int n_model, unsigned int mp, struct GMT_MODEL *M)
{
   	/* mp is row dimension of gtg  */

	uint64_t i;
	unsigned int j, k;
	double wy;

	/* First zero the contents for summing */

	for (j = 0; j < n_model; j++) {
		for (k = 0; k < n_model; k++) gtg[j + k*mp] = 0.0;
		gtd[j] = 0.0;
	}

	/* Sum over all data  */
	for (i = 0; i < n_data; i++) {
		load_g_row_1d (data[i].x, data[i].t, n_model, grow, M);
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

void solve_system_1d (struct GMT_CTRL *GMT, double *gtg, double *gtd, double *model, unsigned int n_model, unsigned int mp, double *lambda, double *v, double *b, double *z, double c_no, unsigned int *ir)
{
	unsigned int i, j, k, rank = 0, nrots;
	double c_test, temp_inverse_ij;

	if (n_model == 1) {
		model[0] = gtd[0] / gtg[0];
		*ir = 1;
	}
	else {
		if (GMT_jacobi (GMT, gtg, n_model, mp, lambda, v, b, z, &nrots)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Matrix Solver Convergence Failure.\n");
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

void GMT_cheb_to_pol (struct GMT_CTRL *GMT, double c[], unsigned int n, double a, double b)
{
	/* Convert from Chebyshev coefficients used on a t =  [-1,+1] interval
	 * to polynomial coefficients on the original x = [a b] interval.
	 * Modified from Numerical Miracles, ...eh Recipes */
	 
	 unsigned int j, k;
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
	for (j = 0; j <= n - 2; j++) for (k = n - 1; k > j; k--) d[k-1] -= cnst * d[k];

	/* Return the new coefficients via c */

	GMT_memcpy (c, d, n, double);

	GMT_free (GMT, d);
	GMT_free (GMT, dd);
}

void *New_trend1d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TREND1D_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct TREND1D_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.value = 1.0e06;		/* Condition number for matrix solution  */	
	C->I.value = 0.51;		/* Confidence interval for significance test  */
	return (C);
}

void Free_trend1d_Ctrl (struct GMT_CTRL *GMT, struct TREND1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_trend1d_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: trend1d [<table>] -F<xymrw|p> -N[p|P|f|F|c|C|s|S|x|X]<list-of-terms>[,...][+l<length>][+o<origin>][+r]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<condition_#>] [-I[<confidence>]] [%s] [-W] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-F Choose at least 1, up to 5, any order, of xymrw for ASCII output to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x=x, y=y, m=model, r=residual=y-m, w=weight.  w determined iteratively if robust fit used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively choose -Fp to output only the model coefficients (Polynomial).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Specify a polynomial, Fourier, or mixed model of any order, separate components by commas:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     p<n> adds complete polynomial up to degree <n>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     P<n> (or xx..x adds the component x^<n> only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f<n> adds the Fourier series components 1-n.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     F<n> adds just the <n>'th Fourier component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c<n> adds the cosine series components 1-n.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     C<n> adds just the <n>'th cosine component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s<n> adds the sine series components 1-n.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     S<n> adds just the <n>'th sine component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +o<orig> to set origin of x [mid-point of x].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l<period> to set fundamental period of x [range of x].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +r for robust model. E.g., robust quadratic = -Np3+r.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data files (in ASCII, binary, netCDF) with (x,y[,w]) data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Truncate eigenvalue spectrum so matrix has <condition_#> [Default = 1.0e06].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Iteratively Increase # model parameters, to a max of <n_model> so long as the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   reduction in variance is significant at the <confidence> level.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I without a number to default to 0.51 confidence level.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Weighted input given, weights in 3rd column [Default is unweighted].\n");
	GMT_Option (API, "bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 2 (or 3 if -W is set) input columns.\n");
	GMT_Option (API, "bo,d,h,i,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_trend1d_parse (struct GMT_CTRL *GMT, struct TREND1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to trend1d and sets parameters in CTRL.
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
					if (j < TREND1D_N_OUTPUT_CHOICES)
						Ctrl->F.col[j] = opt->arg[j];
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Too many output columns selected: Choose from -Fxymrw|p\n");
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
				if (GMT_parse_model (API->GMT, opt->option, opt->arg, 1U, &(Ctrl->N.M)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: See usage for details.\n");
					n_errors++;
				}
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
	n_errors += GMT_check_condition (GMT, Ctrl->N.M.n_terms <= 0, "Syntax error -N option: A positive number of terms must be specified\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.active) ? 3 : 2);
	for (j = Ctrl->n_outputs = 0; j < TREND1D_N_OUTPUT_CHOICES && Ctrl->F.col[j]; j++) {
		if (!strchr ("xymrwp", Ctrl->F.col[j])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Unrecognized output choice %c\n", Ctrl->F.col[j]);
			n_errors++;
		}
		else if (Ctrl->F.col[j] == 'w')
			Ctrl->weighted_output = true;
		else if (Ctrl->F.col[j] == 'p')
			Ctrl->model_parameters = true;
		Ctrl->n_outputs++;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs == 0, "Syntax error -F option: Must specify at least one output columns \n");
	n_errors += GMT_check_condition (GMT, Ctrl->n_outputs > 1 && Ctrl->model_parameters, 
					"Syntax error -F option: When selecting model parameters, it must be the only output\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_trend1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_trend1d (void *V_API, int mode, void *args)
{
	unsigned int i, n_model, rank, np;
	int error = 0;
	bool significant;
	
	uint64_t n_data;

	double *gtg = NULL, *v = NULL, *gtd = NULL, *lambda = NULL, *workb = NULL;
	double *workz = NULL, *c_model = NULL, *o_model = NULL, *w_model = NULL, *work = NULL;
	double xmin, xmax, c_chisq, o_chisq = 0.0, w_chisq, scale = 1.0, prob;

	char format[GMT_BUFSIZ];

	struct TREND1D_DATA *data = NULL;
	struct TREND1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_trend1d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_trend1d_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_trend1d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_trend1d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_trend1d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the trend1d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	np = Ctrl->N.M.n_terms;	/* Row dimension for matrices gtg and v  */
	allocate_the_memory_1d(GMT, np, &gtg, &v, &gtd, &lambda, &workb, &workz, &c_model, &o_model, &w_model);

	if ((error = GMT_set_cols (GMT, GMT_IN, 2 + Ctrl->W.active)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if ((error = read_data_trend1d (GMT, &data, &n_data, &xmin, &xmax, Ctrl->W.active, &work))) Return (error);
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (xmin == xmax) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Min and Max value of input data are the same.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not read any data.\n");
		Return (EXIT_FAILURE);
	}
	if (n_data < (uint64_t)Ctrl->N.M.n_terms) GMT_Report (API, GMT_MSG_NORMAL, "Warning: Ill-posed problem; n_data < n_model_max.\n");

	transform_x_1d (data, n_data, &(Ctrl->N.M), xmin, xmax);	/* Set domain to [-1, 1] or [-pi, pi]  */

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format,"Read %%" PRIu64 " data with X values from %s to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_data, xmin, xmax);
		GMT_Report (API, GMT_MSG_VERBOSE, "N_model%sRank%sChi_Squared%sSignificance\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);
	}

	sprintf (format, "%%d%s%%d%s%s%s%s\n", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);

	if (Ctrl->I.active) {
		n_model = 1;

		/* Fit first model  */
		load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
		solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
		c_chisq = get_chisq_1d (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.0);
		if (Ctrl->N.M.robust) {
			do {
				recompute_weights_1d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
				solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
				c_chisq = get_chisq_1d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.M.n_terms) recompute_weights_1d (GMT, data, n_data, work, &scale);
			}
		}
		/* First [robust] model has been found  */

		significant = true;
		while (n_model < Ctrl->N.M.n_terms && significant) {
			move_model_a_to_b_1d (c_model, o_model, n_model, &c_chisq, &o_chisq);
			n_model++;

			/* Fit next model  */
			load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
			solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
			calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
			c_chisq = get_chisq_1d (data, n_data, n_model);
			GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.00);
			if (Ctrl->N.M.robust) {
				do {
					recompute_weights_1d (GMT, data, n_data, work, &scale);
					move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
					load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
					solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
					calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
					c_chisq = get_chisq_1d (data, n_data, n_model);
					significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
					GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
				} while (significant);
				/* Go back to previous model only if w_chisq < c_chisq  */
				if (w_chisq < c_chisq) {
					move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
					calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
					if (Ctrl->weighted_output && n_model == Ctrl->N.M.n_terms) recompute_weights_1d (GMT, data, n_data, work, &scale);
				}
			}
			/* Next [robust] model has been found  */
			significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, o_chisq, n_data-n_model-1, Ctrl->I.value, &prob);
		}

		if (!(significant) ) {	/* Go back to previous [robust] model, stored in o_model  */
			n_model--;
			rank--;
			move_model_a_to_b_1d (o_model, c_model, n_model, &o_chisq, &c_chisq);
			calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
			if (Ctrl->N.M.robust && Ctrl->weighted_output) recompute_weights_1d (GMT, data, n_data, work, &scale);
		}
	}
	else {
		n_model = Ctrl->N.M.n_terms;
		load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
		solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
		calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
		c_chisq = get_chisq_1d (data, n_data, n_model);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, 1.00);
		if (Ctrl->N.M.robust) {
			do {
				recompute_weights_1d (GMT, data, n_data, work, &scale);
				move_model_a_to_b_1d (c_model, w_model, n_model, &c_chisq, &w_chisq);
				load_gtg_and_gtd_1d (data, n_data, gtg, gtd, workb, n_model, np, &(Ctrl->N.M));
				solve_system_1d (GMT, gtg, gtd, c_model, n_model, np, lambda, v, workb, workz, Ctrl->C.value, &rank);
				calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
				c_chisq = get_chisq_1d (data, n_data, n_model);
				significant = GMT_sig_f (GMT, c_chisq, n_data-n_model, w_chisq, n_data-n_model, Ctrl->I.value, &prob);
				GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq, prob);
			} while (significant);
			/* Go back to previous model only if w_chisq < c_chisq  */
			if (w_chisq < c_chisq) {
				move_model_a_to_b_1d (w_model, c_model, n_model, &w_chisq, &c_chisq);
				calc_m_and_r_1d (data, n_data, c_model, n_model, &(Ctrl->N.M), workb);
				if (Ctrl->weighted_output && n_model == Ctrl->N.M.n_terms) recompute_weights_1d (GMT, data, n_data, work, &scale);
			}
		}
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "Final model stats: N model parameters %%d.  Rank %%d.  Chi-Squared: %s\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, n_model, rank, c_chisq);
		if (Ctrl->N.M.type & 1) {	/* Has polynomial component */
			if (Ctrl->N.M.chebyshev)
				sprintf (format, "Model Coefficients  (Chebyshev");
			else
				sprintf (format, "Model Coefficients  (polynomial");
		}
		if (Ctrl->N.M.type & 2)	/* Has Fourier components */
			strcat (format, " and Fourier");
		strcat (format, ")\n");
		GMT_Report (API, GMT_MSG_VERBOSE, format);
		sprintf (format, "%s%s", GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);
		for (i = 0; i < n_model; i++) GMT_Message (API, GMT_TIME_NONE, format, c_model[i]);
		GMT_Message (API, GMT_TIME_NONE, "\n");
		GMT_cheb_to_pol (GMT, c_model, n_model, xmin, xmax);
		GMT_Report (API, GMT_MSG_VERBOSE, "Model Coefficients (Polynomial): ");
		for (i = 0; i < n_model; i++) GMT_Message (API, GMT_TIME_NONE, format, c_model[i]);
		GMT_Message (API, GMT_TIME_NONE, "\n");
	}

	untransform_x_1d (data, n_data, &(Ctrl->N.M), xmin, xmax);

	i = (Ctrl->model_parameters) ? n_model : Ctrl->n_outputs;
	if ((error = GMT_set_cols (GMT, GMT_OUT, i)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	if (!Ctrl->model_parameters)	/* Write any or all of the 'xymrw' */
		write_output_trend1d (GMT, data, n_data, Ctrl->F.col, Ctrl->n_outputs);
	else {				/* Write only the model parameters */
		if (Ctrl->N.M.chebyshev)
			GMT_cheb_to_pol (GMT, c_model, n_model, xmin, xmax);
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, c_model);
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	free_the_memory_1d (GMT, gtg, v, gtd, lambda, workb, workz, c_model, o_model, w_model, data, work);

	Return (GMT_OK);
}
