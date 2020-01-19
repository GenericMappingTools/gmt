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
 * API functions to support the grdcut application.
 *
 * Author:	Walter Smith, Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: Reads a grid file and writes a portion within it
 * to a new file.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdcut"
#define THIS_MODULE_MODERN_NAME	"grdcut"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract subregion from a grid"
#define THIS_MODULE_KEYS	"<G{,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-JRV"

/* Control structure for grdcut */

struct GRDCUT_CTRL {
	struct GRDCUT_In {
		bool active;
		char *file;
	} In;
	struct GRDCUT_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDCUT_N {	/* -N<nodata> */
		bool active;
		gmt_grdfloat value;
	} N;
	struct GRDCUT_S {	/* -S<lon>/<lat>/[-|=|+]<radius>[d|e|f|k|m|M|n][+n] */
		bool active;
		bool set_nan;
		int mode;	/* Could be negative */
		char unit;
		double lon, lat, radius;
	} S;
	struct GRDCUT_Z {	/* -Z[min/max][+n|N|r] */
		bool active;
		unsigned int mode;	/* 0-2, see below */
		double min, max;
	} Z;
};

#define NAN_IS_IGNORED	0
#define NAN_IS_INRANGE	1
#define NAN_IS_SKIPPED	2
#define NAN_IS_FRAME	3

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCUT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDCUT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->N.value = GMT->session.f_NaN;
	C->Z.min = -DBL_MAX;	C->Z.max = DBL_MAX;			/* No limits on z-range */
	C->Z.mode = NAN_IS_IGNORED;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> -G<outgrid> %s [%s] [-N[<nodata>]]\n\t[%s] [-S<lon>/<lat>/<radius>[+n]] [-Z[<min>/<max>][+n|N|r]] [%s] [%s]\n\n",
		name, GMT_Rgeo_OPT, GMT_J_OPT, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is file to extract a subset from.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t   Typically, the w/e/s/n you specify must be within the region of the input\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   grid.  If in doubt, run grdinfo first and check range of old file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, see -N below.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-J Specify oblique projection and compute corresponding rectangular\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   region that needs to be extracted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Allow grid to be extended if new -R exceeds existing boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append value to initialize nodes outside current region [Default is NaN].\n");
	GMT_Option (API, "V");
	gmt_dist_syntax (API->GMT, 'S', "Specify an origin and radius to find the corresponding rectangular area.");
	GMT_Message (API, GMT_TIME_NONE, "\t   All nodes on or inside the radius are contained in the subset grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to set all nodes in the subset outside the circle to NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Specify an optional range and determine the corresponding rectangular region\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   so that all nodes outside this region are outside the range [-inf/+inf].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to consider NaNs to be outside the range. The resulting grid will be NaN-free.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +N to strip off outside rows and cols that are all populated with NaNs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to consider NaNs to be within the range [Default just ignores NaNs in decision].\n");
	GMT_Option (API, "f,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	char za[GMT_LEN64] = {""}, zb[GMT_LEN64] = {""}, zc[GMT_LEN64] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */
		
			case 'G':	/* Output file */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'N':
				Ctrl->N.active = true;
				if (opt->arg[0])
					Ctrl->N.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (opt->arg);
				break;
 			case 'S':	/* Origin and radius */
				Ctrl->S.active = true;
				k = 0;
				if ((c = strstr (opt->arg, "+n"))) {
					Ctrl->S.set_nan = true;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if (opt->arg[k] == 'n' && gmt_M_compat_check (GMT, 5)) {
					Ctrl->S.set_nan = true;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", za, zb, zc) == 3) {
					n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, za, GMT_IS_LON, false, &Ctrl->S.lon), za);
					n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf_arg (GMT, zb, GMT_IS_LAT, false, &Ctrl->S.lat), zb);
					Ctrl->S.mode = gmt_get_distance (GMT, zc, &(Ctrl->S.radius), &(Ctrl->S.unit));
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'Z':	/* Detect region via z-range -Z[<min>/<max>][+n|N|r]*/
				Ctrl->Z.active = true;
				k = 0;
				if ((c = strstr (opt->arg, "+n")))
					Ctrl->Z.mode = NAN_IS_SKIPPED;
				else if ((c = strstr (opt->arg, "+N")))
					Ctrl->Z.mode = NAN_IS_FRAME;
				else if ((c = strstr (opt->arg, "+r")))
					Ctrl->Z.mode = NAN_IS_INRANGE;
				if (c) c[0] = '\0';	/* Chop off modifier */
				if (c == NULL && gmt_M_compat_check (GMT, 5)) {	/* Oldstyle -Zn|N|r[<min>/<max>] */
					if (opt->arg[k] == 'n') {
						Ctrl->Z.mode = NAN_IS_SKIPPED;
						k = 1;
					}
					else if (opt->arg[k] == 'N') {
						Ctrl->Z.mode = NAN_IS_FRAME;
						k = 1;
					}
					else if (opt->arg[k] == 'r') {
						Ctrl->Z.mode = NAN_IS_INRANGE;
						k = 1;
					}
				}
				if (sscanf (&opt->arg[k], "%[^/]/%s", za, zb) == 2) {
					if (!(za[0] == '-' && za[1] == '\0'))
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, za, gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->Z.min), za);
					if (!(zb[0] == '-' && zb[1] == '\0'))
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, zb, gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->Z.max), zb);
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, (GMT->common.R.active[RSET] + Ctrl->S.active + Ctrl->Z.active) != 1,
	                                   "Syntax error: Must specify only one of the -R, -S or the -Z options\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int count_NaNs (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int row0, unsigned int row1, unsigned int col0, unsigned int col1, unsigned int count[], unsigned int mode, unsigned int *side, bool *all) {
	/* Loop around current perimeter and count # of nans, return sum and pass back which side had most nans */
	unsigned int col, row, sum = 0, k, dim[4] = {0, 0, 0, 0};
	uint64_t node;

	gmt_M_memset (count, 4, unsigned int);	/* Reset count */
	dim[GMT_X] = col1 - col0 + 1;	/* Number of remaining nodes in x */
	dim[GMT_Y] = row1 - row0 + 1;	/* Number of remaining nodes in y */
	*side = 0;
	/* South count: */
	for (col = col0, node = gmt_M_ijp (G->header, row1, col); col <= col1; col++, node++)
		if (gmt_M_is_fnan (G->data[node])) count[0]++;
	/* East count: */
	for (row = row0, node = gmt_M_ijp (G->header, row, col1); row <= row1; row++, node += G->header->mx)
		if (gmt_M_is_fnan (G->data[node])) count[1]++;
	/* North count: */
	for (col = col0, node = gmt_M_ijp (G->header, row0, col); col <= col1; col++, node++)
		if (gmt_M_is_fnan (G->data[node])) count[2]++;
	/* West count: */
	for (row = row0, node = gmt_M_ijp (G->header, row, col0); row <= row1; row++, node += G->header->mx)
		if (gmt_M_is_fnan (G->data[node])) count[3]++;
	for (k = 0; k < 4; k++) {	/* Time to sum up and determine side with most NaNs */
		sum += count[k];
		if (mode == NAN_IS_FRAME) {
			if (k && count[k] > dim[*side]) *side = k;
		}
		else {
			if (k && count[k] > count[*side]) *side = k;
		}
	}
	*all = (count[*side] == dim[*side%2]);	/* True if every node along size is NaN */
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Nans found: W = %d E = %d S = %d N = %d\n",
	            count[3], count[1], count[0], count[2]);
	return ((row0 == row1 && col0 == col1) ? 0 : sum);	/* Return 0 if we run out of grid, else the sum */
}

GMT_LOCAL int set_rectangular_subregion (struct GMT_CTRL *GMT, double wesn[], double inc[]) {
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Default is to take the -R as given */
	if (GMT->common.R.oblique == false || GMT->current.proj.projection == GMT_NO_PROJ) return GMT_NOERROR;	/* Nothing else to do */

	/* Here we got an oblique area and a projection; find the corresponding rectangular -R that covers this oblique area */

	if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, wesn), "")) return (GMT_PROJECTION_ERROR);

	gmt_wesn_search (GMT, GMT->current.proj.rect[XLO], GMT->current.proj.rect[XHI], GMT->current.proj.rect[YLO],
	                 GMT->current.proj.rect[YHI], &GMT->common.R.wesn[XLO], &GMT->common.R.wesn[XHI],
					 &GMT->common.R.wesn[YLO], &GMT->common.R.wesn[YHI]);

	wesn[XLO] = floor (GMT->common.R.wesn[XLO] / inc[GMT_X]) * inc[GMT_X];
	wesn[XHI] = ceil  (GMT->common.R.wesn[XHI] / inc[GMT_X]) * inc[GMT_X];
	wesn[YLO] = floor (GMT->common.R.wesn[YLO] / inc[GMT_Y]) * inc[GMT_Y];
	wesn[YHI] = ceil  (GMT->common.R.wesn[YHI] / inc[GMT_Y]) * inc[GMT_Y];

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE) && rint (inc[GMT_X] * 60.0) == (inc[GMT_X] * 60.0)) {	/* Spacing in whole arc minutes */
		int w, e, s, n, wm, em, sm, nm;

		w = irint (floor (wesn[XLO]));	wm = irint ((wesn[XLO] - w) * 60.0);
		e = irint (floor (wesn[XHI]));	em = irint ((wesn[XHI] - e) * 60.0);
		s = irint (floor (wesn[YLO]));	sm = irint ((wesn[YLO] - s) * 60.0);
		n = irint (floor (wesn[YHI]));	nm = irint ((wesn[YHI] - n) * 60.0);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "%s -> -R%d:%02d/%d:%02d/%d:%02d/%d:%02d\n",
		            GMT->common.R.string, w, wm, e, em, s, sm, n, nm);
	}
	else if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE) && rint (inc[GMT_X] * 3600.0) == (inc[GMT_X] * 3600.0)) {	/* Spacing in whole arc seconds */
		int w, e, s, n, wm, em, sm, nm, ws, es, ss, ns;

		w = irint (floor (wesn[XLO]));	wm = irint (floor ((wesn[XLO] - w) * 60.0));
		ws = irint (floor ((wesn[XLO] - w - wm/60.0) * 3600.0));
		e = irint (floor (wesn[XHI]));	em = irint (floor ((wesn[XHI] - e) * 60.0));
		es = irint (floor ((wesn[XHI] - e - em/60.0) * 3600.0));
		s = irint (floor (wesn[YLO]));	sm = irint (floor ((wesn[YLO] - s) * 60.0));
		ss = irint (floor ((wesn[YLO] - s - sm/60.0) * 3600.0));
		n = irint (floor (wesn[YHI]));	nm = irint (floor ((wesn[YHI] - n) * 60.0));
		ns = irint (floor ((wesn[YHI] - n - nm/60.0) * 3600.0));
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "%s -> -R%d:%02d:%02d/%d:%02d:%02d/%d:%02d:%02d/%d:%02d:%02d\n",
		            GMT->common.R.string, w, wm, ws, e, em, es, s, sm, ss, n, nm, ns);
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "%s -> -R%g/%g/%g/%g\n",
		            GMT->common.R.string, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdcut (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int nx_old, ny_old, add_mode = 0U, side, extend, type = 0U, def_pad[4], pad[4];
	uint64_t node;
	bool outside[4] = {false, false, false, false}, all;

	char *name[2][4] = {{"left", "right", "bottom", "top"}, {"west", "east", "south", "north"}};

	double wesn_new[4], wesn_old[4], wesn_requested[4];
	double lon, lat, distance, radius;

	struct GMT_GRID_HEADER test_header;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GRDCUT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
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

	/*---------------------------- This is the grdcut main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid\n");
	if (Ctrl->Z.active) {	/* Must determine new region via -Z, so get entire grid first */
		unsigned int row0 = 0, row1 = 0, col0 = 0, col1 = 0, row, col, sum, side, count[4];
		bool go;
		struct GMT_GRID_HIDDEN *GH = NULL;
	
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get entire grid */
		}
		row1 = G->header->n_rows - 1;	col1 = G->header->n_columns - 1;
		if (Ctrl->Z.mode == NAN_IS_SKIPPED) {	/* Must scan in from outside to the inside, one side at the time, remove side with most Nans */
			sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_SKIPPED, &side, &all);	/* Initial border count */
			while (sum) {	/* Must eliminate the row or col with most NaNs, and move grid boundary inwards */
				if (side == 3 && col0 < col1) {	/* Need to move in from the left */
					col0++;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off a leftmost column\n");
				}
				else if (side == 1 && col1 > col0) {	/* Need to move in from the right */
					col1--;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off rightmost column\n");
				}
				else if (side == 0 && row1 > row0) {	/* Need to move up from the bottom */
					row1--;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off bottom row\n");
				}
				else if (side == 2 && row0 < row1) {	/* Need to move down from the top */
					row0++;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off top row\n");
				}
				sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_SKIPPED, &side, &all);
			}
			if (col0 == col1 || row0 == row1) {
				GMT_Report (API, GMT_MSG_NORMAL, "The sub-region implied by -Z+n is empty!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else if (Ctrl->Z.mode == NAN_IS_FRAME) {	/* Must scan in from outside to the inside, one side at the time, remove sides with all Nans */
			sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_FRAME, &side, &all);	/* Initial border count */
			while (all) {	/* Must eliminate the row or col with most NaNs, and move grid boundary inwards */
				if (side == 3 && col0 < col1) {	/* Need to move in from the left */
					col0++;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off a leftmost column\n");
				}
				else if (side == 1 && col1 > col0) {	/* Need to move in from the right */
					col1--;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off rightmost column\n");
				}
				else if (side == 0 && row1 > row0) {	/* Need to move up from the bottom */
					row1--;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off bottom row\n");
				}
				else if (side == 2 && row0 < row1) {	/* Need to move down from the top */
					row0++;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Stip off top row\n");
				}
				sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_FRAME, &side, &all);
			}
			if (col0 == col1 || row0 == row1) {
				GMT_Report (API, GMT_MSG_NORMAL, "The sub-region implied by -Z+N is empty!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		/* Here NaNs have either been skipped by inward search, or will be ignored (NAN_IS_IGNORED)
		   or will beconsider to be within range (NAN_IS_INRANGE) */
		for (row = row0, go = true; go && row <= row1; row++) {	/* Scan from ymax towards ymin */
			for (col = col0, node = gmt_M_ijp (G->header, row, 0); go && col <= col1; col++, node++) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row0 = row;	/* Found starting row */
			}
		}
		if (go) {
			GMT_Report (API, GMT_MSG_NORMAL, "The sub-region implied by -Z is empty!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		for (row = row1, go = true; go && row > row0; row--) {	/* Scan from ymin towards ymax */
			for (col = col0, node = gmt_M_ijp (G->header, row, 0); go && col <= col1; col++, node++) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row1 = row;	/* Found stopping row */
			}
		}
		for (col = col0, go = true; go && col <= col1; col++) {	/* Scan from xmin towards xmax */
			for (row = row0, node = gmt_M_ijp (G->header, row0, col); go && row <= row1; row++, node += G->header->mx) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col0 = col;	/* Found starting col */
			}
		}
		for (col = col1, go = true; go && col >= col0; col--) {	/* Scan from xmax towards xmin */
			for (row = row0, node = gmt_M_ijp (G->header, row0, col); go && row <= row1; row++, node += G->header->mx) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col1 = col;	/* Found stopping col */
			}
		}
		if (row0 == 0 && col0 == 0 && row1 == (G->header->n_rows-1) && col1 == (G->header->n_columns-1)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Your -Z limits produced no subset - output grid is identical to input grid\n");
			gmt_M_memcpy (wesn_new, G->header->wesn, 4, double);
		}
		else {	/* Adjust boundaries inwards */
			wesn_new[XLO] = G->header->wesn[XLO] + col0 * G->header->inc[GMT_X];
			wesn_new[XHI] = G->header->wesn[XHI] - (G->header->n_columns - 1 - col1) * G->header->inc[GMT_X];
			wesn_new[YLO] = G->header->wesn[YLO] + (G->header->n_rows - 1 - row1) * G->header->inc[GMT_Y];
			wesn_new[YHI] = G->header->wesn[YHI] - row0 * G->header->inc[GMT_Y];
		}
		GH = gmt_get_G_hidden (G);
		if (GH->alloc_mode == GMT_ALLOC_INTERNALLY) gmt_M_free_aligned (GMT, G->data);	/* Free the grid array only as we need the header below */
		add_mode = GMT_IO_RESET;	/* Pass this to allow reading the data again. */
	}
	else if (Ctrl->S.active) {	/* Must determine new region via -S, so only need header */
		int row, col;
		bool wrap;
	
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		if (gmt_M_is_cartesian (GMT, GMT_IN)) {
			GMT_Report (API, GMT_MSG_NORMAL, "The -S option requires a geographic grid\n");
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
		/* Set w/e to center and adjust in case of -/+ 360 stuff */
		wesn_new[XLO] = wesn_new[XHI] = Ctrl->S.lon;
		while (wesn_new[XLO] < G->header->wesn[XLO]) wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
		while (wesn_new[XLO] > G->header->wesn[XHI]) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
		wesn_new[YLO] = wesn_new[YHI] = Ctrl->S.lat;
		/* First adjust the S and N boundaries */
		radius = R2D * (Ctrl->S.radius / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.mean_radius;	/* Approximate radius in degrees */
		wesn_new[YLO] -= radius;	/* Approximate south limit in degrees */
		if (wesn_new[YLO] <= G->header->wesn[YLO]) {	/* Way south, reset to grid S limit */
			wesn_new[YLO] = G->header->wesn[YLO];
		}
		else {	/* Possibly adjust south a bit using chosen distance calculation */
			row = (int)gmt_M_grd_y_to_row (GMT, wesn_new[YLO], G->header);		/* Nearest row with this latitude */
			lat = gmt_M_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat -= G->header->inc[GMT_Y];
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YLO] = lat + (1.0 - G->header->xy_off) * G->header->inc[GMT_Y];	/* Go one back since last row was outside */
			if (wesn_new[YLO] <= G->header->wesn[YLO]) wesn_new[YLO] = G->header->wesn[YLO];
		}
		wesn_new[YHI] += radius;	/* Approximate north limit in degrees */
		if (wesn_new[YHI] >= G->header->wesn[YHI]) {	/* Way north, reset to grid N limit */
			wesn_new[YHI] = G->header->wesn[YHI];
		}
		else {	/* Possibly adjust north a bit using chosen distance calculation */
			row = (int)gmt_M_grd_y_to_row (GMT, wesn_new[YHI], G->header);		/* Nearest row with this latitude */
			lat = gmt_M_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat += G->header->inc[GMT_Y];
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YHI] = lat - (1.0 - G->header->xy_off) * G->header->inc[GMT_Y];	/* Go one back since last row was outside */
			if (wesn_new[YHI] >= G->header->wesn[YHI]) wesn_new[YHI] = G->header->wesn[YHI];
		}
		if (doubleAlmostEqual (wesn_new[YLO], -90.0) || doubleAlmostEqual (wesn_new[YHI], 90.0)) {	/* Need all longitudes when a pole is included */
			wesn_new[XLO] = G->header->wesn[XLO];
			wesn_new[XHI] = G->header->wesn[XHI];
		}
		else {	/* Determine longitude limits */
			wrap = gmt_M_360_range (G->header->wesn[XLO], G->header->wesn[XHI]);	/* true if grid is 360 global */
			radius /= cosd (Ctrl->S.lat);					/* Approximate e-w width in degrees longitude */
			wesn_new[XLO] -= radius;					/* Approximate west limit in degrees */
			if (!wrap && wesn_new[XLO] < G->header->wesn[XLO]) {		/* Outside non-periodic grid range */
				wesn_new[XLO] = G->header->wesn[XLO];
			}
			else {
				col = (int)gmt_M_grd_x_to_col (GMT, wesn_new[XLO], G->header);		/* Nearest col with this longitude */
				lon = gmt_M_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon -= G->header->inc[GMT_X];
					distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XLO] = lon + (1.0 - G->header->xy_off) * G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			wesn_new[XHI] += radius;					/* Approximate east limit in degrees */
			if (!wrap && wesn_new[XHI] > G->header->wesn[XHI]) {		/* Outside grid range */
				wesn_new[XHI] = G->header->wesn[XHI];
			}
			else {
				col = (int)gmt_M_grd_x_to_col (GMT, wesn_new[XHI], G->header);		/* Nearest col with this longitude */
				lon = gmt_M_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon += G->header->inc[GMT_X];
					distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XHI] = lon - (1.0 - G->header->xy_off) * G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			if (wesn_new[XHI] > 360.0) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
			if (wesn_new[XHI] < 0.0)   wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
		}
	}
	else {	/* Just the usual subset selection via -R.  First get the header */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		if (set_rectangular_subregion (GMT, wesn_new, G->header->inc)) {
			Return (API->error);	/* Get header only */
		}
	}

	gmt_M_memcpy (wesn_requested, wesn_new, 4, double);
	if (wesn_new[YLO] < G->header->wesn[YLO]) wesn_new[YLO] = G->header->wesn[YLO], outside[YLO] = true;
	if (wesn_new[YHI] > G->header->wesn[YHI]) wesn_new[YHI] = G->header->wesn[YHI], outside[YHI] = true;
	HH = gmt_get_H_hidden (G->header);

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Geographic data */
		if (wesn_new[XLO] < G->header->wesn[XLO] && wesn_new[XHI] < G->header->wesn[XLO]) {
			G->header->wesn[XLO] -= 360.0;
			G->header->wesn[XHI] -= 360.0;
		}
		if (wesn_new[XLO] > G->header->wesn[XHI] && wesn_new[XHI] > G->header->wesn[XHI]) {
			G->header->wesn[XLO] += 360.0;
			G->header->wesn[XHI] += 360.0;
		}
		if (!gmt_grd_is_global (GMT, G->header)) {
			if (wesn_new[XLO] < G->header->wesn[XLO]) wesn_new[XLO] = G->header->wesn[XLO], outside[XLO] = true;
			if (wesn_new[XHI] > G->header->wesn[XHI]) wesn_new[XHI] = G->header->wesn[XHI], outside[XHI] = true;
		}
		type = 1U;
	}
	else {
		if (wesn_new[XLO] < G->header->wesn[XLO]) wesn_new[XLO] = G->header->wesn[XLO], outside[XLO] = true;
		if (wesn_new[XHI] > G->header->wesn[XHI]) wesn_new[XHI] = G->header->wesn[XHI], outside[XHI] = true;
	}

	for (side = extend = 0; side < 4; side++) {
		if (!outside[side]) continue;
		extend++;
		if (Ctrl->N.active)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Requested subset exceeds data domain on the %s side - nodes in the extra area will be initialized to %g\n", name[type][side], Ctrl->N.value);
		else
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Requested subset exceeds data domain on the %s side - truncated to match grid bounds\n", name[type][side]);
	}

	/* Make sure output grid is kosher */

	gmt_adjust_loose_wesn (GMT, wesn_new, G->header);

	gmt_M_memcpy (test_header.wesn, wesn_new, 4, double);
	gmt_M_memcpy (test_header.inc, G->header->inc, 2, double);
	gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, &test_header, 1), Ctrl->G.file);

	/* OK, so far so good. Check if new wesn differs from old wesn by integer dx/dy */

	if (gmt_minmaxinc_verify (GMT, G->header->wesn[XLO], wesn_new[XLO], G->header->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new x_min do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, wesn_new[XHI], G->header->wesn[XHI], G->header->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new x_max do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, G->header->wesn[YLO], wesn_new[YLO], G->header->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new y_min do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, wesn_new[YHI], G->header->wesn[YHI], G->header->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new y_max do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}

	gmt_grd_init (GMT, G->header, options, true);

	gmt_M_memcpy (wesn_old, G->header->wesn, 4, double);
	nx_old = G->header->n_columns;		ny_old = G->header->n_rows;

	if (Ctrl->N.active && extend) {	/* Determine the pad needed for the extended area */
		gmt_M_memcpy (def_pad, GMT->current.io.pad, 4, unsigned int);	/* Default pad */
		gmt_M_memcpy (pad, def_pad, 4, unsigned int);			/* Starting pad */
		if (outside[XLO]) pad[XLO] += urint ((G->header->wesn[XLO] - wesn_requested[XLO]) * HH->r_inc[GMT_X]);
		if (outside[XHI]) pad[XHI] += urint ((wesn_requested[XHI] - G->header->wesn[XHI]) * HH->r_inc[GMT_X]);
		if (outside[YLO]) pad[YLO] += urint ((G->header->wesn[YLO] - wesn_requested[YLO]) * HH->r_inc[GMT_Y]);
		if (outside[YHI]) pad[YHI] += urint ((wesn_requested[YHI] - G->header->wesn[YHI]) * HH->r_inc[GMT_Y]);
		gmt_M_memcpy (GMT->current.io.pad, pad, 4, unsigned int);	/* Change default pad */
		if (!gmt_M_file_is_memory (Ctrl->In.file)) {	/* If a memory grid we end up duplicating below so only do this in the other cases */
			gmt_M_grd_setpad (GMT, G->header, pad);	/* Set the active pad before reading */
			gmt_set_grddim (GMT, G->header);	/* Update dimensions given the change of pad */
		}
	}
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY | add_mode, wesn_new, Ctrl->In.file, G) == NULL) {	/* Get subset (unless memory file) */
		Return (API->error);
	}
	if (gmt_M_file_is_memory (Ctrl->In.file) && (Ctrl->N.active || gmt_M_file_is_memory (Ctrl->G.file))) {
		/* Cannot manipulate the same grid in two different ways so we must make a duplicate of the input grid */
		struct GMT_GRID *G_dup = NULL;	/* For the duplicate; we eliminate any unnecessary padding using GMT_DUPLICATE_RESET */
		if ((G_dup = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA | GMT_DUPLICATE_RESET, G)) == NULL)
			Return (API->error);
		G = G_dup;	/* Since G was not allocated here anyway - it came from the outside and will be deleted there */
	}
	if (Ctrl->N.active && extend) {	/* Now shrink pad back to default and simultaneously extend region and apply nodata values */
		unsigned int xlo, xhi, ylo, yhi, row, col;
		gmt_M_memcpy (G->header->wesn, wesn_requested, 4, double);
		gmt_M_memcpy (GMT->current.io.pad, def_pad, 4, unsigned int);	/* Reset default pad */
		gmt_M_grd_setpad (GMT, G->header, GMT->current.io.pad);	/* Set the default pad */
		gmt_set_grddim (GMT, G->header);			/* Update dimensions given the change of wesn and pad */
		gmt_M_memcpy (wesn_new, wesn_requested, 4, double);	/* So reporting below is accurate */
		xlo = outside[XLO] ? (unsigned int)gmt_M_grd_x_to_col (GMT, wesn_old[XLO], G->header) : 0;
		xhi = outside[XHI] ? (unsigned int)gmt_M_grd_x_to_col (GMT, wesn_old[XHI], G->header) : G->header->n_columns - 1;
		ylo = outside[YLO] ? (unsigned int)gmt_M_grd_y_to_row (GMT, wesn_old[YLO], G->header) : G->header->n_rows - 1;
		yhi = outside[YHI] ? (unsigned int)gmt_M_grd_y_to_row (GMT, wesn_old[YHI], G->header) : 0;
		if (outside[XLO]) {
			for (row = 0; row < G->header->n_rows; row++)
				for (col = 0; col < xlo; col++) G->data[gmt_M_ijp(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[XHI]) {
			for (row = 0; row < G->header->n_rows; row++)
				for (col = xhi+1; col < G->header->n_columns; col++) G->data[gmt_M_ijp(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[YLO]) {
			for (row = ylo+1; row < G->header->n_rows; row++)
				for (col = xlo; col <= xhi; col++) G->data[gmt_M_ijp(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[YHI]) {
			for (row = 0; row < yhi; row++)
				for (col = xlo; col <= xhi; col++) G->data[gmt_M_ijp(G->header,row,col)] = Ctrl->N.value;
		}
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {
		char format[GMT_BUFSIZ];
		sprintf (format, "\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "File spec:\tW E S N dx dy n_columns n_rows:\n");
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Old:");
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, wesn_old[XLO], wesn_old[XHI], wesn_old[YLO],
		            wesn_old[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], nx_old, ny_old);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "New:");
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, wesn_new[XLO], wesn_new[XHI], wesn_new[YLO],
		            wesn_new[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], G->header->n_columns, G->header->n_rows);
	}

	if (Ctrl->S.set_nan) {	/* Set all nodes outside the circle to NaN */
		unsigned int row, col;
		uint64_t n_nodes = 0;
	
		for (row = 0; row < G->header->n_rows; row++) {
			for (col = 0; col < G->header->n_columns; col++) {
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, G->x[col], G->y[row]);
				if (distance > Ctrl->S.radius) {	/* Outside circle */
					node = gmt_M_ijp (G->header, row, col);
					G->data[node] = GMT->session.f_NaN;
					n_nodes++;
				}
			}
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Set %" PRIu64 " nodes outside circle to NaN\n", n_nodes);
	}

	/* Send the subset of the grid to the gridfile destination. */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
