/*--------------------------------------------------------------------
 *	$Id: sample1d_func.c,v 1.8 2011-04-23 02:14:13 guru Exp $
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
 
#include "gmt.h"

#define INT_1D	0	/* Regular 1-D interpolation */
#define INT_2D	1	/* Spatial 2-D path interpolation */

struct SAMPLE1D_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A[m|p] */
		GMT_LONG active;
		int mode;
	} A;
	struct F {	/* -Fl|a|c */
		GMT_LONG active;
		GMT_LONG mode;
	} F;
	struct I {	/* -I<inc>[d|e|k||M|n|c|C] */
		GMT_LONG active;
		GMT_LONG mode;
		double inc;
		char unit;
	} I;
	struct T {	/* -T<time_col> */
		GMT_LONG active;
		GMT_LONG col;
	} T;
	struct N {	/* -N<knotfile> */
		GMT_LONG active;
		char *file;
	} N;
	struct S {	/* -S<xstart>[/<xstop>] */
		GMT_LONG active;
		GMT_LONG mode;
		double start, stop;
	} S;
};

void *New_sample1d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SAMPLE1D_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct SAMPLE1D_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->F.mode = GMT->current.setting.interpolant;
		
	return ((void *)C);
}

void Free_sample1d_Ctrl (struct GMT_CTRL *GMT, struct SAMPLE1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free ((void *)C->Out.file);	
	if (C->N.file) free ((void *)C->N.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_sample1d_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	char type[3] = {'l', 'a', 'c'};
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "sample1d %s [API] - Resampling of 1-D data sets\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: sample1d <infile(s)> [-A[m|p]] [-Fl|a|c|n] [-I<inc>[<unit>]] [-N<knotfile>]\n");
	GMT_message (GMT, "\t[-S<start>[/<stop]] [-T<time_col>] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<infile> is one or more multicolumn ASCII (or binary, see -b) tables. [Default is standard input].\n");
	GMT_message (GMT, "\tThe independent variable (see -T) must be monotonically in/de-creasing.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A For spherical surface sampling we follow great circle paths.\n");
	GMT_message (GMT, "\t   Append m or p to first follow meridian then parallel, or vice versa.\n");
	GMT_message (GMT, "\t-F Sets the interpolation mode.  Choose from:\n");
	GMT_message (GMT, "\t   l Linear interpolation.\n");
	GMT_message (GMT, "\t   a Akima spline interpolation.\n");
	GMT_message (GMT, "\t   c Cubic spline interpolation.\n");
	GMT_message (GMT, "\t   n No interpolation (nearest point).\n");
	GMT_message (GMT, "\t   [Default is -F%c]\n", type[GMT->current.setting.interpolant]);
	GMT_message (GMT, "\t-I Sets equidistant grid interval <inc> [t1 - t0].\n");
	GMT_message (GMT, "\t   Append %s to indicate that the first two columns contain\n", GMT_LEN_UNITS_DISPLAY);
	GMT_message (GMT, "\t   longitude, latitude and you wish to resample this path using spherical\n");
	GMT_message (GMT, "\t   segments with a nominal spacing of <inc> in the chosen units.\n");
	GMT_message (GMT, "\t   See -Am|p to only sample along meridians and parallels.\n");
	GMT_message (GMT, "\t-N The <knotfile> is an ASCII table with the desired time positions in column 0.\n");
	GMT_message (GMT, "\t   Overrides the -I and -S settings.  If none of -I, -S, and -N is set\n");
	GMT_message (GMT, "\t   then <tstart> = first input point, <t_inc> = (t[1] - t[0]).\n");
	GMT_message (GMT, "\t-S Sets the first output point to be <start> [first multiple of inc in range].\n");
	GMT_message (GMT, "\t   Optionally, append /<stop> for last output point [last multiple of inc in range].\n");
	GMT_message (GMT, "\t-T Gives column number of the independent variable (time) [Default is 0 (first)].\n");
	GMT_explain_options (GMT, "VC2D0fghi.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_sample1d_parse (struct GMTAPI_CTRL *C, struct SAMPLE1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sample1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n, n_files = 0;
	char A[GMT_TEXT_LEN64], B[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change spherical sampling mode (but cannot turn it off) */
				Ctrl->A.active = TRUE;
				switch (opt->arg[0]) {
					case 'm': Ctrl->A.mode = 1; break;
					case 'p': Ctrl->A.mode = 2; break;
				}
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				switch (opt->arg[0]) {
					case 'l':
						Ctrl->F.mode = 0;
						break;
					case 'a':
						Ctrl->F.mode = 1;
						break;
					case 'c':
						Ctrl->F.mode = 2;
						break;
					case 'n':
						Ctrl->F.mode = 3;
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				Ctrl->I.inc = atof (opt->arg);
				n = strlen (opt->arg) - 1;
				if (strchr ("defkMn", opt->arg[n])) {
					Ctrl->I.unit = opt->arg[n];
					Ctrl->I.mode = INT_2D;
				}
				break;
			case 'N':
				Ctrl->N.file = strdup (opt->arg);
				Ctrl->N.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				if (strchr (opt->arg, '/')) {	/* Got start/stop */
					Ctrl->S.mode = 1;
					sscanf (opt->arg, "%[^/]/%s", A, B);
					GMT_scanf_arg (GMT, A, GMT_IS_UNKNOWN, &Ctrl->S.start);
					GMT_scanf_arg (GMT, B, GMT_IS_UNKNOWN, &Ctrl->S.stop);
				}
				else
					GMT_scanf_arg (GMT, opt->arg, GMT_IS_UNKNOWN, &Ctrl->S.start);
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.col = atoi (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->T.col < 0, "Syntax error -T option: Column number cannot be negative\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == 1 && Ctrl->S.stop <= Ctrl->S.start, "Syntax error -S option: <stop> must exceed <start>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->I.active, "Syntax error: Specify only one of -N and -S\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->I.inc <= 0.0, "Syntax error -I option: Must specify positive increment\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && GMT_access (GMT, Ctrl->N.file, R_OK), "Syntax error -N. Cannot read file %s\n", Ctrl->N.file);
	n_errors += GMT_check_binary_io (GMT, (Ctrl->T.col >= 2) ? Ctrl->T.col + 1 : 2);
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_sample1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_sample1d (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG result, i, j, k, m = 0, m_alloc, tbl, seg, col, row, rows = 1, m_supplied = 0;
	GMT_LONG error = FALSE, spatial = FALSE, *nan_flag = NULL;

	double *t_supplied_out = NULL, *t_out = NULL, *dist_in = NULL, *ttime = NULL, *data = NULL;
	double tt, low_t, high_t, last_t, inc_degrees;

	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_TABLE *T = NULL, *Tout = NULL;
	struct GMT_LINE_SEGMENT *S = NULL, *Sout = NULL;
	struct SAMPLE1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_sample1d_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_sample1d_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_sample1d", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf", "ghis>" GMT_OPT("HMm"), options))) Return (error);
	Ctrl = (struct SAMPLE1D_CTRL *) New_sample1d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sample1d_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the sample1d main code ----------------------------*/

	GMT->current.setting.interpolant = Ctrl->F.mode;
	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = FALSE;	/* Turn off default GMT NaN-handling for (x,y) which is not the case here */
	GMT->current.io.skip_if_NaN[Ctrl->T.col] = TRUE;				/* ... But disallow NaN in "time" column */
	
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */

	t_out = (double *)NULL;

	if (Ctrl->N.active) {	/* read file with abscissae */
		struct GMT_DATASET *Cin = NULL;
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->N.file, (void **)&Cin)) Return ((error = GMT_DATA_READ_ERROR));
		T = Cin->table[0];	/* Since we only have one table here */
		t_supplied_out = GMT_memory (GMT, NULL, Cin->table[0]->n_records, double);
		for (seg = 0; seg < T->n_segments; seg++) {
			GMT_memcpy (&t_supplied_out[m], T->segment[seg]->coord[GMT_X], T->segment[seg]->n_rows, double);
			m += T->segment[seg]->n_rows;
		}
		m_supplied = m;
		t_out = GMT_memory (GMT, NULL, m_supplied, double);
                GMT_report (GMT, GMT_MSG_NORMAL, "Read %ld knots from file\n", m_supplied);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Cin);
	}

	if (Ctrl->I.mode) GMT_init_distaz (GMT, Ctrl->I.unit, 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);

	if (Ctrl->I.active && Ctrl->I.mode == INT_2D) {
		spatial = TRUE;
		inc_degrees = GMT->current.proj.DIST_M_PR_DEG * Ctrl->I.inc / GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Convert increment to spherical degrees */
	}
	if ((error = GMT_set_cols (GMT, GMT_IN, 0))) Return (error);
	if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&Din)) Return ((error = GMT_DATA_READ_ERROR));
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	Dout = GMT_memory (GMT, NULL, 1, struct GMT_DATASET);				/* Output dataset... */
	Dout->table = GMT_memory (GMT, NULL, Din->n_tables, struct GMT_TABLE *);	/* with table array */
	Dout->n_tables = Din->n_tables;

	nan_flag = GMT_memory (GMT, NULL, Din->n_columns, GMT_LONG);
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		GMT_create_table (GMT, &Tout, Din->table[tbl]->n_segments, Din->n_columns, 0);
		Dout->table[tbl] = Tout;
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			GMT_memset (nan_flag, Din->n_columns, GMT_LONG);
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			for (col = 0; col < Din->n_columns; col++) for (row = 0; row < S->n_rows; row++) if (GMT_is_dnan (S->coord[col][row])) nan_flag[col] = TRUE;
			if (spatial) {	/* Need distance for spatial interpolation */
				double *lon = NULL, *lat = NULL;
				GMT_dist_array (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows, 1.0, 2, &dist_in);
				lon = GMT_memory (GMT, NULL, S->n_rows, double);
				lat = GMT_memory (GMT, NULL, S->n_rows, double);
				GMT_memcpy (lon, S->coord[GMT_X], S->n_rows, double);
				GMT_memcpy (lat, S->coord[GMT_Y], S->n_rows, double);
				m = GMT_fix_up_path (GMT, &lon, &lat, S->n_rows, inc_degrees, Ctrl->A.mode);
				GMT_dist_array (GMT, lon, lat, m, 1.0, 2, &t_out);
			}
			else if (Ctrl->N.active) {	/* Get relevant t_out segment */
				low_t  = MIN (S->coord[Ctrl->T.col][0], S->coord[Ctrl->T.col][S->n_rows-1]);
				high_t = MAX (S->coord[Ctrl->T.col][0], S->coord[Ctrl->T.col][S->n_rows-1]);
				for (i = m = 0; i < m_supplied; i++) {
					if (t_supplied_out[i] < low_t || t_supplied_out[i] > high_t) continue;
					t_out[m++] = t_supplied_out[i];
				}
				if (m == 0) {
					GMT_report (GMT, GMT_MSG_FATAL, "Warning: No output points for range %g to %g\n", S->coord[Ctrl->T.col][0], S->coord[Ctrl->T.col][S->n_rows-1]);
					continue;
				}
			}
			else {	/* Generate evenly spaced output */
				if (!Ctrl->I.active) Ctrl->I.inc = S->coord[Ctrl->T.col][1] - S->coord[Ctrl->T.col][0];
				if (Ctrl->I.active && (S->coord[Ctrl->T.col][1] - S->coord[Ctrl->T.col][0]) < 0.0 && Ctrl->I.inc > 0.0) Ctrl->I.inc = -Ctrl->I.inc;	/* For monotonically decreasing data */
				if (!Ctrl->S.active) {
					if (Ctrl->I.inc > 0.0) {
						Ctrl->S.start = floor (S->coord[Ctrl->T.col][0] / Ctrl->I.inc) * Ctrl->I.inc;
						if (Ctrl->S.start < S->coord[Ctrl->T.col][0]) Ctrl->S.start += Ctrl->I.inc;
					}
					else {
						Ctrl->S.start = ceil (S->coord[Ctrl->T.col][0] / (-Ctrl->I.inc)) * (-Ctrl->I.inc);
						if (Ctrl->S.start > S->coord[Ctrl->T.col][0]) Ctrl->S.start += Ctrl->I.inc;
					}
				}
				last_t = (Ctrl->S.mode) ? Ctrl->S.stop : S->coord[Ctrl->T.col][S->n_rows-1];
				m = m_alloc = irint (fabs((last_t - Ctrl->S.start) / Ctrl->I.inc)) + 1;
				t_out = GMT_memory (GMT, t_out, m_alloc, double);
				t_out[0] = Ctrl->S.start;
				i = 1;
				if (Ctrl->I.inc > 0.0) {
					while (i < m && (tt = Ctrl->S.start + i * Ctrl->I.inc) <= last_t) {
						t_out[i] = tt;
						i++;
					}
				}
				else {
					while (i < m && (tt = Ctrl->S.start + i * Ctrl->I.inc) >= last_t) {
						t_out[i] = tt;
						i++;
					}
				}
				m = i;
				if (fabs (t_out[m-1]-last_t) < GMT_SMALL) t_out[m-1] = last_t;	/* Fix roundoff */
			}
			Sout = Tout->segment[seg];	/* Current output segment */
			GMT_alloc_segment (GMT, Sout, m, Din->n_columns, FALSE);	/* Readjust the row allocation */
			if (!spatial) GMT_memcpy (Sout->coord[Ctrl->T.col], t_out, m, double);
			if (S->header) Sout->header = strdup (S->header);	/* Duplicate header */
			Sout->n_rows = m;
				
			for (j = 0; m && j < Din->n_columns; j++) {

				if (j == Ctrl->T.col && !spatial) continue;	/* Skip the time column */

				if (nan_flag[j] && !GMT->current.setting.io_nan_records) {	/* NaN's present, need "clean" time and data columns */

					ttime = GMT_memory (GMT, NULL, S->n_rows, double);
					data = GMT_memory (GMT, NULL, S->n_rows, double);
					for (i = k = 0; i < S->n_rows; i++) {
						if (GMT_is_dnan (S->coord[j][i])) continue;
						ttime[k] = (spatial) ? dist_in[i] : S->coord[Ctrl->T.col][i];
						data[k++] = S->coord[j][i];
					}
					result = GMT_intpol (GMT, ttime, data, k, m, t_out, Sout->coord[j], Ctrl->F.mode);
					GMT_free (GMT, ttime);
					GMT_free (GMT, data);
				}
				else {
					ttime = (spatial) ? dist_in : S->coord[Ctrl->T.col];
					result = GMT_intpol (GMT, ttime, S->coord[j], S->n_rows, m, t_out, Sout->coord[j], Ctrl->F.mode);
				}

				if (result != GMT_OK) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error from GMT_intpol near row %ld!\n", rows+result+1);
					return (result);
				}
			}
		}
	}
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */
	if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, Dout->io_mode, (void **)&Ctrl->Out.file, (void *)Dout))) Return (error);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Din);
	GMT_free_dataset (GMT, &Dout);
	GMT_free (GMT, t_out);
	if (nan_flag) GMT_free (GMT, nan_flag);
	if (Ctrl->N.active) GMT_free (GMT, t_supplied_out);
	
	Return (GMT_OK);
}
