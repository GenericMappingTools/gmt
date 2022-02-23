/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#define THIS_MODULE_OPTIONS "-:JRVbdefhiw" GMT_OPT("H")

struct GRDEDIT_CTRL {
	struct GRDEDIT_In {
		bool active;
		char *file;
	} In;
	struct GRDEDIT_A {	/* -A */
		bool active;
	} A;
	struct GRDEDIT_C {	/* -C[b|c|n|p] */
		bool active;
		unsigned int mode;	/* see gmt_constants.h */
	} C;
	struct GRDEDIT_D {	/* -D[+x<xname>][+yyname>][+z<zname>][+s<scale>][+ooffset>][+n<invalid>][+t<title>][+r<remark>] */
		bool active;
		char *information;
	} D;
	struct GRDEDIT_E {	/* -E[a|h|l|r|t|v] */
		bool active;
		char mode;	/* l rotate 90 degrees left (CCW), t = transpose, r = rotate 90 degrees right (CW) */
				/* a rotate around (180), h flip grid horizontally (FLIPLR), v flip grid vertically (FLIPUD) */
	} E;
	struct GRDEDIT_G {
		bool active;
		char *file;
	} G;
	struct GRDEDIT_L {	/* -L[+n|p] */
		bool active;
		int mode;
	} L;
	struct GRDEDIT_N {	/* N<xyzfile> */
		bool active;
		char *file;
	} N;
	struct GRDEDIT_S {	/* -S */
		bool active;
	} S;
	struct GRDEDIT_T {	/* -T */
		bool active;
	} T;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDEDIT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDEDIT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.information);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s [-A] [-Cb|c|n|p] [%s] [-E[a|e|h|l|r|t|v]] [-G%s] [%s] [-L[+n|p]] "
		"[-N<table>] [%s] [-S] [-T] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_INGRID, GMT_GRDEDIT2D,
		GMT_OUTGRID, GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of grid to be modified");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A Adjust dx/dy to be compatible with the file domain (or new -R).");
	GMT_Usage (API, 1, "\n-Cb|c|n|p");
	GMT_Usage (API, -2, "Control how the current and previous command histories should be handled, via directives:");
	GMT_Usage (API, 3, "b: Append the current command's history to the previous history.");
	GMT_Usage (API, 3, "c: Only save the current command's history.");
	GMT_Usage (API, 3, "n: Save no history at all [Default].");
	GMT_Usage (API, 3, "p: Only preserve the previous history.");
	gmt_grd_info_syntax (API->GMT, 'D');
	GMT_Usage (API, 1, "\n-E[a|e|h|l|r|t|v]");
	GMT_Usage (API, -2, "Transform the entire grid (this may exchange x and y). Append operation:");
	GMT_Usage (API, 3, "a: Flip grid horizontally and vertically (h + v).");
	GMT_Usage (API, 3, "e: Exchange x(lon) and y(lat).");
	GMT_Usage (API, 3, "h: Flip grid left-to-right (as grdmath FLIPLR).");
	GMT_Usage (API, 3, "l: Rotate grid 90 degrees left (counter-clockwise).");
	GMT_Usage (API, 3, "r: Rotate grid 90 degrees right (clockwise).");
	GMT_Usage (API, 3, "t: Transpose grid [Default].");
	GMT_Usage (API, 3, "v: Flip grid top-to-bottom (as grdmath FLIPUD).");
	gmt_outgrid_syntax (API, 'G', "Specify new output grid file [Default updates given grid file]");
	GMT_Option (API, "J");
	GMT_Usage (API, 1, "\n-L[+n|p]");
	GMT_Usage (API, -2, "Shift the grid\'s longitude range (geographic grids only):");
	GMT_Usage (API, 3, "+n Adjust <west>/<east> so <east> <= 0.");
	GMT_Usage (API, 3, "+p Adjust <west>/<east> so <west> >= 0.");
	GMT_Usage (API, -2, "Default adjusts <west>/<east> so <west> >= -180 or <east> <= +180.");
	GMT_Usage (API, 1, "\n-N<table>");
	GMT_Usage (API, -2, "Give <table> with new xyz values to replace the corresponding, existing grid nodes.");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S For global grids of 360 degree longitude range: "
		"Will rotate entire grid in longitude to coincide with new borders in -R.");
	GMT_Usage (API, 1, "\n-T Toggle header from grid-line to pixel-registered grid or vice versa. "
		"Note: This shrinks the grid region by 0.5*{dx,dy} going from pixel to grid-line registration "
		"and expands it by 0.5*{dx,dy} going from grid-line to pixel registration. "
		"No grid values will be changed (it is a non-destructive change to the grid).");
	GMT_Option (API, "V,bi3,di,e,f,h,i,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDEDIT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdedit and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust increments */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				break;
			case 'C':	/* Control history output -Cb|c|n|p */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				switch (opt->arg[0]) {
					case 'b': Ctrl->C.mode = GMT_GRDHISTORY_BOTH;	break;
					case 'c': Ctrl->C.mode = GMT_GRDHISTORY_NEW;	break;
					case 'p': Ctrl->C.mode = GMT_GRDHISTORY_OLD;	break;
					case 'n': case '\0': Ctrl->C.mode = GMT_GRDHISTORY_NONE;	break;	/* Default */
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Unrecognized directive %s\n", opt->arg);
						n_errors++;
						break;
				}
				break;
			case 'D':	/* Give grid information */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				Ctrl->D.information = strdup (opt->arg);
				break;
			case 'E':	/* Transpose or rotate grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				if (opt->arg[0] == '\0')	/* Default transpose */
					Ctrl->E.mode = 't';
				else if (strchr ("aehlrtv", opt->arg[0]))
					Ctrl->E.mode = opt->arg[0];
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -E: Unrecognized modifier %c\n", opt->arg[0]);
				}
				break;
			case 'G':	/* Separate output grid file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'L':	/* Rotate w/e */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				if (strstr (opt->arg, "+n")) Ctrl->L.mode = -1;
				else if (strstr (opt->arg, "+p")) Ctrl->L.mode = +1;
				else if (opt->arg[0])
					n_errors++;
				break;
			case 'N':	/* Replace nodes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				if (opt->arg[0]) Ctrl->N.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->N.file))) n_errors++;
				break;
			case 'S':	/* Rotate global grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				break;
			case 'T':	/* Toggle registration */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file, "Option -G: Must specify an output grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->A.active,
	                                 "Option -S: Incompatible with -A\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active &&
	                                 (Ctrl->A.active || Ctrl->D.active || Ctrl->N.active || Ctrl->S.active || Ctrl->T.active),
	                                 "Option -E: Incompatible with -A, -D, -N, -S, and -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->T.active,
	                                 "Option -S: Incompatible with -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->N.active,
	                                 "Option -S: Incompatible with -N\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !GMT->common.R.active[RSET],
	                                 "Option -S: Must also specify -R\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]),
	                                 "Option -S: -R longitudes must span exactly 360 degrees\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify a single grid file\n");
	if (Ctrl->N.active) {
		n_errors += gmt_check_binary_io (GMT, 3);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdedit (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdedit task */
	bool grid_was_read = false, do_J = false;

	openmp_int row, col;
	int error;

	uint64_t ij, n_data, n_use;

	double shift_amount = 0.0;

	char *registration[2] = {"gridline", "pixel"}, *out_file = NULL, *projstring = NULL, command[GMT_BUFSIZ] = {""};

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
		free(projstring);
		Return (API->error);
	}
	grid_was_read = Ctrl->G.active;
	HH = gmt_get_H_hidden (G->header);
	if (gmt_M_is_geographic (GMT, GMT_IN)) gmt_set_geographic (GMT, GMT_OUT);	/* Out same as in */

	if ((G->header->type == GMT_GRID_IS_SF || G->header->type == GMT_GRID_IS_SD) && Ctrl->T.active) {
		GMT_Report (API, GMT_MSG_ERROR, "Toggling registrations not possible for Surfer grid formats\n");
		GMT_Report (API, GMT_MSG_ERROR, "(Use grdconvert to convert to GMT default format and work on that file)\n");
		free(projstring);
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->S.active && !gmt_grd_is_global (GMT, G->header)) {
		GMT_Report (API, GMT_MSG_ERROR, "Shift only allowed for global grids\n");
		free(projstring);
		Return (GMT_RUNTIME_ERROR);
	}

	out_file = (Ctrl->G.active) ? Ctrl->G.file : Ctrl->In.file;	/* Where to write the modified grid */

	GMT_Report (API, GMT_MSG_INFORMATION, "Editing parameters for grid %s:\n", out_file);

	/* Decode grd information given, if any */

	if (do_J)	/* Save proj4 string in the header */
		G->header->ProjRefPROJ4 = projstring;

	if (Ctrl->D.active) {
		bool was_NaN, is_NaN;
		double scale_factor, add_offset;
		gmt_grdfloat nan_value;
		GMT_Report (API, GMT_MSG_INFORMATION, "Decode and change attributes in file %s\n", out_file);
		scale_factor = G->header->z_scale_factor;
		add_offset = G->header->z_add_offset;
		nan_value = G->header->nan_value;
		if (gmt_decode_grd_h_info (GMT, Ctrl->D.information, G->header))
			Return (GMT_PARSE_ERROR);

		was_NaN = gmt_M_is_dnan (nan_value);
		is_NaN  = gmt_M_is_dnan (G->header->nan_value);
		if (was_NaN != is_NaN || (was_NaN == false && is_NaN == false && nan_value != G->header->nan_value)) {
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

	gmt_change_grid_history (API, Ctrl->C.mode, G->header, command);	/* Set grid history per -C mode */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_COMMAND, command, G))
		Return (API->error);

	if (Ctrl->S.active) {
		shift_amount = GMT->common.R.wesn[XLO] - G->header->wesn[XLO];
		GMT_Report (API, GMT_MSG_INFORMATION, "Shifting longitudes in file %s by %g degrees\n", out_file, shift_amount);
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
		GMT_Report (API, GMT_MSG_INFORMATION, "Replacing nodes using xyz values from file %s\n", Ctrl->N.file);

		if (!grid_was_read && GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, G) == NULL) {	/* Get data */
			Return (API->error);
		}
		/* Must register Ctrl->N.file first since we are going to read rec-by-rec from all available sources */
		if ((in_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IN, NULL, Ctrl->N.file)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to register file %s\n", Ctrl->N.file);
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
			if (In->data == NULL) {
				gmt_quit_bad_record (API, In);
				Return (API->error);
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
				else if (col == (openmp_int)(G->header->n_columns-1)) {ij = gmt_M_ijp (G->header, row, 0); G->data[ij] = (gmt_grdfloat)in[GMT_Z]; n_use++; }
			}
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_file, G) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Read %" PRIu64 " new data points, updated %" PRIu64 ".\n", n_data, n_use);
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
				GMT_Report (API, GMT_MSG_INFORMATION, "Flip grid horizontally and vertically\n");
				break;
			case 'e': /* Exchange lon and lat */
				GMT_Report (API, GMT_MSG_INFORMATION, "Exchange x|longitude and y|latitude\n");
				break;
			case 'h': /* Flip grid horizontally */
				GMT_Report (API, GMT_MSG_INFORMATION, "Flip grid horizontally (FLIPLR)\n");
				break;
			case 'l': /* Rotate grid 90 CW */
				GMT_Report (API, GMT_MSG_INFORMATION, "Rotate grid 90 degrees left (Counter-Clockwise)\n");
				break;
			case 't': 	/* Transpose grid */
				GMT_Report (API, GMT_MSG_INFORMATION, "Transpose grid\n");
				break;
			case 'r': /* Rotate grid 90 CCW */
				GMT_Report (API, GMT_MSG_INFORMATION, "Rotate grid 90 degrees right (Clockwise)\n");
				break;
			case 'v': /* Flip grid vertically */
				GMT_Report (API, GMT_MSG_INFORMATION, "Flip grid vertically (FLIPUD)\n");
				break;
		}

		h_tr = gmt_get_header (GMT);
		gmt_copy_gridheader (GMT, h_tr, G->header);	/* First make a copy of header */
		if (strchr ("eltr", Ctrl->E.mode)) {	/* These operators interchange x and y */
			h_tr->wesn[XLO] = G->header->wesn[YLO];
			h_tr->wesn[XHI] = G->header->wesn[YHI];
			h_tr->inc[GMT_X] = G->header->inc[GMT_Y];
			strncpy (h_tr->x_units, G->header->y_units, GMT_GRID_UNIT_LEN80);
			h_tr->wesn[YLO] = G->header->wesn[XLO];
			h_tr->wesn[YHI] = G->header->wesn[XHI];
			h_tr->inc[GMT_Y] = G->header->inc[GMT_X];
			strncpy (h_tr->y_units, G->header->x_units, GMT_GRID_UNIT_LEN80);
			gmt_set_grddim (GMT, h_tr);	/* Recompute n_columns, n_rows, mx, size, etc */
			gmt_M_doublep_swap (G->x, G->y);
		}

		/* Now transpose the matrix */

		a_tr = gmt_M_memory (GMT, NULL, G->header->size, gmt_grdfloat);
		gmt_M_grd_loop (GMT, G, row, col, ij) {
			switch (Ctrl->E.mode) {
				case 'a': /* Rotate grid around 180 degrees */
					ij_tr = gmt_M_ijp (h_tr, G->header->n_rows-1-row, G->header->n_columns-1-col);
					break;
				case 'e': /* Exchange coordinates */
					ij_tr = gmt_M_ijp (h_tr, h_tr->n_rows-1-col, h_tr->n_columns-1-row);
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
			GMT_Report (API, GMT_MSG_INFORMATION, "Change region in file %s from %g/%g/%g/%g to %g/%g/%g/%g\n", out_file,
				G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI],
				wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		}
		else {
			GMT_Report (API, GMT_MSG_INFORMATION, "Region in file remains unchanged: %g/%g/%g/%g\n", out_file,
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
			GMT_Report (API, GMT_MSG_INFORMATION, "Toggled registration mode in file %s from %s to %s\n",
				out_file, registration[1-G->header->registration], registration[G->header->registration]);
			GMT_Report (API, GMT_MSG_INFORMATION, "Reset region in file %s to %g/%g/%g/%g\n",
				out_file, G->header->wesn[XLO], G->header->wesn[XHI], G->header->wesn[YLO], G->header->wesn[YHI]);
		}
		if (GMT->common.R.active[RSET]) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Reset region in file %s to %g/%g/%g/%g\n",
				out_file, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			gmt_M_memcpy (G->header->wesn, GMT->common.R.wesn, 4, double);
			Ctrl->A.active = true;	/* Must ensure -R -I compatibility */
		}
		if (Ctrl->A.active) {
			G->header->inc[GMT_X] = gmt_M_get_inc (GMT, G->header->wesn[XLO], G->header->wesn[XHI], G->header->n_columns, G->header->registration);
			G->header->inc[GMT_Y] = gmt_M_get_inc (GMT, G->header->wesn[YLO], G->header->wesn[YHI], G->header->n_rows, G->header->registration);
			GMT_Report (API, GMT_MSG_INFORMATION, "Reset grid-spacing in file %s to %g/%g\n",
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

	GMT_Report (API, GMT_MSG_INFORMATION, Ctrl->G.active ? "Modified grid written to file %s.\n" : "File %s updated.\n", out_file);

	Return (GMT_NOERROR);
}
