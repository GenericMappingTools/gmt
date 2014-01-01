/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: gmtwhich.c will read a list of data files and return the
 * full path by searching the DATADIR list of directories.
 *
 * Author:	Paul Wessel
 * Date:	15-OCT-2009
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"gmtwhich"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Find full path to specified files"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-V"

struct GMTWHICH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
};

void *New_gmtwhich_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTWHICH_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTWHICH_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
		
	return (C);
}

void Free_gmtwhich_Ctrl (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_gmtwhich_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtwhich [files] [-A] [-C] [-D] [%s]\n", GMT_V_OPT);
     
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-A Only consider files you have permission to read [all files].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Print Y if found and N if not found.  No path is returned.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Print the directory where a file is found [full path to file].\n");
	GMT_Option (API, "V,.");
	
	return (EXIT_FAILURE);
}

int GMT_gmtwhich_parse (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtwhich and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Only consider readable files */
				Ctrl->A.active = true;
				break;
			case 'C':	/* Print Y or N instead of names */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Want directory instead */
				Ctrl->D.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, n_files == 0, "Syntax error: No files specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Syntax error: Cannot use -D if -C is set\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtwhich_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtwhich (void *V_API, int mode, void *args)
{
	int error = 0, fmode;
	
	char path[GMT_BUFSIZ] = {""}, *Yes = "Y", *No = "N", cwd[GMT_BUFSIZ] = {""}, *p = NULL;
	
	struct GMTWHICH_CTRL *Ctrl = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtwhich_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmtwhich_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtwhich_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtwhich_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtwhich_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtwhich main code ----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	
	if (Ctrl->D.active) getcwd (cwd, GMT_BUFSIZ);	/* Get full path, even for current dir */
	fmode = (Ctrl->A.active) ? R_OK : F_OK;	/* Either readable or existing files */
		
	for (opt = options; opt; opt = opt->next) {
		if (opt->option != '<') continue;	/* Skip anything but filenames */
		if (!opt->arg[0]) continue;		/* Skip empty arguments */

		if (GMT_getdatapath (GMT, opt->arg, path, fmode)) {	/* Found the file */
			if (Ctrl->D.active) {
				p = strstr (path, opt->arg);	/* Start of filename */
				if (!strcmp (p, path)) /* Current directory */
					GMT_Put_Record (API, GMT_WRITE_TEXT, cwd);
				else {
					*(--p) = 0;	/* Chop off file, report directory */
					GMT_Put_Record (API, GMT_WRITE_TEXT, path);
				}
			}
			else
				GMT_Put_Record (API, GMT_WRITE_TEXT, ((Ctrl->C.active) ? Yes : path));
		}
		else {
			if (Ctrl->C.active) GMT_Put_Record (API, GMT_WRITE_TEXT, No);
			GMT_Report (API, GMT_MSG_VERBOSE, "File %s not found!\n", opt->arg);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_OK);
}
