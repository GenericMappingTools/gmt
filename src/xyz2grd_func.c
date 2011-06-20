/*--------------------------------------------------------------------
 *	$Id: xyz2grd_func.c,v 1.21 2011-06-20 21:45:16 guru Exp $
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
 * Brief synopsis: xyz2grd reads a xyz file from standard input and creates the
 * corresponding grd-file. The input file does not have to be
 * sorted. xyz2grd will report if some nodes are missing.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

#ifdef GMT_COMPAT
EXTERN_MSC void GMT_str_tolower (char *string);
#endif

struct XYZ2GRD_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A[f|l|n|s|u|z] */
		GMT_LONG active;
		char mode;
	} A;
	struct D {	/* -D<xname>/<yname>/<zname>/<scale>/<offset>/<title>/<remark> */
		GMT_LONG active;
		char *information;
	} D;
#ifdef GMT_COMPAT
	struct E {	/* -E[<nodata>] */
		GMT_LONG active;
		GMT_LONG set;
		double nodata;
	} E;
#endif
	struct G {	/* -G<output_grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct N {	/* -N<nodata> */
		GMT_LONG active;
		double value;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
		char *file;
	} S;
	struct GMT_PARSE_Z_IO Z;
};

void *New_xyz2grd_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZ2GRD_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct XYZ2GRD_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->N.value = GMT->session.d_NaN;
	C->Z.type = 'a';
	C->Z.format[0] = 'T';	C->Z.format[1] = 'L';
	return ((void *)C);
}

void Free_xyz2grd_Ctrl (struct GMT_CTRL *GMT, struct XYZ2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->D.information) free ((void *)C->D.information);	
	if (C->G.file) free ((void *)C->G.file);	
	if (C->S.file) free ((void *)C->S.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_xyz2grd_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "xyz2grd %s [API] - Converting [xy]z data to a GMT grid file\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: xyz2grd [<table>] -G<outgrid> %s %s\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-A[f|l|n|s|u|z]] [%s]\n", GMT_GRDEDIT);
	GMT_message (GMT, "\t[-N<nodata>] [-S[<zfile]] [%s] [-Z[<flags>]] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Sets name of the output grid file.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Determine what to do if multiple entries are found for a node:\n");
	GMT_message (GMT, "\t   -Af: Keep first value if multiple entries per node.\n");
	GMT_message (GMT, "\t   -Al: Keep lower (minimum) value if multiple entries per node.\n");
	GMT_message (GMT, "\t   -An: Count number of multiple entries per node instead.\n");
	GMT_message (GMT, "\t   -As: Keep last value if multiple entries per node.\n");
	GMT_message (GMT, "\t   -Au: Keep upper (maximum) value if multiple entries per node.\n");
	GMT_message (GMT, "\t   -Az: Sum multiple entries at the same node.\n");
	GMT_message (GMT, "\t   [Default will compute mean values].\n");
	GMT_message (GMT, "\t-D Append header information; specify '=' to get default value.\n");
	GMT_message (GMT, "\t-N Set value for nodes without input xyz triplet [Default is NaN].\n");
	GMT_message (GMT, "\t   Z-table entries that equal <nodata> are replaced by NaN.\n");
	GMT_message (GMT, "\t-S Swap the byte-order of the input data and write resut to <zfile>\n");
	GMT_message (GMT, "\t   (or stdout if no file given).  Requires -Z, and no grid file created!\n");
	GMT_message (GMT, "\t   For this option, only one input file (or stdin) is allowed.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-Z Set exact specification of incoming 1-column z-table.\n");
	GMT_message (GMT, "\t   If data is in row format, state if first row is at T(op) or B(ottom).\n");
	GMT_message (GMT, "\t     Then, append L or R to indicate starting point in row.\n");
	GMT_message (GMT, "\t   If data is in column format, state if first columns is L(left) or R(ight).\n");
	GMT_message (GMT, "\t     Then, append T or B to indicate starting point in column.\n");
	GMT_message (GMT, "\t   To skip a header of size <n> bytes, append s<n> [<n> = 0].\n");
	GMT_message (GMT, "\t   To swap byte-order in 2-byte words, append w.\n");
	GMT_message (GMT, "\t   Append x if gridline-registered, periodic data in x without repeating column at xmax.\n");
	GMT_message (GMT, "\t   Append y if gridline-registered, periodic data in y without repeating row at ymax.\n");
	GMT_message (GMT, "\t   Specify one of the following data types (all binary except a):\n");
	GMT_message (GMT, "\t     A  Ascii (multiple floating point values per record).\n");
	GMT_message (GMT, "\t     a  Ascii (one value per record).\n");
	GMT_message (GMT, "\t     c  signed 1-byte character.\n");
	GMT_message (GMT, "\t     u  unsigned 1-byte character.\n");
	GMT_message (GMT, "\t     h  signed short 2-byte integer.\n");
	GMT_message (GMT, "\t     H  unsigned short 2-byte integer.\n");
	GMT_message (GMT, "\t     i  signed 4-byte integer.\n");
	GMT_message (GMT, "\t     I  unsigned 4-byte integer.\n");
	if (sizeof (GMT_LONG) == 8) {
		GMT_message (GMT, "\t     l  signed long (8-byte) integer.\n");
		GMT_message (GMT, "\t     L  unsigned long (8-byte) integer.\n");
	}
	GMT_message (GMT, "\t     f  4-byte floating point single precision.\n");
	GMT_message (GMT, "\t     d  8-byte floating point double precision.\n");
	GMT_message (GMT, "\t   [Default format is scanline orientation in ASCII representation: -ZTLa].\n");
	GMT_message (GMT, "\t   This option assumes all nodes have data values.\n");
	GMT_explain_options (GMT, "C3fhiF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_xyz2grd_parse (struct GMTAPI_CTRL *C, struct XYZ2GRD_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options)
{
	/* This parses the options provided to xyz2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, do_grid, b_only = FALSE;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
#ifdef GMT_COMPAT
				if (!opt->arg[0]) {	/* In GMT4, just -A implied -Az */
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -A is deprecated; use -Az instead.\n");
					Ctrl->A.active = TRUE;
					Ctrl->A.mode = 'z';
				}
				else
#endif
				if (!strchr ("flnsuz", opt->arg[0])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -A option: Select -Af, -Al, -An, -As, -Au, or -Az\n");
					n_errors++;
				}
				else {
					Ctrl->A.active = TRUE;
					Ctrl->A.mode = opt->arg[0];
				}
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.information = strdup (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'E':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -E is deprecated; use grdreformat instead.\n");
				Ctrl->E.active = TRUE;
				if (opt->arg[0]) {
					Ctrl->E.nodata = atof (opt->arg);
					Ctrl->E.set = TRUE;
				}
				break;
#endif
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				if (opt->arg[0])
					Ctrl->N.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -N option: Must specify value or NaN\n");
					n_errors++;
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.file = strdup (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				n_errors += (GMT_LONG)GMT_parse_z_io (GMT, opt->arg, &Ctrl->Z);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				if (opt->option == 'b') b_only = TRUE;
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !Ctrl->Z.active, "Syntax error -S option: Must also specify -Z\n");
	if (Ctrl->S.active) {	/* Reading and writing binary file */
		if (n_files > 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: -S can only handle one input file\n");
			n_errors++;
		}
		else {
			strcpy (GMT->current.io.r_mode, "rb");
			strcpy (GMT->current.io.w_mode, "wb");
		}
	}

	GMT_init_z_io (GMT, Ctrl->Z.format, Ctrl->Z.repeat, Ctrl->Z.swab, Ctrl->Z.skip, Ctrl->Z.type, io);
	if (b_only && Ctrl->Z.active) {
		GMT->common.b.active[GMT_IN] = FALSE;
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: -Z overrides -bi\n");
	}

#ifdef GMT_COMPAT
	do_grid = !(Ctrl->S.active || Ctrl->E.active);
#else
	do_grid = !(Ctrl->S.active);
#endif
	if (do_grid) {
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !(Ctrl->G.active || Ctrl->G.file), "Syntax error option -G: Must specify output file\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_xyz2grd_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_xyz2grd (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, count = FALSE, average = TRUE, previous = 0;
	GMT_LONG zcol, row, col, n_fields, high_low = 0, *flag = NULL;
	GMT_LONG i, ij, gmt_ij, n_read = 0, n_filled = 0, n_used = 0, first_last = 0;
	GMT_LONG n_empty = 0, n_stuffed = 0, n_bad = 0, n_confused = 0;

	double *in = NULL, wesn[4];

	float no_data_f;

	PFL save = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GMT_Z_IO io;
	struct XYZ2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	char c;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_xyz2grd_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_xyz2grd_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_xyz2grd", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "hirs" GMT_OPT("FH"), options))) Return (error);
	Ctrl = (struct XYZ2GRD_CTRL *) New_xyz2grd_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_xyz2grd_parse (API, Ctrl, &io, options))) Return (error);

	/*---------------------------- This is the xyz2grd main code ----------------------------*/

	if (Ctrl->S.active) {	/* Just swap data and bail */
		GMT_LONG in_ID;
		double bucket;
		
		save = GMT->current.io.input;			/* Save previous input parameters */
		previous = GMT->common.b.active[GMT_IN];
		GMT->current.io.input = GMT_z_input;		/* Override input reader */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set input binary as well */
		if ((error = GMT_set_cols (GMT, GMT_IN, 1))) Return (error);
		/* Initialize the i/o since we are doing record-by-record reading/writing */
		GMT_report (GMT, GMT_MSG_NORMAL, "Swapping data bytes only\n");
		if (Ctrl->S.active) io.swab = TRUE;	/* Need to pass swabbing down to the gut level */

		if (!Ctrl->S.file)
			GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, (void **)&Ctrl->S.file, NULL, NULL, &in_ID);
		else if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
		
		GMT->current.io.input = save;			/* Reset input pointer */
		GMT->common.b.active[GMT_IN] = previous;	/* Reset input binary */
		save = GMT->current.io.output;			/* Save previous output parameters */
		previous = GMT->common.b.active[GMT_OUT];
		GMT->current.io.output = GMT_z_output;		/* Override output writer */
		GMT->common.b.active[GMT_OUT] = io.binary;	/* May have to set output binary as well */
		in = &bucket;
		while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in))) GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)in);

		GMT->current.io.output = save;			/* Reset output pointer */
		GMT->common.b.active[GMT_OUT] = previous;	/* Reset output binary */

		Return (EXIT_SUCCESS);	/* We are done here */
	}

	/* Here we will need a grid */
	
	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

#ifdef GMT_COMPAT	/* PW: This is now done in grdreformat since ESRI Arc Interchange is a recognized format */
	if (Ctrl->E.active) {	/* Read an ESRI Arc Interchange grid format in ASCII.  This must be a single physical file. */
		GMT_LONG n_left;
		float value;
		char line[GMT_BUFSIZ], *not_used = NULL;
		FILE *fp = GMT->session.std[GMT_IN];
		
		if (Ctrl->In.file && (fp = GMT_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}
		
		Grid->header->registration = GMT_GRIDLINE_REG;
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->nx) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error decoding ncols record\n");
			Return (EXIT_FAILURE);
		}
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->ny) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error decoding ncols record\n");
			Return (EXIT_FAILURE);
		}
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[XLO]) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error decoding xll record\n");
			Return (EXIT_FAILURE);
		}
		if (!strncmp (line, "xllcorner", (size_t)9)) Grid->header->registration = GMT_PIXEL_REG;	/* Pixel grid */
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[YLO]) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error decoding yll record\n");
			Return (EXIT_FAILURE);
		}
		if (!strncmp (line, "yllcorner", (size_t)9)) Grid->header->registration = GMT_PIXEL_REG;	/* Pixel grid */
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->inc[GMT_X]) != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error decoding cellsize record\n");
			Return (EXIT_FAILURE);
		}
		Grid->header->inc[GMT_Y] = Grid->header->inc[GMT_X];
		Grid->header->xy_off = 0.5 * Grid->header->registration;
		Grid->header->wesn[XHI] = Grid->header->wesn[XLO] + (Grid->header->nx - 1 + Grid->header->registration) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YHI] = Grid->header->wesn[YLO] + (Grid->header->ny - 1 + Grid->header->registration) * Grid->header->inc[GMT_Y];
		GMT_set_grddim (GMT, Grid->header);
		GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);

		GMT_report (GMT, GMT_MSG_NORMAL, "nx = %d  ny = %d\n", Grid->header->nx, Grid->header->ny);
		n_left = Grid->header->nm;

		Grid->data = GMT_memory (GMT, NULL, Grid->header->nm, float);
		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids */
		row = col = 0;
		fscanf (fp, "%s", line);	GMT_str_tolower (line);
		if (!strcmp (line, "nodata_value")) {	/* Found the optional nodata word */
			fscanf (fp, "%f", &value);
			if (Ctrl->E.set && !GMT_IS_ZERO (value - Ctrl->E.nodata)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Your -E%g overrides the nodata_value of %g found in the ESRI file\n", Ctrl->E.nodata, value);
			}
			else
				Ctrl->E.nodata = value;
		}
		else {	/* Instead got the very first data value */
			ij = GMT_IJP (Grid->header, row, col);
			value = (float)atof (line);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (float) value;
			if (++col == Grid->header->nx) col = 0, row++;
			n_left--;
		}
		while (fscanf (fp, "%f", &value) == 1 && n_left) {
			ij = GMT_IJP (Grid->header, row, col);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (float) value;
			if (++col == Grid->header->nx) col = 0, row++;
			n_left--;
		}
		GMT_fclose (GMT, fp);
		if (n_left) {
			GMT_report (GMT, GMT_MSG_FATAL, "Expected %ld points, found only %ld\n", Grid->header->nm, Grid->header->nm - n_left);
			Return (EXIT_FAILURE);
		}
		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
		if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
		Return (EXIT_SUCCESS);
	}
#endif

	/* Here we will read either x,y,z or z data, using -R -I [-r] for sizeing */
	
	no_data_f = (float)Ctrl->N.value;
	
	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);
	
	if (Ctrl->A.active) {
		switch (Ctrl->A.mode) {
			case 'f':	/* Return the first value at each node */
				first_last = -1;
				break;
			case 'n':	/* Count the number of values for each node */
				count = TRUE;
				average = FALSE;
				break;
			case 'l':	/* Return the lowest (minimum) value at each node */
				high_low = -1;
				break;
			case 's':	/* Return the laSt value at each node */
				first_last = +1;
				break;
			case 'u':	/* Return the upper (maximum)  value at each node */
				high_low = +1;
				break;
			case 'z':	/* Return the sum of multiple values at each node */
			case '\0':
				average = count = FALSE;
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->current.io.col_type[GMT_IN][GMT_Z] & GMT_IS_RATIME && GMT->current.io.fmt[GMT_IN][GMT_Z].type == GMT_FLOAT_TYPE) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Your single precision binary input data are unlikely to hold absolute time coordinates without serious truncation.\n");
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: You must use double precision when storing absolute time coordinates in binary data tables.\n");
	}

	if (Ctrl->D.active) GMT_decode_grd_h_info (GMT, Ctrl->D.information, Grid->header);

	GMT_report (GMT, GMT_MSG_NORMAL, "nx = %d  ny = %d  nm = %ld  size = %ld\n", Grid->header->nx, Grid->header->ny, Grid->header->nm, Grid->header->size);

	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);		/* Allow for padding to be restored later */

	GMT_err_fail (GMT, GMT_set_z_io (GMT, &io, Grid), Ctrl->G.file);

	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	if (Ctrl->Z.active && Ctrl->N.active && GMT_is_dnan (Ctrl->N.value)) Ctrl->N.active = FALSE;	/* No point testing */

	if (Ctrl->Z.active) {	/* Need to override input method */
		zcol = GMT_X;
		save = GMT->current.io.input;
		previous = GMT->common.b.active[GMT_IN];
		GMT->current.io.input = GMT_z_input;		/* Override and use chosen input mode */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set binary as well */
		in = GMT->current.io.curr_rec;
		GMT->current.io.fmt[GMT_IN][zcol].type = GMT_get_io_type (GMT, Ctrl->Z.type);
	}
	else {
		zcol = GMT_Z;
		flag = GMT_memory (GMT, NULL, Grid->header->nm, GMT_LONG);	/* No padding needed for flag array */
		GMT_memset (Grid->header->pad, 4, GMT_LONG);	/* Algorithm below expects no padding; we repad at the end */
		GMT->current.setting.io_nan_records = FALSE;	/* Cannot have x,y as NaNs here */
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, Ctrl->Z.active ? 1 : 3))) Return (error);
	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
	
	n_read = 0;
	ij = -1;	/* Will be incremented to 0 in -Z section or recomputed in the xyz section */
	if (Ctrl->Z.active) for (i = 0; i < io.skip; i++) fread ((void *)&c, sizeof (char), 1, API->object[API->current_item[GMT_IN]]->fp);

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
	
		n_read++;
		if (Ctrl->Z.active) {	/* Read separately because of all the possible formats */
			ij++;
			if (ij == io.n_expected) {
				GMT_report (GMT, GMT_MSG_FATAL, "More than %ld records, only %ld was expected (aborting)!\n", ij, io.n_expected);
				Return (EXIT_FAILURE);
			}
			gmt_ij = io.get_gmt_ij (&io, Grid, ij);	/* Convert input order to output node (with padding) as per -Z */
			Grid->data[gmt_ij] = (Ctrl->N.active && in[zcol] == Ctrl->N.value) ? GMT->session.f_NaN : (float)in[zcol];
		}
		else {	/* Get x, y, z */
			if (GMT_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
			if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

			/* Ok, we are inside the region - process data */

			col = GMT_grd_x_to_col (GMT, in[GMT_X], Grid->header);
			if (col == -1) col++, n_confused++;
			if (col == Grid->header->nx) col--, n_confused++;
			row = GMT_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
			if (row == -1) row++, n_confused++;
			if (row == Grid->header->ny) row--, n_confused++;
			ij = GMT_IJ0 (Grid->header, row, col);	/* Because padding is turned off we can use ij for both Grid and flag */
			if (first_last == -1) {	/* Want the first value to matter only */
				if (flag[ij] == 0) {	/* Assign first value and that is the end of it */
					Grid->data[ij] = (float)in[zcol];
					flag[ij] = 1;
				}
			}
			else if (first_last == +1) {	/* Want the last value to matter only */
				Grid->data[ij] = (float)in[zcol];	/* Assign last value and that is it */
				flag[ij] = 1;
			}
			if (high_low) {	/* Always come here if looking for extreme values */
				if (flag[ij]) {	/* Already assigned the first value */
					if ((high_low == -1 && (in[zcol] < (double)Grid->data[ij])) || (high_low == +1 && (in[zcol] > (double)Grid->data[ij]))) Grid->data[ij] = (float)in[zcol];
				}
				else {	/* First time, just assign the current value */
					Grid->data[ij] = (float)in[zcol];
					flag[ij] = 1;
				}
			}
			else { 	/* Add up in case we must average */
				Grid->data[ij] += (float)in[zcol];
				flag[ij]++;
			}
			n_used++;
		}
	}

	if (Ctrl->Z.active) {
		GMT->current.io.input = save;	/* Reset pointer */
		GMT->common.b.active[GMT_IN] = previous;	/* Reset binary */
		ij++;
		if (ij != io.n_expected) {	/* Input amount does not match expectations */
			GMT_report (GMT, GMT_MSG_FATAL, "Found %ld records, but %ld was expected (aborting)!\n", ij, io.n_expected);
			Return (EXIT_FAILURE);
		}
		GMT_check_z_io (GMT, &io, Grid);	/* This fills in missing periodic row or column */
	}
	else {	/* xyz data could have resulted in duplicates */
		if (GMT->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && GMT_360_RANGE (Grid->header->wesn[XHI], Grid->header->wesn[XLO]) && Grid->header->registration == GMT_GRIDLINE_REG) {	/* Make sure longitudes got replicated */
			GMT_LONG ij_west, ij_east, first_bad = TRUE;

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
					if (first_last) {	/* Trouble since we did not store when we added these points */
						if (first_bad) {
							GMT_report (GMT, GMT_MSG_NORMAL, "Using -Af|s with replicated longitude bins may give inaccurate values");
							first_bad = FALSE;
						}
					}
					if (high_low) {	/* Always come here if looking for extreme values */
						if ((high_low == -1 && (Grid->data[ij_east] < Grid->data[ij_west])) || (high_low == +1 && (Grid->data[ij_east] > Grid->data[ij_west]))) Grid->data[ij_west] = Grid->data[ij_east];
					}
					else { 	/* Add up incase we must average */
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
				if (count) Grid->data[ij] = 1.0;
				n_filled++;
			}
			else if (flag[ij] == 0) {
				n_empty++;
				Grid->data[ij] = no_data_f;
			}
			else {	/* More than 1 value went to this node */
				if (count)
					Grid->data[ij] = (float)flag[ij];
				else if (average)
					Grid->data[ij] /= (float)flag[ij];
				/* implicit else means return the sum of the values */
				n_filled++;
				n_stuffed++;
			}
		}
		GMT_free (GMT, flag);
		
		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			char line[GMT_BUFSIZ];
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			GMT_report (GMT, GMT_MSG_NORMAL, " n_read: %ld  n_used: %ld  n_filled: %ld  n_empty: %ld set to ",
				n_read, n_used, n_filled, n_empty);
			(GMT_is_dnan (Ctrl->N.value)) ? GMT_report (GMT, GMT_MSG_NORMAL, "NaN\n") : GMT_report (GMT, GMT_MSG_NORMAL, line, Ctrl->N.value);
			if (n_bad) GMT_report (GMT, GMT_MSG_NORMAL, "%ld records unreadable\n", n_bad);
			if (n_stuffed && !count) GMT_report (GMT, GMT_MSG_NORMAL, "Warning - %ld nodes had multiple entries that were processed\n", n_stuffed);
			if (n_confused) GMT_report (GMT, GMT_MSG_NORMAL, "Warning - %ld values gave bad indices: Pixel vs gridline confusion?\n", n_confused);
		}
	}

	GMT_grd_pad_on (GMT, Grid, GMT->current.io.pad);	/* Restore padding */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	Return (GMT_OK);
}
