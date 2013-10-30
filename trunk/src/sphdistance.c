/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2008-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * so that f2c libs were not needed.  For any translation errors, blame me.
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2011
 * Version:	5 API
 *
 */
 
#define THIS_MODULE_NAME	"sphdistance"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make grid of distances to nearest points on a sphere"

#include "gmt_dev.h"
#include "gmt_sph.h"

#define GMT_PROG_OPTIONS "-:RVbhirs" GMT_OPT("F")

struct SPHDISTANCE_CTRL {
	struct C {	/* -C */
		bool active;
	} C;
	struct E {	/* -E */
		bool active;
	} E;
	struct G {	/* -G<maskfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -L<unit>] */
		bool active;
		char unit;
	} L;
	struct N {	/* -N */
		bool active;
		char *file;
	} N;
	struct Q {	/* -Q */
		bool active;
		char *file;
	} Q;
};

void prepare_polygon (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *P)
{
	/* Set the min/max extent of this polygon and determine if it
	 * is a polar cap; if so set the required metadata flags */
	uint64_t row;
	double lon_sum = 0.0, lat_sum = 0.0, dlon;
	
	GMT_set_seg_minmax (GMT, P);	/* Set the domain of the segment */
	
	/* Then loop over points to accumulate sums */
	
	for (row = 1; row < P->n_rows; row++) {	/* Start at row = 1 since (a) 0'th point is repeated at end and (b) we are doing differences */
		GMT_set_delta_lon (P->coord[GMT_X][row-1], P->coord[GMT_X][row], dlon);
		lon_sum += dlon;
		lat_sum += P->coord[GMT_Y][row];
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
	return (C);
}

void Free_sphdistance_Ctrl (struct GMT_CTRL *GMT, struct SPHDISTANCE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	if (C->N.file) free (C->N.file);	
	if (C->Q.file) free (C->Q.file);	
	GMT_free (GMT, C);	
}

int GMT_sphdistance_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: sphdistance [<table>] -G<outgrid> %s [-C] [-E]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L<unit>] [-N<nodetable>] [-Q<voronoitable>] [%s] [%s]\n", GMT_V_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT);
        
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output distance grid file.\n");
	GMT_Option (API, "I");
        
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read (but see -Q).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory]. Not used with -Q.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Assign to grid nodes the Voronoi polygon ID [Calculate distances].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set distance unit arc (d)egree, m(e)ter, (f)eet, (k)m, arc (m)inute, (M)ile, (n)autical mile, or arc (s)econd [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   PROJ_ELLIPSOID determines if geodesic or gerat-circle distances are used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Specify node filename for the Voronoi polygons (sphtriangulate -N output).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Specify table with Voronoi polygons in sphtriangulate -Qv format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default performs Voronoi construction on input data first].\n");
	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no region is specified we default to the entire world [-Rg].\n");
	GMT_Option (API, "V,bi2,h,i,r,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_sphdistance_parse (struct GMT_CTRL *GMT, struct SPHDISTANCE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sphdistance and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				if (GMT_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -D option is deprecated; duplicates are automatically removed.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'E':
				Ctrl->E.active = true;
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				if (!(opt->arg && strchr (GMT_LEN_UNITS, opt->arg[0]))) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -L%s\n", GMT_LEN_UNITS_DISPLAY);
					n_errors++;
				}
				else
					Ctrl->L.unit = opt->arg[0];
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				Ctrl->Q.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

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

int GMT_sphdistance (void *V_API, int mode, void *args)
{
	bool first = false, periodic, duplicate_col;
	int error = 0, s_row, south_row, north_row, w_col, e_col;

	unsigned int row, col, p_col, west_col, east_col, nx1;
	uint64_t n_dup = 0, n_set = 0, side, ij, node, n = 0;
	uint64_t vertex, node_stop, node_new, vertex_new, node_last, vertex_last;

	size_t n_alloc, p_alloc = 0;

	double first_x = 0.0, first_y = 0.0, prev_x = 0.0, prev_y = 0.0, X[3];
	double *grid_lon = NULL, *grid_lat = NULL, *in = NULL;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL;

	struct GMT_GRID *Grid = NULL;
	struct SPHDISTANCE_CTRL *Ctrl = NULL;
	struct STRIPACK T;
	struct GMT_DATASEGMENT *P = NULL;
	struct GMT_DATASET *Qin = NULL;
	struct GMT_DATATABLE *Table = NULL;
	struct STRIPACK_VORONOI *V = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_sphdistance_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_sphdistance_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_sphdistance_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_sphdistance_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sphdistance_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphdistance main code ----------------------------*/

	GMT_memset (&T, 1, struct STRIPACK);

	GMT_init_distaz (GMT, Ctrl->L.unit, 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);

	if (!GMT->common.R.active) {	/* Default to a global grid */
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;	GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if (Ctrl->Q.active) {	/* Expect a single file with Voronoi polygons */
		GMT_Report (API, GMT_MSG_VERBOSE, "Read Volonoi polygons from %s ...", Ctrl->Q.file);
		if ((Qin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->Q.file, NULL)) == NULL) {
			Return (API->error);
		}
		Table = Qin->table[0];	/* Only one table in a file */
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " segments\n", Table->n_segments);
	 	lon = GMT_memory (GMT, NULL, Table->n_segments, double);
	 	lat = GMT_memory (GMT, NULL, Table->n_segments, double);
		if (Ctrl->N.active) {	/* Must get nodes from separate file */
			struct GMT_DATASET *Nin = NULL;
			struct GMT_DATATABLE *NTable = NULL;
			if ((error = GMT_set_cols (GMT, GMT_IN, 3)) != GMT_OK) {
				Return (error);
			}
			GMT_Report (API, GMT_MSG_VERBOSE, "Read Nodes from %s ...", Ctrl->N.file);
			if ((Nin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
				Return (API->error);
			}
			NTable = Nin->table[0];	/* Only one table in a file with a single segment */
			if (NTable->n_segments != 1) {
				GMT_Report (API, GMT_MSG_NORMAL, "File %s can only have 1 segment!\n", Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Table->n_segments != NTable->n_records) {
				GMT_Report (API, GMT_MSG_NORMAL, "Files %s and %s do not have same number of items!\n", Ctrl->Q.file, Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			GMT_memcpy (lon, NTable->segment[0]->coord[GMT_X], NTable->n_records, double);
			GMT_memcpy (lat, NTable->segment[0]->coord[GMT_Y], NTable->n_records, double);
			if (GMT_Destroy_Data (API, &Nin) != GMT_OK) {
				Return (API->error);
			}
			GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " records\n", NTable->n_records);
		}
		else {	/* Get extract them from the segment header */
			for (node = 0; node < Table->n_segments; node++) {
				sscanf (Table->segment[node]->header, "%*s %*d %lf %lf", &lon[node], &lat[node]);
			}
		}
	}
	else {	/* Must process input point/line data */
		if ((error = GMT_set_cols (GMT, GMT_IN, 2)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {
			Return (API->error);	/* Enables data input and sets access mode */
		}

		n_alloc = 0;
		if (!Ctrl->C.active) GMT_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
		n_alloc = 0;
		GMT_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);
		
		n = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TABLE_HEADER (GMT)) 	/* Skip all table headers */
					continue;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
				else if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {			/* Parse segment headers */
					first = true;
					continue;
				}
			}

			/* Data record to process - avoid duplicate points as stripack_lists cannot handle that */

			if (first) {	/* Beginning of new segment; keep track of the very first coordinate in case of duplicates */
				first_x = prev_x = in[GMT_X];	first_y = prev_y = in[GMT_Y];
			}
			else {	/* Look for duplicate point at end of segments that replicate start point */
				if (in[GMT_X] == first_x && in[GMT_Y] == first_y) {	/* If any point after the first matches the first */
					n_dup++;
					continue;
				}
				if (n && in[GMT_X] == prev_x && in[GMT_Y] == prev_y) {	/* Identical neighbors */
					n_dup++;
					continue;
				}
				prev_x = in[GMT_X];	prev_y = in[GMT_Y];
			}
			
			/* Convert lon,lat in degrees to Cartesian x,y,z triplets */
			GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);
	
			xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];
			if (!Ctrl->C.active) {
				lon[n] = in[GMT_X];	lat[n] = in[GMT_Y];
			}
			
			if (++n == n_alloc) {	/* Get more memory */
				if (!Ctrl->C.active) { size_t n_tmp = n_alloc; GMT_malloc2 (GMT, lon, lat, n, &n_tmp, double); }
				GMT_malloc3 (GMT, xx, yy, zz, n, &n_alloc, double);
			}
			first = false;
		} while (true);

		n_alloc = n;
		if (!Ctrl->C.active) GMT_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
		GMT_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);

		if (n_dup) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %" PRIu64 " duplicate points in segments\n", n_dup);
		GMT_Report (API, GMT_MSG_VERBOSE, "Do Voronoi construction using %" PRIu64 " points\n", n);

		T.mode = VORONOI;
		stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
		GMT_free (GMT, T.D.tri);	/* Don't need the triangulation */
		if (Ctrl->C.active) {	/* Recompute lon,lat and set pointers */
			cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
			lon = xx;
			lat = yy;
		}
		GMT_free (GMT,  zz);
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}
	
	/* OK, time to create and work on the distance grid */

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);
	GMT_Report (API, GMT_MSG_VERBOSE, "Start processing distance grid\n");

	grid_lon = GMT_grd_coord (GMT, Grid->header, GMT_X);
	grid_lat = GMT_grd_coord (GMT, Grid->header, GMT_Y);

	nx1 = (Grid->header->registration == GMT_GRID_PIXEL_REG) ? Grid->header->nx : Grid->header->nx - 1;
	periodic = GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	duplicate_col = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */

	if (Ctrl->Q.active) {	/* Pre-chewed, just get number of nodes */
		n = Table->n_segments;
	}
	else {	/* Need a single polygon structure that we reuse for each polygon */
		P = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);	/* Needed as pointer below */
		P->coord = GMT_memory (GMT, NULL, 2, double *);	/* Needed as pointers below */
		P->min = GMT_memory (GMT, NULL, 2, double);	/* Needed to hold min lon/lat */
		P->max = GMT_memory (GMT, NULL, 2, double);	/* Needed to hold max lon/lat */

		GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], GMT_TINY_CHUNK, &p_alloc, double);
	
		V = &T.V;
	}
	for (node = 0; node < n; node++) {

		GMT_Report (API, GMT_MSG_VERBOSE, "Processing polygon %7ld\r", node);
		if (Ctrl->Q.active) {	/* Just point to next polygon */
			P = Table->segment[node];
		}
		else {	/* Obtain current polygon from Voronoi listings */
			node_new = node_stop = V->lend[node];
			vertex_new = V->listc[node_new];

			/* Each iteration of this do-loop walks along one side of the polygon,
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
				if (vertex == p_alloc) GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, &p_alloc, double);

				/* When we reach the vertex where we started, we are done with this polygon */
			} while (node_new != node_stop);
			P->coord[GMT_X][vertex] = P->coord[GMT_X][0];	/* Close polygon explicitly */
			P->coord[GMT_Y][vertex] = P->coord[GMT_Y][0];
			if ((++vertex) == p_alloc) GMT_malloc2 (GMT, P->coord[GMT_X], P->coord[GMT_Y], vertex, &p_alloc, double);
			P->n_rows = vertex;
		}
		
		/* Here we have the polygon in P */
		
		prepare_polygon (GMT, P);	/* Determine the enclosing sector */

		south_row = (int)GMT_grd_y_to_row (GMT, P->min[GMT_Y], Grid->header);
		north_row = (int)GMT_grd_y_to_row (GMT, P->max[GMT_Y], Grid->header);
		w_col  = (int)GMT_grd_x_to_col (GMT, P->min[GMT_X], Grid->header);
		while (w_col < 0) w_col += nx1;
		west_col = w_col;
		e_col = (int)GMT_grd_x_to_col (GMT, P->max[GMT_X], Grid->header);
		while (e_col < w_col) e_col += nx1;
		east_col = e_col;
		/* So here, any polygon will have a positive (or 0) west_col with an east_col >= west_col */
		for (s_row = north_row; s_row <= south_row; s_row++) {	/* For each scanline intersecting this polygon */
			if (s_row < 0) continue;	/* North of region */
			row = s_row; if (row >= Grid->header->ny) continue;	/* South of region */
			for (p_col = west_col; p_col <= east_col; p_col++) {	/* March along the scanline using col >= 0 */
				if (p_col >= Grid->header->nx) {	/* Off the east end of the grid */
					if (periodic)	/* Just shuffle to the corresponding point inside the global grid */
						col = p_col - nx1;
					else		/* Sorry, really outside the region */
						continue;
				}
				else
					col = p_col;
				side = GMT_inonout_sphpol (GMT, grid_lon[col], grid_lat[row], P);	/* No holes to worry about here */
				if (side == 0) continue;	/* Outside spherical polygon */
				ij = GMT_IJP (Grid->header, row, col);
				Grid->data[ij] = (Ctrl->E.active) ? (float)node : (float)GMT_distance (GMT, grid_lon[col], grid_lat[row], lon[node], lat[node]);
				n_set++;
				if (duplicate_col) {	/* Duplicate the repeating column on the other side of this one */
					if (col == 0) Grid->data[ij+nx1] = Grid->data[ij], n_set++;
					else if (col == nx1) Grid->data[ij-nx1] = Grid->data[ij], n_set++;
				}
			}
		}
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing polygon %7ld\n", node);
	
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
	
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
		Return (API->error);
	}

	if (n_set > Grid->header->nm) n_set = Grid->header->nm;	/* Not confuse the public */
	GMT_Report (API, GMT_MSG_VERBOSE, "Spherical distance calculation completed, %" PRIu64 " nodes visited (at least once)\n", n_set);
	
	Return (GMT_OK);
}
