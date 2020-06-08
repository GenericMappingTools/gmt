/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifdef	__APPLE__
	/* Apple Xcode expects _Nullable to be defined but it is not if gcc */
#ifndef _Nullable
#	define _Nullable
#	endif
#	endif

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

GMT_LOCAL struct GMT_DATA_INFO *gmtremote_data_load (struct GMT_CTRL *GMT, int *n) {
	/* Read contents of the info file into an array of structs */
	int k = 0, nr;
	FILE *fp = NULL;
	struct GMT_DATA_INFO *I = NULL;
	char unit, line[GMT_LEN512] = {""}, file[PATH_MAX] = {""}, *c = NULL;

	snprintf (file, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, GMT_INFO_SERVER_FILE);

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Load contents from %s\n", file);
	*n = 0;
	if ((fp = fopen (file, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open file %s\n", file);
		return NULL;
	}
	if (fgets (line, GMT_LEN256, fp) == NULL) {	/* Try to get first record */
		fclose (fp);
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Read error first record in file %s\n", file);
		return NULL;
	}
	*n = atoi (line);		/* Number of non-commented records to follow */
	if (*n <= 0 || *n > GMT_BIG_CHUNK) {	/* Probably not a good value */
		fclose (fp);
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad record counter in file %s\n", file);
		return NULL;
	}
	if ((I = gmt_M_memory (GMT, NULL, *n, struct GMT_DATA_INFO)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to allocated %d GMT_DATA_INFO structures!\n", *n);
		return NULL;
	}

	while (fgets (line, GMT_LEN512, fp) != NULL) {
		if (line[0] == '#') continue;	/* Skip any comments */
		if ((nr = sscanf (line, "%s %s %s %c %lg %lg %s %lg %s %s %s %[^\n]", I[k].dir, I[k].file, I[k].inc, &I[k].reg, &I[k].scale, &I[k].offset, I[k].size, &I[k].tile_size, I[k].date, I[k].coverage, I[k].filler, I[k].remark)) != 12) {
			/* New format - soon the only format once testing of 6.1 is over */
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s should have 12 fields but only %d read for record %d - download error???\n", file, nr, k);
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
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s said it has %d records but only found %d - download error???\n", file, *n, k);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s should be deleted.  Please try again\n", file);
		*n = 0;	/* Flag that excrement hit the fan */
	}
	/* Soft alphabetically on file names */
	qsort (I, *n, sizeof (struct GMT_DATA_INFO), gmtremote_compare_names);
	for (k = 0; k < *n; k++) I[k].id = k;	/* Give running number as ID in the sorted array */
	return (I);
};

GMT_LOCAL int gmtremote_compare_key (const void *item_1, const void *item_2) {
	/* We are passing string item_1 without any leading @ */
	const char *name_1 = (char *)item_1;
	const char *name_2 = ((struct GMT_DATA_INFO *)item_2)->file;
	size_t len = strlen (name_1);

	return (strncmp (name_1, name_2, len));
}

int gmt_remote_dataset_id (struct GMTAPI_CTRL *API, const char *file) {
	/* Return the entry in the remote file table of file is found, else -1 */
	int pos = 0;
	struct GMT_DATA_INFO *key = NULL;
	if (file == NULL || file[0] == '\0') return GMT_NOTSET;	/* No file name given */
	if (file[0] == '@') pos = 1;	/* Skip any leading remote flag */
	key = bsearch (&file[pos], API->remote_info, API->n_remote_info, sizeof (struct GMT_DATA_INFO), gmtremote_compare_key);
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
	char newfile[GMT_LEN256] = {""}, reg[2] = {'p', 'g'}, *file = NULL, *infile = NULL, *ext = NULL;
	int k_data, k;
	if (file_ptr == NULL || (file = *file_ptr) == NULL || file[0] == '\0') return;
	if (file[0] != '@') return;
	infile = strdup (file);
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
	if (!API->server_announced && !strchr (file, ':')) {	/* Server file has no http:// here */
		if ((c = strrchr (API->GMT->session.DATASERVER, '/')))	/* FOund last slash in http:// */
			strcpy (name, ++c);
		else /* Just in case */
			strcpy (name, API->GMT->session.DATASERVER);
		if ((c = strchr (name, '.'))) c[0] = '\0';	/* Chop off stuff after the initial name */
		gmt_str_toupper (name);
		GMT_Report (API, GMT_MSG_NOTICE, "Remote data courtesy of GMT data server %s [%s]\n\n", name, API->GMT->session.DATASERVER);
		API->server_announced = true;
	}
	if (key == GMT_NOTSET)	/* A Cache file */
		GMT_Report (API, GMT_MSG_NOTICE, "  -> Download cache file: %s\n", file);
	else {
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

GMT_LOCAL size_t gmtremote_skip_large_files (struct GMT_CTRL *GMT, char* URL, size_t limit) {
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

/* Deal with hash values of cache/data files */

#define GMT_HASH_TIME_OUT 10L	/* Not waiting longer than this to time out on getting the hash file */

GMT_LOCAL int gmtremote_get_url (struct GMT_CTRL *GMT, char *url, char *file, char *orig, unsigned int index) {
	int curl_err = 0;
	long time_spent;
	CURL *Curl = NULL;
	struct FtpFile urlfile = {NULL, NULL};
	time_t begin, end;

	if ((Curl = curl_easy_init ()) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to initiate curl - cannot obtain %s\n", url);
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_SSL_VERIFYPEER, 0L)) {		/* Tell libcurl to not verify the peer */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to not verify the peer\n");
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FOLLOWLOCATION, 1L)) {		/* Tell libcurl to follow 30x redirects */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to follow redirects\n");
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FAILONERROR, 1L)) {		/* Tell libcurl to fail on 4xx responses (e.g. 404) */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to fail for 4xx responses\n");
		return 1;
	}
 	if (curl_easy_setopt (Curl, CURLOPT_URL, url)) {	/* Set the URL to copy */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to read from %s\n", url);
		return 1;
	}
 	if (curl_easy_setopt (Curl, CURLOPT_TIMEOUT, GMT_HASH_TIME_OUT)) {	/* Set a max timeout */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to time out after %ld seconds\n", GMT_HASH_TIME_OUT);
		return 1;
	}
	urlfile.filename = file;	/* Set pointer to local filename */
	/* Define our callback to get called when there's data to be written */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, gmtremote_fwrite_callback)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl output callback function\n");
		return 1;
	}
	/* Set a pointer to our struct to pass to the callback */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, &urlfile)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to write to %s\n", file);
		return 1;
	}
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	gmtremote_turn_on_ctrl_C_check (file);
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
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "You can turn remote file download off by setting GMT_AUTO_DOWNLOAD off.\n");
			if (orig && !access (orig, F_OK)) {	/* Refresh modification time of original hash file */
#ifdef WIN32
				_utime (orig, NULL);
#else
				utimes (orig, NULL);
#endif
				GMT->current.io.refreshed[index] = GMT->current.io.internet_error = true;
			}
		}
		return 1;
	}
	curl_easy_cleanup (Curl);
	if (urlfile.fp) /* close the local file */
		fclose (urlfile.fp);
	gmtremote_turn_off_ctrl_C_check ();
	return 0;
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

GMT_LOCAL int gmtremote_refresh (struct GMT_CTRL *GMT, unsigned int index) {
	/* This function is called every time we are about to access a @remotefile.
	 * It is called twice: Once for the hash table and once for the info table.
	 * First we check that we have the GMT_HASH_SERVER_FILE in the server directory.
	 * If we don't then we download it and return since no old file to compare to.
	 * If we do find the hash file then we get its creation time [st_mtime] as
	 * well as the current system time.  If the file is < 1 day old we are done.
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

	if (GMT->current.io.refreshed[index]) return GMT_NOERROR;	/* Already been here */

	snprintf (indexpath, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, index_file);

	if (access (indexpath, R_OK)) {    /* Not found locally so need to download the first time */
		char serverdir[PATH_MAX] = {""};
		snprintf (serverdir, PATH_MAX, "%s/server", GMT->session.USERDIR);
		if (access (serverdir, R_OK) && gmt_mkdir (serverdir)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create GMT server directory : %s\n", serverdir);
			return 1;
		}
		snprintf (url, PATH_MAX, "%s/%s", GMT->session.DATASERVER, index_file);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Download remote file %s for the first time\n", url);
		if (gmtremote_get_url (GMT, url, indexpath, NULL, index)) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Failed to get remote file %s\n", url);
			if (!access (indexpath, F_OK)) gmt_remove_file (GMT, indexpath);	/* Remove index file just in case it got corrupted or zero size */
			GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;	/* Temporarily turn off auto download in this session only */
			GMT->current.io.internet_error = true;		/* No point trying again */
			return 1;
		}
		GMT->current.io.refreshed[index] = true;	/* Done our job */
		return GMT_NOERROR;
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Local file %s found\n", indexpath);

	GMT->current.io.refreshed[index] = true;	/* Done our job */

	/* Here we have the existing index file and its path is in indexpath. Check how old it is */

	if (stat (indexpath, &buf)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to get information about %s - abort\n", indexpath);
		return 1;
	}
	/*  Get its modification (creation) time */
#ifdef __APPLE__
	mod_time = buf.st_mtimespec.tv_sec;	/* Apple even has tv_nsec for nano-seconds... */
#else
	mod_time = buf.st_mtime;
#endif

	if ((right_now - mod_time) > GMT_DAY2SEC_I) {	/* Older than 1 day; Time to get a new index file */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s older than 24 hours, get latest from server.\n", indexpath);
		strcpy (new_indexpath, indexpath);	/* Duplicate path name */
		strcat (new_indexpath, ".new");		/* Append .new to the copied path */
		strcpy (old_indexpath, indexpath);	/* Duplicate path name */
		strcat (old_indexpath, ".old");		/* Append .old to the copied path */
		snprintf (url, PATH_MAX, "%s/%s", GMT->session.DATASERVER, index_file);	/* Set remote path to new index file */
		if (gmtremote_get_url (GMT, url, new_indexpath, indexpath, index)) {	/* Get the new index file from server */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Failed to download %s - Internet troubles?\n", url);
			if (!access (new_indexpath, F_OK)) gmt_remove_file (GMT, new_indexpath);	/* Remove index file just in case it got corrupted or zero size */
			return 1;	/* Unable to update the file (no Internet?) - skip the tests */
		}
		if (!access (old_indexpath, F_OK))
			remove (old_indexpath);	/* Remove old index file if it exists */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s to %s\n", indexpath, old_indexpath);
		if (gmt_rename_file (GMT, indexpath, old_indexpath, GMT_RENAME_FILE)) {	/* Rename existing file to .old */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", indexpath, old_indexpath);
			return 1;
		}
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s to %s\n", new_indexpath, indexpath);
		if (gmt_rename_file (GMT, new_indexpath, indexpath, GMT_RENAME_FILE)) {	/* Rename newly copied file to existing file */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", new_indexpath, indexpath);
			return 1;
		}

		if (index == GMT_HASH_INDEX) {	/* Special processing to upgrade or remove deprecated files */
			bool found;
			int nO, nN, n, o;
			struct GMT_DATA_HASH *O = NULL, *N = NULL;

			if ((N = gmtremote_hash_load (GMT, indexpath, &nN)) == 0) {	/* Read in the new array of hash structs, will return 0 if mismatch of entries */
				gmt_remove_file (GMT, indexpath);	/* Remove corrupted index file */
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
							GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Server and cache versions of %s have different hash codes - must download new copy.\n", N[n].name);
							gmt_remove_file (GMT, url);	/* Need to re-download so be gone with it */
						}
						else {	/* Do size check */
							struct stat buf;
							if (stat (url, &buf)) {
								GMT_Report (GMT->parent, GMT_MSG_WARNING, "Could not determine size of file %s.\n", url);
								continue;
							}
							if (N[n].size != (size_t)buf.st_size) {	/* Downloaded file size differ - need to re-download */
								GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Server and cache versions of %s have different byte sizes (%" PRIuS " versus %" PRIuS ") - must download new copy.\n", N[n].name, N[n].size, (size_t)buf.st_size);
								gmt_remove_file (GMT, url);	/* Need to re-download so be gone with it */
							}
							else
								GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Server and cache versions of %s are identical - no need to download new file.\n", N[n].name);
						}
					}
				}
				if (!found) {	/* This file was present locally but is no longer part of files on the server and should be removed */
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s no longer supported on server - deleting local copy.\n", O[o].name);
					gmt_remove_file (GMT, url);
				}
			}
			gmt_M_free (GMT, O);	/* Free old hash table structures */
			gmt_M_free (GMT, N);	/* Free new hash table structures */
			/* We now have an updated hash file */
			if (!access (old_indexpath, F_OK))
				remove (old_indexpath);	/* Remove old index file if it exists */
		}
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s less than 24 hours old, refresh is premature.\n", indexpath);
	return GMT_NOERROR;
}

void gmtlib_refresh_server (struct GMT_CTRL *GMT) {
	/* Called once in gmt_begin from GMT_Create_Session,  The following actions take place:
	 *
	 * The data info table is refreshed if missing or older than 24 hours.
	 * The hash table is refreshed if missing or older than 24 hours.
	 *   If a new hash table is obtained and there is a previous one, we determine
	 *   if there are entries that have changed (e.g., newer, different files, different
	 *   sizes, or gone altogether).  In all these case we delete the file so that when the
	 *   user requests it, it forces a download of the updated file.
	 */

	if (gmtremote_refresh (GMT, GMT_INFO_INDEX))	/* Watch out for changes on the server info once a day */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Unable to obtain remote information file %s\n", GMT_INFO_SERVER_FILE);
	else {	/* Get server file attribution info */
		if ((GMT->parent->remote_info = gmtremote_data_load (GMT, &GMT->parent->n_remote_info)) == NULL) {	/* Failed to load the info file */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Unable to read server information file\n");
		}
	}

	if (gmtremote_refresh (GMT, GMT_HASH_INDEX)) {	/* Watch out for changes on the server hash once a day */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Unable to obtain remote hash table %s\n", GMT_HASH_SERVER_FILE);
	}
}

GMT_LOCAL char * gmtremote_get_jp2_tilename (char *file) {
	/* Must do special legacy checks for  SRTMGL1|3 tag names for SRTM tiles.
	 * We also strip off the leading @ since we are building an URL for curl  */
	char res = 0, *c = NULL, *new_file = NULL;
	if ((c = strstr (file, ".earth_relief_01s_g")))
		res = '1';
	else if ((c = strstr (file, ".earth_relief_03s_g")))
		res = '3';
	if (c) {	/* Found one of the SRTM tile families */
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
	static char *args = "=ns -fg -Vq --IO_NC4_DEFLATION_LEVEL=9 --GMT_HISTORY=false";
	char cmd[GMT_LEN512] = {""},  *ncfile = NULL;
	int k_data;

	if (API->GMT->current.io.leave_as_jp2) return GMT_NOERROR;	/* Conversion temporarily turned off by gmtget -N */
	if ((k_data = gmtlib_file_is_jpeg2000_tile (API, localfile)) == GMT_NOTSET) return GMT_NOERROR;	/* Nothing to do */

	/* Convert JP2 file to NC for local cache storage */
	ncfile = gmt_strrep (localfile, GMT_TILE_EXTENSION_REMOTE, GMT_TILE_EXTENSION_LOCAL);
	sprintf (cmd, "%s -G%s%s", localfile, ncfile, args);
	if (!doubleAlmostEqual (API->remote_info[k_data].scale, 1.0) || !gmt_M_is_zero (API->remote_info[k_data].offset)) {
		/* Integer is not the original data unit and/or has an offset - must scale/shift jp2 integers to units first */
		char extra[GMT_LEN64] = {""};
		sprintf (extra, " -Z+s%g+o%g", API->remote_info[k_data].scale, API->remote_info[k_data].offset);
		strcat (cmd, extra);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Convert SRTM tile from JPEG2000 to netCDF grid [%s]\n", ncfile);
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
		clean_file = gmt_get_filename (API, file, "honsuU");	/* Strip off any file modifier or netCDF directives */
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
			snprintf (local_path, PATH_MAX, "%s/%s", GMT->session.CACHEDIR, &file[1]);	/* This is where all cache files live */
			if (access (local_path, R_OK)) goto not_local;	/* No such file yet */
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Remote file %s exists locally as %s\n", file, local_path);
		remote_path[0] = '\0';	/* No need to get from elsewhere */
		return GMT_NOERROR;

not_local:	/* Get here if we failed to find a remote file already on disk */
		/* Set remote path */
		if (is_tile) {	/* Tile not yet downloaded, but must switch to .jp2 format on the server (and deal with legacy SRTM tile tags) */
			jp2_file = gmtremote_get_jp2_tilename ((char *)file);
			snprintf (remote_path, PATH_MAX, "%s%s%s%s", GMT->session.DATASERVER, GMT->parent->remote_info[t_data].dir, GMT->parent->remote_info[t_data].file, jp2_file);
		}
		else if (k_data == GMT_NOTSET) {	/* Cache file not yet downloaded */
			snprintf (remote_path, PATH_MAX, "%s/cache/%s", GMT->session.DATASERVER, &file[1]);
			if (mode == 0) mode = GMT_CACHE_DIR;	/* Just so we default to the cache dir for cache files */
		}
		else	/* Remote data set */
			snprintf (remote_path, PATH_MAX, "%s%s%s", GMT->session.DATASERVER, API->remote_info[k_data].dir, API->remote_info[k_data].file);

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
		GMT_Report (API, GMT_MSG_DEBUG, "Get remote file %s and write to %s\n", remote_path, local_path);

		return GMT_NOERROR;
	}

	/* URL files and queries and netcdf grids with directives all have characters like ? and = that
	 * are not part of an actual file name (but is part of a query).  Need to deal with those complexities
	 * here an keep track of if we got a query or a file request. */

	is_url = (gmt_M_file_is_url (file));	/* A remote file or query given via an URL */
	is_query = (gmt_M_file_is_query (file));	/* A remote file or query given via an URL */

	if ((c = strchr (file, '?')) && !strchr (file, '=')) {	/* Must be a netCDF sliced URL file so chop off the layer/variable specifications */
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

	clean_file = gmt_get_filename (API, file, "honsuU");	/* Strip off any file modifier or netCDF directives */
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
	char *c, tmp[GMT_LEN64] = {""};
	if (file == NULL || file[0] == '\0') return GMT_NOTSET;	/* Bad argument */
	if ((c = strrchr (file, '/')) == NULL) return GMT_NOTSET;	/* Get place of the last slash */
	sprintf (tmp, "@%s", &c[1]);	/* Now should have something like @N22W160.earth_relief_01m_p.jp2 */
	return (gmt_file_is_a_tile (API, tmp, GMT_REMOTE_DIR));
}

int gmt_download_file (struct GMT_CTRL *GMT, const char *name, char *url, char *localfile, bool be_fussy) {
	int curl_err, error;
	size_t fsize;
	CURL *Curl = NULL;
	struct FtpFile urlfile = {NULL, NULL};
	struct GMTAPI_CTRL *API = GMT->parent;

	if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) {  /* Not allowed to use remote copying */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Remote download is currently deactivated\n");
		return 1;
	}
	if (GMT->current.io.internet_error) return 1;   			/* Not able to use remote copying in this session */

	/* Check if the file is too big for our current limit */

	if ((fsize = gmtremote_skip_large_files (GMT, url, GMT->current.setting.url_size_limit))) {
		char *S = strdup (gmt_memory_use (fsize, 3));
		GMT_Report (API, GMT_MSG_WARNING, "File %s skipped as size [%s] exceeds limit set by GMT_DATA_SERVER_LIMIT [%s]\n", name, S, gmt_memory_use (GMT->current.setting.url_size_limit, 0));
		gmt_M_str_free (S);
		return 1;
	}

	/* Here we will try to download a file */

  	if ((Curl = curl_easy_init ()) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to initiate curl\n");
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_SSL_VERIFYPEER, 0L)) {		/* Tell libcurl to not verify the peer */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to not verify the peer\n");
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FOLLOWLOCATION, 1L)) {		/* Tell libcurl to follow 30x redirects */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to follow redirects\n");
		return 1;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FAILONERROR, 1L)) {		/* Tell libcurl to fail on 4xx responses (e.g. 404) */
		return 1;
	}

 	if (curl_easy_setopt (Curl, CURLOPT_URL, url)) {	/* Set the URL to copy */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to read from %s\n", url);
		return 1;
	}
	urlfile.filename = localfile;	/* Set pointer to local filename */
	/* Define our callback to get called when there's data to be written */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, gmtremote_fwrite_callback)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl output callback function\n");
		return 1;
	}
	/* Set a pointer to our struct to pass to the callback */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, &urlfile)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to set curl option to write to %s\n", localfile);
		return 1;
	}

	gmtremote_find_and_give_data_attribution (API, name);

	GMT_Report (API, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	gmtremote_turn_on_ctrl_C_check (localfile);
	if ((curl_err = curl_easy_perform (Curl))) {	/* Failed, give error message */
		if (be_fussy || !(curl_err == CURLE_REMOTE_FILE_NOT_FOUND || curl_err == CURLE_HTTP_RETURNED_ERROR)) {	/* Unexpected failure - want to bitch about it */
			GMT_Report (API, GMT_MSG_ERROR, "Libcurl Error: %s\n", curl_easy_strerror (curl_err));
			GMT_Report (API, GMT_MSG_WARNING, "You can turn remote file download off by setting GMT_AUTO_DOWNLOAD off.\n");
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
	gmtremote_turn_off_ctrl_C_check ();

	error = gmtremote_convert_jp2_to_nc (API, localfile);

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

char ** gmt_get_dataset_tiles (struct GMTAPI_CTRL *API, double wesn[], int k_data, unsigned int *n_tiles) {
	/* Return the full list of tiles for this tiled dataset */
	char **list = NULL, YS, XS, file[GMT_LEN64] = {""};
	int x, lon, lat, iw, ie, is, in,  t_size;
	uint64_t node, row, col;
	unsigned int n_alloc = GMT_CHUNK, n = 0;
	struct GMT_DATA_INFO *I = &API->remote_info[k_data];	/* Pointer to primary tiled dataset */
	struct GMT_GRID *Coverage = NULL;

	if (gmt_M_is_zero (I->tile_size)) return NULL;

	if (strcmp (I->coverage, "-")) {	/* This primary tiled dataset has limited coverage as described by a named hit grid */
		char coverage_file[GMT_LEN64] = {""};
		sprintf (coverage_file, "@%s", I->coverage);	/* Prepend the remote flag since we may need to download the file */
		if ((Coverage = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, coverage_file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to obtain coverage grid %s of available tiles.\n", I->coverage);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
	}

	if ((list = gmt_M_memory (API->GMT, NULL, n_alloc, char *)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "gmt_get_dataset_tiles: Unable to allocate memory.\n");
		API->error = GMT_RUNTIME_ERROR;
		return NULL;
	}

	/* Get nearest whole multiple of tile size wesn boundary.  This ASSUMES all global grids are -Rd  */
	iw = (int)(-180 + floor ((wesn[XLO] + 180) / I->tile_size) * I->tile_size);
	ie = (int)(-180 + ceil  ((wesn[XHI] + 180) / I->tile_size) * I->tile_size);
	is = (int)( -90 + floor ((wesn[YLO] +  90) / I->tile_size) * I->tile_size);
	in = (int)( -90 + ceil  ((wesn[YHI] +  90) / I->tile_size) * I->tile_size);
	t_size = rint (I->tile_size);

	for (lat = is; lat < in; lat += t_size) {	/* Loop over the rows of tiles */
		if (Coverage && (lat < Coverage->header->wesn[YLO] || lat > Coverage->header->wesn[YHI])) continue;	/* Outside Coverage band */
		YS = (lat < 0) ? 'S' : 'N';
		for (x = iw; x < ie; x += t_size) {	/* Loop over the columns of tiles */
			lon = (x < 0) ? x + 360 : x;	/* Get longitude in 0-360 range */
			if (Coverage) {
				if (lon < Coverage->header->wesn[XLO] || lon > Coverage->header->wesn[XHI]) continue;	/* Outside Coverage band */
				row  = gmt_M_grd_y_to_row (GMT, (double)lat, Coverage->header);
				col  = gmt_M_grd_x_to_col (GMT, (double)lon, Coverage->header);
				node = gmt_M_ijp (Coverage->header, row, col);
				if (Coverage->data[node] == 0) continue;	/* No such tile exists */
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
			sprintf (file, "@%c%2.2d%c%3.3d.%s.%s", YS, abs(lat), XS, abs(lon), I->tag, GMT_TILE_EXTENSION_LOCAL);
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

	return (list);
}

char *gmtlib_get_tile_list (struct GMTAPI_CTRL *API, double wesn[], int k_data, bool plot_region, unsigned int srtm_flag) {
	/* Builds a list of the tiles to download for the chosen region, dataset and resolution.
	 * Uses the optional tile information grid to know if a particular tile exists. */
	char tile_list[PATH_MAX] = {""}, *file = NULL, **tile = NULL, datatype[3] = {'L', 'O', 'X'}, regtype[2] = {'G', 'P'};
	unsigned int k, k_filler = GMT_NOTSET, n_tiles = 0, ocean = (srtm_flag) ? 0 : 2;
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

	if (API->GMT->current.setting.run_mode == GMT_MODERN) {	/* Isolation mode is baked in, so just use 000000 (or any 6 characters) as extension */
		snprintf (tile_list, PATH_MAX, "%s/=tiled_%d_%c%c.000000", API->GMT->parent->gwf_dir, k_data, regtype[plot_region], datatype[ocean]);
		file = tile_list;
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Unable to create list of tiles: %s.\n", file);
			return NULL;
		}
	}
	else {	/* Under classic mode we must create a unique filename for the list */
		char name[GMT_LEN32] = {""};
#ifndef _WIN32
		int fd = 0;
#endif
		if (API->tmp_dir)	/* Have a recognized temp directory */
			snprintf (tile_list, PATH_MAX, "%s/", API->tmp_dir);
		snprintf (name, GMT_LEN32, "=tiled_%d_%c%c.XXXXXX", k_data, regtype[plot_region], datatype[ocean]);
		strcat (tile_list, name);
#ifdef _WIN32
		if ((file = mktemp (tile_list)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Could not create temporary file name %s.\n", tile_list);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Unable to create list of tiles: %s.\n", file);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
#else
		if ((fd = mkstemp (tile_list)) == -1) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Could not create temporary file name %s.\n", tile_list);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
		file = tile_list;
		if ((fp = fdopen (fd, "w")) == NULL) {
			API->error = GMT_RUNTIME_ERROR;
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_tile_list: Could not fdopen the temporary file %s.\n", file);
			return NULL;
		}
#endif
	}

	/* Get the primary tiles */
	tile = gmt_get_dataset_tiles (API, wesn, k_data, &n_tiles);

	/* Write primary tiles to list file */
	for (k = 0; k < n_tiles; k++)
		fprintf (fp, "%s\n", tile[k]);

	gmt_free_list (API->GMT, tile, n_tiles);	/* Free the primary tile list */

	if (k_filler != GMT_NOTSET) {	/* Want the secondary tiles */
		if ((tile = gmt_get_dataset_tiles (API, wesn, k_filler, &n_tiles))) {
			/* Write secondary tiles to list file */
			for (k = 0; k < n_tiles; k++)
				fprintf (fp, "%s\n", tile[k]);
			gmt_free_list (API->GMT, tile, n_tiles);	/* Free the secondary tile list */
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
	}
	fclose (fp);

	gmt_M_memcpy (API->tile_wesn, wesn, 4, double);	/* Retain this knowledge in case it was obtained via map_setup for an oblique area */

	return (strdup (file));
}

struct GMT_GRID *gmtlib_assemble_tiles (struct GMTAPI_CTRL *API, double *region, char *file) {
	/* Get here if file is a =tiled_<id>_G|P.xxxxxx file.  Need to do:
	 * Set up a grdblend command and return the assembled grid
	 */
	int k_data;
	struct GMT_GRID *G = NULL;
	double *wesn = (region) ? region : API->tile_wesn;	/* Default to -R */
	char grid[GMT_VF_LEN] = {""}, cmd[GMT_LEN256] = {""};
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;

	if ((k_data = gmt_get_tile_id (API, file)) == GMT_NOTSET) {
		GMT_Report (API, GMT_MSG_ERROR, "Internal error: Non-recognized tiled ID embedded in file %s\n", file);
		return NULL;
	}

	GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, grid);
	/* Pass -N0 so that missing tiles (oceans) yield z = 0 and not NaN, and -Co- to override using negative earth_relief_15s values */
	snprintf (cmd, GMT_LEN256, "%s -R%.16g/%.16g/%.16g/%.16g -I%s -r%c -G%s -N0 -Co-", file, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], API->remote_info[k_data].inc, API->remote_info[k_data].reg, grid);
	if (GMT_Call_Module (API, "grdblend", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to produce blended grid from %s\n", file);
		return NULL;
	}
	if ((G = GMT_Read_VirtualFile (API, grid)) == NULL) {	/* Load in the resampled grid */
		GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to receive blended grid from grdblend\n");
		return NULL;
	}
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
