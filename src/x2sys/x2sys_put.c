/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2015 by P. Wessel
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

#define THIS_MODULE_NAME	"x2sys_put"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Update track index database from track bin file"
#define THIS_MODULE_KEYS	""

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->V"

struct X2SYS_PUT_CTRL {
	struct In {	/* -In */
		bool active;
		char *file;
	} In;
	struct D {	/* -D */
		bool active;
	} D;
	struct F {	/* -F */
		bool active;
	} F;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T */
		bool active;
		char *TAG;
	} T;
};

void *New_x2sys_put_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_PUT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_PUT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_x2sys_put_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_PUT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

int GMT_x2sys_put_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_put [<info.tbf>] -T<TAG> [-D] [-F] [%s]\n\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t<info.tbf> is one track bin file from x2sys_binlist [Default reads stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for this compilation.\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-D Remove the listed tracks  [Default will add to database].\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-F Force updates to earlier entries for a track with new information.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   [Default refuses to process tracks already in the database].\n");
	GMT_Option (API, "V,.");
	
	return (EXIT_FAILURE);
}

int GMT_x2sys_put_parse (struct GMT_CTRL *GMT, struct X2SYS_PUT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				Ctrl->In.active = true;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Remove all traces of these tracks from the database */
				Ctrl->D.active = true;
				break;
			case 'F':	/* Force update of existing tracks if new data are found */
				Ctrl->F.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = true;	/* Swap option for index.b reading [Obsolete but left for backwardness] */
				break;
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->F.active, "Syntax error: Only specify one of -D and -F\n");
	
	if (Ctrl->F.active) Ctrl->D.active = true;	/* Ironic, given previous if-test, but that is how the logic below in the main */

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int x2sys_bix_remove_track (struct GMT_CTRL *GMT, uint32_t track_id, struct X2SYS_BIX *B)
{
	/* Remove all traces of the track with id track_id from structure tree for all bins */

	struct X2SYS_BIX_TRACK *track = NULL, *skip_track = NULL;
	uint64_t bin;

	for (bin = 0; bin < B->nm_bin; bin++) {
		if (B->base[bin].n_tracks == 0) continue;	/* No tracks crossed this bin */

		/* Start from anchor first_track (which is a dummy) and look if the next track in the list matches our ID */
		for (track = B->base[bin].first_track; track->next_track && track->next_track->track_id != track_id; track = track->next_track);	/* Finds the track or end-of-list */

		if (!track->next_track) continue;	/* Got end-of-list so not found in this bin; move on */
		
		/* Ok, found it. Remove it from this bin's list by moving pointer and freeing the memory */
		skip_track = track->next_track;			/* Get pointer to item to be removed */
		track->next_track = skip_track->next_track;	/* Bypass this item in the link */
		GMT_free (GMT, skip_track);			/* Remove memory associated with the track we removed */
		B->base[bin].n_tracks--;			/* One less entry for this bin */
		if (!track->next_track) B->base[bin].last_track = track;			/* Update the last track in case we just removed it */
		if (B->base[bin].n_tracks == 0) GMT_free (GMT, B->base[bin].first_track);	/* OK, that was the only track in this bin, apparently */
	}
	return (track_id);	/* Return the track id we passed in */
}

struct X2SYS_BIX_TRACK_INFO * x2sys_bix_find_track (char *track, bool *found_it, struct X2SYS_BIX *B)
{	/* Looks for given track in data base and if found returns pointer to the track before it and sets found_it to true.
	 * I.e., the track is actually this_info->next_info.  If not found set found_it to false and return pointer where
	 * this track should be inserted */
	
	struct X2SYS_BIX_TRACK_INFO *this_info;
	for (this_info = B->head; this_info->next_info && strcmp (this_info->next_info->trackname, track) < 0; this_info = this_info->next_info);
	*found_it = (this_info->next_info != NULL && !strcmp (this_info->next_info->trackname, track));
	return (this_info);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_put_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_put (void *V_API, int mode, void *args)
{
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;

	struct X2SYS_BIX_TRACK_INFO *this_info = NULL, *new_info = NULL;
	struct X2SYS_BIX_TRACK *this_track = NULL;

	char track[GMT_LEN64] = {""}, line[GMT_BUFSIZ] = {""};
	char track_file[GMT_BUFSIZ] = {""}, index_file[GMT_BUFSIZ] = {""}, old_track_file[GMT_BUFSIZ] = {""}, old_index_file[GMT_BUFSIZ] = {""};
	char track_path[GMT_BUFSIZ] = {""}, index_path[GMT_BUFSIZ] = {""}, old_track_path[GMT_BUFSIZ] = {""}, old_index_path[GMT_BUFSIZ] = {""};

	int error = 0;
	bool found_it, skip;

	FILE *fp = NULL, *fbin = NULL, *ftrack = NULL;

	uint32_t last_id, index, id, free_id, max_flag, flag, i, bit, total_flag; /* These must remain uint32_t */
	
	struct X2SYS_PUT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_x2sys_put_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) bailout (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) 
		bailout (GMT_x2sys_put_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) 
		bailout (GMT_x2sys_put_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_x2sys_put_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_put_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_put main code ----------------------------*/

	if (Ctrl->In.active && (fp = GMT_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not open file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);
	}
	if (fp == NULL) fp = GMT->session.std[GMT_IN];	/* No file given; read stdin instead */

	if (!GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {	/* Got the first record from the track binindex file */
		GMT_Report (API, GMT_MSG_NORMAL, "Read error in 1st line of track binindex file\n");
		Return (EXIT_FAILURE);
	}
	if (strncmp (&line[2], Ctrl->T.TAG, strlen(Ctrl->T.TAG))) {	/* Hard check to see if the TAG matches what we says it should be */
		GMT_Report (API, GMT_MSG_NORMAL, "The TAG specified (%s) does not match the one in the .tbf file (%s)\n", Ctrl->T.TAG, &line[2]);
		Return (EXIT_FAILURE);
	}

	/* Open TAG file and set the operational parameters */

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	for (i = max_flag = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) max_flag |= bit;

	x2sys_bix_init (GMT, &B, false);

	/* Read existing track-information from <ID>_tracks.d file */

	 x2sys_err_fail (GMT, x2sys_bix_read_tracks (GMT, s, &B, 0, &last_id), "");
	 last_id--;	/* Since last_id as returned is the number of IDs */

	/* Read geographical track-info from <ID>_index.b file */

	x2sys_err_fail (GMT, x2sys_bix_read_index (GMT, s, &B, Ctrl->S.active), "");

	/* Ok, now we can start reading new info */

	if (!GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Read error in 2nd line of track binindex file\n");
		Return (EXIT_FAILURE);
	}
	while (line[0] == '>') {	/* Next segment */
		sscanf (line, "> %s", track);

		/* Determine if this track is already in the data base */

		this_info = x2sys_bix_find_track (track, &found_it, &B);	/* Returns found_it = true if found */

		/* In either case, this_info now points to the previous track so that this_info->next is the found track OR
		 * it is the point after which a new track should be inserted */

		free_id = 0;	/* Default is to add a new track entry */
		if (found_it) {	/* This track already exists in the database */
			if (Ctrl->D.active) {	/* Here we wish to delete it (and possibly replace the contents) */
				GMT_Report (API, GMT_MSG_VERBOSE, "Removing existing information for track: %s\n", track);
				free_id = x2sys_bix_remove_track (GMT, this_info->next_info->track_id, &B);
				GMT_Report (API, GMT_MSG_VERBOSE, "track %s removed\n", track);
				this_info->next_info = this_info->next_info->next_info;
				skip = !Ctrl->F.active;	/* If we are not replacing the info then we skip the new info */
			}
			else {	/* Refuse to process tracks already in the database without the delete[and replace] options set */
				GMT_Report (API, GMT_MSG_VERBOSE, "Track already in database (skipped): %s\n", track);
				skip = true;
			}
		}
		else if (Ctrl->D.active) {	/* Here we did not found the track: Give message and go back and read next track information */
			if (!Ctrl->F.active) GMT_Report (API, GMT_MSG_VERBOSE, "track %s was not found in the database!\n", track);
			skip = !Ctrl->F.active;	/* If not found but -F is active then we just add this track as normal */
		}
		else	/* Get here when we wish to add a new track not in the database */
			skip = false;

		if (skip) {	/* Just wind past this segment */
			if (!GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Read error in a segment line of track binindex file\n");
				Return (EXIT_FAILURE);
			}
			while (line[0] != '>' && (GMT_fgets (GMT, line, GMT_BUFSIZ, fp) != NULL));	/* Keep reading until EOF of next segment header */
		}
		else {	/* Read the tbf information for this track */

			GMT_Report (API, GMT_MSG_VERBOSE, "Adding track: %s\n", track);

			/* If a track is replaced, then use the same id_no recover above, else increment to get a new one */

			id = (free_id) ? free_id : ++last_id;
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
					GMT_Report (API, GMT_MSG_NORMAL, "Error processing record for track %s [%s]\n", track, line);
					GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
				}
				else if (flag > max_flag) {
					GMT_Report (API, GMT_MSG_NORMAL, "data flag (%d) exceeds maximum (%d) for track %s!\n", flag, max_flag, track);
					GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
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

	/* Done, now we must rewrite the <ID>_index.b and <ID>_tracks.d files */

	sprintf (track_file, "%s/%s_tracks.d", Ctrl->T.TAG, Ctrl->T.TAG);
	sprintf (index_file, "%s/%s_index.b",  Ctrl->T.TAG, Ctrl->T.TAG);
	x2sys_path (GMT, track_file, track_path);
	x2sys_path (GMT, index_file, index_path);

	sprintf (old_track_file, "%s_old", track_file);
	sprintf (old_index_file, "%s_old", index_file);
	x2sys_path (GMT, old_track_file, old_track_path);
	x2sys_path (GMT, old_index_file, old_index_path);

	remove (old_track_path);	/* First delete old files */
	if (rename (track_path, old_track_path)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Rename failed for %s\t%s. Aborting %d!\n", track_path, old_track_path, i);
		Return (EXIT_FAILURE);
	}
	remove (old_index_path);	/* First delete old files */
	if (rename (index_path, old_index_path)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Rename failed for %s. Aborts!\n", index_path);
		Return (EXIT_FAILURE);
	}

	if ((ftrack = fopen (track_path, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to create %s. Aborts!\n", track_path);
		Return (EXIT_FAILURE);
	}
	if ((fbin = fopen (index_path, "wb")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to create %s. Aborts!\n", index_path);
		Return (EXIT_FAILURE);
	}
	fprintf (ftrack,"# %s\n", Ctrl->T.TAG);
	for (this_info = B.head->next_info; this_info; this_info = this_info->next_info)
		fprintf (ftrack,"%s %d %d\n",this_info->trackname, this_info->track_id, this_info->flag);

	fclose (ftrack);
	chmod (track_file, (mode_t)S_RDONLY);

	for (index = 0; index < B.nm_bin; index++) {
		if (B.base[index].n_tracks == 0) continue;

		if (fwrite (&index, sizeof (uint32_t), 1U, fbin) != 1U) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to write to binary file. Aborts!\n");
			Return (EXIT_FAILURE);
		}
		if (fwrite (&B.base[index].n_tracks, sizeof (uint32_t), 1U, fbin) != 1U) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to write to binary file. Aborts!\n");
			Return (EXIT_FAILURE);
		}
		for (this_track = B.base[index].first_track->next_track; this_track; this_track = this_track->next_track) {
			if (fwrite (&this_track->track_id, sizeof (uint32_t), 1U, fbin) != 1U) {
				GMT_Report (API, GMT_MSG_NORMAL, "Failed to write to binary file. Aborts!\n");
				Return (EXIT_FAILURE);
			}
			if (fwrite (&this_track->track_flag, sizeof (uint32_t), 1U, fbin) != 1U) {
				GMT_Report (API, GMT_MSG_NORMAL, "Failed to write to binary file. Aborts!\n");
				Return (EXIT_FAILURE);
			}
		}
	}
	fclose (fbin);
	chmod (index_file, (mode_t)S_RDONLY);

	GMT_Report (API, GMT_MSG_VERBOSE, "completed successfully\n");

	x2sys_end (GMT, s);

	Return (GMT_OK);
}
