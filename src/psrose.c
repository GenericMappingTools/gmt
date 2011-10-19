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
 * Brief synopsis: psrose reads a file [or standard input] with azimuth and length information
 * and draws a sector or rose diagram.  Several options for plot layout are available.
 * 2 diagrams are possible: Full circle (360) or half circle (180).  In the
 * latter case azimuths > 180 are reversed (-= 180).
 *
 * To be compatible with GMT, I assume radial distance to be "x"
 * and azimuth to be "y".  Hence, west = 0.0 and east = max_radius
 * south/north is -90,90 for halfcircle and 0,360 for full circle
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "pslib.h"
#include "gmt.h"

struct PSROSE_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A<sector_angle>[r] */
		GMT_LONG active;
		GMT_LONG rose;
		double inc;
	} A;
	struct C {	/* -C[<modefile>] */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		char *w, *e, *s, *n;
	} L;
	struct M {	/* -M<params> */
		GMT_LONG active;
		double v_width, h_length, h_width;
		double rgb[4];
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -Sscale[n] */
		GMT_LONG active;
		GMT_LONG normalize;
		double scale;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Zscale */
		GMT_LONG active;
		double scale;
	} Z;
};

void *New_psrose_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSROSE_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct PSROSE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->M.rgb[0] = C->M.rgb[1] = C->M.rgb[2] = 0;
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->W.pen = GMT->current.setting.map_default_pen;
	C->S.scale = 3.0;
	C->Z.scale = 1.0;
	if (GMT->current.setting.proj_length_unit == GMT_CM) {
		C->S.scale = 7.5 / 2.54;
	}
	C->M.v_width  = VECTOR_LINE_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 2p */
	C->M.h_width  = VECTOR_HEAD_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 7p */
	C->M.h_length = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
	return (C);
}

void Free_psrose_Ctrl (struct GMT_CTRL *GMT, struct PSROSE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->C.file) free (C->C.file);	
	if (C->L.w) free (C->L.w);	
	if (C->L.e) free (C->L.e);	
	if (C->L.s) free (C->L.s);	
	if (C->L.n) free (C->L.n);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_psrose_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	double r;
	char *choice[2] = {"OFF", "ON"};
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the psrose synopsis and optionally full usage information */

	GMT_message (GMT, "psrose %s [API] - Plot a polar histogram (rose, sector, windrose diagrams)\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: psrose [<table>] [-A<sector_angle>[r]] [%s] [-C[<modes>]] [-D] [-G<fill>] [-I]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-K] [-L[<wlab>/<elab>/<slab>/<nlab>]] [-M<parameters>] [-N] [-O] [-P]\n");
	GMT_message (GMT, "\t[-R<r0>/<r1>/<theta0>/<theta1>] [-S<scale>[n]] [-T] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [-W<pen>] [%s] [%s] [-Z<scale>]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_c_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Sector width in degrees for sector diagram [Default is windrose];\n");
	GMT_message (GMT, "\t   append r to get rose diagram.\n");
	GMT_explain_options (GMT, "B");
	GMT_message (GMT, "\t   (Remember: radial is x-direction, azimuthal is y-direction).\n");
	GMT_message (GMT, "\t-C Plot vectors listed in the <modes> file.  If no file, use mean direction.\n");
	GMT_message (GMT, "\t-D Will center the sectors.\n");
	GMT_message (GMT, "\t-F Do not draw the scale length bar [Default plots scale in lower right corner].\n");
	GMT_fill_syntax (GMT, 'G', "Specify color for diagram [Default is no fill].");
	GMT_message (GMT, "\t-I Inquire mode; only compute statistics - no plot is created.\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-L Override default labels [Default is WEST/EAST/SOUTH/NORTH for full circle and 90W/90E/-/0 for half-circle].\n");
	GMT_message (GMT, "\t   If no argument is given then labels will be disabled.  Give - to disable an individual label.\n");
	GMT_message (GMT, "\t-M Append <vectorwidth>/<headlength>/<headwidth>/<color> to set arrow attributes [%gp/%gp/%gp/black].\n", VECTOR_LINE_WIDTH, VECTOR_HEAD_LENGTH, VECTOR_HEAD_WIDTH);
	GMT_message (GMT, "\t-N Normalize rose plots for area, i.e. takes sqrt(r) before plotting [FALSE].\n");
	GMT_message (GMT, "\t   Only applicable if normalization has been specified with -S<radius>n.\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-R Specifys the region.  (<r0> = 0, <r1> = max_radius.  For azimuth:\n");
	GMT_message (GMT, "\t   Specify <theta0>/<theta1> = -90/90 (half-circle) or 0/360 only).\n");
	GMT_message (GMT, "\t   If <r0> = <r1> = 0, psrose will compute a reasonable <r1> value.\n");
	r = (GMT->current.setting.proj_length_unit == GMT_CM) ? 7.5 : 3.0;
	GMT_message (GMT, "\t-S Specify the radius of the unit circle in %s [%g]. Normalize r if n is appended.\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit], r);
	GMT_message (GMT, "\t-T Indicate that the vectors are oriented (two-headed), not directed [Default].\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Set pen attributes for outline of rose [Default is no outline].");
	GMT_explain_options (GMT, "X");
	GMT_message (GMT, "\t-Z Multiply the radii by <scale> before plotting.\n");
	GMT_explain_options (GMT, "c");
	GMT_message (GMT, "\t-: Expect (azimuth,radius) input rather than (radius,azimuth) [%s].\n", choice[GMT->current.setting.io_lonlat_toggle[GMT_IN]]);
	GMT_explain_options (GMT, "C2hipt.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_psrose_parse (struct GMTAPI_CTRL *C, struct PSROSE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psrose and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n, n_errors = 0, n_files = 0;
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256], txt_d[GMT_TEXT_LEN256];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Get Sector angle in degrees */
				Ctrl->A.active = TRUE;
				Ctrl->A.inc = atof (opt->arg);
				if (opt->arg[strlen (opt->arg)-1] == 'r') Ctrl->A.rose = TRUE;
				break;
			case 'C':	/* Read mode file and plot directions */
				Ctrl->C.active = TRUE;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Center the bins */
				Ctrl->D.active = TRUE;
				break;
			case 'F':	/* Disable scalebar plotting */
				Ctrl->F.active = TRUE;
				break;
			case 'G':	/* Set Gray shade */
				Ctrl->G.active = TRUE;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':	/* Compute statistics only - no plot */
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Overwride default labeling */
				Ctrl->L.active = TRUE;
				n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d) != 4, "Syntax error -L option: Expected\n\t-L<westlabel/eastlabel/southlabel/<northlabel>>\n");
				Ctrl->L.w = strdup (txt_a);	Ctrl->L.e = strdup (txt_b);
				Ctrl->L.s = strdup (txt_c);	Ctrl->L.n = strdup (txt_d);
				break;
			case 'M':	/* Get arrow parameters */
				Ctrl->M.active = TRUE;
				n = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				if (n != 4 || GMT_getrgb (GMT, txt_d, Ctrl->M.rgb)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -M option: Expected\n\t-M<tailwidth/headlength/headwidth/<color>>\n");
					n_errors++;
				}
				else {
					Ctrl->M.v_width = GMT_to_inch (GMT, txt_a);
					Ctrl->M.h_length = GMT_to_inch (GMT, txt_b);
					Ctrl->M.h_width = GMT_to_inch (GMT, txt_c);
				}
				break;
			case 'N':	/* Make sectors area be proportional to frequency instead of radius */
				Ctrl->N.active = TRUE;
				break;
			case 'S':	/* Get radius of unit circle in inches */
				Ctrl->S.active = TRUE;
				n = strlen (opt->arg) - 1;
				if (opt->arg[n] == 'n') {
					Ctrl->S.normalize = TRUE;
					opt->arg[n] = 0;	/* Temporarily remove the n */
				}
				Ctrl->S.scale = GMT_to_inch (GMT, opt->arg);
				if (Ctrl->S.normalize) opt->arg[n] = 'n';	/* Put back the n */
				break;
			case 'T':	/* Oriented instead of directed data */
				Ctrl->T.active = TRUE;
				break;
			case 'W':	/* Get pen width for outline */
				Ctrl->W.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':	/* Scale radii before using data */
				Ctrl->Z.active = TRUE;
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	GMT->common.R.wesn[XLO] = 0.0;
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->C.file && GMT_access (GMT, Ctrl->C.file, R_OK), "Syntax error -C: Cannot read file %s!\n", Ctrl->C.file);
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale <= 0.0, "Syntax error -S option: radius must be nonzero\n");
	n_errors += GMT_check_condition (GMT, GMT_IS_ZERO (Ctrl->Z.scale), "Syntax error -Z option: factor must be nonzero\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.inc < 0.0, "Syntax error -A option: sector width must be positive\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !((GMT->common.R.wesn[YLO] == -90.0 && GMT->common.R.wesn[YHI] == 90.0) || (GMT->common.R.wesn[YLO] == 0.0 && GMT->common.R.wesn[YHI] == 360.0)), "Syntax error -R option: theta0/theta1 must be either -90/90 or 0/360\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psrose_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_psrose (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, find_mean = FALSE, half_only = FALSE;
	GMT_LONG automatic = FALSE, sector_plot = FALSE, windrose = TRUE;
	GMT_LONG n_bins, n_annot, n_alpha, n_modes, n_fields, form;
	GMT_LONG i, bin, n = 0, do_fill = FALSE, n_alloc = GMT_CHUNK;

	char text[GMT_BUFSIZ], format[GMT_BUFSIZ];

	double max = 0.0, radius, az, x_origin, y_origin, tmp, one_or_two = 1.0, s, c;
	double angle1, angle2, angle, x, y, mean_theta, mean_radius, xr = 0.0, yr = 0.0;
	double x1, x2, y1, y2, total = 0.0, total_arc, off, max_radius, az_offset;
	double asize, lsize, this_az, half_bin_width, wesn[4];
	double *xx = NULL, *yy = NULL, *in = NULL, *sum = NULL, *azimuth = NULL;
	double *length = NULL, *mode_direction = NULL, *mode_length = NULL, dim[7];

	struct PSROSE_CTRL *Ctrl = NULL;
	struct GMT_DATASET *Cin = NULL;
	struct GMT_TABLE *P = NULL;
	struct GMT_FILL f;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_psrose_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_psrose_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_psrose", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRb:", "BKOPUXxYychipst>" GMT_OPT("E"), options))) Return (error);
	Ctrl = New_psrose_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psrose_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the psrose main code ----------------------------*/

	asize = GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
	lsize = GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];

	max_radius = GMT->common.R.wesn[XHI];
	half_only = GMT_IS_ZERO (GMT->common.R.wesn[YLO] + 90.0);
	if (Ctrl->A.rose) windrose = FALSE;
	sector_plot = (Ctrl->A.inc > 0.0);
	if (sector_plot) windrose = FALSE;	/* Draw rose diagram instead of sector diagram */
	if (!Ctrl->S.normalize) Ctrl->N.active = FALSE;	/* Only do this is data is normalized for length also */
	if (!Ctrl->I.active && !GMT->common.R.active) automatic = TRUE;
	if (Ctrl->T.active) one_or_two = 2.0;
	half_bin_width = Ctrl->D.active * Ctrl->A.inc * 0.5;
	if (half_only) {
		total_arc = 180.0;
		az_offset = 90.0;
	}
	else {
		total_arc = 360.0;
		az_offset = 0.0;
	}
	n_bins = (Ctrl->A.inc <= 0.0) ? 1 : irint (total_arc / Ctrl->A.inc);

	sum = GMT_memory (GMT, NULL, n_bins, double);
	xx = GMT_memory (GMT, NULL, n_bins+2, double);
	yy = GMT_memory (GMT, NULL, n_bins+2, double);
	azimuth = GMT_memory (GMT, NULL, n_alloc, double);
	length = GMT_memory (GMT, NULL, n_alloc, double);

	/* Read data and do some stats */

	n = 0;
	if ((error = GMT_set_cols (GMT, GMT_IN, 2))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, &in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);	/* Bail for any i/o error */
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;		/* Skip all table and segment headers */

		length[n]  = in[GMT_X];
		azimuth[n] = in[GMT_Y];

		if (Ctrl->Z.scale != 1.0) length[n] *= Ctrl->Z.scale;

		/* Make sure azimuth is in 0 <= az < 360 range */

		while (azimuth[n] < 0.0)    azimuth[n] += 360.0;
		while (azimuth[n] >= 360.0) azimuth[n] -= 360.0;

		if (half_only) {	/* Flip azimuths about E-W line i.e. -90 < az <= 90 */
			if (azimuth[n] > 90.0 && azimuth[n] <= 270.0) azimuth[n] -= 180.0;
			if (azimuth[n] > 270.0) azimuth[n] -= 360.0;
		}

		/* Double angle to find mean azimuth */

		sincosd (one_or_two * azimuth[n], &s, &c);
		xr += length[n] * c;
		yr += length[n] * s;

		total += length[n];

		n++;
		if (n == n_alloc) {	/* Get more memory */
			n_alloc <<= 1;
			azimuth = GMT_memory (GMT, azimuth, n_alloc, double);
			length = GMT_memory (GMT, length, n_alloc, double);
		}
	}

	if (Ctrl->A.inc > 0.0) {	/* Sum up sector diagram info */
		for (i = 0; i < n; i++) {
			if (Ctrl->D.active) {	/* Center bin by removing half bin width here */
				this_az = azimuth[i] - half_bin_width;
				if (!half_only && this_az < 0.0)   this_az += 360.0;
				if (half_only  && this_az < -90.0) this_az += 180.0;
			}
			else
				this_az = azimuth[i];
			bin = (GMT_LONG) ((this_az + az_offset) / Ctrl->A.inc);
			sum[bin] += length[i];
		}
	}

	mean_theta = d_atan2d (yr, xr) / one_or_two;
	if (mean_theta < 0.0) mean_theta += 360.0;
	mean_radius = hypot (xr, yr) / total;
	if (!Ctrl->S.normalize) mean_radius *= max_radius;

	if (Ctrl->A.inc > 0.0) {	/* Find max of the bins */
		for (bin = 0; bin < n_bins; bin++) if (sum[bin] > max) max = sum[bin];
		if (Ctrl->S.normalize) for (bin = 0; bin < n_bins; bin++) sum[bin] /= max;
		if (Ctrl->N.active) for (bin = 0; bin < n_bins; bin++) sum[bin] = sqrt (sum[bin]);
	}
	else {
		for (i = 0; i < n; i++) if (length[i] > max) max = length[i];
		if (Ctrl->S.normalize) {
			max = 1.0 / max;
			for (i = 0; i < n; i++) length[i] *= max;
			max = 1.0 / max;
		}
	}

	if (Ctrl->I.active || GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		if (Ctrl->In.file) strcpy (text, Ctrl->In.file); else strcpy (text, "<stdin>");
		sprintf (format, "Info for %%s: n = %%ld rmax = %s mean r/az = (%s/%s) totlength = %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_FATAL, format, text, n, max, mean_radius, mean_theta, total);
		if (Ctrl->I.active) {
			GMT_free (GMT, sum);
			GMT_free (GMT, xx);
			GMT_free (GMT, yy);
			GMT_free (GMT, azimuth);
			GMT_free (GMT, length);
			Return (EXIT_SUCCESS);
		}
	}

	if (automatic) {
		if (GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval)) {
			tmp = pow (10.0, floor (d_log10 (GMT, max)));
			if ((max / tmp) < 3.0) tmp *= 0.5;
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
		max_radius = ceil (max / tmp) * tmp;
		if (GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) || GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval)) {	/* Tickmarks not set */
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval = tmp;
			GMT->current.map.frame.draw = TRUE;
		}
	}

	if (GMT->current.map.frame.draw && GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval)) GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval = 30.0;

	/* Ready to plot.  So set up GMT projections (not used by psrose), we set region to actual plot width and scale to 1 */

	GMT_parse_common_options (GMT, "J", 'J', "x1i");
	GMT->common.R.active = GMT->common.J.active = TRUE;
	wesn[XLO] = wesn[YLO] = -Ctrl->S.scale;	wesn[XHI] = wesn[YHI] = Ctrl->S.scale;
	GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");

	if (GMT->current.map.frame.paint) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		GMT->current.map.frame.paint = FALSE;	/* Turn off so GMT_plotinit wont fill */
		do_fill = TRUE;
	}
	GMT_plotinit (GMT, options);

	x_origin = Ctrl->S.scale;	y_origin = ((half_only) ? 0.0 : Ctrl->S.scale);
	PSL_setorigin (PSL, x_origin, y_origin, 0.0, PSL_FWD);
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	if (!Ctrl->S.normalize) Ctrl->S.scale /= max_radius;

	if (do_fill) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		double dim = 2.0 * Ctrl->S.scale;
		GMT->current.map.frame.paint = TRUE;	/* Restore original setting */
		if (half_only) {	/* Clip the circle */
			double xc[4], yc[4];
			xc[0] = xc[3] = -Ctrl->S.scale;	xc[1] = xc[2] = Ctrl->S.scale;
			yc[0] = yc[1] = 0.0;	yc[2] = yc[3] = Ctrl->S.scale;
			PSL_beginclipping (PSL, xc, yc, 4, GMT->session.no_rgb, 3);
		}
		GMT_setfill (GMT, &GMT->current.map.frame.fill, FALSE);
		PSL_plotsymbol (PSL, 0.0, 0.0, &dim, GMT_SYMBOL_CIRCLE);
		if (half_only) PSL_endclipping (PSL, 1);		/* Reduce polygon clipping by one level */
	}
	if (GMT->common.B.active) {	/* Draw frame */
		GMT_LONG symbol = (half_only) ? GMT_SYMBOL_WEDGE : GMT_SYMBOL_CIRCLE;
		double dim[3];
		struct GMT_FILL no_fill;
		GMT_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);
		dim[0] = (half_only) ? Ctrl->S.scale : 2.0 * Ctrl->S.scale;
		dim[1] = 0.0;
		dim[2] = (half_only) ? 180.0 : 360.0;
		GMT_setpen (GMT, &GMT->current.setting.map_frame_pen);
		GMT_setfill (GMT, &no_fill, TRUE);
		PSL_plotsymbol (PSL, 0.0, 0.0, dim, symbol);
	}

	GMT_setpen (GMT, &Ctrl->W.pen);
	if (windrose) {
		for (i = 0; i < n; i++) {
			sincosd (90.0 - azimuth[i], &s, &c);
			radius = length[i] * Ctrl->S.scale;
			PSL_plotsegment (PSL, 0.0, 0.0, radius * c, radius * s);
		}
	}

	if (sector_plot && !Ctrl->A.rose && Ctrl->G.fill.rgb[0] >= 0) {	/* Draw pie slices for sector plot if fill is requested */

		GMT_setfill (GMT, &(Ctrl->G.fill), FALSE);
		for (bin = 0; bin < n_bins; bin++) {
			az = bin * Ctrl->A.inc - az_offset + half_bin_width;
			dim[1] = (90.0 - az - Ctrl->A.inc);
			dim[2] = dim[1] + Ctrl->A.inc;
			dim[0] = sum[bin] * Ctrl->S.scale;
			PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_WEDGE);
		}
	}
	else if (Ctrl->A.rose) {	/* Draw rose diagram */

		for (bin = i = 0; bin < n_bins; bin++, i++) {
			az = (bin + 0.5) * Ctrl->A.inc - az_offset - half_bin_width;
			sincosd (90.0 - az, &s, &c);
			xx[i] = Ctrl->S.scale * sum[bin] * c;
			yy[i] = Ctrl->S.scale * sum[bin] * s;
		}
		if (half_only) {
			xx[i] = Ctrl->S.scale * 0.5 * (sum[0] + sum[n_bins-1]);
			yy[i++] = 0.0;
			xx[i] = -xx[i-1];
			yy[i++] = 0.0;
		}
		PSL_setfill (PSL, Ctrl->G.fill.rgb, Ctrl->W.active);
		PSL_plotpolygon (PSL, xx, yy, i);
	}

	if (sector_plot && Ctrl->W.active && !Ctrl->A.rose) {	/* Draw a line outlining the pie slices */
		angle1 = ((half_only) ? 180.0 : 90.0) - half_bin_width;
		angle2 = ((half_only) ?   0.0 : 90.0) - half_bin_width;
		sincosd (angle1, &s, &c);
		x1 = (sum[0] * Ctrl->S.scale) * c;
		y1 = (sum[0] * Ctrl->S.scale) * s;
		sincosd (angle2, &s, &c);
		x2 = (sum[n_bins-1] * Ctrl->S.scale) * c;
		y2 = (sum[n_bins-1] * Ctrl->S.scale) * s;
		PSL_plotpoint (PSL, x1, y1, PSL_MOVE);
		PSL_plotpoint (PSL, x2, y2, PSL_DRAW);
		for (bin = n_bins-1; bin >= 0; bin--) {
			az = bin * Ctrl->A.inc - az_offset + half_bin_width;
			angle1 = 90.0 - az - Ctrl->A.inc;
			angle2 = angle1 + Ctrl->A.inc;
			PSL_plotarc (PSL, 0.0, 0.0, sum[bin] * Ctrl->S.scale, angle1, angle2, (bin == 0) ? PSL_STROKE : PSL_DRAW);
		}
	}

	if (Ctrl->C.active) {

		if (!Ctrl->C.file) {	/* Not given, calculate and use mean direction only */
			find_mean = TRUE;
			n_modes = 1;
			mode_direction = GMT_memory (GMT, NULL, 1, double);
			mode_length = GMT_memory (GMT, NULL, 1, double);
			mode_direction[0] = mean_theta;
			mode_length[0] = mean_radius;
		}
		else {	/* Get mode parameters from separate file */
			if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->C.file, &Cin)) Return ((error = GMT_DATA_READ_ERROR));
			P = Cin->table[0];	/* Can only be one table since we read a single file; We also only use the first segment */
			n_modes = P->n_records;
			mode_direction = P->segment[0]->coord[GMT_X];
			mode_length = P->segment[0]->coord[GMT_Y];
		}
		for (i = 0; i < n_modes; i++) {
			if (Ctrl->N.active) mode_length[i] = sqrt (mode_length[i]);
			if (half_only && mode_direction[i] > 90.0 && mode_direction[i] <= 270.0) mode_direction[i] -= 180.0;
			angle = 90.0 - mode_direction[i];
			sincosd (angle, &s, &c);
			xr = Ctrl->S.scale * mode_length[i] * c;
			yr = Ctrl->S.scale * mode_length[i] * s;
			GMT_init_fill (GMT, &f, Ctrl->M.rgb[0], Ctrl->M.rgb[1], Ctrl->M.rgb[2]);       /* Initialize fill structure */
			dim[0] = xr, dim[1] = yr;
			dim[2] = Ctrl->M.v_width, dim[3] = Ctrl->M.h_length, dim[4] = Ctrl->M.h_width;
			dim[5] = GMT->current.setting.map_vector_shape, dim[6] = 0.0;
			GMT_setfill (GMT, &f, TRUE);       /* Use fill structure */
			PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_VECTOR);
		}

	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	if (Ctrl->L.active) {	/* Deactivate those with - */
		if (Ctrl->L.w[0] == '-' && Ctrl->L.w[1] == '\0') Ctrl->L.w[0] = '\0';
		if (Ctrl->L.e[0] == '-' && Ctrl->L.e[1] == '\0') Ctrl->L.e[0] = '\0';
		if (Ctrl->L.s[0] == '-' && Ctrl->L.s[1] == '\0') Ctrl->L.s[0] = '\0';
		if (Ctrl->L.n[0] == '-' && Ctrl->L.n[1] == '\0') Ctrl->L.n[0] = '\0';
	}
	else {	/* Use default labels */
		Ctrl->L.w = strdup ("WEST");	Ctrl->L.e = strdup ("EAST");	Ctrl->L.s = strdup ("SOUTH");	Ctrl->L.n = strdup ("NORTH");
	}
	if (GMT->current.map.frame.draw) {	/* Draw grid lines etc */
		GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
		off = max_radius * Ctrl->S.scale;
		n_alpha = (GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval > 0.0) ? irint (total_arc / GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval) : -1;
		for (i = 0; i <= n_alpha; i++) {
			angle = i * GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval;
			sincosd (angle, &s, &c);
			x = max_radius * Ctrl->S.scale * c;
			y = max_radius * Ctrl->S.scale * s;
			PSL_plotsegment (PSL, 0.0, 0.0, x, y);
		}

		n_bins = (GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval > 0.0) ? irint (max_radius / GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) : -1;
		for (i = 1; i <= n_bins; i++)
			PSL_plotarc (PSL, 0.0, 0.0, i * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval * Ctrl->S.scale, 0.0, total_arc, PSL_MOVE + PSL_STROKE);
		PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
		y = lsize + 6.0 * GMT->current.setting.map_annot_offset[0];
		form = GMT_setfont (GMT, &GMT->current.setting.font_title);
		PSL_plottext (PSL, 0.0, off + y, GMT->current.setting.font_title.size, GMT->current.map.frame.header, 0.0, 2, form);

		GMT_get_format (GMT, GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_X].unit, GMT->current.map.frame.axis[GMT_X].prefix, format);

		if (half_only) {
			char text[GMT_TEXT_LEN64];
			if (!Ctrl->L.active) {	/* Use default labels */
				free (Ctrl->L.w);	free (Ctrl->L.e);	free (Ctrl->L.n);
				if (GMT->current.setting.map_degree_symbol == gmt_none) {
					Ctrl->L.w = strdup ("90W");
					Ctrl->L.e = strdup ("90E");
					Ctrl->L.n = strdup ("0");
				}
				else {
					sprintf (text, "90%cW", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.w = strdup (text);
					sprintf (text, "90%cE", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.e = strdup (text);
					sprintf (text, "0%c",   (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.n = strdup (text);
				}
			}
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			y = -(3.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * asize);
			if (GMT->current.map.frame.axis[GMT_X].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_X].label, 0.0, 10, form);
			y = -(5.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, 10, form);
			form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
			PSL_plottext (PSL, 0.0, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, "0", 0.0, 10, form);
			n_annot = (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval > 0.0) ? irint (max_radius / GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) : -1;
			for (i = 1; i <= n_annot; i++) {
				x = i * GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
				sprintf (text, format, x);
				x *= Ctrl->S.scale;
				PSL_plottext (PSL, x, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
				PSL_plottext (PSL, -x, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
			}
		}
		else {
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			PSL_plottext (PSL, 0.0, -off - 2.0 * GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_label.size, Ctrl->L.s, 0.0, 10, form);
			if (!Ctrl->F.active) {	/* Draw scale bar */
				PSL_plotsegment (PSL, off, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off);
				PSL_plotsegment (PSL, off, -off, off, GMT->current.setting.map_tick_length[0] - off);
				PSL_plotsegment (PSL, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, GMT->current.setting.map_tick_length[0] - off);
				if (GMT->current.map.frame.axis[GMT_X].label[0]) {
					strcat (format, " %s");
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval, GMT->current.map.frame.axis[GMT_X].label);
				}
				else
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval);
				form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
				PSL_plottext (PSL, (max_radius - 0.5 * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -(off + GMT->current.setting.map_annot_offset[0]), GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
			}
			y = -(off + 5.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, 10, form);
		}
		form = GMT_setfont (GMT, &GMT->current.setting.font_label);
		PSL_plottext (PSL, off + 2.0 * GMT->current.setting.map_annot_offset[0], 0.0, GMT->current.setting.font_label.size, Ctrl->L.e, 0.0, 5, form);
		PSL_plottext (PSL, -off - 2.0 * GMT->current.setting.map_annot_offset[0], 0.0, GMT->current.setting.font_label.size, Ctrl->L.w, 0.0, 7, form);
		PSL_plottext (PSL, 0.0, off + 2.0 * GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_label.size, Ctrl->L.n, 0.0, 2, form);
		PSL_setcolor (PSL, GMT->current.setting.map_default_pen.rgb, PSL_IS_STROKE);
	}
	
	GMT_plane_perspective (GMT, -1, 0.0);
	PSL_setorigin (PSL, -x_origin, -y_origin, 0.0, PSL_INV);
	GMT_plotend (GMT);

	GMT_free (GMT, sum);
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, azimuth);
	GMT_free (GMT, length);
	if (Ctrl->C.active) {
		if (find_mean) {
			GMT_free (GMT, mode_length);
			GMT_free (GMT, mode_direction);
		}
	}

	Return (GMT_OK);
}
