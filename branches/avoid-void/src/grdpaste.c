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
 * Brief synopsis: grdpaste.c reads two grid files and writes a new file with
 * the first two pasted together along their common edge.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

struct GRDPASTE_CTRL {
	struct In {
		GMT_LONG active;
		char *file[2];
	} In;
	struct G {	/* -G<output_grdfile> */
		GMT_LONG active;
		char *file;
	} G;
};

void *New_grdpaste_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPASTE_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDPASTE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
			
	return (C);
}

void Free_grdpaste_Ctrl (struct GMT_CTRL *GMT, struct GRDPASTE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	if (C->In.file[0]) free (C->In.file[0]);	
	if (C->In.file[1]) free (C->In.file[1]);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdpaste_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdpaste %s [API] - Join two grids along their common edge\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdpaste <grid1> <grid2> -G<outgrid> [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\twhere grids <grid1> and <grid2> are to be combined into <outgrid>.\n");
	GMT_message (GMT, "\t<grid1> and <grid2> must have same dx,dy and one edge in common.\n");
	GMT_message (GMT, "\tIf in doubt, run grdinfo first and check your files.\n");
	GMT_message (GMT, "\tUse grdpaste and/or grdsample to adjust files as necessary.\n");
	GMT_message (GMT, "\t-G Specify file name for output grid file.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "Vf.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdpaste_parse (struct GMTAPI_CTRL *C, struct GRDPASTE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdpaste and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_in = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_in == 0)
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else if (n_in == 1)
					Ctrl->In.file[n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Only two files may be pasted\n");
				}
				break;

			/* Processes program-specific parameters */

 			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file[0] || !Ctrl->In.file[1], "Syntax error: Must specify two input files\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdpaste_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdpaste (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, way, one_or_zero;

	char format[GMT_BUFSIZ];

	double x_noise, y_noise, west, east;

	struct GMT_GRID *A = NULL, *B = NULL, *C = NULL;
	struct GRDPASTE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdpaste_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdpaste_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdpaste", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vf", "", options))) Return (error);
	Ctrl = New_grdpaste_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdpaste_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdpaste main code ----------------------------*/

	/* Try to find a common side to join on  */

	C = GMT_create_grid (GMT);
	GMT_grd_init (GMT, C->header, options, FALSE);
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
	if ((A = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file[0], NULL)) == NULL) Return (API->error);	/* Get header only */
	if ((B = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file[1], NULL)) == NULL) Return (API->error);	/* Get header only */

	if (A->header->registration != B->header->registration) error++;
	if ((A->header->z_scale_factor != B->header->z_scale_factor) || (A->header->z_add_offset != B->header->z_add_offset)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Scale/offset not compatible!\n");
		Return (EXIT_FAILURE);
	}

	if (fabs (A->header->inc[GMT_X] - B->header->inc[GMT_X]) < 1.0e-6 && fabs (A->header->inc[GMT_Y] - B->header->inc[GMT_Y]) < 1.0e-6) {
		C->header->inc[GMT_X] = A->header->inc[GMT_X];
		C->header->inc[GMT_Y] = A->header->inc[GMT_Y];
	}
	else {
		GMT_report (GMT, GMT_MSG_FATAL, "Grid intervals do not match!\n");
		Return (EXIT_FAILURE);
	}

	one_or_zero = 1 - A->header->registration;
	x_noise = GMT_SMALL * C->header->inc[GMT_X];
	y_noise = GMT_SMALL * C->header->inc[GMT_Y];

	west = B->header->wesn[XLO];	east = B->header->wesn[XHI];	/* Get copies */
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Must be careful in determining a match */
		west -= 360.0;	east -= 360.0;	/* Wind to left first */
		while ((A->header->wesn[XLO] - west) > x_noise) {	/* Wind back to match grid A if possible */
			west += 360.0;
			east += 360.0;
		}
		B->header->wesn[XLO] = west;	B->header->wesn[XHI] = east;	/* Possibly update lon range */
	}

	GMT_memcpy (C->header->wesn, A->header->wesn, 4, double);	/* Output region is set as the same as A... */
	if (fabs (A->header->wesn[XLO] - B->header->wesn[XLO]) < x_noise && fabs (A->header->wesn[XHI] - B->header->wesn[XHI]) < x_noise) {

		C->header->nx = A->header->nx;

		if (fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < y_noise) {			/* B is exactly on top of A */
			way = 1;
			C->header->ny = A->header->ny + B->header->ny - (int)one_or_zero;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if (fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < y_noise) {	/* A is exactly on top of B */
			way = 2;
			C->header->ny = A->header->ny + B->header->ny - (int)one_or_zero;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else if ( !one_or_zero && (fabs (A->header->wesn[YHI] - B->header->wesn[YLO]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* B is on top of A but their pixel reg limits overlap by one cell */
			way = 11;
			C->header->ny = A->header->ny + B->header->ny - 1;
			C->header->wesn[YHI] = B->header->wesn[YHI];			/* ...but not for north */
		}
		else if ( !one_or_zero && (fabs (A->header->wesn[YLO] - B->header->wesn[YHI]) < (C->header->inc[GMT_Y] + y_noise)) ) {
			/* A is on top of B but their pixel reg limits overlap by one cell */
			way = 22;
			C->header->ny = A->header->ny + B->header->ny - 1;
			C->header->wesn[YLO] = B->header->wesn[YLO];			/* ...but not for south */
		}
		else {
			GMT_report (GMT, GMT_MSG_FATAL, "Grids do not share a common edge!\n");
			Return (EXIT_FAILURE);
		}
	}
	else if (fabs (A->header->wesn[YLO] - B->header->wesn[YLO]) < y_noise && fabs (A->header->wesn[YHI] - B->header->wesn[YHI]) < y_noise) {

		C->header->ny = A->header->ny;

		if (fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < x_noise) {			/* A is on the right of B */
			way = 3;
			C->header->nx = A->header->nx + B->header->nx - (int)one_or_zero;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if (fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < x_noise) {	/* A is on the left of B */
			way = 4;
			C->header->nx = A->header->nx + B->header->nx - (int)one_or_zero;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else if ( !one_or_zero && (fabs (A->header->wesn[XLO] - B->header->wesn[XHI]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on right of B but their pixel reg limits overlap by one cell */
			way = 33;
			C->header->nx = A->header->nx + B->header->nx - 1;
			C->header->wesn[XLO] = B->header->wesn[XLO];			/* ...but not for west */
		}
		else if ( !one_or_zero && (fabs (A->header->wesn[XHI] - B->header->wesn[XLO]) < (C->header->inc[GMT_X] + x_noise)) ) {
			/* A is on left of B but their pixel reg limits overlap by one cell */
			way = 44;
			C->header->nx = A->header->nx + B->header->nx - 1;
			C->header->wesn[XHI] = B->header->wesn[XHI];			/* ...but not for east */
		}
		else {
			GMT_report (GMT, GMT_MSG_FATAL, "Grids do not share a common edge!\n");
			Return (EXIT_FAILURE);
		}
	}
	else {
		GMT_report (GMT, GMT_MSG_FATAL, "Grids do not share a common edge!\n");
		Return (EXIT_FAILURE);
	}
	if (GMT_is_geographic (GMT, GMT_IN) && C->header->wesn[XHI] > 360.0) {	/* Must be careful in determining a match */
		C->header->wesn[XLO] -= 360.0;
		C->header->wesn[XHI] -= 360.0;
	}

	/* Now we can do it  */

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "\t%s\t%s\t%s\t%s\t%s\t%s\t%%ld\t%%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, "File spec:\tW E S N dx dy nx ny:\n");
		GMT_report (GMT, GMT_MSG_NORMAL, format, Ctrl->In.file[0], A->header->wesn[XLO], A->header->wesn[XHI], A->header->wesn[YLO], A->header->wesn[YHI], A->header->inc[GMT_X], A->header->inc[GMT_Y], A->header->nx, A->header->ny);
		GMT_report (GMT, GMT_MSG_NORMAL, format, Ctrl->In.file[1], B->header->wesn[XLO], B->header->wesn[XHI], B->header->wesn[YLO], B->header->wesn[YHI], B->header->inc[GMT_X], B->header->inc[GMT_Y], B->header->nx, B->header->ny);
		GMT_report (GMT, GMT_MSG_NORMAL, format, Ctrl->G.file, C->header->wesn[XLO], C->header->wesn[XHI], C->header->wesn[YLO], C->header->wesn[YHI], C->header->inc[GMT_X], C->header->inc[GMT_Y], C->header->nx, C->header->ny);
	}

	C->header->registration = A->header->registration;
	GMT_set_grddim (GMT, C->header);
	C->data = GMT_memory (GMT, NULL, C->header->size, float);
	A->data = B->data = C->data;	/* A and B share the same final matrix declared for C */
	A->header->size = B->header->size = C->header->size;	/* Set A & B's nm,size to the same as C */
	A->header->nm = B->header->nm = C->header->nm;
	A->header->no_BC = B->header->no_BC = TRUE;	/* We must disable the BC machinery */

	switch (way) {      /* How A and B are positioned relative to each other */
		case 1:         /* B is on top of A */
		case 11:        /* B is on top of A but their pixel reg limits overlap by one cell */
			GMT->current.io.pad[YHI] = B->header->ny - one_or_zero + 2;
			if (way == 11) GMT->current.io.pad[YHI] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[0], A) == NULL) Return (API->error);	/* Get data from A */
			GMT->current.io.pad[YHI] = 2;	GMT->current.io.pad[YLO] = A->header->ny - one_or_zero + 2;
			if (way == 11) GMT->current.io.pad[YLO] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[1], B) == NULL) Return (API->error);	/* Get data from B */
			break;
		case 2:         /* A is on top of B */
		case 22:        /* A is on top of B but their pixel reg limits overlap by one cell */
			GMT->current.io.pad[YLO] = B->header->ny - one_or_zero + 2;
			if (way == 22) GMT->current.io.pad[YLO] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[0], A) == NULL) Return (API->error);	/* Get data from A */
			GMT->current.io.pad[YLO] = 2;	GMT->current.io.pad[YHI] = A->header->ny - one_or_zero + 2;
			if (way == 22) GMT->current.io.pad[YHI] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[1], B) == NULL) Return (API->error);	/* Get data from B */
			break;
		case 3:         /* A is on the right of B */
		case 33:        /* A is on right of B but their pixel reg limits overlap by one cell */
			GMT->current.io.pad[XLO] = B->header->nx - one_or_zero + 2;
			if (way == 33) GMT->current.io.pad[XLO] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[0], A) == NULL) Return (API->error);	/* Get data from A */
			GMT->current.io.pad[XLO] = 2;	GMT->current.io.pad[XHI] = A->header->nx - one_or_zero + 2;
			if (way == 33) GMT->current.io.pad[XHI] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[1], B) == NULL) Return (API->error);	/* Get data from B */
			break;
		case 4:         /* A is on the left of B */
		case 44:        /* A is on left of B but their pixel reg limits overlap by one cell */
			GMT->current.io.pad[XHI] = B->header->nx - one_or_zero + 2;
			if (way == 44) GMT->current.io.pad[XHI] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[0], A) == NULL) Return (API->error);	/* Get data from A */
			GMT->current.io.pad[XHI] = 2;	GMT->current.io.pad[XLO] = A->header->nx - one_or_zero + 2;
			if (way == 44) GMT->current.io.pad[XLO] -= 1;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file[1], B) == NULL) Return (API->error);	/* Get data from B */
			break;
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	GMT_set_pad (GMT, 2);	/* Restore to GMT Defaults */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, C);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
	A->data = B->data = NULL;	/* Since these were never actually allocated */

	Return (GMT_OK);
}
