/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Date:	2-May-2013
 * Version:	5 API
 *
 * Brief synopsis: gmt read lets us read (and write to memory) any of the 5 GMT resources.
 *
 */

#define THIS_MODULE_NAME	"read"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Read GMT objects into external API"

#include "gmt_lib.h"

#define GMT_PROG_OPTIONS "->RV"

/* Control structure for read */

struct GMTREAD_CTRL {
	struct IO {	/* Need two args with filenames */
		bool active[2];
		char *file[2];
	} IO;
	struct T {	/* -T sets data type */
		bool active;
		enum GMT_enum_family mode;
	} T;
};

void *New_gmtread_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTREAD_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTREAD_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_gmtread_Ctrl (struct GMT_CTRL *GMT, struct GMTREAD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->IO.file[GMT_IN]) free ((void *)C->IO.file[GMT_IN]);
	if (C->IO.file[GMT_OUT]) free ((void *)C->IO.file[GMT_OUT]);
	GMT_free (GMT, C);
}

int GMT_gmtread_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: read <infile> <outfile> -Td|t|g|c|i [%s] [%s]\n", GMT_Rx_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t   Specify input and output file names\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify data type.  Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : Dataset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t : Textset\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g : Grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c : CPT\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i : Image\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "R,V,.");
	
	return (EXIT_FAILURE);
}

int GMT_gmtread_parse (struct GMT_CTRL *GMT, struct GMTREAD_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Processes program-specific parameters */

			case GMT_OPT_INFILE:	/* File args */
				if (Ctrl->IO.active[GMT_OUT]) {	/* User gave output as ->outfile and it was found on the command line earlier */
					Ctrl->IO.active[GMT_IN] = true;
					Ctrl->IO.file[GMT_IN] = strdup (opt->arg);
				}
				else if (n_files < 2) {	/* Encountered input file(s); the 2nd would be output */
					Ctrl->IO.active[n_files] = true;
					Ctrl->IO.file[n_files] = strdup (opt->arg);
				}
				n_files++;
				break;
			case GMT_OPT_OUTFILE:	/* Got specific output argument */
				Ctrl->IO.active[GMT_OUT] = true;
				Ctrl->IO.file[GMT_OUT] = strdup (opt->arg);
				n_files++;
				break;
			case 'T':	/* Type */
				Ctrl->T.active = true;
				switch (opt->arg[0]) {
					case 'd': Ctrl->T.mode = GMT_IS_DATASET; break;
					case 't': Ctrl->T.mode = GMT_IS_TEXTSET; break;
					case 'g': Ctrl->T.mode = GMT_IS_GRID;	 break;
					case 'c': Ctrl->T.mode = GMT_IS_CPT;	 break;
					case 'i': Ctrl->T.mode = GMT_IS_IMAGE;	 break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized data type %c.  Choose from c, d, g, i, and t\n");
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, !(Ctrl->IO.active[GMT_IN] && Ctrl->IO.active[GMT_OUT]), "Syntax error: Must specify both input and output filenames\n");
	n_errors += GMT_check_condition (GMT, Ctrl->IO.active[GMT_IN] && (!Ctrl->IO.file[GMT_IN] || !Ctrl->IO.file[GMT_IN][0]), "Syntax error: Must specify input filename\n");
	n_errors += GMT_check_condition (GMT, Ctrl->IO.active[GMT_OUT] && (!Ctrl->IO.file[GMT_OUT] || !Ctrl->IO.file[GMT_OUT][0]), "Syntax error: Must specify output filename\n");
	n_errors += GMT_check_condition (GMT, n_files != 2, "Syntax error: Must specify only two filenames (input and output)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active, "Syntax error -T option: Must specify a valid datatype\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtread_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_copy (struct GMTAPI_CTRL *API, enum GMT_enum_family family, unsigned int direction, char *ifile, char *ofile);

int GMT_read (void *V_API, int mode, void *args)
{
	int error = 0;
	struct GMTREAD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtread_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmtread_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtread_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtread_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtread_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtread main code ----------------------------*/

	error = GMT_copy (API, Ctrl->T.mode, GMT_IN, Ctrl->IO.file[GMT_IN], Ctrl->IO.file[GMT_OUT]);
	Return (error);
}
