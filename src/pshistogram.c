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
 * Brief synopsis: pshistogram.c -- a program for plotting histograms
 *
 * Author:	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#include "pslib.h"
#include "gmt.h"

#ifdef GMT_COMPAT
EXTERN_MSC GMT_LONG gmt_parse_i_option (struct GMT_CTRL *C, char *arg);
#endif

struct PSHISTOGRAM_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct G {	/* -Gfill */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I[o] */
		GMT_LONG active;
		GMT_LONG mode;
	} I;
	struct L {	/* -L<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} L;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct W {	/* -W<width> */
		GMT_LONG active;
		double inc;
	} W;
	struct Z {	/* -Z<type> */
		GMT_LONG active;
		GMT_LONG mode;
	} Z;
};

#define	PSHISTOGRAM_COUNTS		0
#define PSHISTOGRAM_FREQ_PCT		1
#define PSHISTOGRAM_LOG_COUNTS		2
#define PSHISTOGRAM_LOG_FREQ_PCT	3
#define PSHISTOGRAM_LOG10_COUNTS	4
#define PSHISTOGRAM_LOG10_FREQ_PCT	5

struct PSHISTOGRAM_INFO {	/* Control structure for pshistogram */
	double yy0, yy1;
	double box_width;
	double wesn[4];
	GMT_LONG n_boxes;
	GMT_LONG n_counted;
	GMT_LONG center_box, cumulative;
	GMT_LONG hist_type;
	GMT_LONG *boxh;
};

GMT_LONG fill_boxes (struct GMT_CTRL *GMT, struct PSHISTOGRAM_INFO *F, double *data, GMT_LONG n) {

	double add_half = 0.0;
	GMT_LONG b0, b1, i, ibox, count_sum;

	F->n_boxes = (GMT_LONG)ceil(((F->wesn[XHI] - F->wesn[XLO]) / F->box_width) + 0.5);

	if (F->center_box) {
		F->n_boxes++;
		add_half = 0.5;
	}

	if (F->n_boxes <= 0) return (-1);

	F->boxh = GMT_memory (GMT, NULL, F->n_boxes, GMT_LONG);

	F->n_counted = 0;

	/* First fill boxes with counts  */

	for (i = 0; i < n; i++) {
		ibox = (GMT_LONG)floor (((data[i] - F->wesn[XLO]) / F->box_width) + add_half);
		if (ibox < 0 || ibox >= F->n_boxes) continue;
		F->boxh[ibox]++;
		F->n_counted++;
	}

	if (F->cumulative) {
		for (ibox = count_sum = b0 = 0; ibox < F->n_boxes; ibox++) {
			count_sum += F->boxh[ibox];
			F->boxh[ibox] = (GMT_LONG)count_sum;
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

GMT_LONG plot_boxes (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, struct PSHISTOGRAM_INFO *F, GMT_LONG stairs, GMT_LONG flip_to_y, GMT_LONG draw_outline, struct GMT_PEN *pen, struct GMT_FILL *fill, GMT_LONG cpt)
{
	GMT_LONG i, ibox, first = TRUE, index;
	double rgb[4], x[4], y[4], xx, yy, xval, *px = NULL, *py = NULL;
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

			for (i = 0; i < 4; i++) {
				GMT_geo_to_xy (GMT, px[i], py[i], &xx, &yy);
				px[i] = xx;	py[i] = yy;
			}

			if (stairs) {
				if (first) {
					first = FALSE;
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
				PSL_plotpolygon (PSL, px, py, (GMT_LONG)4);
			}
			else {
				GMT_setfill (GMT, fill, draw_outline);
				PSL_plotpolygon (PSL, px, py, (GMT_LONG)4);
			}
		}
	}

	if (stairs) PSL_plotpoint (PSL, px[1], py[1], PSL_DRAW + PSL_STROKE);

	return (0);
}

GMT_LONG get_loc_scl (struct GMT_CTRL *GMT, double *data, GMT_LONG n, double *stats)
{
	/* Returns stats[] = L2, L1, LMS location, L2, L1, LMS scale  */

	GMT_LONG i, j, n_multiples;
	double dx;

	if (n < 3) return (-1);

	GMT_sort_array (GMT, data, n, GMT_DOUBLE_TYPE);

	/* Get median */
	j = n/2;
	stats[1] = (n%2) ? data[j] : (0.5 * (data[j] + data[j-1]));

	/* Get mode */

	GMT_mode (GMT, data, n, j, 0, 0, &n_multiples, &stats[2]);
	if (n_multiples > 0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %ld multiple modes found\n", n_multiples);

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
	
void *New_pshistogram_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSHISTOGRAM_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct PSHISTOGRAM_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Do not fill is default */
	C->L.pen = GMT->current.setting.map_default_pen;
		
	return (C);
}

void Free_pshistogram_Ctrl (struct GMT_CTRL *GMT, struct PSHISTOGRAM_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	if (C->C.file) free (C->C.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_pshistogram_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "pshistogram %s [API] - Calculate and plot histograms\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: pshistogram [<table>] %s -W<width> [%s] [-C<cpt>] [-F] [-G<fill>] [-I[o|O]]\n", GMT_Jx_OPT, GMT_B_OPT);
	GMT_message (GMT, "\t[%s] [-K] [-L<pen>] [-O] [-P] [-Q] [%s] [-S]\n", GMT_Jz_OPT, GMT_Rx_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[-Z[0-5]] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\n", GMT_bi_OPT, GMT_c_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-Jx|X for linear projection.  Scale in %s/units (or width in %s)\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit], GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t    Use / to specify separate x/y scaling.\n");
	GMT_message (GMT, "\t    If -JX is used then give axes lengths in %s rather than scales\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t-W Set the bin width.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<b");
	GMT_message (GMT, "\t-A Plot horizontal bars [Default is vertical].\n");
	GMT_message (GMT, "\t-C Use cpt-file to assign fill to bars based on the mid x-value.\n");
	GMT_message (GMT, "\t-F Center the bins.\n");
	GMT_fill_syntax (GMT, 'G', "Select color/pattern for columns.");
	GMT_message (GMT, "\t-I Inquire about min/max x and y.  No plotting is done.\n");
	GMT_message (GMT, "\t   Append o to output the resulting x, y data.\n");
	GMT_message (GMT, "\t   Append O to output all resulting x, y data even with y=0.\n");
	GMT_explain_options (GMT, "ZK");
	GMT_pen_syntax (GMT, 'L', "Specify pen to draw histogram.");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q Plot a cumulative histogram.\n");
	GMT_explain_options (GMT, "r");
	GMT_message (GMT, "\t   If neither -R nor -I are set, w/e/s/n will be based on input data.\n");
	GMT_message (GMT, "\t-S Draw a stairs-step diagram [Default is bar histogram].\n");
	GMT_explain_options (GMT, "UVX");
	GMT_message (GMT, "\t-Z To choose type of vertical axis.  Select from\n");
	GMT_message (GMT, "\t   0 - Counts [Default].\n");
	GMT_message (GMT, "\t   1 - Frequency percent.\n");
	GMT_message (GMT, "\t   2 - Log (1+counts).\n");
	GMT_message (GMT, "\t   3 - Log (1+frequency percent).\n");
	GMT_message (GMT, "\t   4 - Log10 (1+counts).\n");
	GMT_message (GMT, "\t   5 - Log10 (1+frequency percent).\n");
	GMT_explain_options (GMT, "C2cfhipt.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_pshistogram_parse (struct GMTAPI_CTRL *C, struct PSHISTOGRAM_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pshistogram and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				break;
			case 'C':
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = TRUE;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (opt->arg[0] == 'o') Ctrl->I.mode = 1;
				if (opt->arg[0] == 'O') Ctrl->I.mode = 2;
				break;
			case 'L':		/* Set line attributes */
				Ctrl->L.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
#ifdef GMT_COMPAT
			case 'T':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: The -T option is deprecated; use -i instead.\n");
				n_errors += gmt_parse_i_option (GMT, opt->arg);
				break;
#endif
			case 'W':
				Ctrl->W.active = TRUE;
				Ctrl->W.inc = atof (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				Ctrl->Z.mode = atoi (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->I.active && !GMT_IS_LINEAR (GMT), "Syntax error -J option: Only linear projection supported.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.mode < PSHISTOGRAM_COUNTS || Ctrl->Z.mode > PSHISTOGRAM_LOG10_FREQ_PCT, "Syntax error -Z option: histogram type must be in 0-5 range\n");
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

GMT_LONG GMT_pshistogram (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, automatic = FALSE, n_alloc = GMT_CHUNK, n, n_fields;

	char format[GMT_BUFSIZ];
	
	double *data = NULL, stats[6], tmp, x_min, x_max, *in = NULL;
	
	struct PSHISTOGRAM_INFO F;
	struct PSHISTOGRAM_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_pshistogram_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_pshistogram_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pshistogram", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRbf", "BKOPUXxYychips>" GMT_OPT("E"), options))) Return (error);
	Ctrl = New_pshistogram_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pshistogram_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pshistogram main code ----------------------------*/

	GMT_memset (&F, 1, struct PSHISTOGRAM_INFO);
	F.hist_type = PSHISTOGRAM_COUNTS;
	F.hist_type = Ctrl->Z.mode;
	F.box_width = Ctrl->W.inc;
	F.cumulative = Ctrl->Q.active;
	F.center_box = Ctrl->F.active;
	if (Ctrl->I.active) GMT->current.setting.verbose = GMT_MSG_NORMAL;
	if (!Ctrl->I.active && !GMT->common.R.active) automatic = TRUE;
	if (GMT->common.R.active) GMT_memcpy (F.wesn, GMT->common.R.wesn, 4, double);

	if ((error = GMT_set_cols (GMT, GMT_IN, 1))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

	if (Ctrl->C.active) {
		if ((P = GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->C.file, NULL)) == NULL) Return (GMT_DATA_READ_ERROR);
	}

	data = GMT_memory (GMT, NULL, n_alloc , double);

	n = 0;
	x_min = DBL_MAX;	x_max = -DBL_MAX;

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, &in)) != EOF) {	/* Keep returning records until we have no more files */

		if (GMT_REC_IS_ERROR(GMT)) Return (EXIT_FAILURE);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip all headers */
		
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
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (n == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Fatal error, read only 0 points.\n");
		Return (EXIT_FAILURE);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "%ld points read\n", n);

	data = GMT_memory (GMT, data, n, double);

	get_loc_scl (GMT, data, n, stats);

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "Extreme values of the data :\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, data[0], data[n-1]);
		sprintf (format, "Locations: L2, L1, LMS; Scales: L2, L1, LMS\t%s\t%s\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, stats[0], stats[1], stats[2], stats[3], stats[4], stats[5]);
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
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].active = TRUE;
			GMT->current.map.frame.draw = TRUE;
		}
	}

	if (fill_boxes (GMT, &F, data, n)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Fatal error during box fill.\n");
		Return (EXIT_FAILURE);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "min/max values are :\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, x_min, x_max, F.yy0, F.yy1);
	}

	if (Ctrl->I.active) {	/* Only info requested, quit before plotting */
		if (Ctrl->I.mode) {
			GMT_LONG ibox, dim[4] = {1, 1, 2, 0};
			double xx, yy;
			struct GMT_DATASET *D = NULL;
			struct GMT_LINE_SEGMENT *S = NULL;
			
			dim[3] = F.n_boxes;
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, dim, -1)) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a data set for spectrum\n");
				return (API->error);
			}
			if ((error = GMT_set_cols (GMT, GMT_OUT, 2))) Return (error);
			if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */
			S = D->table[0]->segment[0];	/* Only one table with one segment here, with 2 cols and F.n_boxes rows */
			for (ibox = 0; ibox < F.n_boxes; ibox++) {
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
				S->coord[GMT_X][ibox] = xx;
				S->coord[GMT_Y][ibox] = yy;
			}
			if ((error = GMT_Put_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, NULL, D->io_mode, Ctrl->Out.file, D))) return (error);
			GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, &D);
		}
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
		GMT_free (GMT, data);
		GMT_free (GMT, F.boxh);
		Return (EXIT_SUCCESS);
	}

	if (automatic) {	/* Set up s/n based on 'clever' rounding up of the minmax values */
		GMT->common.R.active = TRUE;
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
			GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].active = TRUE;
			GMT->current.map.frame.draw = TRUE;
		}
		if (GMT->current.proj.pars[0] == 0.0 && GMT->current.proj.pars[1] == 0.0) {
			GMT_report (GMT, GMT_MSG_FATAL, "Need to provide both x- and y-scale.\n");
			Return (EXIT_FAILURE);
		}
	}

	if (automatic && GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "Use w/e/s/n = %s/%s/%s/%s and x-tick/y-tick = %s/%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, F.wesn[XLO], F.wesn[XHI], F.wesn[YLO], F.wesn[YHI], GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval);
	}

	if (Ctrl->A.active) {
		char buffer[GMT_TEXT_LEN256];
		d_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval);
		d_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_LOWER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_LOWER].interval);
		d_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_UPPER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_UPPER].interval);
		d_swap (GMT->current.map.frame.axis[GMT_X].item[GMT_TICK_LOWER].interval, GMT->current.map.frame.axis[GMT_Y].item[GMT_TICK_LOWER].interval);
		strcpy (buffer, GMT->current.map.frame.axis[GMT_X].label);
		strcpy (GMT->current.map.frame.axis[GMT_X].label, GMT->current.map.frame.axis[GMT_Y].label);
		strcpy (GMT->current.map.frame.axis[GMT_Y].label, buffer);
		GMT_err_fail (GMT, GMT_map_setup (GMT, F.wesn), "");
	}
	else
		GMT_err_fail (GMT, GMT_map_setup (GMT, F.wesn), "");

	GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
	if (plot_boxes (GMT, PSL, P, &F, Ctrl->S.active, Ctrl->A.active, Ctrl->L.active, &Ctrl->L.pen, &Ctrl->G.fill, Ctrl->C.active) ) {
		GMT_report (GMT, GMT_MSG_FATAL, "Fatal error during box plotting.\n");
		Return (EXIT_FAILURE);
	}
	GMT_map_clip_off (GMT);

	GMT_map_basemap (GMT);
	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	GMT_free (GMT, data);
	GMT_free (GMT, F.boxh);

	Return (GMT_OK);
}
