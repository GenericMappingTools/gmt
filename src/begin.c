/*--------------------------------------------------------------------
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
 * Brief synopsis: gmt begin starts a modern mode session.
 *	gmt begin [<prefix>] [<format>] [-V<level>]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"begin"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Initiate a new GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

#include "gmt_gsformats.h"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<prefix>] [<format(s)>] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<prefix> is the prefix to use for unnamed figures [%s].\n", GMT_SESSION_NAME);
	GMT_Message (API, GMT_TIME_NONE, "\t<format(s)> sets the default plot format(s) [%s].\n", gmt_session_format[GMT_SESSION_FORMAT]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose one or more of these valid extensions separated by commas:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     bmp:	MicroSoft BitMap.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     eps:	Encapsulated PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     jpg:	Joint Photographic Experts Group format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     pdf:	Portable Document Format [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     png:	Portable Network Graphics (opaque).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     PNG:	Portable Network Graphics (transparent).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ppm:	Portable Pixel Map.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ps:	PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     tif:	Tagged Image Format File.\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to begin */

	unsigned int n_errors = 0, pos = 0;
	char p[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;

	GMT->current.ps.crop_to_fit = true;	/* Default is to make a tight PDF plot, unless PS */
	if ((opt = options))	/* Gave a replacement session name and possibly more */
		opt = opt->next;	/* Skip session name */
	if (opt && opt->option == 'V')	/* Skip any -V already processed by GMT_Parse_Common */
		opt = opt->next;
	if (opt) {	/* Also gave replacement primary format(s) */
		int k;
		while (gmt_strtok (opt->arg, ",", &pos, p)) {	/* Check each format to make sure each is OK */
			if ((k = gmt_get_graphics_id (GMT, p)) == GMT_NOTSET) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized graphics format %s\n", p);
				n_errors++;
			}
			else if (!strcmp (p, "ps"))	/* Need to honor PS_MEDIA setting */
				GMT->current.ps.crop_to_fit = false;
		}
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

char *get_session_name_and_format (struct GMT_OPTION *opt) {
	/* Extract session arguments (including optional graphics format) from options */
	char buffer[GMT_LEN256] = {""};
	bool space = false;
	unsigned int n = 0;
	if (opt == NULL) return NULL;	/* Go with the default settings */
	while (opt && n < 2) {
		if (opt->option == GMT_OPT_INFILE) {	/* Valid file argument */
			if (space) strcat (buffer, " ");
			strcat (buffer, opt->arg);
			space = true;
			n++;
		}
		opt = opt->next;
	}
	return strdup (buffer);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_begin (void *V_API, int mode, void *args) {
	int error = 0;
	char *arg = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options) {
		if (options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));		/* Return the usage message */
		if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */
	}

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) return (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the begin main code ----------------------------*/

	arg = get_session_name_and_format (options);
	if (gmt_manage_workflow (API, GMT_BEGIN_WORKFLOW, arg))
		error = GMT_RUNTIME_ERROR;

	if (arg) gmt_M_str_free (arg);
	Return (error);
}
