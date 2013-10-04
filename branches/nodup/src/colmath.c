/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the colmath application.
 *
 * Author:	Remko Scharroo
 * Date:	17-FEB-2011
 * Version:	5 API
 *
 * Brief synopsis: Read one or more data tables and can concatenated them
 * vertically [Default] or horizontally (pasting), select certain columns,
 * and do math on those columns. Then report the combined result.
 */

#define THIS_MODULE_NAME	"colmath"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Do mathematics on columns from data tables"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbfghios" GMT_OPT("HMm")

/* Control structure for colmath */

struct COLMATH_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A */
		bool active;
	} A;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q<segno> */
		bool active;
		uint64_t seg;
	} Q;
	struct S {	/* -S[~]\"search string\" */
		bool active;
		bool inverse;
		char *pattern;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
};

void *New_colmath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct COLMATH_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct COLMATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_colmath_Ctrl (struct GMT_CTRL *GMT, struct COLMATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	if (C->S.pattern) free (C->S.pattern);	
	GMT_free (GMT, C);	
}

int GMT_colmath_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: colmath [<table>] [-A] [-N] [-Q<seg>] [-S[~]\"search string\"] [-T] [%s]\n\t[%s] [%s]", GMT_V_OPT, GMT_b_OPT, GMT_f_OPT);
	GMT_Message (API, GMT_TIME_NONE, " [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] OPERATORS\n\n", GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Paste files horizontally, not concatenate vertically [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   All files must have the same number of segments and rows,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   but they may differ in their number of columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Only output segment number <seg> [All].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Only output segments whose headers contain the pattern \"string\".\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -S~\"string\" to output segment that DO NOT contain this pattern.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If your pattern begins with ~, escape it with \\~.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: -S requires -m and ASCII input data [Output all segments].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Prevent the writing of segment headers.\n");
	GMT_Option (API, "V,bi,bo,f,g,h,i,o,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_colmath_parse (struct GMT_CTRL *GMT, struct COLMATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to colmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* pAste mode */
				Ctrl->A.active = true;
				break;
			case 'N':	/* Skip all-NaN records */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Only report for specified segment number */
				Ctrl->Q.active = true;
				Ctrl->Q.seg = atol (opt->arg);
				break;
			case 'S':	/* Segment header pattern search */
				Ctrl->S.active = true;
				k = (opt->arg[0] == '\\' && strlen (opt->arg) > 3 && opt->arg[1] == '~') ? 1 : 0;	/* Special escape if pattern starts with ~ */
				if (opt->arg[0] == '~') Ctrl->S.inverse = true;
				Ctrl->S.pattern = strdup (&opt->arg[k+Ctrl->S.inverse]);
				break;
			case 'T':	/* Do not write segment headers */
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0, "Syntax error: Must specify number of columns in binary input data (-bi)\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && Ctrl->S.active, "Syntax error: -S requires ASCII input data\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && Ctrl->S.active, "Syntax error: Only one of -Q and -S can be used simultaneously\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_colmath_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_colmath (void *V_API, int mode, void *args)
{
	int error = 0;
	bool match = false, warn = false;
	uint64_t tbl, col, n_cols_in, n_cols_out, out_col;
	uint64_t n_horizontal_tbls, n_vertical_tbls, tbl_ver, tbl_hor, use_tbl;
	uint64_t row, last_row, n_rows, seg, out_seg = 0, n_out_seg = 0;
	
	double *val = NULL;

	char *method[2] = {"concatenated", "pasted"};
	
	struct GMT_OPTION *options = NULL;
	struct COLMATH_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D[2] = {NULL, NULL};	/* Pointer to GMT multisegment table(s) in and out */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_colmath_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_colmath_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_colmath_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_colmath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_colmath_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the colmath main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");

	if (Ctrl->T.active) GMT_set_segmentheader (GMT, GMT_OUT, false);	/* Turn off segment headers on output */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {
		Return (API->error);	/* Establishes data input */
	}
	
	/* Read in the input tables */
	
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

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
		GMT_Report (API, GMT_MSG_NORMAL, "Parsing requires files with same number of records.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (n_cols_out == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Selection lead to no output columns.\n");
		Return (GMT_RUNTIME_ERROR);
		
	}
	if (warn) GMT_Report (API, GMT_MSG_VERBOSE, "Some requested columns are outside the range of some tables and will be skipped.\n");
	
	/* We now know the exact number of segments and columns and an upper limit on total records.
	 * Allocate data set with a single table with those proportions. This copies headers as well */
	
	D[GMT_OUT] = GMT_alloc_dataset (GMT, D[GMT_IN], 0, n_cols_out, (Ctrl->A.active) ? GMT_ALLOC_HORIZONTAL : GMT_ALLOC_NORMAL);
	
	n_horizontal_tbls = (Ctrl->A.active) ? D[GMT_IN]->n_tables : 1;	/* Only with pasting do we go horizontally */
	n_vertical_tbls   = (Ctrl->A.active) ? 1 : D[GMT_IN]->n_tables;	/* Only for concatenation do we go vertically */
	val = GMT_memory (GMT, NULL, n_cols_in, double);
	
	for (tbl_ver = 0; tbl_ver < n_vertical_tbls; tbl_ver++) {	/* Number of tables to place vertically */
		for (seg = 0; seg < D[GMT_IN]->table[tbl_ver]->n_segments; seg++) {	/* For each segment in the tables */
			if (Ctrl->S.active) {		/* See if the combined segment header has text matching our search string */
				match = (D[GMT_OUT]->table[tbl_ver]->segment[seg]->header && strstr (D[GMT_OUT]->table[tbl_ver]->segment[seg]->header, Ctrl->S.pattern) != NULL);		/* true if we matched */
				if (Ctrl->S.inverse == match) D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			}
			if (Ctrl->Q.active && seg != Ctrl->Q.seg) D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			if (D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode) continue;	/* No point copying values given segment content will be skipped */
			n_out_seg++;	/* Number of segments that passed the test */
			last_row = D[GMT_IN]->table[tbl_ver]->segment[seg]->n_rows - 1;
			for (row = n_rows = 0; row <= last_row; row++) {	/* Go down all the rows */
				/* Pull out current virtual row (may consist of a single or many (-A) table rows) */
				for (tbl_hor = out_col = 0; tbl_hor < n_horizontal_tbls; tbl_hor++) {	/* Number of tables to place horizontally */
					use_tbl = (Ctrl->A.active) ? tbl_hor : tbl_ver;
					for (col = 0; col < D[GMT_IN]->table[use_tbl]->segment[seg]->n_columns; col++, out_col++) {	/* Now go across all columns in current table */
						val[out_col] = D[GMT_IN]->table[use_tbl]->segment[seg]->coord[col][row];
					}
				}
				for (col = 0; col < n_cols_out; col++) {	/* Now go across the single virtual row */
					if (col >= n_cols_in) continue;			/* This column is beyond end of this table */
					D[GMT_OUT]->table[tbl_ver]->segment[seg]->coord[col][n_rows] = val[col];
				}
				n_rows++;
			}
			D[GMT_OUT]->table[tbl_ver]->segment[seg]->id = out_seg++;
			D[GMT_OUT]->table[tbl_ver]->segment[seg]->n_rows = n_rows;	/* Possibly shorter than originally allocated if -E is used */
			D[GMT_OUT]->table[tbl_ver]->n_records += n_rows;
			D[GMT_OUT]->n_records = D[GMT_OUT]->table[tbl_ver]->n_records;
		}
		D[GMT_OUT]->table[tbl_ver]->id = tbl_ver;
	}
	GMT_free (GMT, val);

	/* Now ready for output */
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, D[GMT_IN]->geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, D[GMT_IN]->geometry, D[GMT_OUT]->io_mode, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_OK) {
		Return (API->error);
	}
	
	GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " tables %s, %" PRIu64 " records passed (input cols = %" PRIu64 "; output cols = %" PRIu64 ")\n", D[GMT_IN]->n_tables, method[Ctrl->A.active], D[GMT_OUT]->n_records, n_cols_in, n_cols_out);
	if (Ctrl->S.active) GMT_Report (API, GMT_MSG_VERBOSE, "Extracted %" PRIu64 " from a total of %" PRIu64 "segments\n", n_out_seg, D[GMT_OUT]->table[tbl_ver]->n_segments);

	Return (GMT_OK);
}
