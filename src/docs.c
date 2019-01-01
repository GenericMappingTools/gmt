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
 * Author:	Joaquim Luis
 * Date:	1-Mar-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt docs 
 *	gmt docs 
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"docs"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Show the HTML documentation of the specified module"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	""

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <module-name> [<-option>]\n\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t<module-name> is one of the core or supplemental modules\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<-option> is the one-letter option of the module in question (e.g, -R)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Display the documentation positioned at that specific option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	struct GMT_OPTION *opt = NULL;

	if ((opt = options) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Required module name not specified\n");
		return GMT_PARSE_ERROR;
	}
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC const char * api_get_module_group (void *V_API, char *module);

int GMT_docs (void *V_API, int mode, void *args) {
	bool other_file = false;
	int error = 0, k;
	char cmd[PATH_MAX] = {""}, URL[PATH_MAX] = {""}, module[GMT_LEN64] = {""}, name[PATH_MAX] = {""}, *t = NULL;
	const char *group = NULL, *docname = NULL;
	static const char *known_group[2] = {"core", "other"}, *known_doc[5] = {"cookbook", "api", "tutorial", "Gallery", "gmt.conf"};
	static const char *can_opener[3] = {"cmd /c start", "open", "xdg-open"};
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

	/*---------------------------- This is the docs main code ----------------------------*/

	opt = GMT_Find_Option (API, GMT_OPT_INFILE, options);	/* action target will appear as file name */
	if (!opt) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot use an option (-%c) without a module name\n", options->option);
		Return (GMT_RUNTIME_ERROR);
	}

	docname = gmt_current_name (opt->arg, name);
	
	if (strcmp (opt->arg, docname))
		GMT_Report (GMT->parent, GMT_MSG_NORMAL,
		            "For now, HTML documentation only uses GMT modern mode names, hence %s will display as %s\n",
		            opt->arg, docname);

	t = strdup (docname);	/* Make a copy because gmt_str_tolower changes the input that may be a const char */
	gmt_str_tolower (t);
	if (!strcmp (t, "cookbook")) {
		docname = known_doc[0];	group   = known_group[0];		/* Pretend it is */
	}
	else if (!strcmp (t, "api")) {
		docname = known_doc[1];		group   = known_group[0];		/* Pretend it is */
	}
	else if (!strcmp (t, "tutorial")) {
		docname = known_doc[2];	group   = known_group[0];		/* Pretend it is */
	}
	else if (!strcmp (t, "gallery")) {
		docname = known_doc[3];	group   = known_group[0];		/* Pretend it is */
	}
	else if (!strcmp (t, "gmt.conf")) {
		docname = known_doc[4];	group   = known_group[0];		/* Pretend it is */
	}
	else if (gmt_get_ext (docname)) {
		group = known_group[1];
		other_file = true;
	}
	else if ((group = api_get_module_group (API, name)) == NULL) {
		gmt_M_str_free (t);
		Return (GMT_RUNTIME_ERROR);
	}

	gmt_M_str_free (t);
	if (!strcmp (group, "core"))	/* Core module */
		sprintf (module, "%s.html", docname);
	else if (!other_file)			/* A supplemental module */
		sprintf (module, "supplements/%s/%s.html", group, docname);

	/* Get the local URL (which may not exist) */
	if (other_file) {		/* A local or Web file */
		if (!strncmp (docname, "http", 4U) || !strncmp (docname, "ftp", 3U))
			sprintf (URL, "%s", docname);	/* Must assume that the address is correct */
		else	/* Must assume this is a local file */
			sprintf (URL, "file:///%s", docname);
	}
	else {	/* One of the fixed doc files */
		sprintf (URL, "file:///%s/doc/html/%s", API->GMT->session.SHAREDIR, module);
		if (access (&URL[8], R_OK)) 	/* File does not exists, go to SOEST site */
			sprintf (URL, "http://gmt.soest.hawaii.edu/doc/latest/%s", module);
	}

	if (opt->next) {	/* If an option request was made we position the doc there */
		char t[4] = {""};
		sprintf (t, "#%c", tolower (opt->next->option));
		strncat (URL, t, PATH_MAX-1);
	}

#ifdef WIN32
	k = 0;
#elif defined(__APPLE__)
	k = 1;
#else
	k = 2;
#endif

	sprintf (cmd, "%s %s", can_opener[k], URL);
	if ((error = system (cmd))) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Opening %s in the default browser via %s failed with error %d\n", URL, can_opener[k], error);
		perror ("docs");
		Return (GMT_RUNTIME_ERROR);
	}

	Return (error);
}
