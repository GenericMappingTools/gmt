/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Brief synopsis: minmax.c will read ascii or binary tables and report the
 * extreme values for all columns
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	 API
 */

#define THIS_MODULE k_mod_minmax /* I am minmax */

#include "gmt.h"

EXTERN_MSC int gmt_geo_C_format (struct GMT_CTRL *C);
EXTERN_MSC unsigned int GMT_log_array (struct GMT_CTRL *C, double min, double max, double delta, double **array);

#define REPORT_PER_DATASET	0
#define REPORT_PER_TABLE	1
#define REPORT_PER_SEGMENT	2

struct MINMAX_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
		unsigned int mode;	/* 0 reports range for all tables, 1 is per table, 2 is per segment */
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct E {	/* -E<L|l|H|h><col> */
		bool active;
		bool abs;
		int mode;	/* -1, 0, +1 */
		unsigned int col;
	} E;
	struct I {	/* -Idx[/dy[/<dz>..]] */
		bool active;
		unsigned int ncol;
		double inc[GMT_MAX_COLUMNS];
	} I;
	struct S {	/* -S[x|y] */
		bool active;
		bool xbar, ybar;
	} S;
	struct T {	/* -T<dz>[/<col>] */
		bool active;
		double inc;
		unsigned int col;
	} T;
};

int strip_blanks_and_output (struct GMT_CTRL *GMT, char *text, double x, int col)
{	/* Alternative to GMT_ascii_output_col that strips off leading blanks first */

	int k;

	GMT_ascii_format_col (GMT, text, x, col);
	for (k = 0; text[k] && text[k] == ' '; k++);
	return (k);	/* This is the position in text that we should start reporting from */
}

void *New_minmax_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MINMAX_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MINMAX_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.col = UINT_MAX;	/* Meaning not set */
	return (C);
}

void Free_minmax_Ctrl (struct GMT_CTRL *GMT, struct MINMAX_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

int GMT_minmax_usage (struct GMTAPI_CTRL *C, int level)
{
	struct GMT_CTRL *GMT = C->GMT;

	gmt_module_show_name_and_purpose (THIS_MODULE);
	GMT_message (GMT, "usage: minmax [<table>] [-Aa|f|s] [-C] [-E<L|l|H|h><col>] [-I[p]<dx>[/<dy>[/<dz>..]]\n");
	GMT_message (GMT, "\t[-S[x][y]] [-T<dz>[/<col>]] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Select reports for (a)ll [Default], per (f)ile, or per (s)egment.\n");
	GMT_message (GMT, "\t-C Format the min and max into separate columns.\n");
	GMT_message (GMT, "\t-E Return the record with extreme value in specified column <col> [last column].\n");
	GMT_message (GMT, "\t   Specify l or h for min or max value, respectively.  Upper case L or H\n");
	GMT_message (GMT, "\t   means we operate instead on the absolute values of the data.\n");
	GMT_message (GMT, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy (assumes at least 2 columns).\n");
	GMT_message (GMT, "\t   If -C is set then no -R string is issued.  Instead, the number of increments\n");
	GMT_message (GMT, "\t   given determines how many columns are rounded off to the nearest multiple.\n");
	GMT_message (GMT, "\t   If only one increment is given we also use it for the second column (for backwards compatibility).\n");
	GMT_message (GMT, "\t   To override this behaviour, use -Ip<dx>.\n");
	GMT_message (GMT, "\t   If input data are regularly distributed we use observed phase shifts in determining -R [no phase shift]\n");
	GMT_message (GMT, "\t     and allow -r to change from gridline-registration to pixel-registration.\n");
	GMT_message (GMT, "\t-S Add extra space for error bars. Useful together with -I.\n");
	GMT_message (GMT, "\t   -Sx leaves space for horizontal error bar using value in third (2) column.\n");
	GMT_message (GMT, "\t   -Sy leaves space for vertical error bar using value in third (2) column.\n");
	GMT_message (GMT, "\t   -S or -Sxy leaves space for both error bars using values in third&fourth (2&3) columns.\n");
	GMT_message (GMT, "\t-T Return textstring -Tzmin/zmax/dz to nearest multiple of the given dz.\n");
	GMT_message (GMT, "\t   Calculations are based on the first (0) column only.  Append /<col> to use another column.\n");
	GMT_explain_options (GMT, "VC2fghiF:.");
	
	return (EXIT_FAILURE);
}

int GMT_minmax_parse (struct GMTAPI_CTRL *C, struct MINMAX_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to minmax and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j;
	unsigned int n_errors = 0, k;
	bool special = false;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Reporting unit */
				Ctrl->A.active = true;
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
				Ctrl->C.active = true;
				break;
			case 'E':	/* Extrema reporting */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->E.abs = true;
					case 'l':
						Ctrl->E.mode = -1;
						break;
					case 'H':
						Ctrl->E.abs = true;
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
				Ctrl->I.active = true;
				if (opt->arg[0] == 'p') special = true;
				j = (special) ? 1 : 0;
				Ctrl->I.ncol = GMT_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS);
				break;
			case 'S':	/* Error bar output */
				Ctrl->S.active = true;
				j = 0;
				while (opt->arg[j]) {
					if (opt->arg[j] == 'x') Ctrl->S.xbar = true;
					if (opt->arg[j] == 'y') Ctrl->S.ybar = true;
					j++;
				}
				if (j == 2) Ctrl->S.xbar = Ctrl->S.ybar = true;
				break;
			case 'T':	/* makecpt range/inc string */
				Ctrl->T.active = true;
				j = sscanf (opt->arg, "%lf/%d", &Ctrl->T.inc, &Ctrl->T.col);
				if (j == 1) Ctrl->T.col = 0;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);
	if (Ctrl->I.active && !special && Ctrl->I.ncol == 1) {		/* Special case of dy = dx if not given */
		Ctrl->I.inc[GMT_Y] = Ctrl->I.inc[GMT_X];
		Ctrl->I.ncol = 2;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->C.active && Ctrl->I.ncol < 2, "Syntax error: -Ip requires -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->T.active, "Syntax error: Only one of -I and -T can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0 , "Syntax error -T option: Must specify a positive increment\n");
	for (k = 0; Ctrl->I.active && k < Ctrl->I.ncol; k++) {
		n_errors += GMT_check_condition (GMT, Ctrl->I.inc[k] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += GMT_check_binary_io (GMT, 1);
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_minmax_Ctrl (GMT, Ctrl); GMT_free (GMT, xyzmin); GMT_free (GMT, xyzmax); GMT_free (GMT, Q); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_minmax (struct GMTAPI_CTRL *API, int mode, void *args)
{
	bool error = false, got_stuff = false, first_data_record, give_r_string = false;
	bool brackets = false, work_on_abs_value, do_report, save_range, done;
	int i, j;
	unsigned int col, ncol = 0, fixed_phase[2] = {1, 1}, min_cols;
	uint64_t n = 0;

	char file[GMT_BUFSIZ], chosen[GMT_BUFSIZ], record[GMT_BUFSIZ], buffer[GMT_BUFSIZ], delimeter[2];

	double *xyzmin = NULL, *xyzmax = NULL, *in = NULL, phase[2] = {0.0, 0.0}, this_phase, off;
	double west, east, south, north, low, high, value, e_min = DBL_MAX, e_max = -DBL_MAX;

	struct GMT_QUAD *Q = NULL;
	struct MINMAX_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options && options->option == GMTAPI_OPT_USAGE) bailout (GMT_minmax_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_minmax_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_gmt_module (API, THIS_MODULE, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghirs>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_minmax_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_minmax_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the minmax main code ----------------------------*/

	give_r_string = (Ctrl->I.active && !Ctrl->C.active);
	delimeter[0] = (Ctrl->C.active) ? '\t' : '/';
	delimeter[1] = '\0';
	GMT_memset (file, GMT_BUFSIZ, char);
	off = (GMT->common.r.active) ? 0.5 : 0.0;

	brackets = !Ctrl->C.active;
	work_on_abs_value = (Ctrl->E.active && Ctrl->E.abs);
	if (GMT_x_is_lon (GMT, GMT_IN)) {	/* Must check that output format won't mess things up by printing west > east */
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
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT) != GMT_OK) {
		Return (API->error);
	}

	if (Ctrl->C.active) {	/* Must set output column types since each input col will take up two output cols. */
		int col_type[GMT_MAX_COLUMNS];
		GMT_memcpy (col_type, GMT->current.io.col_type[GMT_IN], GMT_MAX_COLUMNS, int);	/* Duplicate input col types */
		for (col = 0; col < GMT_MAX_COLUMNS/2; col++) GMT->current.io.col_type[GMT_OUT][2*col] = GMT->current.io.col_type[GMT_OUT][2*col+1] = col_type[col];
	}
		
	save_range = GMT->current.io.geo.range;
	first_data_record = true;
	done = false;
	while (!done) {	/* Keep returning records until we reach EOF of last file */
		in = GMT_Get_Record (API, GMT_READ_DOUBLE | GMT_FILE_BREAK, NULL);
		do_report = false;

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
		if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */
		if ((GMT_REC_IS_SEG_HEADER (GMT) && Ctrl->A.mode != REPORT_PER_SEGMENT)) continue;	/* Since we are not reporting per segment they are just headers as far as we are concerned */
		
		if (GMT_REC_IS_SEG_HEADER (GMT) || (GMT_REC_IS_FILE_BREAK (GMT) && Ctrl->A.mode == REPORT_PER_TABLE) || GMT_REC_IS_EOF (GMT)) {	/* Time to report */
			if (GMT_REC_IS_SEG_HEADER (GMT) && GMT->current.io.seg_no == 0) continue;	/* Very first segment header means there is no prior segment to report on yet */
			if (GMT_REC_IS_EOF (GMT)) {	/* We are done after this since we hit EOF */
				done = true;
				GMT->current.io.seg_no++;	/* Must manually increment since we are not reading any futher */
			}
			if (n == 0) continue;			/* This segment, table, or data set had no data records, skip */
			
			/* Here we must issue a report */
			
			do_report = true;
 			for (col = 0; col < ncol; col++) if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {	/* Must finalize longitudes first */
				j = GMT_quad_finalize (GMT, &Q[col]);
				GMT->current.io.geo.range = Q[col].range[j];		/* Override this setting explicitly */
				xyzmin[col] = Q[col].min[j];	xyzmax[col] = Q[col].max[j];
			}
			if (give_r_string) {	/* Return -R string */
				if (fixed_phase[GMT_X] && fixed_phase[GMT_Y]) {	/* Got xy[z] data that lined up on a grid, so use the common phase shift */
					GMT_report (GMT, GMT_MSG_NORMAL, "Input (x,y) data are regularly distributed; fixed phase shifts are %g/%g.\n", phase[GMT_X], phase[GMT_Y]);
				}
				else {	/* Data not on grid, just return bounding box rounded off to nearest inc */
					buffer[0] = '.';	buffer[1] = 0;
					if (GMT->common.r.active) strcpy (buffer, " (-r is ignored).");
					GMT_report (GMT, GMT_MSG_NORMAL, "Input (x,y) data are irregularly distributed; phase shifts set to 0/0%s\n", buffer);
					phase[GMT_X] = phase[GMT_Y] = off = 0.0;
				}
				west  = (floor ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
				east  = (ceil  ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
				south = (floor ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
				north = (ceil  ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
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
						sprintf (record, "%s-%" PRIu64, file, GMT->current.io.seg_no);
					else									/* Either table mode or only one table in dataset */
						sprintf (record, "%s", file);
					sprintf (buffer, ": N = %" PRIu64 "\t", n);					/* Number of records in this item */
					strcat (record, buffer);
				}
				for (col = 0; col < ncol; col++) {	/* Report min/max for each column in the format controlled by -C */
					if (xyzmin[col] == DBL_MAX)	/* Encountered NaNs only */
						low = high = GMT->session.d_NaN;
					else if (col < Ctrl->I.ncol) {	/* Special treatment for x and y (and perhaps more) if -I selected */
						low  = (Ctrl->I.active) ? floor (xyzmin[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmin[col];
						high = (Ctrl->I.active) ? ceil  (xyzmax[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmax[col];
					}
					else {	/* Just the facts, ma'am */
						low = xyzmin[col];
						high = xyzmax[col];
					}
					if (Ctrl->C.active) {
						GMT->current.io.curr_rec[2*col] = low;
						GMT->current.io.curr_rec[2*col+1] = high;
					}
					else {
						if (brackets) strcat (record, "<");
						GMT_ascii_format_col (GMT, buffer, low, col);
						strcat (record, buffer);
						strcat (record, delimeter);
						GMT_ascii_format_col (GMT, buffer, high, col);
						strcat (record, buffer);
						if (brackets) strcat (record, ">");
						if (col < (ncol - 1)) strcat (record, "\t");
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
			got_stuff = true;		/* We have at least reported something */
			for (col = 0; col < ncol; col++) {	/* Reset counters for next block */
				xyzmin[col] = DBL_MAX;
				xyzmax[col] = -DBL_MAX;
			}
			GMT_quad_reset (GMT, Q, ncol);
			n = 0;
			file[0] = '\0';
			fixed_phase[GMT_X] = fixed_phase[GMT_Y] = 1;	/* Get ready for next batch */
			if (done || do_report || GMT_REC_IS_FILE_BREAK (GMT)) continue;	/* We are done OR have no data record to process yet */
		}
		if (GMT_REC_IS_FILE_BREAK (GMT)) continue;

		/* We get here once we have read a data record */
		
		if (first_data_record) {	/* First time we read data, we must allocate arrays based on the number of columns */

			ncol = GMT_get_cols (GMT, GMT_IN);
			if (Ctrl->E.active && Ctrl->E.col == UINT_MAX) Ctrl->E.col = ncol - 1;	/* Default is last column */
			min_cols = 2;	if (Ctrl->S.xbar) min_cols++;	if (Ctrl->S.ybar) min_cols++;
			if (Ctrl->S.active && min_cols > ncol) {
				GMT_report (GMT, GMT_MSG_FATAL, "Not enough columns to support the -S option\n");
				Return (EXIT_FAILURE);
			}
			if (Ctrl->E.active && Ctrl->E.col >= ncol) {
  				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -E option: Chosen column exceeds column range (0-%d)\n", ncol-1);
				Return (EXIT_FAILURE);
			}
			if (Ctrl->T.active && Ctrl->T.col >= ncol) {
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Chosen column exceeds column range (0-%d)\n", ncol-1);
				Return (EXIT_FAILURE);
			}
			if (Ctrl->T.active) ncol = Ctrl->T.col + 1;
			if (give_r_string) ncol = 2;	/* Since we only will concern ourselves with lon/lat or x/y */

			/* Now we know number of columns, so allocate memory */

			Q = GMT_quad_init (GMT, ncol);
			xyzmin = GMT_memory (GMT, NULL, ncol, double);
			xyzmax = GMT_memory (GMT, NULL, ncol, double);

			for (col = 0; col < ncol; col++) {	/* Initialize */
				xyzmin[col] = DBL_MAX;
				xyzmax[col] = -DBL_MAX;
			}
			n = 0;
			if (Ctrl->I.active && ncol < 2 && !Ctrl->C.active) Ctrl->I.active = false;
			first_data_record = false;
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
			for (col = 0; col < ncol; col++) {
				if (GMT_is_dnan (in[col])) continue;	/* We always skip NaNs */
				if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {	/* Longitude requires more work */
					/* We must keep separate min/max for both Dateline and Greenwich conventions */
					GMT_quad_add (GMT, &Q[col], in[col]);
				}
				else if ((col == 0 && Ctrl->S.xbar) || (col == 1 && Ctrl->S.ybar)) {
					/* Add/subtract value from error bar column */
					j = (col == 1 && Ctrl->S.xbar) ? 3 : 2;
					value = fabs(in[j]);
					if (in[col] - value < xyzmin[col]) xyzmin[col] = in[col] - value;
					if (in[col] + value > xyzmax[col]) xyzmax[col] = in[col] + value;
				}
				else {	/* Plain column value */
					if (in[col] < xyzmin[col]) xyzmin[col] = in[col];
					if (in[col] > xyzmax[col]) xyzmax[col] = in[col];
				}
				if (give_r_string && col < GMT_Z && fixed_phase[col]) {
					this_phase = MOD (in[col], Ctrl->I.inc[col]);
					if (fixed_phase[col] == 1)
						phase[col] = this_phase, fixed_phase[col] = 2;	/* Initializes phase the first time */
					if (!doubleAlmostEqualZero (phase[col], this_phase))
						fixed_phase[col] = 0;	/* Phase not constant, not a grid */
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

	GMT->current.io.geo.range = save_range;	/* Restore what we changed */

	Return (GMT_OK);
}
