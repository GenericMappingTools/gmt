/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grd2xyz.c reads a grid file and prints out the x,y,z values to
 * standard output.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grd2xyz"
#define THIS_MODULE_MODERN_NAME	"grd2xyz"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert grid to data table"
#define THIS_MODULE_KEYS	"<G{+,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>RVbdfhoqs" GMT_OPT("H")

enum grd2xyz_mode {
	GRD2XYZ_COL = 0,
	GRD2XYZ_ROW = 1,
	GRD2XYZ_X = 2,
	GRD2XYZ_Y = 3
};

struct GRD2XYZ_CTRL {
	struct GRD2XYZ_C {	/* -C[f|i] */
		bool active;
		unsigned int mode;
	} C;
	struct GRD2XYZ_E {	/* -E[f][<nodata>] */
		bool active;
		bool floating;
		double nodata;
	} E;
	struct GRD2XYZ_L {	/* -Lc|r|x|y<value> */
		bool active;
		unsigned int mode;
		int item;
		double value;
	} L;
	struct GRD2XYZ_W {	/* -W[a|<weight>][+u<unit>] */
		bool active;
		bool area;
		char unit;
		double weight;
	} W;
	struct GMT_PARSE_Z_IO Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2XYZ_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRD2XYZ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->E.nodata = -9999.0;
	C->W.weight = 1.0;
	C->W.unit = 'k';	/* km^2 for geo if not set */
	C->Z.type = 'a';
	C->Z.format[0] = 'T';	C->Z.format[1] = 'L';

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2XYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s [-C[f|i]] [-Lc|r|x|y<value>] [%s] [%s] [-W[a[+u<unit>]|<weight>]] "
		"[-Z[<flags>]] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_INGRID, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_d_OPT, GMT_f_OPT, GMT_ho_OPT,
		GMT_o_OPT, GMT_qo_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of one or more grid files");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C[f|i]");
	GMT_Usage (API, -2, "Write row,col instead of x,y. Append f to start at 1, else 0 [Default]. "
		"Use -Ci to write grid index instead of (x,y).");
	GMT_Usage (API, 1, "\n-Lc|r|x|y<value>");
	GMT_Usage (API, -2, "Limit output to a single vector. Specify which one:");
	GMT_Usage (API, 3, "c: Append a column number (0 to nx-1)");
	GMT_Usage (API, 3, "r: Append a row number (0 to ny-1)");
	GMT_Usage (API, 3, "x: Append a column coordinate (xmin to xmax)");
	GMT_Usage (API, 3, "y: Append a row coordinate (ymin to ymax)");
	GMT_Usage (API, -2, "Note: Selections outside the grid will result in no output.");
	GMT_Option (API, "R,V");
	GMT_Usage (API, 1, "\n-W[a[+u<unit>]|<weight>]");
	GMT_Usage (API, -2, "Write xyzw using supplied <weight> (or 1 if not given) [Default is xyz]. "
		"Select -Wa to compute weights equal to the node areas.  If a geographic grid "
		"you may append +u<unit> from %s to set area unit [k].", GMT_LEN_UNITS_DISPLAY);
	GMT_Usage (API, 1, "\n-Z[<flags>]");
	GMT_Usage (API, -2, "Set exact specification of resulting 1-column output z-table. "
		"If data should be in row format, state if first row is at T(op) or B(ottom). "
		"Then, append L or R to indicate left or right starting point in row. "
		"If data should be in column format, state if first columns is L(left) or R(ight). "
		"Then, append T or B to indicate starting point in column. "
		"Now, append any of these three modes if they apply:");
	GMT_Usage (API, 3, "w: Swap the byte-order of each word");
	GMT_Usage (API, 3, "x: Gridline-registered, periodic data in x without repeating column at xmax.");
	GMT_Usage (API, 3, "y: Gridline-registered, periodic data in y without repeating row at ymax.");
	GMT_Usage (API, -2, "Finally, specify one of the following data types (all binary except a):");
	GMT_Usage (API, 3, "a: ASCII (one value per record).");
	GMT_Usage (API, 3, "c: int8_t, signed 1-byte character.");
	GMT_Usage (API, 3, "u: uint8_t, unsigned 1-byte character.");
	GMT_Usage (API, 3, "h: int16_t, signed short 2-byte integer.");
	GMT_Usage (API, 3, "H: uint16_t, unsigned short 2-byte integer.");
	GMT_Usage (API, 3, "i: int32_t, signed 4-byte integer.");
	GMT_Usage (API, 3, "I: uint32_t, unsigned 4-byte integer.");
	GMT_Usage (API, 3, "l: int64_t, signed long (8-byte) integer.");
	GMT_Usage (API, 3, "L: uint64_t, unsigned long (8-byte) integer.");
	GMT_Usage (API, 3, "f: 4-byte floating point single precision.");
	GMT_Usage (API, 3, "d: 8-byte floating point double precision.");
	GMT_Usage (API, -2, "[Default format is scanline orientation in ASCII representation: -ZTLa].");
	GMT_Option (API, "bo,d,f,h,o,qo,s,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRD2XYZ_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	gmt_M_memset (io, 1, struct GMT_Z_IO);

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Write row,col or index instead of x,y */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (opt->arg[0] == 'c') Ctrl->C.mode = 0;
				else if (opt->arg[0] == 'f') Ctrl->C.mode = 1;
				else if (opt->arg[0] == 'i') Ctrl->C.mode = 2;
				break;
			case 'E':	/* Old ESRI option */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (gmt_M_compat_check (GMT, 4)) {
					Ctrl->E.active = true;
					GMT_Report (API, GMT_MSG_COMPAT, "Option -E is deprecated; use grdconvert instead.\n");
					if (opt->arg[0] == 'f') Ctrl->E.floating = true;
					if (opt->arg[Ctrl->E.floating]) Ctrl->E.nodata = atof (&opt->arg[Ctrl->E.floating]);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'L':	/* Select single row or column for output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'c':	Ctrl->L.mode = GRD2XYZ_COL;	Ctrl->L.item = atoi (&opt->arg[1]);	break;
					case 'r':	Ctrl->L.mode = GRD2XYZ_ROW;	Ctrl->L.item = atoi (&opt->arg[1]);	break;
					case 'x':
						Ctrl->L.mode = GRD2XYZ_X;
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
						            gmt_scanf_arg (GMT, &opt->arg[1], gmt_M_type (GMT, GMT_IN, GMT_X), false,
						            &Ctrl->L.value), &opt->arg[1]);
						break;
					case 'y':
						Ctrl->L.mode = GRD2XYZ_Y;
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
						            gmt_scanf_arg (GMT, &opt->arg[1], gmt_M_type (GMT, GMT_IN, GMT_Y), false,
						            &Ctrl->L.value), &opt->arg[1]);
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -L: Append c|r|x|y<value>\n");
						n_errors++;
						break;
				}
				break;
			case 'S':	/* Suppress/no-suppress NaNs on output */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -S is deprecated; use -s common option instead.\n");
					if (gmt_parse_s_option (GMT, opt->arg))
						n_errors++;
					GMT->common.s.active = true;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'N':	/* Nan-value */
				if (gmt_M_compat_check (GMT, 4)) {	/* Honor old -N[i]<value> option */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N is deprecated; use GMT common option -d[i|o]<nodata> instead.\n");
					if (opt->arg[0]) {
						if (opt->arg[0] == 'i')	/* Simulate -di<nodata> */
							n_errors += gmt_parse_d_option (GMT, opt->arg);
						else {	/* Simulate -do<nodata> */
							char arg[GMT_LEN64] = {""};
							sprintf (arg, "o%s", opt->arg);
							n_errors += gmt_parse_d_option (GMT, arg);
						}
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -N: Must specify value or NaN\n");
						n_errors++;
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'W':	/* Add weight on output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg[0] == 'a') {
					char *c = NULL;
					Ctrl->W.area = true;
					if ((c = strstr (opt->arg, "+u"))) {
						Ctrl->W.unit = c[2];
					}
				}
				else if (opt->arg[0])
					Ctrl->W.weight = atof (opt->arg);
				break;
			case 'Z':	/* Control format */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				n_errors += gmt_parse_z_io (GMT, opt->arg, &Ctrl->Z);
					break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->Z.active) n_errors += gmt_init_z_io (GMT, Ctrl->Z.format, Ctrl->Z.repeat, Ctrl->Z.swab, Ctrl->Z.skip, Ctrl->Z.type, io);

	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->L.mode == GRD2XYZ_ROW && Ctrl->L.item < 0, "Option -Lr: Row cannot be negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->L.mode == GRD2XYZ_COL && Ctrl->L.item < 0, "Option -Lc: Column cannot be negative\n");
	n_errors += gmt_M_check_condition (GMT, n_files == 0, "Must specify at least one input file\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1 && Ctrl->E.active, "Option -E can only handle one input file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->E.active, "Option -E is not compatible with -Z\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grd2xyz (void *V_API, int mode, void *args) {
	bool first = true, first_geo = true;
	unsigned int row, col, n_output, w_col = 3, orig_mode;
	int error = 0, write_error = 0;

	uint64_t ij, ij_gmt, n_total = 0, n_suppressed = 0;

	char header[GMT_BUFSIZ];

	double wesn[4], d_value, A_scale = 1.0, A_geo = 0.0, out[4], *x = NULL, *y = NULL;

	struct GMT_GRID *G = NULL, *W = NULL;
	struct GMT_GRID_HEADER_HIDDEN *H = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_Z_IO io;
	struct GMT_OPTION *opt = NULL;
	struct GRD2XYZ_CTRL *Ctrl = NULL;
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
	if ((error = parse (GMT, Ctrl, &io, options)) != 0) Return (error);

	/*---------------------------- This is the grd2xyz main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid(s)\n");

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if (GMT->common.b.active[GMT_OUT]) {
		if (Ctrl->Z.active && !io.binary) {
			GMT_Report (API, GMT_MSG_ERROR, "-Z overrides -bo\n");
			GMT->common.b.active[GMT_OUT] = false;
		}
		if (Ctrl->E.active && gmt_M_compat_check (GMT, 4)) {
			GMT_Report (API, GMT_MSG_ERROR, "-E overrides -bo\n");
			GMT->common.b.active[GMT_OUT] = false;
		}
	}
	else if (io.binary) GMT->common.b.active[GMT_OUT] = true;

	n_output = (Ctrl->Z.active) ? 1 : ((Ctrl->W.active) ? 4 : ((Ctrl->C.mode == 2) ? 2 : 3));
	if (Ctrl->Z.active)
		n_output = 1;
	else {
		n_output = (Ctrl->C.mode == 2) ? 2 : 3;
		if (Ctrl->W.active) n_output++;
		if (Ctrl->C.mode == 2) w_col = 2;
	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, n_output, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers stdout, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	out[w_col] = Ctrl->W.weight;

	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */

		if (opt->option != '<') continue;	/* We are only processing input files here */

		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {
			Return (API->error);
		}

		if (gmt_M_is_subset (GMT, G->header, wesn)) {	/* Subset requested; make sure wesn matches header spacing */
			if ((error = gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "")))
				Return (error);
		}

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, opt->arg, G) == NULL) {
			Return (API->error);	/* Get subset */
		}

		if (Ctrl->L.active) {	/* Check that limits are within range and warn otherwise */
			orig_mode = Ctrl->L.mode;	/* In case we have many grids */
			if (Ctrl->L.mode == GRD2XYZ_Y) {	/* Convert y coordinate to nearest row */
				Ctrl->L.item = gmt_M_grd_y_to_row (GMT, Ctrl->L.value, G->header);
				Ctrl->L.mode = GRD2XYZ_ROW;
			}
			else if (Ctrl->L.mode == GRD2XYZ_X) {	/* Convert x coordinate to nearest column */
				Ctrl->L.item = gmt_M_grd_x_to_col (GMT, Ctrl->L.value, G->header);
				Ctrl->L.mode = GRD2XYZ_COL;
			}
			if (Ctrl->L.mode == GRD2XYZ_COL && (Ctrl->L.item < 0 || Ctrl->L.item >= G->header->n_columns))
				GMT_Report (API, GMT_MSG_WARNING, "Option -L: Your column selection is outside the range of this grid's columns - no output will result\n");
			else if (Ctrl->L.mode == GRD2XYZ_ROW && (Ctrl->L.item < 0 || Ctrl->L.item >= G->header->n_rows))
				GMT_Report (API, GMT_MSG_WARNING, "Option -L: Your column x-value selection is outside the range of this grid's rows - no output will result\n");
			else if (Ctrl->L.mode == GRD2XYZ_ROW)
				n_total += G->header->n_columns;
			else
				n_total += G->header->n_rows;
		}
		else
			n_total += G->header->nm;

		H = gmt_get_H_hidden (G->header);

		if ((error = gmt_M_err_fail (GMT, gmt_set_z_io (GMT, &io, G), opt->arg)))
			Return (error);

		if (Ctrl->Z.active) {	/* Write z-values only to stdout */
			bool previous = GMT->common.b.active[GMT_OUT], rst = false;
			int (*save) (struct GMT_CTRL *, FILE *, uint64_t, double *, char *);
			save = GMT->current.io.output;
			Out = gmt_new_record (GMT, &d_value, NULL);	/* Since we only need to worry about numerics in this module */

			if (Ctrl->Z.swab) GMT_Report (API, GMT_MSG_INFORMATION, "Binary output data will be byte swapped\n");
			GMT->current.io.output = gmt_z_output;		/* Override and use chosen output mode */
			GMT->common.b.active[GMT_OUT] = io.binary;	/* May have to set binary as well */
			GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;	/* Since no x,y involved here */
			if (GMT->current.setting.io_nan_mode && GMT->current.io.io_nan_col[0] == GMT_Z)
				{rst = true; GMT->current.io.io_nan_col[0] = GMT_X;}	/* Since we don't do xy here, only z */
			for (ij = 0; ij < io.n_expected; ij++) {
				ij_gmt = io.get_gmt_ij (&io, G, ij);	/* Get the corresponding grid node */
				d_value = G->data[ij_gmt];
				if ((io.x_missing && io.gmt_i == io.x_period) || (io.y_missing && io.gmt_j == 0)) continue;
				if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (d_value))	/* Grid node is NaN and -d was set, so change to nan-proxy */
					d_value = GMT->common.d.nan_proxy[GMT_OUT];
				else if (gmt_input_is_nan_proxy (GMT, d_value))	/* The inverse: Grid node is nan-proxy and -di was set, so change to NaN */
					d_value = GMT->session.d_NaN;
				write_error = GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				if (write_error == GMT_NOTSET) n_suppressed++;	/* Bad value caught by -s[r] */
			}
			gmt_M_free (GMT, Out);
			GMT->current.io.output = save;			/* Reset pointer */
			GMT->common.b.active[GMT_OUT] = previous;	/* Reset binary */
			if (rst) GMT->current.io.io_nan_col[0] = GMT_Z;	/* Reset to what it was */
		}
		else if (Ctrl->E.active) {	/* ESRI format */
			double slop;
			char *record = NULL, item[GMT_BUFSIZ];
			size_t n_alloc, len, rec_len;
			struct GMT_RECORD *Out = NULL;
			slop = 1.0 - (G->header->inc[GMT_X] / G->header->inc[GMT_Y]);
			if (!gmt_M_is_zero (slop)) {
				GMT_Report (API, GMT_MSG_ERROR, "x_inc must equal y_inc when writing to ESRI format\n");
				Return (GMT_RUNTIME_ERROR);
			}
			n_alloc = G->header->n_columns * 8;	/* Assume we only need 8 bytes per item (but we will allocate more if needed) */
			record = gmt_M_memory (GMT, NULL, G->header->n_columns, char);
			Out = gmt_new_record (GMT, NULL, record);

			sprintf (record, "ncols %d\nnrows %d", G->header->n_columns, G->header->n_rows);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
			if (G->header->registration == GMT_GRID_PIXEL_REG) {	/* Pixel format */
				sprintf (record, "xllcorner ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[XLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
				sprintf (record, "yllcorner ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[YLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
			}
			else {	/* Gridline format */
				sprintf (record, "xllcenter ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[XLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
				sprintf (record, "yllcenter ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[YLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
			}
			sprintf (record, "cellsize ");
			sprintf (item, GMT->current.setting.format_float_out, G->header->inc[GMT_X]);
			strcat  (record, item);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
			sprintf (record, "nodata_value %ld", lrint (Ctrl->E.nodata));
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a text record */
			gmt_M_row_loop (GMT, G, row) {	/* Scanlines, starting in the north (ymax) */
				rec_len = 0;
				gmt_M_col_loop (GMT, G, row, col, ij) {
					if (gmt_M_is_fnan (G->data[ij]))
						sprintf (item, "%ld", lrint (Ctrl->E.nodata));
					else if (Ctrl->E.floating)
						sprintf (item, GMT->current.setting.format_float_out, G->data[ij]);
					else
						sprintf (item, "%ld", lrint ((double)G->data[ij]));
					len = strlen (item);
					if ((rec_len + len + 1) >= n_alloc) {	/* Must get more memory */
						n_alloc <<= 1;
						record = gmt_M_memory (GMT, record, G->header->n_columns, char);
					}
					strcat (record, item);
					rec_len += len;
					if (col < (G->header->n_columns-1)) { strcat (record, " "); rec_len++;}
				}
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write a whole y line */
			}
			gmt_M_free (GMT, record);
			gmt_M_free (GMT, Out);
		}
		else {	/* Regular x,y,z[,w], col,row,z[,w] or index,z[,w] output */
			if (first && GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_STDIO_IF_NONE, 0, options) != GMT_NOERROR) {	/* Establishes data output */
				Return (API->error);
			}

			if (Ctrl->W.area) {	/* calculate area per node */
				W = gmt_duplicate_grid (GMT, G, GMT_DUPLICATE_ALLOC);
				gmt_get_cellarea (GMT, W);
				if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Need geographic area scaling  */
					if (first_geo) {	/* Initialize the distance unit machinery once */
						if (gmt_init_distaz (GMT, Ctrl->W.unit, gmt_M_sph_mode (GMT), GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE) {
							gmt_free_grid (GMT, &W, true);
							Return (GMT_NOT_A_VALID_TYPE);
						}
						first_geo = false;
						if (GMT->current.map.dist[GMT_MAP_DIST].arc)	/* Wants a squared steradian-type area measure, so undo km^2 first by dividing by r^2, then scale to new arc unit^2 */
							A_geo = pow (1000.0 * GMT->current.map.dist[GMT_MAP_DIST].scale / GMT->current.proj.mean_radius, 2.0);
						else
							A_geo = pow (1000.0 * GMT->current.map.dist[GMT_MAP_DIST].scale, 2.0);	/* Get final measure unit for area after converting back to m^2 first */
					}
					A_scale = A_geo;
				}
				else	/* Back to Cartesian grids */
					A_scale = 1.0;
			}

			/* Compute grid node positions once only */

			if (H->var_spacing[GMT_X]) GMT_Report (API, GMT_MSG_WARNING, "Grid %s has non-equidistant x-coordinates (see grd2xyz docs for discussion)\n", opt->arg);
			if (H->var_spacing[GMT_Y]) GMT_Report (API, GMT_MSG_WARNING, "Grid %s has non-equidistant y-coordinates (see grd2xyz docs for discussion)\n", opt->arg);

			x = (G->x) ? G->x : gmt_grd_coord (GMT, G->header, GMT_X);
			y = (G->y) ? G->y : gmt_grd_coord (GMT, G->header, GMT_Y);
			Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
			if (Ctrl->C.active) {	/* Replace x,y with col,row */
				if (Ctrl->C.mode < 2) {
					gmt_M_row_loop  (GMT, G, row) y[row] = row + Ctrl->C.mode;
					gmt_M_col_loop2 (GMT, G, col) x[col] = col + Ctrl->C.mode;
				}
				else
					GMT->current.io.io_nan_col[0] = GMT_Y;	/* Since that is where z will go now */
				gmt_set_cartesian (GMT, GMT_OUT);
			}

			if (GMT->current.setting.io_header[GMT_OUT] && first) {
				if (!G->header->x_units[0]) strcpy (G->header->x_units, "x");
				if (!G->header->y_units[0]) strcpy (G->header->y_units, "y");
				if (Ctrl->C.active) {
					strcpy (G->header->x_units, "col");
					strcpy (G->header->y_units, "row");
				}
				if (!G->header->z_units[0]) strcpy (G->header->z_units, "z");
				if (GMT->current.setting.io_lonlat_toggle[GMT_IN])
					sprintf (header, "%s%s%s%s%s", G->header->y_units, GMT->current.setting.io_col_separator, G->header->x_units, GMT->current.setting.io_col_separator, G->header->z_units);
				else
					sprintf (header, "%s%s%s%s%s", G->header->x_units, GMT->current.setting.io_col_separator, G->header->y_units, GMT->current.setting.io_col_separator, G->header->z_units);
				if (Ctrl->W.active) {
					strcat (header, GMT->current.setting.io_col_separator);
					strcat (header, "weight");
				}
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);	/* Write a header record */
				first = false;
			}

			gmt_M_grd_loop (GMT, G, row, col, ij) {
				if (Ctrl->L.active) {	/* Limit output to one row or column */
					if (Ctrl->L.mode == GRD2XYZ_ROW && row != Ctrl->L.item) continue;	/* Skip these rows */
					if (Ctrl->L.mode == GRD2XYZ_COL && col != Ctrl->L.item) continue;	/* Skip these columns */
					if (Ctrl->L.mode == GRD2XYZ_Y && !doubleAlmostEqualZero (y[row], Ctrl->L.value)) continue;	/* Skip these rows */
					if (Ctrl->L.mode == GRD2XYZ_X && !doubleAlmostEqualZero (x[col], Ctrl->L.value)) continue;	/* Skip these columns */
				}
				if (Ctrl->C.mode == 2) {	/* Write index, z */
					out[GMT_X] = (double)gmt_M_ij0 (G->header, row, col);
					out[GMT_Y] = G->data[ij];
					if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (out[GMT_Y]))	/* Input matched no-data setting, so change to NaN */
						out[GMT_Y] = GMT->common.d.nan_proxy[GMT_OUT];
					else if (gmt_input_is_nan_proxy (GMT, out[GMT_Y]))
						out[GMT_Y] = GMT->session.d_NaN;
				}
				else {
					out[GMT_X] = x[col];	out[GMT_Y] = y[row];	out[GMT_Z] = G->data[ij];
					if (GMT->common.d.active[GMT_OUT] && gmt_M_is_dnan (out[GMT_Z]))	/* Input matched no-data setting, so change to NaN */
						out[GMT_Z] = GMT->common.d.nan_proxy[GMT_OUT];
					else if (gmt_input_is_nan_proxy (GMT, out[GMT_Z]))
						out[GMT_Z] = GMT->session.d_NaN;
				}
				if (Ctrl->W.area) out[w_col] = W->data[ij] * A_scale;	/* Converts area from km^2 to user-selected unit (if active) */
				write_error = GMT_Put_Record (API, GMT_WRITE_DATA, Out);		/* Write this to output */
				if (write_error == GMT_NOTSET) n_suppressed++;	/* Bad value caught by -s[r] */
			}
			if (G->x == NULL) gmt_M_free (GMT, x);
			if (G->y == NULL) gmt_M_free (GMT, y);
			gmt_M_free (GMT, Out);
			if (W) gmt_free_grid (GMT, &W, true);
		}
		if (Ctrl->L.active)	/* Reset */
			Ctrl->L.mode = orig_mode;
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " values extracted\n", n_total - n_suppressed);
	if (n_suppressed) {
		if (GMT->current.setting.io_nan_mode & GMT_IO_NAN_KEEP)
			GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " finite values suppressed\n", n_suppressed);
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64" NaN values suppressed\n", n_suppressed);
	}

	Return (GMT_NOERROR);
}
