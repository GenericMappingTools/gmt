/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: gmtdefaults will list the user's GMT default settings
 * or (by using the -D option), get the site GMT default settings.
 *
 */
 
#define THIS_MODULE_NAME	"gmtdefaults"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"List current GMT default parameters"
#define THIS_MODULE_KEYS	""

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-V"

/* Control structure for gmtdefaults */

struct GMTDEFAULTS_CTRL {
	struct D {	/* -D[s|u] */
		bool active;
		char mode;
	} D;
};

void *New_gmtdefaults_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTDEFAULTS_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTDEFAULTS_CTRL);
	return (C);
}

void Free_gmtdefaults_Ctrl (struct GMT_CTRL *GMT, struct GMTDEFAULTS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_gmtdefaults_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtdefaults [-D[s|u]]\n\n");
	
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_Message (API, GMT_TIME_NONE, "\t-D Print the GMT default settings.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to see the SI version of defaults.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append u to see the US version of defaults.\n");
	
	return (EXIT_FAILURE);
}

int GMT_gmtdefaults_parse (struct GMT_CTRL *GMT, struct GMTDEFAULTS_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtdefaults and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Count input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Get GMT system-wide defaults settings */
				Ctrl->D.active = true;
				Ctrl->D.mode = opt->arg[0];
				break;
			case 'L':	/* List the user's current GMT defaults settings */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; it is now the default behavior.\n");
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files, "Syntax error: No input files are expected\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtdefaults_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtdefaults (void *V_API, int mode, void *args)
{
	int error;
	
	char path[GMT_LEN256] = {""};
	
	struct GMTDEFAULTS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtdefaults_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options) {
		if (options->option == GMT_OPT_USAGE) bailout (GMT_gmtdefaults_usage (API, GMT_USAGE));		/* Return the usage message */
		if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtdefaults_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */
	}

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtdefaults_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtdefaults_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtdefaults main code ----------------------------*/

	if (Ctrl->D.active) {
		GMT_getsharepath (GMT, "conf", "gmt", (Ctrl->D.mode == 's') ? "_SI" : (Ctrl->D.mode == 'u') ? "_US" : ".conf", path, R_OK);
		GMT_loaddefaults (GMT, path);
	}

	GMT_putdefaults (GMT, "-");

	Return (GMT_OK);
}
