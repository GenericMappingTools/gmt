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
#define THIS_MODULE_KEYS	"<D{,ND(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->Vbdefghioqs" GMT_OPT("HMm")

#define INT_1D_CART	0	/* Regular 1-D interpolation */
#define INT_2D_CART	1	/* Cartesian 2-D path interpolation */
#define INT_2D_GEO	2	/* Spherical 2-D path interpolation */

struct SAMPLE1D_CTRL {
	struct SAMP1D_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct SAMP1D_A {	/* -A[f|m|p|r|R|l][+l] */
		bool active, loxo;
		enum GMT_enum_track mode;
	} A;
	struct SAMP1D_F {	/* -Fl|a|c[1|2] */
		bool active;
		unsigned int mode;
		unsigned int type;
	} F;
	struct SAMP1D_N {	/* -N<time_col> */
		bool active;
		unsigned int col;
	} N;
	struct SAMP1D_T {	/* -T<tmin/tmax/tinc>[+a|n] */
		bool active;
		struct GMT_ARRAY T;
	} T;
	/* Deprecated options in GMT 6 now handled by -T */
	/* -I<inc>[d|m|s|e|f|k|M|n|u|c] (c means x/y Cartesian path) */
	/* -N<knotfile> */
	/* -S<xstart>[/<xstop>] */
	/* -T<time_col> */
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SAMPLE1D_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct SAMPLE1D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.mode = GMT->current.setting.interpolant;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SAMPLE1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	char type[3] = {'l', 'a', 'c'};

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A[f|m|p|r|R]+l] [-Fl|a|c|n][+1|2] [-N<time_col>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t-T[<min>/<max>/]<inc>[+n|a] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
	             GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_j_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t   The independent variable (see -N) must be monotonically in/de-creasing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Controls how the input track in <table> is resampled when increment has a unit appended:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f: Keep original points, but add intermediate points if needed [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: Same, but first follow meridian (along y) then parallel (along x).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p: Same, but first follow parallel (along x) then meridian (along y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r: Resample at equidistant locations; input points not necessarily included.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   R: Same, but adjust given spacing to fit the track length exactly.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l to compute distances along rhumblines (loxodromes) [no].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the interpolation mode.  Choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l Linear interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Akima spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Cubic spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n No interpolation (nearest point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +1 for 1st derivative or +2 for 2nd derivative.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -F%c].\n", type[API->GMT->current.setting.interpolant]);
	GMT_Message (API, GMT_TIME_NONE, "\t-N Give column number of the independent variable (time) [Default is 0 (first)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Make evenly spaced output time steps from <min> to <max> by <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to indicate <inc> is the number of t-values to produce over the range instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For absolute time resampling, append a valid time unit (%s) to the increment and add +t.\n", GMT_TIME_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   For spatial resampling with distance computed from the first two columns, specify increment as\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <inc> and append a geospatial distance unit (%s) or c (for Cartesian distances).\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   See -A to control how the spatial resampling is done.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +a to add such internal distances as a final output column [no distances added].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with output times in the first column, or a comma-separated list.\n");
	GMT_Option (API, "V,bi2,bo,d,e,f,g,h,i,j,o,q,s,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SAMPLE1D_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change track resampling mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'f': Ctrl->A.mode = GMT_TRACK_FILL;   break;
					case 'm': Ctrl->A.mode = GMT_TRACK_FILL_M; break;
					case 'p': Ctrl->A.mode = GMT_TRACK_FILL_P; break;
					case 'r': Ctrl->A.mode = GMT_TRACK_SAMPLE_FIX; break;
					case 'R': Ctrl->A.mode = GMT_TRACK_SAMPLE_ADJ; break;
					default: GMT_Report (API, GMT_MSG_ERROR, "Option -A: Bad modifier %c\n", opt->arg[0]); n_errors++; break;
				}
				if (strstr (opt->arg, "+l")) Ctrl->A.loxo = true;
				break;
			case 'F':
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
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (opt->arg[1] == '+') Ctrl->F.type = (opt->arg[2] - '0');	/* Want first or second derivatives */
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
					Ctrl->T.active = true;
					t_arg = opt->arg;
				}
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

	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && s_arg, "Specify only one of -N and -S\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->N.col >= 2) ? Ctrl->N.col + 1 : 2);
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Option -F: Only 1st or 2nd derivatives may be requested\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sample1d (void *V_API, int mode, void *args) {
	unsigned int geometry, int_mode;
	int error = 0, result;

	unsigned char *nan_flag = NULL;

	uint64_t k, tbl, col, row, seg, m = 0, dim[GMT_DIM_SIZE] = {0, 0, 0, 0};

	double *t_out = NULL, *dist_in = NULL, *ttime = NULL, *data = NULL;
	double low_t, high_t, *lon = NULL, *lat = NULL;

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
	int_mode = Ctrl->F.mode + 10*Ctrl->F.type;

	if (Ctrl->T.T.spatial) {
		if (gmt_M_is_cartesian (GMT, GMT_IN) && Ctrl->A.loxo) {
			GMT_Report (API, GMT_MSG_ERROR, "Loxodrome mode ignored for Cartesian data.\n");
			Ctrl->A.loxo = false;
		}
		if (Ctrl->A.loxo) GMT->current.map.loxodrome = true;
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
	if (Din->n_columns < 2) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least 2 are needed\n", (int)Din->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	if (Ctrl->N.active && Ctrl->N.col >= Din->n_columns) {	/*  */
		GMT_Report (API, GMT_MSG_ERROR, "Requested time column is greater than data number of columns (%d)\n", (int)Din->n_columns);
		Return (GMT_RUNTIME_ERROR);
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
				GMT_Report (API, GMT_MSG_WARNING, "Segment %" PRIu64 " in table %" PRIu64 " has < 2 records - skipped as no interpolation is possible\n", seg, tbl);
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
				min = (Ctrl->T.T.delay[GMT_X]) ? ceil (S->data[Ctrl->N.col][0] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.min;
				max = (Ctrl->T.T.delay[GMT_Y]) ? floor (S->data[Ctrl->N.col][S->n_rows-1] / Ctrl->T.T.inc) * Ctrl->T.T.inc : Ctrl->T.T.max;
				gmt_create_array (GMT, 'T', &(Ctrl->T.T), &min, &max);
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
				if (Ctrl->T.T.spatial && col <= GMT_Y) continue;		/* Skip the lon,lat columns */

				if (nan_flag[col] && !GMT->current.setting.io_nan_records) {	/* NaN's present, need "clean" time and data columns */

					ttime = gmt_M_memory (GMT, NULL, S->n_rows, double);
					data = gmt_M_memory (GMT, NULL, S->n_rows, double);
					for (row = k = 0; row < S->n_rows; row++) {
						if (gmt_M_is_dnan (S->data[col][row])) continue;
						ttime[k] = (Ctrl->T.T.spatial) ? dist_in[row] : S->data[Ctrl->N.col][row];
						data[k++] = S->data[col][row];
					}
					result = gmt_intpol (GMT, ttime, data, k, m, t_out, Sout->data[col], int_mode);
					gmt_M_free (GMT, ttime);
					gmt_M_free (GMT, data);
				}
				else {
					ttime = (Ctrl->T.T.spatial) ? dist_in : S->data[Ctrl->N.col];
					result = gmt_intpol (GMT, ttime, S->data[col], S->n_rows, m, t_out, Sout->data[col], int_mode);
				}

				if (result != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "Failure in gmt_intpol near row %d!\n", result+1);
					return (result);
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
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, geometry, 0, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
		Return (API->error);
	}

	gmt_M_free (GMT, nan_flag);

	Return (GMT_NOERROR);
}
