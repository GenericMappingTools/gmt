/*--------------------------------------------------------------------
 *
 *   Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *   Contact info: www.generic-mapping-tools.org
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

#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_CLASSIC_NAME	"backtracker"
#define THIS_MODULE_MODERN_NAME	"backtracker"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Generate forward and backward flowlines and hotspot tracks"
#define THIS_MODULE_KEYS	"<D{,>D},FD("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghioqs"

#define SPOTTER_TOWARDS_PRESENT 0
#define SPOTTER_TOWARDS_PAST	1

#define SPOTTER_TRAILLINE	0
#define SPOTTER_FLOWLINE 	1

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
	struct E {	/* -Erotfile[+i], -E<ID1>-<ID2>[+i], or -E<lon/lat/angle> */
		bool active;
		struct SPOTTER_ROT rot;
	} E;
	struct F {	/* -Fdriftfile */
		bool active;
		char *file;
	} F;
	struct L {	/* -L */
		bool active;
		unsigned int mode;	/* 0 = hotspot tracks, 1 = flowlines */
		bool stage_id;	/* 1 returns stage id instead of ages */
		double d_km;	/* Resampling spacing */
	} L;
	struct M {	/* -M[<value>] */
		bool active;
		double value;
	} M;
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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct BACKTRACKER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct BACKTRACKER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->M.value = 0.5;	/* To get half-angles */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct BACKTRACKER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->E.rot.file);
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->S.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s [-A[<young></old>]] [-Df|b]\n", name, SPOTTER_E_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<driftfile] [-Lf|b<d_km>] [-M[<factor>]] [-N<upper_age>] [-Q<t_fix>] [-S<stem>] [-T<t_zero>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> (in ASCII, binary, or netCDF) has 3 or more columns.  If no file(s) is given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   First 3 columns must have lon, lat (or lat, lon, see -:) and age (Ma).\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify a single finite rotation (in degrees) to be applied to all input points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-M Reduce opening angles for stage rotations by <factor> [0.5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Typically used to get half-rates needed for flowlines.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Extend earliest stage pole back to <upper_age> [no extension].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Assign <t_fix> age to all input points [Use 3rd column ages].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Add -L<smt_no> to segment header and 4th output column (requires -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the current age in Ma [0].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Return projected point and confidence ellipse for the finite rotation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The input time must exactly match the age of a finite rotation or else we skip the point.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output record will be lon,lat,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wt will output lon,lat,time,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wa will output lon,lat,angle,az,major,minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -D to specify which direction to rotate [forward in time].\n");
	GMT_Option (API, "bi3,bo,d,e,h,i,o,q,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct BACKTRACKER_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
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
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Append <young>/<old> age or stage limits.\n");
						n_errors++;
					}
				}
				else {	/* Limits for each input point given in columns 4 and 5 */
					Ctrl->A.mode = 2;
				}
				break;

			case 'C':	/* Now done automatically in spotter_init */
				if (gmt_M_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "-C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'D':	/* Specify in which direction we should project */
				Ctrl->D.active = true;
				switch (opt->arg[0]) {
					case 'B':	/* Go from locations formed in the past towards present zero time */
					case 'b':
						Ctrl->D.mode = SPOTTER_TOWARDS_PRESENT;
						break;
					case 'F':	/* Go from locations with zero age towards the past locations */
					case 'f':
						Ctrl->D.mode = SPOTTER_TOWARDS_PAST;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -D: Append b or f\n");
						break;
				}
				break;

			case 'e':
				GMT_Report (API, GMT_MSG_COMPAT, "-e is deprecated and was removed in 5.3. Use -E instead.\n");
				/* Fall-through on purpose */
			case 'E':	/* File with stage poles or a single rotation pole */
				Ctrl->E.active = true;
				n_errors += spotter_parse (GMT, opt->option, opt->arg, &(Ctrl->E.rot));
				break;

			case 'F':	/* File with hotspot motion history */
				if ((Ctrl->F.active = gmt_check_filearg (GMT, 'F', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->F.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			case 'L':	/* Specify what kind of track to project */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'F':	/* Calculate flowlines */
						Ctrl->L.stage_id = true;
						/* Fall through on purpose to 'f' */
					case 'f':
						Ctrl->L.mode = SPOTTER_FLOWLINE;
						break;
					case 'B':	/* Calculate hotspot tracks */
						Ctrl->L.stage_id = true;
						/* Fall through on purpose to 'b' */
					case 'b':
						Ctrl->L.mode = SPOTTER_TRAILLINE;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -L: Append f or b\n");
						break;
				}
				Ctrl->L.d_km = (opt->arg[1]) ? atof (&opt->arg[1]) : -1.0;
				break;

			case 'M':	/* Convert to total reconstruction rotation poles instead */
				Ctrl->M.active = true;
				if (opt->arg[0]) Ctrl->M.value = atof (opt->arg);
				break;

			case 'N':	/* Extend oldest stage back to this time [no extension] */
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
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
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Append a file stem\n");
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

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active, "Must give -E\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->L.active, "Option -W cannot be set if -Lf or -Lb are set\n");
        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && !Ctrl->L.active, "Option -A requires -L.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->M.active, "Option -M cannot be used with -F.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->L.active && Ctrl->D.mode == SPOTTER_TOWARDS_PRESENT && Ctrl->L.mode == SPOTTER_TRAILLINE,
		"Options -Lb -Db with -F not possible.  Backtrack end point first with -Db, then draw -Lb -Df from that point forward instead.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define SPOTTER_BACK -1
#define SPOTTER_FWD  +1

GMT_LOCAL int spotter_track (struct GMT_CTRL *GMT, int way, double xp[], double yp[], double tp[], unsigned int np, struct EULER p[], unsigned int ns, double d_km, double t_zero, unsigned int time_flag, double wesn[], double **c) {
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
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad use of spotter_track\n");
			break;
	}

	return (n);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_backtracker (void *V_API, int mode, void *args) {
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
	unsigned int geometry;
	int n_fields, error;		/* Misc. signed counters */
	int spotter_way = 0;		/* Either SPOTTER_FWD or SPOTTER_BACK */
	bool make_path = false;		/* true means create continuous path, false works on discrete points */
	bool E_first = true;


	double *c = NULL;		/* Array of track chunks returned by libeuler routines */
	double lon, lat;		/* Seamounts location in decimal degrees */
	double age, t, t_end;		/* Age of seamount, in Ma */
	double hlon, hlat;		/* Location of drifting hotspot at a given time */
	double t_low = 0.0, t_high = 0.0;
	double *in = NULL, out[10];	/* i/o arrays used by GMT */
	double R[3][3];			/* Rotation matrix */
	double x[3], y[3];		/* Two 3-D unit vectors */

	char type[50] = {""};		/* What kind of line (flowline or hotspot track) */
	char dir[8] = {""};		/* From or To */

	struct GMT_OPTION *ptr = NULL;
	struct GMT_DATASET *F = NULL;
	struct GMT_DATASEGMENT *H = NULL;
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct BACKTRACKER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args); if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the backtracker main code ----------------------------*/

	if (Ctrl->E.rot.single) {	/* Get rotation matrix R */
		gmt_make_rot_matrix (GMT, Ctrl->E.rot.lon, Ctrl->E.rot.lat, Ctrl->E.rot.w, R);
	}
	else {	/* Load in the stage poles */
		char *emode[2] = {"trail", "flow"};
		char *fmode[2] = {"back", "for"};
		if (Ctrl->M.active) {	/* Must convert to stage poles and adjust opening angles */
			char tmpfile[GMT_LEN32] = {""}, cmd[GMT_LEN128] = {""};
			sprintf (tmpfile, "gmt_half_rots.%d", (int)getpid());
			sprintf (cmd, "%s -M%g -Fs ->%s", Ctrl->E.rot.file, Ctrl->M.value, tmpfile);
			if (GMT_Call_Module (API, "rotconverter", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to convert %s to half-rates\n", Ctrl->E.rot.file);
				Return (API->error);
			}
			n_stages = spotter_init (GMT, tmpfile, &p, Ctrl->L.mode, Ctrl->W.active, Ctrl->E.rot.invert, &Ctrl->N.t_upper);
			gmt_remove_file (GMT, tmpfile);
		}
		else
			n_stages = spotter_init (GMT, Ctrl->E.rot.file, &p, Ctrl->L.mode, Ctrl->W.active, Ctrl->E.rot.invert, &Ctrl->N.t_upper);

		spotter_way = ((Ctrl->L.mode + Ctrl->D.mode) == 1) ? SPOTTER_FWD : SPOTTER_BACK;
		GMT_Report (API, GMT_MSG_INFORMATION, "Loaded rotations in order for %slines; calling spotter_track with direction %swards\n", emode[mode], fmode[(spotter_way+1)/2]);

		if (fabs (Ctrl->L.d_km) > GMT_CONV4_LIMIT) {		/* User wants to interpolate tracks rather than project individual points */
			make_path = true;
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			(Ctrl->L.mode == SPOTTER_FLOWLINE) ? sprintf (type, "Flowline") : sprintf (type, "Hotspot track");
			(Ctrl->D.mode == SPOTTER_TOWARDS_PAST) ? sprintf (dir, "from") : sprintf (dir, "to");
		}
	}

	if (Ctrl->F.active) {	/* Get and use hotspot motion file */
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
		if ((F = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (F->n_columns < 3) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least 3 are needed\n", Ctrl->F.file, (int)F->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		if (F->table[0]->segment[0]->data[GMT_Z][0] > GMT_CONV4_LIMIT) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s does not start at the present\n", Ctrl->F.file);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		H = F->table[0]->segment[0];	/* Only one table with one segment for histories */
		for (row = 0; row < H->n_rows; row++) H->data[GMT_Y][row] = gmt_lat_swap (GMT, H->data[GMT_Y][row], GMT_LATSWAP_G2O);	/* Convert to geocentric */
	}

	n_points = n_segments = 0;

	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : 3 + ((Ctrl->A.mode == 2) ? 2 : 0);
	if (Ctrl->Q.active && !GMT->common.b.ncol[GMT_IN]) n_expected_fields = 2;	/* Lon, lat only; use fixed t */
	if (Ctrl->E.rot.single && !GMT->common.b.active[GMT_IN]) n_expected_fields = GMT_MAX_COLUMNS;	/* Allow any input for -E single rotation */

	n_out = (Ctrl->S.active) ? 4 : 3;	/* Append smt id number as 4th column when individual files are requested */
	if (Ctrl->W.active) n_out = 5 + !(Ctrl->W.mode == 0);
	geometry = (make_path) ? GMT_IS_LINE : GMT_IS_POINT;
	gmt_M_memset (out, 10, double);

	/* Specify input and output expected columns */
	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_expected_fields, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}

	/* Read the seamount data from file or stdin */

	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, geometry) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_table_header (GMT)) {	/* Skip all table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
				continue;
			}
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_new_segment (GMT)) {			/* Parse segment headers */
				if (!make_path) GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				continue;
			}
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */
		in[GMT_Y] = gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric latitude */

		if (Ctrl->E.rot.single) {	/* Simple reconstruction, then return to top of read loop */
			if (E_first) {
				if ((error = GMT_Set_Columns (API, GMT_OUT, n_fields, (In->text == NULL) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX)) != GMT_NOERROR) {
					Return (error);
				}
				E_first = false;
			}
			Out->text = In->text;
			gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], x, true);		/* Get x-vector */
			gmt_matrix_vect_mult (GMT, 3U, R, x, y);			/* Rotate the x-vector */
			gmt_cart_to_geo (GMT, &out[GMT_Y], &out[GMT_X], y, true);	/* Recover lon lat representation; true to get degrees */
			out[GMT_Y] = gmt_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			if (n_fields > 2) gmt_M_memcpy (&out[GMT_Z], &in[GMT_Z], n_fields - 2, double);	/* Append any other items */
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			continue;	/* Go back for next point */
		}

		if (Ctrl->Q.active) in[GMT_Z] = Ctrl->Q.t_fix;	/* Override point age with a fixed age */
		if (in[GMT_Z] < 0.0) {	/* Negative ages are flags for points to be skipped */
			n_skipped++;
			continue;
		}

		if (Ctrl->A.mode) {	/* Consider just a time interval */
			if (Ctrl->A.mode == 1) {	/* Get limits from -A */
				t_low  = Ctrl->A.t_low;
				t_high = Ctrl->A.t_high;
			}
			else if (Ctrl->A.mode == 2) {	/* Get limits from the input file cols 4 and 5 */
				t_low  = in[3];
				t_high = in[4];
			}
			age = t_high;	/* No point working more than necessary */
		}
		else	/* Use the age as given (but see -Q) */
			age = in[GMT_Z];

		if (age > Ctrl->N.t_upper) {	/* Points older than oldest stage cannot be used */
			GMT_Report (API, GMT_MSG_WARNING, "Point %" PRIu64 " has age (%g) > oldest stage (%g) (skipped)\n", n_read, in[GMT_Z], Ctrl->N.t_upper);
			n_skipped++;
			continue;
		}

		if (make_path) {	/* Asked for flowline or backtrack paths, must write out several multiple segment tracks */
			if (Ctrl->S.active) {
				out[3] = (double)n_points;	/* Put the seamount id number in 4th column and use -L in header */
				sprintf (GMT->current.io.segment_header, "%s %s %g %g -L%" PRIu64, type, dir, in[GMT_X], in[GMT_Y], n_points);
			}
			else
				sprintf (GMT->current.io.segment_header, "%s %s %g %g", type, dir, in[GMT_X], in[GMT_Y]);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);

			if (Ctrl->F.active && Ctrl->L.mode == SPOTTER_TRAILLINE && Ctrl->D.mode == SPOTTER_TOWARDS_PAST) {	/* Must deal with drift before or after rotations */
				/* Take the drift curve and obtain dlon,dlat for a given time, then apply rotation of that point for the given time.  This combines
				 * both the drift and the rotation and is used to PREDICT positions along trails based on hotspot location */
				t = (Ctrl->A.mode) ? t_low : 0.0;
				t_end = (Ctrl->A.mode) ? t_high : age;

				while (t <= t_end) {
					/* Determine location along the drift curve for this time */
					gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_X], H->n_rows, 1, &t, &hlon, GMT->current.setting.interpolant);
					gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_Y], H->n_rows, 1, &t, &hlat, GMT->current.setting.interpolant);
					lon = in[GMT_X];	lat = in[GMT_Y];
					if (Ctrl->D.mode == SPOTTER_TOWARDS_PAST) {	/* Add the drift amount to our input point to simulate where drift would have placed this point */
						lon += (hlon - H->data[GMT_X][0]);
						lat += (hlat - H->data[GMT_Y][0]);
					}
					lon *= D2R;	lat *= D2R;
					if (spotter_track (GMT, spotter_way, &lon, &lat, &t, 1L, p, n_stages, 0.0, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
						GMT_Report (API, GMT_MSG_ERROR, "Nothing returned from spotter_track - aborting\n");
						Return (GMT_RUNTIME_ERROR);
					}
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = lat * R2D;
					if (Ctrl->F.active && Ctrl->D.mode == SPOTTER_TOWARDS_PRESENT && Ctrl->L.mode == SPOTTER_TRAILLINE) {	/* Must account for hotspot drift, use interpolation for given age */
						out[GMT_X] -= (hlon - H->data[GMT_X][0]);
						out[GMT_Y] -= (hlat - H->data[GMT_Y][0]);
					}
					out[GMT_Y] = gmt_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t;
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					t += Ctrl->L.d_km;	/* dt, actually */
				}
				t -= Ctrl->L.d_km;	/* Last time used in the loop */
				if (!(doubleAlmostEqualZero (t_end, t))) {	/* One more point since t_end was not a multiple of d_km from t_start */
					gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_X], H->n_rows, 1, &t_end, &hlon, GMT->current.setting.interpolant);
					gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_Y], H->n_rows, 1, &t_end, &hlat, GMT->current.setting.interpolant);
					lon = in[GMT_X];	lat = in[GMT_Y];
					if (Ctrl->D.mode == SPOTTER_TOWARDS_PAST) {	/* Add the drift amount to our input point to simulate where drift would have placed this point */
						lon += (hlon - H->data[GMT_X][0]);
						lat += (hlat - H->data[GMT_Y][0]);
					}
					lon *= D2R;	lat *= D2R;
					if (spotter_track (GMT, spotter_way, &lon, &lat, &t_end, 1L, p, n_stages, 0.0, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
						GMT_Report (API, GMT_MSG_ERROR, "Nothing returned from spotter_track - aborting\n");
						Return (GMT_RUNTIME_ERROR);
					}
					out[GMT_X] = lon * R2D;
					out[GMT_Y] = lat * R2D;
					if (Ctrl->F.active && Ctrl->D.mode == SPOTTER_TOWARDS_PRESENT && Ctrl->L.mode == SPOTTER_TRAILLINE) {	/* Must account for hotspot drift, use interpolation for given age */
						out[GMT_X] -= (hlon - H->data[GMT_X][0]);
						out[GMT_Y] -= (hlat - H->data[GMT_Y][0]);
					}
					out[GMT_Y] = gmt_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					out[GMT_Z] = t_end;
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else {	/* Either no drift or we are asking for flowlines */
				lon = in[GMT_X];	lat = in[GMT_Y];
				lon *= D2R;		lat *= D2R;
				if (spotter_track (GMT, spotter_way, &lon, &lat, &age, 1L, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
					GMT_Report (API, GMT_MSG_ERROR, "Nothing returned from spotter_track - aborting\n");
					Return (GMT_RUNTIME_ERROR);
				}
				n_track = lrint (c[0]);
				for (j = 0, i = 1; j < n_track; j++, i += 3) {
					out[GMT_Z] = c[i+2];
					if (Ctrl->A.mode && (out[GMT_Z] < t_low || out[GMT_Z] > t_high)) continue;
					out[GMT_X] = c[i] * R2D;
					out[GMT_Y] = c[i+1] * R2D;
					if (Ctrl->F.active && Ctrl->D.mode == SPOTTER_TOWARDS_PRESENT && Ctrl->L.mode == SPOTTER_TRAILLINE) {	/* Must account for hotspot drift, use interpolation for given age */
						double t_use = H->data[GMT_Z][H->n_rows-1] - out[GMT_Z];
						gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_X], H->n_rows, 1, &t_use, &hlon, GMT->current.setting.interpolant);
						gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_Y], H->n_rows, 1, &t_use, &hlat, GMT->current.setting.interpolant);
						out[GMT_X] -= (hlon - H->data[GMT_X][0]);
						out[GMT_Y] -= (hlat - H->data[GMT_Y][0]);
					}
					out[GMT_Y] = gmt_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			gmt_M_free (GMT, c);
		}
		else {	/* Just return the projected final locations */
			if (Ctrl->F.active) {	/* Must account for hotspot drift, use interpolation at given age */
				gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_X], H->n_rows, 1, &age, &hlon, GMT->current.setting.interpolant);
				gmt_intpol (GMT, H->data[GMT_Z], H->data[GMT_Y], H->n_rows, 1, &age, &hlat, GMT->current.setting.interpolant);
			}
			lon = in[GMT_X];	lat = in[GMT_Y];
			if (Ctrl->F.active && Ctrl->D.mode == SPOTTER_TOWARDS_PAST && Ctrl->L.mode == SPOTTER_TRAILLINE) {	/* Must account for hotspot drift */
				lon += (hlon - H->data[GMT_X][0]);
				lat += (hlat - H->data[GMT_Y][0]);
			}
			if (Ctrl->W.active) {	/* Asked for confidence ellipses on reconstructed points */
				if (spotter_conf_ellipse (GMT, lon, lat, age, p, n_stages, Ctrl->W.mode, Ctrl->D.mode, out)) {
					GMT_Report (API, GMT_MSG_WARNING, "Confidence ellipses only for the age of rotations.  Point with age %g skipped\n", age);
					continue;
				}
			}
			else {	/* Reconstruct the point */
				lon *= D2R;	lat *= D2R;
				if (spotter_track (GMT, spotter_way, &lon, &lat, &age, 1L, p, n_stages, Ctrl->L.d_km, Ctrl->T.t_zero, 1 + Ctrl->L.stage_id, NULL, &c) <= 0) {
					GMT_Report (API, GMT_MSG_ERROR, "Nothing returned from spotter_track - aborting\n");
					Return (GMT_RUNTIME_ERROR);
				}
				out[GMT_X] = lon * R2D;
				out[GMT_Y] = lat * R2D;
				if (Ctrl->F.active && Ctrl->D.mode == SPOTTER_TOWARDS_PRESENT && Ctrl->L.mode == SPOTTER_TRAILLINE) {	/* Must account for hotspot drift, use interpolation for given age */
					out[GMT_X] -= (hlon - H->data[GMT_X][0]);
					out[GMT_Y] -= (hlat - H->data[GMT_Y][0]);
				}
				for (col = GMT_Z; col < n_expected_fields; col++) out[col] = in[col];
			}
			Out->text = In->text;
			out[GMT_Y] = gmt_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}

		n_points++;
	} while (true);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	gmt_M_free (GMT, Out);

	if (make_path)
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " segments written\n", n_points);
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " points projected\n", n_points);

	if (n_skipped) GMT_Report (API, GMT_MSG_WARNING, "%" PRIu64 " points skipped because age < 0\n", n_skipped);

	/* Clean up and exit */

	if (!Ctrl->E.rot.single) gmt_M_free (GMT, p);

	Return (GMT_NOERROR);
}
