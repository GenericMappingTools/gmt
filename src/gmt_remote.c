/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
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

/* Copy a file from the GMT auto-download directory or the Internet.  We recognize
 * different types and names of files.
 * 1. There are data sets of use to all GMT users, such as global relief:
 * 	   @earth_relief_<res>.grd	Various global relief grids
 * We may add more data later but this is our start.
 * 2. Data sets only used to run an example or a test script
 * and these are all called @*, i.e., a '@' is pre-pended to the name.
 * They live in a cache subdirectory under the GMT_DATA_URL
 * and will be placed in a cache directory in the users ~/.gmt directory.
 * 3. Generic URLs starting with http:, https:, or ftp:  These will be
 * downloaded to the cache directory.
 * If auto-download is enabled and a requested input file matches these
 * names and not found by normal means then we download the file to the
 * user-directory.  All places that open files (GMT_Read_Data) will do
 * this check by first calling gmt_download_file_if_not_found.
 */

struct FtpFile {
	const char *filename;
	FILE *fp;
};

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

GMT_LOCAL int give_data_attribution (struct GMT_CTRL *GMT, const char *file) {
	/* Print attribution when the @earth_relief_xxx.grd file is downloaded for the first time */
	char tag[4] = {""};
	int k, match = -1;
	strncpy (tag, &file[strlen(file)-7], 3U);
	for (k = 0; k < GMT_N_DATA_INFO_ITEMS; k++) {
		if (!strncmp (tag, gmt_data_info[k].tag, 3U)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s: Download file from the GMT ftp data server [size is %s].\n", &file[1], gmt_data_info[k].size);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s: %s.\n\n", &file[1], gmt_data_info[k].remark);
			match = k;
		}
	}
	return (match == -1);	/* Not found */
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
	CURL *Curl = NULL;
	static char *ftp_dir[4] = {"/cache", "", "/srtm1", "/srtm3"}, *name[3] = {"CACHE", "USER", "LOCAL"};
	char *user_dir[3] = {GMT->session.CACHEDIR, GMT->session.USERDIR, NULL};
	char url[PATH_MAX] = {""}, local_path[PATH_MAX] = {""}, *c = NULL, *file = NULL;
	char srtmdir[PATH_MAX] = {""};
	struct FtpFile ftpfile = {NULL, NULL};

	if (!file_name) return 0;   /* Got nutin' */

	be_fussy = ((mode & 4) == 0);	if (be_fussy == 0) mode -= 4;	/* Handle the optional 4 value */
	
	file = strdup (file_name);
	/* Because file_name may be <file>, @<file>, or URL/<file> we must find start of <file> */
	if (gmt_M_file_is_cache (file)) {	/* A leading '@' was found */
		pos = 1;
		if ((c = strchr (file, '?')))	/* Netcdf directive since URL was handled above */
			c[0] = '\0';
		else if ((c = strchr (file, '=')))	/* Grid attributes */
			c[0] = '\0';
	}
	else if (gmt_M_file_is_url (file)) {	/* A remote file given via an URL */
		pos = gmtlib_get_pos_of_filename (file);	/* Start of file in URL (> 0) */
		if ((c = strchr (file, '?')) && !strchr (file, '='))	/* Must be a netCDF sliced URL file so chop off the layer/variable specifications */
			c[0] = '\0';
		else if ((c = strchr (file, '=')))	/* Grid attributes */
			c[0] = '\0';
	}
	else if ((c = strchr (file, '?')))	/* Netcdf directive since URLs and caches were handled above */
		c[0] = '\0';	/* and pos = 0 */

	/* Return immediately if cannot be downloaded (for various reasons) */
	if (!gmtlib_file_is_downloadable (GMT, file, &kind)) {
		gmt_M_str_free (file);
		return (pos);
	}
	from = (kind == GMT_DATA_FILE) ? GMT_DATA_DIR : GMT_CACHE_DIR;	/* Determine source directory on cache server */
	to = (mode == GMT_LOCAL_DIR) ? GMT_LOCAL_DIR : from;
	if (gmtlib_file_is_srtmtile (GMT->parent, file, &res)) {	/* Select the right sub-dir on the server and cache locally */
		from = (res == 1) ? 2 : 3;
		to = GMT_CACHE_DIR;
		sprintf (srtmdir, "%s/srtm%d", user_dir[GMT_CACHE_DIR], res);
		/* Check if srtm1|3 subdir exist - if not create it */
		if (access (srtmdir, R_OK)) {
#ifndef _WIN32
			if (mkdir (srtmdir, (mode_t)0777))
#else
			if (mkdir (srtmdir))
#endif
	            		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create GMT Cache directory : %s\n", srtmdir);
		}
	}
	if (mode == GMT_LOCAL_DIR || user_dir[to] == NULL) {
		if (mode != GMT_LOCAL_DIR) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "The GMT_%s directory is not defined - download file to current directory\n", name[to]);
		sprintf (local_path, "%s", &file[pos]);
	}
	/* Here we will try to download a file */

  	if ((Curl = curl_easy_init ()) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to initiate curl - cannot obtain %s\n", &file[pos]);
		gmt_M_str_free (file);
		return 0;
	}
	if (curl_easy_setopt (Curl, CURLOPT_SSL_VERIFYPEER, 0L)) {		/* Tell libcurl to not verify the peer */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to set curl option to not verify the peer\n");
		gmt_M_str_free (file);
		return 0;
	}
	if (curl_easy_setopt (Curl, CURLOPT_FOLLOWLOCATION, 1L)) {		/* Tell libcurl to follow 30x redirects */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to set curl option to follow redirects\n");
		gmt_M_str_free (file);
		return 0;
	}
	if (mode != GMT_LOCAL_DIR && user_dir[to]) {
		if (from >= 2)
			sprintf (local_path, "%s/%s", srtmdir, &file[pos]);
		else
			sprintf (local_path, "%s/%s", user_dir[to], &file[pos]);
	}
	if (kind == GMT_URL_QUERY) {	/* Cannot have ?para=value etc in filename */
		c = strchr (local_path, '?');
		if (c) c[0] = '\0';	/* Chop off ?CGI parameters from local_path */
	}
	if (kind == GMT_URL_FILE || kind == GMT_URL_QUERY)	/* General URL given */
		sprintf (url, "%s", file);
	else			/* Use GMT ftp dir, possible from subfolder cache */
		sprintf (url, "%s%s/%s", GMT_DATA_URL, ftp_dir[from], &file[pos]);

 	if (curl_easy_setopt (Curl, CURLOPT_URL, url)) {	/* Set the URL to copy */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to set curl option to read from %s\n", url);
		gmt_M_str_free (file);
		return 0;
	}
	ftpfile.filename = local_path;	/* Set pointer to local filename */
	/* Define our callback to get called when there's data to be written */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEFUNCTION, fwrite_callback)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to set curl output callback function\n");
		gmt_M_str_free (file);
		return 0;
	}
	/* Set a pointer to our struct to pass to the callback */
	if (curl_easy_setopt (Curl, CURLOPT_WRITEDATA, &ftpfile)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to set curl option to write to %s\n", local_path);
		gmt_M_str_free (file);
		return 0;
	}
	if (kind == GMT_DATA_FILE) give_data_attribution (GMT, file);
	
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Downloading file %s ...\n", url);
	if ((curl_err = curl_easy_perform (Curl))) {	/* Failed, give error message */
		if (be_fussy || curl_err != CURLE_REMOTE_FILE_NOT_FOUND) {	/* Unexpected failure - want to bitch about it */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Libcurl Error: %s\n", curl_easy_strerror (curl_err));
			if (ftpfile.fp != NULL) {
				fclose (ftpfile.fp);
				ftpfile.fp = NULL;
			}
			if (gmt_remove_file (GMT, local_path))
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not even remove file %s\n", local_path);
		}
	}
	curl_easy_cleanup (Curl);
	if (ftpfile.fp) /* close the local file */
		fclose (ftpfile.fp);
	if (kind == GMT_URL_QUERY) {	/* Cannot have ?para=value etc in local filename */
		c = strchr (file_name, '?');
		if (c) c[0] = '\0';	/* Chop off ?CGI parameters from local_path */
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {	/* Say a few things about the file we got */
		struct stat buf;
		if (stat (local_path, &buf))
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not determine size of downloaded file %s\n", &file_name[pos]);
		else
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Download complete [Got %s].\n", gmt_memory_use (buf.st_size, 3));
	}
	gmt_M_str_free (file);

	return (pos);
}

/* Support functions for SRTM tile grids
 * gmtlib_file_is_srtm:	Determine if a SRTM 1 or 3 arc second request was given
 * gmtlib_file_is_srtmlist: Determine if file is a listfile with SRTM tile info
 * gmtlib_get_srtmlist:		Convert -Rw/e/s/n into a file with a list of the tiles needed
 */

bool gmtlib_file_is_srtmrequest (struct GMTAPI_CTRL *API, const char *file, unsigned int *res) {
	size_t len = strlen(GMT_DATA_PREFIX);
	gmt_M_unused (API);
	if (file[0] != '@') return false;	/* Not a remote file */
	if (strncmp (&file[1], GMT_DATA_PREFIX, len)) return false;	/* Not a remote earth_relief grid */
	if (file[len+3] != 's') return false;	/* Not an arcsec grid */
	if (!(file[len+1] == '0' && (file[len+2] == '1' || file[len+2] == '3'))) return false;
	/* Yes: a 1 or 3 arc sec SRTM request */
	*res = (unsigned int)(file[len+2] - '0');
	return true;
}

bool gmtlib_file_is_srtmlist (struct GMTAPI_CTRL *API, const char *file) {
	size_t len = strlen(file);
	gmt_M_unused (API);
	if (len < 13) return false;	/* Too short a filename */
	if (strncmp (&file[len-13], "=srtm", 5U)) return false;	/* Not that kind of file */
	return true;	/* We got one */
}

bool gmtlib_file_is_srtmtile (struct GMTAPI_CTRL *API, const char *file, unsigned int *res) {
	/* Recognizes remote files like N42E126.SRTMGL1.nc */
	char *p = NULL;
	gmt_M_unused (API);
	if ((p = strstr (file, ".SRTMGL")) == NULL) return false;	/* Nope */
	*res = (unsigned int)(p[7] - '0');
	return true;	/* Found it */
}

char *gmtlib_get_srtmlist (struct GMTAPI_CTRL *API, double wesn[], unsigned int res) {
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
		sprintf (srtmlist, "%s/=srtm%d.000000", API->GMT->parent->gwf_dir, res);
		file = srtmlist;
	}
	else {	/* Must select a unique filename for the list */
		char name[GMT_LEN16] = {""};
		if (API->tmp_dir)			/* Have a recognized temp directory */
			sprintf (srtmlist, "%s/", API->tmp_dir);
		sprintf (name, "=srtm%d.XXXXXX", res);
		strcat (srtmlist, name);
		if ((file = mktemp (srtmlist)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_srtmlist: Could not create temporary file name.\n");
			API->error = GMT_RUNTIME_ERROR;
			return NULL;
		}
	}
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR - Unable to create job file %s\n", file);
		return NULL;
	}
	if ((SRTM = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, "@srtm_tiles.nc", NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_srtmlist: Unable to obtain list of available SRTM tiles.\n");
		API->error = GMT_RUNTIME_ERROR;
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
			fprintf (fp, "@%c%2.2d%c%3.3d.SRTMGL%d.%s\n", YS, abs(lat), XS, abs(lon), res, GMT_SRTM_EXTENSION);
			n_tiles++;
		}
	}
	fclose (fp);
	if (GMT_Destroy_Data (API, &SRTM) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "gmtlib_get_srtmlist: Unable to destroy list of available SRTM tiles.\n");
	}
	if (n_tiles == 0)	/* No tiles inside region */
		GMT_Report (API, GMT_MSG_VERBOSE, "gmtlib_get_srtmlist: Warning: No SRTM tiles available for your region.\n");
	return (strdup (file));
}

struct GMT_GRID * gmtlib_assemble_srtm (struct GMTAPI_CTRL *API, double *region, char *file) {
	/* Get here if file is a =srtm?.xxxxxx file.  Need to do:
	 * Set up a grdblend command and return the assembled grid
	 */
	unsigned int res = (file[strlen(file)-8] - '0');
	struct GMT_GRID *G = NULL;
	double *wesn = (region) ? region : API->GMT->common.R.wesn;	/* Default to -R */
	char grid[GMT_STR16] = {""}, cmd[GMT_LEN128] = {""};
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Assembling SRTM grid from listfile %s\n", file);
	GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_OUT, NULL, grid);
	
	sprintf (cmd, "%s -R%g/%g/%g/%g -I%ds -G%s", file, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], res, grid);
	if (GMT_Call_Module (API, "grdblend", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR - Unable to produce blended grid from %s\n", file);
		return NULL;
	}
	if ((G = GMT_Read_VirtualFile (API, grid)) == NULL) {	/* Load in the resampled grid */
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR - Unable to receive blended grid from grdblend\n");
		return NULL;
	}
	G->header->orig_datatype = GMT_SHORT;	/* Since we know this */
	return (G);
}

#if 0
/* Code to apply and restore d2dxdy on integer grids (SRTM) */
void gmtlib_srtm_d2dxdy (struct GMTAPI_CTRL *API, struct GMT_GRID *G, int way) {
	/* way = +1 apply d2dxdy to grid, -1 undo this effect */
	int row, col;
	uint64_t ij;
	
	if (way == +1) {	/* Apply d2dxdy to our grid */
		int last_row = (int)(G->header->n_rows - 1), last_col = (int)(G->header->n_columns - 1);
		for (row = last_row; row >= 0; row--) {	/* Start on last row and move to top */
			ij = gmt_M_ijp (G->header, row, last_col);	/* Last node on this row */
			for (col = last_col; col > 0; col--, ij--)
				G->data[ij] -= G->data[ij-1];	/* Do d/dx from end to beginning of row */
			if (row < last_row) {	/* Once past last row we can do d/dy */
				ij = gmt_M_ijp (G->header, row, 0);	/* Start of current row */
				for (col = 0; col < (int)G->header->n_columns; col++, ij++)
					G->data[ij+G->header->mx] -= G->data[ij];
			}
		}
	}
	else if (way == -1) {	/* Undo d2dxdy for our grid */
		for (row = 0; row < (int)G->header->n_rows; row++) {	/* Undo d/dx */
			ij = gmt_M_ijp (G->header, row, 1);	/* 2nd node on this row */
			for (col = 1; col < (int)G->header->n_columns; col++, ij++)
				G->data[ij] += G->data[ij-1];
			if (row) {	/* Undo d/dy once we have one row above us */
				ij = gmt_M_ijp (G->header, row, 0);	/* Start of current row */
				for (col = 0; col < (int)G->header->n_columns; col++, ij++)
					G->data[ij] += G->data[ij-G->header->mx];
			}
		}
	}
	else
		GMT_Report (API, GMT_MSG_NORMAL, "Must select way = -1 | +1!\n");
}
#endif
