/*
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
 * API functions to support the gmtconnect application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmtconnect will combine pieces of coastlines or similar segments
 * into a continuous line, polygon, or group of lines/polygons so that the jump
 * between segment endpoints exceeds a specified threshold.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtconnect"
#define THIS_MODULE_MODERN_NAME	"gmtconnect"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Connect individual lines whose end points match within tolerance"
#define THIS_MODULE_KEYS	"<D{,>D},CD),LD),QD)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioqs" GMT_OPT("HMm")

/* Control structure for gmtconnect */

struct GMTCONNECT_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct C {	/* -C[<file>] */
		bool active;
		char *file;	/* File for storing closed polygons */
	} C;
	struct D {	/* -D[<file>] */
		bool active;
		char *format;
	} D;
	struct L {	/* -L[<file>] */
		bool active;
		char *file;
	} L;
	struct Q {	/* -Q[<file>] */
		bool active;
		char *file;
	} Q;
	struct T {	/* -T[<cutoff[unit][+s<sdist>]] */
		bool active[2];
		int mode;
		double dist[2];
		char unit;
	} T;
};

#define SEG_I	0
#define SEG_J	1
#define END_A	0
#define END_B	1

#define CLOSED	0
#define OPEN	1

struct BUDDY {	/* Holds information on nearest segment to one end of a segment */
	uint64_t id;
	uint64_t orig_id;
	unsigned int end_order;
	double dist, next_dist;
};

struct LINK {	/* Information on linking segments together */
	uint64_t id;
	uint64_t orig_id;
	uint64_t pos;
	uint64_t n;
	uint64_t group;
	bool used;
	double x_end[2];
	double y_end[2];
	struct BUDDY buddy[2];
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTCONNECT_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct GMTCONNECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->T.unit = 'X';	/* Cartesian units unless override later */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTCONNECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.format);
	gmt_M_str_free (C->L.file);
	gmt_M_str_free (C->Q.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-C<closedfile>] [-D[<template>]] [-L[<linkfile>]] [-Q<listfile>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t-T[%s[+s<sdist>]] [%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_DIST_OPT, GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Write already-closed polygons to a separate <closedfile> [gmtconnect_closed.txt],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   with the remaining open segments written separately to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [All segments are written to a single output file; but see -D].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Write individual segments to separate files [Default writes one multisegment file to stdout].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append file name template which MUST contain a C-format specifier for an integer (e.g., %%d).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If the format also includes a %%c string BEFORE the %%d part we replace it with C(losed) or O(pen)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses gmtconnect_segment_%%d.txt].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Write link information (seg id, begin/end nearest seg id, end, and distance) to file [gmtconnect_link.txt].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Link output excludes duplicates and segments already forming a closed polygon.\n");
	GMT_Option (API, "V");
	gmt_dist_syntax (API->GMT, 'T', "Set cutoff distance to determine if a segment is closed.");
	GMT_Message (API, GMT_TIME_NONE, "\t   If two lines has endpoints closer than this cutoff they will be joined.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +s<sdist> which adds the requirement that the second closest\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   match must exceed <sdist> (must be in the same units as <cutoff>).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no arguments are given the we close all polygons regardless of the gaps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Used with -D to write names of files to a list.  Optionally give listfile name [gmtconnect_list.txt].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Embed %%c in the list name to write two separate lists: one for C(losed) and one for O(pen).\n");
	GMT_Option (API, "a,bi2,bo,d,e,f,g,h,i,o,q,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTCONNECT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtconnect and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int n = 0;
	char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;

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

			case 'C':	/* Separate closed from open segments  */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Write each segment to a separate output file */
				Ctrl->D.active = true;
				gmt_M_str_free (Ctrl->D.format);
				if (opt->arg[0]) Ctrl->D.format = strdup (opt->arg);
				break;
			case 'L':	/* Write link information to file */
				Ctrl->L.active = true;
				gmt_M_str_free (Ctrl->L.file);
				if (opt->arg[0]) Ctrl->L.file = strdup (opt->arg);
				break;
			case 'Q':	/* Write names of individual files to list(s) */
				Ctrl->Q.active = true;
				gmt_M_str_free (Ctrl->Q.file);
				if (opt->arg[0]) Ctrl->Q.file = strdup (opt->arg);
				break;
			case 'T':	/* Set threshold distance */
				Ctrl->T.active[0] = true;
				if (opt->arg[0]) {	/* Specified a distance */
					char *c = NULL;
					if ((c = strstr (opt->arg, "+s"))) {	/* Gave second distance via +s<dist> */
						c[0] = '\0';	/* Temporarily chop off modifier */
						Ctrl->T.mode = gmt_get_distance (GMT, A, &(Ctrl->T.dist[0]), &(Ctrl->T.unit));
						Ctrl->T.dist[1] = atof (&c[2]);
						Ctrl->T.active[1] = true;
						c[0] = '+';	/* Restore modifier */
					}
					else {	/* Only one distance but possibly old-style /dist */
						n = sscanf (opt->arg, "%[^/]/%s", A, B);
						Ctrl->T.mode = gmt_get_distance (GMT, A, &(Ctrl->T.dist[0]), &(Ctrl->T.unit));
						if (n == 2) {	/* Gave second distance via /<dist> */
							Ctrl->T.dist[1] = atof (B);
							Ctrl->T.active[1] = true;
						}
					}
				}
				else	/* Do distance, connect segments regardless of separation */
					Ctrl->T.dist[0] = DBL_MAX;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -1, "Syntax error -T: Unrecognized unit\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -2, "Syntax error -T: Unable to decode distance\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -3, "Syntax error -T: Distance is negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active[1] && Ctrl->T.dist[1] < 0.0, "Syntax error -T: Secondary distance is negative\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 2 columns\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Syntax error: Option -C cannot be used with -D!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Syntax error: Option -C cannot be used with -D!\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int found_a_near_segment (struct LINK *S, uint64_t id, int order, double cutoff, bool nn_check, double sdist) {
	/* Checks if OK to connect this segment to its nearest neighbor and returns true if OK */

	if (S[S[id].buddy[order].id].used) return (false);		/* Segment has been used already */
	if (S[id].buddy[order].dist > cutoff) return (false);		/* Exceeds minimum gap */
	if (!nn_check) return (true);					/* No other requirement specified, so done */
	if (S[id].buddy[order].next_dist > sdist) return (true);	/* Next nearest neighbour is far enough away */
	return (false);							/* Failed all tests */
}

GMT_LOCAL uint64_t Copy_This_Segment (struct GMT_DATASEGMENT *in, struct GMT_DATASEGMENT *out, uint64_t out_start, uint64_t in_start, uint64_t in_end) {
	uint64_t row_in, row_out, col;
	int64_t inc;
	bool done = false;

	/* We will copy the records from the in segment from rows in_start up to and including in_end.
	 * If in_start > in_end then we will end up reversing the order of the records.
	 * The records are copied to the out segment starting at output record out_start (initially 0).
	 * We return the next row number for output.
	 */
	inc = (in_start < in_end) ? +1 : -1;	/* Go forwards or backwards through the input */
	for (row_in = in_start, row_out = out_start; !done; row_in += inc, row_out++) {	/* Either loop from 1st to last or the other way */
		for (col = 0; col < in->n_columns; col++) out->data[col][row_out] = in->data[col][row_in];	/* Copy this row */
		done = (row_in == in_end);	/* Stop when finishing the end row */
	}
	return (row_out);	/* The next output record number */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, segment); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtconnect (void *V_API, int mode, void *args) {
	int error = 0;

	bool save_type = false, first, wrap_up = false, done, closed, *skip = NULL;

	unsigned int nearest_end[2][2], j, n_qfiles = 0, end_order, smode = GMT_NO_STRINGS;
	unsigned int io_mode = GMT_WRITE_SET, q_mode = GMT_WRITE_SET, d_mode = 0, ii, end, first_end_order;

	size_t n_seg_alloc[2] = {0, 0}, n_alloc_pts, b_alloc = GMT_BUFSIZ, b_len = 0, len;

	uint64_t tbl, n_columns, n, k, n_rows, seg, np, ns, n_open, out_seg, out_p, id, id2;
	uint64_t n_islands = 0, n_trouble = 0, n_closed = 0, chain = 0, match = 0, L, n_steps_pass_1;
	uint64_t dim_tscr[4] = {1, 1, 0, 0}, start_id, iseg, jseg, n_seg_length, G, n_steps_pass_2;

	double dd[2][2], p_last_x, p_last_y, p_first_x, p_first_y, distance, closed_dist = 0.0;

	char *buffer = NULL, msg[GMT_BUFSIZ] = {""}, text[GMT_LEN64] = {""}, *BE = "BE", *ofile = NULL;

	struct LINK *segment = NULL;
	struct GMT_DATASET *D[2] = {NULL, NULL}, *C = NULL;
	struct GMT_DATASET *Q = NULL;
	struct GMT_DATASEGMENT **T[2] = {NULL, NULL}, *S = NULL;
	struct GMT_DATASEGMENT *QT[2] = {NULL, NULL};
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMTCONNECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));		/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);		/* Allocate and initialize defaults in a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtconnect main code ----------------------------*/

	/* Now we are ready to take on some input values */

	buffer = gmt_M_memory (GMT, NULL, b_alloc, char);	/* Initial buffer size for header */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");

	if (Ctrl->D.active) {	/* We want output to go to individual files for each segment [Default writes to stdout] */
		io_mode = GMT_WRITE_SEGMENT;	/* This means "write segments to separate files" */
		if (!Ctrl->D.format) Ctrl->D.format = strdup ("gmtconnect_segment_%d.txt");	/* Default naming convention for segments */
		if (strstr (Ctrl->D.format, "%c")) save_type = true;	/* Also add C (closed) or O (open) to the filenames */
		if (Ctrl->Q.active) {	/* We also want to build a list file(s) */
			if (!Ctrl->Q.file) Ctrl->Q.file = strdup ("gmtconnect_list.txt");	/* Default -Q list name if none was given */
			dim_tscr[GMT_TBL] = n_qfiles = (strstr (Ctrl->Q.file, "%c")) ? 2 : 1;	/* Build one or two tables (closed and open) */
			/* Allocate one or two tables with 1 segment each */
			if ((Q = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_TEXT, GMT_WITH_STRINGS, dim_tscr, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a text set for segment lists\n");
				gmt_M_free (GMT, buffer);
				Return (API->error);
			}
			if (n_qfiles == 2) {	/* We want to build two lists (closed and open) */
				static char status[2] = {'C', 'O'};
				q_mode = GMT_WRITE_TABLE;	/* This means write tables to separate files */
				dim_tscr[GMT_TBL] = 1;	/* Reset from 2 to 1 */
				/* Create the two filenames with C and O identifiers */
				for (k = CLOSED; k <= OPEN; k++) {
					TH = gmt_get_DT_hidden (Q->table[k]);
					sprintf (buffer, Ctrl->Q.file, status[k]);
					TH->file[GMT_OUT] = strdup (buffer);
					QT[k] = Q->table[k]->segment[0];	/* Shorthand for segments in closed polygon list */
					SH = gmt_get_DS_hidden (QT[k]);
					SH->n_alloc = GMT_CHUNK;		/* Initial allocation sizes */
					QT[k]->text = gmt_M_memory (GMT, NULL, SH->n_alloc, char *);	/* Allocate n_alloc records for now */
				}
			}
			else {	/* A single list will do */
				q_mode = GMT_WRITE_SET;	/* This means write tables to a single file */
				TH = gmt_get_DT_hidden (Q->table[0]);
				TH->file[GMT_OUT] = strdup (Ctrl->Q.file);
				QT[OPEN] = QT[CLOSED] = Q->table[0]->segment[0];	/* The two QT pointers point to the same table, which is 0 [OPEN] */
				SH = gmt_get_DS_hidden (QT[OPEN]);
				SH->n_alloc = GMT_CHUNK;				/* Initial allocation sizes */
				QT[OPEN]->text = gmt_M_memory (GMT, NULL, SH->n_alloc, char *);		/* Allocate n_alloc records for now */
			}
		}
	}

	gmt_init_distaz (GMT, Ctrl->T.unit, Ctrl->T.mode, GMT_MAP_DIST);	/* Initialize distance-computing machinery with proper unit */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input assuming lines */
		gmt_M_free (GMT, buffer);
		Return (API->error);
	}
	/* Read in all the input segments into one virtual dataset */
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		gmt_M_free (GMT, buffer);
		Return (API->error);
	}
	if (D[GMT_IN]->n_columns < 2) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)D[GMT_IN]->n_columns);
		gmt_M_free (GMT, buffer);
		Return (GMT_DIM_TOO_SMALL);
	}

	if (D[GMT_IN]->n_records == 0) {	/* Empty files, nothing to do */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data records found.\n");
		gmt_M_free (GMT, buffer);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Surely don't need any more segment space than the number of input segments */
	segment = gmt_M_memory (GMT, NULL, D[GMT_IN]->n_segments, struct LINK);
	id = ns = out_seg = 0;
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Check for already closed polygons\n");

	/* Closed polygons are already finished - just identify, write out, and move on */

	/* Allocate D[GMT_OUT] and possibly C, both with nrows = ncolumns so just segment structs are allocated */

	n_columns = dim_tscr[GMT_COL] = D[GMT_IN]->n_columns;	/* Set the required columns for output to match that of input file */

	dim_tscr[GMT_SEG] = 0;	/* Allocate no segments for now - we will do this as needed */
	if ((D[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET, D[GMT_IN]->geometry, 0, dim_tscr, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for output segments\n");
		gmt_M_free (GMT, buffer);
		Return (API->error);
	}
	n_seg_alloc[OPEN] = D[GMT_IN]->n_segments;	/* Cannot end up with more open segments than given on input so this is an upper limit  */
	T[OPEN] = gmt_M_memory (GMT, NULL, n_seg_alloc[OPEN], struct GMT_DATASEGMENT *);

	if (Ctrl->C.active) {	/* Wish to return already-closed polygons via a separate file */
		if (Ctrl->C.file == NULL)	/* No such filename given, select default name */
			Ctrl->C.file = strdup ("gmtconnect_closed.txt");
		if ((C = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim_tscr, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for closed segments\n");
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
		n_seg_alloc[CLOSED] = n_seg_alloc[OPEN];	/* Cannot end up with more closed segments than given on input so this is an upper limit  */
		T[CLOSED] = gmt_M_memory (GMT, NULL, n_seg_alloc[CLOSED], struct GMT_DATASEGMENT *);
	}
	else
		T[CLOSED] = T[OPEN];	/* Everything returned via the same dataset */

	/* Start processing all the input segments in D[GMT_IN] */

	ns = 0;			/* So the ID of the very first segment will be 0 */
	n_open = n_closed = 0;	/* Numbers of open segments and closed polygons found */
	closed_dist = (Ctrl->C.active) ? Ctrl->T.dist[0] : 0.0;

	for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {	/* For each input data table */
		for (seg = 0; seg < D[GMT_IN]->table[tbl]->n_segments; seg++) {	/* For each input segment */
			np = D[GMT_IN]->table[tbl]->segment[seg]->n_rows;	/* Short-hand to avoid the full expression below */
			S = D[GMT_IN]->table[tbl]->segment[seg];		/* Short hand to current in segment */
			/* Get distance between first and last point in this segment */
			distance = gmt_distance (GMT, S->data[GMT_X][0], S->data[GMT_Y][0], S->data[GMT_X][np-1], S->data[GMT_Y][np-1]);
			if (np > 2 && distance <= closed_dist) {	/* Already a closed segment, just write out and forget in the rest of the program */
				/* Allocate space for this segment */
				smode = (S->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
				n_rows = (Ctrl->C.active && distance > 0.0) ? np + 1 : np;	/* Add one extra row if closure is not exact */
				T[CLOSED][out_seg] = GMT_Alloc_Segment (GMT->parent, smode, n_rows, n_columns, S->header, NULL);
				if (Ctrl->D.active) {	/* Write closed polygons to individual files */
					SH = gmt_get_DS_hidden (T[CLOSED][out_seg]);
					(save_type) ? sprintf (buffer, Ctrl->D.format, 'C', out_seg) : sprintf (buffer, Ctrl->D.format, out_seg);
					SH->file[GMT_OUT] = strdup (buffer);	/* Assign the name of this segment-file */
					if (Ctrl->Q.active) {	/* Also maintain list of such files */
						SH = gmt_get_DS_hidden (QT[CLOSED]);
						QT[CLOSED]->text[QT[CLOSED]->n_rows++] = strdup (buffer);
						if (QT[CLOSED]->n_rows == SH->n_alloc) QT[CLOSED]->text = gmt_M_memory (GMT, QT[CLOSED]->text, (SH->n_alloc <<= 1), char *);
					}
				}
				out_p = Copy_This_Segment (S, T[CLOSED][out_seg], 0, 0, np-1);	/* Duplicate input to output */
				if (Ctrl->C.active && distance > 0.0) (void) Copy_This_Segment (S, T[CLOSED][out_seg], out_p, 0, 0);	/* Close polygon explicitly */
				n_islands++;	/* Number of originally closed polygons found in input */
				out_seg++;	/* Number of closed segments placed in T[CLOSED] so far */
				n_closed++;	/* Number of closed polygons (which will grow when we connect below) */
			}
			else if (Ctrl->C.active) {	/* NOT closed: Copy this open segment to the separate output dataset */
				/* Allocate space for this segment */
				smode = (S->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
				T[OPEN][n_open] = GMT_Alloc_Segment (GMT->parent, smode, np, n_columns, S->header, NULL);
				(void) Copy_This_Segment (S, T[OPEN][n_open], 0, 0, np-1);		/* Duplicate input to output */
				n_open++;	/* Number of open segments placed in T[OPEN] so far */
			}
			else { /* No -C was given: Here we have a segment that is not closed.  Store refs to D[GMT_IN]->table and copy end points; more work on linking takes place below */
				/* Store information about this segment (end points, ID, etc) in the array "segment" of segment structures */
				if (np == 1) GMT_Report (API, GMT_MSG_VERBOSE, "Segment %" PRIu64 " only consists of a single point.  May require additional connections.\n", id);
				segment[id].id = id;		/* Running number ID starting at 0 for open segments only */
				segment[id].orig_id = ns;	/* ns is input segment number */
				segment[id].group = tbl;	/* Remember which input table this segment came from */
				segment[id].pos = seg;		/* Remember which input segment in this table it came from */
				segment[id].n = np;		/* Number of points in this segment */
				/* Record start and end coordinates for this segment and initialize buddy structure to having no nearest neighbor segment yet */
				segment[id].x_end[END_A] = S->data[GMT_X][0];
				segment[id].y_end[END_A] = S->data[GMT_Y][0];
				segment[id].x_end[END_B] = S->data[GMT_X][np-1];
				segment[id].y_end[END_B] = S->data[GMT_Y][np-1];
				segment[id].buddy[END_A].dist = segment[id].buddy[END_B].dist = DBL_MAX;
				segment[id].buddy[END_A].next_dist = segment[id].buddy[END_B].next_dist = DBL_MAX;
				id++;	/* Increment open segment ID number */
			}
			ns++;	/* Increment running segment ID */
		}
	}

	/* Here we have gone through all input segment and separated open from closed */

	if (Ctrl->C.active) {	/* Finalize allocation for closed segments now that we know how many there were */
		C->table[0]->segment = gmt_M_memory (GMT, T[CLOSED], n_closed, struct GMT_DATASEGMENT *);
		C->n_segments = C->table[0]->n_segments = n_closed;
		/* With -C we only separate closed from open and then we are done */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Separated %" PRIu64 " closed and %" PRIu64 " open segments\n", n_closed, n_open);
		wrap_up = true;	/* Means to quit once we have written those results to file - no nesting takes place */
	}
	else if (id == 0) {	/* All segments were already closed polygons */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "All segments already form closed polygons - no new segment file created\n");
		wrap_up = true;	/* Means to quit once we have written those results to file - no nesting possible */
	}

	gmt_set_segmentheader (GMT, GMT_OUT, n_open > 1 || n_closed > 1);	/* Turn on segment headers on output if we have more than one segment */
	if (wrap_up) {	/* Write out results and exit */
		D[GMT_OUT]->table[0]->segment = gmt_M_memory (GMT, T[OPEN], n_open, struct GMT_DATASEGMENT *);	/* Finalize allocation */
		D[GMT_OUT]->n_segments = D[GMT_OUT]->table[0]->n_segments = n_open;
		if (Ctrl->C.active) { /* Write n_open segments to D[OUT] and n_closed to C; here we do C */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->C.file, C) != GMT_NOERROR) {
				gmt_M_free (GMT, buffer);
				Return (API->error);
			}
		}
		/* Write open segments to the outfile (probably stdout) */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_WRITE_SET, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_NOERROR) {
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
		if (Ctrl->Q.active) {	/* Also finalize link file and write it out to 1 or 2 files depending on q_mode */
			Q->table[CLOSED]->segment[0]->text = gmt_M_memory (GMT, QT[CLOSED]->text, QT[CLOSED]->n_rows, char *);
			if (n_qfiles == 2) Q->table[OPEN]->segment[0]->text = gmt_M_memory (GMT, QT[OPEN]->text, QT[OPEN]->n_rows, char *);
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, q_mode, NULL, Ctrl->Q.file, Q) != GMT_NOERROR) {
				gmt_M_free (GMT, buffer);
				Return (API->error);
			}
		}
		gmt_M_free (GMT, buffer);
		Return (GMT_NOERROR);	/* That is it, we are done here */
	}

	/* Below here, -C was NOT given since those cases have already been dealt with.
	 * Also T[OPEN] = T[CLOSED] and we already have found out_seg segments.
	 * Here we need to do the connect work.  We already have n_closed polygons in D[GMT_OUT] at this point */

	ns = id;	/* Number of open segments remaining, adjust necessary memory */
	if (ns < D[GMT_IN]->n_segments) segment = gmt_M_memory (GMT, segment, ns, struct LINK);
	skip = gmt_M_memory (GMT, NULL, ns, bool);	/* Used when looking for duplicate segments */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " closed polygons\n", n_islands);

	/* The connect algorithm will be confused if there are identical duplicates of segments - thus we check first */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Check for duplicate lines\n");
	for (iseg = 0; iseg < ns; iseg++) {	/* Loop over remaining open lines */
		if (skip[iseg]) continue;	/* Skip lines that has been determined to be a duplicate line */
		for (jseg = iseg + 1; jseg < ns; jseg++) {	/* Loop over all other open lines */
			if (skip[jseg]) continue;	/* Skip line that has been determined to be a duplicate line */
			/* See if any pair of endpoints matches (duplicate line may be reversed so check both ways) */
			if ((doubleAlmostEqualZero (segment[iseg].x_end[END_A], segment[jseg].x_end[END_A]) && doubleAlmostEqualZero (segment[iseg].y_end[END_A], segment[jseg].y_end[END_A])) ||
			    (doubleAlmostEqualZero (segment[iseg].x_end[END_A], segment[jseg].x_end[END_B]) && doubleAlmostEqualZero (segment[iseg].y_end[END_A], segment[jseg].y_end[END_B])) ||
			    (doubleAlmostEqualZero (segment[iseg].x_end[END_B], segment[jseg].x_end[END_A]) && doubleAlmostEqualZero (segment[iseg].y_end[END_B], segment[jseg].y_end[END_A])) ||
			    (doubleAlmostEqualZero (segment[iseg].x_end[END_B], segment[jseg].x_end[END_B]) && doubleAlmostEqualZero (segment[iseg].y_end[END_B], segment[jseg].y_end[END_B]))) {	/* Yes, identical end points */
			    	if (segment[iseg].n == segment[jseg].n) {	/* and same number of points */
					for (k = match = 0; k < segment[iseg].n && k == match; k++) {	/* Compute number of duplicate points */
						match += (doubleAlmostEqualZero (D[GMT_IN]->table[segment[iseg].group]->segment[segment[iseg].pos]->data[GMT_X][k], D[GMT_IN]->table[segment[jseg].group]->segment[segment[jseg].pos]->data[GMT_X][k]) &&
						          doubleAlmostEqualZero (D[GMT_IN]->table[segment[iseg].group]->segment[segment[iseg].pos]->data[GMT_Y][k], D[GMT_IN]->table[segment[jseg].group]->segment[segment[jseg].pos]->data[GMT_Y][k]));
					}
					if (match == segment[iseg].n) {	/* An exact match */
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Line segments %" PRIu64 " and %" PRIu64 "are duplicates - Line segment %" PRIu64 " will be ignored\n", iseg, jseg, jseg);
						skip[jseg] = true;	/* Flag this line for skipping */
					}
				}
			}
		}
	}

	/* Eliminate the duplicate segments from further consideration by shuffling the others up front */

	for (iseg = jseg = 0; iseg < ns; iseg++) {
		if (skip[iseg]) continue;
		if (iseg > jseg) segment[jseg] = segment[iseg];
		jseg++;
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " duplicate segments\n", ns - jseg);
	if (jseg < ns) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " duplicate segment removed\n", ns - jseg);
	ns = jseg;	/* The new number of open segments after duplicates have been removed */
	gmt_M_free (GMT, skip);	/* Done with this array */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Try to connect %" PRIu64 " open segments\n", ns);

	if (Ctrl->T.unit == 'X')
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate and rank end point separations [cutoff = %g sdist = %g]\n", Ctrl->T.dist[0], Ctrl->T.dist[1]);
	else
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Calculate and rank end point separations [cutoff = %g%c sdist = %g%c]\n", Ctrl->T.dist[0], Ctrl->T.unit, Ctrl->T.dist[1], Ctrl->T.unit);

	/* We determine the distance from each segment's two endpoints to the two endpoints on every other
	 * segment; this yields four distances per segment.  We then assign the nearest endpoint to each end
	 * of a segment to the buddy structure which keeps the id of the nearest segment found so far.
	 */

	for (iseg = 0; iseg < ns; iseg++) {	/* Loop over all open line segments and their two endpoints... */
		for (jseg = iseg; jseg < ns; jseg++) {	/* ...and compare distances to all other open line segments' two endpoints  */
			/* nearest_end indicates which end is closest to this end */
			if (iseg == jseg) {	/* Store offset between the endpoints of a single segment (would be 0 if closed but those polygons have already been dealt with) */
				dd[SEG_I][END_A] = dd[SEG_J][END_B] = DBL_MAX;	/* Flag as single line segment so two ends are not used */
				dd[SEG_I][END_B] = dd[SEG_J][END_A] = (segment[iseg].n == 1) ? DBL_MAX : gmt_distance (GMT, segment[iseg].x_end[END_A], segment[iseg].y_end[END_A], segment[iseg].x_end[END_B], segment[iseg].y_end[END_B]);
    				nearest_end[SEG_I][END_A] = nearest_end[SEG_J][END_A] = END_B;	/* Duplicate the nearest ID info since it is a single line segment compared to itself */
    				nearest_end[SEG_J][END_B] = nearest_end[SEG_I][END_B] = END_A;
			}
			else {	/* Store the distances between the 4 possible end-to-end configurations */
				dd[SEG_I][END_A] = gmt_distance (GMT, segment[iseg].x_end[END_A], segment[iseg].y_end[END_A], segment[jseg].x_end[END_A], segment[jseg].y_end[END_A]);
				dd[SEG_I][END_B] = gmt_distance (GMT, segment[iseg].x_end[END_A], segment[iseg].y_end[END_A], segment[jseg].x_end[END_B], segment[jseg].y_end[END_B]);
				dd[SEG_J][END_A] = gmt_distance (GMT, segment[iseg].x_end[END_B], segment[iseg].y_end[END_B], segment[jseg].x_end[END_A], segment[jseg].y_end[END_A]);
				dd[SEG_J][END_B] = gmt_distance (GMT, segment[iseg].x_end[END_B], segment[iseg].y_end[END_B], segment[jseg].x_end[END_B], segment[jseg].y_end[END_B]);
				/* Determine which end is nearest */
    				for (end = 0; end < 2; end++) nearest_end[SEG_I][end] = (dd[end][END_A] <= dd[end][END_B]) ? END_A : END_B;
    				for (end = 0; end < 2; end++) nearest_end[SEG_J][end] = (dd[END_A][end] <= dd[END_B][end]) ? END_A : END_B;
    			}
			sprintf (msg, "Pair %d - %d, dd[i][j] = %g, %g, %g, %g\n", (int)iseg, (int)jseg, dd[0][0], dd[0][1], dd[1][0], dd[1][1]);
			GMT_Report (API, GMT_MSG_DEBUG, msg);
 			/* Update list of closest matches for both ends */
    			for (ii = 0; ii < 2; ii++) {	/* For each end of the segment */
    				end = nearest_end[SEG_I][ii];	/* The end of segment jseg that was closest to segment iseg's end ii */
    				if (dd[ii][end] < segment[iseg].buddy[ii].dist) {	/* This distance is shorter than the previous shortest distance, so time to update */
					segment[iseg].buddy[ii].next_dist = segment[iseg].buddy[ii].dist;	/* Previous closest distance becomes the next-nearest distance */
					segment[iseg].buddy[ii].orig_id = segment[jseg].orig_id;
					segment[iseg].buddy[ii].id = jseg;
					segment[iseg].buddy[ii].dist = dd[ii][end];
					segment[iseg].buddy[ii].end_order = end;
    				}
				else if (dd[ii][end] < segment[iseg].buddy[ii].next_dist) segment[iseg].buddy[ii].next_dist = dd[ii][end];
    				end = nearest_end[SEG_J][ii];	/* The end of segment iseg that was closest to segment jseg's end ii */
    				if (dd[end][ii] < segment[jseg].buddy[ii].dist) {	/* This distance is shorter than the previous shortest distance, so time to update */
 					segment[jseg].buddy[ii].next_dist = segment[jseg].buddy[ii].dist;	/* Previous closest distance becomes the next-nearest distance */
					segment[jseg].buddy[ii].orig_id = segment[iseg].orig_id;
 					segment[jseg].buddy[ii].id = iseg;
					segment[jseg].buddy[ii].dist = dd[end][ii];
					segment[jseg].buddy[ii].end_order = end;
    				}
				else if (dd[end][ii] < segment[jseg].buddy[ii].next_dist) segment[jseg].buddy[ii].next_dist = dd[end][ii];
    			}
		}
		sprintf (msg, "Seg %d dist[0], next_dist[0], dist[1], next_dist[1] = %g, %g, %g, %g\n", (int)iseg, segment[iseg].buddy[0].dist, segment[iseg].buddy[0].next_dist, segment[iseg].buddy[1].dist, segment[iseg].buddy[1].next_dist);
		GMT_Report (API, GMT_MSG_DEBUG, msg);
	}

	/* Done determining distances from endpoints to nearest endpoints for all line segments */

	if (Ctrl->L.active) {	/* We can now write out the link information we found */
		struct GMT_DATASET *LNK = NULL;
		char name[GMT_BUFSIZ] = {""}, name0[GMT_BUFSIZ] = {""}, name1[GMT_BUFSIZ] = {""}, fmt[GMT_BUFSIZ] = {""};
		char *pp = NULL, *s = GMT->current.setting.io_col_separator;

		if (!Ctrl->L.file) Ctrl->L.file = strdup ("gmtconnect_link.txt");	/* Use default output filename since none was provided */
		dim_tscr[GMT_TBL] = 1;	dim_tscr[GMT_SEG] = 1;	dim_tscr[GMT_ROW] = ns;	dim_tscr[GMT_COL] = 0;	/* Dimensions of single output table with single segment of ns rows */
		if ((LNK = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_WITH_STRINGS, dim_tscr, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a text set for link lists\n");
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
		/* Set up a format statement for the link output */
		sprintf (fmt, "%%" PRIu64 "%s%%s%s%%s%s%%c%s%s%s%s%s%%s%s%%c%s%s%s%s", s, s, s, s, GMT->current.setting.format_float_out, s, GMT->current.setting.format_float_out, \
			s, s, s, GMT->current.setting.format_float_out, s, GMT->current.setting.format_float_out);

		/* Create a single table header */
		GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on table headers on output */
		sprintf (buffer, "#id%ssegid%s@begin%sb_pt%sb_dist%sb_next%s@end%se_pt%se_dist%se_next", s, s, s, s, s, s, s, s, s);
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, buffer, LNK)) Return (API->error);
		for (iseg = 0; iseg < ns; iseg++) {	/* Loop over open segments */
			G = segment[iseg].group;	L = segment[iseg].pos;	/* Short hand notation */
			/* If -L is in the segment header, extract the ID from that, else use the input running number as ID */
			if (D[GMT_IN]->table[G]->segment[L]->header && (pp = strstr (D[GMT_IN]->table[G]->segment[L]->header, "-L")) != NULL) {
				strncpy (name, &pp[2], GMT_BUFSIZ);
				for (j = 0; name[j]; j++) if (name[j] == ' ') name[j] = '\0';		/* Just truncate after 1st word */
			} else sprintf (name, "%" PRIu64, segment[iseg].orig_id);
			G = segment[segment[iseg].buddy[0].id].group;	L = segment[segment[iseg].buddy[0].id].pos;
			/* If -L is in the segment header, extract the ID from that, else use the input running number as ID */
			if (D[GMT_IN]->table[G]->segment[L]->header && (pp = strstr (D[GMT_IN]->table[G]->segment[L]->header, "-L")) != NULL) {
				strncpy (name0, &pp[2], GMT_BUFSIZ);
				for (j = 0; name0[j]; j++) if (name0[j] == ' ') name0[j] = '\0';	/* Just truncate after 1st word */
			} else sprintf (name0, "%" PRIu64, segment[iseg].buddy[0].orig_id);
			G = segment[segment[iseg].buddy[1].id].group;	L = segment[segment[iseg].buddy[1].id].pos;
			/* If -L is in the segment header, extract the ID from that, else use the input running number as ID */
			if (D[GMT_IN]->table[G]->segment[L]->header && (pp = strstr (D[GMT_IN]->table[G]->segment[L]->header, "-L")) != NULL) {
				strncpy (name1, &pp[2], GMT_BUFSIZ);
				for (j = 0; name1[j]; j++) if (name1[j] == ' ') name1[j] = '\0';	/* Just truncate after 1st word */
			} else sprintf (name1, "%" PRIu64, segment[iseg].buddy[1].orig_id);
			/* OK, compose the output record using the format and information provided */
			sprintf (buffer, fmt, segment[iseg].orig_id, name, name0, BE[segment[iseg].buddy[0].end_order], segment[iseg].buddy[0].dist, segment[iseg].buddy[0].next_dist, name1, \
				BE[segment[iseg].buddy[1].end_order], segment[iseg].buddy[1].dist, segment[iseg].buddy[1].next_dist);
			LNK->table[0]->segment[0]->text[iseg] = strdup (buffer);
		}
		LNK->table[0]->n_records = LNK->table[0]->segment[0]->n_rows = ns;	/* Number of records for this file */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_WRITE_SET, NULL, Ctrl->L.file, LNK) != GMT_NOERROR) {
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &LNK) != GMT_NOERROR) {
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
	}

	start_id = n_closed = n_open = 0;	/* Initialize counters for the connection of line segments into closed polygons */
	done = false;

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Assemble new line segments and polygons\n");

	/* We start at the very first open line segment (start_id = 0) and trace through its nearest line segments
	 * until closed or running out of new line segments */
	while (!done) {

		/* Find the 'beginning' of the chain that this segment belongs to by tracing the connections
		 * until we either reappear at the starting point (a closed loop) or we reach an end (i.e.,
		 * the nearest next endpoint is beyond the separation threshold. */

		id = start_id;	/* This is the first line segment in a new chain */
		sprintf (buffer, "%" PRIu64, segment[id].orig_id);
		b_len = strlen (buffer);
		/* Select the endpoint that has the closest buddy */
		end_order = first_end_order = (segment[id].buddy[0].dist < segment[id].buddy[1].dist) ? 0 : 1;		/* Exceeds minimum gap */
		
		closed = false;
		n_steps_pass_1 = 0;		/* Nothing appended yet to this single line segment */
		n_alloc_pts = segment[id].n;	/* Number of points needed so far is just those from this first (start_id) segment */
		while (!done && found_a_near_segment (segment, id, end_order, Ctrl->T.dist[0], Ctrl->T.active[1], Ctrl->T.dist[1])) {	/* found_a_near_segment returns true if nearest segment is close enough */
			id2 = segment[id].buddy[end_order].id;	/* ID of nearest segment at end 0 */
			snprintf (text, GMT_LEN64, " -> %" PRIu64, segment[id2].orig_id);
			b_len += strlen (text);
			if (b_len >= b_alloc) {	/* Resize buffer for header */
				b_alloc <<= 1;
				buffer = gmt_M_memory (GMT, buffer, b_alloc, char);
			}
			strcat (buffer, text);	/* Append this connection */
			if (id2 == start_id)	/* Ended up at the starting polygon so it is now a closed polygon */
				done = closed = true;
			if (id2 == id || n_steps_pass_1 > ns) {	/* Not good... [NOT SURE WHAT THIS MEANS BUT SEEMS LIKE A CRAZY SAFETY VALVE AND SHOULD NEVER HAPPEN] */
				done = true;
				n_trouble++;
			}
			else {	/* Good. Trace the connection to the next segment */
				/* Having hooked line segment to current end_order, we must flip to the other end for the next connection */
#if 0
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Connecting segment %" PRIu64 " to segment %" PRIu64 "\n", id, id2);
#endif
				end_order = !segment[id].buddy[end_order].end_order;
				id = id2;	/* Update what is the current segment */
				n_alloc_pts += segment[id].n;		/* Update length of combined line segment so far */
			}
			n_steps_pass_1++;	/* Number of segments in this growing chain */
		}

		if (!closed) {	/* Now search backwards from the stating line to see what should be hooked on in that direction */
			id = start_id;	/* This is the first line segment in a new chain */
			end_order = (first_end_order + 1) % 2;	/* Go the other way */
			while (!done && found_a_near_segment (segment, id, end_order, Ctrl->T.dist[0], Ctrl->T.active[1], Ctrl->T.dist[1])) {	/* found_a_near_segment returns true if nearest segment is close enough */
				id2 = segment[id].buddy[end_order].id;	/* ID of nearest segment at end 0 */
				snprintf (text, GMT_LEN64, "%" PRIu64 " <- ", segment[id2].orig_id);
				len = strlen (text);
				if ((b_len + len) >= b_alloc) {	/* Resize buffer for header */
					b_alloc <<= 1;
					buffer = gmt_M_memory (GMT, buffer, b_alloc, char);
				}
				/* Shift buffer to the right first by len characters */
				memmove (&buffer[len], buffer, b_len);
				b_len += len;
				memcpy (buffer, text, len);	/* Prepend to buffer */
				if (id2 == start_id)	/* Ended up at the starting polygon so it is now a closed polygon */
					done = closed = true;
				if (id2 == id || n_steps_pass_1 > ns) {	/* Not good... [NOT SURE WHAT THIS MEANS BUT SEEMS LIKE A CRAZY SAFETY VALVE AND SHOULD NEVER HAPPEN] */
					done = true;
					n_trouble++;
				}
				else {	/* Good. Trace the connection to the next segment */
					/* Having hooked line segment to current end_order, we must flip to the other end for the next connection */
#if 0
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Connecting segment %" PRIu64 " to segment %" PRIu64 "\n", id, id2);
#endif
					end_order = !segment[id].buddy[end_order].end_order;
					id = id2;	/* Update what is the current segment */
					n_alloc_pts += segment[id].n;		/* Update length of combined line segment so far */
				}
				n_steps_pass_1++;	/* Number of segments in this growing chain */
			}
		}
		/* Here we either have closed a polygon or still have a (possibly much longer) open line segment. */
		/* This id should be the beginning of a segment.  Now trace forward and dump out the chain */

		/* Get a new segment structure with enough rows */
		T[CLOSED][out_seg] = GMT_Alloc_Segment (GMT->parent, smode, n_alloc_pts, n_columns, NULL, NULL);

		if (n_steps_pass_1 == 1) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Connect %s -> none [1 segment]\n", buffer);
			sprintf (buffer, "Single open segment not enlarged by connection");
		}
		else {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Connect %s [%" PRIu64 " segments]\n", buffer, n_steps_pass_1);
			sprintf (buffer, "Composite segment made from %" PRIu64 " line segments", n_steps_pass_1);
		}
		T[OPEN][out_seg]->header = strdup (buffer);

		start_id = id;	/* Having reached the end of a chain, we let the last line segment be our starting line segment for the output */
		/* Note this start line segment has current end_order 0 or 1; if 1 we must first reverse below */

		gmt_M_memset (GMT->current.io.segment_header, GMT_BUFSIZ, char);	/* Blank the current segment header */
		if (Ctrl->D.active) {	/* Prepare and set segment output file name */
			SH = gmt_get_DS_hidden (T[OPEN][out_seg]);
			d_mode = OPEN;
			(save_type) ? sprintf (buffer, Ctrl->D.format, 'O', out_seg) : sprintf (buffer, Ctrl->D.format, out_seg);
			SH->file[GMT_OUT] = strdup (buffer);
		}

		/* Initialize values of previous end point coordinates */
		p_first_x = p_last_x = DBL_MAX;
		p_first_y = p_last_y = DBL_MAX;
		n_steps_pass_2 = out_p = n_seg_length = 0;
		first = true;
		do {
			G = segment[id].group;	/* This is which table this line segment came from */
			L = segment[id].pos;	/* This is the segment number in that table */
			np = segment[id].n;	/* Length of this line segment */
			S = D[GMT_IN]->table[G]->segment[L];	/* Short hand for the current segment */
			if (end_order == 0) {	/* Already in the right order */
				if (doubleAlmostEqualZero (S->data[GMT_X][0], p_last_x) && doubleAlmostEqualZero (S->data[GMT_Y][0], p_last_y)) {	/* Skip duplicate anchor point */
					j = 1;		/* Start at 1 instead of 0 to skip this point */
					n = np - 1;	/* Hence there is one less point to copy */
				}
				else {	/* We need all the points */
					j = 0;
					n = np;
				}
				GMT_Report (API, GMT_MSG_DEBUG, "Forward Segment no %" PRIu64 " [Table %d Segment %" PRIu64 "]\n", segment[id].orig_id, G, L);
				out_p = Copy_This_Segment (S, T[OPEN][out_seg], out_p, j, np-1);	/* Copy points, return array index where next point goes */
				/* Remember the last point we copied as that is the end of the growing output line segment */
				p_last_x = S->data[GMT_X][np-1];
				p_last_y = S->data[GMT_Y][np-1];
				if (first) p_first_x = S->data[GMT_X][0], p_first_y = S->data[GMT_Y][0];	/* Also remember start point of this chain */
			}
			else {	/* Must reverse the segment's order of points */
				if (doubleAlmostEqualZero (S->data[GMT_X][np-1], p_last_x) && doubleAlmostEqualZero (S->data[GMT_Y][np-1], p_last_y)) {	/* Skip duplicate anchor point */
					j = 1;		/* Start at the penultimate rather than last point to skip this duplicate point */
					n = np - 1;	/* Hence there is one less point to copy */
				}
				else {	/* We need all the points */
					j = 0;
					n = np;
				}
				GMT_Report (API, GMT_MSG_DEBUG, "Reverse Segment no %" PRIu64 " [Table %d Segment %" PRIu64 "]\n", segment[id].orig_id, G, L);
				out_p = Copy_This_Segment (S, T[OPEN][out_seg], out_p, np-1-j, 0);	/* Copy points in reverse order, return array index where next point goes */
				/* Remember the last point we copied as that is the end of the growing output line segment */
				p_last_x = S->data[GMT_X][0];
				p_last_y = S->data[GMT_Y][0];
				/* Note for next line: Could use [np-1-j] but if j == 1 then the entry in [np-1] is a duplicate of [np-2] so no need */
				if (first) p_first_x = S->data[GMT_X][np-1], p_first_y = S->data[GMT_Y][np-1];	/* Also remember start point of line segment */
			}
			n_seg_length += n;	/* Length of combined line segment after adding this one */
			first = false;		/* Done with setting the very first line segment in the composite output chain */
			end_order = !end_order;		/* Go to the other end of the line segment */
			segment[id].used = true;	/* Finished appending this line segment to our output line segmnent */
			if (segment[id].buddy[end_order].dist <= Ctrl->T.dist[0] && !segment[segment[id].buddy[end_order].id].used) {
				/* Not done, trace into the next connecting segment */
				id2 = segment[id].buddy[end_order].id;			/* The ID of the nearest line segment */
				end_order = segment[id].buddy[end_order].end_order;	/* Which end of this line segment is closest to our end */
				done = (id2 == start_id || id2 == id);			/* We are done if they are the same line segment */
				id = id2;						/* Update what is the current line segment */
			}
			else	/* End of the D[GMT_IN]->table for this segment */
				done = true;
			n_steps_pass_2++;	/* Count of number of pieces being connected into this single line segment */
		} while (!done);
		if (n_steps_pass_2 != n_steps_pass_1) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Trouble: Pass 1 found %" PRIu64 " while pass 2 found %" PRIu64 " connections!\n", n_steps_pass_1, n_steps_pass_2);
		}
		if (n_seg_length < n_alloc_pts) T[OPEN][out_seg] = GMT_Alloc_Segment (GMT->parent, smode, n_seg_length, n_columns, NULL, T[OPEN][out_seg]);	/* Trim memory allocation */

		if (doubleAlmostEqualZero (p_first_x, p_last_x) && doubleAlmostEqualZero (p_first_y, p_last_y)) {	/* Definitively closed polygon resulting from connections */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "New closed segment %" PRIu64 " made from %" PRIu64 " pieces\n", out_seg, n_steps_pass_2);
			if (Ctrl->D.active && save_type) {	/* Ended up closed, rename output filename with the C type instead of O set above */
				SH = gmt_get_DS_hidden (T[OPEN][out_seg]);
				sprintf (buffer, Ctrl->D.format, 'C', out_seg);
				gmt_M_str_free (SH->file[GMT_OUT]);
				SH->file[GMT_OUT] = strdup (buffer);
				d_mode = CLOSED;	/* Mode is used with -Q only */
			}
			n_closed++;	/* Another closed polygon completed */
		}
		else {
			n_open++;	/* This one remained open */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "New open segment %" PRIu64 " made from %" PRIu64 " pieces\n", out_seg, n_steps_pass_2);
		}
		if (Ctrl->Q.active) {	/* Add this polygon info to the info list */
			SH = gmt_get_DS_hidden (QT[d_mode]);
			QT[d_mode]->text[QT[d_mode]->n_rows++] = strdup (buffer);
			if (QT[d_mode]->n_rows == SH->n_alloc) QT[d_mode]->text = gmt_M_memory (GMT, QT[d_mode]->text, (SH->n_alloc <<= 1), char *);
		}

		chain++;	/* Number of composite line segments (closed or open) processed via connecting */
		out_seg++;	/* Number of output segment so far */

		/* Wind to the next unused segments to start the connection search again */
		start_id = 0;	/* Reset and wind */
		while (start_id < ns && segment[start_id].used) start_id++;
		done = (start_id == ns);	/* No more unused segments */
	}

	gmt_set_segmentheader (GMT, GMT_OUT, out_seg > 1);	/* Turn on|off segment headers on output */
	if (Ctrl->Q.active) {	/* Write out the list(s) with individual file names */
		Q->table[CLOSED]->segment[0]->text = gmt_M_memory (GMT, QT[CLOSED]->text, QT[CLOSED]->n_rows, char *);
		if (n_qfiles == 2) Q->table[OPEN]->segment[0]->text = gmt_M_memory (GMT, QT[OPEN]->text, QT[OPEN]->n_rows, char *);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, q_mode, NULL, Ctrl->Q.file, Q) != GMT_NOERROR) {
			gmt_M_free (GMT, buffer);
			Return (API->error);
		}
	}

	/* Write out the new multisegment file with polygons and segments */

	D[GMT_OUT]->table[0]->segment = gmt_M_memory (GMT, T[OPEN], out_seg, struct GMT_DATASEGMENT *);
	D[GMT_OUT]->n_segments = D[GMT_OUT]->table[0]->n_segments = out_seg;
	ofile = (Ctrl->D.active) ? Ctrl->D.format : Ctrl->Out.file;
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, ofile, D[GMT_OUT]) != GMT_NOERROR) {
		gmt_M_free (GMT, buffer);
		Return (API->error);
	}

	/* Tell us some statistics of what we found, if -V */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " segments read\n", ns + n_islands);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " new open segments\n", n_open);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " new closed segments\n", n_closed);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " segments were already closed\n", n_islands);
	if (n_trouble) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " trouble spots\n", n_trouble);

	gmt_M_free (GMT, buffer);

	Return (GMT_NOERROR);
}
