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
 * Brief synopsis: xyz2grd reads a xyz file from standard input and creates the
 * corresponding grd-file. The input file does not have to be
 * sorted. xyz2grd will report if some nodes are missing.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#define THIS_MODULE_NAME	"xyz2grd"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert data table to a grid file"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:RVbfhirs" GMT_OPT("FH")

void GMT_str_tolower (char *string);

struct XYZ2GRD_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A[f|l|n|m|r|s|u|z] */
		bool active;
		char mode;
	} A;
	struct D {	/* -D<xname>/<yname>/<zname>/<scale>/<offset>/<invalid>/<title>/<remark> */
		bool active;
		char *information;
	} D;
	struct E {	/* -E[<nodata>] */
		bool active;
		bool set;
		double nodata;
	} E;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct N {	/* -N<nodata> */
		bool active;
		double value;
	} N;
	struct S {	/* -S */
		bool active;
		char *file;
	} S;
	struct GMT_PARSE_Z_IO Z;
};

void *New_xyz2grd_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZ2GRD_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct XYZ2GRD_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.value = GMT->session.d_NaN;
	C->Z.type = 'a';
	C->Z.format[0] = 'T';	C->Z.format[1] = 'L';
	return (C);
}

void Free_xyz2grd_Ctrl (struct GMT_CTRL *GMT, struct XYZ2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->D.information) free (C->D.information);	
	if (C->G.file) free (C->G.file);	
	if (C->S.file) free (C->S.file);	
	GMT_free (GMT, C);	
}

int GMT_xyz2grd_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: xyz2grd [<table>] -G<outgrid> %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A[f|l|m|n|r|s|u|z]]\n\t[%s]\n", GMT_Rgeo_OPT, GMT_GRDEDIT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<nodata>] [-S[<zfile]] [%s] [-Z[<flags>]] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets name of the output grid file.\n");
	GMT_Option (API, "IR");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Determine what to do if multiple entries are found for a node:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Af: Keep first value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al: Keep lower (minimum) value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Am: Compute mean of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -An: Count number of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ar: Compute RMS of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -As: Keep last value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Au: Keep upper (maximum) value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Az: Sum multiple entries at the same node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will compute mean values].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Append header information; leave field blank to get default value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set value for nodes without input xyz triplet [Default is NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Z-table entries that equal <nodata> are replaced by NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Swap the byte-order of the input data and write result to <zfile>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (or stdout if no file given).  Requires -Z, and no grid file created!\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For this option, only one input file (or stdin) is allowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: Cannot handle swapping of 64-bit integers.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set exact specification of incoming 1-column z-table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unless -S is used, append two chars that indicate file organization:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If data is in row format, state if first row is at T(op) or B(ottom).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Then, append L or R to indicate starting point in row.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If data is in column format, state if first columns is L(left) or R(ight).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Then, append T or B to indicate starting point in column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To skip a header of size <n> bytes, append s<n> [<n> = 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To swap the byte-order of each word, append w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append x if gridline-registered, periodic data in x without repeating column at xmax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append y if gridline-registered, periodic data in y without repeating row at ymax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify one of the following data types (all binary except a):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     A  Ascii (multiple floating point values per record).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a  Ascii (one value per record).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c  int8_t, signed 1-byte character.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     u  uint8_t, unsigned 1-byte character.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h  int16_t, signed 2-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     H  uint16_t, unsigned 2-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     i  int32_t, signed 4-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     I  uint32_t, unsigned 4-byte integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l  int64_t, signed long (8-byte) integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     L  uint64_t, unsigned long (8-byte) integer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f  4-byte floating point single precision.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     d  8-byte floating point double precision.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default format is scanline orientation in ASCII representation: -ZTLa].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This option assumes all nodes have data values.\n");
	GMT_Option (API, "bi3,f,h,i,r,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_xyz2grd_parse (struct GMT_CTRL *GMT, struct XYZ2GRD_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options)
{
	/* This parses the options provided to xyz2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	bool do_grid, b_only = false;
	char *ptr_to_arg = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	memset (io, 0, sizeof(struct GMT_Z_IO)); /* Initialize with zero */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				if (!opt->arg[0] && GMT_compat_check (GMT, 4)) {	/* In GMT4, just -A implied -Az */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -A is deprecated; use -Az instead.\n");
					Ctrl->A.active = true;
					Ctrl->A.mode = 'z';
				}
				else if (!strchr ("flmnrsuz", opt->arg[0])) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: Select -Af, -Al, -Am, -An, -Ar, -As, -Au, or -Az\n");
					n_errors++;
				}
				else {
					Ctrl->A.active = true;
					Ctrl->A.mode = opt->arg[0];
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.information = strdup (opt->arg);
				break;
			case 'E':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -E is deprecated; use grdreformat instead.\n");
					Ctrl->E.active = true;
					if (opt->arg[0]) {
						Ctrl->E.nodata = atof (opt->arg);
						Ctrl->E.set = true;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				if (opt->arg[0])
					Ctrl->N.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Must specify value or NaN\n");
					n_errors++;
				}
				break;
			case 'S':
				Ctrl->S.active = true;
				if (opt->arg[0]) Ctrl->S.file = strdup (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				ptr_to_arg = opt->arg;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				if (opt->option == 'b') b_only = true;
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);
	if (Ctrl->Z.active) {
		if (Ctrl->S.active) Ctrl->Z.not_grid = true;	/* The row/col organization does not apply */
		n_errors += GMT_parse_z_io (GMT, ptr_to_arg, &Ctrl->Z);
	}

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !Ctrl->Z.active, "Syntax error -S option: Must also specify -Z\n");
	if (Ctrl->S.active) {	/* Reading and writing binary file */
		if (n_files > 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -S can only handle one input file\n");
			n_errors++;
		}
		else {
			strcpy (GMT->current.io.r_mode, "rb");
			strcpy (GMT->current.io.w_mode, "wb");
		}
		Ctrl->Z.swab = 1;	/* Only swap on input */
	}

	if (Ctrl->Z.active) {
		GMT_init_z_io (GMT, Ctrl->Z.format, Ctrl->Z.repeat, Ctrl->Z.swab, Ctrl->Z.skip, Ctrl->Z.type, io);
		GMT->common.b.type[GMT_IN] = Ctrl->Z.type;
		if (b_only) {
			GMT->common.b.active[GMT_IN] = false;
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: -Z overrides -bi\n");
		}
	}

	do_grid = !(Ctrl->S.active || Ctrl->E.active);
	if (do_grid) {
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !(Ctrl->G.active || Ctrl->G.file), "Syntax error option -G: Must specify output file\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_xyz2grd_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_xyz2grd (void *V_API, int mode, void *args)
{
	bool previous_bin_i = false, previous_bin_o = false;
	int error = 0, scol, srow;
	unsigned int zcol, row, col, i, *flag = NULL;
	uint64_t n_empty = 0, n_stuffed = 0, n_bad = 0, n_confused = 0;
	uint64_t ij, gmt_ij, n_read = 0, n_filled = 0, n_used = 0;

	char c, Amode;

	double *in = NULL, wesn[4];

	float no_data_f;

	void * (*save_i) (struct GMT_CTRL *, FILE *, uint64_t *, int *) = NULL;
	int (*save_o) (struct GMT_CTRL *, FILE *, uint64_t, double *);
	
	struct GMT_GRID *Grid = NULL;
	struct GMT_Z_IO io;
	struct XYZ2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_xyz2grd_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_xyz2grd_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_xyz2grd_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_xyz2grd_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_xyz2grd_parse (GMT, Ctrl, &io, options))) Return (error);

	/*---------------------------- This is the xyz2grd main code ----------------------------*/

	if (Ctrl->S.active) {	/* Just swap data and bail */
		int out_ID;
		unsigned w_mode = GMT_ADD_DEFAULT;
		
		save_i = GMT->current.io.input;			/* Save previous i/0 parameters */
		save_o = GMT->current.io.output;
		previous_bin_i = GMT->common.b.active[GMT_IN];
		previous_bin_o = GMT->common.b.active[GMT_OUT];
		GMT->current.io.input = GMT_z_input;		/* Override input reader with chosen binary reader for selected type */
		GMT->current.io.output = GMT_z_output;		/* Override output writer with chosen binary writer for selected type */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set input binary as well */
		GMT->common.b.active[GMT_OUT] = io.binary;	/* May have to set output binary as well */
		if ((error = GMT_set_cols (GMT, GMT_IN, 1))) Return (error);
		/* Initialize the i/o since we are doing record-by-record reading/writing */
		GMT_Report (API, GMT_MSG_VERBOSE, "Swapping data bytes only\n");
		if (Ctrl->S.active) io.swab = true;	/* Need to pass swabbing down to the gut level */

		/* Register the data source */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set via file */
			Return (API->error);
		}
		if (Ctrl->S.file) {	/* Specified an output file */
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_OUT, NULL, Ctrl->S.file)) == GMT_NOTSET) {
				Return (API->error);
			}
			w_mode = GMT_ADD_EXISTING;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, w_mode, 0, options) != GMT_OK) {	/* Establishes data output to stdout */
			Return (API->error);
		}
		if ((error = GMT_set_cols (GMT, GMT_IN, 1)) != GMT_OK) {	/* We dont really care or know about columns so must use 1 */
			Return (API->error);
		}
		if ((error = GMT_set_cols (GMT, GMT_OUT, 1)) != GMT_OK) {	/* We dont really care or know about columns so must use 1 */
			Return (API->error);
		}
		/* Initialize the i/o for doing record-by-record reading/writing */
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_OFF) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all headers */
					continue;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			/* Data record to process */

			GMT_Put_Record (API, GMT_WRITE_DOUBLE, in);
		} while (true);

		GMT->current.io.input = save_i;			/* Reset input pointer */
		GMT->common.b.active[GMT_IN] = previous_bin_i;	/* Reset input binary */
		GMT->current.io.output = save_o;		/* Reset output pointer */
		GMT->common.b.active[GMT_OUT] = previous_bin_o;	/* Reset output binary */

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
		Return (GMT_OK);	/* We are done here */
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");

	/* PW: This is now done in grdreformat since ESRI Arc Interchange is a recognized format */
	if (Ctrl->E.active && GMT_compat_check (GMT, 4)) {	/* Read an ESRI Arc Interchange grid format in ASCII.  This must be a single physical file. */
		uint64_t n_left;
		double value;
		char line[GMT_BUFSIZ];
		FILE *fp = GMT->session.std[GMT_IN];
		
		if (Ctrl->In.file && (fp = GMT_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}
		
		if ((Grid = GMT_create_grid (GMT)) == NULL) Return (API->error);
		GMT_grd_init (GMT, Grid->header, options, false);
		Grid->header->registration = GMT_GRID_NODE_REG;
		GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->nx) != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding ncols record\n");
			Return (EXIT_FAILURE);
		}
		GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->ny) != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding ncols record\n");
			Return (EXIT_FAILURE);
		}
		GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[XLO]) != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding xll record\n");
			Return (EXIT_FAILURE);
		}
		if (!strncmp (line, "xllcorner", 9U)) Grid->header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
		GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[YLO]) != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding yll record\n");
			Return (EXIT_FAILURE);
		}
		if (!strncmp (line, "yllcorner", 9U)) Grid->header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
		GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->inc[GMT_X]) != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error decoding cellsize record\n");
			Return (EXIT_FAILURE);
		}
		Grid->header->inc[GMT_Y] = Grid->header->inc[GMT_X];
		Grid->header->xy_off = 0.5 * Grid->header->registration;
		Grid->header->wesn[XHI] = Grid->header->wesn[XLO] + (Grid->header->nx - 1 + Grid->header->registration) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YHI] = Grid->header->wesn[YLO] + (Grid->header->ny - 1 + Grid->header->registration) * Grid->header->inc[GMT_Y];
		GMT_set_grddim (GMT, Grid->header);
		GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);

		GMT_Report (API, GMT_MSG_VERBOSE, "nx = %d  ny = %d\n", Grid->header->nx, Grid->header->ny);
		n_left = Grid->header->nm;

		Grid->data = GMT_memory_aligned (GMT, NULL, Grid->header->nm, float);
		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids */
		row = col = 0;
		fscanf (fp, "%s", line);
		GMT_str_tolower (line);
		if (!strcmp (line, "nodata_value")) {	/* Found the optional nodata word */
			fscanf (fp, "%lf", &value);
			if (Ctrl->E.set && !doubleAlmostEqualZero (value, Ctrl->E.nodata)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Your -E%g overrides the nodata_value of %g found in the ESRI file\n", Ctrl->E.nodata, value);
			}
			else
				Ctrl->E.nodata = value;
		}
		else {	/* Instead got the very first data value */
			ij = GMT_IJP (Grid->header, row, col);
			value = atof (line);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (float) value;
			if (++col == Grid->header->nx) col = 0, row++;
			n_left--;
		}
		while (fscanf (fp, "%lf", &value) == 1 && n_left) {
			ij = GMT_IJP (Grid->header, row, col);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (float) value;
			if (++col == Grid->header->nx) col = 0, row++;
			n_left--;
		}
		GMT_fclose (GMT, fp);
		if (n_left) {
			GMT_Report (API, GMT_MSG_NORMAL, "Expected %" PRIu64 " points, found only %" PRIu64 "\n", Grid->header->nm, Grid->header->nm - n_left);
			Return (EXIT_FAILURE);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
			Return (API->error);
		}
		Return (EXIT_SUCCESS);
	}

	/* Here we will read either x,y,z or z data, using -R -I [-r] for sizeing */
	
	no_data_f = (float)Ctrl->N.value;
	
	/* Set up and allocate output grid [note: zero padding specificied] */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, 0, Ctrl->G.file)) == NULL) Return (API->error);
	
	Amode = Ctrl->A.active ? Ctrl->A.mode : 'm';

	if (GMT->common.b.active[GMT_IN] && GMT->current.io.col_type[GMT_IN][GMT_Z] & GMT_IS_RATIME && GMT->current.io.fmt[GMT_IN][GMT_Z].type == GMT_FLOAT) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Your single precision binary input data are unlikely to hold absolute time coordinates without serious truncation.\n");
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: You must use double precision when storing absolute time coordinates in binary data tables.\n");
	}

	if (Ctrl->D.active) GMT_decode_grd_h_info (GMT, Ctrl->D.information, Grid->header);

	GMT_Report (API, GMT_MSG_VERBOSE, "nx = %d  ny = %d  nm = %" PRIu64 "  size = %" PRIuS "\n", Grid->header->nx, Grid->header->ny, Grid->header->nm, Grid->header->size);

	GMT_err_fail (GMT, GMT_set_z_io (GMT, &io, Grid), Ctrl->G.file);

	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	if (Ctrl->Z.active && Ctrl->N.active && GMT_is_dnan (Ctrl->N.value)) Ctrl->N.active = false;	/* No point testing */

	if (Ctrl->Z.active) {	/* Need to override input method since reading single input column as z (not x,y) */
		zcol = GMT_X;
		save_i = GMT->current.io.input;
		previous_bin_i = GMT->common.b.active[GMT_IN];
		GMT->current.io.input = GMT_z_input;		/* Override and use chosen input mode */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set binary as well */
		in = GMT->current.io.curr_rec;
		GMT->current.io.fmt[GMT_IN][zcol].type = GMT_get_io_type (GMT, Ctrl->Z.type);
	}
	else {
		zcol = GMT_Z;
		flag = GMT_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* No padding needed for flag array */
		GMT_memset (Grid->header->pad, 4, unsigned int);	/* Algorithm below expects no padding; we repad at the end */
		GMT->current.setting.io_nan_records = false;	/* Cannot have x,y as NaNs here */
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, Ctrl->Z.active ? 1 : Ctrl->A.mode == 'n' ? 2 : 3)) != GMT_OK) {
		Return (error);
	}
	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {
		Return (API->error);	/* Establishes data input */
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {
		Return (API->error);	/* Enables data input and sets access mode */
	}

	n_read = ij = 0;
	if (Ctrl->Z.active) for (i = 0; i < io.skip; i++)
		fread (&c, sizeof (char), 1, API->object[API->current_item[GMT_IN]]->fp);

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
	
		n_read++;
		if (Ctrl->Z.active) {	/* Read separately because of all the possible formats */
			if (ij == io.n_expected) {
				GMT_Report (API, GMT_MSG_NORMAL, "More than %" PRIu64 " records, only %" PRIu64 " was expected (aborting)!\n", ij, io.n_expected);
				Return (EXIT_FAILURE);
			}
			gmt_ij = io.get_gmt_ij (&io, Grid, ij);	/* Convert input order to output node (with padding) as per -Z */
			Grid->data[gmt_ij] = (Ctrl->N.active && in[zcol] == Ctrl->N.value) ? GMT->session.f_NaN : (float)in[zcol];
			ij++;
		}
		else {	/* Get x, y, z */
			if (GMT_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
			if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

			/* Ok, we are inside the region - process data */

			scol = (int)GMT_grd_x_to_col (GMT, in[GMT_X], Grid->header);
			if (scol == -1) scol++, n_confused++;
			col = scol;
			if (col == Grid->header->nx) col--, n_confused++;
			srow = (int)GMT_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
			if (srow == -1) srow++, n_confused++;
			row = srow;
			if (row == Grid->header->ny) row--, n_confused++;
			ij = GMT_IJ0 (Grid->header, row, col);	/* Because padding is turned off we can use ij for both Grid and flag */
			if (Amode == 'f') {	/* Want the first value to matter only */
				if (flag[ij] == 0) {	/* Assign first value and that is the end of it */
					Grid->data[ij] = (float)in[zcol];
					flag[ij] = 1;
				}
			}
			else if (Amode == 's') {	/* Want the last value to matter only */
				Grid->data[ij] = (float)in[zcol];	/* Assign last value and that is it */
				flag[ij] = 1;
			}
			else if (Amode == 'l') {	/* Keep lowest value */
				if (flag[ij]) {	/* Already assigned the first value */
					if (in[zcol] < (double)Grid->data[ij]) Grid->data[ij] = (float)in[zcol];
				}
				else {	/* First time, just assign the current value */
					Grid->data[ij] = (float)in[zcol];
					flag[ij] = 1;
				}
			}
			else if (Amode == 'u') {	/* Keep highest value */
				if (flag[ij]) {	/* Already assigned the first value */
					if (in[zcol] > (double)Grid->data[ij]) Grid->data[ij] = (float)in[zcol];
				}
				else {	/* First time, just assign the current value */
					Grid->data[ij] = (float)in[zcol];
					flag[ij] = 1;
				}
			}
			else if (Amode == 'r') { 	/* Add up squares in case we must rms */
				Grid->data[ij] += (float)in[zcol] * (float)in[zcol];
				flag[ij]++;
			}
			else { 	/* Add up in case we must sum or mean */
				Grid->data[ij] += (float)in[zcol];
				flag[ij]++;
			}
			n_used++;
		}
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (Ctrl->Z.active) {
		GMT->current.io.input = save_i;	/* Reset pointer */
		GMT->common.b.active[GMT_IN] = previous_bin_i;	/* Reset binary */
		if (ij != io.n_expected) {	/* Input amount does not match expectations */
			GMT_Report (API, GMT_MSG_NORMAL, "Found %" PRIu64 " records, but %" PRIu64 " was expected (aborting)!\n", ij, io.n_expected);
			Return (EXIT_FAILURE);
		}
		GMT_check_z_io (GMT, &io, Grid);	/* This fills in missing periodic row or column */
	}
	else {	/* xyz data could have resulted in duplicates */
		if (GMT_grd_duplicate_column (GMT, Grid->header, GMT_IN)) {	/* Make sure longitudes got replicated */
			uint64_t ij_west, ij_east;
			bool first_bad = true;

			for (row = 0; row < Grid->header->ny; row++) {	/* For each row, look at west and east bin */
				ij_west = GMT_IJ0 (Grid->header, row, 0);
				ij_east = GMT_IJ0 (Grid->header, row, Grid->header->nx - 1);
				
				if (flag[ij_west] && !flag[ij_east]) {		/* Nothing in east bin, just copy from west */
					Grid->data[ij_east] = Grid->data[ij_west];
					flag[ij_east] = flag[ij_west];
				}
				else if (flag[ij_east] && !flag[ij_west]) {	/* Nothing in west bin, just copy from east */
					Grid->data[ij_west] = Grid->data[ij_east];
					flag[ij_west] = flag[ij_east];
				}
				else {	/* Both have some stuff, consolidate combined value into the west bin, then replicate to the east */
					if (Amode == 'f' || Amode == 's') {	/* Trouble since we did not store when we added these points */
						if (first_bad) {
							GMT_Report (API, GMT_MSG_VERBOSE, "Using -Af|s with replicated longitude bins may give inaccurate values");
							first_bad = false;
						}
					}
					else if (Amode == 'l') {
						if (Grid->data[ij_east] < Grid->data[ij_west]) Grid->data[ij_west] = Grid->data[ij_east];
					}
					else if (Amode == 'u') {
						if (Grid->data[ij_east] > Grid->data[ij_west]) Grid->data[ij_west] = Grid->data[ij_east];
					}
					else { 	/* Add up incase we must sum, rms or mean */
						Grid->data[ij_west] += Grid->data[ij_east];
						flag[ij_west] += flag[ij_east];
					}
					/* Replicate: */
					Grid->data[ij_east] = Grid->data[ij_west];
					flag[ij_east] = flag[ij_west];
				}
			}
		}

		for (ij = 0; ij < Grid->header->nm; ij++) {	/* Check if all nodes got one value only */
			if (flag[ij] == 1) {	/* This catches nodes with one value or the -Al|u single values */
				if (Amode == 'n') Grid->data[ij] = 1.0f;
				n_filled++;
			}
			else if (flag[ij] == 0) {
				n_empty++;
				Grid->data[ij] = no_data_f;
			}
			else {	/* More than 1 value went to this node */
				if (Amode == 'n')
					Grid->data[ij] = (float)flag[ij];
				else if (Amode == 'm')
					Grid->data[ij] /= (float)flag[ij];
				else if (Amode == 'r')
					Grid->data[ij] = (float)sqrt (Grid->data[ij] / (float)flag[ij]);
				/* implicit else means return the sum of the values */
				n_filled++;
				n_stuffed++;
			}
		}
		GMT_free (GMT, flag);
		
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			char line[GMT_BUFSIZ];
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			GMT_Report (API, GMT_MSG_VERBOSE, " n_read: %" PRIu64 "  n_used: %" PRIu64 "  n_filled: %" PRIu64 " n_empty: %" PRIu64 " set to ",
				n_read, n_used, n_filled, n_empty);
			(GMT_is_dnan (Ctrl->N.value)) ? GMT_Report (API, GMT_MSG_VERBOSE, "NaN\n") : GMT_Report (API, GMT_MSG_VERBOSE, line, Ctrl->N.value);
			if (n_bad) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " records unreadable\n", n_bad);
			if (n_stuffed && Amode != 'n') GMT_Report (API, GMT_MSG_VERBOSE, "Warning - %" PRIu64 " nodes had multiple entries that were processed\n", n_stuffed);
			if (n_confused) GMT_Report (API, GMT_MSG_VERBOSE, "Warning - %" PRIu64 " values gave bad indices: Pixel vs gridline confusion?\n", n_confused);
		}
	}

	GMT_grd_pad_on (GMT, Grid, GMT->current.io.pad);	/* Restore padding */
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
		Return (API->error);
	}

	Return (GMT_OK);
}
