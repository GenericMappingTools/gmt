/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* Program:	gmt_remote.c
 * Purpose:	routines involved in handling remote files and tiles
 *
 * Author:	Paul Wessel
 * Date:	15-Sept-2017
 */

#include "gmt_dev.h"
#include "gmt_internals.h"
#include <curl/curl.h>
#ifdef WIN32
#include <sys/utime.h>
#endif

#define GMT_HASH_INDEX	0
#define GMT_INFO_INDEX	1

/* Copy a file from the GMT auto-download directory or the Internet.  We recognize
 * different types and names of files.
 * 1. There are data sets of use to all GMT users, such as global relief:
 * 	   @earth_relief_<res>.grd	Various global relief grids
 * We may add more data later but this is our start.
 * 2. Data sets only used to run an example or a test script
 * and these are all called @*, i.e., a '@' is pre-pended to the name.
 * They live in a cache subdirectory under the GMT_DATA_SERVER
 * and will be placed in a cache directory in the users ~/.gmt directory.
 * 3. Generic URLs starting with http:, https:, or ftp:  These will be
 * downloaded to the cache directory.
 * If auto-download is enabled and a requested input file matches these
 * names and not found by normal means then we download the file to the
 * user-directory.  All places that open files (GMT_Read_Data) will do
 * this check by first calling gmt_download_file_if_not_found.
 */

/* Need global variables for this black magic. Here is the problem:
 * When a user accesses a large remote file and there is a power-outage
 * or the user types Ctrl-C, the remote file is only partially copied
 * over and is useless.  In those cases we should make sure we delete
 * the incomplete file before killing ourselves.  Below is the implementation
 * for Linux/macOS that handles these cases.  The actual CURL calls
 * are bracketed by gmtremote_turn_on_ctrl_C_check and gmtremote_turn_off_ctrl_C_check which
 * temporarily activates or deactivates our signal action on Ctrl-C.
 * P. Wessel, June 30, 2019.
 */

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
#define GMT_CATCH_CTRL_C
#include <signal.h>
struct sigaction cleanup_action, default_action;
char *file_to_delete_if_ctrl_C;
#endif

struct LOCFILE_FP {
	char *file;	/* Pointer to file name */
	FILE *fp;	/* Open file pointer */
};

GMT_LOCAL void gmtremote_delete_file_then_exit (int sig_no) {
	/* If we catch a CTRL-C during CURL download we must assume file is corrupted and remove it before exiting */
	gmt_M_unused (sig_no);
#ifdef GMT_CATCH_CTRL_C
#ifdef DEBUG
	fprintf (stderr, "Emergency removal of file %s due to Ctrl-C action\n", file_to_delete_if_ctrl_C);
#endif
	remove (file_to_delete_if_ctrl_C);	   /* Remove if we can, ignore any returns */
	sigaction (SIGINT, &default_action, NULL); /* Reset the default action */
	kill (0, SIGINT);			   /* Perform the final Ctrl-C action and die */
#endif
}

GMT_LOCAL void gmtremote_turn_on_ctrl_C_check (char *file) {
#ifdef GMT_CATCH_CTRL_C
	file_to_delete_if_ctrl_C = file;			/* File to delete if CTRL-C is caught */
	gmt_M_memset (&cleanup_action, 1, struct sigaction);	/* Initialize the structure to NULL */
	cleanup_action.sa_handler = &gmtremote_delete_file_then_exit;	/* Set function we should call if CTRL-C is caught */
	sigaction(SIGINT, &cleanup_action, &default_action);	/* Activate the alternative signal checking */
#else
	gmt_M_unused (file);
#endif
}

GMT_LOCAL void gmtremote_turn_off_ctrl_C_check () {
#ifdef GMT_CATCH_CTRL_C
	file_to_delete_if_ctrl_C = NULL;		/* Remove trace of any file name */
	sigaction (SIGINT, &default_action, NULL);	/* Reset default signal action */
#endif
}

struct FtpFile {	/* Needed for argument to libcurl */
	const char *filename;	/* Name of file to write */
	FILE *fp;	/* File pointer to said file */
};

GMT_LOCAL size_t gmtremote_throw_away (void *ptr, size_t size, size_t nmemb, void *data) {
	gmt_M_unused (ptr);
	gmt_M_unused (data);
	/* We are not interested in the headers itself,
	   so we only return the file size we would have saved ... */
	return (size_t)(size * nmemb);
}

GMT_LOCAL size_t gmtremote_fwrite_callback (void *buffer, size_t size, size_t nmemb, void *stream) {
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out == NULL) return GMT_NOERROR;	/* This cannot happen but Coverity fusses */
	if (!out->fp) { /* Open file for writing */
		out->fp = fopen (out->filename, "wb");
		if (!out->fp)
			return -1; /* failure, can't open file to write */
	}
	return fwrite (buffer, size, nmemb, out->fp);
}

GMT_LOCAL int gmtremote_compare_names (const void *item_1, const void *item_2) {
	/* Compare function used to sort the GMT_DATA_INFO array of structures into alphabetical order */
	const char *name_1 = ((struct GMT_DATA_INFO *)item_1)->file;
	const char *name_2 = ((struct GMT_DATA_INFO *)item_2)->file;

	return (strcmp (name_1, name_2));
}

GMT_LOCAL int gmtremote_parse_version (char *line) {
	/* Parse a line like "# 6.1.0 or later GMT version required" and we will make no
	 * assumptions about how much space before the version. */
	int k = 1, start, major, minor, release;
	char text[GMT_LEN64] = {""};
	if (line[0] != '#') return 1;	/* Not a comment record! */
	strncpy (text, line, GMT_LEN64-1);
	while (isspace (text[k])) k++;	/* Skip until we get to the version */
	start = k;
	while (isdigit(text[k]) || text[k] == '.') k++;	/* Wind to end of version */
	text[k] = '\0';	/* Chop off the rest */
	if (sscanf (&text[start], "%d.%d.%d", &major, &minor, &release) != 3) return 1;
	if (major > GMT_MAJOR_VERSION) return 2;	/* Definitively too old */
	if (major < GMT_MAJOR_VERSION) return 0;	/* Should be fine */
	if (minor > GMT_MINOR_VERSION) return 2;	/* Definitively too old */
	if (minor < GMT_MINOR_VERSION) return 0;	/* Should be fine */
	if (release > GMT_RELEASE_VERSION) return 2;	/* Definitively too old */
	return GMT_NOERROR;
}

GMT_LOCAL int gmtremote_remove_item (struct GMTAPI_CTRL *API, char *path, bool directory) {
	int error = GMT_NOERROR;
	if (directory) {	/* Delete populated directories via an operating system remove call */
		char del_cmd[PATH_MAX] = {""};
#ifdef _WIN32
		char *t = gmt_strrep (path, "/", "\\");	/* DOS rmdir needs paths with back-slashes */
		strcpy (del_cmd, "rmdir /s /q ");
		strncat (del_cmd, t, PATH_MAX-1);
		gmt_M_str_free (t);
#else
		sprintf (del_cmd, "rm -rf %s", path);
#endif
		if ((error = system (del_cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to remove %s [error = %d]\n", path, error);
			error = GMT_RUNTIME_ERROR;
		}
	}
	else	/* Just delete a single file  */
		gmt_remove_file (API->GMT, path);
	return error;
}

GMT_LOCAL struct GMT_DATA_INFO *gmtremote_data_load (struct GMTAPI_CTRL *API, int *n) {
	/* Read contents of the info file into an array of structs */
	int k = 0, nr;
	FILE *fp = NULL;
	struct GMT_DATA_INFO *I = NULL;
	char unit, line[GMT_LEN512] = {""}, file[PATH_MAX] = {""}, *c = NULL;
	struct GMT_CTRL *GMT = API->GMT;

	snprintf (file, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, GMT_INFO_SERVER_FILE);

	GMT_Report (API, GMT_MSG_DEBUG, "Load contents from %s\n", file);
	*n = 0;
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to open file %s\n", file);
		return NULL;
	}
	if (fgets (line, GMT_LEN256, fp) == NULL) {	/* Try to get first record */
		fclose (fp);
		GMT_Report (API, GMT_MSG_ERROR, "Read error first record in file %s\n", file);
		GMT_Report (API, GMT_MSG_ERROR, "Deleting %s so it can get regenerated - please try again\n", file);
		gmt_remove_file (GMT, file);
		return NULL;
	}
	*n = atoi (line);		/* Number of non-commented records to follow */
	if (*n <= 0 || *n > GMT_BIG_CHUNK) {	/* Probably not a good value */
		fclose (fp);
		GMT_Report (API, GMT_MSG_ERROR, "Bad record counter in file %s\n", file);
		GMT_Report (API, GMT_MSG_ERROR, "Deleting %s so it can get regenerated - please try again\n", file);
		gmt_remove_file (GMT, file);
		return NULL;
	}
	if (fgets (line, GMT_LEN256, fp) == NULL) {	/* Try to get second record */
		fclose (fp);
		GMT_Report (API, GMT_MSG_ERROR, "Read error second record in file %s\n", file);
		return NULL;
	}
	if ((k = gmtremote_parse_version (line))) {
		fclose (fp);
		if (k == 2)
			GMT_Report (API, GMT_MSG_NOTICE, "Your GMT version too old to use the remote data mechanism - please upgrade to %s or later\n", line);
		else
			GMT_Report (API, GMT_MSG_ERROR, "Unable to parse \"%s\" to extract GMT version\n", line);
		return NULL;
	}	
	if ((I = gmt_M_memory (GMT, NULL, *n, struct GMT_DATA_INFO)) == NULL) {
		fclose (fp);
		GMT_Report (API, GMT_MSG_ERROR, "Unable to allocated %d GMT_DATA_INFO structures!\n", *n);
		return NULL;
	}

	while (fgets (line, GMT_LEN512, fp) != NULL) {
		if (line[0] == '#') continue;	/* Skip any comments */
		if ((nr = sscanf (line, "%s %s %s %c %lg %lg %s %lg %s %s %s %s %[^\n]", I[k].dir, I[k].file, I[k].inc, &I[k].reg, &I[k].scale, &I[k].offset, I[k].size, &I[k].tile_size, I[k].date, I[k].coverage, I[k].filler, I[k].CPT, I[k].remark)) != 13) {
			GMT_Report (API, GMT_MSG_WARNING, "File %s should have 13 fields but only %d read for record %d - download error???\n", file, nr, k);
			gmt_M_free (GMT, I);
			fclose (fp);
			return NULL;
		}
		/* Extract some useful bits to have in separate variables */
		sscanf (I[k].inc, "%lg%c", &I[k].d_inc, &unit);
		if (unit == 'm') I[k].d_inc *= GMT_MIN2DEG;		/* E.g., 30m becomes 0.5 */
		else if (unit == 's') I[k].d_inc *= GMT_SEC2DEG;	/* E.g., 30s becomes 0.00833333333333 */
		if ((c = strchr (I[k].file, '.')))	/* Get the file extension */
			strcpy (I[k].ext, c);
		if (I[k].tile_size > 0.0) {	/* A tiled dataset */
			size_t len = strlen (I[k].file);
			strncpy (I[k].tag, I[k].file, len-1);	/* Remote trailing slash */
		}
		k++;
	}
	fclose (fp);

	if (k != *n) {
		GMT_Report (API, GMT_MSG_WARNING, "File %s said it has %d records but only found %d - download error???\n", file, *n, k);
		GMT_Report (API, GMT_MSG_WARNING, "File %s should be deleted.  Please try again\n", file);
		*n = 0;	/* Flag that excrement hit the fan */
	}
	/* Soft alphabetically on file names */
	qsort (I, *n, sizeof (struct GMT_DATA_INFO), gmtremote_compare_names);
	for (k = 0; k < *n; k++) I[k].id = k;	/* Give running number as ID in the sorted array */

	if (GMT->current.io.new_data_list) {	/* Take this opportunity to delete datasets that are past their expiration date */
		time_t mod_time;
		struct tm *UTC = NULL;
		struct stat buf;
		int year, month, day, kyear, kmonth, kday;
		size_t L;

		GMT->current.io.new_data_list = false;	/* We only do this once after a gmt_data_server.txt update */
		if (GMT->session.USERDIR == NULL) goto out_of_here;	/* Cannot have server data if no user directory is set */
		if (access (GMT->session.USERDIR, R_OK)) goto out_of_here;	/* Set, but have not made a user directory yet, so cannot have any remote data yet either */

		for (k = 0; k < *n; k++) {	/* Check the release date of each data set that has been downloaded against the local file date */
			if (sscanf (I[k].date, "%d-%d-%d", &kyear, &kmonth, &kday) != 3) continue;	/* Maybe malformed datestring or on purpose to never check */
			snprintf (file, PATH_MAX, "%s/%s%s", GMT->session.USERDIR, I[k].dir, I[k].file);	/* Local path, may end in slash if a tile directory*/
			if ((L = strlen (file) - 1) && file[L] == '/') file[L] = '\0';	/* Chop off trailing / that indicates directory of tiles */
			if (access (file, R_OK)) continue;	/* No such file or directory yet */
			/* Here we have a local copy of this remote file or directory - we examine its creation date */
			if (stat (file, &buf)) {
				GMT_Report (API, GMT_MSG_WARNING, "Unable to get information about %s - skip\n", file);
				continue;
			}
			/*  Get its modification (creation) time */
#ifdef __APPLE__
			mod_time = buf.st_mtimespec.tv_sec;	/* Apple even has tv_nsec for nano-seconds... */
#else
			mod_time = buf.st_mtime;
#endif
			/* Extract the year, month, day integers */
			UTC   = gmtime (&mod_time);
			year  = UTC->tm_year + 1900;	/* Yep, how stupid is that, Y2K lovers. I guess 2030 might overflow a 32-bit int... */
			month = UTC->tm_mon + 1;		/* Make it 1-12 since it is 0-11 */
			day   = UTC->tm_mday;			/* Yep, lets start at 1 for days and 0 for months, makes sense */
			if (kyear < year) continue;	/* The origin year is older than our file so no need to check further */
			if (kyear == year) {	/* Our file and the server file is both from the same year */
				if (kmonth < month) continue;	/* The origin month is older than our copy so no need to check further */
				if (kmonth == month) {	/* Same year, same month, we are so close! */
					if (kday < day) continue; 	/* The origin day is older than our copy so no need to check further */
				}
			}
			/* If we get here we need to remove the outdated file or directory so we may download the latest on next try */
			if (gmtremote_remove_item (API, file, I[k].tile_size > 0.0)) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to remove %s \n", file);
			}
		}
	}

out_of_here:
	return (I);
};

GMT_LOCAL int gmtremote_compare_key (const void *item_1, const void *item_2) {
	/* We are passing string item_1 without any leading @ */
	const char *name_1 = (char *)item_1;
	const char *name_2 = ((struct GMT_DATA_INFO *)item_2)->file;
	size_t len = strlen (name_1);

	return (strncmp (name_1, name_2, len));
}

int gmtremote_wind_to_file (const char *file) {
	int k = strlen (file) - 2;	/* This jumps past any trailing / for tiles */
	while (k >= 0 && file[k] != '/') k--;
	return (k+1);
}

int gmt_remote_no_resolution_given (struct GMTAPI_CTRL *API, const char *rfile, int *registration) {
	/* Return first entry to a list of different resolutions for the
	 * same data set. For instance, if file is "earth_relief" then we
	 * return the ID to the first one listed. */
	char *c = NULL, *p = NULL, dir[GMT_LEN64] = {""}, file[GMT_LEN128] = {""};
	int ID = GMT_NOTSET, reg = GMT_NOTSET;
	size_t L;

	if (rfile == NULL || rfile[0] == '\0') return GMT_NOTSET;	/* No file name given */
	if (rfile[0] != '@') return GMT_NOTSET;	/* No remote file name given */
	strcpy (file, &rfile[1]);	/* Make a copy but skipping leading @ character */
	if ((c = strchr (file, '+'))) c[0] = '\0';	/* Chop of modifiers such as in grdimage -I */
	L = strlen (file);
	if (!strncmp (&file[L-2], "_g", 2U)) {	/* Want a gridline-registered version */
		reg = GMT_GRID_NODE_REG;
		file[L-2] = '\0';
	}
	else if (!strncmp (&file[L-2], "_p", 2U)) {	/* Want a pixel-registered version */
		reg = GMT_GRID_PIXEL_REG;
		file[L-2] = '\0';
	}
	for (int k = 0; ID == GMT_NOTSET && k < API->n_remote_info; k++) {
		strncpy (dir, API->remote_info[k].dir, strlen (API->remote_info[k].dir)-1);	/* Make a copy without the trailing slash */
		p = strrchr (dir, '/');	/* Start of final subdirectory */
		p++;	/* Skip past the slash */
		if (!strcmp (p, file)) ID = k;
	}
	if (ID != GMT_NOTSET && registration)
		*registration = reg;	/* Pass back desired [or any] registration */

	return (ID);	/* Start of the family or -1 */
}

struct GMT_RESOLUTION *gmt_remote_resolutions (struct GMTAPI_CTRL *API, const char *rfile, unsigned int *n) {
	/* Return list of available resolutions and registrations for the specified data set family.
	 * For instance, if file is "earth_relief" then we return an array of structs
	 * with the resolution and registration from 01d (g&p) to 91s (g) .*/
	char *c = NULL, *p = NULL, dir[GMT_LEN64] = {""}, file[GMT_LEN128] = {""};
	static char *registration = "gp";	/* The two types of registrations */
	int id = 0, reg = GMT_NOTSET;
	size_t L, n_alloc = GMT_SMALL_CHUNK;
	struct GMT_RESOLUTION *R = NULL;

	if (rfile == NULL || rfile[0] == '\0') return NULL;	/* No file name given */
	if (rfile[0] != '@') return NULL;	/* No remote file name given */
	strcpy (file, &rfile[1]);	/* Make a copy but skipping leading @ character */
	if ((c = strchr (file, '+'))) c[0] = '\0';	/* Chop of modifiers such as in grdimage -I */
	L = strlen (file);
	if (!strncmp (&file[L-2], "_g", 2U)) {	/* Want a gridline-registered version */
		reg = GMT_GRID_NODE_REG;
		file[L-2] = '\0';
	}
	else if (!strncmp (&file[L-2], "_p", 2U)) {	/* Want a pixel-registered version */
		reg = GMT_GRID_PIXEL_REG;
		file[L-2] = '\0';
	}
	if ((R = gmt_M_memory (API->GMT, NULL, n_alloc, struct GMT_RESOLUTION)) == NULL)
		return NULL;	/* No memory */

	for (int k = 0; k < API->n_remote_info; k++) {
		strncpy (dir, API->remote_info[k].dir, strlen (API->remote_info[k].dir)-1);	/* Make a copy without the trailing slash */
		p = strrchr (dir, '/');	/* Start of final subdirectory */
		p++;	/* Skip past the slash */
		if (!strcmp (p, file) && (reg == GMT_NOTSET || registration[reg] == API->remote_info[k].reg)) {	/* Got one to keep */
			R[id].resolution = urint (1.0 / API->remote_info[k].d_inc);	/* Number of nodes per degree */
			strncpy (R[id].inc, API->remote_info[k].inc, GMT_LEN8);	/* Copy the formatted inc string */
			R[id].reg = API->remote_info[k].reg;	/* Copy the registration */
			id++;
		}
		if (id == n_alloc) {	/* Need more memory */
			n_alloc += GMT_SMALL_CHUNK;
			if ((R = gmt_M_memory (API->GMT, NULL, n_alloc, struct GMT_RESOLUTION)) == NULL)
				return NULL;	/* No memory */
		}
	}
	if (id) {	/* Did find some */
		if ((R = gmt_M_memory (API->GMT, R, id, struct GMT_RESOLUTION)) == NULL)
			return NULL;	/* No memory */	
		*n = id;
	}
	else {	/* No luck, probably filename typo */
		gmt_M_free (API->GMT, R);
		*n = 0;
	}

	return (R);	
}

int gmt_remote_dataset_id (struct GMTAPI_CTRL *API, const char *ifile) {
	/* Return the entry in the remote file table of file is found, else -1.
	 * Complications to consider before finding a match:
	 * Input file may or may not have leading @
	 * Input file may or may not have _g or _p for registration
	 * Input file may or many not have an extension
	 * Key file may be a tiled data set and thus ends with '/'
	 */
	int pos = 0;
	char file[PATH_MAX] = {""};
	struct GMT_DATA_INFO *key = NULL;
	if (ifile == NULL || ifile[0] == '\0') return GMT_NOTSET;	/* No file name given */
	if (ifile[0] == '@') pos = 1;	/* Skip any leading remote flag */
	/* Exclude leading directory for local saved versions of the file */
	if (pos == 0) pos = gmtremote_wind_to_file (ifile);	/* Skip any leading directories */
	/* Must handle the use of srtm_relief vs earth_relief for the 01s and 03s data */
	if (strncmp (&ifile[pos], "srtm_relief_0", 13U) == 0)	/* Gave strm special name */
		sprintf (file, "earth_%s", &ifile[pos+5]);	/* Replace srtm with earth */
	else	/* Just copy as is from pos */
		strcpy (file, &ifile[pos]);
	key = bsearch (file, API->remote_info, API->n_remote_info, sizeof (struct GMT_DATA_INFO), gmtremote_compare_key);
	if (key) {	/* Make sure we actually got a real hit since file = "earth" will find a key starting with "earth****" */
		char *ckey = strrchr (key->file, '.');		/* Find location of the start of the key file extension (or NULL if no extension) */
		char *cfile = strrchr (file, '.');	/* Find location of the start of the input file extension (or NULL if no extension) */
		size_t Lfile = (cfile) ? (size_t)(cfile - file) : strlen (file);	/* Length of key file name without extension */
		size_t Lkey  = (ckey)  ? (size_t)(ckey  - key->file)  : strlen (key->file);		/* Length of key file name without extension */
		if (ckey == NULL && Lkey > 1 && key->file[Lkey-1] == '/') Lkey--;	/* Skip trailing dir flag */
		if (Lkey > Lfile && Lkey > 2 && key->file[Lkey-2] == '_' && strchr ("gp", key->file[Lkey-1])) Lkey -= 2;	/* Remove the length of _g or _p from Lkey */
		if (Lfile != Lkey)	/* Not an exact match (apart from trailing _p|g) */
			key = NULL;
	}
	return ((key == NULL) ? GMT_NOTSET : key->id);
}

bool gmt_file_is_cache (struct GMTAPI_CTRL *API, const char *file) {
	/* Returns true if a remote file is a cache file */
	if (file == NULL || file[0] == '\0') return false;	/* Nothing given */
	if (gmt_M_file_is_memory (file)) return false;	/* Memory files are not remote */
	if (file[0] != '@') return false;	/* Cannot be a remote file, let alone cache */
	if (gmt_remote_dataset_id (API, file) != GMT_NOTSET) return false;	/* Found a remote dataset, but not cache */
	return true;
}

void gmt_set_unspecified_remote_registration (struct GMTAPI_CTRL *API, char **file_ptr) {
	/* If a remote file is missing _g or _p we find which one we should use and revise the file accordingly.
	 * There are a few different scenarios where this can happen:
	 * 1. Users of GMT <= 6.0.0 are used to say earth_relief_01m. These will now get p.
	 * 2. Users who do not care about registration.  If so, they get p if available. */
	char newfile[GMT_LEN256] = {""}, reg[2] = {'p', 'g'}, *file = NULL, *infile = NULL, *ext = NULL, *c = NULL;
	int k_data, k;
	if (file_ptr == NULL || (file = *file_ptr) == NULL || file[0] == '\0') return;
	if (gmt_M_file_is_memory (file)) return;	/* Not a remote file for sure */
	if (file[0] != '@') return;
	infile = strdup (file);
	if ((c = strchr (infile, '+')))	/* Got modifiers, probably from grdimage or similar, chop off for now */
		c[0] = '\0';
	/* Deal with any extension the user may have added */
	ext = gmt_chop_ext (infile);
	/* If the remote file is found then there is nothing to do */
	if ((k_data = gmt_remote_dataset_id (API, infile)) == GMT_NOTSET) goto clean_up;
	if (strstr (file, "_p") || strstr (file, "_g")) goto clean_up;	/* Already have the registration codes */
	for (k = 0; k < 2; k++) {
		/* First see if this _<reg> version exists of this dataset */
		sprintf (newfile, "%s_%c", infile, reg[k]);
		if ((k_data = gmt_remote_dataset_id (API, newfile)) != GMT_NOTSET) {
			/* Found, replace given file name with this */
			if (c) {	/* Restore the modifiers */
				c[0] = '+';
				if (gmt_found_modifier (API->GMT, c, "os"))
					GMT_Report (API, GMT_MSG_WARNING, "Cannot append +s<scl> and/or +o<offset> to the remote global grid %s - ignored\n", newfile);
				else
					strcat (newfile, c);
			}
			gmt_M_str_free (*file_ptr);
			*file_ptr = strdup (newfile);
			goto clean_up;
		}
	}
clean_up:
	gmt_M_str_free (infile);
}

int gmt_remote_no_extension (struct GMTAPI_CTRL *API, const char *file) {
	int k_data = gmt_remote_dataset_id (API, file);
	if (k_data == GMT_NOTSET) return GMT_NOTSET;
	if (API->remote_info[k_data].ext[0] == '\0') return GMT_NOTSET;	/* Tiled grid */
	if (strstr (file, API->remote_info[k_data].ext)) return GMT_NOTSET;	/* Already has extension */
	return k_data;	/* Missing its extension */
}

GMT_LOCAL void gmtremote_display_attribution (struct GMTAPI_CTRL *API, int key, const char *file, int tile) {
	/* Display a notice regarding the source of this data set */
	char *c = NULL, name[GMT_LEN128] = {""};
	if (key != GMT_NOTSET && !API->server_announced && !strchr (file, ':')) {	/* Server file has no http:// here */
		if ((c = strrchr (API->GMT->session.DATASERVER, '/')))	/* Found last slash in http:// */
			strcpy (name, ++c);
		else /* Just in case */
			strncpy (name, gmt_dataserver_url (API), GMT_LEN128-1);
		if ((c = strchr (name, '.'))) c[0] = '\0';	/* Chop off stuff after the initial name */
		gmt_str_toupper (name);
		GMT_Report (API, GMT_MSG_NOTICE, "Remote data courtesy of GMT data server %s [%s]\n\n", API->GMT->session.DATASERVER, gmt_dataserver_url (API));
		API->server_announced = true;
	}
	if (key == GMT_NOTSET) {	/* A Cache file */
		if (strchr (file, ':'))	/* Generic URL */
			GMT_Report (API, GMT_MSG_INFORMATION, "  -> Download URL file: %s\n", file);
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "  -> Download cache file: %s\n", file);
	}
	else {	/* Remote data sets */
		if (!API->remote_info[key].used) {
			GMT_Report (API, GMT_MSG_NOTICE, "%s.\n", API->remote_info[key].remark);
			API->remote_info[key].used = true;
		}
		if (tile) {	/* Temporarily remote the trailing slash when printing the dataset name */
			c = strrchr (API->remote_info[key].file, '/');
			c[0] = '\0';
			strncpy (name, &file[1], 7U);	name[7] = '\0';
			GMT_Report (API, GMT_MSG_NOTICE, "  -> Download %lgx%lg degree grid tile (%s): %s\n",
					API->remote_info[key].tile_size, API->remote_info[key].tile_size, API->remote_info[key].file, name);
			c[0] = '/';
		}
		else
			GMT_Report (API, GMT_MSG_NOTICE, "  -> Download grid file [%s]: %s\n", API->remote_info[key].size, API->remote_info[key].file);
	}
}

GMT_LOCAL int gmtremote_find_and_give_data_attribution (struct GMTAPI_CTRL *API, const char *file) {
	/* Print attribution when the @remotefile is downloaded for the first time */
	char *c = NULL;
	int match, tile = 0;

	if (file == NULL || file[0] == '\0') return GMT_NOTSET;	/* No file name given */
	if ((c = strstr (file, ".grd")) || (c = strstr (file, ".tif")))	/* Ignore extension in comparison */
		c[0] = '\0';
	if ((match = gmt_remote_dataset_id (API, file)) == GMT_NOTSET) {	/* Check if it is a tile */
		if ((match = gmt_file_is_a_tile (API, file, GMT_LOCAL_DIR)) != GMT_NOTSET)	/* Got a remote tile */
			tile = 1;
	}
	gmtremote_display_attribution (API, match, file, tile);
	if (c) c[0] = '.';	/* Restore extension */
	return (match);
}

GMT_LOCAL char *gmtremote_lockfile (struct GMT_CTRL *GMT, char *file) {
	/* Create a dummy file in temp with extension .download and use as a lock file */
	char *c = strrchr (file, '/');
	char Lfile[PATH_MAX] = {""};
	if (c)	/* Found the last slash, skip it */
		c++;
	else	/* No path, just point to file */
		c = file;
	if (c[0] == '@') c++;	/* Skip any leading @ sign */
	sprintf (Lfile, "%s/%s.download", GMT->parent->tmp_dir, c);
	return (strdup (Lfile));
}

GMT_LOCAL size_t gmtremote_skip_large_files (struct GMT_CTRL *GMT, char * URL, size_t limit) {
	/* Get the remote file's size and if too large we refuse to download */
	CURL *curl = NULL;
	CURLcode res;
	double filesize = 0.0;
	size_t action = 0;

	if (limit == 0) return 0;	/* Download regardless of size */
	curl_global_init (CURL_GLOBAL_DEFAULT);

	if ((curl = curl_easy_init ())) {
		curl_easy_setopt (curl, CURLOPT_URL, URL);
		/* Do not download the file */
		curl_easy_setopt (curl, CURLOPT_NOBODY, 1L);
		/* Tell libcurl to not verify the peer */
		curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);
		/* Tell libcurl to fail on 4xx responses (e.g. 404) */
		curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1L);
		/* Tell libcurl to follow 30x redirects */
		curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
		/* No header output: TODO 14.1 http-style HEAD output for ftp */
		curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, gmtremote_throw_away);
		curl_easy_setopt (curl, CURLOPT_HEADER, 0L);
		/* Complete connection within 10 seconds */
		 curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, GMT_CONNECT_TIME_OUT);

		res = curl_easy_perform (curl);

		if ((res = curl_easy_perform (curl)) == CURLE_OK) {
      			res = curl_easy_getinfo (curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
			if ((res == CURLE_OK) && (filesize > 0.0)) {	/* Got the size */
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Remote file %s: Size is %0.0f bytes\n", URL, filesize);
				action = (filesize < (double)limit) ? 0 : (size_t)filesize;
			}
		}
		else	/* We failed */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Remote file %s: Curl returned error %d\n", URL, res);

		/* always cleanup */
		curl_easy_cleanup (curl);
	}

	curl_global_cleanup ();

	return action;
}

CURL * gmtremote_setup_curl (struct GMTAPI_CTRL *API, char *url, char *local_file, struct FtpFile *urlfile, unsigned int time_out) {
	/* Single function that sets up an impending CURL operation */
	CURL *Curl = NULL;
	if ((Curl = curl_easy_init ()) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to initiate curl - cannot obtain %s\n", url);
		return NULL;
	}
	if (curl_easy_setopt (Curl, CURLOPT_SSL_VERIFYPEER, 0L)) {		/* Tell libcurl to not verify the peer */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to not verify the peer\n");
		return NULL;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FOLLOWLOCATION, 1L)) {		/* Tell libcurl to follow 30x redirects */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to follow redirects\n");
		return NULL;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FAILONERROR, 1L)) {		/* Tell libcurl to fail on 4xx responses (e.g. 404) */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to fail for 4xx responses\n");
		return NULL;
	}
 	if (curl_easy_setopt (Curl, CURLOPT_URL, url)) {	/* Set the URL to copy */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to read from %s\n", url);
		return NULL;
	}
 	if (curl_easy_setopt (Curl, CURLOPT_CONNECTTIMEOUT, GMT_CONNECT_TIME_OUT)) {	/* Set connection timeout to 10s [300] */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to limit connection timeout to %lds\n", GMT_CONNECT_TIME_OUT);
		return NULL;
	}
	if (time_out) {	/* Set a timeout limit */
		if (curl_easy_setopt (Curl, CURLOPT_TIMEOUT, time_out)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to time out after %d seconds\n", time_out);
			return NULL;
		}
	}
	urlfile->filename = local_file;	/* Set pointer to local filename */
	/* Define our callback to get called when there's data to be written */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, gmtremote_fwrite_callback)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl output callback function\n");
		return NULL;
	}
	/* Set a pointer to our struct that will be passed to the callback function */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, urlfile)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to write to %s\n", local_file);
		return NULL;
	}

	return Curl;	/* Happily return the Curl pointer */
}

struct LOCFILE_FP *gmtremote_lock_on (struct GMT_CTRL *GMT, char *file) {
	/* Creates filename for lock and activates the lock */
	struct LOCFILE_FP *P = gmt_M_memory (GMT, NULL, 1, struct LOCFILE_FP);
	P->file = gmtremote_lockfile (GMT, file);
	if ((P->fp = fopen (P->file, "w")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to create lock file %s\n", P->file);
		gmt_M_str_free (P->file);
		gmt_M_free (GMT, P);
		return NULL;
	}
	gmtlib_file_lock (GMT, fileno(P->fp));	/* Attempt exclusive lock */
	return P;
}

void gmtremote_lock_off (struct GMT_CTRL *GMT, struct LOCFILE_FP **P) {
	/* Deactivates the lock on the file */
	gmtlib_file_unlock (GMT, fileno((*P)->fp));
	fclose ((*P)->fp);
	gmt_remove_file (GMT, (*P)->file);
	gmt_M_str_free ((*P)->file);
	gmt_M_free (GMT, *P);
}

/* Deal with hash values of cache/data files */

GMT_LOCAL int gmtremote_get_url (struct GMT_CTRL *GMT, char *url, char *file, char *orig, unsigned int index, bool do_lock) {
	bool turn_ctrl_C_off = false;
	int curl_err = 0, error = GMT_NOERROR;
	long time_spent;
	CURL *Curl = NULL;
	struct LOCFILE_FP *LF = NULL;
	struct FtpFile urlfile = {NULL, NULL};
	struct GMTAPI_CTRL *API = GMT->parent;
	time_t begin, end;

	if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) {  /* Not allowed to use remote copying */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Remote download is currently deactivated\n");
		return 1;
	}
	if (GMT->current.io.internet_error) return 1;   			/* Not able to use remote copying in this session */

	/* Make a lock */
	if (do_lock && (LF = gmtremote_lock_on (GMT, file)) == NULL)
		return 1;

	/* If file locking held us up as another process was downloading the same file,
	 * then that file should now be available.  So we check again if it is before proceeding */

	if (do_lock && !access (file, F_OK))
		goto unlocking1;	/* Yes it was, unlock and return no error */

	/* Initialize the curl session */
	if ((Curl = gmtremote_setup_curl (API, url, file, &urlfile, GMT_HASH_TIME_OUT)) == NULL)
		goto unlocking1;

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	gmtremote_turn_on_ctrl_C_check (file);	turn_ctrl_C_off = true;
	begin = time (NULL);
	if ((curl_err = curl_easy_perform (Curl))) {	/* Failed, give error message */
		end = time (NULL);
		time_spent = (long)(end - begin);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Unable to download file %s\n", url);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Libcurl Error: %s\n", curl_easy_strerror (curl_err));
		if (urlfile.fp != NULL) {
			fclose (urlfile.fp);
			urlfile.fp = NULL;
		}
		if (time_spent >= GMT_HASH_TIME_OUT) {	/* Ten seconds is too long time - server down? */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "GMT data server may be down - delay checking hash file for 24 hours\n");
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "You can turn remote file download off by setting GMT_DATA_UPDATE_INTERVAL to \"off\"\n");
			if (orig && !access (orig, F_OK)) {	/* Refresh modification time of original hash file */
#ifdef WIN32
				_utime (orig, NULL);
#else
				utimes (orig, NULL);
#endif
				GMT->current.io.refreshed[index] = GMT->current.io.internet_error = true;
			}
		}
		error = 1; goto unlocking1;
	}
	curl_easy_cleanup (Curl);
	if (urlfile.fp) /* close the local file */
		fclose (urlfile.fp);

unlocking1:

	/* Remove lock file after successful download */
	if (do_lock) gmtremote_lock_off (GMT, &LF);

	if (turn_ctrl_C_off) gmtremote_turn_off_ctrl_C_check ();

	return error;
}

GMT_LOCAL struct GMT_DATA_HASH *gmtremote_hash_load (struct GMT_CTRL *GMT, char *file, int *n) {
	/* Read contents of the hash file into an array of structs */
	int k;
	FILE *fp = NULL;
	struct GMT_DATA_HASH *L = NULL;
	char line[GMT_LEN256] = {""};

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Load contents from %s\n", file);
	*n = 0;
	if ((fp = fopen (file, "r")) == NULL) return NULL;
	if (fgets (line, GMT_LEN256, fp) == NULL) {	/* Try to get first record */
		fclose (fp);
		return NULL;
	}
	*n = atoi (line);		/* Number of records to follow */
	if (*n <= 0 || *n > GMT_BIG_CHUNK) {	/* Probably not a good value */
		fclose (fp);
		return NULL;
	}
	L = gmt_M_memory (GMT, NULL, *n, struct GMT_DATA_HASH);
	for (k = 0; k < *n; k++) {
		if (fgets (line, GMT_LEN256, fp) == NULL) break;	/* Next record */
		sscanf (line, "%s %s %" PRIuS, L[k].name, L[k].hash, &L[k].size);
	}
	fclose (fp);
	if (k != *n) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s said it has %d records but only found %d - download error???\n", file, *n, k);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s will be deleted.  Please try again\n", file);
		*n = 0;	/* Flag that excrement hit the fan */
	}
	return (L);
};

GMT_LOCAL int gmtremote_refresh (struct GMTAPI_CTRL *API, unsigned int index) {
	/* This function is called every time we are about to access a @remotefile.
	 * It is called twice: Once for the hash table and once for the info table.
	 * First we check that we have the GMT_HASH_SERVER_FILE in the server directory.
	 * If we don't then we download it and return since no old file to compare to.
	 * If we do find the hash file then we get its creation time [st_mtime] as
	 * well as the current system time.  If the file is < GMT->current.setting.refresh_time
	 * days old we are done.
	 * If the file is older we rename it to *.old and download the latest hash file.
	 * This is the same for both values of index (hash and info).  For hash, we do more:
	 * Next, we load the contents of both files and do a double loop to find the
	 * entries for each old file in the new list, for all files.  If the old file
	 * is no longer in the list then we delete the data file.  If the hash code for
	 * a file has changed then we delete the local file so that the new versions
	 * will be downloaded from the server.  Otherwise we do nothing.
	 * The result of this is that any file(s) that have changed will be removed
	 * so that they must be downloaded again to get the new versions.
	 */
	struct stat buf;
	time_t mod_time, right_now = time (NULL);	/* Unix time right now */
	char indexpath[PATH_MAX] = {""}, old_indexpath[PATH_MAX] = {""}, new_indexpath[PATH_MAX] = {""}, url[PATH_MAX] = {""};
	const char *index_file = (index == GMT_HASH_INDEX) ? GMT_HASH_SERVER_FILE : GMT_INFO_SERVER_FILE;
	struct LOCFILE_FP *LF = NULL;
	struct GMT_CTRL *GMT = API->GMT;	/* Short hand */

	if (GMT->current.io.refreshed[index]) return GMT_NOERROR;	/* Already been here */

	snprintf (indexpath, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, index_file);

	if (access (indexpath, R_OK)) {    /* Not found locally so need to download the first time */
		char serverdir[PATH_MAX] = {""};
		snprintf (serverdir, PATH_MAX, "%s/server", GMT->session.USERDIR);
		if (access (serverdir, R_OK) && gmt_mkdir (serverdir)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create GMT server directory : %s\n", serverdir);
			return 1;
		}
		snprintf (url, PATH_MAX, "%s/%s", gmt_dataserver_url (API), index_file);
		GMT_Report (API, GMT_MSG_DEBUG, "Download remote file %s for the first time\n", url);
		if (gmtremote_get_url (GMT, url, indexpath, NULL, index, true)) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Failed to get remote file %s\n", url);
			if (!access (indexpath, F_OK)) gmt_remove_file (GMT, indexpath);	/* Remove index file just in case it got corrupted or zero size */
			GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;	/* Temporarily turn off auto download in this session only */
			GMT->current.io.internet_error = true;		/* No point trying again */
			return 1;
		}
		GMT->current.io.refreshed[index] = true;	/* Done our job */
		return GMT_NOERROR;
	}
	else
		GMT_Report (API, GMT_MSG_DEBUG, "Local file %s found\n", indexpath);

	GMT->current.io.refreshed[index] = true;	/* Done our job */

	/* Here we have the existing index file and its path is in indexpath. Check how old it is */

	if (stat (indexpath, &buf)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to get information about %s - abort\n", indexpath);
		return 1;
	}
	/*  Get its modification (creation) time */
#ifdef __APPLE__
	mod_time = buf.st_mtimespec.tv_sec;	/* Apple even has tv_nsec for nano-seconds... */
#else
	mod_time = buf.st_mtime;
#endif

	if ((right_now - mod_time) > (GMT_DAY2SEC_I * GMT->current.setting.refresh_time)) {	/* Older than selected number of days; Time to get a new index file */
		GMT_Report (API, GMT_MSG_DEBUG, "File %s older than 24 hours, get latest from server.\n", indexpath);
		strcpy (new_indexpath, indexpath);	/* Duplicate path name */
		strcat (new_indexpath, ".new");		/* Append .new to the copied path */
		strcpy (old_indexpath, indexpath);	/* Duplicate path name */
		strcat (old_indexpath, ".old");		/* Append .old to the copied path */
		snprintf (url, PATH_MAX, "%s/%s", gmt_dataserver_url (API), index_file);	/* Set remote path to new index file */

		/* Here we will try to download a file */

		/* Make a lock on the file */
		if ((LF = gmtremote_lock_on (GMT, (char *)new_indexpath)) == NULL)
			return 1;

		/* If file locking held us up as another process was downloading the same file,
		 * then that file should now be available.  So we check again if it is before proceeding */

		if (!access (new_indexpath, F_OK)) {	/* Yes it was! Undo lock and return no error */
			gmtremote_lock_off (GMT, &LF);	/* Remove lock file after successful download (unless query) */
			return GMT_NOERROR;
		}

		if (gmtremote_get_url (GMT, url, new_indexpath, indexpath, index, false)) {	/* Get the new index file from server */
			GMT_Report (API, GMT_MSG_DEBUG, "Failed to download %s - Internet troubles?\n", url);
			if (!access (new_indexpath, F_OK)) gmt_remove_file (GMT, new_indexpath);	/* Remove index file just in case it got corrupted or zero size */
			gmtremote_lock_off (GMT, &LF);
			return 1;	/* Unable to update the file (no Internet?) - skip the tests */
		}
		if (!access (old_indexpath, F_OK))
			remove (old_indexpath);	/* Remove old index file if it exists */
		GMT_Report (API, GMT_MSG_DEBUG, "Rename %s to %s\n", indexpath, old_indexpath);
		if (gmt_rename_file (GMT, indexpath, old_indexpath, GMT_RENAME_FILE)) {	/* Rename existing file to .old */
			GMT_Report (API, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", indexpath, old_indexpath);
			gmtremote_lock_off (GMT, &LF);
			return 1;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Rename %s to %s\n", new_indexpath, indexpath);
		if (gmt_rename_file (GMT, new_indexpath, indexpath, GMT_RENAME_FILE)) {	/* Rename newly copied file to existing file */
			GMT_Report (API, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", new_indexpath, indexpath);
			gmtremote_lock_off (GMT, &LF);
			return 1;
		}

		if (index == GMT_HASH_INDEX) {	/* Special processing to upgrade or remove deprecated files */
			bool found;
			int nO, nN, n, o;
			struct GMT_DATA_HASH *O = NULL, *N = NULL;

			if ((N = gmtremote_hash_load (GMT, indexpath, &nN)) == 0) {	/* Read in the new array of hash structs, will return 0 if mismatch of entries */
				gmt_remove_file (GMT, indexpath);	/* Remove corrupted index file */
				gmtremote_lock_off (GMT, &LF);
				return 1;
			}

			O = gmtremote_hash_load (GMT, old_indexpath, &nO);	/* Read in the old array of hash structs */
			for (o = 0; o < nO; o++) {	/* Loop over items in old file */
				if (gmt_getdatapath (GMT, O[o].name, url, R_OK) == NULL) continue;	/* Don't have this file downloaded yet */
				/* Here the file was found locally and the full path is in the url */
				found = false;	/* Not found this file in the new list yet */
				for (n = 0; !found && n < nN; n++) {	/* Loop over items in new file */
					if (!strcmp (N[n].name, O[o].name)) {	/* File is in the current hash table */
						found = true;	/* We will exit this loop regardless of what happens next below */
						if (strcmp (N[n].hash, O[o].hash)) {	/* New hash differs from entry in hash old file */
							GMT_Report (API, GMT_MSG_DEBUG, "Server and cache versions of %s have different hash codes - must download new copy.\n", N[n].name);
							gmt_remove_file (GMT, url);	/* Need to re-download so be gone with it */
						}
						else {	/* Do size check */
							struct stat buf;
							if (stat (url, &buf)) {
								GMT_Report (API, GMT_MSG_WARNING, "Could not determine size of file %s.\n", url);
								continue;
							}
							if (N[n].size != (size_t)buf.st_size) {	/* Downloaded file size differ - need to re-download */
								GMT_Report (API, GMT_MSG_DEBUG, "Server and cache versions of %s have different byte sizes (%" PRIuS " versus %" PRIuS ") - must download new copy.\n", N[n].name, N[n].size, (size_t)buf.st_size);
								gmt_remove_file (GMT, url);	/* Need to re-download so be gone with it */
							}
							else
								GMT_Report (API, GMT_MSG_DEBUG, "Server and cache versions of %s are identical - no need to download new file.\n", N[n].name);
						}
					}
				}
				if (!found) {	/* This file was present locally but is no longer part of files on the server and should be removed */
					GMT_Report (API, GMT_MSG_DEBUG, "File %s no longer supported on server - deleting local copy.\n", O[o].name);
					gmt_remove_file (GMT, url);
				}
			}
			gmt_M_free (GMT, O);	/* Free old hash table structures */
			gmt_M_free (GMT, N);	/* Free new hash table structures */
			/* We now have an updated hash file */
			if (!access (old_indexpath, F_OK))
				remove (old_indexpath);	/* Remove old index file if it exists */
		}
		else 
			GMT->current.io.new_data_list = true;	/* Flag that we wish to delete datasets older than entries in this file */
		/* Remove lock file after successful download */
		gmtremote_lock_off (GMT, &LF);
	}
	else
		GMT_Report (API, GMT_MSG_DEBUG, "File %s less than 24 hours old, refresh is premature.\n", indexpath);
	return GMT_NOERROR;
}

void gmt_refresh_server (struct GMTAPI_CTRL *API) {
	/* Called once in gmt_begin from GMT_Create_Session,  The following actions take place:
	 *
	 * The data info table is refreshed if missing or older than 24 hours.
	 * The hash table is refreshed if missing or older than 24 hours.
	 *   If a new hash table is obtained and there is a previous one, we determine
	 *   if there are entries that have changed (e.g., newer, different files, different
	 *   sizes, or gone altogether).  In all these case we delete the file so that when the
	 *   user requests it, it forces a download of the updated file.
	 */

	if (gmtremote_refresh (API, GMT_INFO_INDEX))	/* Watch out for changes on the server info once a day */
		GMT_Report (API, GMT_MSG_INFORMATION, "Unable to obtain remote information file %s\n", GMT_INFO_SERVER_FILE);
	else if (API->remote_info == NULL) {	/* Get server file attribution info if not yet loaded */
		if ((API->remote_info = gmtremote_data_load (API, &API->n_remote_info)) == NULL) {	/* Failed to load the info file */
			GMT_Report (API, GMT_MSG_INFORMATION, "Unable to read server information file\n");
		}
	}

	if (gmtremote_refresh (API, GMT_HASH_INDEX)) {	/* Watch out for changes on the server hash once a day */
		GMT_Report (API, GMT_MSG_INFORMATION, "Unable to obtain remote hash table %s\n", GMT_HASH_SERVER_FILE);
	}
}

GMT_LOCAL char * gmtremote_switch_to_srtm (char *file, char *res) {
	/* There may be more than one remote Earth DEM product that needs to share the
	 * same 1x1 degree SRTM tiles.  This function handles this overlap; add more cases if needed. */
	char *c = NULL;
	if ((c = strstr (file, ".earth_relief_01s_g")) || (c = strstr (file, ".earth_synbath_01s_g")))
		*res = '1';
	else if ((c = strstr (file, ".earth_relief_03s_g")) || (c = strstr (file, ".earth_synbath_03s_g")))
		*res = '3';
	return (c);	/* Returns pointer to this "extension" or NULL */
}

GMT_LOCAL char * gmtremote_get_jp2_tilename (char *file) {
	/* Must do special legacy checks for SRTMGL1|3 tag names for SRTM tiles.
	 * We also strip off the leading @ since we are building an URL for curl  */
	char res, *c = NULL, *new_file = NULL;
	
	if ((c = gmtremote_switch_to_srtm (file, &res))) {
		/* Found one of the SRTM tile families, now replace the tag with SRTMGL1|3 */
		char remote_name[GMT_LEN64] = {""};
		c[0] = '\0';	/* Temporarily chop off tag and beyond */
		sprintf (remote_name, "%s.SRTMGL%c.%s", &file[1], res, GMT_TILE_EXTENSION_REMOTE);
		c[0] = '.';	/* Restore period */
		new_file = strdup (remote_name);
	}
	else
		new_file = gmt_strrep (&file[1], GMT_TILE_EXTENSION_LOCAL, GMT_TILE_EXTENSION_REMOTE);
	return (new_file);
}

GMT_LOCAL int gmtremote_convert_jp2_to_nc (struct GMTAPI_CTRL *API, char *localfile) {
	static char *args = " -fg -Vq --IO_NC4_DEFLATION_LEVEL=9 --GMT_HISTORY=readonly";
	char cmd[GMT_LEN512] = {""},  *ncfile = NULL;
	int k_data;

	if (API->GMT->current.io.leave_as_jp2) return GMT_NOERROR;	/* Conversion temporarily turned off by gmtget -N */
	if ((k_data = gmtlib_file_is_jpeg2000_tile (API, localfile)) == GMT_NOTSET) return GMT_NOERROR;	/* Nothing to do */

	/* Convert JP2 file to NC for local cache storage */
	ncfile = gmt_strrep (localfile, GMT_TILE_EXTENSION_REMOTE, GMT_TILE_EXTENSION_LOCAL);
	sprintf (cmd, "%s -G%s=ns", localfile, ncfile);	/* We know we are writing a netCDF short int grid */
	if (!doubleAlmostEqual (API->remote_info[k_data].scale, 1.0) || !gmt_M_is_zero (API->remote_info[k_data].offset)) {
		/* Integer is not the original data unit and/or has an offset - must scale/shift jp2 integers to units first.
		 * Because we are inverting the scaling and because grdconvert applies z' = z * scale + offset, we must
		 * pre-scale and change the sign of the offset here to get the translation we want */
		char extra[GMT_LEN64] = {""};
		sprintf (extra, "+s%g+o%g", API->remote_info[k_data].scale, API->remote_info[k_data].offset);
		strcat (cmd, extra);	/* This will embed the scale and offset in the netCDF file so we can use the full range */
		sprintf (extra, " -Z+s%g+o%g", API->remote_info[k_data].scale, -API->remote_info[k_data].offset / API->remote_info[k_data].scale);
		strcat (cmd, extra);	/* This converts the integers we got back to Myr before we let netCDF do the offset/scaling above */
	}
	strcat (cmd, args);	/* Append the common arguments */
	GMT_Report (API, GMT_MSG_INFORMATION, "Convert SRTM tile from JPEG2000 to netCDF grid [%s]\n", ncfile);
	GMT_Report (API, GMT_MSG_DEBUG, "Running: grdconvert %sn", cmd);
	if (GMT_Call_Module (API, "grdconvert", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to convert SRTM file %s to compressed netCDF format\n", localfile);
		gmt_M_free (API->GMT, ncfile);
		return GMT_RUNTIME_ERROR;
	}
	gmt_M_str_free (ncfile);
	if (gmt_remove_file (API->GMT, localfile))
		GMT_Report (API, GMT_MSG_WARNING, "Could not even remove file %s\n", localfile);
	return GMT_NOERROR;
}

int gmt_set_remote_and_local_filenames (struct GMT_CTRL *GMT, const char * file, char *local_path, char *remote_path, unsigned int mode) {
	/* Determines the remote and local files for any given file_name.
	 * For remote files, the mode controls where they are written locally:
 	 *    0 : Place file where GMT wants it to be (e.g., server/earth/earth_relief, /cache etc depending on file type).
 	 *    1 : Place file in the cache directory
	 *    2 : Place file in user directory
	 *    3 : Place file in local (current) directory
	 * If the file must be downloaded from a remote location then remote_path will be populated.
	 * If the file exists in a local location then local_path will be populated and remote_path empty.
	 * If both paths are set it means we want to download file and place it in local_path.
	 * If a local file is not found we return an error code, else 0.
	 */

	int k_data = GMT_NOTSET, t_data = GMT_NOTSET;
	unsigned int pos;
	bool is_url = false, is_query = false, is_tile = false;
	char was, *c = NULL, *jp2_file = NULL, *clean_file = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	local_path[0] = remote_path[0] = '\0';

	/* 0. Were we even given an argument? */
	if (!file || !file[0]) return GMT_ARG_IS_NULL;   /* Got nutin' */

	if (gmt_M_file_is_memory (file)) return GMT_NOERROR;	/* Memory location always exists */

	if (gmtlib_found_url_for_gdal ((char *)file)) {	/* Special URLs for grids to be read via GDAL */
		snprintf (remote_path, PATH_MAX, "%s", file);
		pos = gmtlib_get_pos_of_filename (file);	/* Start of file in URL (> 0) */
		snprintf (local_path, PATH_MAX, "%s", &file[pos]);	/* Same. No directives when writing the file */
		return GMT_NOERROR;
	}

	/* 1. First handle full paths as given */
#ifdef WIN32
	if (file[0] == '/' || file[1] == ':')
#else
	if (file[0] == '/')
#endif
	{
		clean_file = gmt_get_filename (API, file, gmtlib_valid_filemodifiers (GMT));	/* Strip off any file modifier or netCDF directives */
		if (access (clean_file, F_OK)) {
			GMT_Report (API, GMT_MSG_ERROR, "File %s was not found\n", file);
			gmt_M_str_free (clean_file);
			return GMT_FILE_NOT_FOUND;
		}
		if (access (clean_file, R_OK)) {
			GMT_Report (API, GMT_MSG_ERROR, "File %s is not readable\n", file);
			gmt_M_str_free (clean_file);
			return GMT_BAD_PERMISSION;
		}
		gmt_M_str_free (clean_file);
		strncpy (local_path, file, PATH_MAX-1);
		remote_path[0] = '\0';	/* No need to get from elsewhere */
		GMT_Report (API, GMT_MSG_DEBUG, "Given full path to file %s\n", local_path);
		return GMT_NOERROR;
	}

	if (file[0] == '@') {	/* Either a cache file or a remote data set */
		if ((k_data = gmt_remote_dataset_id (API, file)) != GMT_NOTSET) {
			/* Got a valid remote server data filename and we know the local path to those */
			if (GMT->session.USERDIR == NULL) goto not_local;	/* Cannot have server data if no user directory created yet */
			snprintf (local_path, PATH_MAX, "%s", GMT->session.USERDIR);	/* This is the top-level directory for user data */
			if (access (local_path, R_OK)) goto not_local;	/* Have not made a user directory yet, so cannot have the file yet either */
			strcat (local_path, GMT->parent->remote_info[k_data].dir);	/* Append the subdir (/ or /server/earth/earth_relief/, etc) */
			strcat (local_path, GMT->parent->remote_info[k_data].file);	/* Append filename */
			if (access (local_path, R_OK)) goto not_local;	/* No such file yet */
		}
		else if ((t_data = gmt_file_is_a_tile (API, file, GMT_LOCAL_DIR)) != GMT_NOTSET) {	/* Got a remote tile */
			/* Got a valid remote server tile filename and we know the local path to those */
			if (GMT->session.USERDIR == NULL) goto not_local;	/* Cannot have server data if no user directory created yet */
			snprintf (local_path, PATH_MAX, "%s", GMT->session.USERDIR);	/* This is the top-level directory for user data */
			if (access (local_path, R_OK)) goto not_local;	/* Have not made a user directory yet, so cannot have the file yet either */
			strcat (local_path, GMT->parent->remote_info[t_data].dir);	/* Append the subdir (/ or /server/earth/earth_relief/, etc) */
			strcat (local_path, GMT->parent->remote_info[t_data].file);	/* Append the tiledir to get full path to dir for this type of tiles */
			strcat (local_path, &file[1]);	/* Append filename */
			is_tile = true;
			if (access (local_path, R_OK)) {	/* A local tile in netCDF format was not found.  See if it exists as compressed JP2000 */
				char *local_jp2 = gmt_strrep (local_path, GMT_TILE_EXTENSION_LOCAL, GMT_TILE_EXTENSION_REMOTE);
 				if (access (local_jp2, R_OK))
	 				goto not_local;	/* No such file yet */
				else {	/* Yep, do the just-in-time conversion now */
					int error = gmtremote_convert_jp2_to_nc (API, local_jp2);
					gmt_M_str_free (local_jp2);
					if (error) return error;	/* Something failed in the conversion */
				}
			}
		}
		else {	/* Must be cache file */
			if (GMT->session.CACHEDIR == NULL) goto not_local;	/* Cannot have cache data if no cache directory created yet */
			clean_file = gmt_get_filename (API, file, gmtlib_valid_filemodifiers (GMT));	/* Strip off any file modifier or netCDF directives */
			snprintf (local_path, PATH_MAX, "%s/%s", GMT->session.CACHEDIR, &clean_file[1]);	/* This is where all cache files live */
			if ((c = strchr (local_path, '=')) || (c = strchr (local_path, '?'))) {
				was = c[0];	c[0] = '\0';
			}
			GMT->parent->cache = true;
			if (access (local_path, R_OK)) {
				if (c) c[0] = was;
				goto not_local;	/* No such file yet */
			}
			if (c) c[0] = was;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Remote file %s exists locally as %s\n", clean_file, local_path);
		remote_path[0] = '\0';	/* No need to get from elsewhere */
		if (clean_file)	gmt_M_str_free (clean_file);
		return GMT_NOERROR;

not_local:	/* Get here if we failed to find a remote file already on disk */
		/* Set remote path */
		if (is_tile) {	/* Tile not yet downloaded, but must switch to .jp2 format on the server (and deal with legacy SRTM tile tags) */
			jp2_file = gmtremote_get_jp2_tilename ((char *)file);
			snprintf (remote_path, PATH_MAX, "%s%s%s%s", gmt_dataserver_url (API), GMT->parent->remote_info[t_data].dir, GMT->parent->remote_info[t_data].file, jp2_file);
		}
		else if (k_data == GMT_NOTSET) {	/* Cache file not yet downloaded */
			snprintf (remote_path, PATH_MAX, "%s/cache/%s", gmt_dataserver_url (API), &file[1]);
			if (mode == 0) mode = GMT_CACHE_DIR;	/* Just so we default to the cache dir for cache files */
		}
		else	/* Remote data set */
			snprintf (remote_path, PATH_MAX, "%s%s%s", gmt_dataserver_url (API), API->remote_info[k_data].dir, API->remote_info[k_data].file);

		/* Set local path */
		switch (mode) {
			case GMT_CACHE_DIR:
				if (GMT->session.CACHEDIR == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Cache directory storage requested for %s but your cache directory is undefined\n", file);
					return GMT_FILE_NOT_FOUND;
				}
				else if (access (GMT->session.CACHEDIR, R_OK) && gmt_mkdir (GMT->session.CACHEDIR))
					GMT_Report (API, GMT_MSG_ERROR, "Cache directory storage requested for %s but your cache directory could not be created\n", file);
				else
					snprintf (local_path, PATH_MAX, "%s/%s", GMT->session.CACHEDIR, &file[1]);
				break;
			case GMT_DATA_DIR:
				if (GMT->session.USERDIR == NULL || access (GMT->session.USERDIR, R_OK))
					GMT_Report (API, GMT_MSG_ERROR, "User directory storage requested for %s but your user directory is undefined or does not exist\n", file);
				else
					snprintf (local_path, PATH_MAX, "%s/%s", GMT->session.USERDIR, &file[1]);
				break;
			case GMT_LOCAL_DIR:
				snprintf (local_path, PATH_MAX, "%s", &file[1]);
				break;
			default:	/* Place remote data files locally per the internal rules */
				if (GMT->session.USERDIR == NULL || access (GMT->session.USERDIR, R_OK))
					GMT_Report (API, GMT_MSG_ERROR, "User directory storage requested for %s but your user directory is undefined or does not exist\n", file);
				else {	/* Have a user dir */
					snprintf (local_path, PATH_MAX, "%s/server", GMT->session.USERDIR);
					if (access (local_path, R_OK) && gmt_mkdir (local_path))	/* Have or just made a server subdirectory */
						GMT_Report (API, GMT_MSG_ERROR, "Unable to create GMT data directory : %s\n", local_path);
					if (is_tile) {	/* One of the tiles */
						if (jp2_file) gmt_M_str_free (jp2_file);
						jp2_file = gmt_strrep (&file[1], GMT_TILE_EXTENSION_LOCAL, GMT_TILE_EXTENSION_REMOTE);
						snprintf (local_path, PATH_MAX, "%s%s%s", GMT->session.USERDIR, GMT->parent->remote_info[t_data].dir, GMT->parent->remote_info[t_data].file);
						if (access (local_path, R_OK) && gmt_mkdir (local_path))	/* Have or just made a server/tile subdirectory */
							GMT_Report (API, GMT_MSG_ERROR, "Unable to create GMT data directory : %s\n", local_path);
						strcat (local_path, jp2_file);
					}
					else if (!strcmp (API->remote_info[k_data].dir, "/"))	/* One of the symbolic links in server */
						snprintf (local_path, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, API->remote_info[k_data].file);
					else {
						snprintf (local_path, PATH_MAX, "%s%s", GMT->session.USERDIR, API->remote_info[k_data].dir);
						if (access (local_path, R_OK) && gmt_mkdir (local_path))	/* Have or just made a subdirectory under server */
							GMT_Report (API, GMT_MSG_ERROR, "Unable to create GMT data directory : %s\n", local_path);
						strcat (local_path, API->remote_info[k_data].file);
					}
				}
				break;
		}
		if (jp2_file) gmt_M_str_free (jp2_file);
		if (clean_file)	gmt_M_str_free (clean_file);
		GMT_Report (API, GMT_MSG_DEBUG, "Get remote file %s and write to %s\n", remote_path, local_path);

		return GMT_NOERROR;
	}

	/* URL files and queries and netcdf grids with directives all have characters like ? and = that
	 * are not part of an actual file name (but is part of a query).  Need to deal with those complexities
	 * here an keep track of if we got a query or a file request. */

	is_url = (gmt_M_file_is_url (file));	/* A remote file or query given via an URL */
	is_query = (gmt_M_file_is_query (file));	/* A remote file or query given via an URL */

	if (strchr (file, '?') && (c = strstr (file, "=gd"))) {	/* Must be a netCDF sliced file to be read via GDAL so chop off the =gd?layer/variable specifications */
		was = c[0]; c[0] = '\0';
	}
	else if ((c = strchr (file, '?')) && !strchr (file, '=')) {	/* Must be a netCDF sliced URL file so chop off the layer/variable specifications */
		was = c[0]; c[0] = '\0';
	}
	else if (c == NULL && (c = strchr (file, '='))) {	/* If no ? then = means grid attributes (e.g., =bf) */
		was = c[0]; c[0] = '\0';
	}
	else if (c) {	/* else we have both ? and = which means file is an URL query */
		strncpy (remote_path, file, PATH_MAX-1);	/* Pass whatever we were given, no check possible */
		was = c[0]; c[0] = '\0';
	}

	if (is_url) {	/* A remote file or query given via an URL never exists locally */
		pos = gmtlib_get_pos_of_filename (file);	/* Start of file in URL (> 0) */
		if (is_query) /* We have truncated off all the ?specifications part */
			snprintf (local_path, PATH_MAX, "%s", &file[pos]);
		else {	/* URL file, we have truncated off any netCDF directives */
			strncpy (remote_path, file, PATH_MAX-1);
			snprintf (local_path, PATH_MAX, "%s", &file[pos]);	/* Same. No directives when writing the file */
		}
		if (c) c[0] = was;
		return GMT_NOERROR;
	}

	/* Looking for local files given a relative path - must search directories we are allowed.
	 * Note: Any netCDF files with directives have had those chopped off earlier, so file is a valid name */

	clean_file = gmt_get_filename (API, file, gmtlib_valid_filemodifiers (GMT));	/* Strip off any file modifier or netCDF directives */
	if (gmt_getdatapath (GMT, clean_file, local_path, R_OK)) {	/* Found it */
		/* Return full path */
		if (c &&!is_query) {	/* We need to pass the ?var[]() or [id][+mods]stuff as part of the filename */
			c[0] = was;
			strcpy (local_path, file);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Replace file %s with path %s\n", file, local_path);
		gmt_M_str_free (clean_file);
		return GMT_NOERROR;
	}

	/* No luck whatsoever */

	gmt_M_str_free (clean_file);

	return GMT_FILE_NOT_FOUND;
}

int gmtlib_file_is_jpeg2000_tile (struct GMTAPI_CTRL *API, char *file) {
	/* Detect if a file matches the name <path>/[N|S]yy[E|W]xxx.tag.jp2 (e.g., N22W160.earth_relief_01m_p.jp2) */
	char *c, tmp[PATH_MAX] = {""};
	if (file == NULL || file[0] == '\0') return GMT_NOTSET;	/* Bad argument */
	if ((c = strrchr (file, '/')) == NULL)	/* Get place of the last slash */
		sprintf (tmp, "@%s", file);	/* Now should have something like @N22W160.earth_relief_01m_p.jp2 */
	else
		sprintf (tmp, "@%s", &c[1]);	/* Now should have something like @N22W160.earth_relief_01m_p.jp2 */
	return (gmt_file_is_a_tile (API, tmp, GMT_REMOTE_DIR));
}

int gmt_download_file (struct GMT_CTRL *GMT, const char *name, char *url, char *localfile, bool be_fussy) {
	bool query = gmt_M_file_is_query (url), turn_ctrl_C_off = false;
	int curl_err, error = 0;
	size_t fsize;
	CURL *Curl = NULL;
	struct FtpFile urlfile = {NULL, NULL};
	struct LOCFILE_FP *LF = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) {  /* Not allowed to use remote copying */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Remote download is currently deactivated\n");
		return 1;
	}
	if (GMT->current.io.internet_error) return 1;   /* Not able to use remote copying in this session */

	/* Check if the file is too big for our current limit */

	if ((fsize = gmtremote_skip_large_files (GMT, url, GMT->current.setting.url_size_limit))) {
		char *S = strdup (gmt_memory_use (fsize, 3));
		GMT_Report (API, GMT_MSG_WARNING, "File %s skipped as size [%s] exceeds limit set by GMT_DATA_SERVER_LIMIT [%s]\n", name, S, gmt_memory_use (GMT->current.setting.url_size_limit, 0));
		gmt_M_str_free (S);
		return 1;
	}

	/* Here we will try to download a file */

	/* Only make a lock if not a query */
	if (!query && (LF = gmtremote_lock_on (GMT, (char *)name)) == NULL)
		return 1;

	/* If file locking held us up as another process was downloading the same file,
	 * then that file should now be available.  So we check again if it is before proceeding */

	if (!access (localfile, F_OK)) {	/* Yes it was! Undo lock and return no error */
		if (!query)	/* Remove lock file after successful download (unless query) */
			gmtremote_lock_off (GMT, &LF);
		return GMT_NOERROR;
	}

	/* Initialize the curl session */
	if ((Curl = gmtremote_setup_curl (API, url, localfile, &urlfile, 0)) == NULL)
		goto unlocking2;

	gmtremote_find_and_give_data_attribution (API, name);

	GMT_Report (API, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	gmtremote_turn_on_ctrl_C_check (localfile);
	turn_ctrl_C_off = true;
	if ((curl_err = curl_easy_perform (Curl))) {	/* Failed, give error message */
		if (be_fussy || !(curl_err == CURLE_REMOTE_FILE_NOT_FOUND || curl_err == CURLE_HTTP_RETURNED_ERROR)) {	/* Unexpected failure - want to bitch about it */
			GMT_Report (API, GMT_MSG_ERROR, "Libcurl Error: %s\n", curl_easy_strerror (curl_err));
			GMT_Report (API, GMT_MSG_WARNING, "You can turn remote file download off by setting GMT_DATA_UPDATE_INTERVAL to \"off\"\n");
			if (urlfile.fp != NULL) {
				fclose (urlfile.fp);
				urlfile.fp = NULL;
			}
			if (!access (localfile, F_OK) && gmt_remove_file (GMT, localfile))	/* Failed to clean up as well */
				GMT_Report (API, GMT_MSG_WARNING, "Could not even remove file %s\n", localfile);
		}
		else if (curl_err == CURLE_COULDNT_CONNECT)
			GMT->current.io.internet_error = true;	/* Prevent GMT from trying again in this session */
	}
	curl_easy_cleanup (Curl);
	if (urlfile.fp) /* close the local file */
		fclose (urlfile.fp);

unlocking2:

	if (!query)	/* Remove lock file after successful download (unless query) */
		gmtremote_lock_off (GMT, &LF);

	if (turn_ctrl_C_off) gmtremote_turn_off_ctrl_C_check ();

	if (error == 0) error = gmtremote_convert_jp2_to_nc (API, localfile);

	return (error);
}

unsigned int gmt_download_file_if_not_found (struct GMT_CTRL *GMT, const char *file, unsigned int mode) {
	/* Downloads a file if not found locally.  Returns the position in file_name of the
 	 * start of the actual file (e.g., if given an URL). Values for mode:
 	 * 0 : Place file in the cache directory
	 * 1 : Place file in user directory
	 * 2 : Place file in local (current) directory
	 * Add 4 if the file may not be found and we should not complain about this here.
	 */
	unsigned int pos = 0;
	bool be_fussy;
	char remote_path[PATH_MAX] = {""}, local_path[PATH_MAX] = {""};

	if (gmt_M_file_is_memory (file)) return GMT_NOERROR;	/* Memory location always exists */
	if (gmtlib_found_url_for_gdal ((char *)file)) return GMT_NOERROR;	/* /vis.../ files are read in GDAL */

	be_fussy = ((mode & 4) == 0);	if (!be_fussy) mode -= 4;	/* Handle the optional 4 value */

	if (file[0] == '@')	/* Make sure we have a refreshed server this session */
		gmt_refresh_server (GMT->parent);

	if (gmt_set_remote_and_local_filenames (GMT, file, local_path, remote_path, mode)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot find file %s\n", file);
		return GMT_NOERROR;
	}

	if (remote_path[0]) {	/* Remote file given but not yet stored locally */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Download %s to %s\n", remote_path, local_path);
		if (gmt_download_file (GMT, file, remote_path, local_path, be_fussy)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to obtain remote file %s\n", file);
			return GMT_NOERROR;
		}
	}
	if (gmt_M_file_is_url (file))	/* A remote file or query given via an URL */
		pos = gmtlib_get_pos_of_filename (file);	/* Start of file in URL (> 0) */
	else if (strchr ("@=", file[0]))
		pos = 1;

	return (pos);
}

/* Support functions for tiled grids
 * gmtlib_file_is_tiled	Determine if a request was given for a tiled dataset.
 * gmt_file_is_tiled_list: Determine if file is a listfile with SRTM tile info
 * gmtlib_get_tile_list:	Convert -Rw/e/s/n into a file with a list of the tiles needed
 * gmtlib_assemble_tiles: Take the list, run grdblend to built the grid.
 *
 * The way the SRTM tiling works is that a user gives the virtual file name
 * @earth_relief_0[1|3]s or @srtm_relief_0[1|3]s. It is possible the user will append
 * _p for pixel registration but that is the only registration we have anyway.
 * If that is true (determined by gmtlib_remote_file_is_tiled) then we use the given
 * -Rw/e/s/n, the resolution given, and whether the ocean should be included (if the
 * srtm_relief_* name is given we only do land) and we create a list of all the
 * tiles that must be included to cover the region.  If ocean is requested we add in
 * the filler grid as well.  All these files are written in the form of remote
 * filenames (start with @).  This blend file is written and given a name that
 * is unique from the mask =tiled_<ID>_[G|P].######, where<ID> is the dataset iD
 * and G|P reflects if -R was a grid or plot region.
 * This filename is then used to replace the virtual grid we gave.  This happens
 * in gmt_init_module at the end of the function.  Then, when GMT_Read_Data is given
 * the tiled list file it knows what to do: Call gmtlib_assemble_tiles which will set
 * up and run a grdblend job that returns the desired custom-built grid.  Thus, when
 * grdblend starts accessing the files it finds that they are all remote files and
 * we will download a series of individual tiles (e.g., @N29W081.SRTMGL1.nc).
 */

GMT_LOCAL bool gmtremote_is_directory (struct GMTAPI_CTRL *API, int k) {
	/* If entry ends in / then it is a directory, else a file */
	size_t len = strlen (API->remote_info[k].file);
	if (len < 1) return false;
	return (API->remote_info[k].file[len-1] == '/');
}

int gmtlib_remote_file_is_tiled (struct GMTAPI_CTRL *API, const char *file, unsigned int *mode) {
	/* Determine if file is referring to a tiled remote data set. */
	int k_data;
	if (!file || file[0] != '@') return GMT_NOTSET;	/* Not a remote file */
	if (mode) *mode = 0;
	if (strncmp (file, "@srtm_relief_0", 14U) == 0) {	/* This virtual tile set does not exist. It means earth_relief_0xs_g but do not add filler */
		char tmpfile[GMT_LEN32] = {""};
		sprintf (tmpfile, "@earth_relief_0%cs_g", file[14]);
		if ((k_data = gmt_remote_dataset_id (API, tmpfile)) == GMT_NOTSET) return GMT_NOTSET;	/* Not a recognized remote dataset */
		if (mode) *mode = GMT_SRTM_ONLY;
		return (k_data);	/* Since we know earth_relief_01|d_g is a tiled directory */
	}
	if ((k_data = gmt_remote_dataset_id (API, file)) == GMT_NOTSET) return GMT_NOTSET;	/* Not a recognized remote dataset */
	return (gmtremote_is_directory (API, k_data) ? k_data : GMT_NOTSET);	/* -1 for a regular, remote file, valid index for a directory */
}

bool gmt_file_is_tiled_list (struct GMTAPI_CTRL *API, const char *file, int *ID, char *wetdry, char *region_type) {
	char *c = NULL, dummy2, dummy3, *wet, *region;
	int dummy1, *kval;
	kval = (ID) ? ID : &dummy1;	/* Allow passing NULL if we don't care */
	wet = (wetdry) ? wetdry : &dummy2;	/* Allow passing NULL if we don't care */
	region = (region_type) ? region_type : &dummy3;	/* Allow passing NULL if we don't care */
	*kval = GMT_NOTSET;
	*wet = *region = 0;
	if (file == NULL) return false;
	if ((c = strstr (file, "=tiled_")) == NULL) return false;	/* Not that kind of file */
	if (c[7] && sscanf (&c[7], "%d_%c%c", kval, region, wet) != 3) return false;	/* Not finding the id, land/ocean, and grid/plot markers */
	if (strchr ("LOX", *wet) == NULL || strchr ("GP", *region) == NULL) return false;	/* Invalid characters for two keys */
	if (*kval < 0 || (*kval) > API->n_remote_info) return false;	/* Outside recognized range of remote file IDs */
	return true;	/* We got one */
}

int gmt_get_tile_id (struct GMTAPI_CTRL *API, char *file) {
	int k_data;
	if (!gmt_file_is_tiled_list (API, file, &k_data, NULL, NULL)) return GMT_NOTSET;
	return (k_data);
}

int gmt_file_is_a_tile (struct GMTAPI_CTRL *API, const char *infile, unsigned int where) {
	/* Recognizes remote files like @N42E126.SRTMGL1.nc|jp2
	 * where == 0 means local file (nc extension) and where = 1 means on server (jp2 extension) */
	char *p = NULL, *file, tag[GMT_LEN64] = {""}, ext[GMT_LEN16] = {""};
	int k_data, n;
	size_t len = strlen (infile);
	gmt_M_unused (API);
	if (len < 12) return GMT_NOTSET;	/* Filename too short to hold a full tile name */
	file = (char *)((infile[0] == '@') ? &infile[1] : infile);	/* Now, file starts at N|S */
	if (strchr ("NS", file[0]) == 0) return GMT_NOTSET;	/* Does not start with N|S */
	if (strchr ("EW", file[3]) == 0) return GMT_NOTSET;	/* Does not contain E|W */
	if (!(isdigit (file[1]) && isdigit (file[2]))) return GMT_NOTSET;	/* No yy latitude */
	if (!(isdigit (file[4]) && isdigit (file[5]) && isdigit (file[6]))) return GMT_NOTSET;	/* No xxx longitude */
	if ((n = sscanf (file, "%*[^.].%[^.].%s", tag, ext)) != 2) return GMT_NOTSET;	/* Could not extract tag and extension */
	if (where == GMT_REMOTE_DIR) { 	/* On the server the extension is jp2 */
		if (strncmp (ext, GMT_TILE_EXTENSION_REMOTE, GMT_TILE_EXTENSION_REMOTE_LEN)) return GMT_NOTSET; /* Incorrect extension */
	}
	else if (where == GMT_LOCAL_DIR) {
		if (strncmp (ext, GMT_TILE_EXTENSION_LOCAL, GMT_TILE_EXTENSION_LOCAL_LEN)) return GMT_NOTSET; /* Incorrect extension */
	}
	else {
		GMT_Report (API, GMT_MSG_ERROR, "gmt_file_is_a_tile: Internal error - bad where assignment %d.\n", where);
		return GMT_NOTSET;
	}
	if ((p = strstr (file, ".SRTMGL"))) /* Convert to the new tag for legacy SRTM tiles tag of SRTMGL[1|3] so it reflects the name of the dataset */
		sprintf (tag, "earth_relief_0%cs_g", p[7]);	/* 7th char in p is the 1|3 resolution character */
	k_data = gmt_remote_dataset_id (API, tag);
	return (k_data);
}

GMT_LOCAL bool gmtremote_is_earth_dem (struct GMT_DATA_INFO *I) {
	/* Returns true if this data set is one of the earth_relief clones that must share SRTM tiles with @earth_relief.
	 * Should we add more such DEMs then just add more cases like the synbath test */
	if (strstr (I->tag, "synbath") && (!strcmp (I->inc, "03s") || !strcmp (I->inc, "01s"))) return true;
	return false;
}

char ** gmt_get_dataset_tiles (struct GMTAPI_CTRL *API, double wesn_in[], int k_data, unsigned int *n_tiles, bool *need_filler) {
	/* Return the full list of tiles for this tiled dataset. If need_filler is not NULL we return
	 * true if some of the tiles inside the wesn do not exist based on the coverage map. */
	bool partial_tile = false;
	char **list = NULL, YS, XS, file[GMT_LEN64] = {""}, tag[GMT_LEN64] = {""};
	int x, lon, clat, iw, ie, is, in, t_size;
	uint64_t node, row, col;
	unsigned int n_alloc = GMT_CHUNK, n = 0, n_missing = 0;
	double wesn[4];
	struct GMT_DATA_INFO *I = &API->remote_info[k_data];	/* Pointer to primary tiled dataset */
	struct GMT_GRID *Coverage = NULL;

	if (gmt_M_is_zero (I->tile_size)) return NULL;

	strncpy (tag, I->tag, GMT_LEN64);	/* Initialize tag since it may change below */
	if (strcmp (I->coverage, "-")) {	/* This primary tiled dataset has limited coverage as described by a named hit grid */
		char coverage_file[GMT_LEN64] = {""};
		sprintf (coverage_file, "@%s", I->coverage);	/* Prepend the remote flag since we may need to download the file */
		if ((Coverage = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, coverage_file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to obtain coverage grid %s of available tiles.\n", I->coverage);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
		if (gmtremote_is_earth_dem (I))	/* Dataset shares SRTM1|3 tiles with @earth_relief */
			sprintf (tag, "earth_relief_%s_%c", I->inc, I->reg);
	}

	if ((list = gmt_M_memory (API->GMT, NULL, n_alloc, char *)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to allocate memory.\n");
		API->error = GMT_RUNTIME_ERROR;
		return NULL;
	}

	/* Preprocess the selected wesn to make it work better for a -180/180 window */
	gmt_M_memcpy (wesn, wesn_in, 4U, double);
	if (gmt_M_360_range (wesn[XLO], wesn[XHI])) wesn[XLO] = -180, wesn[XHI] = +180.0;
	else if (wesn[XLO] < -180.0) wesn[XLO] += 360.0, wesn[XHI] += 360.0;
	else if (wesn[XLO] > +180.0) wesn[XLO] -= 360.0, wesn[XHI] -= 360.0;
	/* Get nearest whole multiple of tile size wesn boundary.  This ASSUMES all global grids are -Rd  */
	/* Also, the srtm_tiles.nc grid is gridline-registered and we check if the node corresponding to
	 * the lon/lat of the SW corner of a tile is 1 or 0 */
	iw = (int)(-180 + floor ((wesn[XLO] + 180) / I->tile_size) * I->tile_size);
	ie = (int)(-180 + ceil  ((wesn[XHI] + 180) / I->tile_size) * I->tile_size);
	is = (int)( -90 + floor ((wesn[YLO] +  90) / I->tile_size) * I->tile_size);
	in = (int)( -90 + ceil  ((wesn[YHI] +  90) / I->tile_size) * I->tile_size);
	t_size = rint (I->tile_size);

	for (clat = is; clat < in; clat += t_size) {	/* Loop over the rows of tiles */
		if (Coverage && (clat < Coverage->header->wesn[YLO] || clat >= Coverage->header->wesn[YHI])) continue;	/* Outside Coverage band */
		YS = (clat < 0) ? 'S' : 'N';
		for (x = iw; x < ie; x += t_size) {	/* Loop over the columns of tiles */
			lon = (x < 0) ? x + 360 : x;	/* Get longitude in 0-360 range */
			if (Coverage) {	/* We will assume nothing about the west/east bounds of the coverage grid */
				int clon = lon - 360;	/* Ensure we are far west */
				while (clon < Coverage->header->wesn[XLO]) clon += 360;	/* Wind until past west */
				if (clon > Coverage->header->wesn[XHI]) continue;	/* Outside Coverage band */
				row  = gmt_M_grd_y_to_row (GMT, (double)clat, Coverage->header);
				col  = gmt_M_grd_x_to_col (GMT, (double)clon, Coverage->header);
				node = gmt_M_ijp (Coverage->header, row, col);
				if (Coverage->data[node] == GMT_NO_TILE) {	/* No such tile exists */
					n_missing++;	/* Add up missing tiles */
					continue;		/* Go to next tile */
				}
				else if (Coverage->data[node] == GMT_PARTIAL_TILE)	/* Not missing, but still need ocean filler for partial tile */
					partial_tile = true;	/* Note: We also get here with GMT < 6.3 so that @earth_relief_15s is always considered */
			}
			lon = (x >= 180) ? x - 360 : x;	/* Need longitudes 0-179 for E and 1-180 for W */
			XS = (lon < 0) ? 'W' : 'E';
			/* Write remote tile name to list */
			if (n >= n_alloc) {
				n_alloc <<= 1; 
				if ((list = gmt_M_memory (API->GMT, list, n_alloc, char *)) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to reallocate memory.\n");
					API->error = GMT_RUNTIME_ERROR;
					return NULL;
				}
			}
			sprintf (file, "@%c%2.2d%c%3.3d.%s.%s", YS, abs(clat), XS, abs(lon), tag, GMT_TILE_EXTENSION_LOCAL);
			list[n++] = strdup (file);
		}
	}
	if (Coverage && GMT_Destroy_Data (API, &Coverage) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_WARNING, "gmtlib_get_tile_list: Unable to destroy coverage grid.\n");
	}

	*n_tiles = n;
	if (n == 0) {	/* Nutin' */
		gmt_M_free (API->GMT, list);
		GMT_Report (API, GMT_MSG_WARNING, "gmt_get_dataset_tiles: No %s tiles available for your region.\n", I->tag);
		return NULL;
	}

	if ((list = gmt_M_memory (API->GMT, list, n, char *)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to finalize memory.\n");
		API->error = GMT_RUNTIME_ERROR;
		return NULL;
	}
	if (need_filler) *need_filler = (partial_tile || n_missing > 0);	/* Incomplete coverage of this data set within wesn */

	return (list);
}

char *gmtlib_get_tile_list (struct GMTAPI_CTRL *API, double wesn[], int k_data, bool plot_region, unsigned int srtm_flag) {
	/* Builds a list of the tiles to download for the chosen region, dataset and resolution.
	 * Uses the optional tile information grid to know if a particular tile exists. */
	bool need_filler;
	char tile_list[PATH_MAX] = {""}, stem[GMT_LEN32] = {""}, *file = NULL, **tile = NULL, datatype[3] = {'L', 'O', 'X'}, regtype[2] = {'G', 'P'};
	int k_filler = GMT_NOTSET;
	unsigned int k, n_tiles = 0, ocean = (srtm_flag) ? 0 : 2;
	FILE *fp = NULL;
	struct GMT_DATA_INFO *Ip = &API->remote_info[k_data], *Is = NULL;	/* Pointer to primary tiled dataset */

	/* See if we want a background filler - this is most likely when using SRTM tiles and a 15s background ocean */
	if (strcmp (Ip->filler, "-") && srtm_flag == 0) {	/* Want background filler, except special case when srtm_relief is the given dataset name (srtm_flag == 1) */
		if ((k_filler = gmt_remote_dataset_id (API, Ip->filler)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Internal error - Filler grid %s is not a recognized remote data set.\n", Ip->filler);
			return NULL;			
		}
		Is = &API->remote_info[k_filler];	/* Pointer to secondary tiled dataset */
		ocean = (strcmp (Is->inc, "15s") == 0);
	}

	/* Create temporary filename for list of tiles */

	snprintf (stem, GMT_LEN32, "=tiled_%d_%c%c", k_data, regtype[plot_region], datatype[ocean]);
	if ((fp = gmt_create_tempfile (API, stem, NULL, tile_list)) == NULL) {	/* Not good... */
		GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Unable to create list of tiles from template: %s.\n", tile_list);
		return NULL;
	}
	file = tile_list;	/* Pointer to the buffer with the name */

	/* Get the primary tiles and determine if the filler grid is needed */
	tile = gmt_get_dataset_tiles (API, wesn, k_data, &n_tiles, &need_filler);

	/* Write primary tiles to list file */
	for (k = 0; k < n_tiles; k++)
		fprintf (fp, "%s\n", tile[k]);

	gmt_free_list (API->GMT, tile, n_tiles);	/* Free the primary tile list */
	if (k_filler != GMT_NOTSET) {	/* Want the secondary tiles */
		if (need_filler && (tile = gmt_get_dataset_tiles (API, wesn, k_filler, &n_tiles, NULL))) {
			/* Write secondary tiles to list file */
			for (k = 0; k < n_tiles; k++)
				fprintf (fp, "%s\n", tile[k]);
			gmt_free_list (API->GMT, tile, n_tiles);	/* Free the secondary tile list */
		}
		if (Ip->d_inc < Is->d_inc) {
			/* If selected dataset has smaller increment that the filler grid then we adjust -R to be  a multiple of the larger spacing. */
			/* Enforce multiple of tile grid resolution in wesn so requested region is in phase with tiles and at least covers the given
			 * region. The GMT_CONV8_LIMIT is there to ensure we won't round an almost exact x/dx away from the truth. */
			wesn[XLO] = floor ((wesn[XLO] / Is->d_inc) + GMT_CONV8_LIMIT) * Is->d_inc;
			wesn[XHI] = ceil  ((wesn[XHI] / Is->d_inc) - GMT_CONV8_LIMIT) * Is->d_inc;
			wesn[YLO] = floor ((wesn[YLO] / Is->d_inc) + GMT_CONV8_LIMIT) * Is->d_inc;
			wesn[YHI] = ceil  ((wesn[YHI] / Is->d_inc) - GMT_CONV8_LIMIT) * Is->d_inc;
		}
	}
	fclose (fp);

	gmt_M_memcpy (API->tile_wesn, wesn, 4, double);	/* Retain this knowledge in case it was obtained via map_setup for an oblique area */

	return (strdup (file));
}

struct GMT_GRID *gmtlib_assemble_tiles (struct GMTAPI_CTRL *API, double *region, char *file) {
	/* Get here if file is a =tiled_<id>_G|P.xxxxxx file.  Need to do:
	 * Set up a grdblend command and return the assembled grid
	 */
	int k_data, v_level = API->verbose;
	struct GMT_GRID *G = NULL;
	double *wesn = (region) ? region : API->tile_wesn;	/* Default to -R */
	char grid[GMT_VF_LEN] = {""}, cmd[GMT_LEN256] = {""}, code = 0;;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;

	(void) gmt_file_is_tiled_list (API, file, NULL, &code, NULL);	/* Just get the code*/

	if ((k_data = gmt_get_tile_id (API, file)) == GMT_NOTSET) {
		GMT_Report (API, GMT_MSG_ERROR, "Internal error: Non-recognized tiled ID embedded in file %s\n", file);
		return NULL;
	}

	if (API->verbose == GMT_MSG_WARNING) API->verbose = GMT_MSG_ERROR;	/* Drop from warnings to errors only when calling grdblend to avoid annoying messages about phase/shift from SRTM01|3 and 15s */
	GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT|GMT_IS_REFERENCE, NULL, grid);
	/* Pass -N0 so that missing tiles (oceans) yield z = 0 and not NaN, and -Co+n to override using negative earth_relief_15s values */
	snprintf (cmd, GMT_LEN256, "%s -R%.16g/%.16g/%.16g/%.16g -I%s -r%c -G%s -fg -Co+n", file, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], API->remote_info[k_data].inc, API->remote_info[k_data].reg, grid);
	if (code != 'X') strcat (cmd, " -N0 -Ve");	/* If ocean/land, set empty nodes to 0, else NaN. Also turn of warnings since mixing pixel and gridline grids, possibly */
	if (GMT_Call_Module (API, "grdblend", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
		API->verbose = v_level;
		GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to produce blended grid from %s\n", file);
		return NULL;
	}
	if ((G = GMT_Read_VirtualFile (API, grid)) == NULL) {	/* Load in the resampled grid */
		API->verbose = v_level;
		GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to receive blended grid from grdblend\n");
		return NULL;
	}

	API->verbose = v_level;
	HH = gmt_get_H_hidden (G->header);
	HH->orig_datatype = GMT_SHORT;	/* Since we know this */
	return (G);
}

int gmt_download_tiles (struct GMTAPI_CTRL *API, char *list, unsigned int mode) {
	/* Download all tiles not already here given by the list */
	uint64_t n, k;
	char **file = NULL;

	if (!gmt_file_is_tiled_list (API, list, NULL, NULL, NULL)) return GMT_RUNTIME_ERROR;
	if ((n = gmt_read_list (API->GMT, list, &file)) == 0) return GMT_RUNTIME_ERROR;
	for (k = 0; k < n; k++) {
		gmt_download_file_if_not_found (API->GMT, file[k], mode);
	}
	gmt_free_list (API->GMT, file, n);
	return GMT_NOERROR;
}

char *gmt_dataserver_url (struct GMTAPI_CTRL *API) {
	/* Build the full URL to the currently selected data server */
	static char URL[GMT_LEN256] = {""}, *link = URL;
	if (strncmp (API->GMT->session.DATASERVER, "http", 4U)) {	/* Not an URL so must assume it is the country/unit name, e.g., oceania */
		/* We make this part case insensitive since all official GMT servers are lower-case */
		char name[GMT_LEN64] = {""};
		strncpy (name, API->GMT->session.DATASERVER, GMT_LEN64-1);
		gmt_str_tolower (name);
		snprintf (URL, GMT_LEN256-1, "http://%s.generic-mapping-tools.org", name);
	}
	else	/* Must use the URL as is */
		snprintf (URL, GMT_LEN256-1, "%s", API->GMT->session.DATASERVER);
	return (link);
}
