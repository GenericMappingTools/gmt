/*--------------------------------------------------------------------
 *	$Id: gmtconvert_func.c,v 1.13 2011-05-14 00:04:06 guru Exp $
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
 * API functions to support the gmtconvert application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Read one or more data tables and can concatenated them
 * vertically [Default] or horizontally (pasting), select certain columns,
 * report only first and/or last record per segment, only print segment
 * headers, and only report segments that passes a segment header search.
 */

#include "gmt.h"

/* Control structure for gmtconvert */

struct GMTCONVERT_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct D {	/* -D[<template>] */
		GMT_LONG active;
		char *name;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q<segno> */
		GMT_LONG active;
		GMT_LONG seg;
	} Q;
	struct S {	/* -S[~]\"search string\" */
		GMT_LONG active;
		GMT_LONG inverse;
		char *pattern;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
};

void *New_gmtconvert_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTCONVERT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTCONVERT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_gmtconvert_Ctrl (struct GMT_CTRL *GMT, struct GMTCONVERT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free ((void *)C->Out.file);	
	if (C->D.name) free ((void *)C->D.name);	
	if (C->S.pattern) free ((void *)C->S.pattern);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gmtconvert_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtconvert %s [API] - Convert format or paste/extract columns from table data\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtconvert [files] [-A] [-D[<template>]] [-E[f|l]] [-I] [-L] [-N] [-Q<seg>]\n");
	GMT_message (GMT, "\t[-S[~]\"search string\"] [-T] [%s] [%s]\n\t[%s] [%s] [%s]\n", GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\n", GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Paste files horizontally, not concatenate vertically [Default].\n");
	GMT_message (GMT, "\t   All files must have the same number of segments and rows,\n");
	GMT_message (GMT, "\t   but they may differ in their number of columns.\n");
	GMT_message (GMT, "\t-D Writes individual segments to separate files [Default writes one\n");
	GMT_message (GMT, "\t   multisegment file to stdout].  Append file name template which MUST\n");
	GMT_message (GMT, "\t   contain a C-style format for a long integer (e.g., %%ld) that represents\n");
	GMT_message (GMT, "\t   a sequential segment number across all tables (if more than one table).\n");
	GMT_message (GMT, "\t   [Default uses gmtconvert_segment_%%ld.txt (or .bin for binary)].\n");
	GMT_message (GMT, "\t   Alternatively, supply a template with two long formats and we will\n");
	GMT_message (GMT, "\t   replace them with the table number and table segment numbers.\n");
	GMT_message (GMT, "\t-E Extract first and last point per segment only [Output all points].\n");
	GMT_message (GMT, "\t   Append f for first only or l for last only.\n");
	GMT_message (GMT, "\t-I Invert order of rows, i.e., output rows in reverse order.\n");
	GMT_message (GMT, "\t-L Output only segment headers and skip all data records.\n");
	GMT_message (GMT, "\t   Requires -m and ASCII input data [Output headers and data].\n");
	GMT_message (GMT, "\t-N Skip records where all fields == NaN [Write all records].\n");
	GMT_message (GMT, "\t-Q Only output segment number <seg> [All].\n");
	GMT_message (GMT, "\t-S Only output segments whose headers contain the pattern \"string\".\n");
	GMT_message (GMT, "\t   Use -S~\"string\" to output segment that DO NOT contain this pattern.\n");
	GMT_message (GMT, "\t   If your pattern begins with ~, escape it with \\~.\n");
	GMT_message (GMT, "\t   To match OGR aspatial values, use name=value.\n");
	GMT_message (GMT, "\t-T Prevent the writing of segment headers.\n");
	GMT_explain_options (GMT, "VaC0Dfghios:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtconvert_parse (struct GMTAPI_CTRL *C, struct GMTCONVERT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtconvert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
#ifdef GMT_COMPAT
	GMT_LONG gmt_parse_o_option (struct GMT_CTRL *GMT, char *arg);
#endif

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* pAste mode */
				Ctrl->A.active = TRUE;
				break;
			case 'D':	/* Write each segment to a separate output file */
				Ctrl->D.active = TRUE;
				Ctrl->D.name = strdup (opt->arg);
				break;
			case 'E':	/* Extract ends only */
				Ctrl->E.active = TRUE;
				switch (opt->arg[0]) {
					case 'f':		/* Get first point only */
						Ctrl->E.mode = 1; break;
					case 'l':		/* Get last point only */
						Ctrl->E.mode = 2; break;
					default:		/* Get first and last point only */
						Ctrl->E.mode = 3; break;
				}
				break;
#ifdef GMT_COMPAT
			case 'F':	/* Now obsolete, using -o instead */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -F is deprecated; use -o instead\n");
				gmt_parse_o_option (GMT, opt->arg);
				break;
#endif
			case 'I':	/* Invert order or rows */
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Only output segment headers */
				Ctrl->L.active = TRUE;
				break;
			case 'N':	/* Skip all-NaN records */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':	/* Only report for specified segment number */
				Ctrl->Q.active = TRUE;
				Ctrl->Q.seg = atoi (opt->arg);
				break;
			case 'S':	/* Segment header pattern search */
				Ctrl->S.active = TRUE;
				k = (opt->arg[0] == '\\' && strlen (opt->arg) > 3 && opt->arg[1] == '~') ? 1 : 0;	/* Special escape if pattern starts with ~ */
				if (opt->arg[0] == '~') Ctrl->S.inverse = TRUE;
				Ctrl->S.pattern = strdup (&opt->arg[k+Ctrl->S.inverse]);
				break;
			case 'T':	/* Do not write segment headers */
				Ctrl->T.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0, "Syntax error: Must specify number of columns in binary input data (-bi)\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && (Ctrl->L.active || Ctrl->S.active), "Syntax error: -L or -S requires ASCII input data\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->D.name && !strstr (Ctrl->D.name, "%"), "Syntax error: -D Output template must contain %%d\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && Ctrl->S.active, "Syntax error: Only one of -Q and -S can be used simultaneously\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define Return(code) {Free_gmtconvert_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_gmtconvert (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG last_row, n_rows, out_col, n_out_seg = 0, error = 0;
	GMT_LONG tbl, seg, col, row, n_cols_in, n_cols_out, out_seg = 0;
	GMT_LONG n_horizontal_tbls, n_vertical_tbls, tbl_ver, tbl_hor, use_tbl;
	GMT_LONG match = FALSE, warn = FALSE, ogr_match = FALSE, ogr_item;
	
	double *val = NULL;

	char *method[2] = {"concatenated", "pasted"}, *p = NULL;
	
	struct GMTCONVERT_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D[2] = {NULL, NULL};	/* Pointer to GMT multisegment table(s) in and out */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_gmtconvert_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_gmtconvert_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtconvert", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "aghios>" GMT_OPT("HMm"), options))) Return (error);
	Ctrl = (struct GMTCONVERT_CTRL *) New_gmtconvert_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtconvert_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtconvert main code ----------------------------*/

	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	
	/* Read in the input tables */
	
	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&D[GMT_IN])) Return ((error = GMT_DATA_READ_ERROR));
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->T.active) GMT->current.io.multi_segments[GMT_OUT] = FALSE;	/* Turn off segment headers on output */

	if (GMT->common.a.active && D[GMT_IN]->n_tables > 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "The -a option requires a single table only.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT->common.a.active && D[GMT_IN]->table[0]->ogr) {
		GMT_report (GMT, GMT_MSG_FATAL, "The -a option requires a single table without OGR metadata.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	
	/* Determine number of input and output columns for the selected options.
	 * For -A, require all tables to have the same number of segments and records. */

	for (tbl = n_cols_in = n_cols_out = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		if (Ctrl->A.active) {	/* All tables must be of the same vertical shape */
			if (tbl && D[GMT_IN]->table[tbl]->n_records  != D[GMT_IN]->table[tbl-1]->n_records)  error = TRUE;
			if (tbl && D[GMT_IN]->table[tbl]->n_segments != D[GMT_IN]->table[tbl-1]->n_segments) error = TRUE;
		}
		n_cols_in += D[GMT_IN]->table[tbl]->n_columns;				/* This is the case for -A */
		n_cols_out = MAX (n_cols_out, D[GMT_IN]->table[tbl]->n_columns);	/* The widest table encountered */
	}
	n_cols_out = (Ctrl->A.active) ? n_cols_in : n_cols_out;	/* Default or Reset since we did not have -A */
	
	if (error) {
		GMT_report (GMT, GMT_MSG_FATAL, "Parsing requires files with same number of records.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (n_cols_out == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Selection lead to no output columns.\n");
		Return (GMT_RUNTIME_ERROR);
		
	}
	if (warn) GMT_report (GMT, GMT_MSG_NORMAL, "Some requested columns are outside the range of some tables and will be skipped.\n");
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_cols_out))) Return (error);
	
	if (Ctrl->S.active && GMT->current.io.ogr == 1 && (p = strchr (Ctrl->S.pattern, '=')) != NULL) {	/* Want to search for an aspatial value */
		EXTERN_MSC GMT_LONG gmt_get_ogr_id (struct GMT_OGR *G, char *name);
		*p = 0;	/* Skip the = sign */
		if ((ogr_item = gmt_get_ogr_id (GMT->current.io.OGR, Ctrl->S.pattern)) != GMTAPI_NOTSET) {
			ogr_match = TRUE;
			p++;
			strcpy (Ctrl->S.pattern, p);	/* Move the value over to the start */
		}
	}
	
	/* We now know the exact number of segments and columns and an upper limit on total records.
	 * Allocate data set with a single table with those proportions. This copies headers as well */
	
	GMT_alloc_dataset (GMT, D[GMT_IN], &D[GMT_OUT], n_cols_out, 0, (Ctrl->A.active) ? GMT_ALLOC_HORIZONTAL : GMT_ALLOC_NORMAL);
	
	n_horizontal_tbls = (Ctrl->A.active) ? D[GMT_IN]->n_tables : 1;	/* Only with pasting do we go horizontally */
	n_vertical_tbls   = (Ctrl->A.active) ? 1 : D[GMT_IN]->n_tables;	/* Only for concatenation do we go vertically */
	val = GMT_memory (GMT, NULL, n_cols_in, double);
	
	for (tbl_ver = 0; tbl_ver < n_vertical_tbls; tbl_ver++) {	/* Number of tables to place vertically */
		for (seg = 0; seg < D[GMT_IN]->table[tbl_ver]->n_segments; seg++) {	/* For each segment in the tables */
			if (Ctrl->L.active) D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode = GMT_WRITE_HEADER;	/* Only write segment header */
			if (Ctrl->S.active) {		/* See if the combined segment header has text matching our search string */
				if (match && GMT_polygon_is_hole (D[GMT_IN]->table[tbl_ver]->segment[seg])) match = TRUE;	/* Extend a true match on a perimeter to the trailing holes */
				else if (ogr_match)	/* Compare to aspatial value */
					match = (D[GMT_IN]->table[tbl_ver]->segment[seg]->ogr && strstr (D[GMT_IN]->table[tbl_ver]->segment[seg]->ogr->value[ogr_item], Ctrl->S.pattern) != NULL);		/* TRUE if we matched */
				else match = (D[GMT_IN]->table[tbl_ver]->segment[seg]->header && strstr (D[GMT_IN]->table[tbl_ver]->segment[seg]->header, Ctrl->S.pattern) != NULL);		/* TRUE if we matched */
				if (Ctrl->S.inverse == match) D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			}
			if (Ctrl->Q.active && seg != Ctrl->Q.seg) D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode = GMT_WRITE_SKIP;	/* Mark segment to be skipped */
			if (D[GMT_OUT]->table[tbl_ver]->segment[seg]->mode) continue;	/* No point copying values given segment content will be skipped */
			n_out_seg++;	/* Number of segments that passed the test */
			last_row = D[GMT_IN]->table[tbl_ver]->segment[seg]->n_rows - 1;
			for (row = n_rows = 0; row <= last_row; row++) {	/* Go down all the rows */
				if (Ctrl->E.active) {	/* Only pass first or last or both of them, skipping all others */ 
					if (row > 0 && row < last_row) continue;		/* Always skip the middle of the segment */
					if (row == 0 && !(Ctrl->E.mode & 1)) continue;		/* First record, but we are to skip it */
					if (row == last_row && !(Ctrl->E.mode & 2)) continue;	/* Last record, but we are to skip it */
				}
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
			if (D[GMT_IN]->table[tbl_ver]->segment[seg]->label) D[GMT_OUT]->table[tbl_ver]->segment[seg]->label = strdup (D[GMT_IN]->table[tbl_ver]->segment[seg]->label);
			if (D[GMT_IN]->table[tbl_ver]->segment[seg]->header) D[GMT_OUT]->table[tbl_ver]->segment[seg]->header = strdup (D[GMT_IN]->table[tbl_ver]->segment[seg]->header);
			if (D[GMT_IN]->table[tbl_ver]->segment[seg]->ogr) GMT_duplicate_ogr_seg (GMT, D[GMT_OUT]->table[tbl_ver]->segment[seg], D[GMT_IN]->table[tbl_ver]->segment[seg]);
		}
		D[GMT_OUT]->table[tbl_ver]->id = tbl_ver;
	}
	GMT_free (GMT, val);

	if (Ctrl->I.active) {	/* Must reverse the row order within segments */
		GMT_LONG row1, row2;
		for (tbl = 0; tbl < D[GMT_OUT]->n_tables; tbl++) {	/* Number of output tables */
			for (seg = 0; seg < D[GMT_OUT]->table[tbl]->n_segments; seg++) {	/* For each segment in the tables */
				for (row1 = 0, row2 = D[GMT_OUT]->table[tbl]->segment[seg]->n_rows - 1; row1 < D[GMT_OUT]->table[tbl]->segment[seg]->n_rows/2; row1++, row2--) {
					for (col = 0; col < D[GMT_OUT]->table[tbl]->segment[seg]->n_columns; col++) {
						d_swap (D[GMT_OUT]->table[tbl]->segment[seg]->coord[col][row1], D[GMT_OUT]->table[tbl]->segment[seg]->coord[col][row2]);
					}
				}
			}
		}
	}
		
	/* Now ready for output */

	if (Ctrl->D.active) {	/* Must write individual segments to separate files so create the needed template */
		GMT_LONG n_formats = 0;
		if (!Ctrl->D.name) Ctrl->D.name = (GMT->common.b.active[GMT_OUT]) ? strdup ("gmtconvert_segment_%ld.bin") : strdup ("gmtconvert_segment_%ld.txt");
		for (col = 0; Ctrl->D.name[col]; col++) if (Ctrl->D.name[col] == '%') n_formats++;
		D[GMT_OUT]->io_mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENTS: GMT_WRITE_SEGMENTS;
		/* The io_mode tells the i/o function to split segments into files */
		if (Ctrl->Out.file) free ((void*)Ctrl->Out.file);
		Ctrl->Out.file = strdup (Ctrl->D.name);
	}
	else {	/* Just register output to stdout or given file via ->outfile */
		if (GMT->common.a.output) D[GMT_OUT]->io_mode = GMT_WRITE_OGR;
	}
	
	if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, D[GMT_OUT]->io_mode, (void **)&Ctrl->Out.file, (void *)D[GMT_OUT]))) Return (error);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "%ld tables %s, %ld records passed (input cols = %ld; output cols = %ld)\n", D[GMT_IN]->n_tables, method[Ctrl->A.active], D[GMT_OUT]->n_records, n_cols_in, n_cols_out);
	if (Ctrl->S.active) GMT_report (GMT, GMT_MSG_NORMAL, "Extracted %ld from a total of %ld segments\n", n_out_seg, D[GMT_OUT]->table[tbl_ver]->n_segments);

	Return (GMT_OK);
}
