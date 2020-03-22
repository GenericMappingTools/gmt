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
#include "gmt_remote.h"
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
 * are bracketed by turn_on_ctrl_C_check and turn_off_ctrl_C_check which
 * temporarily activates or deactivates our signal action on Ctrl-C.
 * P. Wessel, June 30, 2019.
 */

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
#define GMT_CATCH_CTRL_C
#include <signal.h>
struct sigaction cleanup_action, default_action;
char *file_to_delete_if_ctrl_C;
#endif

static char *all_grid_files[GMT_N_DATASETS]= {
#include <gmt_datasets.h>
};

GMT_LOCAL void delete_file_then_exit (int sig_no)
{	/* If we catch a CTRL-C during CURL download we must assume file is corrupted and remove it before exiting */
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

GMT_LOCAL void turn_on_ctrl_C_check (char *file) {
#ifdef GMT_CATCH_CTRL_C
	file_to_delete_if_ctrl_C = file;			/* File to delete if CTRL-C is caught */
	gmt_M_memset (&cleanup_action, 1, struct sigaction);	/* Initialize the structure to NULL */
	cleanup_action.sa_handler = &delete_file_then_exit;	/* Set function we should call if CTRL-C is caught */
	sigaction(SIGINT, &cleanup_action, &default_action);	/* Activate the alternative signal checking */
#else
	gmt_M_unused (file);
#endif
}

GMT_LOCAL void turn_off_ctrl_C_check () {
#ifdef GMT_CATCH_CTRL_C
	file_to_delete_if_ctrl_C = NULL;		/* Remove trace of any file name */
	sigaction (SIGINT, &default_action, NULL);	/* Reset default signal action */
#endif
}

struct FtpFile {	/* Needed for argument to libcurl */
	const char *filename;
	FILE *fp;
};

GMT_LOCAL size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
	(void)ptr;
	(void)data;
	/* we are not interested in the headers itself,
	   so we only return the size we would have saved ... */
	return (size_t)(size * nmemb);
}

GMT_LOCAL size_t fwrite_callback (void *buffer, size_t size, size_t nmemb, void *stream) {
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out == NULL) return 0;	/* This cannot happen but Coverity fusses */
	if (!out->fp) { /* open file for writing */
		out->fp = fopen (out->filename, "wb");
		if (!out->fp)
			return -1; /* failure, can't open file to write */
	}
	return fwrite (buffer, size, nmemb, out->fp);
}

GMT_LOCAL int give_data_attribution (struct GMT_CTRL *GMT, char *file) {
	/* Print attribution when the @earth_relief_xxx.grd file is downloaded for the first time */
	char tag[4] = {""};
	int k, match = -1, len = (int)strlen(file);
	if (strstr (file, ".grd")) len -= 4;		/* If .grd extension was provided we must skip that as well */
	strncpy (tag, &file[len-3], 3U);	/* Get the xxy part of the file */
	for (k = 0; k < GMT_N_DATA_INFO_ITEMS; k++) {
		if (!strncmp (tag, gmt_data_info[k].tag, 3U)) {	/* Found the matching information */
			char name[GMT_LEN32] = {""}, *c = NULL;
			if ((c = strstr (file, ".grd"))) c[0] = '\0';	/* Chop off extension for this message */
			(len == 3) ? snprintf (name, GMT_LEN32, "earth_relief_%s", file) : snprintf (name, GMT_LEN32, "%s", &file[1]);
			if (len > 3) GMT_Report (GMT->parent, GMT_MSG_NOTICE, "%s: Download file from the GMT data server [data set size is %s].\n", name, gmt_data_info[k].size);
			GMT_Report (GMT->parent, GMT_MSG_NOTICE, "%s.\n\n", gmt_data_info[k].remark);
			match = k;
			if (c) c[0] = '.';	/* Restore extension */
		}
	}
	return (match == -1);	/* Not found */
}

GMT_LOCAL size_t skip_large_files (struct GMT_CTRL *GMT, char* URL, size_t limit) {
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
		curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, throw_away);
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

GMT_LOCAL int gmthash_get_url (struct GMT_CTRL *GMT, char *url, char *file, char *orig) {
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
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, fwrite_callback)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl output callback function\n");
		return 1;
	}
	/* Set a pointer to our struct to pass to the callback */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, &urlfile)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to write to %s\n", file);
		return 1;
	}
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	turn_on_ctrl_C_check (file);
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
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "You can turn remote file download off by setting GMT_DATA_SERVER_LIMIT = 0.\n");
			if (orig && !access (orig, F_OK)) {	/* Refresh modification time of original hash file */
#ifdef WIN32
				_utime (orig, NULL);
#else
				utimes (orig, NULL);
#endif
				GMT->current.io.hash_refreshed = GMT->current.io.internet_error = true;
			}
		}
		return 1;
	}
	curl_easy_cleanup (Curl);
	if (urlfile.fp) /* close the local file */
		fclose (urlfile.fp);
	turn_off_ctrl_C_check ();
	return 0;
}

struct GMT_DATA_HASH * hash_load (struct GMT_CTRL *GMT, char *file, int *n) {
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

GMT_LOCAL int hash_refresh (struct GMT_CTRL *GMT) {
	/* This function is called every time we are about to access a @remotefile.
	 * First we check that we have gmt_hash_server.txt in the server directory.
	 * If we don't then we download it and return since no old file to compare to.
	 * If we do find the hash file then we get its creation time [st_mtime] as
	 * well as the current system time.  If the file is < 1 day old we are done.
	 * If the file is older we rename it to *.old and download the latest hash file.
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
	char hashpath[PATH_MAX] = {""}, old_hashpath[PATH_MAX] = {""}, new_hashpath[PATH_MAX] = {""}, url[PATH_MAX] = {""};

	if (GMT->current.io.hash_refreshed) return 0;	/* Already been here */

	snprintf (hashpath, PATH_MAX, "%s/server/gmt_hash_server.txt", GMT->session.USERDIR);

	if (access (hashpath, R_OK)) {    /* Not found locally so need to download the first time */
		char serverdir[PATH_MAX] = {""};
		snprintf (serverdir, PATH_MAX, "%s/server", GMT->session.USERDIR);
		if (access (serverdir, R_OK) && gmt_mkdir (serverdir)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create GMT server directory : %s\n", serverdir);
			return 1;
		}
		snprintf (url, PATH_MAX, "%s/gmt_hash_server.txt", GMT->session.DATASERVER);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Download remote file %s for the first time\n", url);
		if (gmthash_get_url (GMT, url, hashpath, NULL)) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Failed to get remote file %s\n", url);
			if (!access (hashpath, F_OK)) gmt_remove_file (GMT, hashpath);	/* Remove hash file just in case it got corrupted or zero size */
			GMT->current.setting.auto_download = GMT_NO_DOWNLOAD;		/* Temporarily turn off auto download in this session only */
			GMT->current.io.internet_error = true;				/* No point trying again */
			return 1;
		}
		GMT->current.io.hash_refreshed = true;	/* Done our job */
		return 0;
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Local file %s found\n", hashpath);

	GMT->current.io.hash_refreshed = true;	/* Done our job */

	/* Here we have the existing hash file and its path is in hashpath */

	if (stat (hashpath, &buf)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to get information about %s - abort\n", hashpath);
		return 1;
	}
	/*  Get its modification (creation) time */
#ifdef __APPLE__
	mod_time = buf.st_mtimespec.tv_sec;	/* Apple even has tv_nsec for nan-seconds... */
#else
	mod_time = buf.st_mtime;
#endif

	if ((right_now - mod_time) > GMT_DAY2SEC_I) {	/* Older than 1 day; Time to get a new hash file */
		bool found;
		int nO, nN, n, o;
		struct GMT_DATA_HASH *O = NULL, *N = NULL;

		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s older than 24 hours, get latest from server.\n", hashpath);
		strcpy (new_hashpath, hashpath);	/* Duplicate path name */
		strcat (new_hashpath, ".new");		/* Append .new to the copied path */
		strcpy (old_hashpath, hashpath);	/* Duplicate path name */
		strcat (old_hashpath, ".old");		/* Append .old to the copied path */
		snprintf (url, PATH_MAX, "%s/gmt_hash_server.txt", GMT->session.DATASERVER);	/* Set remote path to new hash file */
		if (gmthash_get_url (GMT, url, new_hashpath, hashpath)) {	/* Get the new hash file from server */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Failed to download %s - Internet troubles?\n", url);
			if (!access (new_hashpath, F_OK)) gmt_remove_file (GMT, new_hashpath);	/* Remove hash file just in case it got corrupted or zero size */
			return 1;	/* Unable to update the file (no Internet?) - skip the tests */
		}
		if (!access (old_hashpath, F_OK))
			remove (old_hashpath);	/* Remove old hash file if it exists */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s to %s\n", hashpath, old_hashpath);
		if (gmt_rename_file (GMT, hashpath, old_hashpath, GMT_RENAME_FILE)) {	/* Rename existing file to .old */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", hashpath, old_hashpath);
			return 1;
		}
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s to %s\n", new_hashpath, hashpath);
		if (gmt_rename_file (GMT, new_hashpath, hashpath, GMT_RENAME_FILE)) {	/* Rename newly copied file to existing file */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to rename %s to %s.\n", new_hashpath, hashpath);
			return 1;
		}
		if ((N = hash_load (GMT, hashpath, &nN)) == 0) {	/* Read in the new array of hash structs, will return 0 if mismatch of entries */
			gmt_remove_file (GMT, hashpath);		/* Remove corrupted hash file */
			return 1;
		}

		O = hash_load (GMT, old_hashpath, &nO);	/* Read in the old array of hash structs */
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
		if (!access (old_hashpath, F_OK))
			remove (old_hashpath);	/* Remove old hash file if it exists */
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s less than 24 hours old, refresh is premature.\n", hashpath);
	return 0;
}

GMT_LOCAL bool is_not_a_valid_grid (char *file, char *all_grid_files[], int N) {
	/* Determine if file is among the valid datasets */
	for (int k = 0; k < N; k++) {
		if (!strncmp (all_grid_files[k], file, strlen (all_grid_files[k]))) return false;
	}
	return true;
}


unsigned int gmt_download_file_if_not_found (struct GMT_CTRL *GMT, const char* file_name, unsigned int mode) {
	/* Downloads a file if not found locally.  Returns the position in file_name of the
 	 * start of the actual file (e.g., if given an URL). Values for mode:
 	 * 0 : Place file in the cache directory
	 * 1 : Place file in user directory
	 * 2 : Place file in local (current) directory
	 * Add 4 if the file may not be found and we should not complain about this here.
	 */
	unsigned int kind = 0, pos = 0, from = 0, to = 0, res = 0, be_fussy;
	int curl_err = 0;
	bool is_srtm = false, is_data = false, is_url = false;
	size_t len, fsize;
	CURL *Curl = NULL;
	static char *cache_dir[4] = {"/cache", "", "/srtm1", "/srtm3"}, *name[3] = {"CACHE", "USER", "LOCAL"};
	char *user_dir[3] = {GMT->session.CACHEDIR, GMT->session.USERDIR, NULL};
	char url[PATH_MAX] = {""}, local_path[PATH_MAX] = {""}, *c = NULL, *file = NULL;
	char srtmdir[PATH_MAX] = {""}, serverdir[PATH_MAX] = {""}, *srtm_local = NULL;
	struct FtpFile urlfile = {NULL, NULL};

	if (!file_name || !file_name[0]) return 0;   /* Got nutin' */

	if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) return 0;   /* Not allowed to use remote copying */
	if (GMT->current.io.internet_error) return 0;   			/* Not able to use remote copying in this session */

	be_fussy = ((mode & 4) == 0);	if (be_fussy == 0) mode -= 4;	/* Handle the optional 4 value */

	file = gmt_M_memory (GMT, NULL, strlen (file_name)+2, char);	/* One extra in case need to change nc to jp2 for download of SRTM */
	strcpy (file, file_name);
	/* Because file_name may be <file>, @<file>, or URL/<file> we must find start of <file> */
	if (gmt_M_file_is_remotedata (file)) {	/* A remote @earth_relief_xxm|s grid */
		if (is_not_a_valid_grid (&file[1], all_grid_files, GMT_N_DATASETS)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid file %s is not a recognized remote grid\n", file);
			gmt_M_free (GMT, file);
			return 1;
		}
		pos = 1;
		is_data = true;
	}
	else if (gmt_M_file_is_cache (file)) {	/* A leading '@' was found */
		pos = 1;
		if ((c = strchr (file, '?')))	/* Netcdf directive since URL was handled above */
			c[0] = '\0';
		else if ((c = strchr (file, '=')))	/* Grid attributes */
			c[0] = '\0';
	}
	else if (gmt_M_file_is_url (file)) {	/* A remote file given via an URL */
		is_url = true;
		pos = gmtlib_get_pos_of_filename (file);	/* Start of file in URL (> 0) */
		if ((c = strchr (file, '?')) && !strchr (file, '='))	/* Must be a netCDF sliced URL file so chop off the layer/variable specifications */
			c[0] = '\0';
		else if (c && c[0] != '?' && (c = strchr (file, '=')))	/* If no ? then = means grid attributes */
			c[0] = '\0';
		/* else we have both ? and = which is an URL query */
	}
	else if ((c = strchr (file, '?')))	/* Netcdf directive since URLs and caches were handled above */
		c[0] = '\0';	/* and pos = 0 */

	/* Return immediately if cannot be downloaded (for various reasons) */
	if (!gmtlib_file_is_downloadable (GMT, file, &kind)) {
		gmt_M_free (GMT, file);
		return (pos);
	}

	if (hash_refresh (GMT)) {	/* Watch out for changes on the server once a day */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Unable to obtain remote file %s\n", file);
		gmt_M_free (GMT, file);
		return 1;
	}

	/* Any old files have now been replaced.  Now we can check if the file exists already */

	if (kind != GMT_URL_QUERY) {	/* Regular file, see if we have it already */
		bool found;
		if (kind == GMT_DATA_FILE) {	/* Special remote data est @earth_relief_xxm|s request */
			if (strstr (file, ".grd"))	/* User specified the extension */
				found = (!gmt_access (GMT, &file[pos], F_OK));
			else {	/* Must append the extension .grd */
				char *tmpfile = malloc (strlen (file) + 5);
				sprintf (tmpfile, "%s.grd", &file[pos]);
				found = (!gmt_access (GMT, tmpfile, F_OK));
				gmt_M_str_free (tmpfile);
			}
		}
		else
			found = (!gmt_access (GMT, &file[pos], F_OK));
		if (found) {	/* Got it already */
			gmt_M_free (GMT, file);
			return (pos);
		}
	}

	from = (kind == GMT_DATA_FILE) ? GMT_DATA_DIR : GMT_CACHE_DIR;	/* Determine source directory on cache server */
	to = (mode == GMT_LOCAL_DIR) ? GMT_LOCAL_DIR : from;
	snprintf (serverdir, PATH_MAX, "%s/server", user_dir[GMT_DATA_DIR]);
	if ((is_data || is_srtm) && access (serverdir, R_OK) && gmt_mkdir (serverdir))
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to create GMT data directory : %s\n", serverdir);
	if (gmt_file_is_srtmtile (GMT->parent, file, &res)) {	/* Select the right sub-dir on the server and cache locally */
		from = (res == 1) ? 2 : 3;
		to = GMT_CACHE_DIR;
		is_srtm = true;
		snprintf (srtmdir, PATH_MAX, "%s/srtm%d", serverdir, res);
		/* Check if srtm1|3 subdir exist - if not create it */
		if (access (srtmdir, R_OK) && gmt_mkdir (srtmdir))
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to create GMT data directory : %s\n", srtmdir);
	}
	if (mode == GMT_LOCAL_DIR || user_dir[to] == NULL) {
		if (mode != GMT_LOCAL_DIR)
			GMT_Report (GMT->parent, GMT_MSG_WARNING,
			            "The GMT_%s directory is not defined - download file to current directory\n", name[to]);
		snprintf (local_path, PATH_MAX, "%s", &file[pos]);
	}

	/* Create the remote URL */
	if (kind == GMT_URL_FILE || kind == GMT_URL_QUERY)	/* General URL given */
		snprintf (url, PATH_MAX, "%s", file);
	else {	/* Use GMT data dir, possible from subfolder cache */
		snprintf (url, PATH_MAX, "%s%s/%s", GMT->session.DATASERVER, cache_dir[from], &file[pos]);
		if (kind == GMT_DATA_FILE && !strstr (url, ".grd")) strcat (url, ".grd");	/* Must supply the .grd */
		len = strlen (url);
		if (is_srtm && !strncmp (&url[len-GMT_SRTM_EXTENSION_LOCAL_LEN-1U], ".nc", GMT_SRTM_EXTENSION_LOCAL_LEN+1U))
			strncpy (&url[len-GMT_SRTM_EXTENSION_LOCAL_LEN], GMT_SRTM_EXTENSION_REMOTE, GMT_SRTM_EXTENSION_REMOTE_LEN);	/* Switch extension for download */
	}

	if ((fsize = skip_large_files (GMT, url, GMT->current.setting.url_size_limit))) {
		char *S = strdup (gmt_memory_use (fsize, 3));
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "File %s skipped as size [%s] exceeds limit set by GMT_DATA_SERVER_LIMIT [%s]\n", &file[pos], S, gmt_memory_use (GMT->current.setting.url_size_limit, 0));
		gmt_M_free (GMT, file);
		gmt_M_str_free (S);
		return 0;
	}

	/* Here we will try to download a file */

  	if ((Curl = curl_easy_init ()) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to initiate curl - cannot obtain %s\n", &file[pos]);
		gmt_M_free (GMT, file);
		return 0;
	}
	if (curl_easy_setopt (Curl, CURLOPT_SSL_VERIFYPEER, 0L)) {		/* Tell libcurl to not verify the peer */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to not verify the peer\n");
		gmt_M_free (GMT, file);
		return 0;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FOLLOWLOCATION, 1L)) {		/* Tell libcurl to follow 30x redirects */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to follow redirects\n");
		gmt_M_free (GMT, file);
		return 0;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FAILONERROR, 1L)) {		/* Tell libcurl to fail on 4xx responses (e.g. 404) */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to fail for 4xx responses\n");
		gmt_M_free (GMT, file);
		return 0;
	}

	if (mode != GMT_LOCAL_DIR && user_dir[to]) {
		if (is_srtm) {	/* Doing SRTM tiles */
			snprintf (local_path, PATH_MAX, "%s/%s", srtmdir, &file[pos]);
			srtm_local = strdup (local_path);	/* What we want the local file to be called */
			len = strlen (local_path);
			if (!strncmp (&local_path[len-GMT_SRTM_EXTENSION_LOCAL_LEN-1U], ".nc", GMT_SRTM_EXTENSION_LOCAL_LEN+1U))
				strncpy (&local_path[len-GMT_SRTM_EXTENSION_LOCAL_LEN], GMT_SRTM_EXTENSION_REMOTE, GMT_SRTM_EXTENSION_REMOTE_LEN);	/* Switch extension for download */
		}
		else if (is_data) {
			snprintf (local_path, PATH_MAX, "%s/server", user_dir[GMT_DATA_DIR]);
			if (access (local_path, R_OK) && gmt_mkdir (local_path))
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create GMT data directory : %s\n", local_path);
			snprintf (local_path, PATH_MAX, "%s/server/%s", user_dir[GMT_DATA_DIR], &file[pos]);
		}
		else if (is_url) {	/* Plaec in current dir */
			snprintf (local_path, PATH_MAX, "%s", &file[pos]);
		}
		else {	/* Goes to cache */
			if (access (user_dir[to], R_OK) && gmt_mkdir (user_dir[to]))
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create GMT data directory : %s\n", user_dir[to]);
			snprintf (local_path, PATH_MAX, "%s/%s", user_dir[to], &file[pos]);
		}
	}
	if (kind == GMT_DATA_FILE && !strstr (local_path, ".grd")) strcat (local_path, ".grd");	/* Must supply the .grd */
	if (kind == GMT_URL_QUERY) {	/* Cannot have ?para=value etc in filename */
		c = strchr (local_path, '?');
		if (c) c[0] = '\0';	/* Chop off ?CGI parameters from local_path */
	}

 	if (curl_easy_setopt (Curl, CURLOPT_URL, url)) {	/* Set the URL to copy */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to read from %s\n", url);
		gmt_M_free (GMT, file);
		if (is_srtm) gmt_M_str_free (srtm_local);
		return 0;
	}
	urlfile.filename = local_path;	/* Set pointer to local filename */
	/* Define our callback to get called when there's data to be written */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, fwrite_callback)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl output callback function\n");
		gmt_M_free (GMT, file);
		if (is_srtm) gmt_M_str_free (srtm_local);
		return 0;
	}
	/* Set a pointer to our struct to pass to the callback */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, &urlfile)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to set curl option to write to %s\n", local_path);
		gmt_M_free (GMT, file);
		if (is_srtm) gmt_M_str_free (srtm_local);
		return 0;
	}
	if (kind == GMT_DATA_FILE) give_data_attribution (GMT, file);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Downloading file %s ...\n", url);
	turn_on_ctrl_C_check (local_path);
	if ((curl_err = curl_easy_perform (Curl))) {	/* Failed, give error message */
		if (be_fussy || !(curl_err == CURLE_REMOTE_FILE_NOT_FOUND || curl_err == CURLE_HTTP_RETURNED_ERROR)) {	/* Unexpected failure - want to bitch about it */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Libcurl Error: %s\n", curl_easy_strerror (curl_err));
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "You can turn remote file download off by setting GMT_DATA_SERVER_LIMIT = 0.\n");
			if (urlfile.fp != NULL) {
				fclose (urlfile.fp);
				urlfile.fp = NULL;
			}
			if (!access (local_path, F_OK) && gmt_remove_file (GMT, local_path))	/* Failed to clean up as well */
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Could not even remove file %s\n", local_path);
		}
		else if (curl_err == CURLE_COULDNT_CONNECT)
			GMT->current.io.internet_error = true;	/* Prevent GMT from trying again in this session */
	}
	curl_easy_cleanup (Curl);
	if (urlfile.fp) /* close the local file */
		fclose (urlfile.fp);
	turn_off_ctrl_C_check ();
	if (kind == GMT_URL_QUERY) {	/* Cannot have ?para=value etc in local filename */
		c = strchr (file_name, '?');
		if (c) c[0] = '\0';	/* Chop off ?CGI parameters from local_path */
	}

	if (is_srtm && srtm_local) {	/* Convert JP2 file to NC for local cache storage */
		static char *args = "=ns -fg -Vq --IO_NC4_DEFLATION_LEVEL=9 --GMT_HISTORY=false";
		char *cmd = gmt_M_memory (GMT, NULL, strlen (local_path) + strlen (srtm_local) + strlen(args) + 2, char);
		sprintf (cmd, "%s %s%s", local_path, srtm_local, args);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Convert SRTM tile from JPEG2000 to netCDF grid [%s]\n", file);
		if (GMT_Call_Module (GMT->parent, "grdconvert", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR - Unable to convert SRTM file %s to compressed netCDF format\n", local_path);
			gmt_M_free (GMT, file);
			gmt_M_free (GMT, cmd);
			if (is_srtm) gmt_M_str_free (srtm_local);
			return 0;
		}
		gmt_M_free (GMT, cmd);
		if (gmt_remove_file (GMT, local_path))
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Could not even remove file %s\n", local_path);
		strcpy (local_path, srtm_local);
		gmt_M_str_free (srtm_local);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {	/* Say a few things about the file we got */
		struct stat buf;
		if (stat (local_path, &buf))
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Could not determine size of downloaded file %s\n", &file_name[pos]);
		else
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Download complete [Got %s].\n", gmt_memory_use (buf.st_size, 3));
	}
	gmt_M_free (GMT, file);

	return (pos);
}

/* Support functions for SRTM tile grids
 * gmtlib_file_is_srtm:	Determine if a SRTM 1 or 3 arc second request was given
 * gmtlib_file_is_srtmlist: Determine if file is a listfile with SRTM tile info
 * gmtlib_get_srtmlist:	Convert -Rw/e/s/n into a file with a list of the tiles needed
 */

bool gmtlib_file_is_srtmrequest (struct GMTAPI_CTRL *API, const char *file, unsigned int *res, bool *ocean) {
	size_t len1 = strlen (GMT_DATA_PREFIX), len2 = strlen (GMT_SRTM_PREFIX);	/* Note: len[12] do not include the leading @ character */
	size_t len, flen = strlen (file);	/* Note: len1 does not include the leading @ character */
	gmt_M_unused (API);
	if (file[0] != '@') return false;	/* Not a remote file */
	if (strncmp (&file[1], GMT_DATA_PREFIX, len1) && strncmp (&file[1], GMT_SRTM_PREFIX, len2)) return false;	/* Not a remote earth_relief_xxx or srtm_relief_xxx grid */
	len = (!strncmp (&file[1], GMT_DATA_PREFIX, len1)) ? len1 : len2;	/* Get the corresponding length */
	if ((len+3) >= flen) return false;	/* Prevent accessing beyond length of file */
	if (file[len+3] != 's') return false;	/* Not an arcsec grid */
	if (!(file[len+1] == '0' && (file[len+2] == '1' || file[len+2] == '3'))) return false;	/* Not 1 or 3 arc sec */
	/* Yes: a 1 or 3 arc sec SRTM request, set res to 1 or 3 */
	*res = (unsigned int)(file[len+2] - '0');
	*ocean = (len == len1);	/* If we asked for earth_relief_xxx then we resample the ocean side as well, else pure SRTM land */
	return true;
}

bool gmtlib_file_is_srtmlist (struct GMTAPI_CTRL *API, const char *file) {
	size_t len = strlen(file);
	gmt_M_unused (API);
	if (len < 13) return false;	/* Too short a filename */
	if (strncmp (&file[len-13], "=srtm", 5U)) return false;	/* Not that kind of file */
	return true;	/* We got one */
}

bool gmt_file_is_srtmtile (struct GMTAPI_CTRL *API, const char *file, unsigned int *res) {
	/* Recognizes remote files like N42E126.SRTMGL1.nc */
	char *p = NULL;
	gmt_M_unused (API);
	if ((p = strstr (file, ".SRTMGL")) == NULL) return false;	/* Nope */
	*res = (unsigned int)(p[7] - '0');
	return true;	/* Found it */
}

char *gmtlib_get_srtmlist (struct GMTAPI_CTRL *API, double wesn[], unsigned int res, bool ocean) {
	/* Builds a list of SRTM tile to download for the chosen region and resolution.
	 * Uses the srtm_tiles.nc grid on the cache to know if a 1x1 degree has a tile. */
	int x, lon, lat, iw, ie, is, in, n_tiles = 0;
	uint64_t node;
	char srtmlist[PATH_MAX] = {""}, YS, XS, *file = NULL;
	FILE *fp = NULL;
	struct GMT_GRID *SRTM = NULL;

	/* Get nearest whole integer wesn boundary */
	iw = (int)floor (wesn[XLO]);	ie = (int)ceil (wesn[XHI]);
	is = (int)floor (wesn[YLO]);	in = (int)ceil (wesn[YHI]);
	if (API->GMT->current.setting.run_mode == GMT_MODERN) {	/* Isolation mode is baked in */
		snprintf (srtmlist, PATH_MAX, "%s/=srtm%d.000000", API->GMT->parent->gwf_dir, res);
		file = srtmlist;
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to create job file %s\n", file);
			return NULL;
		}
	}
	else {	/* Must select a unique filename for the list */
		char name[GMT_LEN16] = {""};
#ifndef _WIN32
		int fd = 0;
#endif
		if (API->tmp_dir)			/* Have a recognized temp directory */
			snprintf (srtmlist, PATH_MAX, "%s/", API->tmp_dir);
		snprintf (name, GMT_LEN16, "=srtm%d.XXXXXX", res);
		strcat (srtmlist, name);
#ifdef _WIN32
		if ((file = mktemp (srtmlist)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Could not create temporary file name %s.\n", srtmlist);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "ERROR - Unable to create job file %s\n", file);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
#else
		if ((fd = mkstemp (srtmlist)) == -1) {
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Could not create temporary file name %s.\n", srtmlist);
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
		file = srtmlist;
		if ((fp = fdopen (fd, "w")) == NULL) {
			API->error = GMT_RUNTIME_ERROR;
			GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Could not fdopen temporary file %s.\n", file);
			return NULL;
		}
#endif
	}
	if ((SRTM = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, "@srtm_tiles.nc", NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to obtain list of available SRTM tiles.\n");
		API->error = GMT_RUNTIME_ERROR;
		fclose (fp);
		return NULL;
	}
	for (lat = is; lat < in; lat++) {	/* Rows of tiles */
		if (lat < -60 || lat > 59) continue;	/* Outside SRTM band */
		YS = (lat < 0) ? 'S' : 'N';
		for (x = iw; x < ie; x++) {	/* Columns of tiles */
			lon = (x < 0) ? x + 360 : x;	/* Get lon in 0-360 range */
			node = gmt_M_ijp (SRTM->header, 60-lat, lon);
			if (SRTM->data[node] == 0) continue;	/* Missing SRTM tile */
			lon = (x >= 180) ? x - 360 : x;	/* Need lons 0-179 for E and 1-180 for W */
			XS = (lon < 0) ? 'W' : 'E';
			fprintf (fp, "@%c%2.2d%c%3.3d.SRTMGL%d.%s\n", YS, abs(lat), XS, abs(lon), res, GMT_SRTM_EXTENSION_LOCAL);
			n_tiles++;
		}
	}
	if (ocean) {	/* OK, so we requested to fill in with sampled 15s grid.  Do we have the file already */
		if (gmt_access (API->GMT, "earth_relief_15s.grd", F_OK)) {	/* Not downloaded yet */
			/* We will loop acround the perimeter of the requested grid and generate lon, lat points
			 * that we will pass to gmtselect.  We determine if there are points that fall in the ocean
			 * which means we must get the 15s grid for sure. */
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, 2, 2};	/* Just a single data table with one segment with two 2-column records */
			uint64_t row, col, n = 0;
			double inc[2], lon, lat;
			char output[GMT_VF_LEN], input[GMT_VF_LEN], cmd[GMT_LEN128] = {""};
			struct GMT_GRID *G = NULL;
			struct GMT_DATASET *Din = NULL, *Dout = NULL;
			struct GMT_DATASEGMENT *S = NULL;

			/* Create a grid header that we can use to create the perimeter coordinates */
			if (res == 1) inc[GMT_X] = inc[GMT_Y] = GMT_SEC2DEG; else inc[GMT_X] = inc[GMT_Y] = 3.0 * GMT_SEC2DEG;
			if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, wesn, inc, \
				GMT_GRID_NODE_REG, GMT_NOTSET, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to create grid header used for building perimeter input.\n");
				return NULL;
			}
			/* Create dataset to hold the perimeter coordinates. Compute how many there are */
			dim[GMT_ROW] = 2 * ((uint64_t)G->header->n_rows + (G->header->n_columns - 2));
			if ((Din = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to create input dataset used for holding perimeter input.\n");
				return NULL;
			}
			S = Din->table[0]->segment[0];	/* Shorthand to only segment */
			gmt_M_row_loop (API->GMT, G, row) {	/* For entire west and east columns */
				lat = gmt_M_grd_row_to_y (API->GMT, row, G->header);
				S->data[GMT_X][n] = wesn[XLO];
				S->data[GMT_Y][n++] = lat;
				S->data[GMT_X][n] = wesn[XHI];
				S->data[GMT_Y][n++] = lat;
			}
			for (col = 1; col < (G->header->n_columns-1); col++) {	/* For shortened north and south rows */
				lon = gmt_M_grd_col_to_x (API->GMT, col, G->header);
				S->data[GMT_X][n] = lon;
				S->data[GMT_Y][n++] = wesn[YLO];
				S->data[GMT_X][n] = lon;
				S->data[GMT_Y][n++] = wesn[YHI];
			}
			/* Done with the grid for now */
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to destroy grid used for perimeter.\n");
				return NULL;
			}
			/* Set up virtual files for both input and output perimeter points */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, Din, input) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable opening virtual file for reading.\n");
				return NULL;
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, output) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable opening virtual file for writing.\n");
				return NULL;
			}
			/* Call gmt select and return coordinates on land */
			snprintf (cmd, GMT_LEN128, "-Ns/k -Df %s ->%s", input, output);
			if (GMT_Call_Module (API, "select", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Failure */
				GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: gmtselect command for perimeter failed.\n");
				return NULL;
			}
			if ((Dout = GMT_Read_VirtualFile (API, output)) != NULL) {	/* Access the output data table */
				if (Din->n_records == Dout->n_records) {	/* All perimeter nodes on land; no need to read the ocean layer */
					ocean = false;
					GMT_Report (API, GMT_MSG_INFORMATION, "gmtlib_get_srtmlist: Perimeter on land - no need to get @earth_relief_15s.\n");
				}
				else
					GMT_Report (API, GMT_MSG_INFORMATION, "gmtlib_get_srtmlist: Perimeter partly in ocean - must get @earth_relief_15s.\n");
				/* Free structures used to hold the perimeter data tables */
				if (GMT_Destroy_Data (API, &Din) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to destroy data table used for perimeter input.\n");
					return NULL;
				}
				if (GMT_Destroy_Data (API, &Dout) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "gmtlib_get_srtmlist: Unable to destroy data table used for perimeter output.\n");
					return NULL;
				}
			}
			/* So here, ocean may have changed from true to false if we found no ocean... */
		}
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "gmtlib_get_srtmlist: Already have @earth_relief_15s.\n");
		if (ocean)	/* Either some perimiter nodes were in the ocean, so get the file, or we already have the file locally */
			fprintf (fp, "@earth_relief_15s\n");	/* End with a resampled 15s grid to get bathymetry [-Co- clobber mode] */
	}
	fclose (fp);
	if (GMT_Destroy_Data (API, &SRTM) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_WARNING, "gmtlib_get_srtmlist: Unable to destroy list of available SRTM tiles.\n");
	}
	if (n_tiles == 0)	/* No tiles inside region */
		GMT_Report (API, GMT_MSG_WARNING, "gmtlib_get_srtmlist: No SRTM tiles available for your region.\n");
	return (strdup (file));
}

struct GMT_GRID * gmtlib_assemble_srtm (struct GMTAPI_CTRL *API, double *region, char *file) {
	/* Get here if file is a =srtm?.xxxxxx file.  Need to do:
	 * Set up a grdblend command and return the assembled grid
	 */
	char res = file[strlen(file)-8];
	struct GMT_GRID *G = NULL;
	double *wesn = (region) ? region : API->GMT->common.R.wesn;	/* Default to -R */
	char grid[GMT_VF_LEN] = {""}, cmd[GMT_LEN256] = {""}, tag[4] = {"01s"};
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;

	tag[1] = res;
	give_data_attribution (API->GMT, tag);
	GMT_Report (API, GMT_MSG_INFORMATION, "Assembling SRTM grid from 1x1 degree tiles given by listfile %s\n", file);
	GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, grid);
	/* Pass -N0 so that missing tiles (oceans) yield z = 0 and not NaN, and -Co- to override using negative earth_relief_15s values */
	snprintf (cmd, GMT_LEN256, "%s -R%.16g/%.16g/%.16g/%.16g -I%cs -G%s -N0 -Co-", file, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], res, grid);
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
