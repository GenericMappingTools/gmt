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
 * Brief synopsis: grdreformat.c reads a grid file in one format and outputs it in another
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE k_mod_grdreformat /* I am grdreformat */

#include "gmt.h"

struct GRDREFORMAT_CTRL {
	struct IO {
		bool active;
		char *file[2];
	} IO;
	struct N {	/* -N */
		bool active;
	} N;
};

void *New_grdreformat_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDREFORMAT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDREFORMAT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	return (C);
}

void Free_grdreformat_Ctrl (struct GMT_CTRL *GMT, struct GRDREFORMAT_CTRL *C) {	/* Deallocate control structure */
	if (C->IO.file[0]) free (C->IO.file[0]);	
	if (C->IO.file[1]) free (C->IO.file[1]);	
	GMT_free (GMT, C);	
}

int GMT_grdreformat_usage (struct GMTAPI_CTRL *C, int level)
{
	int i;
	struct GMT_CTRL *GMT = C->GMT;
	char **grdformats = GMT_grdformats_sorted (GMT);

	gmt_module_show_name_and_purpose (THIS_MODULE);
	GMT_message (GMT, "usage: grdreformat <ingrid>[=<id>[/<scale>/<offset>[/<nan>]]] <outgrid>[=<id>[/<scale>/<offset>[/<nan>]][:<driver>[/<dataType>]]]\n\t [-N] [%s] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<ingrid> is the grid file to convert.\n");
	GMT_message (GMT, "\t<outgrid> is the new converted grid file.\n");
	GMT_message (GMT, "\tscale and offset, if given, will multiply data by scale and add offset.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-N Do NOT write the header (for native grids only - ignored otherwise).\n");
	GMT_message (GMT, "\t   Useful when creating files to be used by grdraster.\n");
	GMT_explain_options (GMT, "rfV.");

	GMT_message (GMT, "\nThe following grid file formats are supported:\n");
	for (i = 1; i < GMT_N_GRD_FORMATS; ++i) {
		if (!strstr (grdformats[i], "not supported"))
			GMT_message (GMT, "\t%s\n", grdformats[i]);
	}
#ifdef HAVE_GDAL
	GMT_message (GMT, "\n	When <id>=gd on output, the grid will be saved using the GDAL library.\n");
	GMT_message (GMT, "	Specify <driver> and optionally <dataType>. Driver names are as in GDAL\n		(e.g., netCDF, GTiFF, etc.)\n");
	GMT_message (GMT, "	<dataType> is u8|u16|i16|u32|i32|float32; i|u denote signed|unsigned\n		integer.  Default type is float32.\n");
	GMT_message (GMT, "	Both driver names and data types are case insensitive.\n");
#endif
	return (EXIT_FAILURE);
}

int GMT_grdreformat_parse (struct GMTAPI_CTRL *C, struct GRDREFORMAT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdreformat and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_in = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input and Output files */
				if (n_in == 0)
					Ctrl->IO.file[n_in++] = strdup (opt->arg);
				else if (n_in == 1)
					Ctrl->IO.file[n_in++] = strdup (opt->arg);
				else {
					n_in++;
					GMT_report (GMT, GMT_MSG_NORMAL, "Syntax error: Specify only one input and one output file\n");
				}
				break;
			case '>':	/* Output file */
				Ctrl->IO.file[GMT_OUT] = strdup (opt->arg);
				n_in++;
				break;

			/* Processes program-specific parameters */

			case 'N':
				Ctrl->N.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_in != 2, "Syntax error: Must specify both input and output file names\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdreformat_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdreformat (void *V_API, int mode, void *args)
{
	int error = 0;
	unsigned int hmode, type[2];

	char fname[2][GMT_BUFSIZ];

	struct GMT_GRID *Grid = NULL;
	struct GRDREFORMAT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdreformat_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdreformat_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_gmt_module (API, THIS_MODULE, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, "-VfR", "", options)) Return (API->error);
	Ctrl = New_grdreformat_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdreformat_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdreformat main code ----------------------------*/

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
	GMT_grd_init (GMT, Grid->header, options, false);
	hmode = (Ctrl->N.active) ? GMT_GRID_NO_HEADER : 0;
	GMT_err_fail (GMT, GMT_grd_get_format (GMT, Ctrl->IO.file[0], Grid->header, true), Ctrl->IO.file[0]);
	type[0] = Grid->header->type;
	strncpy (fname[0], Grid->header->name, GMT_BUFSIZ);
	GMT_err_fail (GMT, GMT_grd_get_format (GMT, Ctrl->IO.file[1], Grid->header, false), Ctrl->IO.file[1]);
	type[1] = Grid->header->type;
	strncpy (fname[1], Grid->header->name, GMT_BUFSIZ);

	if (type[1] == GMT_GRID_IS_SD) {
		/* Golden Surfer format 7 is read-only */
		GMT_report (GMT, GMT_MSG_NORMAL, "Writing unsupported: %s\n", GMT->session.grdformat[GMT_GRID_IS_SD]);
		Return (EXIT_FAILURE);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		if (Ctrl->IO.file[0][0] == '=') strcpy (fname[0], "<stdin>");
		if (Ctrl->IO.file[1][0] == '=') strcpy (fname[1], "<stdout>");
		GMT_report (GMT, GMT_MSG_VERBOSE, "Translating file %s (format %s)\nto file %s (format %s)\n", fname[0], GMT->session.grdformat[type[0]], fname[1], GMT->session.grdformat[type[1]]);
		if (hmode && GMT->session.grdformat[type[1]][0] != 'c' && GMT->session.grdformat[type[1]][0] != 'n') GMT_report (GMT, GMT_MSG_NORMAL, "No grd header will be written\n");
	}
	GMT_free_grid (GMT, &Grid, true);

	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER, NULL, Ctrl->IO.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (GMT->common.R.active) {	/* Specified a subset */
		bool global = false;
		global = GMT_grd_is_global (GMT, Grid->header);
		if (!global && (GMT->common.R.wesn[XLO] < Grid->header->wesn[XLO] || GMT->common.R.wesn[XHI] > Grid->header->wesn[XHI])) error++;
		if (GMT->common.R.wesn[YLO] < Grid->header->wesn[YLO] || GMT->common.R.wesn[YHI] > Grid->header->wesn[YHI]) error++;
		if (error) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Subset exceeds data domain!\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA, GMT->common.R.wesn, Ctrl->IO.file[0], Grid) == NULL) {
			Return (API->error);	/* Get subset */
		}
	}
	else if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA, NULL, Ctrl->IO.file[0], Grid) == NULL) {
		Return (API->error);	/* Get all */
	}

	Grid->header->type = type[1];

	GMT_grd_init (GMT, Grid->header, options, true);

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, hmode, NULL, Ctrl->IO.file[1], Grid) != GMT_OK) {
		Return (API->error);
	}

	Return (GMT_OK);
}
