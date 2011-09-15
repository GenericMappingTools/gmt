/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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
 
#define GMT_WITH_NO_PS
#include "gmt.h"

/* Control structure for gmtget */

struct GMTGET_CTRL {
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct G {	/* -Gfilename */
		GMT_LONG active;
		char *file;
	} G;
};

void *New_gmtget_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTGET_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTGET_CTRL);
	return ((void *)C);
}

void Free_gmtget_Ctrl (struct GMT_CTRL *GMT, struct GMTGET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gmtget_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtget %s [API] - Get individual GMT default parameters\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtget [-L] [-G<defaultsfile>] PARAMETER1 PARAMETER2 PARAMETER3 ...\n");
	GMT_message (GMT, "\n\tFor available PARAMETERS, see gmt.conf man page\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-G Set name of specific gmt.conf file to process.\n");
	GMT_message (GMT, "\t   [Default looks for file in current directory.  If not found,\n");
	GMT_message (GMT, "\t   it looks in the home directory, if not found it uses GMT defaults].\n");
	GMT_message (GMT, "\t-L Write one parameter value per line [Default writes all on one line].\n");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtget_parse (struct GMTAPI_CTRL *C, struct GMTGET_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtget and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = 1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Optional defaults file on input and output */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'L':	/* One per line */
				Ctrl->L.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtget_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmtget (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = 0;
	
	struct GMTGET_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (options) {
		if (options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtget_usage (API, GMTAPI_USAGE));		/* Return the usage message */
		if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtget_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */
	}

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtget", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", "", options))) Return (error);
	Ctrl = (struct GMTGET_CTRL *) New_gmtget_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtget_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtget main code ----------------------------*/

	/* Read the supplied default file or the users defaults to override system settings */

	if (Ctrl->G.active) GMT_getdefaults (GMT, Ctrl->G.file);

	GMT_pickdefaults (GMT, Ctrl->L.active, options);		/* Process command line arguments */

	Return (GMT_OK);
}
