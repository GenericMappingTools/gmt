/*--------------------------------------------------------------------
 *	$Id: nearneighbor_func.c,v 1.8 2011-04-19 19:10:44 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: Based on a specified grid size, nearneighbor reads an xyz file and
 * determines the nearest points to each node in sectors.  The default
 * looks for the nearest point for each quadrant.  The points must also
 * be within a maximum search-radius from the node.  For the nodes that
 * have a full set of nearest neighbors, a weighted average value is
 * computed.  New feature is full support for boundary conditions so
 * that geographic or periodic conditions are explicitly dealt with
 * in the sense that a data point may wrap around to serve as a
 * constraint on the other side of the periodic boundary.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

#define NN_DEF_SECTORS	4
#define NN_MIN_SECTORS	2

struct NEARNEIGHBOR_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct E {	/* -E<empty> */
		GMT_LONG active;
		double value;
	} E;
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct L {	/* -L<flag> */
		GMT_LONG active;
		char mode[4];
	} L;
	struct N {	/* -N<sectors> */
		GMT_LONG active;
		GMT_LONG sectors, min_sectors;
	} N;
	struct S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n] */
		GMT_LONG active;
		GMT_LONG mode;
		double radius;
		char unit;
	} S;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
};

struct NEARNEIGHBOR_NODE {	/* Structure with point id and distance pairs for all sectors */
	float *distance;	/* Distance of nearest datapoint to this node per sector */
	GMT_LONG *datum;	/* Point id of this data point */
};

struct NEARNEIGHBOR_POINT {	/* Structure with input data constraints */
	float x, y, z, w;
};

void *New_nearneighbor_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct NEARNEIGHBOR_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct NEARNEIGHBOR_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->N.sectors = NN_DEF_SECTORS;
	C->N.min_sectors = NN_MIN_SECTORS;
	return ((void *)C);
}

void Free_nearneighbor_Ctrl (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);
	GMT_free (GMT, C);
}

struct NEARNEIGHBOR_NODE *add_new_node (struct GMT_CTRL *GMT, GMT_LONG n)
{	/* Allocate an initialize a new node to have -1 in all the datum sectors */
	struct NEARNEIGHBOR_NODE *new = GMT_memory (GMT, NULL, 1, struct NEARNEIGHBOR_NODE);
	new->distance = GMT_memory (GMT, NULL, n, float);
	new->datum = GMT_memory (GMT, NULL, n, GMT_LONG);
	while (n > 0) new->datum[--n] = -1;

	return (new);
}

void assign_node (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_NODE **node, GMT_LONG n_sector, GMT_LONG sector, double distance, GMT_LONG id)
{	/* Allocates node space if not already used and updates the value if closer to node than the current value */

	if (!(*node)) *node = add_new_node (GMT, n_sector);
	if ((*node)->datum[sector] == -1 || (*node)->distance[sector] > distance) {
		(*node)->distance[sector] = (float)distance;
		(*node)->datum[sector] = id;
	}
}

void free_node (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_NODE *node)
{	/* Frees allocated node space */

	if (!node) return;	/* Nothing to do */
	GMT_free (GMT, node->distance);
	GMT_free (GMT, node->datum);
	GMT_free (GMT, node);
}

GMT_LONG GMT_nearneighbor_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "nearneighbor %s [API] - A \"Nearest neighbor\" gridding algorithm\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: nearneighbor [xyzfile(s)] -G<out_grdfile> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t-N<sectors>[/<min_sectors>] %s -S%s\n", GMT_Rgeo_OPT, GMT_RADIUS_OPT);
	GMT_message (GMT, "\t[-E<empty>] [-L<flags>] [%s] [-W] [%s]\n", GMT_V_OPT, GMT_bi_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s] [%s]\n\n", GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Name of output grid.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t-N Sets number of sectors and minimum number to be filled.\n");
	GMT_message (GMT, "\t   Default is quadrant search [%d], requiring at least %d to be filled.\n", NN_DEF_SECTORS, NN_MIN_SECTORS);
	GMT_explain_options (GMT, "R");
	GMT_dist_syntax (GMT, 'S', "Only consider points inside this search radius.");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-E Value to use for empty nodes [Default is NaN].\n");
	GMT_message (GMT, "\t-L Sets boundary conditions.  <flags> can be either\n");
	GMT_message (GMT, "\t   g for geographic boundary conditions, or one or both of\n");
	GMT_message (GMT, "\t   x for periodic boundary conditions on x.\n");
	GMT_message (GMT, "\t   y for periodic boundary conditions on y.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Input file has observation weights in 4th column.\n");
	GMT_explain_options (GMT, "C0");
	GMT_message (GMT, "\t   Default is 3 (or 4 if -W is set) columns.\n");
	GMT_explain_options (GMT, "fhiF:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_nearneighbor_parse (struct GMTAPI_CTRL *C, struct NEARNEIGHBOR_CTRL *Ctrl, struct GMT_EDGEINFO *edgeinfo, struct GMT_OPTION *options)
{
	/* This parses the options provided to nearneighbor and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n, n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_boundcond_init (GMT, edgeinfo);

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'E':	/* NaN value */
				Ctrl->E.active = TRUE;
				if (opt->arg[0])
					Ctrl->E.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					n_errors++;
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -E option: Must specify value or NaN\n");
				}
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Grid spacings */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* BCs */
				if (opt->arg[0]) {
					Ctrl->L.active = TRUE;
					strncpy (Ctrl->L.mode, opt->arg, (size_t)4);
				}
				break;
			case 'N':	/* Sectors */
				Ctrl->N.active = TRUE;
				n = sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &Ctrl->N.sectors, &Ctrl->N.min_sectors);
				if (n < 1) Ctrl->N.sectors = NN_DEF_SECTORS;
				if (n < 2) Ctrl->N.min_sectors = NN_MIN_SECTORS;
				break;
			case 'S':	/* Search radius */
				Ctrl->S.active = TRUE;
				Ctrl->S.mode = GMT_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;
			case 'W':	/* Use weights */
				Ctrl->W.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}


	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active, "Syntax error: Must specify -S option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error option -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.sectors <= 0, "Syntax error -N option: Must specify a positive number of sectors\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -1, "Syntax error -S: Unrecognized unit\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -2, "Syntax error -S: Unable to decode radius\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -3, "Syntax error -S: Radius is negative\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);
	if (Ctrl->L.active && GMT_boundcond_parse (GMT, edgeinfo, Ctrl->L.mode)) n_errors++;

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_nearneighbor_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_nearneighbor (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG row, col, k, col_0, row_0, ij, n, n_alloc = GMT_CHUNK, n_set;
	GMT_LONG d_row, sector, n_fields, ii, jj, ij0, n_read, *d_col = NULL;
	GMT_LONG max_d_col, actual_max_d_col, x_wrap, y_wrap, n_almost, n_none;
	GMT_LONG error = FALSE, wrap_180, replicate_x, replicate_y, n_filled;

	double weight, weight_sum, grd_sum, dx, dy, delta, distance = 0.0;
	double x_left, x_right, y_top, y_bottom, idx, idy, factor;
	double half_y_width, y_width, half_x_width, x_width, three_over_radius;
	double *x0 = NULL, *y0 = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_EDGEINFO edgeinfo;
	struct NEARNEIGHBOR_NODE **grid_node = NULL;
	struct NEARNEIGHBOR_POINT *point = NULL;
	struct NEARNEIGHBOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_nearneighbor_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_nearneighbor_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_nearneighbor", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "hirs" GMT_OPT("FH"), options))) Return (error);
	Ctrl = (struct NEARNEIGHBOR_CTRL *) New_nearneighbor_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_nearneighbor_parse (API, Ctrl, &edgeinfo, options))) Return (error);

	/*---------------------------- This is the nearneighbor main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	GMT_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	/* Initialize the input since we are doing record-by-record reading/writing */
	if ((error = GMT_set_cols (GMT, GMT_IN, 3 + Ctrl->W.active))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */

	idx = 1.0 / Grid->header->inc[GMT_X];
	idy = 1.0 / Grid->header->inc[GMT_Y];

	GMT_boundcond_param_prep (GMT, Grid->header, &edgeinfo);

	GMT_report (GMT, GMT_MSG_NORMAL, "Grid dimensions are nx = %d, ny = %d\n", Grid->header->nx, Grid->header->ny);

	grid_node = GMT_memory (GMT, NULL, Grid->header->nm, struct NEARNEIGHBOR_NODE *);
	point = GMT_memory (GMT, NULL, n_alloc, struct NEARNEIGHBOR_POINT);

	d_col = GMT_memory (GMT, NULL, Grid->header->ny, GMT_LONG);
	x0 = GMT_memory (GMT, NULL, Grid->header->nx, double);
	y0 = GMT_memory (GMT, NULL, Grid->header->ny, double);
	for (col = 0; col < Grid->header->nx; col++) x0[col] = GMT_grd_col_to_x (col, Grid->header);
	for (row = 0; row < Grid->header->ny; row++) y0[row] = GMT_grd_row_to_y (row, Grid->header);
	if (Ctrl->S.mode) {	/* Input data is geographical */
		max_d_col = (GMT_LONG) (ceil (Grid->header->nx / 2.0) + 0.1);
		actual_max_d_col = 0;
		for (row = 0; row < Grid->header->ny; row++) {
			d_col[row] = (fabs (y0[row]) == 90.0) ? max_d_col : (GMT_LONG)(ceil (Ctrl->S.radius / (GMT->current.proj.DIST_KM_PR_DEG * Grid->header->inc[GMT_X] * cosd (y0[row]))) + 0.1);
			if (d_col[row] > max_d_col) d_col[row] = max_d_col;
			if (d_col[row] > actual_max_d_col) actual_max_d_col = d_col[row];
		}
		d_row = (GMT_LONG) (ceil (Ctrl->S.radius / (GMT->current.proj.DIST_KM_PR_DEG * Grid->header->inc[GMT_Y])) + 0.1);
	}
	else {	/* Plain Cartesian data */
		max_d_col = (GMT_LONG) (ceil (Ctrl->S.radius * idx) + 0.1);
		for (row = 0; row < Grid->header->ny; row++) d_col[row] = max_d_col;
		d_row = (GMT_LONG) (ceil (Ctrl->S.radius * idy) + 0.1);
		actual_max_d_col = max_d_col;
	}

	factor = Ctrl->N.sectors / (2.0 * M_PI);

	x_left = Grid->header->wesn[XLO] - actual_max_d_col * Grid->header->inc[GMT_X];	x_right = Grid->header->wesn[XHI] + actual_max_d_col * Grid->header->inc[GMT_X];
	y_top = Grid->header->wesn[YHI] + d_row * Grid->header->inc[GMT_Y];	y_bottom = Grid->header->wesn[YLO] - d_row * Grid->header->inc[GMT_Y];
	x_width = Grid->header->wesn[XHI] - Grid->header->wesn[XLO];		y_width = Grid->header->wesn[YHI] - Grid->header->wesn[YLO];
	half_x_width = 0.5 * x_width;			half_y_width = 0.5 * y_width;
	n = n_read = 0;
	replicate_x = (edgeinfo.nxp && Grid->header->registration == GMT_GRIDLINE_REG);	/* Gridline registration has duplicate column */
	replicate_y = (edgeinfo.nyp && Grid->header->registration == GMT_GRIDLINE_REG);	/* Gridline registration has duplicate row */
	x_wrap = Grid->header->nx - 1;				/* Add to node index to go to right column */
	y_wrap = (Grid->header->ny - 1) * Grid->header->nx;	/* Add to node index to go to bottom row */

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);

		n_read++;
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
		
		if (GMT_is_dnan (in[GMT_Z])) continue;					/* Skip if z = NaN */
		if (GMT_y_is_outside (in[GMT_Y], y_bottom, y_top)) continue;		/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], x_left, x_right)) continue;	/* Outside x-range (or longitude) */

		/* Store this point in memory */
		
		point[n].x = (float)in[GMT_X];
		point[n].y = (float)in[GMT_Y];
		point[n].z = (float)in[GMT_Z];
		if (Ctrl->W.active) point[n].w = (float)in[3];

		/* Find row/col indices of the node closest to this data point */

		col_0 = GMT_grd_x_to_col (in[GMT_X], Grid->header);
		row_0 = GMT_grd_y_to_row (in[GMT_Y], Grid->header);

		/* Loop over all nodes within radius of this node */

		for (row = row_0 - d_row; row <= (row_0 + d_row); row++) {

			jj = row;
			if (GMT_y_out_of_bounds (GMT, &jj, Grid->header, &edgeinfo, &wrap_180)) continue;	/* Outside y-range */

			for (col = col_0 - d_col[jj]; col <= (col_0 + d_col[jj]); col++) {

				ii = col;
				if (GMT_x_out_of_bounds (GMT, &ii, Grid->header, &edgeinfo, wrap_180)) continue;	/* Outside x-range */ 

				/* Here, (ii,jj) is index of a node (k) inside the grid */

				distance = GMT_distance (GMT, x0[ii], y0[jj], in[GMT_X], in[GMT_Y]);

				if (distance > Ctrl->S.radius) continue;	/* Data constraint is too far from this node */
				k = GMT_IJ0 (Grid->header, jj, ii);		/* No padding used for gridnode array */
				dx = in[GMT_X] - x0[ii];	dy = in[GMT_Y] - y0[jj];

				/* Check for wrap-around in x or y.  This should only occur if the
				   search radius is larger than 1/2 the grid width/height so that
				   the shortest distance is going through the periodic boundary.
				   For longitudes the dx obviously cannot exceed 180 (half_x_width)
				   since we could then go the other direction instead.
				*/
				if (edgeinfo.nxp && fabs (dx) > half_x_width) dx -= copysign (x_width, dx);
				if (edgeinfo.nyp && fabs (dy) > half_y_width) dy -= copysign (y_width, dy);

				/* OK, this point should constrain this node.  Calculate which sector and assign the value */

				sector = ((GMT_LONG)((d_atan2 (dy, dx) + M_PI) * factor)) % Ctrl->N.sectors;
				assign_node (GMT, &grid_node[k], Ctrl->N.sectors, sector, distance, n);

				/* With periodic, gridline-registered grids there are duplicate rows and/or columns
				   so we may have to assign the point to more than one node.  The next section deals
				   with this situation.
				*/

				if (replicate_x) {	/* Must check if we have to replicate a column */
					if (ii == 0) 	/* Must replicate left to right column */
						assign_node (GMT, &grid_node[k+x_wrap], Ctrl->N.sectors, sector, distance, n);
					else if (ii == edgeinfo.nxp)	/* Must replicate right to left column */
						assign_node (GMT, &grid_node[k-x_wrap], Ctrl->N.sectors, sector, distance, n);
				}
				if (replicate_y) {	/* Must check if we have to replicate a row */
					if (jj == 0)	/* Must replicate top to bottom row */
						assign_node (GMT, &grid_node[k+y_wrap], Ctrl->N.sectors, sector, distance, n);
					else if (jj == edgeinfo.nyp)	/* Must replicate bottom to top row */
						assign_node (GMT, &grid_node[k-y_wrap], Ctrl->N.sectors, sector, distance, n);
				}
			}
		}
		n++;
		if (!(n%1000)) GMT_report (GMT, GMT_MSG_NORMAL, "Processed record %10ld\r", n);
		if (n == n_alloc) {
			n_alloc <<= 1;
			point = GMT_memory (GMT, point, n_alloc, struct NEARNEIGHBOR_POINT);
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	GMT_report (GMT, GMT_MSG_NORMAL, "Processed record %10ld\n", n);

	point = GMT_memory (GMT, point, n, struct NEARNEIGHBOR_POINT);
	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);

	/* Compute weighted averages based on the nearest neighbors */

	n_set = n_almost = n_none = 0;

	if (!Ctrl->E.active) Ctrl->E.value = GMT->session.d_NaN;
	three_over_radius = 3.0 / Ctrl->S.radius;

	ij0 = -1;
	GMT_row_loop (Grid, row) {
		GMT_col_loop (Grid, row, col, ij) {
			if (!grid_node[++ij0]) {	/* No nearest neighbors, set to empty and goto next node */
				n_none++;
				Grid->data[ij] = (float)Ctrl->E.value;
				continue;
			}

			for (k = 0, n_filled = 0; k < Ctrl->N.sectors; k++) if (grid_node[ij0]->datum[k] >= 0) n_filled++;
			if (n_filled < Ctrl->N.min_sectors) { 	/* Not minimum set of neighbors in all sectors, set to empty and goto next node */
				n_almost++;
				Grid->data[ij] = (float)Ctrl->E.value;
				free_node (GMT, grid_node[ij0]);
				continue;
			}

			/* OK, here we have enough data and need to calculate the weighted value */

			n_set++;
			weight_sum = grd_sum = 0.0;	/* Initialize sums */
			for (k = 0; k < Ctrl->N.sectors; k++) {
				if (grid_node[ij0]->datum[k] >= 0) {
					delta = three_over_radius * grid_node[ij0]->distance[k];
					weight = 1.0 / (1.0 + delta * delta);	/* This is distance weight */
					if (Ctrl->W.active) weight *= point[grid_node[ij0]->datum[k]].w;	/* This is observation weight */
					grd_sum += weight * point[grid_node[ij0]->datum[k]].z;
					weight_sum += weight;
				}
			}
			Grid->data[ij] = (float)(grd_sum / weight_sum);
			free_node (GMT, grid_node[ij0]);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Gridded row %10ld\r", row);
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Gridded row %10ld\n", row);

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) {
		char line[BUFSIZ];
		sprintf (line, "%s)\n", GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld nodes were assigned an average value\n", n_set);
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld nodes failed sector criteria and %ld nodes had no neighbor points (all set to ", n_almost, n_none);
		(GMT_is_dnan (Ctrl->E.value)) ? GMT_message (GMT, "NaN)\n") : GMT_message (GMT,  line, Ctrl->E.value);
	}

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	GMT_free (GMT, point);
	GMT_free (GMT, grid_node);
	GMT_free (GMT, d_col);
	GMT_free (GMT, x0);
	GMT_free (GMT, y0);

	Return (GMT_OK);
}
