/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: sample1d reads a 1-D dataset, and resamples the values on (1) user
 * supplied time values or (2) equidistant time-values based on <timestart> and
 * <dt>, both supplied at the command line. Choose among linear, cubic
 * spline, and Akima's spline.  sample1d will handle multiple column files,
 * user must choose which column contains the independent, monotonically
 * increasing variable.
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2010
 * Version:	5 API
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_NAME	"sample1d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Resample 1-D table data using splines"
#define THIS_MODULE_KEYS	"<D{,ND(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->Vbdefghios" GMT_OPT("HMm")

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
	struct SAMP1D_I {	/* -I<inc>[d|m|s|e|f|k|M|n|u|c] (c means x/y Cartesian path) */
		bool active;
		unsigned int mode;
		int smode;
		double inc;
		char unit;
	} I;
	struct SAMP1D_N {	/* -N<knotfile> */
		bool active;
		char *file;
	} N;
	struct SAMP1D_S {	/* -S<xstart>[/<xstop>] */
		bool active;
		unsigned int mode;
		double start, stop;
	} S;
	struct SAMP1D_T {	/* -T<time_col> */
		bool active;
		unsigned int col;
	} T;
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
	gmt_M_str_free (C->N.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	char type[3] = {'l', 'a', 'c'};

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: sample1d [<table>] [-A[f|m|p|r|R]+l] [-Fl|a|c|n][+1|2] [-I<inc>[<unit>]] [-N<knottable>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<start>[/<stop]] [-T<time_col>] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n",
	             GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t   The independent variable (see -T) must be monotonically in/de-creasing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Controls how the input track in <table> is resampled when -I..<unit> is selected:\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-I Set equidistant grid interval <inc> [t1 - t0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append %s to indicate that the first two columns contain\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   longitude, latitude and you wish to resample this path with a nominal spacing of <inc>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in the chosen units.  For Cartesian paths specify unit as c.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See -A to control how the resampling is done.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N The <knottable> is an ASCII table with the desired time positions in column 0.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Overrides the -I and -S settings.  If none of -I, -S, and -N is set\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then <tstart> = first input point, <t_inc> = (t[1] - t[0]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set the first output point to be <start> [first multiple of inc in range].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append /<stop> for last output point [last multiple of inc in range].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Give column number of the independent variable (time) [Default is 0 (first)].\n");
	GMT_Option (API, "V,bi2,bo,d,e,f,g,h,i,o,s,.");
	
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
	char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

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
					default: GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: Bad modifier %c\n", opt->arg[0]); n_errors++; break;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (opt->arg[1] == '+') Ctrl->F.type = (opt->arg[2] - '0');	/* Want first or second derivatives */
				break;
			case 'I':
				Ctrl->I.active = true;
				if ((c = strchr (opt->arg, 'c')) != NULL) Ctrl->I.mode = INT_2D_CART, c[0] = 0;	/* Chop off unit c as not understood by gmt_get_distance */
				Ctrl->I.smode = gmt_get_distance (GMT, opt->arg, &(Ctrl->I.inc), &(Ctrl->I.unit));
				if (strchr (GMT_LEN_UNITS, Ctrl->I.unit)) Ctrl->I.mode = INT_2D_GEO;
				if (Ctrl->I.mode == INT_2D_CART && c) c[0] = 'c';	/* Restore the c */
				break;
			case 'N':
				if ((Ctrl->N.active = gmt_check_filearg (GMT, 'N', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->N.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'S':
				Ctrl->S.active = true;
				if (strchr (opt->arg, '/')) {	/* Got start/stop */
					Ctrl->S.mode = 1;
					sscanf (opt->arg, "%[^/]/%s", A, B);
					gmt_scanf_arg (GMT, A, GMT_IS_UNKNOWN, &Ctrl->S.start);
					gmt_scanf_arg (GMT, B, GMT_IS_UNKNOWN, &Ctrl->S.stop);
				}
				else
					gmt_scanf_arg (GMT, opt->arg, GMT_IS_UNKNOWN, &Ctrl->S.start);
				break;
			case 'T':
				Ctrl->T.active = true;
				col = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, col < 0, "Syntax error -T option: Column number cannot be negative\n");
				Ctrl->T.col = col;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == 1 && Ctrl->S.stop <= Ctrl->S.start, "Syntax error -S option: <stop> must exceed <start>\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->I.active, "Syntax error: Specify only one of -N and -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.inc <= 0.0, "Syntax error -I option: Must specify positive increment\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && gmt_access (GMT, Ctrl->N.file, R_OK), "Syntax error -N. Cannot read file %s\n", Ctrl->N.file);
	n_errors += gmt_check_binary_io (GMT, (Ctrl->T.col >= 2) ? Ctrl->T.col + 1 : 2);
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Syntax error -F option: Only 1st or 2nd derivatives may be requested\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sample1d (void *V_API, int mode, void *args) {
	unsigned int geometry, int_mode;
	bool resample_path = false;
	int error = 0, result;
	
	unsigned char *nan_flag = NULL;
	
	size_t m_alloc;
	uint64_t k, tbl, col, row, seg, m = 0, m_supplied = 0, dim[GMT_DIM_SIZE] = {0, 0, 0, 0};

	double *t_supplied_out = NULL, *t_out = NULL, *dist_in = NULL, *ttime = NULL, *data = NULL;
	double tt, low_t, high_t, last_t, *lon = NULL, *lat = NULL;

	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_DATATABLE *T = NULL, *Tout = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sout = NULL;
	struct SAMPLE1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the sample1d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	GMT->current.setting.interpolant = Ctrl->F.mode % 10;
	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;	/* Turn off default GMT NaN-handling for (x,y) which is not the case here */
	GMT->current.io.skip_if_NaN[Ctrl->T.col] = true;				/* ... But disallow NaN in "time" column */
	int_mode = Ctrl->F.mode + 10*Ctrl->F.type;
	
	if (Ctrl->I.mode) {
		if (Ctrl->I.smode == GMT_GEODESIC) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Cannot use geodesic distances as path interpolation is spherical; changed to spherical\n");
			Ctrl->I.smode = GMT_GREATCIRCLE;
		}
		if (Ctrl->I.mode == INT_2D_GEO && !gmt_M_is_geographic (GMT, GMT_IN)) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
		if (!gmt_M_is_geographic (GMT, GMT_IN) && Ctrl->A.loxo) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Loxodrome mode ignored for Cartesian data.\n");
			Ctrl->A.loxo = false;
		}
		if (Ctrl->A.loxo) GMT->current.map.loxodrome = true;
		gmt_init_distaz (GMT, Ctrl->I.unit, Ctrl->I.smode, GMT_MAP_DIST);		
		resample_path = true;	/* Resample (x,y) track according to -I and -A */
	}

	geometry = (resample_path) ? GMT_IS_LINE : GMT_IS_NONE;
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}

	/* First read input data to be sampled */
	
	if ((error = gmt_set_cols (GMT, GMT_IN, 0)) != 0) Return (error);
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < 2) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)Din->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	if (Ctrl->T.active && Ctrl->T.col >= Din->n_columns) {	/*  */
		GMT_Report (API, GMT_MSG_NORMAL, "Requested time column is greater than data number of columns (%d)\n", (int)Din->n_columns);
		Return (GMT_RUNTIME_ERROR);
	}
	
	if (Ctrl->N.active) {	/* read file with abscissae */
		struct GMT_DATASET *Cin = NULL;
		gmt_init_io_columns (GMT, GMT_IN);	/* Reset any effects of -i */
		
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
		if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			Return (API->error);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		T = Cin->table[0];	/* Since we only have one table here */
		t_supplied_out = gmt_M_memory (GMT, NULL, Cin->table[0]->n_records, double);
		for (seg = 0; seg < T->n_segments; seg++) {
			gmt_M_memcpy (&t_supplied_out[m], T->segment[seg]->data[GMT_X], T->segment[seg]->n_rows, double);
			m += T->segment[seg]->n_rows;
		}
		m_supplied = m;
		t_out = gmt_M_memory (GMT, NULL, m_supplied, double);
		GMT_Report (API, GMT_MSG_VERBOSE, "Read %" PRIu64 " knots from file\n", m_supplied);
		if (GMT_Destroy_Data (API, &Cin) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	dim[GMT_COL] = Din->n_columns;	/* Only known dimension, the rest is 0 */
	if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
	Dout->table = gmt_M_memory (GMT, NULL, Din->n_tables, struct GMT_DATATABLE *);	/* with table array */
	Dout->n_tables = Din->n_tables;

	nan_flag = gmt_M_memory (GMT, NULL, Din->n_columns, unsigned char);
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		Tout = gmt_create_table (GMT, Din->table[tbl]->n_segments, 0, Din->n_columns, false);
		Dout->table[tbl] = Tout;
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			if (S->n_rows < 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Segment %" PRIu64 " in table %" PRIu64 " has < 2 records - skipped as no interpolation is possible\n", seg, tbl);
				continue;
			}
			gmt_M_memset (nan_flag, Din->n_columns, unsigned char);
			for (col = 0; col < Din->n_columns; col++) for (row = 0; row < S->n_rows; row++) if (gmt_M_is_dnan (S->data[col][row])) nan_flag[col] = true;
			if (resample_path) {	/* Need distances for path interpolation */
				dist_in = gmt_dist_array (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, true);
				lon = gmt_M_memory (GMT, NULL, S->n_rows, double);
				lat = gmt_M_memory (GMT, NULL, S->n_rows, double);
				gmt_M_memcpy (lon, S->data[GMT_X], S->n_rows, double);
				gmt_M_memcpy (lat, S->data[GMT_Y], S->n_rows, double);
				m = gmt_resample_path (GMT, &lon, &lat, S->n_rows, Ctrl->I.inc, Ctrl->A.mode);
				t_out = gmt_dist_array (GMT, lon, lat, m, true);
			}
			else if (Ctrl->N.active) {	/* Get relevant t_out segment */
				uint64_t n_outside = 0;
				low_t  = MIN (S->data[Ctrl->T.col][0], S->data[Ctrl->T.col][S->n_rows-1]);
				high_t = MAX (S->data[Ctrl->T.col][0], S->data[Ctrl->T.col][S->n_rows-1]);
				for (row = m = 0; row < m_supplied; row++) {
					if (t_supplied_out[row] < low_t || t_supplied_out[row] > high_t) n_outside++;
					t_out[m++] = t_supplied_out[row];
				}
				if (n_outside) {
					GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %" PRIu64 " knot points outside range %g to %g\n", n_outside, S->data[Ctrl->T.col][0], S->data[Ctrl->T.col][S->n_rows-1]);
				}
			}
			else {	/* Generate evenly spaced output */
				uint64_t first_k, last_k;
				double abs_inc;
				if (!Ctrl->I.active) Ctrl->I.inc = S->data[Ctrl->T.col][1] - S->data[Ctrl->T.col][0];
				if (Ctrl->I.active && (S->data[Ctrl->T.col][1] - S->data[Ctrl->T.col][0]) < 0.0 && Ctrl->I.inc > 0.0) Ctrl->I.inc = -Ctrl->I.inc;	/* For monotonically decreasing data */
				first_k = (Ctrl->I.inc > 0.0) ? 0 : S->n_rows-1;	/* Index of point with earliest time */
				last_k  = (Ctrl->I.inc > 0.0) ? S->n_rows-1 : 0;	/* Index of point with latest time */
				abs_inc = fabs (Ctrl->I.inc);
				if (!Ctrl->S.active) {	/* Set first output time */
					Ctrl->S.start = floor (S->data[Ctrl->T.col][first_k] / abs_inc) * abs_inc;
					if (Ctrl->S.start < S->data[Ctrl->T.col][first_k]) Ctrl->S.start += abs_inc;
				}
				last_t = (Ctrl->S.mode) ? Ctrl->S.stop : S->data[Ctrl->T.col][last_k];
				/* So here, Ctrl->S.start holds to smallest t value and last_t holds the largest t_value, regardless of inc sign */
				m = m_alloc = lrint (fabs((last_t - Ctrl->S.start) / abs_inc)) + 1;
				t_out = gmt_M_memory (GMT, t_out, m_alloc, double);
				row = 1;
				if (Ctrl->I.inc > 0.0) {
					t_out[0] = Ctrl->S.start;
					while (row < m && (tt = Ctrl->S.start + row * Ctrl->I.inc) <= last_t) {
						t_out[row] = tt;
						row++;
					}
					if (fabs (t_out[row-1]-last_t) < GMT_CONV4_LIMIT) t_out[row-1] = last_t;	/* Fix roundoff */
				}
				else {
					t_out[0] = last_t;
					while (row < m && (tt = last_t + row * Ctrl->I.inc) >= Ctrl->S.start) {
						t_out[row] = tt;
						row++;
					}
					if (fabs (t_out[row-1]-Ctrl->S.start) < GMT_CONV4_LIMIT) t_out[row-1] = Ctrl->S.start;	/* Fix roundoff */
				}
				m = row;
			}
			/* Readjust the row allocation for the current output segment */
			Sout = GMT_Alloc_Segment (GMT->parent, GMT_IS_DATASET, m, Din->n_columns, NULL, Tout->segment[seg]);
			if (resample_path) {	/* Use resampled path coordinates */
				gmt_M_memcpy (Sout->data[GMT_X], lon, m, double);
				gmt_M_memcpy (Sout->data[GMT_Y], lat, m, double);
			}
			else
				gmt_M_memcpy (Sout->data[Ctrl->T.col], t_out, m, double);
			if (S->header) Sout->header = strdup (S->header);	/* Duplicate header */
			Sout->n_rows = m;
				
			for (col = 0; m && col < Din->n_columns; col++) {

				if (col == Ctrl->T.col && !resample_path) continue;	/* Skip the time column */
				if (resample_path && col <= GMT_Y) continue;		/* Skip the lon,lat columns */
				
				if (nan_flag[col] && !GMT->current.setting.io_nan_records) {	/* NaN's present, need "clean" time and data columns */

					ttime = gmt_M_memory (GMT, NULL, S->n_rows, double);
					data = gmt_M_memory (GMT, NULL, S->n_rows, double);
					for (row = k = 0; row < S->n_rows; row++) {
						if (gmt_M_is_dnan (S->data[col][row])) continue;
						ttime[k] = (resample_path) ? dist_in[row] : S->data[Ctrl->T.col][row];
						data[k++] = S->data[col][row];
					}
					result = gmt_intpol (GMT, ttime, data, k, m, t_out, Sout->data[col], int_mode);
					gmt_M_free (GMT, ttime);
					gmt_M_free (GMT, data);
				}
				else {
					ttime = (resample_path) ? dist_in : S->data[Ctrl->T.col];
					result = gmt_intpol (GMT, ttime, S->data[col], S->n_rows, m, t_out, Sout->data[col], int_mode);
				}

				if (result != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error from gmt_intpol near row %d!\n", result+1);
					return (result);
				}
			}
			if (resample_path) {	/* Free up memory used */
				gmt_M_free (GMT, dist_in);	gmt_M_free (GMT, t_out);
				gmt_M_free (GMT, lon);		gmt_M_free (GMT, lat);
			}
			else if (!Ctrl->N.active)
				gmt_M_free (GMT, t_out);
		}
	}
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, geometry, Dout->io_mode, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
		Return (API->error);
	}
#if 0
	gmt_free_dataset (API->GMT, &Dout);	/* Since not registered */
#endif

	if (Ctrl->N.active) gmt_M_free (GMT, t_out);
	gmt_M_free (GMT, nan_flag);
	if (Ctrl->N.active) gmt_M_free (GMT, t_supplied_out);
	
	Return (GMT_NOERROR);
}
