/*-----------------------------------------------------------------
 *
 *      Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* x2sys_get will read the track index database and report all the tracks
 * that matches the specified geographical or data-type criteria given
 * on the command line.
 *
 * Author:	Paul Wessel
 * Date:	14-JUN-2004
 * Version:	1.1, based on the spirit of the old mgg code
 *		31-MAR-2006: Changed -X to -L to avoid GMT -X clash
 *		06-DEC-2007: -L did not honor -F -N settings
 *
 */

#include "gmt_dev.h"
#include "mgd77/mgd77.h"
#include "x2sys.h"

#define THIS_MODULE_CLASSIC_NAME	"x2sys_get"
#define THIS_MODULE_MODERN_NAME	"x2sys_get"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Get track listing from track index database"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RV"

struct X2SYS_GET_CTRL {
	struct S2S_GET_C {	/* -C */
		bool active;
	} C;
	struct S2S_GET_D {	/* -D */
		bool active;
	} D;
	struct S2S_GET_F {	/* -F */
		bool active;
		char *flags;
	} F;
	struct S2S_GET_G {	/* -G */
		bool active;
	} G;
	struct S2S_GET_L {	/* -L */
		bool active;
		int mode;
		char *file;
	} L;
	struct S2S_GET_N {	/* -N */
		bool active;
		char *flags;
	} N;
	struct S2S_GET_S {	/* -S */
		bool active;
	} S;
	struct S2S_GET_T {	/* -T */
		bool active;
		char *TAG;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_GET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_GET_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->L.mode = 1;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_GET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->F.flags);
	gmt_M_str_free (C->L.file);
	gmt_M_str_free (C->N.flags);
	gmt_M_str_free (C->T.TAG);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -T<TAG> [-C] [-D] [-F<fflags>] [-G] [-L[<list>][+i]] [-N<nflags>]\n\t[%s] [%s] [%s]\n\n", name, GMT_Rgeo_OPT, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Report center of each tile with tracks instead of track listing [Default is track files].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Only report the track names and skip the report for each field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Comma-separated list of column field names that must ALL be present [Default is any field].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Report global flags per track [Default reports for segments inside region].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Setup mode: Return all pairs of tracks that might intersect given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the bin distribution.  Optionally, give file with a list of tracks.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Then, only pairs with at least one track from the list is output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i to include internal pairs in the list [external only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Comma-separated list of column field names that ALL must be missing.\n");
	GMT_Option (API, "R");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   [Default region is the entire data domain].\n");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_GET_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k = 0, n_files = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Does not take input! */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: No input files expected\n");
				n_errors++;
				break;
			case '>':	/* Got named output file */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				break;
			case 'E':	/* Just accept and ignore (it was an option in GMT4 but the default in 5) */
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = true;
				break;
			case 'L':
				if (opt->arg[0] == '+') {k = 1; Ctrl->L.mode = 0;}
				else if ((c = strstr (opt->arg, "+i"))) {k = 0; Ctrl->L.mode = 0; c[0] = '\0';}
				if (opt->arg[k]) Ctrl->L.file = strdup (&opt->arg[k]);
				Ctrl->L.active = true;
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.flags = strdup (opt->arg);
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = true;	/* Undocumented swap option for index.b reading */
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: More than one output file given\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int find_leg (char *name, struct X2SYS_BIX *B, unsigned int n) {
	/* Return track id # for this leg */
	unsigned int i;
	
	for (i = 0; i < n; i++) if (B->head[i].trackname && !strcmp (name, B->head[i].trackname)) return (i);
	return (-1);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_get (void *V_API, int mode, void *args) {
	char *y_match = NULL, *n_match = NULL, line[GMT_BUFSIZ] = {""}, *p = NULL;
	
	uint64_t *ids_in_bin = NULL, ij, n_pairs, jj, kk, ID;
	uint32_t *in_bin_flag = NULL;   /* Match type in struct X2SYS_BIX_TRACK */
	uint32_t *matrix = NULL;        /* Needs to be a 32-bit unsigned int, not int */
	uint64_t row, col;
	
	double out[2];

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;
	struct X2SYS_BIX_TRACK *track = NULL;

	bool y_ok, n_ok, first, *include = NULL;
	int error = 0, i, j, k, start_j, start_i, stop_j, stop_i;
	unsigned int combo = 0, n_tracks_found, n_tracks, ii, cmode, wmode;
	unsigned int bit, missing = 0, id1, id2, item, n_flags = 0, ncols = 0;

	FILE *fp = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_OPTION *opt = NULL;
	struct X2SYS_GET_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) bailout (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the x2sys_get main code ----------------------------*/
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);
		
	if (s->geographic) {	/* Meaning longitude, latitude */
		gmt_set_geographic (GMT, GMT_OUT);
		GMT->current.io.geo.range = s->geodetic;
	}
	else	/* Cartesian data */
		gmt_set_cartesian (GMT, GMT_OUT);
		
	if (!GMT->common.R.active[RSET]) gmt_M_memcpy (GMT->common.R.wesn, B.wesn, 4, double);	/* Set default region to match TAG region */

	if (Ctrl->F.flags) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->F.flags, s), "-F");
	for (ii = combo = 0; ii < s->n_out_columns; ii++) combo |= X2SYS_bit (s->out_order[ii]);

	if (Ctrl->N.flags) {
		x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->N.flags, s), "-N");
		for (ii = missing = 0; ii < s->n_out_columns; ++ii)
			missing |= X2SYS_bit (s->out_order[ii]);
	}
	
	x2sys_bix_init (GMT, &B, false);

	/* Read existing track-information from <ID>_tracks.d file */

	x2sys_err_fail (GMT, x2sys_bix_read_tracks (GMT, s, &B, 1, &n_tracks), "");

	/* Read geographical track-info from <ID>_index.b file */

	x2sys_err_fail (GMT, x2sys_bix_read_index (GMT, s, &B, Ctrl->S.active), "");

	if (Ctrl->L.active) {
		n_flags = urint (ceil (n_tracks / 32.0));
		include = gmt_M_memory (GMT, NULL, n_tracks, bool);
		if (Ctrl->L.file) {
			if ((fp = fopen (Ctrl->L.file, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: -L unable to open file %s\n", Ctrl->L.file);
				Return (GMT_ERROR_ON_FOPEN);
			}
			while (fgets (line, GMT_BUFSIZ, fp)) {
				gmt_chop (line);	/* Get rid of [CR]LF */
				if (line[0] == '#' || line[0] == '\0') continue;
				if ((p = strchr (line, '.')) != NULL) line[(size_t)(p-line)] = '\0';	/* Remove extension */
				k = find_leg (line, &B, n_tracks);	/* Return track id # for this leg */
				if (k == -1) {
					GMT_Report (API, GMT_MSG_VERBOSE, "Leg %s not in the data base\n", line);
					continue;
				}
				include[k] = true;
			}
			fclose (fp);
		}
		else {	/* Use all */
			for (ii = 0; ii < n_tracks; ii++) include[ii] = true;
		}
		matrix = gmt_M_memory (GMT, NULL, n_tracks * n_flags + n_tracks / 32, uint32_t);
		ids_in_bin = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);
	}
	else {
		y_match = gmt_M_memory (GMT, NULL, n_tracks, char);
		n_match = gmt_M_memory (GMT, NULL, n_tracks, char);
	}
	in_bin_flag = gmt_M_memory (GMT, NULL, n_tracks, uint32_t);
	
	wmode = (Ctrl->C.active) ? GMT_IS_POINT : GMT_IS_TEXT;
	ncols = (Ctrl->C.active) ? 2 : 0;
	cmode = (Ctrl->C.active) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX;
	if (GMT_Init_IO (API, GMT_IS_DATASET, wmode, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		gmt_M_free (GMT, y_match);
		gmt_M_free (GMT, n_match);
		gmt_M_free (GMT, in_bin_flag);
		gmt_M_free (GMT, include);
		gmt_M_free (GMT, matrix);
		x2sys_end (GMT, s);
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		gmt_M_free (GMT, y_match);
		gmt_M_free (GMT, n_match);
		gmt_M_free (GMT, in_bin_flag);
		gmt_M_free (GMT, include);
		gmt_M_free (GMT, matrix);
		x2sys_end (GMT, s);
		Return (API->error);
	}
	GMT_Set_Columns (API, GMT_OUT, ncols, cmode);
	gmt_set_tableheader (GMT, GMT_OUT, true);	/* Turn on -ho explicitly */
	if (Ctrl->C.active)
		Out = gmt_new_record (GMT, out, NULL);	/* Only data output */
	else
		Out = gmt_new_record (GMT, NULL, line);	/* Only text output */
	
	/* Ok, now we can start finding the tracks requested */

	x2sys_err_fail (GMT, x2sys_bix_get_index (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &start_i, &start_j, &B, &ID), "");
	x2sys_err_fail (GMT, x2sys_bix_get_index (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &stop_i, &stop_j, &B, &ID), "");
	if (B.periodic && stop_i < start_i) stop_i += B.nx_bin;	/* Deal with longitude periodicity */

	for (j = start_j; j <= stop_j; j++) {
		for (i = start_i; i <= stop_i; i++) {
			ij = j * B.nx_bin + (i % B.nx_bin);	/* Since i may exceed nx_bin due to longitude periodicity */
			if (B.base[ij].n_tracks == 0) continue;

			for (jj = kk = 0, track = B.base[ij].first_track->next_track, first = true; first && jj < B.base[ij].n_tracks; jj++, track = track->next_track) {
				in_bin_flag[track->track_id] |= track->track_flag;	/* Build the total bit flag for this track INSIDE the region only */
				if (Ctrl->L.active) {	/* Just build integer list of track ids for this bin */
					y_ok = (Ctrl->F.flags) ? ((track->track_flag & combo) == combo) : true;		/* Each track must have the requested fields if -F is set */
					n_ok = (Ctrl->N.flags) ? ((track->track_flag & missing) == missing) : true;	/* Each track must have the missing fields */
					if (y_ok && n_ok) ids_in_bin[kk++] = track->track_id;
				}
				else {
					/* -F is straightforward: If at least one bin has all required cols then we flag the track to be reported */
					y_ok = (Ctrl->F.flags) ? ((track->track_flag & combo) == combo && y_match[track->track_id] == 0) : true;
					/* -N is less straightforward: We will skip it if any bin has any of the columns that all should be missing */
					n_ok = (Ctrl->N.flags) ? ((track->track_flag & missing) != 0 && n_match[track->track_id] == 0) : false;
					if (n_ok) n_match[track->track_id] = 1;
					if (y_ok) y_match[track->track_id] = 1;
					if (y_ok && !n_ok && Ctrl->C.active && first) {
						row = ij / B.nx_bin;	/* To hold the row number */
						col = ij % B.nx_bin;	/* To hold the col number */
						out[GMT_X] = B.wesn[XLO] + (col + 0.5) * B.inc[GMT_X];
						out[GMT_Y] = B.wesn[YLO] + (row + 0.5) * B.inc[GMT_Y];
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
						first = false;
					}
				}
			}
			if (Ctrl->L.active) {	/* Set bits for every possible pair, but exclude pairs not involving legs given */
				for (id1 = 0; id1 < kk; id1++) {
					for (id2 = id1 + 1; id2 < kk; id2++) {	/* Loop over all pairs */
						if (!(include[ids_in_bin[id1]] || include[ids_in_bin[id2]])) continue;	/* At last one leg must be from our list (if given) */
						/* This all requires matrix to be an in (32-bit) */
						item = (unsigned int)(ids_in_bin[id2] / 32);
						bit = (unsigned int)(ids_in_bin[id2] % 32);
						matrix[ids_in_bin[id1]*n_flags+item] |= (1 << bit);
						item = (unsigned int)(ids_in_bin[id1] / 32);
						bit = (unsigned int)(ids_in_bin[id1] % 32);
						matrix[ids_in_bin[id2]*n_flags+item] |= (1 << bit);
					}
				}
			}
				
		}
	}

	if (Ctrl->L.active) {
		for (id1 = 0, n_pairs = 0; id1 < n_tracks; id1++) {
			for (id2 = id1 + Ctrl->L.mode; id2 < n_tracks; id2++) {
				item = id2 / 32;
				bit = id2 % 32;
				if ((id2 > id1) && !(matrix[id1*n_flags+item] & (1 << bit))) continue;	/* Pair not selected */
				if (!B.head[id1].trackname || !B.head[id2].trackname) continue;	/* No such track in list */
				n_pairs++;
				/* OK, print out pair, with lega alphabetically lower than legb */
				if (strcmp (B.head[id1].trackname, B.head[id2].trackname) < 0)
					sprintf (line, "%s%s%s\n", B.head[id1].trackname, GMT->current.setting.io_col_separator, B.head[id2].trackname);
				else
					sprintf (line, "%s%s%s\n", B.head[id2].trackname, GMT->current.setting.io_col_separator, B.head[id1].trackname);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
		}
		gmt_M_free (GMT, matrix);
		gmt_M_free (GMT, include);
		gmt_M_free (GMT, ids_in_bin);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " pairs for crossover consideration\n", n_pairs);
	}
	else if (!Ctrl->C.active) {
		char text[GMT_LEN64] = {""};
		for (ii = n_tracks_found = 0; ii < n_tracks; ++ii) {
			if (y_match[ii] == 1 && n_match[ii] == 0)
				++n_tracks_found;
		}
		if (n_tracks_found) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %d tracks\n", n_tracks_found);

			if (!Ctrl->D.active) {
				sprintf (line, "Search command: %s", THIS_MODULE_CLASSIC_NAME);
				for (opt = options; opt; opt = opt->next) {
					(opt->option == GMT_OPT_INFILE) ? sprintf (text, " %s", opt->arg) : sprintf (text, " -%c%s", opt->option, opt->arg);
					strcat (line, text);
				}
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, line);
				sprintf (line, "track_ID%s", GMT->current.setting.io_col_separator);
				for (ii = 0; ii < (s->n_fields-1); ii++) {
					sprintf (text, "%s%s", s->info[ii].name, GMT->current.setting.io_col_separator);
					strcat (line, text);
				}
				strcat (line, s->info[s->n_fields-1].name);
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, line);
			}

			for (kk = 0; kk < n_tracks; kk++) {
				if (y_match[kk] == 0 || n_match[kk] == 1) continue;
				sprintf (line, "%s.%s", B.head[kk].trackname, s->suffix);
				if (!Ctrl->D.active) {
					for (ii = 0, bit = 1; ii < s->n_fields; ii++, bit <<= 1) {
						strcat (line, GMT->current.setting.io_col_separator);
						(((Ctrl->G.active) ? B.head[kk].flag : in_bin_flag[kk]) & bit) ? strcat (line, "Y") : strcat (line, "N");
					}
				}
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
		}
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "Search found no tracks\n");
	}
	
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	
	x2sys_bix_free (GMT, &B);

	gmt_M_free (GMT, Out);
	gmt_M_free (GMT, y_match);
	gmt_M_free (GMT, n_match);
	gmt_M_free (GMT, in_bin_flag);
	x2sys_end (GMT, s);

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "completed successfully\n");

	Return (GMT_NOERROR);
}
