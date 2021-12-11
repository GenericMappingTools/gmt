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
 * Brief synopsis: sample1d reads a 1-D dataset, and resamples the values on (1) user
 * supplied time values or (2) equidistant time-values based on <timestart> and
 * <dt>, both supplied at the command line. Choose among linear, cubic
 * spline, and Akima's spline.  sample1d will handle multiple column files,
 * user must choose which column contains the independent, monotonically
 * increasing variable.
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"sample1d"
#define THIS_MODULE_MODERN_NAME	"sample1d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Resample 1-D table data using splines"
#define THIS_MODULE_KEYS	"<D{,ND(,TD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->Vbdefghijoqsw" GMT_OPT("HMm")

#define INT_1D_CART	0	/* Regular 1-D interpolation */
#define INT_2D_CART	1	/* Cartesian 2-D path interpolation */
#define INT_2D_GEO	2	/* Spherical 2-D path interpolation */

struct SAMPLE1D_CTRL {
	struct SAMPLE1D_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct SAMPLE1D_A {	/* -A[f|m|p|r|R|l][+d][+l] */
		bool active, loxo, delete;
		enum GMT_enum_track mode;
	} A;
	struct SAMPLE1D_E {	/* -E */
		bool active;
	} E;
	struct SAMPLE1D_F {	/* -Fl|a|c|n|s<p>[+d1|2] */
		bool active;
		unsigned int mode;
		unsigned int type;
		double fit;
	} F;
	struct SAMPLE1D_N {	/* -N<time_col> */
		bool active;
		unsigned int col;
	} N;
	struct SAMPLE1D_T {	/* -T<tmin/tmax/tinc>[+a|n] */
		bool active;
		struct GMT_ARRAY T;
	} T;
	struct SAMPLE1D_W {	/* -W<w_col> */
		bool active;
		unsigned int col;
	} W;
	/* Deprecated options in GMT 6 now handled by -T */
	/* -I<inc>[d|m|s|e|f|k|M|n|u|c] (c means x/y Cartesian path) */
	/* -N<knotfile> */
	/* -S<xstart>[/<xstop>] */
	/* -T<time_col> */
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SAMPLE1D_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct SAMPLE1D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.mode = GMT->current.setting.interpolant;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct SAMPLE1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	char type[3] = {'l', 'a', 'c'};

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-A[f|m|p|r|R][+d][+l]] [-E] [-Fl|a|c|n|s<p>[+d1|2]] [-N<time_col>] "
		"[-T[<min>/<max>/]<inc>[+i|n][+a][+t][+u]] [%s] [-W<w_col>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_j_OPT,
		GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_w_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Usage (API, -1, "Note: The independent variable column (see -N) must be monotonically in/de-creasing.");
	GMT_Message (API, GMT_TIME_NONE, "\n  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A[f|m|p|r|R][+d][+l]");
	GMT_Usage (API, -2, "Control how input track in <table> is resampled when increment has a unit appended:");
	GMT_Usage (API, 3, "f: Keep original points, but add intermediate points if needed [Default].");
	GMT_Usage (API, 3, "m: Same, but first follow meridian (along y) then parallel (along x).");
	GMT_Usage (API, 3, "p: Same, but first follow parallel (along x) then meridian (along y).");
	GMT_Usage (API, 3, "r: Resample at equidistant locations; input points not necessarily included.");
	GMT_Usage (API, 3, "R: Same, but adjust given spacing to fit the track length exactly.");
	GMT_Usage (API, -2, "Optional modifiers:");
	GMT_Usage (API, 3, "+d Skip records that has no increase in <time_col> value [no skipping].");
	GMT_Usage (API, 3, "+l Compute distances along rhumblines (loxodromes) [no].");
	GMT_Usage (API, -2, "Note: +l uses spherical calculations - cannot be combined with -je.");
	GMT_Usage (API, 1, "\n-E Add input data trailing text to output records when possible [Ignore trailing text].");
	GMT_Usage (API, 1, "\n-Fl|a|c|n|s<p>[+d1|2]");
	GMT_Usage (API, -2, "Set the interpolation mode.  Choose from:");
	GMT_Usage (API, 3, "l: Linear interpolation.");
	GMT_Usage (API, 3, "a: Akima spline interpolation.");
	GMT_Usage (API, 3, "c: Cubic spline interpolation.");
	GMT_Usage (API, 3, "n: No interpolation (nearest point).");
	GMT_Usage (API, 3, "s: Smooth spline interpolation (append fit parameter <p>).");
	GMT_Usage (API, -2, "Optionally, request a spline derivative via a modifier:");
	GMT_Usage (API, 3, "+d Append 1 for 1st derivative or 2 for 2nd derivative.");
	GMT_Usage (API, -2, "[Default is -F%c].", type[API->GMT->current.setting.interpolant]);
	GMT_Usage (API, 1, "\n-N<time_col>");
	GMT_Usage (API, -2, "Give column number of the independent variable (time) [Default is 0 (first)].");
	GMT_Usage (API, 1, "\n-T[<min>/<max>/]<inc>[+i|n][+a][+t][+u]");
	GMT_Usage (API, -2, "Make evenly spaced output time steps from <min> to <max> by <inc>. "
		"For absolute time resampling, append a valid time unit (%s) to the increment and add +t. "
		"For spatial resampling with distance computed from the first two columns, specify increment as "
		"<inc> and append a geospatial distance unit (%s) or c (for Cartesian distances). "
		"See -A to control how the spatial resampling is done.", GMT_TIME_UNITS_DISPLAY, GMT_LEN_UNITS_DISPLAY);
	GMT_Usage (API, 3, "+a Add any internally computed distances as a final output column [no distances added].");
	GMT_Usage (API, 3, "+n Indicate <inc> is the number of t-values to produce over the range instead.");
	GMT_Usage (API, 3, "+i Indicate <inc> is the reciprocal of desired <inc> (e.g., 3 for 0.3333.....).");
	GMT_Usage (API, -2, "Alternatively, <inc> is a file with output times in the first column, or a comma-separated list.");
	GMT_Usage (API, 3, "+u Ensure unique and sorted entries in the array, eliminating duplicates.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W<w_col>");
	GMT_Usage (API, -2, "Give column number of weights for smoothing spline (requires -Fs) [no weights].");
	GMT_Option (API, "bi2,bo,d,e,f,g,h,i,j,o,q,s,w,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct SAMPLE1D_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to sample1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int col;
	bool old_syntax = false;
	char string[GMT_LEN64] = {""};
	char *i_arg = NULL, *s_arg = NULL, *t_arg = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	/* Determine if we are using GMT 6 syntax with -T<tmin/tmax/tinc>[+a|n] vs old ways of using -N -I -S -T<col> */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case 'I':	/* Always indicative of old option */
				old_syntax = true;
				break;
			case 'N':	/* Indicative of old option if a valid file is appended */
				if (!gmt_access (GMT, opt->arg, F_OK))
					old_syntax = true;
				break;
			case 'S':	/* Always indicative of old option */
				old_syntax = true;
				break;
			default:
				break;
		}
	}

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;
			case '>':	/* Got named output file */
				if (n_files++ > 0) { n_errors++; continue; }
				Ctrl->Out.active = true;
				if (opt->arg[0]) Ctrl->Out.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->Out.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change track resampling mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				if (opt->arg[0] != '+') {	/* Gave a mode */
					switch (opt->arg[0]) {
						case 'f': Ctrl->A.mode = GMT_TRACK_FILL;   break;
						case 'm': Ctrl->A.mode = GMT_TRACK_FILL_M; break;
						case 'p': Ctrl->A.mode = GMT_TRACK_FILL_P; break;
						case 'r': Ctrl->A.mode = GMT_TRACK_SAMPLE_FIX; break;
						case 'R': Ctrl->A.mode = GMT_TRACK_SAMPLE_ADJ; break;
						default: GMT_Report (API, GMT_MSG_ERROR, "Option -A: Bad modifier %c\n", opt->arg[0]); n_errors++; break;
					}
				}
				if (strstr (opt->arg, "+d")) Ctrl->A.delete = true;
				if (strstr (opt->arg, "+l")) Ctrl->A.loxo = true;		/* Note: spherical only */
				break;
			case 'E':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				break;

			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'l':
						Ctrl->F.mode = GMT_SPLINE_LINEAR;
						break;
					case 'a':
						Ctrl->F.mode = GMT_SPLINE_AKIMA;
						break;
					case 'c':
						Ctrl->F.mode = GMT_SPLINE_CUBIC;
						break;
					case 'n':
						Ctrl->F.mode = GMT_SPLINE_NN;
						break;
					case 's':
						Ctrl->F.mode = GMT_SPLINE_SMOOTH;
						if (opt->arg[1])
							Ctrl->F.fit = atof (&opt->arg[1]);
						else {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Fs: No fit parameter given\n");
							n_errors++;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (strstr (&opt->arg[1], "+d1")) Ctrl->F.type = 1;	/* Want first derivative */
				else if (strstr (&opt->arg[1], "+d2")) Ctrl->F.type = 2;	/* Want second derivative */
				else if (strstr (&opt->arg[1], "+1")) Ctrl->F.type = 1;	/* Want first derivative (backwards compatibility) */
				else if (strstr (&opt->arg[1], "+2")) Ctrl->F.type = 2;	/* Want second derivative (backwards compatibility) */
				break;
			case 'I':	/* Deprecated, but keep pointer to the arguments so we can build -T argument */
				i_arg = opt->arg;
				if (i_arg[0] == '+') i_arg++;	/* Skip possible geodesic mode */
				break;
			case 'N':
				if (!gmt_access (GMT, opt->arg, F_OK)) {
					if (gmt_M_compat_check (GMT, 5)) {
						GMT_Report (API, GMT_MSG_COMPAT, "-N<knotfile> is deprecated; use -T<knotfile> instead.\n");
						Ctrl->T.active = true;
						t_arg = opt->arg;
					}
					else
						n_errors += gmt_default_error (GMT, opt->option);
				}
				else if (opt->arg[0]) {
					col = atoi (opt->arg);
					n_errors += gmt_M_check_condition (GMT, col < 0, "Option -N: Column number cannot be negative\n");
					Ctrl->N.col = col;
					Ctrl->N.active = true;
				}
				else	/* Gave no argument */
					n_errors++;
				break;
			case 'S':	/* Deprecated, but keep pointers to the arguments so we can build -T argument */
				if (gmt_M_compat_check (GMT, 5)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-S<start>[/<stop>] option is deprecated; use -T<tmin/tmax/tinc>[+a|n] instead.\n");
					if (strchr (opt->arg, '/'))	/* Got start/stop */
						sprintf (string, "%s", opt->arg);
					else
						sprintf (string, "%s/-", opt->arg);
					s_arg = string;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);

				break;
			case 'T':
				if (old_syntax && gmt_M_compat_check (GMT, 5)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-T<col> option is deprecated; use -N<col> instead.\n");
					col = atoi (opt->arg);
					n_errors += gmt_M_check_condition (GMT, col < 0, "Option -T: Column number cannot be negative\n");
					Ctrl->N.col = col;
				}
				else {	/* Set output knots */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
					Ctrl->T.active = true;
					t_arg = opt->arg;
				}
				break;
			case 'W':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if (opt->arg[0]) {
					col = atoi (opt->arg);
					n_errors += gmt_M_check_condition (GMT, col < 0, "Option -W: Column number cannot be negative\n");
					Ctrl->W.col = col;
					Ctrl->W.active = true;
				}
				else	/* Gave no argument */
					n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (i_arg && !Ctrl->T.active) {	/* Combine -I [and maybe -Sstart/stop] into a temporary -T option and parse that */
		if (s_arg) {	/* Build -Tmin/max/inc */
			strcat (string, "/");
			strcat (string, i_arg);
		}
		else	/* Build -Tinc */
			sprintf (string, "%s", i_arg);
		t_arg = string;
		Ctrl->T.active = true;
	}
	if (Ctrl->T.active) {	/* Do this one here since we need Ctrl->N.col to be set first, if selected */
		n_errors += gmt_parse_array (GMT, 'T', t_arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_DIST | GMT_ARRAY_NOMINMAX, Ctrl->N.col);
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->A.loxo && GMT->common.j.mode == GMT_GEODESIC,
	                                   "Option -A+l: Requires spherical calculations so -je cannot be used\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && s_arg, "Specify only one of -N and -S\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->N.col >= 2) ? Ctrl->N.col + 1 : 2);
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Option -F: Only 1st or 2nd derivatives may be requested\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->F.mode != GMT_SPLINE_SMOOTH,
	                                   "Option -W: Only available with -Fs<p>\n");

	if (Ctrl->F.mode == GMT_SPLINE_SMOOTH && gmt_M_is_zero (Ctrl->F.fit))	/* Convenience check so -Fs0 is the same as -Fc. Place this hear to allow -W (which will be ignore) */
		Ctrl->F.mode = GMT_SPLINE_CUBIC;

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_sample1d (void *V_API, int mode, void *args) {
	unsigned int geometry, int_mode;
	int error = 0, result;

	unsigned char *nan_flag = NULL;

	uint64_t k, tbl, col, row, seg, m = 0, dim[GMT_DIM_SIZE] = {0, 0, 0, 0};

	double *t_out = NULL, *dist_in = NULL, *ttime = NULL, *data = NULL;
	double low_t, high_t, *lon = NULL, *lat = NULL, *weight = NULL;

	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_DATATABLE *Tout = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sout = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct SAMPLE1D_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the sample1d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	GMT->current.setting.interpolant = Ctrl->F.mode % 10;
	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;	/* Turn off default GMT NaN-handling for (x,y) which is not the case here */
	GMT->current.io.skip_if_NaN[Ctrl->N.col] = true;				/* ... But disallow NaN in "time" column */
	int_mode = gmt_set_interpolate_mode (GMT, Ctrl->F.mode, Ctrl->F.type);	/* What mode we pass to the interpolater */

	if (Ctrl->T.T.spatial) {
		if (gmt_M_is_cartesian (GMT, GMT_IN) && Ctrl->A.loxo) {
			GMT_Report (API, GMT_MSG_ERROR, "Loxodrome mode ignored for Cartesian data.\n");
			Ctrl->A.loxo = false;
		}
		if (Ctrl->A.loxo) GMT->current.map.loxodrome = true;
		GMT->hidden.sample_along_arc = true;	/* Ensure equidistant sampling along arc, not cord */
	}

	geometry = (Ctrl->T.T.spatial) ? GMT_IS_LINE : GMT_IS_NONE;
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}

	/* First read input data to be sampled */

	if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) Return (error);
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have no data column(s) but at least 1 is needed\n");
		Return (GMT_DIM_TOO_SMALL);
	}
	if (Din->n_columns < 2 && Ctrl->W.active) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least 2 are needed since -W is used\n", (int)Din->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	if (Ctrl->N.active && Ctrl->N.col >= Din->n_columns) {	/*  */
		GMT_Report (API, GMT_MSG_ERROR, "Requested time column is greater than data number of columns (%d)\n", (int)Din->n_columns);
		Return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->A.delete) {	/* Remove duplicate rows based on time column */
		uint64_t tcol = Ctrl->N.col;	/* The single time column */
		int64_t n_dup = gmt_eliminate_duplicates (API, Din, &tcol, 1, false);
		if (n_dup < 0) {
			Return (GMT_RUNTIME_ERROR);
		}
		else if (n_dup)
			GMT_Report (API, GMT_MSG_INFORMATION, "Removed %" PRId64 " records with no change in the time column\n", n_dup);
	}
	if (!Ctrl->T.active) {	/* Did not have information for -Tinc during parsing, do now */
		char string[GMT_LEN32] = {""};
		/* Get the first data increment as our fixed increment */
		double inc = Din->table[0]->segment[0]->data[Ctrl->N.col][1] - Din->table[0]->segment[0]->data[Ctrl->N.col][0];
		sprintf (string, "%g", inc);
		if (gmt_parse_array (GMT, 'T', string, &(Ctrl->T.T), 0, Ctrl->N.col))
			Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->T.T.file) {	/* Read file with abscissae */
		if (gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL))
			Return (GMT_RUNTIME_ERROR);
		GMT_Report (API, GMT_MSG_INFORMATION, "Read %" PRIu64 " knots from file\n", Ctrl->T.T.n);
	}

	dim[GMT_COL] = Din->n_columns;	/* Only known dimension, the rest is 0 */
	if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
	Dout->table = gmt_M_memory (GMT, NULL, Din->n_tables, struct GMT_DATATABLE *);	/* with table array */
	Dout->n_tables = Din->n_tables;

	nan_flag = gmt_M_memory (GMT, NULL, Din->n_columns, unsigned char);
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		Tout = gmt_create_table (GMT, Din->table[tbl]->n_segments, 0, Din->n_columns, 0U, false);
		Dout->table[tbl] = Tout;
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			if (S->n_rows < 2) {
				GMT_Report (API, GMT_MSG_WARNING, "Table %" PRIu64 " Segment %" PRIu64 " has %" PRIu64 " record - no interpolation possible\n", tbl, seg, S->n_rows);
				continue;
			}
			gmt_M_memset (nan_flag, Din->n_columns, unsigned char);
			for (col = 0; col < Din->n_columns; col++) for (row = 0; row < S->n_rows; row++) if (gmt_M_is_dnan (S->data[col][row])) nan_flag[col] = true;
			if (Ctrl->T.T.spatial) {	/* Need distances for path interpolation */
				dist_in = gmt_dist_array (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, true);
				lon = gmt_M_memory (GMT, NULL, S->n_rows, double);
				lat = gmt_M_memory (GMT, NULL, S->n_rows, double);
				gmt_M_memcpy (lon, S->data[GMT_X], S->n_rows, double);
				gmt_M_memcpy (lat, S->data[GMT_Y], S->n_rows, double);
				m = gmt_resample_path (GMT, &lon, &lat, S->n_rows, Ctrl->T.T.inc, Ctrl->A.mode);
				t_out = gmt_dist_array (GMT, lon, lat, m, true);
			}
			else if (Ctrl->T.T.file) {	/* Get relevant t_out segment */
				uint64_t n_outside = 0;
				low_t  = MIN (S->data[Ctrl->N.col][0], S->data[Ctrl->N.col][S->n_rows-1]);
				high_t = MAX (S->data[Ctrl->N.col][0], S->data[Ctrl->N.col][S->n_rows-1]);
				t_out = gmt_M_memory (GMT, NULL, Ctrl->T.T.n, double);
				for (row = m = 0; row < Ctrl->T.T.n; row++) {
					if (Ctrl->T.T.array[row] < low_t || Ctrl->T.T.array[row] > high_t) n_outside++;
					t_out[m++] = Ctrl->T.T.array[row];
				}
				if (n_outside) {
					GMT_Report (API, GMT_MSG_WARNING, "%" PRIu64 " knot points outside range %g to %g\n", n_outside, S->data[Ctrl->N.col][0], S->data[Ctrl->N.col][S->n_rows-1]);
				}
			}
			else {	/* Generate evenly spaced output */
				double min, max;
				if (S->data[Ctrl->N.col][0] > S->data[Ctrl->N.col][S->n_rows-1]) {	/* t-column is monotonically decreasing */
					min = (Ctrl->T.T.delay[GMT_X]) ? floor (S->data[Ctrl->N.col][0] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.min;
					max = (Ctrl->T.T.delay[GMT_Y]) ? ceil (S->data[Ctrl->N.col][S->n_rows-1] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.max;
					Ctrl->T.T.reverse = true;	/* Flag we are monotonically decreasing in time for this segment */
				}
				else {
					min = (Ctrl->T.T.delay[GMT_X]) ? ceil (S->data[Ctrl->N.col][0] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.min;
					max = (Ctrl->T.T.delay[GMT_Y]) ? floor (S->data[Ctrl->N.col][S->n_rows-1] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.max;
					Ctrl->T.T.reverse = false;	/* Flag we are monotonically increasing in time for this segment */
				}
				if (gmt_create_array(GMT, 'T', &(Ctrl->T.T), &min, &max) != GMT_NOERROR) {
					GMT_Report(API, GMT_MSG_WARNING, "Segment %" PRIu64 " in table %" PRIu64 " had troubles.\n", seg, tbl);
					continue;
				}
				m = Ctrl->T.T.n;
				t_out = Ctrl->T.T.array;
			}
			/* Readjust the row allocation for the current output segment */
			Sout = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, m, Din->n_columns, NULL, Tout->segment[seg]);
			if (Ctrl->T.T.spatial) {	/* Use resampled path coordinates */
				gmt_M_memcpy (Sout->data[GMT_X], lon, m, double);
				gmt_M_memcpy (Sout->data[GMT_Y], lat, m, double);
			}
			else
				gmt_M_memcpy (Sout->data[Ctrl->N.col], t_out, m, double);
			if (S->header) Sout->header = strdup (S->header);	/* Duplicate header */
			SH = gmt_get_DS_hidden (Sout);
			Sout->n_rows = SH->n_alloc = m;

			for (col = 0; m && col < Din->n_columns; col++) {

				if (col == Ctrl->N.col && !Ctrl->T.T.spatial) continue;	/* Skip the time column */
				if (Ctrl->W.active && col == Ctrl->W.col) continue;	/* Skip the weight column */
				if (Ctrl->T.T.spatial && col <= GMT_Y) continue;		/* Skip the lon,lat columns */

				if (nan_flag[col] && !GMT->current.setting.io_nan_records) {	/* NaN's present, need "clean" time and data columns */

					ttime = gmt_M_memory (GMT, NULL, S->n_rows, double);
					data = gmt_M_memory (GMT, NULL, S->n_rows, double);
					if (Ctrl->W.active) weight = gmt_M_memory (GMT, NULL, S->n_rows, double);
					for (row = k = 0; row < S->n_rows; row++) {
						if (gmt_M_is_dnan (S->data[col][row])) continue;
						ttime[k] = (Ctrl->T.T.spatial) ? dist_in[row] : S->data[Ctrl->N.col][row];
						data[k] = S->data[col][row];
						if (Ctrl->W.active) weight[k] = S->data[Ctrl->W.col][row];
						k++;
					}
					result = gmt_intpol (GMT, ttime, data, weight, k, m, t_out, Sout->data[col], Ctrl->F.fit, int_mode);
					gmt_M_free (GMT, ttime);
					gmt_M_free (GMT, data);
					if (Ctrl->W.active) gmt_M_free (GMT, weight);
				}
				else {
					ttime = (Ctrl->T.T.spatial) ? dist_in : S->data[Ctrl->N.col];
					weight = (Ctrl->W.active) ? S->data[Ctrl->W.col] : NULL;
					result = gmt_intpol (GMT, ttime, S->data[col], weight, S->n_rows, m, t_out, Sout->data[col], Ctrl->F.fit, int_mode);
				}

				if (result != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "Failure in gmt_intpol near row %d!\n", result+1);
					return (result);
				}
			}
			if (Ctrl->E.active && S->text) {	/* Copy over any trailing text if input times are matched exactly in output */
				for (k = row = 0; k < m; k++) {	/* For all output times */
					while (row < S->n_rows && S->data[Ctrl->N.col][row] < Sout->data[Ctrl->N.col][k]) row++;	/* Wind to next potential matching input time */
					if (row < S->n_rows && doubleAlmostEqualZero (S->data[Ctrl->N.col][row], Sout->data[Ctrl->N.col][k]) && S->text[row]) {	/* Matching time and we have text */
						if (Sout->text == NULL) {	/* Must allocate text array the first time */
							if ((Sout->text = gmt_M_memory (GMT, NULL, m, char *)) == NULL) {
								GMT_Report(API, GMT_MSG_ERROR, "Unable to allocate trailing text for egment %" PRIu64 " in table %" PRIu64 ".\n", seg, tbl);
								Return (GMT_MEMORY_ERROR);
							}
						}
						Sout->text[k] = strdup (S->text[row++]);	/* Duplicate trailing text on output */
					}
				}
			}

			if (Ctrl->T.T.spatial) {	/* Free up memory used */
				gmt_M_free (GMT, dist_in);	gmt_M_free (GMT, t_out);
				gmt_M_free (GMT, lon);		gmt_M_free (GMT, lat);
			}
			else if (Ctrl->T.T.file)
				gmt_M_free (GMT, t_out);
			Dout->table[tbl]->segment[seg] = Sout;
		}
	}
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, geometry, GMT_WRITE_NORMAL, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
		Return (API->error);
	}

	gmt_M_free (GMT, nan_flag);

	GMT->hidden.sample_along_arc = false;	/* Reset */

	Return (GMT_NOERROR);
}
