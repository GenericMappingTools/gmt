/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2025 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: pshistogram.c -- a program for plotting histograms
 *
 * Author:	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "longopt/pshistogram_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"pshistogram"
#define THIS_MODULE_MODERN_NAME	"histogram"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Calculate and plot histograms"
#define THIS_MODULE_KEYS	"<D{,CC(,>X},>D),>DI"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYbdefhilopqstwxy" GMT_OPT("c")

/* Note: The NEEDS must be JR.  Although pshistogram can create a region from data, it
 * does so indirectly by building the histogram and setting the ymin/ymax that way, NOT by
 * reading y data (which is what a NEEDS of d or r would do).
 */

struct PSHISTOGRAM_CTRL {
	struct PSHISTOGRAM_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct PSHISTOGRAM_A {	/* -A */
		bool active;
	} A;
	struct PSHISTOGRAM_C {	/* -C<cpt>[+b] */
		bool active;
		bool binval;	/* Select CPT based on binned value (i.e., the hist count) and not the mid-point of the bin */
		char *file;
	} C;
	struct PSHISTOGRAM_D {	/* -D[+r][+f<font>][+o<off>][+b] */
		bool active;
		unsigned int mode;	/* 0 for horizontal, 1 for vertical */
		unsigned int just;	/* 0 for top of bar, 1 for below */
		struct GMT_FONT font;
		double offset;
	} D;
	struct PSHISTOGRAM_E {	/* -E<width>[u][+o<off>[u]] */
		bool active;
		bool do_offset, w_is_dim, o_is_dim;
		double width;
		double off;
	} E;
	struct PSHISTOGRAM_F {	/* -F */
		bool active;
	} F;
	struct PSHISTOGRAM_G {	/* -Gfill */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct PSHISTOGRAM_I {	/* -I[o] */
		bool active;
		unsigned int mode;
	} I;
	struct PSHISTOGRAM_L {	/* -Ll|h|b */
		bool active;
		unsigned int mode;
	} L;
	struct PSHISTOGRAM_N {	/* -N[<kind>]+p<pen>, <kind = 0,1,2 */
		bool active;
		bool selected[3];
		struct GMT_PEN pen[3];
	} N;
	struct PSHISTOGRAM_Q {	/* -Q[r] */
		bool active;
		int mode;
	} Q;
	struct PSHISTOGRAM_S {	/* -S */
		bool active;
	} S;
	struct PSHISTOGRAM_T {	/* -T<tmin/tmax/tinc>[+n] | -Tfile|list  */
		bool active;
		struct GMT_ARRAY T;
	} T;
	struct PSHISTOGRAM_W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSHISTOGRAM_Z {	/* -Z<type>[+w] */
		bool active;
		bool weights;
		unsigned int mode;
	} Z;
};

enum Pshistogram_mode {
	PSHISTOGRAM_COUNTS = 0,
	PSHISTOGRAM_FREQ_PCT,
	PSHISTOGRAM_LOG_COUNTS,
	PSHISTOGRAM_LOG_FREQ_PCT,
	PSHISTOGRAM_LOG10_COUNTS,
	PSHISTOGRAM_LOG10_FREQ_PCT};

enum Pshistogram_loc {
	PSHISTOGRAM_L2 = 0,
	PSHISTOGRAM_L1,
	PSHISTOGRAM_LMS};

enum Pshistogram_extreme {
	PSHISTOGRAM_LEFT = 1,
	PSHISTOGRAM_RIGHT,
	PSHISTOGRAM_BOTH};

struct PSHISTOGRAM_INFO {	/* Control structure for pshistogram */
	double yy0, yy1;
	double sum_w;
	double wesn[4];
	double *boxh;
	uint64_t n_boxes;
	uint64_t n_counted;
	bool center_box, weights;
	unsigned int hist_type;
	int cumulative;
	enum Pshistogram_extreme extremes;
	struct GMT_ARRAY *T;
};

#define LOG10_2 0.301029995664
#define LOG10_5 0.698970004336

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	unsigned int k;
	struct PSHISTOGRAM_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct PSHISTOGRAM_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.offset = 6.0 / 72.0;	/* 6 points */
	C->D.font = GMT->current.setting.font_annot[GMT_PRIMARY];		/* Default font */
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Do not fill is default */
	C->W.pen = GMT->current.setting.map_default_pen;
	for (k = 0; k < 3; k++) C->N.pen[k] = GMT->current.setting.map_default_pen;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->C.file);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

/* For some useful discussion regarding the next to functions, see the postings on this issue:
 * https://github.com/GenericMappingTools/gmt/issues/5343
 */

GMT_LOCAL int64_t pshistogram_get_constant_bin (struct GMT_ARRAY *T, double x, int64_t dummy) {
	/* Find the bin for this value of x when all bins have constant width.
	 * If x falls outside our range then we return -1 or n
	 * The left boundary array index == bin index.
	 * One caveat for this approach is that if the data very often fall exactly on a bin
	 * boundary then the rounding schemes will end up alternating every bin.  Of course,
	 * in this case probably -F should be used.  To protect from such cases we only use
	 * this function for large data set where speed may be important. */
	int64_t bin;
	gmt_M_unused (dummy);
	if (x < T->min)
		bin = -1;
	else if (x > T->max)
		bin = T->n;
	else
		bin = (int64_t)floor ((x - T->min) / T->inc);
	return bin;
}

GMT_LOCAL int64_t pshistogram_get_variable_bin (struct GMT_ARRAY *T, double x, int64_t last_bin) {
	/* Find the bin for this value of x.  If x falls outside our range then
	 * we return -1 or n.  The left boundary array index == bin index.
	 * We also use this function even if the bins are equidistant as the
	 * approach will always round values landing on bin boundaries to the
	 * same side (in contrast to pshistogram_get_constant_bin) */
	int64_t bin = last_bin;
	while (bin >= 0 && x < T->array[bin]) bin--;	/* Need a previous bin */
	if (bin != last_bin) return bin;		/* Means we searched to the left and either found the bin or ran off and got -1 */
	/* We come here when no revised bin was computed (so bin == last_bin).  We now search to the right */
	while (bin < (int64_t)T->n && x >= T->array[bin+1]) bin++;	/* Need a later bin */
	return bin;	/* Either a valid bin or F->T.n (which is 1 larger than n_boxes) */
}

#define BIN_FASTER_IF_THIS_LARGE	1000000	/* If you bin a million points then bin rounding details won't matter */

GMT_LOCAL int pshistogram_fill_boxes (struct GMT_CTRL *GMT, struct PSHISTOGRAM_INFO *F, double *data, double *weights, uint64_t n) {

	double w, b0, b1, count_sum;
	uint64_t ibox, i;
	int64_t sbox, last_box = 0, hi_bin = F->T->n - 2;
	int64_t (*pshistogram_get_bin) (struct GMT_ARRAY *, double, int64_t);	

	gmt_M_tic (GMT);
	F->n_boxes = F->T->n - 1;	/* One less than the bin boundaries */
	F->boxh = gmt_M_memory (GMT, NULL, F->n_boxes, double);
	F->n_counted = 0;
	/* Pick variable bin search unless not variable bounds and the data set is large */
	pshistogram_get_bin = (F->T->var_inc || n < BIN_FASTER_IF_THIS_LARGE) ? &pshistogram_get_variable_bin : &pshistogram_get_constant_bin;

	/* First fill boxes with counts  */

	for (i = 0; i < n; i++) {
		sbox = pshistogram_get_bin (F->T, data[i], last_box);	/* Get the bin where this data point falls */
		if (sbox < 0) {	/* Extreme value left of first bin; check if -W was set */
			if ((F->extremes & PSHISTOGRAM_LEFT) == 0) continue;	/* No, we skip this value */
			sbox = 0;	/* Put in first bin instead */
		}
		ibox = (uint64_t)sbox;	/* We know sbox is positive now */
		if (ibox >= F->n_boxes) {	/* Extreme value right of last bin; check if -W was set */
			if ((F->extremes & PSHISTOGRAM_RIGHT) == 0) continue;	/* No, we skip this value */
			ibox = hi_bin;	/* Put in last bin instead */
		}
		w = (weights) ? weights[i] : 1.0;
		F->boxh[ibox] += w;
		F->n_counted++;
		F->sum_w += w;
		last_box = sbox;
	}

	if (F->cumulative) {
		for (ibox = 0, count_sum = b0 = 0.0; ibox < F->n_boxes; ibox++) {
			count_sum += F->boxh[ibox];
			F->boxh[ibox] = count_sum;
		}
		b1 = count_sum;
		if (F->cumulative == -1) {	/* Reverse cumulative */
			for (ibox = 0; ibox < F->n_boxes; ibox++)
				F->boxh[ibox] = count_sum - F->boxh[ibox];
		}
	}
	else {
		b0 = F->sum_w;
		for (ibox = 0, b1 = 0.0; ibox < F->n_boxes; ibox++) {
			if (b0 > F->boxh[ibox]) b0 = F->boxh[ibox];
			if (b1 < F->boxh[ibox]) b1 = F->boxh[ibox];
		}
	}

	/* Now find out what the min max y will be  */

	if (b0 > 0) {
		if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
			F->yy0 = d_log1p (GMT, b0);
		else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
			F->yy0 = d_log101p (GMT, b0);
		else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
			F->yy0 = (F->sum_w > 0.0) ? (100.0 * b0) / F->sum_w : 0.0;
		else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
			F->yy0 = (F->sum_w > 0.0) ? d_log1p (GMT, 100.0 * b0 / F->sum_w) : 0.0;
		else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
			F->yy0 = (F->sum_w > 0.0) ? d_log101p (GMT, 100.0 * b0 / F->sum_w) : 0.0;
		else
			F->yy0 = b0;
	}
	else
		F->yy0 = 0.0;
	if (b1 > 0) {
		if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
			F->yy1 = d_log1p (GMT, b1);
		else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
			F->yy1 = d_log101p (GMT, b1);
		else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
			F->yy1 = (F->sum_w > 0.0) ? (100.0 * b1) / F->sum_w : 0.0;
		else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
			F->yy1 = (F->sum_w > 0.0) ? d_log1p (GMT, 100.0 * b1 / F->sum_w) : 0.0;
		else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
			F->yy1 = (F->sum_w > 0.0) ? d_log101p (GMT, 100.0 * b1 / F->sum_w) : 0.0;
		else
			F->yy1 = b1;
	}
	else
		F->yy1 = 0.0;

	gmt_M_toc (GMT, "After filling bin array");

	return (0);
}

GMT_LOCAL double pshistogram_set_xy_array (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *Ctrl, struct PSHISTOGRAM_INFO *F, uint64_t ibox, double *x, double *y, double *px, double *py) {
	/* Compute the x- and y-coordinates for this bar given bin ibox and return polygon coordinates via px, py.
	 * We also return the z-value of the bar for CPT lookup purposes. */
	unsigned int i;
	double dx, off, xx, yy, zval;

	x[0] = F->T->array[ibox];
	x[1] = F->T->array[ibox+1];
	dx = x[1] - x[0];	/* This box width */
	if (x[0] < F->wesn[XLO]) x[0] = F->wesn[XLO];
	if (x[1] > F->wesn[XHI]) x[1] = F->wesn[XHI];
	x[2] = x[1];
	x[3] = x[0];
	y[0] = y[1] = F->wesn[YLO];
	if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
		y[2] = d_log1p (GMT, F->boxh[ibox]);
	else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
		y[2] = d_log101p (GMT, F->boxh[ibox]);
	else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
		y[2] = (100.0 * F->boxh[ibox]) / F->sum_w;
	else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
		y[2] = d_log1p (GMT, 100.0 * F->boxh[ibox] / F->sum_w );
	else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
		y[2] = d_log101p (GMT, 100.0 * F->boxh[ibox] / F->sum_w );
	else
		y[2] = F->boxh[ibox];

	/* For cpt purposes we either return counts or percent */
	if (F->hist_type == PSHISTOGRAM_FREQ_PCT || F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT || F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
		zval = (100.0 * F->boxh[ibox]) / F->sum_w;
	else
		zval = F->boxh[ibox];

	y[3] = y[2];
	if (Ctrl->E.active) {	/* Adjust histogram plot width [and possibly shift positions] if they are given in data units */
		if (!Ctrl->E.w_is_dim) {	/* Must adjust this bins x-coords to have the given x-width instead */
			/* dx is current width in x-units, shift/center to use the new width */
			off = (dx - Ctrl->E.width) / 2.0;	/* Adjustment to center the new narrower bin */
			x[0] += off;	x[3] += off;
			x[1] -= off;	x[2] -= off;
		}
		if (Ctrl->E.do_offset && !Ctrl->E.o_is_dim) {	/* Must adjust this bins x-coords for this x-shift */
			for (i = 0; i < 4; i++) x[i] += Ctrl->E.off;
		}
	}
	/* Now convert locations to plot coordinates */
	for (i = 0; i < 4; i++) {
		gmt_geo_to_xy (GMT, px[i], py[i], &xx, &yy);
		px[i] = xx;	py[i] = yy;
	}
	dx = px[1] - px[0];	/* Update bar width, now in plot units */
	if (Ctrl->E.active) {	/* Adjust histogram plot width and possibly shift position if they are given in plot units (c|i|p)*/
		if (Ctrl->E.w_is_dim) {	/* Must adjust this bins x-coords to have this x-width instead */
			/* dx is current width in plot-units, shift/center to use the new width */
			off = (dx - Ctrl->E.width) / 2.0;	/* Adjustment to center the new narrower bin */
			px[0] += off;	px[3] += off;
			px[1] -= off;	px[2] -= off;
		}
		if (Ctrl->E.do_offset && Ctrl->E.o_is_dim) {	/* Must adjust this bins x-coords for this shift */
			for (i = 0; i < 4; i++) px[i] += Ctrl->E.off;
		}
	}
	return (zval);
}

GMT_LOCAL double pshistogram_plot_boxes (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct PSHISTOGRAM_CTRL *Ctrl, struct GMT_PALETTE *P, struct PSHISTOGRAM_INFO *F, struct PSHISTOGRAM_D *D) {
	int k = 0, index, fmode = 0, label_justify;
	uint64_t ibox;
	char label[GMT_LEN64] = {""};
	bool first = true, stairs = Ctrl->S.active, flip_to_y = Ctrl->A.active, draw_outline = Ctrl->W.active, cpt = Ctrl->C.active;
	double area = 0.0, rgb[4], x[4], y[4], bin_width, xval, zval, cval, label_angle = 0.0, *px = NULL, *py = NULL;
	double plot_x = 0.0, plot_y = 0.0, *xpol = NULL, *ypol = NULL;
	struct GMT_FILL *f = NULL;
	struct GMT_PEN *pen = &Ctrl->W.pen;
	struct GMT_FILL *fill = &Ctrl->G.fill;

	if (gmt_M_is_dnan (D->font.size))	/* Did not specify another font and this one was NaN in modern mode */
		D->font = GMT->current.setting.font_annot[GMT_PRIMARY];		/* Update font */

	if (flip_to_y) {	/* Trick by cross-referencing x with y for horizontal bars */
		px = y;
		py = x;
	}
	else {	/* Normal vertical bars */
		px = x;
		py = y;
	}

	if (stairs && F->n_boxes) {	/* Build the outline polygon */
		xpol = gmt_M_memory (GMT, NULL, 2*(F->n_boxes+1), double);
		ypol = gmt_M_memory (GMT, NULL, 2*(F->n_boxes+1), double);
	}

	if (draw_outline) gmt_setpen (GMT, pen);
	if (!cpt)	/* Just set fill once since constant for all bars */
		gmt_setfill (GMT, fill, draw_outline);

	/* First lay down the bars or curve */
	for (ibox = 0; ibox < F->n_boxes; ibox++) {
		if (stairs || F->boxh[ibox]) {
			bin_width = F->T->array[ibox+1] - F->T->array[ibox];
			xval = 0.5 * (F->T->array[ibox] + F->T->array[ibox+1]);
			if (F->cumulative)
				area = F->boxh[ibox];	/* Just pick up the final bin as it has the entire sum */
			else	/* Add up as we go along */
				area += bin_width * F->boxh[ibox];
			zval = pshistogram_set_xy_array (GMT, Ctrl, F, ibox, x, y, px, py);	/* Get polygon coordinates for this bar in plot units */

			if (stairs) {	/* Need to build up the full cumulative polygon one step at the time */
				if (first) {	/* Initialization of start point */
					first = false;
					xpol[k] = px[0];	ypol[k++] = py[0];
				}
				xpol[k] = px[3];	ypol[k++] = py[3];
				xpol[k] = px[2];	ypol[k++] = py[2];
				/* The final polygon will be plotted after the loop */
			}
			else if (cpt) {	/* Each bar will have a unique color based on its value */
				cval = (Ctrl->C.binval) ? zval : xval;	/* Used for cpt lookup */

				index = gmt_get_rgb_from_z (GMT, P, cval, rgb);
				f = gmt_M_get_cptslice_pattern (P,index);
				if (f)	/* Pattern */
					gmt_setfill (GMT, f, draw_outline);
				else
					PSL_setfill (PSL, rgb, draw_outline);
				PSL_plotpolygon (PSL, px, py, 4);
			}
			else
				PSL_plotpolygon (PSL, px, py, 4);
		}
	}
	if (stairs && F->n_boxes) {	/* Finalize cumulative polygon and plot it */
		xpol[k] = px[1];	ypol[k++] = py[1];
		PSL_plotpolygon (PSL, xpol, ypol, k);
		gmt_M_free (GMT, xpol);
		gmt_M_free (GMT, ypol);
	}

	/* If -D then place labels */
	if (D->active) {	/* Place label, so set font */
		fmode = gmt_setfont (GMT, &D->font);
		if (D->just) {	/* Want labels beneath the bar, not above */
			label_justify = (flip_to_y) ? PSL_MR: PSL_TC;
			if (D->mode) label_justify = (flip_to_y) ? PSL_TC : PSL_MR;
			if (D->mode) label_angle = (flip_to_y) ? -90.0 : 90.0;
		}
		else {	/* Want labels above the bar */
			label_justify = (flip_to_y) ? PSL_ML : PSL_BC;
			if (D->mode) label_justify = (flip_to_y) ? PSL_BC : PSL_ML;
			if (D->mode) label_angle = (flip_to_y) ? -90.0 : 90.0;
		}
		for (ibox = 0; ibox < F->n_boxes; ibox++) {
			if (stairs || F->boxh[ibox] > 0.0) {
				(void)pshistogram_set_xy_array (GMT, Ctrl, F, ibox, x, y, px, py);	/* Get polygon coordinates for this bar in plot units */
				/* Place label */
				if (flip_to_y) {
					plot_y = 0.5 * (py[0] + py[1]);
					plot_x = (D->just) ? px[0] - D->offset : px[2] + D->offset;
				}
				else {
					plot_x = 0.5 * (px[0] + px[1]);
					plot_y = (D->just) ? py[0] - D->offset : py[2] + D->offset;
				}
				if (!F->weights) {
					sprintf (label, "%d", irint (F->boxh[ibox]));
					PSL_plottext (PSL, plot_x, plot_y, D->font.size, label, label_angle, label_justify, fmode);
				}
			}
		}
	}

	return (area);
}

GMT_LOCAL int pshistogram_get_loc_scl (struct GMT_CTRL *GMT, double *data, uint64_t n, bool selected[], double *stats) {
	/* Returns stats[] = L2, L1, LMS location, L2, L1, LMS scale as requested */

	uint64_t i, j = n / 2;
	unsigned int n_multiples = 0;
	double dx;

	if (n < 3) return (-1);

	if (GMT->common.w.active) {	/* Test wrapping on circle */
		double *d = gmt_M_memory (GMT, NULL, n, double);
		double f = 360.0 / GMT->current.io.cycle_range;	/* COnvert data to a 0-360 circular data set */
		for (i = 0; i < n; i++)
			d[i] = f * data[i];
		stats[0] = gmt_von_mises_mu_and_kappa (GMT, d, NULL, n, &stats[3]);
		stats[6] = f;	/* Save this here since probably needed to draw the Von Mises curve to convert to 0-360 angles */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "For wrapped data: mu = %g kappa = %g\n", stats[0], stats[3]);
		gmt_M_free (GMT, d);
		return (0);	/* Since only L2 solution is available */
	}
	gmt_M_tic (GMT);	/* Initialize elapsed time */

	if (selected[PSHISTOGRAM_L1] || selected[PSHISTOGRAM_LMS])	/* Must sort array */
		gmt_sort_array (GMT, data, n, GMT_DOUBLE);

	if (selected[PSHISTOGRAM_L1]) {	/* Get median */
		stats[1] = (n%2) ? data[j] : (0.5 * (data[j] + data[j-1]));
		/* Get MAD for L1 */
		gmt_getmad (GMT, data, n, stats[1], &stats[4]);
	}

	if (selected[PSHISTOGRAM_LMS]) {	/* Get mode */
		gmt_mode (GMT, data, n, j, 0, 0, &n_multiples, &stats[2]);
		if (n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "The histogram has multiple (%d) modes (peaks)\n", n_multiples);
		/* Get LMSscale for mode */
		gmt_getmad (GMT, data, n, stats[2], &stats[5]);
	}

	/* Calculate mean and stdev in two passes to minimize risk of overflow */

	if (selected[PSHISTOGRAM_L2]) {	/* Get L2 statistics */
		stats[0] = stats[3] = 0.0;
		for (i = 0; i < n; i++) stats[0] += data[i];	/* Sum up the data */
		stats[0] /= n;	/* This is the mean value */
		for (i = 0; i < n; i++) {
			dx = data[i] - stats[0];
			stats[3] += (dx * dx);
		}
		stats[3] = sqrt (stats[3] / (n - 1));
	}

	gmt_M_toc (GMT, "After pshistogram_get_loc_scl");	/* Report elapsed time */

	return (0);
}

GMT_LOCAL bool pshistogram_new_syntax (struct GMT_CTRL *GMT, char *L, char *T, char *W) {
	 /* If there is no -T then we know for sure it is old syntax (or bad arguments).
	 * If there is a -T then -T<col> vs -T<width> cannot uniquely be told apart (e.g., -T2).
	 * Thus, here we need to examine the other options.  If there is no -W then it means
	 * new syntax and no pen was set.  If -L is given and it is -Lh|l|b then it is new syntax,
	 * else -L must be the pen and it is old syntax.  The remaining case to consider is this:
	 * -T<number> -W<something>
	 * if -W contains +l|h|b then it is old syntax, and if there is a trailing unit c,i,p then
	 * is a pen and hence new syntax.  Thus, things like -T1 -W2 cannot be uniquely identified.
	 * In that case all we can do is warn the user as to how we interpreted their command line. */
	double w_val, t_val;
	if (T == NULL) return false;	/* Cannot be new syntax since -T is required */
	if (W == NULL) return true;	/* Cannot be old syntax since -W is required */
	if (L && strchr ("bhl", L[0])) return true;	/* Gave -Lb|h|l so clearly new syntax */
	if (L) return false;				/* Here, must have given -L<pen> */
	if (W && gmt_found_modifier (GMT, W, "bhl")) return false;	/* Gave -W<width>+b|h|l */
	if (W && strchr (GMT_DIM_UNITS, W[strlen(W)-1])) return true;	/* Must have given a -W<pen> */
	if (W && strchr (W, ',')) return true;	/* Must have given a -W<pen>,<color> */
	/* Unclear, get -T and -W args and see if we can learn from their values */
	w_val = atof (W);	t_val = atof (T);
	if (w_val == 0.0) return true;	/* Must have given a zero pen width (faint) */
	if (fabs (rint (t_val) - t_val)) return true;	/* Argument to -T is not an integer, hence new style */
	if (t_val > 5) {	/* Here we must guess that 6 is too large to be a column entry and hence it is a new syntax */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Cannot tell if -T%s -W%s is new or deprecated syntax; selected new.\n", T, W);
		return true;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Cannot tell if -T%s -W%s is new or deprecated syntax; selected deprecated.\n", T, W);
		return false;
	}
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s -T[<min>/<max>/]<inc>[+i|n] [-A] [%s] [-C<cpt>[+b]] [-D[+b][+f<font>][+o<off>][+r]] "
		"[-E<width>[+o<offset>]] [-F] [-G<fill>] [-I[o|O]] %s[-Ll|h|b] [-N[<mode>][+p<pen>]] %s%s[-Q[r]] [%s] [-S] [%s] [%s] "
		"[-W<pen>] [%s] [%s] [-Z[<mode>][+w]] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_Jx_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_Rx_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT,
		API->c_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_s_OPT,
		GMT_t_OPT, GMT_w_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, 1, "\n-Jx|X<x-scl>|<width>[/<y-scl>|<height>] for Cartesian scaling.");
	GMT_Usage (API, 1, "\n-T[<min>/<max>/]<inc>[+i|n]");
	GMT_Usage (API, -2, "Make evenly spaced bin boundaries from <min> to <max> by <inc>. "
		"If <min>/<max> is not given then boundaries in -R is used. "
		"For absolute time bins, append a valid time unit (%s) to the increment.", GMT_TIME_UNITS_DISPLAY);
	GMT_Usage (API, 3, "+n Indicate <inc> is the number of bin boundaries to produce instead.");
	GMT_Usage (API, 3, "+i Indicate <inc> is the reciprocal of desired <inc> (e.g., 3 for 0.3333.....).");
	GMT_Usage (API, -2, "Alternatively, give a file with bin boundaries in the first column, or a comma-separate list of values.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A Plot horizontal bars, i.e., flip x and y axis [Default is vertical].");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<cpt>[+b]");
	GMT_Usage (API, -2, "Use CPT to assign color to bars based on the mid-bar coordinate.  Alternatively, append +b "
		"to assign color based on the histogram value instead (count or percent only; see -Z).");
	GMT_Usage (API, 1, "\n-D[+b][+f<font>][+o<off>][+r]");
	GMT_Usage (API, -2, "Place histogram count labels on top of each bar; optionally append modifiers:");
	GMT_Usage (API, 3, "+b Place the labels beneath the bars [above].");
	GMT_Usage (API, 3, "+f Sets the label <font> [FONT_ANNOT_PRIMARY].");
	GMT_Usage (API, 3, "+o Sets the offset <off> between bar and label [6p].");
	GMT_Usage (API, 3, "+r Rotate the label to be vertical [horizontal].");
	GMT_Usage (API, 1, "\n-E<width>[+o<offset>]");
	GMT_Usage (API, -2, "Use custom bar <width> and optionally <offset>. "
		"By default, the bar width is implicitly set via -T and the offset is zero. "
		"Append desired bar width in data units, or append a valid unit (%s) for a fixed width.", GMT_DIM_UNITS_DISPLAY);
	GMT_Usage (API, 3, "+o Add an offset in data units, or append a valid unit (%s) for a fixed offset [0].", GMT_DIM_UNITS_DISPLAY);
	GMT_Usage (API, 1, "\n-F The bin boundaries given should be considered bin centers instead.");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Select color/pattern for columns.");
	GMT_Usage (API, 1, "\n-I[o|O]");
	GMT_Usage (API, -2, "Inquire about min/max x and y.  No plotting is done. Optionally append a directive:");
	GMT_Usage (API, 3, "o: Output the resulting x, y data.");
	GMT_Usage (API, 3, "O: Output all resulting x, y data even with y=0.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-Ll|h|b");
	GMT_Usage (API, -2, "Append l|h|b to place extreme values in the first, last, or both bins [skip extremes].");
	GMT_Usage (API, 1, "\n-N[<mode>][+p<pen>]");
	GMT_Usage (API, -2, "Draw the equivalent normal distribution; append desired pen [0.25p,black]. "
		"The <mode> selects which central location and scale to use:");
	GMT_Usage (API, 3, "0: Mean and standard deviation [Default].");
	GMT_Usage (API, 3, "1: Median and L1 scale (MAD w.r.t. median).");
	GMT_Usage (API, 3, "2: LMS mode and LMS scale (MAD w.r.t. mode).");
	GMT_Usage (API, -2, "Note: The -N option may be repeated to draw several of these curves.");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q[r]");
	GMT_Usage (API, -2, "Plot a cumulative histogram; append r for reverse cumulative histogram. "
		"Note: If neither -R nor -I are set, w/e/s/n will be based on input data.");
	GMT_Usage (API, 1, "\n-S Draw a stairs-step diagram [Default is bar histogram].");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Specify pen for histogram outline or stair-step curves.", NULL, 0);
	GMT_Option (API, "X");
	GMT_Usage (API, 1, "\n-Z[<mode>][+w]");
	GMT_Usage (API, -2, "Choose type of vertical axis. Select from these modes:");
	GMT_Usage (API, 3, "0: Counts [Default].");
	GMT_Usage (API, 3, "1: Frequency percent.");
	GMT_Usage (API, 3, "2: Log (1+counts).");
	GMT_Usage (API, 3, "3: Log (1+frequency percent).");
	GMT_Usage (API, 3, "4: Log10 (1+counts).");
	GMT_Usage (API, 3, "5: Log10 (1+frequency percent).");
	GMT_Usage (API, -2, "Append +w to sum bin weights in 2nd column rather than counts.");
	GMT_Option (API, "bi2,c,di,e,f,h,i,l,o,p,qi,s,t,w,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pshistogram and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, mode = 0, pos = 0;
	int sval;
	size_t L;
	char *c = NULL, *l_arg = NULL, *t_arg = NULL, *w_arg = NULL, p[GMT_BUFSIZ] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files after checking they exist */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;
			case '>':	/* Got named output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Out.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->Out.file));
				break;

			/* Processes program-specific parameters */

			case 'A':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'C':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				if (opt->arg[0] && (c = strstr (opt->arg, "+b"))) {
					Ctrl->C.binval = true;
					c[0] = '\0';	/* Remove modifier */
				}
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				while (gmt_getmodopt (GMT, 'D', opt->arg, "bfor", &pos, p, &n_errors) && n_errors == 0) {	/* Looking for +b, +f, +o, +r */
					switch (p[0]) {
						case 'b':	/* beneath */
							Ctrl->D.just = 1;
							break;
						case 'o':	/* offset */
							Ctrl->D.offset = gmt_M_to_inch (GMT, &p[1]);
							break;
						case 'f':	/* offset */
							n_errors += gmt_getfont (GMT, &p[1], &(Ctrl->D.font));
							break;
						case 'r':	/* rotate */
							Ctrl->D.mode = 1;
							break;
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				break;
			case 'E':	/* Alternative histogram bar width */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if ((c = strstr (opt->arg, "+o"))) {	/* Asking for offset */
					Ctrl->E.do_offset = true;
					L = strlen (c);
					if (strchr (GMT_DIM_UNITS, c[L-1])) {	/* In plot-dimension unit */
						Ctrl->E.off = gmt_M_to_inch (GMT, &c[2]);
						Ctrl->E.o_is_dim = true;
					}
					else
						Ctrl->E.off = atof (&c[2]);	/* In data units */
					c[0] = '\0';	/* Chop off the modifier */
				}
				if (opt->arg[0]) {	/* Gave a different bar width */
					L = strlen (opt->arg);
					if (strchr (GMT_DIM_UNITS, opt->arg[L-1])) {	/* In plot-dimension unit */
						Ctrl->E.width = gmt_M_to_inch (GMT, opt->arg);
						Ctrl->E.w_is_dim = true;
					}
					else
						Ctrl->E.width = atof (opt->arg);	/* In data units */
				}
				if (c) c[0] = '+';	/* Restore the modifier */
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				if (opt->arg[0] == 'o') Ctrl->I.mode = 1;
				if (opt->arg[0] == 'O') Ctrl->I.mode = 2;
				break;
			case 'L':		/* Set line attributes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				l_arg = opt->arg;
				break;
			case 'N':		/* Draw normal distribution */
				Ctrl->N.active = true;
				switch (opt->arg[0]) {	/* See which distribution to draw */
					case '0': case '+': case '\0': mode = PSHISTOGRAM_L2; break;
					case '1': mode = PSHISTOGRAM_L1;	break;
					case '2': mode = PSHISTOGRAM_LMS;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -N: mode %c unrecognized.\n", opt->arg[0]);
						n_errors++;
						break;
				}
				Ctrl->N.selected[mode] = true;
				if ((c = strstr (opt->arg, "+p")) != NULL) {
					if (gmt_getpen (GMT, &c[2], &Ctrl->N.pen[mode])) {
						gmt_pen_syntax (GMT, 'N', NULL, " ", NULL, 0);
						n_errors++;
					}
				}
				break;
			case 'Q':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.mode = (opt->arg[0] == 'r') ? -1 : +1;
				break;
			case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'T':
				t_arg = opt->arg;
				break;
			case 'W':
				w_arg = opt->arg;
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				if ((c = strstr (opt->arg, "+w")) != NULL) {	/* Use weights instead of counts */
					Ctrl->Z.weights = true;
					c[0] = '\0';	/* Chop of temporarily */
				}
				if (opt->arg[0]) {	/* Gave an argument */
					sval = atoi (opt->arg);
					n_errors += gmt_M_check_condition (GMT, sval < PSHISTOGRAM_COUNTS || sval > PSHISTOGRAM_LOG10_FREQ_PCT, "Option -Z: histogram type must be in 0-5 range\n");
					Ctrl->Z.mode = sval;
				}
				if (c) c[0] = '+';	/* Restore */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	/* Must handle some backwards compatible issues first. The problem is a change in syntax:
	 * Old syntax: -W<width>[+l|h|b] [-L<pen>] [-T<col>]
	 * New syntax: -T<width> [-Ll|h|b] [-W<pen>]
	 * See logic in get_syntax. */
	if (pshistogram_new_syntax (GMT, l_arg, t_arg, w_arg)) {
		/* Process -T<width> [-Lb|h|l] [-W<pen>] */
		Ctrl->T.active = true;
		n_errors += gmt_parse_array (GMT, 'T', t_arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_DIST | GMT_ARRAY_UNIQUE, 0);
		gmt_reset_array_time (GMT, &(Ctrl->T.T));	/* Correct any conflicts between T unit and TIME_UNIT */
		if (l_arg) {	/* Gave -Lb|h|l */
			Ctrl->L.active = true;
			if (l_arg[0] == 'l') Ctrl->L.mode = PSHISTOGRAM_LEFT;
			else if (l_arg[0] == 'h') Ctrl->L.mode = PSHISTOGRAM_RIGHT;
			else if (l_arg[0] == 'b') Ctrl->L.mode = PSHISTOGRAM_BOTH;
		}
		if (w_arg) {	/* Gave -W<pen> */
			Ctrl->W.active = true;
			if (gmt_getpen (GMT, w_arg, &Ctrl->W.pen)) {
				gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
				n_errors++;
			}
		}
	}
	else if (w_arg) {
		/* Process -W<width>[+b|h|l] [-L<pen>] */
		c = strrchr (w_arg, '+');
		/* Worry about the modes, if any.  This handles +l|h|b */
		if (c) {	/* Gave old -W with mode, convert to new syntax.  Error if -T was also given */
			if (c[1] == 'l')      Ctrl->L.mode = PSHISTOGRAM_LEFT;
			else if (c[1] == 'h') Ctrl->L.mode = PSHISTOGRAM_RIGHT;
			else if (c[1] == 'b') Ctrl->L.mode = PSHISTOGRAM_BOTH;
			if (Ctrl->L.mode) {
				Ctrl->L.active = true;
				c[0] = '\0';	/* Chop off modifier */
			}
		}
		if (t_arg) {	/* Any -T is a GMT4-style -Tcol selection.  Deal with that first */
			if (gmt_M_compat_check (GMT, 4)) {
				GMT_Report (API, GMT_MSG_COMPAT, "The -T<col> option is deprecated; use -i<col> instead.\n");
				n_errors += gmt_parse_i_option (GMT, t_arg);
			}
			else
				n_errors += gmt_default_error (GMT, 'T');
			t_arg = NULL;	/* So it is not confused with new -T parsed below */
		}
			/* Now parse bin width via -T */
		n_errors += gmt_parse_array (GMT, 'T', w_arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_DIST, 0);
		Ctrl->T.active = true;
		if (c) c[0] = '+';	/* Restore */
		if (l_arg) {	/* Gave -L<pen> ==> -W<pen> */
			if (gmt_M_compat_check (GMT, 6)) {
				Ctrl->W.active = true;
				//GMT_Report (API, GMT_MSG_COMPAT, "The -L<pen> option is deprecated; use -W<pen> instead.\n");
				if (gmt_getpen (GMT, l_arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
			}
			else
				n_errors += gmt_default_error (GMT, 'T');
		}
	}
	else {
		GMT_Report (API, GMT_MSG_ERROR, "Required argument for bin width not set\n");
		n_errors++;
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->T.T.vartime, "Option -F: Cannot be used with variable time bin widths\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active, "Option -T: Must specify bin width\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->E.active, "Option -S: Cannot be used with -E\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->I.active && !gmt_M_is_linear (GMT), "Option -J: Only linear projection supported.\n");

	/* Now must specify either fill color with -G or outline pen with -W */
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->C.active || Ctrl->I.active || Ctrl->G.active || Ctrl->W.active), "Must specify either fill (-G) or lookup colors (-C), outline pen attributes (-W), or both.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->G.active, "Cannot specify both fill (-G) and lookup colors (-C).\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.w.active && (Ctrl->N.selected[PSHISTOGRAM_L1] || Ctrl->N.selected[PSHISTOGRAM_LMS]), "Option -N: Only -N is supported when -w is selected.\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.w.active && Ctrl->N.selected[PSHISTOGRAM_L2] && Ctrl->Q.active, "Option -N: Cannot use -Q when -w is selected.\n");
	n_errors += gmt_check_binary_io (GMT, 0);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_pshistogram (void *V_API, int mode, void *args) {
	bool automatic = false;
	int error = 0;

	uint64_t n, n_cols;
	size_t n_alloc = GMT_CHUNK;

	char format[GMT_BUFSIZ] = {""};

	double *data = NULL, *weights = NULL, stats[7], area, tmp, x_min, x_max, *in = NULL;

	struct PSHISTOGRAM_INFO F;
	struct PSHISTOGRAM_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (gmt_M_compat_check (GMT, 4)) {	/* Must see if -E was given and temporarily change it */
		struct GMT_OPTION *opt = NULL;
		for (opt = options; opt->next; opt = opt->next) {
			if (opt->option == 'E' && (opt->arg[0] == '\0' || opt->arg[0] == 'l' || opt->arg[0] == 'h'))
				opt->option = '@';	/* Temporary turn -E[l|h] into -Q[l|h] */
		}
	}
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pshistogram main code ----------------------------*/

	if (!Ctrl->I.active) {	/* Need a linear projection either set explicitly (classic) or implicitly (modern only) */
		if (GMT->current.setting.run_mode == GMT_CLASSIC && !GMT->common.J.active) {	/* -J is required, exit at this point */
			GMT_Report (API, GMT_MSG_ERROR, "Must specify Cartesian scales or dimensions of the domain via -Jx or -JX.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT->current.proj.projection != GMT_LINEAR) {	/* Must have given a nonlinear map projection by mistake */
			GMT_Report (API, GMT_MSG_ERROR, "Option -J: Only Cartesian scaling available in this module.\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	gmt_M_memset (&F, 1, struct PSHISTOGRAM_INFO);
	gmt_M_memset (stats, 7, double);
	F.hist_type  = Ctrl->Z.mode;
	F.cumulative = Ctrl->Q.mode;
	F.center_box = Ctrl->F.active;
	F.extremes = Ctrl->L.mode;
	F.weights = Ctrl->Z.weights;
	F.T = &(Ctrl->T.T);
	if (!Ctrl->I.active && (!GMT->common.R.active[RSET] || GMT->common.R.wesn[YLO] == GMT->common.R.wesn[YHI])) automatic = true;
	if (GMT->common.R.active[RSET]) {	/* Gave -R which initially defines the bins also */
		gmt_M_memcpy (F.wesn, GMT->common.R.wesn, 4, double);
		Ctrl->T.T.min = F.wesn[XLO]; Ctrl->T.T.max = F.wesn[XHI];
	}
	if (Ctrl->T.T.set == 3) {	/* Gave a specific -Tmin/max/inc setting */
		F.wesn[XLO] = Ctrl->T.T.min; F.wesn[XHI] = Ctrl->T.T.max;
	}
	n_cols = (F.weights) ? 2 : 1;

	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->C.active && (P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (GMT_DATA_READ_ERROR);
	}

	data = gmt_M_memory (GMT, NULL, n_alloc , double);
	if (F.weights) weights = gmt_M_memory (GMT, NULL, n_alloc , double);

	n = 0;
	x_min = DBL_MAX;	x_max = -DBL_MAX;

	gmt_M_tic (GMT);
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) { 		/* Bail if there are any read errors */
				gmt_M_free (GMT, data);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			gmt_M_free (GMT, data);
			if (F.weights) gmt_M_free (GMT, weights);
			Return (API->error);
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		data[n] = in[GMT_X];
		if (!gmt_M_is_dnan (data[n])) {
			x_min = MIN (x_min, data[n]);
			x_max = MAX (x_max, data[n]);
			if (F.weights)
				weights[n] = in[GMT_Y];
			n++;
		}

		if (n == n_alloc) {
			n_alloc <<= 1;
			data = gmt_M_memory (GMT, data, n_alloc, double);
			if (F.weights) weights = gmt_M_memory (GMT, weights, n_alloc, double);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {
		gmt_M_free (GMT, data);
		if (F.weights) gmt_M_free (GMT, weights);
		Return (API->error);	/* Disables further data input */
	}

	gmt_M_toc (GMT, "Finished reading data");

	if (n == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "Fatal error, read only 0 points.\n");
		gmt_M_free (GMT, data);
		if (F.weights) gmt_M_free (GMT, weights);
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " points read\n", n);

	data = gmt_M_memory (GMT, data, n, double);

	{	/* Must do some work on the array for statistics */
		bool mmm[3];
		mmm[PSHISTOGRAM_L2] = mmm[PSHISTOGRAM_L1] = mmm[PSHISTOGRAM_LMS] = true;	/* Need to know mean, median, mode plus deviations */
		if (F.weights) {	/* Must use a copy since get_loc_scale sorts the array and that does not work if we have weights */
			double *tmp = gmt_M_memory (GMT, NULL, n, double);
			gmt_M_memcpy (tmp, data, n, double);
			weights = gmt_M_memory (GMT, weights, n, double);
			pshistogram_get_loc_scl (GMT, tmp, n, mmm, stats);
			gmt_M_free (GMT, tmp);
		}
		else
			pshistogram_get_loc_scl (GMT, data, n, mmm, stats);
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (format, "Extreme values of the data :\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, data[0], data[n-1]);
		sprintf (format, "Locations: L2, L1, LMS; Scales: L2, L1, LMS\t%s\t%s\t%s\t%s\t%s\t%s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, stats[0], stats[1], stats[2], stats[3], stats[4], stats[5]);
	}

	if (F.wesn[XHI] == F.wesn[XLO]) {	/* Set automatic x range [and tickmarks] when -R -T missing */
		/* Adjust the min/max found for finite bin width */
		double b_min = x_min, b_max = x_max;
		if (Ctrl->F.active) {	/* First and last bin will stick out half a bin width from the data limits */
			b_min -= 0.5 * F.T->inc;
			b_max += 0.5 * F.T->inc;
		}
		else	/* Only b_max needs to change since it may fall in the last bin starting at x_max */
			b_max += F.T->inc;
		if (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval == 0.0) {	/* No tick info set, pick something */
			if (GMT->current.proj.xyz_projection[GMT_X] == GMT_LOG10)
				tmp = 1.0;	/* Do powers of 10 only */
			else {	/* Linear */
				tmp = pow (10.0, floor (d_log10 (GMT, b_max-b_min)));
				if (((b_max-b_min) / tmp) < 3.0) tmp *= 0.5;
			}
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
		if (GMT->current.proj.xyz_projection[GMT_X] == GMT_LOG10) {	/* Round to nearest multiples of 1,2,5 * 10^p only */
			double f = log10 (b_min), p = floor (f), df = f - p;
			if (df > LOG10_5) F.wesn[XLO] = pow (10.0, p + LOG10_5);
			else if (df > LOG10_2) F.wesn[XLO] = pow (10.0, p + LOG10_2);
			else F.wesn[XLO] = pow (10.0, p);
			f = log10 (b_max), p = floor (f), df = f - p;
			if (df > LOG10_5) F.wesn[XHI] = pow (10.0, p + 1.0);
			else if (df > LOG10_2) F.wesn[XHI] = pow (10.0, p + LOG10_5);
			else F.wesn[XHI] = pow (10.0, p + LOG10_2);
		}
		else {	/* Linear */
			F.wesn[XLO] = floor (b_min / tmp) * tmp;
			F.wesn[XHI] = ceil  (b_max / tmp) * tmp;
		}
		if (GMT->current.proj.xyz_projection[GMT_X] == GMT_LOG10 && F.wesn[XLO] == 0.0) F.wesn[XLO] = 1.0;	/* To avoid any log10 of zero issues */
		if (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval == 0.0) {
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_UPPER].interval = tmp;
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].parent = 0;
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].active = true;
			GMT->current.map.frame.draw = true;
		}
		Ctrl->T.T.min = F.T->inc * floor (F.wesn[XLO] / F.T->inc);
		Ctrl->T.T.max = F.T->inc * ceil  (F.wesn[XHI] / F.T->inc);
	}


	/* Set up bin boundaries array */

	if (F.center_box) {	/* Initial specification was for bin centers, adjust limits to get bin boundaries */
		F.T->min -= 0.5 * F.T->inc;
		F.T->max += 0.5 * F.T->inc;
	}
	if (gmt_create_array (GMT, 'T', F.T, NULL, NULL)) {
		gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
		if (F.weights) gmt_M_free (GMT, weights);
		Return (GMT_RUNTIME_ERROR);
	}
	else if (F.T->list && F.T->set > 1 && !GMT->common.R.active[RSET]) {	/* Update min/max */
		F.wesn[XLO] = F.T->min; F.wesn[XHI] = F.T->max;
	}

	if (pshistogram_fill_boxes (GMT, &F, data, weights, n)) {
		GMT_Report (API, GMT_MSG_ERROR, "Fatal error during box fill.\n");
		gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
		if (F.weights) gmt_M_free (GMT, weights);
		Return (GMT_RUNTIME_ERROR);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (format, "min/max values are :\t%s\t%s\t%s\t%s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, x_min, x_max, F.yy0, F.yy1);
	}

	if (Ctrl->I.active) {	/* Only info requested, quit before plotting */
		if (Ctrl->I.mode) {
			uint64_t n_boxes = 0;
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 2}, ibox, row;
			double xx, yy;
			struct GMT_DATASET *D = NULL;
			struct GMT_DATASEGMENT *S = NULL;

			if (Ctrl->I.mode == 1) {
				for (ibox = 0; ibox < F.n_boxes; ibox++) {
					if (Ctrl->I.mode == 1 && gmt_M_is_zero (F.boxh[ibox])) continue;
					n_boxes++;
				}
			}
			else
				n_boxes = F.n_boxes;

			dim[GMT_ROW] = n_boxes;
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for histogram\n");
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
			if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (error);
			}
			S = D->table[0]->segment[0];	/* Only one table with one segment here, with 2 cols and F.n_boxes rows */
			for (ibox = row = 0; ibox < F.n_boxes; ibox++) {
				if (Ctrl->I.mode == 1 && gmt_M_is_zero (F.boxh[ibox])) continue;
				xx = F.T->array[ibox];
				if (F.hist_type == PSHISTOGRAM_LOG_COUNTS)
					yy = d_log1p (GMT, F.boxh[ibox]);
				else if (F.hist_type == PSHISTOGRAM_LOG10_COUNTS)
					yy = d_log101p (GMT, F.boxh[ibox]);
				else if (F.hist_type == PSHISTOGRAM_FREQ_PCT)
					yy = (100.0 * F.boxh[ibox]) / F.sum_w;
				else if (F.hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
					yy = d_log1p (GMT, 100.0 * F.boxh[ibox] / F.sum_w );
				else if (F.hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
					yy = d_log101p (GMT, 100.0 * F.boxh[ibox] / F.sum_w );
				else
					yy = F.boxh[ibox];
				S->data[GMT_X][row] = xx;
				S->data[GMT_Y][row] = yy;
				row++;
			}
			S->n_rows = row;
			if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, GMT_WRITE_NORMAL, NULL, Ctrl->Out.file, D) != GMT_NOERROR) {
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
			if (GMT_Destroy_Data (GMT->parent, &D) != GMT_NOERROR) {
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
		}
		else {	/* Report the min/max values as the data result */
			double out[4];
			unsigned int col_type[4];
			struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);
			gmt_M_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], 4U, unsigned int);	/* Save first 4 current output col types */
			gmt_set_column_type (GMT, GMT_OUT, 0, gmt_M_type (GMT, GMT_IN, GMT_X));
			gmt_set_column_type (GMT, GMT_OUT, 1, gmt_M_type (GMT, GMT_IN, GMT_Y));
			gmt_set_column_type (GMT, GMT_OUT, 2, GMT_IS_FLOAT);
			gmt_set_column_type (GMT, GMT_OUT, 3, GMT_IS_FLOAT);
			if ((error = GMT_Set_Columns (API, GMT_OUT, 4U, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (error);
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);	/* Enables data output and sets access mode */
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
			sprintf (format, "xmin\txmax\tymin\tymax from pshistogram -I -T%g -Z%u", Ctrl->T.T.inc, Ctrl->Z.mode);
			if (Ctrl->F.active) strcat (format, " -F");
			out[0] = x_min;	out[1] = x_max;	out[2] = F.yy0;	out[3] = F.yy1;
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, format);	/* Write this to output if -ho */
			GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
				gmt_M_free (GMT, data);		gmt_M_free (GMT, F.boxh);
				if (F.weights) gmt_M_free (GMT, weights);
				Return (API->error);
			}
			gmt_M_free (GMT, Rec);
			gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, 4U, unsigned int);	/* Restore 4 current output col types */
		}
		gmt_M_free (GMT, data);
		gmt_M_free (GMT, F.boxh);
		if (F.weights) gmt_M_free (GMT, weights);
		Return (GMT_NOERROR);
	}

	if (automatic) {	/* Set up s/n based on 'clever' rounding up of the minmax values */
		GMT->common.R.active[RSET] = true;
		if (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval == 0.0) {
			if (GMT->current.proj.xyz_projection[GMT_Y] == GMT_LOG10)
				tmp = 1.0;	/* Do powers of 10 only for annotations */
			else {	/* Linear */
				tmp = pow (10.0, floor (d_log10 (GMT, F.yy1)));
				if ((F.yy1 / tmp) < 3.0) tmp *= 0.5;
			}
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval;
		if (GMT->current.proj.xyz_projection[GMT_Y] == GMT_LOG10) {	/* Round to nearest multiples of 1,2,5 * 10^p only */
			double f, p, df;
			if (F.yy0 > 0) {
				f = log10 (F.yy0), p = floor (f), df = f - p;
				if (df > LOG10_5) F.wesn[YLO] = pow (10.0, p + LOG10_5);
				else if (df > LOG10_2) F.wesn[YLO] = pow (10.0, p + LOG10_2);
				else F.wesn[YLO] = pow (10.0, p);
			}
			else	/* Safety valve for log 0 */
				F.wesn[YLO] = 1.0;
			f = log10 (F.yy1), p = floor (f), df = f - p;
			if (df > LOG10_5) F.wesn[YHI] = pow (10.0, p + 1.0);
			else if (df > LOG10_2) F.wesn[YHI] = pow (10.0, p + LOG10_5);
			else F.wesn[YHI] = pow (10.0, p + LOG10_2);
		}
		else {
			F.wesn[YLO] = 0.0;
			F.wesn[YHI] = ceil (F.yy1 / tmp) * tmp;
		}
		if (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval == 0.0) {	/* Tickmarks not set */
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_UPPER].interval = tmp;
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].parent = 1;
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].active = true;
			GMT->current.map.frame.draw = true;
		}
		if (GMT->current.proj.pars[0] == 0.0 && GMT->current.proj.pars[1] == 0.0) {
			GMT_Report (API, GMT_MSG_ERROR, "Need to provide both x- and y-scale.\n");
			gmt_M_free (GMT, data);
			if (F.weights) gmt_M_free (GMT, weights);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (automatic && gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (format, "Use w/e/s/n = %s/%s/%s/%s and x-tick/y-tick = %s/%s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, F.wesn[XLO], F.wesn[XHI], F.wesn[YLO], F.wesn[YHI],
		            GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval,
		            GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval);
	}

	if (Ctrl->A.active) {	/* Must flip x and y axis */
		struct GMT_PLOT_AXIS shelf;
		double wesn[4];
		unsigned int k;
		gmt_M_memcpy (&shelf, &GMT->current.map.frame.axis[GMT_X], 1, struct GMT_PLOT_AXIS);
		gmt_M_memcpy (&GMT->current.map.frame.axis[GMT_X], &GMT->current.map.frame.axis[GMT_Y], 1, struct GMT_PLOT_AXIS);
		gmt_M_memcpy (&GMT->current.map.frame.axis[GMT_Y], &shelf, 1, struct GMT_PLOT_AXIS);
		gmt_M_int_swap (GMT->current.proj.xyz_projection[GMT_X], GMT->current.proj.xyz_projection[GMT_Y]);
		/* But must update the ids of parents and children since x and y ids have been swapped too */
		GMT->current.map.frame.axis[GMT_X].id = GMT_X;
		GMT->current.map.frame.axis[GMT_Y].id = GMT_Y;
		for (k = 0; k < 6; k++) {
			GMT->current.map.frame.axis[GMT_Y].item[k].parent = GMT_Y;
			GMT->current.map.frame.axis[GMT_X].item[k].parent = GMT_X;
		}
		wesn[XLO] = F.wesn[YLO];	wesn[XHI] = F.wesn[YHI];
		wesn[YLO] = F.wesn[XLO];	wesn[YHI] = F.wesn[XHI];
		if (GMT->current.io.cycle_col == GMT_X) GMT->current.io.cycle_col = GMT_Y;
		gmt_M_memcpy (GMT->common.R.wesn, wesn, 4, double);
		if (gmt_map_setup (GMT, wesn)) Return (GMT_PROJECTION_ERROR);
	}
	else {
		gmt_M_memcpy (GMT->common.R.wesn, F.wesn, 4, double);
		if (gmt_map_setup (GMT, F.wesn)) {
			gmt_M_free (GMT, data);
			if (F.weights) gmt_M_free (GMT, weights);
			Return (GMT_PROJECTION_ERROR);
		}
	}
	if (automatic) {	/* Must add the result to the history */
		char Rtxt[GMT_LEN128] = {""};
		int id = gmt_get_option_id (0, "R");	/* The -RP history index */
		if (GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);	/* Remove whatever this was */
		sprintf (Rtxt, "%.16g/%.16g/%.16g/%.16g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->init.history[id] = strdup (Rtxt);	/* Update with the dimension of the whole subplot frame */
	}
	
	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_AFTER);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);

	if (Ctrl->D.just == 0) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	area = pshistogram_plot_boxes (GMT, PSL, Ctrl, P, &F, &Ctrl->D);
	GMT_Report (API, GMT_MSG_INFORMATION, "Area under histogram is %g\n", area);

	if (GMT->common.l.active) {	/* Add auto-legend entry */
		/* Always plot a 3:2 width:height rectangle, possibly via user sizing, using fill and pen */
		struct GMT_SYMBOL S;
		gmt_M_memset (&S, 1U, struct GMT_SYMBOL);
		S.symbol = PSL_RECT;
		if (GMT->common.l.item.size == 0.0) {	/* Select default width given by annotation height scaled by actual fractional height times 1.5 */
			S.size_y = GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];
			S.size_x = 1.5 * S.size_y;	/* Width to height ratio is 3:2 */
		}
		else {	/* Use given size as rectangle width */
			S.size_x = GMT->common.l.item.size;
			if (GMT->common.l.item.size2 > 0.0)	/* Gave both width and height */
				S.size_y = GMT->common.l.item.size2;
			else
				S.size_y = S.size_x / 1.5;	/* Width to height ratio is 3:2 */
		}
		gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item), NULL);
	}
	
	if (Ctrl->N.active) {	/* Want to draw one or more normal distributions; we use 101 points to do so */
		unsigned int type, k, NP = 101U;
		double f, z, xtmp, ytmp, inc;
		double *xp = gmt_M_memory (GMT, NULL, NP, double);
		double *yp = gmt_M_memory (GMT, NULL, NP, double);
		inc = (F.wesn[XHI] - F.wesn[XLO]) / (NP - 1);
		for (type = 0; type < 3; type++) {
			if (!Ctrl->N.selected[type]) continue;
			/* Draw this estimation of a normal distribution */
			gmt_setpen (GMT, &Ctrl->N.pen[type]);
			f = (Ctrl->Q.active) ? 0.5 : 1.0 / (stats[type+3] * sqrt (M_PI * 2.0));
			if (GMT->common.w.active) f = stats[6] * D2R;	/* Scale area by bin-width in radians */
			f *= area;
			for (k = 0; k < NP; k++) {
				xp[k] = F.wesn[XLO] + inc * k;
				z = (xp[k] - stats[type]) / stats[type+3];	/* z-score for chosen statistic */
				if (GMT->common.w.active)	/* stats[6] converts wrapped z to 0-360 degrees, stats[0] is mu and stats[3] is kappa */
					yp[k] = f * gmt_vonmises_pdf (GMT, stats[6] * xp[k], stats[0], stats[3]);
				else if (Ctrl->Q.active) {	/* Want a cumulative curve */
					yp[k] = f * (1.0 + erf (z / M_SQRT2));
					if (Ctrl->Q.mode == -1) yp[k] = f - yp[k];
				}
				else
					yp[k] = f * exp (-0.5 * z * z);
				switch (F.hist_type) {	/* Must adjust yp[k] accordingly */
					case PSHISTOGRAM_LOG_COUNTS:		yp[k] = d_log1p (GMT, yp[k]);	break;
					case PSHISTOGRAM_LOG10_COUNTS:		yp[k] = d_log101p (GMT, yp[k]);	break;
					case PSHISTOGRAM_FREQ_PCT:		yp[k] = (100.0 * yp[k]) / F.sum_w;	break;
					case PSHISTOGRAM_LOG_FREQ_PCT:		yp[k] = d_log1p (GMT, 100.0 * yp[k] / F.sum_w);	break;
					case PSHISTOGRAM_LOG10_FREQ_PCT:	yp[k] = d_log101p (GMT, 100.0 * yp[k] / F.sum_w);	break;
				}


				if (Ctrl->A.active)
					gmt_geo_to_xy (GMT, yp[k], xp[k], &xtmp, &ytmp);
				else
					gmt_geo_to_xy (GMT, xp[k], yp[k], &xtmp, &ytmp);
				xp[k] = xtmp;	yp[k] = ytmp;
			}
			PSL_plotline (PSL, xp, yp, NP, PSL_MOVE|PSL_STROKE);
		}
		gmt_M_free (GMT, xp);
		gmt_M_free (GMT, yp);
	}

	if (Ctrl->D.just == 0) gmt_map_clip_off (GMT);

	gmt_map_basemap (GMT);
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	gmt_M_free (GMT, data);
	if (F.weights) gmt_M_free (GMT, weights);
	gmt_M_free (GMT, F.boxh);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_histogram (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: histogram\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pshistogram (V_API, mode, args);
}
