/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: gmt figure sets name, formats and options for current figure.
 *	gmt figure <prefix> [<formats>] [<psconvertoptions>]
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"figure"
#define THIS_MODULE_MODERN_NAME	"figure"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Set attributes for the current modern mode session figure"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

#include "gmt_gsformats.h"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <prefix> [<formats>] [<psconvertoptions] [%s]\n\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<prefix> is the prefix to use for the registered figure\'s name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<formats> contains one or more comma-separated formats [%s].\n", gmt_session_format[API->GMT->current.setting.graphics_format]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose from these valid extensions:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     bmp:	MicroSoft BitMap.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     eps:	Encapsulated PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     jpg:	Joint Photographic Experts Group format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     pdf:	Portable Document Format [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     png:	Portable Network Graphics.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     PNG:	Portable Network Graphics (with transparency layer).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ppm:	Portable Pixel Map.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ps:	PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     tif:	Tagged Image Format File.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<psconvertoptions> contains one or more comma-separated options that\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   will be passed to psconvert when preparing this figure [%s].\n", GMT_SESSION_CONVERT);
	GMT_Message (API, GMT_TIME_NONE, "\t   The valid subset of psconvert options are\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     A[<args>],C<args>,D<dir>,E<dpi>,H<factor>,Mb|f<file>,Q<args>,S\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See the psconvert documentation for details.\n");
	GMT_Option (API, "V,;");

	return (GMT_MODULE_USAGE);
}

#define GMT_IS_FMT	0
#define GMT_IS_OPT	1

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0;
	int arg_category = GMT_NOTSET;
	char p[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	if ((opt = options) == NULL) {	/* Gave no arguments */
		if (GMT->parent->external) return GMT_NOERROR;
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Required figure name not specified!\n");
		return GMT_PARSE_ERROR;
	}
	gmt_filename_set (opt->arg);

	/* Gave a figure prefix so can go on to check optional items */

	opt = opt->next;	/* Skip the figure prefix since we don't need to check it here */

	while (opt) {
		gmt_filename_set (opt->arg);
		arg_category = GMT_NOTSET;	/* We know noothing */
		pos = 0;
		while (gmt_strtok (opt->arg, ",", &pos, p)) {	/* Check args to determine what kind it is */
			if (arg_category == GMT_NOTSET)
				arg_category = (strlen (p) == 1 || strchr (p, '+') || (isupper (p[0]) && strcmp (p, "PNG"))) ? GMT_IS_OPT : GMT_IS_FMT;
			if (arg_category == GMT_IS_FMT) {	/* Got format specifications, check if OK */
				int k = gmt_get_graphics_id (GMT, p);
				if (k == GMT_NOTSET) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized graphics format %s\n", p);
					n_errors++;
				}
			}
			else {	/* Check if valid psconvert options */
				if (!strchr ("ACDEHMQS", p[0])) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized psconvert option  -%s\n", p);
					n_errors++;
				}
			}
		}
		opt = opt->next;
		if (opt && opt->option == 'V') opt = opt->next;	/* Skip the verbose option */
	}

	/* If we get here without errors then we know the input arguments are all valid */

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_figure (void *V_API, int mode, void *args) {
	int error = 0;
	char *arg = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options && !API->external) {
		if (options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));		/* Return the usage message */
		if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */
	}
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_ERROR, "Not available in classic mode\n");
		return (GMT_NOT_MODERN_MODE);
	}

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the figure main code ----------------------------*/

	if (options) arg = GMT_Create_Cmd (API, options);
	if (gmt_add_figure (API, arg))
		error = GMT_RUNTIME_ERROR;

	if (options) GMT_Destroy_Cmd (API, &arg);

	opt = options;
	while (opt) {
		gmt_filename_get (opt->arg);
		opt = opt->next;
	}

	gmt_reset_history (GMT);	/* Prevent gmt figure from copying previous history to this new fig */

	Return (error);
}
