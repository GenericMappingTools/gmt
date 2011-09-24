/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2011 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
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

#include "x2sys.h"

#define EA_LAT "37:04:17.1660757541775"

/* Control structure for x2sys_binlist */

struct X2SYS_BINLIST_CTRL {
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
};

struct BINCROSS {
	double x, y, d;
};

void *New_x2sys_binlist_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_BINLIST_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_BINLIST_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_x2sys_binlist_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_BINLIST_CTRL *C) {	/* Deallocate control structure */
	if (C->T.TAG) free ((void *)C->T.TAG);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_binlist_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;
	
	GMT_message (GMT, "x2sys_binlist %s - Create bin index listing from track data files\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_binlist <files> -T<TAG> [-D] [-E] [%s]\n\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<files> is one or more datafiles, or give =<files.lis> for a file with a list of datafiles.\n");
	GMT_message (GMT, "\t-T <TAG> is the system tag for this compilation.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-D Calculate track-lengths per bin (see x2sys_init -C for method and -N for units).\n");
	GMT_message (GMT, "\t-E Bin tracks using equal-area bins (with -D only).\n");
	GMT_explain_options (GMT, "V");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_binlist_parse (struct GMTAPI_CTRL *C, struct X2SYS_BINLIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */
			
			case 'D':
				Ctrl->D.active = TRUE;
				break;
			case 'E':
				Ctrl->E.active = TRUE;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
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

int outside (double x, double y, struct X2SYS_BIX *B, GMT_LONG geo)
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

unsigned int get_data_flag (double *data[], GMT_LONG j, struct X2SYS_INFO *s)
{
	int i;
	unsigned int bit, flag;
	for (i = flag = 0, bit = 1; i < s->n_fields; i++, bit <<= 1) {
		if (GMT_is_dnan (data[i][j])) continue;	/* NaN, so no data here */
		flag |= bit;
	}
	return (flag);
}

int comp_bincross (const void *p1, const void *p2)
{
	struct BINCROSS *a = (struct BINCROSS *)p1, *b = (struct BINCROSS *)p2;

	if (a->d < b->d) return (-1);
	if (a->d > b->d) return (+1);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_binlist_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_x2sys_binlist (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	char **trk_name = NULL;

	GMT_LONG i, j, k, n_tracks, k1, ij, ii, jj, nx, bi, bj, start_i, end_i, jump_180, jump_360;
	GMT_LONG this_bin_i;	/* This i node for bin */
	GMT_LONG this_bin_j;	/* This j node for bin */
	GMT_LONG this_bin_ij;	/* This bin */
	GMT_LONG last_bin_i;	/* Previous i node for bin */
	GMT_LONG last_bin_j;	/* Previous j node for bin */
	GMT_LONG last_bin_ij;	/* Previous bin */
	GMT_LONG n_alloc = 0, nx_alloc = GMT_SMALL_CHUNK;
	GMT_LONG error = FALSE, gap, cmdline_files;

	unsigned int nav_flag;

	double **data = NULL, *dist_km = NULL, *dist_bin = NULL, dist_scale, x, y, dx, del_x, del_y, y_max = 90.0;

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_FILE_INFO p;		/* File information */
	struct X2SYS_BIX B;
	struct BINCROSS *X = NULL;
	struct X2SYS_BINLIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_x2sys_binlist_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_x2sys_binlist_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_binlist", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vf", ">", options))) Return (error);
	Ctrl = (struct X2SYS_BINLIST_CTRL *)New_x2sys_binlist_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_binlist_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_binlist main code ----------------------------*/

	if ((n_tracks = x2sys_get_tracknames (GMT, options, &trk_name, &cmdline_files)) == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "No datafiles given!\n");
		Return (EXIT_FAILURE);		
	}

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (Ctrl->E.active && !s->geographic) {
		GMT_report (GMT, GMT_MSG_FATAL, "-E requires geographic data; your TAG implies Cartesian\n");
		Return (EXIT_FAILURE);		
	}

	if (s->geographic) {
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
		GMT->current.io.geo.range = s->geodetic;
	}
	else
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_OUT][GMT_Z] = GMT_IS_FLOAT;
	
	MGD77_Set_Unit (GMT, s->unit[X2SYS_DIST_SELECTION], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */

	if (Ctrl->E.active) {
		double mid;
		char proj[80];
		/* Do the equal area map projection so W = 360 and H = 180 */
		if (!(GMT_IS_ZERO (B.wesn[XHI] - B.wesn[XLO] - 360.0) && GMT_IS_ZERO (B.wesn[YHI] - B.wesn[YLO] - 180.0))) {
			GMT_report (GMT, GMT_MSG_FATAL, "-E requires a global region (-Rg or -Rd)");
			Return (EXIT_FAILURE);
		}
		GMT->current.setting.proj_ellipsoid = GMT_get_ellipsoid (GMT, "Sphere");	/* Make sure we use a spherical projection */
		mid = 0.5 * (B.wesn[XHI] + B.wesn[XLO]);	/* Central longitude to use */
		GMT_report (GMT, GMT_MSG_NORMAL, "To undo equal-area projection, use -R%g/%g/%g/%g -JY%g/%s/360i\n", B.wesn[XLO], B.wesn[XHI], B.wesn[YLO], B.wesn[YHI], mid, EA_LAT);
		sprintf (proj, "Y%g/%s/360", mid, EA_LAT);
		GMT_parse_common_options (GMT, "J", 'J', proj);
		GMT_err_fail (GMT, GMT_map_setup (GMT, B.wesn), "");
		GMT_geo_to_xy (GMT, B.wesn[XLO], B.wesn[YLO], &B.wesn[XLO], &B.wesn[YLO]);
		GMT_geo_to_xy (GMT, B.wesn[XHI], B.wesn[YHI], &B.wesn[XHI], &B.wesn[YHI]);
		y_max = B.wesn[YHI];
	}
	
	x2sys_bix_init (GMT, &B, TRUE);
	nav_flag = (1 << s->x_col) + (1 << s->y_col);	/* For bins just cut by track but no points inside the bin */
	jump_180 = irint (180.0 / B.inc[GMT_X]);
	jump_360 = irint (360.0 / B.inc[GMT_X]);

	X = GMT_memory (GMT, NULL, nx_alloc, struct BINCROSS);
	
	if (Ctrl->D.active) {
		n_alloc = GMT_CHUNK;
		dist_bin = GMT_memory (GMT, NULL, B.nm_bin, double);
	}

	fprintf (GMT->session.std[GMT_OUT], "# %s\n", Ctrl->T.TAG);

	for (i = 0; i < n_tracks; i++) {

		GMT_report (GMT, GMT_MSG_NORMAL, "Reading file %s ", trk_name[i]);

		x2sys_err_fail (GMT, (s->read_file) (GMT, trk_name[i], &data, s, &p, &GMT->current.io, &j), trk_name[i]);
		GMT_report (GMT, GMT_MSG_NORMAL, "[%s]\n", s->path);
		
		if (Ctrl->E.active) {	/* Project coordinates */
			for (j = 0; j < p.n_rows; j++) GMT_geo_to_xy (GMT, data[s->x_col][j], data[s->y_col][j], &data[s->x_col][j], &data[s->y_col][j]);
		}
		
		/* Reset bin flags */

		GMT_memset (B.binflag, B.nm_bin, unsigned int);
		if (Ctrl->D.active) {
			GMT_memset (dist_bin, B.nm_bin, double);
			GMT_err_fail (GMT, GMT_dist_array (GMT, data[s->x_col], data[s->y_col], p.n_rows, dist_scale, -s->dist_flag, &dist_km), "");	/* -ve gives increments */
		}

		last_bin_ij = last_bin_i = last_bin_j = -1;
		for (j = 0; j < p.n_rows; j++) {
			if (outside (data[s->x_col][j], data[s->y_col][j], &B, s->geographic)) continue;
			 x2sys_err_fail (GMT, x2sys_bix_get_ij (GMT, data[s->x_col][j], data[s->y_col][j], &this_bin_i, &this_bin_j, &B, &this_bin_ij), "");

			/* While this may be the same bin as the last bin, the data available may have changed so we keep
			 * turning the data flags on again and again. */
			 
			B.binflag[this_bin_ij] |= get_data_flag (data, j, s);

			if (!Ctrl->D.active) continue;	/* Not worried about trackline lengths */
			
			if (last_bin_ij == -1) last_bin_ij = this_bin_ij;	/* Initialize last bin to this bin the first time */
			
			if (j > 0) { /* Can check for gaps starting with 1st to 2nd point */
				gap = FALSE;
				if (s->t_col >= 0) {	/* There is a time column in the data*/
					if (GMT_is_dnan (data[s->t_col][j])&& (dist_km[j] - dist_km[j-1]) > B.dist_gap) /* but time = NaN, so test for gaps based on distance */
						gap = TRUE;
			   		else if ((data[s->t_col][j] - data[s->t_col][j-1]) > B.time_gap)	/* We have a time data gap so we skip this interval */
						gap = TRUE;
				}
				else if ((dist_km[j] - dist_km[j-1]) > B.dist_gap) /* There is no time column, must test for gaps based on distance */
					gap = TRUE;
				
				if (gap) {
					last_bin_ij = this_bin_ij;	/* Update the last point's index info */
					last_bin_i = this_bin_i;
					last_bin_j = this_bin_j;
					continue;
				}
			}

			if (this_bin_ij == last_bin_ij) {	/* Same bin, keep adding up incremental distances (this adds 0 the very first time) */
				dist_bin[this_bin_ij] += dist_km[j];
			}
			else  {	/* Crossed into another bin */
				if (s->geographic && (this_bin_i - last_bin_i) > jump_180) {		/* Jumped from east to west across Greenwich */
					start_i = this_bin_i + 1;
					end_i = last_bin_i + jump_360;
					dx = (data[s->x_col][j] - 360.0) - data[s->x_col][j-1];
				}
				else if (s->geographic && (this_bin_i - last_bin_i) < -jump_180) {	/* Jumped from west to east across Greenwich */
					start_i = last_bin_i + 1;
					end_i = this_bin_i + jump_360;
					dx = data[s->x_col][j] - (data[s->x_col][j-1] - 360.0);
				}
				else {								/* Did no such thing */
					start_i = MIN (last_bin_i, this_bin_i) + 1;
					end_i = MAX (last_bin_i, this_bin_i);
					dx = data[s->x_col][j] - data[s->x_col][j-1];
				}
				
				/* Find all the bin-line intersections */
				
				/* Add the start and stop coordinates to the xc/yc arrays so we can get mid-points of the intervals */
				
				X[0].x = data[s->x_col][j-1];	X[0].y = data[s->y_col][j-1];	X[0].d = 0.0;
				X[1].x = data[s->x_col][j];	X[1].y = data[s->y_col][j];	X[1].d = hypot (dx, data[s->y_col][j] - data[s->y_col][j-1]);
				nx = 2;
				for (bj = MIN (last_bin_j, this_bin_j) + 1; bj <= MAX (last_bin_j, this_bin_j); bj++) {	/* If we go in here we know dy is non-zero */
					y = B.wesn[YLO] + bj * B.inc[GMT_Y];
					del_y = y - data[s->y_col][j-1];
					del_x = del_y * dx / (data[s->y_col][j] - data[s->y_col][j-1]);
					x = data[s->x_col][j-1] + del_x;
					X[nx].x = x;	X[nx].y = y;	X[nx].d = hypot (del_x , del_y);
					nx++;
					if (nx == nx_alloc) {
						nx_alloc <<= 1;
						X = GMT_memory (GMT, X, nx_alloc, struct BINCROSS);
					}
				}
				for (bi = start_i; bi <= end_i; bi++) {	/* If we go in here we think dx is non-zero (we do a last-ditch dx check just in case) */
					x = B.wesn[XLO] + bi * B.inc[GMT_X];
					if (s->geographic && x >= 360.0) x -= 360.0;
					del_x = x - data[s->x_col][j-1];
					if (fabs (del_x) > 180.0) del_x = copysign (360.0 - fabs (del_x), -del_x);
					del_y = (dx == 0.0) ? 0.5 * (data[s->y_col][j] - data[s->y_col][j-1]) : del_x * (data[s->y_col][j] - data[s->y_col][j-1]) / dx;
					y = data[s->y_col][j-1] + del_y;
					if (s->geographic && fabs (y) > y_max) {
						y = copysign (y_max, y);
						del_y = y - data[s->y_col][j-1];
					}
					X[nx].x = x;	X[nx].y = y;	X[nx].d = hypot (del_x, del_y);
					nx++;
					if (nx == nx_alloc) {
						nx_alloc <<= 1;
						X = GMT_memory (GMT, X, nx_alloc, struct BINCROSS);
					}
				}
				
				/* Here we have 1 or more intersections */
				
				qsort ((void *)X, (size_t)nx, sizeof (struct BINCROSS), comp_bincross);
				
				for (k = 1, k1 = 0; k < nx; k++, k1++) {	/* Process the intervals, getting mid-points and using that to get bin */
					dx = X[k].x - X[k1].x;
					if (s->geographic && dx < -180.0)
						x = 0.5 * (X[k].x + (X[k1].x - 360.0));
					else if (s->geographic && dx > +180.0)
						x = 0.5 * (X[k].x - 360.0 + X[k1].x);
					else
						x = 0.5 * (X[k].x + X[k1].x);
					y = 0.5 * (X[k].y + X[k1].y);
					if (s->geographic && fabs (y) > y_max) y = copysign (y_max, y);
					x2sys_err_fail (GMT, x2sys_bix_get_ij (GMT, x, y, &ii, &jj, &B, &ij), "");
					dist_bin[ij] += GMT_distance (GMT, X[k].x, X[k].y, X[k1].x, X[k1].y);
					B.binflag[ij] |= nav_flag;		/* Only update nav flags we have not been here already */
				}
			}
			last_bin_ij = this_bin_ij;
			last_bin_i = this_bin_i;
			last_bin_j = this_bin_j;
		}

		x2sys_free_data (GMT, data, s->n_fields, &p);

		/* Time for bin index output */

		fprintf (GMT->session.std[GMT_OUT], "> %s\n", trk_name[i]);
		for (ij = 0; ij < B.nm_bin; ij++) {
			if (B.binflag[ij] == 0) continue;
			x = B.wesn[XLO] + ((ij % B.nx_bin) + 0.5) * B.inc[GMT_X];
			y = B.wesn[YLO] + ((ij / B.nx_bin) + 0.5) * B.inc[GMT_Y];
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x, GMT_X);
			fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y, GMT_Y);
			fprintf (GMT->session.std[GMT_OUT], "%s%ld%s%u", GMT->current.setting.io_col_separator, ij, GMT->current.setting.io_col_separator, B.binflag[ij]);
			if (Ctrl->D.active) {
				fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], dist_bin[ij], GMT_Z);
			}
			fprintf (GMT->session.std[GMT_OUT], "\n");
		}

		if (Ctrl->D.active) GMT_free (GMT, dist_km);
	}

	GMT_free (GMT, X);
	x2sys_end (GMT, s);
	GMT_free (GMT, B.binflag);
	if (Ctrl->D.active) GMT_free (GMT, dist_bin);
	x2sys_free_list (GMT, trk_name, n_tracks);

	Return (GMT_OK);
}
