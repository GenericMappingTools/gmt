/*--------------------------------------------------------------------
 *    $Id$
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
 * Brief synopsis: gmtinfo.c will read ascii or binary tables and report the
 * extreme values for all columns
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	 API
 */

#define THIS_MODULE_NAME	"gmtinfo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Get information about data tables"
#define THIS_MODULE_KEYS	"<DI,>?O"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbdfghiors" GMT_OPT("HMm")

int gmt_geo_C_format (struct GMT_CTRL *GMT);
unsigned int GMT_log_array (struct GMT_CTRL *GMT, double min, double max, double delta, double **array);

#define REPORT_PER_DATASET	0
#define REPORT_PER_TABLE	1
#define REPORT_PER_SEGMENT	2

#define BEST_FOR_SURF	1
#define BEST_FOR_FFT	2
#define ACTUAL_BOUNDS	3

struct MINMAX_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
		unsigned int mode;	/* 0 reports range for all tables, 1 is per table, 2 is per segment */
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D[dx[/dy[/<dz>..]]] */
		bool active;
		unsigned int ncol;
		unsigned int mode;	/* 0 means center, 1 means use dx granularity */
		double inc[GMT_MAX_COLUMNS];
	} D;
	struct E {	/* -E<L|l|H|h><col> */
		bool active;
		bool abs;
		int mode;	/* -1, 0, +1 */
		uint64_t col;
	} E;
	struct I {	/* -I[f|p|s]dx[/dy[/<dz>..]] */
		bool active;
		unsigned int ncol;
		unsigned int mode;	/* Nominally 0, unless set to BEST_FOR_SURF, BEST_FOR_FFT or ACTUAL_BOUNDS */
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

	GMT_ascii_format_col (GMT, text, x, GMT_OUT, col);
	for (k = 0; text[k] && text[k] == ' '; k++);
	return (k);	/* This is the position in text that we should start reporting from */
}

void *New_gmtinfo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MINMAX_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MINMAX_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.col = UINT_MAX;	/* Meaning not set */
	return (C);
}

void Free_gmtinfo_Ctrl (struct GMT_CTRL *GMT, struct MINMAX_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_gmtinfo_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtinfo [<table>] [-Aa|f|s] [-C] [-D[<dx>[/<dy>]] [-E<L|l|H|h><col>] [-I[p|f|s]<dx>[/<dy>[/<dz>..]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-S[x][y]] [-T<dz>[/<col>]] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_d_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Select reports for (a)ll [Default], per (f)ile, or per (s)egment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Format the min and max into separate columns; -o may be used to limit output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Modifies results obtained by -I by shifting the region to better align with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the data center.  Optionally, append granularity for this shift [exact].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Return the record with extreme value in specified column <col> [last column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify l or h for min or max value, respectively.  Upper case L or H\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   means we operate instead on the absolute values of the data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy (assumes at least 2 columns).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I- to just report the min/max extent in the -Rw/e/s/n string (no multiples).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -C is set then no -R string is issued.  Instead, the number of increments\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   given determines how many columns are rounded off to the nearest multiple.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only one increment is given we also use it for the second column (for backwards compatibility).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To override this behaviour, use -Ip<dx>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If input data are regularly distributed we use observed phase shifts in determining -R [no phase shift]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     and allow -r to change from gridline-registration to pixel-registration.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -If<dx>[/<dy>] to report an extended region optimized for fastest results in FFTs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Is<dx>[/<dy>] to report an extended region optimized for fastest results in surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Add extra space for error bars. Useful together with -I.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sx leaves space for horizontal error bar using value in third (2) column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sy leaves space for vertical error bar using value in third (2) column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -S or -Sxy leaves space for both error bars using values in third&fourth (2&3) columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Return textstring -Tzmin/zmax/dz to nearest multiple of the given dz.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Calculations are based on the first (0) column only.  Append /<col> to use another column.\n");
	GMT_Option (API, "V,bi2,d,f,g,h,i,o,r,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_gmtinfo_parse (struct GMT_CTRL *GMT, struct MINMAX_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtinfo and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j;
	unsigned int n_errors = 0, k;
	bool special = false;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A. Flags are a|f|s.\n");
						break;
				}
				break;
			case 'C':	/* Column output */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Region adjustment Granularity */
				Ctrl->D.active = true;
				if (opt->arg[0]) {
					Ctrl->D.ncol = GMT_getincn (GMT, opt->arg, Ctrl->D.inc, GMT_MAX_COLUMNS);
					Ctrl->D.mode = 1;
					GMT_check_lattice (GMT, Ctrl->D.inc, NULL, &Ctrl->D.active);
				}
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E. Flags are L|l|H|h.\n");
						break;
				}
				if (opt->arg[1]) Ctrl->E.col = atoi (&opt->arg[1]);
				break;
			case 'I':	/* Granularity */
				Ctrl->I.active = true;
				j = 1;
				switch (opt->arg[0]) {
					case 'p': special = true; break;
					case 'f': Ctrl->I.mode = BEST_FOR_FFT; break;
					case 's': Ctrl->I.mode = BEST_FOR_SURF; break;
					case '-': Ctrl->I.mode = ACTUAL_BOUNDS; break;
					default: j = 0;	break;
				}
				Ctrl->I.ncol = (Ctrl->I.mode == ACTUAL_BOUNDS) ? 2 : GMT_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS);
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

			case 'b':	/* -b[i]c will land here */
				if (GMT_compat_check (GMT, 4)) break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->I.mode != ACTUAL_BOUNDS) GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);
	if (Ctrl->I.active && special && Ctrl->I.ncol > 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Ip. Only a single increment is expected.\n");
		n_errors++;
	}
	if (Ctrl->I.active && !special && Ctrl->I.ncol == 1) {		/* Special case of dy = dx if not given */
		Ctrl->I.inc[GMT_Y] = Ctrl->I.inc[GMT_X];
		Ctrl->I.ncol = 2;
	}
	if (Ctrl->D.active && Ctrl->D.ncol == 1) {		/* Special case of dy = dx if not given */
		Ctrl->D.inc[GMT_Y] = Ctrl->D.inc[GMT_X];
		Ctrl->D.ncol = 2;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && !Ctrl->I.active, "Syntax error: -D requires -I\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->C.active && Ctrl->I.ncol < 2, "Syntax error: -Ip requires -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->T.active, "Syntax error: Only one of -I and -T can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0 , "Syntax error -T option: Must specify a positive increment\n");
	for (k = 0; Ctrl->I.active && k < Ctrl->I.ncol; k++) {
		n_errors += GMT_check_condition (GMT, Ctrl->I.mode != ACTUAL_BOUNDS && Ctrl->I.inc[k] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += GMT_check_binary_io (GMT, 1);
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtinfo_Ctrl (GMT, Ctrl); if (xyzmin) GMT_free (GMT, xyzmin); if (xyzmax) GMT_free (GMT, xyzmax); if (Q) GMT_free (GMT, Q); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtinfo (void *V_API, int mode, void *args)
{
	bool got_stuff = false, first_data_record, give_r_string = false;
	bool brackets = false, work_on_abs_value, do_report, done;
	int i, j, error = 0, col_type[GMT_MAX_COLUMNS];
	unsigned int fixed_phase[2] = {1, 1}, min_cols, o_mode, save_range;
	uint64_t col, ncol = 0, n = 0;

	char file[GMT_BUFSIZ] = {""}, chosen[GMT_BUFSIZ] = {""}, record[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = {""}, delimeter[2] = {""};

	double *xyzmin = NULL, *xyzmax = NULL, *in = NULL, *dchosen = NULL, phase[2] = {0.0, 0.0}, this_phase, off;
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0, low, high, value, e_min = DBL_MAX, e_max = -DBL_MAX;

	struct GMT_QUAD *Q = NULL;
	struct MINMAX_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtinfo_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options && options->option == GMT_OPT_USAGE) bailout (GMT_gmtinfo_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options && options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtinfo_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtinfo_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtinfo_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtinfo main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	give_r_string = (Ctrl->I.active && !Ctrl->C.active);
	delimeter[0] = (Ctrl->C.active) ? '\t' : '/';
	delimeter[1] = '\0';
	off = (GMT->common.r.active) ? 0.5 : 0.0;

	brackets = !Ctrl->C.active;
	work_on_abs_value = (Ctrl->E.active && Ctrl->E.abs);
	if (GMT_x_is_lon (GMT, GMT_IN)) {	/* Must check that output format won't mess things up by printing west > east */
		if (!strcmp (GMT->current.setting.format_geo_out, "D")) {
			strcpy (GMT->current.setting.format_geo_out, "+D");
			GMT_err_fail (GMT, gmt_geo_C_format (GMT), "");
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: FORMAT_GEO_OUT reset from D to %s to ensure east > west\n", GMT->current.setting.format_geo_out);
		}
		else if (!strcmp (GMT->current.setting.format_geo_out, "ddd:mm:ss")) {
			strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ssF");
			GMT_err_fail (GMT, gmt_geo_C_format (GMT), "");
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: FORMAT_GEO_OUT reset from ddd:mm:ss to %s to ensure east > west\n", GMT->current.setting.format_geo_out);
		}
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, 0))) Return (error);
	o_mode = (Ctrl->C.active) ? GMT_IS_DATASET : GMT_IS_TEXTSET;
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, o_mode, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Begin_IO (API, o_mode, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) {
		Return (API->error);
	}

	if (Ctrl->C.active) {	/* Must set output column types since each input col will produce two output cols. */
		GMT_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], GMT_MAX_COLUMNS, int);	/* Save previous output col types */
		for (col = 0; col < GMT_MAX_COLUMNS/2; col++) GMT->current.io.col_type[GMT_OUT][2*col] = GMT->current.io.col_type[GMT_OUT][2*col+1] = GMT->current.io.col_type[GMT_IN][col];
	}
		
	save_range = GMT->current.io.geo.range;
	first_data_record = true;
	done = false;
	while (!done) {	/* Keep returning records until we reach EOF of last file */
		in = GMT_Get_Record (API, GMT_READ_DOUBLE | GMT_READ_FILEBREAK, NULL);
		do_report = false;

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
		if (GMT_REC_IS_TABLE_HEADER (GMT)) continue;	/* Skip table headers */
		if ((GMT_REC_IS_SEGMENT_HEADER (GMT) && Ctrl->A.mode != REPORT_PER_SEGMENT)) continue;	/* Since we are not reporting per segment they are just headers as far as we are concerned */
		
		if (GMT_REC_IS_SEGMENT_HEADER (GMT) || (GMT_REC_IS_FILE_BREAK (GMT) && Ctrl->A.mode == REPORT_PER_TABLE) || GMT_REC_IS_EOF (GMT)) {	/* Time to report */
			if (GMT_REC_IS_SEGMENT_HEADER (GMT) && GMT->current.io.seg_no == 0) continue;	/* Very first segment header means there is no prior segment to report on yet */
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
			if (Ctrl->I.active) {	/* Must report multiples of dx/dy etc */
				if (n > 1 && fixed_phase[GMT_X] && fixed_phase[GMT_Y]) {	/* Got xy[z] data that lined up on a grid, so use the common phase shift */
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input (x,y) data are regularly distributed; fixed phase shifts are %g/%g.\n", phase[GMT_X], phase[GMT_Y]);
				}
				else {	/* Data not on grid, just return bounding box rounded off to nearest inc */
					buffer[0] = '.';	buffer[1] = 0;
					if (GMT->common.r.active) strcpy (buffer, " (-r is ignored).");
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input (x,y) data are irregularly distributed; phase shifts set to 0/0%s\n", buffer);
					phase[GMT_X] = phase[GMT_Y] = off = 0.0;
				}
				if (Ctrl->I.mode == ACTUAL_BOUNDS) {
					west  = xyzmin[GMT_X];	east  = xyzmax[GMT_X];
					south = xyzmin[GMT_Y];	north = xyzmax[GMT_Y];
				}
				else { /* Round off to nearest inc */
					west  = (floor ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					east  = (ceil  ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					south = (floor ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
					north = (ceil  ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
					if (Ctrl->D.active) {	/* Center the selected region * better */
						double off = 0.5 * (xyzmin[GMT_X] + xyzmax[GMT_X] - west - east);
						if (Ctrl->D.inc[GMT_X] > 0.0) off = rint (off / Ctrl->D.inc[GMT_X]) * Ctrl->D.inc[GMT_X];
						west += off;	east += off;
						off = 0.5 * (xyzmin[GMT_Y] + xyzmax[GMT_Y] - south - north);
						if (Ctrl->D.inc[GMT_Y] > 0.0) off = rint (off / Ctrl->D.inc[GMT_Y]) * Ctrl->D.inc[GMT_Y];
						south += off;	north += off;
					}
				}
				if (GMT_is_geographic (GMT, GMT_IN)) {
					if (east < west) east += 360.0;
					if (south < -90.0) south = -90.0;
					if (north > +90.0) north = +90.0;
				}
				if (Ctrl->I.mode == BEST_FOR_FFT || Ctrl->I.mode == BEST_FOR_SURF) {	/* Wish to extend the region to optimize the resulting nx/ny */
					unsigned int sub, add, in_dim[2], out_dim[2];
					double ww, ee, ss, nn;
					in_dim[GMT_X] = GMT_get_n (GMT, west, east, Ctrl->I.inc[GMT_X], GMT->common.r.active);
					in_dim[GMT_Y] = GMT_get_n (GMT, south, north, Ctrl->I.inc[GMT_Y], GMT->common.r.active);
					ww = west;	ee = east; ss = south;	nn = north;
					GMT_best_dim_choice (GMT, Ctrl->I.mode, in_dim, out_dim);
					sub = (out_dim[GMT_X] - in_dim[GMT_X]) / 2;	add = out_dim[GMT_X] - in_dim[GMT_X] - sub;
					west  -= sub * Ctrl->I.inc[GMT_X];		east  += add * Ctrl->I.inc[GMT_X];
					sub = (out_dim[GMT_Y] - in_dim[GMT_Y]) / 2;	add = out_dim[GMT_Y] - in_dim[GMT_Y] - sub;
					south -= sub * Ctrl->I.inc[GMT_Y];		north += add * Ctrl->I.inc[GMT_Y];
					GMT_Report (API, GMT_MSG_VERBOSE, "Initial -R: %g/%g/%g/%g [nx = %u ny = %u] --> Suggested -R:  %g/%g/%g/%g [nx = %u ny = %u].\n",
						ww, ee, ss, nn, in_dim[GMT_X], in_dim[GMT_Y], west, east, south, north, out_dim[GMT_X], out_dim[GMT_Y]);
				}
			}
			if (give_r_string) {	/* Return -R string */
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
			else if (Ctrl->E.active) {	/* Return extreme record */
				int sep = 0;
				if (GMT->common.b.active[GMT_IN]) {	/* Make ascii record */
					GMT_memset (chosen, GMT_BUFSIZ, char);
					for (col = 0; col < ncol; col++) {	/* Report min/max for each column in the format controlled by -C */
						GMT_add_to_record (GMT, chosen, dchosen[col], col, sep);
						sep = 1;
					}
				}
				strncpy (record, chosen, GMT_BUFSIZ);
			}
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
						if (Ctrl->I.mode && col == GMT_X) {
							low  = west;
							high = east;
						}
						else if (Ctrl->I.mode && col == GMT_Y) {
							low  = south;
							high = north;
						}
						else {
							low  = (Ctrl->I.active) ? floor (xyzmin[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmin[col];
							high = (Ctrl->I.active) ? ceil  (xyzmax[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmax[col];
						}
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
						GMT_ascii_format_col (GMT, buffer, low, GMT_OUT, col);
						strcat (record, buffer);
						strcat (record, delimeter);
						GMT_ascii_format_col (GMT, buffer, high, GMT_OUT, col);
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
			if (Ctrl->E.active) {
				if (Ctrl->E.col == UINT_MAX) Ctrl->E.col = ncol - 1;	/* Default is last column */
				if (GMT->common.b.active[GMT_IN]) dchosen = GMT_memory (GMT, NULL, ncol, double);
			}
			min_cols = 2;	if (Ctrl->S.xbar) min_cols++;	if (Ctrl->S.ybar) min_cols++;
			if (Ctrl->S.active && min_cols > ncol) {
				GMT_Report (API, GMT_MSG_NORMAL, "Not enough columns to support the -S option\n");
				Return (EXIT_FAILURE);
			}
			if (Ctrl->E.active && Ctrl->E.col >= ncol) {
  				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Chosen column exceeds column range (0-%d)\n", ncol-1);
				Return (EXIT_FAILURE);
			}
			if (Ctrl->T.active && Ctrl->T.col >= ncol) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Chosen column exceeds column range (0-%d)\n", ncol-1);
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
					if (GMT->common.b.active[GMT_IN])
						GMT_memcpy (dchosen, in, ncol, double);
					else
						strncpy (chosen, GMT->current.io.current_record, GMT_BUFSIZ);
				}
				else if (Ctrl->E.mode == +1 && value > e_max) {	/* Higher than previous high */
					e_max = value;
					if (GMT->common.b.active[GMT_IN])
						GMT_memcpy (dchosen, in, ncol, double);
					else
						strncpy (chosen, GMT->current.io.current_record, GMT_BUFSIZ);
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
		}
		n++;	/* Number of records processed in current block (all/table/segment; see -A) */
		if (file[0] == 0) strncpy (file, GMT->current.io.current_filename[GMT_IN], GMT_BUFSIZ);	/* Grab name of current file while we can */
		
	}
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (!got_stuff) GMT_Report (API, GMT_MSG_NORMAL, "No input data found!\n");

	GMT->current.io.geo.range = save_range;	/* Restore what we changed */
	if (Ctrl->C.active) {	/* Restore previous output col types */
		GMT_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, GMT_MAX_COLUMNS, int);
	}
	if (Ctrl->E.active && GMT->common.b.active[GMT_IN])
		GMT_free (GMT, dchosen);

	Return (GMT_OK);
}
