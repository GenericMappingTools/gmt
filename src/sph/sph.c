/*--------------------------------------------------------------------
 *	$Id: sph.c,v 1.6 2009-03-13 01:48:27 myself Exp $
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
 * Spherical interpolation - tension or smoothing.
 * Relies on SSRFPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of
 *  Scattered Data on the Surface of a Sphere with a Surface under Tension,
 *  AMC Trans. Math. Software, 23 (3), 435-442.
 * We translate both to C using f2c and link with -lf2c
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
/* SSRFPACK FORTRAN SUBROUTINES PROTOTYPES */
extern int getsig_ (int *, double *, double *, double *, double *, int *, int *, int *, double *, double *, double *, double *, int *);
extern int gradg_ (int *, double *, double *, double *, double *, int *, int *, int *, int *, double *, int *, double *, double *, int *);
extern int gradl_ (int *, int *, double *, double *, double *, double *, int *, int *, int *, double *, int *);
extern int intrc0_ (int *, double *, double *, double *, double *, double *, double *, int *, int *, int *, int *, double *, int *);
extern int intrc1_ (int *, double *, double *, double *, double *, double *, double *, int *, int *, int *, int *, double *, int *, double *, int *, double *, int *);
extern int sgprnt_ (int *, int *, int *, int *, int *, double *);
extern int smsurf_ (int *, double *, double *, double *, double *, int *, int *, int *, int *, double *, double *, double *, double *, double *, int *, double *, double *, int *);
extern int unif_ (int *, double *, double *, double *, double *, int *, int *, int *, int *, double *, int *, int *, int *, double *, double *, int *, double *, double *, int *);

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

/* Functions for spherical surface interpolation */

void ssrfpack_grid (double *x, double *y, double *z, double *w, int n, int mode, double *par, BOOLEAN vartens, struct GRD_HEADER *h, double *f)
{
	int lnew, ierror, n4, nm, k, k1, i, j, n_sig, nxp, ist, ij, iflgs, itgs, iter, nitg;
	int *iwk, *list, *lptr, *lend;
	size_t n_alloc;
	double *ds, *sigma, *grad;
	double *plon, *plat;
	double dgmax = 0.01, tol = 0.01, dsm, dgmx;
	int maxit = 10, plus = 1, minus = -1, lsig = 4;

	ds = (double *)GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program);
	lend = (int *)GMT_memory (VNULL, (size_t)n, sizeof (int), GMT_program);
	iwk = (int *)GMT_memory (VNULL, (size_t)(2*n), sizeof (int), GMT_program);
	n_alloc = 6 * (n - 2);
	lptr = (int *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (int), GMT_program);
	list = (int *)GMT_memory (VNULL, (size_t)n_alloc, sizeof (int), GMT_program);
	n_sig = (vartens) ? n_alloc : 1;

	/* Create the triangulation. Main output is (list, lptr, lend) */

	n4 = (int)n;
	if (gmtdefs.verbose) fprintf (stderr, "%s: Call STRIPACK TRMESH subroutine...", GMT_program);
	trmesh_ (&n4, x, y, z, list, lptr, lend, &lnew, iwk, &iwk[n], ds, &ierror);
	if (gmtdefs.verbose) fprintf (stderr, "done\n");
	GMT_free ((void *)ds);
	GMT_free ((void *)iwk);
	
	/* Set out output nodes */
	
	plon = (double *) GMT_memory (VNULL, h->nx, sizeof (double), GMT_program);
	plat = (double *) GMT_memory (VNULL, h->ny, sizeof (double), GMT_program);
	for (i = 0; i < h->nx; i++) plon[i] = GMT_i_to_x (i, h->x_min, h->x_max, h->x_inc, h->xy_off, h->nx);
	for (j = 0; j < h->ny; j++) plat[j] = GMT_j_to_y (j, h->y_min, h->y_max, h->y_inc, h->xy_off, h->ny);
	nm = h->nx * h->ny;
	
	/* Time to work on the interpolation */

	sigma = (double *)GMT_memory (VNULL, (size_t)n_sig, sizeof (double), GMT_program);
	if (mode > 0) grad = (double *)GMT_memory (VNULL, (size_t)(3*n), sizeof (double), GMT_program);
	
	if (mode == 0) {	 /* C-0 interpolation (INTRC0). */
		nxp = 0;
		ist = 1;
		for (j = 0; j < h->nx; j++) {
			for (i = 0; i < h->ny; i++) {
				ij = j * h->nx + i;
				intrc0_ (&n4, &plat[i], &plon[j], x, y, z, w, list, lptr, lend, &ist, &f[ij], &ierror);
				if (ierror > 0) nxp++;
	            		if (ierror < 0) {
					fprintf (stderr, "%s: Error in INTRC0:  I = %d, J = %d, IER = %d\n", GMT_program, i, j, ierror);
					GMT_exit (EXIT_FAILURE);
	            		}
			}
		}
		if (gmtdefs.verbose) fprintf (stderr, "%s: INTRC0:  Number of evaluations = %d, number of extrapolations = %d\n", GMT_program, nm, nxp);
	}
	else if (mode == 1) {	/* C-1 interpolation (INTRC1) with local gradients GRADL. */
	   	/* Accumulate the sum of the numbers of nodes used in the least squares fits in sum. */
		double sum = 0.0;
		for (k = 0; k < n; k++) {
			k1 = k + 1;
			gradl_ (&n4, &k1, x, y, z, w, list, lptr, lend, &grad[k], &ierror);
			if (ierror < 0) {
				fprintf (stderr, "%s: Error in GRADL:  K = %d IER = %d\n", GMT_program, k1, ierror);
				GMT_exit (EXIT_FAILURE);
            		}
			sum += (double)ierror;
		}
		sum /= n;
		if (gmtdefs.verbose) fprintf (stderr, "%s: GRADL:  Average number of nodes used in the least squares fits = %g\n", GMT_program, sum);
	        if (vartens) {	/* compute tension factors sigma (getsig). */
			getsig_ (&n4, x, y, z, w, list, lptr, lend, grad, &tol, sigma, &dsm, &ierror);
			if (ierror < 0) {
				fprintf (stderr, "%s: Error in GETSIG:  ier = %d\n", GMT_program, ierror);
				GMT_exit (EXIT_FAILURE);
			}
			if (gmtdefs.verbose) {
				fprintf (stderr, "%s: GETSIG:  %d tension factors altered;  Max change = %g\n", GMT_program, ierror, dsm);
			}
			/* write the tension factors to disk (sgprnt). */
			sgprnt_ (&n4, &lsig, list, lptr, lend, sigma);
	        }
	
		/* compute interpolated values on the uniform grid (unif). */

		iflgs = 0;
		if (vartens) iflgs = 1;
		unif_ (&n4, x, y, z, w, list, lptr, lend, &iflgs, sigma, &h->ny, &h->ny, &h->nx, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			fprintf (stderr, "%s: Error in UNIF:  ier = %d\n", GMT_program, ierror);
			GMT_exit (EXIT_FAILURE);
		}
		if (gmtdefs.verbose) fprintf (stderr, "%s: UNIF:  Number of evaluation points = %d, number of extrapolation points = %d\n", GMT_program, nm, ierror);
	}
	else if (mode == 2) {	/* c-1 interpolation (intrc1) with global gradients gradg. */
		/* initialize gradients grad to zeros. */
		memset ((void *)grad, 0, (size_t)(3*n*sizeof(double)));
		if (!vartens) itgs = 1;

		/* loop on gradg/getsig iterations. */

		iflgs = 0;
		for (iter = 0; iter < itgs; iter++) {
			nitg = maxit;
			dgmx = dgmax;
			gradg_ (&n4, x, y, z, w, list, lptr, lend, &iflgs, sigma, &nitg, &dgmx, grad, &ierror);
			if (ierror < 0) {
				fprintf (stderr, "%s: Error in GRADG:  ier = %d\n", GMT_program, ierror);
				GMT_exit (EXIT_FAILURE);
			}
			if (gmtdefs.verbose) {
				fprintf (stderr, "%s: GRADG:  tolerance = %g max change = %g  maxit = %d no. iterations = %d ier = %d\n",
					GMT_program, dgmax, dgmx, maxit, nitg, ierror);
			}
			if (vartens) {
				/* compute tension factors sigma (getsig).  iflgs > 0 if vartens = true */
				iflgs = 1;
				getsig_ (&n4, x, y, z, w, list, lptr, lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					fprintf (stderr, "%s: Error in GETSIG:  ier = %d\n", GMT_program, ierror);
					GMT_exit (EXIT_FAILURE);
				}
				if (gmtdefs.verbose) fprintf (stderr, "%s: GETSIG:  %d tension factors altered;  Max change = %g\n", GMT_program, ierror, dsm);
				sgprnt_ (&n4, &lsig, list, lptr, lend, sigma);	/* write the tension factors to disk (sgprnt). */
			}
		}
		/* compute interpolated values on the uniform grid (unif). */

		unif_ (&n4, x, y, z, w, list, lptr, lend, &iflgs, sigma, &h->ny, &h->ny, &h->nx, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			fprintf (stderr, "%s: Error in UNIF:  IER = %d\n", GMT_program, ierror);
			GMT_exit (EXIT_FAILURE);
		}
		if (gmtdefs.verbose) fprintf (stderr, "%s: UNIF:  Number of evaluations = %d, number of extrapolations = %d\n", GMT_program, nm, ierror);
	}
	else if (mode == 3) {	/* c-1 smoothing method smsurf. */
		double wtk, smtol, gstol, e, sm, *wt;
		wt = (double *)GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program);
		e = (par[0] == 0.0) ? 0.01 : par[0];
		sm = (par[1] <= 0.0) ? (double)n : par[1];
		wtk = 1.0/e;
		/* store the weights wt. */
		for (k = 0; k < n; k++) wt[k] = wtk;
		/* compute and print smsurf parameters. */

		smtol = sqrt (2.0 / sm);
		gstol = 0.05*e;
		if (gmtdefs.verbose) {
			fprintf (stderr, "%s: smsurf parameters:\n\texpected squared error = %g\n\tsmoothing parameter sm = %g\n", GMT_program, e, sm);
			fprintf (stderr, "\tgauss-seidel tolerance = %g\n\tsmoothing tolerance = %g\n\tweights = %g\n", gstol, smtol, wtk);
		}
		if (!vartens) itgs = 1;
		/* loop on smsurf/getsig iterations. */
		iflgs = 0;
		for (iter = 0; iter < itgs; iter++) {
			smsurf_ (&n4, x, y, z, w, list, lptr, lend, &iflgs, sigma, wt, &sm, &smtol, &gstol, &minus, f, grad, &ierror);
			if (ierror < 0) {
				fprintf (stderr, "%s: Error in SMSURF:  IER = %d\n", GMT_program, ierror);
				GMT_exit (EXIT_FAILURE);
			}
			if (ierror == 1) fprintf (stderr, "%s: UNIF: inactive constraint in smsurf.  f is a constant function\n", GMT_program);
			if (vartens) {	/* compute tension factors sigma (getsig).  iflgs > 0 if vt = true. */
				iflgs = 1;
				getsig_ (&n4, x, y, z, f, list, lptr, lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					fprintf (stderr, "%s: Error in GETSIG:  ier = %d\n", GMT_program, ierror);
					GMT_exit (EXIT_FAILURE);
				}
				if (gmtdefs.verbose) fprintf (stderr, "%s: GETSIG:  %d tension factors altered;  Max change = %g\n", GMT_program, ierror, dsm);
				sgprnt_ (&n4, &lsig, list, lptr, lend, sigma);	/* write the tension factors to disk (sgprnt). */
			}
		}
		/* compute interpolated values on the uniform grid (unif). */
		unif_ (&n4, x, y, z, w, list, lptr, lend, &iflgs, sigma, &h->ny, &h->ny, &h->nx, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			fprintf (stderr, "%s: Error in UNIF:  ier = %d\n", GMT_program, ierror);
			GMT_exit (EXIT_FAILURE);
		}
		if (gmtdefs.verbose) fprintf (stderr, "%s: UNIF:  Number of evaluations = %d, number of extrapolations = %d\n", GMT_program, nm, ierror);
	}
	
	GMT_free ((void *)plon);
	GMT_free ((void *)plat);
}