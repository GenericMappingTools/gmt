/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	gmt clear [all | cache | data | sessions | settings ]
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"clear"
#define THIS_MODULE_MODERN_NAME	"clear"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delete current default settings, or the cache, data or sessions directories"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

EXTERN_MSC uint64_t gmtlib_glob_list (struct GMT_CTRL *GMT, const char *pattern, char ***list);
EXTERN_MSC void gmtlib_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s all|cache|data|sessions|settings [%s]\n\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tDeletes the specified item.  Choose one of these targets:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   cache     Deletes the user\'s cache directory [%s].\n", API->GMT->session.CACHEDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   data      Deletes the user\'s data download directory [%s/server].\n", API->GMT->session.USERDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   sessions  Deletes the user\'s sessions directory [%s].\n", API->session_dir);
	GMT_Message (API, GMT_TIME_NONE, "\t   settings  Deletes a modern mode session\'s gmt.conf file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   all       All of the above.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,;");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to clear.
	 */

	unsigned int n_errors = 0;

	if (options == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No target specified\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int clear_cache (struct GMTAPI_CTRL *API) {
	if (gmt_remove_dir (API, API->GMT->session.CACHEDIR, false))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

GMT_LOCAL int clear_defaults (struct GMTAPI_CTRL *API) {
	char file[PATH_MAX] = {""};
	sprintf (file, "%s/gmt.conf", API->gwf_dir);
	if (gmt_remove_file (API->GMT, file))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

GMT_LOCAL int clear_data (struct GMTAPI_CTRL *API) {
	char dir[PATH_MAX] = {""};
	sprintf (dir, "%s/server/srtm1", API->GMT->session.USERDIR);
	if (access (dir, F_OK) == 0 && gmt_remove_dir (API, dir, false))
		return GMT_RUNTIME_ERROR;
	sprintf (dir, "%s/server/srtm3", API->GMT->session.USERDIR);
	if (access (dir, F_OK) == 0 && gmt_remove_dir (API, dir, false))
		return GMT_RUNTIME_ERROR;
	sprintf (dir, "%s/server", API->GMT->session.USERDIR);
	if (access (dir, F_OK) == 0 && gmt_remove_dir (API, dir, false))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

GMT_LOCAL int clear_sessions (struct GMTAPI_CTRL *API) {
	unsigned int n_dirs, k;
	char **dirlist = NULL, *here = NULL;
	if (access (API->session_dir, F_OK)) {
		GMT_Report (API, GMT_MSG_VERBOSE, "No directory named %s\n", API->session_dir);
		return GMT_FILE_NOT_FOUND;
	}
	if ((here = getcwd (NULL, 0)) == NULL) {	/* Get the current directory */
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot determine current directory!\n");
		return GMT_RUNTIME_ERROR;
	}
	if (chdir (API->session_dir)) {	/* Cd into sessions directory */
		perror (API->session_dir);
		return GMT_RUNTIME_ERROR;
	}
	if ((n_dirs = (unsigned int)gmtlib_glob_list (API->GMT, "gmt*", &dirlist))) {	/* Find the gmt.<session_name> directories */
		for (k = 0; k < n_dirs; k++) {
			if (gmt_remove_dir (API, dirlist[k], false))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove directory %s [permissions?]\n", dirlist[k]);
		}
		gmtlib_free_list (API->GMT, dirlist, n_dirs);	/* Free the dir list */
	}
	if (chdir (here)) {		/* Get back to where we were */
		perror (here);
		gmt_M_str_free (here);
		return GMT_RUNTIME_ERROR;
	}
	gmt_M_str_free (here);
	if (gmt_remove_dir (API, API->session_dir, false))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_clear (void *V_API, int mode, void *args) {
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the clear main code ----------------------------*/

	for (opt = options; opt; opt = opt->next) {
		if (opt->option != GMT_OPT_INFILE) continue;	/* action target will appear as file name */

		n_given++;
		if (!strcmp (opt->arg, "all")) {	/* Clear all */
			if (clear_cache (API))
				error = GMT_RUNTIME_ERROR;
			if (clear_data (API))
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
		else if (!strcmp (opt->arg, "data")) {	/* Clear the data */
			if (clear_data (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "sessions")) {	/* Clear the sessions dir */
			if (clear_sessions (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "settings") || !strcmp (opt->arg, "defaults") || !strcmp (opt->arg, "conf")) {	/* Clear the default settings in modern mode */
			if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
				GMT_Report (API, GMT_MSG_NORMAL, "Target \"%s\" is only valid in a modern mode session\n", opt->arg);
			}
			else if (clear_defaults (API))
				error = GMT_RUNTIME_ERROR;
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Unrecognized target %s - skipped\n", opt->arg);
			n_given--;	/* Undo the premature increment */
		}
	}
	if (n_given == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No clear target given\n");
		Return (GMT_RUNTIME_ERROR);
	}

	Return (error);
}
