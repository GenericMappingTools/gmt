/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#define THIS_MODULE_CLASSIC_NAME	"begin"
#define THIS_MODULE_MODERN_NAME	"begin"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Initiate a new GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

#include "gmt_gsformats.h"

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<prefix>] [<formats>] [<psconvertoptions] [-C] [%s]\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<prefix>");
	GMT_Usage (API, -2, "The prefix used for unnamed figures [%s].", GMT_SESSION_NAME);
	GMT_Usage (API, 1, "\n<formats>");
	GMT_Usage (API, -2, "Sets the default plot format(s) [%s].", gmt_session_format[API->GMT->current.setting.graphics_format]);
	GMT_Usage (API, -2, "\nChoose one or more of these valid extensions separated by commas:");
	GMT_Usage (API, 3, "bmp:	MicroSoft BitMap.");
	GMT_Usage (API, 3, "eps:	Encapsulated PostScript.");
	GMT_Usage (API, 3, "jpg:	Joint Photographic Experts Group format.");
	GMT_Usage (API, 3, "pdf:	Portable Document Format [Default].");
	GMT_Usage (API, 3, "png:	Portable Network Graphics.");
	GMT_Usage (API, 3, "PNG:	Portable Network Graphics (with transparency layer).");
	GMT_Usage (API, 3, "ppm:	Portable Pixel Map.");
	GMT_Usage (API, 3, "ps:	PostScript.");
	GMT_Usage (API, 3, "tif:	Tagged Image Format File.");
	GMT_Usage (API, -2, "Two raster modifiers may be appended:");
	GMT_Usage (API, 3, "+m For bmp, png, jpg, and tif, make a monochrome (grayscale) image [color].");
	GMT_Usage (API, 3, "+q Append quality in 0-100 for jpg only [%d].", GMT_JPEG_DEF_QUALITY);
	GMT_Usage (API, 1, "\n<psconvertoptions>");
	GMT_Usage (API, -2,	"contains one or more comma-separated options that"
		" will be passed to psconvert when preparing figures [%s].", GMT_SESSION_CONVERT);
	GMT_Usage (API, -2, "\nThe valid subset of psconvert options are");
	GMT_Usage (API, -3, "A[<args>], C<args>, D<dir>, E<dpi>, H<factor>, Mb|f<file>, Q<args>, S");
	GMT_Usage (API, -2, "See the psconvert documentation for details.");
	GMT_Usage (API, 1, "\n-C");
	GMT_Usage (API, -2, "Clean start: Ignore any %s files in the normal search path.", GMT_SETTINGS_FILE);
	GMT_Option (API, "V,;");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to begin */

	unsigned int n_errors = 0, pos = 0;
	char p[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;

	if ((opt = options) == NULL) return GMT_NOERROR;	/* No arguments given to begin, done here */

	/* May have <sessionname> [<formats> [<psconvertoptions>]] with options -C and -V anywhere */

	if (opt->option == 'V' || opt->option == 'C')	/* Skip any -V -C here */
		opt = opt->next;
	if (opt == NULL) return GMT_NOERROR;	/* Done */
	if (opt->option == GMT_OPT_INFILE) opt = opt->next;	/* Skip session name, now at formats */
	if (opt == NULL) return GMT_NOERROR;	/* Done, no formats */
	if (opt->option == 'V' || opt->option == 'C')	/* Skip any -V -C here given in-between words */
		opt = opt->next;
	if (opt == NULL) return GMT_NOERROR;	/* Done, still no formats found */
	if (opt) {	/* Also gave replacement primary format(s) */
		int k;
		while (gmt_strtok (opt->arg, ",", &pos, p)) {	/* Check each format to make sure each is OK */
			if ((k = gmt_get_graphics_id (GMT, p)) == GMT_NOTSET) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized graphics format %s\n", p);
				n_errors++;
			}
		}
	}
	/* There may be other arguments as well (psconvert string, -C -V) but not tested here */

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static char * begin_get_session_name_and_format (struct GMTAPI_CTRL *API, struct GMT_OPTION *opt, unsigned int *mode, int *error) {
	/* Extract session arguments (including optional graphics format) from options:
	 * gmt begin [<sessionname>] [<formats>] [<psconvertopts>] [-C] [-V<arg>]  */
	char buffer[GMT_LEN256] = {""};
	bool space = false;
	unsigned int n = 0;
	size_t len = 0;
	*error = GMT_NOERROR;
	*mode = GMT_BEGIN_WORKFLOW;
	if (opt == NULL) return NULL;	/* Go with the default settings */
	while (opt && n < 3) {	/* May have up to 3 valid arguments (not counting -C -V) */
		if (opt->option == GMT_OPT_INFILE) {	/* Valid "file" argument */
			gmt_filename_set (opt->arg);	/* Replace any spaces with ASCII 29 */
			if (space) len++, strncat (buffer, " ", GMT_LEN256-len);
			len += strlen (opt->arg);
			strncat (buffer, opt->arg, GMT_LEN256-len);
			space = true;
			gmt_filename_get (opt->arg);	/* Undo ASCII 29 */
			n++;
		}
		else if (opt->option == 'C')	/* Flag mode that we want a clean start with no user setting overrides */
			*mode |= GMT_CLEAN_WORKFLOW;
		else if (opt->option != 'V') {
			GMT_Report (API, GMT_MSG_ERROR, "Unrecognized argument -%c%s\n", opt->option, opt->arg);
			*error = GMT_PARSE_ERROR;
		}
		opt = opt->next;
	}
	return strdup (buffer);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_begin (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int workflow_mode;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) return (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the begin main code ----------------------------*/

	arg = begin_get_session_name_and_format (API, options, &workflow_mode, &error);
	if (error) {
		if (arg) gmt_M_str_free (arg);
		Return (error);
	}
	if (gmt_manage_workflow (API, workflow_mode, arg))
		error = GMT_RUNTIME_ERROR;

	if (arg) gmt_M_str_free (arg);
	Return (error);
}
