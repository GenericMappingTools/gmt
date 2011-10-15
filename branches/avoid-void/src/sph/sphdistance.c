/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2008-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Spherical nearest distances - via Voronoi polygons.  We read input
 * data, assumed to be things like coastlines, and want to create a grid
 * with distances to the nearest line.  The approach here is to break
 * the data into voronoi polygons and then visit all nodes inside each
 * polygon and use geodesic distance calculation from each node to the
 * unique Voronoi interior data node.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *     and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *     Software, 23 (3), 416-434.
 * We translated to C using f2c -r8 and and manually edited the code
 * so that f2c libs were not needed.
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2011
 * Version:	5 API
 *
 */
 
#include "gmt_sph.h"
#include "sph.h"

struct SPHDISTANCE_CTRL {
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct G {	/* -G<maskfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct L {	/* -L<unit>] */
		GMT_LONG active;
		char unit;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		char *file;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
		char *file;
	} Q;
};

void prepare_polygon (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *P)
{
	/* Set the min/max extent of this polygon and determine if it
	 * is a polar cap; if so set the required metadata flags */
	GMT_LONG i;
	double lon_sum = 0.0, lat_sum = 0.0, dlon;
	
	GMT_set_seg_minmax (C, P);	/* Set the domain of the segment */
	
	/* Then loop over points to accumulate sums */
	
	for (i = 1; i < P->n_rows; i++) {	/* Start at i = 1 since (a) 0'th point is repeated at end and (b) we are doing differences */
		dlon = P->coord[GMT_X][i] - P->coord[GMT_X][i-1];
		if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
		lon_sum += dlon;
		lat_sum += P->coord[GMT_Y][i];
	}
	P->pole = 0;
	if (GMT_360_RANGE (lon_sum, 0.0)) {	/* Contains a pole */
		if (lat_sum < 0.0) { /* S */
			P->pole = -1;
			P->min[GMT_Y] = -90.0;
		}
		else {	/* N */
			P->pole = +1;
			P->max[GMT_Y] = 90.0;
		}
		P->min[GMT_X] = 0.0;	P->max[GMT_X] = 360.0;
	}
}

void *New_sphdistance_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHDISTANCE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPHDISTANCE_CTRL);
	C->L.unit = 'e';	/* Default is meter distances */
	return ((void *)C);
}

void Free_sphdistance_Ctrl (struct GMT_CTRL *GMT, struct SPHDISTANCE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	if (C->N.file) free ((void *)C->N.file);	
	if (C->Q.file) free ((void *)C->Q.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_sphdistance_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "sphdistance %s - Make grid of distances to nearest points on a sphere\n\n", GMT_VERSION);
	GMT_message (GMT, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_message (GMT, "usage: sphdistance [<table>] -G<outgrid> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t[-C] [-D] [-E] [-L<unit>] [-N<nodetable>] [-Q<voronoitable>]\n");
	GMT_message (GMT, "\t[-V] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_colon_OPT, GMT_b_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT);
        
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Specify file name for output distance grid file.\n");
	GMT_inc_syntax (GMT, 'I', 0);
        
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_message (GMT, "\t   If no files are given, standard input is read (but see -Q).\n");
	GMT_message (GMT, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory]. Not used with -Q.\n");
	GMT_message (GMT, "\t-D Skip repeated input vertex at the end of a closed segment.\n");
	GMT_message (GMT, "\t-E Assign to grid nodes the Voronoi polygon ID [Calculate distances].\n");
	GMT_message (GMT, "\t-L Set distance unit arc (d)egree, m(e)ter, (f)eet, (k)m, arc (m)inute, (M)ile, (n)autical mile, or arc (s)econd [e].\n");
	GMT_message (GMT, "\t   PROJ_ELLIPSOID determines if geodesic or gerat-circle distances are used.\n");
	GMT_message (GMT, "\t-N Specify node filename for the Voronoi polygons (sphtriangulate -N output).\n");
	GMT_message (GMT, "\t-Q Specify table with Voronoi polygons in sphtriangulate -Qv format\n");
	GMT_message (GMT, "\t   [Default performs Voronoi construction on input data first].\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   If no region is specified we default to the entire world [-Rg].\n");
	GMT_explain_options (GMT, "VC2hiF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_sphdistance_parse (struct GMTAPI_CTRL *C, struct SPHDISTANCE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sphdistance and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = TRUE;
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				break;
			case 'E':
				Ctrl->E.active = TRUE;
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				if (!(opt->arg && strchr (GMT_LEN_UNITS, opt->arg[0]))) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -L%s\n", GMT_LEN_UNITS_DISPLAY);
					n_errors++;
				}
				else
					Ctrl->L.unit = opt->arg[0];
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				Ctrl->Q.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && GMT->common.b.active[GMT_IN] && !Ctrl->N.active, "Syntax error: Binary input data (-bi) with -Q also requires -N.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_sphdistance_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_sphdistance (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG row, col, n = 0, n_dup = 0, n_set = 0, ij, ii, s_row, n_row, w_col, e_col, side;
	GMT_LONG n_fields,  n_alloc, p_alloc = 0, nx1, error = FALSE, first = FALSE;
	GMT_LONG node, vertex, node_stop, node_new, vertex_new, node_last, vertex_last;

	double first_x = 0.0, first_y = 0.0, X[3], *grid_lon = NULL, *grid_lat = NULL, *in = NULL;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL;

	struct GMT_GRID *Grid = NULL;
	struct SPHDISTANCE_CTRL *Ctrl = NULL;
	struct STRIPACK T;
	struct GMT_LINE_SEGMENT *P = NULL;
	struct GMT_DATASET *Qin = NULL;
	struct GMT_TABLE *Table = NULL;
	struct STRIPACK_VORONOI *V = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_sphdistance_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_sphdistance_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_sphdistance", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRb:", "himrs" GMT_OPT("F"), options))) Return (error);
	GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	Ctrl = (struct SPHDISTANCE_CTRL *) New_sphdistance_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sphdistance_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphdistance main code ----------------------------*/

	GMT_memset (&T, 1, struct STRIPACK);
	GMT_create_grid (GMT, &Grid);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	GMT_init_distaz (GMT, Ctrl->L.unit, 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);

	if (!GMT->common.R.active) {	/* Default to a global grid */
		Grid->header->wesn[XLO] = 0.0;	Grid->header->wesn[XHI] = 360.0;	Grid->header->wesn[YLO] = -90.0;	Grid->header->wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if (Ctrl->Q.active) {	/* Expect a single file with Voronoi polygons */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		GMT_report (GMT, GMT_MSG_NORMAL, "Read Volonoi polygons from %s ...", Ctrl->Q.file);
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, Ctrl->Q.file, &Qin)) Return ((error = GMT_DATA_READ_ERROR));
		Table = Qin->table[0];	/* Only one table in a file */
		GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld segments\n", Table->n_segments);
	 	lon = GMT_memory (GMT, NULL, Table->n_segments, double);
	 	lat = GMT_memory (GMT, NULL, Table->n_segments, double);
		if (Ctrl->N.active) {	/* Must get nodes from separate file */
			struct GMT_DATASET *Nin = NULL;
			struct GMT_TABLE *NTable = NULL;
			if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
			GMT_report (GMT, GMT_MSG_NORMAL, "Read Nodes from %s ...", Ctrl->N.file);
			if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->N.file, &Nin)) Return ((error = GMT_DATA_READ_ERROR));
			NTable = Nin->table[0];	/* Only one table in a file with a single segment */
			if (NTable->n_segments != 1) {
				GMT_report (GMT, GMT_MSG_FATAL, "File %s can only have 1 segment!\n", Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Table->n_segments != NTable->n_records) {
				GMT_report (GMT, GMT_MSG_FATAL, "Files %s and %s do not have same number of items!\n", Ctrl->Q.file, Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			GMT_memcpy (lon, NTable->segment[0]->coord[GMT_X], NTable->n_records, double);
			GMT_memcpy (lat, NTable->segment[0]->coord[GMT_Y], NTable->n_records, double);
			GMT_Destroy_Data (API, GMT_ALLOCATED, &Nin);
			GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld records\n", NTable->n_records);
		}
		else {	/* Get extract them from the segment header */
			for (node = 0; node < Table->n_segments; node++) {
				sscanf (Table->segment[node]->header, "%*s %*d %lf %lf", &lon[node], &lat[node]);
			}
		}
	}
	else {	/* Must process input point/line data */
		if ((error = GMT_set_cols (GMT, GMT_IN, 2))) Return (error);
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

		if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, 0, 0, double);
		n_alloc = GMT_malloc3 (GMT, xx, yy, zz, 0, 0, double);
		
		n = 0;
		while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, &in)) != EOF) {	/* Keep returning records until we reach EOF */

			if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */

			while (GMT_REC_IS_SEG_HEADER (GMT)) {	/* Segment header, get next record */
				n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, &in);	
				first_x = in[GMT_X];	first_y = in[GMT_Y];
				first = TRUE;
			}
			if (Ctrl->D.active && !first) {	/* Look for duplicate point at end of segments that replicate start point */
				if (in[GMT_X] == first_x && in[GMT_Y] == first_y) {	/* If any point after the first matches the first */
					n_dup++;
					continue;
				}
			}
			/* Convert lon,lat in degrees to Cartesian x,y,z triplets */
			GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, TRUE);
			xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];
			if (!Ctrl->C.active) {
				lon[n] = in[GMT_X];	lat[n] = in[GMT_Y];
			}
			
			if (++n == n_alloc) {	/* Get more memory */
				if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, n, n_alloc, double);
				n_alloc = GMT_malloc3 (GMT, xx, yy, zz, n, n_alloc, double);
			}
			first = FALSE;
		}

		if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, 0, n, double);
		n_alloc = GMT_malloc3 (GMT, xx, yy, zz, 0, n, double);

		if (Ctrl->D.active && n_dup) GMT_report (GMT, GMT_MSG_NORMAL, "Skipped %ld duplicate points in segments\n", n_dup);
		GMT_report (GMT, GMT_MSG_NORMAL, "Do Voronoi construction using %ld points\n", n);

		T.mode = VORONOI;
		stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
		GMT_free (GMT, T.D.tri);	/* Don't need the triangulation */
		if (Ctrl->C.active) {	/* Recompute lon,lat and set pointers */
			cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
			lon = xx;
			lat = yy;
		}
		GMT_free (GMT,  zz);
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	
	/* OK, time to create and work on the distance grid */
	
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	GMT_report (GMT, GMT_MSG_NORMAL, "Start processing distance grid\n");

	nx1 = (Grid->header->registration) ? Grid->header->nx : Grid->header->nx - 1;
	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	grid_lon = GMT_memory (GMT, NULL, Grid->header->nx, double);
	grid_lat = GMT_memory (GMT, NULL, Grid->header->ny, double);
	for (col = 0; col < Grid->header->nx; col++) grid_lon[col] = GMT_grd_col_to_x (GMT, col, Grid->header);
	for (row = 0; row < Grid->header->ny; row++) grid_lat[row] = GMT_grd_row_to_y (GMT, row, Grid->header);
	
	if (Ctrl->Q.active) {	/* Pre-chewed, just get number of nodes */
		n = Table->n_segments;
	}
	else {	/* Need a single polygon structure that we reuse for each polygon */
		P = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);	/* Needed as pointer below */
		P->coord = GMT_memory (GMT, NULL, 2, double *);	/* Needed as pointers below */
		P->min = GMT_memory (GMT, NULL, 2, double);	/* Needed to hold min lon/lat */
		P->max = GMT_memory (GMT, NULL, 2, double);	/* Needed to hold max lon/lat */

		p_alloc = GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], GMT_TINY_CHUNK, 0, double);
	
		V = &T.V;
	}
	for (node = 0; node < n; node++) {

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing polygon %7ld\r", node);
		if (Ctrl->Q.active) {	/* Just point to next polygon */
			P = Table->segment[node];
		}
		else {	/* Obtain current polygon from Voronoi listings */
			node_new = node_stop = V->lend[node];
			vertex_new = V->listc[node_new];

			/* Each iteration of this DO walks along one side of the polygon,
			   considering the subtriangle NODE --> VERTEX_LAST --> VERTEX. */

			vertex = 0;
		    	do {
				node_last = node_new;
				node_new = V->lptr[node_last];
				vertex_last = vertex_new;
				vertex_new = V->listc[node_new];

				P->coord[GMT_X][vertex] = V->lon[vertex_last];
				P->coord[GMT_Y][vertex] = V->lat[vertex_last];
				if (P->coord[GMT_X][vertex] < 0.0) P->coord[GMT_X][vertex] += 360.0;
				if (P->coord[GMT_X][vertex] == 360.0) P->coord[GMT_X][vertex] = 0.0;
				vertex++;
				if (vertex == p_alloc) p_alloc = GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, p_alloc, double);

				/* When we reach the vertex where we started, we are done with this polygon */
			} while (node_new != node_stop);
			P->coord[GMT_X][vertex] = P->coord[GMT_X][0];	/* Close polygon explicitly */
			P->coord[GMT_Y][vertex] = P->coord[GMT_Y][0];
			if (++vertex == p_alloc) p_alloc = GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, p_alloc, double);
			P->n_rows = vertex;
		}
		
		/* Here we have the polygon in P */
		
		prepare_polygon (GMT, P);	/* Determine the enclosing sector */

		s_row = GMT_grd_y_to_row (GMT, P->min[GMT_Y], Grid->header);
		n_row = GMT_grd_y_to_row (GMT, P->max[GMT_Y], Grid->header);
		w_col = GMT_grd_x_to_col (GMT, P->min[GMT_X], Grid->header);
		e_col = GMT_grd_x_to_col (GMT, P->max[GMT_X], Grid->header);

		for (row = n_row; row <= s_row; row++) {	/* For each scanline intersecting this polygon */
			for (ii = w_col; ii <= e_col; ii++) {	/* March along the scanline */
				col = (ii >= 0) ? ii : ii + nx1;
				side = GMT_inonout_sphpol (GMT, grid_lon[col], grid_lat[row], P);	/* No holes to worry about here */
				if (side == 0) continue;	/* Outside spherical polygon */
				ij = GMT_IJP (Grid->header, row, col);
				Grid->data[ij] = (Ctrl->E.active) ? (float)node : (float)GMT_distance (GMT, grid_lon[col], grid_lat[row], lon[node], lat[node]);
				n_set++;
			}
		}
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Processing polygon %7ld\n", node);
	
	if (!Ctrl->Q.active) {
		GMT_free (GMT, P->coord[GMT_X]);
		GMT_free (GMT, P->coord[GMT_Y]);
		GMT_free (GMT, P->min);
		GMT_free (GMT, P->max);
		GMT_free (GMT, P->coord);
		GMT_free (GMT, P);
		GMT_free (GMT, T.V.lon);
		GMT_free (GMT, T.V.lat);
		GMT_free (GMT, T.V.lend);
		GMT_free (GMT, T.V.listc);
		GMT_free (GMT, T.V.lptr);
		GMT_free (GMT, xx);
		GMT_free (GMT, yy);
	}
	GMT_free (GMT, grid_lon);	GMT_free (GMT, grid_lat);
	if (!Ctrl->C.active) {
		GMT_free (GMT, lon);
		GMT_free (GMT, lat);
	}
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->G.file, Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "Spherical distance calculation completed, %ld nodes visited (at least once)\n", n_set);
	
	Return (GMT_OK);
}
