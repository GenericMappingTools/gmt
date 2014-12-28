/*--------------------------------------------------------------------
 *	$Id$
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
 * API functions to support the grdcut application.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Reads a grid file and writes a portion within it
 * to a new file.
 */
 
#define THIS_MODULE_NAME	"grdcut"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract subregion from a grid"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-RVf"

/* Control structure for grdcontour */

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
		float value;
	} N;
	struct GRDCUT_S {	/* -S[n]<lon>/<lat>/[-|=|+]<radius>[d|e|f|k|m|M|n] */
		bool active;
		bool set_nan;
		int mode;	/* Could be negative */
		char unit;
		double lon, lat, radius;
	} S;
	struct GRDCUT_Z {	/* -Z[min/max] */
		bool active;
		unsigned int mode;	/* 0-2, see below */
		double min, max;
	} Z;
};

#define NAN_IS_IGNORED	0
#define NAN_IS_INRANGE	1
#define NAN_IS_SKIPPED	2

void *New_grdcut_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCUT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDCUT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->N.value = GMT->session.f_NaN;
	C->Z.min = -DBL_MAX;	C->Z.max = DBL_MAX;			/* No limits on z-range */
	C->Z.mode = NAN_IS_IGNORED;
	return (C);
}

void Free_grdcut_Ctrl (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdcut_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdcut <ingrid> -G<outgrid> %s [-N[<nodata>]]\n\t[%s] [-S[n]<lon>/<lat>/<radius>] [-Z[n|r][<min>/<max>]] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is file to extract a subset from.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t   The WESN you specify must be within the WESN of the input grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If in doubt, run grdinfo first and check range of old file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, see -N below.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Allow grid to be extended if new -R exceeds existing boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append value to initialize nodes outside current region [Default is NaN].\n");
	GMT_Option (API, "V");
	GMT_dist_syntax (API->GMT, 'S', "Specify an origin and radius to find the corresponding rectangular area.");
	GMT_Message (API, GMT_TIME_NONE, "\t   All nodes on or inside the radius are contained in the subset grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Sn to set all nodes in the subset outside the circle to NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Specify a range and determine the corresponding rectangular region so that\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   all values outside this region are outside the range [-inf/+inf].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Zn to consider NaNs to be outside the range.  The resulting grid will be NaN-free.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Zr to consider NaNs inrange instead [Default just ignores NaNs in decision].\n");
	GMT_Option (API, "f,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdcut_parse (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	char za[GMT_LEN64] = {""}, zb[GMT_LEN64] = {""}, zc[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */
			
 			case 'G':	/* Output file */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'N':
				Ctrl->N.active = true;
				if (opt->arg[0])
					Ctrl->N.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.f_NaN : (float)atof (opt->arg);
				break;
 			case 'S':	/* Origin and radius */
				Ctrl->S.active = true;
				k = 0;
				if (opt->arg[k] == 'n') {
					Ctrl->S.set_nan = true;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", za, zb, zc) == 3) {
					n_errors += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf_arg (GMT, za, GMT_IS_LON, &Ctrl->S.lon), za);
					n_errors += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf_arg (GMT, zb, GMT_IS_LAT, &Ctrl->S.lat), zb);
					Ctrl->S.mode = GMT_get_distance (GMT, zc, &(Ctrl->S.radius), &(Ctrl->S.unit));
				}
				break;
			case 'Z':	/* Detect region via z-range */
				Ctrl->Z.active = true;
				k = 0;
				if (opt->arg[k] == 'n') {
					Ctrl->Z.mode = NAN_IS_SKIPPED;
					k = 1;
				}
				else if (opt->arg[k] == 'r') {
					Ctrl->Z.mode = NAN_IS_INRANGE;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%s", za, zb) == 2) {
					if (!(za[0] == '-' && za[1] == '\0')) n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, za, GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->Z.min), za);
					if (!(zb[0] == '-' && zb[1] == '\0')) n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, zb, GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->Z.max), zb);
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, (GMT->common.R.active + Ctrl->S.active + Ctrl->Z.active) != 1, "Syntax error: Must specify only one of the -R, -S or the -Z options\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

unsigned int count_NaNs (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int row0, unsigned int row1, unsigned int col0, unsigned int col1, unsigned int count[], unsigned int *side)
{	/* Looo around current perimeter and count # of nans, return sum and pass back which side had most nans */
	unsigned int col, row, sum = 0, k;
	uint64_t node;
	
	GMT_memset (count, 4, unsigned int);	/* Reset count */
	*side = 0;
	/* South count: */
	for (col = col0, node = GMT_IJP (G->header, row1, col); col <= col1; col++, node++) if (GMT_is_fnan (G->data[node])) count[0]++;
	/* East count: */
	for (row = row0, node = GMT_IJP (G->header, row, col1); row <= row1; row++, node += G->header->mx) if (GMT_is_fnan (G->data[node])) count[1]++;
	/* North count: */
	for (col = col0, node = GMT_IJP (G->header, row0, col); col <= col1; col++, node++) if (GMT_is_fnan (G->data[node])) count[2]++;
	/* West count: */
	for (row = row0, node = GMT_IJP (G->header, row, col0); row <= row1; row++, node += G->header->mx) if (GMT_is_fnan (G->data[node])) count[3]++;
	for (k = 0; k < 4; k++) {	/* TIme to sum up and determine side with most NaNs */
		sum += count[k];
		if (k && count[k] > count[*side]) *side = k;
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Nans found: W = %d E = %d S = %d N = %d\n", count[3], count[1], count[0], count[2]);
	return ((row0 == row1 && col0 == col1) ? 0 : sum);	/* Return 0 if we run out of grid, else the sum */
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdcut_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdcut (void *V_API, int mode, void *args)
{
	int error = 0;
	unsigned int nx_old, ny_old, add_mode = 0U, side, extend, type = 0U, def_pad[4], pad[4];
	uint64_t node;
	bool outside[4] = {false, false, false, false};
	
	char *name[2][4] = {{"left", "right", "bottom", "top"}, {"west", "east", "south", "north"}};

	double wesn_new[4], wesn_old[4], wesn_requested[4];
	double lon, lat, distance, radius;

	struct GMT_GRID_HEADER test_header;
	struct GRDCUT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdcut_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdcut_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdcut_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdcut_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdcut_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdcut main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	if (Ctrl->Z.active) {	/* Must determine new region via -Z, so get entire grid first */
		unsigned int row0 = 0, row1 = 0, col0 = 0, col1 = 0, row, col, sum, side, count[4];
		bool go;
		
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get entire grid */
		}
		row1 = G->header->ny - 1;	col1 = G->header->nx - 1;
		if (Ctrl->Z.mode == NAN_IS_SKIPPED) {	/* Must scan in from outside to the inside, one side at the time, remove side with most Nans */
			sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, &side);	/* Initial border count */
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
				sum = count_NaNs (GMT, G, row0, row1, col0, col1, count, &side);
			}
			if (col0 == col1 || row0 == row1) {
				GMT_Report (API, GMT_MSG_NORMAL, "The sub-region implied by -Zn is empty!\n");
				Return (EXIT_FAILURE);
			}
		}
		/* Here NaNs have either been skipped by inward search, or will be ignored (NAN_IS_IGNORED) or will beconsider to be within range (NAN_IS_INRANGE) */
		for (row = row0, go = true; go && row <= row1; row++) {	/* Scan from ymax towards ymin */
			for (col = col0, node = GMT_IJP (G->header, row, 0); go && col <= col1; col++, node++) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row0 = row;	/* Found starting row */
			}
		}
		if (go) {
			GMT_Report (API, GMT_MSG_NORMAL, "The sub-region implied by -Z is empty!\n");
			Return (EXIT_FAILURE);
		}
		for (row = row1, go = true; go && row > row0; row--) {	/* Scan from ymin towards ymax */
			for (col = col0, node = GMT_IJP (G->header, row, 0); go && col <= col1; col++, node++) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row1 = row;	/* Found stopping row */
			}
		}
		for (col = col0, go = true; go && col <= col1; col++) {	/* Scan from xmin towards xmax */
			for (row = row0, node = GMT_IJP (G->header, row0, col); go && row <= row1; row++, node += G->header->mx) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col0 = col;	/* Found starting col */
			}
		}
		for (col = col1, go = true; go && col >= col0; col--) {	/* Scan from xmax towards xmin */
			for (row = row0, node = GMT_IJP (G->header, row0, col); go && row <= row1; row++, node += G->header->mx) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col1 = col;	/* Found stopping col */
			}
		}
		if (row0 == 0 && col0 == 0 && row1 == (G->header->ny-1) && col1 == (G->header->nx-1)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Your -Z limits produced no subset - output grid is identical to input grid\n");
			GMT_memcpy (wesn_new, G->header->wesn, 4, double);
		}
		else {	/* Adjust boundaries inwards */
			wesn_new[XLO] = G->header->wesn[XLO] + col0 * G->header->inc[GMT_X];
			wesn_new[XHI] = G->header->wesn[XHI] - (G->header->nx - 1 - col1) * G->header->inc[GMT_X];
			wesn_new[YLO] = G->header->wesn[YLO] + (G->header->ny - 1 - row1) * G->header->inc[GMT_Y];
			wesn_new[YHI] = G->header->wesn[YHI] - row0 * G->header->inc[GMT_Y];
		}
		GMT_free_aligned (GMT, G->data);	/* Free the grid array only as we need the header below */
		add_mode = GMT_IO_RESET;	/* Pass this to allow reading the data again. */
	}
	else if (Ctrl->S.active) {	/* Must determine new region via -S, so only need header */
		int row, col;
		bool wrap;
		
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		if (!GMT_is_geographic (GMT, GMT_IN)) {
			GMT_Report (API, GMT_MSG_NORMAL, "The -S option requires a geographic grid\n");
			Return (EXIT_FAILURE);
		}
		GMT_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
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
			row = (int)GMT_grd_y_to_row (GMT, wesn_new[YLO], G->header);		/* Nearest row with this latitude */
			lat = GMT_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat -= G->header->inc[GMT_Y];
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YLO] = lat + (1.0 - G->header->xy_off) * G->header->inc[GMT_Y];	/* Go one back since last row was outside */
			if (wesn_new[YLO] <= G->header->wesn[YLO]) wesn_new[YLO] = G->header->wesn[YLO];
		}
		wesn_new[YHI] += radius;	/* Approximate north limit in degrees */
		if (wesn_new[YHI] >= G->header->wesn[YHI]) {	/* Way north, reset to grid N limit */
			wesn_new[YHI] = G->header->wesn[YHI];
		}
		else {	/* Possibly adjust north a bit using chosen distance calculation */
			row = (int)GMT_grd_y_to_row (GMT, wesn_new[YHI], G->header);		/* Nearest row with this latitude */
			lat = GMT_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat += G->header->inc[GMT_Y];
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YHI] = lat - (1.0 - G->header->xy_off) * G->header->inc[GMT_Y];	/* Go one back since last row was outside */
			if (wesn_new[YHI] >= G->header->wesn[YHI]) wesn_new[YHI] = G->header->wesn[YHI];
		}
		if (doubleAlmostEqual (wesn_new[YLO], -90.0) || doubleAlmostEqual (wesn_new[YHI], 90.0)) {	/* Need all longitudes when a pole is included */
			wesn_new[XLO] = G->header->wesn[XLO];
			wesn_new[XHI] = G->header->wesn[XHI];
		}
		else {	/* Determine longitude limits */
			wrap = GMT_360_RANGE (G->header->wesn[XLO], G->header->wesn[XHI]);	/* true if grid is 360 global */
			radius /= cosd (Ctrl->S.lat);					/* Approximate e-w width in degrees longitude */
			wesn_new[XLO] -= radius;					/* Approximate west limit in degrees */
			if (!wrap && wesn_new[XLO] < G->header->wesn[XLO]) {		/* Outside non-periodic grid range */
				wesn_new[XLO] = G->header->wesn[XLO];
			}
			else {
				col = (int)GMT_grd_x_to_col (GMT, wesn_new[XLO], G->header);		/* Nearest col with this longitude */
				lon = GMT_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon -= G->header->inc[GMT_X];
					distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XLO] = lon + (1.0 - G->header->xy_off) * G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			wesn_new[XHI] += radius;					/* Approximate east limit in degrees */
			if (!wrap && wesn_new[XHI] > G->header->wesn[XHI]) {		/* Outside grid range */
				wesn_new[XHI] = G->header->wesn[XHI];
			}
			else {
				col = (int)GMT_grd_x_to_col (GMT, wesn_new[XHI], G->header);		/* Nearest col with this longitude */
				lon = GMT_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon += G->header->inc[GMT_X];
					distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XHI] = lon - (1.0 - G->header->xy_off) * G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			if (wesn_new[XHI] > 360.0) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
			if (wesn_new[XHI] < 0.0)   wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
		}
	}
	else {	/* Just the usual subset selection via -R.  First get the header */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		GMT_memcpy (wesn_new, GMT->common.R.wesn, 4, double);
	}
	
	GMT_memcpy (wesn_requested, wesn_new, 4, double);
	if (wesn_new[YLO] < G->header->wesn[YLO]) wesn_new[YLO] = G->header->wesn[YLO], outside[YLO] = true;
	if (wesn_new[YHI] > G->header->wesn[YHI]) wesn_new[YHI] = G->header->wesn[YHI], outside[YHI] = true;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Geographic data */
		if (wesn_new[XLO] < G->header->wesn[XLO] && wesn_new[XHI] < G->header->wesn[XLO]) {
			G->header->wesn[XLO] -= 360.0;
			G->header->wesn[XHI] -= 360.0;
		}
		if (wesn_new[XLO] > G->header->wesn[XHI] && wesn_new[XHI] > G->header->wesn[XHI]) {
			G->header->wesn[XLO] += 360.0;
			G->header->wesn[XHI] += 360.0;
		}
		if (!GMT_grd_is_global (GMT, G->header)) {
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
			GMT_Report (API, GMT_MSG_VERBOSE, "Requested subset exceeds data domain on the %s side - nodes in the extra area will be initialized to %g\n", name[type][side], Ctrl->N.value);
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Requested subset exceeds data domain on the %s side - truncated to match grid bounds\n", name[type][side]);
	}

	/* Make sure output grid is kosher */

	GMT_adjust_loose_wesn (GMT, wesn_new, G->header);

	GMT_memcpy (test_header.wesn, wesn_new, 4, double);
	GMT_memcpy (test_header.inc, G->header->inc, 2, double);
	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, &test_header, 1), Ctrl->G.file);

	/* OK, so far so good. Check if new wesn differs from old wesn by integer dx/dy */

	if (GMT_minmaxinc_verify (GMT, G->header->wesn[XLO], wesn_new[XLO], G->header->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new x_min do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, wesn_new[XHI], G->header->wesn[XHI], G->header->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new x_max do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, G->header->wesn[YLO], wesn_new[YLO], G->header->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new y_min do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, wesn_new[YHI], G->header->wesn[YHI], G->header->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Old and new y_max do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_grd_init (GMT, G->header, options, true);

	GMT_memcpy (wesn_old, G->header->wesn, 4, double);
	nx_old = G->header->nx;		ny_old = G->header->ny;
	
	if (Ctrl->N.active && extend) {	/* Determine the pad needed for the extended area */
		GMT_memcpy (def_pad, GMT->current.io.pad, 4, unsigned int);	/* Default pad */
		GMT_memcpy (pad, def_pad, 4, unsigned int);			/* Starting pad */
		if (outside[XLO]) pad[XLO] += urint ((G->header->wesn[XLO] - wesn_requested[XLO]) * G->header->r_inc[GMT_X]);
		if (outside[XHI]) pad[XHI] += urint ((wesn_requested[XHI] - G->header->wesn[XHI]) * G->header->r_inc[GMT_X]);
		if (outside[YLO]) pad[YLO] += urint ((G->header->wesn[YLO] - wesn_requested[YLO]) * G->header->r_inc[GMT_Y]);
		if (outside[YHI]) pad[YHI] += urint ((wesn_requested[YHI] - G->header->wesn[YHI]) * G->header->r_inc[GMT_Y]);
		GMT_grd_setpad (GMT, G->header, pad);	/* Set the active pad */
		GMT_memcpy (GMT->current.io.pad, pad, 4, unsigned int);	/* Change default pad */
		GMT_set_grddim (GMT, G->header);	/* Update dimensions given the change of pad */
	}
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY | add_mode, wesn_new, Ctrl->In.file, G) == NULL) {	/* Get subset */
		Return (API->error);
	}
	if (Ctrl->N.active && extend) {	/* Now shrink pad back to default and simultaneously extend region and apply nodata values */
		unsigned int xlo, xhi, ylo, yhi, row, col;
		GMT_memcpy (G->header->wesn, wesn_requested, 4, double);
		GMT_memcpy (GMT->current.io.pad, def_pad, 4, unsigned int);	/* Reset default pad */
		GMT_grd_setpad (GMT, G->header, GMT->current.io.pad);	/* Set the default pad */
		GMT_set_grddim (GMT, G->header);			/* Update dimensions given the change of wesn and pad */
		GMT_memcpy (wesn_new, wesn_requested, 4, double);	/* So reporting below is accurate */
		xlo = GMT_grd_x_to_col (GMT, wesn_old[XLO], G->header);
		xhi = GMT_grd_x_to_col (GMT, wesn_old[XHI], G->header);
		ylo = GMT_grd_y_to_row (GMT, wesn_old[YLO], G->header);
		yhi = GMT_grd_y_to_row (GMT, wesn_old[YHI], G->header);
		if (outside[XLO]) {
			for (row = 0; row < G->header->ny; row++) for (col = 0; col < xlo; col++) G->data[GMT_IJP(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[XHI]) {
			for (row = 0; row < G->header->ny; row++) for (col = xhi+1; col < G->header->nx; col++) G->data[GMT_IJP(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[YLO]) {
			for (row = ylo+1; row < G->header->ny; row++) for (col = xlo; col <= xhi; col++) G->data[GMT_IJP(G->header,row,col)] = Ctrl->N.value;
		}
		if (outside[YHI]) {
			for (row = 0; row < yhi; row++) for (col = xlo; col <= xhi; col++) G->data[GMT_IJP(G->header,row,col)] = Ctrl->N.value;
		}
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char format[GMT_BUFSIZ];
		sprintf (format, "\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, "File spec:\tW E S N dx dy nx ny:\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "Old:");
		GMT_Report (API, GMT_MSG_VERBOSE, format, wesn_old[XLO], wesn_old[XHI], wesn_old[YLO], wesn_old[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], nx_old, ny_old);
		GMT_Report (API, GMT_MSG_VERBOSE, "New:");
		GMT_Report (API, GMT_MSG_VERBOSE, format, wesn_new[XLO], wesn_new[XHI], wesn_new[YLO], wesn_new[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], G->header->nx, G->header->ny);
	}

	if (Ctrl->S.set_nan) {	/* Set all nodes outside the circle to NaN */
		unsigned int row, col;
		uint64_t n_nodes = 0;
		double *grd_lon = GMT_grd_coord (GMT, G->header, GMT_X);
		
		for (row = 0; row < G->header->ny; row++) {
			lat = GMT_grd_row_to_y (GMT, row, G->header);
			for (col = 0; col < G->header->nx; col++) {
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, grd_lon[col], lat);
				if (distance > Ctrl->S.radius) {	/* Outside circle */
					node = GMT_IJP (G->header, row, col);
					G->data[node] = GMT->session.f_NaN;
					n_nodes++;
				}
			}
		}
		GMT_free (GMT, grd_lon);	
		GMT_Report (API, GMT_MSG_VERBOSE, "Set %" PRIu64 " nodes outside circle to NaN\n", n_nodes);
	}
	
	/* Send the subset of the grid to the destination. */
	
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G) != GMT_OK) {
		Return (API->error);
	}

	Return (GMT_OK);
}
