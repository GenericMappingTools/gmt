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
 * API functions to support the gmtconvert application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: Read one or more data tables and can concatenated them
 * vertically [Default] or horizontally (pasting), select certain columns,
 * report only first and/or last record per segment, only print segment
 * headers, and only report segments that passes a segment header search.
 */

#include "gmt_dev.h"
#include "longopt/gmtconvert_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtconvert"
#define THIS_MODULE_MODERN_NAME	"gmtconvert"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert, paste, or extract columns from data tables"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioqsw" GMT_OPT("HMm")

#define INV_ROWS	1
#define INV_SEGS	2
#define INV_TBLS	4

#define EXCLUDE_HEADERS		0
#define EXCLUDE_DUPLICATES	1

/* Control structure for gmtconvert */

struct GMTCONVERT_CTRL {
	struct GMTCONVERT_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct GMTCONVERT_A {	/* -A */
		bool active;
	} A;
	struct GMTCONVERT_C {	/* -C[+l<min>+u<max>+i>] */
		bool active, invert;
		uint64_t min, max;
	} C;
	struct GMTCONVERT_D {	/* -D[<template>][+o<orig>] */
		bool active;
		bool origin;
		unsigned int mode;
		unsigned int t_orig, s_orig;
		char *name;
	} D;
	struct GMTCONVERT_E {	/* -E */
		bool active;
		bool end;
		int mode;	/* -3, -1, -1, 0, or increment stride */
	} E;
	struct GMTCONVERT_F {	/* -F<mode> */
		bool active;
		struct GMT_SEGMENTIZE S;
	} F;
	struct GMTCONVERT_I {	/* -I[ast] */
		bool active;
		unsigned int mode;
	} I;
	struct GMTCONVERT_L {	/* -L */
		bool active;
	} L;
	struct GMTCONVERT_N {	/* -N<col>[+a|d] sorting */
		bool active;
		int dir;	/* +1 ascending [default], -1 descending */
		uint64_t col;
	} N;
	struct GMTCONVERT_Q {	/* -Q<selections> */
		bool active;
		struct GMT_INT_SELECTION *select;
	} Q;
	struct GMTCONVERT_S {	/* -S[~]\"search string\" */
		bool active;
		bool exact;
		struct GMT_TEXT_SELECTION *select;
	} S;
	struct GMTCONVERT_T {	/* -T[h][d[[~]selection]] */
		bool active[2];
		bool text;
		struct GMT_INT_SELECTION *C;
	} T;
	struct GMTCONVERT_W {	/* -W[+n] */
		bool active;
		unsigned int mode;
	} W;
	struct GMTCONVERT_Z {	/* -Z[<first>]:[<last>] [DEPRECATED - use -q instead] */
		bool active;
		bool transpose;	/* -Z with no arguments means transpose dataset */
		int64_t first, last;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTCONVERT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTCONVERT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.max = ULONG_MAX;	/* Max records possible in one segment */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTCONVERT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->D.name);
	if (C->S.active) gmt_free_text_selection (GMT, &C->S.select);
	if (C->Q.active) gmt_free_int_selection (GMT, &C->Q.select);
	if (C->T.active[EXCLUDE_DUPLICATES]) gmt_free_int_selection (GMT, &C->T.C);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-A] [-C[+l<min>][+u<max>][+i]] [-D[<template>[+o<orig>]]] "
		"[-E[f|l|m|M<stride>]] [-F%s] [-I[tsr]] [-L] [-N<col>[+a|d]] [-Q[~]<selection>] [-S[~]\"search string\"|+f<file>[+e] | -S[~]/<regexp>/[i][+e]]"
		"[-T[h][d[[~]<selection>]]] [%s] [-W[+n]] [-Z] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_SEGMENTIZE4, GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT,
		GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A Paste files horizontally, not concatenate vertically [Default]. "
		"All files must have the same number of segments and rows, "
		"but they may differ in their number of columns.");
	GMT_Usage (API, 1, "\n-C[+l<min>][+u<max>][+i]");
	GMT_Usage (API, -2, "Only output segments whose number of records matches criteria:");
	GMT_Usage (API, 3, "+l Segment must have at least <min> records [0].");
	GMT_Usage (API, 3, "+u Segment must have at most <max> records [inf].");
	GMT_Usage (API, 3, "+i Invert the test.");
	GMT_Usage (API, 1, "\n-D[<template>[+o<orig>]]");
	GMT_Usage (API, -2, "Write individual segments to separate files [Default writes one "
		"multisegment file to standard output].  Append file name template which MUST "
		"contain a C-style format for an integer (e.g., %%d) that represents "
		"a sequential segment number across all tables (if more than one table) "
		"[Default uses gmtconvert_segment_%%d.txt (or .bin for binary)]. "
		"Use +o<orig> to start numbering at <orig> [0]. "
		"Alternatively, supply a template with two long formats and we will "
		"replace them with the table number and table segment numbers. "
		"Use +o<t_orig>/<s_orig> to start numbering at <t_orig> for tables and <s_orig> for segments [0/0].");
	GMT_Usage (API, 1, "\n-E[f|l|m|M<stride>]");
	GMT_Usage (API, -2, "Extract first and last record per segment only [Output all records]. Optional directives:");
	GMT_Usage (API, 3, "f: Output first record only.");
	GMT_Usage (API, 3, "l: Output last record only.");
	GMT_Usage (API, 3, "m: Output every <stride> records.");
	GMT_Usage (API, 3, "M: Same as m but always includes the last record.");
	gmt_segmentize_syntax (API->GMT, 'F', 0);
	GMT_Usage (API, 1, "\n-I[tsr]");
	GMT_Usage (API, -2, "Invert output order of (t)ables, (s)egments, or (r)ecords.  Append any combination of directives:");
	GMT_Usage (API, 3, "t: Reverse the order of input tables on output.");
	GMT_Usage (API, 3, "s: Reverse the order of segments within each table on output.");
	GMT_Usage (API, 3, "r: Reverse the order of records within each segment on output [Default].");
	GMT_Usage (API, 1, "\n-L Output only segment headers and skip all data records. "
		"Requires ASCII input data [Output headers and data].");
	GMT_Usage (API, 1, "\n-N<col>[+a|d]");
	GMT_Usage (API, -2, "Numerically sort all records per segment based on data in column <col>:");
	GMT_Usage (API, 3, "+a Sort into ascending order [Default].");
	GMT_Usage (API, 3, "+d Sort into descending order.");
	GMT_Usage (API, 1, "\n-Q[~]<selection>");
	GMT_Usage (API, -2, "Only output specified segment numbers in <selection> [All]. "
		"<selection> syntax is [~]<range>[,<range>,...] where each <range> of items is "
		"either a single number, start-stop (for range), start:step:stop (for stepped range), "
		"or +f<file> for a file list with one <range> selection per line. "
		"A leading ~ will invert the selection and write all segments but the ones listed.");
	GMT_Usage (API, 1, "\n-S[~]\"search string\"|+f<file>[+e] | -S[~]/<regexp>/[i][+e]");
	GMT_Usage (API, -2, "Only output segments whose headers contain the pattern \"search string\". "
		"Use -S~\"search string\" to output segment that DO NOT contain this pattern. "
		"If your pattern begins with ~, escape it with \\~. "
		"To match OGR aspatial values, use name=value, and to match headers against "
		"extended regular expressions use -S[~]/regexp/[i] (i for case-insensitive). "
		"Instead of \"search string\", give +f<file> for a file with such patterns, one per line. "
		"To give a single pattern starting with +f, escape it with \\+f. "
		"Any of these three forms accept an optional +e to require an exact match [Default will match sub-strings]. ");
	GMT_Usage (API, 1, "\n-T[h][d[[~]<selection>]]");
	GMT_Usage (API, -2, "Skip certain types of records.  Append one or both of these directives:");
	GMT_Usage (API, 3, "h: Prevent the writing of segment headers [Default].");
	GMT_Usage (API, 3, "d: Prevent the writing of duplicate data records.");
	GMT_Usage (API, -2, "Optionally, append selection of columns to consider in the test [all]. "
		"<selection> syntax is [~]<range>[,<range>,...] where each <range> of items is "
		"either a single number, start-stop (for range), start:step:stop (for stepped range). "
		"To include trailing text in the comparison, add column t.  If no numerical columns "
		"are specified, only t, then we only use trailing text comparisons to decide.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W[+n]");
	GMT_Usage (API, -2, "Convert trailing text to numbers, if possible.  Append +n to suppress NaN columns.");
	GMT_Usage (API, 1, "\n-Z Transpose the single segment in the dataset. Any trailing text is lost.");
	GMT_Option (API, "a,bi,bo,d,e,f,g,h,i,o,q,s,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTCONVERT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtconvert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int pos, n_errors = 0, k;
	int n = 0;
	int64_t value = 0;
	char p[GMT_BUFSIZ] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;
			case '>':	/* Got named output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Out.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->Out.file));
				break;

			/* Processes program-specific parameters */

			case 'A':	/* pAste mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'C':	/* record-count selection mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				pos = 0;
				while (gmt_getmodopt (GMT, 'C', opt->arg, "ilu", &pos, p, &n_errors) && n_errors == 0) {	/* Looking for +i, +l, +u */
					switch (p[0]) {
						case 'i':	/* Invert selection */
							Ctrl->C.invert = true;	break;
						case 'l':	/* Set fewest records required */
						 	if ((value = atol (&p[1])) < 0)
								GMT_Report (API, GMT_MSG_ERROR, "The -C+l modifier was given negative record count!\n");
							else
								Ctrl->C.min = (uint64_t)value;
							break;
						case 'u':	/* Set max records required */
					 		if ((value = atol (&p[1])) < 0)
								GMT_Report (API, GMT_MSG_ERROR, "The -C+u modifier was given negative record count!\n");
							else
								Ctrl->C.max = (uint64_t)value;
							break;
						default:	/* These are caught in gmt_getmodopt so break is just for Coverity */
							break;
					}
				}
				break;
			case 'D':	/* Write each segment to a separate output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if ((c = strstr (opt->arg, "+o"))) {	/* Gave new origins for tables and segments (or just segments) */
					n = sscanf (&c[2], "%d/%d", &Ctrl->D.t_orig, &Ctrl->D.s_orig);
					if (n == 1) Ctrl->D.s_orig = Ctrl->D.t_orig, Ctrl->D.t_orig = 0;
					c[0] = '\0';	/* Chop off modifier */
					Ctrl->D.origin = true;
				}
				if (*opt->arg) /* arg is optional */
					Ctrl->D.name = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'E':	/* Extract ends only */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				switch (opt->arg[0]) {
					case 'f':		/* Get first point only */
						Ctrl->E.mode = -1; break;
					case 'l':		/* Get last point only */
						Ctrl->E.mode = -2; break;
					case 'M':		/* Set modulo step */
						Ctrl->E.end = true;	/* Include last point */
						/* Intentionally fall through - to set the step */
					case 'm':		/* Set modulo step */
						Ctrl->E.mode = atoi (&opt->arg[1]); break;
					default:		/* Get first and last point only */
						Ctrl->E.mode = -3; break;
				}
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				if (opt->arg[0] == '\0') {	/* No arguments, must be old GMT4 option -F */
					if (gmt_M_compat_check (GMT, 4)) {
						GMT_Report (API, GMT_MSG_COMPAT,
						            "Option -F for output columns is deprecated; use -o instead\n");
						gmt_parse_o_option (GMT, opt->arg);
					}
					else
						n_errors += gmt_default_option_error (GMT, opt);
					break;
				}
				/* Modern options */
				n_errors += gmt_parse_segmentize (GMT, opt->option, opt->arg, 0, &(Ctrl->F.S));
				break;
			case 'I':	/* Invert order or tables, segments, rows as indicated */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				for (k = 0; opt->arg[k]; k++) {
					switch (opt->arg[k]) {
						case 't': Ctrl->I.mode |= INV_TBLS; break;	/* Reverse table order */
						case 's': Ctrl->I.mode |= INV_SEGS; break;	/* Reverse segment order */
						case 'r': Ctrl->I.mode |= INV_ROWS; break;	/* Reverse record order */
						default:
							GMT_Report (API, GMT_MSG_ERROR,
							            "The -I option does not recognize modifier %c\n", (int)opt->arg[k]);
							n_errors++;
							break;
					}
				}
				if (Ctrl->I.mode == 0) Ctrl->I.mode = INV_ROWS;	/* Default is -Ir */
				break;
			case 'L':	/* Only output segment headers */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'N':	/* Sort per segment on specified column */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				if ((c = strstr (opt->arg, "+a")) || (c = strstr (opt->arg, "+d"))) {	/* New syntax */
					Ctrl->N.dir = (c[1] == 'd') ? -1 : +1;
					c[0] = '\0';	/* Chop off modifier */
				}
				value = atol (opt->arg);
				if (c)
					c[0] = '+';	/* Restore modifier */
				else
					Ctrl->N.dir = (value < 0 || opt->arg[0] == '-') ? -1 : +1;
				Ctrl->N.col = int64_abs (value);
				break;
			case 'Q':	/* Only report for specified segment numbers */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.select = gmt_set_int_selection (GMT, opt->arg);
				break;
			case 'S':	/* Segment header pattern search */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				if ((c = strstr (opt->arg, "+e"))) {	/* exact match */
					Ctrl->S.exact = true;
					c[0] = '\0';	/* Temporarily hide this string */
				}
				Ctrl->S.select = gmt_set_text_selection (GMT, opt->arg);
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'T':	/* -T[h]: Do not write segment headers, -Td: Skip duplicate records */
				strncpy (p, opt->arg, GMT_BUFSIZ-1);
				if ((c = strchr (p, 'd'))) { /* Skip duplicates */
					char *d = NULL;
					n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active[EXCLUDE_DUPLICATES]);
					Ctrl->T.active[EXCLUDE_DUPLICATES] = true;
					if ((d = strstr (c, ",t")) || (d = strchr (c, 't'))) {	/* Got either d<cols>,t or just t */
						Ctrl->T.text = true;
						d[0] = '\0';
					}
					if (c[1]) Ctrl->T.C = gmt_set_int_selection (GMT, &c[1]);	/* If we gave -Tdt then no columns and c[1] is 0 */
				}
				if (!p[0] || strchr (p, 'h')) {	/* Skip segment headers */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active[EXCLUDE_HEADERS]);
					Ctrl->T.active[EXCLUDE_HEADERS] = true;
				}
				break;
			case 'W':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if (!strncmp (opt->arg, "+n", 2U))
					Ctrl->W.mode = 1;
				break;
			case 'Z':
				if (opt->arg[0]) {	/* Deprecated column selector */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
					GMT_Report (API, GMT_MSG_COMPAT, "Option -Z is deprecated (but still works); Use common option -q instead\n");
					if ((c = strchr (opt->arg, ':')) || (c = strchr (opt->arg, '/'))) {	/* Got [<first>]:[<last>] or [<first>]/[<last>] */
						char div = c[0];	/* Either : or / */
						if (opt->arg[0] == div) /* No first given, default to 0 */
							Ctrl->Z.last = atol (&opt->arg[1]);
						else { /* Gave first, and maybe last */
							c[0] = '\0';	/* Chop off last */
							Ctrl->Z.first = atol (opt->arg);
							Ctrl->Z.last = (c[1]) ? (int64_t)atol (&c[1]) : INTMAX_MAX;	/* Last record if not given */
							c[0] = div;	/* Restore */
						}
					}
					else /* No colon means first is 0 and given value is last */
						Ctrl->Z.last = atol (opt->arg);
					/* Adjust to system where first record is 1 since we must increment n_in_rows before applying -Z check later */
					Ctrl->Z.first++;	if (Ctrl->Z.last < INTMAX_MAX) Ctrl->Z.last++;
					GMT_Report (API, GMT_MSG_DEBUG, "Output record numbers %" PRIu64 " through = %" PRIu64 "\n", Ctrl->Z.first, Ctrl->Z.last);
				}
				else {	/* Transpose dataset */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.transpose);
					n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->D.active) {	/* Validate the name template, if given */
		/* Must write individual segments to separate files so create the needed name template */
		unsigned int n_formats = 0;
		if (!Ctrl->D.name) {	/* None give, auto-assign to segment files with extension based on binary or not */
			Ctrl->D.name = GMT->common.b.active[GMT_OUT] ?
				strdup ("gmtconvert_segment_%d.bin") : strdup ("gmtconvert_segment_%d.txt");
			Ctrl->D.mode = GMT_WRITE_SEGMENT;
		}
		else { /* Ctrl->D.name given, need to check correct format */
			char *p = Ctrl->D.name;
			while ( (p = strchr (p, '%')) ) {
				/* found %, now check format */
				++p;	/* Skip the % sign */
				p += strspn (p, "0123456789."); /* span past digits and decimal */
				if ( strspn (p, "diu") == 0 ) {
					/* No valid conversion specifier */
					GMT_Report (API, GMT_MSG_ERROR,
						"Option -D: Use of unsupported conversion specifier at position %" PRIuS " in format string '%s'.\n",
						p - Ctrl->D.name + 1, Ctrl->D.name);
					n_errors++;
				}
				++n_formats;
			}
			if ( n_formats == 0 || n_formats > 2 ) {	/* Incorrect number of format specifiers */
				GMT_Report (API, GMT_MSG_ERROR,
					"Option -D: Incorrect number of format specifiers in format string '%s'.\n",
					Ctrl->D.name);
				n_errors++;
			}
			/* The io_mode tells the i/o function to split tables or segments into files, if requested */
			Ctrl->D.mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENT: GMT_WRITE_SEGMENT;
		}
	}
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0,
	                                 "Must specify number of columns in binary input data (-bi)\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && (Ctrl->L.active || Ctrl->S.active),
	                                 "Options -L or -S requires ASCII input data\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (Ctrl->C.min > Ctrl->C.max),
	                                 "Option -C: minimum records cannot exceed maximum records\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.name && !strstr (Ctrl->D.name, "%"),
	                                 "Option -D: Output template must contain %%d\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->S.active,
	                                 "Only one of -Q and -S can be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->F.active,
	                                 "The -N option cannot be used with -F\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Out.active && Ctrl->D.active,
	                                 "The -D option cannot be used with ->\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && GMT->common.q.active[GMT_OUT],
	                                 "The deprecated -Z option cannot be used with -qo\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL bool gmtconvert_is_duplicate_row (struct GMT_DATASEGMENT *S, struct GMT_INT_SELECTION *C, bool text, uint64_t row) {
	uint64_t col, k;
	if (C == NULL) {
		/* Loop over all columns and compare the two records, if any differ then return false.
		 * If passes all columns then they are the same and we return true. */
		if (!text) {
			for (col = 0; col < S->n_columns; col++)
				if (!doubleAlmostEqualZero (S->data[col][row], S->data[col][row-1])) return false;
		}
	}
	else if (C->invert) {	/* Only compare the columns not given in select */
		for (col = k = 0; col < S->n_columns; col++) {
			if (col == C->item[k]) {	/* Skip this guy */
				k++;
				continue;
			}
			if (!doubleAlmostEqualZero (S->data[col][row], S->data[col][row-1])) return false;
		}
	}
	else {	/* Only compare the columns given in select */
		for (k = 0; k < C->n; k++) {
			col = C->item[k];
			if (!doubleAlmostEqualZero (S->data[col][row], S->data[col][row-1])) return false;
		}
	}
	if (text && S->text && S->text[row] && S->text[row-1]) return (!strcmp (S->text[row], S->text[row-1]));	/* Also compare trailing text */
	return true;
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtconvert (void *V_API, int mode, void *args) {
	bool match = false, prevent_seg_headers = false;
	int error = 0;
	uint64_t out_col, col, n_cols_in = 0, n_cols_out, tbl, tlen, n_duplicates = 0;
	uint64_t n_horizontal_tbls, n_vertical_tbls, tbl_ver, tbl_hor, use_tbl;
	uint64_t last_row, n_rows, row, seg, n_out_seg = 0, out_seg = 0;
	int64_t n_in_rows;

	double *val = NULL;

	char *method[2] = {"concatenated", "pasted"}, *p = NULL;

	struct GMTCONVERT_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D[2] = {NULL, NULL};	/* Pointer to GMT multisegment table(s) in and out */
	struct GMT_DATASEGMENT *S = NULL, *Si = NULL, *So = NULL;
	struct GMT_DATASET_HIDDEN *DHi = NULL, *DHo = NULL;
	struct GMT_DATATABLE_HIDDEN *THi = NULL, *THo = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SHi = NULL, *SHo = NULL;
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

	/*---------------------------- This is the gmtconvert main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if (GMT->common.g.active) {
		unsigned int i;
		for (i = 0; i < GMT->common.g.n_methods; i++) {	/* Go through each criterion */
			if (GMT->common.g.method[i] == GMT_NEGGAP_IN_MAP_COL ||
				GMT->common.g.method[i] == GMT_POSGAP_IN_MAP_COL ||
				GMT->common.g.method[i] == GMT_ABSGAP_IN_MAP_COL ||
				GMT->common.g.method[i] == GMT_GAP_IN_PDIST) {
				GMT_Report (API, GMT_MSG_ERROR, "The -g option cannot use X, Y, or D since no projection can be set. Use mapproject first\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		Return (API->error);	/* Establishes data files or stdin */
	}

	/* Read in the input tables */

	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	if (D[GMT_IN]->n_records == 0) {
		GMT_Report (API, GMT_MSG_WARNING, "No data records provided\n");
		Return (GMT_NOERROR);
	}

	if (Ctrl->Z.transpose) {	/* Transpose the input data set per segment */
		struct GMT_DATASET *Dt = NULL;
		if ((Dt = gmt_transpose_dataset (GMT, D[GMT_IN])) == NULL) {	/* Transposing failed */
			GMT_Report (API, GMT_MSG_ERROR, "Transposing of input data segment failed.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Transposed dimensions: n_rows = %" PRIu64 " n_columns = %" PRIu64"\n", Dt->n_records, Dt->n_columns);
		if (GMT_Destroy_Data (API, &D[GMT_IN]) != GMT_NOERROR) {	/* Be gone with the original input */
			Return (API->error);
		}
		D[GMT_IN] = Dt;	/* Replace the input pointer with the transposed input */
	}

	if (GMT->common.a.active && D[GMT_IN]->n_tables > 1) {
		GMT_Report (API, GMT_MSG_ERROR, "The -a option requires a single table only.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	DHi = gmt_get_DD_hidden (D[GMT_IN]);
	THi = gmt_get_DT_hidden (D[GMT_IN]->table[0]);
	if (GMT->common.a.active && THi->ogr) {
		GMT_Report (API, GMT_MSG_ERROR, "The -a option requires a single table without OGR metadata.\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->T.active[EXCLUDE_HEADERS] && !gmt_M_file_is_memory (Ctrl->Out.file))
		prevent_seg_headers = true;

	if (Ctrl->T.active[EXCLUDE_DUPLICATES] && Ctrl->T.C) {
		for (unsigned int k = 0; k <  Ctrl->T.C->n; k++) {
			if (Ctrl->T.C->item[k] >= D[GMT_IN]->n_columns) {
				GMT_Report (API, GMT_MSG_ERROR, "The -Td<cols> option has entries (%d) exceeding the number of data columns (%d).\n", (unsigned int)Ctrl->T.C->item[k], (unsigned int)D[GMT_IN]->n_columns);
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	if (prevent_seg_headers)	/* Turn off segment headers on file output */
		GMT->current.io.skip_headers_on_outout = true;

	if (Ctrl->F.active) {	/* Segmentizing happens here and then we are done */
		D[GMT_OUT] = gmt_segmentize_data (GMT, D[GMT_IN], &(Ctrl->F.S));	/* Segmentize the data */
		if (GMT_Destroy_Data (API, &D[GMT_IN]) != GMT_NOERROR) {	/* Be gone with the original */
			Return (API->error);
		}
		if (D[GMT_OUT]->n_segments > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

		DHo = gmt_get_DD_hidden (D[GMT_OUT]);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, D[GMT_OUT]->geometry, DHo->io_mode, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_NOERROR) {
			Return (API->error);
		}
		if (prevent_seg_headers) GMT->current.io.skip_headers_on_outout = false;	/* Restore to default if it was changed */
		Return (GMT_NOERROR);	/* We are done! */
	}

	if (Ctrl->W.active) {	/* Text to data happens here and then we are done */
		double out[GMT_BUFSIZ], *tmp = NULL;
		struct GMT_RECORD *Out = NULL;
		char *nan = NULL;
		uint64_t n_col = 0, k;
		S = D[GMT_IN]->table[0]->segment[0];	/* Short-hand */
		if (S->text) {	/* Has trailing text */
			n_col = n_cols_in = GMT_Get_Values (API, S->text[0], out, GMT_BUFSIZ);
			if (Ctrl->W.mode) {	/* Only wants non-NaN columns from trailing text */
				nan = gmt_M_memory (GMT, NULL, n_col, char);
				tmp = gmt_M_memory (GMT, NULL, n_col, double);
				for (col = 0; col < n_col; col++) if (gmt_M_is_dnan (out[col])) nan[col] = 1, n_cols_in--;
				GMT_Report (API, GMT_MSG_INFORMATION, "First record trailing text converted to %" PRIu64 " columns, with %" PRIu64 " of them yielding NaNs [skipped]\n", n_col, n_col - n_cols_in);
			}
			else
				GMT_Report (API, GMT_MSG_INFORMATION, "First record trailing text converted to %" PRIu64 " columns\n", n_col);
		}
		else
			GMT_Report (API, GMT_MSG_WARNING, "No trialing text found in first record; -W will not have any effect.\n");
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			if (Ctrl->W.mode) {gmt_M_free (GMT, nan); gmt_M_free (GMT, tmp);}
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			if (Ctrl->W.mode) {gmt_M_free (GMT, nan); gmt_M_free (GMT, tmp);}
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)(n_cols_in + S->n_columns), GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			if (Ctrl->W.mode) {gmt_M_free (GMT, nan); gmt_M_free (GMT, tmp);}
			Return (error);
		}
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
		for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
			for (seg = 0; seg < D[GMT_IN]->table[tbl]->n_segments; seg++) {	/* For each segment in the tables */
				S = D[GMT_IN]->table[tbl]->segment[seg];	/* Short-hand */
				if (D[GMT_IN]->n_segments > 1 || S->header) GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, S->header);
				for (row = 0; row < S->n_rows; row++) {
					for (col = 0; col < S->n_columns; col++)
						out[col] = S->data[col][row];
					if (S->text) {
						if (Ctrl->W.mode) {	/* Exclude NaN columns */
							n_cols_in = GMT_Get_Values (API, S->text[row], tmp, (int)n_col);
							for (col = k = 0; col < n_cols_in; col++) if (!nan[col]) out[S->n_columns+k] = tmp[col], k++;
						}
						else
							GMT_Get_Values (API, S->text[row], &out[S->n_columns], (int)n_col);
					}
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
				}
			}
		}
		gmt_M_free (GMT, Out);
		if (Ctrl->W.mode) {
			gmt_M_free (GMT, nan);
			gmt_M_free (GMT, tmp);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		Return (GMT_NOERROR);	/* We are done! */
	}

	/* Determine number of input and output columns for the selected options.
	 * For -A, require all tables to have the same number of segments and records. */

	for (tbl = n_cols_in = n_cols_out = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		if (Ctrl->A.active) {	/* All tables must be of the same vertical shape */
			if (tbl && D[GMT_IN]->table[tbl]->n_records  != D[GMT_IN]->table[tbl-1]->n_records)  error = true;
			if (tbl && D[GMT_IN]->table[tbl]->n_segments != D[GMT_IN]->table[tbl-1]->n_segments) error = true;
		}
		n_cols_in += D[GMT_IN]->table[tbl]->n_columns;				/* This is the case for -A */
		n_cols_out = MAX (n_cols_out, D[GMT_IN]->table[tbl]->n_columns);	/* The widest table encountered */
	}
	n_cols_out = (Ctrl->A.active) ? n_cols_in : n_cols_out;	/* Default or Reset since we did not have -A */

	if (error) {
		GMT_Report (API, GMT_MSG_ERROR, "Parsing requires files with same number of records.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (n_cols_out == 0 && !GMT->current.io.trailing_text[GMT_OUT]) {
		GMT_Report (API, GMT_MSG_ERROR, "Selection led to no output columns.\n");
		Return (GMT_RUNTIME_ERROR);

	}
	if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_cols_out, (D[GMT_IN]->type == GMT_READ_DATA) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}

	if (Ctrl->S.active && GMT->current.io.ogr == GMT_OGR_TRUE && (p = strchr (Ctrl->S.select->pattern[0], '=')) != NULL) {	/* Want to search for an aspatial value */
		*p = 0;	/* Skip the = sign */
		if ((Ctrl->S.select->ogr_item = gmt_get_ogr_id (GMT->current.io.OGR, Ctrl->S.select->pattern[0])) != GMT_NOTSET) {
			Ctrl->S.select->ogr_match = true;
			p++;
			strcpy (Ctrl->S.select->pattern[0], p);	/* Shift over the string after skipping name= */
		}
	}

	/* We now know the exact number of segments and columns and an upper limit on total records.
	 * Allocate data set with a single table with those proportions. This copies headers as well */

	DHi->dim[GMT_COL] = n_cols_out;	/* State we want a different set of columns on output */
	D[GMT_OUT] = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_ALLOC + ((Ctrl->A.active) ? GMT_ALLOC_HORIZONTAL : GMT_ALLOC_NORMAL), D[GMT_IN]);
	DHo = gmt_get_DD_hidden (D[GMT_OUT]);

	n_horizontal_tbls = (Ctrl->A.active) ? D[GMT_IN]->n_tables : 1;	/* Only with pasting do we go horizontally */
	n_vertical_tbls   = (Ctrl->A.active) ? 1 : D[GMT_IN]->n_tables;	/* Only for concatenation do we go vertically */
	val = gmt_M_memory (GMT, NULL, n_cols_in, double);

	for (tbl_ver = 0; tbl_ver < n_vertical_tbls; tbl_ver++) {	/* Number of tables to place vertically */
		D[GMT_OUT]->table[tbl_ver]->n_records = 0;	/* Reset record count per table since we may return fewer than the original */
		THo = gmt_get_DT_hidden (D[GMT_OUT]->table[tbl_ver]);
		for (seg = 0; seg < D[GMT_IN]->table[tbl_ver]->n_segments; seg++) {	/* For each segment in the tables */
			S  = D[GMT_IN]->table[tbl_ver]->segment[seg];	/* Current input segment */
			So = D[GMT_OUT]->table[tbl_ver]->segment[seg];	/* Current output segment */
			SHi = gmt_get_DS_hidden (S);
			SHo = gmt_get_DS_hidden (So);
			if (Ctrl->L.active) SHo->mode = GMT_WRITE_HEADER;	/* Only write segment header */
			if (Ctrl->S.active) {		/* See if the combined segment header has text matching our search string */
				match = gmt_get_segtext_selection (GMT, Ctrl->S.select, S, Ctrl->S.exact, match);
				if (Ctrl->S.select->invert == match) SHo->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			}
			if (Ctrl->Q.active && !gmt_get_int_selection (GMT, Ctrl->Q.select, seg)) SHo->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			if (Ctrl->C.active) {	/* See if the number of records in this segment passes our test for output */
				match = (S->n_rows >= Ctrl->C.min && S->n_rows <= Ctrl->C.max);
				if (Ctrl->C.invert == match) SHo->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			}
			if (SHo->mode) continue;	/* No point copying values given segment content will be skipped */
			n_out_seg++;	/* Number of segments that passed the test */
			last_row = S->n_rows - 1;
			for (row = n_rows = 0, n_in_rows = 0; row <= last_row; row++) {	/* Go down all the rows */
				n_in_rows++;
				if (Ctrl->Z.active && (n_in_rows < Ctrl->Z.first || n_in_rows > Ctrl->Z.last)) continue;	/* Skip if outside limited record range */
				if (!Ctrl->E.active) {
					if (Ctrl->T.active[EXCLUDE_DUPLICATES] && row && gmtconvert_is_duplicate_row (S, Ctrl->T.C, Ctrl->T.text, row)) {
						n_duplicates++;
						continue;	/* Skip duplicate records */
					}
				}
				else if (Ctrl->E.mode < 0) {	/* Only pass first or last or both of them, skipping all others */
					if (row > 0 && row < last_row) continue;		/* Always skip the middle of the segment */
					if (row == 0 && !(-Ctrl->E.mode & 1)) continue;		/* First record, but we are to skip it */
					if (row == last_row && !(-Ctrl->E.mode & 2)) continue;	/* Last record, but we are to skip it */
				}
				else {	/* Only pass modulo E.mode records (this always includes the first row), if -EM then make sure we also write the last row */
					if ((row % Ctrl->E.mode) != 0) {	/* Check if last row and -EM was used */
						if (!Ctrl->E.end || row < last_row) continue;
					}
				}
				/* Pull out current virtual row (may consist of a single or many (-A) table rows) */
				for (tbl_hor = out_col = tlen = 0; tbl_hor < n_horizontal_tbls; tbl_hor++) {	/* Number of tables to place horizontally */
					use_tbl = (Ctrl->A.active) ? tbl_hor : tbl_ver;
					Si = D[GMT_IN]->table[use_tbl]->segment[seg];	/* Current input segment in this table */
					if (Si->text && Si->text[row]) tlen += strlen (Si->text[row]) + 1;	/* String + separator */
					for (col = 0; col < Si->n_columns; col++, out_col++) {	/* Now go across all columns in current table */
						val[out_col] = Si->data[col][row];
					}
				}
				for (col = 0; col < n_cols_out; col++) {	/* Now go across the single virtual row */
					if (col >= n_cols_in) continue;			/* This column is beyond end of this table */
					So->data[col][n_rows] = val[col];
				}
				if (tlen) {	/* must deal with trailing text(s) */
					bool add_separator = false;
					if (So->text == NULL) So->text = gmt_M_memory (GMT, NULL, S->n_rows, char *);
					So->text[n_rows] = calloc (tlen+1, sizeof(char));	/* Space for trailing \0 */
					for (tbl_hor = 0; tbl_hor < n_horizontal_tbls; tbl_hor++) {	/* Number of tables to place horizontally */
						use_tbl = (Ctrl->A.active) ? tbl_hor : tbl_ver;
						Si = D[GMT_IN]->table[use_tbl]->segment[seg];	/* Current input segment in this table */
						if (Si->text && Si->text[row]) {
							if (add_separator) strcat (So->text[n_rows], GMT->current.setting.io_col_separator);
							strcat (So->text[n_rows], Si->text[row]);
							add_separator = true;
						}
					}
				}
				n_rows++;
			}
			SHo->id = out_seg++;
			So->n_rows = n_rows;	/* Possibly shorter than originally allocated if -E is used */
			D[GMT_OUT]->table[tbl_ver]->n_records += n_rows;
			D[GMT_OUT]->n_records = D[GMT_OUT]->table[tbl_ver]->n_records;
			if (SHi->ogr) gmt_duplicate_ogr_seg (GMT, So, S);
		}
		THo->id = tbl_ver;
	}
	gmt_M_free (GMT, val);

	if (Ctrl->I.active) {	/* Must reverse the order of tables, segments and/or records */
		uint64_t tbl1, tbl2, row1, row2, seg1, seg2;
		void *p = NULL;
		if (Ctrl->I.mode & INV_ROWS) {	/* Must actually swap rows */
			GMT_Report (API, GMT_MSG_INFORMATION, "Reversing order of records within each segment.\n");
			for (tbl = 0; tbl < D[GMT_OUT]->n_tables; tbl++) {	/* Number of output tables */
				for (seg = 0; seg < D[GMT_OUT]->table[tbl]->n_segments; seg++) {	/* For each segment in the tables */
					for (row1 = 0, row2 = D[GMT_OUT]->table[tbl]->segment[seg]->n_rows - 1; row1 < D[GMT_OUT]->table[tbl]->segment[seg]->n_rows/2; row1++, row2--) {
						for (col = 0; col < D[GMT_OUT]->table[tbl]->segment[seg]->n_columns; col++)
							gmt_M_double_swap (D[GMT_OUT]->table[tbl]->segment[seg]->data[col][row1], D[GMT_OUT]->table[tbl]->segment[seg]->data[col][row2]);
					}
				}
			}
		}
		if (Ctrl->I.mode & INV_SEGS) {	/* Must reorder pointers to segments within each table */
			GMT_Report (API, GMT_MSG_INFORMATION, "Reversing order of segments within each table.\n");
			for (tbl = 0; tbl < D[GMT_OUT]->n_tables; tbl++) {	/* Number of output tables */
				for (seg1 = 0, seg2 = D[GMT_OUT]->table[tbl]->n_segments-1; seg1 < D[GMT_OUT]->table[tbl]->n_segments/2; seg1++, seg2--) {	/* For each segment in the table */
					p = D[GMT_OUT]->table[tbl]->segment[seg1];
					D[GMT_OUT]->table[tbl]->segment[seg1] = D[GMT_OUT]->table[tbl]->segment[seg2];
					D[GMT_OUT]->table[tbl]->segment[seg2] = p;
				}
			}
		}
		if (Ctrl->I.mode & INV_TBLS) {	/* Must reorder pointers to tables within dataset  */
			GMT_Report (API, GMT_MSG_INFORMATION, "Reversing order of tables within the data set.\n");
			for (tbl1 = 0, tbl2 = D[GMT_OUT]->n_tables-1; tbl1 < D[GMT_OUT]->n_tables/2; tbl1++, tbl2--) {	/* For each table */
				p = D[GMT_OUT]->table[tbl1];
				D[GMT_OUT]->table[tbl1] = D[GMT_OUT]->table[tbl2];
				D[GMT_OUT]->table[tbl2] = p;
			}
		}
	}

	/* Now ready for output */

	if (Ctrl->D.active) {	/* Set composite name and io-mode */
		Ctrl->Out.file = strdup (Ctrl->D.name);
		DHo->io_mode = Ctrl->D.mode;
		if (Ctrl->D.origin) {	/* Update IDs */
			for (tbl = 0; tbl < D[GMT_OUT]->n_tables; tbl++) {	/* For each table */
				THo = gmt_get_DT_hidden (D[GMT_OUT]->table[tbl]);
				THo->id += Ctrl->D.t_orig;
				for (seg = 0; seg < D[GMT_OUT]->table[tbl]->n_segments; seg++) {	/* For each segment */
					SHo = gmt_get_DS_hidden (D[GMT_OUT]->table[tbl]->segment[seg]);
					SHo->id += Ctrl->D.s_orig;
				}
			}
		}
	}
	else {	/* Just register output to stdout or the given file via ->outfile */
		if (GMT->common.a.output)	/* Must notify the machinery of this output type */
			DHo->io_mode = GMT_WRITE_OGR;
	}

	if (Ctrl->T.active[EXCLUDE_HEADERS] && gmt_M_file_is_memory (Ctrl->Out.file) && D[GMT_OUT]->n_segments > 1) {
		/* Since no file is written we must physically collate segments into a single segment per table first */
		unsigned int flag[3] = {0, 0, GMT_WRITE_SEGMENT};
		struct GMT_DATASET *D2 = NULL;
		if ((D2 = GMT_Convert_Data (API, D[GMT_OUT], GMT_IS_DATASET, NULL, GMT_IS_DATASET, flag)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while collating each table's segments into a single segment per table.\n");
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D[GMT_OUT]) != GMT_NOERROR) {	/* Remove the previously registered output dataset */
			Return (API->error);
		}
		D[GMT_OUT] = D2;	/* Hook up the reformatted dataset */
	}
	if (Ctrl->N.active) {	/* Sort the output data on selected column before writing */
		struct GMT_ORDER *Z = NULL;
		uint64_t max_len = 0;
		bool do_it = true;
		char *way[3] = {"descending", "", "ascending"};
		if (Ctrl->N.col >= D[GMT_OUT]->n_columns) {
			GMT_Report (API, GMT_MSG_WARNING, "Column selected (%d) as sorting key is outside range of valid columns [0-%d].  No sorting performed\n", (int)Ctrl->N.col, (int)(D[GMT_OUT]->n_columns - 1));
			do_it = false;
		}
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "Sort data based on column %d in %s order\n", (int)Ctrl->N.col, way[Ctrl->N.dir+1]);
		GMT->current.io.record_type[GMT_OUT] = GMT->current.io.record_type[GMT_IN];
		for (tbl = 0; do_it && tbl < D[GMT_OUT]->n_tables; tbl++) {	/* Number of output tables */
			for (seg = 0; seg < D[GMT_OUT]->table[tbl]->n_segments; seg++) {	/* For each segment in the tables */
				S = D[GMT_OUT]->table[tbl]->segment[seg];	/* Current segment */
				if (S->n_rows > max_len) {	/* (Re-)allocate memory for sort order array */
					Z = gmt_M_memory (GMT, Z, S->n_rows, struct GMT_ORDER);
					max_len =  S->n_rows;
				}
				for (row = 0; row < S->n_rows; row++) {	/* Load up value/order struct array */
					Z[row].value = S->data[Ctrl->N.col][row];
					Z[row].order = row;
				}
				gmt_sort_order (GMT, Z, S->n_rows, Ctrl->N.dir);	/* Sort per segment */
				gmt_prep_tmp_arrays (GMT, GMT_OUT, S->n_rows, 1);	/* Init or reallocate tmp vector */
				for (col = 0; col < S->n_columns; col++) {
					for (row = 0; row < S->n_rows; row++) {	/* Do the shuffle via a temp vector */
						GMT->hidden.mem_coord[GMT_X][row] = S->data[col][Z[row].order];
						if (S->text) GMT->hidden.mem_txt[row] = S->text[Z[row].order];
					}
					gmt_M_memcpy (S->data[col], GMT->hidden.mem_coord[GMT_X], S->n_rows, double);
					if (S->text) gmt_M_memcpy (S->text, GMT->hidden.mem_txt, S->n_rows, char *);
				}
			}
		}
		if (do_it) gmt_M_free (GMT, Z);
	}

	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, D[GMT_IN]->geometry, DHo->io_mode, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_NOERROR) {
		Return (API->error);
	}
	if (prevent_seg_headers) GMT->current.io.skip_headers_on_outout = false;	/* Restore to default if it was changed for file output */

	GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " tables %s, %" PRIu64 " records passed (input cols = %d; output cols = %d)\n",
		D[GMT_IN]->n_tables, method[Ctrl->A.active], D[GMT_OUT]->n_records, n_cols_in, n_cols_out);
	if (Ctrl->Q.active || Ctrl->S.active) GMT_Report (API, GMT_MSG_INFORMATION, "Extracted %" PRIu64 " from a total of %" PRIu64 " segments\n", n_out_seg, D[GMT_OUT]->n_segments);
	if (n_duplicates) GMT_Report (API, GMT_MSG_INFORMATION, "Eliminated %" PRIu64 " duplicate records\n", n_duplicates);

	Return (GMT_NOERROR);
}
