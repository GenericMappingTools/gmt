/*--------------------------------------------------------------------
 *	$Id: grdtrend_func.c,v 1.6 2011-04-23 02:14:13 guru Exp $
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
 * Brief synopsis: Reads a grid file and fits a trend surface.  Trend surface
 * is defined by:
 * 
 * m1 +m2*x + m3*y + m4*xy + m5*x*x + m6*y*y + m7*x*x*x
 * 	+ m8*x*x*y + m9*x*y*y + m10*y*y*y.
 * 
 * n_model is set by the user to be an integer in [1,10]
 * which sets the number of model coefficients to fit.
 * 
 * Author:		W. H. F. Smith
 * Date:		1 JAN, 2010
 * Version:	5 API
 * 
 * Explanations:
 * 
 * Thus:
 * n_model = 1 gives the mean value of the surface,
 * n_model = 3 fits a plane,
 * n_model = 4 fits a bilinear surface,
 * n_model = 6 fits a biquadratic,
 * n_model = 10 fits a bicubic surface.
 * 
 * The user may write out grid files of the fitted surface
 * [-T<trend.grd>] and / or of the residuals (input data
 * minus fitted trend) [-D<differences.grd] and / or of
 * the weights used in iterative fitting [-W<weight.grd].
 * This last option applies only when the surface is fit
 * iteratively [-N<n>[r]].
 * 
 * A robust fit may be achieved by iterative fitting of
 * a weighted least squares problem, where the weights
 * are set according to a scale length based on the 
 * Median absolute deviation (MAD: Huber, 1982).  The
 * -N<n>r option achieves this.
 * 
 * Calls:		uses the QR solution of the Normal
 * 		equations furnished by Wm. Menke's
 * 		C routine "gauss".  We gratefully
 * 		acknowledge this contribution, now
 * 		as GMT_gauss in gmt_vector.c
 * 
 * Remarks:
 * 
 * We adopt a translation and scaling of the x,y coordinates.
 * We choose x,y such that they are in [-1,1] over the range
 * of the grid file.  If the problem is unweighted, all input
 * values are filled (no "holes" or NaNs in the input grid file),
 * and n_model <= 4 (bilinear or simpler), then the normal
 * equations matrix (G'G in Menke notation) is diagonal under
 * this change of coordinates, and the solution is trivial.
 * In this case, it would be dangerous to try to accumulate
 * the sums which are the elements of the normal equations;
 * while they analytically cancel to zero, the addition errors
 * would likely prevent this.  Therefore we have written a
 * routine, grd_trivial_model(), to handle this case.
 * 
 * If the problem is more complex than the above trivial case,
 * (missing values, weighted problem, or n_model > 4), then
 * G'G is not trivial and we just naively accumulate sums in
 * the G'G matrix.  We hope that the changed coordinates in
 * [-1,1] will help the accuracy of the problem.  We also use
 * Legendre polynomials in this case so that the matrix elements
 * are conveniently sized near 0 or 1.
 */

#include "gmt.h"

struct GRDTREND_CTRL {	/* All control options for this program (except common args) */
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct D {	/* -D<diffgrid> */
		GMT_LONG active;
		char *file;
	} D;
	struct N {	/* -N[r]<n_model> */
		GMT_LONG active;
		GMT_LONG robust;
		GMT_LONG value;
	} N;
	struct T {	/* -T<trend.grd> */
		GMT_LONG active;
		char *file;
	} T;
	struct W {	/* -W<weight.grd> */
		GMT_LONG active;
		char *file;
	} W;
};

void *New_grdtrend_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDTREND_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDTREND_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
		
	return ((void *)C);
}

void Free_grdtrend_Ctrl (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->D.file) free ((void *)C->D.file);	
	if (C->T.file) free ((void *)C->T.file);	
	if (C->W.file) free ((void *)C->W.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdtrend_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdtrend %s [API] - Fit trend surface to gridded data\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdtrend <input.grd> -N<n_model>[r] [-D<diff.grd>]\n");
	GMT_message (GMT, "\t[%s] [-T<trend.grd>] [%s] [-W<weight.grd>]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<input.grd> is name of grid file to fit trend to.\n");
	GMT_message (GMT, "\t-N Number of model parameters to fit; integer in [1,10].  Append r for robust fit.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-D Supply filename to write grid file of differences (input - trend).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-T Supply filename to write grid file of trend.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Supply filename if you want to [read and] write grid file of weights.\n");
	GMT_message (GMT, "\t   If <weight.grd> can be read at run, and if robust = FALSE, weighted problem will be solved.\n");
	GMT_message (GMT, "\t   If robust = TRUE, weights used for robust fit will be written to <weight.grd>.\n");
	GMT_explain_options (GMT, ".");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdtrend_parse (struct GMTAPI_CTRL *C, struct GRDTREND_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdtrend and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, j;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'D':
				Ctrl->D.active = TRUE;
				if (opt->arg[0])
					Ctrl->D.file = strdup (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: Must specify file name\n");
					n_errors++;
				}
				break;
			case 'N':
				/* Must check for both -N[r]<n_model> and -N<n_model>[r] due to confusion */
				Ctrl->N.active = TRUE;
				if (strchr (opt->arg, 'r')) Ctrl->N.robust = TRUE;
				j = (opt->arg[0] == 'r') ? 1 : 0;
				if (opt->arg[j]) Ctrl->N.value = atoi(&opt->arg[j]);
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				if (opt->arg[0])
					Ctrl->T.file = strdup (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Must specify file name\n");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				if (opt->arg[0])
					Ctrl->W.file = strdup (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -W option: Must specify file name\n");
					n_errors++;
				}
				/* OK if this file doesn't exist */
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify an input grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.value <= 0 || Ctrl->N.value > 10, "Syntax error -N option: Specify 1-10 model parameters\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void set_up_vals (double *val, GMT_LONG nval, double vmin, double vmax, double dv, GMT_LONG pixel_reg)
{	/* Store x[i], y[j] once for all to save time  */
	GMT_LONG i;
	double v, middle, drange, true_min, true_max;

	true_min = (pixel_reg) ? vmin + 0.5 * dv : vmin;
	true_max = (pixel_reg) ? vmax - 0.5 * dv : vmax;

	middle = 0.5 * (true_min + true_max);
	drange = 2.0 / (true_max - true_min);
	for (i = 0; i < nval; i++) {
		v = true_min + i * dv;
		val[i] = (v - middle) * drange;
	}
	/* Just to be sure no rounding outside */
	val[0] = -1.0;
	val[nval - 1] = 1.0;
	return;
}

void load_pstuff (double *pstuff, GMT_LONG n_model, double x, double y, GMT_LONG newx, GMT_LONG newy)
{	/* Compute Legendre polynomials of x[i],y[j] as needed  */
	/* If either x or y has changed, compute new Legendre polynomials as needed  */

	if (newx) {
		if (n_model >= 2) pstuff[1] = x;
		if (n_model >= 5) pstuff[4] = 0.5*(3.0*pstuff[1]*pstuff[1] - 1.0);
		if (n_model >= 7) pstuff[6] = (5.0*pstuff[1]*pstuff[4] - 2.0*pstuff[1])/3.0;
	}
	if (newy) {
		if (n_model >= 3) pstuff[2] = y;
		if (n_model >= 6) pstuff[5] = 0.5*(3.0*pstuff[2]*pstuff[2] - 1.0);
		if (n_model >= 10) pstuff[9] = (5.0*pstuff[2]*pstuff[5] - 2.0*pstuff[2])/3.0;
	}
	/* In either case, refresh cross terms */

	if (n_model >= 4) pstuff[3] = pstuff[1]*pstuff[2];
	if (n_model >= 8) pstuff[7] = pstuff[4]*pstuff[2];
	if (n_model >= 9) pstuff[8] = pstuff[1]*pstuff[5];

	return;
}

void compute_trend (struct GMT_GRID *T, double *xval, double *yval, double *gtd, GMT_LONG n_model, double *pstuff)
{	/* Find trend from a model  */
	GMT_LONG row, col, k, ij;

	GMT_grd_loop (T, row, col, ij) {
		load_pstuff (pstuff, n_model, xval[col], yval[row], 1, (!(col)));
		T->data[ij] = 0.0;
		for (k = 0; k < n_model; k++) T->data[ij] += (float)(pstuff[k]*gtd[k]);
	}
}

void compute_resid (struct GMT_GRID *D, struct GMT_GRID *T, struct GMT_GRID *R)
{	/* Find residuals from a trend  */
	GMT_LONG row, col, ij;

	GMT_grd_loop (T, row, col, ij) R->data[ij] = D->data[ij] - T->data[ij];
}

void grd_trivial_model (struct GMT_GRID *G, double *xval, double *yval, double *gtd, GMT_LONG n_model)
{
	/* Routine to fit up elementary polynomial model of grd data, 
	model = gtd[0] + gtd[1]*x + gtd[2]*y + gtd[3] * x * y,
	where x,y are normalized to range [-1,1] and there are no
	NaNs in grid file, and problem is unweighted least squares.  */

	GMT_LONG row, col, ij;
	double x2, y2, sumx2 = 0.0, sumy2 = 0.0, sumx2y2 = 0.0;

	/* First zero the model parameters to use for sums */

	GMT_memset (gtd, n_model, double);

	/* Now accumulate sums */

	GMT_row_loop (G, row) {
		y2 = yval[row] * yval[row];
		GMT_col_loop (G, row, col, ij) {
			x2 = xval[col] * xval[col];
			sumx2 += x2;
			sumy2 += y2;
			sumx2y2 += (x2 * y2);
			gtd[0] += G->data[ij];
			if (n_model >= 2) gtd[1] += G->data[ij] * xval[col];
			if (n_model >= 3) gtd[2] += G->data[ij] * yval[row];
			if (n_model == 4) gtd[3] += G->data[ij] * xval[col] * yval[row];
		}
	}

	/* See how trivial it is?  */

	gtd[0] /= G->header->nm;
	if (n_model >= 2) gtd[1] /= sumx2;
	if (n_model >= 3) gtd[2] /= sumy2;
	if (n_model == 4) gtd[3] /= sumx2y2;

	return;
}

double compute_chisq (struct GMT_GRID *R, struct GMT_GRID *W, double scale)
{	/* Find Chi-Squared from weighted residuals  */
	GMT_LONG row, col, ij;
	double tmp, chisq = 0.0;

	GMT_grd_loop (R, row, col, ij) {
		if (GMT_is_fnan (R->data[ij])) continue;
		tmp = R->data[ij];
		if (scale != 1.0) tmp /= scale;
		tmp *= tmp;
		if (W->data[ij] != 1.0) tmp *= W->data[ij];
		chisq += tmp;
	}
	return (chisq);
}

double compute_robust_weight (struct GMT_GRID *R, struct GMT_GRID *W)
{	/* Find weights from residuals  */
	GMT_LONG row, col, j = 0, j2, ij;
	double r, mad, scale;

	GMT_grd_loop (R, row, col, ij) {
		if (GMT_is_fnan (R->data[ij])) continue;
		W->data[j++] = (float)fabs((double)R->data[ij]);
	}

	GMT_sort_array ((void *)R->data, j, GMT_FLOAT_TYPE);

	j2 = j / 2;
	mad = (j%2) ? W->data[j2] : 0.5 *(W->data[j2] + W->data[j2 - 1]);

	/* Adjust mad to equal Gaussian sigma */

	scale = 1.4826 * mad;

	/* Use weight according to Huber (1981), but squared */

	GMT_grd_loop (R, row, col, ij) {
		if (GMT_is_fnan (R->data[ij])) {
			W->data[ij] = R->data[ij];
			continue;
		}
		r = fabs (R->data[ij]) / scale;

		W->data[ij] = (float)((r <= 1.5) ? 1.0 : (3.0 - 2.25/r) / r);
	}
	return (scale);
}

void write_model_parameters (struct GMT_CTRL *GMT, double *gtd, GMT_LONG n_model)
{	/* Do reports if gmtdefs.verbose = NORMAL or above  */
	GMT_LONG i;
	char pbasis[10][16], format[BUFSIZ];

	sprintf (pbasis[0], "Mean");
	sprintf (pbasis[1], "X");
	sprintf (pbasis[2], "Y");
	sprintf (pbasis[3], "X*Y");
	sprintf (pbasis[4], "P2(x)");
	sprintf (pbasis[5], "P2(y)");
	sprintf (pbasis[6], "P3(x)");
	sprintf (pbasis[7], "P2(x)*P1(y)");
	sprintf (pbasis[8], "P1(x)*P2(y)");
	sprintf (pbasis[9], "P3(y)");

	sprintf(format, "Coefficient fit to %%s: %s\n", GMT->current.setting.format_float_out);
	for (i = 0; i < n_model; i++) GMT_message (GMT, format, pbasis[i], gtd[i]);

	return;
}

void load_gtg_and_gtd (struct GMT_GRID *G, double *xval, double *yval, double *pstuff, double *gtg, double *gtd, GMT_LONG n_model, struct GMT_GRID *W, GMT_LONG weighted)
{
	/* Routine to load the matrix G'G (gtg) and vector G'd (gtd)
	for the normal equations.  Routine uses indices i,j to refer
	to the grid file of data, and k,l to refer to the k_row, l_col
	of the normal equations matrix.  We need sums of [weighted]
	data and model functions in gtg and gtd.  We save time by
	loading only lower triangular part of gtg and then filling
	by symmetry after i,j loop.  */

	GMT_LONG row, col, k, l, n_used = 0, ij;

	/* First zero things out to start */

	GMT_memset (gtd, n_model, double);
	GMT_memset (gtg, n_model * n_model, double);

	/* Now get going.  Have to load_pstuff separately in i and j,
	   because it is possible that we skip data when i = 0.
	   Loop over all data */

	GMT_row_loop (G, row) {
		load_pstuff (pstuff, n_model, xval[0], yval[row], 0, 1);
		GMT_col_loop (G, row, col, ij) {

			if (GMT_is_fnan (G->data[ij]))continue;

			n_used++;
			load_pstuff (pstuff, n_model, xval[col], yval[row], 1, 0);

			if (weighted) {
				/* Loop over all gtg and gtd elements */
				for (k = 0; k < n_model; k++) {
					gtd[k] += (G->data[ij] * W->data[ij] * pstuff[k]);
					gtg[k] += (W->data[ij] * pstuff[k]);
					for (l = k; l < n_model; l++) gtg[k + l*n_model] += (pstuff[k]*pstuff[l]*W->data[ij]);
				}
			}
			else {	/* If !weighted  */
				/* Loop over all gtg and gtd elements */
				for (k = 0; k < n_model; k++) {
					gtd[k] += (G->data[ij] * pstuff[k]);
					gtg[k] += pstuff[k];
					for (l = k; l < n_model; l++) gtg[k + l*n_model] += (pstuff[k]*pstuff[l]);
				}
			}	/* End if  */
		}
	}	/* End of loop over data i,j  */

	/* Now if !weighted, use more accurate sum for gtg[0], and set symmetry */

	if (!weighted) gtg[0] = (double)n_used;

	for (k = 0; k < n_model; k++) {
		for (l = 0; l < k; l++) gtg[l + k*n_model] = gtg[k + l*n_model];
	}
	/* That is all there is to it!  */

	return;
}

#define Return(code) {Free_grdtrend_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdtrend (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {
	/* High-level function that implements the grdcontour task */

	GMT_LONG trivial, weighted, error = 0, i, k, ierror = 0, iterations;
	
	char format[BUFSIZ];

	double chisq, old_chisq, zero_test = 1.0e-08, scale = 1.0, dv;
	double *xval = NULL;	/* Pointer for array of change of variable: x[i]  */
	double *yval = NULL;	/* Pointer for array of change of variable: y[j]  */
	double *gtg = NULL;	/* Pointer for array for matrix G'G normal equations  */
	double *gtd = NULL;	/* Pointer for array for vector G'd normal equations  */
	double *old = NULL;	/* Pointer for array for old model, used for robust sol'n  */
	double *pstuff = NULL;	/* Pointer for array for Legendre polynomials of x[i],y[j]  */
	double wesn[4];		/* For optional subset specification */

	struct GRDTREND_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *R = NULL, *T = NULL, *W = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdtrend_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdtrend_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	GMT = GMT_begin_module (API, "GMT_grdtrend", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", "", options))) Return (error);
	Ctrl = (struct GRDTREND_CTRL *) New_grdtrend_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdtrend_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdtrend main code ----------------------------*/

	weighted = (Ctrl->N.robust || Ctrl->W.active);
	trivial = (Ctrl->N.value < 5 && !weighted);

	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);
	if (GMT_is_subset (G->header, wesn)) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Subset requested; make sure wesn matches header spacing */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);	/* Get subset */

	/* Check for NaNs (we include the pad for simplicity)  */
	i = 0;
	while (trivial && i < G->header->size) {
		if (GMT_is_fnan (G->data[i])) trivial = FALSE;
		i++;
	}

	/* Allocate other required arrays */

	T = GMT_create_grid (GMT);	/* Pointer for grid with array containing fitted surface  */
	GMT_memcpy (T->header, G->header, 1, struct GRD_HEADER);
	T->data = GMT_memory (GMT, NULL, G->header->size, float);
	if (Ctrl->D.active || Ctrl->N.robust) {	/* If !D but robust, we would only need to allocate the data array */
		R = GMT_create_grid (GMT);	/* Pointer for grid with array containing residual surface  */
		GMT_memcpy (R->header, G->header, 1, struct GRD_HEADER);
		R->data = GMT_memory (GMT, NULL, G->header->size, float);
	}
	xval = GMT_memory (GMT, NULL, G->header->nx, double);
	yval = GMT_memory (GMT, NULL, G->header->ny, double);
	gtg = GMT_memory (GMT, NULL, Ctrl->N.value*Ctrl->N.value, double);
	gtd = GMT_memory (GMT, NULL, Ctrl->N.value, double);
	old = GMT_memory (GMT, NULL, Ctrl->N.value, double);
	pstuff = GMT_memory (GMT, NULL, Ctrl->N.value, double);
	pstuff[0] = 1.0; /* This is P0(x) = 1, which is not altered in this program. */

	/* If a weight array is needed, get one */

	W = GMT_create_grid (GMT);	/* Pointer for grid with array containing data weights  */
	if (weighted) {
		GMT_LONG set_ones = TRUE;
		if (!GMT_access (GMT, Ctrl->W.file, R_OK)) {	/* We have weights on input  */
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->W.file), (void **)&W)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
			if (W->header->nx != G->header->nx || W->header->ny != G->header->ny)
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Input weight file does not match input data file.  Ignoring.\n");
			else {
				if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(Ctrl->W.file), (void **)&W)) Return (GMT_DATA_READ_ERROR);	/* Get data */
				set_ones = FALSE;
			}
		}
		if (set_ones) {
			W->data = GMT_memory (GMT, NULL, G->header->size, float);
			GMT_setnval (W->data, G->header->size, 1.0);
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	/* End of weight set up.  */

	/* Set up xval and yval lookup tables */

	dv = 2.0 / (double)(G->header->nx - 1);
	for (k = 0; k < G->header->nx - 1; k++) xval[k] = -1.0 + k * dv;
	dv = 2.0 / (double)(G->header->ny - 1);
	for (k = 0; k < G->header->ny - 1; k++) yval[k] = -1.0 + k * dv;
	xval[G->header->nx - 1] = yval[G->header->ny - 1] = 1.0;

	/* Do the problem */

	if (trivial) {
		grd_trivial_model (G, xval, yval, gtd, Ctrl->N.value);
		compute_trend (T, xval, yval, gtd, Ctrl->N.value, pstuff);
		if (Ctrl->D.active) compute_resid (G, T, R);
	}
	else {	/* Problem is not trivial  !!  */

		load_gtg_and_gtd (G, xval, yval, pstuff, gtg, gtd, Ctrl->N.value, W, weighted);
		GMT_gauss (GMT, gtg, gtd, Ctrl->N.value, Ctrl->N.value, zero_test, &ierror, 1);
		if (ierror) {
			GMT_report (GMT, GMT_MSG_FATAL, "Gauss returns error code %ld\n", ierror);
			return (EXIT_FAILURE);
		}
		compute_trend (T, xval, yval, gtd, Ctrl->N.value, pstuff);
		if (Ctrl->D.active || Ctrl->N.robust) compute_resid (G, T, R);

		if (Ctrl->N.robust) {
			chisq = compute_chisq (R, W, scale);
			iterations = 1;
			sprintf (format, "Robust iteration %%d:  Old Chi Squared: %s  New Chi Squared: %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			do {
				old_chisq = chisq;
				GMT_memcpy (old, gtd, Ctrl->N.value, double);
				scale = compute_robust_weight (R, W);
				load_gtg_and_gtd (G, xval, yval, pstuff, gtg, gtd, Ctrl->N.value, W, weighted);
				GMT_gauss (GMT, gtg, gtd, Ctrl->N.value, Ctrl->N.value, zero_test, &ierror, 1);
				if (ierror) {
					GMT_report (GMT, GMT_MSG_FATAL, "Gauss returns error code %ld\n", ierror);
					return (EXIT_FAILURE);
				}
				compute_trend (T, xval, yval, gtd, Ctrl->N.value, pstuff);
				compute_resid (G, T, R);
				chisq = compute_chisq (R, W, scale);
				GMT_report (GMT, GMT_MSG_NORMAL, format, GMT->init.progname, iterations, old_chisq, chisq);
				iterations++;
			} while (old_chisq / chisq > 1.0001);

			/* Get here when new model not significantly better; use old one */

			GMT_memcpy (gtd, old, Ctrl->N.value, double);
			compute_trend (T, xval, yval, gtd, Ctrl->N.value, pstuff);
			compute_resid (G, T, R);
		}
	}

	/* End of do the problem section.  */

	/* Get here when ready to do output */

	if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) write_model_parameters (GMT, gtd, Ctrl->N.value);
	if (Ctrl->T.file) {
		strcpy (T->header->title, "trend surface");
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->T.file, (void *)T);
	}
	if (Ctrl->D.file) {
		strcpy (R->header->title, "trend residuals");
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->D.file, (void *)R);
	}
	if (Ctrl->W.file && Ctrl->N.robust) {
		strcpy (W->header->title, "trend weights");
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->W.file, (void *)W);
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	/* That's all, folks!  */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	GMT_free_grid (GMT, &T, TRUE);
	GMT_free_grid (GMT, &W, TRUE);
	if (Ctrl->D.active || Ctrl->N.robust) GMT_free_grid (GMT, &R, TRUE);

	GMT_free (GMT, pstuff);
	GMT_free (GMT, gtd);
	GMT_free (GMT, gtg);
	GMT_free (GMT, old);
	GMT_free (GMT, yval);
	GMT_free (GMT, xval);

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

	Return (EXIT_SUCCESS);
}
