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
/* Include file for sph supplement */

#define DELAUNAY	0
#define VORONOI		1
#define INTERPOLATE	2

#define TRI_NROW	6	/* Don't request arc indices from STRIPACK  */

/* PW note: We largely use int64_t since it is too hard to determine which variables
 * in the Fortran-translated code could make unsigned. */

struct STRIPACK_DELAUNAY {	/* Information about Delaunay triangulation */
	uint64_t n;	/* Number of Delaunay triangles */
	int64_t *tri;		/* Delaunay triplet node numbers and more */
};

struct STRIPACK_VORONOI {	/* Information about Voronoi polygons */
	double *lon, *lat;		/* Voronoi polygon vertices */
	uint64_t n;		/* Number of boundary nodes for Voronoi */
	int64_t *lend, *listc, *lptr;	/* Voronoi vertex lists and pointers */
	int64_t *list;			/* Additional list from trmesh */		
};

struct STRIPACK_INTERPOLATE {	/* Information about triangles */
	int64_t *lend, *list, *lptr;	/* lists and pointers */
};

struct STRIPACK {
	unsigned int mode;	/* VORONOI, DELAUNAY, or INTERPOLATE */
	struct STRIPACK_DELAUNAY D;
	struct STRIPACK_VORONOI V;
	struct STRIPACK_INTERPOLATE I;
};

struct STRPACK_ARC {
	uint64_t begin, end;
};

EXTERN_MSC int stripack_lists (struct GMT_CTRL *GMT, uint64_t n, double *x, double *y, double *z, struct STRIPACK *T);
EXTERN_MSC double stripack_areas (double *V1, double *V2, double *V3);
EXTERN_MSC void cart_to_geo (struct GMT_CTRL *GMT, uint64_t n, double *x, double *y, double *z, double *lon, double *lat);
EXTERN_MSC int compare_arc (const void *p1, const void *p2);
EXTERN_MSC int ssrfpack_grid (struct GMT_CTRL *GMT, double *x, double *y, double *z, double *w, uint64_t n, unsigned int mode, double *par, bool vartens, struct GMT_GRID_HEADER *h, double *f);
