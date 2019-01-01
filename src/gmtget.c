/*--------------------------------------------------------------------
 *	$Id$
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
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: gmtget will return the values of selected parameters.
 *
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_NAME	"gmtget"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Get individual GMT default parameters"
#define THIS_MODULE_KEYS	">T}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

/* Control structure for gmtget */

struct GMTGET_CTRL {
	struct L {	/* -L */
		bool active;
	} L;
	struct G {	/* -Gfilename */
		bool active;
		char *file;
	} G;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTGET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTGET_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTGET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtget [-L] [-G<defaultsfile>] PARAMETER1 PARAMETER2 PARAMETER3 ...\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tFor available PARAMETERS, see gmt.conf man page\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of specific gmt.conf file to process.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default looks for file in current directory.  If not found,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   it looks in the home directory, if not found it uses GMT defaults].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Write one parameter value per line [Default writes all on one line].\n");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTGET_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtget and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = 1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Optional defaults file on input and output */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_IN, GMT_IS_TEXTSET)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* One per line */
				Ctrl->L.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtget (void *V_API, int mode, void *args) {
	int error = GMT_NOERROR;

	struct GMTGET_CTRL *Ctrl = NULL;
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
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtget main code ----------------------------*/

	/* Read the supplied default file or the users defaults to override system settings */

	if (Ctrl->G.active || API->external) gmt_getdefaults (GMT, Ctrl->G.file);	/* Update defaults if using external API */

	error = gmt_pickdefaults (GMT, Ctrl->L.active, options);		/* Process command line arguments */

	Return (error);
}
