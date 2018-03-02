/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#ifdef WEBVIEW
#define WEBVIEW_IMPLEMENTATION
#if defined(WIN32)
#	define WEBVIEW_WINAPI
#elif defined __APPLE__
#	define WEBVIEW_COCOA
#else 			// Not particular safe this default to unix
#	define WEBVIEW_GTK
#endif
#include "webview.h"
#endif

#define THIS_MODULE_NAME	"docs"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Show the HTML doc of the module name passed in"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: docs <module-name> [<-option>] [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<-option> is the one-letter option of the module in question (e.g, -R)\n");
	GMT_Message (API, GMT_TIME_NONE, "\tThen, we display the help positioned at that specific option.\n");
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
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Required module name not specified\n");
		return GMT_PARSE_ERROR;
	}
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC const char * api_get_module_group (void *V_API, char *module);

int GMT_docs (void *V_API, int mode, void *args) {
	int error = 0;
	char URL[PATH_MAX] = {""}, module[GMT_LEN64] = {""};
	const char *group = NULL;
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

	/*---------------------------- This is the docs main code ----------------------------*/

	opt = GMT_Find_Option (API, GMT_OPT_INFILE, options);	/* action target will appear as file name */
	if ((group = api_get_module_group (API, opt->arg)) == NULL) {
		//GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Module name not recognized\n");
		//Return (GMT_RUNTIME_ERROR);
	};

	if (group != NULL && !strcmp (group, "core"))	/* Core module */
		sprintf (module, "%s.html", opt->arg);
	else if (group != NULL)			/* A supplemental module */
		sprintf (module, "supplements/%s/%s.html", group, opt->arg);
	else							/* Assume it is modern mode name (e.g. plot). If not, browser will complain */
		sprintf(module, "%s.html", opt->arg);

	/* Get the local URL (which may not exist) */
	sprintf (URL, "file:///%s/doc/html/%s", API->GMT->session.SHAREDIR, module);
	if (access (&URL[8], R_OK)) 		/* file does not exists, go to SOEST site */
		sprintf (URL, "https://gmt.soest.hawaii.edu/doc/latest/%s", module);

	if (opt->next) {		/* If an option request was made */
		char t[4];
		sprintf (t, "#%c", tolower (opt->next->option));
		strncat (URL, t, PATH_MAX-1);
	}
#ifdef WEBVIEW
	webview ("GMT docs", URL, 900, 600, 1);
#endif

	Return (error);
}