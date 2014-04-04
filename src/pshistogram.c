/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: pshistogram.c -- a program for plotting histograms
 *
 * Author:	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"pshistogram"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Calculate and plot histograms"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYbcdfhipsxy" GMT_OPT("E")

EXTERN_MSC int gmt_parse_i_option (struct GMT_CTRL *GMT, char *arg);

struct PSHISTOGRAM_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D[+r][+f<font>][+o<off>][+b] */
		bool active;
		unsigned int mode;	/* 0 for horizontal, 1 for vertical */
		unsigned int just;	/* 0 for top of bar, 1 for below */
		struct GMT_FONT font;
		double offset;
	} D;
	struct F {	/* -F */
		bool active;
	} F;
	struct G {	/* -Gfill */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I[o] */
		bool active;
		unsigned int mode;
	} I;
	struct L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct N {	/* -N[<kind>]+<pen>, <kind = 0,1,2 */
		bool active;
		bool selected[3];
		struct GMT_PEN pen[3];
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
	struct W {	/* -W<width> */
		bool active;
		double inc;
	} W;
	struct Z {	/* -Z<type> */
		bool active;
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

struct PSHISTOGRAM_INFO {	/* Control structure for pshistogram */
	double yy0, yy1;
	double box_width;
	double wesn[4];
	uint64_t n_boxes;
	uint64_t n_counted;
	uint64_t *boxh;
	bool center_box, cumulative;
	unsigned int hist_type;
};

void *New_pshistogram_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	unsigned int k;
	struct PSHISTOGRAM_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct PSHISTOGRAM_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.offset = 6.0 / 72.0;	/* 6 points */
	C->D.font = GMT->current.setting.font_annot[0];		/* Default font */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Do not fill is default */
	C->L.pen = GMT->current.setting.map_default_pen;
	for (k = 0; k < 3; k++) C->N.pen[k] = GMT->current.setting.map_default_pen;
		
	return (C);
}

void Free_pshistogram_Ctrl (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	if (C->C.file) free (C->C.file);	
	GMT_free (GMT, C);	
}

int fill_boxes (struct GMT_CTRL *GMT, struct PSHISTOGRAM_INFO *F, double *data, uint64_t n) {

	double add_half = 0.0;
	uint64_t b0, b1, ibox, count_sum;
	uint64_t i;
	int64_t sbox;

	F->n_boxes = lrint (ceil(((F->wesn[XHI] - F->wesn[XLO]) / F->box_width) + 0.5));

	if (F->center_box) {
		F->n_boxes++;
		add_half = 0.5;
	}

	if (F->n_boxes == 0) return (-1);

	F->boxh = GMT_memory (GMT, NULL, F->n_boxes, uint64_t);

	F->n_counted = 0;

	/* First fill boxes with counts  */

	for (i = 0; i < n; i++) {
		sbox = lrint (floor (((data[i] - F->wesn[XLO]) / F->box_width) + add_half));
		if (sbox < 0) continue;
		ibox = (uint64_t)sbox;
		if (ibox >= F->n_boxes) continue;
		F->boxh[ibox]++;
		F->n_counted++;
	}

	if (F->cumulative) {
		for (ibox = count_sum = b0 = 0; ibox < F->n_boxes; ibox++) {
			count_sum += F->boxh[ibox];
			F->boxh[ibox] = count_sum;
		}
		b1 = count_sum;
	}
	else {
		b0 = F->n_counted;
		for (ibox = b1 = 0; ibox < F->n_boxes; ibox++) {
			if (b0 > F->boxh[ibox]) b0 = F->boxh[ibox];
			if (b1 < F->boxh[ibox]) b1 = F->boxh[ibox];
		}
	}

	/* Now find out what the min max y will be  */

	if (b0) {
		if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
			F->yy0 = d_log1p (GMT, (double)b0);
		else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
			F->yy0 = d_log101p (GMT, (double)b0);
		else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
			F->yy0 = (100.0 * b0) / F->n_counted;
		else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
			F->yy0 = d_log1p (GMT, 100.0 * b0 / F->n_counted );
		else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
			F->yy0 = d_log101p (GMT, 100.0 * b0 / F->n_counted );
		else
			F->yy0 = (double)b0;
	}
	else
		F->yy0 = 0.0;
	if (b1) {
		if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
			F->yy1 = d_log1p (GMT, (double)b1);
		else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
			F->yy1 = d_log101p (GMT, (double)b1);
		else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
			F->yy1 = (100.0 * b1) / F->n_counted;
		else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
			F->yy1 = d_log1p (GMT, 100.0 * b1 / F->n_counted );
		else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
			F->yy1 = d_log101p (GMT, 100.0 * b1 / F->n_counted );
		else
			F->yy1 = (double)b1;
	}
	else
		F->yy1 = 0.0;

	return (0);
}

double plot_boxes (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, struct PSHISTOGRAM_INFO *F, int stairs, int flip_to_y, int draw_outline, struct GMT_PEN *pen, struct GMT_FILL *fill, int cpt, struct D *D)
{
	int i, index, fmode = 0, label_justify = (flip_to_y) ? PSL_ML : PSL_BC;
	uint64_t ibox;
	char label[GMT_LEN64] = {""};
	bool first = true;
	double area = 0.0, rgb[4], x[4], y[4], xx, yy, xval, label_angle = 0.0, *px = NULL, *py = NULL;
	double plot_x = 0.0, plot_y = 0.0;
	struct GMT_FILL *f = NULL;

	if (draw_outline) GMT_setpen (GMT, pen);

	if (flip_to_y) {
		px = y;
		py = x;
	}
	else {
		px = x;
		py = y;
	}

	if (D->active) {	/* Place label, so set font */
		fmode = GMT_setfont (GMT, &D->font);
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
	}
	for (ibox = 0; ibox < F->n_boxes; ibox++) {
		if (stairs || F->boxh[ibox]) {
			x[0] = F->wesn[XLO] + ibox * F->box_width;
			if (F->center_box) x[0] -= (0.5 * F->box_width);
			x[1] = x[0] + F->box_width;
			if (x[0] < F->wesn[XLO]) x[0] = F->wesn[XLO];
			if (x[1] > F->wesn[XHI]) x[1] = F->wesn[XHI];
			xval = 0.5 * (x[0] + x[1]);	/* Used for cpt lookup */
			x[2] = x[1];
			x[3] = x[0];
			y[0] = y[1] = F->wesn[YLO];
			if (F->hist_type == PSHISTOGRAM_LOG_COUNTS)
				y[2] = d_log1p (GMT, (double)F->boxh[ibox]);
			else if (F->hist_type == PSHISTOGRAM_LOG10_COUNTS)
				y[2] = d_log101p (GMT, (double)F->boxh[ibox]);
			else if (F->hist_type == PSHISTOGRAM_FREQ_PCT)
				y[2] = (100.0 * F->boxh[ibox]) / F->n_counted;
			else if (F->hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
				y[2] = d_log1p (GMT, 100.0 * F->boxh[ibox] / F->n_counted );
			else if (F->hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
				y[2] = d_log101p (GMT, 100.0 * F->boxh[ibox] / F->n_counted );
			else
				y[2] = (double)F->boxh[ibox];
			y[3] = y[2];
			if (F->cumulative)
				area = (double)F->boxh[ibox];	/* Just pick up the final bin as it has the entire sum */
			else
				area += F->boxh[ibox];

			for (i = 0; i < 4; i++) {
				GMT_geo_to_xy (GMT, px[i], py[i], &xx, &yy);
				px[i] = xx;	py[i] = yy;
			}

			if (stairs) {
				if (first) {
					first = false;
					PSL_plotpoint (PSL, px[0], py[0], PSL_MOVE);
				}
				PSL_plotpoint (PSL, px[3], py[3], PSL_DRAW);
				PSL_plotpoint (PSL, px[2], py[2], PSL_DRAW);
			}
			else if (cpt) {
				index = GMT_get_rgb_from_z (GMT, P, xval, rgb);
				if ((index >= 0 && (f = P->range[index].fill)) || (index < 0 && (f = P->patch[index+3].fill)))	/* Pattern */
					GMT_setfill (GMT, f, draw_outline);
				else
					PSL_setfill (PSL, rgb, draw_outline);
				PSL_plotpolygon (PSL, px, py, 4);
			}
			else {
				GMT_setfill (GMT, fill, draw_outline);
				PSL_plotpolygon (PSL, px, py, 4);
			}
			if (D->active) {	/* Place label */
				if (flip_to_y) {
					plot_y = 0.5 * (py[0] + py[1]);
					plot_x = (D->just) ? px[0] - D->offset : px[2] + D->offset;
				}
				else {
					plot_x = 0.5 * (px[0] + px[1]);
					plot_y = (D->just) ? py[0] - D->offset : py[2] + D->offset;
				}
				sprintf (label, "%" PRIu64, F->boxh[ibox]);
				PSL_plottext (PSL, plot_x, plot_y, D->font.size, label, label_angle, label_justify, fmode);
			}
		}
	}

	if (stairs) PSL_plotpoint (PSL, px[1], py[1], PSL_DRAW + PSL_STROKE);
	if (F->cumulative) return (area);
	return (area * F->box_width);
}

int get_loc_scl (struct GMT_CTRL *GMT, double *data, uint64_t n, double *stats)
{
	/* Returns stats[] = L2, L1, LMS location, L2, L1, LMS scale  */

	uint64_t i, j;
	unsigned int n_multiples = 0;
	double dx;

	if (n < 3) return (-1);

	GMT_sort_array (GMT, data, n, GMT_DOUBLE);

	/* Get median */
	j = n/2;
	stats[1] = (n%2) ? data[j] : (0.5 * (data[j] + data[j-1]));

	/* Get mode */

	GMT_mode (GMT, data, n, j, 0, 0, &n_multiples, &stats[2]);
	if (n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: %d multiple modes found\n", n_multiples);

	/* Get MAD for L1 */

	GMT_getmad (GMT, data, n, stats[1], &stats[4]);

	/* Get LMSscale for mode */

	GMT_getmad (GMT, data, n, stats[2], &stats[5]);

	/* Calculate mean and stdev in two passes to minimize risk of overflow */

	stats[0] = stats[3] = 0.0;
	for (i = 0; i < n; i++) stats[0] += data[i];	/* Sum up the data */
	stats[0] /= n;	/* This is the mean value */
	for (i = 0; i < n; i++) {
		dx = data[i] - stats[0];
		stats[3] += (dx * dx);
	}
	stats[3] = sqrt (stats[3] / (n - 1));

	return (0);
}
	
int GMT_pshistogram_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pshistogram [<table>] %s -W<width> [-A] [%s] [-C<cpt>] [-D[+b][+f<font>][+o<off>][+r]]\n", GMT_Jx_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F] [-G<fill>] [-I[o|O]] [%s] [-K] [-L<pen>] [-N[<mode>][+p<pen>]] [-O] [-P] [-Q]\n", GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S] [%s]\n\t[%s] [%s] [%s] [-Z[0-5]]\n", GMT_Rx_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_bi_OPT, GMT_di_OPT, GMT_c_OPT, GMT_f_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_p_OPT, GMT_s_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "JXZ");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set the bin width.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Plot horizontal bars [Default is vertical].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use cpt-file to assign fill to bars based on the mid x-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Place histogram count labels on top of each bar; optionally append modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +b places the labels beneath the bars [above]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<font> sets the label font [FONT_ANNOT_PRIMARY]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o sets the offset <off> between bar and label [6p]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +r rotates the label to be vertical [horizontal]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Center the bins.\n");
	GMT_fill_syntax (API->GMT, 'G', "Select color/pattern for columns.");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inquire about min/max x and y.  No plotting is done.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append o to output the resulting x, y data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append O to output all resulting x, y data even with y=0.\n");
	GMT_Option (API, "K");
	GMT_pen_syntax (API->GMT, 'L', "Specify pen to draw histogram.");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Draw the equivalent normal distribution; append desired pen [0.25p,black].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <mode> selects which central location and scale to use:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   0 = mean and standard deviation [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1 = median and L1 scale\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2 = LMS mode and scale\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The -N option may be repeated to draw several of these curves.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Plot a cumulative histogram.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If neither -R nor -I are set, w/e/s/n will be based on input data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Draw a stairs-step diagram [Default is bar histogram].\n");
	GMT_Option (API, "U,V,X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z To choose type of vertical axis.  Select from\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   0 - Counts [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1 - Frequency percent.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2 - Log (1+counts).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3 - Log (1+frequency percent).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   4 - Log10 (1+counts).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   5 - Log10 (1+frequency percent).\n");
	GMT_Option (API, "bi2,c,di,f,h,i,p,s,t,.");

	return (EXIT_FAILURE);
}

int GMT_pshistogram_parse (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pshistogram and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, mode = 0, pos = 0;
	int sval;
	char *c = NULL, p[GMT_BUFSIZ] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0 && GMT_check_filearg (GMT, '>', opt->arg, GMT_OUT))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				break;
			case 'C':
				if (Ctrl->C.file) free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				while (GMT_getmodopt (GMT, opt->arg, "bfor", &pos, p)) {	/* Looking for +b, +f, +o, +r */
					switch (p[0]) {
						case 'b':	/* beneath */
							Ctrl->D.just = 1;
							break;
						case 'o':	/* offset */
							Ctrl->D.offset = GMT_to_inch (GMT, &p[1]);
							break;
						case 'f':	/* offset */
							n_errors += GMT_getfont (GMT, &p[1], &(Ctrl->D.font));
							break;
						case 'r':	/* rotate */
							Ctrl->D.mode = 1;
							break;
						default:
							n_errors++;
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D: modifier +%s unrecognized.\n", p);
							break;
					}
				}
				break;		
			case 'F':
				Ctrl->F.active = true;
				break;
			case 'G':
				Ctrl->G.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0] == 'o') Ctrl->I.mode = 1;
				if (opt->arg[0] == 'O') Ctrl->I.mode = 2;
				break;
			case 'L':		/* Set line attributes */
				Ctrl->L.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'N':		/* Draw normal distribution */
				Ctrl->N.active = true;
				switch (opt->arg[0]) {	/* See which distribution to draw */
					case '0': case '+': case '\0': mode = PSHISTOGRAM_L2; break;
					case '1': mode = PSHISTOGRAM_L1;	break;
					case '2': mode = PSHISTOGRAM_LMS;	break;
					default:
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N: mode %c unrecognized.\n", opt->arg[0]);
					n_errors++;
				}
				Ctrl->N.selected[mode] = true;
				if ((c = strstr (opt->arg, "+p"))) {
					if (GMT_getpen (GMT, &c[2], &Ctrl->N.pen[mode])) {
						GMT_pen_syntax (GMT, 'L', " ");
						n_errors++;
					}
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: The -T option is deprecated; use -i instead.\n");
					n_errors += gmt_parse_i_option (GMT, opt->arg);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'W':
				Ctrl->W.active = true;
				Ctrl->W.inc = atof (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				sval = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, sval < PSHISTOGRAM_COUNTS || sval > PSHISTOGRAM_LOG10_FREQ_PCT, "Syntax error -Z option: histogram type must be in 0-5 range\n");
				Ctrl->Z.mode = sval;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->I.active && !GMT_IS_LINEAR (GMT), "Syntax error -J option: Only linear projection supported.\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->W.active, "Syntax error -W option: Must specify bin width\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.inc <= 0.0, "Syntax error -W option: bin width must be nonzero\n");

	/* Now must specify either fill color with -G or outline pen with -L */
	n_errors += GMT_check_condition (GMT, !(Ctrl->C.active || Ctrl->I.active || Ctrl->G.active || Ctrl->L.active), "Must specify either fill (-G) or lookup colors (-C), outline pen attributes (-L), or both.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->G.active, "Cannot specify both fill (-G) and lookup colors (-C).\n");
	n_errors += GMT_check_binary_io (GMT, 0);
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pshistogram_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_pshistogram (void *V_API, int mode, void *args)
{
	bool automatic = false;
	int error = 0;
	
	uint64_t n;
	size_t n_alloc = GMT_CHUNK;

	char format[GMT_BUFSIZ];
	
	double *data = NULL, stats[6], area, tmp, x_min, x_max, *in = NULL;
	
	struct PSHISTOGRAM_INFO F;
	struct PSHISTOGRAM_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pshistogram_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pshistogram_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pshistogram_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pshistogram_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pshistogram_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the pshistogram main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	GMT_memset (&F, 1, struct PSHISTOGRAM_INFO);
	F.hist_type  = Ctrl->Z.mode;
	F.box_width  = Ctrl->W.inc;
	F.cumulative = Ctrl->Q.active;
	F.center_box = Ctrl->F.active;
	if (!Ctrl->I.active && !GMT->common.R.active) automatic = true;
	if (GMT->common.R.active) GMT_memcpy (F.wesn, GMT->common.R.wesn, 4, double);

	if ((error = GMT_set_cols (GMT, GMT_IN, 1)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->C.active && (P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (GMT_DATA_READ_ERROR);
	}

	data = GMT_memory (GMT, NULL, n_alloc , double);

	n = 0;
	x_min = DBL_MAX;	x_max = -DBL_MAX;

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
		
		data[n] = in[GMT_X];
		if (!GMT_is_dnan (data[n])) {
			x_min = MIN (x_min, data[n]);
			x_max = MAX (x_max, data[n]);
			n++;
		}

		if (n == n_alloc) {
			n_alloc <<= 1;
			data = GMT_memory (GMT, data,  n_alloc, double);
		}
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {
		Return (API->error);	/* Disables further data input */
	}

	if (n == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Fatal error, read only 0 points.\n");
		Return (EXIT_FAILURE);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points read\n", n);

	data = GMT_memory (GMT, data, n, double);

	get_loc_scl (GMT, data, n, stats);

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "Extreme values of the data :\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, data[0], data[n-1]);
		sprintf (format, "Locations: L2, L1, LMS; Scales: L2, L1, LMS\t%s\t%s\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, stats[0], stats[1], stats[2], stats[3], stats[4], stats[5]);
	}

	if (F.wesn[XHI] == F.wesn[XLO]) {	/* Set automatic x range [ and tickmarks] */
		if (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval == 0.0) {
			tmp = pow (10.0, floor (d_log10 (GMT, x_max-x_min)));
			if (((x_max-x_min) / tmp) < 3.0) tmp *= 0.5;
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
		F.wesn[XLO] = floor (x_min / tmp) * tmp;
		F.wesn[XHI] = ceil  (x_max / tmp) * tmp;
		if (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval == 0.0) {
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_UPPER].interval = tmp;
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].parent = 0;
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].active = true;
			GMT->current.map.frame.draw = true;
		}
	}

	if (fill_boxes (GMT, &F, data, n)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Fatal error during box fill.\n");
		Return (EXIT_FAILURE);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "min/max values are :\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, x_min, x_max, F.yy0, F.yy1);
	}

	if (Ctrl->I.active) {	/* Only info requested, quit before plotting */
		if (Ctrl->I.mode) {
			uint64_t n_boxes = 0;
			uint64_t dim[4] = {1, 1, 0, 2}, ibox, row;
			double xx, yy;
			struct GMT_DATASET *D = NULL;
			struct GMT_DATASEGMENT *S = NULL;
			
			if (Ctrl->I.mode == 1) {
				for (ibox = 0; ibox < F.n_boxes; ibox++) {
					if (Ctrl->I.mode == 1 && F.boxh[ibox] == 0) continue;
					n_boxes++;
				}
			}
			else
				n_boxes = F.n_boxes;
			
			dim[GMT_ROW] = n_boxes;
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, Ctrl->Out.file)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for histogram\n");
				Return (API->error);
			}
			if ((error = GMT_set_cols (GMT, GMT_OUT, 2)) != GMT_OK) {
				Return (error);
			}
			S = D->table[0]->segment[0];	/* Only one table with one segment here, with 2 cols and F.n_boxes rows */
			for (ibox = row = 0; ibox < F.n_boxes; ibox++) {
				if (Ctrl->I.mode == 1 && F.boxh[ibox] == 0) continue;
				xx = F.wesn[XLO] + ibox * F.box_width;
				if (F.center_box) xx -= (0.5 * F.box_width);
				if (F.hist_type == PSHISTOGRAM_LOG_COUNTS)
					yy = d_log1p (GMT, (double)F.boxh[ibox]);
				else if (F.hist_type == PSHISTOGRAM_LOG10_COUNTS)
					yy = d_log101p (GMT, (double)F.boxh[ibox]);
				else if (F.hist_type == PSHISTOGRAM_FREQ_PCT)
					yy = (100.0 * F.boxh[ibox]) / F.n_counted;
				else if (F.hist_type == PSHISTOGRAM_LOG_FREQ_PCT)
					yy = d_log1p (GMT, 100.0 * F.boxh[ibox] / F.n_counted );
				else if (F.hist_type == PSHISTOGRAM_LOG10_FREQ_PCT)
					yy = d_log101p (GMT, 100.0 * F.boxh[ibox] / F.n_counted );
				else
					yy = (double)F.boxh[ibox];
				S->coord[GMT_X][row] = xx;
				S->coord[GMT_Y][row] = yy;
				row++;
			}
			if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, D->io_mode, NULL, Ctrl->Out.file, D) != GMT_OK) {
				Return (API->error);
			}
			if (GMT_Destroy_Data (GMT->parent, &D) != GMT_OK) {
				Return (API->error);
			}
		}
		else {	/* Report the min/max values as the data result */
			double out[4];
			unsigned int col_type[4];
			GMT_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], 4U, unsigned int);	/* Save first 4 current output col types */
			GMT->current.io.col_type[GMT_OUT][0] = GMT->current.io.col_type[GMT_OUT][1] = GMT->current.io.col_type[GMT_IN][0];
			GMT->current.io.col_type[GMT_OUT][2] = GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_FLOAT;
			if ((error = GMT_set_cols (GMT, GMT_OUT, 4U)) != GMT_OK) {
				Return (error);
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {
				Return (API->error);	/* Enables data output and sets access mode */
			}
			sprintf (format, "xmin\txmax\tymin\tymax from pshistogram -I -W%g -Z%u", Ctrl->W.inc, Ctrl->Z.mode);
			if (Ctrl->F.active) strcat (format, " -F");
			out[0] = x_min;	out[1] = x_max;	out[2] = F.yy0;	out[3] = F.yy1;
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, format);	/* Write this to output if -ho */
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
				Return (API->error);
			}
			GMT_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, 4U, unsigned int);	/* Restore 4 current output col types */
		}
		GMT_free (GMT, data);
		GMT_free (GMT, F.boxh);
		Return (EXIT_SUCCESS);
	}

	if (automatic) {	/* Set up s/n based on 'clever' rounding up of the minmax values */
		GMT->common.R.active = true;
		F.wesn[YLO] = 0.0;
		if (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval == 0.0) {
			tmp = pow (10.0, floor (d_log10 (GMT, F.yy1)));
			if ((F.yy1 / tmp) < 3.0) tmp *= 0.5;
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval;
		F.wesn[YHI] = ceil (F.yy1 / tmp) * tmp;
		if (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval == 0.0) {	/* Tickmarks not set */
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_UPPER].interval = tmp;
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].parent = 1;
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].active = true;
			GMT->current.map.frame.draw = true;
		}
		if (GMT->current.proj.pars[0] == 0.0 && GMT->current.proj.pars[1] == 0.0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Need to provide both x- and y-scale.\n");
			Return (EXIT_FAILURE);
		}
	}

	if (automatic && GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "Use w/e/s/n = %s/%s/%s/%s and x-tick/y-tick = %s/%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, F.wesn[XLO], F.wesn[XHI], F.wesn[YLO], F.wesn[YHI], GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval);
	}

	if (Ctrl->A.active) {
		char buffer[GMT_LEN256] = {""};
		double wesn[4];
		double_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval);
		double_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_LOWER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_LOWER].interval);
		double_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_UPPER].interval,  GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_UPPER].interval);
		double_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_LOWER].interval,  GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_LOWER].interval);
		strncpy (buffer, GMT->current.map.frame.axis[GMT_X].label, GMT_LEN256);
		strncpy (GMT->current.map.frame.axis[GMT_X].label, GMT->current.map.frame.axis[GMT_Y].label, GMT_LEN256);
		strncpy (GMT->current.map.frame.axis[GMT_Y].label, buffer, GMT_LEN256);
		wesn[XLO] = F.wesn[YLO];	wesn[XHI] = F.wesn[YHI];
		wesn[YLO] = F.wesn[XLO];	wesn[YHI] = F.wesn[XHI];
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}
	else
		GMT_err_fail (GMT, GMT_map_setup (GMT, F.wesn), "");

	PSL = GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	if (Ctrl->D.just == 0) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
	area = plot_boxes (GMT, PSL, P, &F, Ctrl->S.active, Ctrl->A.active, Ctrl->L.active, &Ctrl->L.pen, &Ctrl->G.fill, Ctrl->C.active, &Ctrl->D);
	GMT_Report (API, GMT_MSG_VERBOSE, "Area under histogram is %g\n", area);
	
	if (Ctrl->N.active) {	/* Want to draw one or more normal distributions; we use 101 points to do so */
		unsigned int type, k, NP = 101U;
		double f, z, xtmp, ytmp, inc;
		double *xp = GMT_memory (GMT, NULL, NP, double);
		double *yp = GMT_memory (GMT, NULL, NP, double);
		inc = (F.wesn[XHI] - F.wesn[XLO]) / (NP - 1);
		for (type = 0; type < 3; type++) {
			if (!Ctrl->N.selected[type]) continue;
			/* Draw this estimation of a normal distribution */
			GMT_setpen (GMT, &Ctrl->N.pen[type]);
			f = (Ctrl->Q.active) ? 0.5 : 1.0 / (stats[type+3] * sqrt (M_PI * 2.0));
			f *= area;
			for (k = 0; k < NP; k++) {
				xp[k] = F.wesn[XLO] + inc * k;
				z = (xp[k] - stats[type]) / stats[type+3];	/* z-score for chosen statistic */
				if (Ctrl->Q.active)	/* Want cumulative curve */
					yp[k] = f * (1.0 + erf (z / M_SQRT2));
				else
					yp[k] = f * exp (-0.5 * z * z);
				switch (F.hist_type) {	/* Must adjust yp[k] accordingly */
					case PSHISTOGRAM_LOG_COUNTS:		yp[k] = d_log1p (GMT, yp[k]);	break;
					case PSHISTOGRAM_LOG10_COUNTS:		yp[k] = d_log101p (GMT, yp[k]);	break;
					case PSHISTOGRAM_FREQ_PCT:		yp[k] = (100.0 * yp[k]) / F.n_counted;	break;
					case PSHISTOGRAM_LOG_FREQ_PCT:		yp[k] = d_log1p (GMT, 100.0 * yp[k] / F.n_counted);	break;
					case PSHISTOGRAM_LOG10_FREQ_PCT:	yp[k] = d_log101p (GMT, 100.0 * yp[k] / F.n_counted);	break;
				}
				
				GMT_geo_to_xy (GMT, xp[k], yp[k], &xtmp, &ytmp);
				xp[k] = xtmp;	yp[k] = ytmp;
			}
			PSL_plotline (PSL, xp, yp, NP, PSL_MOVE + PSL_STROKE);
		}
		GMT_free (GMT, xp);
		GMT_free (GMT, yp);
	}
	
	if (Ctrl->D.just == 0) GMT_map_clip_off (GMT);

	GMT_map_basemap (GMT);
	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	GMT_free (GMT, data);
	GMT_free (GMT, F.boxh);

	Return (GMT_OK);
}
