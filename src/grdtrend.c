/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Version:	6 API
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
 * 		as gmt_gauss in gmt_vector.c
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
 * routine, grdtrend_grd_trivial_model(), to handle this case.
 *
 * If the problem is more complex than the above trivial case,
 * (missing values, weighted problem, or n_model > 4), then
 * G'G is not trivial and we just naively accumulate sums in
 * the G'G matrix.  We hope that the changed coordinates in
 * [-1,1] will help the accuracy of the problem.  We also use
 * Legendre polynomials in this case so that the matrix elements
 * are conveniently sized near 0 or 1.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdtrend"
#define THIS_MODULE_MODERN_NAME	"grdtrend"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Fit trend surface to grids and compute residuals"
#define THIS_MODULE_KEYS	"<G{,DG),TG),WG(,WG)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RV"

#ifdef DOUBLE_PRECISION_GRID
#define fabsf(x) fabs(x)
#endif

typedef void (*p_to_eval_func) (double *, unsigned int, double, double, unsigned int, unsigned int);

struct GRDTREND_CTRL {	/* All control options for this program (except common args) */
	struct GRDTREND_In {
		bool active;
		char *file;
	} In;
	struct GRDTREND_D {	/* -D<diffgrid> */
		bool active;
		char *file;
	} D;
	struct GRDTREND_N {	/* -N[r]<n_model> */
		bool active;
		bool robust;
		bool x_only, y_only;
		unsigned int value;
	} N;
	struct GRDTREND_T {	/* -T<trend.grd> */
		bool active;
		char *file;
	} T;
	struct GRDTREND_W {	/* -W<weight.grd>[+s] */
		bool active;
		unsigned int mode;
		char *file;
	} W;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDTREND_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDTREND_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->W.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s -N<n_model>[+r] [-D<diffgrid>] [%s] [-T<trendgrid>] "
		"[%s] [-W<weightgrid>[+s]] [%s]\n", name, GMT_INGRID, GMT_Rgeo_OPT, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of input grid to fit trend to");
	GMT_Usage (API, 1, "\n-N<n_model>[+r][+x|y]");
	GMT_Usage (API, -2, "Fit a [robust] model with <n_model> terms; append +r for robust solution.  "
		"Optionally append +x OR +y to fit a model only along the x or y axes. "
		"Model parameters IDs are given as follows:");
	GMT_Usage (API, -3, "%s m1 + m2*x + m3*y + m4*x*y + m5*x^2 + m6*y^2 + m7*x^3 + m8*x^2*y + m9*x*y^2 + m10*y^3.", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "%s m1 + m2*x + m3*x^2 + m4*x^3 (for +x).", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "%s m1 + m2*y + m3*y^2 + m4*y^3 (for +y).", GMT_LINE_BULLET);
	GMT_Usage (API, -2, "E.g., a robust x/y planar trend is -N3+r while a cubic trend in x only is -N4+x.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-D<diffgrid>");
	GMT_Usage (API, -2, "Supply filename to write grid file of differences (input - trend).");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-T<trendgrid>");
	GMT_Usage (API, -2, "Supply filename to write grid file of trend.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W<weightgrid>[+s]");
	GMT_Usage (API, -2, "Supply filename if you want to [read and] write grid file of weights. "
		"If <weightgrid> can be read at run-time, and if robust = false, weighted problem will be solved. "
		"If robust = true, weights used for robust fit will be written to <weightgrid>. "
		"Append +s to read standard deviations from file and compute weights as 1/s^2.");
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdtrend and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	char *c = NULL;
	unsigned int n_errors = 0, n_files = 0, j;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->D.file))) n_errors++;
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				/* Must check for both -N[r]<n_model> and -N<n_model>[r] due to confusion */
				Ctrl->N.active = true;
				if (strchr (opt->arg, 'r')) Ctrl->N.robust = true;
				if (strstr (opt->arg, "+x"))
					Ctrl->N.x_only = true;
				else if (strstr (opt->arg, "+y"))
					Ctrl->N.y_only = true;
				j = (opt->arg[0] == 'r') ? 1 : 0;	/* GMT4 backwardness */
				if (opt->arg[j]) Ctrl->N.value = atoi (&opt->arg[j]);
				break;
			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->T.file))) n_errors++;
				break;
			case 'W':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg[0] != '+' && (c = strstr (opt->arg, "+s"))) {	/* Gave a file arg plus a +s modifier */
					Ctrl->W.mode = 2;
					c[0] = '\0';	/* Chop off modifier */
				}
				else
					Ctrl->W.mode = 1;
				/* OK if this file doesn't exist; we always write to that file on output */
				Ctrl->W.file = strdup (opt->arg);
				if (!gmt_access (GMT, Ctrl->W.file, R_OK)) {	/* FOund the file */
					if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->W.file))) n_errors++;
				}
				else {
					if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->W.file))) n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify an input grid file\n");
	if (Ctrl->N.x_only || Ctrl->N.y_only)
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.value == 0 || Ctrl->N.value > 4, "Option -N: Specify 1-4 model parameters when +x or +y are active\n");
	else
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.value == 0 || Ctrl->N.value > 10, "Option -N: Specify 1-10 model parameters\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#if 0
GMT_LOCAL void grdtrend_set_up_vals (double *val, unsigned int nval, double vmin, double vmax, double dv, unsigned int pixel_reg) {
	/* Store x[i], y[j] once for all to save time  */
	unsigned int i;
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
#endif

GMT_LOCAL void grdtrend_load_pstuff_xonly (double *pstuff, unsigned int n_model, double x, double y, unsigned int newx, unsigned int newy) {
	/* Compute Legendre polynomials of x[i] as needed  */
	/* If x has changed, compute new Legendre polynomials as needed.
	 * Remember: pstuff[0] == 1 throughout.   Here, n_models is in 1-4 range*/

	if (newx) {
		if (n_model > 1) pstuff[1] = x;
		if (n_model > 2) pstuff[2] = 0.5 * (3.0 * x * x - 1.0);
		if (n_model > 3) pstuff[3] = 0.5 * (5.0 * pow (x, 3.0) - 3.0 * x);
	}

	return;
}

GMT_LOCAL void grdtrend_load_pstuff_yonly (double *pstuff, unsigned int n_model, double x, double y, unsigned int newx, unsigned int newy) {
	/* Compute Legendre polynomials of y[j] as needed  */
	/* If y has changed, compute new Legendre polynomials as needed.
	 * Remember: pstuff[0] == 1 throughout.  Here, n_models is in 0-3 range */

	if (newy) {
		if (n_model > 1) pstuff[1] = y;
		if (n_model > 2) pstuff[2] = 0.5 * (3.0 * y * y - 1.0);
		if (n_model > 3) pstuff[3] = 0.5 * (5.0 * pow (y, 3.0) - 3.0 * y);
	}

	return;
}


GMT_LOCAL void grdtrend_load_pstuff_xy (double *pstuff, unsigned int n_model, double x, double y, unsigned int newx, unsigned int newy) {
	/* Compute Legendre polynomials of x[i],y[j] as needed  */
	/* If either x or y has changed, compute new Legendre polynomials as needed.
	 * Remember: pstuff[0] == 1 throughout.  */

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

GMT_LOCAL void grdtrend_compute_trend (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *Ctrl, struct GMT_GRID *T, double *xval, double *yval, double *gtd, double *pstuff, p_to_eval_func eval) {
	/* Find trend from a model  */
	unsigned int row, col, k;
	uint64_t ij;
	gmt_M_unused(GMT);

	gmt_M_grd_loop (GMT, T, row, col, ij) {
		(*eval) (pstuff, Ctrl->N.value, xval[col], yval[row], 1, (!(col)));
		T->data[ij] = 0.0f;
		for (k = 0; k < Ctrl->N.value; k++) T->data[ij] += (gmt_grdfloat)(pstuff[k]*gtd[k]);
	}
}

GMT_LOCAL void grdtrend_compute_resid (struct GMT_CTRL *GMT, struct GMT_GRID *D, struct GMT_GRID *T, struct GMT_GRID *R) {
	/* Find residuals from a trend  */
	unsigned int row, col;
	uint64_t ij;
	gmt_M_unused(GMT);

	gmt_M_grd_loop (GMT, T, row, col, ij) R->data[ij] = D->data[ij] - T->data[ij];
}

GMT_LOCAL void grdtrend_grd_trivial_model (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *Ctrl, struct GMT_GRID *G, double *xval, double *yval, double *gtd, unsigned int n_model) {
	/* Routine to fit up elementary polynomial model of grd data,
	model = gtd[0] + gtd[1]*x + gtd[2]*y + gtd[3] * x * y,
	where x,y are normalized to range [-1,1] and there are no
	NaNs in grid file, and problem is unweighted least squares.  */

	unsigned int row, col;
	uint64_t ij;
	double x2, y2, sumx2 = 0.0, sumy2 = 0.0, sumx2y2 = 0.0;
	gmt_M_unused(GMT);

	/* First zero the model parameters to use for sums */

	gmt_M_memset (gtd, n_model, double);

	/* Now accumulate sums */

	gmt_M_row_loop (GMT, G, row) {
		y2 = yval[row] * yval[row];
		gmt_M_col_loop (GMT, G, row, col, ij) {
			x2 = xval[col] * xval[col];
			sumx2 += x2;
			sumy2 += y2;
			gtd[0] += G->data[ij];
			if (Ctrl->N.x_only) {
				if (n_model == 2) gtd[1] += G->data[ij] * xval[col];
			}
			else if (Ctrl->N.y_only) {
				if (n_model == 2) gtd[1] += G->data[ij] * yval[row];
			}
			else {
				sumx2y2 += (x2 * y2);
				if (n_model >= 2) gtd[1] += G->data[ij] * xval[col];
				if (n_model >= 3) gtd[2] += G->data[ij] * yval[row];
				if (n_model == 4) gtd[3] += G->data[ij] * xval[col] * yval[row];
			}
		}
	}

	/* See how trivial it is?  */

	gtd[0] /= G->header->nm;
	if (Ctrl->N.x_only) {
		if (n_model == 2) gtd[1] /= sumx2;
	}
	else if (Ctrl->N.y_only) {
		if (n_model == 2) gtd[1] /= sumy2;
	}
	else {
		if (n_model >= 2) gtd[1] /= sumx2;
		if (n_model >= 3) gtd[2] /= sumy2;
		if (n_model == 4) gtd[3] /= sumx2y2;
	}

	return;
}

GMT_LOCAL double grdtrend_compute_chisq (struct GMT_CTRL *GMT, struct GMT_GRID *R, struct GMT_GRID *W, double scale) {
	/* Find Chi-Squared from weighted residuals  */
	unsigned int row, col;
	uint64_t ij;
	double tmp, chisq = 0.0;
	gmt_M_unused(GMT);

	gmt_M_grd_loop (GMT, R, row, col, ij) {
		if (gmt_M_is_fnan (R->data[ij])) continue;
		tmp = R->data[ij];
		if (scale != 1.0) tmp /= scale;
		tmp *= tmp;
		if (W->data[ij] != 1.0) tmp *= W->data[ij];
		chisq += tmp;
	}
	return (chisq);
}

GMT_LOCAL double grdtrend_compute_robust_weight (struct GMT_CTRL *GMT, struct GMT_GRID *R, struct GMT_GRID *W) {
	/* Find weights from residuals  */
	unsigned int row, col;
	uint64_t j = 0, j2, ij;
	gmt_grdfloat r, mad, scale;

	gmt_M_grd_loop (GMT, R, row, col, ij) {
		if (gmt_M_is_fnan (R->data[ij])) continue;
		W->data[j++] = fabsf (R->data[ij]);
	}

	gmt_sort_array (GMT, W->data, j, GMT_FLOAT);

	j2 = j / 2;
	mad = (j%2) ? W->data[j2] : 0.5f *(W->data[j2] + W->data[j2 - 1]);

	/* Adjust mad to equal Gaussian sigma */

	scale = 1.4826f * mad;

	/* Use weight according to Huber (1981), but squared */

	gmt_M_memset (W->data, W->header->size, gmt_grdfloat);	/* Wipe W clean */
	gmt_M_grd_loop (GMT, R, row, col, ij) {
		if (gmt_M_is_fnan (R->data[ij])) {
			W->data[ij] = R->data[ij];
			continue;
		}
		r = fabsf (R->data[ij]) / scale;

		W->data[ij] = (r <= 1.5f) ? 1.0f : (3.0f - 2.25f/r) / r;
	}
	return (scale);
}

GMT_LOCAL void grdtrend_write_model_parameters (struct GMT_CTRL *GMT, struct GRDTREND_CTRL *Ctrl, double *gtd) {
	/* Do reports if gmtdefs.verbose = NORMAL or above  */
	unsigned int i;
	char pbasis[10][16], format[GMT_BUFSIZ];

	sprintf (pbasis[0], "P0");
	if (!Ctrl->N.y_only)
		sprintf (pbasis[1], "P1(x)");
	if (Ctrl->N.x_only) {
		sprintf (pbasis[2], "P2(x)");
		sprintf (pbasis[3], "P3(x)");
	}
	else if (Ctrl->N.y_only) {
		sprintf (pbasis[1], "P1(y)");
		sprintf (pbasis[2], "P2(y)");
		sprintf (pbasis[3], "P3(y)");
	}
	else {
		sprintf (pbasis[2], "P1(y)");
		sprintf (pbasis[3], "P1(x)*P1(y)");
		sprintf (pbasis[4], "P2(x)");
		sprintf (pbasis[5], "P2(y)");
		sprintf (pbasis[6], "P3(x)");
		sprintf (pbasis[7], "P2(x)*P1(y)");
		sprintf (pbasis[8], "P1(x)*P2(y)");
		sprintf (pbasis[9], "P3(y)");
	}

	sprintf(format, "Coefficient fit to %%s: %s\n", GMT->current.setting.format_float_out);
	for (i = 0; i < Ctrl->N.value; i++) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, format, pbasis[i], gtd[i]);

	return;
}

GMT_LOCAL void grdtrend_load_gtg_and_gtd (struct GMT_CTRL *GMT, struct GMT_GRID *G, double *xval, double *yval, double *pstuff, double *gtg, double *gtd, unsigned int n_model, struct GMT_GRID *W, bool weighted, p_to_eval_func eval) {
	/* Routine to load the matrix G'G (gtg) and vector G'd (gtd)
	for the normal equations.  Routine uses indices row,col to refer
	to the grid file of data, and k,l to refer to the k_row, l_col
	of the normal equations matrix.  We need sums of [weighted]
	data and model functions in gtg and gtd.  We save time by
	loading only lower triangular part of gtg and then filling
	by symmetry after row,col loop.  */

	unsigned int row, col, k, l, n_used = 0;
	uint64_t ij;
	gmt_M_unused(GMT);

	/* First zero things out to start */

	gmt_M_memset (gtd, n_model, double);
	gmt_M_memset (gtg, n_model * n_model, double);

	/* Now get going.  Have to eval separately in i and j,
	   because it is possible that we skip data when i = 0.
	   Loop over all data */

	gmt_M_row_loop (GMT, G, row) {
		(*eval) (pstuff, n_model, xval[0], yval[row], 0, 1);
		gmt_M_col_loop (GMT, G, row, col, ij) {

			if (gmt_M_is_fnan (G->data[ij]))continue;

			n_used++;
			(*eval) (pstuff, n_model, xval[col], yval[row], 1, 0);

			if (weighted) {
				/* Loop over all gtg and gtd elements */
				gtd[0] += (G->data[ij] * W->data[ij]);	/* Implicitly multiply by pstuff[0] which is 1 */
				gtg[0] += W->data[ij];			/* Implicitly multiply by pstuff[0] which is 1 */
				for (k = 1; k < n_model; k++) {
					gtd[k] += (G->data[ij] * W->data[ij] * pstuff[k]);
					gtg[k*n_model] += (W->data[ij] * pstuff[k]);
					for (l = 1; l <= k; l++)
						gtg[k*n_model + l] += (pstuff[k]*pstuff[l]*W->data[ij]);
				}
			}
			else {	/* If !weighted  */
				/* Loop over all gtg and gtd elements; do gtg[0] afterwards */
				gtd[0] += G->data[ij];	/* Implicitly multiply by pstuff[0] which is 1 */
				for (k = 1; k < n_model; k++) {	/* Rows in GTG */
					gtd[k] += (G->data[ij] * pstuff[k]);
					gtg[k*n_model] += pstuff[k];
					for (l = 1; l <= k; l++)
						gtg[k*n_model + l] += (pstuff[k]*pstuff[l]);
				}
			}	/* End if  */
		}
	}	/* End of loop over data row,col  */

	/* Now if !weighted, use more accurate sum for gtg[0], and set symmetry */

	if (!weighted) gtg[0] = (double)n_used;

	/* Fill in upper triangular part of normal equation matrix G'*G */
	for (k = 0; k < n_model; k++) {	/* Rows */
		for (l = k + 1; l < n_model; l++) gtg[k*n_model + l] = gtg[l*n_model + k];
	}
	/* That is all there is to it!  */

	return;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdtrend (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdcontour task */

	bool trivial, weighted, set_ones = true;
	int error = 0;
	unsigned int row, col, iterations;

	uint64_t ij;

	char format[GMT_BUFSIZ];

	double chisq, old_chisq, scale = 1.0, dv;
	double *xval = NULL;	/* Pointer for array of change of variable: x[i]  */
	double *yval = NULL;	/* Pointer for array of change of variable: y[j]  */
	double *gtg = NULL;	/* Pointer for array for matrix G'G normal equations  */
	double *gtd = NULL;	/* Pointer for array for vector G'd normal equations  */
	double *old = NULL;	/* Pointer for array for old model, used for robust sol'n  */
	double *pstuff = NULL;	/* Pointer for array for Legendre polynomials of x[i],y[j]  */
	double wesn[4];		/* For optional subset specification */
	p_to_eval_func eval = NULL;		/* Pointer to chosen evaluation function */

	struct GRDTREND_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *R = NULL, *T = NULL, *W = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdtrend main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
	weighted = (Ctrl->N.robust || Ctrl->W.active);
	if (Ctrl->N.x_only) {
		eval = grdtrend_load_pstuff_xonly;
		trivial = (Ctrl->N.value < 3 && !weighted);
	}
	else if (Ctrl->N.y_only) {
		eval = grdtrend_load_pstuff_yonly;
		trivial = (Ctrl->N.value < 3 && !weighted);
	}
	else {
		eval = grdtrend_load_pstuff_xy;
		trivial = (Ctrl->N.value < 5 && !weighted);
	}

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (gmt_M_is_subset (GMT, G->header, wesn)) {	/* Subset requested; make sure wesn matches header spacing */
		if ((error = gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "")))
			Return (error);
	}
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {	/* Get subset */
		Return (API->error);
	}

	/* Check for NaNs (we include the pad for simplicity)  */
	ij = 0;
	while (trivial && ij < G->header->size) if (gmt_M_is_fnan (G->data[ij++])) trivial = false;

	/* Allocate other required arrays */

	if ((T = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G)) == NULL) Return (API->error);	/* Pointer for grid with array containing fitted surface  */
	if (Ctrl->D.active || Ctrl->N.robust) {	/* If !D but robust, we would only need to allocate the data array */
		if ((R = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G)) == NULL) Return (API->error);	/* Pointer for grid with array containing residual surface  */
	}
	xval = gmt_M_memory (GMT, NULL, G->header->n_columns, double);
	yval = gmt_M_memory (GMT, NULL, G->header->n_rows, double);
	gtg = gmt_M_memory (GMT, NULL, Ctrl->N.value*Ctrl->N.value, double);
	gtd = gmt_M_memory (GMT, NULL, Ctrl->N.value, double);
	old = gmt_M_memory (GMT, NULL, Ctrl->N.value, double);
	pstuff = gmt_M_memory (GMT, NULL, Ctrl->N.value, double);
	pstuff[0] = 1.0; /* This is P0(x) = 1, which is not altered in this program. */

	/* If a weight array is needed, get one */

	if (weighted) {
		if (!gmt_access (GMT, Ctrl->W.file, R_OK)) {	/* We have weights on input  */
			if ((W = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->W.file, NULL)) == NULL) {	/* Get header only */
				error = API->error;
				goto END;
			}
			if (W->header->n_columns != G->header->n_columns || W->header->n_rows != G->header->n_rows)
				GMT_Report (API, GMT_MSG_ERROR, "Input weight file does not match input data file.  Ignoring.\n");
			else {
				if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->W.file, W) == NULL) {	/* Get data */
					error = API->error;
					goto END;
				}
				gmt_M_str_free (Ctrl->W.file);	/* Prevent that the weights grid is overwritten later down */
				if (Ctrl->W.mode == 2) {	/* Convert sigmas to weights */
					gmt_M_grd_loop (GMT, W, row, col, ij) {
						W->data[ij] = (gmt_grdfloat)(1.0 / (W->data[ij] * W->data[ij]));
					}
				}
				set_ones = false;
			}
		}
		if (set_ones) {
			if ((W = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G)) == NULL) { /* Pointer for grid with unit weights  */
				error = API->error;
				goto END;
			}
			gmt_M_setnval (W->data, W->header->size, 1.0f);
		}
	}

	/* End of weight set up.  */

	/* Set up xval and yval lookup tables */

	dv = 2.0 / (double)(G->header->n_columns - 1);
	for (col = 0; col < G->header->n_columns - 1; col++) xval[col] = -1.0 + col * dv;
	xval[G->header->n_columns - 1] = 1.0;
	dv = 2.0 / (double)(G->header->n_rows - 1);
	for (row = 0; row < G->header->n_rows - 1; row++) yval[row] = -1.0 + row * dv;
	yval[G->header->n_rows - 1] = 1.0;
	/* In the above cases, this will cause the existence of a bad last row (or col)
	   but cannot set it to zero because: "grdtrend [ERROR]: Gauss returns error code 3".
	   Leaving last value assugned to a value, even if 1e-15, results in a bad last line. 
	   The solution is to call the grdtrend_fix_trend function, which is done in grdtrend_compute_trend
	*/
	xval[G->header->n_columns - 1] = yval[G->header->n_rows - 1] = 1.0;

	/* Do the problem */

	if (trivial) {
		grdtrend_grd_trivial_model (GMT, Ctrl, G, xval, yval, gtd, Ctrl->N.value);
		grdtrend_compute_trend (GMT, Ctrl, T, xval, yval, gtd, pstuff, eval);
		if (Ctrl->D.active) grdtrend_compute_resid (GMT, G, T, R);
	}
	else {	/* Problem is not trivial  !!  */
		int ierror;
		grdtrend_load_gtg_and_gtd (GMT, G, xval, yval, pstuff, gtg, gtd, Ctrl->N.value, W, weighted, eval);
		ierror = gmt_gauss (GMT, gtg, gtd, Ctrl->N.value, Ctrl->N.value, true);
		if (ierror) {
			GMT_Report (API, GMT_MSG_ERROR, "Gauss returns error code %d\n", ierror);
			error = GMT_RUNTIME_ERROR;
			goto END;
		}
		grdtrend_compute_trend (GMT, Ctrl, T, xval, yval, gtd, pstuff, eval);
		if (Ctrl->D.active || Ctrl->N.robust) grdtrend_compute_resid (GMT, G, T, R);

		if (Ctrl->N.robust) {
			chisq = grdtrend_compute_chisq (GMT, R, W, scale);
			iterations = 1;
			sprintf (format, "grdtrend: Robust iteration %%d:  Old Chi Squared: %s  New Chi Squared: %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			do {
				old_chisq = chisq;
				gmt_M_memcpy (old, gtd, Ctrl->N.value, double);
				scale = grdtrend_compute_robust_weight (GMT, R, W);
				grdtrend_load_gtg_and_gtd (GMT, G, xval, yval, pstuff, gtg, gtd, Ctrl->N.value, W, weighted, eval);
				ierror = gmt_gauss (GMT, gtg, gtd, Ctrl->N.value, Ctrl->N.value, true);
				if (ierror) {
					GMT_Report (API, GMT_MSG_ERROR, "Gauss returns error code %d\n", ierror);
					error = GMT_RUNTIME_ERROR;
					goto END;
				}
				grdtrend_compute_trend (GMT, Ctrl, T, xval, yval, gtd, pstuff, eval);
				grdtrend_compute_resid (GMT, G, T, R);
				chisq = grdtrend_compute_chisq (GMT, R, W, scale);
				GMT_Report (API, GMT_MSG_INFORMATION, format, iterations, old_chisq, chisq);
				iterations++;
			} while (old_chisq / chisq > 1.0001);

			/* Get here when new model not significantly better; use old one */

			gmt_M_memcpy (gtd, old, Ctrl->N.value, double);
			grdtrend_compute_trend (GMT, Ctrl, T, xval, yval, gtd, pstuff, eval);
			grdtrend_compute_resid (GMT, G, T, R);
		}
	}

	/* End of do the problem section.  */

	/* Get here when ready to do output */

	if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) grdtrend_write_model_parameters (GMT, Ctrl, gtd);
	if (Ctrl->T.file) {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "trend surface", T)) Return (API->error);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, T)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->T.file, T) != GMT_NOERROR) {
			error = API->error;
			goto END;
		}
	}
	else if (GMT_Destroy_Data (API, &T) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free T\n");
	}
	if (Ctrl->D.file) {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "trend residuals", R)) Return (API->error);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, R)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->D.file, R) != GMT_NOERROR) {
			error = API->error;
			goto END;
		}
	}
	else if (Ctrl->D.active || Ctrl->N.robust) {
		if (GMT_Destroy_Data (API, &R) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to free R\n");
		}
	}
	if (Ctrl->W.file && Ctrl->N.robust) {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "trend weights", W)) Return (API->error);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, W)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->W.file, W) != GMT_NOERROR) {
			error = API->error;
			goto END;
		}
	}
	else if (set_ones && GMT_Destroy_Data (API, &W) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free W\n");
	}

	/* That's all, folks!  */

END:
	gmt_M_free (GMT, pstuff);
	gmt_M_free (GMT, gtd);
	gmt_M_free (GMT, gtg);
	gmt_M_free (GMT, old);
	gmt_M_free (GMT, yval);
	gmt_M_free (GMT, xval);

	Return (error);
}
