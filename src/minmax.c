/*--------------------------------------------------------------------
 *    $Id$
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
 * Brief synopsis: minmax.c will read ascii or binary tables and report the
 * extreme values for all columns
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	 API
 */

#include "gmt.h"

EXTERN_MSC GMT_LONG gmt_geo_C_format (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_log_array (struct GMT_CTRL *C, double min, double max, double delta, double **array);

#define REPORT_PER_DATASET	0
#define REPORT_PER_TABLE	1
#define REPORT_PER_SEGMENT	2

struct MINMAX_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 reports range for all tables, 1 is per table, 2 is per segment */
	} A;
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct E {	/* -E<L|l|H|h><col> */
		GMT_LONG active;
		GMT_LONG abs;
		GMT_LONG mode;
		GMT_LONG col;
	} E;
	struct I {	/* -Idx[/dy[/<dz>..]] */
		GMT_LONG active;
		GMT_LONG ncol;
		double inc[GMT_MAX_COLUMNS];
	} I;
	struct S {	/* -S[x|y] */
		GMT_LONG active;
		GMT_LONG xbar, ybar;
	} S;
	struct T {	/* -T<dz>[/<col>] */
		GMT_LONG active;
		double inc;
		GMT_LONG col;
	} T;
};

GMT_LONG strip_blanks_and_output (struct GMT_CTRL *GMT, char *text, double x, GMT_LONG col)
{	/* Alternative to GMT_ascii_output_col that strips off leading blanks first */

	GMT_LONG k;

	GMT_ascii_format_col (GMT, text, x, col);
	for (k = 0; text[k] && text[k] == ' '; k++);
	return (k);	/* This is the position in text that we should start reporting from */
}

void *New_minmax_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MINMAX_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MINMAX_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return (C);
}

void Free_minmax_Ctrl (struct GMT_CTRL *GMT, struct MINMAX_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_minmax_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "minmax %s [API] - Find extreme values in data tables\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: minmax [<table>] [-Aa|f|s] [-C] [-E<L|l|H|h><col>] [-I[p]<dx>[/<dy>[/<dz>..]]\n");
	GMT_message (GMT, "\t[-S[x][y]] [-T<dz>[/<col>]] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);
     
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Select reports for (a)ll [Default], per (f)ile, or per (s)egment.\n");
	GMT_message (GMT, "\t-C Format the min and max into separate columns.\n");
	GMT_message (GMT, "\t-E Return the record with extreme value in specified column <col> [last column].\n");
	GMT_message (GMT, "\t   Specify l or h for min or max value, respectively.  Upper case L or H\n");
	GMT_message (GMT, "\t   means we operate instead on the absolute values of the data.\n");
	GMT_message (GMT, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy (assumes 2+ col data).\n");
	GMT_message (GMT, "\t   If -C is set then no -R string is issued.  Instead, the number of increments\n");
	GMT_message (GMT, "\t   given determines how many columns are rounded off to the nearest multiple.\n");
	GMT_message (GMT, "\t   If only one increment is given we also use it for the second column (for backwards compatibility).\n");
	GMT_message (GMT, "\t   To override this behaviour, use -Ip<dx>.\n");
	GMT_message (GMT, "\t-S Add extra space for error bars. Useful together with -I.\n");
	GMT_message (GMT, "\t   -Sx leaves space for horizontal error bar using value in third (2) column.\n");
	GMT_message (GMT, "\t   -Sy leaves space for vertical error bar using value in third (2) column.\n");
	GMT_message (GMT, "\t   -S or -Sxy leaves space for both error bars using values in third&fourth (2&3) columns.\n");
	GMT_message (GMT, "\t-T Return textstring -Tzmin/zmax/dz to nearest multiple of the given dz.\n");
	GMT_message (GMT, "\t   Calculations are based on the first (0) column only.  Append /<col> to use another column.\n");
	GMT_explain_options (GMT, "VC2fghi:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_minmax_parse (struct GMTAPI_CTRL *C, struct MINMAX_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to minmax and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j, special = FALSE;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Reporting unit */
				Ctrl->A.active = TRUE;
				switch (opt->arg[0]) {
					case 'a':
						Ctrl->A.mode = REPORT_PER_DATASET;
						break;
					case 'f':
						Ctrl->A.mode = REPORT_PER_TABLE;
						break;
					case 's':
						Ctrl->A.mode = REPORT_PER_SEGMENT;
						break;
					default:
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -A. Flags are a|f|s.\n");
						break;
				}
				break;
			case 'C':	/* Column output */
				Ctrl->C.active = TRUE;
				break;
			case 'E':	/* Extrema reporting */
				Ctrl->E.active = TRUE;
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->E.abs = TRUE;
					case 'l':
						Ctrl->E.mode = -1;
						break;
					case 'H':
						Ctrl->E.abs = TRUE;
					case 'h':
						Ctrl->E.mode = +1;
						break;
					default:
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -E. Flags are L|l|H|h.\n");
						break;
				}
				if (opt->arg[1]) Ctrl->E.col = atoi (&opt->arg[1]);
				break;
			case 'I':	/* Granularity */
				Ctrl->I.active = TRUE;
				if (opt->arg[0] == 'p') special = TRUE;
				j = (special) ? 1 : 0;
				Ctrl->I.ncol = GMT_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS);
				break;
			case 'S':	/* Error bar output */
				Ctrl->S.active = TRUE;
				j = 0;
				while (opt->arg[j]) {
					if (opt->arg[j] == 'x') Ctrl->S.xbar = TRUE;
					if (opt->arg[j] == 'y') Ctrl->S.ybar = TRUE;
					j++;
				}
				if (j == 2) Ctrl->S.xbar = Ctrl->S.ybar = TRUE;
				break;
			case 'T':	/* makecpt range/inc string */
				Ctrl->T.active = TRUE;
				j = sscanf (opt->arg, "%lf/%" GMT_LL "d", &Ctrl->T.inc, &Ctrl->T.col);
				if (j == 1) Ctrl->T.col = 0;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);
	if (Ctrl->I.active && !special && Ctrl->I.ncol == 1) {		/* Special case of dy = dx if not given */
		Ctrl->I.inc[1] = Ctrl->I.inc[0];
		Ctrl->I.ncol = 2;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->C.active && Ctrl->I.ncol < 2, "Syntax error: -Ip requires -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->T.active, "Syntax error: Only one of -I and -T can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0 , "Syntax error -T option: Must specify a positive increment\n");
	for (j = 0; Ctrl->I.active && j < Ctrl->I.ncol; j++) {
		n_errors += GMT_check_condition (GMT, Ctrl->I.inc[j] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += GMT_check_binary_io (GMT, 1);
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_minmax_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_minmax (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, got_stuff = FALSE, first_data_record, give_r_string = FALSE;
	GMT_LONG brackets = FALSE, work_on_abs_value, do_report;
	GMT_LONG i, j, ncol = 0, n = 0, n_fields, save_range, wmode, done;

	char file[GMT_BUFSIZ], chosen[GMT_BUFSIZ], record[GMT_BUFSIZ], buffer[GMT_BUFSIZ], delimeter[2];

	double *xyzmin = NULL, *xyzmax = NULL, *in = NULL, value;
	double west, east, south, north, low, high, e_min = DBL_MAX, e_max = -DBL_MAX;

	struct GMT_QUAD *Q = NULL;
	struct MINMAX_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (options && options->option == GMTAPI_OPT_USAGE) bailout (GMT_minmax_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_minmax_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_minmax", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghis>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_minmax_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_minmax_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the minmax main code ----------------------------*/

	give_r_string = (Ctrl->I.active && !Ctrl->C.active);
	delimeter[0] = (Ctrl->C.active) ? '\t' : '/';
	delimeter[1] = '\0';
	wmode = (Ctrl->C.active) ? GMT_WRITE_DOUBLE : GMT_WRITE_TEXT;
	GMT_memset (file, GMT_BUFSIZ, char);
	
	brackets = !Ctrl->C.active;
	work_on_abs_value = (Ctrl->E.active && Ctrl->E.abs);
	if (GMT->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON) {	/* Must check that output format won't mess things up by printing west > east */
		if (!strcmp (GMT->current.setting.format_geo_out, "D")) {
			strcpy (GMT->current.setting.format_geo_out, "+D");
			GMT_err_fail (GMT, gmt_geo_C_format (GMT), "");
			GMT_report (GMT, GMT_MSG_NORMAL, "Warning: FORMAT_GEO_OUT reset from D to %s to ensure east > west\n", GMT->current.setting.format_geo_out);
		}
		else if (!strcmp (GMT->current.setting.format_geo_out, "ddd:mm:ss")) {
			strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ssF");
			GMT_err_fail (GMT, gmt_geo_C_format (GMT), "");
			GMT_report (GMT, GMT_MSG_NORMAL, "Warning: FORMAT_GEO_OUT reset from ddd:mm:ss to %s to ensure east > west\n", GMT->current.setting.format_geo_out);
		}
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, 0))) Return (error);
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC) != GMT_OK) {
		Return (API->error);
	}

	if (Ctrl->C.active) {	/* Must set output column types since each input col will take up two output cols. */
		GMT_LONG col_type[GMT_MAX_COLUMNS];
		GMT_memcpy (col_type, GMT->current.io.col_type[GMT_IN], GMT_MAX_COLUMNS, GMT_LONG);	/* Duplicate input col types */
		for (i = 0; i < GMT_MAX_COLUMNS/2; i++) GMT->current.io.col_type[GMT_OUT][2*i] = GMT->current.io.col_type[GMT_OUT][2*i+1] = col_type[i];
	}
		
	save_range = GMT->current.io.geo.range;
	first_data_record = TRUE;
	done = FALSE;
	while (!done) {	/* Keep returning records until we reach EOF of last file */
		in = GMT_Get_Record (API, GMT_READ_DOUBLE | GMT_FILE_BREAK, &n_fields);
		do_report = FALSE;

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
		if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */
		if ((GMT_REC_IS_SEG_HEADER (GMT) && Ctrl->A.mode != REPORT_PER_SEGMENT)) continue;	/* Since we are not reporting per segment they are just headers as far as we are concerned */
		
		if (GMT_REC_IS_SEG_HEADER (GMT) || (GMT_REC_IS_FILE_BREAK (GMT) && Ctrl->A.mode == REPORT_PER_TABLE) || GMT_REC_IS_EOF (GMT)) {	/* Time to report */
			if (GMT_REC_IS_SEG_HEADER (GMT) && GMT->current.io.seg_no == 0) continue;	/* Very first segment header means there is no prior segment to report on yet */
			if (GMT_REC_IS_EOF (GMT)) {	/* We are done after this since we hit EOF */
				done = TRUE;
				GMT->current.io.seg_no++;	/* Must manually increment since we are not reading any futher */
			}
			if (n == 0) continue;			/* This segment, table, or data set had no data records, skip */
			
			/* Here we must issue a report */
			
			do_report = TRUE;
 			for (i = 0; i < ncol; i++) if (GMT->current.io.col_type[GMT_IN][i] == GMT_IS_LON) {	/* Must finalize longitudes first */
				j = GMT_quad_finalize (GMT, &Q[i]);
				GMT->current.io.geo.range = Q[i].range[j];		/* Override this setting explicitly */
				xyzmin[i] = Q[i].min[j];	xyzmax[i] = Q[i].max[j];
			}
			if (give_r_string) {	/* Return -R string */
				west  = floor (xyzmin[GMT_X] / Ctrl->I.inc[0]) * Ctrl->I.inc[0];
				east  = ceil  (xyzmax[GMT_X] / Ctrl->I.inc[0]) * Ctrl->I.inc[0];
				south = floor (xyzmin[GMT_Y] / Ctrl->I.inc[1]) * Ctrl->I.inc[1];
				north = ceil  (xyzmax[GMT_Y] / Ctrl->I.inc[1]) * Ctrl->I.inc[1];
				if (east < west) east += 360.0;
				sprintf (record, "-R");
				i = strip_blanks_and_output (GMT, buffer, west, GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, east, GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, south, GMT_Y);	strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, north, GMT_Y);	strcat (record, &buffer[i]);
			}
			else if (Ctrl->T.active) {	/* Return -T string */
				west  = floor (xyzmin[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;
				east  = ceil  (xyzmax[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;
				sprintf (record, "-T");
				i = strip_blanks_and_output (GMT, buffer, west, Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, east, Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, Ctrl->T.inc, Ctrl->T.col);	strcat (record, &buffer[i]);
			}
			else if (Ctrl->E.active)	/* Return extreme record */
				strcpy (record, chosen);
			else {				/* Return min/max for each column */
				if (!Ctrl->C.active) {	/* Want info about each item */
					record[0] = '\0';	/* Start with blank slate */
					if (Ctrl->A.mode == REPORT_PER_DATASET && GMT->current.io.tbl_no > 1)	/* More than one table given */
						strcpy (record, "dataset");
					else if (Ctrl->A.mode == REPORT_PER_SEGMENT)				/* Want segment number after file name */
						sprintf (record, "%s-%ld", file, GMT->current.io.seg_no);
					else									/* Either table mode or only one table in dataset */
						sprintf (record, "%s", file);
					sprintf (buffer, ": N = %ld\t", n);					/* Number of records in this item */
					strcat (record, buffer);
				}
				for (i = 0; i < ncol; i++) {	/* Report min/max for each column in the format controlled by -C */
					if (xyzmin[i] == DBL_MAX)	/* Encountered NaNs only */
						low = high = GMT->session.d_NaN;
					else if (i < Ctrl->I.ncol) {	/* Special treatment for x and y (and perhaps more) if -I selected */
						low  = (Ctrl->I.active) ? floor (xyzmin[i] / Ctrl->I.inc[i]) * Ctrl->I.inc[i] : xyzmin[i];
						high = (Ctrl->I.active) ? ceil  (xyzmax[i] / Ctrl->I.inc[i]) * Ctrl->I.inc[i] : xyzmax[i];
					}
					else {	/* Just the facts, ma'am */
						low = xyzmin[i];
						high = xyzmax[i];
					}
					if (Ctrl->C.active) {
						GMT->current.io.curr_rec[2*i] = low;
						GMT->current.io.curr_rec[2*i+1] = high;
					}
					else {
						if (brackets) strcat (record, "<");
						GMT_ascii_format_col (GMT, buffer, low, i);
						strcat (record, buffer);
						strcat (record, delimeter);
						GMT_ascii_format_col (GMT, buffer, high, i);
						strcat (record, buffer);
						if (brackets) strcat (record, ">");
						if (i < (ncol - 1)) strcat (record, "\t");
					}
				}
			}
			if (Ctrl->C.active) {	/* Plain data record */
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, GMT->current.io.curr_rec);	/* Write data record to output destination */
			}
			else {
				strcat (record, "\n");
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write text record to output destination */
			}
			got_stuff = TRUE;		/* We have at least reported something */
			for (i = 0; i < ncol; i++) {	/* Reset counters for next block */
				xyzmin[i] = +DBL_MAX;
				xyzmax[i] = -DBL_MAX;
			}
			GMT_quad_reset (GMT, Q, ncol);
			n = 0;
			file[0] = '\0';
			if (done || do_report || GMT_REC_IS_FILE_BREAK (GMT)) continue;	/* We are done OR have no data record to process yet */
		}
		if (GMT_REC_IS_FILE_BREAK (GMT)) continue;

		/* We get here once we have read a data record */
		
		if (first_data_record) {	/* First time we read data, we must allocate arrays based on the number of columns */

			ncol = GMT_get_cols (GMT, GMT_IN);
			if (Ctrl->E.active && Ctrl->E.col == -1) Ctrl->E.col = ncol - 1;	/* Default is last column */
			if (Ctrl->S.active && 2 + Ctrl->S.xbar + Ctrl->S.ybar > ncol) {
				GMT_report (GMT, GMT_MSG_FATAL, "Not enough columns to support the -S option\n");
				Return (EXIT_FAILURE);
			}
			if (Ctrl->E.active && Ctrl->E.col >= ncol) {
  				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -E option: Chosen column exceeds column range (0-%ld)\n", ncol-1);
				Return (EXIT_FAILURE);
			}
			if (Ctrl->T.active && Ctrl->T.col >= ncol) {
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Chosen column exceeds column range (0-%ld)\n", ncol-1);
				Return (EXIT_FAILURE);
			}
			if (Ctrl->T.active) ncol = Ctrl->T.col + 1;
			if (give_r_string) ncol = 2;	/* Since we only will concern ourselves with lon/lat or x/y */

			/* Now we know number of columns, so allocate memory */

			Q = GMT_quad_init (GMT, ncol);
			xyzmin = GMT_memory (GMT, NULL, ncol, double);
			xyzmax = GMT_memory (GMT, NULL, ncol, double);

			for (i = 0; i < ncol; i++) {	/* Initialize */
				xyzmin[i] = +DBL_MAX;
				xyzmax[i] = -DBL_MAX;
			}
			n = 0;
			if (Ctrl->I.active && ncol < 2 && !Ctrl->C.active) Ctrl->I.active = FALSE;
			first_data_record = FALSE;
			if (Ctrl->C.active && (error = GMT_set_cols (GMT, GMT_OUT, 2*ncol))) Return (error);
		}

		/* Process all columns and update the corresponding minmax arrays */

		if (Ctrl->E.active) {		/* See if this record should become the chosen one  */
			if (!GMT_is_dnan (in[Ctrl->E.col])) {		/* But skip NaNs */
				value = (work_on_abs_value) ? fabs (in[Ctrl->E.col]) : in[Ctrl->E.col];
				if (Ctrl->E.mode == -1 && value < e_min) {	/* Lower than previous low */
					e_min = value;
					strcpy (chosen, GMT->current.io.current_record);
				}
				else if (Ctrl->E.mode == +1 && value > e_max) {	/* Higher than previous high */
					e_max = value;
					strcpy (chosen, GMT->current.io.current_record);
				}
			}
		}
		else {	/* Update min/max values for each column */
			for (i = 0; i < ncol; i++) {
				if (GMT_is_dnan (in[i])) continue;	/* We always skip NaNs */
				if (GMT->current.io.col_type[GMT_IN][i] == GMT_IS_LON) {	/* Longitude requires more work */
					/* We must keep separate min/max for both Dateline and Greenwich conventions */
					GMT_quad_add (GMT, &Q[i], in[i]);
				}
				else if ((i == 0 && Ctrl->S.xbar) || (i == 1 && Ctrl->S.ybar)) {
					/* Add/subtract value from error bar column */
					j = (i == 1 && Ctrl->S.xbar) ? 3 : 2;
					value = fabs(in[j]);
					if (in[i] - value < xyzmin[i]) xyzmin[i] = in[i] - value;
					if (in[i] + value > xyzmax[i]) xyzmax[i] = in[i] + value;
				}
				else {	/* Plain column value */
					if (in[i] < xyzmin[i]) xyzmin[i] = in[i];
					if (in[i] > xyzmax[i]) xyzmax[i] = in[i];
				}
			}
			n++;	/* Number of records processed in current block (all/table/segment; see -A) */
		}
		if (file[0] == 0) strcpy (file, GMT->current.io.current_filename[GMT_IN]);	/* Grab name of current file while we can */
		
	}
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (!got_stuff) GMT_report (GMT, GMT_MSG_FATAL, "No input data found!\n");

	GMT_free (GMT, xyzmin);
	GMT_free (GMT, xyzmax);
	GMT_free (GMT, Q);

	GMT->current.io.geo.range = save_range;	/* Restore what we changed */

	Return (GMT_OK);
}
