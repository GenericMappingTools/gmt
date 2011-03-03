/*--------------------------------------------------------------------
 *    $Id: gmtdigitize.c,v 1.33 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
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
 *	Contact info: www.soest.hawaii.edu/gmt
 *--------------------------------------------------------------------*/
/*
 * gmtdigitize reads digitized points from a serial port connected to a
 * digitizer.  It is currently only been tested with a Calcomp DrawingBoard III
 * hooked up to a Linux box.  It is largely based on mapproject for the main
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

struct GMTDIGITIZE_CTRL {	/* Things needed while digitizing */
	double sin_theta, cos_theta, map_x0, map_y0, map_scale[2], INV_LPI;
};

int main (int argc, char **argv)
{
	GMT_LONG error = FALSE, suppress = FALSE, ok, output_key = FALSE, multi_files;
	GMT_LONG audible = FALSE, output_val = FALSE, force_4 = FALSE;
	
	char line[BUFSIZ], format[BUFSIZ], unit_name[80], this_file[BUFSIZ];
	char *control[4] = {"first", "second", "third", "fourth"};
	char *corner[4] = {"lower left", "lower right", "upper right", "upper left"};
	char *name = CNULL, *device = CNULL, *xname[2] = {"longitude", "x-coordinate"};
	char *yname[2] = {"latitude ", "y-coordinate"}, *not_used = NULL;
	
	GMT_LONG i, j, n = 0, n_read = 0, unit = 0, n_expected_fields, n_segments = 0, digunit;
	GMT_LONG val_pos = 2, key_pos = 2, m_button, LPI = DIG_LPI, type, button, i_unused = 0;
	
	gid_t gid = 0;
	uid_t uid = 0;
	
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0, out[3], mean_dig_x;
	double mean_dig_y, z_val = 0.0, d_limit = 0.0, dist, xmin, xmax, ymin, ymax;
	double utm_correct, fwd_scale, inv_scale, xmap, ymap, X_DIG[4], Y_DIG[4];
	double last_xmap, last_ymap, x_in_min, x_in_max, y_in_min, y_in_max, inch_to_unit;
	double unit_to_inch, rotation, mean_map_x, mean_map_y, x_out_min, x_out_max, y_out_min;
	double y_out_max, rms, u_scale, LON[4], LAT[4], X_MAP[4], Y_MAP[4], XP[4], YP[4];
	
	FILE *fp = NULL;

	struct GMTDIGITIZE_CTRL	C;
	
	GMT_LONG get_digitize_raw (GMT_LONG digunit, double *xdig, double *ydig, struct GMTDIGITIZE_CTRL *C);
	GMT_LONG get_digitize_xy (GMT_LONG digunit, double *xmap, double *ymap, struct GMTDIGITIZE_CTRL *C);
	FILE *next_file (char *name, int n_segments, char *this_file);

	argc = GMT_begin (argc, argv);
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
		
				/* Common parameters */
			
				case 'H':
				case 'J':
				case 'M':
				case 'R':
				case 'V':
				case 'b':
				case 'f':
				case 'm':
				case ':':
				case '\0':
					error += GMT_parse_common_options (argv[i], &west, &east, &south, &north);
					break;
				
				/* Supplemental parameters */
				
				case 'A':
					audible = TRUE;
					break;
				case 'C':               /* Device to use */
					device = &argv[i][2];
					break;
				case 'D':               /* Minimum distance between continuously digitized points */
					d_limit = GMT_convert_units (&argv[i][2], GMT_INCH);
					break;
				case 'F':
					force_4 = TRUE;
					break;
				case 'L':               /* Resolution of digitizer in lines per inch */
					LPI = atoi (&argv[i][2]);
					break;
				case 'Z':
					if (argv[i][2] == 'k') output_key = TRUE;
					if (argv[i][2] == 'v') output_val = TRUE;
					break;
				case 'N':               /* Multiple line segments */
					name = &argv[i][2];
					break;
				case 'S':
					suppress = TRUE;
					break;
				default:
					error = TRUE;
					GMT_default_error (argv[i][1]);
					break;
			}
		}
	}
	
	if (argc == 1 || GMT_give_synopsis_and_exit) {
		fprintf (stderr, "gmtdigitize %s - Digitizing and inverse map transformation of map x/y coordinates\n\n", GMT_VERSION);
		fprintf (stderr, "usage: gmtdigitize %s %s\n", GMT_J_OPT, GMT_Rgeo_OPT);
		fprintf (stderr, "\t[-A] [-C<device>] [-D<limit>] [-F] [-H] [-L<lpi>] [-N<namestem>] [-S]\n");
		fprintf (stderr, "\t[-V] [-Zk|v] [%s] [%s] [%s]\n\n", GMT_bo_OPT, GMT_f_OPT, GMT_mo_OPT);
		
		if (GMT_give_synopsis_and_exit) exit (EXIT_FAILURE);
		
		GMT_explain_option ('j');
		fprintf (stderr, "\tScale or width is arbitrary as %s solves for it based on calibration points\n", GMT_program);
		GMT_explain_option ('R');
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr, "\t-A gives an audible alert each time a point is clicked [Default is silent]\n");
		fprintf (stderr, "\t-C Device (port) to use [Default is %s]\n", DIG_PORT);
		fprintf (stderr, "\t-D Only pass point when it is more than <limit> %s from previous point [0]\n", GMT_unit_names[gmtdefs.measure_unit]);
		fprintf (stderr, "\t   Append c, i, m, or p for cm, inch, meter, or points [Default is %s]\n", GMT_unit_names[gmtdefs.measure_unit]);
		fprintf (stderr, "\t-F Force program to accept 4 arbitrary calibration points [Default will use\n");
		fprintf (stderr, "\t   the corners of the map if they are known to the program]\n");
		GMT_explain_option ('H');
		fprintf (stderr, "\t-L sets the lines-pr-inch resolution of the digitizer [%d]\n", DIG_LPI);
		fprintf (stderr, "\t-N sets name for output file(s).  If name contains a C-format\n");
		fprintf (stderr, "\t   for integer (e.g., line_%%d.d) then each segment will be written\n");
		fprintf (stderr, "\t   to separate files [Default is stdout]\n");
		fprintf (stderr, "\t-S means Suppress points outside region\n");
		GMT_explain_option ('V');
		fprintf (stderr, "\t   It will also duplicate data output to stderr for monitoring\n");
		fprintf (stderr, "\t-Zv will prompt for z-value for each segment and output xyz triplets\n");
		fprintf (stderr, "\t-Zk means append button key id as a final column\n");
		GMT_explain_option ('o');
		GMT_explain_option ('f');
		GMT_explain_option ('m');
		GMT_explain_option ('.');
		exit (EXIT_FAILURE);
	}
	
	if (!project_info.region_supplied) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR:  Must specify -R option\n", GMT_program);
		error++;
	}

	if (error) exit (EXIT_FAILURE);

	/* OK, time to connect to digitizer */

	if (!device) device = DIG_PORT;	/* Default setup */
	C.INV_LPI = 1.0 / LPI;		/* To save a divide later */
	
#ifdef DIGTEST
	digunit = 0;
#else
	if ((digunit = open (device, O_RDONLY | O_NOCTTY)) < 0) {
		fprintf (stderr, "%s: failed to open port %s\n", GMT_program, device);
		exit (EXIT_FAILURE);
	}
#endif
	tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
	
#ifdef SET_IO_MODE
	GMT_setmode (1);
#endif

	GMT_init_scales (unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	u_scale = inv_scale;

	GMT_err_fail (GMT_map_setup (west, east, south, north), "");

	fprintf (stderr, "\n==>   %s version %s  Paul Wessel, SOEST, July 9, 2007   <==\n\n", GMT_program, GMT_VERSION);
	fprintf (stderr, "================  HOW TO DIGITIZE ================\n\n");
	fprintf (stderr, "To exit, click the %c button.\n", END_BUTTON_CHAR);
	if (GMT_io.multi_segments[GMT_OUT]) {
		fprintf (stderr, "To initiate a new segment, click the %c button.\n", MULTISEG_BUTTON1_CHAR);
		fprintf (stderr, "To initiate a new segment and typing in a comment, click the %c button.\n", MULTISEG_BUTTON2_CHAR);
	}
	fprintf (stderr, "To digitize a point, click on any other button.\n\n");

	/* Get 3 control points */
	
	type = (GMT_IS_MAPPING) ? 0 : 1;
	if (output_val) key_pos = 3;	/* z-values go in 3rd column (if chosen), key values go in the next (if chosen) */
	
	
	ok = FALSE;
	while (!ok) {
		double sum_dig_distance, sum_map_distance, new_x, new_y, dist;
		
		fprintf (stderr, "-------------------------------------------------------------------|\n");
		fprintf (stderr, "| Calibration of map scale, paper rotation, and mapping offsets.   |\n");
		fprintf (stderr, "| We need 4 known points on the map to solve for these parameters. |\n");
		fprintf (stderr, "-------------------------------------------------------------------|\n\n");
		ok = TRUE;
		tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
		
		if (project_info.region && !force_4) {	/* -R is w/e/s/n and we dont want to override */
			fprintf (stderr, "%s will use the map corners as calibration points\n\n", GMT_program);
			LON[0] = LON[3] = project_info.w;
			LON[1] = LON[2] = project_info.e;
			LAT[0] = LAT[1] = project_info.s;
			LAT[2] = LAT[3] = project_info.n;
			for (i = 0; i < 4; i++) {
				fprintf (stderr, "==> Please digitize the %s corner (%g/%g): ", corner[i], LON[i], LAT[i]);
				button = get_digitize_raw (digunit, &X_DIG[i], &Y_DIG[i], &C);
				fprintf (stderr, "[x:%g y:%g]\n", X_DIG[i], Y_DIG[i]);
				GMT_geo_to_xy (LON[i], LAT[i], &X_MAP[i], &Y_MAP[i]);
			}
		}
		else {
			fprintf (stderr, "%s will request 4 arbitrary calibration points\n\n", GMT_program);
			for (i = 0; i < 4; i++) {
				fprintf (stderr, "==> Please digitize the %s point: ", control[i]);
				button = get_digitize_raw (digunit, &X_DIG[i], &Y_DIG[i], &C);
				fprintf (stderr, "[x:%g y:%g]\n", X_DIG[i], Y_DIG[i]);
			}
			fprintf (stderr, "\n");
			for (i = 0; i < 4; i++) {
				fprintf (stderr, "Please Enter %s of %s point: ", xname[type], control[i]);
				not_used = GMT_fgets (line, BUFSIZ, GMT_stdin);
				GMT_chop (line);
				if (!(GMT_scanf (line, GMT_io.in_col_type[0], &LON[i]))) {
					fprintf (stderr, "%s: Conversion error for %sx [%s]\n", GMT_program, xname[type], line);
					exit (EXIT_FAILURE);
				}
				fprintf (stderr, "Please Enter %s of %s point: ", yname[type], control[i]);
				not_used = GMT_fgets (line, BUFSIZ, GMT_stdin);
				GMT_chop (line);
				if (!(GMT_scanf (line, GMT_io.in_col_type[1], &LAT[i]))) {
					fprintf (stderr, "%s: Conversion error for %s [%s]\n", GMT_program, yname[type], line);
					exit (EXIT_FAILURE);
				}
				GMT_geo_to_xy (LON[i], LAT[i], &X_MAP[i], &Y_MAP[i]);
			}
		}
	
		/* First calculate map scale */

		if (GMT_IS_MAPPING) {	/* x and y scale is the same */
			sum_dig_distance = sum_map_distance = 0.0;
			for (i = 0; i < 4; i++) for (j = i + 1; j < 4; j++) {	/* Calculate all interpoint distances */
				sum_dig_distance += hypot ((X_DIG[i] - X_DIG[j]), (Y_DIG[i] - Y_DIG[j]));
				sum_map_distance += hypot ((X_MAP[i] - X_MAP[j]), (Y_MAP[i] - Y_MAP[j]));
			}
			C.map_scale[0] = C.map_scale[1] = sum_map_distance / sum_dig_distance;	/* Get average scale */
		}
		else {	/* Linear scale may differ in x/y */
			sum_dig_distance = hypot ((X_DIG[1] - X_DIG[0]), (Y_DIG[1] - Y_DIG[0])) + hypot ((X_DIG[2] - X_DIG[3]), (Y_DIG[2] - Y_DIG[3]));
			sum_map_distance = hypot ((X_MAP[1] - X_MAP[0]), (Y_MAP[1] - Y_MAP[0])) + hypot ((X_MAP[2] - X_MAP[3]), (Y_MAP[2] - Y_MAP[3]));
			C.map_scale[0] = sum_map_distance / sum_dig_distance;	/* Get average x scale based on ~horizontal line */
			sum_dig_distance = hypot ((X_DIG[3] - X_DIG[0]), (Y_DIG[3] - Y_DIG[0])) + hypot ((X_DIG[2] - X_DIG[1]), (Y_DIG[2] - Y_DIG[1]));
			sum_map_distance = hypot ((X_MAP[3] - X_MAP[0]), (Y_MAP[3] - Y_MAP[0])) + hypot ((X_MAP[2] - X_MAP[1]), (Y_MAP[2] - Y_MAP[1]));
			C.map_scale[1] = sum_map_distance / sum_dig_distance;	/* Get average y scale based on ~vertical line */
		}
		/* Then undo the scale and find mean positions */

		mean_map_x = mean_map_y = mean_dig_x = mean_dig_y = 0.0;
		for (i = 0; i < 4; i++) {
			XP[i] = X_MAP[i] / C.map_scale[0];
			YP[i] = Y_MAP[i] / C.map_scale[1];
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
			fprintf (stderr, "\aError: Your four points project to give a r.m.s of %g %s\n", rms, unit_name);
			fprintf (stderr, "Tolerance is set to %g %s.  Please redo the calibration\n", inch_to_unit * TOLERANCE, unit_name);
			ok = FALSE;
		}
	}
	if (gmtdefs.verbose) fprintf (stderr, "\nFound: rotation = %.3f, map_scale = %g/%g, rms = %g\n\n", rotation * R2D, C.map_scale[0], C.map_scale[1], rms);
	
	C.map_x0 = (mean_map_x - mean_dig_x * C.cos_theta + mean_dig_y * C.sin_theta) * C.map_scale[0];
	C.map_y0 = (mean_map_y - mean_dig_x * C.sin_theta - mean_dig_y * C.cos_theta) * C.map_scale[1];
	
	utm_correct = (project_info.projection == GMT_UTM && !project_info.north_pole) ? GMT_FALSE_NORTHING : 0.0;
	
	if (gmtdefs.verbose) {
		sprintf (format, "%s/%s/%s/%s", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
		xmin = project_info.xmin;
		xmax = project_info.xmax;
		ymin = project_info.ymin;
		ymax = project_info.ymax;
	
		/* Set correct GMT min/max values in light of actual scale */
	
		xmin /= C.map_scale[0];	xmax /= C.map_scale[0];
		ymin /= C.map_scale[1];	ymax /= C.map_scale[1];
	
		/* Convert inches to chosen MEASURE */
		xmin *= inch_to_unit;
		xmax *= inch_to_unit;
		ymin *= inch_to_unit;
		ymax *= inch_to_unit;
		fprintf (stderr, "%s:  Transform ", GMT_program);
		fprintf (stderr, format, project_info.w, project_info.e, project_info.s, project_info.n);
		fprintf (stderr, " <- ");
		fprintf (stderr, format, xmin, xmax, ymin, ymax);
		fprintf (stderr, " [%s]\n", unit_name);
	}
		
	/* Now we are ready to take on some input values */
	
	n_expected_fields = 2 + output_val + output_key;
	if (output_val)
		sprintf (format, "%s\t%s\t%s\t%%ld\n", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
	else
		sprintf (format, "%s\t%s\t%%ld\n", gmtdefs.d_format, gmtdefs.d_format);
	
	x_in_min = y_in_min = x_out_min = y_out_min = DBL_MAX;
	x_in_max = y_in_max = x_out_max = y_out_max = -DBL_MAX;
	last_xmap = -DBL_MAX;
	last_ymap = -DBL_MAX;
	
	multi_files = (name && strchr (name, '%') != NULL);	/* Individual file names */

	if (!multi_files) {
		fp = next_file (name, n_segments, this_file);
		fprintf (stderr, "If you want, you can enter some header records (comments) here.\n");
		fprintf (stderr, "These records will be written out starting with a leading #.\n");
		fprintf (stderr, "(Do not start with # - it will be prepended automatically.\n\n");
		do {
			fprintf (stderr, "==> Please enter comment records, end with blank line: ");
			not_used = GMT_fgets (line, BUFSIZ, stdin);
			GMT_chop (line);
			if (line[0] != '\0' && !GMT_io.binary[1]) fprintf (fp, "# %s\n", line);
		} while (line[0] != '\0');
	}
	else {	/* Need real user's UID and GID since we need to chown */
		gid = getgid();
		uid = getuid();
	}
	
	fprintf (stderr, "\n\nStart digitizing, end by clicking %c button\n\n", END_BUTTON_CHAR);
	if (GMT_io.multi_segments[GMT_OUT]) {
		fprintf (stderr, "[Remember to initialize the first segment by clicking the %c or %c buttons!]\n",
		MULTISEG_BUTTON1_CHAR, MULTISEG_BUTTON2_CHAR);
	}
	
	tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
	while ((button = get_digitize_xy (digunit, &xmap, &ymap, &C)) != END_BUTTON) {	/* Not yet done */

		m_button = 0;
		while (button == MULTISEG_BUTTON1 || button == MULTISEG_BUTTON2) {
			m_button++;
			if (m_button == 1 && GMT_io.multi_segments[GMT_OUT]) {
				if (multi_files) {
					if (fp) {
						GMT_fclose (fp);
						i_unused = chown (this_file, uid, gid);
					}
					fp = next_file (name, n_segments, this_file);
				}

				if (output_val) {
					fprintf (stderr, "Enter z-value for next segment: ");
					not_used = GMT_fgets (line, BUFSIZ, GMT_stdin);
					GMT_chop (line);
					z_val = atof (line);
				}
				if (button == MULTISEG_BUTTON1) {	/* Just write blank multisegment header */
					sprintf (GMT_io.segment_header, "%c %ld\n", GMT_io.EOF_flag[GMT_OUT], n_segments);
				}
				else {	/* Ask for what to write out */
					fprintf (stderr, "Enter segment header: ");
					not_used = GMT_fgets (line, BUFSIZ, GMT_stdin);
					GMT_chop (line);
					sprintf (GMT_io.segment_header, "%c %ld %s\n", GMT_io.EOF_flag[GMT_OUT], n_segments, line);
				}
				GMT_write_segmentheader (fp, n_expected_fields);
				if (gmtdefs.verbose) fprintf (stderr, "%s", GMT_io.segment_header);
				last_xmap = -DBL_MAX;
				last_ymap = -DBL_MAX;
				n_segments++;
			}
			else if (m_button == 1)
				fprintf (stderr, "%s: Segment header buttons only active if -M is set (ignored)\n", GMT_program);
			else
				tcflush (digunit, TCIFLUSH);	/* Clean the muzzle */
				
			button = get_digitize_xy (digunit, &xmap, &ymap, &C);
		}

		dist = hypot (xmap - last_xmap, ymap - last_ymap);
		if (dist > d_limit) {
			if (audible) fputc ('\a', stderr);
			GMT_xy_to_geo (&out[0], &out[1], xmap, ymap);
			n_read++;
			if (suppress && GMT_map_outside (out[0], out[1])) continue;
			if (gmtdefs.verbose) {
				x_in_min = MIN (x_in_min, xmap);
				x_in_max = MAX (x_in_max, xmap);
				y_in_min = MIN (y_in_min, ymap);
				y_in_max = MAX (y_in_max, ymap);
				x_out_min = MIN (x_out_min, out[0]);
				x_out_max = MAX (x_out_max, out[0]);
				y_out_min = MIN (y_out_min, out[1]);
				y_out_max = MAX (y_out_max, out[1]);
			}
			last_xmap = xmap;
			last_ymap = ymap;

			if (output_val) out[val_pos] = z_val;
			if (output_key) out[key_pos] = (double)button;
			GMT_output (fp, n_expected_fields, out);
			if (gmtdefs.verbose) {
				(output_val) ? fprintf (stderr, format, out[0], out[1], z_val, button) : fprintf (stderr, format, out[0], out[1], button);
			}
			n++;
		}
	}
	if (multi_files && fp) {
		GMT_fclose (fp);
		i_unused = chown (this_file, uid, gid);
	}
	
	if (gmtdefs.verbose && n_read > 0) {
		sprintf (format, "%%s: Input extreme values:  Xmin: %s Xmax: %s Ymin: %s Ymax %s\n", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
		fprintf (stderr, format, GMT_program, x_in_min, x_in_max, y_in_min, y_in_max);
		sprintf (format, "%%s: Output extreme values:  Xmin: %s Xmax: %s Ymin: %s Ymax %s\n", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
		fprintf (stderr, format, GMT_program, x_out_min, x_out_max, y_out_min, y_out_max);
		fprintf (stderr, "%s: Digitized %ld points\n", GMT_program, n);
		if (suppress && n != n_read) fprintf (stderr, "%s: %ld fell outside region\n", GMT_program, n_read - n);
	}

	GMT_end (argc, argv);
	
	exit (EXIT_SUCCESS);
}

GMT_LONG get_digitize_raw (GMT_LONG digunit, double *xdig, double *ydig, struct GMTDIGITIZE_CTRL *C) {
	GMT_LONG i, n, ix, iy;
	char buffer[256], button;
	
	n = read (digunit, buffer, (size_t)255);
	if (n <= 0) return (END_BUTTON);
	n--;
	buffer[n] = 0;
#ifdef DEBUG
	fprintf (stderr, "Got %ld bytes [%s]\n", n, buffer);
#endif
	for (i = 0; i < n; i++) if (buffer[i] == ',') buffer[i] = ' ';
	sscanf (buffer, "%" GMT_LL "d %" GMT_LL "d %c", &ix, &iy, &button);
	
	*xdig = (double)(ix) * C->INV_LPI;	/* Convert from lines per inch to inches */
	*ydig = (double)(iy) * C->INV_LPI;
	
	return ((GMT_LONG)(button - '0'));
}

GMT_LONG get_digitize_xy (GMT_LONG digunit, double *xmap, double *ymap, struct GMTDIGITIZE_CTRL *C) {
	GMT_LONG button;
	double x_raw, y_raw;
	
	button = get_digitize_raw (digunit, &x_raw, &y_raw, C);
	if (button == END_BUTTON) return (END_BUTTON);

	/* Undo rotation and scaling to give map inches */
	
	*xmap = (x_raw * C->cos_theta - y_raw * C->sin_theta) * C->map_scale[0] + C->map_x0;
	*ymap = (x_raw * C->sin_theta + y_raw * C->cos_theta) * C->map_scale[1] + C->map_y0;
	
	return (button);
}

FILE *next_file (char *name, int n_segments, char *this_file) {
	FILE *fp;
	
	if (name) {
		if (strchr (name, '%') != NULL) {	/* Individual file names */
			sprintf (this_file, name, n_segments);
		}
		else {
			strncpy (this_file, name, (size_t)BUFSIZ);
		}
		if ((fp = GMT_fopen (this_file, "w")) == NULL) {
			fprintf (stderr, "%s: Could not create file %s\n", GMT_program, this_file);
			exit (EXIT_FAILURE);
		}
	}
	else {
		strcpy (this_file, "<stdout>");
		fp = GMT_stdout;
	}
		
	return (fp);
}
