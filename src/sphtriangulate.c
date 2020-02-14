/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2008-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
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
 * Version:	6 API
 *
 */

#include "gmt_dev.h"
#include "gmt_sph.h"

#define THIS_MODULE_CLASSIC_NAME	"sphtriangulate"
#define THIS_MODULE_MODERN_NAME	"sphtriangulate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Delaunay or Voronoi construction of spherical data"
#define THIS_MODULE_KEYS	"<D{,>D},ND)"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdehijqs"

struct SPHTRIANGULATE_CTRL {
	struct SPHTRI_Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct SPHTRI_A {	/* -A */
		bool active;
	} A;
	struct SPHTRI_C {	/* -C */
		bool active;
	} C;
	struct SPHTRI_D {	/* -D */
		bool active;
	} D;
	struct SPHTRI_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct SPHTRI_L {	/* -L<unit>] */
		bool active;
		char unit;
	} L;
	struct SPHTRI_N {	/* -N */
		bool active;
		char *file;
	} N;
	struct SPHTRI_Q {	/* -Q */
		bool active;
		unsigned int mode;	/* 0 is Delaunay, 1 is Voronoi */
	} Q;
	struct SPHTRI_T {	/* -T */
		bool active;
	} T;
};

/* Must be int due to qsort requirement */
GMT_LOCAL int sph_compare_arc (const void *p1, const void *p2) {
	const struct STRPACK_ARC *a = p1, *b = p2;
	if (a->begin < b->begin) return (-1);
	if (a->begin > b->begin) return (1);
	if (a->end < b->end) return (-1);
	if (a->end > b->end) return (1);
	return (0);
}

GMT_LOCAL int stripack_delaunay_output (struct GMT_CTRL *GMT, double *lon, double *lat, struct STRIPACK_DELAUNAY *D, uint64_t get_arcs, unsigned int get_area, uint64_t nodes, struct GMT_DATASET *Dout[]) {
	/* Prints out the Delaunay triangles either as polygons (for filling) or arcs (lines). */
	uint64_t i, ij;
	bool do_authalic;
	uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 0}, k;
	double area_sphere = 0.0, area_triangle = GMT->session.d_NaN, V[3][3], R2, y, dist;
	char segment_header[GMT_BUFSIZ];
	struct GMT_DATASEGMENT *S[2] = {NULL, NULL};
	if (get_area == 2) /* Return area in steradians */
		R2 = GMT->current.map.dist[GMT_MAP_DIST].scale = 1.0;
	else {	/* Want area in units like m^2 or km^2 */
		R2 = pow (R2D * GMT->current.proj.M_PR_DEG, 2.0);	/* squared mean radius in meters */
		R2 *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);	/* Get final measure unit for area */
	}
	do_authalic = (get_area && !get_arcs && !gmt_M_is_zero (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (!get_arcs) {	/* All output polygons consist of three points.  Not explicitly closed (use -L or -G in psxy) */
		dim[GMT_SEG] = D->n;	/* Number of segments */
		dim[GMT_COL] = 2;	/* Just 2 columns */
		dim[GMT_ROW] = 3;	/* All segments has 3 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate\n");
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		if (nodes) {	/* Want Voronoi node and area information via Dout[1] */
			dim[GMT_SEG] = 1;	/* Just one segment */
			dim[GMT_COL] = 3 + get_area;	/* Here we use 3-4 columns */
			dim[GMT_ROW] = D->n;	/* One row per node */
			if ((Dout[1] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate nodes\n");
				GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			}
			S[1] = Dout[1]->table[0]->segment[0];
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output %d unique triangle polygons\n", D->n);
		for (k = ij = 0; k < D->n; k++, ij += TRI_NROW) {	/* For each triangle */
			S[0] = Dout[0]->table[0]->segment[k];	/* Short hand for current triangle segment */
			/* Write segment header with triangle # and the three node numbers */
			if (get_area) {	/* Compute area */
				for (i = 0; i < 3; i++) {
					y = (do_authalic) ? gmt_lat_swap (GMT, lat[D->tri[ij+i]], GMT_LATSWAP_G2A) : lat[D->tri[ij+i]];	/* Convert to authalic latitude */
					gmt_geo_to_cart (GMT, y, lon[D->tri[ij+i]], V[i], true);
				}
				area_triangle = gmt_stripack_areas (V[0], V[1], V[2]);
				area_sphere += area_triangle;
				sprintf (segment_header, "Triangle: %" PRIu64 " %" PRIu64 "-%" PRIu64 "-%" PRIu64 " Area: %g -Z%" PRIu64,
				         k, D->tri[ij], D->tri[ij+1], D->tri[ij+2], area_triangle * R2, k);
			}
			else	/* Just a plain header with triangle number */
				sprintf (segment_header, "Triangle: %" PRIu64 " -Z%" PRIu64, k, k);
			if (nodes) {	/* Output Voronoi node and area information via S[1] */
				S[1]->data[GMT_X][k] = (double)D->tri[ij];
				S[1]->data[GMT_Y][k] = (double)D->tri[ij+1];
				S[1]->data[GMT_Z][k] = (double)D->tri[ij+2];
				if (get_area) S[1]->data[3][k] = area_triangle * R2;
			}
			S[0]->header = strdup (segment_header);
			for (i = 0; i < 3; i++) {	/* Copy the three vertices */
				S[0]->data[GMT_X][i] = lon[D->tri[ij+i]];
				S[0]->data[GMT_Y][i] = lat[D->tri[ij+i]];
			}
			S[0]->n_rows = 3;
			Dout[0]->table[0]->n_records += S[0]->n_rows;
		}
		Dout[0]->n_records = Dout[0]->table[0]->n_records;
		if (get_area) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Total surface area = %g\n", area_sphere * R2);
	}
	else {	/* Want just the arcs (to draw then, probably).  This avoids repeating shared arcs between triangles */
		uint64_t j, ij1, ij2, ij3, n_arcs, kk;
		struct STRPACK_ARC *arc = NULL;

		n_arcs = 3 * D->n;
		arc = gmt_M_memory (GMT, NULL, n_arcs, struct STRPACK_ARC);
		for (k = ij = ij1 = 0, ij2 = 1, ij3 = 2; k < D->n; k++, ij1 += TRI_NROW, ij2 += TRI_NROW, ij3 += TRI_NROW) {	/* For each triangle */
			arc[ij].begin = D->tri[ij1];	arc[ij].end = D->tri[ij2];	ij++;
			arc[ij].begin = D->tri[ij2];	arc[ij].end = D->tri[ij3];	ij++;
			arc[ij].begin = D->tri[ij1];	arc[ij].end = D->tri[ij3];	ij++;
		}
		for (kk = 0; kk < n_arcs; ++kk)
			if (arc[kk].begin > arc[kk].end)
				gmt_M_uint64_swap (arc[kk].begin, arc[kk].end);

		/* Sort and eliminate duplicate arcs */
		qsort (arc, n_arcs, sizeof (struct STRPACK_ARC), sph_compare_arc);
		for (i = 1, j = 0; i < n_arcs; i++) {
			if (arc[i].begin != arc[j].begin || arc[i].end != arc[j].end) j++;
			arc[j] = arc[i];
		}
		n_arcs = j + 1;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output %" PRIu64 " unique triangle arcs\n", n_arcs);

		dim[GMT_SEG] = n_arcs;	/* Number of output arcs = segments */
		dim[GMT_COL] = 2;		/* Only use 2 columns */
		dim[GMT_ROW] = 2;		/* Each arc has 2 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate arcs\n");
			gmt_M_free (GMT, arc);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		for (i = 0; i < n_arcs; i++) {
			S[0] = Dout[0]->table[0]->segment[i];	/* Shorthand for this output segment */
			S[0]->data[GMT_X][0] = lon[arc[i].begin];	S[0]->data[GMT_Y][0] = lat[arc[i].begin];
			S[0]->data[GMT_X][1] = lon[arc[i].end];	S[0]->data[GMT_Y][1] = lat[arc[i].end];
			if (get_area) {	/* Compute arc lengths */
				dist = gmt_distance (GMT, S[0]->data[GMT_X][0], S[0]->data[GMT_Y][0], S[0]->data[GMT_X][1], S[0]->data[GMT_Y][1]);
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " Length: %g -Z%" PRIu64, arc[i].begin, arc[i].end, dist, i);
			}
			else	/* Plain header */
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " -Z%" PRIu64, arc[i].begin, arc[i].end, i);
			S[0]->header = strdup (segment_header);
			S[0]->n_rows = 2;
		}
		Dout[0]->table[0]->n_records = Dout[0]->n_records = 2 * n_arcs;
		gmt_M_free (GMT, arc);
	}
	return (GMT_NOERROR);
}

GMT_LOCAL int stripack_voronoi_output (struct GMT_CTRL *GMT, uint64_t n, double *lon, double *lat, struct STRIPACK_VORONOI *V, bool get_arcs, unsigned int get_area, uint64_t nodes, struct GMT_DATASET *Dout[]) {
	/* Prints out the Voronoi polygons either as polygons (for filling) or arcs (lines) */
	bool do_authalic;
	unsigned int geometry;

	uint64_t i, j, k, node, vertex, node_stop, node_new, vertex_new, node_last, vertex_last, n_arcs = 0;
	uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 0};
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC, p_alloc = GMT_TINY_CHUNK;

	char segment_header[GMT_BUFSIZ];

	double area_sphere = 0.0, area_polygon, area_triangle, area_km2 = GMT->session.d_NaN, dist;
	double y[3], V1[3], V2[3], V3[3];
	double *plat = NULL, *plon = NULL, R2;

	struct GMT_DATASEGMENT *S[2] = {NULL, NULL};
	struct STRPACK_ARC *arc = NULL;

	if (get_area == 2) /* Want areas in steradians */
		R2 = GMT->current.map.dist[GMT_MAP_DIST].scale = 1.0;
	else {	/* Want areas in units of m^2 or km^2 or similar */
		R2 = pow (R2D * GMT->current.proj.M_PR_DEG, 2.0);	/* squared mean radius in meters */
		R2 *= (GMT->current.map.dist[GMT_MAP_DIST].scale * GMT->current.map.dist[GMT_MAP_DIST].scale);	/* Get final measure unit for area */
	}
	do_authalic = (get_area && !get_arcs && !gmt_M_is_zero (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (get_arcs) arc = gmt_M_memory (GMT, NULL, n_alloc, struct STRPACK_ARC);
	plon = gmt_M_memory (GMT, NULL, p_alloc, double);
	plat = gmt_M_memory (GMT, NULL, p_alloc, double);

	dim[GMT_SEG] = n;	/* Number of segments is known */
	dim[GMT_COL] = 2;	/* Each segment only has 2 columns */
	dim[GMT_ROW] = (get_arcs) ? 2 : 0;	/* Rows (unknown length if polygons; fixed 2 if arcs) */
	geometry = (get_arcs) ? GMT_IS_LINE : GMT_IS_POLY;
	if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, geometry, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate\n");
		gmt_M_free (GMT, plon);		gmt_M_free (GMT, plat);
		gmt_M_free (GMT, arc);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}
	if (nodes) {	/* Want Voronoi node and area information via Dout[1] */
		dim[GMT_SEG] = 1;	/* Only need one segment */
		dim[GMT_COL] = 2 + get_area;	/* Need 2 or 3 columns */
		dim[GMT_ROW] = n;	/* One row per node */
		if ((Dout[1] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate nodes\n");
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
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
					arc = gmt_M_memory (GMT, arc, n_alloc, struct STRPACK_ARC);
				}
				vertex++;
			}
			else {	/* Need to assemble the polygon */
				plon[vertex] = V->lon[vertex_last];
				plat[vertex] = V->lat[vertex_last];
				if (get_area) {	/* Convert three corners to Cartesian */
					y[0] = (do_authalic) ? gmt_lat_swap (GMT, V->lat[node], GMT_LATSWAP_G2A) : V->lat[node];		/* Convert to authalic latitude */
					y[1] = (do_authalic) ? gmt_lat_swap (GMT, V->lat[vertex_last], GMT_LATSWAP_G2A) : V->lat[vertex_last];	/* Convert to authalic latitude */
					y[2] = (do_authalic) ? gmt_lat_swap (GMT, V->lat[vertex_new], GMT_LATSWAP_G2A) : V->lat[vertex_new];	/* Convert to authalic latitude */
					gmt_geo_to_cart (GMT, y[0], V->lon[node], V1, true);
					gmt_geo_to_cart (GMT, y[1], V->lon[vertex_last], V2, true);
					gmt_geo_to_cart (GMT, y[2], V->lon[vertex_new], V3, true);
					area_triangle = gmt_stripack_areas (V1, V2, V3);
					area_polygon += area_triangle;
				}
				vertex++;
				if (vertex == p_alloc) {	/* Need more space for polygon */
					p_alloc <<= 1;
					plon = gmt_M_memory (GMT, plon, p_alloc, double);
					plat = gmt_M_memory (GMT, plat, p_alloc, double);
				}
			}

			/* When we reach the vertex where we started, we are done with this polygon */
		} while (node_new != node_stop);

		if (!get_arcs) {	/* Finalize the polygon information */
			/* Explicitly close the polygon */
			plon[vertex] = plon[0];
			plat[vertex] = plat[0];
			vertex++;
			S[0] = Dout[0]->table[0]->segment[node];	/* Local shorthand to current output segment */
			if (get_area) {
				area_km2 = area_polygon * R2;	/* Get correct area units */
				sprintf (segment_header, "Pol: %" PRIu64 " %g %g Area: %g -Z%" PRIu64, node, lon[node], lat[node], area_km2, node);
			}
			else
				sprintf (segment_header, "Pol: %" PRIu64 " %g %g -Z%" PRIu64, node, lon[node], lat[node], node);

			if (nodes) {	/* Also output node info via S[1] */
				S[1]->data[GMT_X][node] = lon[node];
				S[1]->data[GMT_Y][node] = lat[node];
				if (get_area) S[1]->data[GMT_Z][node] = area_km2;
			}
			/* Realloc this output polygon to actual size and set header */
			S[0] = Dout[0]->table[0]->segment[node] = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, vertex, 2U, segment_header, Dout[0]->table[0]->segment[node]);
			gmt_M_memcpy (S[0]->data[GMT_X], plon, vertex, double);
			gmt_M_memcpy (S[0]->data[GMT_Y], plat, vertex, double);
			S[0]->n_rows =  vertex;
			Dout[0]->table[0]->n_records += vertex;
			Dout[0]->n_records += vertex;
			if (get_area) area_sphere += area_polygon;
		}
	}
	if (get_arcs) {	/* Process arcs */
		for (k = 0; k < n_arcs; ++k)
			if (arc[k].begin > arc[k].end)
			gmt_M_uint64_swap (arc[k].begin, arc[k].end);

		/* Sort and exclude duplicates */
		qsort (arc, n_arcs, sizeof (struct STRPACK_ARC), sph_compare_arc);
		for (i = 1, j = 0; i < n_arcs; i++) {
			if (arc[i].begin != arc[j].begin || arc[i].end != arc[j].end) j++;
			arc[j] = arc[i];
		}
		n_arcs = j + 1;
		dim[GMT_SEG] = n_arcs;	/* Number of arc segments */
		dim[GMT_COL] = 2;	/* Only 2 columns */
		dim[GMT_ROW] = 2;	/* Each arc needs 2 rows */
		if ((Dout[0] = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create a data set for sphtriangulate Voronoi nodes\n");
			gmt_M_free (GMT, arc);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output %d unique Voronoi arcs\n", n_arcs);

		for (i = 0; i < n_arcs; i++) {
			S[0] = Dout[0]->table[0]->segment[i];	/* Shorthand for this output segment */
			S[0]->data[GMT_X][0] = V->lon[arc[i].end];	S[0]->data[GMT_Y][0] = V->lat[arc[i].end];
			S[0]->data[GMT_X][1] = V->lon[arc[i].begin];	S[0]->data[GMT_Y][1] = V->lat[arc[i].begin];
			if (get_area) {
				dist = gmt_distance (GMT, S[0]->data[GMT_X][0], S[0]->data[GMT_Y][0], S[0]->data[GMT_X][1], S[0]->data[GMT_Y][1]);
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " Length: %g -Z%" PRIu64, arc[i].begin, arc[i].end, dist, i);
			}
			else
				sprintf (segment_header, "Arc: %" PRIu64 "-%" PRIu64 " -Z%" PRIu64, arc[i].begin, arc[i].end, i);
			S[0]->header = strdup (segment_header);
			S[0]->n_rows = 2;
		}
		gmt_M_free (GMT, arc);
		Dout[0]->table[0]->n_records = Dout[0]->n_records = 2 * n_arcs;
	}
	else {
		if (get_area) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Total surface area = %g\n", area_sphere * R2);
	}
	gmt_M_free (GMT, plon);
	gmt_M_free (GMT, plat);
	return (GMT_NOERROR);
}

GMT_LOCAL char *unit_name (char unit, int arc) {
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

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHTRIANGULATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SPHTRIANGULATE_CTRL);
	C->L.unit = 'e';	/* Default is meter distances */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SPHTRIANGULATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "\t==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A] [-C] [-D] [-L<unit>] [-N<table>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Qd|v] [-T] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n", GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\n", GMT_j_OPT, GMT_qi_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Compute and print triangle or polygon areas in header records (see -L for units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T is selected we print arc lengths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with the binary output option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Skip repeated input vertex at the end of a closed segment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set distance unit arc (d)egree, m(e)ter, (f)oot, (k)m, (M)ile, (n)autical mile, or s(u)rvey foot [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Output filename for Delaunay or Voronoi polygon information [Store in output segment headers].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Delaunay: output is the node triplets and area (i, j, k, area).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Voronoi: output is the node coordinates and polygon area (lon, lat, area).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append d for Delaunay triangles or v for Voronoi polygons [Delaunay].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -bo is used then -N may be used to specify a separate file where the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   polygon information normally is written.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Write arcs [Default writes polygons].\n");
	GMT_Option (API, "V,bi2,bo,di,e,h,i,j,qi,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SPHTRIANGULATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to sphtriangulate and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
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
					GMT_Report (API, GMT_MSG_ERROR, "Expected -L%s\n", GMT_LEN_UNITS_DISPLAY);
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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3,
	                                 "Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_OUT] && Ctrl->A.active && !Ctrl->N.active,
	                                 "Binary output does not support storing areas unless -N is used\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->T.active, "Option -N: Cannot be used with -T.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Option -N: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sphtriangulate (void *V_API, int mode, void *args) {
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
	struct GMT_RECORD *In = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	gmt_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the sphtriangulate main code ----------------------------*/

	gmt_init_distaz (GMT, Ctrl->L.unit, gmt_M_sph_mode (GMT), GMT_MAP_DIST);
	do_authalic = (Ctrl->A.active && !Ctrl->T.active && !gmt_M_is_zero (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening));
	if (do_authalic) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Will convert to authalic latitudes for area calculations\n");
	}
	steradians = (Ctrl->L.unit == 'd' && !Ctrl->T.active);	/* Flag so we can do steradians */

	/* Now we are ready to take on some input values */

	if ((error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */
	n_alloc = 0;
	if (!Ctrl->C.active) gmt_M_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
	n_alloc = 0;
	gmt_M_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);

	n = 0;

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) { 		/* Bail if there are any read errors */
				gmt_M_free (GMT, lon);	gmt_M_free (GMT, lat);
				gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
				gmt_M_free (GMT,  zz);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_segment_header (GMT))			/* Parse segment headers */
				first = true;
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

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
		gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);
		xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];
		if (!Ctrl->C.active) {	/* Keep copy of lon/lat */
			lon[n] = in[GMT_X];
			lat[n] = in[GMT_Y];
		}

		if (++n == n_alloc) {	/* Get more memory */
			if (!Ctrl->C.active) {size_t n_tmp = n_alloc; gmt_M_malloc2 (GMT, lon, lat, n, &n_tmp, double); }
			gmt_M_malloc3 (GMT, xx, yy, zz, n, &n_alloc, double);
		}
		first = false;
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	/* Reallocate memory to n points */
	n_alloc = n;
	if (!Ctrl->C.active) gmt_M_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
	gmt_M_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);
	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (Ctrl->D.active && n_dup) GMT_Report (API, GMT_MSG_INFORMATION, "Skipped %d duplicate points in segments\n", n_dup);
	GMT_Report (API, GMT_MSG_INFORMATION, "Do Voronoi construction using %d points\n", n);

	gmt_M_memset (&T, 1, struct STRIPACK);
	T.mode = Ctrl->Q.mode;
	gmt_stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
	if (Ctrl->C.active) {	/* Must recover lon,lat and set pointers */
		gmt_n_cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
		lon = xx;
		lat = yy;
	}
	gmt_M_free (GMT,  zz);	/* Done with zz for now */

	GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on table headers on output */
	if (Ctrl->Q.mode == VORONOI) {	/* Selected Voronoi polygons */
		stripack_voronoi_output (GMT, n, lon, lat, &T.V, Ctrl->T.active, Ctrl->A.active + steradians, Ctrl->N.active, Dout);
		gmt_M_free (GMT, T.V.lon);	gmt_M_free (GMT, T.V.lat);
		gmt_M_free (GMT, T.V.lend);	gmt_M_free (GMT, T.V.listc);
		gmt_M_free (GMT, T.V.lptr);
	}
	else {	/* Selected Delaunay triangles */
		stripack_delaunay_output (GMT, lon, lat, &T.D, Ctrl->T.active, Ctrl->A.active + steradians, Ctrl->N.active, Dout);
	}

	Dout[0]->table[0]->header = gmt_M_memory (GMT, NULL, 1, char *);	/* One header record only */
	sprintf (header, "sphtriangulate %s output via STRPACK", tmode[Ctrl->Q.mode]);
	if (Ctrl->A.active) {
		strcat (header, (Ctrl->T.active) ? ".  Arc lengths in " : ".  Areas in ");
		strncat (header, unit_name (Ctrl->L.unit, Ctrl->T.active), GMT_BUFSIZ-1);
	}
	strcat (header, ".");
	Dout[0]->table[0]->n_headers = 1;
	Dout[0]->table[0]->header[0] = strdup (header);

	if (Ctrl->T.active) gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Must produce multisegment output files */

	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, 0, NULL, Ctrl->Out.file, Dout[0]) != GMT_NOERROR) {
		gmt_M_free (GMT, lon);	gmt_M_free (GMT, lat);
		gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
		Return (API->error);
	}
	if (Ctrl->N.active) {
		gmt_set_segmentheader (GMT, GMT_OUT, false);	/* Since we only have one segment */
		if (Ctrl->A.active) sprintf (header, "sphtriangulate nodes (lon, lat, area)");
		else sprintf (header, "sphtriangulate nodes (lon, lat)");
		Dout[1]->table[0]->header = gmt_M_memory (GMT, NULL, 1, char *);
		Dout[1]->table[0]->n_headers = 1;
		Dout[1]->table[0]->header[0] = strdup (header);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, 0, NULL, Ctrl->N.file, Dout[1]) != GMT_NOERROR) {
			gmt_M_free (GMT, lon);	gmt_M_free (GMT, lat);
			gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
			Return (API->error);
		}
	}

	gmt_M_free (GMT, T.D.tri);
	if (!Ctrl->C.active) {
		gmt_M_free (GMT, lon);	gmt_M_free (GMT, lat);
	}
	gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);

	GMT_Report (API, GMT_MSG_INFORMATION, "Triangularization completed\n");

	Return (GMT_NOERROR);
}
