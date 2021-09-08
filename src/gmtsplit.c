/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: read a file of lon, lat, zvalue[, distance, azimuth]
 * and split it into profile segments.
 *
 * Author:	W. H. F. Smith
 * Date:	1 JAN 2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtsplit"
#define THIS_MODULE_MODERN_NAME	"gmtsplit"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Split xyz[dh] data tables into individual segments"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghiqs" GMT_OPT("H")

#define SPLITXYZ_F_RES			1000	/* Number of points in filter halfwidth  */
#define SPLITXYZ_N_OUTPUT_CHOICES	5

struct SPLITXYZ_CTRL {
	struct SPLITXYZ_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct SPLITXYZ_A {	/* -A<azimuth>/<tolerance> */
		bool active;
		double azimuth, tolerance;
	} A;
	struct SPLITXYZ_C {	/* -C<course_change> */
		bool active;
		double value;
	} C;
	struct SPLITXYZ_D {	/* -D<mindist> */
		bool active;
		double value;
	} D;
	struct SPLITXYZ_F {	/* -F<xy_filter>/<z_filter> */
		bool active;
		double xy_filter, z_filter;
	} F;
	struct SPLITXYZ_N {	/* -N<namestem> */
		bool active;
		char *name;
	} N;
	struct SPLITXYZ_Q {	/* -Q[<xyzdg>] */
		bool active;
		bool z_selected;
		char col[SPLITXYZ_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} Q;
	struct SPLITXYZ_S {	/* -S */
		bool active;
	} S;
};

GMT_LOCAL double *gmtsplit_filterxy_setup (struct GMT_CTRL *GMT) {
	unsigned int i;
	double tmp, sum = 0.0, *fwork = NULL;

	fwork = gmt_M_memory (GMT, NULL, SPLITXYZ_F_RES, double);	/* Initialized to zeros */
	tmp = M_PI / SPLITXYZ_F_RES;
	for (i = 0; i < SPLITXYZ_F_RES; i++) {
		fwork[i] = 1.0 + cos (i * tmp);
		sum += fwork[i];
	}
	for (i = 1; i < SPLITXYZ_F_RES; i++) fwork[i] /= sum;
	return (fwork);
}

GMT_LOCAL void gmtsplit_filter_cols (struct GMT_CTRL *GMT, double *data[], uint64_t begin, uint64_t end, unsigned int d_col, unsigned int n_cols, unsigned int cols[], double filter_width, double *fwork) {
	uint64_t i, j, k, p, istart, istop, ndata;
	int64_t kk;
	bool hilow;
	double half_width, dt, sum, **w = NULL;

	if (filter_width == 0.0) return;	/* No filtering */
	hilow = (filter_width < 0.0);
	half_width = 0.5 * fabs (filter_width);
	dt = SPLITXYZ_F_RES / half_width;
	ndata = end - begin;
	w = gmt_M_memory (GMT, NULL, n_cols, double *);
	for (k = 0; k < n_cols; k++) w[k] = gmt_M_memory (GMT, NULL, ndata, double);	/* Initialized to zeros */
	j = istart = istop = begin;
	while (j < end) {
		while (istart < end && data[d_col][istart] - data[d_col][j] <= -half_width) istart++;
		while (istop  < end && data[d_col][istop]  - data[d_col][j] <   half_width) istop++;
		for (i = istart, sum = 0.0; i < istop; i++) {
			kk = lrint (floor (dt * fabs (data[d_col][i] - data[d_col][j])));
			if (kk < 0 || kk >= SPLITXYZ_F_RES) continue;	/* Safety valve */
			k = kk;
			sum += fwork[k];
			for (p = 0; p < n_cols; p++) w[p][j] += (data[cols[p]][i] * fwork[k]);
		}
		for (p = 0; p < n_cols; p++) w[p][j] /= sum;
		j++;
	}
	if (hilow) {
		for (i = begin; i < end; i++) for (p = 0; p < n_cols; p++) data[cols[p]][i] -= w[p][i];
	}
	else {
		for (i = begin; i < end; i++) for (p = 0; p < n_cols; p++) data[cols[p]][i] = w[p][i];
	}
	for (p = 0; p < n_cols; p++) gmt_M_free (GMT, w[p]);
	gmt_M_free (GMT, w);
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPLITXYZ_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct SPLITXYZ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.azimuth = 90.0;
	C->A.tolerance = 360.0;
	C->C.value = 180.0;	/* Tolerate any course change */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct SPLITXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->N.name);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the gmtsplit synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] [-A<azimuth>/<tolerance>] [-C<course_change>] [-D<minimum_distance>] "
		"[-F<xy_filter>/<z_filter>] [-N<template>] [-Q<flags>] [-S] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_q_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<table>");
	GMT_Usage (API, -2, "One or more data files (in ASCII, binary, netCDF) with 2, 3 or 5 columns. "
		"If no files are given, standard input is read.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A<azimuth>/<tolerance>");
	GMT_Usage (API, -2, "Only write profile if mean direction is within +/- <tolerance> "
		"degrees of <azimuth> [Default = All].");
	GMT_Usage (API, 1, "\n-C<course_change>");
	GMT_Usage (API, -2, "Profile ends when change of heading exceeds <course_change> [ignore course changes].");
	GMT_Usage (API, 1, "\n-D<minimum_distance>");
	GMT_Usage (API, -2, "Only write profile if length is at least <minimum_distance> [0].");
	GMT_Usage (API, 1, "\n-F<xy_filter>/<z_filter>");
	GMT_Usage (API, -2, "Filter the data: Give full widths of cosine arch filters for xy and z. "
		"Use negative width for high-pass filter [Default is no filtering].");
	GMT_Usage (API, 1, "\n-N<template>");
	GMT_Usage (API, -2, "Write individual segments to separate files [Default writes one "
		"multisegment file to standard output].  Append file name template which MUST "
		"contain a C-style format code for a long integer (e.g., %%d) that represents "
		"a sequential segment number across all tables (if more than one table) "
		"[Default uses gmtsplit_segment_%%d.txt (or .bin for binary)]. "
		"Alternatively, supply a template with two long format codes and we will "
		"replace them with the table number and table segment numbers.");
	GMT_Usage (API, 1, "\n-Q<flags>");
	GMT_Usage (API, -2, "Indicate what output you want as one or more of xyzdh in any order, "
		"where x,y,z refer to input data locations and optional z-value(s), "
		"and d,h are the distance and heading along track "
		"[Default is all fields, i.e., -Qxyzdh (or -Qxydh if no z-column in the input)].");
	GMT_Usage (API, 1, "\n-S d,h is supplied: Input is 5 col x,y,z,d,h with d non-decreasing "
		"[Default input is 3 col x,y,z only and computes d,h from the data].");
	GMT_Option (API, "V,bi");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Default input columns is set via -S.");
	GMT_Option (API, "bo,d,e,f,g,h,i,q,s,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct SPLITXYZ_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtsplit and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, n_errors = 0, n_outputs = 0, n_files = 0;
	char txt_a[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				n_errors += gmt_M_check_condition (GMT, (sscanf(opt->arg, "%lf/%lf", &Ctrl->A.azimuth, &Ctrl->A.tolerance)) != 2,
				                                       "Option -A: Can't decipher values\n");
				break;
			case 'C':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				n_errors += gmt_M_check_condition (GMT, (sscanf(opt->arg, "%lf", &Ctrl->C.value)) != 1,
				                                       "Option -C: Can't decipher value\n");
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				n_errors += gmt_M_check_condition (GMT, (sscanf(opt->arg, "%lf", &Ctrl->D.value)) != 1,
				                                       "Option -D: Can't decipher value\n");
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				n_errors += gmt_M_check_condition (GMT, (sscanf(opt->arg, "%lf/%lf", &Ctrl->F.xy_filter, &Ctrl->F.z_filter)) != 2,
				                                       "Option -F: Can't decipher values\n");
				break;
			case 'G':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-G option is deprecated; use -g instead.\n");
					GMT->common.g.active = true;
					if (gmt_M_is_geographic (GMT, GMT_IN))
						sprintf (txt_a, "D%sk", opt->arg);	/* Hardwired to be km */
					else
						sprintf (txt_a, "d%s", opt->arg);	/* Cartesian */
					n_errors += gmt_parse_g_option (GMT, txt_a);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'M':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (gmt_M_is_cartesian (GMT, GMT_IN)) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				if (opt->arg[0]) Ctrl->N.name = strdup (opt->arg);
				break;
			case 'Q':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				for (j = 0; opt->arg[j]; j++) {
					if (j < SPLITXYZ_N_OUTPUT_CHOICES) {
						Ctrl->Q.col[j] = opt->arg[j];
						if (!strchr ("xyzdh", Ctrl->Q.col[j])) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Unrecognized output choice %c\n", Ctrl->Q.col[j]);
							n_errors++;
						}
						if (opt->arg[j] == 'z') Ctrl->Q.z_selected = true;
						n_outputs++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Too many output columns selected: Choose from -Qxyzdg\n");
						n_errors++;
					}
				}
				break;
			case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				break;
			case 'Z':
				if (gmt_M_compat_check (GMT, 5)) /* Warn and pass through */
					GMT_Report (API, GMT_MSG_COMPAT, "-Z option is deprecated and not longer required.\n");
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->D.value < 0.0, "Option -D: Minimum segment distance must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.value <= 0.0, "Option -C: Course change tolerance must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.tolerance < 0.0, "Option -A: Azimuth tolerance must be positive\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_OUT] && !Ctrl->N.name,
	                                 "Binary output requires a namestem in -N\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->S.active) ? 5 : 3);
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->N.name && !strstr (Ctrl->N.name, "%"),
	                                 "Option -N: Output template must contain %%d\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtsplit (void *V_API, int mode, void *args) {
	bool ok, first = true, no_z_column;

	unsigned int i, j, d_col, h_col, z_cols, xy_cols[2] = {0, 1}, io_mode = 0;
	unsigned int output_choice[SPLITXYZ_N_OUTPUT_CHOICES], n_outputs = 0, n_in;

	int error = 0;

	uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 0};	/* Output dataset will have one table only */
	uint64_t tbl, col, n_out = 0, k, n, row, seg, seg2 = 0, begin, end, n_total = 0, n_columns = 0, nprofiles = 0, *rec = NULL;

	size_t n_alloc_seg = 0, n_alloc = 0;

	double dy, dx, last_c, last_s, csum, ssum, this_c, this_s, dotprod, mean_azim, *fwork = NULL;

	char header[GMT_LEN64] = {""};

	struct GMT_DATASET *D[2] = {NULL, NULL};
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASEGMENT *S = NULL, *S_out = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct SPLITXYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != GMT_NOERROR) Return (error);

	/*---------------------------- This is the gmtsplit main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	n_in = (Ctrl->S.active) ? 5 : 3;
	if ((error = GMT_Set_Columns (API, GMT_IN, n_in, GMT_COL_VAR)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_FILEBREAK, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D[GMT_IN]->n_columns < (n_in-1)) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least %u are needed\n", (int)D[GMT_IN]->n_columns, n_in);
		Return (GMT_DIM_TOO_SMALL);
	}

	gmt_M_memset (output_choice, SPLITXYZ_N_OUTPUT_CHOICES, int);
	no_z_column = (D[GMT_IN]->n_columns == 2);

	if (no_z_column && Ctrl->S.active) {
		GMT_Report (API, GMT_MSG_ERROR, "The -S option requires a 3rd data column\n");
		Return (GMT_PARSE_ERROR);
	}
	if (no_z_column && Ctrl->F.z_filter != 0.0) {
		GMT_Report (API, GMT_MSG_ERROR, "The -F option requires a 3rd data column\n");
		Return (GMT_PARSE_ERROR);
	}
	if (Ctrl->Q.z_selected && no_z_column) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -Q: Cannot request z if unless data have a 3rd column\n");
		Return (GMT_PARSE_ERROR);
	}

	/* Determine output choices and order */
	for (k = n_outputs = 0; k < SPLITXYZ_N_OUTPUT_CHOICES && Ctrl->Q.col[k]; k++) {
		switch (Ctrl->Q.col[k]) {
			case 'x':
				output_choice[k] = GMT_X;
				break;
			case 'y':
				output_choice[k] = GMT_Y;
				break;
			case 'z':
				if (no_z_column) {
					GMT_Report (API, GMT_MSG_ERROR, "Cannot specify z when there is no z column!\n");
					Return (-1);
				}
				output_choice[k] = GMT_Z;
				break;
			case 'd':
				output_choice[k] = 3 - no_z_column;
				break;
			case 'h':
				output_choice[k] = 4 - no_z_column;
				break;
		}
		n_outputs++;
	}
	if (gmt_M_is_geographic (GMT, GMT_IN))
		gmt_set_geographic (GMT, GMT_OUT);

	if (n_outputs == 0) {	/* No -Q given, generate default -Q setting (all) */
		n_outputs = 5 - no_z_column;
		for (i = 0; i < 2; i++) output_choice[i] = i;
		if (!no_z_column) output_choice[2] = 2;
		for (i = 3-no_z_column; i < n_outputs; i++) output_choice[i] = i;
	}

	/* Convert to radians */
	Ctrl->A.tolerance *= D2R;
	Ctrl->A.azimuth = D2R * (90.0 - Ctrl->A.azimuth);	/* Work in Cartesian angle and radians  */
	Ctrl->C.value *= D2R;
	if (Ctrl->F.active) fwork = gmtsplit_filterxy_setup (GMT);
	if (!Ctrl->N.active)
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	if ((error = GMT_Set_Columns (API, GMT_OUT, n_outputs, (D[GMT_IN]->type == GMT_READ_DATA) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}
	/* Registers default output destination, unless already set */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
		Return (API->error);
	}

	if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold distance and azimuth */
		n_columns = D[GMT_IN]->n_columns + 2;
		d_col = (unsigned int)D[GMT_IN]->n_columns;
		h_col = d_col + 1;
		gmt_adjust_dataset (GMT, D[GMT_IN], n_columns);
	}
	else {	/* Comes with d and az in file */
		d_col = no_z_column + 2;
		h_col = no_z_column + 3;
	}
	z_cols = 2;
	S_out = gmt_get_segment (GMT);	/* We will use a single segment to hold all output records and keep track of start/stop via rec[] array */

	nprofiles = 0;
	for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		T = D[GMT_IN]->table[tbl];	/* Shorthand for curent table */
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
			struct GMT_DATATABLE_HIDDEN *TH = gmt_get_DT_hidden (T);
			GMT_Report (API, GMT_MSG_INFORMATION, "Working on file %s\n", TH->file[GMT_IN]);
		}

		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in this table */
			S = T->segment[seg];
			if (Ctrl->S.active) S->data[h_col][0] = D2R * (90.0 - S->data[h_col][0]);	/* Angles are stored as CCW angles in radians */
			for (row = 1; row < S->n_rows; row++) {
				if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold distance and azimuth */
					dy = S->data[GMT_Y][row] - S->data[GMT_Y][row-1];
					if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* We do flat earth approximation for distances, which assumes points are close to each other */
						gmt_M_set_delta_lon (S->data[GMT_X][row-1], S->data[GMT_X][row], dx);
						dy *= GMT->current.proj.DIST_KM_PR_DEG;
						dx *= (GMT->current.proj.DIST_KM_PR_DEG * cosd (0.5 * (S->data[GMT_Y][row] + S->data[GMT_Y][row-1])));
					}
					else
						dx = S->data[GMT_X][row] - S->data[GMT_X][row-1];
					if (dy == 0.0 && dx == 0.0) {
						S->data[d_col][row] = S->data[d_col][row-1];
						S->data[h_col][row] = S->data[h_col][row-1];
					}
					else {
						S->data[d_col][row] = S->data[d_col][row-1] + hypot (dx,dy);
						S->data[h_col][row] = d_atan2(dy,dx);	/* Angles are stored as CCW angles in radians */
					}
				}
				else
					S->data[h_col][row] = D2R * (90.0 - S->data[h_col][row]);	/* Angles are stored as CCW angles in radians */
			}
			if (!Ctrl->S.active) S->data[h_col][0] = S->data[h_col][1];

			/* Here a complete segment is ready for further processing */
			/* Now we have read the data and can filter z, if necessary.  */

			if (Ctrl->F.active)
				gmtsplit_filter_cols (GMT, S->data, 0, S->n_rows, d_col, 1, &z_cols, Ctrl->F.z_filter, fwork);

			/* Now we are ready to search for segments */

			begin = end = 0;
			while (end < S->n_rows-1) {
				sincos (S->data[h_col][begin], &last_s, &last_c);
				csum = last_c;	ssum = last_s;
				ok = true;
				while (ok && end < S->n_rows-1) {
					end++;
					sincos (S->data[h_col][end], &this_s, &this_c);
					dotprod = this_c * last_c + this_s * last_s;
					if (fabs (dotprod) > 1.0) dotprod = copysign (1.0, dotprod);
					if (d_acos (dotprod) > Ctrl->C.value) {	/* Fails due to too much change in azimuth */
						ok = false;
						continue;
					}
					/* Get here when this point belongs with last one */
					csum  += this_c;
					ssum  += this_s;
					last_c = this_c;
					last_s = this_s;
				}

				/* Get here when we have found a beginning and end  */

				if (ok) end++;	/* Last point in input should be included in this segment */

				if (end - begin - 1) { /* There are at least two points in the list.  */
					if ((S->data[d_col][end-1] - S->data[d_col][begin]) >= Ctrl->D.value) {
						/* List is long enough.  Check strike. Compute mean_azim in range [-pi/2, pi/2] */

						mean_azim = d_atan2 (ssum, csum);
						mean_azim = fabs (mean_azim - Ctrl->A.azimuth);
						if (mean_azim <= Ctrl->A.tolerance) {	/* List has acceptable strike */
							if (Ctrl->F.active)
								gmtsplit_filter_cols (GMT, S->data, begin, end, d_col, 2, xy_cols, Ctrl->F.xy_filter, fwork);
							nprofiles++;

							n_out = end - begin;
							if ((n_total + n_out) >= n_alloc) {	/* Allocate more memory for temporary segment */
								n_alloc = (first) ? D[GMT_IN]->n_records : n_alloc * 2;
								gmt_alloc_segment (GMT, S_out, n_alloc, n_outputs, 0U, first);
								first = false;
							}

							for (row = begin; row < end; row++, k++) {
								for (j = 0; j < n_outputs; j++) {	/* Remember to convert CCW angles back to azimuths */
									S_out->data[j][k] = (output_choice[j] == h_col) ? 90.0 - R2D * S->data[h_col][row] : S->data[output_choice[j]][row];
								}
							}
							if (seg2 == n_alloc_seg) {
								n_alloc_seg = (n_alloc_seg == 0) ? D[GMT_IN]->n_segments : n_alloc_seg * 2;
								rec = gmt_M_memory (GMT, rec, n_alloc_seg, uint64_t);
							}
							rec[seg2++] = k;
							n_total += n_out;
						}
					}
				}
				begin = end;
			}
		}
	}

	if (!Ctrl->S.active)	/* Must remove the 2 cols we added  */
		gmt_adjust_dataset (GMT, D[GMT_IN], n_columns-2);

	if (Ctrl->F.active) gmt_M_free (GMT, fwork);

	/* Get here when all profiles have been found and written */

	if (nprofiles > 1)
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	dim[GMT_SEG] = seg2;	dim[GMT_COL] = n_outputs;	/* dim[GMT_ROW] is zero so segment pointers have no data */
	if ((D[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		gmt_M_free (GMT, rec);
		goto end;
	}
	for (seg = 0; seg < seg2; seg++) {	/* We fake a table by setting the data pointers to point to various points in our single S_out arrays */
		S = D[GMT_OUT]->table[0]->segment[seg];
		SH = gmt_get_DS_hidden (S);
		k = (seg == 0) ? 0 : rec[seg-1];	/* Record number of first row in the new segment */
		n = (seg == 0) ? rec[seg] : rec[seg] - rec[seg-1];	/* How many rows in the new segment */
		if (gmt_alloc_segment (GMT, S, n, n_outputs, S->n_columns, true)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to allocate output memory for a new segment");
			goto end;
		}

		for (j = 0; j < n_outputs; j++)	/* Duplicate the rows */
			gmt_M_memcpy (S->data[j], &S_out->data[j][k], n, double);
		S->n_rows = n;
		sprintf (header, "Profile %" PRIu64" -I%" PRIu64, seg, seg);
		S->header = strdup (header);
		SH->id = seg;
	}
	gmt_M_free (GMT, rec);

	GMT_Report (API, GMT_MSG_INFORMATION, " Split %" PRIu64 " data into %" PRIu64 " segments.\n", D[GMT_IN]->n_records, nprofiles);
	if (Ctrl->N.active) {	/* Want tables or segments written to separate files */
		int n_formats = 0;
		for (k = 0; Ctrl->N.name[k]; k++) if (Ctrl->N.name[k] == '%') n_formats++;
		io_mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENT: GMT_WRITE_SEGMENT;
		/* The io_mode tells the i/o function to split segments into files */
		gmt_M_str_free (Ctrl->Out.file);
		Ctrl->Out.file = strdup (Ctrl->N.name);
	}
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_PLP, io_mode, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_NOERROR) {
		goto end;
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		goto end;
	}

end:	/* Clean up memory and return */

	gmt_free_segment (GMT, &S_out);

	Return (API->error);
}

EXTERN_MSC int GMT_splitxyz (void *V_API, int mode, void *args) {
	/* This was the GMT6.1.1 name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (gmt_M_compat_check (API->GMT, 6)) {
		GMT_Report (API, GMT_MSG_COMPAT, "Module splitxyz is deprecated; use gmtsplit.\n");
		return (GMT_Call_Module (API, "gmtsplit", mode, args));
	}
	GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: splitxyz\n");
	return (GMT_NOT_A_VALID_MODULE);
}
