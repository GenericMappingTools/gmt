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
 * Brief synopsis: gmtinfo.c will read ASCII or binary tables and report the
 * extreme values for all columns
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtinfo"
#define THIS_MODULE_MODERN_NAME	"gmtinfo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Get information about data tables"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioqrs" GMT_OPT("HMm")

EXTERN_MSC int gmtlib_geo_C_format (struct GMT_CTRL *GMT);
EXTERN_MSC unsigned int gmtlib_log_array (struct GMT_CTRL *GMT, double min, double max, double delta, double **array);

#define REPORT_PER_DATASET	0
#define REPORT_PER_TABLE	1
#define REPORT_PER_SEGMENT	2

#define BEST_FOR_SURF	1
#define BEST_FOR_FFT	2
#define ACTUAL_BOUNDS	3
#define BOUNDBOX		4

#define GMT_INFO_TOTAL		1
#define GMT_INFO_TABLEINFO	2
#define GMT_INFO_DATAINFO	3

struct MINMAX_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	unsigned int n_files;
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
	struct E {	/* -E<L|l|H|h>[<col>] */
		bool active;
		bool abs;
		int mode;	/* -1, 0, +1 */
		uint64_t col;
	} E;
	struct F {	/* -F<i|d|t> */
		bool active;
		int mode;	/*  */
	} F;
	struct I {	/* -I[b|e|f|p|s]dx[/dy[/<dz>..]][[+e|r|R<incs>]] */
		bool active;
		unsigned int extend;
		unsigned int ncol;
		unsigned int mode;	/* Nominally 0, unless set to BEST_FOR_SURF, BEST_FOR_FFT or ACTUAL_BOUNDS */
		double inc[GMT_MAX_COLUMNS];
		double delta[4];
	} I;
	struct L {	/* -L */
		bool active;
	} L;
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

GMT_LOCAL int strip_blanks_and_output (struct GMT_CTRL *GMT, char *text, double x, int col) {
	/* Alternative to GMT_ascii_output_col that strips off leading blanks first */

	int k;

	gmt_ascii_format_col (GMT, text, x, GMT_OUT, col);
	for (k = 0; text[k] && text[k] == ' '; k++);
	return (k);	/* This is the position in text that we should start reporting from */
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MINMAX_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct MINMAX_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.col = UINT_MAX;	/* Meaning not set */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MINMAX_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-Aa|f|s] [-C] [-D[<dx>[/<dy>]] [-E<L|l|H|h>[<col>]] [-Fi|d|t] [-I[b|e|f|p|s]<dx>[/<dy>[/<dz>..][+e|r|R<incs>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L] [-S[x][y]] [-T<dz>[+c<col>]] [%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Select reports for (a)ll [Default], per (f)ile, or per (s)egment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Format the min and max into separate columns; -o may be used to limit output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Modifies results obtained by -I by shifting the region to better align with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the data center.  Optionally, append granularity for this shift [exact].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Return the record with extreme value in specified column <col> [last column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify l or h for min or max value, respectively.  Upper case L or H\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   means we operate instead on the absolute values of the data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Return various counts of tables, segments, headers, and records, depending on mode:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i: One record with the number of tables, segments, data records, headers, and overall records.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d: Dataset: One record per segment with tbl_no, seg_no, nrows, start_rec, stop_rec.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t: Tables:  Same as D but the counts resets per table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy (assumes at lwesn[XHI] 2 columns).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -Ie to just report the min/max extent in the -Rw/e/s/n string (no multiples).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -C is set then no -R string is issued.  Instead, the number of increments\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   given determines how many columns are rounded off to the nearest multiple.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only one increment is given we also use it for the second column (for backwards compatibility).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To override this behavior, use -Ip<dx>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If input data are regularly distributed we use observed phase shifts in determining -R [no phase shift]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     and allow -r to change from gridline-registration to pixel-registration.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ib to report the bounding box polygon for the data files (or segments; see -A).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -If<dx>[/<dy>] to report an extended region optimized for fastest results in FFTs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Is<dx>[/<dy>] to report an extended region optimized for fastest results in surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to modify the region further: Append <inc>, <xinc>/<yinc>, or <winc>/<einc>/<sinc>/<ninc>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to round region to these multiples; use +R to extend region by those increments instead,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or use +e which is like +r but makes sure the region extends at least by %g x <inc>.\n", GMT_REGION_INCFACTOR);
	GMT_Message (API, GMT_TIME_NONE, "\t-L Determine limiting region. With -I it rounds inward so bounds are within data range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -A to find the limiting common bounds of all segments or tables.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Add extra space for error bars. Useful together with -I.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sx leaves space for horizontal error bar using value in third (2) column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sy leaves space for vertical error bar using value in third (2) column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -S or -Sxy leaves space for both error bars using values in third&fourth (2&3) columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Return textstring -Tzmin/zmax/dz to nearest multiple of the given dz.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Calculations are based on the first (0) column; append +c<col> to use another column.\n");
	GMT_Option (API, "V,a");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Reports the names and data types of the aspatial fields.\n");
	GMT_Option (API, "bi2,d,e,f,g,h,i,o,qi,r,s,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MINMAX_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtinfo and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j;
	unsigned int n_errors = 0, k;
	bool special = false;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				Ctrl->n_files++;
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
					Ctrl->D.ncol = gmt_getincn (GMT, opt->arg, Ctrl->D.inc, GMT_MAX_COLUMNS);
					Ctrl->D.mode = 1;
				}
				break;
			case 'E':	/* Extrema reporting */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->E.abs = true;
						/* fall through on purpose to 'l' */
					case 'l':
						Ctrl->E.mode = -1;
						break;
					case 'H':
						Ctrl->E.abs = true;
						/* fall through on purpose to 'h' */
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
			case 'F':	/* Record/segment reporting only */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'i': Ctrl->F.mode = GMT_INFO_TOTAL; break;
					case 'd': Ctrl->F.mode = GMT_INFO_DATAINFO; break;
					case 't': Ctrl->F.mode = GMT_INFO_TABLEINFO; break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F. Flags are i|d|t.\n");
						break;
				}
				break;
			case 'I':	/* Granularity */
				Ctrl->I.active = true;
				n_errors += gmt_parse_region_extender (GMT, 'I', opt->arg, &(Ctrl->I.extend), Ctrl->I.delta);	/* Possibly extend the final region before reporting */
				j = 1;
				switch (opt->arg[0]) {
					case 'p': special = true; break;
					case 'f': Ctrl->I.mode = BEST_FOR_FFT; break;
					case 's': Ctrl->I.mode = BEST_FOR_SURF; break;
					case 'b': Ctrl->I.mode = BOUNDBOX; break;
					case '+':		/* -I+e|r|R */
						if (opt->arg[1] && strchr ("erR", opt->arg[1]))
							Ctrl->I.mode = ACTUAL_BOUNDS;
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I%s not recognized\n", opt->arg);					
							n_errors++;						
						}
						break;
					case 'e': case '-': Ctrl->I.mode = ACTUAL_BOUNDS;
						if (opt->arg[1]) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Ie (or obsolete -I-). No argument allowed.\n");					
							n_errors++;
						}
						break;	/* -I- is backwards compatible */
					default: j = 0;	break;
				}
				if (opt->arg[j] == '\0' && !(Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX)) {
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I. No increment given.\n");					
				}
				else
					Ctrl->I.ncol = (Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX) ? 2 : gmt_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS);
				break;
			case 'L':	/* Detect limiting range */
				Ctrl->L.active = true;
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
			case 'T':	/* makecpt inc string */
				Ctrl->T.active = true;
				if ((c = strchr (opt->arg, '/')) && gmt_M_compat_check (GMT, 5)) {	/* Let it slide for now */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -T<inc>[/<col>] syntax is deprecated; use -T<inc>[+c<col>] instead.\n");
					j = sscanf (opt->arg, "%lf/%d", &Ctrl->T.inc, &Ctrl->T.col);
					if (j == 1) Ctrl->T.col = 0;
				}
				else if (c || opt->arg[0] == '\0') {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -T: Syntax is -T<inc>[+c<col>].\n");
					n_errors++;
				}
				else {	/* Modern syntax and parsing */
					if ((c = strstr (opt->arg, "+c"))) {
						Ctrl->T.col = atoi (&c[2]);
						c[0] = '\0';	/* Temporarily hide the modifier */
					}
					Ctrl->T.inc = atof (opt->arg);
					if (c) c[0] = '+';	/* Restore the modifier */
				}
				break;

			case 'b':	/* -b[i]c will land here */
				if (gmt_M_compat_check (GMT, 4)) break;
				/* Otherwise we fall through on purpose to get an error */

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

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
	if (Ctrl->I.active && !(Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX)) {	/* SHould have increments */
		for (k = 0; k < Ctrl->I.ncol; k++) if (Ctrl->I.inc[k] <= 0.0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I. Must specify positive increment for column %d.\n", k);
			n_errors++;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && !Ctrl->I.active, "Syntax error: -D requires -I\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.mode != BOUNDBOX && !Ctrl->C.active && Ctrl->I.ncol < 2,
	                                   "Syntax error: -Ip requires -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->T.active,
	                                   "Syntax error: Only one of -I and -T can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0 ,
	                                   "Syntax error -T option: Must specify a positive increment\n");
	n_errors += gmt_check_binary_io (GMT, 1);
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, xyzmin); gmt_M_free (GMT, xyzmax); gmt_M_free (GMT, xyzminL); gmt_M_free (GMT, lonmin);  gmt_M_free (GMT, lonmax); gmt_M_free (GMT, xyzmaxL); gmt_M_free (GMT, Q); gmt_M_free (GMT, Z); gmt_M_free (GMT, Out); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtinfo (void *V_API, int mode, void *args) {
	bool got_stuff = false, first_data_record, give_r_string = false;
	bool brackets = false, work_on_abs_value, do_report, done, full_range = false;
	int i, j, error = 0, col_type[GMT_MAX_COLUMNS];
	unsigned int fixed_phase[2] = {1, 1}, min_cols, save_range, n_items = 0;
	uint64_t col, ncol = 0, n = 0, n_alloc = GMT_BIG_CHUNK;

	char file[PATH_MAX] = {""}, chosen[GMT_BUFSIZ] = {""}, record[GMT_BUFSIZ] = {""};
	char buffer[GMT_BUFSIZ] = {""}, delimiter[2] = {""}, *t_ptr = NULL;

	double *xyzmin = NULL, *xyzmax = NULL, *in = NULL, *dchosen = NULL, phase[2] = {0.0, 0.0}, this_phase, off;
	double wesn[4] = {0.0, 0.0, 0.0,  0.0}, low, high, value, e_min = DBL_MAX, e_max = -DBL_MAX;
	double *xyzminL = NULL, *xyzmaxL = NULL, *d_ptr = NULL;
	double out[5] = {0.0, 0.0, 0.0, 0.0, 0.0}, *lonmin = NULL, *lonmax = NULL;

	struct GMT_QUAD *Q = NULL;
	struct GMT_RANGE **Z = NULL;
	struct MINMAX_CTRL *Ctrl = NULL;
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 1, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtinfo main code ----------------------------*/

	if (Ctrl->n_files > 1 && GMT->common.a.active) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Cannot give multiple files when -a is selected!\n");
		Return (GMT_DIM_TOO_LARGE);
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
	
	if (Ctrl->F.active) {	/* Special case of reporting on record numbers */
		/* This is best done by reading the whole thing in */
		struct GMT_DATASET *D = NULL;
		struct GMT_DATATABLE *T = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		uint64_t tbl, seg, start_rec = 0;
		
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
			Return (API->error);	/* Establishes data files or stdin */
		}
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 5, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);
		for (col = 0; col < 5; col++) GMT->current.io.col_type[GMT_OUT][col] = GMT_IS_FLOAT;
		
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

		switch (Ctrl->F.mode) {
			case GMT_INFO_TOTAL:	/* Report total number of tables */
				out[0] = (double)D->n_tables;
				out[1] = (double)D->n_segments;
				out[2] = (double)D->n_records;
				for (tbl = 0; tbl < D->n_tables; tbl++)
					out[3] += (double)(D->table[tbl]->n_headers);
				if (D->n_segments == 1) out[4] -= 1.0;	/* Still unsure about this.  How do we know file actually had a segment header? */
				out[4] = out[3] + out[1] + out[2];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				break;
			case GMT_INFO_TABLEINFO:	/* Virtual data set or individual tables */
			case GMT_INFO_DATAINFO:
				for (tbl = 0; tbl < D->n_tables; tbl++) {
					T = D->table[tbl];
					if (Ctrl->F.mode == GMT_INFO_TABLEINFO) {
						out[0] = (double)tbl;
						start_rec = T->n_headers;	/* Start fresh for each table */
					}
					else
						start_rec += T->n_headers;
					for (seg = 0; seg < T->n_segments; seg++) {
						S = T->segment[seg];
						start_rec++;	/* The segment header */
						out[1] = (double)seg;
						out[2] = (double)S->n_rows;
						out[3] = (double)start_rec;
						out[4] = (double)(start_rec + S->n_rows - 1);
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
						start_rec += S->n_rows;
					}
				}
				break;
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {	/* Remove the dataset */
			Return (API->error);
		}
		Return (GMT_NOERROR);	/* We are done */
	}
	
	give_r_string = (Ctrl->I.active && !(Ctrl->I.mode == BOUNDBOX || Ctrl->C.active));
	delimiter[0] = (Ctrl->C.active || Ctrl->I.mode == BOUNDBOX) ? '\t' : '/';
	delimiter[1] = '\0';
	off = (GMT->common.R.active[GSET]) ? 0.5 : 0.0;

	brackets = !Ctrl->C.active;
	work_on_abs_value = (Ctrl->E.active && Ctrl->E.abs);
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Must check that output format won't mess things up by printing wesn[XLO] > wesn[XHI] */
		if (!strcmp (GMT->current.setting.format_geo_out, "D")) {
			strcpy (GMT->current.setting.format_geo_out, "+D");
			gmt_M_err_fail (GMT, gmtlib_geo_C_format (GMT), "");
			GMT_Report (API, GMT_MSG_VERBOSE, "FORMAT_GEO_OUT reset from D to %s to ensure wesn[XHI] > wesn[XLO]\n",
			            GMT->current.setting.format_geo_out);
		}
		else if (!strcmp (GMT->current.setting.format_geo_out, "ddd:mm:ss")) {
			strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ssF");
			gmt_M_err_fail (GMT, gmtlib_geo_C_format (GMT), "");
			GMT_Report (API, GMT_MSG_VERBOSE, "FORMAT_GEO_OUT reset from ddd:mm:ss to %s to ensure wesn[XHI] > wesn[XLO]\n",
			            GMT->current.setting.format_geo_out);
		}
	}

	if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) Return (error);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
		Return (API->error);
	}

	save_range = GMT->current.io.geo.range;
	GMT->current.io.geo.range = GMT_IGNORE_RANGE;	/* Ensure no adjustment will happen since we control it here */
	d_ptr = (Ctrl->C.active || Ctrl->E.active) ? GMT->current.io.curr_rec : ((Ctrl->I.mode == BOUNDBOX) ? out : NULL);
	t_ptr = (d_ptr && !Ctrl->E.active) ? NULL : record;
	Out = gmt_new_record (GMT, d_ptr, t_ptr);
	first_data_record = true;
	done = false;
	while (!done) {	/* Keep returning records until we reach EOF of last file */
		In = GMT_Get_Record (API, GMT_READ_DATA | GMT_READ_FILEBREAK, NULL);

		if (gmt_M_rec_is_error (GMT)) Return (GMT_RUNTIME_ERROR);
		if (gmt_M_rec_is_table_header (GMT)) continue;	/* Skip table headers */
		if (gmt_M_rec_is_segment_header (GMT) || gmt_M_rec_is_file_break (GMT) || gmt_M_rec_is_eof (GMT)) {	/* Need per segment scrutiny of longitudes */
			bool alloc_more = (n_items == n_alloc);
			if (alloc_more) n_alloc <<= 1;	/* Double the memory */
			for (col = 0; col < ncol; col++) if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {	/* Must finalize longitudes first */
				j = gmt_quad_finalize (GMT, &Q[col]);
				if (alloc_more)
					Z[col] = gmt_M_memory (GMT, Z[col], n_alloc, struct GMT_RANGE);
				Z[col][n_items].west = Q[col].min[j];	Z[col][n_items].east = Q[col].max[j];
				n_items++;
			}
			gmt_quad_reset (GMT, Q, ncol);
			if (gmt_M_rec_is_segment_header (GMT) && Ctrl->A.mode != REPORT_PER_SEGMENT) continue;	/* Since we are not reporting per segment they are just headers as far as we are concerned */
		}
		
		if (gmt_M_rec_is_segment_header (GMT) || (Ctrl->A.mode == REPORT_PER_TABLE && gmt_M_rec_is_file_break (GMT)) || gmt_M_rec_is_eof (GMT)) {	/* Time to report */
			if (Ctrl->A.active) {
				if (Ctrl->A.mode == REPORT_PER_TABLE && gmt_M_rec_is_file_break (GMT)) {
					e_min = DBL_MAX; e_max = -DBL_MAX;
				}
				else if (Ctrl->A.mode == REPORT_PER_SEGMENT && gmt_M_rec_is_segment_header (GMT)) {
					e_min = DBL_MAX; e_max = -DBL_MAX;
				}
			}
			if (gmt_M_rec_is_segment_header (GMT) && GMT->current.io.seg_no == 0) continue;	/* Very first segment header means there is no prior segment to report on yet */
			if (gmt_M_rec_is_eof (GMT)) {	/* We are done after this since we hit EOF */
				done = true;
				GMT->current.io.seg_no++;	/* Must manually increment since we are not reading any further */
			}
			if (n == 0) continue;			/* This segment, table, or data set had no data records, skip */
			
			/* Here we must issue a report */
			
			do_report = true;
			if (Ctrl->L.active && !gmt_M_rec_is_eof (GMT)) do_report = false;	/* Only final report for -L */
 			for (col = 0; col < ncol; col++) if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {	/* Must update longitudes separately */
				gmt_find_range (GMT, Z[col], n_items, &xyzmin[col], &xyzmax[col]);
				n_items = 0;
			}
			if (Ctrl->I.active) {	/* Must report multiples of dx/dy etc */
				if (n > 1 && fixed_phase[GMT_X] && fixed_phase[GMT_Y]) {	/* Got xy[z] data that lined up on a grid, so use the common phase shift */
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input (x,y) data are regularly distributed; fixed phase shifts are %g/%g.\n",
					            phase[GMT_X], phase[GMT_Y]);
				}
				else {	/* Data not on grid, just return bounding box rounded off to nearest inc */
					buffer[0] = '.';	buffer[1] = 0;
					if (GMT->common.R.active[GSET]) strcpy (buffer, " (-r is ignored).");
					GMT_Report (API, GMT_MSG_LONG_VERBOSE,
					            "Input (x,y) data are irregularly distributed; phase shifts set to 0/0%s\n", buffer);
					phase[GMT_X] = phase[GMT_Y] = off = 0.0;
				}
				if (Ctrl->I.mode == ACTUAL_BOUNDS) {
					wesn[XLO]  = xyzmin[GMT_X];	wesn[XHI]  = xyzmax[GMT_X];
					wesn[YLO] = xyzmin[GMT_Y];	wesn[YHI] = xyzmax[GMT_Y];
				}
				else if (Ctrl->I.mode == BOUNDBOX) {	/* Write out bounding box */
					sprintf (buffer, "Bounding box for table data");
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, buffer);
					out[GMT_X] = xyzmin[GMT_X];	out[GMT_Y] = xyzmin[GMT_Y];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[GMT_X] = xyzmax[GMT_X];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[GMT_Y] = xyzmax[GMT_Y];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[GMT_X] = xyzmin[GMT_X];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					out[GMT_Y] = xyzmin[GMT_Y];
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					do_report = false;
				}
				else if (Ctrl->L.active) { /* Round down to nearest inc for this segment or table */
					wesn[XLO]  = (ceil ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					wesn[XHI]  = (floor  ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					wesn[YLO] = (ceil ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
					wesn[YHI] = (floor  ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
				}
				else { /* Round up to nearest inc */
					wesn[XLO]  = (floor ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					wesn[XHI]  = (ceil  ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X];
					wesn[YLO] = (floor ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
					wesn[YHI] = (ceil  ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y];
					if (Ctrl->D.active) {	/* Center the selected region * better */
						double off = 0.5 * (xyzmin[GMT_X] + xyzmax[GMT_X] - wesn[XLO] - wesn[XHI]);
						if (Ctrl->D.inc[GMT_X] > 0.0) off = rint (off / Ctrl->D.inc[GMT_X]) * Ctrl->D.inc[GMT_X];
						wesn[XLO] += off;	wesn[XHI] += off;
						off = 0.5 * (xyzmin[GMT_Y] + xyzmax[GMT_Y] - wesn[YLO] - wesn[YHI]);
						if (Ctrl->D.inc[GMT_Y] > 0.0) off = rint (off / Ctrl->D.inc[GMT_Y]) * Ctrl->D.inc[GMT_Y];
						wesn[YLO] += off;	wesn[YHI] += off;
					}
				}
				gmt_extend_region (GMT, wesn, Ctrl->I.extend, Ctrl->I.delta);	/* Possibly extend the region */
				if (gmt_M_is_geographic (GMT, GMT_IN)) {
					if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
						if (wesn[YLO] < -90.0) {
							wesn[YLO] = -90.0;
							GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused wesn[YLO] to become < -90. Reset to -90.\n");
						}
						if (wesn[YHI] > 90.0) {
							wesn[YHI] = 90.0;
							GMT_Report (API, GMT_MSG_VERBOSE, "Using -I caused wesn[YHI] to become > +90. Reset to +90.\n");
						}
						if (fabs (wesn[XHI] - wesn[XLO]) > 360.0) {
							GMT_Report (API, GMT_MSG_VERBOSE,
							            "Using -I caused longitude range to exceed 360. Reset to a range of 360.\n");
							wesn[XLO] = (wesn[XLO] < 0.0) ? -180.0 : 0.0;
							wesn[XHI] = (wesn[XLO] < 0.0) ? +180.0 : 360.0;
							full_range = true;
						}
					}
				}
				if (Ctrl->L.active) {
					for (col = 0; col < ncol; col++) {	/* Report innermost min/max for each column */
						if (xyzmin[col] > xyzminL[col]) xyzminL[col] = xyzmin[col];
						if (xyzmax[col] < xyzmaxL[col]) xyzmaxL[col] = xyzmax[col];
					}
				}
				if (Ctrl->I.mode == BEST_FOR_FFT || Ctrl->I.mode == BEST_FOR_SURF) {	/* Wish to extend the region to optimize the resulting n_columns/n_rows */
					unsigned int sub, add, in_dim[2], out_dim[2];
					double ww, ee, ss, nn;
					in_dim[GMT_X] = gmt_M_get_n (GMT, wesn[XLO], wesn[XHI], Ctrl->I.inc[GMT_X], GMT->common.R.active[GSET]);
					in_dim[GMT_Y] = gmt_M_get_n (GMT, wesn[YLO], wesn[YHI], Ctrl->I.inc[GMT_Y], GMT->common.R.active[GSET]);
					ww = wesn[XLO];	ee = wesn[XHI]; ss = wesn[YLO];	nn = wesn[YHI];
					gmt_best_dim_choice (GMT, Ctrl->I.mode, in_dim, out_dim);
					sub = (out_dim[GMT_X] - in_dim[GMT_X]) / 2;	add = out_dim[GMT_X] - in_dim[GMT_X] - sub;
					wesn[XLO]  -= sub * Ctrl->I.inc[GMT_X];		wesn[XHI]  += add * Ctrl->I.inc[GMT_X];
					sub = (out_dim[GMT_Y] - in_dim[GMT_Y]) / 2;	add = out_dim[GMT_Y] - in_dim[GMT_Y] - sub;
					wesn[YLO] -= sub * Ctrl->I.inc[GMT_Y];		wesn[YHI] += add * Ctrl->I.inc[GMT_Y];
					GMT_Report (API, GMT_MSG_LONG_VERBOSE,
					            "Initial -R: %g/%g/%g/%g [n_columns = %u n_rows = %u] --> Suggested -R:  %g/%g/%g/%g [n_columns = %u n_rows = %u].\n",
						ww, ee, ss, nn, in_dim[GMT_X], in_dim[GMT_Y], wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], out_dim[GMT_X], out_dim[GMT_Y]);
				}
			}
			if (give_r_string) {	/* Return -R string */
				if (full_range)
					sprintf (record, "-R%g/%g/", wesn[XLO], wesn[XHI]);
				else {
					sprintf (record, "-R");
					i = strip_blanks_and_output (GMT, buffer, wesn[XLO], GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
					i = strip_blanks_and_output (GMT, buffer, wesn[XHI], GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
				}
				i = strip_blanks_and_output (GMT, buffer, wesn[YLO], GMT_Y);	strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, wesn[YHI], GMT_Y);	strcat (record, &buffer[i]);
			}
			else if (Ctrl->T.active) {	/* Return -T string */
				wesn[XLO]  = floor (xyzmin[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;
				wesn[XHI]  = ceil  (xyzmax[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;
				sprintf (record, "-T");
				i = strip_blanks_and_output (GMT, buffer, wesn[XLO], Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, wesn[XHI], Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = strip_blanks_and_output (GMT, buffer, Ctrl->T.inc, Ctrl->T.col);	strcat (record, &buffer[i]);
			}
			else if (Ctrl->E.active) {	/* Return extreme record */
				gmt_M_memcpy (Out->data, dchosen, ncol, double);
				if (Out->text) strncpy (record, chosen, GMT_BUFSIZ);
			}
			else {				/* Return min/max for each column */
				if (!Ctrl->C.active) {	/* Want info about each item */
					if (GMT->common.a.active) {	/* Write text record with name[type] of the aspatial fields */
						gmt_list_aspatials (GMT, record);
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
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
							low  = wesn[XLO];
							high = wesn[XHI];
						}
						else if (Ctrl->I.mode && col == GMT_Y) {
							low  = wesn[YLO];
							high = wesn[YHI];
						}
						else if (Ctrl->L.active) {
							low  = (Ctrl->I.active) ? ceil  (xyzmin[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmin[col];
							high = (Ctrl->I.active) ? floor (xyzmax[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmax[col];
							if (low  > xyzminL[col]) xyzminL[col] = low;
							if (high < xyzmax[col])  xyzmaxL[col] = high;
							if (do_report) {	/* Last time so finalize */
								low  = xyzminL[col];
								high = xyzmaxL[col];
							}
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
						gmt_ascii_format_col (GMT, buffer, low, GMT_OUT, col);
						strcat (record, buffer);
						strcat (record, delimiter);
						gmt_ascii_format_col (GMT, buffer, high, GMT_OUT, col);
						strcat (record, buffer);
						if (brackets) strcat (record, ">");
						if (col < (ncol - 1)) strcat (record, "\t");
					}
				}
			}
			if (do_report)
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write data record to output destination */
			got_stuff = true;		/* We have at lwesn[XHI] reported something */
			for (col = 0; col < ncol; col++) {	/* Reset counters for next block */
				xyzmin[col] = DBL_MAX;
				xyzmax[col] = -DBL_MAX;
				lonmin[col] = lonmax[col] = 0.0;
			}
			n = 0;
			file[0] = '\0';
			fixed_phase[GMT_X] = fixed_phase[GMT_Y] = 1;	/* Get ready for next batch */
			if (done || do_report || Ctrl->I.mode == BOUNDBOX || gmt_M_rec_is_file_break (GMT)) continue;	/* We are done OR have no data record to process yet */
		}
		if (gmt_M_rec_is_file_break (GMT)) continue;

		/* We get here once we have read a data record */
		if ((in = In->data) == NULL) {	/* Only need to process numerical part here */
			GMT_Report (API, GMT_MSG_NORMAL, "No data columns to work with - exiting\n");
			Return (GMT_DIM_TOO_SMALL);
		}
		if (first_data_record) {	/* First time we read data, we must allocate arrays based on the number of columns */

			if (Ctrl->C.active) {	/* Must set output column types since each input col will produce two output cols. */
				gmt_M_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], GMT_MAX_COLUMNS, int);	/* Save previous output col types */
				for (col = 0; col < GMT_MAX_COLUMNS/2; col++)
					GMT->current.io.col_type[GMT_OUT][2*col] = GMT->current.io.col_type[GMT_OUT][2*col+1] = GMT->current.io.col_type[GMT_IN][col];
			}

			ncol = gmt_get_cols (GMT, GMT_IN);
			if (Ctrl->E.active) {
				if (Ctrl->E.col == UINT_MAX) Ctrl->E.col = ncol - 1;	/* Default is last column */
				dchosen = gmt_M_memory (GMT, NULL, ncol, double);
			}
			min_cols = 2;	if (Ctrl->S.xbar) min_cols++;	if (Ctrl->S.ybar) min_cols++;
			if (Ctrl->S.active && min_cols > ncol) {
				GMT_Report (API, GMT_MSG_NORMAL, "Not enough columns to support the -S option\n");
				Return (GMT_DIM_TOO_SMALL);
			}
			if (Ctrl->E.active && Ctrl->E.col >= ncol) {
  				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Chosen column exceeds column range (0-%d)\n", ncol-1);
				Return (GMT_DIM_TOO_LARGE);
			}
			if (Ctrl->T.active && Ctrl->T.col >= ncol) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Chosen column exceeds column range (0-%d)\n", ncol-1);
				Return (GMT_DIM_TOO_LARGE);
			}
			if (Ctrl->T.active) ncol = Ctrl->T.col + 1;
			if (give_r_string) ncol = 2;	/* Since we only will concern ourselves with lon/lat or x/y */
			if (ncol == 0) {
				GMT_Report (API, GMT_MSG_NORMAL, "Something went wrong, ncol is still = 0. Maybe a call from an external program? Have to abort here.\n");
				Return (GMT_RUNTIME_ERROR);
			}
			
			/* Now we know number of columns, so allocate memory */

			Q = gmt_quad_init (GMT, ncol);
			Z = gmt_M_memory (GMT, NULL, ncol, struct GMT_RANGE *);
			xyzmin = gmt_M_memory (GMT, NULL, ncol, double);
			xyzmax = gmt_M_memory (GMT, NULL, ncol, double);
			xyzminL = gmt_M_memory (GMT, NULL, ncol, double);
			xyzmaxL = gmt_M_memory (GMT, NULL, ncol, double);
			lonmin = gmt_M_memory (GMT, NULL, ncol, double);
			lonmax = gmt_M_memory (GMT, NULL, ncol, double);

			for (col = 0; col < ncol; col++) {	/* Initialize */
				xyzmin[col] = xyzmaxL[col] = DBL_MAX;
				xyzmax[col] = xyzminL[col] = -DBL_MAX;
				if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON)
					Z[col] = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_RANGE);
			}
			n = 0;
			if (Ctrl->I.active && ncol < 2 && !Ctrl->C.active) Ctrl->I.active = false;
			first_data_record = false;
			if (Ctrl->C.active) {
				if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)(2*ncol), GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
					Return (error);
			}
			else if (Ctrl->E.active) {
				if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)ncol, (In->text) ? GMT_COL_FIX : GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
					Return (error);
			}
			else if (Ctrl->I.mode == BOUNDBOX) {
				if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
					Return (error);
			}
			else {
				if ((error = GMT_Set_Columns (API, GMT_OUT, 0, GMT_COL_FIX)) != GMT_NOERROR)
					Return (error);
			}
		}

		/* Process all columns and update the corresponding minmax arrays */

		if (Ctrl->E.active) {		/* See if this record should become the chosen one  */
			if (!gmt_M_is_dnan (in[Ctrl->E.col])) {		/* But skip NaNs */
				value = (work_on_abs_value) ? fabs (in[Ctrl->E.col]) : in[Ctrl->E.col];
				if (Ctrl->E.mode == -1 && value < e_min) {	/* Lower than previous low */
					e_min = value;
					gmt_M_memcpy (dchosen, in, ncol, double);
					if (In->text) strncpy (chosen, In->text, GMT_BUFSIZ-1);
				}
				else if (Ctrl->E.mode == +1 && value > e_max) {	/* Higher than previous high */
					e_max = value;
					gmt_M_memcpy (dchosen, in, ncol, double);
					if (In->text) strncpy (chosen, In->text, GMT_BUFSIZ-1);
				}
			}
		}
		else {	/* Update min/max values for each column */
			for (col = 0; col < ncol; col++) {
				if (gmt_M_is_dnan (in[col])) continue;	/* We always skip NaNs */
				if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {	/* Longitude requires more work */
					/* We must keep separate min/max for both Dateline and Greenwich conventions */
					gmt_quad_add (GMT, &Q[col], in[col]);
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
		if (file[0] == 0) strncpy (file, GMT->current.io.filename[GMT_IN], PATH_MAX-1);	/* Grab name of current file while we can */
		
	}
	if (GMT->common.a.active) {
		gmt_list_aspatials (GMT, buffer);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, buffer);
	}
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (!got_stuff) GMT_Report (API, GMT_MSG_NORMAL, "No input data found!\n");

	GMT->current.io.geo.range = save_range;	/* Restore what we changed */
	if (Ctrl->C.active) {	/* Restore previous output col types */
		gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, GMT_MAX_COLUMNS, int);
	}
	if (Ctrl->E.active)
		gmt_M_free (GMT, dchosen);

	for (col = 0; col < ncol; col++) gmt_M_free (GMT, Z[col]);

	Return (GMT_NOERROR);
}
