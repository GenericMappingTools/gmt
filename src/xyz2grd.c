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
 * Brief synopsis: xyz2grd reads a xyz file from standard input and creates the
 * corresponding grd-file. The input file does not have to be
 * sorted. xyz2grd will report if some nodes are missing.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"xyz2grd"
#define THIS_MODULE_MODERN_NAME	"xyz2grd"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert data table to a grid"
#define THIS_MODULE_KEYS	"<D{,SD),GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:JRVbdefhiqrs" GMT_OPT("FH")

struct XYZ2GRD_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A[f|l|n|m|r|s|u|z] */
		bool active;
		char mode;
	} A;
	struct D {	/* -D[+x<xname>][+yyname>][+z<zname>][+s<scale>][+ooffset>][+n<invalid>][+t<title>][+r<remark>] */
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
	struct S {	/* -S */
		bool active;
		char *file;
	} S;
	struct GMT_PARSE_Z_IO Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZ2GRD_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct XYZ2GRD_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->Z.type = 'a';
	C->Z.format[0] = 'T';	C->Z.format[1] = 'L';
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct XYZ2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.information);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->S.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -G<outgrid> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A[d|f|l|m|n|r|S|s|u|z]]\n\t[%s]\n", GMT_Rgeo_OPT, GMT_GRDEDIT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S[<zfile]] [%s] [-Z[<flags>]] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_J_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets name of the output grid file.\n");
	GMT_Option (API, "IR");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Determine what to do if multiple entries are found for a node:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ad: Compute the range (between min and max) of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Af: Keep first value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Al: Keep lower (minimum) value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Am: Compute mean of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -An: Count number of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ar: Compute RMS of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -AS: Compute standard deviation of multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -As: Keep last value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Au: Keep upper (maximum) value if multiple entries per node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Az: Sum multiple entries at the same node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will compute mean values].\n");
	gmt_grd_info_syntax (API->GMT, 'D');
	GMT_Option (API, "J");
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
	GMT_Message (API, GMT_TIME_NONE, "\t     A  ASCII (multiple floating point values per record).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a  ASCII (one value per record).\n");
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
	GMT_Option (API, "bi3,di");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Also sets value for nodes without input xyz triplet [Default is NaN].\n");
	GMT_Option (API, "e,f,h,i,qi,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

EXTERN_MSC unsigned int gmt_parse_d_option (struct GMT_CTRL *GMT, char *arg);

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct XYZ2GRD_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {
	/* This parses the options provided to xyz2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	uint64_t n_req;
	bool do_grid, b_only = false;
	char *ptr_to_arg = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	memset (io, 0, sizeof(struct GMT_Z_IO)); /* Initialize with zero */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				if (!opt->arg[0] && gmt_M_compat_check (GMT, 4)) {	/* In GMT4, just -A implied -Az */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -A is deprecated; use -Az instead.\n");
					Ctrl->A.active = true;
					Ctrl->A.mode = 'z';
				}
				else if (!strchr ("dflmnrSsuz", opt->arg[0])) {
					GMT_Report (API, GMT_MSG_ERROR, "Select -Ad, -Af, -Al, -Am, -An, -Ar, -AS, -As, -Au, or -Az\n");
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
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -E is deprecated; use grdconvert instead.\n");
					Ctrl->E.active = true;
					if (opt->arg[0]) {
						Ctrl->E.nodata = atof (opt->arg);
						Ctrl->E.set = true;
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'N':
				if (gmt_M_compat_check (GMT, 4)) {	/* Honor old -N<value> option */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N is deprecated; use GMT common option -di<nodata> instead.\n");
					if (opt->arg[0]) {
						char arg[GMT_LEN64] = {""};
						sprintf (arg, "i%s", opt->arg);
						n_errors += gmt_parse_d_option (GMT, arg);
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -N: Must specify value or NaN\n");
						n_errors++;
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
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
				n_errors += gmt_default_error (GMT, opt->option);
				if (opt->option == 'b') b_only = true;
				break;
		}
	}

	if (Ctrl->Z.active) {
		if (Ctrl->S.active) Ctrl->Z.not_grid = true;	/* The row/col organization does not apply */
		n_errors += gmt_parse_z_io (GMT, ptr_to_arg, &Ctrl->Z);
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !Ctrl->Z.active, "Option -S: Must also specify -Z\n");
	if (Ctrl->S.active) {	/* Reading and writing binary file */
		if (n_files > 1) {
			GMT_Report (API, GMT_MSG_ERROR, "-S can only handle one input file\n");
			n_errors++;
		}
		else {
			strcpy (GMT->current.io.r_mode, "rb");
			strcpy (GMT->current.io.w_mode, "wb");
		}
		Ctrl->Z.swab = 1;	/* Only swap on input */
	}

	if (Ctrl->Z.active) {
		gmt_init_z_io (GMT, Ctrl->Z.format, Ctrl->Z.repeat, Ctrl->Z.swab, Ctrl->Z.skip, Ctrl->Z.type, io);
		GMT->common.b.type[GMT_IN] = Ctrl->Z.type;
		if (b_only) {
			GMT->common.b.active[GMT_IN] = false;
			GMT_Report (API, GMT_MSG_ERROR, "-Z overrides -bi\n");
		}
	}

	do_grid = !(Ctrl->S.active || Ctrl->E.active);
	if (do_grid) {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	}
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active && !(Ctrl->G.active || Ctrl->G.file), "Option -G: Must specify output file\n");
	n_req = (Ctrl->Z.active) ? 1 : ((Ctrl->A.mode == 'n') ? 2 : 3);
	n_errors += gmt_check_binary_io (GMT, n_req);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL void protect_J(struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {
	if (GMT_Find_Option (API, 'J', options) != NULL) {
#ifdef HAVE_GDAL
		struct GMT_OPTION *opt = GMT_Make_Option (API, 'f', "0f,1f");
		(void)GMT_Append_Option(API, opt, options);
#else
		GMT_Report(API, GMT_MSG_ERROR,
		           "-J option to set grid's referencing system is only available when GMT was build with GDAL\n");
#endif
	}
}

int GMT_xyz2grd (void *V_API, int mode, void *args) {
	bool previous_bin_i = false, previous_bin_o = false;
	int error = 0, scol, srow;
	unsigned int zcol, row, col, i, *flag = NULL, n_min = 1;
	uint64_t n_empty = 0, n_confused = 0;
	uint64_t ij, ij_gmt, n_read, n_filled = 0, n_used = 0, n_req;

	char c, Amode;

	double *in = NULL, wesn[4];

	gmt_grdfloat no_data_f, *data = NULL;

	void * (*save_i) (struct GMT_CTRL *, FILE *, uint64_t *, int *) = NULL;
	int (*save_o) (struct GMT_CTRL *, FILE *, uint64_t, double *, char *);

	struct GMT_GRID *Grid = NULL;
	struct GMT_Z_IO io;
	struct GMT_RECORD *In = NULL;
	struct XYZ2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	protect_J(API, options);	/* If -J is used, add a -f0f,1f option to avoid later parsing errors due to -R & -J conflicts */
	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, &io, options)) != 0) Return (error);

	/*---------------------------- This is the xyz2grd main code ----------------------------*/

	n_req = (Ctrl->Z.active) ? 1 : ((Ctrl->A.mode == 'n') ? 2 : 3);	/* Required input columns */

	if (Ctrl->S.active) {	/* Just swap data and bail */
		int out_ID;
		unsigned w_mode = GMT_ADD_DEFAULT;

		save_i = GMT->current.io.input;			/* Save previous i/0 parameters */
		save_o = GMT->current.io.output;
		previous_bin_i = GMT->common.b.active[GMT_IN];
		previous_bin_o = GMT->common.b.active[GMT_OUT];
		GMT->current.io.input = gmt_z_input;		/* Override input reader with chosen binary reader for selected type */
		GMT->current.io.output = gmt_z_output;		/* Override output writer with chosen binary writer for selected type */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set input binary as well */
		GMT->common.b.active[GMT_OUT] = io.binary;	/* May have to set output binary as well */
		if ((error = GMT_Set_Columns (API, GMT_IN, 1, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);
		/* Initialize the i/o since we are doing record-by-record reading/writing */
		GMT_Report (API, GMT_MSG_INFORMATION, "Swapping data bytes only\n");
		if (Ctrl->S.active) io.swab = true;	/* Need to pass swabbing down to the gut level */

		/* Register the data source */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set via file */
			Return (API->error);
		}
		if (Ctrl->S.file) {	/* Specified an output file */
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_OUT, NULL, Ctrl->S.file)) == GMT_NOTSET) {
				Return (API->error);
			}
			w_mode = GMT_ADD_EXISTING;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, w_mode, 0, options) != GMT_NOERROR) {	/* Establishes data output to stdout */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_IN, 1, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {	/* We don't really care or know about columns so must use 1 */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 1, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {	/* We don't really care or know about columns so must use 1 */
			Return (API->error);
		}
		/* Initialize the i/o for doing record-by-record reading/writing */
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all headers */
					continue;
				if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				assert (false);						/* Should never get here */
			}

			/* Data record to process */

			GMT_Put_Record (API, GMT_WRITE_DATA, In);
		} while (true);

		GMT->current.io.input = save_i;			/* Reset input pointer */
		GMT->common.b.active[GMT_IN] = previous_bin_i;	/* Reset input binary */
		GMT->current.io.output = save_o;		/* Reset output pointer */
		GMT->common.b.active[GMT_OUT] = previous_bin_o;	/* Reset output binary */

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		Return (GMT_NOERROR);	/* We are done here */
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	/* PW: This is now done in grdconvert since ESRI Arc Interchange is a recognized format */
	if (Ctrl->E.active && gmt_M_compat_check (GMT, 4)) {	/* Read an ESRI Arc Interchange grid format in ASCII.  This must be a single physical file. */
		uint64_t n_left;
		double value;
		char line[GMT_BUFSIZ];
		FILE *fp = GMT->session.std[GMT_IN];

		if (Ctrl->In.file && (fp = gmt_fopen (GMT, Ctrl->In.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->In.file);
			Return (GMT_ERROR_ON_FOPEN);
		}

		if ((Grid = gmt_create_grid (GMT)) == NULL) Return (API->error);
		gmt_grd_init (GMT, Grid->header, options, false);
		Grid->header->registration = GMT_GRID_NODE_REG;
		gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->n_columns) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not decode ncols record\n");
			Return (GMT_DATA_READ_ERROR);
		}
		gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %d", &Grid->header->n_rows) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not decode ncols record\n");
			Return (GMT_DATA_READ_ERROR);
		}
		gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[XLO]) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not decode xll record\n");
			Return (GMT_DATA_READ_ERROR);
		}
		if (!strncmp (line, "xllcorner", 9U)) Grid->header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
		gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->wesn[YLO]) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not decode yll record\n");
			Return (GMT_DATA_READ_ERROR);
		}
		if (!strncmp (line, "yllcorner", 9U)) Grid->header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
		gmt_fgets (GMT, line, GMT_BUFSIZ, fp);
		if (sscanf (line, "%*s %lf", &Grid->header->inc[GMT_X]) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not decode cellsize record\n");
			Return (GMT_DATA_READ_ERROR);
		}
		Grid->header->inc[GMT_Y] = Grid->header->inc[GMT_X];
		Grid->header->xy_off = 0.5 * Grid->header->registration;
		Grid->header->wesn[XHI] = Grid->header->wesn[XLO] + (Grid->header->n_columns - 1 + Grid->header->registration) * Grid->header->inc[GMT_X];
		Grid->header->wesn[YHI] = Grid->header->wesn[YLO] + (Grid->header->n_rows - 1 + Grid->header->registration) * Grid->header->inc[GMT_Y];
		gmt_set_grddim (GMT, Grid->header);
		gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, Grid->header, 1), Ctrl->G.file);

		GMT_Report (API, GMT_MSG_INFORMATION, "n_columns = %d  n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);
		n_left = Grid->header->nm;

		Grid->data = gmt_M_memory_aligned (GMT, NULL, Grid->header->nm, gmt_grdfloat);
		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids */
		row = col = 0;
		if (fscanf (fp, "%s", line) != 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read nodata-flag or first data record from ESRI file\n");
			Return (GMT_DATA_READ_ERROR);
		}
		gmt_str_tolower (line);
		if (!strcmp (line, "nodata_value")) {	/* Found the optional nodata word */
			if (fscanf (fp, "%lf", &value) != 1) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to parse nodata-flag from ESRI file\n");
				Return (GMT_DATA_READ_ERROR);
			}
			if (Ctrl->E.set && !doubleAlmostEqualZero (value, Ctrl->E.nodata)) {
				GMT_Report (API, GMT_MSG_ERROR, "Your -E%g overrides the nodata_value of %g found in the ESRI file\n", Ctrl->E.nodata, value);
			}
			else
				Ctrl->E.nodata = value;
		}
		else {	/* Instead got the very first data value */
			ij = gmt_M_ijp (Grid->header, row, col);
			value = atof (line);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (gmt_grdfloat) value;
			if (++col == Grid->header->n_columns) col = 0, row++;
			n_left--;
		}
		while (fscanf (fp, "%lf", &value) == 1 && n_left) {
			ij = gmt_M_ijp (Grid->header, row, col);
			Grid->data[ij] = (value == Ctrl->E.nodata) ? GMT->session.f_NaN : (gmt_grdfloat) value;
			if (++col == Grid->header->n_columns) col = 0, row++;
			n_left--;
		}
		gmt_fclose (GMT, fp);
		if (n_left) {
			GMT_Report (API, GMT_MSG_ERROR, "Expected %" PRIu64 " points, found only %" PRIu64 "\n", Grid->header->nm, Grid->header->nm - n_left);
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* Here we will read either x,y,z or z data, using -R -I [-r] for sizeing */

	no_data_f = (GMT->common.d.active[GMT_IN]) ? (gmt_grdfloat)GMT->common.d.nan_proxy[GMT_IN] : GMT->session.f_NaN;

	/* Set up and allocate output grid [note: zero padding specified since no BCs required] */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, 0, NULL)) == NULL) Return (API->error);

	/* See if we have a projection info to add */
	if (GMT->common.J.active)		/* Convert the GMT -J<...> into a proj4 string and save it in the header */
		Grid->header->ProjRefPROJ4 = gmt_export2proj4(GMT);

	Amode = Ctrl->A.active ? Ctrl->A.mode : 'm';

	/* For Amode = 'd' or 'S' we need a second grid, and also require a minimum of 2 points per grid */
	if (Amode == 'd' || Amode == 'S') {
		data = gmt_M_memory_aligned (GMT, NULL, Grid->header->nm, gmt_grdfloat);
		n_min = 2;
	}

	if (GMT->common.b.active[GMT_IN] && gmt_M_type (GMT, GMT_IN, GMT_Z) & GMT_IS_RATIME && GMT->current.io.fmt[GMT_IN][GMT_Z].type == GMT_FLOAT) {
		GMT_Report (API, GMT_MSG_WARNING, "Your single precision binary input data are unlikely to hold absolute time coordinates without serious truncation.\n");
		GMT_Report (API, GMT_MSG_WARNING, "You must use double precision when storing absolute time coordinates in binary data tables.\n");
	}

	if (Ctrl->D.active && gmt_decode_grd_h_info (GMT, Ctrl->D.information, Grid->header)) {
		gmt_M_free (GMT, data);
		Return (GMT_PARSE_ERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "n_columns = %d  n_rows = %d  nm = %" PRIu64 "  size = %" PRIuS "\n", Grid->header->n_columns, Grid->header->n_rows, Grid->header->nm, Grid->header->size);

	gmt_M_err_fail (GMT, gmt_set_z_io (GMT, &io, Grid), Ctrl->G.file);

	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	if (Ctrl->Z.active && GMT->common.d.active[GMT_IN] && gmt_M_is_fnan (no_data_f)) GMT->common.d.active[GMT_IN] = false;	/* No point testing since nan_proxy is NaN... */

	if (Ctrl->Z.active) {	/* Need to override input method since reading single input column as z (not x,y) */
		zcol = GMT_X;
		save_i = GMT->current.io.input;
		previous_bin_i = GMT->common.b.active[GMT_IN];
		GMT->current.io.input = gmt_z_input;		/* Override and use chosen input mode */
		GMT->common.b.active[GMT_IN] = io.binary;	/* May have to set binary as well */
		GMT->current.io.fmt[GMT_IN][zcol].type = gmt_get_io_type (GMT, Ctrl->Z.type);
	}
	else {
		zcol = GMT_Z;
		flag = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* No padding needed for flag array */
		gmt_M_memset (Grid->header->pad, 4, unsigned int);	/* Algorithm below expects no padding; we repad at the end */
		GMT->current.setting.io_nan_records = false;	/* Cannot have x,y as NaNs here */
	}

	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_req, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		gmt_M_free (GMT, data);
		gmt_M_free (GMT, flag);
		Return (error);
	}
	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		gmt_M_free (GMT, data);
		gmt_M_free (GMT, flag);
		Return (API->error);	/* Establishes data input */
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {
		gmt_M_free (GMT, data);		gmt_M_free (GMT, flag);
		Return (API->error);	/* Enables data input and sets access mode */
	}

	n_read = ij = 0;
	if (Ctrl->Z.active) {
		size_t nr = 0;
		if (Ctrl->Z.swab) GMT_Report (API, GMT_MSG_INFORMATION, "Binary input data will be byte swapped\n");
		for (i = 0; i < io.skip; i++)
			nr += fread (&c, sizeof (char), 1, API->object[API->current_item[GMT_IN]]->fp);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				gmt_M_free (GMT, data);		gmt_M_free (GMT, flag);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */
		n_read++;
		in = In->data;	/* Only need to process numerical part here */

		if (Ctrl->Z.active) {	/* Read separately because of all the possible formats */
			if (ij == io.n_expected) {
				GMT_Report (API, GMT_MSG_ERROR, "More than %" PRIu64 " records, only %" PRIu64 " was expected (aborting)!\n", ij, io.n_expected);
				GMT_Report (API, GMT_MSG_ERROR, "(You are probably misterpreting xyz2grd with an interpolator; see 'surface' man page)\n");
				gmt_M_free (GMT, data);		gmt_M_free (GMT, flag);
				Return (GMT_RUNTIME_ERROR);
			}
			ij_gmt = io.get_gmt_ij (&io, Grid, ij);	/* Convert input order to output node (with padding) as per -Z */
			Grid->data[ij_gmt] = (gmt_input_is_nan_proxy (GMT, in[zcol])) ? GMT->session.f_NaN : (gmt_grdfloat)in[zcol];
			ij++;
		}
		else {	/* Get x, y, z */
			if (gmt_M_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
			if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

			/* Ok, we are inside the region - process data */

			scol = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);
			if (scol == -1) scol++, n_confused++;
			col = scol;
			if (col == Grid->header->n_columns) col--, n_confused++;
			srow = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
			if (srow == -1) srow++, n_confused++;
			row = srow;
			if (row == Grid->header->n_rows) row--, n_confused++;
			ij = gmt_M_ij0 (Grid->header, row, col);	/* Because padding is turned off we can use ij for both Grid and flag */
			switch (Amode) {
				case 'f':	/* Want the first value to matter only */
					if (flag[ij] == 0) {	/* Assign first value and that is the end of it */
						Grid->data[ij] = (gmt_grdfloat)in[zcol];
						flag[ij] = (unsigned int)n_read;
					}
					break;
				case 's':	/* Want the last value to matter only */
					Grid->data[ij] = (gmt_grdfloat)in[zcol];	/* Assign last value and that is it */
					flag[ij] = (unsigned int)n_read;
					break;
				case 'l':	/* Keep lowest value */
					if (flag[ij]) {	/* Already assigned the first value */
						if (in[zcol] < (double)Grid->data[ij]) Grid->data[ij] = (gmt_grdfloat)in[zcol];
					}
					else {	/* First time, just assign the current value */
						Grid->data[ij] = (gmt_grdfloat)in[zcol];
					}
					flag[ij]++;
					break;
				case 'u':	/* Keep highest value */
					if (flag[ij]) {	/* Already assigned the first value */
						if (in[zcol] > (double)Grid->data[ij]) Grid->data[ij] = (gmt_grdfloat)in[zcol];
					}
					else {	/* First time, just assign the current value */
						Grid->data[ij] = (gmt_grdfloat)in[zcol];
					}
					flag[ij]++;
					break;
				case 'd':	/* Keep highest and lowest value */
					if (flag[ij]) {	/* Already assigned the first value */
						if (in[zcol] > (double)Grid->data[ij]) Grid->data[ij] = (gmt_grdfloat)in[zcol];
						if (in[zcol] < (double)data[ij]) data[ij] = (gmt_grdfloat)in[zcol];
					}
					else {	/* First time, just assign the current value */
						Grid->data[ij] = data[ij] = (gmt_grdfloat)in[zcol];
					}
					flag[ij]++;
					break;
				case 'S': 	/* Add up squares and means to compute standard deviation */
					data[ij] += (gmt_grdfloat)in[zcol];	/* This adds up the means; we fall through to next case on purpose to also add up squares */
				case 'r': 	/* Add up squares in case we must rms */
					Grid->data[ij] += (gmt_grdfloat)in[zcol] * (gmt_grdfloat)in[zcol];
					flag[ij]++;
					break;
				default:	/* Add up in case we must sum or mean */
					Grid->data[ij] += (gmt_grdfloat)in[zcol];
					flag[ij]++;
					break;
			}
			n_used++;
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		gmt_M_free (GMT, flag);		gmt_M_free (GMT, data);
		Return (API->error);
	}

	if (Ctrl->Z.active) {
		GMT->current.io.input = save_i;	/* Reset pointer */
		GMT->common.b.active[GMT_IN] = previous_bin_i;	/* Reset binary */
		if (ij != io.n_expected) {	/* Input amount does not match expectations */
			GMT_Report (API, GMT_MSG_ERROR, "Found %" PRIu64 " records, but %" PRIu64 " was expected (aborting)!\n", ij, io.n_expected);
				GMT_Report (API, GMT_MSG_ERROR, "(You are probably misterpreting xyz2grd with an interpolator; see 'surface' man page)\n");
			gmt_M_free (GMT, flag);		gmt_M_free (GMT, data);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_check_z_io (GMT, &io, Grid);	/* This fills in missing periodic row or column */
	}
	else {	/* xyz data could have resulted in duplicates */
		if (gmt_M_grd_duplicate_column (GMT, Grid->header, GMT_IN)) {	/* Make sure longitudes got replicated */
			uint64_t ij_west, ij_east;

			for (row = 0; row < Grid->header->n_rows; row++) {	/* For each row, look at west and east bin */
				ij_west = gmt_M_ij0 (Grid->header, row, 0);
				ij_east = gmt_M_ij0 (Grid->header, row, Grid->header->n_columns - 1);

				if (flag[ij_west] && !flag[ij_east]) {		/* Nothing in east bin, just copy from west */
					Grid->data[ij_east] = Grid->data[ij_west];
					flag[ij_east] = flag[ij_west];
					if (n_min == 2) data[ij_east] = data[ij_west];
				}
				else if (flag[ij_east] && !flag[ij_west]) {	/* Nothing in west bin, just copy from east */
					Grid->data[ij_west] = Grid->data[ij_east];
					flag[ij_west] = flag[ij_east];
					if (n_min == 2) data[ij_west] = data[ij_east];
				}
				else {	/* Both have some stuff, consolidate combined value into the west bin, then replicate to the east */
					switch (Amode) {
						case 'f':	/* Keep the first */
							if (flag[ij_east] < flag[ij_west]) Grid->data[ij_west] = Grid->data[ij_east], flag[ij_west] = flag[ij_east];
							break;
						case 's':	/* Keep the last */
							if (flag[ij_east] > flag[ij_west]) Grid->data[ij_west] = Grid->data[ij_east], flag[ij_west] = flag[ij_east];
							break;
						case 'l':	/* Keep the lowest */
							if (Grid->data[ij_east] < Grid->data[ij_west]) Grid->data[ij_west] = Grid->data[ij_east];
							flag[ij_west] += flag[ij_east];
							break;
						case 'd':	/* Keep the lowest in 'data' */
							if (data[ij_east] < data[ij_west]) data[ij_west] = data[ij_east];
							/* Fall through on purpose since range also needs the highsets */
						case 'u':	/* Keep the highest */
							if (Grid->data[ij_east] > Grid->data[ij_west]) Grid->data[ij_west] = Grid->data[ij_east];
							flag[ij_west] += flag[ij_east];
							break;
						case 'S':	/* Sum up the sums in 'data' */
							data[ij_west] += data[ij_east];
							/* Fall through on purpose */
						default:	/* Add up in case we must sum, rms, mean, or standard deviation */
							Grid->data[ij_west] += Grid->data[ij_east];
							flag[ij_west] += flag[ij_east];
							break;
					}
					/* Replicate: */
					Grid->data[ij_east] = Grid->data[ij_west];
					flag[ij_east] = flag[ij_west];
				}
			}
		}

		for (ij = 0; ij < Grid->header->nm; ij++) {	/* Check if all nodes got one value only */
			if (flag[ij] < n_min) {	/* Cells are not filled enough */
				n_empty++;
				Grid->data[ij] = no_data_f;
			}
			else {	/* Enough values went to this node */
				if (Amode == 'n')
					Grid->data[ij] = (gmt_grdfloat)flag[ij];
				else if (Amode == 'm')
					Grid->data[ij] /= (gmt_grdfloat)flag[ij];
				else if (Amode == 'r')
					Grid->data[ij] = (gmt_grdfloat)sqrt (Grid->data[ij] / (gmt_grdfloat)flag[ij]);
				else if (Amode == 'd')
					Grid->data[ij] -= data[ij];
				else if (Amode == 'S') {
					Grid->data[ij] = (gmt_grdfloat)sqrt ((Grid->data[ij] - data[ij] * data[ij] / (gmt_grdfloat)flag[ij]) / (gmt_grdfloat)(flag[ij] - 1));

				}
				/* Implicit else means return the currently stored value */
				n_filled++;
			}
		}
		gmt_M_free (GMT, flag);
		gmt_M_free (GMT, data);

		if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) {
			char line[GMT_BUFSIZ], e_value[GMT_LEN32];
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			(GMT->common.d.active[GMT_IN]) ? sprintf (e_value, GMT->current.setting.format_float_out, GMT->common.d.nan_proxy[GMT_IN]) : sprintf (e_value, "NaN");
			GMT_Report (API, GMT_MSG_INFORMATION, "Data records read: %" PRIu64 "  used: %" PRIu64 "  nodes filled: %" PRIu64 " nodes empty: %" PRIu64 " [set to %s]\n",
				n_read, n_used, n_filled, n_empty, e_value);
			if (n_confused) GMT_Report (API, GMT_MSG_WARNING, "%" PRIu64 " values gave bad indices: Pixel vs Gridline registration confusion?\n", n_confused);
		}
	}

	gmt_grd_pad_on (GMT, Grid, GMT->current.io.pad);	/* Restore padding */
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
		gmt_M_free (GMT, flag);		gmt_M_free (GMT, data);
		Return (API->error);
	}
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		gmt_M_free (GMT, flag);		gmt_M_free (GMT, data);
		Return (API->error);
	}

	gmt_M_free (GMT, flag);		gmt_M_free (GMT, data);
	Return (GMT_NOERROR);
}
