/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2008-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Call STRIPACK TRMESH subroutine\n");
	trmesh_ (&n, x, y, z, list, lptr, lend, &lnew, iwk, &iwk[n], ds, &ierror);
	gmt_M_free (GMT, ds);
	gmt_M_free (GMT, iwk);

	if (ierror == -2) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "STRIPACK: Failure in TRMESH. The first 3 nodes are collinear.\n");
		gmt_M_free (GMT, lptr);
		gmt_M_free (GMT, list);
		gmt_M_free (GMT, lend);
		return GMT_RUNTIME_ERROR;
	}

	if (ierror > 0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "STRIPACK: Failure in TRMESH.  Duplicate nodes encountered.\n");
		gmt_M_free (GMT, lptr);
		gmt_M_free (GMT, list);
		gmt_M_free (GMT, lend);
		return GMT_RUNTIME_ERROR;
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
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Call STRIPACK TRLIST subroutine\n");
	trlist_ (&n, list, lptr, lend, &nrow, &n_out, T->D.tri, &ierror);
	T->D.n = n_out;

	if (ierror) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "STRIPACK: Failure in TRLIST.\n");
		gmt_M_free (GMT, list);		gmt_M_free (GMT, lend);		gmt_M_free (GMT, lptr);
		return GMT_RUNTIME_ERROR;
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

		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Call STRIPACK CRLIST subroutine\n");
		crlist_ (&n, &n, x, y, z, list, lend, lptr, &lnew, lbtri, T->V.listc, &n_out, xc, yc, zc, rc, &ierror);
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
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "STRIPACK: Failure in CRLIST.  IERROR = %" PRId64 ".\n", ierror);
			gmt_M_free (GMT, list);
			return GMT_RUNTIME_ERROR;
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
	int error = GMT_NOERROR;
	int64_t ierror, plus = 1, minus = -1, ij, nxp, k, n = n_in;
	int64_t nm, n_sig, ist, iflgs, iter, itgs, n_columns = h->n_columns, n_rows = h->n_rows;
	unsigned int row, col;
	double *sigma = NULL, *grad = NULL, *plon = NULL, *plat = NULL, *wt = NULL, tol = 0.01, dsm, dgmx;
	struct STRIPACK P;

	n_sig = ((vartens) ? 6 * (n - 2) : 1);

	/* Create the triangulation. Main output is (P.I->(list, lptr, lend) */

	gmt_M_memset (&P, 1, struct STRIPACK);
	P.mode = INTERPOLATE;
	if (gmt_stripack_lists (GMT, n, x, y, z, &P)) return GMT_RUNTIME_ERROR;

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
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in INTRC0: I = %d, J = %d, IER = %" PRId64 "\n", row, col, ierror);
					error = GMT_RUNTIME_ERROR;
					goto free_mem;
				}
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "INTRC0: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, nxp);
	}
	else if (mode == 1) {	/* C-1 interpolation (INTRC1) with local gradients GRADL. */
	   	/* Accumulate the sum of the numbers of nodes used in the least squares fits in sum. */
		int64_t k1;
		double sum = 0.0;
		for (k = 0; k < n; k++) {
			k1 = k + 1;	/* Since gradl expects Fortran indexing */
			gradl_ (&n, &k1, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &grad[3*k], &ierror);
			if (ierror < 0) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in GRADL: K = %" PRId64 " IER = %" PRId64 "\n", k1, ierror);
				error = GMT_RUNTIME_ERROR;
				goto free_mem;
			}
			sum += (double)ierror;
		}
		sum /= n;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "GRADL: Average number of nodes used in the least squares fits = %g\n", sum);
	        if (vartens) {	/* compute tension factors sigma (getsig). */
				getsig_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in GETSIG: IER = %" PRId64 "\n", ierror);
					error = GMT_RUNTIME_ERROR;
					goto free_mem;
				}
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "GETSIG: %" PRId64 " tension factors altered;  Max change = %g\n", ierror, dsm);
	        }

		/* compute interpolated values on the uniform grid (unif). */

		iflgs = 0;
		if (vartens) iflgs = 1;
		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in UNIF: IER = %" PRId64 "\n", ierror);
			error = GMT_RUNTIME_ERROR;
			goto free_mem;
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
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
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in GRADG (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
				error = GMT_RUNTIME_ERROR;
				goto free_mem;
			}
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
			            "GRADG (iteration %" PRId64 "): tolerance = %g max change = %g  maxit = %" PRId64 " no. iterations = %" PRId64 " ier = %" PRId64 "\n",
				iter, dgmax, dgmx, maxit, nitg, ierror);
			if (vartens) {
				/* compute tension factors sigma (getsig).  iflgs > 0 if vartens = true */
				iflgs = 1;
				getsig_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in GETSIG (iteration %" PRId64 "): ier = %" PRId64 "\n", iter, ierror);
					error = GMT_RUNTIME_ERROR;
					goto free_mem;
				}
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
				            "GETSIG (iteration %" PRId64 "): %" PRId64 " tension factors altered;  Max change = %g\n", iter, ierror, dsm);
			}
		}
		/* compute interpolated values on the uniform grid (unif). */

		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in UNIF: IER = %" PRId64 "\n", ierror);
			error = GMT_RUNTIME_ERROR;
			goto free_mem;
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
		            "UNIF: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, ierror);
	}
	else if (mode == 3) {	/* c-1 smoothing method smsurf. */
		double wtk, smtol, gstol, e, sm;
		wt = gmt_M_memory (GMT, NULL, n, double);
		e    = (par[0] == 0.0) ? 0.01 : par[0];
		sm   = (par[1] <= 0.0) ? (double)n : par[1];
		itgs = (par[2] == 0.0) ? 3 : lrint (par[2]);
		if (!vartens) itgs = 1;
		wtk = 1.0 / e;
		for (k = 0; k < n; k++) wt[k] = wtk;	/* store the weights wt. */
		/* compute and print smsurf parameters. */
		smtol = sqrt (2.0 / sm);
		gstol = 0.05 * e;
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
		            "SMSURF parameters:\n\texpected squared error = %g\n\tsmoothing parameter sm = %g\n", e, sm);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
		            "\tgauss-seidel tolerance = %g\n\tsmoothing tolerance = %g\n\tweights = %g\n", gstol, smtol, wtk);

		/* loop on smsurf/getsig iterations. */
		for (iter = iflgs = 0; iter < itgs; iter++) {
			smsurf_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, wt, &sm, &smtol, &gstol, &minus, f, grad, &ierror);
			if (ierror < 0) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in SMSURF (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
				error = GMT_RUNTIME_ERROR;
				goto free_mem;
			}
			if (ierror == 1)
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
				            "Failure in SMSURF: inactive constraint in SMSURF (iteration %" PRId64 ").  f is a constant function\n", iter);
			if (vartens) {	/* compute tension factors sigma (getsig).  iflgs > 0 if vt = true. */
				iflgs = 1;
				getsig_ (&n, x, y, z, f, P.I.list, P.I.lptr, P.I.lend, grad, &tol, sigma, &dsm, &ierror);
				if (ierror < 0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in GETSIG (iteration %" PRId64 "): IER = %" PRId64 "\n", iter, ierror);
					error = GMT_RUNTIME_ERROR;
					goto free_mem;
				}
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
				            "GETSIG (iteration %" PRId64 "): %" PRId64 " tension factors altered;  Max change = %g\n", iter, ierror, dsm);
			}
		}
		/* compute interpolated values on the uniform grid (unif). */
		unif_ (&n, x, y, z, w, P.I.list, P.I.lptr, P.I.lend, &iflgs, sigma, &n_rows, &n_rows, &n_columns, plat, plon, &plus, grad, f, &ierror);
		gmt_M_free (GMT, wt);
		if (ierror < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in UNIF: ier = %" PRId64 "\n", ierror);
			error = GMT_RUNTIME_ERROR;
			goto free_mem;
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION,
		            "UNIF: Number of evaluations = %" PRId64 ", number of extrapolations = %" PRId64 "\n", nm, ierror);
	}

free_mem:	/* Time to free our memory */
	gmt_M_free (GMT, plon);
	gmt_M_free (GMT, plat);
	gmt_M_free (GMT, P.I.list);
	gmt_M_free (GMT, P.I.lptr);
	gmt_M_free (GMT, P.I.lend);
	gmt_M_free (GMT, sigma);
	gmt_M_free (GMT, grad);
	gmt_M_free (GMT, wt);

	return (error);
}

/* Determine if spherical triangle is oriented clockwise or counter-clockwise */
GMT_LOCAL int gmtsph_orientation (struct GMT_CTRL *GMT, double A[], double B[], double C[]) {
	double X[3];
	gmt_cross3v (GMT, A, B, X);
	return (gmt_dot3v (GMT, X, C) < 0.0 ? -1 : +1);
}

/* Compute spherical triangle area */

double gmtlib_geo_centroid_area (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n, double *centroid) {
	/* Based on ideas in http://www.jennessent.com/downloads/Graphics_Shapes_Online.pdf and Renka's area_ function.
	 * Compute area of a spherical polygon and its centroid. */
	unsigned int k, kind;
	int sgn;
	uint64_t p;
	double pol_area = 0.0, tri_area, clat, P0[3], P1[3], N[3], M[3], C[3] = {0.0, 0.0, 0.0}, center[2], R2;
	static char *way[2] = {"CCW", "CW"};

	/* pol_area is given in steradians and must be scaled by radius squared (R2) to yield a dimensioned area */
	R2 = pow (GMT->current.proj.mean_radius * GMT->current.map.dist[GMT_MAP_DIST].scale, 2.0);	/* R in desired length unit squared (R2) */
	if (n == 4) {	/* Apply directly on the spherical triangle (4 because of repeated point) */
		clat = gmt_lat_swap (GMT, lat[0], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
		gmt_geo_to_cart (GMT, clat, lon[0], N, true);	/* get x/y/z for first point*/
		clat = gmt_lat_swap (GMT, lat[1], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
		gmt_geo_to_cart (GMT, clat, lon[1], P0, true);	/* get x/y/z for 2nd point*/
		clat = gmt_lat_swap (GMT, lat[2], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
		gmt_geo_to_cart (GMT, clat, lon[2], P1, true);	/* get x/y/z for 3rd point*/
		sgn = gmtsph_orientation (GMT, N, P0, P1);
		pol_area = areas_ (N, P0, P1);	/* Absolute area of this spherical triangle N-P0-P1 */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
			kind = (sgn == -1) ? 0 : 1;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Spherical triangle %.4lg/%.4lg via %.4lg/%.4lg to %.4lg/%.4lg : Unit area %7.5lg oriented %3s\n", lon[0], lat[1], lon[1], lat[1], lon[2], lat[2], pol_area, way[kind]);
		}
		if (centroid) {
			for (k = 0; k < 3; k++) C[k] = N[k] + P0[k] + P1[k];
			gmt_normalize3v (GMT, C);
			gmt_cart_to_geo (GMT, &clat, &centroid[GMT_X], C, true);
			centroid[GMT_Y] = gmt_lat_swap (GMT, clat, GMT_LATSWAP_G2O+1);	/* Get geodetic latitude */
		}
		return (sgn * pol_area * R2);	/* Signed area in selected unit squared [km^2] */
	}

	/* Must split up polygon in a series of triangles */

	/* Pick a reference point.  Here, we use the mean vector location N */
	gmt_mean_point (GMT, lon, lat, n-1, 1, center);	/* One less to skip repeated point */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Mean spherical polygon point is %lg/%lg\n", center[0], center[1]);
	clat = gmt_lat_swap (GMT, center[GMT_Y], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
	gmt_geo_to_cart (GMT, clat, center[GMT_X], N, true);	/* Get x/y/z for mean point */
	/* Get first point in the polygon */
	clat = gmt_lat_swap (GMT, lat[0], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
	gmt_geo_to_cart (GMT, clat, lon[0], P0, true);	/* Get x/y/z for first point*/

	/* In the loop compute the signed areas N-P0-P1 */
	n--;	/* So we can use <= in loop */
	for (p = 1; p <= n; p++) {
		/* Each spherical triangle is defined by point N to p-1 (P0) to p (P1) */
		clat = gmt_lat_swap (GMT, lat[p], GMT_LATSWAP_G2O);	/* Get geocentric latitude */
		gmt_geo_to_cart (GMT, clat, lon[p], P1, true);	/* Get x/y/z for next point P1 */
		tri_area = areas_ (N, P0, P1);	/* Absolute area of this spherical triangle N-P0-P1 */
		sgn = gmtsph_orientation (GMT, N, P0, P1);	/* Sign of this area */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
			kind = (sgn == -1) ? 0 : 1;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Spherical triangle %.4lg/%.4lg via %.4lg/%.4lg to %.4lg/%.4lg : Unit area %7.5lg oriented %3s\n", center[0], center[1], lon[p-1], lat[p-1], lon[p], lat[p], tri_area, way[kind]);
		}
		/* Add up weighted contribution to polygon centroid */
		tri_area *= sgn;
		pol_area += tri_area;
		if (centroid) {	/* Compute centroid of this spherical triangle */
			for (k = 0; k < 3; k++) M[k] = N[k] + P0[k] + P1[k];
			gmt_normalize3v (GMT, M);
			for (k = 0; k < 3; k++) C[k] += M[k] * tri_area;
		}
		if (p < n) gmt_M_memcpy (P0, P1, 3, double);	/* Make P1 the next P0 except at end */
	}
	if (centroid) {
		for (k = 0; k < 3; k++) C[k] /= pol_area;	/* Get raw centroid */
		gmt_normalize3v (GMT, C);
		gmt_cart_to_geo (GMT, &clat, &centroid[GMT_X], C, true);
		centroid[GMT_Y] = gmt_lat_swap (GMT, clat, GMT_LATSWAP_G2O+1);	/* Get geodetic latitude */
	}
	return (pol_area * R2);	/* Signed area in selected unit squared [km^2] */
}
