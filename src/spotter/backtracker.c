/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2015 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; version 3 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
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

#define THIS_MODULE_NAME	"backtracker"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Generate forward and backward flowlines and hotspot tracks"

#include "spotter.h"

#define GMT_PROG_OPTIONS "-:>Vbdfghios" GMT_OPT("HMm")

struct BACKTRACKER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A[young/old] */
		bool active;
		unsigned int mode;	/* 1 specific limits for all input points, 2 if limits are in cols 4 + 5  */
		double t_low, t_high;
	} A;
	struct D {	/* -Df|b */
		bool active;
		unsigned int mode;		/* 1 we go FROM hotspot to seamount, 0 is reverse */
	} D;
	struct E {	/* -E[+]rotfile or -E<lon/lat/angle> */
		bool active;
		bool single;
		bool mode;
		char *file;
		double lon, lat, w;
	} E;
	struct F {	/* -Fdriftfile */
		bool active;
		char *file;
	} F;
	struct L {	/* -L */
		bool active;
		bool mode;		/* false = hotspot tracks, true = flowlines */
		bool stage_id;	/* 1 returns stage id instead of ages */
		double d_km;	/* Resampling spacing */
	} L;
	struct N {	/* -N */
		bool active;
		double t_upper;
	} N;
	struct Q {	/* -Q<tfix> */
		bool active;
		double t_fix;	/* Set fixed age*/
	} Q;
	struct S {	/* -S */
		bool active;
		char *file;
	} S;
	struct T {	/* -T<tzero> */
		bool active;
		double t_zero;	/* Set zero age*/
	} T;
	struct W {	/* -W<flag> */
		bool active;
		char mode;
	} W;
};

void *New_backtracker_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct BACKTRACKER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct BACKTRACKER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	return (C);
}

void Free_backtracker_Ctrl (struct GMT_CTRL *GMT, struct BACKTRACKER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->E.file) free (C->E.file);	
	if (C->F.file) free (C->F.file);	
	if (C->S.file) free (C->S.file);	
	GMT_free (GMT, C);	
}

int GMT_backtracker_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: backtracker [<table>] -E[+]<rottable>|<plon>/<plat>/<prot> [-A[<young></old>]] [-Df|b]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<driftfile] [-Lf|b<d_km>] [-N<upper_age>] [-Q<t_fix>] [-S<stem>] [-T<t_zero>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> (in ASCII, binary, or netCDF) has 3 or more columns.  If no file(s) is given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   First 3 columns must have lon, lat (or lat, lon, see -:) and age (Ma).\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify a single finite rotation (in degrees) to be applied to all input points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Output tracks for ages (or stages, see -L) between young and old [Default is entire track].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no limit is given, then each seamount should have their limits in columns 4 and 5 instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Only applicable in conjunction with the -L option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Db Backtrack mode: move forward in time (from older to younger positions) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Df Flowline mode: move backward in time (from younger to older positions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Give file with lon, lat, time records describing motion of hotspot responsible for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the seamount/path we are concerned with [fixed hotspots].  If given, then the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   input lon, lat is replaced by the position of the drifting hotspot at the given age.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: If -F is used the <d_km> in -L is assumed to be point spacing in Ma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Lb Compute hotspot tracks sampled every <d_km> interval [Default projects single points].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Lf Compute flowline for seamounts of unknown but maximum age [Default projects single points].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    If no <d_km> is given, the start/stop points for each stage are returned.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    If B and F is used instead, stage id is returned as z-value [Default is predicted ages].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Extend earliest stage pole back to <upper_age> [no extension].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Assigned a fixed age to all input points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Add -L<smt_no> to segment header and 4th output column (requires -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the current age in Ma [0].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Return projected point and confidence ellipse for the finite rotation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The input time must exactly match the age of a finite rotation or else we skip the point.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output record will be lon,lat,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wt will output lon,lat,time,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wa will output lon,lat,angle,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -D to specify which direction to rotate [forward in time].\n");
	GMT_Option (API, "bi3,bo,d,h,i,o,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_backtracker_parse (struct GMT_CTRL *GMT, struct BACKTRACKER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to backtracker and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int k;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Supplemental parameters */

			case 'A':	/* Output only an age-limited segment of the track */
				Ctrl->A.active = true;
				if (opt->arg[0]) {	/* Gave specific limits for all input points */
					k = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					if (k == 2) {
						Ctrl->A.t_low = atof (txt_a);
						Ctrl->A.t_high= atof (txt_b);
						Ctrl->A.mode = 1;
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "ERROR Option -A: Append <young>/<old> age or stage limits.\n");
						n_errors++;
					}
				}
				else {	/* Limits for each input point given in columns 4 and 5 */
					Ctrl->A.mode = 2;
				}
				break;

			case 'C':	/* Now done automatically in spotter_init */
				if (GMT_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'D':	/* Specify in which direction we should project */
				Ctrl->D.active = true;
				switch (opt->arg[0]) {
					case 'B':	/* Go FROM hotspot TO seamount */
					case 'b':
						Ctrl->D.mode = false;
						break;
					case 'F':	/* Go FROM seamount TO hotspot */
					case 'f':
						Ctrl->D.mode = true;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "ERROR Option -D: Append b or f\n");
						break;
				}
				break;

			case 'e':
				GMT_Report (API, GMT_MSG_COMPAT, "Warning: -e is deprecated and will be removed in 5.2.x. Use -E instead.\n");
				/* Fall-through on purpose */
			case 'E':	/* File with stage poles or a single rotation pole */
				Ctrl->E.active = true;
				k = (opt->arg[0] == '+') ? 1 : 0;
				if (!GMT_access (GMT, &opt->arg[k], F_OK) && GMT_check_filearg (GMT, 'E', &opt->arg[k], GMT_IN, GMT_IS_DATASET)) {	/* Was given a file (with possible leading + flag) */
					Ctrl->E.file  = strdup (&opt->arg[k]);
					if (k == 1) Ctrl->E.mode = true;
				}
				else {	/* Apply a fixed total reconstruction rotation to all input points  */
					unsigned int ns = 0;
					size_t kk;
					for (kk = 0; kk < strlen (opt->arg); kk++) if (opt->arg[kk] == '/') ns++;
					if (ns == 2) {	/* Looks like we got lon/lat/omega */
						Ctrl->E.single  = true;
						sscanf (opt->arg, "%[^/]/%[^/]/%lg", txt_a, txt_b, &Ctrl->E.w);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->E.lon), txt_a);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->E.lat), txt_b);
					}
					else	/* Junk of some sort */
						n_errors++;
				}
				break;

			case 'F':	/* File with hotspot motion history */
				if ((Ctrl->F.active = GMT_check_filearg (GMT, 'F', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->F.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			case 'L':	/* Specify what kind of track to project */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'F':	/* Calculate flowlines */
						Ctrl->L.stage_id = true;
					case 'f':
						Ctrl->L.mode = true;
						break;
					case 'B':	/* Calculate hotspot tracks */
						Ctrl->L.stage_id = true;
					case 'b':
						Ctrl->L.mode = false;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "ERROR Option -L: Append f or b\n");
						break;
				}
				Ctrl->L.d_km = (opt->arg[1]) ? atof (&opt->arg[1]) : -1.0;
				break;

			case 'Q':	/* Fixed age for all points */
				Ctrl->Q.active = true;
				Ctrl->Q.t_fix = atof (opt->arg);
				break;

			case 'S':	/* Set file stem for individual output files */
				Ctrl->S.active = true;
				if (opt->arg[0]) {
					Ctrl->S.file = strdup (opt->arg);
					Ctrl->S.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "ERROR Option -S: Append a file stem\n");
					n_errors++;
				}
				break;

			case 'T':	/* Current age [0 Ma] */
				Ctrl->T.active = true;
				Ctrl->T.t_zero = atof (opt->arg);
				break;

			case 'W':	/* Report confidence ellipses */
				Ctrl->W.active = true;
				Ctrl->W.mode = opt->arg[0];
				break;

			case 'N':	/* Extend oldest stage back to this time [no extension] */
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->E.active, "Syntax error: Must give -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->L.active, "Syntax error: -W cannot be set if -Lf or -Lb are set\n");
        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && !Ctrl->L.active, "Syntax error: -A requires -L.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define SPOTTER_BACK -1
#define SPOTTER_FWD  +1

int spotter_track (struct GMT_CTRL *GMT, int way, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int time_flag, double wesn[], double **c)
{
	int n = -1;
	/* Call either spotter_forthtrack (way = 1) or spotter_backtrack (way = -1) */
	
	switch (way) {
		case SPOTTER_BACK:
			n = spotter_backtrack (GMT, xp, yp, tp, np, p, ns, d_km, t_zero, time_flag, wesn, c);
			break;
		case SPOTTER_FWD:
			n = spotter_forthtrack (GMT, xp, yp, tp, np, p, ns, d_km, t_zero, time_flag, wesn, c);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad use of spotter_track\n");
			break;
	}
		
	return (n);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_backtracker_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_backtracker (void *V_API, int mode, void *args)
{
	struct EULER *p = NULL;			/* Pointer to array of stage poles */

	uint64_t n_points;		/* Number of data points read */
	uint64_t n_track;		/* Number of points in a track segment */
	uint64_t n_segments;		/* Number of path segments written out */
	uint64_t n_skipped = 0;		/* Number of points skipped because t < 0 */
	uint64_t n_read = 0;		/* Number of records read */
	uint64_t row;
	uint64_t i, j;
	uint64_t n_out, n_expected_fields, col;
	unsigned int n_stages = 0;	/* Number of stage poles */
	int n_fields, error;		/* Misc. signed counters */
	int spotter_way = 0;		/* Either SPOTTER_FWD or SPOTTER_BACK */
	bool make_path = false;		/* true means create continuous path, false works on discrete points */
	

	double *c = NULL;		/* Array of track chunks returned by libeuler routines */
	double lon, lat;		/* Seamounts location in decimal degrees */
	double age, t, t_end;		/* Age of seamount, in Ma */
	double t_low = 0.0, t_high = 0.0;
	double *in = NULL, out[10];	/* i/o arrays used by GMT */
	double R[3][3];			/* Rotation matrix */
	double x[3], y[3];		/* Two 3-D unit vectors */

	char type[50] = {""};		/* What kind of line (flowline or hotspot track) */
	char dir[8] = {""};		/* From or To */

	struct GMT_OPTION *ptr = NULL;
	struct GMT_DATASET *F = NULL;
	struct GMT_DATASEGMENT *H = NULL;
	struct BACKTRACKER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_backtracker_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args); if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_backtracker_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_backtracker_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = New_backtracker_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_backtracker_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the backtracker main code ----------------------------*/
	
	if (Ctrl->E.single) {	/* Get rotation matrix R */
		GMT_make_rot_matrix (GMT, Ctrl->E.lon, Ctrl->E.lat, Ctrl->E.w, R);
	}
	else {	/* Load in the stage poles */
		n_stages = spotter_init (GMT, Ctrl->E.file, &p, Ctrl->L.mode, Ctrl->W.active, Ctrl->E.mode, &Ctrl->N.t_upper);
		spotter_way = ((Ctrl->L.mode + Ctrl->D.mode) == 1) ? SPOTTER_FWD : SPOTTER_BACK;

		if (fabs (Ctrl->L.d_km) > GMT_CONV4_LIMIT) {		/* User wants to interpolate tracks rather than project individual points */
			make_path = true;
			GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			(Ctrl->L.mode) ? sprintf (type, "Flowline") : sprintf (type, "Hotspot track");
			(Ctrl->D.mode) ? sprintf (dir, "from") : sprintf (dir, "to");
		}
	}

	if (Ctrl->F.active) {	/* Get hotspot motion file */
		if ((F = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		H = F->table[0]->segment[0];	/* Only one table with one segment for histories */
		for (row = 0; row < H->n_rows; row++) H->coord[GMT_Y][row] = GMT_lat_swap (GMT, H->coord[GMT_Y][row], GMT_LATSWAP_G2O);	/* Convert to geocentric */
	}

	n_points = n_segments = 0;

	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	if (Ctrl->Q.active && !GMT->common.b.ncol[GMT_IN]) n_expected_fields = 2;	/* Lon, lat only; use fixed t */
	if (Ctrl->E.single && !GMT->common.b.active[GMT_IN]) n_expected_fields = GMT_MAX_COLUMNS;	/* Allow any input for -E single rotation */

	n_out = (Ctrl->S.active) ? 4 : 3;	/* Append smt id number as 4th column when individual files are requested */
	if (Ctrl->W.active) n_out = 5 + !(Ctrl->W.mode == 0);

	/* Specify input and output expected columns */
	if ((error = GMT_set_cols (GMT, GMT_IN, n_expected_fields)) != GMT_OK) {
		Return (error);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_out)) != GMT_OK) {
		Return (error);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}

	/* Read the seamount data from file or stdin */

	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_TABLE_HEADER (GMT)) {	/* Skip all table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
				continue;
			}
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
			else if (GMT_REC_IS_NEW_SEGMENT (GMT) && !make_path) {			/* Parse segment headers */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				continue;
			}
		}

		/* Data record to process */
	
		if (Ctrl->E.single) {	/* Simple reconstruction, then exit */
			in[GMT_Y] = GMT_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric */
			GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], x, true);		/* Get x-vector */
			GMT_matrix_vect_mult (GMT, 3U, R, x, y);			/* Rotate the x-vector */
			GMT_cart_to_geo (GMT, &out[GMT_Y], &out[GMT_X], y, true);	/* Recover lon lat representation; true to get degrees */
			out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			GMT_memcpy (&out[GMT_Z], &in[GMT_Z], n_fields - 2, double);
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
			GMT_Report (API, GMT_MSG_VERBOSE, "Point %" PRIu64 " has age (%g) > oldest stage (%g) (skipped)\n", n_read, in[GMT_Z], Ctrl->N.t_upper);
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

		if (make_path) {	/* Asked for paths, now write out several multiple segment tracks */
			if (Ctrl->S.active) {
				out[3] = (double)n_points;	/* Put the seamount id number in 4th column and use -L in header */
				sprintf (GMT->current.io.segment_header, "%s %s %g %g -L%" PRIu64, type, dir, in[GMT_X], in[GMT_Y], n_points);
			}
			else
				sprintf (GMT->current.io.segment_header, "%s %s %g %g", type, dir, in[GMT_X], in[GMT_Y]);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
			
			if (Ctrl->F.active) {	/* Must generate intermediate points in time, then rotatate each adjusted location */
				t = (Ctrl->A.mode) ? t_low : 0.0;
				t_end = (Ctrl->A.mode) ? t_high : age;
				while (t <= t_end) {
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_X], H->n_rows, 1, &t, &lon, GMT->current.setting.interpolant);
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_Y], H->n_rows, 1, &t, &lat, GMT->current.setting.interpolant);
					lon *= D2R;	lat *= D2R;
					if (spotter_track (GMT, spotter_way, &lon, &lat, &t, 1L, p, n_stages, 0.0, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
						GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_track - aborting\n");
						Return (GMT_RUNTIME_ERROR);
					}
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t;
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
					t += Ctrl->L.d_km;	/* dt, actually */
				}
				t -= Ctrl->L.d_km;	/* Last time used in the loop */
				if (!(doubleAlmostEqualZero (t_end, t))) {	/* One more point since t_end was not a multiple of d_km from t_start */
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_X], H->n_rows, 1, &t_end, &lon, GMT->current.setting.interpolant);
					GMT_intpol (GMT, H->coord[GMT_Z], H->coord[GMT_Y], H->n_rows, 1, &t_end, &lat, GMT->current.setting.interpolant);
					lon *= D2R;	lat *= D2R;
					if (spotter_track (GMT, spotter_way, &lon, &lat, &t_end, 1L, p, n_stages, 0.0, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
						GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_track - aborting\n");
						Return (GMT_RUNTIME_ERROR);
					}
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t_end;
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}
			else {
				lon *= D2R;	lat *= D2R;
				if (!Ctrl->W.active) {
					if (spotter_track (GMT, spotter_way, &lon, &lat, &age, 1L, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
						GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_track - aborting\n");
						Return (GMT_RUNTIME_ERROR);
					}
				}
				
				n_track = lrint (c[0]);
				for (j = 0, i = 1; j < n_track; j++, i += 3) {
					out[GMT_Z] = c[i+2];
					if (Ctrl->A.mode && (out[GMT_Z] < t_low || out[GMT_Z] > t_high)) continue;
					out[GMT_X] = c[i] * R2D;
					out[GMT_Y] = GMT_lat_swap (GMT, c[i+1] * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
				}
			}
			if (c) GMT_free (GMT, c);
		}
		else {	/* Just return the projected locations */
			if (Ctrl->W.active) {	/* Asked for confidence ellipses on reconstructed points */
				if (spotter_conf_ellipse (GMT, lon, lat, age, p, n_stages, Ctrl->W.mode, Ctrl->D.mode, out)) {
					GMT_Report (API, GMT_MSG_VERBOSE, "Confidence ellipses only for the age of rotations.  Point with age %g skipped\n", age);
					continue;
				}
			}
			else {
				lon *= D2R;	lat *= D2R;
				if (spotter_track (GMT, spotter_way, &lon, &lat, &age, 1L, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_track - aborting\n");
					Return (GMT_RUNTIME_ERROR);
				}
				out[GMT_X] = lon * R2D;
				out[GMT_Y] = lat * R2D;
				for (col = 2; col < n_expected_fields; col++) out[col] = in[col];
			}
			out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
		}

		n_points++;
	} while (true);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	if (make_path)
		GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " segments written\n", n_points);
	else
		GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points projected\n", n_points);

	if (n_skipped) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points skipped because age < 0\n", n_skipped);

	/* Clean up and exit */

	if (!Ctrl->E.single) GMT_free (GMT, p);
	
	Return (GMT_OK);
}
