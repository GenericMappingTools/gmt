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

#define THIS_MODULE_NAME	"grdedit"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Modify header or content of a grid"
#define THIS_MODULE_KEYS	"<GI,NDi,GGO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:JRVbdfhi" GMT_OPT("H")

struct GRDEDIT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct D {	/* -D<xname>/<yname>/<zname>/<scale>/<offset>/<invalid>/<title>/<remark> */
		bool active;
		char *information;
	} D;
	struct E {	/* -E[a|h|l|r|t|v] */
		bool active;
		char mode;	/* l rotate 90 degrees left (CCW), t = transpose, r = rotate 90 degrees right (CW) */
				/* a rotate around (180), h flip grid horizontally (FLIPLR), v flip grid vertically (FLIPUD) */
	} E;
	struct G {
		bool active;
		char *file;
	} G;
	struct N {	/* N<xyzfile> */
		bool active;
		char *file;
	} N;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
};

void *New_grdedit_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDEDIT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDEDIT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_grdedit_Ctrl (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->D.information) free (C->D.information);
	if (C->G.file) free (C->G.file);
	if (C->N.file) free (C->N.file);
	GMT_free (GMT, C);
}

int GMT_grdedit_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdedit <grid> [-A] [%s]\n", GMT_GRDEDIT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E[a|h|l|r|t|v]] [-G<outgrid>] [-N<table>] [%s] [-S] [-T] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n", GMT_bi_OPT, GMT_di_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is file to be modified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust dx/dy to be compatible with the file domain or -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Enter grid information; leave field blank to get default value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Tranpose or rotate the entire grid (this may exchange x and y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  a Rotate grid around 180 degrees.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  h Flip grid left-to-right (as grdmath FLIPLR).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  l Rotate grid 90 degrees left (counter-clockwise).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  r Rotate grid 90 degrees right (clockwise).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  t Transpose grid [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  v Flip grid top-to-bottom (as grdmath FLIPUD).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Tranpose the entire grid (this will exchange x and y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify new output grid file [Default updates given grid file].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N <table> has new xyz values to replace existing grid nodes.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S For global grids of 360 degree longitude range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Will rotate entire grid to coincide with new borders in -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggle header from grid-line to pixel-registered grid or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This shrinks -R by 0.5*{dx,dy} going from pixel to grid-line registration\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and expands  -R by 0.5*{dx,dy} going from grid-line to pixel registration.\n");
	GMT_Option (API, "J,V,bi3,di,f,h,i,:,.");

	return (EXIT_FAILURE);
}

int GMT_grdedit_parse (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdedit and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust increments */
				Ctrl->A.active = true;
				break;
			case 'D':	/* Give grid information */
				Ctrl->D.active = true;
				Ctrl->D.information = strdup (opt->arg);
				break;
			case 'E':	/* Transpose or rotate grid */
				Ctrl->E.active = true;
				if (opt->arg[0] == '\0')	/* Default transpose */
					Ctrl->E.mode = 't';
				else if (strchr ("ahlrtv", opt->arg[0]))
					Ctrl->E.mode = opt->arg[0];
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax Error -E: Unrecognized modifier %c\n", opt->arg[0]);
				}
				break;
			case 'G':	/* Separate output grid file */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'N':	/* Replace nodes */
				if ((Ctrl->N.active = GMT_check_filearg (GMT, 'N', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->N.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'S':	/* Rotate global grid */
				Ctrl->S.active = true;
				break;
			case 'T':	/* Toggle registration */
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file, "Syntax error -G option: Must specify an output grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->A.active,
	                                 "Syntax error -S option: Incompatible with -A\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active &&
	                                 (Ctrl->A.active || Ctrl->D.active || Ctrl->N.active || Ctrl->S.active || Ctrl->T.active),
	                                 "Syntax error -E option: Incompatible with -A, -D, -N, -S, and -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->T.active,
	                                 "Syntax error -S option: Incompatible with -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active,
	                                 "Syntax error -S option: Incompatible with -N\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !GMT->common.R.active,
	                                 "Syntax error -S option: Must also specify -R\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]),
	                                 "Syntax error -S option: -R longitudes must span exactly 360 degrees\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	if (Ctrl->N.active) {
		n_errors += GMT_check_binary_io (GMT, 3);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdedit_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdedit (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdedit task */

	unsigned int row, col;
	int error;

	uint64_t ij, n_data, n_use;

	double shift_amount = 0.0, *in = NULL;

	char *registration[2] = {"gridline", "pixel"}, *out_file = NULL;

	struct GRDEDIT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdedit_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdedit_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdedit_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdedit_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdedit_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdedit main code ----------------------------*/

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if ((G->header->type == GMT_GRID_IS_SF || G->header->type == GMT_GRID_IS_SD) && Ctrl->T.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Toggling registrations not possible for Surfer grid formats\n");
		GMT_Report (API, GMT_MSG_NORMAL, "(Use grdconvert to convert to GMT default format and work on that file)\n");
		Return (EXIT_FAILURE);
	}

	if (Ctrl->S.active && !GMT_grd_is_global (GMT, G->header)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shift only allowed for global grids\n");
		Return (EXIT_FAILURE);
	}

	out_file = (Ctrl->G.active) ? Ctrl->G.file : Ctrl->In.file;	/* Where to write the modified grid */

	GMT_Report (API, GMT_MSG_VERBOSE, "Editing parameters for grid %s:\n", Ctrl->In.file);

	/* Decode grd information given, if any */

	if (GMT->common.J.active)		/* Convert the GMT -J<...> into a proj4 string and save it in the header */
#ifdef HAVE_GDAL
		G->header->ProjRefPROJ4 = GMT_export2proj4(GMT);
#else
		GMT_Report(API, GMT_MSG_NORMAL, "-J option to set grid's referencing system is only availabe for builds that link with GDAL\n");
#endif

	if (Ctrl->D.active) {
		double scale_factor, add_offset;
		float nan_value;
		GMT_Report (API, GMT_MSG_VERBOSE, "Decode and change attributes in file %s\n", Ctrl->In.file);
		scale_factor = G->header->z_scale_factor;
		add_offset = G->header->z_add_offset;
		nan_value = G->header->nan_value;
		GMT_decode_grd_h_info (GMT, Ctrl->D.information, G->header);
		if (nan_value != G->header->nan_value) {
			/* Must read data */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL)
				Return (API->error);
			/* Recalculate z_min/z_max */
			GMT_grd_zminmax (GMT, G->header, G->data);
		}
		if (scale_factor != G->header->z_scale_factor || add_offset != G->header->z_add_offset) {
			G->header->z_min = (G->header->z_min - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
			G->header->z_max = (G->header->z_max - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
		}
	}

	if (Ctrl->S.active) {
		shift_amount = GMT->common.R.wesn[XLO] - G->header->wesn[XLO];
		GMT_Report (API, GMT_MSG_VERBOSE, "Shifting longitudes in file %s by %g degrees\n", Ctrl->In.file, shift_amount);
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		GMT_grd_shift (GMT, G, shift_amount);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, out_file, G) != GMT_OK) {
			Return (API->error);
		}
	}
	else if (Ctrl->N.active) {
		int in_ID;
		GMT_Report (API, GMT_MSG_VERBOSE, "Replacing nodes using xyz values from file %s\n", Ctrl->N.file);

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IN, NULL, Ctrl->N.file)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to register file %s\n", Ctrl->N.file);
			Return (EXIT_FAILURE);
		}
		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_EXISTING, 0, options) != GMT_OK) {
			Return (API->error);	/* Establishes data input */
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}

		n_data = n_use = 0;
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
			if (GMT_row_col_out_of_bounds (GMT, in, G->header, &row, &col)) continue;			/* Outside grid node range */

			ij = GMT_IJP (G->header, row, col);
			G->data[ij] = (float)in[GMT_Z];
			n_use++;
			if (GMT_grd_duplicate_column (GMT, G->header, GMT_IN)) {	/* Make sure longitudes got replicated */
				/* Possibly need to replicate e/w value */
				if (col == 0) {ij = GMT_IJP (G->header, row, G->header->nx-1); G->data[ij] = (float)in[GMT_Z]; n_use++; }
				else if (col == (G->header->nx-1)) {ij = GMT_IJP (G->header, row, 0); G->data[ij] = (float)in[GMT_Z]; n_use++; }
			}
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, out_file, G) != GMT_OK) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Read %" PRIu64 " new data points, updated %" PRIu64 ".\n", n_data, n_use);
	}
	else if (Ctrl->E.active) {	/* Transpose, flip, or rotate the matrix and possibly exchange x and y info */
		struct GMT_GRID_HEADER *h_tr = NULL;
		uint64_t ij, ij_tr;
		float *a_tr = NULL;

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}

		switch (Ctrl->E.mode) {
			case 'a': /* Rotate grid around 180 degrees */
				GMT_Report (API, GMT_MSG_VERBOSE, "Rotate grid around 180 degrees\n");
				break;
			case 'h': /* Flip grid horizontally */
				GMT_Report (API, GMT_MSG_VERBOSE, "Flip grid horizontally (FLIPLR)\n");
				break;
			case 'l': /* Rotate grid 90 CW */
				GMT_Report (API, GMT_MSG_VERBOSE, "Rotate grid 90 degrees left (Counter-Clockwise)\n");
				break;
			case 't': 	/* Transpose grid */
				GMT_Report (API, GMT_MSG_VERBOSE, "Transpose grid\n");
				break;
			case 'r': /* Rotate grid 90 CCW */
				GMT_Report (API, GMT_MSG_VERBOSE, "Rotate grid 90 degrees right (Clockwise)\n");
				break;
			case 'v': /* Flip grid vertically */
				GMT_Report (API, GMT_MSG_VERBOSE, "Flip grid vertically (FLIPUD)\n");
				break;
		}

		h_tr = GMT_memory (GMT, NULL, 1, struct GMT_GRID_HEADER);
		GMT_memcpy (h_tr, G->header, 1, struct GMT_GRID_HEADER);	/* First make a copy of header */
		if (strchr ("ltr", Ctrl->E.mode)) {	/* These operators interchange x and y */
			h_tr->wesn[XLO] = G->header->wesn[YLO];
			h_tr->wesn[XHI] = G->header->wesn[YHI];
			h_tr->inc[GMT_X] = G->header->inc[GMT_Y];
			strncpy (h_tr->x_units, G->header->y_units, GMT_GRID_UNIT_LEN80);
			h_tr->wesn[YLO] = G->header->wesn[XLO];
			h_tr->wesn[YHI] = G->header->wesn[XHI];
			h_tr->inc[GMT_Y] = G->header->inc[GMT_X];
			strncpy (h_tr->y_units, G->header->x_units, GMT_GRID_UNIT_LEN80);
			GMT_set_grddim (GMT, h_tr);	/* Recompute nx, ny, mx, size, etc */
		}

		/* Now transpose the matrix */

		a_tr = GMT_memory (GMT, NULL, G->header->size, float);
		GMT_grd_loop (GMT, G, row, col, ij) {
			switch (Ctrl->E.mode) {
				case 'a': /* Rotate grid around 180 degrees */
					ij_tr = GMT_IJP (h_tr, G->header->ny-1-row, G->header->nx-1-col);
					break;
				case 'h': /* Flip horizontally (FLIPLR) */
					ij_tr = GMT_IJP (h_tr, row, G->header->nx-1-col);
					break;
				case 'l': /* Rotate 90 CCW */
					ij_tr = GMT_IJP (h_tr, G->header->nx-1-col, row);
					break;
				case 'r': /* Rotate 90 CW */
					ij_tr = GMT_IJP (h_tr, col, G->header->ny-1-row);
					break;
				case 't': 	/* Transpose */
					ij_tr = GMT_IJP (h_tr, col, row);
					break;
				case 'v': /* Flip vertically (FLIPUD) */
					ij_tr = GMT_IJP (h_tr, G->header->ny-1-row, col);
					break;
			}
			a_tr[ij_tr] = G->data[ij];
		}
		GMT_free_aligned (GMT, G->data);	/* Eliminate original grid and hook on the modified grid instead */
		G->data = a_tr;
		GMT_memcpy (G->header, h_tr, 1, struct GMT_GRID_HEADER);	/* Update to the new header */
		GMT_free (GMT, h_tr);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, out_file, G) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Change the domain boundaries */
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		if (Ctrl->T.active) {	/* Grid-line <---> Pixel toggling of the header */
			GMT_change_grdreg (GMT, G->header, 1 - G->header->registration);
			GMT_Report (API, GMT_MSG_VERBOSE, "Toggled registration mode in file %s from %s to %s\n",
				Ctrl->In.file, registration[1-G->header->registration], registration[G->header->registration]);
			GMT_Report (API, GMT_MSG_VERBOSE, "Reset region in file %s to %g/%g/%g/%g\n",
				Ctrl->In.file, G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
		}
		if (GMT->common.R.active) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Reset region in file %s to %g/%g/%g/%g\n",
				Ctrl->In.file, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			GMT_memcpy (G->header->wesn, GMT->common.R.wesn, 4, double);
			Ctrl->A.active = true;	/* Must ensure -R -I compatibility */
		}
		if (Ctrl->A.active) {
			G->header->inc[GMT_X] = GMT_get_inc (GMT, G->header->wesn[XLO], G->header->wesn[XHI], G->header->nx, G->header->registration);
			G->header->inc[GMT_Y] = GMT_get_inc (GMT, G->header->wesn[YLO], G->header->wesn[YHI], G->header->ny, G->header->registration);
			GMT_Report (API, GMT_MSG_VERBOSE, "Reset grid-spacing in file %s to %g/%g\n",
				Ctrl->In.file, G->header->inc[GMT_X], G->header->inc[GMT_Y]);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, out_file, G) != GMT_OK) {
			Return (API->error);
		}
	}

	if (Ctrl->G.active)
		GMT_Report (API, GMT_MSG_VERBOSE, "Modified grid written to file %s.\n", Ctrl->G.file);
	else
		GMT_Report (API, GMT_MSG_VERBOSE, "File %s updated.\n", Ctrl->In.file);

	Return (EXIT_SUCCESS);
}
