/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2008-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Spherical triangulation - with Delaunay or Voronoi output.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *  and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *  Software, 23 (3), 416-434.
 * We translated to C using f2c -r8 and and manually edited the code
 * so that f2c libs were not needed.  For any translation errors, blame me.
 *
 * Author:      Paul Wessel
 * Date:	1-AUG-2011
 * Version:	5 API
 *
 */

#define THIS_MODULE_NAME	"sphtriangulate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delaunay or Voronoi construction of spherical lon,lat data"

#include "gmt_dev.h"
#include "gmt_sph.h"

#define GMT_PROG_OPTIONS "-:RVbhis"

struct SPHTRIANGULATE_CTRL {
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
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
		unsigned int mode;	/* 0 is Delaunay, 1 is Voronoi */
	} Q;
	struct T {	/* -T */
		bool active;
	} T;
};

int stripack_delaunay_output (struct GMT_CTRL *GMT, double *lon, double *lat, struct STRIPACK_DELAUNAY *D, uint64_t get_arcs, \
	unsigned int get_area, uint64_t nodes, struct GMT_DATASET *Dout[], char *file0, char *file1)
{	/* Prints out the Delaunay triangles either as polygons (for filling) or arcs (lines). */
	uint64_t i, ij;
	bool do_authalic;
	uint64_t dim[4] = {1, 0, 0, 0}, k;
	double area_sphere = 0.0, area_triangle = GMT->session.d_NaN, V[3][3], R2, y, dist = GMT->session.d_NaN;
	char segment_header[GMT_BUFSIZ];
	struct GMT_DATASEGMENT *S[2] = {NULL, NULL};
	if (get_area == 2) /* Return area in steradians */
		R2 = GMT->current.map.dist[GMT_MAP_DIST].scale = 1.0;
	else {	/* Want area in units like m^2 or km^2 */
		R2 = pow (R2D * GMT->current.proj.M_PR_DEG, 2.0);	/* squared mean radius in meters */
		R2 *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);	/* Get final measure unit for area */
	}
	do_authalic = (get_area && !get_arcs && !GMT_IS_ZERO (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (!get_arcs) {	/* All output polygons consist of three points.  Not explicitly closed (use -L or -G in psxy) */
		dim[GMT_SEG] = D->n;	/* Number of segments */
		dim[GMT_COL] = 2;	/* Just 2 columns */
		dim[GMT_ROW] = 3;	/* All segments has 3 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, file0)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		if (nodes) {	/* Want Voronoi node and area information via Dout[1] */
			dim[GMT_SEG] = 1;	/* Just one segment */
			dim[GMT_COL] = 3 + get_area;	/* Here we use 3-4 columns */
			dim[GMT_ROW] = D->n;	/* One row per node */
			if ((Dout[1] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, file1)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate nodes\n");
				GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
			}
			S[1] = Dout[1]->table[0]->segment[0];
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Output %d unique triangle polygons\n", D->n);
		for (k = ij = 0; k < D->n; k++, ij += TRI_NROW) {	/* For each triangle */
			S[0] = Dout[0]->table[0]->segment[k];	/* Short hand for current triangle segment */
			/* Write segment header with triangle # and the three node numbers */
			if (get_area) {	/* Compute area */
				for (i = 0; i < 3; i++) {
					y = (do_authalic) ? GMT_lat_swap (GMT, lat[D->tri[ij+i]], GMT_LATSWAP_G2A) : lat[D->tri[ij+i]];	/* Convert to authalic latitude */
					GMT_geo_to_cart (GMT, y, lon[D->tri[ij+i]], V[i], true);
				}
				area_triangle = stripack_areas (V[0], V[1], V[2]);
				area_sphere += area_triangle;
				sprintf (segment_header, "Triangle: %" PRIu64 " %" PRIu64 "-%" PRIu64 "-%" PRIu64 " Area: %g", k, D->tri[ij], D->tri[ij+1], D->tri[ij+2], area_triangle * R2);
			}
			else	/* Just a plain header with triangle number */
				sprintf (segment_header, "Triangle: %" PRIu64, k);
			if (nodes) {	/* Output Voronoi node and area information via S[1] */
				S[1]->coord[GMT_X][k] = (double)D->tri[ij];
				S[1]->coord[GMT_Y][k] = (double)D->tri[ij+1];
				S[1]->coord[GMT_Z][k] = (double)D->tri[ij+2];
				if (get_area) S[1]->coord[3][k] = area_triangle * R2;
			}
			S[0]->header = strdup (segment_header);
			for (i = 0; i < 3; i++) {	/* Copy the three vertices */
				S[0]->coord[GMT_X][i] = lon[D->tri[ij+i]];
				S[0]->coord[GMT_Y][i] = lat[D->tri[ij+i]];
			}
		}
		if (get_area) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Total surface area = %g\n", area_sphere * R2);
	}
	else {	/* Want just the arcs (to draw then, probably).  This avoids repeating shared arcs between triangles */
		uint64_t j, ij1, ij2, ij3, n_arcs, kk;
		struct STRPACK_ARC *arc = NULL;

		n_arcs = 3 * D->n;
		arc = GMT_memory (GMT, NULL, n_arcs, struct STRPACK_ARC);
		for (k = ij = ij1 = 0, ij2 = 1, ij3 = 2; k < D->n; k++, ij1 += TRI_NROW, ij2 += TRI_NROW, ij3 += TRI_NROW) {	/* For each triangle */
			arc[ij].begin = D->tri[ij1];	arc[ij].end = D->tri[ij2];	ij++;
			arc[ij].begin = D->tri[ij2];	arc[ij].end = D->tri[ij3];	ij++;
			arc[ij].begin = D->tri[ij1];	arc[ij].end = D->tri[ij3];	ij++;
		}
		for (kk = 0; kk < n_arcs; ++kk)
			if (arc[kk].begin > arc[kk].end)
				uint64_swap (arc[kk].begin, arc[kk].end);

		/* Sort and eliminate duplicate arcs */
		qsort (arc, n_arcs, sizeof (struct STRPACK_ARC), compare_arc);
		for (i = 1, j = 0; i < n_arcs; i++) {
			if (arc[i].begin != arc[j].begin || arc[i].end != arc[j].end) j++;
			arc[j] = arc[i];
		}
		n_arcs = j + 1;
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Output %" PRIu64 " unique triangle arcs\n", n_arcs);

		dim[GMT_SEG] = n_arcs;	/* Number of output arcs = segments */
		dim[GMT_COL] = 2;		/* Only use 2 columns */
		dim[GMT_ROW] = 2;		/* Each arc has 2 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, file0)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate arcs\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		for (i = 0; i < n_arcs; i++) {
			S[0] = Dout[0]->table[0]->segment[i];	/* Shorthand for this output segment */
			S[0]->coord[GMT_X][0] = lon[arc[i].begin];	S[0]->coord[GMT_Y][0] = lat[arc[i].begin];
			S[0]->coord[GMT_X][1] = lon[arc[i].end];	S[0]->coord[GMT_Y][1] = lat[arc[i].end];
			if (get_area) {	/* Compute arc lengths */
				dist = GMT_distance (GMT, S[0]->coord[GMT_X][0], S[0]->coord[GMT_Y][0], S[0]->coord[GMT_X][1], S[0]->coord[GMT_Y][1]);
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " Length: %g", arc[i].begin, arc[i].end, dist);
			}
			else	/* Plain header */
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64, arc[i].begin, arc[i].end);
			S[0]->header = strdup (segment_header);
		}
		GMT_free (GMT, arc);
	}
	return (GMT_OK);
}

int stripack_voronoi_output (struct GMT_CTRL *GMT, uint64_t n, double *lon, double *lat, struct STRIPACK_VORONOI *V, bool get_arcs, \
	unsigned int get_area, uint64_t nodes, struct GMT_DATASET *Dout[], char *file0, char *file1)
{	/* Prints out the Voronoi polygons either as polygons (for filling) or arcs (lines) */
	bool do_authalic;
	unsigned int geometry;

	uint64_t i, j, k, node, vertex, node_stop, node_new, vertex_new, node_last, vertex_last, n_arcs = 0;
	uint64_t dim[4] = {1, 0, 0, 0};
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC, p_alloc = GMT_TINY_CHUNK;

	char segment_header[GMT_BUFSIZ];

	double area_sphere = 0.0, area_polygon, area_triangle, area_km2 = GMT->session.d_NaN, dist = GMT->session.d_NaN, y[3], V1[3], V2[3], V3[3];
	double *plat = NULL, *plon = NULL, R2;

	struct GMT_DATASEGMENT *S[2] = {NULL, NULL};
	struct STRPACK_ARC *arc = NULL;

	if (get_area == 2) /* Want areas in steradians */
		R2 = GMT->current.map.dist[GMT_MAP_DIST].scale = 1.0;
	else {	/* Want areas in units of m^2 or km^2 or similar */
		R2 = pow (R2D * GMT->current.proj.M_PR_DEG, 2.0);	/* squared mean radius in meters */
		R2 *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);	/* Get final measure unit for area */
	}
	do_authalic = (get_area && !get_arcs && !GMT_IS_ZERO (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (get_arcs) arc = GMT_memory (GMT, NULL, n_alloc, struct STRPACK_ARC);
	plon = GMT_memory (GMT, NULL, p_alloc, double);
	plat = GMT_memory (GMT, NULL, p_alloc, double);

	dim[GMT_SEG] = n;	/* Number of segments is known */
	dim[GMT_COL] = 2;	/* Each segment only has 2 columns */
	dim[GMT_ROW] = (get_arcs) ? 2 : 0;	/* Rows (unknown length if polygons; fixed 2 if arcs) */
	geometry = (get_arcs) ? GMT_IS_LINE : GMT_IS_POLY;
	if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, geometry, 0, dim, NULL, NULL, 0, 0, file0)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate\n");
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	if (nodes) {	/* Want Voronoi node and area information via Dout[1] */
		dim[GMT_SEG] = 1;	/* Only need one segment */
		dim[GMT_COL] = 2 + get_area;	/* Need 2 or 3 columns */
		dim[GMT_ROW] = n;	/* One row per node */
		if ((Dout[1] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, file1)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate nodes\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		S[1] = Dout[1]->table[0]->segment[0];	/* Shorthand for this segment */
	}

	for (node = 0; node < n; node++) {

		area_polygon = 0.0;

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

			if (get_arcs) {	/* Only collect the arcs - we'll sort out duplicates later */
				arc[n_arcs].begin = vertex_last;	arc[n_arcs].end = vertex_new;
				n_arcs++;
				if (n_arcs == n_alloc) {
					n_alloc <<= 1;
					arc = GMT_memory (GMT, arc, n_alloc, struct STRPACK_ARC);
				}
				vertex++;
			}
			else {	/* Need to assemble the polygon */
				plon[vertex] = V->lon[vertex_last];
				plat[vertex] = V->lat[vertex_last];
				if (get_area) {	/* Convert three corners to Cartesian */
					y[0] = (do_authalic) ? GMT_lat_swap (GMT, V->lat[node], GMT_LATSWAP_G2A) : V->lat[node];		/* Convert to authalic latitude */
					y[1] = (do_authalic) ? GMT_lat_swap (GMT, V->lat[vertex_last], GMT_LATSWAP_G2A) : V->lat[vertex_last];	/* Convert to authalic latitude */
					y[2] = (do_authalic) ? GMT_lat_swap (GMT, V->lat[vertex_new], GMT_LATSWAP_G2A) : V->lat[vertex_new];	/* Convert to authalic latitude */
					GMT_geo_to_cart (GMT, y[0], V->lon[node], V1, true);
					GMT_geo_to_cart (GMT, y[1], V->lon[vertex_last], V2, true);
					GMT_geo_to_cart (GMT, y[2], V->lon[vertex_new], V3, true);
					area_triangle = stripack_areas (V1, V2, V3);
					area_polygon += area_triangle;
				}
				vertex++;
				if (vertex == p_alloc) {	/* Need more space for polygon */
					p_alloc <<= 1;
					plon = GMT_memory (GMT, plon, p_alloc, double);
					plat = GMT_memory (GMT, plat, p_alloc, double);
				}
			}

			/* When we reach the vertex where we started, we are done with this polygon */
		} while (node_new != node_stop);

		if (!get_arcs) {	/* Finalize the polygon information */
			S[0] = Dout[0]->table[0]->segment[node];	/* Local shorthand to current output segment */
			if (get_area) {
				area_km2 = area_polygon * R2;	/* Get correct area units */
				sprintf (segment_header, "Pol: %" PRIu64 " %g %g Area: %g", node, lon[node], lat[node], area_km2);
			}
			else
				sprintf (segment_header, "Pol: %" PRIu64 " %g %g", node, lon[node], lat[node]);

			if (nodes) {	/* Also output node info via S[1] */
				S[1]->coord[GMT_X][node] = lon[node];
				S[1]->coord[GMT_Y][node] = lat[node];
				if (get_area) S[1]->coord[GMT_Z][node] = area_km2;
			}
			S[0]->header = strdup (segment_header);

			GMT_alloc_segment (GMT, S[0], vertex, 2, false);	/* Realloc this output polygon to actual size */
			GMT_memcpy (S[0]->coord[GMT_X], plon, vertex, double);
			GMT_memcpy (S[0]->coord[GMT_Y], plat, vertex, double);

			if (get_area) area_sphere += area_polygon;
		}
	}
	if (get_arcs) {	/* Process arcs */
		for (k = 0; k < n_arcs; ++k)
			if (arc[k].begin > arc[k].end)
			uint64_swap (arc[k].begin, arc[k].end);

		/* Sort and exclude duplicates */
		qsort (arc, n_arcs, sizeof (struct STRPACK_ARC), compare_arc);
		for (i = 1, j = 0; i < n_arcs; i++) {
			if (arc[i].begin != arc[j].begin || arc[i].end != arc[j].end) j++;
			arc[j] = arc[i];
		}
		n_arcs = j + 1;
		dim[GMT_SEG] = n_arcs;	/* Number of arc segments */
		dim[GMT_COL] = 2;	/* Only 2 columns */
		dim[GMT_ROW] = 2;	/* Each arc needs 2 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, file0)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for sphtriangulate Voronoi nodes\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Output %d unique Voronoi arcs\n", n_arcs);

		for (i = 0; i < n_arcs; i++) {
			S[0] = Dout[0]->table[0]->segment[i];	/* Shorthand for this output segment */
			S[0]->coord[GMT_X][0] = V->lon[arc[i].end];	S[1]->coord[GMT_Y][0] = V->lat[arc[i].end];
			S[0]->coord[GMT_X][1] = V->lon[arc[i].begin];	S[1]->coord[GMT_Y][1] = V->lat[arc[i].begin];
			if (get_area) {
				dist = GMT_distance (GMT, S[0]->coord[GMT_X][0], S[0]->coord[GMT_Y][0], S[0]->coord[GMT_X][1], S[0]->coord[GMT_Y][1]);
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " Length: %g", arc[i].begin, arc[i].end, dist);
			}
			else
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64, arc[i].begin, arc[i].end);
			S[0]->header = strdup (segment_header);
		}
		GMT_free (GMT, arc);
	}
	else {
		GMT_free (GMT, plon);
		GMT_free (GMT, plat);
		if (get_area) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Total surface area = %g\n", area_sphere * R2);
	}
	return (GMT_OK);
}

char *unit_name (char unit, int arc) {
	/* Get unit names for length or area */
	char *name;
	switch (unit) {
		case 'k':	/* km */
			name = (arc) ? "km" : "km^2";
			break;
		case 'M':	/* Miles */
			name = (arc) ? "miles" : "miles^2";
			break;
		case 'n':	/* Nautical miles */
			name = (arc) ? "nautical miles" : "nautical miles^2";
			break;
		case 'd':	/* Degrees */
			name = (arc) ? "degrees" : "steradians";
			break;
		case 'f':	/* Feet */
			name = (arc) ? "feet" : "feet^2";
			break;
		case 'u':	/* US Survey Feet */
			name = (arc) ? "sfeet" : "sfeet^2";
			break;
		default:
			name = (arc) ? "m" : "m^2";
			break;
	}
	return (name);
}

void *New_sphtriangulate_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHTRIANGULATE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct SPHTRIANGULATE_CTRL);
	C->L.unit = 'e';	/* Default is meter distances */

	return (C);
}

void Free_sphtriangulate_Ctrl (struct GMT_CTRL *GMT, struct SPHTRIANGULATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);
	if (C->N.file) free (C->N.file);
	GMT_free (GMT, C);
}

int GMT_sphtriangulate_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: sphtriangulate [<table>] [-A] [-C] [-D] [-L<unit>] [-N<nodetable>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Qd|v] [-T] [%s] [%s]\n\t[%s] [%s]\n", GMT_V_OPT, GMT_b_OPT, GMT_h_OPT, GMT_i_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\n", GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Compute and print triangle or polygon areas in header records (see -L for units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T is selected we print arc lengths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with the binary output option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Skip repeated input vertex at the end of a closed segment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set distance unit arc (d)egree, m(e)ter, (f)oot, (k)m, (M)ile, (n)autical mile, or s(u)rvey foot [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   PROJ_ELLIPSOID determines if geodesic or great-circle distances are used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Output filename for Delaunay or Voronoi polygon information [Store in output segment headers].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Delaunay: output is the node triplets and area (i, j, k, area).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Voronoi: output is the node coordinates and polygon area (lon, lat, area).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append d for Delaunay triangles or v for Voronoi polygons [Delaunay].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -bo is used then -N may be used to specify a separate file where the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   polygon information normally is written.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Write arcs [Default writes polygons].\n");
	GMT_Option (API, "V,bi2,bo,h,i,s,:,.");

	return (EXIT_FAILURE);
}

int GMT_sphtriangulate_parse (struct GMT_CTRL *GMT, struct SPHTRIANGULATE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sphtriangulate and sets parameters in CTRL.
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
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				break;
			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
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
				Ctrl->Q.mode = (opt->arg[0] == 'v') ? VORONOI : DELAUNAY;
				break;
			case 'T':
				Ctrl->T.active = true;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_OUT] && Ctrl->A.active && !Ctrl->N.active, "Syntax error: Binary output does not support storing areas unless -N is used\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->T.active, "Syntax error -N: Cannot be used with -T.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Syntax error -N: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_sphtriangulate_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sphtriangulate (void *V_API, int mode, void *args)
{
	char *tmode[2] = {"Delaunay", "Voronoi"}, header[GMT_BUFSIZ];
	int error = 0;
	bool first = false, steradians = false, do_authalic = false;

	uint64_t n = 0, n_dup = 0;
	size_t n_alloc;

	double first_x = 0.0, first_y = 0.0, X[3], *in = NULL;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL;

	struct SPHTRIANGULATE_CTRL *Ctrl = NULL;
	struct STRIPACK T;
	struct GMT_DATASET *Dout[2] = {NULL, NULL};
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_sphtriangulate_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_sphtriangulate_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_sphtriangulate_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_sphtriangulate_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sphtriangulate_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphtriangulate main code ----------------------------*/

	GMT_init_distaz (GMT, Ctrl->L.unit, GMT_sph_mode (GMT), GMT_MAP_DIST);
	do_authalic = (Ctrl->A.active && !Ctrl->T.active && !GMT_IS_ZERO (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (do_authalic) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Will convert to authalic latitudes for area calculations\n");
	}
	steradians = (Ctrl->L.unit == 'd' && !Ctrl->T.active);	/* Flag so we can do steradians */

	/* Now we are ready to take on some input values */

	if ((error = GMT_set_cols (GMT, GMT_IN, 2)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */ 
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

		/* Data record to process */

		if (first) {	/* Beginning of new segment; kep track of the very first coordinate in case of duplicates */
			first_x = in[GMT_X];	first_y = in[GMT_Y];
		}
		else if (Ctrl->D.active) {	/* Look for duplicate point at end of segments that replicate start point */
			if (in[GMT_X] == first_x && in[GMT_Y] == first_y) {	/* If any point after the first matches the first */
				n_dup++;
				continue;
			}
		}
		/* Convert lon,lat in degrees to Cartesian x,y,z triplets */
		GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);
		xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];
		if (!Ctrl->C.active) {	/* Keep copy of lon/lat */
			lon[n] = in[GMT_X];
			lat[n] = in[GMT_Y];
		}

		if (++n == n_alloc) {	/* Get more memory */
			if (!Ctrl->C.active) {size_t n_tmp = n_alloc; GMT_malloc2 (GMT, lon, lat, n, &n_tmp, double); }
			GMT_malloc3 (GMT, xx, yy, zz, n, &n_alloc, double);
		}
		first = false;
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	/* Reallocate memory to n points */
	n_alloc = n;
	if (!Ctrl->C.active) GMT_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
	GMT_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);
	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (Ctrl->D.active && n_dup) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %d duplicate points in segments\n", n_dup);
	GMT_Report (API, GMT_MSG_VERBOSE, "Do Voronoi construction using %d points\n", n);

	GMT_memset (&T, 1, struct STRIPACK);
	T.mode = Ctrl->Q.mode;
	stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
	if (Ctrl->C.active) {	/* Must recover lon,lat and set pointers */
		cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
		lon = xx;
		lat = yy;
	}
	GMT_free (GMT,  zz);	/* Done with zz for now */

	GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on table headers on output */
	if (Ctrl->Q.mode == VORONOI) {	/* Selected Voronoi polygons */
		stripack_voronoi_output (GMT, n, lon, lat, &T.V, Ctrl->T.active, Ctrl->A.active + steradians, Ctrl->N.active, Dout, NULL, Ctrl->N.file);
		GMT_free (GMT, T.V.lon);	GMT_free (GMT, T.V.lat);
		GMT_free (GMT, T.V.lend);	GMT_free (GMT, T.V.listc);
		GMT_free (GMT, T.V.lptr);
	}
	else {	/* Selected Delaunay triangles */
		stripack_delaunay_output (GMT, lon, lat, &T.D, Ctrl->T.active, Ctrl->A.active + steradians, Ctrl->N.active, Dout, NULL, Ctrl->N.file);
	}

	Dout[0]->table[0]->header = GMT_memory (GMT, NULL, 1, char *);	/* One header record only */
	sprintf (header, "# sphtriangulate %s output via STRPACK", tmode[Ctrl->Q.mode]);
	if (Ctrl->A.active) {
		strcat (header, (Ctrl->T.active) ? ".  Arc lengths in " : ".  Areas in ");
		strcat (header, unit_name (Ctrl->L.unit, Ctrl->T.active));
	}
	strcat (header, ".");
	Dout[0]->table[0]->n_headers = 1;
	Dout[0]->table[0]->header[0] = strdup (header);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output sources, unless already set */
		Return (API->error);
	}

	if (Ctrl->T.active) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Must produce multisegment output files */

	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, Dout[0]->io_mode, NULL, NULL, Dout[0]) != GMT_OK) {
		Return (API->error);
	}
	if (Ctrl->N.active) {
		GMT_set_segmentheader (GMT, GMT_OUT, false);	/* Since we only have one segment */
		if (Ctrl->A.active) sprintf (header, "# sphtriangulate nodes (lon, lat, area)"); else sprintf (header, "# sphtriangulate nodes (lon, lat)");
		Dout[1]->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
		Dout[1]->table[0]->n_headers = 1;
		Dout[1]->table[0]->header[0] = strdup (header);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, Dout[1]->io_mode, NULL, Ctrl->N.file, Dout[1]) != GMT_OK) {
			Return (API->error);
		}
	}

	GMT_free (GMT, T.D.tri);
	if (!Ctrl->C.active) {
		GMT_free (GMT, lon);
		GMT_free (GMT, lat);
	}
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);

	GMT_Report (API, GMT_MSG_VERBOSE, "Triangularization completed\n");

	Return (GMT_OK);
}
