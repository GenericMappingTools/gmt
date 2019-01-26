/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	23-Jun-2017
 * Version:	6 API
 *
 * Brief synopsis: gmt clear cleans up by removing files or dirs.
 *	gmt clear [all | cache | conf | cpt | data | history | sessions ]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"clear"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delete current history, conf, cpt, or the cache, data or sessions directories"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

EXTERN_MSC uint64_t gmtlib_glob_list (struct GMT_CTRL *GMT, const char *pattern, char ***list);
EXTERN_MSC void gmtlib_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s all|cache|cpt|conf|data|history|sessions [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tDeletes the specified item.  Choose on of these targets:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   cache     Deletes the user\'s cache directory [%s].\n", API->GMT->session.CACHEDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   conf      Deletes the user\'s gmt.conf file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   cpt       Deletes the current CPT file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   data      Deletes the user\'s data download directory [%s/server].\n", API->GMT->session.USERDIR);
	GMT_Message (API, GMT_TIME_NONE, "\t   history   Deletes the user\'s gmt.history file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   sessions  Deletes the user\'s sessions directory [%s].\n", API->session_dir);
	GMT_Message (API, GMT_TIME_NONE, "\t   all       All of the above.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;

	if (options == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No target specified\n");
		n_errors++;
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int clear_cache (struct GMTAPI_CTRL *API) {
	if (gmt_remove_dir (API, API->GMT->session.CACHEDIR, true))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

GMT_LOCAL int clear_cpt (struct GMTAPI_CTRL *API) {
	int error = GMT_NOERROR;
	char *cpt = gmt_get_current_cpt (API->GMT);
	if (cpt) {
		if (gmt_remove_file (API->GMT, cpt))
			error = GMT_RUNTIME_ERROR;
		gmt_M_str_free (cpt);
	}
	return error;
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
		GMT_Report (API, GMT_MSG_NORMAL, "No directory named %s\n", API->session_dir);
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the clear main code ----------------------------*/

	for (opt = options; opt; opt = opt->next) {
		if (opt->option != GMT_OPT_INFILE) continue;	/* action target will appear as file name */
		
		n_given++;
		if (!strcmp (opt->arg, "all")) {	/* Clear all */
			if (clear_cache (API))
				error = GMT_RUNTIME_ERROR;
			if (clear_cpt (API))
				error = GMT_RUNTIME_ERROR;
			if (clear_data (API))
				error = GMT_RUNTIME_ERROR;
			if (clear_sessions (API))
				error = GMT_RUNTIME_ERROR;
			if (gmt_remove_file (API->GMT, "gmt.history"))
				error = GMT_RUNTIME_ERROR;
			if (gmt_remove_file (API->GMT, "gmt.conf"))
				error = GMT_RUNTIME_ERROR;
			/* Must turn off history writing otherwise we write out history again... */
			API->clear = true;
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
		else if (!strcmp (opt->arg, "cpt")) {	/* Clear the current CPT */
			if (clear_cpt (API))
				error = GMT_RUNTIME_ERROR;
		}
		else if (!strcmp (opt->arg, "history")) {	/* Clear the history */
			if (gmt_remove_file (API->GMT, "gmt.history"))
				error = GMT_RUNTIME_ERROR;
			/* Must turn off history writing otherwise we write out history again... */
			API->clear = true;
		}
		else if (!strcmp (opt->arg, "conf")) {	/* Clear the configuration */
			if (gmt_remove_file (API->GMT, "gmt.conf"))
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
