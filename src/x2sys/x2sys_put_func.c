/*-----------------------------------------------------------------
 *	$Id: x2sys_put_func.c,v 1.6 2011-04-29 03:08:12 guru Exp $
 *
 *      Copyright (c) 1999-2011 by P. Wessel
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
/* x2sys_put will read one track bin file (.tbf) (from x2sys_binlist)
 * and update the track index database. The tbf file has the index
 * numbers of all the x by x degree boxes traversed by each data set.
 * The bin dimension and extent are determined by the tbf header.
 * This bin info is added to the track index database. This database
 * is then used by program x2sys_get that locates all the data track
 * within a given area that optionally contain specific data columns.
 *
 * Author:	Paul Wessel
 * Date:	14-JUN-2004
 * Version:	1.1, based on the spirit of the old mgg code
 *
 */

#include "x2sys.h"

struct X2SYS_PUT_CTRL {
	struct In {	/* -In */
		GMT_LONG active;
		char *file;
	} In;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
};

void *New_x2sys_put_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_PUT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_PUT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_x2sys_put_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_PUT_CTRL *C) {	/* Deallocate control structure */
	if (C->In.file) free ((void *)C->In.file);
	if (C->T.TAG) free ((void *)C->T.TAG);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_put_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;
	
	GMT_message (GMT, "x2sys_put %s - Update track index database from track bin file\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_put [<info.tbf>] -T<TAG> [-D] [-F] [%s]\n\n", GMT_V_OPT);
	GMT_message (GMT, "\t<info.tbf> is one track bin file from x2sys_binlist [Default reads stdin]\n");
	GMT_message (GMT, "\t-T <TAG> is the system tag for this compilation\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT,"\t-D will remove the listed tracks  [Default will add to database]\n");
	GMT_message (GMT,"\t-F will force updates to earlier entries for a track with new information\n");
	GMT_message (GMT,"\t   [Default refuses to process tracks already in the database]\n");
	GMT_explain_options (GMT, "V");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_put_parse (struct GMTAPI_CTRL *C, struct X2SYS_PUT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Remove all traces of these tracks from the database */
				Ctrl->D.active = TRUE;
				break;
			case 'F':	/* Force update of existing tracks if new data are found */
				Ctrl->F.active = TRUE;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;	/* Swap option for index.b reading [Obsolete but left for backwardness] */
				break;
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->F.active, "Syntax error: Only specify one of -D and -F\n");
	
	if (Ctrl->F.active) Ctrl->D.active = TRUE;	/* Ironic, given previous if-test, but that is how the logic below in the main */

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int x2sys_bix_remove_track (struct GMT_CTRL *GMT, int track_id, struct X2SYS_BIX *B)
{
	/* Remove all traces of the track with id track_id from structure tree for all bins */

	struct X2SYS_BIX_TRACK *track, *skip_track;
	int bin;

	for (bin = 0; bin < B->nm_bin; bin++) {
		if (B->base[bin].n_tracks == 0) continue;	/* No tracks crossed this bin */

		for (track = B->base[bin].first_track; track->next_track && track->next_track->track_id != track_id; track = track->next_track);	/* Finds the track or end-of-list */

		if (track->next_track) {	/* Ok, found it. Lets remove it from this bin's list */
			skip_track = track->next_track;	/* These 3 lines sets the next points to skip the item to be removed */
			track->next_track = skip_track->next_track;
			skip_track->next_track = NULL;
			B->base[bin].n_tracks--;	/* One less entry for this bin */
			if (!track->next_track) B->base[bin].last_track = track;	/* Update the last track in case we just removed it */
			GMT_free (GMT,  skip_track);	/* Remove memory associated with the track to be removed */
			if (B->base[bin].n_tracks == 0) GMT_free (GMT, B->base[bin].first_track);	/* OK, that was the only track in this bin, apparently */
		}
	}
	return (track_id);
}

struct X2SYS_BIX_TRACK_INFO * x2sys_bix_find_track (char *track, GMT_LONG *found_it, struct X2SYS_BIX *B)
{	/* Looks for given track in data base and if found returns pointer to the track before it and sets found_it to TRUE.
	 * I.e., the track is actually this_info->next_info.  If not found set found_it to FALSE and return pointer where
	 * this track should be inserted */
	
	struct X2SYS_BIX_TRACK_INFO *this_info;
	for (this_info = B->head; this_info->next_info && strcmp (this_info->next_info->trackname, track) < 0; this_info = this_info->next_info);
	*found_it = (this_info->next_info != (struct X2SYS_BIX_TRACK_INFO *)NULL && !strcmp (this_info->next_info->trackname, track));
	return (this_info);
}

#define Return(code) {Free_x2sys_put_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_x2sys_put (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;

	struct X2SYS_BIX_TRACK_INFO *this_info = NULL, *new_info = NULL;
	struct X2SYS_BIX_TRACK *this_track = NULL;

	char track[GMT_TEXT_LEN64], line[GMT_BUFSIZ], *c_unused = NULL;
	char track_file[GMT_BUFSIZ], index_file[GMT_BUFSIZ], old_track_file[GMT_BUFSIZ], old_index_file[GMT_BUFSIZ];
	char track_path[GMT_BUFSIZ], index_path[GMT_BUFSIZ], old_track_path[GMT_BUFSIZ], old_index_path[GMT_BUFSIZ];

	GMT_LONG error = FALSE, found_it, skip, last_id;

	FILE *fp = NULL, *fbin = NULL, *ftrack = NULL;

	/* Leave these as 4-byte ints */
	int index, id, bin, free_id, max_flag, flag;
	int i, bit, total_flag;
	
	size_t s_unused = 0;
	
	struct X2SYS_PUT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) 
		return (GMT_x2sys_put_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) 
		return (GMT_x2sys_put_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_put", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", ">", options))) Return (error);
	Ctrl = (struct X2SYS_PUT_CTRL *)New_x2sys_put_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_put_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_put main code ----------------------------*/

	if (Ctrl->In.active && (fp = GMT_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
		GMT_message (GMT, "Error: Could not open file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}
	if (fp == NULL) fp = GMT->session.std[GMT_IN];	/* No file given; read stdin instead */

	c_unused = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);	/* Got the first record from the track binindex file */
	if (strncmp (&line[2], Ctrl->T.TAG, strlen(Ctrl->T.TAG))) {	/* Hard check to see if the TAG matches what we says it should be */
		GMT_message (GMT, "The TAG specified (%s) does not match the one in the .tbf file (%s)\n", Ctrl->T.TAG, &line[2]);
		Return (EXIT_FAILURE);
	}

	/* Open TAG file and set the operational parameters */

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	for (i = max_flag = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) max_flag |= bit;

	x2sys_bix_init (GMT, &B, FALSE);

	/* Read existing track-information from <ID>_tracks.d file */

	 x2sys_err_fail (GMT, x2sys_bix_read_tracks (GMT, s, &B, 0, &last_id), "");
	 last_id--;

	/* Read geographical track-info from <ID>_index.b file */

	x2sys_err_fail (GMT, x2sys_bix_read_index (GMT, s, &B, Ctrl->S.active), "");

	/* Ok, now we can start reading new info */

#ifdef DEBUG
	GMT_memtrack_off (GMT, GMT_mem_keeper);
#endif
	c_unused = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
	while (line[0] == '>') {	/* Next segment */
		sscanf (line, "> %s", track);
		
		/* Determine if this track is already in the data base */
		
		this_info = x2sys_bix_find_track (track, &found_it, &B);	/* Returns found_it = TRUE if found */
		
		/* In either case, this_info now points to the previous track so that this_info->next is the found track OR
		 * it is the point after which a new track should be inserted */
		
		free_id = 0;	/* Default is to add a new track entry */
		if (found_it) {	/* This track already exists in the database */
			if (Ctrl->D.active) {	/* Here we wish to delete it (and possibly replace the contents) */
				GMT_report (GMT, GMT_MSG_NORMAL, "Removing existing information for track: %s\n", track);
				free_id = x2sys_bix_remove_track (GMT, (int)this_info->next_info->track_id, &B);
				GMT_message (GMT, "track %s removed\n", track);
				this_info->next_info = this_info->next_info->next_info;
				skip = !Ctrl->F.active;	/* If we are not replacing the info then we skip the new info */
			}
			else {	/* Refuse to process tracks already in the database without the delete[and replace] options set */
				GMT_report (GMT, GMT_MSG_NORMAL, "Track already in database (skipped): %s\n", track);
				skip = TRUE;
			}
		}
		else if (Ctrl->D.active) {	/* Here we did not found the track: Give message and go back and read next track information */
			GMT_message (GMT, "track %s was not found in the database!\n", track);
			skip = TRUE;
		}
		else	/* Get here when we wish to add a new track not in the database */
			skip = FALSE;

		if (skip) {	/* Just wind past this segment */
			c_unused = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
			while (line[0] != '>' && (GMT_fgets (GMT, line, GMT_BUFSIZ, fp) != NULL));
		}
		else {	/* Read the tbf information for this track */

			GMT_report (GMT, GMT_MSG_NORMAL, "Adding track: %s\n", track);

			/* If a track is replaced, then use the same id_no, else increment to get a new one */

			id = (free_id) ? free_id : (int)++last_id;
			if (!free_id) {	/* Must create a new entry */
				new_info = x2sys_bix_make_entry (GMT, track, id, 0);
				new_info->next_info = this_info->next_info;
				this_info->next_info = new_info;
				this_info = new_info;
			}
			else	/* Reuse the previously deleted entry */
				this_info = this_info->next_info;

			total_flag = 0;
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp) && line[0] != '>') {
				i = sscanf (line, "%*s %*s %d %d", &index, &flag);
				if (i != 2) {	/* Could not decode the index and the flag entries */
					GMT_message (GMT, "Error processing record for track %s [%s]\n", track, line);
					exit (EXIT_FAILURE);
				}
				else if (flag > max_flag) {
					GMT_message (GMT, "data flag (%d) exceed maximum (%d) for track %s!\n", flag, max_flag, track);
					exit (EXIT_FAILURE);
				}
				if (B.base[index].n_tracks == 0) {	/* First track to cross this bin */
					B.base[index].first_track = x2sys_bix_make_track (GMT, 0, 0);
					B.base[index].last_track = B.base[index].first_track;
				}
				B.base[index].last_track->next_track = x2sys_bix_make_track (GMT, id, flag);
				B.base[index].last_track = B.base[index].last_track->next_track;
				B.base[index].n_tracks++;
				total_flag |= flag;	/* Accumulate flags for the entire track */
			}
			this_info->flag = total_flag;	/* Store the track flags here */
		}
	}
	GMT_fclose (GMT, fp);
#ifdef DEBUG
	GMT_memtrack_on (GMT, GMT_mem_keeper);
#endif

	/* Done, now we must rewrite the <ID>_index.b and <ID>_tracks.d files */

	sprintf (track_file, "%s%c%s_tracks.d", Ctrl->T.TAG, DIR_DELIM, Ctrl->T.TAG);
	sprintf (index_file, "%s%c%s_index.b",  Ctrl->T.TAG, DIR_DELIM, Ctrl->T.TAG);
	x2sys_path (GMT, track_file, track_path);
	x2sys_path (GMT, index_file, index_path);

	sprintf (old_track_file, "%s_old", track_file);
	sprintf (old_index_file, "%s_old", index_file);
	x2sys_path (GMT, old_track_file, old_track_path);
	x2sys_path (GMT, old_index_file, old_index_path);

	remove (old_track_path);	/* First delete old files */
	if (rename (track_path, old_track_path)) {
		GMT_message (GMT, "Rename failed for %s\t%s. Aborting %d!\n", track_path, old_track_path, i);
		Return (EXIT_FAILURE);
	}
	remove (old_index_path);	/* First delete old files */
	if (rename (index_path, old_index_path)) {
		GMT_message (GMT, "Rename failed for %s. Aborts!\n", index_path);
		Return (EXIT_FAILURE);
	}

	if ((ftrack = fopen (track_path, "w")) == NULL) {
		GMT_message (GMT, "Failed to create %s. Aborts!\n", track_path);
		Return (EXIT_FAILURE);
	}
	if ((fbin = fopen (index_path, "wb")) == NULL) {
		GMT_message (GMT, "Failed to create %s. Aborts!\n", index_path);
		Return (EXIT_FAILURE);
	}
	fprintf (ftrack,"# %s\n", Ctrl->T.TAG);
	for (this_info = B.head->next_info; this_info; this_info = this_info->next_info)
		fprintf (ftrack,"%s %ld %ld\n",this_info->trackname, this_info->track_id, this_info->flag);

	fclose (ftrack);
	chmod (track_file, (mode_t)S_RDONLY);

	for (bin = 0; bin < B.nm_bin; bin++) {
		if (B.base[bin].n_tracks == 0) continue;

		s_unused = fwrite ((void *)(&bin), (size_t)4, (size_t)1, fbin);
		s_unused = fwrite ((void *)(&B.base[bin].n_tracks), (size_t)4, (size_t)1, fbin);
		for (this_track = B.base[bin].first_track->next_track; this_track; this_track = this_track->next_track) {
			s_unused = fwrite ((void *)(&this_track->track_id), (size_t)4, (size_t)1, fbin);
			s_unused = fwrite ((void *)(&this_track->track_flag), (size_t)4, (size_t)1, fbin);
		}
	}
	fclose (fbin);
	chmod (index_file, (mode_t)S_RDONLY);

	GMT_report (GMT, GMT_MSG_NORMAL, "completed successfully\n");

	x2sys_end (GMT, s);

	Return (GMT_OK);
}
