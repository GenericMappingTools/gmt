/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	gmt clear [all | cache | conf | cpt | history ]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"clear"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delete history, conf, cpt, the cache dir, or all of them"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: clear all|cache|history|conf [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tDeletes the specified item.  Choose on of these targets:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   cache    Deletes the user\'s cache directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   conf     Deletes the user\'s gmt.conf file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   cpt      Deletes the current CPT file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   history  Deletes the user\'s gmt.history file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   all      All of the above.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	if ((opt = options) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Required target not specified\n");
		return GMT_PARSE_ERROR;
	}
	if (strcmp (opt->arg, "all") && strcmp (opt->arg, "cache") && strcmp (opt->arg, "conf") && strcmp (opt->arg, "cpt") && strcmp (opt->arg, "history")) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized target %s\n", opt->arg);
		n_errors++;
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int clear_cache (struct GMTAPI_CTRL *API) {
	char dir[PATH_MAX] = {""};
	sprintf (dir, "%s/srtm1", API->GMT->session.CACHEDIR);
	if (access (dir, F_OK) == 0 && gmt_remove_dir (API, dir, false))
		return GMT_RUNTIME_ERROR;
	sprintf (dir, "%s/srtm3", API->GMT->session.CACHEDIR);
	if (access (dir, F_OK) == 0 && gmt_remove_dir (API, dir, false))
		return GMT_RUNTIME_ERROR;
	if (gmt_remove_dir (API, API->GMT->session.CACHEDIR, true))
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_clear (void *V_API, int mode, void *args) {
	int error = 0;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the clear main code ----------------------------*/

	opt = GMT_Find_Option (API, GMT_OPT_INFILE, options);	/* action target will appear as file name */
	if (!strcmp (opt->arg, "all")) {	/* Clear all */
		if (clear_cache (API))
			error = GMT_RUNTIME_ERROR;
		if (gmt_remove_file (API->GMT, "gmt.history"))
			error = GMT_RUNTIME_ERROR;
		if (gmt_remove_file (API->GMT, "gmt.conf"))
			error = GMT_RUNTIME_ERROR;
	}
	else if (!strcmp (opt->arg, "cache")) {	/* Clear the cache */
		if (clear_cache (API))
			error = GMT_RUNTIME_ERROR;
	}
	else if (!strcmp (opt->arg, "cpt")) {	/* Clear the current CPT */
		char *cpt = gmt_get_current_cpt (API->GMT);
		if (cpt && gmt_remove_file (API->GMT, cpt))
			error = GMT_RUNTIME_ERROR;
		if (cpt) gmt_M_str_free (cpt);
	}
	else if (!strcmp (opt->arg, "history")) {	/* Clear the history */
		if (gmt_remove_file (API->GMT, "gmt.history"))
			error = GMT_RUNTIME_ERROR;
	}
	else if (!strcmp (opt->arg, "conf")) {	/* Clear the configuration */
		if (gmt_remove_file (API->GMT, "gmt.conf"))
			error = GMT_RUNTIME_ERROR;
	}
		
	Return (error);
}
