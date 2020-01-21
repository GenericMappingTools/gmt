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
 * Spherical triangulation - Delaunay or Voronoi options.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *    and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *    Software, 23 (3), 416-434.
 * Spherical interpolation - tension or smoothing.
 * Relies on SSRFPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of
 *    Scattered Data on the Surface of a Sphere with a Surface under Tension,
 *    AMC Trans. Math. Software, 23 (3), 435-442.
 * We translated both to C using f2c and removed/rewrite statements that needed -lf2c
 *
 * Author:      Paul Wessel
 * Date:        1-AUG-2011
 * Version:	API 5 64-bit
 *
 */

#include "gmt_dev.h"
#include "gmt_sph.h"

typedef double doublereal;
typedef int64_t integer;
typedef uint32_t logical;

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif
#define FALSE_ 0
#define TRUE_ 1

/* define SPH_DEBUG to get more original verbose output from s*pack.c */

/* This makes all the static functions in the next to files local here */
#include "stripack.c"
#include "ssrfpack.c"

int gmt_stripack_lists (struct GMT_CTRL *GMT, uint64_t n_in, double *x, double *y, double *z, struct STRIPACK *T) {
 	/* n, the number of points.
	 * x, y, z, the arrays with coordinates of points
	 *
	 * xc, yc, zc: the coordinates of the Voronoi polygon vertices.
	 * lend, points to the "first" vertex in the Voronoi polygon around a particular node.
	 * lptr: given a vertex, returns the next vertex in the Voronoi polygon.
	 *
	 * NOTE: All indices returned are C (0->) adjusted from FORTRAN (1->).
	 */

	uint64_t kk;
	int64_t *iwk = NULL, *list = NULL, *lptr = NULL, *lend = NULL;
	int64_t n = n_in, n_out, k, ierror= 0, lnew, nrow = TRI_NROW;	/* Since the Fortran funcs expect signed ints */
	size_t n_alloc;
	double *ds = NULL;

	ds = gmt_M_memory (GMT, NULL, n, double);
	lend = gmt_M_memory (GMT, NULL, n, int64_t);
 	iwk = gmt_M_memory (GMT, NULL, 2*n, int64_t);
	n_alloc = 6 * (n - 2);
	lptr = gmt_M_memory (GMT, NULL, n_alloc, int64_t);
	list = gmt_M_memory (GMT, NULL, n_alloc, int64_t);

	/* Create the triangulation. Main output is (list, lptr, lend) */

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Call STRIPACK TRMESH subroutine...");
	trmesh_ (&n, x, y, z, list, lptr, lend, &lnew, iwk, &iwk[n], ds, &ierror);
	gmt_M_free (GMT, ds);
	gmt_M_free (GMT, iwk);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "OK\n");

	if (ierror == -2) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "STRIPACK: Error in TRMESH. The first 3 nodes are collinear.\n");
		gmt_M_free (GMT, lptr);
		gmt_M_free (GMT, list);
		gmt_M_free (GMT, lend);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	if (ierror > 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "STRIPACK: Error in TRMESH.  Duplicate nodes encountered.\n");
		gmt_M_free (GMT, lptr);
		gmt_M_free (GMT, list);
		gmt_M_free (GMT, lend);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	if (T->mode == INTERPOLATE) {	/* Pass back the three lists from trmesh_ */
		T->I.list = list;	/* Save these for output */
		T->I.lptr = lptr;
		T->I.lend = lend;
		return GMT_OK;
	}

	/* Create a triangle list which returns the number of triangles and their node list tri */

	n_alloc = 2 * (n - 2);
	T->D.tri = gmt_M_memory (GMT, NULL, TRI_NROW*n_alloc, int64_t);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Call STRIPACK TRLIST subroutine...");
	trlist_ (&n, list, lptr, lend, &nrow, &n_out, T->D.tri, &ierror);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "OK\n");
	T->D.n = n_out;

	if (ierror) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "STRIPACK: Error in TRLIST.\n");
		gmt_M_free (GMT, list);		gmt_M_free (GMT, lend);		gmt_M_free (GMT, lptr);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	if (T->mode == VORONOI) {	/* Construct the Voronoi diagram */
		int64_t *lbtri = NULL;
		double *rc = NULL;
		double *xc = NULL, *yc = NULL, *zc = NULL;	/* Voronoi polygon vertices */

		/* Note that the triangulation data structure is altered if NB > 0 */

		n_alloc = 2 * (n - 2);
		xc = gmt_M_memory (GMT, NULL, n_alloc, double);
		yc = gmt_M_memory (GMT, NULL, n_alloc, double);
		zc = gmt_M_memory (GMT, NULL, n_alloc, double);
		rc = gmt_M_memory (GMT, NULL, n_alloc, double);
		n_alloc = 6 * (n - 2);
		T->V.listc = gmt_M_memory (GMT, NULL, n_alloc, int64_t);
		lbtri = gmt_M_memory (GMT, NULL, 6*n, int64_t);

		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Call STRIPACK CRLIST subroutine...");
		crlist_ (&n, &n, x, y, z, list, lend, lptr, &lnew, lbtri, T->V.listc, &n_out, xc, yc, zc, rc, &ierror);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "OK\n");
		T->V.n = n_out;
		gmt_M_free (GMT, lbtri);
		gmt_M_free (GMT, rc);
		T->V.lend = lend;	/* Save these for output */
		T->V.lptr = lptr;
		/* Convert polygon vertices vectors to lon, lat */
		n_alloc = 2 * (n - 2);
		T->V.lon = gmt_M_memory (GMT, NULL, n_alloc, double);
		T->V.lat = gmt_M_memory (GMT, NULL, n_alloc, double);
		gmt_n_cart_to_geo (GMT, n_alloc, xc, yc, zc, T->V.lon, T->V.lat);
		gmt_M_free (GMT, xc);
		gmt_M_free (GMT, yc);
		gmt_M_free (GMT, zc);

		if (0 < ierror) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "STRIPACK: Error in CRLIST.  IERROR = %" PRId64 ".\n", ierror);
			gmt_M_free (GMT, list);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
	
		/* Adjust Fortran to GMT indices */
		n_alloc = 6 * (n - 2);
		for (kk = 0; kk < n_alloc; kk++) T->V.listc[kk]--;
		for (kk = 0; kk < n_alloc; kk++) T->V.lptr[kk]--;
		for (k = 0; k < n; k++) T->V.lend[k]--;
	}
	else {	/* Free things not needed */
		gmt_M_free (GMT, lend);
		gmt_M_free (GMT, lptr);
	}

	/* Adjust Fortran to GMT indices */
	for (kk = 0; kk < TRI_NROW*T->D.n; kk++) T->D.tri[kk]--;

	gmt_M_free (GMT, list);
	return (GMT_OK);
}

double gmt_stripack_areas (double *V1, double *V2, double *V3) {
	/* Wrapper for STRIPACK areas_ */
	return (areas_ (V1, V2, V3));
}

/* Functions for spherical surface interpolation */

int gmt_ssrfpack_grid (struct GMT_CTRL *GMT, double *x, double *y, double *z, double *w, uint64_t n_in, unsigned int mode, double *par, bool vartens, struct GMT_GRID_HEADER *h, double *f) {
	int64_t ierror, plus = 1, minus = -1, ij, nxp, k, n = n_in;
	int64_t nm, n_sig, ist, iflgs, iter, itgs, n_columns = h->n_columns, n_rows = h->n_rows;
	unsigned int row, col;
	double *sigma = NULL, *grad = NULL, *plon = NULL, *plat = NULL, tol = 0.01, dsm, dgmx;
	struct STRIPACK P;

	n_sig = ((vartens) ? 6 * (n - 2) : 1);

	/* Create the triangulation. Main output is (P.I->(list, lptr, lend) */

	gmt_M_memset (&P, 1, struct STRIPACK);
	P.mode = INTERPOLATE;
	gmt_stripack_lists (GMT, n, x, y, z, &P);

	/* Set out output nodes */

	plon = gmt_M_memory (GMT, NULL, h->n_columns, double);
	plat = gmt_M_memory (GMT, NULL, h->n_rows, double);
	for (col = 0; col < h->n_columns; col++) plon[col] = D2R * gmt_M_grd_col_to_x (GMT, col, h);
	for (row = 0; row < h->n_rows; row++) plat[row] = D2R * gmt_M_grd_row_to_y (GMT, row, h);
	nm = h->n_columns * h->n_rows;

	/* Time to work on the interpolation */

	sigma = gmt_M_memory (GMT, NULL, n_sig, double);
	if (mode) grad = gmt_M_memory (GMT, NULL, 3*n, double);

	if (mode == 0) {	 /* C-0 interpolation (INTRC0). */
		nxp = 0;
		ist = 1;
		for (row = 0; row < h->n_rows; row++) {
			for (col = 0; col < h->n_columns; col++) {
				ij = (uint64_t)col * (uint64_t)h->n_rows + (uint64_t)row; /* Use Fortran indexing since calling program will transpose to GMT order */
				intrc0_ (&n, &plat[row], &plon[col], x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &ist, &f[ij], &ierror);
				if (ierror > 0) nxp++;
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in INTRC0: I = %d, J = %d, IER = %" PRId64 "\n", row, col, ierror);
					gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
					gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "INTRC0: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, nxp);
	}
	else if (mode == 1) {	/* C-1 interpolation (INTRC1) with local gradients GRADL. */
	   	/* Accumulate the sum of the numbers of nodes used in the least squares fits in sum. */
		int64_t k1;
		double sum = 0.0;
		for (k = 0; k < n; k++) {
			k1 = k + 1;	/* Since gradl expects Fortran indexing */
			gradl_ (&n, &k1, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &grad[3*k], &ierror);
			if (ierror < 0) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GRADL: K = %" PRId64 " IER = %" PRId64 "\n", k1, ierror);
				gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
				gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
				GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			}
			sum += (double)ierror;
		}
		sum /= n;
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GRADL: Average number of nodes used in the least squares fits = %g\n", sum);
	        if (vartens) {	/* compute tension factors sigma (getsig). */
				getsig_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GETSIG: IER = %" PRId64 "\n", ierror);
					gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
					gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GETSIG: %" PRId64 " tension factors altered;  Max change = %g\n", ierror, dsm);
	        }

		/* compute interpolated values on the uniform grid (unif). */

		iflgs = 0;
		if (vartens) iflgs = 1;
		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in UNIF: IER = %" PRId64 "\n", ierror);
			gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
			gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		            "UNIF: Number of evaluation points = %" PRId64 ", number of extrapolation points = %" PRId64 "\n", nm, ierror);
	}
	else if (mode == 2) {	/* c-1 interpolation (intrc1) with global gradients gradg. */
		int64_t maxit, nitg;
		double dgmax;
		/* initialize gradients grad to zeros. */
		gmt_M_memset (grad, 3*n, double);
		itgs  = (par[0] == 0.0) ? 3    : lrint (par[0]);
		maxit = (par[1] == 0.0) ? 10   : lrint (par[1]);
		dgmax = (par[2] == 0.0) ? 0.01 : par[2];
		if (!vartens) itgs = 1;

		/* loop on gradg/getsig iterations. */

		for (iter = iflgs = 0; iter < itgs; iter++) {
			nitg = maxit;
			dgmx = dgmax;
			gradg_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &nitg, &dgmx, grad, &ierror);
			if (ierror < 0) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GRADG (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
				gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
				gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
				GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
			            "GRADG (iteration %" PRId64 "): tolerance = %g max change = %g  maxit = %" PRId64 " no. iterations = %" PRId64 " ier = %" PRId64 "\n",
				iter, dgmax, dgmx, maxit, nitg, ierror);
			if (vartens) {
				/* compute tension factors sigma (getsig).  iflgs > 0 if vartens = true */
				iflgs = 1;
				getsig_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GETSIG (iteration %" PRId64 "): ier = %" PRId64 "\n", iter, ierror);
					gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
					gmt_M_free (GMT, sigma);	gmt_M_free (GMT, grad);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
				            "GETSIG (iteration %" PRId64 "): %" PRId64 " tension factors altered;  Max change = %g\n", iter, ierror, dsm);
			}
		}
		/* compute interpolated values on the uniform grid (unif). */

		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in UNIF: IER = %" PRId64 "\n", ierror);
			gmt_M_free (GMT, grad);		gmt_M_free (GMT, plat);
			gmt_M_free (GMT, plon);		gmt_M_free (GMT, sigma);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		            "UNIF: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, ierror);
	}
	else if (mode == 3) {	/* c-1 smoothing method smsurf. */
		double wtk, smtol, gstol, e, sm, *wt = gmt_M_memory (GMT, NULL, n, double);
		e    = (par[0] == 0.0) ? 0.01 : par[0];
		sm   = (par[1] <= 0.0) ? (double)n : par[1];
		itgs = (par[2] == 0.0) ? 3 : lrint (par[2]);
		if (!vartens) itgs = 1;
		wtk = 1.0 / e;
		for (k = 0; k < n; k++) wt[k] = wtk;	/* store the weights wt. */
		/* compute and print smsurf parameters. */
		smtol = sqrt (2.0 / sm);
		gstol = 0.05 * e;
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		            "SMSURF parameters:\n\texpected squared error = %g\n\tsmoothing parameter sm = %g\n", e, sm);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		            "\tgauss-seidel tolerance = %g\n\tsmoothing tolerance = %g\n\tweights = %g\n", gstol, smtol, wtk);

		/* loop on smsurf/getsig iterations. */
		for (iter = iflgs = 0; iter < itgs; iter++) {
			smsurf_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, wt, &sm, &smtol, &gstol, &minus, f, grad, &ierror);
			if (ierror < 0) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in SMSURF (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
				gmt_M_free (GMT, grad);		gmt_M_free (GMT, plat);		gmt_M_free (GMT, plon);
				gmt_M_free (GMT, wt);		gmt_M_free (GMT, sigma);
				GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			}
			if (ierror == 1)
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
				            "Error in SMSURF: inactive constraint in SMSURF (iteration %" PRId64 ").  f is a constant function\n", iter);
			if (vartens) {	/* compute tension factors sigma (getsig).  iflgs > 0 if vt = true. */
				iflgs = 1;
				getsig_ (&n, x, y, z, f, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GETSIG (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
					gmt_M_free (GMT, grad);		gmt_M_free (GMT, wt);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
				            "GETSIG (iteration %" PRId64 "): %" PRId64 " tension factors altered;  Max change = %g\n", iter, ierror, dsm);
			}
		}
		/* compute interpolated values on the uniform grid (unif). */
		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		gmt_M_free (GMT, wt);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in UNIF: ier = %" PRId64 "\n", ierror);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
		            "UNIF: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, ierror);
	}

	gmt_M_free (GMT, plon);
	gmt_M_free (GMT, plat);
	gmt_M_free (GMT, P.I.list);
	gmt_M_free (GMT, P.I.lptr);
	gmt_M_free (GMT, P.I.lend);
	gmt_M_free (GMT, sigma);
	gmt_M_free (GMT, grad);
	return (GMT_OK);
}
