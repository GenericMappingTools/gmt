/*--------------------------------------------------------------------
 *	$Id: sphdistance_func.c,v 1.6 2011-04-12 13:06:44 remko Exp $
 *
 *	Copyright (c) 2008-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Spherical nearest distances - via Voronoi polygons.  We read input
 * data, assumed to be things like coastlines, and want to create a grid
 * with distances to the nearest line.  The approach here is to break
 * the data into voronoi polygons and then visit all nodes inside each
 * polygon and use geodesic distance calculation from each node to the
 * unique Voronoi interior data node.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *  and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *  Software, 23 (3), 416-434.
 * We translate to C using f2c -r8 and link with -lf2c
 *
 * Author:	Paul Wessel
 * Date:	16-FEB-2008
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

void prepare_polygon (struct GMT_LINE_SEGMENT *P)
{
	GMT_LONG i, quad_no, n_quad;
	GMT_LONG quad[4] = {FALSE, FALSE, FALSE, FALSE};
	double lon_sum = 0.0, lat_sum = 0.0, lon, dlon;
	double xmin1 = 360.0, xmin2 = 360.0, xmax1 = -360.0, xmax2 = -360.0;
	
	P->min[GMT_X] = P->max[GMT_X] = P->coord[GMT_X][0];
	P->min[GMT_Y] = P->max[GMT_Y] = P->coord[GMT_Y][0];
	for (i = 1; i < P->n_rows; i++) {
		lon = P->coord[GMT_X][i];
		while (lon < 0.0) lon += 360.0;	/* Start off with everything in 0-360 range */
		xmin1 = MIN (lon, xmin1);
		xmax1 = MAX (lon, xmax1);
		quad_no = (GMT_LONG)floor (lon/90.0);	/* Yields quadrants 0-3 */
		if (quad_no == 4) quad_no = 0;		/* When lon == 360.0 */
		quad[quad_no] = TRUE;
		while (lon > 180.0) lon -= 360.0;	/* Switch to -180+/180 range */
		xmin2 = MIN (lon, xmin2);
		xmax2 = MAX (lon, xmax2);
		dlon = P->coord[GMT_X][i] - P->coord[GMT_X][i-1];
		if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
		lon_sum += dlon;
		lat_sum += P->coord[GMT_Y][i];
		if (P->coord[GMT_Y][i] < P->min[GMT_Y]) P->min[GMT_Y] = P->coord[GMT_Y][i];
		if (P->coord[GMT_Y][i] > P->max[GMT_Y]) P->max[GMT_Y] = P->coord[GMT_Y][i];
		if (P->coord[GMT_X][i] < P->min[GMT_X]) P->min[GMT_X] = P->coord[GMT_X][i];
		if (P->coord[GMT_X][i] > P->max[GMT_X]) P->max[GMT_X] = P->coord[GMT_X][i];
	}
	if (GMT_360_RANGE (lon_sum, 0.0)) {	/* Contains a pole */
		if (lat_sum < 0.0) { /* S */
			P->pole = -1;
			P->min[GMT_Y] = -90.0;
		}
		else {	/* N */
			P->pole = +1;
			P->max[GMT_Y] = 90.0;
		}
		P->min[GMT_X] = 0.0;
		P->max[GMT_X] = 360.0;
	}
	else {
		P->pole = 0;
		n_quad = quad[0] + quad[1] + quad[2] + quad[3];	/* How many quadrants had data */
		if (quad[0] && quad[3]) {	/* Longitudes on either side of Greenwhich only, must use -180/+180 notation */
			P->min[GMT_X] = xmin2;
			P->max[GMT_X] = xmax2;
		}
		else if (quad[1] && quad[2]) {	/* Longitudes on either side of the date line, must user 0/360 notation */
			P->min[GMT_X] = xmin1;
			P->max[GMT_X] = xmax1;
		}
		else if (n_quad == 2 && ((quad[0] && quad[2]) || (quad[1] && quad[3]))) {	/* Funny quadrant gap, pick shortest longitude extent */
			if ((xmax1 - xmin1) < (xmax2 - xmin2)) {	/* 0/360 more compact */
				P->min[GMT_X] = xmin1;
				P->max[GMT_X] = xmax1;
			}
			else {						/* -180/+180 more compact */
				P->min[GMT_X] = xmin2;
				P->max[GMT_X] = xmax2;
			}
		}
		else {						/* Either will do, use default settings */
			P->min[GMT_X] = xmin1;
			P->max[GMT_X] = xmax1;
		}
		if (P->min[GMT_X] > P->max[GMT_X]) P->min[GMT_X] -= 360.0;
		if (P->min[GMT_X] < 0.0 && P->max[GMT_X] < 0.0) P->min[GMT_X] += 360.0, P->max[GMT_X] += 360.0;
	}
}

void *New_sphdistance_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHDISTANCE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPHDISTANCE_CTRL);
	C->L.unit = 'E';	/* Default is meter distances */
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
	GMT_message (GMT, "usage: sphdistance [<infiles>] -G<grdfile> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t[-C] [-D] [-E] [-L<unit>] [-N<nodefile>] [-Q<voronoifile>]\n");
	GMT_message (GMT, "\t[-V] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_colon_OPT, GMT_b_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT);
        
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Specify file name for output distance grid file.\n");
	GMT_inc_syntax (GMT, 'I', 0);
        
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\tinfiles (in ASCII) has 2 or more columns.  If no file(s) is given, standard input is read (but see -Q).\n");
	GMT_message (GMT, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory]. Not used with -Q.\n");
	GMT_message (GMT, "\t-D Used to skip repeated input vertex at the end of a closed segment\n");
	GMT_message (GMT, "\t-E Assign to grid nodes the Voronoi polygon ID [Calculate distances]\n");
	GMT_message (GMT, "\t-L Specify distance unit as (f)eet, m(e)ter, (k)m, (M)ile, (n)autical mile, or (d)egree.\n");
	GMT_message (GMT, "\t   Calculations uses spherical approximations.  Default unit is meters.\n");
	GMT_message (GMT, "\t   Set ELLIPSOID to WGS-84 to get geodesic distances.\n");
	GMT_message (GMT, "\t-N Specify node file for the Voronoi polygons (sphtriangulate -N output)\n");
	GMT_message (GMT, "\t-Q Specify file with Voronoi polygons in sphtriangulate -Qv format\n");
	GMT_message (GMT, "\t   [Default performs Voronoi construction on input data first]\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   If no region is specified we default to the entire world [-Rg]\n");
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
				if (!(opt->arg && strchr ("defkMn", opt->arg[0]))) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -Ld|e|f|k|M|n]\n");
					n_errors++;
				}
				else
					Ctrl->L.unit = (char)toupper (opt->arg[0]);	/* Make sure we pass upper so Geodesic is possible */
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

	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->current.io.io_header[GMT_IN], "Syntax error: Binary input data cannot have header -h\n");
	if (GMT_native_binary (GMT, GMT_IN) && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += GMT_check_condition (GMT, GMT_native_binary (GMT, GMT_IN) && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && GMT->common.b.active[GMT_IN] && !Ctrl->N.active, "Syntax error: Binary input data (-bi) with -Q also requires -N.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_sphdistance_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_sphdistance (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG row, col, n = 0, n_dup = 0, n_set = 0, ij, ii, s_row, n_row, w_col, e_col, side;
	GMT_LONG n_fields,  n_alloc, p_alloc = 0, nx1, error = FALSE, first = FALSE;
	GMT_LONG node, vertex, node_stop, node_new, vertex_new, node_last, vertex_last;

	double first_x = 0.0, first_y = 0.0, X[3];
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL;
	double *grid_lon = NULL, *grid_lat = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct SPHDISTANCE_CTRL *Ctrl = NULL;
	struct STRIPACK T;
	struct GMT_LINE_SEGMENT *P = NULL;
	struct GMT_DATASET *Qin = NULL;
	struct GMT_TABLE *Table = NULL;
	struct STRIPACK_VORONOI *V = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_sphdistance_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_sphdistance_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_sphdistance", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRb:", "himrs" GMT_OPT("F"), options))) Return (error);
	Ctrl = (struct SPHDISTANCE_CTRL *) New_sphdistance_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sphdistance_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphdistance main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	GMT_init_distaz (GMT, Ctrl->L.unit, 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);

	if (GMT->common.b.active[GMT_IN] && GMT->current.setting.verbose) {
		char *type[2] = {"double", "single"};
		GMT_report (GMT, GMT_MSG_NORMAL, "Expects %ld-column %s-precision binary data\n", GMT->common.b.ncol[GMT_IN], type[GMT->common.b.single_precision[GMT_IN]]);
	}
	if (!GMT->common.R.active) {	/* Default to a global grid */
		Grid->header->wesn[XLO] = 0.0;	Grid->header->wesn[XHI] = 360.0;	Grid->header->wesn[YLO] = -90.0;	Grid->header->wesn[YHI] = 90.0;
	}

#ifdef SET_IO_MODE
	GMT_setmode (GMT, GMT_OUT);
#endif

	/* Now we are ready to take on some input values */

	if (Ctrl->Q.active) {	/* Expect a single file with Voronoi polygons */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
		GMT_report (GMT, GMT_MSG_NORMAL, "Read Volonoi polygons from %s ...", Ctrl->Q.file);
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->Q.file, (void **)&Qin)) Return ((error = GMT_DATA_READ_ERROR));
		Table = Qin->table[0];	/* Only one table in a file */
		GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld segments\n", Table->n_segments);
	 	lon = GMT_memory (GMT, NULL, Table->n_segments, double);
	 	lat = GMT_memory (GMT, NULL, Table->n_segments, double);
		if (Ctrl->N.active) {	/* Must get nodes from separate file */
			struct GMT_DATASET *Nin = NULL;
			struct GMT_TABLE *NTable = NULL;
			if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
			GMT_report (GMT, GMT_MSG_NORMAL, "Read Nodes from %s ...", Ctrl->N.file);
			if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->N.file, (void **)&Nin)) Return ((error = GMT_DATA_READ_ERROR));
			NTable = Nin->table[0];	/* Only one table in a file with a single segment */
			if (NTable->n_segments != 1) {
				GMT_report (GMT, GMT_MSG_FATAL, "File %s can only have 1 segment!\n", Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Table->n_segments != NTable->n_records) {
				GMT_report (GMT, GMT_MSG_FATAL, "Files %s and %s do not have same number of items!\n", Ctrl->Q.file, Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			for (node = 0; node < NTable->n_records; node++) {
				lon[node] = NTable->segment[0]->coord[GMT_X][node];
				lat[node] = NTable->segment[0]->coord[GMT_Y][node];
			}
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Nin);
			GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld records\n", NTable->n_records);
		}
		else {
			for (node = 0; node < Table->n_segments; node++) {
				sscanf (Table->segment[node]->header, "%*s %*s %*d %lf %lf", &lon[node], &lat[node]);
			}
		}
	}
	else {	/* Must process input point/line data */
		if ((error = GMT_set_cols (GMT, GMT_IN, 2))) Return (error);
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

		if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, 0, 0, double);
		n_alloc = GMT_malloc3 (GMT, xx, yy, zz, 0, 0, double);
		
		n = 0;
		while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

			if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */

			while (GMT_REC_IS_SEG_HEADER (GMT)) {	/* Segment header, get next record */
				n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in);	
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
			xx[n] = X[0];	yy[n] = X[1];	zz[n] = X[2];
			if (!Ctrl->C.active) {
				lon[n] = in[GMT_X];
				lat[n] = in[GMT_Y];
			}
			
			n++;
			if (n == n_alloc) {	/* Get more memory */
				if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, n, n_alloc, double);
				n_alloc = GMT_malloc3 (GMT, xx, yy, zz, n, n_alloc, double);
			}
			first = FALSE;
		}

		if (!Ctrl->C.active) (void)GMT_malloc2 (GMT, lon, lat, 0, n, double);
		n_alloc = GMT_malloc3 (GMT, xx, yy, zz, 0, n, double);

		if (Ctrl->D.active && n_dup) GMT_report (GMT, GMT_MSG_NORMAL, "Skipped %ld duplicate points in segments\n", n_dup);
		GMT_report (GMT, GMT_MSG_NORMAL, "Do Voronoi construction using %ld points\n", n);

		GMT_memset (&T, 1, struct STRIPACK);
		T.mode = VORONOI;
		stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
		GMT_free (GMT, T.D.tri);
		if (Ctrl->C.active) {	/* Recompute lon,lat and set pointers */
			cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
			lon = xx;
			lat = yy;
		}
		GMT_free (GMT,  zz);
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
	
	/* OK, time to work on the distance grid */
	
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	GMT_report (GMT, GMT_MSG_NORMAL, "Start processing distance grid\n");

	nx1 = (Grid->header->registration) ? Grid->header->nx : Grid->header->nx - 1;
	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	grid_lon = GMT_memory (GMT, NULL, Grid->header->nx, double);
	grid_lat = GMT_memory (GMT, NULL, Grid->header->ny, double);
	for (col = 0; col < Grid->header->nx; col++) grid_lon[col] = GMT_grd_col_to_x (col, Grid->header);
	for (row = 0; row < Grid->header->ny; row++) grid_lat[row] = GMT_grd_row_to_y (row, Grid->header);
	
	if (Ctrl->Q.active) {	/* Pre-chewed, just get number of nodes */
		n = Table->n_segments;
	}
	else {	/* Need a single polygon structure that we reuse for each polygon */
		P = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);	/* Needed as pointer below */
		P->coord = GMT_memory (GMT, NULL, 2, double *);	/* Needed as pointers below */
		P->min = GMT_memory (GMT, NULL, 2, double);		/* Needed to hold min lon/lat */
		P->max = GMT_memory (GMT, NULL, 2, double);		/* Needed to hold max lon/lat */

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
				if (vertex == (int)p_alloc) p_alloc = GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, p_alloc, double);

				/* When we reach the vertex where we started, we are done with this polygon */
			} while (node_new != node_stop);
			P->coord[GMT_X][vertex] = P->coord[GMT_X][0];
			P->coord[GMT_Y][vertex] = P->coord[GMT_Y][0];
			vertex++;
			if (vertex == (int)p_alloc) p_alloc = GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, p_alloc, double);
			P->n_rows = vertex;
		}
		
		/* Here we have the polygon in P */
		
		prepare_polygon (P);	/* Determine the enclosing sector */

		s_row = GMT_grd_y_to_row (P->min[GMT_Y], Grid->header);
		n_row = GMT_grd_y_to_row (P->max[GMT_Y], Grid->header);
		w_col = GMT_grd_x_to_col (P->min[GMT_X], Grid->header);
		e_col = GMT_grd_x_to_col (P->max[GMT_X], Grid->header);

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
	else {
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Qin);
	}
	GMT_free (GMT, grid_lon);
	GMT_free (GMT, grid_lat);
	if (!Ctrl->C.active) {
		GMT_free (GMT, lon);
		GMT_free (GMT, lat);
	}
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);
	GMT_report (GMT, GMT_MSG_NORMAL, "Gridding completed, %ld nodes visited (at least once)\n", n_set);
	
	Return (GMT_OK);
}
