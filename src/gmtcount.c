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
 * Brief synopsis: Based on a specified grid size, gmtcount reads an xy[z][w] file and
 * find all values inside a radius R centered on each output node.  Points inside the
 * circle will be ued to compute the desired output statistic.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtcount"
#define THIS_MODULE_MODERN_NAME	"gmtcount"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Grid table data by geospatial counting"
#define THIS_MODULE_KEYS	"<D{,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVabdefghinqrs" GMT_OPT("FH")

enum grdcount_types {
	GMTCOUNT_LOWER = 1,
	GMTCOUNT_LOWERP,
	GMTCOUNT_UPPER,
	GMTCOUNT_UPPERP,
	GMTCOUNT_ZRANGE,
	GMTCOUNT_SUM,
	GMTCOUNT_COUNT,
	GMTCOUNT_COUNTW,
	GMTCOUNT_MEAN,
	GMTCOUNT_MEANW,
	GMTCOUNT_MEDIAN,
	GMTCOUNT_MEDIANW,
	GMTCOUNT_MODE,
	GMTCOUNT_MODEW,
	GMTCOUNT_QUANT,
	GMTCOUNT_QUANTW,
	GMTCOUNT_IRANGE,
	GMTCOUNT_IRANGEW,
	GMTCOUNT_STD,
	GMTCOUNT_STDW,
	GMTCOUNT_MAD,
	GMTCOUNT_MADW,
	GMTCOUNT_LMSSCL,
	GMTCOUNT_LMSSCLW,
	GMTCOUNT_RMS,
	GMTCOUNT_RMSW
};

struct GMTCOUNT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct GMTCOUNT_C {	/* -Ca|d|g|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z */
		bool active;
		unsigned int mode;
		double quant;
	} C;
	struct GMTCOUNT_E {	/* -E<empty> */
		bool active;
		double value;
	} E;
	struct GMTCOUNT_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GMTCOUNT_N {	/* -N */
		bool active;
	} N;
	struct GMTCOUNT_S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n] */
		bool active;
		int mode;	/* May be negative */
		double radius;
		char unit;
	} S;
	struct GMTCOUNT_W {	/* -W[+s] */
		bool active;
		bool sigma;
	} W;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTCOUNT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1U, struct GMTCOUNT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.mode = GMTCOUNT_COUNT;
	C->C.quant = 50.0;	/* Median*/
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTCOUNT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -Ca|d|g|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z -G<outgrid> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s -S%s [-E<empty>] [-N] [%s] [-W[+s]]\n", GMT_Rgeo_OPT, GMT_RADIUS_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n", GMT_a_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Name of output grid.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify the statistic of data inside the circle we should report per node.  Choose from.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a: The mean (average)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d: The median absolute deviation (MAD)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g: The full data range (max-min)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i: The 25-75%% interquartile range\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l: The minimum (low)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   L: The minimum of all positive values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: The median\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n: The number of values [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   o: The LMS scale\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p: The mode (maximum likelihood)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   q: The selected quantile value; append quantile [50%%]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r: The r.m.s.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s: The standard deviation\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   u: The maximum (upper)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   U: The maximum of all negative values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z: The sum\n");
	GMT_Option (API, "R");
	gmt_dist_syntax (API->GMT, 'S', "Compute statistics using points inside this search radius.");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Value to use for empty nodes [Default is NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize the output by the area of the circle [no normalization].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Input <table> has observation weights in 4th column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   We then compute weighted version of selection in -C.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s to read standard deviations s instead and compute w = 1/s.\n");
	GMT_Option (API, "a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 (or 4 if -W is set) columns.\n");
	GMT_Option (API, "di,e,f,h,i");
	if (gmt_M_showusage (API)) {
		GMT_Message (API, GMT_TIME_NONE, "\t-n+b<BC> Set boundary conditions.  <BC> can be either:\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   g for geographic, p for periodic, and n for natural boundary conditions.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   For p and n you may optionally append x or y [default is both]:\n");
		GMT_Message (API, GMT_TIME_NONE, "\t     x applies the boundary condition for x only\n");
		GMT_Message (API, GMT_TIME_NONE, "\t     y applies the boundary condition for y only\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   [Default: Natural conditions, unless grid is geographic].\n");
	}
	GMT_Option (API, "qi,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTCOUNT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtcount and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'E':	/* NaN value */
				Ctrl->E.active = true;
				if (opt->arg[0])
					Ctrl->E.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -E: Must specify value or NaN\n");
				}
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'C':	/* -Ca|d|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z */
				Ctrl->C.active = true;
				switch (opt->arg[0]) {
					case 'a': Ctrl->C.mode = GMTCOUNT_MEAN;	break;
					case 'd': Ctrl->C.mode = GMTCOUNT_MAD;	break;
					case 'g': Ctrl->C.mode = GMTCOUNT_ZRANGE;	break;
					case 'i': Ctrl->C.mode = GMTCOUNT_IRANGE;	break;
					case 'l': Ctrl->C.mode = GMTCOUNT_LOWER;	break;
					case 'L': Ctrl->C.mode = GMTCOUNT_LOWERP;	break;
					case 'm': Ctrl->C.mode = GMTCOUNT_MEDIAN;	break;
					case 'n': Ctrl->C.mode = GMTCOUNT_COUNT;	break;
					case 'o': Ctrl->C.mode = GMTCOUNT_LMSSCL;	break;
					case 'p': Ctrl->C.mode = GMTCOUNT_MODE;	break;
					case 'q': Ctrl->C.mode = GMTCOUNT_QUANT;
						if (opt->arg[1]) Ctrl->C.quant = atof (&opt->arg[1]);
						break;
					case 'r': Ctrl->C.mode = GMTCOUNT_RMS;	break;
					case 's': Ctrl->C.mode = GMTCOUNT_STD;	break;
					case 'u': Ctrl->C.mode = GMTCOUNT_UPPER;	break;
					case 'U': Ctrl->C.mode = GMTCOUNT_UPPERP;	break;
					case 'z': Ctrl->C.mode = GMTCOUNT_SUM;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Method %s not recognized!\n", opt->arg);
						n_errors++;
						break;
				}
				break;
			case 'M':	/* Normalize by area */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Search radius */
				Ctrl->S.active = true;
				Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;
			case 'W':	/* Use weights */
				Ctrl->W.active = true;
				Ctrl->W.sigma = (strstr (opt->arg, "+s")) ? true : false;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.active, "Option -C: Must specify a method for the processing");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -1, "Option -S: Unrecognized unit\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -2, "Option -S: Unable to decode radius\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -3, "Option -S: Radius is negative\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);
	if (Ctrl->W.active) {	/* Update the mode if median or mode */
		if (Ctrl->C.mode == GMTCOUNT_MEDIAN) Ctrl->C.mode = GMTCOUNT_MEDIANW;
		if (Ctrl->C.mode == GMTCOUNT_MODE) Ctrl->C.mode = GMTCOUNT_MODEW;
		if (Ctrl->C.mode == GMTCOUNT_MEAN) Ctrl->C.mode = GMTCOUNT_MEANW;
		if (Ctrl->C.mode == GMTCOUNT_MAD) Ctrl->C.mode = GMTCOUNT_MADW;
		if (Ctrl->C.mode == GMTCOUNT_STD) Ctrl->C.mode = GMTCOUNT_STDW;
		if (Ctrl->C.mode == GMTCOUNT_LMSSCL) Ctrl->C.mode = GMTCOUNT_LMSSCLW;
		if (Ctrl->C.mode == GMTCOUNT_COUNT) Ctrl->C.mode = GMTCOUNT_COUNTW;
		if (Ctrl->C.mode == GMTCOUNT_QUANT) Ctrl->C.mode = GMTCOUNT_QUANTW;
		if (Ctrl->C.mode == GMTCOUNT_IRANGE) Ctrl->C.mode = GMTCOUNT_IRANGEW;
		if (Ctrl->C.mode == GMTCOUNT_RMS) Ctrl->C.mode = GMTCOUNT_RMSW;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void gmtcount_assign_node (struct GMT_CTRL *GMT, struct GMT_GRID *G, char *visited, gmt_grdfloat *w, gmt_grdfloat *s, gmt_grdfloat **zp, struct GMT_OBSERVATION **p, \
		unsigned int *np, unsigned int *n_alloc, uint64_t ij, uint64_t kk, unsigned int mode, double *in, double weight) {
	/* Assign the input value/weight to the output grid and/or helper arrays, depending on the mode */

	switch (mode) {
		case GMTCOUNT_MEAN:
			G->data[ij] += in[GMT_Z];
			w[kk] += 1.0;
			break;
		case GMTCOUNT_MEANW:
			G->data[ij] += in[GMT_Z] * weight;
			w[kk] += weight;
			break;
		case GMTCOUNT_STD:
			G->data[ij] += in[GMT_Z];
			s[kk] += in[GMT_Z] * in[GMT_Z];	/* Sum of z squared */
			np[kk]++;
			break;
		case GMTCOUNT_STDW:
			G->data[ij] += in[GMT_Z] * weight;	/* Sum of weighted z */
			w[kk] += weight;	/* Sum of weights */
			s[kk] += in[GMT_Z] * in[GMT_Z] * weight;	/* Sum of weighted z^2 */
			np[kk]++;
			break;
		case GMTCOUNT_RMS:
			G->data[ij] += in[GMT_Z] * in[GMT_Z];	/* Sum of z squared */
			np[kk]++;
			break;
		case GMTCOUNT_RMSW:
			G->data[ij] += in[GMT_Z] * in[GMT_Z] * weight;	/* Sum of weighted z^2 */
			w[kk] += weight;	/* Sum of weights */
			break;
		case GMTCOUNT_MEDIAN: case GMTCOUNT_MODE: case GMTCOUNT_QUANT: case GMTCOUNT_MAD: case GMTCOUNT_LMSSCL: case GMTCOUNT_IRANGE:
			if (n_alloc[kk] >= np[kk]) {
				n_alloc[kk] += GMT_SMALL_CHUNK;
				zp[kk] = gmt_M_memory (GMT, zp[kk], n_alloc[kk], gmt_grdfloat);
			}
			zp[kk][np[kk]++] = in[GMT_Z];
			break;
		case GMTCOUNT_MEDIANW: case GMTCOUNT_MODEW: case GMTCOUNT_QUANTW: case GMTCOUNT_MADW: case GMTCOUNT_LMSSCLW: case GMTCOUNT_IRANGEW:
			if (n_alloc[kk] >= np[kk]) {
				n_alloc[kk] += GMT_SMALL_CHUNK;
				p[kk] = gmt_M_memory (GMT, p[kk], n_alloc[kk], struct GMT_OBSERVATION);
			}
			p[kk][np[kk]].value    = in[GMT_Z];
			p[kk][np[kk]++].weight = weight;
			break;
		case GMTCOUNT_LOWERP:
			if (in[GMT_Z] <= 0.0) return;	/* Only consider positive values */
			/* Fall through on purpose */
		case GMTCOUNT_LOWER:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] < G->data[ij]) G->data[ij] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTCOUNT_UPPERP:
			if (in[GMT_Z] >= 0.0) return;	/* Only consider negative values */
			/* Fall through on purpose */
		case GMTCOUNT_UPPER:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] > G->data[ij]) G->data[ij] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTCOUNT_ZRANGE:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] > G->data[ij]) G->data[ij] = in[GMT_Z];
				if (in[GMT_Z] < s[kk]) s[kk] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = s[kk] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTCOUNT_SUM: case GMTCOUNT_COUNTW:
			G->data[ij] += in[GMT_Z];
			break;
		default:	/* count */
			G->data[ij] += 1.0;
			break;
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtcount (void *V_API, int mode, void *args) {
	char *visited = NULL;

	bool wrap_180, replicate_x, replicate_y;

	int col_0, row_0, row, col, row_end, col_end, ii, jj, error = 0;
	unsigned int n_cols, k, rowu, colu, d_row, y_wrap_kk, y_wrap_ij, max_d_col, x_wrap;
	unsigned int *d_col = NULL, *n_in_circle = NULL, *n_alloc = NULL;
	unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;

	uint64_t ij, kk, n = 0, n_read = 0;

	gmt_grdfloat *w = NULL, *s = NULL, **zp = NULL;

	double dx, dy, distance = 0.0, x_left, x_right, y_top, y_bottom, weight = 1.0;
	double half_y_width, y_width, half_x_width, x_width, median, zmode, mad;
	double *x0 = NULL, *y0 = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_OBSERVATION **zw_pair = NULL;
	struct GMTCOUNT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtcount main code ----------------------------*/

	if (gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
		Return (GMT_NOT_A_VALID_TYPE);

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	HH = gmt_get_H_hidden (Grid->header);

	/* Initialize the input since we are doing record-by-record reading/writing */
	n_cols = (Ctrl->C.mode == GMTCOUNT_COUNT) ? 2 : 3;
	if ((error = GMT_Set_Columns (API, GMT_IN, n_cols + Ctrl->W.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Grid dimensions are n_columns = %d, n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);

	x0 = Grid->x;	/* Short-hand */
	y0 = Grid->y;

	d_col = gmt_prep_nodesearch (GMT, Grid, Ctrl->S.radius, Ctrl->S.mode, &d_row, &max_d_col);	/* Init d_row/d_col etc */

	/* To allow data points falling outside -R but within the search radius we extend the data domain in all directions */

	x_left = Grid->header->wesn[XLO];	x_right = Grid->header->wesn[XHI];	/* This is what -R says */
	if (gmt_M_is_cartesian (GMT, GMT_IN) || !gmt_grd_is_global (GMT, Grid->header)) {
		x_left  -= max_d_col * Grid->header->inc[GMT_X];	/* OK to extend x-domain since not a periodic geographic grid */
		x_right += max_d_col * Grid->header->inc[GMT_X];
	}
	y_top = Grid->header->wesn[YHI] + d_row * Grid->header->inc[GMT_Y];	y_bottom = Grid->header->wesn[YLO] - d_row * Grid->header->inc[GMT_Y];
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* For geographic grids we must ensure the extended y-domain is physically possible */
		if (y_bottom < -90.0) y_bottom = -90.0;
		if (y_top > 90.0) y_top = 90.0;
	}
	x_width = Grid->header->wesn[XHI] - Grid->header->wesn[XLO];		y_width = Grid->header->wesn[YHI] - Grid->header->wesn[YLO];
	half_x_width = 0.5 * x_width;			half_y_width = 0.5 * y_width;
	replicate_x = (HH->nxp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate column */
	replicate_y = (HH->nyp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate row */
	x_wrap = Grid->header->n_columns - 1;	/* Add to node index to go to right column */
	y_wrap_kk = (Grid->header->n_rows - 1) * Grid->header->n_columns;	/* Add to node index to go to bottom row if no padding */
	y_wrap_ij = (Grid->header->n_rows - 1) * Grid->header->mx;	/* Add to node index to go to bottom row if padded */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		gmt_M_free (GMT, d_col);
		Return (API->error);
	}

	/* Add helper arrays depending on the mode */
	switch (Ctrl->C.mode) {
		case GMTCOUNT_MEAN: case GMTCOUNT_MEANW: case GMTCOUNT_RMSW:	/* Will need to keep track of n or sum of weights */
			w = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weights */
			break;
		case GMTCOUNT_MEDIAN: case GMTCOUNT_MODE: case GMTCOUNT_QUANT: case GMTCOUNT_MAD: case GMTCOUNT_LMSSCL: case GMTCOUNT_IRANGE:
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			n_alloc = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Allocation size per node */
			zp = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat *);	/* Matrix with pointers to an array of grdfloats */
			break;
		case GMTCOUNT_MEDIANW: case GMTCOUNT_MODEW: case GMTCOUNT_QUANTW: case GMTCOUNT_MADW: case GMTCOUNT_LMSSCLW: case GMTCOUNT_IRANGEW:
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			n_alloc = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Allocation size per node */
			zw_pair = gmt_M_memory (GMT, NULL, Grid->header->nm, struct GMT_OBSERVATION *);	/* Matrix with pointers to an structure array of z,w pairs */
			break;
		case GMTCOUNT_LOWER: case GMTCOUNT_LOWERP: case GMTCOUNT_UPPER: case GMTCOUNT_UPPERP:
			visited = gmt_M_memory (GMT, NULL, Grid->header->nm, char);	/* Flag so we know if we have been here */
			break;
		case GMTCOUNT_STD:	/* Will need to keep track of n or sum of weights, plus sum of squares */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of squares */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			break;
		case GMTCOUNT_STDW:	/* Will need to keep track of n or sum of weights, plus sum of squares */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weighted squares */
			w = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weights */
			break;
		case GMTCOUNT_ZRANGE:	/* Will need to keep track of max */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* For minimum value */
			visited = gmt_M_memory (GMT, NULL, Grid->header->nm, char);	/* Flag so we know if we have been here */
			break;
		case GMTCOUNT_RMS:	/* Will need to keep track of n */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			break;
		default:	/* No extra memory needed, just Grid->data */
			break;
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				gmt_M_free (GMT, d_col);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		in = In->data;	/* Only need to process numerical part here */

		if (gmt_M_is_dnan (in[GMT_Z])) continue;					/* Skip if z = NaN */
		if (gmt_M_y_is_outside (GMT, in[GMT_Y], y_bottom, y_top)) continue;	/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], x_left, x_right)) continue;	/* Outside x-range (or longitude) */

		/* Data record to process */

		if (Ctrl->W.active) weight = (Ctrl->W.sigma) ? 1.0 / in[3] : in[3];	/* Got sigma, make weight = 1 / sigma */

		/* Find row/col indices of the node closest to this data point.  Note: These may be negative */

		col_0 = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		row_0 = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);

		/* Loop over all nodes within radius of this node */

		row_end = row_0 + d_row;
		for (row = row_0 - d_row; row <= row_end; row++) {

			jj = row;
			if (gmt_y_out_of_bounds (GMT, &jj, Grid->header, &wrap_180)) continue;	/* Outside y-range.  This call must happen BEFORE gmt_x_out_of_bounds as it sets wrap_180 */
			rowu = jj;
			col_end = col_0 + d_col[jj];
			for (col = col_0 - d_col[jj]; col <= col_end; col++) {

				ii = col;
				if (gmt_x_out_of_bounds (GMT, &ii, Grid->header, wrap_180)) continue;	/* Outside x-range,  This call must happen AFTER gmt_y_out_of_bounds which sets wrap_180 */

				/* Here, (ii,jj) [both are >= 0] is index of a node (kk) inside the grid */
				colu = ii;

				distance = gmt_distance (GMT, x0[colu], y0[rowu], in[GMT_X], in[GMT_Y]);

				if (distance > Ctrl->S.radius) continue;	/* Data constraint is too far from this node */
				ij = gmt_M_ijp (Grid->header, rowu, colu);	/* Padding used for output grid */
				kk = gmt_M_ij0 (Grid->header, rowu, colu);	/* No padding used for helper arrays */
				dx = in[GMT_X] - x0[colu];	dy = in[GMT_Y] - y0[rowu];

				/* Check for wrap-around in x or y.  This should only occur if the
				   search radius is larger than 1/2 the grid width/height so that
				   the shortest distance is going through the periodic boundary.
				   For longitudes the dx obviously cannot exceed 180 (half_x_width)
				   since we could then go the other direction instead.
				*/
				if (HH->nxp && fabs (dx) > half_x_width) dx -= copysign (x_width, dx);
				if (HH->nyp && fabs (dy) > half_y_width) dy -= copysign (y_width, dy);

				/* OK, this point should affect this node.  */

				gmtcount_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij, kk, Ctrl->C.mode, in, weight);

				/* With periodic, gridline-registered grids there are duplicate rows and/or columns
				   so we may have to assign the point to more than one node.  The next section deals
				   with this situation.
				*/

				if (replicate_x) {	/* Must check if we have to replicate a column */
					if (colu == 0) 	/* Must replicate left to right column */
						gmtcount_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij+x_wrap, kk+x_wrap, Ctrl->C.mode, in, weight);
					else if (colu == HH->nxp)	/* Must replicate right to left column */
						gmtcount_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij-x_wrap, kk-x_wrap, Ctrl->C.mode, in, weight);
				}
				if (replicate_y) {	/* Must check if we have to replicate a row */
					if (rowu == 0)	/* Must replicate top to bottom row */
						gmtcount_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij+y_wrap_ij, kk+y_wrap_kk, Ctrl->C.mode, in, weight);
					else if (rowu == HH->nyp)	/* Must replicate bottom to top row */
						gmtcount_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij-y_wrap_ij, kk-y_wrap_kk, Ctrl->C.mode, in, weight);
				}
			}
		}
		if (!(n_read % 16384)) GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\r", n_read);	/* 16384 = 2^14 */
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\n", n_read);

	/* Finalize the statistical calculations, as needed */
	kk = 0;
	switch (Ctrl->C.mode) {
		case GMTCOUNT_MEAN: case GMTCOUNT_MEANW:	/* Compute [weighted] mean */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (w[kk] > 0.0) Grid->data[ij] /= w[kk], n++;
				kk++;
			}
			break;
		case GMTCOUNT_MEDIAN:	/* Compute plain medians */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_sort_array (GMT, zp[kk], k, GMT_FLOAT);
					Grid->data[ij] = (k%2) ? zp[kk][k/2] : 0.5 * (zp[kk][(k-1)/2] + zp[kk][row/2]);
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_MEDIANW:	/* Compute weighted medians */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_MODE:	/* Compute plain modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_mode_f (GMT, zp[kk], k, k/2, true, gmt_mode_selection, &GMT_n_multiples, &zmode);
					Grid->data[ij] = (gmt_grdfloat)zmode;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_MODEW:	/* Compute weighted modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_mode_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_QUANT:	/* Compute plain quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) gmt_quantile_f (GMT, zp[kk], Ctrl->C.quant, k);
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_QUANTW:	/* Compute weighted quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_quantile_weighted (GMT, zw_pair[kk], n_in_circle[kk], 0.01 * Ctrl->C.quant);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_IRANGE:	/* Compute plain inter quartile range */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) (gmt_quantile_f (GMT, zp[kk], 75.0, k) - gmt_quantile_f (GMT, zp[kk], 25.0, k));
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_IRANGEW:	/* Compute weighted quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) (gmt_quantile_weighted (GMT, zw_pair[kk], k, 0.75) - gmt_quantile_weighted (GMT, zw_pair[kk], k, 0.25));
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_COUNT: case GMTCOUNT_COUNTW: case GMTCOUNT_SUM:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (Grid->data[ij] > 0.0) n++;
			}
			break;
		case GMTCOUNT_ZRANGE:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (visited[kk]) {
					Grid->data[ij] -= s[kk];
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_STD:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk] > 1) {
					Grid->data[ij] = sqrt ((n_in_circle[kk] * s[kk] - Grid->data[ij] * Grid->data[ij]) / (n_in_circle[kk] * (n_in_circle[kk] - 1)));
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_STDW:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk] > 1) {
					Grid->data[ij] = sqrt ((w[kk] * s[kk] - Grid->data[ij] * Grid->data[ij]) / ((n_in_circle[kk] - 1) * w[kk] * w[kk] /n_in_circle[kk]));
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_RMS:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = sqrt (Grid->data[ij] / n_in_circle[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_RMSW:
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (w[kk] > 0.0) {
					Grid->data[ij] = sqrt (Grid->data[ij] / w[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_MAD:	/* COmpute plain MAD */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_sort_array (GMT, zp[kk], k, GMT_FLOAT);
					median = (k%2) ? zp[kk][k/2] : 0.5 * (zp[kk][(k-1)/2] + zp[kk][row/2]);
					gmt_getmad_f (GMT, zp[kk], k, median, &mad);
					Grid->data[ij] = mad;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_MADW:	/* Compute weighted MAD */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					median = gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					/* Compute the absolute deviations from this median */
					for (k = 0; k < n_in_circle[kk]; k++) zw_pair[kk][k].value = (gmt_grdfloat)fabs (zw_pair[kk][k].value - median);
					/* Find the weighted median absolute deviation */
					Grid->data[ij] = (gmt_grdfloat)gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_LMSSCL:	/* Compute plain mode lmsscale */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_mode_f (GMT, zp[kk], k, k/2, true, gmt_mode_selection, &GMT_n_multiples, &zmode);
					gmt_getmad_f (GMT, zp[kk], k, zmode, &mad);
					Grid->data[ij] = (gmt_grdfloat)mad;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTCOUNT_LMSSCLW:	/* Compute weighted modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					zmode = gmt_mode_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					for (k = 0; k < n_in_circle[kk]; k++) zw_pair[kk][k].value = (gmt_grdfloat)fabs (zw_pair[kk][k].value - zmode);
					Grid->data[ij] = (gmt_grdfloat)(MAD_NORMALIZE * gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]));
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		default:	/* Count the visits */
			for (kk = 0; kk < Grid->header->nm; kk++)
				if (visited[kk]) n++;
			break;
	}

	if (Ctrl->N.active) {	/* Area normalization */
		double scale = 1.0 / (M_PI * Ctrl->S.radius * Ctrl->S.radius);
		gmt_M_grd_loop (GMT, Grid, row, col, ij)
			Grid->data[ij] *= scale;
	}

	gmt_M_free (GMT, visited);
	gmt_M_free (GMT, w);
	gmt_M_free (GMT, s);
	gmt_M_free (GMT, zp);
	gmt_M_free (GMT, zw_pair);
	gmt_M_free (GMT, n_in_circle);
	gmt_M_free (GMT, n_alloc);
	gmt_M_free (GMT, d_col);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
		Return (API->error);
	}

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		char line[GMT_BUFSIZ];
		sprintf (line, "%s)\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " nodes were assigned a statistical value\n", n);
		(gmt_M_is_dnan (Ctrl->E.value)) ? GMT_Message (API, GMT_TIME_NONE, "NaN)\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->E.value);
	}

	Return (GMT_NOERROR);
}
