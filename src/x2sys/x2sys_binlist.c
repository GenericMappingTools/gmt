/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2015 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* x2sys_binlist will read one or several data files and dump their
 * contents to stdout in ascii or binary (double precision) mode.
 * Input data file formats are determined by the definition file
 * given by the -D option.
 *
 * Author:	Paul Wessel
 * Date:	15-JUN-2004
 * Version:	1.2, based on the spirit of the old xsystem code
 *
 */

#define THIS_MODULE_NAME	"x2sys_binlist"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Create bin index listing from track data files"
#define THIS_MODULE_KEYS	">TO"

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->V"

#define EA_LAT "37:04:17.1660757541775"

/* Control structure for x2sys_binlist */

struct X2SYS_BINLIST_CTRL {
	struct D {	/* -D */
		bool active;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct T {	/* -T */
		bool active;
		char *TAG;
	} T;
};

struct BINCROSS {
	double x, y, d;
};

void *New_x2sys_binlist_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_BINLIST_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_BINLIST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_x2sys_binlist_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_BINLIST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

int GMT_x2sys_binlist_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_binlist <files> -T<TAG> [-D] [-E] [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<files> is one or more datafiles, or give =<files.lis> for a file with a list of datafiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for this compilation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Calculate track-lengths per bin (see x2sys_init -C for method and -N for units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Bin tracks using equal-area bins (with -D only).\n");
	GMT_Option (API, "V,.");
	
	return (EXIT_FAILURE);
}

int GMT_x2sys_binlist_parse (struct GMT_CTRL *GMT, struct X2SYS_BINLIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */
			
			case 'D':
				Ctrl->D.active = true;
				break;
			case 'E':
				Ctrl->E.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->D.active, "Syntax error: -E requires -D\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int outside (double x, double y, struct X2SYS_BIX *B, int geo)
{
	if (y < B->wesn[YLO] || y > B->wesn[YHI]) return (1);
	if (geo) {	/* Geographic data with periodic longitudes */
		while (x < B->wesn[XLO]) x += 360.0;
		while (x > B->wesn[XHI]) x -= 360.0;
		if (x < B->wesn[XLO]) return (1);
	}
	else {	/* Plain Cartesian test */
		if (x < B->wesn[XLO] || x > B->wesn[XHI]) return (1);
	}
	return (0);	/* Inside */
}

unsigned int get_data_flag (double *data[], uint64_t j, struct X2SYS_INFO *s)
{
	unsigned int i, bit, flag;
	for (i = flag = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) {
		if (GMT_is_dnan (data[i][j])) continue;	/* NaN, so no data here */
		flag |= bit;
	}
	return (flag);
}

int comp_bincross (const void *p1, const void *p2)
{
	const struct BINCROSS *a = p1, *b = p2;

	if (a->d < b->d) return (-1);
	if (a->d > b->d) return (+1);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_binlist_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_binlist (void *V_API, int mode, void *args)
{
	char **trk_name = NULL, record[GMT_BUFSIZ] = {""}, text[GMT_LEN64] = {""};

	uint64_t this_bin_index, index, last_bin_index, row;
	unsigned int trk, curr_x_pt, prev_x_pt, n_tracks;
	int ii_notused, jj_notused, bcol, brow, start_col, end_col, jump_180, jump_360;
	int this_bin_col;	/* This col node for bin */
	int this_bin_row;	/* This row node for bin */
	int last_bin_col;	/* Previous col node for bin */
	int last_bin_row;	/* Previous row node for bin */
	int error = 0;
	bool gap, cmdline_files, last_not_set;
	size_t nx, nx_alloc = GMT_SMALL_CHUNK;
	
	unsigned int nav_flag;

	double **data = NULL, *dist_km = NULL, *dist_bin = NULL, dist_scale, x, y, dx, del_x, del_y, y_max = 90.0;

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_FILE_INFO p;		/* File information */
	struct X2SYS_BIX B;
	struct BINCROSS *X = NULL;
	struct X2SYS_BINLIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_x2sys_binlist_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_x2sys_binlist_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_x2sys_binlist_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_x2sys_binlist_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_binlist_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_binlist main code ----------------------------*/

	if ((n_tracks = x2sys_get_tracknames (GMT, options, &trk_name, &cmdline_files)) == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No datafiles given!\n");
		Return (EXIT_FAILURE);		
	}

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (Ctrl->E.active && !s->geographic) {
		GMT_Report (API, GMT_MSG_NORMAL, "-E requires geographic data; your TAG implies Cartesian\n");
		Return (EXIT_FAILURE);		
	}

	if (s->geographic) {
		GMT_set_geographic (GMT, GMT_OUT);
		GMT->current.io.geo.range = s->geodetic;
	}
	else
		GMT_set_cartesian (GMT, GMT_OUT);
	GMT->current.io.col_type[GMT_OUT][GMT_Z] = GMT_IS_FLOAT;
	
	MGD77_Set_Unit (GMT, s->unit[X2SYS_DIST_SELECTION], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */

	if (Ctrl->E.active) {
		double mid;
		char proj[80] = {""};
		/* Do the equal area map projection so W = 360 and H = 180 */
		if (!(doubleAlmostEqual (B.wesn[XHI] - B.wesn[XLO], 360.0)
					&& doubleAlmostEqualZero (B.wesn[YHI] - B.wesn[YLO], 180.0))) {
			GMT_Report (API, GMT_MSG_NORMAL, "-E requires a global region (-Rg or -Rd)");
			Return (EXIT_FAILURE);
		}
		GMT->current.setting.proj_ellipsoid = GMT_get_ellipsoid (GMT, "Sphere");	/* Make sure we use a spherical projection */
		mid = 0.5 * (B.wesn[XHI] + B.wesn[XLO]);	/* Central longitude to use */
		GMT_Report (API, GMT_MSG_VERBOSE, "To undo equal-area projection, use -R%g/%g/%g/%g -JY%g/%s/360i\n", B.wesn[XLO], B.wesn[XHI], B.wesn[YLO], B.wesn[YHI], mid, EA_LAT);
		sprintf (proj, "Y%g/%s/360", mid, EA_LAT);
		GMT_parse_common_options (GMT, "J", 'J', proj);
		GMT_err_fail (GMT, GMT_map_setup (GMT, B.wesn), "");
		GMT_geo_to_xy (GMT, B.wesn[XLO], B.wesn[YLO], &B.wesn[XLO], &B.wesn[YLO]);
		GMT_geo_to_xy (GMT, B.wesn[XHI], B.wesn[YHI], &B.wesn[XHI], &B.wesn[YHI]);
		y_max = B.wesn[YHI];
	}
	
	x2sys_bix_init (GMT, &B, true);
	nav_flag = (1 << s->x_col) + (1 << s->y_col);	/* For bins just cut by track but no points inside the bin */
	jump_180 = irint (180.0 / B.inc[GMT_X]);
	jump_360 = irint (360.0 / B.inc[GMT_X]);

	X = GMT_memory (GMT, NULL, nx_alloc, struct BINCROSS);
	
	if (Ctrl->D.active) {
		GMT_init_distaz (GMT, s->unit[X2SYS_DIST_SELECTION][0], s->dist_flag, GMT_MAP_DIST);
		dist_bin = GMT_memory (GMT, NULL, B.nm_bin, double);
	}

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	
	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, Ctrl->T.TAG);

	for (trk = 0; trk < n_tracks; trk++) {

		GMT_Report (API, GMT_MSG_VERBOSE, "Reading file %s ", trk_name[trk]);

		x2sys_err_fail (GMT, (s->read_file) (GMT, trk_name[trk], &data, s, &p, &GMT->current.io, &row), trk_name[trk]);
		GMT_Report (API, GMT_MSG_VERBOSE, "[%s]\n", s->path);
		
		if (Ctrl->E.active) {	/* Project coordinates */
			for (row = 0; row < p.n_rows; row++) GMT_geo_to_xy (GMT, data[s->x_col][row], data[s->y_col][row], &data[s->x_col][row], &data[s->y_col][row]);
		}
		
		/* Reset bin flags */

		GMT_memset (B.binflag, B.nm_bin, unsigned int);
		if (Ctrl->D.active) {
			int signed_flag = s->dist_flag;
			GMT_memset (dist_bin, B.nm_bin, double);
			if ((dist_km = GMT_dist_array_2 (GMT, data[s->x_col], data[s->y_col], p.n_rows, dist_scale, -signed_flag)) == NULL) GMT_err_fail (GMT, GMT_MAP_BAD_DIST_FLAG, "");	/* -ve gives increments */
		}

		last_bin_index = UINT_MAX;
		last_not_set = true;
		last_bin_col = last_bin_row = -1;
		for (row = 0; row < p.n_rows; row++) {
			if (outside (data[s->x_col][row], data[s->y_col][row], &B, s->geographic)) continue;
			 x2sys_err_fail (GMT, x2sys_bix_get_index (GMT, data[s->x_col][row], data[s->y_col][row], &this_bin_col, &this_bin_row, &B, &this_bin_index), "");

			/* While this may be the same bin as the last bin, the data available may have changed so we keep
			 * turning the data flags on again and again. */
			 
			B.binflag[this_bin_index] |= get_data_flag (data, row, s);

			if (!Ctrl->D.active) continue;	/* Not worried about trackline lengths */
			
			if (last_not_set) {	/* Initialize last bin to this bin the first time */
				last_bin_index = this_bin_index;
				last_not_set = false;
			}
			
			if (row > 0) { /* Can check for gaps starting with 1st to 2nd point */
				gap = false;
				if (s->t_col >= 0) {	/* There is a time column in the data*/
					if (GMT_is_dnan (data[s->t_col][row])&& (dist_km[row] - dist_km[row-1]) > B.dist_gap) /* but time = NaN, so test for gaps based on distance */
						gap = true;
			   		else if ((data[s->t_col][row] - data[s->t_col][row-1]) > B.time_gap)	/* We have a time data gap so we skip this interval */
						gap = true;
				}
				else if ((dist_km[row] - dist_km[row-1]) > B.dist_gap) /* There is no time column, must test for gaps based on distance */
					gap = true;
				
				if (gap) {
					last_bin_index = this_bin_index;	/* Update the last point's index info */
					last_bin_col = this_bin_col;
					last_bin_row = this_bin_row;
					continue;
				}
			}

			if (this_bin_index == last_bin_index) {	/* Same bin, keep adding up incremental distances (this adds 0 the very first time) */
				dist_bin[this_bin_index] += dist_km[row];
			}
			else  {	/* Crossed into another bin */
				if (s->geographic && (this_bin_col - last_bin_col) > jump_180) {		/* Jumped from east to west across Greenwich */
					start_col = this_bin_col + 1;
					end_col = last_bin_col + jump_360;
					dx = (data[s->x_col][row] - 360.0) - data[s->x_col][row-1];
				}
				else if (s->geographic && (this_bin_col - last_bin_col) < -jump_180) {	/* Jumped from west to east across Greenwich */
					start_col = last_bin_col + 1;
					end_col = this_bin_col + jump_360;
					dx = data[s->x_col][row] - (data[s->x_col][row-1] - 360.0);
				}
				else {								/* Did no such thing */
					start_col = MIN (last_bin_col, this_bin_col) + 1;
					end_col = MAX (last_bin_col, this_bin_col);
					dx = data[s->x_col][row] - data[s->x_col][row-1];
				}
				
				/* Find all the bin-line intersections */
				
				/* Add the start and stop coordinates to the xc/yc arrays so we can get mid-points of the intervals */
				
				X[0].x = data[s->x_col][row-1];	X[0].y = data[s->y_col][row-1];	X[0].d = 0.0;
				X[1].x = data[s->x_col][row];	X[1].y = data[s->y_col][row];	X[1].d = hypot (dx, data[s->y_col][row] - data[s->y_col][row-1]);
				nx = 2;
				for (brow = MIN (last_bin_row, this_bin_row) + 1; brow <= MAX (last_bin_row, this_bin_row); brow++) {	/* If we go in here we know dy is non-zero */
					y = B.wesn[YLO] + brow * B.inc[GMT_Y];
					del_y = y - data[s->y_col][row-1];
					del_x = del_y * dx / (data[s->y_col][row] - data[s->y_col][row-1]);
					x = data[s->x_col][row-1] + del_x;
					X[nx].x = x;	X[nx].y = y;	X[nx].d = hypot (del_x , del_y);
					nx++;
					if (nx == nx_alloc) {
						nx_alloc <<= 1;
						X = GMT_memory (GMT, X, nx_alloc, struct BINCROSS);
					}
				}
				for (bcol = start_col; bcol <= end_col; bcol++) {	/* If we go in here we think dx is non-zero (we do a last-ditch dx check just in case) */
					x = B.wesn[XLO] + bcol * B.inc[GMT_X];
					if (s->geographic && x >= 360.0) x -= 360.0;
					GMT_set_delta_lon (data[s->x_col][row-1], x, del_x);
					del_y = (dx == 0.0) ? 0.5 * (data[s->y_col][row] - data[s->y_col][row-1]) : del_x * (data[s->y_col][row] - data[s->y_col][row-1]) / dx;
					y = data[s->y_col][row-1] + del_y;
					if (s->geographic && fabs (y) > y_max) {
						y = copysign (y_max, y);
						del_y = y - data[s->y_col][row-1];
					}
					X[nx].x = x;	X[nx].y = y;	X[nx].d = hypot (del_x, del_y);
					nx++;
					if (nx == nx_alloc) {
						nx_alloc <<= 1;
						X = GMT_memory (GMT, X, nx_alloc, struct BINCROSS);
					}
				}
				
				/* Here we have 1 or more intersections */
				
				qsort (X, nx, sizeof (struct BINCROSS), comp_bincross);
				
				for (curr_x_pt = 1, prev_x_pt = 0; curr_x_pt < nx; curr_x_pt++, prev_x_pt++) {	/* Process the intervals, getting mid-points and using that to get bin */
					dx = X[curr_x_pt].x - X[prev_x_pt].x;
					if (s->geographic && dx < -180.0)
						x = 0.5 * (X[curr_x_pt].x + (X[prev_x_pt].x - 360.0));
					else if (s->geographic && dx > +180.0)
						x = 0.5 * (X[curr_x_pt].x - 360.0 + X[prev_x_pt].x);
					else
						x = 0.5 * (X[curr_x_pt].x + X[prev_x_pt].x);
					y = 0.5 * (X[curr_x_pt].y + X[prev_x_pt].y);
					if (s->geographic && fabs (y) > y_max) y = copysign (y_max, y);
					x2sys_err_fail (GMT, x2sys_bix_get_index (GMT, x, y, &ii_notused, &jj_notused, &B, &index), "");
					dist_bin[index] += GMT_distance (GMT, X[curr_x_pt].x, X[curr_x_pt].y, X[prev_x_pt].x, X[prev_x_pt].y);
					B.binflag[index] |= nav_flag;		/* Only update nav flags we have not been here already */
				}
			}
			last_bin_index = this_bin_index;
			last_bin_col = this_bin_col;
			last_bin_row = this_bin_row;
		}

		x2sys_free_data (GMT, data, s->n_fields, &p);

		/* Time for bin index output */

		GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, trk_name[trk]);
		for (index = 0; index < B.nm_bin; index++) {
			if (B.binflag[index] == 0) continue;
			x = B.wesn[XLO] + ((index % B.nx_bin) + 0.5) * B.inc[GMT_X];
			y = B.wesn[YLO] + ((index / B.nx_bin) + 0.5) * B.inc[GMT_Y];
			GMT_ascii_format_col (GMT, record, x, GMT_OUT, GMT_X);
			strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, y, GMT_OUT, GMT_Y);
			strcat (record, text);
			sprintf (text, "%s%" PRIu64 "%s%u", GMT->current.setting.io_col_separator, index, GMT->current.setting.io_col_separator, B.binflag[index]);
			strcat (record, text);
			if (Ctrl->D.active) {
				strcat (record, GMT->current.setting.io_col_separator);
				GMT_ascii_format_col (GMT, text, dist_bin[index], GMT_OUT, GMT_Z);
				strcat (record, text);
			}
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		}

		if (Ctrl->D.active) GMT_free (GMT, dist_km);
	}
	
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_free (GMT, X);
	x2sys_end (GMT, s);
	GMT_free (GMT, B.binflag);
	if (Ctrl->D.active) GMT_free (GMT, dist_bin);
	x2sys_free_list (GMT, trk_name, n_tracks);

	Return (GMT_OK);
}
