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
 * Author:	Joaquim Luis
 * Date:	1-Mar-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt docs 
 *	Opens GMT HTML docs in the default viewer (browser); when called from
 *	gmt end show it also opens illustrations via default viewer. 
 */

#include "gmt_dev.h"
#include "gmt_gsformats.h"
#ifdef WIN32
#	include <windows.h>
#endif

#define THIS_MODULE_CLASSIC_NAME	"docs"
#define THIS_MODULE_MODERN_NAME	"docs"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Show HTML documentation of specified module"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-Q] [-S] [%s] <module-name> [<-option>]\n\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);
	GMT_Message (API, GMT_TIME_NONE, "\t<module-name> is one of the core or supplemental modules,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or one of gmt, api, colors, cookbook, gallery, settings, and tutorial.\n");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q will only display the URLs and not open them in a viewer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If given, -Q must be the first argument to %s.\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t-S will open documentation files from the GMT server.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<-option> is the one-letter option of the module in question (e.g, -R).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Display the documentation positioned at that specific option.\n");
	GMT_Option (API, "V,;");
	
	return (GMT_MODULE_USAGE);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC const char *api_get_module_group (void *V_API, char *module);

int GMT_docs (void *V_API, int mode, void *args) {
	bool other_file = false, print_url = false, got_file = false, called = false, remote = false;
	int error = 0, id;
	size_t vlen = 0;
	char cmd[PATH_MAX] = {""}, view[PATH_MAX] = {""}, URL[PATH_MAX] = {""}, module[GMT_LEN64] = {""}, name[PATH_MAX] = {""}, *t = NULL, *ext = NULL;
	const char *group = NULL, *docname = NULL;
	char *ps_viewer = NULL;
	static const char *known_group[2] = {"core", "other"}, *known_doc[7] = {"gmtcolors", "cookbook", "api", "tutorial", "gallery", "gmt.conf", "gmt"};
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

#ifdef WIN32
	HKEY hkey;              	/* Handle to registry key */
	static const char *file_viewer = "cmd /c start";
	bool together = false;		/* Must call file_viewer separately on each file */
	long RegO = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.ps\\UserChoice", 0, KEY_READ, &hkey);
	if (RegO == ERROR_SUCCESS)		/* User has postscript viewer installed */
		ps_viewer = (char *)file_viewer;
	else
		ps_viewer = "cmd /c start gsview64c";	/* No previewer. Resort to this. */
#elif defined(__APPLE__)
	static const char *file_viewer = "open";
	bool together = true;	/* Can call file_viewer once with all files */
	ps_viewer = (char *)file_viewer;
#else
	static const char *file_viewer = "xdg-open";
	bool together = false;	/* Must call file_viewer separately on each file */
	ps_viewer = (char *)file_viewer;
#endif

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */
	/* Here we do not want to call GMT_Parse_Common since common options may be passed as sections in the docs */
	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */

	/*---------------------------- This is the docs main code ----------------------------*/

	opt = options;	/* Start at first option to gmt docs */
	
	while (opt) {	/* For all possible arguments */
		if (opt->option == 'Q') { print_url = true, opt = opt->next; continue; }	/* Process optional -Q option which must be first (or maybe second if -S also) */
		else if (opt->option == 'S') { remote = true, opt = opt->next; continue; }	/* Process optional -S option which is either first or second (if -Q) */
		else if (opt->option == 'V') { opt = opt->next; continue; }	/* Skip the optional -V common option */
		
		if (opt->option != GMT_OPT_INFILE) {	/* This is not good */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown option (-%c)\n", opt->option);
			Return (GMT_RUNTIME_ERROR);
		}
		
		if ((ext = gmt_get_ext (opt->arg)) && (id = gmt_get_graphics_id (GMT, ext)) != GMT_NOTSET) {	/* Got a graphics file */
			if (strchr (opt->arg, GMT_ASCII_RS)) {	/* Got a file with spaces there are temporarily represented by RS */
				sprintf (name, "\'%s\'", opt->arg);
				gmt_filename_get (name);	/* Reinstate the spaces since we added quotes */
			}
			else
				strcpy (name, opt->arg);
			if (GMT->hidden.func_level == GMT_TOP_MODULE) {	/* Can only open figs if called indirectly via gmt end show */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Argument %s is not a known module or documentation short-hand\n",
					name);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (print_url) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Reporting local file %s to stdout\n", name);
				printf ("%s\n", opt->arg);
			}
			else {	/* Open in suitable viewer */
#ifdef __APPLE__
				/* Under macOS a PostScript file will be opened by open via conversion to PDF.
				 * If the user has gv installed then we rather open the PostScript file in gv */
				if (gmt_session_code[id] == 'p') {	/* PostScript file, check if a PostScript viewer "gv" is available */
					if (gmt_check_executable (GMT, "gv", "--version", NULL, NULL)) {
						ps_viewer = "gv";
						together = false;	/* gv takes one file at the time */
						GMT_Report (API, GMT_MSG_DEBUG, "Found gv and will open PostScript files with it.\n");
					}
					else
						GMT_Report (API, GMT_MSG_DEBUG, "gv is not installed or not in your executable path, use %s instead.\n", file_viewer);
				}
#endif
				if (!together || !got_file) {	/* Either Windows|Linux, or first time under macOS */
					snprintf (view, PATH_MAX, "%s %s", (gmt_session_code[id] == 'p') ? ps_viewer : file_viewer, name);
					got_file = true;
					vlen = PATH_MAX - strlen (view);
#ifdef __APPLE__
					if (!strncmp (ps_viewer, "gv", 2U)) strcat (view, " &");	/* Need to put gv in the background */
#endif
				}
				else {	/* Append more arguments to the same open command */
					strncat (view, " ", vlen--);
					strncat (view, opt->arg, vlen);
					vlen -= strlen (opt->arg);
				}
				if (!together) {	/* Must call file_viewer separately on each file */
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Opening local file %s via %s\n", opt->arg, file_viewer);
					if ((error = system (view))) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Opening local file %s via %s failed with error %d\n",
							opt->arg, file_viewer, error);
						perror ("docs");
						Return (GMT_RUNTIME_ERROR);
					}
				}
			}
			called = true;
		}
		else {	/* Documentation references */

			docname = gmt_current_name (opt->arg, name);	/* Get modern mode name */
	
			if (strcmp (opt->arg, docname))	/* Gave classic name so change to what was given */
				docname = opt->arg;
			else
				docname = gmt_get_full_name (API, opt->arg);
				

			t = strdup (docname);	/* Make a copy because gmt_str_tolower changes the input that may be a const char */
			gmt_str_tolower (t);
			if (!strcmp (t, "colors")) {
				docname = known_doc[0];	group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "cookbook")) {
				docname = known_doc[1];	group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "api")) {
				docname = known_doc[2]; group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "tutorial")) {
				docname = known_doc[3];	group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "gallery")) {
				docname = known_doc[4];	group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "gmt.conf") || !strncmp (t, "setting", 7U)) {
				docname = known_doc[5];	group   = known_group[0];	/* Pretend it is in the core */
			}
			else if (!strcmp (t, "gmt")) {
				docname = known_doc[6];	group   = known_group[0];	/* Pretend it is in the core */
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
				snprintf (module, GMT_LEN64, "%s.html", docname);
			else if (!other_file)		/* A supplemental module */
				snprintf (module, GMT_LEN64, "supplements/%s/%s.html", group, docname);

			if (opt->next && opt->next->option != GMT_OPT_INFILE) remote = true;	/* Can only use anchors on actual URLs not local files */
			
			/* Get the local URL (which may not exist) */
			if (other_file) {	/* A local or Web file */
				if (!strncmp (docname, "file:", 5U) || !strncmp (docname, "http", 4U) || !strncmp (docname, "ftp", 3U))	/* Looks like an URL already */
					snprintf (URL, PATH_MAX, "%s", docname);	/* Must assume that the address is correct */
				else {	/* Must assume this is a local file */
					if (docname[0] == '/' || docname[1] == ':')	/* Gave full path to file, use as is */
						snprintf (URL, PATH_MAX, "file://%s", docname);
					else {	/* Insert file:// if we can determine the current directory */
						char cwd[PATH_MAX] = {""};
						if (getcwd (cwd, PATH_MAX) == NULL) {
							GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory - pass file name as is.\n");
							snprintf (URL, PATH_MAX, "%s", docname);
						}
						else	/* Prepend CWD */
							snprintf (URL, PATH_MAX, "file://%s/%s", cwd, docname);
					}
				}
			}
			else {	/* One of the fixed doc files */
				if (remote) {	/* Go directly to GMT server online */
						snprintf (URL, PATH_MAX, "%s/%s", GMT_DOC_URL, module);
						GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Try URL path: %s\n", URL);
				}
				else {	/* Try local paths first */
					snprintf (URL, PATH_MAX, "file:///%s/doc/html/%s", API->GMT->session.SHAREDIR, module);
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Try URL path: %s\n", URL);
					if (access (&URL[8], R_OK)) { 	/* File does not exists, try GMT_DOC_DIR */
						snprintf (URL, PATH_MAX, "file:///%s/html/%s", GMT_DOC_DIR, module);
						GMT_Report (GMT->parent, GMT_MSG_DEBUG, "No access, Now try URL path: %s\n", URL);
						if (access (&URL[8], R_OK)) { 	/* File does not exists, try GMT_SHARE_DIR */
							snprintf (URL, PATH_MAX, "file:///%s/doc/html/%s", GMT_SHARE_DIR, module);
							GMT_Report (GMT->parent, GMT_MSG_DEBUG, "No access, Now try URL path: %s\n", URL);
							if (access (&URL[8], R_OK)) { 	/* File does not exists, give up and use remote link */
								snprintf (URL, PATH_MAX, "%s/%s", GMT_DOC_URL, module);
								GMT_Report (GMT->parent, GMT_MSG_DEBUG, "No local access,use server URL path: %s\n", URL);
							}
						}
					}
				}
			}

			if (opt->next && opt->next->option != GMT_OPT_INFILE) {	/* If an option request was made we position the doc there */
				char t[4] = {""};
				snprintf (t, 4U, "#%c", tolower (opt->next->option));
				strncat (URL, t, PATH_MAX-1);
				opt = opt->next;	/* Skip past this module option */
			}
			called = true;

			if (print_url) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Reporting URL %s to stdout\n", URL);
				printf ("%s\n", URL);
			}
			else {
				sprintf (cmd, "%s %s", file_viewer, URL);
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Opening %s via %s\n", URL, file_viewer);
				if ((error = system (cmd))) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Opening %s via %s failed with error %d\n",
						URL, file_viewer, error);
					perror ("docs");
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		opt = opt->next;
	}

	if (together && got_file) {	/* Call file_viewer once with all given files */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Opening local file(s) via %s\n", view);
		if ((error = system (view))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Opening local files via %s failed with error %d\n",
				view, error);
			perror ("docs");
			Return (GMT_RUNTIME_ERROR);
		}
		called = true;
	}
	
	if (!called) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No files or documents given\n");
		Return (GMT_RUNTIME_ERROR);
	}
	
	Return (error);
}
