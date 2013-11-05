/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Brief synopsis: gmtwhich.c will read a list of data files and return the
 * full path by searching the DATADIR list of directories.
 *
 * Author:	Paul Wessel
 * Date:	15-OCT-2009
 * Version:	5 API
 */

#include "gmt.h"

struct GMTWHICH_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct C {	/* -C */
		GMT_LONG active;
	} C;
};

void *New_gmtwhich_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTWHICH_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTWHICH_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
		
	return (C);
}

void Free_gmtwhich_Ctrl (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gmtwhich_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtwhich %s [API] - Find full path to specified files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtwhich [files] [-C] [%s]\n", GMT_V_OPT);
     
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-C Print Y if found and N if not found.  No path is returned.\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtwhich_parse (struct GMTAPI_CTRL *C, struct GMTWHICH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtwhich and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Want Yes/No response instead */
				Ctrl->C.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, n_files == 0, "Syntax error: No files specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtwhich_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmtwhich (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = 0;
	
	char path[GMT_BUFSIZ], *Yes = "Y", *No = "N";
	
	struct GMTWHICH_CTRL *Ctrl = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtwhich_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtwhich_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtwhich", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-V", "", options)) Return (API->error);
	Ctrl = New_gmtwhich_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtwhich_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtwhich main code ----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	
	for (opt = options; opt; opt = opt->next) {
		if (opt->option != '<') continue;	/* Skip anything but filenames */
		if (!opt->arg[0]) continue;		/* Skip empty arguments */

		if (GMT_getdatapath (GMT, opt->arg, path))	/* Found the file */
			GMT_Put_Record (API, GMT_WRITE_TEXT, ((Ctrl->C.active) ? Yes : path));
		else {
			if (Ctrl->C.active) GMT_Put_Record (API, GMT_WRITE_TEXT, No);
			GMT_report (GMT, GMT_MSG_NORMAL, "File %s not found!\n", opt->arg);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_OK);
}
