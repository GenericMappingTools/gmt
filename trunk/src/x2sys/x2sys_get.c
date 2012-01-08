/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2012 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
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

#include "x2sys.h"

struct X2SYS_GET_CTRL {
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct F {	/* -F */
		GMT_LONG active;
		char *flags;
	} F;
	struct G {	/* -G */
		GMT_LONG active;
	} G;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;
		char *file;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		char *flags;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
};

void *New_x2sys_get_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_GET_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_GET_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->L.mode = 1;
	return (C);
}

void Free_x2sys_get_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_GET_CTRL *C) {	/* Deallocate control structure */
	if (C->F.flags) free (C->F.flags);
	if (C->L.file) free (C->L.file);
	if (C->N.flags) free (C->N.flags);
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_get_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "x2sys_get %s - Get track listing from track index database\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_get -T<TAG> [-C] [-F<fflags>] [-G] [-L[+][list]] [-N<nflags>] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Report center of each tile with tracks instead [Default is track files].\n");
	GMT_message (GMT, "\t-F Comma-separated list of column names that must ALL be present [Default is any field].\n");
	GMT_message (GMT, "\t-G Report global flags per track [Default reports for segments inside region].\n");
	GMT_message (GMT, "\t-L Setup mode: Return all pairs of cruises that might intersect given\n");
	GMT_message (GMT, "\t   the bin distribution.  Optionally, give file with a list of cruises.\n");
	GMT_message (GMT, "\t   Then, only pairs with at least one cruise from the list is output.\n");
	GMT_message (GMT, "\t   Use -L+ to include internal pairs in the list [external only].\n");
	GMT_message (GMT, "\t-N Comma-separated list of column names that ALL must be missing.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   [Default region is the entire data domain].\n");
	GMT_explain_options (GMT, "V");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_get_parse (struct GMTAPI_CTRL *C, struct X2SYS_GET_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = TRUE;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				break;
			case 'L':
				if (opt->arg[0] == '+') {k = 1; Ctrl->L.mode = 0;}
				if (opt->arg[k]) Ctrl->L.file = strdup (&opt->arg[k]);
				Ctrl->L.active = TRUE;
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.flags = strdup (opt->arg);
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;	/* Undocumented swap option for index.b reading */
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int find_leg (char *name, struct X2SYS_BIX *B, int n)
{	/* Return track id # for this leg */
	int i;
	
	for (i = 0; i < n; i++) if (!strcmp (name, B->head[i].trackname)) return (i);
	return (-1);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_get_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_x2sys_get (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	char *y_match = NULL, *n_match = NULL, line[GMT_BUFSIZ], *p = NULL;
	
	int *in_bin_flag = NULL;	/* Match type in struct X2SYS_BIX_TRACK */
	
	double x, y;

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;
	struct X2SYS_BIX_TRACK *track = NULL;

	GMT_LONG error = FALSE, no_suffix = FALSE, y_ok, n_ok, first;
	GMT_LONG combo = 0, ij, n_tracks_found, n_tracks, bit;
	GMT_LONG id1, id2, item, n_flags = 0, n_pairs, missing = 0;
	GMT_LONG i, j, k, kk, start_j, start_i, stop_j, stop_i;
	GMT_LONG *ids_in_bin = NULL, *include = NULL;
	
	unsigned int *matrix = NULL;	/* Needs to be a 32-bit unsigned int, not GMT_LONG */

	FILE *fp = NULL;
	struct GMT_OPTION *opt = NULL;
	struct X2SYS_GET_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) 
		return (GMT_x2sys_get_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) 
		return (GMT_x2sys_get_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_get", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VR", ">", options)) Return (API->error);
	Ctrl = New_x2sys_get_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_get_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_get main code ----------------------------*/

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (s->geographic) {
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
		GMT->current.io.geo.range = s->geodetic;
	}
	else
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;
		
	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, B.wesn, 4, double);	/* Set default region */

	if (Ctrl->F.flags) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->F.flags, s), "-F");
	for (i = combo = 0; i < s->n_out_columns; i++) combo |= X2SYS_bit (s->out_order[i]);

	if (Ctrl->N.flags) {
		x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->N.flags, s), "-N");
		for (i = missing = 0; i < s->n_out_columns; i++) missing |= X2SYS_bit (s->out_order[i]);
	}
	
	x2sys_bix_init (GMT, &B, FALSE);

	/* Read existing track-information from <ID>_tracks.d file */

	x2sys_err_fail (GMT, x2sys_bix_read_tracks (GMT, s, &B, 1, &n_tracks), "");

	/* Read geographical track-info from <ID>_index.b file */

	x2sys_err_fail (GMT, x2sys_bix_read_index (GMT, s, &B, Ctrl->S.active), "");

	if (Ctrl->L.active) {
		n_flags = (int)ceil (n_tracks / 32.0);
		include = GMT_memory (GMT, NULL, n_tracks, GMT_LONG);
		if (Ctrl->L.file) {
			if ((fp = fopen (Ctrl->L.file, "r")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: -L unable to open file %s\n", Ctrl->L.file);
				Return (EXIT_FAILURE);
			}
			while (fgets (line, GMT_BUFSIZ, fp)) {
				GMT_chop (GMT, line);	/* Get rid of [CR]LF */
				if (line[0] == '#' || line[0] == '\0') continue;
				if ((p = strchr (line, '.'))) line[(int)(p-line)] = '\0';	/* Remove extension */
				k = find_leg (line, &B, (int)n_tracks);	/* Return track id # for this leg */
				if (k == -1) {
					GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Leg %s not in the data base\n", line);
					continue;
				}
				include[k] = TRUE;
			}
			fclose (fp);
		}
		else {	/* Use all */
			for (k = 0; k < n_tracks; k++) include[k] = TRUE;
		}
		matrix = GMT_memory (GMT, NULL, n_tracks * n_flags, unsigned int);
		ids_in_bin = GMT_memory (GMT, NULL, n_tracks, GMT_LONG);
	}
	else {
		y_match = GMT_memory (GMT, NULL, n_tracks, char);
		n_match = GMT_memory (GMT, NULL, n_tracks, char);
	}
	in_bin_flag = GMT_memory (GMT, NULL, n_tracks, int);
	
	/* Ok, now we can start finding the tracks requested */

	x2sys_err_fail (GMT, x2sys_bix_get_ij (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &start_i, &start_j, &B, &j), "");
	x2sys_err_fail (GMT, x2sys_bix_get_ij (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &stop_i, &stop_j, &B, &j), "");
	if (B.periodic && stop_i < start_i) stop_i += B.nx_bin;	/* Deal with longitude periodicity */

	for (j = start_j; j <= stop_j; j++) {
		for (i = start_i; i <= stop_i; i++) {
			ij = j * B.nx_bin + (i % B.nx_bin);	/* Since i may exceed nx_bin due to longitude periodicity */
			if (B.base[ij].n_tracks == 0) continue;

			for (k = kk = 0, track = B.base[ij].first_track->next_track, first = TRUE; first && k < B.base[ij].n_tracks; k++, track = track->next_track) {
				in_bin_flag[track->track_id] |= track->track_flag;	/* Build the total bit flag for this cruise INSIDE the region only */
				if (Ctrl->L.active) {	/* Just build integer list of track ids for this bin */
					y_ok = n_ok = TRUE;
					y_ok = (Ctrl->F.flags) ? ((track->track_flag & combo) == combo) : TRUE;		/* Each cruise must have the requested fields if -F is set */
					n_ok = (Ctrl->N.flags) ? ((track->track_flag & missing) == missing) : TRUE;	/* Each cruise must have the missing fields */
					if (y_ok && n_ok) ids_in_bin[kk++] = track->track_id;
				}
				else {
					/* -F is straightforward: If at least one bin has all required cols then we flag the track to be reported */
					y_ok = (Ctrl->F.flags) ? ((track->track_flag & combo) == combo && y_match[track->track_id] == 0) : TRUE;
					/* -N is less straightforward: We will skip it if any bin has any of the columns that all should be missing */
					n_ok = (Ctrl->N.flags) ? ((track->track_flag & missing) != 0 && n_match[track->track_id] == 0) : FALSE;
					if (n_ok) n_match[track->track_id] = 1;
					if (y_ok) y_match[track->track_id] = 1;
					if (y_ok && !n_ok && Ctrl->C.active && first) {
						x = B.wesn[XLO] + ((ij % B.nx_bin) + 0.5) * B.inc[GMT_X];
						y = B.wesn[YLO] + ((ij / B.nx_bin) + 0.5) * B.inc[GMT_Y];
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x, GMT_X);
						fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y, GMT_Y);
						fprintf (GMT->session.std[GMT_OUT], "\n");
						first = FALSE;
					}
				}
			}
			if (Ctrl->L.active) {	/* Set bits for every possible pair, but exclude pairs not involving legs given */
				for (id1 = 0; id1 < kk; id1++) {
					for (id2 = id1 + 1; id2 < kk; id2++) {	/* Loop over all pairs */
						if (!(include[ids_in_bin[id1]] || include[ids_in_bin[id2]])) continue;	/* At last one leg must be from our list (if given) */
						/* This all requires matrix to be an in (32-bit) */
						item = ids_in_bin[id2] / 32;
						bit = ids_in_bin[id2] % 32;
						matrix[ids_in_bin[id1]*n_flags+item] |= (1 << bit);
						item = ids_in_bin[id1] / 32;
						bit = ids_in_bin[id1] % 32;
						matrix[ids_in_bin[id2]*n_flags+item] |= (1 << bit);
					}
				}
			}
				
		}
	}

	if (Ctrl->L.active) {
		for (id1 = n_pairs = 0; id1 < n_tracks; id1++) {
			for (id2 = id1 + Ctrl->L.mode; id2 < n_tracks; id2++) {
				item = id2 / 32;
				bit = id2 % 32;
				if ((id2 > id1) && !(matrix[id1*n_flags+item] & (1 << bit))) continue;	/* Pair not selected */
				n_pairs++;
				/* OK, print out pair, with lega alphabetically lower than legb */
				if (strcmp (B.head[id1].trackname, B.head[id2].trackname) < 0)
					printf ("%s\t%s\n", B.head[id1].trackname, B.head[id2].trackname);
				else
					printf ("%s\t%s\n", B.head[id2].trackname, B.head[id1].trackname);
			}
		}
		GMT_free (GMT, matrix);
		GMT_free (GMT, include);
		GMT_free (GMT, ids_in_bin);
		GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld pairs for crossover consideration\n", n_pairs);
	}
	else if (!Ctrl->C.active) {
		for (k = n_tracks_found = 0; k < n_tracks; k++) if (y_match[k] == 1 && n_match[k] == 0) n_tracks_found++;
		if (n_tracks_found) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld tracks\n", n_tracks_found);
	
			printf ("# Search command: %s", GMT->init.progname);
			for (opt = options; opt; opt = opt->next) (opt->option == GMTAPI_OPT_INFILE) ? printf (" %s", opt->arg) : printf (" -%c%s", opt->option, opt->arg);
			printf ("\n#track_ID\t");
			for (i = 0; i < (s->n_fields-1); i++) printf ("%s\t", s->info[i].name);
			printf ("%s\n", s->info[s->n_fields-1].name);
			for (k = 0; k < n_tracks; k++) {
				if (y_match[k] == 0 || n_match[k] == 1) continue;
				if (no_suffix) {
					for (i = strlen (B.head[k].trackname) - 1; i > 0 && B.head[k].trackname[i] != '.'; i--);
					if (i) B.head[k].trackname[i] = '\0';
				}
				printf ("%s", B.head[k].trackname);
				for (i = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) {
					(((Ctrl->G.active) ? B.head[k].flag : in_bin_flag[k]) & bit) ? printf ("\tY") : printf ("\tN");
				}
				printf ("\n");
			}
		}
		else
			GMT_report (GMT, GMT_MSG_NORMAL, "Search found no tracks\n");
	}
	
	GMT_free (GMT, y_match);
	GMT_free (GMT, n_match);
	x2sys_end (GMT, s);

	GMT_report (GMT, GMT_MSG_NORMAL, "completed successfully\n");

	Return (GMT_OK);
}
