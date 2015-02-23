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
 * Brief synopsis: grd2xyz.c reads a grid file and prints out the x,y,z values to
 * standard output.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grd2xyz"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert grid file to data table"
#define THIS_MODULE_KEYS	"<GI,>DO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>RVbdfhos" GMT_OPT("H")

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
	struct GRD2XYZ_W {	/* -W[<weight>] */
		bool active;
		double weight;
	} W;
	struct GMT_PARSE_Z_IO Z;
};

void *New_grd2xyz_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2XYZ_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRD2XYZ_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->E.nodata = -9999.0;
	C->W.weight = 1.0;
	C->Z.type = 'a';
	C->Z.format[0] = 'T';	C->Z.format[1] = 'L';
		
	return (C);
}

void Free_grd2xyz_Ctrl (struct GMT_CTRL *GMT, struct GRD2XYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_grd2xyz_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grd2xyz <grid> [-C[f]] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W[<weight>]] [-Z[<flags>]] [%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s] > xyzfile\n",
		GMT_bo_OPT, GMT_d_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\t<grid> is one or more grid files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Write row, col instead of x,y.  Append f to start at 1, else 0 [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ci to write grid index instead of (x,y).\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write xyzw using supplied weight (or 1 if not given) [Default is xyz].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set exact specification of resulting 1-column output z-table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If data is in row format, state if first row is at T(op) or B(ottom).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Then, append L or R to indicate starting point in row.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If data is in column format, state if first columns is L(left) or R(ight).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Then, append T or B to indicate starting point in column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append x if gridline-registered, periodic data in x without repeating column at xmax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append y if gridline-registered, periodic data in y without repeating row at ymax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify one of the following data types (all binary except a):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a  Ascii.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c  int8_t, signed 1-byte character.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     u  uint8_t, unsigned 1-byte character.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h  int16_t, signed short 2-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     H  uint16_t, unsigned short 2-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     i  int32_t, signed 4-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     I  uint32_t, unsigned 4-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l  int64_t, signed long (8-byte) integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     L  uint64_t, unsigned long (8-byte) integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f  4-byte floating point single precision.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     d  8-byte floating point double precision.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default format is scanline orientation in ascii representation: -ZTLa].\n");
	GMT_Option (API, "bo,d,f,h,o,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_grd2xyz_parse (struct GMT_CTRL *GMT, struct GRD2XYZ_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	EXTERN_MSC unsigned int gmt_parse_d_option (struct GMT_CTRL *GMT, char *arg);

	GMT_memset (io, 1, struct GMT_Z_IO);
	
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input files */
				n_files++;
				break;
				
			/* Processes program-specific parameters */

			case 'C':	/* Write row,col or index instead of x,y */
				Ctrl->C.active = true;
				if (opt->arg[0] == 'c') Ctrl->C.mode = 0;
				else if (opt->arg[0] == 'f') Ctrl->C.mode = 1;
				else if (opt->arg[0] == 'i') Ctrl->C.mode = 2;
				break;
			case 'E':	/* Old ESRI option */
				if (GMT_compat_check (GMT, 4)) {
					Ctrl->E.active = true;
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -E is deprecated; use grdconvert instead.\n");
					if (opt->arg[0] == 'f') Ctrl->E.floating = true;
					if (opt->arg[Ctrl->E.floating]) Ctrl->E.nodata = atof (&opt->arg[Ctrl->E.floating]);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'S':	/* Suppress/no-suppress NaNs on output */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -S is deprecated; use -s instead.\n");
					GMT_memset (GMT->current.io.io_nan_col, GMT_MAX_COLUMNS, int);
					GMT->current.io.io_nan_col[0] = GMT_Z;	/* The default is to examine the z-column */
					GMT->current.io.io_nan_ncols = GMT_IO_NAN_SKIP;		/* Default is that single z column */
					GMT->current.setting.io_nan_mode = 1;	/* Plain -S */
					if (opt->arg[0] == 'r') GMT->current.setting.io_nan_mode = GMT_IO_NAN_KEEP;	/* Old -Sr */
					GMT->common.s.active = true;
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':	/* Nan-value */
				if (GMT_compat_check (GMT, 4)) {	/* Honor old -N[i]<value> option */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -N is deprecated; use GMT common option -d[i|o]<nodata> instead.\n");
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Must specify value or NaN\n");
						n_errors++;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'W':	/* Add weight on output */
				Ctrl->W.active = true;
				if (opt->arg[0]) Ctrl->W.weight = atof (opt->arg);
				break;
			case 'Z':	/* Control format */
				Ctrl->Z.active = true;
				n_errors += GMT_parse_z_io (GMT, opt->arg, &Ctrl->Z);
					break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->Z.active) GMT_init_z_io (GMT, Ctrl->Z.format, Ctrl->Z.repeat, Ctrl->Z.swab, Ctrl->Z.skip, Ctrl->Z.type, io);

	n_errors += GMT_check_condition (GMT, n_files == 0, "Syntax error: Must specify at least one input file\n");
	n_errors += GMT_check_condition (GMT, n_files > 1 && Ctrl->E.active, "Syntax error: -E can only handle one input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && Ctrl->E.active, "Syntax error: -E is not compatible with -Z\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grd2xyz_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grd2xyz (void *V_API, int mode, void *args)
{
	bool first = true;
	unsigned int row, col, n_output;
	int error = 0, write_error = 0;
	
	uint64_t ij, gmt_ij, n_total = 0, n_suppressed = 0;

	char header[GMT_BUFSIZ];

	double wesn[4], d_value, out[4], *x = NULL, *y = NULL;

	struct GMT_GRID *G = NULL;
	struct GMT_Z_IO io;
	struct GMT_OPTION *opt = NULL;
	struct GRD2XYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grd2xyz_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grd2xyz_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grd2xyz_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grd2xyz_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grd2xyz_parse (GMT, Ctrl, &io, options))) Return (error);
	
	/*---------------------------- This is the grd2xyz main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid(s)\n");
	
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if (GMT->common.b.active[GMT_OUT]) {
		if (Ctrl->Z.active && !io.binary) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: -Z overrides -bo\n");
			GMT->common.b.active[GMT_OUT] = false;
		}
		if (Ctrl->E.active && GMT_compat_check (GMT, 4)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: -E overrides -bo\n");
			GMT->common.b.active[GMT_OUT] = false;
		}
	}
	else if (io.binary) GMT->common.b.active[GMT_OUT] = true;

	n_output = (Ctrl->Z.active) ? 1 : ((Ctrl->W.active) ? 4 : ((Ctrl->C.mode == 2) ? 2 : 3));
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_output)) != GMT_OK) Return (error);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers stdout, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	out[3] = Ctrl->W.weight;
		
	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */ 

		if (opt->option != '<') continue;	/* We are only processing input files here */

		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, opt->arg, NULL)) == NULL) {
			Return (API->error);
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Working on file %s\n", G->header->name);

		if (GMT_is_subset (GMT, G->header, wesn))	/* Subset requested; make sure wesn matches header spacing */
			GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, G->header), "");

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, opt->arg, G) == NULL) {
			Return (API->error);	/* Get subset */
		}

		n_total += G->header->nm;

		GMT_err_fail (GMT, GMT_set_z_io (GMT, &io, G), opt->arg);

		if (Ctrl->Z.active) {	/* Write z-values only to stdout */
			bool previous = GMT->common.b.active[GMT_OUT], rst = false;
			int (*save) (struct GMT_CTRL *, FILE *, uint64_t, double *);
			save = GMT->current.io.output;
			
			GMT->current.io.output = GMT_z_output;		/* Override and use chosen output mode */
			GMT->common.b.active[GMT_OUT] = io.binary;	/* May have to set binary as well */
			if (GMT->current.setting.io_nan_mode && GMT->current.io.io_nan_col[0] == GMT_Z) 
				{rst = true; GMT->current.io.io_nan_col[0] = GMT_X;}	/* Since we dont do xy here, only z */
			for (ij = 0; ij < io.n_expected; ij++) {
				gmt_ij = io.get_gmt_ij (&io, G, ij);	/* Get the corresponding grid node */
				d_value = G->data[gmt_ij];
				if ((io.x_missing && io.gmt_i == io.x_period) || (io.y_missing && io.gmt_j == 0)) continue;
				if (GMT->common.d.active[GMT_OUT] && GMT_is_dnan (d_value))	/* Grid node is NaN and -d was set, so change to nan-proxy */
					d_value = GMT->common.d.nan_proxy[GMT_OUT];
				else if (GMT_z_input_is_nan_proxy (GMT, GMT_Z, d_value))	/* The inverse: Grid node is nan-proxy and -di was set, so change to NaN */
					d_value = GMT->session.d_NaN;
				write_error = GMT_Put_Record (API, GMT_WRITE_DOUBLE, &d_value);
				if (write_error == 0) n_suppressed++;	/* Bad value caught by -s[r] */
			}
			GMT->current.io.output = save;			/* Reset pointer */
			GMT->common.b.active[GMT_OUT] = previous;	/* Reset binary */
			if (rst) GMT->current.io.io_nan_col[0] = GMT_Z;	/* Reset to what it was */
		}
		else if (Ctrl->E.active) {	/* ESRI format */
			double slop;
			char *record = NULL, item[GMT_BUFSIZ];
			size_t n_alloc, len, rec_len;
			slop = 1.0 - (G->header->inc[GMT_X] / G->header->inc[GMT_Y]);
			if (!GMT_IS_ZERO (slop)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: x_inc must equal y_inc when writing to ESRI format\n");
				Return (EXIT_FAILURE);
			}
			n_alloc = G->header->nx * 8;	/* Assume we only need 8 bytes per item (but we will allocate more if needed) */
			record = GMT_memory (GMT, NULL, G->header->nx, char);
			
			sprintf (record, "ncols %d\nnrows %d", G->header->nx, G->header->ny);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
			if (G->header->registration == GMT_GRID_PIXEL_REG) {	/* Pixel format */
				sprintf (record, "xllcorner ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[XLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
				sprintf (record, "yllcorner ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[YLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
			}
			else {	/* Gridline format */
				sprintf (record, "xllcenter ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[XLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
				sprintf (record, "yllcenter ");
				sprintf (item, GMT->current.setting.format_float_out, G->header->wesn[YLO]);
				strcat  (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
			}
			sprintf (record, "cellsize ");
			sprintf (item, GMT->current.setting.format_float_out, G->header->inc[GMT_X]);
			strcat  (record, item);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
			sprintf (record, "nodata_value %ld", lrint (Ctrl->E.nodata));
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a text record */
			GMT_row_loop (GMT, G, row) {	/* Scanlines, starting in the north (ymax) */
				rec_len = 0;
				GMT_col_loop (GMT, G, row, col, ij) {
					if (GMT_is_fnan (G->data[ij]))
						sprintf (item, "%ld", lrint (Ctrl->E.nodata));
					else if (Ctrl->E.floating)
						sprintf (item, GMT->current.setting.format_float_out, G->data[ij]);
					else
						sprintf (item, "%ld", lrint ((double)G->data[ij]));
					len = strlen (item);
					if ((rec_len + len + 1) >= n_alloc) {	/* Must get more memory */
						n_alloc <<= 1;
						record = GMT_memory (GMT, record, G->header->nx, char);
					}
					strcat (record, item);
					rec_len += len;
					if (col < (G->header->nx-1)) { strcat (record, " "); rec_len++;}
				}
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write a whole y line */
			}
			GMT_free (GMT, record);
		}
		else {	/* Regular x,y,z[,w] output */
			if (first && GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_STDIO_IF_NONE, 0, options) != GMT_OK) {	/* Establishes data output */
				Return (API->error);
			}

			/* Compute grid node positions once only */

			x = GMT_grd_coord (GMT, G->header, GMT_X);
			y = GMT_grd_coord (GMT, G->header, GMT_Y);
			if (Ctrl->C.active) {	/* Replace x,y with col,row */
				if (Ctrl->C.mode < 2) {
					GMT_row_loop  (GMT, G, row) y[row] = row + Ctrl->C.mode;
					GMT_col_loop2 (GMT, G, col) x[col] = col + Ctrl->C.mode;
				}
				else
					GMT->current.io.io_nan_col[0] = GMT_Y;	/* Since that is where z will go now */
				GMT_set_cartesian (GMT, GMT_OUT);
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
					sprintf (header, "# %s%s%s%s%s", G->header->y_units, GMT->current.setting.io_col_separator, G->header->x_units, GMT->current.setting.io_col_separator, G->header->z_units);
				else
					sprintf (header, "# %s%s%s%s%s", G->header->x_units, GMT->current.setting.io_col_separator, G->header->y_units, GMT->current.setting.io_col_separator, G->header->z_units);
				if (Ctrl->W.active) {
					strcat (header, GMT->current.setting.io_col_separator);
					strcat (header, "weight");
				}
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);	/* Write a header record */
				first = false;
			}

			GMT_grd_loop (GMT, G, row, col, ij) {
				if (Ctrl->C.mode == 2) {	/* Write index, z */
					out[GMT_X] = (double)GMT_IJ0 (G->header, row, col);
					out[GMT_Y] = G->data[ij];
					if (GMT->common.d.active[GMT_OUT] && GMT_is_dnan (out[GMT_Y]))	/* Input matched no-data setting, so change to NaN */
						out[GMT_Y] = GMT->common.d.nan_proxy[GMT_OUT];
					else if (GMT_z_input_is_nan_proxy (GMT, GMT_Z, out[GMT_Y]))
						out[GMT_Y] = GMT->session.d_NaN;
				}
				else {
					out[GMT_X] = x[col];	out[GMT_Y] = y[row];	out[GMT_Z] = G->data[ij];
					if (GMT->common.d.active[GMT_OUT] && GMT_is_dnan_func (out[GMT_Z]))	/* Input matched no-data setting, so change to NaN */
						out[GMT_Z] = GMT->common.d.nan_proxy[GMT_OUT];
					else if (GMT_z_input_is_nan_proxy (GMT, GMT_Z, out[GMT_Z]))
						out[GMT_Z] = GMT->session.d_NaN;
				}
				write_error = GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);		/* Write this to output */
				if (write_error != 0) n_suppressed++;	/* Bad value caught by -s[r] */
			}
			GMT_free (GMT, x);
			GMT_free (GMT, y);
		}
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " values extracted\n", n_total - n_suppressed);
	if (n_suppressed) {
		if (GMT->current.setting.io_nan_mode == GMT_IO_NAN_KEEP)
			GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " finite values suppressed\n", n_suppressed);
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64" NaN values suppressed\n", n_suppressed);
	}

	Return (GMT_OK);
}
