/*--------------------------------------------------------------------
 *	$Id: grdreformat_func.c,v 1.5 2011-04-23 02:14:13 guru Exp $
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
 * Brief synopsis: grdreformat.c reads a grid file in one format and outputs it in another
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

struct GRDREFORMAT_CTRL {
	struct IO {
		GMT_LONG active;
		char *file[2];
	} IO;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
};

EXTERN_MSC GMT_LONG GMT_grd_get_format (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header, GMT_LONG magic);

void *New_grdreformat_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDREFORMAT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDREFORMAT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return ((void *)C);
}

void Free_grdreformat_Ctrl (struct GMT_CTRL *GMT, struct GRDREFORMAT_CTRL *C) {	/* Deallocate control structure */
	if (C->IO.file[0]) free ((void *)C->IO.file[0]);	
	if (C->IO.file[1]) free ((void *)C->IO.file[1]);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdreformat_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
#include "grdreformat.h"	/* Defines N_GRDTXT_LINES and char *grd_formats[N_GRDTXT_LINES] array used in usage message */
	GMT_LONG i;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdreformat %s [API] - Converting between different grid file formats\n\n", GMT_VERSION);
	fprintf( GMT->session.std[GMT_ERR], "usage: grdreformat ingrdfile[=id[/scale/offset]] outgrdfile[=id[/scale/offset]] [-N]\n\t[%s] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tingrdfile is the grid file to convert.\n");
	GMT_message (GMT, "\toutgrdfile is the new converted grid file.\n");
	fprintf( GMT->session.std[GMT_ERR], "\tscale and offset, if given, will multiply data by scale and add offset.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-N  Do NOT write the header (native grids only - ignored otherwise).\n");
	GMT_message (GMT, "\t\t  Useful when creating files to be used by grdraster.\n");
	GMT_explain_options (GMT, "rfV.");

	GMT_message (GMT, "\n	The following grid file formats are supported:\n\n");
#ifdef USE_GDAL
	for (i = 0; i < N_GRDTXT_LINES; i++) GMT_message (GMT, "\t%s\n", grd_formats[i]);
#else
	for (i = 0; i < N_GRDTXT_LINES; i++) if (!strstr (grd_formats[i], "GDAL")) GMT_message (GMT, "\t%s\n", grd_formats[i]);
#endif
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdreformat_parse (struct GMTAPI_CTRL *C, struct GRDREFORMAT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdreformat and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_in = 0;
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
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Specify only one input and one output file\n");
				}
				break;
			case '>':	/* Output file */
				Ctrl->IO.file[GMT_OUT] = strdup (opt->arg);
				n_in++;
				break;

			/* Processes program-specific parameters */

			case 'N':
				Ctrl->N.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_in != 2, "Syntax error: Must specify both input and output file names\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdreformat_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdreformat (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, mode, type[2];

	char fname[2][BUFSIZ];

	struct GMT_GRID *Grid = NULL;
	struct GRDREFORMAT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdreformat_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdreformat_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdreformat", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf", "", options))) Return (error);
	Ctrl = (struct GRDREFORMAT_CTRL *) New_grdreformat_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdreformat_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdreformat main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);
	mode = (Ctrl->N.active) ? GMT_GRID_NO_HEADER : 0;
	GMT_err_fail (GMT, GMT_grd_get_format (GMT, Ctrl->IO.file[0], Grid->header, TRUE), Ctrl->IO.file[0]);
	type[0] = Grid->header->type;
	strcpy (fname[0], Grid->header->name);
	GMT_err_fail (GMT, GMT_grd_get_format (GMT, Ctrl->IO.file[1], Grid->header, FALSE), Ctrl->IO.file[1]);
	type[1] = Grid->header->type;
	strcpy (fname[1], Grid->header->name);

	if (type[1] == GMT_GRD_IS_GOLDEN7) {	/* Golden Surfer format 7 (double) is read-only */
		GMT_report (GMT, GMT_MSG_FATAL, "Grid format sd (Golden Software Surfer format 7 (double)) is read-only!\n");
		Return (EXIT_FAILURE);
	}
#ifdef USE_GDAL
	if (type[1] == GMT_GRD_IS_GDAL) {	/* GDAL format is read-only */
		GMT_report (GMT, GMT_MSG_FATAL, "Grid format gd (GDAL) is read-only!\n");
		Return (EXIT_FAILURE);
	}
#endif	
	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) {
		if (Ctrl->IO.file[0][0] == '=') strcpy (fname[0], "<stdin>");
		if (Ctrl->IO.file[1][0] == '=') strcpy (fname[1], "<stdout>");
		GMT_report (GMT, GMT_MSG_NORMAL, "Translating file %s (format = %ld) to file %s (format = %ld)\n", fname[0], type[0], fname[1], type[1]);
		if (mode && GMT_grdformats[type[1]][0] != 'c' && GMT_grdformats[type[1]][0] != 'n') GMT_report (GMT, GMT_MSG_FATAL, "No grd header will be written\n");
	}

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->IO.file[0]), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */

	if (GMT->common.R.active) {	/* Specified a subset */
		GMT_LONG global = FALSE;
		global = GMT_360_RANGE (Grid->header->wesn[XLO], Grid->header->wesn[XHI]);
		if (!global && (GMT->common.R.wesn[XLO] < Grid->header->wesn[XLO] || GMT->common.R.wesn[XHI] > Grid->header->wesn[XHI])) error++;
		if (GMT->common.R.wesn[YLO] < Grid->header->wesn[YLO] || GMT->common.R.wesn[YHI] > Grid->header->wesn[YHI]) error++;
		if (error) {
			GMT_report (GMT, GMT_MSG_FATAL, "Subset exceeds data domain!\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT->common.R.wesn, GMT_GRID_DATA, (void **)&(Ctrl->IO.file[0]), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get subset */
	}
	else
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(Ctrl->IO.file[0]), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get all */

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	Grid->header->type = type[1];

	GMT_grd_init (GMT, Grid->header, options, TRUE);

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, mode, (void **)&Ctrl->IO.file[1], (void *)Grid);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	Return (GMT_OK);
}
