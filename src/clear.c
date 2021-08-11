/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	23-Jun-2017
 * Version:	6 API
 *
 * Brief synopsis: gmt clear cleans up by removing files or dirs.
 *	gmt clear [all | cache | data[=<planet>] | geography[=<name>] | sessions | settings ]
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

#define THIS_MODULE_CLASSIC_NAME	"clear"
#define THIS_MODULE_MODERN_NAME	"clear"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delete current default settings, or the cache, data, geography or sessions directories"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s all|cache|data[=<planet>]|geography[=<name>]|sessions|settings [%s]\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\nDeletes the specified item.  Choose one of these targets:");
	GMT_Usage (API, 2, "cache");
	GMT_Usage (API, -4, "Deletes the user\'s cache directory:");
	GMT_Usage (API, -4, "[%s]", API->GMT->session.CACHEDIR);
	GMT_Usage (API, 2, "data");
	GMT_Usage (API, -4, "Deletes the user\'s data download directory:");
	GMT_Usage (API, -4, "[%s/server]", API->GMT->session.USERDIR);
	GMT_Usage (API, -4, "Append =<planet> to limit removal to such data for a specific <planet> [all].");
	GMT_Usage (API, 2, "geography");
	GMT_Usage (API, -4, "Deletes the user\'s geography directory (gshhg, dcw):");
	GMT_Usage (API, -4, "[%s/geography]", API->GMT->session.USERDIR);
	GMT_Usage (API, -4, "Append =<name> to limit removal to such data for gshhg or dcw only [all].");
	GMT_Usage (API, 2, "sessions");
	GMT_Usage (API, -4, "Deletes the user\'s sessions directory:");
	GMT_Usage (API, -4, "[%s]", API->session_dir);
	GMT_Usage (API, 2, "settings");
	GMT_Usage (API, -4, "Deletes a modern mode session\'s %s file.", GMT_SETTINGS_FILE);
	GMT_Usage (API, 2, "all");
	GMT_Usage (API, -4, "All of the above.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "V,;");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to clear.
	 */

	unsigned int n_errors = 0;

	if (options == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "No target specified\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int clear_cache (struct GMTAPI_CTRL *API) {
	if (gmt_remove_dir (API, API->GMT->session.CACHEDIR, false))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

static int clear_defaults (struct GMTAPI_CTRL *API) {
	char file[PATH_MAX] = {""};
	sprintf (file, "%s/%s", API->gwf_dir, GMT_SETTINGS_FILE);
	if (gmt_remove_file (API->GMT, file))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

static int clear_data (struct GMTAPI_CTRL *API, char *planet) {
	int d1, d2, d3;
	char server_dir[PATH_MAX] = {""}, current_d1[PATH_MAX] = {""}, current_d2[PATH_MAX] = {""}, current_d3[PATH_MAX] = {""};
	char **dir1, **dir2, **dir3;
	struct GMT_CTRL *GMT = API->GMT;

	sprintf (server_dir, "%s/server", API->GMT->session.USERDIR);
	if ((dir1 = gmtlib_get_dirs (GMT, server_dir)) == NULL) {
		GMT_Report (API, GMT_MSG_NOTICE, "No directories found under %s\n", server_dir);
		return GMT_NOERROR;
	}

	d1 = 0;
	while (dir1[d1]) {	/* Look through planetary subdirectories under /server (e.g. earth) */
		if (planet && strcmp (dir1[d1], planet)) {	/* Not a planet that we want to delete */
			d1++;
			continue;
		}
		sprintf (current_d1, "%s/%s", server_dir, dir1[d1]);	/* E.g., ~/.gmt/server/earth */
		if ((dir2 = gmtlib_get_dirs (GMT, current_d1))) {	/* Find all subdirs under this planet (e.g. earth_relief), if any */
			d2 = 0;
			while (dir2[d2]) {	/* Look through an subdirectories under /server/planet.data (e.g. /server/planet/dataset), if any */
				sprintf (current_d2, "%s/%s/%s", server_dir, dir1[d1], dir2[d2]);	/* E.g., ~/.gmt/server/earth/earth_relief */
				if ((dir3 = gmtlib_get_dirs (GMT, current_d2))) {
					d3 = 0;
					while (dir3[d3]) {	/* Look through an subdirectories under /server/planet.data (e.g. /server/planet/dataset/tileddata), if any */
						sprintf (current_d3, "%s/%s/%s/%s", server_dir, dir1[d1], dir2[d2], dir3[d3]);	/* E.g., ~/.gmt/server/earth/earth_relief/earth_relief_15s_p */
						if (gmt_remove_dir (API, current_d3, false)) {
							GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", current_d3);
							gmtlib_free_dir_list (GMT, &dir1);
							gmtlib_free_dir_list (GMT, &dir2);
							gmtlib_free_dir_list (GMT, &dir3);
							return GMT_NOERROR;
						}
						d3++;
					}
					gmtlib_free_dir_list (GMT, &dir3);
				}
				if (gmt_remove_dir (API, current_d2, false)) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", current_d2);
					gmtlib_free_dir_list (GMT, &dir1);
					gmtlib_free_dir_list (GMT, &dir2);
					return GMT_NOERROR;
				}
				d2++;
			}
			gmtlib_free_dir_list (GMT, &dir2);
		}
		if (gmt_remove_dir (API, current_d1, false)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", current_d1);
			gmtlib_free_dir_list (GMT, &dir1);
			return GMT_NOERROR;
		}
		d1++;
	}
	gmtlib_free_dir_list (GMT, &dir1);

	if (planet == NULL) {
		if (gmt_remove_dir (API, server_dir, false)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", server_dir);
			return GMT_NOERROR;
		}
		/* Also remove old legacy SRTM dirs, if found */
		sprintf (current_d1, "%s/srtm1", server_dir);
		if (access (current_d1, F_OK) == 0 && gmt_remove_dir (API, current_d1, false))
			return GMT_RUNTIME_ERROR;
		sprintf (current_d1, "%s/srtm3", server_dir);
		if (access (current_d1, F_OK) == 0 && gmt_remove_dir (API, current_d1, false))
			return GMT_RUNTIME_ERROR;
	}
	return GMT_NOERROR;
}

static int clear_sessions (struct GMTAPI_CTRL *API) {
	int error;
	char del_cmd[PATH_MAX] = {""};
	if (access (API->session_dir, F_OK)) {
		GMT_Report (API, GMT_MSG_INFORMATION, "No directory named %s\n", API->session_dir);
		return GMT_FILE_NOT_FOUND;
	}
#ifdef _WIN32
	char* t = gmt_strrep(API->session_dir, "/", "\\");		/* rmdir needs paths with back-slashes */
	strcpy(del_cmd, "rmdir /s /q ");
	strncat(del_cmd, t, PATH_MAX-1);
	gmt_M_str_free(t);
#else
	sprintf (del_cmd, "rm -rf %s", API->session_dir);
#endif

	if ((error = system (del_cmd))) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to remove session directory %s [error = %d]\n", API->session_dir, error);
		return GMT_RUNTIME_ERROR;
	}

	return GMT_NOERROR;
}

static int clear_geography (struct GMTAPI_CTRL *API, char *data) {
	int d;
	char geography_dir[PATH_MAX] = {""}, current_dir[PATH_MAX] = {""};
	char **dir;
	struct GMT_CTRL *GMT = API->GMT;

	sprintf (geography_dir, "%s/geography", API->GMT->session.USERDIR);
	if (access (geography_dir, F_OK)) {
		GMT_Report (API, GMT_MSG_INFORMATION, "No directory named %s\n", geography_dir);
		return GMT_FILE_NOT_FOUND;
	}
	if ((dir = gmtlib_get_dirs (GMT, geography_dir)) == NULL) {
		GMT_Report (API, GMT_MSG_NOTICE, "No directories found under %s\n", geography_dir);
		return GMT_NOERROR;
	}

	d = 0;
	while (dir[d]) {	/* Look through geography subdirectories */
		if (data && strcmp (dir[d], data)) {	/* Not a dataset that we want to delete */
			d++;
			continue;
		}
		sprintf (current_dir, "%s/%s", geography_dir, dir[d]);	/* E.g., ~/.gmt/geography/gshhg */
		if (gmt_remove_dir (API, current_dir, false)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", current_dir);
			gmtlib_free_dir_list (GMT, &dir);
			return GMT_NOERROR;
		}
		d++;
	}
	gmtlib_free_dir_list (GMT, &dir);

	if (data == NULL) {
		if (gmt_remove_dir (API, geography_dir, false)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove directory %s [permissions?]\n", geography_dir);
			return GMT_NOERROR;
		}
	}
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_clear (void *V_API, int mode, void *args) {
	int error = 0, n_given = 0;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the clear main code ----------------------------*/

	for (opt = options; opt; opt = opt->next) {
		if (opt->option != GMT_OPT_INFILE) continue;	/* action target will appear as file name */

		n_given++;
		if (!strcmp (opt->arg, "all")) {	/* Clear all */
			if (clear_cache (API))
				error = GMT_RUNTIME_ERROR;
			if (clear_data (API, NULL))
				error = GMT_RUNTIME_ERROR;
			if (clear_geography (API, NULL))
				error = GMT_RUNTIME_ERROR;
			if (clear_sessions (API))
				error = GMT_RUNTIME_ERROR;
			if (API->GMT->current.setting.run_mode == GMT_MODERN && clear_defaults (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "cache")) {	/* Clear the cache */
			if (clear_cache (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strncmp (opt->arg, "data", 4U)) {	/* Clear the data */
			char *planet = strchr (opt->arg, '=');
			if (planet) planet++;	/* Skip past the = sign */
			if (clear_data (API, planet))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "sessions")) {	/* Clear the sessions dir */
			if (clear_sessions (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strncmp (opt->arg, "geography", 9U)) {	/* Clear the geography dir */
			char *set = strchr (opt->arg, '=');
			if (set) set++;	/* Skip past the = sign */
			if (clear_geography (API, set))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "settings") || !strcmp (opt->arg, "defaults") || !strcmp (opt->arg, "conf")) {	/* Clear the default settings in modern mode */
			if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
				GMT_Report (API, GMT_MSG_ERROR, "Target \"%s\" is only valid in a modern mode session\n", opt->arg);
			}
			else if (clear_defaults (API))
				error = GMT_RUNTIME_ERROR;
		}
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Unrecognized target %s - skipped\n", opt->arg);
			n_given--;	/* Undo the premature increment */
		}
	}
	if (n_given == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "No clear target given\n");
		Return (GMT_RUNTIME_ERROR);
	}

	Return (error);
}
