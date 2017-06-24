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
 * Brief synopsis: gmt figure sets name, formats and options for current figure.
 *	gmt figure [<prefix>] [<formats>] [<convertoptions>]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"figure"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Set figure format specifics under a GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: figure <prefix> [<formats>] [<convertoptions] [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<prefix> is the prefix to use for the next figure name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<formats> contains one or more comma-separated formats [%s].\n", gmt_session_format[GMT_SESSION_FORMAT]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose from these valid extensions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     bmp:	MicroSoft BitMap\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     eps:	Encapsulated PostScript\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     jpg:	Joint Photographic Experts Group format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     pdf:	Portable Document Format [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     png:	Portable Network Graphics\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ppm:	Portable Pixel Map\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     ps:	PostScript\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     tif:	Tagged Image Format File\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<convertoptions> contains one or more comma-separated options [%s]\n", GMT_SESSION_CONVERT);
	GMT_Message (API, GMT_TIME_NONE, "\t   that will be passed to psconvert when preparing the figures.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The subset of valid options are:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     A[<args>],C<args>,D<dir>,E<dpi>,P,Q<args>,S\n");
	GMT_Option (API, "V");
	
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

	if ((opt = options) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Required figure name not specified\n");
		return GMT_PARSE_ERROR;
	}
	/* Gave a figure prefix so can go on to check optional items */
	while (opt) {
		pos = 0;
		while (gmt_strtok (opt->arg, ",", &pos, p)) {	/* Check args to determine what this is */
			if (arg_category == GMT_NOTSET)
				arg_category = (strlen (p) == 1 || isupper (p[0])) ? GMT_IS_FMT : GMT_IS_OPT;
			if (arg_category == GMT_IS_FMT) {	/* Got format specifications, check if OK */
				int k = gmt_get_graphics_id (GMT, p);
				if (k == GMT_NOTSET) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized graphics format %s\n", p);
					n_errors++;
				}
			}
			else {	/* Check if valid psconvert options */
				if (!strchr ("ACDEPQS", p[0])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized psconvert option  -%s\n", p);
					n_errors++;
				}
			}
		}
		opt = opt->next;
		if (opt && opt->option == 'V') opt = opt->next;	/* Skip the verbose option */
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_figure (void *V_API, int mode, void *args) {
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, options)) != 0) Return (error);

	/*---------------------------- This is the figure main code ----------------------------*/

	if (options) arg = GMT_Create_Cmd (API, options);
	if (gmt_add_figure (API, arg))
		error = GMT_RUNTIME_ERROR;
		
	if (options) GMT_Destroy_Cmd (API, &arg);
	Return (error);
}
