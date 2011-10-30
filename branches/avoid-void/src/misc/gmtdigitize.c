/*--------------------------------------------------------------------
 *    $Id$
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
 * gmtdigitize reads digitized points from a serial port connected to a
 * digitizer.  It is currently only been tested with a Calcomp DrawingBoard III
 * hooked up to a Linux box.  It is largely based on gmtdigitize for the main
 * processing.  I learned that I just needed to open the /dev/tty file by
 * looking at similar code from Scripps and Northwestern U.
 *
 *
 * Author:	Paul Wessel
 * Date:	19-MAY-2000
 * Version:	1.1
 *
 * Updated:	7/30/2007 PW: Also handle -Jx scaling where scaling is not 1:1
 *
 */
 
#include "gmt.h"

#ifndef WIN32
#include <termios.h>
#endif

/* Default port and line density in lines per inch (only tested on RedHat Linux 9)*/
#define DIG_PORT	"/dev/ttyS1"
#define DIG_LPI		2540

/*===========================================================================*/
/* Parameter specific to the digitizer and puck */

/* Currently set up for a Calcomp Drawingboard III with a 16-button puck.
   If you have a 4-digit puck then you need to change things here. */

#define TOLERANCE	(0.1/2.54)	/* Max slop allowed during calibration = 1 mm */
#define MULTISEG_BUTTON1 	13	/* Button number to initialize a new segment without comment */
#define MULTISEG_BUTTON2 	14	/* Button number to initialize a new segment and prompt for comment */
#define END_BUTTON		16	/* Button to exit program */
#define MULTISEG_BUTTON1_CHAR	'C'	/* Label on this button */
#define MULTISEG_BUTTON2_CHAR	'D'	/* Label on this button */
#define END_BUTTON_CHAR		'F'	/* Label on this button */
/*===========================================================================*/

#define K_ID	0
#define V_ID	1

struct GMTDIGITIZE_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct C {	/* Device */
		GMT_LONG active;
		char *device;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		double limit;
	} D;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG LPI;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		char *name;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct Z {	/* -Z */
		GMT_LONG active[2];
	} Z;
};

struct GMTDIGITIZE_INFO {	/* Things needed while digitizing */
	double sin_theta, cos_theta, map_x0, map_y0, map_scale[2], INV_LPI;
};

void *New_gmtdigitize_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTDIGITIZE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTDIGITIZE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->L.LPI = DIG_LPI;
	return (C);
}

void Free_gmtdigitize_Ctrl (struct GMT_CTRL *GMT, struct GMTDIGITIZE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.device) free (C->C.device);	
	if (C->N.name) free (C->N.name);	
	GMT_free (GMT, C);	
}

GMT_LONG get_digitize_raw (struct GMT_CTRL *GMT, GMT_LONG digunit, double *xdig, double *ydig, struct GMTDIGITIZE_INFO *C) {
	GMT_LONG i, n, ix, iy;
	char buffer[256], button;
	
	n = read (digunit, buffer, (size_t)255);
	if (n <= 0) return (END_BUTTON);
	n--;
	buffer[n] = 0;
#ifdef DEBUG
	GMT_report (GMT, GMT_MSG_DEBUG, "Got %ld bytes [%s]\n", n, buffer);
#endif
	for (i = 0; i < n; i++) if (buffer[i] == ',') buffer[i] = ' ';
	sscanf (buffer, "%ld %ld %c", &ix, &iy, &button);
	
	*xdig = (double)(ix) * C->INV_LPI;	/* Convert from lines per inch to inches */
	*ydig = (double)(iy) * C->INV_LPI;
	
	return ((GMT_LONG)(button - '0'));
}

GMT_LONG get_digitize_xy (struct GMT_CTRL *GMT, GMT_LONG digunit, double *xmap, double *ymap, struct GMTDIGITIZE_INFO *C) {
	GMT_LONG button;
	double x_raw, y_raw;
	
	button = get_digitize_raw (GMT, digunit, &x_raw, &y_raw, C);
	if (button == END_BUTTON) return (END_BUTTON);

	/* Undo rotation and scaling to give map inches */
	
	*xmap = (x_raw * C->cos_theta - y_raw * C->sin_theta) * C->map_scale[GMT_X] + C->map_x0;
	*ymap = (x_raw * C->sin_theta + y_raw * C->cos_theta) * C->map_scale[GMT_Y] + C->map_y0;
	
	return (button);
}

FILE *next_file (struct GMT_CTRL *GMT, char *name, int n_segments, char *this_file) {
	FILE *fp = NULL;
	
	if (name) {
		if (strchr (name, '%') != NULL)	/* Individual file names */
			sprintf (this_file, name, n_segments);
		else
			strncpy (this_file, name, (size_t)GMT_BUFSIZ);
		if ((fp = GMT_fopen (GMT, this_file, "w")) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Could not create file %s\n", this_file);
			return (NULL);
		}
	}
	else {
		strcpy (this_file, "<stdout>");
		fp = GMT->session.std[GMT_OUT];
	}
		
	return (fp);
}

GMT_LONG GMT_gmtdigitize_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtdigitize %s - Digitizing and inverse map transformation of map x/y coordinates\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtdigitize %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-A] [-C<device>] [-D<limit>] [-F] [-H] [-L<lpi>] [-N<namestem>] [-S]\n");
	GMT_message (GMT, "\t[-V] [-Zk|v] [%s] [%s]\n\n", GMT_bo_OPT, GMT_f_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_explain_options (GMT, "j");
	GMT_message (GMT, "\tScale or width is arbitrary as gmtdigitize solves for it based on calibration points\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A gives an audible alert each time a point is clicked [Default is silent]\n");
	GMT_message (GMT, "\t-C Device (port) to use [Default is %s]\n", DIG_PORT);
	GMT_message (GMT, "\t-D Only pass point when it is more than <limit> %s from previous point [0]\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t   Append c, i, m, or p for cm, inch, meter, or points [Default is %s]\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t-F Force program to accept 4 arbitrary calibration points [Default will use\n");
	GMT_message (GMT, "\t   the corners of the map if they are known to the program]\n");
	GMT_message (GMT, "\t-L sets the lines-pr-inch resolution of the digitizer [%d]\n", DIG_LPI);
	GMT_message (GMT, "\t-N sets name for output file(s).  If name contains a C-format\n");
	GMT_message (GMT, "\t   for integer (e.g., line_%%d.d) then each segment will be written\n");
	GMT_message (GMT, "\t   to separate files [Default is stdout]\n");
	GMT_message (GMT, "\t-S means Suppress points outside region\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t   It will also duplicate data output to GMT->session.std[GMT_ERR] for monitoring\n");
	GMT_message (GMT, "\t-Zv will prompt for z-value for each segment and output xyz triplets\n");
	GMT_message (GMT, "\t-Zk means append button key id as a final column\n");
	GMT_explain_options (GMT, "Dfho.");
	
	return (EXIT_FAILURE);
}
	
GMT_LONG GMT_gmtdigitize_parse (struct GMTAPI_CTRL *C, struct GMTDIGITIZE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtdigitize and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;


	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				break;
			case 'C':	/* Device to use */
				Ctrl->C.active = TRUE;
				Ctrl->C.device = strdup (opt->arg);
				break;
			case 'D':	/* Minimum distance between continuously digitized points */
				Ctrl->D.active = TRUE;
				Ctrl->D.limit = GMT_to_inch (GMT, opt->arg);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				break;
			case 'L':	/* Resolution of digitizer in lines per inch */
				Ctrl->L.active = TRUE;
				Ctrl->L.LPI = atoi (opt->arg);
				break;
			case 'Z':
				if (opt->arg[0] == 'k') Ctrl->Z.active[K_ID] = TRUE;
				if (opt->arg[0] == 'v') Ctrl->Z.active[V_ID] = TRUE;
				break;
			case 'N':	/* Multiple line segments */
				Ctrl->N.active = TRUE;
				Ctrl->N.name = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files, "Syntax error: No input files allowed\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_gmtdigitize_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_gmtdigitize (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, ok, multi_files, n_segments = 0;
	GMT_LONG i, j, n = 0, n_read = 0, unit = 0, n_expected_fields, digunit;
	GMT_LONG val_pos = 2, key_pos = 2, m_button, type, button, i_unused = 0;
	
	char line[GMT_BUFSIZ], format[GMT_BUFSIZ], unit_name[80], this_file[GMT_BUFSIZ];
	char *control[4] = {"first", "second", "third", "fourth"};
	char *corner[4] = {"lower left", "lower right", "upper right", "upper left"};
	char *xname[2] = {"longitude", "x-coordinate"};
	char *yname[2] = {"latitude ", "y-coordinate"}, *not_used = NULL;
	
	gid_t gid = 0;
	uid_t uid = 0;
	
	double mean_dig_y, z_val = 0.0, dist, mean_dig_x;
	double utm_correct, fwd_scale, inv_scale, xmap, ymap, X_DIG[4], Y_DIG[4], out[3];
	double last_xmap, last_ymap, x_in_min, x_in_max, y_in_min, y_in_max, inch_to_unit;
	double unit_to_inch, rotation, mean_map_x, mean_map_y, x_out_min, x_out_max, y_out_min;
	double y_out_max, rms, u_scale, LON[4], LAT[4], X_MAP[4], Y_MAP[4], XP[4], YP[4];
	
	FILE *fp = NULL;

	struct GMTDIGITIZE_INFO	C;
	struct GMTDIGITIZE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;	/* Linked list of options */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
        options = GMT_Prep_Options (API, mode, args);   /* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_gmtdigitize_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_gmtdigitize_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((error = GMT_Parse_Common (API, "-VJRbf:", "h>" GMT_OPT("HMm"), options))) Return (error);
	GMT = GMT_begin_module (API, "GMT_gmtdigitize", &GMT_cpy);				/* Save current state */
	Ctrl = New_gmtdigitize_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtdigitize_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtdigitize main code ----------------------------*/

	/* OK, time to connect to digitizer */

	if (!Ctrl->C.device) Ctrl->C.device = strdup (DIG_PORT);	/* Default setup */
	C.INV_LPI = 1.0 / Ctrl->L.LPI;		/* To save a divide later */
	
#ifdef DIGTEST
	digunit = 0;
#else
	if ((digunit = open (Ctrl->C.device, O_RDONLY | O_NOCTTY)) < 0) {
		GMT_message (GMT, "failed to open port %s\n", Ctrl->C.device);
		Return (EXIT_FAILURE);
	}
#endif
	tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
	
#ifdef SET_IO_MODE
	GMT_setmode (GMT, GMT_OUT);
#endif

	GMT_init_scales (GMT, unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	u_scale = inv_scale;

	GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");

	GMT_message (GMT, "\n==>   gmtdigitize version %s  Paul Wessel, SOEST, July 9, 2007   <==\n\n", GMT_VERSION);
	GMT_message (GMT, "================  HOW TO DIGITIZE ================\n\n");
	GMT_message (GMT, "To exit, click the %c button.\n", END_BUTTON_CHAR);
	GMT_message (GMT, "To initiate a new segment, click the %c button.\n", MULTISEG_BUTTON1_CHAR);
	GMT_message (GMT, "To initiate a new segment and typing in a comment, click the %c button.\n", MULTISEG_BUTTON2_CHAR);
	GMT_message (GMT, "To digitize a point, click on any other button.\n\n");

	/* Get 3 control points */
	
	type = (GMT_is_geographic (GMT, GMT_IN)) ? 0 : 1;
	if (Ctrl->Z.active[V_ID]) key_pos = 3;	/* z-values go in 3rd column (if chosen), key values go in the next (if chosen) */
	
	ok = FALSE;
	while (!ok) {
		double sum_dig_distance, sum_map_distance, new_x, new_y, dist;
		
		GMT_message (GMT, "-------------------------------------------------------------------|\n");
		GMT_message (GMT, "| Calibration of map scale, paper rotation, and mapping offsets.   |\n");
		GMT_message (GMT, "| We need 4 known points on the map to solve for these parameters. |\n");
		GMT_message (GMT, "-------------------------------------------------------------------|\n\n");
		ok = TRUE;
		tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
		
		if (!GMT->common.R.oblique && !Ctrl->F.active) {	/* -R is w/e/s/n and we dont want to override */
			GMT_message (GMT, "gmtdigitize will use the map corners as calibration points\n\n");
			LON[0] = LON[3] = GMT->common.R.wesn[XLO];
			LON[1] = LON[2] = GMT->common.R.wesn[XHI];
			LAT[0] = LAT[1] = GMT->common.R.wesn[YLO];
			LAT[2] = LAT[3] = GMT->common.R.wesn[YHI];
			for (i = 0; i < 4; i++) {
				GMT_message (GMT, "==> Please digitize the %s corner (%g/%g): ", corner[i], LON[i], LAT[i]);
				button = get_digitize_raw (GMT, digunit, &X_DIG[i], &Y_DIG[i], &C);
				GMT_message (GMT, "[x:%g y:%g]\n", X_DIG[i], Y_DIG[i]);
				GMT_geo_to_xy (GMT, LON[i], LAT[i], &X_MAP[i], &Y_MAP[i]);
			}
		}
		else {
			GMT_message (GMT, "gmtdigitize will request 4 arbitrary calibration points\n\n");
			for (i = 0; i < 4; i++) {
				GMT_message (GMT, "==> Please digitize the %s point: ", control[i]);
				button = get_digitize_raw (GMT, digunit, &X_DIG[i], &Y_DIG[i], &C);
				GMT_message (GMT, "[x:%g y:%g]\n", X_DIG[i], Y_DIG[i]);
			}
			GMT_message (GMT, "\n");
			for (i = 0; i < 4; i++) {
				GMT_message (GMT, "Please Enter %s of %s point: ", xname[type], control[i]);
				not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, GMT->session.std[GMT_IN]);
				GMT_chop (GMT, line);
				if (!(GMT_scanf (GMT, line, GMT->current.io.col_type[GMT_IN][GMT_X], &LON[i]))) {
					GMT_message (GMT, "Conversion error for %sx [%s]\n", xname[type], line);
					exit (EXIT_FAILURE);
				}
				GMT_message (GMT, "Please Enter %s of %s point: ", yname[type], control[i]);
				not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, GMT->session.std[GMT_IN]);
				GMT_chop (GMT, line);
				if (!(GMT_scanf (GMT, line, GMT->current.io.col_type[GMT_IN][GMT_Y], &LAT[i]))) {
					GMT_message (GMT, "Conversion error for %s [%s]\n", yname[type], line);
					exit (EXIT_FAILURE);
				}
				GMT_geo_to_xy (GMT, LON[i], LAT[i], &X_MAP[i], &Y_MAP[i]);
			}
		}
	
		/* First calculate map scale */

		if (GMT_is_geographic (GMT, GMT_IN)) {	/* x and y scale is the same */
			sum_dig_distance = sum_map_distance = 0.0;
			for (i = 0; i < 4; i++) for (j = i + 1; j < 4; j++) {	/* Calculate all interpoint distances */
				sum_dig_distance += hypot ((X_DIG[i] - X_DIG[j]), (Y_DIG[i] - Y_DIG[j]));
				sum_map_distance += hypot ((X_MAP[i] - X_MAP[j]), (Y_MAP[i] - Y_MAP[j]));
			}
			C.map_scale[GMT_X] = C.map_scale[GMT_Y] = sum_map_distance / sum_dig_distance;	/* Get average scale */
		}
		else {	/* Linear scale may differ in x/y */
			sum_dig_distance = hypot ((X_DIG[1] - X_DIG[0]), (Y_DIG[1] - Y_DIG[0])) + hypot ((X_DIG[2] - X_DIG[3]), (Y_DIG[2] - Y_DIG[3]));
			sum_map_distance = hypot ((X_MAP[1] - X_MAP[0]), (Y_MAP[1] - Y_MAP[0])) + hypot ((X_MAP[2] - X_MAP[3]), (Y_MAP[2] - Y_MAP[3]));
			C.map_scale[GMT_X] = sum_map_distance / sum_dig_distance;	/* Get average x scale based on ~horizontal line */
			sum_dig_distance = hypot ((X_DIG[3] - X_DIG[0]), (Y_DIG[3] - Y_DIG[0])) + hypot ((X_DIG[2] - X_DIG[1]), (Y_DIG[2] - Y_DIG[1]));
			sum_map_distance = hypot ((X_MAP[3] - X_MAP[0]), (Y_MAP[3] - Y_MAP[0])) + hypot ((X_MAP[2] - X_MAP[1]), (Y_MAP[2] - Y_MAP[1]));
			C.map_scale[GMT_Y] = sum_map_distance / sum_dig_distance;	/* Get average y scale based on ~vertical line */
		}
		/* Then undo the scale and find mean positions */

		mean_map_x = mean_map_y = mean_dig_x = mean_dig_y = 0.0;
		for (i = 0; i < 4; i++) {
			XP[i] = X_MAP[i] / C.map_scale[GMT_X];
			YP[i] = Y_MAP[i] / C.map_scale[GMT_Y];
			mean_map_x += XP[i];
			mean_map_y += YP[i];
			mean_dig_x += X_DIG[i];
			mean_dig_y += Y_DIG[i];
		}
		
		mean_map_x /= 4.0;	mean_map_y /= 4.0;
		mean_dig_x /= 4.0;	mean_dig_y /= 4.0;

		rotation = 0.0;
		for (i = 0; i < 4; i++) {	/* Take out means and find average rotation */
			XP[i]    -= mean_map_x;	YP[i]    -= mean_map_y;
			X_DIG[i] -= mean_dig_x;	Y_DIG[i] -= mean_dig_y;
			rotation += (atan2 (YP[i], XP[i]) - atan2 (Y_DIG[i], X_DIG[i]));
		}
		rotation /= 4.0;
		
		/* Find misfit RMS */
		
		sincos (rotation, &C.sin_theta, &C.cos_theta);
		for (i = 0, rms = 0.0; i < 4; i++) {
			new_x = X_DIG[i] * C.cos_theta - Y_DIG[i] * C.sin_theta;
			new_y = X_DIG[i] * C.sin_theta + Y_DIG[i] * C.cos_theta;
			dist = hypot (XP[i] - new_x, YP[i] - new_y);
			rms += dist * dist;
		}
		rms = sqrt (rms / 4.0);
		
		if (rms > TOLERANCE) {
			rms *= inch_to_unit;
			GMT_message (GMT, "\aError: Your four points project to give a r.m.s of %g %s\n", rms, unit_name);
			GMT_message (GMT, "Tolerance is set to %g %s.  Please redo the calibration\n", inch_to_unit * TOLERANCE, unit_name);
			ok = FALSE;
		}
	}
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "\nFound: rotation = %.3f, map_scale = %g/%g, rms = %g\n\n", rotation * R2D, C.map_scale[GMT_X], C.map_scale[GMT_Y], rms);
	
	C.map_x0 = (mean_map_x - mean_dig_x * C.cos_theta + mean_dig_y * C.sin_theta) * C.map_scale[GMT_X];
	C.map_y0 = (mean_map_y - mean_dig_x * C.sin_theta - mean_dig_y * C.cos_theta) * C.map_scale[GMT_Y];
	
	utm_correct = (GMT->current.proj.projection == GMT_UTM && !GMT->current.proj.north_pole) ? GMT_FALSE_NORTHING : 0.0;
	
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		double rect[4];
		sprintf (format, "%s/%s/%s/%s", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_memcpy (rect, GMT->current.proj.rect, 4, double);
	
		/* Set correct GMT min/max values in light of actual scale */
		for (i = 0; i < 4; i++) rect[i] /= C.map_scale[i/2];
	
		/* Convert inches to chosen MEASURE */
		for (i = 0; i < 4; i++) rect[i] *= inch_to_unit;
		GMT_message (GMT, " Transform ");
		GMT_message (GMT, format, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT_message (GMT, " <- ");
		GMT_message (GMT, format, rect[XLO], rect[XHI], rect[YLO], rect[YHI]);
		GMT_message (GMT, " [%s]\n", unit_name);
	}
		
	/* Now we are ready to take on some input values */
	
	n_expected_fields = 2 + Ctrl->Z.active[V_ID] + Ctrl->Z.active[K_ID];
	if (Ctrl->Z.active[V_ID])
		sprintf (format, "%s\t%s\t%s\t%%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	else
		sprintf (format, "%s\t%s\t%%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	
	x_in_min = y_in_min = x_out_min = y_out_min = DBL_MAX;
	x_in_max = y_in_max = x_out_max = y_out_max = -DBL_MAX;
	last_xmap = -DBL_MAX;
	last_ymap = -DBL_MAX;
	
	multi_files = (Ctrl->N.name && strchr (Ctrl->N.name, '%') != NULL);	/* Individual file names */

	if (!multi_files) {
		if ((fp = next_file (GMT, Ctrl->N.name, n_segments, this_file))) Return (EXIT_FAILURE);
		GMT_message (GMT, "If you want, you can enter some header records (comments) here.\n");
		GMT_message (GMT, "These records will be written out starting with a leading #.\n");
		GMT_message (GMT, "(Do not start with # - it will be prepended automatically.\n\n");
		do {
			GMT_message (GMT, "==> Please enter comment records, end with blank line: ");
			not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, stdin);
			GMT_chop (GMT, line);
			if (line[0] != '\0' && !GMT->common.b.active[GMT_OUT]) fprintf (fp, "# %s\n", line);
		} while (line[0] != '\0');
	}
	else {	/* Need real user's UID and GID since we need to chown */
#ifdef WIN32
		gid = uid = 0;
#else
		gid = getgid ();
		uid = getuid ();
#endif
	}
	
	GMT_message (GMT, "\n\nStart digitizing, end by clicking %c button\n\n", END_BUTTON_CHAR);
	if (GMT->current.io.multi_segments[GMT_OUT]) {
		GMT_message (GMT, "[Remember to initialize the first segment by clicking the %c or %c buttons!]\n",
		MULTISEG_BUTTON1_CHAR, MULTISEG_BUTTON2_CHAR);
	}
	
	tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
	while ((button = get_digitize_xy (GMT, digunit, &xmap, &ymap, &C)) != END_BUTTON) {	/* Not yet done */

		m_button = 0;
		while (button == MULTISEG_BUTTON1 || button == MULTISEG_BUTTON2) {
			m_button++;
			if (m_button == 1 && GMT->current.io.multi_segments[GMT_OUT]) {
				if (multi_files) {
					if (fp) {
						GMT_fclose (GMT, fp);
						i_unused = chown (this_file, uid, gid);
					}
					if ((fp = next_file (GMT, Ctrl->N.name, n_segments, this_file))) Return (EXIT_FAILURE);
				}

				if (Ctrl->Z.active[V_ID]) {
					GMT_message (GMT, "Enter z-value for next segment: ");
					not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, GMT->session.std[GMT_IN]);
					GMT_chop (GMT, line);
					z_val = atof (line);
				}
				if (button == MULTISEG_BUTTON1) {	/* Just write blank segment header */
					sprintf (GMT->current.io.segment_header, "%ld", n_segments);
				}
				else {	/* Ask for what to write out */
					GMT_message (GMT, "Enter segment header: ");
					not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, GMT->session.std[GMT_IN]);
					GMT_chop (GMT, line);
					sprintf (GMT->current.io.segment_header, "%ld %s", n_segments, line);
				}
				GMT_write_segmentheader (GMT, fp, n_expected_fields);
				GMT_report (GMT, GMT_MSG_NORMAL, "%c %s\n", GMT->current.setting.io_seg_marker[GMT_OUT], GMT->current.io.segment_header);
				last_xmap = -DBL_MAX;
				last_ymap = -DBL_MAX;
				n_segments++;
			}
			else if (m_button == 1)
				GMT_report (GMT, GMT_MSG_NORMAL, "Segment header buttons only active if -M is set (ignored)\n");
			else
				tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
				
			button = get_digitize_xy (GMT, digunit, &xmap, &ymap, &C);
		}

		dist = hypot (xmap - last_xmap, ymap - last_ymap);
		if (dist > Ctrl->D.limit) {
			if (Ctrl->A.active) fputc ('\a', GMT->session.std[GMT_ERR]);
			GMT_xy_to_geo (GMT, &out[GMT_X], &out[GMT_Y], xmap, ymap);
			n_read++;
			if (Ctrl->S.active && GMT_map_outside (GMT, out[GMT_X], out[GMT_Y])) continue;
			if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
				x_in_min = MIN (x_in_min, xmap);
				x_in_max = MAX (x_in_max, xmap);
				y_in_min = MIN (y_in_min, ymap);
				y_in_max = MAX (y_in_max, ymap);
				x_out_min = MIN (x_out_min, out[GMT_X]);
				x_out_max = MAX (x_out_max, out[GMT_X]);
				y_out_min = MIN (y_out_min, out[GMT_Y]);
				y_out_max = MAX (y_out_max, out[GMT_Y]);
			}
			last_xmap = xmap;
			last_ymap = ymap;

			if (Ctrl->Z.active[V_ID]) out[val_pos] = z_val;
			if (Ctrl->Z.active[K_ID]) out[key_pos] = (double)button;
			GMT->current.io.output (GMT, fp, n_expected_fields, out);
			if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
				(Ctrl->Z.active[V_ID]) ? GMT_message (GMT, format, out[GMT_X], out[GMT_Y], z_val, button) : GMT_message (GMT, format, out[GMT_X], out[GMT_Y], button);
			}
			n++;
		}
	}
	if (multi_files && fp) {
		GMT_fclose (GMT, fp);
		i_unused = chown (this_file, uid, gid);
	}
	
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL) && n_read > 0) {
		sprintf (format, "Input extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_message (GMT, format, x_in_min, x_in_max, y_in_min, y_in_max);
		sprintf (format, "Output extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_message (GMT, format, x_out_min, x_out_max, y_out_min, y_out_max);
		GMT_message (GMT, "Digitized %ld points\n", n);
		if (Ctrl->S.active && n != n_read) GMT_message (GMT, "%ld fell outside region\n", n_read - n);
	}

	Return (GMT_OK);
}

int main (int argc, char *argv[]) {

	int status = 0;			/* Status code from GMT API */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], GMTAPI_GMT)) == NULL) exit (EXIT_FAILURE);

	/* 2. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = (int)GMT_gmtdigitize (API, (GMT_LONG)(argc-1), (argv+1));

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (status);		/* Return the status from FUNC */
}
