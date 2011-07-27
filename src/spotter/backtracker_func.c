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
 * Program for moving points along small circles on a sphere given a
 * set of plate motion stage (Euler) poles.
 * backtracker can move a point forward or backward in time.
 * It can do so either along flowlines or hotspot tracks.
 * It can move point to final position or generate a track between
 * starting point and final position.  The output track is a GMT
 * multisegment file and can be plotted with psxy -M.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	29-DEC-1999
 * Version:	1.2 for GMT 5
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
 *
 * Binary data files cannot have header records, and data fields must all be
 * either single or double precision (see -bi option)
 */

#include "spotter.h"
#include "gmt_proj.h"

struct BACKTRACKER_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A[young/old] */
		GMT_LONG active;
		GMT_LONG mode;	/* 1 specific limits for all input points, 2 if limits are in cols 4 + 5  */
		double t_low, t_high;
	} A;
	struct D {	/* -Df|b */
		GMT_LONG active;
		GMT_LONG mode;		/* 1 we go FROM hotspot to seamount, 0 is reverse */
	} D;
	struct E {	/* -E[+]rotfile */
		GMT_LONG active;
		GMT_LONG mode;
		char *file;
	} E;
	struct e {	/* -e<lon/lat/angle> */
		GMT_LONG active;
		double lon, lat, w;
	} e;
	struct F {	/* -Fdriftfile */
		GMT_LONG active;
		char *file;
	} F;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;		/* 0 = hotspot tracks, 1 = flowlines */
		GMT_LONG stage_id;	/* 1 returns stage id instead of ages */
		double d_km;	/* Resampling spacing */
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		double t_upper;
	} N;
	struct Q {	/* -Q<tfix> */
		GMT_LONG active;
		double t_fix;	/* Set fixed age*/
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		char *file;
	} S;
	struct T {	/* -T<tzero> */
		GMT_LONG active;
		double t_zero;	/* Set zero age*/
	} T;
	struct W {	/* -W<flag> */
		GMT_LONG active;
		char mode;
	} W;
};

void *New_backtracker_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct BACKTRACKER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct BACKTRACKER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return ((void *)C);
}

void Free_backtracker_Ctrl (struct GMT_CTRL *GMT, struct BACKTRACKER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->E.file) free ((void *)C->E.file);	
	if (C->F.file) free ((void *)C->F.file);	
	if (C->S.file) free ((void *)C->S.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_backtracker_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "backtracker %s - Generate forward and backward flowlines and hotspot tracks\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: backtracker [<table>] -E[+]<rottable> OR -e<plon>/<plat>/<prot> [-A[<young></old>]] [-Df|b] [-F<driftfile] [-Lf|b<d_km>]\n");
	GMT_message (GMT, "\t[-N<upper_age>] [-Q<t_fix>] [-S<stem>] [-T<t_zero>] [%s] [-W] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<table> (in ASCII, binary, or netCDF) has 3 or more columns.  If no file(s) is given, standard input is read.\n");
	GMT_message (GMT, "\t   First 3 columns must have lon, lat (or lat, lon, see -:) and age (Ma).\n");
	GMT_message (GMT, "\t-E Specify file with the rotations to be used (see man page for format).\n");
	GMT_message (GMT, "\t   Prepend + if you want to invert the finite rotations prior to use.\n");
	GMT_message (GMT, "\t-e Alternatively, specify a single finite rotation (in degrees) to be applied to all input points.\n");
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-A Output tracks for ages (or stages, see -L) between young and old [Default is entire track].\n");
	GMT_message (GMT, "\t   If no limit is given, then each seamount should have their limits in columns 4 and 5 instead.\n");
	GMT_message (GMT, "\t   Only applicable in conjunction with the -L option.\n");
	GMT_message (GMT, "\t-Db Backtrack mode: move forward in time (from older to younger positions) [Default].\n");
	GMT_message (GMT, "\t-Df Flowline mode: move backward in time (from younger to older positions).\n");
	GMT_message (GMT, "\t-F Give file with lon, lat, time records describing motion of hotspot responsible for)\n");
	GMT_message (GMT, "\t   the seamount/path we are concerned with [fixed hotspots].  If given, then the)\n");
	GMT_message (GMT, "\t   input lon, lat is replaced by the position of the drifting hotspot at the given age.)\n");
	GMT_message (GMT, "\t   Note: If -F is used the <d_km> in -L is assumed to be point spacing in Ma.)\n");
	GMT_message (GMT, "\t-Lb Compute hotspot tracks sampled every <d_km> interval [Default projects single points].\n");
	GMT_message (GMT, "\t-Lf Compute flowline for seamounts of unknown but maximum age [Default projects single points].\n");
	GMT_message (GMT, "\t    If no <d_km> is given, the start/stop points for each stage are returned.\n");
	GMT_message (GMT, "\t    If B and F is used instead, stage id is returned as z-value [Default is predicted ages].\n");
	GMT_message (GMT, "\t-N Extend earliest stage pole back to <upper_age> [no extension].\n");
	GMT_message (GMT, "\t-Q Assigned a fixed age to all input points.\n");
	GMT_message (GMT, "\t-S Add -L<smt_no> to segment header and 4th output column (requires -L).\n");
	GMT_message (GMT, "\t-T Set the current age in Ma [0].\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Return projected point and confidence ellipse for the finite rotation.\n");
	GMT_message (GMT, "\t   The input time must exactly match the age of a finite rotation or else we skip the point.\n");
	GMT_message (GMT, "\t   Output record will be lon,lat,az,major,minor.\n");
	GMT_message (GMT, "\t   -Wt will output lon,lat,time,az,major,minor.\n");
	GMT_message (GMT, "\t   -Wa will output lon,lat,angle,az,major,minor.\n");
	GMT_message (GMT, "\t   Use -D to specify which direction to rotate [forward in time].\n");
	GMT_explain_options (GMT, "C3D0hio:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_backtracker_parse (struct GMTAPI_CTRL *C, struct BACKTRACKER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to backtracker and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k;
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				break;

			/* Supplemental parameters */

			case 'A':	/* Output only an age-limited segment of the track */
				Ctrl->A.active = TRUE;
				if (opt->arg[0]) {	/* Gave specific limits for all input points */
					k = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					if (k == 2) {
						Ctrl->A.t_low = atof (txt_a);
						Ctrl->A.t_high= atof (txt_b);
						Ctrl->A.mode = 1;
					}
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR Option -A: Append <young>/<old> age or stage limits.\n");
						n_errors++;
					}
				}
				else {	/* Limits for each input point given in columns 4 and 5 */
					Ctrl->A.mode = 2;
				}
				break;

#ifdef GMT_COMPAT
			case 'C':	/* Now done automatically in spotter_init */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				break;
#endif
			case 'D':	/* Specify in which direction we should project */
				Ctrl->D.active = TRUE;
				switch (opt->arg[0]) {
					case 'B':	/* Go FROM hotspot TO seamount */
					case 'b':
						Ctrl->D.mode = FALSE;
						break;
					case 'F':	/* Go FROM seamount TO hotspot */
					case 'f':
						Ctrl->D.mode = TRUE;
						break;
					default:
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR Option -D: Append b or f\n");
						break;
				}
				break;

			case 'E':	/* File with stage poles */
				Ctrl->E.active = TRUE;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = TRUE; k = 1;}
				Ctrl->E.file  = strdup (&opt->arg[k]);
				break;

			case 'e':	/* Apply a fixed total reconstruction rotation to all input points  */
				Ctrl->e.active  = TRUE;
				sscanf (opt->arg, "%[^/]/%[^/]/%lg", txt_a, txt_b, &Ctrl->e.w);
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->e.lon), txt_a);
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->e.lat), txt_b);
				break;

			case 'F':	/* File with hotspot motion history */
				Ctrl->F.active = TRUE;
				Ctrl->F.file  = strdup (opt->arg);
				break;

			case 'L':	/* Specify what kind of track to project */
				Ctrl->L.active = TRUE;
				switch (opt->arg[0]) {
					case 'F':	/* Calculate flowlines */
						Ctrl->L.stage_id = TRUE;
					case 'f':
						Ctrl->L.mode = TRUE;
						break;
					case 'B':	/* Calculate hotspot tracks */
						Ctrl->L.stage_id = TRUE;
					case 'b':
						Ctrl->L.mode = FALSE;
						break;
					default:
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR Option -L: Append f or b\n");
						break;
				}
				Ctrl->L.d_km = (opt->arg[1]) ? atof (&opt->arg[1]) : -1.0;
				break;

			case 'Q':	/* Fixed age for all points */
				Ctrl->Q.active = TRUE;
				Ctrl->Q.t_fix = atof (opt->arg);
				break;

			case 'S':	/* Set file stem for individual output files */
				Ctrl->S.active = TRUE;
				if (opt->arg[0]) {
					Ctrl->S.file = strdup (opt->arg);
					Ctrl->S.active = TRUE;
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "ERROR Option -S: Append a file stem\n");
					n_errors++;
				}
				break;

			case 'T':	/* Current age [0 Ma] */
				Ctrl->T.active = TRUE;
				Ctrl->T.t_zero = atof (opt->arg);
				break;

			case 'W':	/* Report confidence ellipses */
				Ctrl->W.active = TRUE;
				Ctrl->W.mode = opt->arg[0];
				break;

			case 'N':	/* Extend oldest stage back to this time [no extension] */
				Ctrl->N.active = TRUE;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->e.active && !Ctrl->E.active, "Syntax error: Must give -E or -e\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->L.active, "Syntax error: -W cannot be set if -Lf or -Lb are set\n");
        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && !Ctrl->L.active, "Syntax error: -A requires -L.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_backtracker_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_backtracker (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	struct EULER *p = NULL;			/* Pointer to array of stage poles */

	GMT_LONG n_points;			/* Number of data points read */
	GMT_LONG n_chunk;			/* Total length or array returned by libeuler functions */
	GMT_LONG n_track;			/* Number of points in a track segment */
	GMT_LONG n_stages = 0;			/* Number of stage poles */
	GMT_LONG n_segments;			/* Number of path segments written out */
	GMT_LONG n_skipped = 0;			/* Number of points skipped because t < 0 */
	GMT_LONG n_fields, n_expected_fields;
	GMT_LONG n_read = 0;
	GMT_LONG n_out, error;
	GMT_LONG i, j, k, n;			/* Misc. counters */
	GMT_LONG make_path = FALSE;	/* TRUE means create continuous path, FALSE works on discrete points */

	double *c = NULL;		/* Array of track chunks returned by libeuler routines */
	double lon, lat;		/* Seamounts location in decimal degrees */
	double age, t, t_end;		/* Age of seamount, in Ma */
	double t_low = 0.0, t_high = 0.0;
	double *in = NULL, out[10];	/* i/o arrays used by GMT */
	double R[3][3];			/* Rotation matrix */
	double x[3], y[3];		/* Two 3-D unit vectors */

	char type[50];			/* What kind of line (flowline or hotspot track) */
	char dir[8];			/* From or To */

	PFL spot_func = NULL;			/* Pointer to the required forth/back track function */
	struct GMT_OPTION *ptr = NULL;
	struct GMT_DATASET *F = NULL;
	struct GMT_LINE_SEGMENT *H = NULL;
	struct BACKTRACKER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_backtracker_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_backtracker_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_backtracker", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "ghios>" GMT_OPT("HMm"), options))) Return (error);
	if (GMT_Find_Option (API, 'f', options, &ptr)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = (struct BACKTRACKER_CTRL *) New_backtracker_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_backtracker_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the backtracker main code ----------------------------*/

	GMT_lat_swap_init (GMT);	/* Initialize auxiliary latitude machinery */
	
	if (Ctrl->e.active) {	/* Get rotation matrix R */
		spotter_make_rot_matrix (GMT, Ctrl->e.lon, Ctrl->e.lat, Ctrl->e.w, R);
	}
	else {	/* Load in the stage poles */
		n_stages = spotter_init (GMT, Ctrl->E.file, &p, Ctrl->L.mode, Ctrl->W.active, Ctrl->E.mode, &Ctrl->N.t_upper);
		spot_func = ((Ctrl->L.mode + Ctrl->D.mode) == 1) ? spotter_forthtrack : spotter_backtrack;

		if (fabs (Ctrl->L.d_km) > GMT_SMALL) {		/* User wants to interpolate tracks rather than project individual points */
			make_path = TRUE;
			GMT->current.io.multi_segments[GMT_OUT] = TRUE;		/* Turn on -mo explicitly */
			(Ctrl->L.mode) ? sprintf (type, "Flowline") : sprintf (type, "Hotspot track");
			(Ctrl->D.mode) ? sprintf (dir, "from") : sprintf (dir, "to");
		}
	}

	if (Ctrl->F.active) {	/* Get hotspot motion file */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->F.file, (void **)&F)) Return ((error = GMT_DATA_READ_ERROR));
		H = F->table[0]->segment[0];	/* Only one table with one segment for histories */
		for (j = 0; j < H->n_rows; j++) H->coord[GMT_Y][j] = GMT_lat_swap (GMT, H->coord[GMT_Y][j], GMT_LATSWAP_G2O);	/* Convert to geocentric */
	}

	n_points = n_segments = 0;

	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	if (Ctrl->Q.active && !GMT->common.b.ncol[GMT_IN]) n_expected_fields = 2;	/* Lon, lat only; use fixed t */
	if (Ctrl->e.active && !GMT->common.b.active[GMT_IN]) n_expected_fields = GMT_MAX_COLUMNS;	/* Allow any input for -e mode */

	n_out = (Ctrl->S.active) ? 4 : 3;	/* Append smt id number as 4th column when individual files are requested */
	if (Ctrl->W.active) n_out = 5 + !(Ctrl->W.mode == 0);

	/* Specify input and output expected columns */
	if ((error = GMT_set_cols (GMT, GMT_IN, n_expected_fields))) Return (error);
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return (error);

	/* Initialize the i/o for doing record-by-record reading/writing */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */

	/* Read the seamount data from file or stdin */

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);				/* Enables data output and sets access mode */

	n = 0;
	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT) && n_fields < 2) continue;

		if (GMT_REC_IS_TBL_HEADER (GMT)) GMT_Put_Record (API, GMT_WRITE_TBLHEADER, NULL);	/* Echo table headers */

		if (GMT_REC_IS_NEW_SEGMENT (GMT) && !make_path) GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;
	
		if (Ctrl->e.active) {	/* Simple reconstruction, then exit */
			in[GMT_Y] = GMT_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric */
			GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], x, TRUE);		/* Get x-vector */
			spotter_matrix_vect_mult (GMT, R, x, y);			/* Rotate the x-vector */
			GMT_cart_to_geo (GMT, &out[GMT_Y], &out[GMT_X], y, TRUE);	/* Recover lon lat representation; TRUE to get degrees */
			out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			memcpy ((void *)&out[GMT_Z], (void *)&in[GMT_Z], (n_fields - 2) * sizeof (double));
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			continue;
		}
		
		if (Ctrl->Q.active) in[GMT_Z] = Ctrl->Q.t_fix;
		if (in[GMT_Z] < 0.0) {	/* Negative ages are flags for points to be skipped */
			n_skipped++;
			continue;
		}

		if (Ctrl->A.mode) {	/* Consider just a time interval */
			if (Ctrl->A.mode == 1) {	/* Get limits from -A */
				t_low = Ctrl->A.t_low;
				t_high = Ctrl->A.t_high;
			}
			else if (Ctrl->A.mode == 2) {	/* Get limits from the input file */
				t_low = in[3];
				t_high = in[4];
			}
			age = t_high;	/* No point working more than necessary */
		}
		else
			age = in[GMT_Z];

		if (age > Ctrl->N.t_upper) {	/* Points older than oldest stage cannot be used */
			GMT_report (GMT, GMT_MSG_NORMAL, "Point %ld has age (%g) > oldest stage (%g) (skipped)\n", n_read, in[GMT_Z], Ctrl->N.t_upper);
			n_skipped++;
			continue;
		}

		if (Ctrl->F.active) {	/* Must account for hotspot drift, use interpolation for given age */
			GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_X], H->n_rows, 1, &age, &lon, GMT->current.setting.interpolant);
			GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_Y], H->n_rows, 1, &age, &lat, GMT->current.setting.interpolant);
		}
		else {	/* Use input location */
			in[GMT_Y] = GMT_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric */
			lon = in[GMT_X];
			lat = in[GMT_Y];
		}
		lon *= D2R;	lat *= D2R;

		if (make_path) {	/* Asked for paths, now write out several multiple segment tracks */
			if (Ctrl->S.active) {
				out[3] = (double)n_points;	/* Put the seamount id number in 4th column and use -L in header */
				sprintf (GMT->current.io.segment_header, "%s %s %g %g -L%ld", type, dir, in[GMT_X], in[GMT_Y], n_points);
			}
			else
				sprintf (GMT->current.io.segment_header, "%s %s %g %g", type, dir, in[GMT_X], in[GMT_Y]);
			GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
			
			if (Ctrl->F.active) {	/* Must generate intermediate points in time, then rotatate each adjusted location */
				t = (Ctrl->A.mode) ? t_low : 0.0;
				t_end = (Ctrl->A.mode) ? t_high : age;
				while (t <= t_end) {
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_X], H->n_rows, 1, &t, &lon, GMT->current.setting.interpolant);
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_Y], H->n_rows, 1, &t, &lat, GMT->current.setting.interpolant);
					lon *= D2R;	lat *= D2R;
					n_chunk = (*spot_func) (&lon, &lat, &t, 1, p, n_stages, 0.0, Ctrl->T.t_zero, TRUE + Ctrl->L.stage_id, NULL, &c);
					lat = GMT_lat_swap (GMT, lat, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t;
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
					t += Ctrl->L.d_km;	/* dt, actually */
				}
				t -= Ctrl->L.d_km;	/* Last time used in the loop */
				if (!(GMT_IS_ZERO (t_end - t))) {	/* One more point since t_end was not a multiple of d_km from t_start */
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_X], H->n_rows, 1, &t_end, &lon, GMT->current.setting.interpolant);
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_Y], H->n_rows, 1, &t_end, &lat, GMT->current.setting.interpolant);
					lon *= D2R;	lat *= D2R;
					n_chunk = (*spot_func) (&lon, &lat, &t_end, 1, p, n_stages, 0.0, Ctrl->T.t_zero, TRUE + Ctrl->L.stage_id, NULL, &c);
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t_end;
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}
			else {
				if (!Ctrl->W.active) n_chunk = (*spot_func) (GMT, &lon, &lat, &age, (GMT_LONG)1, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, TRUE + Ctrl->L.stage_id, NULL, &c);
				
				n_track = irint (c[0]);
				for (j = 0, i = 1; j < n_track; j++, i += 3) {
					out[GMT_Z] = c[i+2];
					if (Ctrl->A.mode && (out[GMT_Z] < t_low || out[GMT_Z] > t_high)) continue;
					out[GMT_X] = c[i] * R2D;
					out[GMT_Y] = c[i+1] * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}
			GMT_free (GMT, c);
		}
		else {	/* Just return the projected locations */
			if (Ctrl->W.active) {	/* Asked for confidence ellipses on reconstructed points */
				if (spotter_conf_ellipse (GMT, in[GMT_X], in[GMT_Y], age, p, n_stages, Ctrl->W.mode, Ctrl->D.mode, out)) {
					GMT_report (GMT, GMT_MSG_NORMAL, "Confidence ellipses only for the age of rotations.  Point with age %g skipped\n", age);
					continue;
				}
			}
			else {
				n_chunk = (*spot_func) (GMT, &lon, &lat, &age, 1, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, TRUE + Ctrl->L.stage_id, NULL, &c);
				out[GMT_X] = lon * R2D;
				out[GMT_Y] = lat * R2D;
				for (k = 2; k < n_expected_fields; k++) out[k] = in[k];
			}
			out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
		}

		n_points++;
	}

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	if (make_path)
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld segments written\n", n_points);
	else
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld points projected\n", n_points);

	if (n_skipped) GMT_report (GMT, GMT_MSG_NORMAL, "%ld points skipped because age < 0\n", n_skipped);

	/* Clean up and exit */

	if (!Ctrl->e.active) GMT_free (GMT, p);
	
	Return (GMT_OK);
}
