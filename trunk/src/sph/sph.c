/*--------------------------------------------------------------------
 *	$Id: sph.c,v 1.5 2009-01-09 04:02:48 myself Exp $
 *
 *	Copyright (c) 2008-2009 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
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
 * Spherical triangulation - Delaunay or Voronoi options.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *  and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *  Software, 23 (3), 416-434.
 * We translate to C using f2c and link with -lf2c
 *
 * Author:      Paul Wessel
 * Date:        13-FEB-2008
 *
 */
 
#include "gmt.h"
#include "sph.h"


/* STRIPACK FORTRAN SUBROUTINES PROTOTYPES */
extern int trmesh_ (int *, double *, double *, double *, int *, int *, int *, int *, int *, int *, double *, int *);
extern int trlist_ (int *, int *, int *, int *, int *, int *, int *, int *);
extern int crlist_ (int *, int *, double *, double *, double *, int *, int *, int *, int *, int *, int *, int *, double *, double *, double *, double *, int *);
extern double areas_ (double *, double *, double *);

void stripack_lists (GMT_LONG n, double *x, double *y, double *z, struct STRIPACK *T)
{
 	/* n, the number of points.
	 * x, y, z, the arrays with coordinates of points 
	 *
	 * xc, yc, zc: the coordinates of the Voronoi polygon vertices.
	 * lend, points to the "first" vertex in the Voronoi polygon around a particular node.
	 * lptr: given a vertex, returns the next vertex in the Voronoi polygon.
	 *
	 * NOTE: All indeces returned are C (0->) adjusted from FORTRAN (1->).
	 */

	int k, nrow = TRI_NROW, lnew, ierror, n4;
	int *iwk, *list, *lptr, *lend;
	size_t n_alloc;
	double *ds;
	
	ds = (double *)GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program);
	lend = (int *)GMT_memory (VNULL, (size_t)n, sizeof (int), GMT_program);
 	iwk = (int *)GMT_memory (VNULL, (size_t)(2*n), sizeof (int), GMT_program);
	n_alloc = 2 * (n - 2);
	T->D.tri = (int *)GMT_memory (VNULL, (size_t)(TRI_NROW*n_alloc), sizeof (int), GMT_program);
	n_alloc = 6 * (n - 2);
	lptr = (int *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (int), GMT_program);
	list = (int *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (int), GMT_program);

	/* Create the triangulation. Main output is (list, lptr, lend) */

	n4 = (int)n;
	if (gmtdefs.verbose) fprintf (stderr, "%s: Call STRIPACK TRMESH subroutine...", GMT_program);
	trmesh_ (&n4, x, y, z, list, lptr, lend, &lnew, iwk, &iwk[n], ds, &ierror);
	GMT_free ((void *)ds);
	GMT_free ((void *)iwk);
	if (gmtdefs.verbose) fprintf (stderr, "OK\n");

	if ( ierror == -2 ) {
		fprintf (stderr, "STRIPACK: Error in TRMESH. The first 3 nodes are collinear.\n");
		GMT_exit (EXIT_FAILURE);
	}

	if (ierror > 0) {
		fprintf (stderr, "STRIPACK: Error in TRMESH.  Duplicate nodes encountered.\n");
		GMT_exit (EXIT_FAILURE);
	}

	/* Create a triangle list which returns the number of triangles and their node list tri */

	if (gmtdefs.verbose) fprintf (stderr, "%s: Call STRIPACK TRLIST subroutine...", GMT_program);
	trlist_ (&n4, list, lptr, lend, &nrow, &T->D.n, T->D.tri, &ierror);
	if (gmtdefs.verbose) fprintf (stderr, "OK\n");

	if (ierror) {
		fprintf (stderr, "STRIPACK: Error in TRLIST.\n");
		GMT_exit (EXIT_FAILURE);
	}
	
	if (T->mode == VORONOI) {	/* Construct the Voronoi diagram */
		int *lbtri;
		double *rc;
		double *xc, *yc, *zc;	/* Voronoi polygon vertices */
	
		/* Note that the triangulation data structure is altered if NB > 0 */

		n_alloc = 2 * (n - 2);
		xc = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		yc = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		zc = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		rc = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		n_alloc = 6 * (n - 2);
		T->V.listc = (int *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (int), GMT_program);
		lbtri = (int *)GMT_memory (VNULL, (size_t)(6*n), sizeof (int), GMT_program);

		if (gmtdefs.verbose) fprintf (stderr, "%s: Call STRIPACK CRLIST subroutine...", GMT_program);
		crlist_ (&n4, &n4, x, y, z, list, lend, lptr, &lnew, lbtri, T->V.listc, &T->V.n, xc, yc, zc, rc, &ierror);
		if (gmtdefs.verbose) fprintf (stderr, "OK\n");
		GMT_free ((void *)lbtri);
		GMT_free ((void *)rc);
		T->V.lend = lend;	/* Save these for output */
		T->V.lptr = lptr;
		/* Convert polygon vertices vectors to lon, lat */
		n_alloc = 2 * (n - 2);
		T->V.lon = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		T->V.lat = (double *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
		cart_to_geo (n_alloc, xc, yc, zc, T->V.lon, T->V.lat);
		GMT_free ((void *)xc);
		GMT_free ((void *)yc);
		GMT_free ((void *)zc);

		if ( 0 < ierror ) {
			fprintf (stderr, "STRIPACK: Error in CRLIST.  IERROR = %d.\n", ierror);
			GMT_exit (EXIT_FAILURE);
		}
		
		/* Adjust Fortran to C indeces */
		n_alloc = 6 * (n - 2);
		for (k = 0; k < n_alloc; k++) T->V.listc[k]--;
		for (k = 0; k < n_alloc; k++) T->V.lptr[k]--;
		for (k = 0; k < n; k++) T->V.lend[k]--;
	}
	else {	/* Free things not needed */
		GMT_free ((void *)lend);
		GMT_free ((void *)lptr);
	}
	
	/* Adjust Fortran to C indeces */
	for (k = 0; k < TRI_NROW*T->D.n; k++) T->D.tri[k]--;
	
	GMT_free ((void *)list);
}

double stripack_areas (double *V1, double *V2, double *V3)
{	/* Wrapper for STRIPACK areas_ */
	return (areas_ (V1, V2, V3));
}

void cart_to_geo (GMT_LONG n, double *x, double *y, double *z, double *lon, double *lat)
{	/* Convert Cartesian vectors back to lon, lat vectors */
	int k;
	double V[3];
	for (k = 0; k < n; k++) {
		V[0] = x[k];
		V[1] = y[k];
		V[2] = z[k];
		GMT_cart_to_geo (&lat[k], &lon[k], V, TRUE);
	}
}

void geo_to_cart (double alat, double alon, double *a, int rads)
{	/* Unlike GMT main version we leave lon,lat untouched */
	/* Convert geographic latitude and longitude (alat, alon)
	   to a 3-vector of unit length (a).  rads = TRUE if we
	   need to convert alat, alon from degrees to radians  */

	double clat, clon, slon;

	if (rads) {
		alat *= D2R;
		alon *= D2R;
	}
	sincos (alat, &a[2], &clat);
	sincos (alon, &slon, &clon);
	a[0] = clat * clon;
	a[1] = clat * slon;
}

int compare_arc (const void *p1, const void *p2)
{
	struct STRPACK_ARC *a, *b;

	a = (struct STRPACK_ARC *)p1;
	b = (struct STRPACK_ARC *)p2;
	if (a->begin < b->begin)
		return (-1);
	else if (a->begin > b->begin)
		return (1);
	else {
		if (a->end < b->end)
			return (-1);
		else if (a->end > b->end)
			return (1);
		else
			return (0);
	}
}
