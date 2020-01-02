/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
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
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdedit"
#define THIS_MODULE_MODERN_NAME	"grdedit"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Modify header or content of a grid"
#define THIS_MODULE_KEYS	"<G{,ND(,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:JRVbdefhi" GMT_OPT("H")

struct GRDEDIT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D[+x<xname>][+yyname>][+z<zname>][+s<scale>][+ooffset>][+n<invalid>][+t<title>][+r<remark>] */
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
	struct L {	/* -L[+n|+p] */
		bool active;
		int mode;
	} L;
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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDEDIT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDEDIT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.information);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid> [-A] [-C] [%s]\n", name, GMT_GRDEDIT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E[a|h|l|r|t|v]] [-G<outgrid>] [%s] [-L[+n|+p]] [-N<table>] [%s] [-S] [-T]\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is file to be modified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust dx/dy to be compatible with the file domain or -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Remove the command history from the header.\n");
	gmt_grd_info_syntax (API->GMT, 'D');
	GMT_Message (API, GMT_TIME_NONE, "\t-E Transpose or rotate the entire grid (this may exchange x and y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  a Rotate grid around 180 degrees.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  h Flip grid left-to-right (as grdmath FLIPLR).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  l Rotate grid 90 degrees left (counter-clockwise).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  r Rotate grid 90 degrees right (clockwise).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  t Transpose grid [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  v Flip grid top-to-bottom (as grdmath FLIPUD).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify new output grid file [Default updates given grid file].\n");
	GMT_Option (API, "J");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Shift the grid\'s longitude range (geographic grids only):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -L+n Adjust <west>/<east> so <east> <= 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -L+p Adjust <west>/<east> so <west> >= 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default adjusts <west>/<east> so <west> >= -180 or <east> <= +180\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N <table> has new xyz values to replace existing grid nodes.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S For global grids of 360 degree longitude range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Will rotate entire grid to coincide with new borders in -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggle header from grid-line to pixel-registered grid or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This shrinks -R by 0.5*{dx,dy} going from pixel to grid-line registration\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and expands  -R by 0.5*{dx,dy} going from grid-line to pixel registration.\n");
	GMT_Option (API, "V,bi3,di,e,f,h,i.:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *Ctrl, struct GMT_OPTION *options) {

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
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust increments */
				Ctrl->A.active = true;
				break;
			case 'C':	/* Clear history */
				Ctrl->C.active = true;
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
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* Rotate w/e */
				Ctrl->L.active = true;
				if (strstr (opt->arg, "+n")) Ctrl->L.mode = -1;
				else if (strstr (opt->arg, "+p")) Ctrl->L.mode = +1;
				else if (opt->arg[0])
					n_errors++;
				break;
			case 'N':	/* Replace nodes */
				if ((Ctrl->N.active = gmt_check_filearg (GMT, 'N', opt->arg, GMT_IN, GMT_IS_DATASET)))
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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file, "Syntax error -G option: Must specify an output grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->A.active,
	                                 "Syntax error -S option: Incompatible with -A\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active &&
	                                 (Ctrl->A.active || Ctrl->D.active || Ctrl->N.active || Ctrl->S.active || Ctrl->T.active),
	                                 "Syntax error -E option: Incompatible with -A, -D, -N, -S, and -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->T.active,
	                                 "Syntax error -S option: Incompatible with -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->N.active,
	                                 "Syntax error -S option: Incompatible with -N\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !GMT->common.R.active[RSET],
	                                 "Syntax error -S option: Must also specify -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]),
	                                 "Syntax error -S option: -R longitudes must span exactly 360 degrees\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	if (Ctrl->N.active) {
		n_errors += gmt_check_binary_io (GMT, 3);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdedit (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdedit task */
	bool grid_was_read = false, do_J = false;
	
	unsigned int row, col;
	int error;

	uint64_t ij, n_data, n_use;

	double shift_amount = 0.0;

	char *registration[2] = {"gridline", "pixel"}, *out_file = NULL, *projstring = NULL;

	struct GRDEDIT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdedit main code ----------------------------*/

	if ((do_J = GMT->common.J.active)) {	/* Only gave -J to set the proj4 flag, so grid is already projected (i.e., Cartesian) */
		EXTERN_MSC int gmt_parse_R_option (struct GMT_CTRL *GMT, char *item);
		projstring = gmt_export2proj4 (GMT);	/* Convert the GMT -J<...> into a proj4 string */
 		if (strstr (projstring, "+proj=longlat") || strstr (projstring, "+proj=latlong")) {	/* These means we have a geographic grid */
			gmt_set_geographic (GMT, GMT_IN);	/* Force Geographic */
			gmt_set_geographic (GMT, GMT_OUT);
		}
		else {
			gmt_set_cartesian (GMT, GMT_IN);	/* Force Cartesian since processing -J changed that */
			gmt_set_cartesian (GMT, GMT_OUT);
			gmt_parse_R_option (GMT, GMT->common.R.string);	/* Since parsing under -J would have imposed checks. */
		}
		GMT->current.proj.projection = GMT->current.proj.projection_GMT = GMT_NO_PROJ;
		GMT->common.J.active = false;	/* Leave no trace of this, except in the history... */
	}

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, Ctrl->G.active ? GMT_CONTAINER_AND_DATA : GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	grid_was_read = Ctrl->G.active;
	HH = gmt_get_H_hidden (G->header);
	
	if ((G->header->type == GMT_GRID_IS_SF || G->header->type == GMT_GRID_IS_SD) && Ctrl->T.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Toggling registrations not possible for Surfer grid formats\n");
		GMT_Report (API, GMT_MSG_NORMAL, "(Use grdconvert to convert to GMT default format and work on that file)\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->S.active && !gmt_grd_is_global (GMT, G->header)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shift only allowed for global grids\n");
		Return (GMT_RUNTIME_ERROR);
	}

	out_file = (Ctrl->G.active) ? Ctrl->G.file : Ctrl->In.file;	/* Where to write the modified grid */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Editing parameters for grid %s:\n", out_file);

	/* Decode grd information given, if any */

	if (do_J)	/* Save proj4 string in the header */
		G->header->ProjRefPROJ4 = projstring;

	if (Ctrl->D.active) {
		double scale_factor, add_offset;
		gmt_grdfloat nan_value;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Decode and change attributes in file %s\n", out_file);
		scale_factor = G->header->z_scale_factor;
		add_offset = G->header->z_add_offset;
		nan_value = G->header->nan_value;
		if (gmt_decode_grd_h_info (GMT, Ctrl->D.information, G->header))
			Return (GMT_PARSE_ERROR);
		
		if (nan_value != G->header->nan_value) {
			/* Must read data */
			if (!grid_was_read && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL)
				Return (API->error);
			grid_was_read = true;
			/* Recalculate z_min/z_max */
			gmt_grd_zminmax (GMT, G->header, G->data);
		}
		if (scale_factor != G->header->z_scale_factor || add_offset != G->header->z_add_offset) {
			G->header->z_min = (G->header->z_min - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
			G->header->z_max = (G->header->z_max - add_offset) / scale_factor * G->header->z_scale_factor + G->header->z_add_offset;
		}
	}

	if (Ctrl->C.active) {	/* Wipe history */
		gmt_M_memset (G->header->command, GMT_GRID_COMMAND_LEN320, char);
	}
	
	if (Ctrl->S.active) {
		shift_amount = GMT->common.R.wesn[XLO] - G->header->wesn[XLO];
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Shifting longitudes in file %s by %g degrees\n", out_file, shift_amount);
		if (!grid_was_read && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		gmt_grd_shift (GMT, G, shift_amount);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else if (Ctrl->N.active) {
		int in_ID;
		struct GMT_RECORD *In = NULL;
		double *in = NULL;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Replacing nodes using xyz values from file %s\n", Ctrl->N.file);

		if (!grid_was_read && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		/* Must register Ctrl->N.file first since we are going to read rec-by-rec from all available sources */
		if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IN, NULL, Ctrl->N.file)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to register file %s\n", Ctrl->N.file);
			Return (GMT_RUNTIME_ERROR);
		}
		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_EXISTING, 0, options) != GMT_NOERROR) {
			Return (API->error);	/* Establishes data input */
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		GMT_Set_Columns (API, GMT_IN, 3, GMT_COL_FIX_NO_TEXT);

		n_data = n_use = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				continue;	/* Go back and read the next record */
			}
			in = In->data;	/* Only need to process numerical part here */

			/* Data record to process */

			n_data++;

			if (gmt_M_y_is_outside (GMT, in[GMT_Y],  G->header->wesn[YLO], G->header->wesn[YHI])) continue;	/* Outside y-range */
			if (gmt_x_is_outside (GMT, &in[GMT_X], G->header->wesn[XLO], G->header->wesn[XHI])) continue;	/* Outside x-range */
			if (gmt_row_col_out_of_bounds (GMT, in, G->header, &row, &col)) continue;			/* Outside grid node range */

			ij = gmt_M_ijp (G->header, row, col);
			G->data[ij] = (gmt_grdfloat)in[GMT_Z];
			n_use++;
			if (gmt_M_grd_duplicate_column (GMT, G->header, GMT_IN)) {	/* Make sure longitudes got replicated */
				/* Possibly need to replicate e/w value */
				if (col == 0) {ij = gmt_M_ijp (G->header, row, G->header->n_columns-1); G->data[ij] = (gmt_grdfloat)in[GMT_Z]; n_use++; }
				else if (col == (G->header->n_columns-1)) {ij = gmt_M_ijp (G->header, row, 0); G->data[ij] = (gmt_grdfloat)in[GMT_Z]; n_use++; }
			}
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %" PRIu64 " new data points, updated %" PRIu64 ".\n", n_data, n_use);
	}
	else if (Ctrl->E.active) {	/* Transpose, flip, or rotate the matrix and possibly exchange x and y info */
		struct GMT_GRID_HEADER *h_tr = NULL;
		uint64_t ij, ij_tr = 0;
		gmt_grdfloat *a_tr = NULL, *save_grid_pointer = NULL;

		if (!grid_was_read && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}

		switch (Ctrl->E.mode) {
			case 'a': /* Rotate grid around 180 degrees */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Rotate grid around 180 degrees\n");
				break;
			case 'h': /* Flip grid horizontally */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Flip grid horizontally (FLIPLR)\n");
				break;
			case 'l': /* Rotate grid 90 CW */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Rotate grid 90 degrees left (Counter-Clockwise)\n");
				break;
			case 't': 	/* Transpose grid */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Transpose grid\n");
				break;
			case 'r': /* Rotate grid 90 CCW */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Rotate grid 90 degrees right (Clockwise)\n");
				break;
			case 'v': /* Flip grid vertically */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Flip grid vertically (FLIPUD)\n");
				break;
		}

		h_tr = gmt_get_header (GMT);
		gmt_copy_gridheader (GMT, h_tr, G->header);	/* First make a copy of header */
		if (strchr ("ltr", Ctrl->E.mode)) {	/* These operators interchange x and y */
			h_tr->wesn[XLO] = G->header->wesn[YLO];
			h_tr->wesn[XHI] = G->header->wesn[YHI];
			h_tr->inc[GMT_X] = G->header->inc[GMT_Y];
			strncpy (h_tr->x_units, G->header->y_units, GMT_GRID_UNIT_LEN80);
			h_tr->wesn[YLO] = G->header->wesn[XLO];
			h_tr->wesn[YHI] = G->header->wesn[XHI];
			h_tr->inc[GMT_Y] = G->header->inc[GMT_X];
			strncpy (h_tr->y_units, G->header->x_units, GMT_GRID_UNIT_LEN80);
			gmt_set_grddim (GMT, h_tr);	/* Recompute n_columns, n_rows, mx, size, etc */
		}

		/* Now transpose the matrix */

		a_tr = gmt_M_memory (GMT, NULL, G->header->size, gmt_grdfloat);
		gmt_M_grd_loop (GMT, G, row, col, ij) {
			switch (Ctrl->E.mode) {
				case 'a': /* Rotate grid around 180 degrees */
					ij_tr = gmt_M_ijp (h_tr, G->header->n_rows-1-row, G->header->n_columns-1-col);
					break;
				case 'h': /* Flip horizontally (FLIPLR) */
					ij_tr = gmt_M_ijp (h_tr, row, G->header->n_columns-1-col);
					break;
				case 'l': /* Rotate 90 CCW */
					ij_tr = gmt_M_ijp (h_tr, G->header->n_columns-1-col, row);
					break;
				case 'r': /* Rotate 90 CW */
					ij_tr = gmt_M_ijp (h_tr, col, G->header->n_rows-1-row);
					break;
				case 't': 	/* Transpose */
					ij_tr = gmt_M_ijp (h_tr, col, row);
					break;
				case 'v': /* Flip vertically (FLIPUD) */
					ij_tr = gmt_M_ijp (h_tr, G->header->n_rows-1-row, col);
					break;
			}
			a_tr[ij_tr] = G->data[ij];
		}
		save_grid_pointer = G->data;	/* Save original grid pointer and hook on the modified grid instead */
		G->data = a_tr;
		gmt_copy_gridheader (GMT, G->header, h_tr);	/* Update to the new header */
		gmt_M_free (GMT, h_tr->hidden);
		gmt_M_free (GMT, h_tr);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
		G->data = save_grid_pointer;
		gmt_M_free (GMT, a_tr);
	}
	else if (Ctrl->L.active) {	/* Wrap the longitude boundaries */
		double wesn[4];
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		gmt_M_memcpy (wesn, G->header->wesn, 4, double);	/* Copy */
		if (Ctrl->L.mode == -1) {
			while (G->header->wesn[XHI] > 0.0) G->header->wesn[XLO] -= 360.0, G->header->wesn[XHI] -= 360.0;
		}
		else if (Ctrl->L.mode == +1) {
			while (G->header->wesn[XLO] < 0.0) G->header->wesn[XLO] += 360.0, G->header->wesn[XHI] += 360.0;
		}
		else {	/* Aim for a reasonable w/e range */
			if (G->header->wesn[XHI] < -180.0) G->header->wesn[XLO] += 360.0, G->header->wesn[XHI] += 360.0;
			if (G->header->wesn[XLO] < -180.0 && (G->header->wesn[XHI] + 360.0) <= 180.0) G->header->wesn[XLO] += 360.0, G->header->wesn[XHI] += 360.0;
			if (G->header->wesn[XLO] >  360.0) G->header->wesn[XLO] -= 360.0, G->header->wesn[XHI] -= 360.0;
			if (G->header->wesn[XHI] >  180.0 && (G->header->wesn[XLO] - 360.0) >= -180.0) G->header->wesn[XLO] -= 360.0, G->header->wesn[XHI] -= 360.0;
		}
		if (doubleAlmostEqual (G->header->wesn[XLO], wesn[XLO])) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Change region in file %s from %g/%g/%g/%g to %g/%g/%g/%g\n", out_file,
				G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI],
				wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		}
		else {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Region in file remains unchanged: %g/%g/%g/%g\n", out_file,
				G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, Ctrl->G.active ? GMT_CONTAINER_AND_DATA : GMT_CONTAINER_ONLY, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Change the domain boundaries */
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		if (Ctrl->T.active) {	/* Grid-line <---> Pixel toggling of the header */
			gmt_change_grdreg (GMT, G->header, 1 - G->header->registration);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Toggled registration mode in file %s from %s to %s\n",
				out_file, registration[1-G->header->registration], registration[G->header->registration]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reset region in file %s to %g/%g/%g/%g\n",
				out_file, G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
		}
		if (GMT->common.R.active[RSET]) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reset region in file %s to %g/%g/%g/%g\n",
				out_file, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			gmt_M_memcpy (G->header->wesn, GMT->common.R.wesn, 4, double);
			Ctrl->A.active = true;	/* Must ensure -R -I compatibility */
		}
		if (Ctrl->A.active) {
			G->header->inc[GMT_X] = gmt_M_get_inc (GMT, G->header->wesn[XLO], G->header->wesn[XHI], G->header->n_columns, G->header->registration);
			G->header->inc[GMT_Y] = gmt_M_get_inc (GMT, G->header->wesn[YLO], G->header->wesn[YHI], G->header->n_rows, G->header->registration);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reset grid-spacing in file %s to %g/%g\n",
				out_file, G->header->inc[GMT_X], G->header->inc[GMT_Y]);
		}
		if (gmt_M_is_geographic (GMT, GMT_IN) && gmt_M_is_cartesian (GMT, GMT_OUT)) {	/* Force a switch from geographic to Cartesian */
			HH->grdtype = GMT_GRID_CARTESIAN;
			strcpy (G->header->x_units, "x_units");
			strcpy (G->header->y_units, "y_units");
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, Ctrl->G.active ? GMT_CONTAINER_AND_DATA : GMT_CONTAINER_ONLY, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, Ctrl->G.active ? "Modified grid written to file %s.\n" : "File %s updated.\n", out_file);

	Return (GMT_NOERROR);
}
