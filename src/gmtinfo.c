/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#include "longopt/gmtinfo_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtinfo"
#define THIS_MODULE_MODERN_NAME	"gmtinfo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Get information about data tables"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioqrsw" GMT_OPT("HMm")

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

struct GMTINFO_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	unsigned int n_files;
	struct GMTINFO_A {	/* -A */
		bool active;
		unsigned int mode;	/* 0 reports range for all tables, 1 is per table, 2 is per segment */
	} A;
	struct GMTINFO_C {	/* -C */
		bool active;
		unsigned int extra;	/* Undocumented hidden feature only needed via gmt_init_module to get -R */
	} C;
	struct GMTINFO_D {	/* -D[dx[/dy[/<dz>..]]] */
		bool active;
		unsigned int ncol;
		unsigned int mode;	/* 0 means center, 1 means use dx granularity */
		double inc[GMT_MAX_COLUMNS];
	} D;
	struct GMTINFO_E {	/* -E<L|l|H|h>[<col>] */
		bool active;
		bool abs;
		int mode;	/* -1, 0, +1 */
		uint64_t col;
	} E;
	struct GMTINFO_F {	/* -F<i|d|t> */
		bool active;
		int mode;	/*  */
	} F;
	struct GMTINFO_I {	/* -I[b|e|f|p|s]dx[/dy[/<dz>..]][[+e|r|R<incs>]] */
		bool active;
		unsigned int extend;
		unsigned int ncol;
		unsigned int mode;	/* Nominally 0, unless set to BEST_FOR_SURF, BEST_FOR_FFT or ACTUAL_BOUNDS */
		double inc[GMT_MAX_COLUMNS];
		double delta[4];
	} I;
	struct GMTINFO_L {	/* -L */
		bool active;
	} L;
	struct GMTINFO_S {	/* -S[x|y] [Deprecated in 6.5] */
		bool active;
		bool xbar, ybar;
	} S;
	struct GMTINFO_T {	/* -T<dz>[/<col>] */
		bool active;
		double inc;
		unsigned int col;
	} T;
};

GMT_LOCAL int gmtinfo_strip_blanks_and_output (struct GMT_CTRL *GMT, char *text, double x, int col) {
	/* Alternative to GMT_ascii_output_col that strips off leading blanks first */

	int k;

	gmt_ascii_format_col (GMT, text, x, GMT_OUT, col);
	for (k = 0; text[k] && text[k] == ' '; k++);
	return (k);	/* This is the position in text that we should start reporting from */
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTINFO_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTINFO_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.col = UINT_MAX;	/* Meaning not set */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTINFO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-Aa|t|s] [-C] [-D[<dx>[/<dy>]]] [-EL|l|H|h[<col>]] "
		"[-Fi|d|t] [-I[b|e|f|p|s]<dx>[/<dy>[/<dz>..]][+e|r|R<incs>]] [-L] [-T<dz>[%s][+c<col>]] "
		"[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]",
		name, GMT_TIME_FIX_UNITS_DISPLAY, GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_o_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A[a|t|s]");
	GMT_Usage (API, -2, "Select reports for (a)ll [Default], per (t)able, or per (s)egment.");
	GMT_Usage (API, 1, "\n-C Format the min and max into separate columns; -o may be used to limit output.");
	GMT_Usage (API, 1, "\n-D Modifies results obtained by -I by shifting the region to better align with "
		"the data center.  Optionally, append granularity for this shift [exact].");
	GMT_Usage (API, 1, "\n-EL|l|H|h[<col>]");
	GMT_Usage (API, -2, "Return the record with extreme value in specified column <col> [last column]. "
		"Specify l or h for min or max value, respectively. Upper case L or H "
		"means we operate instead on the absolute values of the data.");
	GMT_Usage (API, 1, "\n-Fi|d|t");
	GMT_Usage (API, -2, "Return various counts of tables, segments, headers, and records, depending on mode:");
	GMT_Usage (API, 3, "i: One record with the number of tables, segments, data records, headers, and overall records.");
	GMT_Usage (API, 3, "d: Dataset: One record per segment with tbl_no, seg_no, nrows, start_rec, stop_rec.");
	GMT_Usage (API, 3, "t: Tables:  Same as D but the counts resets per table.");
	GMT_Usage (API, 1, "\n-I[b|e|f|p|s]<dx>[/<dy>[/<dz>..]][+e|r|R<incs>]");
	GMT_Usage (API, -2, "Return textstring -Rw/e/s/n to nearest multiple of <dx>/<dy> (assumes at least two columns). "
		"Give -Ie to just report the min/max extent in the -Rw/e/s/n string (no multiples). Give -I<dx>/0 or -I0/<dy> for mixed exact and rounded result. "
		"If -C is set then no -R string is issued.  Instead, the number of increments "
		"given determines how many columns are rounded off to the nearest multiple. "
		"If only one increment is given we also use it for the second column (for backwards compatibility). "
		"To override this behavior, use -Ip<dx>. "
		"If input data are regularly distributed we use observed phase shifts in determining -R [no phase shift] "
		"and allow -r to change from gridline-registration to pixel-registration. "
		"Use -Ib to report the bounding box polygon for the data files (or segments; see -A). "
		"Use -If<dx>[/<dy>] to report an extended region optimized for fastest results in FFTs. "
		"Use -Is<dx>[/<dy>] to report an extended region optimized for fastest results in surface. "
		"Append +r to modify the region further: Append <inc>, <xinc>/<yinc>, or <winc>/<einc>/<sinc>/<ninc> "
		"to round region to these multiples; use +R to extend region by those increments instead, "
		"or use +e which is like +r but makes sure the region extends at least by %g x <inc>.\n", GMT_REGION_INCFACTOR);
	GMT_Usage (API, 1, "\n-L Determine limiting region. With -I it rounds inward so bounds are within data range. "
		"Use -A to find the limiting common bounds of all segments or tables.");
	GMT_Usage (API, 1, "\n-T<dz>[%s][+c<col>]", GMT_TIME_FIX_UNITS_DISPLAY);
	GMT_Usage (API, -2, "Return textstring -Tzmin/zmax/dz to nearest multiple of the given <dz>.");
	GMT_Usage (API, -2, "Note: Calculations are based on the first (0) column; append +c<col> to use another column. "
		"If the column is absolute time you may append a valid fixed time unit to <dz> (Default is set by TIME_UNIT [s]).");
	GMT_Option (API, "V,a");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Reports the names and data types of the aspatial fields.\n");
	GMT_Option (API, "bi2,d,e,f,g,h,i,o,qi,r,s,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTINFO_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtinfo and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j, ival;
	unsigned int n_errors = 0, k;
	bool special = false;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				Ctrl->n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Reporting unit */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				switch (opt->arg[0]) {
					case 'a':
						Ctrl->A.mode = REPORT_PER_DATASET;
						break;
					case 't': case 'f':	/* Keep f (file) for backwards compatibility, but "table" is used in all similar situations */
						Ctrl->A.mode = REPORT_PER_TABLE;
						break;
					case 's':
						Ctrl->A.mode = REPORT_PER_SEGMENT;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Flags are a|t|s.\n");
						break;
				}
				break;
			case 'C':	/* Column output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case '0':	/* Hidden option to get 2 more columns back with column types */
				Ctrl->C.extra = 2;	/* Need to report back x and y data types */
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'D':	/* Region adjustment Granularity */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if (opt->arg[0]) {
					if ((Ctrl->D.ncol = gmt_getincn (GMT, opt->arg, Ctrl->D.inc, GMT_MAX_COLUMNS)) < 0) n_errors++;
					Ctrl->D.mode = 1;
				}
				break;
			case 'E':	/* Extrema reporting */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->E.abs = true;
						/* Intentionally fall through - to 'l' */
					case 'l':
						Ctrl->E.mode = -1;
						break;
					case 'H':
						Ctrl->E.abs = true;
						/* Intentionally fall through - to 'h' */
					case 'h':
						Ctrl->E.mode = +1;
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -E: Flags are L|l|H|h.\n");
						break;
				}
				if (opt->arg[1]) Ctrl->E.col = atoi (&opt->arg[1]);
				break;
			case 'F':	/* Record/segment reporting only */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				switch (opt->arg[0]) {
					case '\0': case 'i': Ctrl->F.mode = GMT_INFO_TOTAL; break;
					case 'd': Ctrl->F.mode = GMT_INFO_DATAINFO; break;
					case 't': Ctrl->F.mode = GMT_INFO_TABLEINFO; break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Flags are i|d|t.\n");
						break;
				}
				break;
			case 'I':	/* Granularity */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				if (strchr (opt->arg, '+')) n_errors += gmt_parse_region_extender (GMT, 'I', opt->arg, &(Ctrl->I.extend), Ctrl->I.delta);	/* Possibly extend the final region before reporting */
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
							GMT_Report (API, GMT_MSG_ERROR, "Option -I%s not recognized.\n", opt->arg);
							n_errors++;
						}
						break;
					case 'e': case '-': Ctrl->I.mode = ACTUAL_BOUNDS;
						if (opt->arg[1]) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Ie (or obsolete -I-): No argument allowed.\n");
							n_errors++;
						}
						break;	/* -I- is backwards compatible */
					default:	/* Numbers.  Check for stray letters */
						if (opt->arg[0] && isalpha (opt->arg[0])) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -I: Bad argument %s.\n", opt->arg);
							n_errors++;
						}
						j = 0;
						break;
				}
				if (opt->arg[j] == '\0' && !(Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX)) {
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "Option -I: No increment given.\n");
				}
				else if (!n_errors) {
					if (Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX)
						Ctrl->I.ncol = 2;
					else {
						if ((ival = gmt_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS)) < 0) n_errors++;
						else Ctrl->I.ncol = (unsigned int)ival;
					}
					Ctrl->I.ncol = (Ctrl->I.mode == ACTUAL_BOUNDS || Ctrl->I.mode == BOUNDBOX) ? 2 : gmt_getincn (GMT, &opt->arg[j], Ctrl->I.inc, GMT_MAX_COLUMNS);
				}
				break;
			case 'L':	/* Detect limiting range */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'S':	/* Error bar output [deprecated in 6.5] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				j = 0;
				while (opt->arg[j]) {
					if (opt->arg[j] == 'x') Ctrl->S.xbar = true;
					if (opt->arg[j] == 'y') Ctrl->S.ybar = true;
					j++;
				}
				if (j == 2) Ctrl->S.xbar = Ctrl->S.ybar = true;
				break;
			case 'T':	/* makecpt inc string */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if ((c = strchr (opt->arg, '/')) && gmt_M_compat_check (GMT, 5)) {	/* Let it slide for now */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -T<inc>[/<col>] syntax is deprecated; use -T<inc>[%s][+c<col>] instead.\n", GMT_TIME_FIX_UNITS_DISPLAY);
					j = sscanf (opt->arg, "%lf/%d", &Ctrl->T.inc, &Ctrl->T.col);
					if (j == 1) Ctrl->T.col = 0;
				}
				else if (c || opt->arg[0] == '\0') {
					GMT_Report (API, GMT_MSG_ERROR, "Option -T: Syntax is -T<inc>[%s][<[+c<col>].\n", GMT_TIME_FIX_UNITS_DISPLAY);
					n_errors++;
				}
				else {	/* Modern syntax and parsing */
					size_t L;
					if ((c = strstr (opt->arg, "+c"))) {	/* Specified alternate column */
						Ctrl->T.col = atoi (&c[2]);
						c[0] = '\0';	/* Temporarily hide the modifier */
					}
					Ctrl->T.inc = atof (opt->arg);
					if ((L = strlen (opt->arg))) {	/* Switch to another valid time unit */
						char unit = opt->arg[L-1];
						if (strchr (GMT_TIME_FIX_UNITS, unit)) {	/* Change to another valid time unit */
							API->GMT->current.setting.time_system.unit = unit;
							(void) gmt_init_time_system_structure (API->GMT, &API->GMT->current.setting.time_system);
						}
						else if (isalpha (unit)) {	/* Got junk */
							GMT_Report (API, GMT_MSG_ERROR, "Option -T: Bad time unit %c. Choose from %s.\n", unit, GMT_TIME_FIX_UNITS_DISPLAY);
							n_errors++;
						}	/* else it was just part of dz */
					}
					if (c) c[0] = '+';	/* Restore the modifier */
				}
				break;

			case 'b':	/* -b[i]c will land here */
				if (gmt_M_compat_check (GMT, 4)) break;
				/* Otherwise we fall through on purpose to get an error */
				/* Intentionally fall through */
			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->I.active && special && Ctrl->I.ncol > 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -Ip: Only a single increment is expected.\n");
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
		for (k = 0; k < Ctrl->I.ncol; k++) if (Ctrl->I.inc[k] < 0.0) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -I: Must specify positive increment for column %d.\n", k);
			n_errors++;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && !Ctrl->I.active, "Option-D requires -I\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.mode != BOUNDBOX && !Ctrl->C.active && Ctrl->I.ncol < 2 && special,
	                                   "Option -Ip requires -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->T.active,
	                                   "Only one of -I and -T can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0 ,
	                                   "Option -T: Must specify a positive increment.\n");
	n_errors += gmt_check_binary_io (GMT, 1);
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, xyzmin); gmt_M_free (GMT, xyzmax); gmt_M_free (GMT, xyzminL); gmt_M_free (GMT, lonmin);  gmt_M_free (GMT, lonmax); gmt_M_free (GMT, xyzmaxL); gmt_M_free (GMT, Q); gmt_M_free (GMT, Z); gmt_M_free (GMT, Out); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtinfo (void *V_API, int mode, void *args) {
	bool got_stuff = false, first_data_record, give_r_string = false, save_t, first_time = true;;
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
	struct GMTINFO_CTRL *Ctrl = NULL;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtinfo main code ----------------------------*/

	if (Ctrl->n_files > 1 && GMT->common.a.active) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Cannot give multiple files when -a is selected!\n");
		Return (GMT_DIM_TOO_LARGE);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	save_t = GMT->current.setting.io_lonlat_toggle[GMT_OUT];
	if (Ctrl->C.active) GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;	/* If it was true, we don't want that here since first two cols will both be x */

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
	save_range = GMT->current.io.geo.range;

	brackets = !Ctrl->C.active;
	work_on_abs_value = (Ctrl->E.active && Ctrl->E.abs);
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Must check that output format won't mess things up by printing wesn[XLO] > wesn[XHI] */
		if (!strcmp (GMT->current.setting.format_geo_out, "D")) {
			strcpy (GMT->current.setting.format_geo_out, "+D");
			if ((error = gmt_M_err_fail (GMT, gmtlib_geo_C_format (GMT), "")))
				Return (error);
			GMT_Report (API, GMT_MSG_INFORMATION, "FORMAT_GEO_OUT reset from D to %s to ensure wesn[XHI] > wesn[XLO]\n",
			            GMT->current.setting.format_geo_out);
		}
		else if (!strcmp (GMT->current.setting.format_geo_out, "ddd:mm:ss")) {
			strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ssF");
			if ((error = gmt_M_err_fail (GMT, gmtlib_geo_C_format (GMT), "")))
				Return (error);
			GMT_Report (API, GMT_MSG_INFORMATION, "FORMAT_GEO_OUT reset from ddd:mm:ss to %s to ensure wesn[XHI] > wesn[XLO]\n",
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
		if (first_time && gmt_M_is_geographic (GMT, GMT_IN)) {	/* Learned that an OGR file is geographic so output is the same */
			gmt_set_geographic (GMT, GMT_OUT);
			first_time = false;
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
 			for (col = 0; col < ncol; col++) {		/* Must update longitudes separately */
				if (GMT->current.io.col_type[GMT_IN][col] == GMT_IS_LON) {
					gmt_find_range (GMT, Z[col], n_items, &xyzmin[col], &xyzmax[col]);
					n_items = 0;
				}
			}
			if (Ctrl->I.active) {	/* Must report multiples of dx/dy etc */
				if (n > 1 && fixed_phase[GMT_X] && fixed_phase[GMT_Y]) {	/* Got xy[z] data that lined up on a grid, so use the common phase shift */
					GMT_Report (API, GMT_MSG_INFORMATION, "Input (x,y) data are regularly distributed; fixed phase shifts are %g/%g.\n",
					            phase[GMT_X], phase[GMT_Y]);
				}
				else {	/* Data not on grid, just return bounding box rounded off to nearest inc */
					buffer[0] = '.';	buffer[1] = 0;
					if (GMT->common.R.active[GSET]) strcpy (buffer, " (-r is ignored).");
					GMT_Report (API, GMT_MSG_INFORMATION,
					            "Input (x,y) data are irregularly distributed; phase shifts set to 0/0%s\n", buffer);
					phase[GMT_X] = phase[GMT_Y] = off = 0.0;
				}
				if (Ctrl->I.mode == ACTUAL_BOUNDS) {
					if (n == 1) GMT_Report (API, GMT_MSG_WARNING,
					            "Only one data record processed; bounds will be meaningless\n");
					wesn[XLO] = xyzmin[GMT_X];	wesn[XHI] = xyzmax[GMT_X];
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
					wesn[XLO] = (Ctrl->I.inc[GMT_X] > 0.0) ? (ceil  ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X] : xyzmin[GMT_X];
					wesn[XHI] = (Ctrl->I.inc[GMT_X] > 0.0) ? (floor ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X] : xyzmax[GMT_X];
					wesn[YLO] = (Ctrl->I.inc[GMT_Y] > 0.0) ? (ceil  ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y] : xyzmin[GMT_Y];
					wesn[YHI] = (Ctrl->I.inc[GMT_Y] > 0.0) ? (floor ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y] : xyzmax[GMT_Y];
				}
				else { /* Round up to nearest inc */
					wesn[XLO] = (Ctrl->I.inc[GMT_X] > 0.0) ? (floor ((xyzmin[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) - off) * Ctrl->I.inc[GMT_X] + phase[GMT_X] : xyzmin[GMT_X];
					wesn[XHI] = (Ctrl->I.inc[GMT_X] > 0.0) ? (ceil  ((xyzmax[GMT_X] - phase[GMT_X]) / Ctrl->I.inc[GMT_X]) + off) * Ctrl->I.inc[GMT_X] + phase[GMT_X] : xyzmax[GMT_X];
					wesn[YLO] = (Ctrl->I.inc[GMT_Y] > 0.0) ? (floor ((xyzmin[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) - off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y] : xyzmin[GMT_Y];
					wesn[YHI] = (Ctrl->I.inc[GMT_Y] > 0.0) ? (ceil  ((xyzmax[GMT_Y] - phase[GMT_Y]) / Ctrl->I.inc[GMT_Y]) + off) * Ctrl->I.inc[GMT_Y] + phase[GMT_Y] : xyzmax[GMT_Y];
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
				if (gmt_M_y_is_lat (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
					if (wesn[YLO] < -90.0) {
						wesn[YLO] = -90.0;
						GMT_Report (API, GMT_MSG_WARNING, "Using -I caused south to become < -90. Reset to -90.\n");
					}
					if (wesn[YHI] > 90.0) {
						wesn[YHI] = 90.0;
						GMT_Report (API, GMT_MSG_WARNING, "Using -I caused north to become > +90. Reset to +90.\n");
					}
				}
				if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Must make sure we don't get outside valid bounds */
					if (fabs (wesn[XHI] - wesn[XLO]) > 360.0) {
						GMT_Report (API, GMT_MSG_WARNING,
						            "Using -I caused longitude range to exceed 360. Reset to a range of 360.\n");
						wesn[XLO] = (wesn[XLO] < 0.0) ? -180.0 : 0.0;
						wesn[XHI] = (wesn[XLO] < 0.0) ? +180.0 : 360.0;
						full_range = true;
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
					GMT_Report (API, GMT_MSG_INFORMATION,
					            "Initial -R: %g/%g/%g/%g [n_columns = %u n_rows = %u] --> Suggested -R:  %g/%g/%g/%g [n_columns = %u n_rows = %u].\n",
						ww, ee, ss, nn, in_dim[GMT_X], in_dim[GMT_Y], wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], out_dim[GMT_X], out_dim[GMT_Y]);
				}
			}

			if (gmt_M_is_geographic (GMT, GMT_OUT)) {
				/* Special handling since lon1/lon2 must first have been ensured (above) to satisfy lon2 > lon1, but
			 	* we may wish for a specific range format, so listen to what save_range was before we set it to GMT_IGNORE_RANGE */
				if (wesn[XLO] > 0.0 && save_range == GMT_IS_M180_TO_P180_RANGE) {
					wesn[XLO] -= 360.0;	wesn[XHI] -= 360.0;
				}
				else if (save_range == GMT_IS_0_TO_P360_RANGE && wesn[XHI] < 0.0) {
					wesn[XLO] += 360.0;	wesn[XHI] += 360.0;
				}
				else if (save_range == GMT_IS_M360_TO_0_RANGE && wesn[XHI] > 0.0) {
					wesn[XLO] -= 360.0;	wesn[XHI] -= 360.0;
				}
			}
			if (give_r_string) {	/* Return -R string */
				if (full_range)
					sprintf (record, "-R%g/%g/", wesn[XLO], wesn[XHI]);
				else {
					sprintf (record, "-R");
					i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[XLO], GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
					i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[XHI], GMT_X);		strcat (record, &buffer[i]);	strcat (record, "/");
				}
				i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[YLO], GMT_Y);	strcat (record, &buffer[i]);	strcat (record, "/");
				i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[YHI], GMT_Y);	strcat (record, &buffer[i]);
			}
			else if (Ctrl->T.active) {	/* Return -T string */
				unsigned int save = gmt_get_column_type (GMT, GMT_OUT, Ctrl->T.col);	/* Remember what column type this is */
				wesn[XLO]  = floor (xyzmin[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;	/* Round min/max given the increment */
				wesn[XHI]  = ceil  (xyzmax[Ctrl->T.col] / Ctrl->T.inc) * Ctrl->T.inc;
				sprintf (record, "-T");
				i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[XLO], Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				i = gmtinfo_strip_blanks_and_output (GMT, buffer, wesn[XHI], Ctrl->T.col);		strcat (record, &buffer[i]);	strcat (record, "/");
				if (save == GMT_IS_ABSTIME) gmt_set_column_type (GMT, GMT_OUT, Ctrl->T.col, GMT_IS_FLOAT);	/* Must temporarily switch to float to typeset increment */
				i = gmtinfo_strip_blanks_and_output (GMT, buffer, Ctrl->T.inc, Ctrl->T.col);	strcat (record, &buffer[i]);
				if (save == GMT_IS_ABSTIME) {	/* Append unit used */
					gmt_set_column_type (GMT, GMT_OUT, Ctrl->T.col, save);	/* Reset column type */
					chrcat (record, GMT->current.setting.time_system.unit);
				}
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
							low  = (Ctrl->I.active && Ctrl->I.inc[col] > 0.0) ? ceil  (xyzmin[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmin[col];
							high = (Ctrl->I.active && Ctrl->I.inc[col] > 0.0) ? floor (xyzmax[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmax[col];
							if (low  > xyzminL[col]) xyzminL[col] = low;
							if (high < xyzmax[col])  xyzmaxL[col] = high;
							if (do_report) {	/* Last time so finalize */
								low  = xyzminL[col];
								high = xyzmaxL[col];
							}
						}
						else {
							low  = (Ctrl->I.active && Ctrl->I.inc[col] > 0.0) ? floor (xyzmin[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmin[col];
							high = (Ctrl->I.active && Ctrl->I.inc[col] > 0.0) ? ceil  (xyzmax[col] / Ctrl->I.inc[col]) * Ctrl->I.inc[col] : xyzmax[col];
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
			if (do_report) {
				if (Ctrl->C.extra) {	/* Needed by gmtinit_get_region_from_data */
					Out->data[4] = gmt_get_column_type (GMT, GMT_IN, GMT_X);
					Out->data[5] = gmt_get_column_type (GMT, GMT_IN, GMT_Y);
				}
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write data record to output destination */
			}
			got_stuff = true;		/* We have at least reported something */
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
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		in = In->data;

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
				GMT_Report (API, GMT_MSG_ERROR, "Not enough columns to support the -S option\n");
				Return (GMT_DIM_TOO_SMALL);
			}
			if (Ctrl->E.active && Ctrl->E.col >= ncol) {
  				GMT_Report (API, GMT_MSG_ERROR, "Option -E: Chosen column exceeds column range (0-%d).\n", ncol-1);
				Return (GMT_DIM_TOO_LARGE);
			}
			if (Ctrl->T.active && Ctrl->T.col >= ncol) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -T: Chosen column exceeds column range (0-%d).\n", ncol-1);
				Return (GMT_DIM_TOO_LARGE);
			}
			if (Ctrl->T.active) ncol = Ctrl->T.col + 1;
			if (give_r_string) ncol = 2;	/* Since we only will concern ourselves with lon/lat or x/y */
			if (ncol == 0) {
				GMT_Report (API, GMT_MSG_ERROR, "Something went wrong, ncol is still = 0. Maybe a call from an external program? Have to abort here.\n");
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
				if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)(2*ncol+Ctrl->C.extra), GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
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
		GMT_Report (API, GMT_MSG_INFORMATION, buffer);
	}
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	if (!got_stuff) GMT_Report (API, GMT_MSG_ERROR, "No input data found!\n");

	GMT->current.io.geo.range = save_range;	/* Restore what we changed */
	if (Ctrl->C.active) {	/* Restore previous output col types */
		gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, GMT_MAX_COLUMNS, int);
	}
	if (Ctrl->E.active)
		gmt_M_free (GMT, dchosen);

	for (col = 0; col < ncol; col++) gmt_M_free (GMT, Z[col]);

	GMT->current.setting.io_lonlat_toggle[GMT_OUT] = save_t;

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_minmax (void *V_API, int mode, void *args) {
	/* This was the GMT4 name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (gmt_M_compat_check (API->GMT, 4)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Module minmax is deprecated; use gmtinfo.\n");
		return (GMT_Call_Module (API, "gmtinfo", mode, args));
	}
	GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: minmax\n");
	return (GMT_NOT_A_VALID_MODULE);
}
