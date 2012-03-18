/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2008-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
/* Include file for sph supplement */

#define DELAUNAY	0
#define VORONOI		1
#define INTERPOLATE	2

#define TRI_NROW	6	/* Don't request arc indices from STRIPACK  */

struct STRIPACK_DELAUNAY {	/* Information about Delaunay triangulation */
	GMT_LONG n;		/* Number of Delaunay triangles */
	GMT_LONG *tri;		/* Delaunay triplet node numbers and more */
};

struct STRIPACK_VORONOI {	/* Information about Voronoi polygons */
	double *lon, *lat;		/* Voronoi polygon vertices */
	GMT_LONG n;			/* Number of boundary nodes for Voronoi */
	GMT_LONG *lend, *listc, *lptr;	/* Voronoi vertex lists and pointers */
	GMT_LONG *list;			/* Additional list from trmesh */		
};

struct STRIPACK_INTERPOLATE {	/* Information about triangles */
	GMT_LONG *lend, *list, *lptr;	/* lists and pointers */
};

struct STRIPACK {
	GMT_LONG mode;	/* VORONOI, DELAUNAY, or INTERPOLATE */
	struct STRIPACK_DELAUNAY D;
	struct STRIPACK_VORONOI V;
	struct STRIPACK_INTERPOLATE I;
};

struct STRPACK_ARC {
	GMT_LONG begin, end;
};

EXTERN_MSC void stripack_lists (struct GMT_CTRL *GMT, GMT_LONG n, double *x, double *y, double *z, struct STRIPACK *T);
EXTERN_MSC double stripack_areas (double *V1, double *V2, double *V3);
EXTERN_MSC void cart_to_geo (struct GMT_CTRL *GMT, GMT_LONG n, double *x, double *y, double *z, double *lon, double *lat);
EXTERN_MSC int compare_arc (const void *p1, const void *p2);
EXTERN_MSC void ssrfpack_grid (struct GMT_CTRL *GMT, double *x, double *y, double *z, double *w, GMT_LONG n, GMT_LONG mode, double *par, GMT_LONG vartens, struct GRD_HEADER *h, double *f);
