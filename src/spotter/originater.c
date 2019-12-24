/*--------------------------------------------------------------------
 *
 *   Copyright (c) 2000-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * originater reads file of seamount locations and tries to match each
 * seamount with a probable hotspot by drawing flowines back in time and
 * keeping track of which hotspot is closest to each flowline.  It then
 * reports the closest hotspot, the stage of the flowline involved, the
 * implied pseudo-age of the seamount, and the minimum distance between
 * the flowline and hotspot (in km).
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	29-DEC-1999
 * Version:	1.0
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
 * ASCII seamount location file(s) must have the following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 3. For special header records, see -H
 * 4. Any number of data records which each have the format:
 *    lon lat height radius crustal_age    (or lat lon ..., see -: option).
 *    crustal_age in Ma, height and radius are not used by originater but
 *    are used by hotspotter.
 *
 * Binary files cannot have header records, and data fields must all be
 * either single or double precision (see -bi option).  Output file will
 * be ASCII since it contains a text string (hotspot ID).
 *
 * The file with a list of hotspots must have the following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of hotspot records which each have the format:
 *    lon(deg)  lat(deg)  id  name
 *    the id is a 3-character tag (e.g., HWI), the name is the
 *    full name of the hotspot (e.g., Hawaii).
 *
 * Example: 
 *
 * # Partial list (Pacific) of HotSpots from Table 1 of Yamaji, 1992
 * #Lon		Lat	Abbreviation	Hotspot_name
 * 167		3	CRL	Caroline
 * 230		46	COB	Cobb
 * 205		20	HWI	Hawaii
 * 221.9	-50.9	LSV	Louisville
 * 220		-29	MDN	MacDonald
 * 221		-11	MRQ	Marquesas
 * 231		-27	PTC	Pitcairn
 * 254		-27	SLG	Sala y Gomez
 * 192		-15	SAM	Samoa
 * 212		-18	SOC	Society
 */
 
#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_CLASSIC_NAME	"originater"
#define THIS_MODULE_MODERN_NAME	"originater"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Associate seamounts with nearest hotspot point sources"
#define THIS_MODULE_KEYS	"<D{,FD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdehis" GMT_OPT("HMm")

EXTERN_MSC int gmtlib_great_circle_intersection (struct GMT_CTRL *GMT, double A[], double B[], double C[], double X[], double *CX_dist);
EXTERN_MSC double gmtlib_great_circle_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);

#define KM_PR_RAD (R2D * GMT->current.proj.DIST_KM_PR_DEG)

struct HOTSPOT_ORIGINATOR {
	struct HOTSPOT *h;	/* Pointer to regular HOTSPOT structure */
	/* Extra variables needed for this program */
	double np_dist;		/* Distance to nearest point on the current flowline */
	double np_sign;		/* "Sign" of this distance (see code) */
	double np_time;		/* Predicted time at nearest point */
	double np_lon;		/* Longitude of nearest point on the current flowline */
	double np_lat;		/* Latitude  of nearest point on the current flowline */
	uint64_t nearest;	/* Point id of current flowline node points closest to hotspot */
	unsigned int stage;	/* Stage to which seamount belongs */
	struct GMT_DATASEGMENT *D;	/* Used for drifting hotspots only */
};

struct ORIGINATOR_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct D {	/* -D<factor */
		bool active;
		double value;	
	} D;
	struct E {	/* -Erotfile[+i] */
		bool active;
		bool mode;
		char *file;
	} E;
	struct F {	/* -Fhotspotfile[+d] */
		bool active;
		bool mode;
		char *file;
	} F;
	struct L {	/* -L */
		bool active;
		unsigned int mode;
		bool degree;	/* Report degrees */
	} L;
	struct N {	/* -N */
		bool active;
		double t_upper;
	} N;
	struct Q {	/* -Q<tfix> */
		bool active;
		double t_fix, r_fix;
	} Q;
	struct S {	/* -S */
		bool active;
		unsigned int n;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
	struct W {	/* -W<max_dist> */
		bool active;
		double dist;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct ORIGINATOR_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct ORIGINATOR_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->D.value = 5.0;
	C->N.t_upper = 180.0;
	C->S.n = 1;
	C->W.dist = 1.0e100;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct ORIGINATOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->E.file);	
	gmt_M_str_free (C->F.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int comp_hs (const void *p1, const void *p2) {
	const struct HOTSPOT_ORIGINATOR *a = p1, *b = p2;

	if (a->np_dist < b->np_dist) return (-1);
	if (a->np_dist > b->np_dist) return (+1);
	return (0);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -E<rottable>[+i] -F<hotspottable>[+d] [-D<d_km>] [-H] [-L[<flag>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<upper_age>] [-Qr/t] [-S<n_hs>] [-T] [%s] [-W<maxdist>] [-Z]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify file name for hotspot locations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d if we should look for hotspot drift tables.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If found then we interpolate to get hotspot location as a function of time [fixed].\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> (in ASCII, binary, or netCDF) has 5 or more columns.  If no file(s) is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   standard input is read.  Expects (x,y,z,r,t) records, with t in Ma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set sampling interval in km along tracks [5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Output information for closest approach for nearest hotspot only (ignores -S).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lt gives (time, dist, z) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lw gives (omega, dist, z).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ll gives (lon, lat, time, dist, z).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   dist is in km; use upper case T,W,L to get dist in spherical degrees.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set age (in m.y.) for seafloor where age == NaN [180].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Input files has (x,y,z) only. Append constant r/t to use.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Report the <n_hs> closest hotSpots [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Truncate seamount ages exceeding the upper age set with -N [no truncation].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Report seamounts whose closest encounter to a hotspot is less than <maxdist> km\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default reports for all seamounts].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Write hotspot ID number rather than hotspot TAG.\n");
	GMT_Option (API, "bi5,d,e,h,i,s,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct ORIGINATOR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to originater and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_input;
	int k;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Supplemental parameters */
			case 'C':	/* Now done automatically in spotter_init */
				if (gmt_M_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "-C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.value = atof (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = true;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = true; k = 1;}
				else if ((c = strstr (opt->arg, "+i"))) {Ctrl->E.mode = true; c[0] = '\0';}
				if (gmt_check_filearg (GMT, 'E', &opt->arg[k], GMT_IN, GMT_IS_DATASET))
					Ctrl->E.file  = strdup (&opt->arg[k]);
				else
					n_errors++;
				if (c) c[0] = '+';
				break;
			case 'F':
				Ctrl->F.active = true;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->F.mode = true; k = 1;}
				if (gmt_check_filearg (GMT, 'F', &opt->arg[k], GMT_IN, GMT_IS_DATASET))
					Ctrl->F.file  = strdup (&opt->arg[k]);
				else
					n_errors++;
				break;
			case 'L':
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->L.degree = true;
						/* Fall through on purpose to 'l' */
					case 'l':
						Ctrl->L.mode = 3;
						break;
					case 'w':
					case 'W':
						Ctrl->L.mode = 2;
						break;
					case 'T':
						Ctrl->L.degree = true;
						/* Fall through on purpose to 't' */
					case 't':
						Ctrl->L.mode = 1;
						break;
					default:
						Ctrl->L.mode = 1;
						break;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				sscanf (opt->arg, "%lg/%lg", &Ctrl->Q.r_fix, &Ctrl->Q.t_fix);
				break;
			case 'S':
				Ctrl->S.active = true;
				k = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, k < 1, "Syntax error -S: Must specify a positive number of hotspots\n");
				Ctrl->S.n = k;
				break;
			case 'T':
				Ctrl->T.active = true;
				break;
			case 'W':
				Ctrl->W.active = true;
				Ctrl->W.dist = atof (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_input = (Ctrl->Q.active) ? 3 : 5;
        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_input;

	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_input, "Syntax error: Binary input data (-bi) must have at least %d columns\n", n_input);
	n_errors += gmt_M_check_condition (GMT, !Ctrl->F.file, "Syntax error -F: Must specify hotspot file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.file, "Syntax error -F: Must specify Euler pole file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.value <= 0.0, "Syntax error -D: Must specify a positive interval\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.dist <= 0.0, "Syntax error -W: Must specify a positive distance in km\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_originater (void *V_API, int mode, void *args) {
	unsigned int n_max_spots, n_input;
	unsigned int spot, smt, n_stages, n_hotspots, n_read, n_skipped = 0;
	uint64_t k, kk, np, n_expected_fields, n_out;
	
	int error = 0, ns;
	bool better;

	double x_smt, y_smt, z_smt, r_smt, t_smt, t, *c = NULL, *in = NULL, dist, dlon, lon, lat, out[5];
	double hx_dist, hx_dist_km, dist_NA, dist_NX, del_dist, dt = 0.0, A[3], H[3], N[3], X[3];

	char record[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = {""}, fmt1[GMT_BUFSIZ] = {""}, fmt2[GMT_BUFSIZ] = {""};

	struct EULER *p = NULL;
	struct HOTSPOT *orig_hotspot = NULL;
	struct HOTSPOT_ORIGINATOR *hotspot = NULL, *hot = NULL;
	struct GMT_DATASET *F = NULL;
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct ORIGINATOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the originater main code ----------------------------*/

	ns = spotter_hotspot_init (GMT, Ctrl->F.file, true, &orig_hotspot);	/* Get geocentric hotspot locations */
	if (ns < 0) {
		GMT_exit (GMT, GMT_RUNTIME_ERROR); Return (GMT_RUNTIME_ERROR);		/* An error message was already issued by spotter_hotspot_init() */
	}
	n_hotspots = (unsigned int)ns;
	if (Ctrl->S.n > n_hotspots) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Give value between 1 and %d\n", n_hotspots);
		Return (GMT_RUNTIME_ERROR);
	}
	n_max_spots = MIN (Ctrl->S.n, n_hotspots);

	hotspot = gmt_M_memory (GMT, NULL, n_hotspots, struct HOTSPOT_ORIGINATOR);
	for (spot = 0; spot < n_hotspots; spot++) {
		hotspot[spot].h = &orig_hotspot[spot];	/* Point to the original hotspot structures */
		hotspot[spot].np_dist = 1.0e100;
		if (Ctrl->F.mode) {	/* See if there is a drift file for this hotspot */
			char path[PATH_MAX] = {""}, file[GMT_LEN64] = {""};
			uint64_t row;
			sprintf (file, "%s_drift.txt", hotspot[spot].h->abbrev);
			strncpy (path, file, PATH_MAX);
			if (gmt_access (GMT, path, R_OK)) {	/* Not found in current dir or GMT_DATADIR; check if -F gave an explicit directory */
				if (strchr (Ctrl->F.file, '/')) {	/* Filename has leading path so we will use that path */
					strncpy (path, Ctrl->F.file, PATH_MAX);
					k = strlen (path);
					while (k && path[k] != '/') k--;	/* Look for last slash  */
					k++; path[k] = 0;	/* Truncate anything after last slash */
					strcat (path, file);	/* Prepend path to drift file name */
					if (gmt_access (GMT, path, R_OK)) continue;	/* file do not exist there either */
				}
				else	/* No directory given, so nothing to do */
					continue;
			}
			/* Here we successfully found a drift file, somewhere */
			if ((F = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, file, NULL)) == NULL) {
				Return (API->error);
			}
			if (F->n_columns < 3) {
				GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 3 are needed\n", file, (int)F->n_columns);
				Return (GMT_DIM_TOO_SMALL);
			}
			hotspot[spot].D = F->table[0]->segment[0];	/* Only one table with one segment for histories */
			for (row = 0; row < hotspot[spot].D->n_rows; row++) hotspot[spot].D->data[GMT_Y][row] = gmt_lat_swap (GMT, hotspot[spot].D->data[GMT_Y][row], GMT_LATSWAP_G2O);	/* Convert to geocentric */
		}
	}
	
	n_stages = spotter_init (GMT, Ctrl->E.file, &p, 1, false, Ctrl->E.mode, &Ctrl->N.t_upper);

	hot = gmt_M_memory (GMT, NULL, n_hotspots, struct HOTSPOT_ORIGINATOR);

	sprintf (fmt1, "%s%s%s%s%s%s%s%s%%s", GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, 
		GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator,
		GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator);
	if (Ctrl->Z.active)
		sprintf (fmt2, "%s%%d%s%%d%s%s%s%s", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);
	else
		sprintf (fmt2, "%s%%s%s%%d%s%s%s%s", GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);
	n_input = (Ctrl->Q.active) ? 3 : 5;
	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : n_input;
	if (Ctrl->L.active) {
		n_out = (Ctrl->L.mode == 3) ? 5 : 3;
	}
	else
		n_out = n_expected_fields;
	if (n_out == 3)
		gmt_set_cartesian (GMT, GMT_OUT);	/* Since output is no longer lon/lat */
	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_expected_fields, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	n_read = smt = 0;
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */
		Out->text = In->text;
	
		if (n_input == 3) {	/* Set constant r,t values based on -Q */
			in[3] = Ctrl->Q.r_fix;
			in[4] = Ctrl->Q.t_fix;
		}
		if (gmt_M_is_dnan (in[4]))	/* Age is NaN, assign upper value */
			t_smt = Ctrl->N.t_upper;
		else {			/* Assign given value, truncate if necessary */
			t_smt = in[4];
			if (t_smt > Ctrl->N.t_upper) {
				if (Ctrl->T.active)
					t_smt = Ctrl->N.t_upper;
				else {
					GMT_Report (API, GMT_MSG_VERBOSE, "Seamounts near line %d has age (%g) > oldest stage (%g) (skipped)\n", n_read, t_smt, Ctrl->N.t_upper);
					continue;
				}
			}
		}
		if (t_smt < 0.0) {	/* Negative ages are flags for points to be skipped */
			n_skipped++;
			continue;
		}
		y_smt = D2R * gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric, and radians */
		x_smt = in[GMT_X] * D2R;
		z_smt = in[GMT_Z];
		r_smt = in[3];

		if (!(smt % 10)) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Working on seamount # %5d\r", smt);

		if (spotter_forthtrack (GMT, &x_smt, &y_smt, &t_smt, 1, p, n_stages, Ctrl->D.value, 0.0, 1, NULL, &c) <= 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_forthtrack - aborting\n");
			Return (GMT_RUNTIME_ERROR);
		}

		np = lrint (c[0]);

		gmt_M_memcpy (hot, hotspot, n_hotspots, struct HOTSPOT_ORIGINATOR);

		for (kk = 0, k = 1; kk < np; kk++, k += 3) {	/* For this seamounts track */
			for (spot = 0; spot < n_hotspots; spot++) {	/* For all hotspots */
				if (hot[spot].D) {	/* Must interpolate drifting hotspot location at current time c[k+2] */
					t = c[k+2];	/* Current time */
					gmt_intpol (GMT, hot[spot].D->data[GMT_Z], hot[spot].D->data[GMT_X], hot[spot].D->n_rows, 1, &t, &lon, GMT->current.setting.interpolant);
					gmt_intpol (GMT, hot[spot].D->data[GMT_Z], hot[spot].D->data[GMT_Y], hot[spot].D->n_rows, 1, &t, &lat, GMT->current.setting.interpolant);
				}
				else {	/* Use the fixed hotspot location */
					lon = hot[spot].h->lon;
					lat = hot[spot].h->lat;
				}
				/* Compute distance from track location to (moving or fixed) hotspot */
				dist = gmtlib_great_circle_dist_degree (GMT, lon, lat, R2D * c[k], R2D * c[k+1]);
				if (!Ctrl->L.degree) dist *= GMT->current.proj.DIST_KM_PR_DEG;
				if (dist < hot[spot].np_dist) {
					hot[spot].np_dist = dist;
					hot[spot].nearest = kk;	/* Index of nearest point on the flowline */
				}
			}
		}
		for (spot = 0; spot < n_hotspots; spot++) {

			if (hot[spot].D) {	/* Must interpolate drifting hotspot location at current time c[k+2] */
				t = c[3*hot[spot].nearest+3];	/* Time of closest approach */
				gmt_intpol (GMT, hot[spot].D->data[GMT_Z], hot[spot].D->data[GMT_X], hot[spot].D->n_rows, 1, &t, &lon, GMT->current.setting.interpolant);
				gmt_intpol (GMT, hot[spot].D->data[GMT_Z], hot[spot].D->data[GMT_Y], hot[spot].D->n_rows, 1, &t, &lat, GMT->current.setting.interpolant);
			}
			else {	/* Use the fixed hotspot location */
				lon = hot[spot].h->lon;
				lat = hot[spot].h->lat;
			}
			gmt_geo_to_cart (GMT, lat, lon, H, true);	/* 3-D Cartesian vector of this hotspot */

			/* Fine-tune the nearest point by considering intermediate points along greatcircle between knot points */

			k = 3 * hot[spot].nearest + 1;			/* Corresponding index for x into the (x,y,t) array c */
			gmt_geo_to_cart (GMT, c[k+1], c[k], N, false);	/* 3-D vector of nearest node to this hotspot */
			better = false;
			if (hot[spot].nearest > 0) {	/* There is a point along the flowline before the nearest node */
				gmt_geo_to_cart (GMT, c[k-2], c[k-3], A, false);	/* 3-D vector of end of this segment */
				if (gmtlib_great_circle_intersection (GMT, A, N, H, X, &hx_dist) == 0) {	/* X is between A and N */
					hx_dist_km = d_acos (hx_dist) * KM_PR_RAD;
					if (hx_dist_km < hot[spot].np_dist) {	/* This intermediate point is even closer */
						gmt_cart_to_geo (GMT, &hot[spot].np_lat, &hot[spot].np_lon, X, true);
						hot[spot].np_dist = hx_dist_km;
						dist_NA = d_acos (fabs (gmt_dot3v (GMT, A, N))) * KM_PR_RAD;
						dist_NX = d_acos (fabs (gmt_dot3v (GMT, X, N))) * KM_PR_RAD;
						del_dist = dist_NA - dist_NX;
						dt = (del_dist > 0.0) ? (c[k+2] - c[k-1]) * dist_NX / del_dist : 0.0;
						better = true;
					}
				}
			}
			if (hot[spot].nearest < (np-1) ) {	/* There is a point along the flowline after the nearest node */
				gmt_geo_to_cart (GMT, c[k+4], c[k+3], A, false);	/* 3-D vector of end of this segment */
				if (gmtlib_great_circle_intersection (GMT, A, N, H, X, &hx_dist) == 0) {	/* X is between A and N */
					hx_dist_km = d_acos (hx_dist) * KM_PR_RAD;
					if (hx_dist_km < hot[spot].np_dist) {	/* This intermediate point is even closer */
						gmt_cart_to_geo (GMT, &hot[spot].np_lat, &hot[spot].np_lon, X, true);
						hot[spot].np_dist = hx_dist_km;
						dist_NA = d_acos (fabs (gmt_dot3v (GMT, A, N))) * KM_PR_RAD;
						dist_NX = d_acos (fabs (gmt_dot3v (GMT, X, N))) * KM_PR_RAD;
						del_dist = dist_NA - dist_NX;
						dt = (del_dist > 0.0) ? (c[k+5] - c[k+2]) * dist_NX / del_dist : 0.0;
						better = true;
					}
				}
			}
			if (better) {	/* Point closer to hotspot was found between nodes */
				hot[spot].np_time = c[k+2] + dt;	/* Add time adjustment */
			}
			else {	/* Just use node coordinates */
				hot[spot].np_lon  = c[k] * R2D;	/* Longitude of the flowline's closest approach to hotspot */
				hot[spot].np_lat  = c[k+1] * R2D;	/* Latitude  of the flowline's closest approach to hotspot */
				hot[spot].np_time = c[k+2];	/* Predicted time at the flowline's closest approach to hotspot */
			}

			/* Assign sign to distance: If the vector from the hotspot pointing up along the trail is positive
			 * x-axis and y-axis is normal to that, flowlines whose closest approach point's longitude is
			 * further east are said to have negative distance. */
			 
			gmt_M_set_delta_lon (hot[spot].np_lon, lon, dlon);
			hot[spot].np_sign = copysign (1.0, dlon);
			 
			/* Assign stage id for this point on the flowline */

			k = 0;
			while (k < n_stages && hot[spot].np_time <= p[k].t_stop) k++;
			hot[spot].stage = n_stages - (unsigned int)k;
			if (hot[spot].stage == 0) hot[spot].stage++;
		}

		if (n_hotspots > 1) qsort (hot, n_hotspots, sizeof (struct HOTSPOT_ORIGINATOR), comp_hs);

		if (hot[0].np_dist < Ctrl->W.dist) {
			if (Ctrl->L.mode == 1) {	/* Want time, dist, z output */
				out[0] = hot[0].np_time;
				out[1] = hot[0].np_dist * hot[0].np_sign;
				out[2] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else if (Ctrl->L.mode == 2) {	/* Want omega, dist, z output */
				out[0] = spotter_t2w (GMT, p, n_stages, hot[0].np_time);
				out[1] = hot[0].np_dist * hot[0].np_sign;
				out[2] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else if (Ctrl->L.mode == 3) {	/* Want x, y, time, dist, z output */
				out[GMT_X] = hot[0].np_lon;
				out[GMT_Y] = gmt_lat_swap (GMT, hot[0].np_lat, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
				out[2] = hot[0].np_time;
				out[3] = hot[0].np_dist * hot[0].np_sign;
				out[4] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			else {	/* Conventional originater output */
				out[GMT_X] = in[GMT_X];	out[GMT_Y] = in[GMT_Y];	out[GMT_Z] = z_smt;
				out[3] = r_smt;	out[4] = (t_smt == 180.0) ? GMT->session.d_NaN : t_smt;
				record[0] = '\0';
				for (spot = 0; spot < n_max_spots; spot++) {
					if (Ctrl->Z.active)
						sprintf (buffer, fmt2, hot[spot].h->id, hot[spot].stage, hot[spot].np_time, hot[spot].np_dist);
					else
						sprintf (buffer, fmt2, hot[spot].h->abbrev, hot[spot].stage, hot[spot].np_time, hot[spot].np_dist);
					strcat (record, buffer);
				}
				Out->text = record;
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
		}

		gmt_M_free (GMT, c);
		smt++;
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Working on seamount # %5d\n", smt);

	gmt_M_free (GMT, Out);
	gmt_M_free (GMT, hotspot);
	gmt_M_free (GMT, orig_hotspot);
	gmt_M_free (GMT, hot);
	gmt_M_free (GMT, p);

	Return (GMT_NOERROR);
}
