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
 * API functions to support the grdedit application.
 *
 * Brief synopsis: grdedit reads an existing grid file and takes command
 * line arguments to redefine some of the grdheader parameters:
 *
 *	x_min/x_max OR x_inc (the other is recomputed)
 *	y_min/y_max OR y_inc (the other is recomputed)
 *	z_scale_factor/z_add_offset
 *	x_units/y_units/z_units
 *	title/command/remark
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

struct GRDEDIT_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct D {	/* -D<xname>/<yname>/<zname>/<scale>/<offset>/<title>/<remark> */
		GMT_LONG active;
		char *information;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct N {	/* N<xyzfile> */
		GMT_LONG active;
		char *file;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
};

void *New_grdedit_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDEDIT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDEDIT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_grdedit_Ctrl (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->D.information) free (C->D.information);	
	if (C->N.file) free (C->N.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdedit_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdedit %s [API] - Modify header or content of a grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdedit <grid> [-A] [%s]\n", GMT_GRDEDIT);
	GMT_message (GMT, "\t[-E] [%s] [-N<table>] [-S] [-T] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n", GMT_bi_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grid> is file to be modified.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Adjust dx/dy to be compatible with the file domain or -R.\n");
	GMT_message (GMT, "\t-D Enter grid information.  Specify '=' to get default value.\n");
	GMT_message (GMT, "\t-E Tranpose the entire grid (this will exchange x and y).\n");
	GMT_message (GMT, "\t-N <table> has new xyz values to replace existing grid nodes.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S For global grids of 360 degree longitude range.\n");
	GMT_message (GMT, "\t   Will rotate entire grid to coincide with new borders in -R.\n");
	GMT_message (GMT, "\t-T Toggle header from grid-line to pixel-registered grid or vice versa.\n");
	GMT_message (GMT, "\t   This shrinks -R by 0.5*{dx,dy} going from pixel to grid-line registration\n");
	GMT_message (GMT, "\t   and expands -R by 0.5*{dx,dy} going from grid-line to pixel registration.\n");
	GMT_explain_options (GMT, "VC3fhi:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdedit_parse (struct GMTAPI_CTRL *C, struct GRDEDIT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdedit and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjsut increments */
				Ctrl->A.active = TRUE;
				break;
			case 'D':	/* Give grid information */
				Ctrl->D.active = TRUE;
				Ctrl->D.information = strdup (opt->arg);
				break;
			case 'E':	/* Transpose grid */
				Ctrl->E.active = TRUE;
				break;
			case 'N':	/* Replace nodes */
				Ctrl->N.active = TRUE;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'S':	/* Rotate global grid */
				Ctrl->S.active = TRUE;
				break;
			case 'T':	/* Toggle registration */
				Ctrl->T.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->A.active, "Syntax error -S option: Incompatible with -A\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && (Ctrl->A.active || Ctrl->D.active || Ctrl->N.active || Ctrl->S.active || 
			Ctrl->T.active), "Syntax error -E option: Incompatible with -A, -D, -N, -S, and -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->T.active, "Syntax error -S option: Incompatible with -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active, "Syntax error -S option: Incompatible with -N\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !GMT->common.R.active, 
					"Syntax error -S option: Must also specify -R\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]), 
					"Syntax error -S option: -R longitudes must span exactly 360 degrees\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	if (Ctrl->N.active) {
		n_errors += GMT_check_condition (GMT, !Ctrl->N.file, "Syntax error -N option: Must specify name of xyz file\n");
		n_errors += GMT_check_binary_io (GMT, 3);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdedit_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdedit (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args) {
	/* High-level function that implements the grdedit task */

	GMT_LONG row, col, error, n_data, k;
	
	double shift_amount = 0.0, *in = NULL;

	char *registration[2] = {"gridline", "pixel"};
	
	struct GRDEDIT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdedit_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdedit_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdedit", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRbf:", "hi" GMT_OPT("H"), options)) Return (API->error);
	Ctrl = New_grdedit_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdedit_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdedit main code ----------------------------*/

	if (!strcmp (Ctrl->In.file,  "=")) {		/* DOES THIS TEST STILL MAKE SENSE? */
		GMT_report (GMT, GMT_MSG_FATAL, "Piping of grid file not supported!\n");
		Return (EXIT_FAILURE);
	}

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if ((G->header->type == GMT_GRD_IS_SF || G->header->type == GMT_GRD_IS_SD) && Ctrl->T.active) {
		GMT_report (GMT, GMT_MSG_FATAL, "Toggling registrations not possible for Surfer grid formats\n");
		GMT_report (GMT, GMT_MSG_FATAL, "(Use grdreformat to convert to GMT default format and work on that file)\n");
		Return (EXIT_FAILURE);
	}
	
	if (Ctrl->S.active && !GMT_grd_is_global (GMT, G->header)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Shift only allowed for global grids\n");
		Return (EXIT_FAILURE);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Editing parameters for file %s\n", Ctrl->In.file);

	/* Decode grd information given, if any */

	if (Ctrl->D.active) {
		double scale_factor, add_offset;
		GMT_report (GMT, GMT_MSG_NORMAL, "Decode and change attributes in file %s\n", Ctrl->In.file);
		scale_factor = G->header->z_scale_factor;
		add_offset = G->header->z_add_offset;
		GMT_decode_grd_h_info (GMT, Ctrl->D.information, G->header);
		if (scale_factor != G->header->z_scale_factor || add_offset != G->header->z_add_offset) {
			G->header->z_min = (G->header->z_min - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
			G->header->z_max = (G->header->z_max - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
		}
	}

	if (Ctrl->S.active) {
		shift_amount = GMT->common.R.wesn[XLO] - G->header->wesn[XLO];
		GMT_report (GMT, GMT_MSG_NORMAL, "Shifting longitudes in file %s by %g degrees\n", Ctrl->In.file, shift_amount);
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		GMT_grd_shift (GMT, G, shift_amount);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->In.file, G) != GMT_OK) {
			Return (API->error);
		}
	}
	else if (Ctrl->N.active) {
		GMT_LONG in_ID = 0;
		GMT_report (GMT, GMT_MSG_NORMAL, "Replacing nodes using xyz values from file %s\n", Ctrl->N.file);

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}

		if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IN, Ctrl->N.file, NULL)) == GMTAPI_NOTSET) {
			GMT_report (GMT, GMT_MSG_FATAL, "Unable to register file %s\n", Ctrl->N.file);
			Return (EXIT_FAILURE);
		}

		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}

		n_data = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
					continue;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			/* Data record to process */

			n_data++;

			if (GMT_y_is_outside (GMT, in[GMT_Y],  G->header->wesn[YLO], G->header->wesn[YHI])) continue;	/* Outside y-range */
			if (GMT_x_is_outside (GMT, &in[GMT_X], G->header->wesn[XLO], G->header->wesn[XHI])) continue;	/* Outside x-range */
			row = GMT_grd_y_to_row (GMT, in[GMT_Y], G->header);
			if (row < 0 || row >= G->header->ny) continue;
			col = GMT_grd_x_to_col (GMT, in[GMT_X], G->header);
			if (col < 0 || col >= G->header->nx) continue;
			k = GMT_IJP (G->header, row, col);
			G->data[k] = (float)in[GMT_Z];
			if (GMT_grd_duplicate_column (GMT, G->header, GMT_IN)) {	/* Make sure longitudes got replicated */
				/* Possibly need to replicate e/w value */
				if (col == 0) {k = GMT_IJP (G->header, row, G->header->nx-1); G->data[k] = (float)in[GMT_Z]; }
				if (col == (G->header->nx-1)) {k = GMT_IJP (G->header, row, 0); G->data[k] = (float)in[GMT_Z]; }
			}
		} while (TRUE);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->In.file, G) != GMT_OK) {
			Return (API->error);
		}
	}
	else if (Ctrl->E.active) {	/* Transpose the matrix and exchange x and y info */
		struct GRD_HEADER *h_tr = NULL;
		GMT_LONG ij, ij_tr;
		float *a_tr = NULL;
		
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->In.file, G)) {	/* Get data */
			Return (API->error);
		}

		h_tr = GMT_memory (GMT, NULL, 1, struct GRD_HEADER);
		GMT_memcpy (h_tr, G->header, 1, struct GRD_HEADER);	/* First make a copy of header */
		h_tr->nx = G->header->ny;	/* Then exchange x and y values */
		h_tr->wesn[XLO] = G->header->wesn[YLO];
		h_tr->wesn[XHI] = G->header->wesn[YHI];
		h_tr->inc[GMT_X] = G->header->inc[GMT_Y];
		strcpy (h_tr->x_units, G->header->y_units);
		h_tr->ny = G->header->nx;
		h_tr->wesn[YLO] = G->header->wesn[XLO];
		h_tr->wesn[YHI] = G->header->wesn[XHI];
		h_tr->inc[GMT_Y] = G->header->inc[GMT_X];
		strcpy (h_tr->y_units, G->header->x_units);

		/* Now transpose the matrix */

		a_tr = GMT_memory (GMT, NULL, G->header->size, float);
		GMT_grd_loop (GMT, G, row, col, ij) {
			ij_tr = GMT_IJP (h_tr, col, row);
			a_tr[ij_tr] = G->data[ij];
		}
		GMT_free (GMT, G->data);
		G->data = a_tr;
		GMT_memcpy (G->header, h_tr, 1, struct GRD_HEADER);	/* Update to the new header */
		GMT_free (GMT, h_tr);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->In.file, G) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Change the domain boundaries */
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		if (Ctrl->T.active) {	/* Grid-line <---> Pixel toggling of the header */
			GMT_change_grdreg (GMT, G->header, 1 - G->header->registration);
			GMT_report (GMT, GMT_MSG_NORMAL, "Toggled registration mode in file %s from %s to %s\n", 
				Ctrl->In.file, registration[1-G->header->registration], registration[G->header->registration]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Reset region in file %s to %g/%g/%g/%g\n", 
				Ctrl->In.file, G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
		}
		if (GMT->common.R.active) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Reset region in file %s to %g/%g/%g/%g\n", 
				Ctrl->In.file, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			GMT_memcpy (G->header->wesn, GMT->common.R.wesn, 4, double);
			Ctrl->A.active = TRUE;	/* Must ensure -R -I compatibility */
		}
		if (Ctrl->A.active) {
			G->header->inc[GMT_X] = GMT_get_inc (GMT, G->header->wesn[XLO], G->header->wesn[XHI], G->header->nx, G->header->registration);
			G->header->inc[GMT_Y] = GMT_get_inc (GMT, G->header->wesn[YLO], G->header->wesn[YHI], G->header->ny, G->header->registration);
			GMT_report (GMT, GMT_MSG_NORMAL, "Reset grid-spacing in file %s to %g/%g\n",
				Ctrl->In.file, G->header->inc[GMT_X], G->header->inc[GMT_Y]);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file, G) != GMT_OK) {
			Return (API->error);
		}
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "File %s updated.\n", Ctrl->In.file);

	Return (EXIT_SUCCESS);
}
