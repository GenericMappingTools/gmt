/*--------------------------------------------------------------------
 *	$Id: grdlandmask_func.c,v 1.5 2011-04-12 03:05:18 remko Exp $
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
 * Brief synopsis: grdlandmask defines a grid based on region and xinc/yinc values,
 * reads a shoreline data base, and sets the grid nodes inside, on the
 * boundary, and outside of the polygons to the user-defined values
 * <in>, <on>, and <out>.  These may be any number, including NaN.
 *
 * Author:	P. Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

#define GRDLANDMASK_N_CLASSES	(GMT_MAX_GSHHS_LEVEL + 1)	/* Number of bands separated by the levels */

#define P_IS_OUTSIDE	0
#define P_IS_ONSIDE	1
#define P_IS_INSIDE	2

struct GRDLANDMASK_CTRL {	/* All control options for this program (except common args) */
	/* ctive is TRUE if the option has been activated */
	struct A {	/* -A<min_area>[/<min_level>/<max_level>] */
		GMT_LONG active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct D {	/* -D<resolution> */
		GMT_LONG active;
		GMT_LONG force;	/* if TRUE, select next highest level if current set is not avaialble */
		char set;	/* One of f, h, i, l, c */
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG inside;	/* if 2, then a point exactly on a polygon boundary is considered OUTSIDE, else 1 */
	} E;
	struct G {	/* -G<maskfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct N {	/* -N<maskvalues>[o] */
		GMT_LONG active;
		GMT_LONG mode;	/* 1 if dry/wet only, 0 if 5 mask levels */
		double mask[GRDLANDMASK_N_CLASSES];	/* values for each level */
	} N;
};

void *New_grdlandmask_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDLANDMASK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDLANDMASK_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->A.info.high = GMT_MAX_GSHHS_LEVEL;				/* Include all GSHHS levels */
	C->D.set = 'l';							/* Low-resolution coastline data */
	C->E.inside = P_IS_ONSIDE;					/* Default is that points on a boundary are inside */
	GMT_memset (C->N.mask, GRDLANDMASK_N_CLASSES, double);		/* Default "wet" value = 0 */
	C->N.mask[1] = C->N.mask[3] = 1.0;				/* Default for "dry" areas = 1 (inside) */
	
	return ((void *)C);
}

void Free_grdlandmask_Ctrl (struct GMT_CTRL *GMT, struct GRDLANDMASK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdlandmask_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdlandmask %s [API] - Create \"wet-dry\" mask grid file from shoreline data base\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdlandmask -G<mask_grd_file> %s %s\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [-D<resolution>][+] [-E] [-N<maskvalues>] [%s] [%s]\n\n", GMT_A_OPT, GMT_V_OPT, GMT_r_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Specify file name for output mask grid file.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_GSHHS_syntax (GMT, 'A', "Place limits on coastline features from the GSHHS data base.");
	GMT_message (GMT, "\t-D Choose one of the following resolutions:\n");
	GMT_message (GMT, "\t   f - full resolution (may be very slow for large regions)\n");
	GMT_message (GMT, "\t   h - high resolution (may be slow for large regions)\n");
	GMT_message (GMT, "\t   i - intermediate resolution\n");
	GMT_message (GMT, "\t   l - low resolution [Default]\n");
	GMT_message (GMT, "\t   c - crude resolution, for tasks that need crude continent outlines only\n");
	GMT_message (GMT, "\t   Append + to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_message (GMT, "\t-E Indicate that nodes exactly on a polygon boundary are outside [inside].\n");
	GMT_message (GMT, "\t-N Give values to use if a node is outside or inside a feature.\n");
	GMT_message (GMT, "\t   Specify this information using 1 of 2 formats:\n");
	GMT_message (GMT, "\t   -N<wet>/<dry>.\n");
	GMT_message (GMT, "\t   -N<ocean>/<land>/<lake>/<island>/<pond>.\n");
	GMT_message (GMT, "\t   NaN is a valid entry.  Default values are 0/1/0/1/0 (i.e., 0/1)\n");
	GMT_explain_options (GMT, "VF.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdlandmask_parse (struct GMTAPI_CTRL *C, struct GRDLANDMASK_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdlandmask and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j, pos, n_files = 0;
	char line[GMT_LONG_TEXT], ptr[BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Restrict GSHHS features */
				Ctrl->A.active = TRUE;
				GMT_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'D':	/* Set GSHHS resolution */
				Ctrl->D.active = TRUE;
				Ctrl->D.set = opt->arg[0];
				Ctrl->D.force = (opt->arg[1] == '+');
				break;
			case 'E':	/* On-boundary setting */
				Ctrl->E.active = TRUE;
				Ctrl->E.inside = P_IS_INSIDE;
				break;
			case 'G':	/* OUtput filename */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Grid spacings */
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				Ctrl->I.active = TRUE;
				break;
			case 'N':	/* Mask values */
				Ctrl->N.active = TRUE;
				strcpy (line, opt->arg);
#ifdef GMT_COMPAT
				if (line[strlen(line)-1] == 'o') { /* Edge is considered outside */
					Ctrl->E.active = TRUE;
					Ctrl->E.inside = P_IS_INSIDE;
					line[strlen(line)-1] = 0;
				}
#endif
				j = pos = 0;
				while (j < 5 && (GMT_strtok (line, "/", &pos, ptr))) {
					Ctrl->N.mask[j] = (ptr[0] == 'N' || ptr[0] == 'n') ? GMT->session.f_NaN : (float)atof (ptr);
					j++;
				}
				if (!(j == 2 || j == 5)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -N option:  Specify 2 or 5 arguments\n");
					n_errors++;
				}
				Ctrl->N.mode = (j == 2);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify an output file\n");
	n_errors += GMT_check_condition (GMT, n_files, "Syntax error: Use -G to specify output an file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdlandmask_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdlandmask (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, greenwich = FALSE, temp_shift = FALSE, wrap, used_polygons;
	GMT_LONG i, k, ii, bin, ind, np, side, col_min, col_max, row_min, row_max, nx1, ny1;
	GMT_LONG base = 3, direction, err, ij, row, col, np_new;

	char line[GMT_LONG_TEXT];
	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	double xmin, xmax, ymin, ymax, west_border, east_border, i_dx_inch, i_dy_inch;
	double i_dx, i_dy, del_off, dummy, *x = NULL, *y = NULL;

	struct GMT_SHORE c;
	struct GMT_GRID *Grid = NULL;
	struct GMT_GSHHS_POL *p = NULL;
	struct GRDLANDMASK_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdlandmask_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdlandmask_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdlandmask", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRr", GMT_OPT("F"), options))) Return (error);
	Ctrl = (struct GRDLANDMASK_CTRL *) New_grdlandmask_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdlandmask_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdlandmask main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	if (Grid->header->wesn[XLO] < 0.0 && Grid->header->wesn[XHI] < 0.0) {	/* Shift longitudes */
		temp_shift = TRUE;
		Grid->header->wesn[XLO] += 360.0;
		Grid->header->wesn[XHI] += 360.0;
	}

	if (Ctrl->D.force) Ctrl->D.set = GMT_shore_adjust_res (GMT, Ctrl->D.set);
	base = GMT_set_resolution (GMT, &Ctrl->D.set, 'D');
	
	if (Ctrl->N.mode) {
		Ctrl->N.mask[3] = Ctrl->N.mask[1];
		Ctrl->N.mask[2] = Ctrl->N.mask[4] = Ctrl->N.mask[0];
	}

	if (GMT_init_shore (GMT, Ctrl->D.set, &c, Grid->header->wesn, &Ctrl->A.info)) {
		GMT_report (GMT, GMT_MSG_FATAL, "%s resolution shoreline data base not installed\n", shore_resolution[base]);
		Return (EXIT_FAILURE);
	}
	if (GMT->current.setting.verbose >= GMT_MSG_VERBOSE) {
		GMT_message (GMT, "GSHHS version %s\n%s\n%s\n", c.version, c.title, c.source);

		sprintf (line, "%s\n", GMT->current.setting.format_float_out);
		if (Ctrl->N.mode) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes in water will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[0])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[0]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes on land will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[1])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[1]);
		}
		else {
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes in the oceans will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[0])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[0]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes on land will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[1])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[1]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes in lakes will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[2])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[2]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes in islands will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[3])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[3]);
			GMT_report (GMT, GMT_MSG_NORMAL, "Nodes in ponds will be set to ");
			(GMT_is_fnan (Ctrl->N.mask[4])) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.mask[4]);
		}
	}

	i_dx = 1.0 / Grid->header->inc[GMT_X];
	i_dy = 1.0 / Grid->header->inc[GMT_Y];
	del_off = Grid->header->xy_off;
	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	/* All data nodes are thus initialized to 0 */
	x = GMT_memory (GMT, NULL, Grid->header->nx, double);
	y = GMT_memory (GMT, NULL, Grid->header->ny, double);

	nx1 = Grid->header->nx - 1;	ny1 = Grid->header->ny - 1;

	greenwich = (Grid->header->wesn[XLO] < 0.0 && Grid->header->wesn[XHI] > 0.0);	/* Must shift longitudes */

	GMT_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear projection so the shore machinery will work */
	GMT_err_fail (GMT, GMT_map_setup (GMT, Grid->header->wesn), "");
	GMT->current.map.parallel_straight = GMT->current.map.meridian_straight = 2;	/* No resampling along bin boundaries */
	wrap = GMT_360_RANGE (Grid->header->wesn[XHI], Grid->header->wesn[XLO]);
	
	/* Fill out gridnode coordinates and apply the implicit linear projection */

	for (col = 0; col < Grid->header->nx; col++) GMT_geo_to_xy (GMT, GMT_grd_col_to_x (col, Grid->header), 0.0, &x[col], &dummy);
	for (row = 0; row < Grid->header->ny; row++) GMT_geo_to_xy (GMT, 0.0, GMT_grd_row_to_y (row, Grid->header), &dummy, &y[row]);
	i_dx_inch = 1.0 / fabs (x[1] - x[0]);
	i_dy_inch = 1.0 / fabs (y[1] - y[0]);

        west_border = floor (GMT->common.R.wesn[XLO] / c.bsize) * c.bsize;
        east_border =  ceil (GMT->common.R.wesn[XHI] / c.bsize) * c.bsize;
	for (ind = 0; ind < c.nb; ind++) {	/* Loop over necessary bins only */

		bin = c.bins[ind];
		GMT_report (GMT, GMT_MSG_NORMAL, "Working on block # %5ld\r", bin);

		if ((err = GMT_get_shore_bin (GMT, ind, &c))) {
			GMT_report (GMT, GMT_MSG_FATAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			Return (EXIT_FAILURE);
		}

		/* Use polygons, if any.  Go in both directions to cover both land and sea */

		used_polygons = FALSE;

		for (direction = -1; c.ns > 0 && direction < 2; direction += 2) {

			/* Assemble one or more segments into polygons */

			np = GMT_assemble_shore (GMT, &c, direction, TRUE, greenwich, west_border, east_border, &p);

			/* Get clipped polygons in x,y inches that can be processed */

			np_new = GMT_prep_polygons (GMT, &p, np, FALSE, 0.0, -1);

			for (k = 0; k < np_new; k++) {

				if (p[k].n == 0) continue;

				used_polygons = TRUE;	/* At least som points made it to here */

				/* Find min/max of polygon in inches */

				xmin = xmax = p[k].lon[0];
				ymin = ymax = p[k].lat[0];
				for (i = 1; i < p[k].n; i++) {
					if (p[k].lon[i] < xmin) xmin = p[k].lon[i];
					if (p[k].lon[i] > xmax) xmax = p[k].lon[i];
					if (p[k].lat[i] < ymin) ymin = p[k].lat[i];
					if (p[k].lat[i] > ymax) ymax = p[k].lat[i];
				}
				col_min = (GMT_LONG)MAX (0, ceil (xmin * i_dx_inch - del_off - GMT_CONV_LIMIT));
				if (col_min > nx1) col_min = 0;
				col_max = (GMT_LONG)MIN (nx1, floor (xmax * i_dx_inch - del_off + GMT_CONV_LIMIT));
				if (col_max <= 0 || col_max < col_min) col_max = nx1;
				row_min = (GMT_LONG)MAX (0, ceil ((GMT->current.proj.rect[YHI] - ymax) * i_dy_inch - del_off - GMT_CONV_LIMIT));
				row_max = (GMT_LONG)MIN (ny1, floor ((GMT->current.proj.rect[YHI] - ymin) * i_dy_inch - del_off + GMT_CONV_LIMIT));

				for (row = row_min; row <= row_max; row++) {
					for (col = col_min; col <= col_max; col++) {

						if ((side = GMT_non_zero_winding (GMT, x[col], y[row], p[k].lon, p[k].lat, p[k].n)) < Ctrl->E.inside) continue;	/* Outside */

						/* Here, point is inside, we must assign value */

						ij = GMT_IJP (Grid->header, row, col);
						if (p[k].level > Grid->data[ij]) Grid->data[ij] = (float)p[k].level;
					}
				}
			}

			GMT_free_polygons (GMT, p, np_new);
			GMT_free (GMT, p);
		}

		if (!used_polygons) {	/* Lack of polygons or clipping etc resulted in no polygons after all, must deal with background */

			k = INT_MAX;	/* Initialize to outside range of levels (4 is highest) */
			/* Visit each of the 4 nodes, test if it is inside -R, and if so update lowest level found so far */

			if (!GMT_map_outside (GMT, c.lon_sw, c.lat_sw)) k = MIN (k, c.node_level[0]);			/* SW */
			if (!GMT_map_outside (GMT, c.lon_sw + c.bsize, c.lat_sw)) k = MIN (k, c.node_level[1]);		/* SE */
			if (!GMT_map_outside (GMT, c.lon_sw + c.bsize, c.lat_sw - c.bsize)) k = MIN (k, c.node_level[2]);	/* NE */
			if (!GMT_map_outside (GMT, c.lon_sw, c.lat_sw - c.bsize)) k = MIN (k, c.node_level[3]);		/* NW */

			/* If k is still INT_MAX we must assume this patch should have the min level of the bin */

			if (k == INT_MAX) k = MIN (MIN (c.node_level[0], c.node_level[1]) , MIN (c.node_level[2], c.node_level[3]));

			/* Determine nodes to initialize */

			row_min = (GMT_LONG)MAX (0, ceil ((Grid->header->wesn[YHI] - c.lat_sw - c.bsize) * i_dy - del_off));
			row_max = (GMT_LONG)MIN (ny1, floor ((Grid->header->wesn[YHI] - c.lat_sw) * i_dy - del_off));
			col_min = (GMT_LONG)ceil (fmod (c.lon_sw - Grid->header->wesn[XLO] + 360.0, 360.0) * i_dx - del_off);
			col_max = (GMT_LONG)floor (fmod (c.lon_sw + c.bsize - Grid->header->wesn[XLO] + 360.0, 360.0) * i_dx - del_off);
			if (wrap && col_max < col_min) col_max += Grid->header->nx;
			for (row = row_min; row <= row_max; row++) {
				for (col = col_min; col <= col_max; col++) {
					ii = (wrap) ? col % Grid->header->nx : col;
					if (ii < 0 || ii > nx1) continue;
					ij = GMT_IJP (Grid->header, row, ii);
					Grid->data[ij] = (float)k;
				}
			}
		}

		GMT_free_shore (GMT, &c);
	}

	GMT_shore_cleanup (GMT, &c);
	GMT_free (GMT, x);
	GMT_free (GMT, y);

	GMT_grd_loop (Grid, row, col, ij) {	/* Turn levels into mask values */
		k = irint (Grid->data[ij]);
		Grid->data[ij] = (float)Ctrl->N.mask[k];
	}

	if (wrap && Grid->header->registration == GMT_GRIDLINE_REG) { /* Copy over values to the repeating right column */
		for (row = 0, ij = GMT_IJP (Grid->header, row, 0); row < Grid->header->ny; row++, ij += Grid->header->mx) Grid->data[ij+nx1] = Grid->data[ij];
	}
	
	if (temp_shift) {
		Grid->header->wesn[XLO] -= 360.0;
		Grid->header->wesn[XHI] -= 360.0;
	}

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);				/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Grid);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	Return (GMT_OK);
}
