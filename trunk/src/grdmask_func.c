/*--------------------------------------------------------------------
 *	$Id: grdmask_func.c,v 1.14 2011-05-11 04:01:54 guru Exp $
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
 * Brief synopsis: grdmask defines a grid based on region and xinc/yinc values,
 * reads xy polygon files, and sets the grid nodes inside, on the
 * boundary, and outside of the polygons to the user-defined values
 * <in>, <on>, and <out>.  These may be any number, including NaN.
 *
 * Author:	Walter. H. F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

#define GRDMASK_N_CLASSES	3	/* outside, on edge, and inside */

struct GRDMASK_CTRL {
	struct A {	/* -A[m|p|step] */
		GMT_LONG active;
		GMT_LONG mode;
		double step;
	} A;
	struct G {	/* -G<maskfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct N {	/* -N<maskvalues> */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 for out/on/in, 1 for polygon ID inside, 2 for polygon ID inside+path */
		double mask[GRDMASK_N_CLASSES];	/* values for each level */
	} N;
	struct S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n] */
		GMT_LONG active;
		GMT_LONG mode;
		double radius;
		char unit;
	} S;
};

void *New_grdmask_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMASK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDMASK_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->N.mask[GMT_INSIDE] = 1.0;	/* Default inside value */
	return ((void *)C);
}

void Free_grdmask_Ctrl (struct GMT_CTRL *GMT, struct GRDMASK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdmask_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdmask %s [API] - Create mask grid file from polygons or point coverage\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdmask [<xyfiles>] -G<mask_grd_file> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-A[m|p]] [-N[i|I|p|P][<values>]]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-S%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_RADIUS_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\txyfiles is one or more polygon [or point] files\n");
	GMT_message (GMT, "\t-G Specify file name for output mask grid file.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Suppress connecting points using great circle arcs, i.e. connect by straight lines,\n");
	GMT_message (GMT, "\t   unless m or p is appended to first follow meridian then parallel, or vice versa.\n");
	GMT_message (GMT, "\t   Ignored if -S is used since points are then not considered to be lines.\n");
	GMT_message (GMT, "\t-N Sets <out>/<edge>/<in> to use if point is outside, on the path, or inside.\n");
	GMT_message (GMT, "\t   NaN is a valid entry.  Default values are 0/0/1.\n");
	GMT_message (GMT, "\t   Optionally, use -Ni (inside) or -NI (inside+edge) to set polygon ID:\n");
	GMT_message (GMT, "\t     a) If OGR/GMT files, get polygon ID via -a for Z column.\n");
	GMT_message (GMT, "\t     b) Interpret segment labels (-L<label>) as polygon IDs.\n");
	GMT_message (GMT, "\t   Finally, use -Np|P and append origin for running polygon IDs [0].\n");
	GMT_dist_syntax (GMT, 'S', "Sets search radius to identify inside points.");
	GMT_message (GMT, "\t   Mask nodes are set to <in> or <out> depending on whether they are\n");
	GMT_message (GMT, "\t   inside the circle of specified radius [0] from the nearest data point.\n");
	GMT_message (GMT, "\t   [Default is to treat xyfiles as polygons and use inside/outside searching].\n");
	GMT_explain_options (GMT, "VC2fghiF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdmask_parse (struct GMTAPI_CTRL *C, struct GRDMASK_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdmask and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j, pos;
	char ptr[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = TRUE;
				switch (opt->arg[0]) {
					case 'm': Ctrl->A.mode = 1; break;
					case 'p': Ctrl->A.mode = 2; break;
#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature; requires step in degrees */
#endif
				}
				break;
			case 'G':	/* Output filename */
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
			case 'N':	/* Mask values */
				Ctrl->N.active = TRUE;
				switch (opt->arg[0]) {
					case 'i':	/* Polygon ID from file (inside only) */
						Ctrl->N.mode = 1;
						break;
					case 'I':	/* Polygon ID from file (inside + edge) */
						Ctrl->N.mode = 2;
						break;
					case 'p':	/* Polygon ID from running number (inside only) */
						Ctrl->N.mode = 3;
						Ctrl->N.mask[0] = (opt->arg[1]) ? atof (&opt->arg[1]) - 1.0 : -1.0;	/* Running ID start [0] (we increment first) */
						break;
					case 'P':	/* Polygon ID from running number (inside + edge) */
						Ctrl->N.mode = 4;
						Ctrl->N.mask[0] = (opt->arg[1]) ? atof (&opt->arg[1]) - 1.0 : -1.0;	/* Running ID start [0] (we increment first) */
						break;
					default:	/* Standard out/on/in constant values */
						j = pos = 0;
						while (j < GRDMASK_N_CLASSES && (GMT_strtok (opt->arg, "/", &pos, ptr))) {
							Ctrl->N.mask[j] = (ptr[0] == 'N' || ptr[0] == 'n') ? GMT->session.f_NaN : (float)atof (ptr);
							j++;
						}
						break;
				}
				break;
			case 'S':	/* Search radius */
				Ctrl->S.active = TRUE;
				Ctrl->S.mode = GMT_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -1, "Syntax error -S: Unrecognized unit\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -2, "Syntax error -S: Unable to decode radius\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.mode == -3, "Syntax error -S: Radius is negative\n");
	n_errors += GMT_check_binary_io (GMT, 2);
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdmask_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdmask (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, periodic = FALSE, periodic_grid = FALSE;
	GMT_LONG row, col, side, d_col = 0, d_row = 0, col_0, row_0;
	GMT_LONG tbl, seg, mode, n_pol = 0, k, ij;
	
	char seg_label[GMT_TEXT_LEN64];

	float mask_val[3];
	
	double distance, xx, yy, x0, y0, ID, xtmp;

	struct GMT_GRID *Grid = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct GRDMASK_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdmask_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdmask_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdmask", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "aghirs" GMT_OPT("FHMm"), options))) Return (error);
	Ctrl = (struct GRDMASK_CTRL *) New_grdmask_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdmask_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdmask main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	for (k = 0; k < 3; k++) mask_val[k] = (float)Ctrl->N.mask[k];	/* Copy over the mask values for perimeter polygons */
	ID = Ctrl->N.mask[0];	/* Starting value if running IDs */

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		char line[GMT_BUFSIZ];
		if (Ctrl->N.mode == 1) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes completely inside the polygons will be set to the polygon ID\n");
		}
		else if (Ctrl->N.mode == 2) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes completely inside the polygons or on the edge will be set to the polygon ID\n");
		}
		else {
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes completely outside the polygons will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[GMT_OUTSIDE])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[GMT_OUTSIDE]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes completely inside the polygons will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[GMT_INSIDE])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[GMT_INSIDE]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes on the polygons boundary will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[GMT_ONEDGE])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[GMT_ONEDGE]);
		}
	}

	if (Ctrl->S.active) {	/* Need distance calculations */
		GMT_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
		if (Ctrl->S.mode) {	/* Flat Earth max width calculation */
			double width, shrink;
			shrink = cosd (MAX (fabs (Grid->header->wesn[YLO]), fabs (Grid->header->wesn[YHI])));
			width = (shrink < GMT_CONV_LIMIT) ? DBL_MAX : ceil (Ctrl->S.radius / (GMT->current.proj.DIST_KM_PR_DEG * Grid->header->inc[GMT_X] * shrink));
			d_col = (width < (double)Grid->header->nx) ? (GMT_LONG) width : Grid->header->nx;
			d_row = (GMT_LONG)ceil (Ctrl->S.radius / (GMT->current.proj.DIST_KM_PR_DEG * Grid->header->inc[GMT_Y]));
		}
		else {	/* Cartesian */
			d_col = (GMT_LONG)ceil (Ctrl->S.radius * Grid->header->r_inc[GMT_X]);
			d_row = (GMT_LONG)ceil (Ctrl->S.radius * Grid->header->r_inc[GMT_Y]);
		}
	}
	periodic = GMT_is_geographic (GMT, GMT_IN);	/* Dealing with geographic coordinates */
	if ((Ctrl->A.active && Ctrl->A.mode == 0) || !GMT_is_geographic (GMT, GMT_IN)) GMT->current.map.path_mode = GMT_LEAVE_PATH;	/* Turn off resampling */

	/* Initialize all nodes (including pad) to the 'outside' value */

	for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = mask_val[GMT_OUTSIDE];

	if ((error = GMT_set_cols (GMT, GMT_IN, 2))) Return (error);
	mode = (Ctrl->S.active) ? GMT_IS_POINT : GMT_IS_POLY;
	GMT_skip_xy_duplicates (GMT, TRUE);	/* Avoid repeating x/y points in polygons */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, mode, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if ((error = GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&D))) Return (error);
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	GMT_skip_xy_duplicates (GMT, FALSE);	/* Reset */

	if (!Ctrl->S.active && GMT->current.map.path_mode == GMT_RESAMPLE_PATH) {	/* Resample all polygons to desired resolution, once and for all */
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
				S = D->table[tbl]->segment[seg];	/* Current segment */
				S->n_rows = GMT_fix_up_path (GMT, &S->coord[GMT_X], &S->coord[GMT_Y], S->n_rows, Ctrl->A.step, Ctrl->A.mode);
			}
		}
	}

	for (tbl = n_pol = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++, n_pol++) {	/* For each segment in the table */
			S = D->table[tbl]->segment[seg];		/* Current segment */
			if (Ctrl->S.active) {	/* Assign 'inside' to nodes within given distance of data constrains */
				for (k = 0; k < S->n_rows; k++) {
					if (GMT_y_is_outside (S->coord[GMT_Y][k], Grid->header->wesn[YLO], Grid->header->wesn[YHI])) continue;	/* Outside y-range */
					xtmp = S->coord[GMT_X][k];	/* Make copy since we may have to adjust by +-360 */
					if (GMT_x_is_outside (GMT, &xtmp, Grid->header->wesn[XLO], Grid->header->wesn[XHI])) continue;	/* Outside x-range (or longitude) */

					/* OK, this point is within bounds, but may be exactly on the border */

					col_0 = GMT_grd_x_to_col (xtmp, Grid->header);
					if (col_0 == Grid->header->nx) col_0--;	/* Was exactly on the xmax edge */
					row_0 = GMT_grd_y_to_row (S->coord[GMT_Y][k], Grid->header);
					if (row_0 == Grid->header->ny) row_0--;	/* Was exactly on the ymin edge */
					ij = GMT_IJP (Grid->header, row_0, col_0);
					Grid->data[ij] = mask_val[GMT_INSIDE];	/* This is the nearest node */
					if (Grid->header->registration == GMT_GRIDLINE_REG && (col_0 == 0 || col_0 == (Grid->header->nx-1)) && periodic_grid) {	/* Must duplicate the entry at periodic point */
						col = (col_0 == 0) ? Grid->header->nx-1 : 0;
						ij = GMT_IJP (Grid->header, row_0, col);
						Grid->data[ij] = mask_val[GMT_INSIDE];	/* This is also the nearest node */
					}
					if (Ctrl->S.radius == 0.0) continue;	/* Only consider the nearest node */
					/* Here we also include all the nodes within the search radius */
					for (row = row_0 - d_row; row <= (row_0 + d_row); row++) {
						if (row < 0 || row >= Grid->header->ny) continue;
						for (col = col_0 - d_col; col <= (col_0 + d_col); col++) {
							if (col < 0 || col >= Grid->header->nx) continue;
							ij = GMT_IJP (Grid->header, row, col);
							x0 = GMT_grd_col_to_x (col, Grid->header);
							y0 = GMT_grd_row_to_y (row, Grid->header);
							distance = GMT_distance (GMT, xtmp, S->coord[GMT_Y][k], x0, y0);
							if (distance > Ctrl->S.radius) continue;
							Grid->data[ij] = mask_val[GMT_INSIDE];	/* The inside value */
						}
					}
				}
			}
			else if (S->n_rows > 2) {	/* assign 'inside' to nodes if they are inside given polygon */
				if (GMT_polygon_is_hole (S)) continue;	/* Holes are handled within GMT_inonout */
				if (Ctrl->N.mode == 1 || Ctrl->N.mode == 2) {	/* Look for polygon IDs in the data headers */
					if (S->ogr)	/* OGR data */
						ID = GMT_get_aspatial_value (GMT, GMT_IS_Z, S);
					else if (GMT_parse_segment_item (GMT, S->header, "-L", seg_label))	/* Look for segment header ID */
						ID = atof (seg_label);
					else
						GMT_report (GMT, GMT_MSG_FATAL, "No polygon ID found; ID set to NaN\n");
				}
				else if (Ctrl->N.mode)	/* 3 or 4; Increment running polygon ID */
					ID += 1.0;

				for (row = 0; row < Grid->header->ny; row++) {

					yy = GMT_grd_row_to_y (row, Grid->header);
					
					/* First check if y/latitude is outside, then there is no need to check all the x/lon values */
					
					if (periodic) {	/* Containing annulus test */
						if (S->pole != +1 && yy > S->max[GMT_Y]) continue;	/* No N polar cap and beyond north */
						if (S->pole != -1 && yy < S->min[GMT_Y]) continue;	/* No S polar cap and beyond south */
					}
					else if (yy < S->min[GMT_Y] || yy > S->max[GMT_Y])	/* Cartesian case */
						continue;

					/* Here we will have to consider the x coordinates as well */
					for (col = 0; col < Grid->header->nx; col++) {
						xx = GMT_grd_col_to_x (col, Grid->header);
						if ((side = GMT_inonout (GMT, xx, yy, S)) == 0) continue;	/* Outside polygon, go to next point */
						/* Here, point is inside or on edge, we must assign value */

						ij = GMT_IJP (Grid->header, row, col);
						
						if (Ctrl->N.mode%2 && side == GMT_ONEDGE) continue;	/* Not counting the edge as part of polygon for ID tagging for mode 1 | 3 */
						Grid->data[ij] = (Ctrl->N.mode) ? (float)ID : mask_val[side];
					}
					GMT_report (GMT, GMT_MSG_NORMAL, "Polygon %ld scanning row %5.5ld\r", n_pol, row);
				}
			}
		}
	}

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Grid);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	Return (GMT_OK);
}
