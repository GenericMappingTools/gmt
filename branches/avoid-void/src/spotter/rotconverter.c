/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2011 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/*
 * Program for converting between total reconstruction and stage poles or to add rotations.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	24-OCT-2001
 * Version:	1.0
 *		31-MAR-2006: Changed -H to -C to avoid clash with GMT -H
 *
 *-------------------------------------------------------------------------
 * An ASCII stage pole (Euler) file must have following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of stage pole records which each have the format:
 *    lon(deg)  lat(deg)  tstart(Ma)  tstop(Ma)  ccw-angle(deg)
 * 3. stage records must go from oldest to youngest rotation
 * 4. Note tstart is larger (older) that tstop for each record
 * 5. No gaps allowed: tstart must equal the previous records tstop
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * # lon  lat	tstart	tend	ccw-angle
 * 165     85	150	100	24.0
 * 284     36	100	74	15.0
 * 265     22	74	65	7.5
 * 253     17	65	42	14.0
 * 285     68	42	0	34.0
 *
 * AN ASCII total reconstruction file must have the following format:
*
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of finite pole records which each have the format:
 *    lon(deg)  lat(deg)  tstop(Ma)  ccw-angle(deg)
 * 3. total reconstruction rotations must go from youngest to oldest
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * #longitude	latitude	time(My)	angle(deg)
 * 285.00000	 68.00000	 42.0000	 34.0000
 * 275.66205	 53.05082	 65.0000	 43.5361
 * 276.02501	 48.34232	 74.0000	 50.0405
 * 279.86436	 46.30610	100.0000	 64.7066
 * 265.37800	 55.69932	150.0000	 82.9957
 */

#include "spotter.h"

struct ROTCONVERTER_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct E {	/* -E[<value>] */
		GMT_LONG active;
		double value;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
		GMT_LONG mode;	/* out mode (TRUE if total reconstruction rotations) */
	} F;
	struct G {	/* -G */
		GMT_LONG active;
	} G;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
};

void *New_rotconverter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct ROTCONVERTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct ROTCONVERTER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->E.value = 0.5;	/* To get half-angles */
	C->F.mode = TRUE;	/* Default format is total reconstruction rotation poles for both input and output */
	return (C);
}

void Free_rotconverter_Ctrl (struct GMT_CTRL *GMT, struct ROTCONVERTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

GMT_LONG GMT_rotconverter_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "rotconverter %s - Manipulate total reconstruction and stage rotations\n\n", SPOTTER_VERSION);
	GMT_message (GMT, "usage: rotconverter [+][-] <rotA> [[+][-] <rotB>] [[+][-] <rotC>] ... [-A] [-D] [-E[<factor>]] [-F<out>]\n");
	GMT_message (GMT, "\t[-G] [-N] [-S] [-T] [%s] [%s] > outfile\n\n", GMT_V_OPT, GMT_h_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<rotA>, <rotB>, etc. are total reconstruction or stage rotation pole files.\n");
	GMT_message (GMT, "\t   Alternatively, they can be a single rotation in lon/lat[/tstart[/tstop]]/angle format.\n");
	GMT_message (GMT, "\t   All rotation poles are assumed to be in geocentric coordinates.\n");
	GMT_message (GMT, "\t   Rotations will be added/subtracted in the order given.\n");
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-A Report angles as time [Default uses time].\n");
	GMT_message (GMT, "\t-D Report all longitudes in -180/+180 range [Default is 0-360].\n");
	GMT_message (GMT, "\t-E Reduce opening angles for stage rotations by <factor> [0.5].\n");
	GMT_message (GMT, "\t   Typically used to get half-rates needed for flowlines.\n");
	GMT_message (GMT, "\t-F Set output file type: t for total reconstruction and s for stage rotations [Default is -Ft].\n");
	GMT_message (GMT, "\t-G Write rotations using GPlates format [Default is spotter format].\n");
	GMT_message (GMT, "\t-N Ensure all poles are in northern hemisphere [ Default ensures positive opening angles/rates].\n");
	GMT_message (GMT, "\t-S Ensure all poles are in southern hemisphere [ Default ensures positive opening angles/rates].\n");
	GMT_message (GMT, "\t-T Transpose the result (i.e., change sign of final rotation angle).\n");
	GMT_explain_options (GMT, "Vh.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_rotconverter_parse (struct GMTAPI_CTRL *C, struct ROTCONVERTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to rotconverter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Supplemental parameters */
			
			case 'A':	/* Angle, not time */
				Ctrl->A.active = TRUE;
				break;
			case 'D':	/* Dateline */
				Ctrl->D.active = TRUE;
				break;

			case 'E':	/* Convert to total reconstruction rotation poles instead */
				Ctrl->E.active = TRUE;
				if (opt->arg[0]) Ctrl->E.value = atof (opt->arg);
				break;

			case 'F':
				Ctrl->F.active = TRUE;
				if (strlen (opt->arg) != 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Must specify -F<out>\n");
					n_errors++;
					continue;
				}
				switch (opt->arg[0]) {	/* Output format */
#ifdef GMT_COMPAT
					case 'f':
						GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -Ff is deprecated; use -Ft instead.\n");
#endif
					case 't':
						Ctrl->F.mode = TRUE;
						break;
					case 's':
						Ctrl->F.mode = FALSE;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Error: Must specify t|s\n");
						n_errors++;
						break;
				}
				break;

			case 'G':	/* GPlates output format */
				Ctrl->G.active = TRUE;
				break;

			case 'N':	/* Ensure all poles reported are in northern hemisphere */
				Ctrl->N.active = TRUE;
				break;

			case 'S':	/* Ensure all poles reported are in southern hemisphere */
				Ctrl->S.active = TRUE;
				break;

			case 'T':	/* Transpose the final result (i.e., change sign of rotation) */
				Ctrl->T.active = TRUE;
				break;

			case '0': case '1': case '2': case '3': case '4': case '5': case '6':
				case '7': case '8': case '9': case '.':
				break;	/* Probably a rotation lon/lat/angle with negative longitude */
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active, "Syntax error: Cannot specify both -N and -S!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->F.mode, "Syntax error: -E requires stage rotations on output\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->F.mode, "Syntax error: -G requires total reconstruction rotations on output\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_rotconverter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_rotconverter (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	struct EULER *p = NULL;			/* Pointer to array of stage poles */
	struct EULER *a = NULL, *b = NULL;	/* Pointer to arrays of stage poles */

	GMT_LONG i, j, n_p, n_a = 1, n_b, n, k;	/* Misc. counters */
	GMT_LONG last_sign, n_slash, n_out = 0, n_opt = 0, confusion = FALSE, online_stage = FALSE;
	GMT_LONG error = FALSE;		/* Set to TRUE if arguments are inconsistent */
	GMT_LONG first = TRUE;		/* TRUE for first input file */
	GMT_LONG online_rot = FALSE;	/* TRUE if we gave a rotation on the commandline rather than file name */
	GMT_LONG no_time = FALSE;	/* TRUE if we gave a rotation on the commandline as lon/lat/angle only */

	double zero = 0.0;		/* Needed to pass to spotter_init */
	double lon = 0.0, lat = 0.0;	/* Pole location for online rotations */
	double t0 = 0.0, t1 = 0.0;	/* Start, stop times for online rotations */
	double angle = 0.0;		/* Rotation angle for online rotations */
	double out[20];

	char *start_text[2] = {"tstart(My)", "astart(deg)"};	/* Misc. column titles for rates or angles */
	char *end_text[2] = {"tend(My)", "aend(deg)"};
	char *time_text[2] = {"ttime(My)", "tangle(deg)"};
	char record[GMT_BUFSIZ];

	struct GMT_OPTION *ptr = NULL, *opt = NULL;
	struct ROTCONVERTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	/* Special preprocessing since online rotations like -144/34/-9 and -.55/33/2 will
	 * have been decoded as options -4 and option -., respectively.  Here we simply
	 * undo these and make them all "file" options -<.
	 * Also, a single - sign would have been decoded as the synopsis option.  */
	
	for (opt = options; opt; opt = opt->next, n_opt++) {
		switch (opt->option) {
			case '0': case '1': case '2': case '3': case '4': case '5': 
			case '6': case '7': case '8': case '9': case '.':
				sprintf (record, "-%c%s", opt->option, opt->arg);
				free (opt->arg);
				opt->arg = strdup (record);
				opt->option = GMTAPI_OPT_INFILE;
				break;
			case GMTAPI_OPT_SYNOPSIS:
				opt->arg = strdup ("-");
				opt->option = GMTAPI_OPT_INFILE;
				confusion = TRUE;	/* Since we don't know if just a single - was given */
				break;
			default:	/* Do nothing */
				break;
		}
	}
	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_rotconverter_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (n_opt == 1 && confusion) return (GMT_rotconverter_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_rotconverter", &GMT_cpy);				/* Save current state */
	if (GMT_Parse_Common (API, "-Vf:", "h>", options)) Return (API->error);
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = New_rotconverter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_rotconverter_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the rotconverter main code ----------------------------*/

	if (Ctrl->G.active) {
		GMT->current.io.col_type[GMT_OUT][0] = GMT->current.io.col_type[GMT_OUT][1] = GMT_IS_FLOAT;
		GMT->current.io.col_type[GMT_OUT][2] = GMT_IS_LAT;
		GMT->current.io.col_type[GMT_OUT][3] = GMT_IS_LON;
		strcpy (GMT->current.setting.format_float_out, "%g");
	}
	
	last_sign = +1;
	for (opt = options; opt; opt = opt->next) {
		if (opt->option != GMTAPI_OPT_INFILE) continue;	/* Only consider files (or rotations) from here on */

		/* Here a single + would have been parsed as a file with name "+"; change to a plus sign */
		if (opt->option == GMTAPI_OPT_INFILE && opt->arg[0] == '+' && opt->arg[1] == '\0') {
			last_sign = +1;
			continue;
		}
		/* Here a single - would have been parsed as a file with name "-"; change to a minus sign */
		if (opt->option == GMTAPI_OPT_INFILE && opt->arg[0] == '-' && opt->arg[1] == '\0') {
			last_sign = -1;
			continue;
		}

		if (spotter_GPlates_pair (opt->arg)) {	/* A GPlates plate pair to look up in the rotation table */
			online_rot = FALSE;
		}
		else if (GMT_access (GMT, opt->arg, R_OK)) {	/* Not a readable file, is it a lon/lat/t0[/t1]/omega specification? */
			for (j = n_slash = 0; opt->arg[j]; j++) if (opt->arg[j] == '/') n_slash++;
			if (n_slash < 2 || n_slash > 4) {	/* No way it can be a online rotation, cry foul */
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot read file %s\n", opt->arg);
				Return (EXIT_FAILURE);
			}
			else {	/* Try to decode as a single rotation */

				j = sscanf (opt->arg, "%lf/%lf/%lf/%lf/%lf", &lon, &lat, &t0, &t1, &angle);
				if (j == 4) angle = t1, t1 = 0.0;			/* Only 4 input values */
				if (n_slash == 2) angle = t0, t0 = 1.0, t1 = 0.0, no_time = TRUE;	/* Quick lon/lat/angle total reconstruction rotation, no time */
				if (t0 < t1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Online rotation has t_start (%g) younger than t_stop (%g)\n", t0, t1);
					Return (EXIT_FAILURE);
				}
				if (angle == 0.0) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Online rotation has zero opening angle\n");
					Return (EXIT_FAILURE);
				}
				online_rot = TRUE;
				online_stage = (j == 5);
			}
		}
		else
			online_rot = FALSE;

		if (first) {	/* First time loading a rotation model */
			if (online_rot) {
				n_a = 1;
				a = GMT_memory (GMT, NULL, 1, struct EULER);
				a[0].lon = lon;	a[0].lat = lat;
				a[0].t_start = t0;	a[0].t_stop = t1;
				a[0].duration = t0 - t1;
				a[0].omega = angle / a[0].duration;
				if (online_stage) spotter_stages_to_total (GMT, a, n_a, TRUE, TRUE);
			}
			else
				n_a = spotter_init (GMT, opt->arg, &a, FALSE, TRUE, FALSE, &zero);	/* Return total reconstruction rotations */
			zero = 0.0;
			if (last_sign == -1) {	/* Leading - sign, simply reverse the rotation angles */
				for (j = 0; j < n_a; j++) {
					a[j].omega = -a[j].omega;
					spotter_cov_of_inverse (GMT, &a[j], a[j].C);
				}
				last_sign = 1;
			}
			first = FALSE;
		}
		else {			/* For additional times, load a second model and add/subtract them */
			if (online_rot) {
				n_b = 1;
				b = GMT_memory (GMT, NULL, 1, struct EULER);
				b[0].lon = lon;	b[0].lat = lat;
				b[0].t_start = t0;	b[0].t_stop = t1;
				b[0].duration = t0 - t1;
				b[0].omega = angle / b[0].duration;
				if (online_stage) spotter_stages_to_total (GMT, b, n_b, TRUE, TRUE);
			}
			else
				n_b = spotter_init (GMT, opt->arg, &b, FALSE, TRUE, FALSE, &zero);	/* Return total reconstruction rotations */
			zero = 0.0;
			spotter_add_rotations (GMT, a, n_a, b, last_sign * n_b, &p, &n_p);			/* Add the two total reconstruction rotations sets, returns total reconstruction rotations in p */
			GMT_free (GMT, a);
			GMT_free (GMT, b);
			a = p;
			n_a = n_p;
		}
	}

	n_out = 3 + ((Ctrl->F.mode) ? 1 - no_time : 2);
	if (online_rot && n_out > 3) n_out--;
	if (a[0].has_cov) n_out += 9;
	if (Ctrl->G.active) n_out = 6;
	
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return (error);
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options)) Return (API->error);	/* Establishes data output */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC)) Return (API->error);				/* Enables data output and sets access mode */

	if (Ctrl->G.active)		/* GPlates header */
		sprintf (record, "#plateid\ttime\tlatitude\tlongitude\tangle\tfixedplateid\n");
	else if (Ctrl->F.mode && no_time)
		sprintf (record, "#longitude\tlatitude\tangle(deg)\n");
	else if (Ctrl->F.mode)	/* Easy, simply output what we've got following a header*/
		sprintf (record, "#longitude\tlatitude\t%s\tangle(deg)\n", time_text[Ctrl->A.active]);
	else if (Ctrl->F.mode)		/* Easy, simply output what we've got without a header */
		i = 0;	/* Do nothing here really */
	else {	/* Convert total reconstruction to stages before output */
		spotter_total_to_stages (GMT, a, n_a, TRUE, TRUE);				/* To ensure we have the right kind of poles for output */
		printf (record, "#longitude\tlatitude\t%s\t%s\tangle(deg)\n", start_text[Ctrl->A.active], end_text[Ctrl->A.active]);
	}
	if (GMT->current.io.io_header[GMT_OUT]) GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	
	for (i = 0; i < n_a; i++) {
		if (Ctrl->T.active) a[i].omega = -a[i].omega;
		if ((Ctrl->S.active && a[i].lat > 0.0) || (Ctrl->N.active && a[i].lat < 0.0) || (!(Ctrl->S.active || Ctrl->N.active) && a[i].omega < 0.0))	/* flip to antipole */
			a[i].lat = -a[i].lat, a[i].lon += 180.0, a[i].omega = -a[i].omega;
		while (a[i].lon >= 360.0) a[i].lon -= 360.0;				/* Force geodetic longitude range */
		while (a[i].lon <= -180.0) a[i].lon += 360.0;				/* Force geodetic longitude range */
		while (!Ctrl->D.active && a[i].lon < 0.0) a[i].lon += 360.0;		/* Force geodetic longitude range */
		while (Ctrl->D.active && a[i].lon > 180.0) a[i].lon -= 360.0;		/* Force a geographic longitude range */
		if (Ctrl->E.active) a[i].omega *= Ctrl->E.value;
		out[GMT_X] = a[i].lon;	out[GMT_Y] = a[i].lat;
		n = 2;
		if (Ctrl->G.active) {
			out[0] = (double)a[i].id[0];	out[5] = (double)a[i].id[1];
			out[1] = a[i].t_start;
			out[2] = a[i].lat;
			out[3] = a[i].lon;
			out[4] = a[i].omega * a[i].duration;
		}
		else if (Ctrl->F.mode && no_time) {
			out[n++] = a[i].omega * a[i].duration;
		}
		else if (Ctrl->F.mode) {
			out[n++] = a[i].t_start;
			out[n++] = a[i].omega * a[i].duration;
		}
		else {
			out[n++] = a[i].t_start;
			out[n++] = a[i].t_stop;
			out[n++] = a[i].omega * a[i].duration;
		}
		if (a[i].has_cov) {
			double K[9];
			spotter_covar_to_record (GMT, &a[i], K);
			for (k = 0; k < 9; k++) out[n++] = K[k];
		}
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
	}
	if (GMT_End_IO (API, GMT_OUT, 0)) Return (API->error);		/* Disables further data output */

	GMT_free (GMT, a);
	
	Return (GMT_OK);
}
