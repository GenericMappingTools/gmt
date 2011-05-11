/*--------------------------------------------------------------------
 *	$Id: grdclip_func.c,v 1.10 2011-05-11 04:01:54 guru Exp $
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
 * API functions to support the gmtconvert application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Read a grid file and sets all values < the user-supplied
 * lower limit to the value <below>, and all values > the user-supplied
 * upper limit to the value <above>.  above/below can be any number,
 * including NaN.
 */

#include "gmt.h"

/* Control structure for grdclip */

struct GRDCLIP_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct G {	/* -G<output_grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct S {	/* -Sa<high/above> or -Sb<low/below> */
		GMT_LONG active;
		GMT_LONG mode;
		float high, above;
		float low, below;
	} S;
};

void *New_grdclip_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCLIP_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDCLIP_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
			
	return ((void *)C);
}

void Free_grdclip_Ctrl (struct GMT_CTRL *GMT, struct GRDCLIP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdclip_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdclip %s [API] - Clipping of range in grid files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdclip <input_grdfile> -G<output_grdfile> [%s]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-Sa<high/above>] [-Sb<low/below>] [%s]\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tYou must choose at least one -S option.\n");
	GMT_message (GMT, "\t-G name of output grid.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-Sa will set all data > high to the <above> value.\n");
	GMT_message (GMT, "\t-Sb will set all data < low to the <below> value.\n");
	GMT_message (GMT, "\t    above/below can be any number including NaN.\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdclip_parse (struct GMTAPI_CTRL *C, struct GRDCLIP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, n;
	char txt[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output filename */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (&opt->arg[0]);
				break;
			case 'S':	/* Set limits */
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
				case 'a':
					Ctrl->S.mode |= 1;
					n = sscanf (&opt->arg[1], "%f/%s", &Ctrl->S.high, txt);
					if (n != 2) {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Sa option: Expected -Sa<high>/<above>, <above> may be set to NaN\n");
						n_errors++;
					}
					else 
						Ctrl->S.above = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (float)atof (txt);
					break;
				case 'b':
					Ctrl->S.mode |= 2;
					n = sscanf (&opt->arg[1], "%f/%s", &Ctrl->S.low, txt);
					if (n != 2) {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Sb option: Expected -Sb<low>/<below>, <below> may be set to NaN\n");
						n_errors++;
					}
					else
						Ctrl->S.below = (txt[0] == 'N' || txt[0] == 'n') ? GMT->session.f_NaN : (float)atof (txt);
					break;
				default:
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -S option: Expected -Sa<high>/<above> or -Sb<low>/<below>\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.mode, "Syntax error -S option: Must specify at least one of -Sa, -Sb\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdclip_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdclip (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {
	GMT_LONG k, row, col, n_above = 0, n_below = 0, error, new_grid;
	
	double wesn[4];
	
	struct GRDCLIP_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *Out = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdclip_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdclip_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdclip", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", "", options))) Return (error);
	Ctrl = (struct GRDCLIP_CTRL *) New_grdclip_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdclip_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdclip main code ----------------------------*/

	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	
	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);
	if (GMT_is_subset (G->header, wesn)) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Subset requested; make sure wesn matches header spacing */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);	/* Get subset */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);			/* Disables further data input */

	new_grid = GMT_set_outgrid (GMT, G, &Out);	/* TRUE if input is a read-only array */

	GMT_grd_loop (G, row, col, k) {	/* Checking if extremes are exceeded (need not check NaN) */
		if (Ctrl->S.mode & 1 && G->data[k] > Ctrl->S.high) {
			Out->data[k] = Ctrl->S.above;
			n_above++;
		}
		else if (Ctrl->S.mode & 2 && G->data[k] < Ctrl->S.low) {
			Out->data[k] = Ctrl->S.below;
			n_below++;
		}
		else if (new_grid)
			Out->data[k] = G->data[k];
	}

	if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Out);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	if (new_grid) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Out);

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		char format[GMT_BUFSIZ];
		sprintf (format, "%s set to %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		if (Ctrl->S.mode & 2) {
			GMT_report (GMT, GMT_MSG_NORMAL, "%ld values < ", n_below);
			GMT_report (GMT, GMT_MSG_NORMAL, format, (double)Ctrl->S.low, (double)Ctrl->S.below);
		}
		if (Ctrl->S.mode & 1) {
			GMT_report (GMT, GMT_MSG_NORMAL, "%ld values > ", n_above);
			GMT_report (GMT, GMT_MSG_NORMAL, format, (double)Ctrl->S.high, (double)Ctrl->S.above);
		}
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
	Return (GMT_OK);
}
