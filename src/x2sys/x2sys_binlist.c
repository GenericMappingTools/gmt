/*-----------------------------------------------------------------
 *
 *      Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* x2sys_binlist will read one or several data files and dump their
 * contents to stdout in ASCII or binary (double precision) mode.
 * Input data file formats are determined by the definition file
 * given by the -D option.
 *
 * Author:	Paul Wessel
 * Date:	15-JUN-2004
 * Version:	1.2, based on the spirit of the old xsystem code
 *
 */

#include "gmt_dev.h"
#include "mgd77/mgd77.h"
#include "x2sys.h"

#define THIS_MODULE_CLASSIC_NAME	"x2sys_binlist"
#define THIS_MODULE_MODERN_NAME	"x2sys_binlist"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Create bin index listing from track data files"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->V"

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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_BINLIST_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_BINLIST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_BINLIST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->T.TAG);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <files> -T<TAG> [-D] [-E] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<files> is one or more datafiles, or give =<files.lis> for a file with a list of datafiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for this compilation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Calculate track-lengths per bin (see x2sys_init -j for method and -N for units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Bin tracks using equal-area bins (with -D only).\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_BINLIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files[2] = {0, 0};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files since their paths depend on tag */
				n_files[GMT_IN]++;
				break;
			case '>':	/* Got named output file */
				n_files[GMT_OUT]++;
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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] == 0, "Syntax error: No track files given\n");
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: More than one output file given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->D.active, "Syntax error: -E requires -D\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int outside (double x, double y, struct X2SYS_BIX *B, int geo) {
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

GMT_LOCAL unsigned int get_data_flag (double *data[], uint64_t j, struct X2SYS_INFO *s) {
	unsigned int i, bit, flag;
	for (i = flag = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) {
		if (gmt_M_is_dnan (data[i][j])) continue;	/* NaN, so no data here */
		flag |= bit;
	}
	return (flag);
}

GMT_LOCAL int comp_bincross (const void *p1, const void *p2) {
	const struct BINCROSS *a = p1, *b = p2;

	if (a->d < b->d) return (-1);
	if (a->d > b->d) return (+1);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_binlist (void *V_API, int mode, void *args) {
	char **trk_name = NULL, record[GMT_BUFSIZ] = {""};

	uint64_t this_bin_index, index, last_bin_index, row, col, trk, n_tracks;
	unsigned int curr_x_pt, prev_x_pt;
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
	double out[5];

	struct GMT_RECORD *Out = NULL;
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_FILE_INFO p;		/* File information */
	struct X2SYS_BIX B;
	struct BINCROSS *X = NULL;
	struct X2SYS_BINLIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the x2sys_binlist main code ----------------------------*/

	if ((error = x2sys_get_tracknames (GMT, options, &trk_name, &cmdline_files)) <= 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No datafiles given!\n");
		Return (GMT_RUNTIME_ERROR);		
	}
	n_tracks = (uint64_t)error;
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (Ctrl->E.active && !s->geographic) {
		GMT_Report (API, GMT_MSG_NORMAL, "-E requires geographic data; your TAG implies Cartesian\n");
		x2sys_end (GMT, s);
		x2sys_free_list (GMT, trk_name, n_tracks);
		Return (GMT_RUNTIME_ERROR);		
	}

	if (s->geographic) {
		gmt_set_geographic (GMT, GMT_OUT);
		GMT->current.io.geo.range = s->geodetic;
	}
	else
		gmt_set_cartesian (GMT, GMT_OUT);
	gmt_set_column (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);
	gmt_set_column (GMT, GMT_OUT, 3, GMT_IS_FLOAT);
	if (Ctrl->D.active) gmt_set_column (GMT, GMT_OUT, 4, GMT_IS_FLOAT);
	/* Ensure we write integers for the next two columns */
	GMT->current.io.o_format[2] = GMT->current.io.o_format[3] = strdup ("%g"); 
	
	MGD77_Set_Unit (GMT, s->unit[X2SYS_DIST_SELECTION], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */

	if (Ctrl->E.active) {
		double mid;
		char proj[80] = {""};
		/* Do the equal area map projection so W = 360 and H = 180 */
		if (!(doubleAlmostEqual (B.wesn[XHI] - B.wesn[XLO], 360.0)
					&& doubleAlmostEqualZero (B.wesn[YHI] - B.wesn[YLO], 180.0))) {
			GMT_Report (API, GMT_MSG_NORMAL, "-E requires a global region (-Rg or -Rd)");
			x2sys_free_list (GMT, trk_name, n_tracks);
			x2sys_end (GMT, s);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT->current.setting.proj_ellipsoid = gmt_get_ellipsoid (GMT, "Sphere");	/* Make sure we use a spherical projection */
		mid = 0.5 * (B.wesn[XHI] + B.wesn[XLO]);	/* Central longitude to use */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE,
		            "To undo equal-area projection, use -R%g/%g/%g/%g -JY%g/%s/360i\n",
		            B.wesn[XLO], B.wesn[XHI], B.wesn[YLO], B.wesn[YHI], mid, EA_LAT);
		sprintf (proj, "Y%g/%s/360", mid, EA_LAT);
		gmt_parse_common_options (GMT, "J", 'J', proj);
		if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, B.wesn), "")) {
			x2sys_free_list (GMT, trk_name, n_tracks);
			x2sys_end (GMT, s);
			Return (GMT_PROJECTION_ERROR);
		}
		gmt_geo_to_xy (GMT, B.wesn[XLO], B.wesn[YLO], &B.wesn[XLO], &B.wesn[YLO]);
		gmt_geo_to_xy (GMT, B.wesn[XHI], B.wesn[YHI], &B.wesn[XHI], &B.wesn[YHI]);
		y_max = B.wesn[YHI];
	}
	
	x2sys_bix_init (GMT, &B, true);
	nav_flag = (1 << s->x_col) + (1 << s->y_col);	/* For bins just cut by track but no points inside the bin */
	jump_180 = irint (180.0 / B.inc[GMT_X]);
	jump_360 = irint (360.0 / B.inc[GMT_X]);

	X = gmt_M_memory (GMT, NULL, nx_alloc, struct BINCROSS);
	
	if (Ctrl->D.active) {
		gmt_init_distaz (GMT, s->unit[X2SYS_DIST_SELECTION][0], s->dist_flag, GMT_MAP_DIST);
		dist_bin = gmt_M_memory (GMT, NULL, B.nm_bin, double);
	}

	if ((error = GMT_Set_Columns (API, GMT_OUT, (Ctrl->D.active) ? 5 : 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		gmt_M_free (GMT, X);
		if (Ctrl->D.active) gmt_M_free (GMT, dist_bin);
		x2sys_free_list (GMT, trk_name, n_tracks);
		x2sys_end (GMT, s);
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		gmt_M_free (GMT, X);
		if (Ctrl->D.active) gmt_M_free (GMT, dist_bin);
		x2sys_free_list (GMT, trk_name, n_tracks);
		x2sys_end (GMT, s);
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		gmt_M_free (GMT, X);
		if (Ctrl->D.active) gmt_M_free (GMT, dist_bin);
		x2sys_free_list (GMT, trk_name, n_tracks);
		x2sys_end (GMT, s);
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		gmt_M_free (GMT, X);
		if (Ctrl->D.active) gmt_M_free (GMT, dist_bin);
		x2sys_free_list (GMT, trk_name, n_tracks);
		x2sys_end (GMT, s);
		Return (API->error);
	}
	gmt_set_tableheader (GMT, GMT_OUT, true);	/* Turn on -ho explicitly */
	gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
	sprintf (record, " %s", Ctrl->T.TAG);	/* Preserve the leading space for backwards compatibility */
	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	for (trk = 0; trk < n_tracks; trk++) {

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reading file %s ", trk_name[trk]);

		x2sys_err_fail (GMT, (s->read_file) (GMT, trk_name[trk], &data, s, &p, &GMT->current.io, &row), trk_name[trk]);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "[%s]\n", s->path);
		
		if (p.n_rows == 0) {
			GMT_Report (API, GMT_MSG_VERBOSE, "No data records found - skipping %s\n", trk_name[trk]);
			x2sys_free_data (GMT, data, s->n_fields, &p);
			continue;
		}
		
		if (Ctrl->E.active) {	/* Project coordinates */
			for (row = 0; row < p.n_rows; row++)
				gmt_geo_to_xy (GMT, data[s->x_col][row], data[s->y_col][row], &data[s->x_col][row], &data[s->y_col][row]);
		}
		
		/* Reset bin flags */

		gmt_M_memset (B.binflag, B.nm_bin, unsigned int);
		if (Ctrl->D.active) {
			int signed_flag = s->dist_flag;
			gmt_M_memset (dist_bin, B.nm_bin, double);
			if ((dist_km = gmt_dist_array_2 (GMT, data[s->x_col], data[s->y_col], p.n_rows, dist_scale, -signed_flag)) == NULL)
				gmt_M_err_fail (GMT, GMT_MAP_BAD_DIST_FLAG, "");	/* -ve gives increments */
		}

		last_bin_index = UINT_MAX;
		last_not_set = true;
		last_bin_col = last_bin_row = -1;
		for (row = 0; row < p.n_rows; row++) {
			if (outside (data[s->x_col][row], data[s->y_col][row], &B, s->geographic)) continue;
			x2sys_err_fail (GMT, x2sys_bix_get_index (GMT, data[s->x_col][row], data[s->y_col][row], &this_bin_col,
			                                          &this_bin_row, &B, &this_bin_index), "");

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
					if (gmt_M_is_dnan (data[s->t_col][row])&& (dist_km[row] - dist_km[row-1]) > B.dist_gap) /* but time = NaN, so test for gaps based on distance */
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
						X = gmt_M_memory (GMT, X, nx_alloc, struct BINCROSS);
					}
				}
				for (bcol = start_col; bcol <= end_col; bcol++) {	/* If we go in here we think dx is non-zero (we do a last-ditch dx check just in case) */
					x = B.wesn[XLO] + bcol * B.inc[GMT_X];
					if (s->geographic && x >= 360.0) x -= 360.0;
					gmt_M_set_delta_lon (data[s->x_col][row-1], x, del_x);
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
						X = gmt_M_memory (GMT, X, nx_alloc, struct BINCROSS);
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
					dist_bin[index] += gmt_distance (GMT, X[curr_x_pt].x, X[curr_x_pt].y, X[prev_x_pt].x, X[prev_x_pt].y);
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
			row = index / B.nx_bin;	/* To hold the row number */
			col = index % B.nx_bin;	/* To hold the col number */
			if (B.binflag[index] == 0) continue;
			out[GMT_X] = B.wesn[XLO] + (col + 0.5) * B.inc[GMT_X];
			out[GMT_Y] = B.wesn[YLO] + (row + 0.5) * B.inc[GMT_Y];
			out[GMT_Z] = (double)index;
			out[3] = B.binflag[index];
			if (Ctrl->D.active)
				out[4] = dist_bin[index];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}

		if (Ctrl->D.active) gmt_M_free (GMT, dist_km);
	}
	
	error = GMT_NOERROR;
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) 	/* Disables further data output */
		error = API->error;

	gmt_M_str_free (GMT->current.io.o_format[2]);
	GMT->current.io.o_format[3] = NULL; /* Since we only allocate one for sharing */

	gmt_M_free (GMT, Out);
	gmt_M_free (GMT, X);
	x2sys_end (GMT, s);
	gmt_M_free (GMT, B.binflag);
	if (Ctrl->D.active) gmt_M_free (GMT, dist_bin);
	x2sys_free_list (GMT, trk_name, n_tracks);

	Return (error);
}
