/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *			G M T _ B C R . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_bcr.c contains interpolation routines based on convolution kernels
 *	(BCR = bi-dimensional convolution routines)
 *
 * Designed to operate with two rows and columns around each edge,
 * to allow derivative computations.
 * Follows the outline in Lancaster and Salkauskas, section 9.3
 * Meant to replace Taylor routines in GMT in version 3.0 of GMT.
 * Significantly simplified and extended for GMT version 4.2.1.
 *
 * Currently supports:
 * - Nearest neighbor "interpolation"
 *   This attaches a weight of 1 to the nearest grid cell only.
 *
 * - Bi-linear interpolation
 *   The weight given to each of the 4 vertices surrounding the interpolation
 *   point is the product of the weight function in x and y direction
 *   (wx and wy) who have the same form and depend on the projected distance
 *   |t| between the interpolation point and each vertex.
 *
 *   wx = 1 - |t|	for 0 <= |t| <= 1
 *	= 0		for 1 <= |t|
 *
 * - B-spline smoothing
 *   This is a smoothing kernel. I.e., the interpolated value directly
 *   at a grid node is not equal to the nodal value. This is because wx(0) < 1.
 *   The convolution kernel spans 16 vertices surrounding the interpolation point.
 *
 *   wx = 1/2 |t|^3 - |t|^2 + 2/3		for 0 <= |t| <= 1
 *	= -1/6 |t|^3 + |t|^2 -2 |t| + 4/3	for 1 <= |t| <= 2
 *	= 0					for 2 <= |t|
 *
 * - Bi-cubic interpolation
 *   This is the preferred option. It provides a smooth interpolation that
 *   is twice differential in both directions. The values at the grid nodes
 *   remain unchanged. The convolution kernel spans 16 vertices surrounding the
 *   interpolation point.
 *
 *   wx = (a+2) |t|^3 - (a+3) |t|^2 + 1		for 0 <= |t| <= 1
 *	= a |t|^3 - 5a |t|^2 + 8a |t| - 4a	for 1 <= |t| <= 2
 *	= 0					for 2 <= |t|
 *   and  a = -1/2
 *
 * Because of the size of the convolution kernels, these routines assume that
 * the grid is padded with appropriate (periodic or natural) boundary conditions.
 *
 * Authors:	Walter Smith and Remko Scharroo
 * Date:	23-SEP-1993 and 11-SEP-2007
 * Version:	5
 * Now 64-bit enabled.
 *
 * Public functions:
 *
 *	GMT_bcr_init		Initialize structure for convolution interpolation
 *	GMT_get_bcr_z		Get interpolated grid value by convolution
 *	GMT_get_bcr_img		Get interpolated image value(s) by convolution
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

unsigned int gmt_bcr_reject (struct GMT_GRID_HEADER *h, double *xx, double *yy)
{
	/* First check that xx,yy are not Nan - if so return NaN */

	if (GMT_is_dnan (*xx) || GMT_is_dnan (*yy)) return (2);

	/* First check if the xx and yy are within the grid.
	   16-Sep-2007: Added some slack (GMT_CONV4_LIMIT) here to avoid setting to NaN points
	   that are really on the edge but because of rounding errors are regarded outside.
	   Remember that we have padded the grid with 2 extra values, so this should not be
	   a problem. */
	/* 9-Sep-2014: No, it is a problem when things are outside, since at the end we loop over
	   4 rows.  So if just outside by < GMT_CONV4_LIMIT then we move the points onto the boundary */

	if (*xx < h->wesn[XLO]) {	/* If left of xmin... */
		if (*xx < h->wesn[XLO] - GMT_CONV4_LIMIT) return (1); /* ...by this much then truly outside */
		*xx = h->wesn[XLO];	/* Else we say it is on xmin border */
	}
	else if (*xx > h->wesn[XHI]) {	/* If right of xmax... */
		if (*xx > h->wesn[XHI] + GMT_CONV4_LIMIT) return (1); /* ...by this much then truly outside */
		*xx = h->wesn[XHI];	/* Else we say it is on xmax border */
	}

	if (*yy < h->wesn[YLO]) {	/* If below ymin... */
		if (*yy < h->wesn[YLO] - GMT_CONV4_LIMIT) return (1); /* ...by this much then truly outside */
		*yy = h->wesn[YLO];	/* Else we say it is on ymin border */
	}
	else if (*yy > h->wesn[YHI]) {	/* If above ymax... */
		if (*yy > h->wesn[YHI] + GMT_CONV4_LIMIT) return (1); /* ...by this much then truly outside */
		*yy = h->wesn[YHI];	/* Else we say it is on ymin border */
	}

	return (0);	/* Good to use */
}

uint64_t gmt_bcr_prep (struct GMT_GRID_HEADER *h, double xx, double yy, double wx[], double wy[])
{
	int col, row;
	uint64_t ij;
	double x, y, wp, wq, w, xi, yj;

	/* Compute the normalized real indices (x,y) of the point (xx,yy) within the grid.
	   Note that the y axis points down from the upper left corner of the grid. */

	x = (xx - h->wesn[XLO]) * h->r_inc[GMT_X] - h->xy_off;
	y = (h->wesn[YHI] - yy) * h->r_inc[GMT_Y] - h->xy_off;

	if (h->bcr_interpolant == BCR_NEARNEIGHBOR) {
		/* Find the indices (i,j) of the closest node. */
		col = irint (x);
		row = irint (y);
	}
	else {
		/* Find the indices (i,j) of the node to the upper left of that.
	   	   Because of padding, i and j can be on the edge. */
		xi  = floor (x);
		yj  = floor (y);
		col = irint (xi);
		row = irint (yj);

		/* Determine the offset of (x,y) with respect to (i,j). */
		x -= xi;
		y -= yj;

		/* For 4x4 interpolants, move over one more cell to the upper left corner */
		if (h->bcr_n == 4) { col--; row--; }
	}

	/* Normally, one would expect here a check on the value (i,j) to make sure that the
	   corners of the convolution kernel, (i,j) and (i+bcr->n-1,j+bcr->n-1), are both within
	   the padded grid. However, the check on (xx, yy) above, even with the slack, ensures
	   that the corner points are between (-2,-2) and (G->header->nx+1,G->header->ny+1), the corners
	   of the padding.
	*/

	/* Save the location of the upper left corner point of the convolution kernel */
	ij = GMT_IJP (h, row, col);

	/* Build weights */

	switch (h->bcr_interpolant) {
	case BCR_NEARNEIGHBOR:
		wx[0] = wy[0] = 1.0;
		break;
	case BCR_BILINEAR:
		/* Simple 1-D linear weights */
		wx[0] = 1.0 - x;
		wx[1] = x;

		wy[0] = 1.0 - y;
		wy[1] = y;
		break;
	case BCR_BSPLINE:
		/* These are B-spline weights */
		wp = x * x;
		wq = wp * x;
		wx[1] = wq / 2 - wp + 2.0 / 3.0;
		wx[3] = wq / 6;
		w = 1.0 - x;
		wp = w * w;
		wq = wp * w;
		wx[2] = wq / 2 - wp + 2.0 / 3.0;
		wx[0] = wq / 6;

		wp = y * y;
		wq = wp * y;
		wy[1] = wq / 2 - wp + 2.0 / 3.0;
		wy[3] = wq / 6;
		w = 1.0 - y;
		wp = w * w;
		wq = wp * w;
		wy[2] = wq / 2 - wp + 2.0 / 3.0;
		wy[0] = wq / 6;
		break;
	default:
		/* These weights are based on the cubic convolution kernel, see for example
		   http://undergraduate.csse.uwa.edu.au/units/CITS4241/Handouts/Lecture04.html
		   These weights include a free parameter (a), which is set to -0.5 in this case.

		   In the absence of NaNs, the result of this is identical to the scheme introduced
		   by Walter Smith. The current implementation, however, is much less complex, faster,
		   allows NaNs to be skipped, and much more similar to the bilinear case.

		   Remko Scharroo, 10 Sep 2007.
		*/
		w = 1.0 - x;
		wp = w * x;
		wq = -0.5 * wp;
		wx[0] = wq * w;
		wx[3] = wq * x;
		wx[1] = 3 * wx[3] + w + wp;
		wx[2] = 3 * wx[0] + x + wp;

		w = 1.0 - y;
		wp = w * y;
		wq = -0.5 * wp;
		wy[0] = wq * w;
		wy[3] = wq * y;
		wy[1] = 3 * wy[3] + w + wp;
		wy[2] = 3 * wy[0] + y + wp;
		break;
	}

	return (ij);
}

double GMT_get_bcr_z_fast (struct GMT_CTRL *GMT, struct GMT_GRID *G, double xx, double yy)
{
	/* Given xx, yy in user's grid file (in non-normalized units)
	   this routine returns the desired interpolated value (nearest-neighbor, bilinear
	   B-spline or bicubic) at xx, yy.
	   Same as GMT_get_bcr_z but no check for Nan or outside, so calling program
	   must make sure we dont go outside array or pass nans.
	*/

	unsigned int i, j;
	uint64_t ij, node;
	double retval, wsum, wx[4], wy[4], w;

	/* Determine nearest node ij and set weights wx, wy */

	ij = gmt_bcr_prep (G->header, xx, yy, wx, wy);

	retval = wsum = 0.0;
	for (j = 0; j < G->header->bcr_n; j++) {
		for (i = 0; i < G->header->bcr_n; i++) {
			/* assure that index is inside bounds of the array G->data: */
			node = ij + i;
			assert (node < G->header->size);
			w = wx[i] * wy[j];
			retval += G->data[node] * w;
			wsum += w;
		}
		ij += G->header->mx;
	}
	if ((wsum + GMT_CONV8_LIMIT - G->header->bcr_threshold) > 0.0) {
		retval /= wsum;
		if (GMT->common.n.truncate) {
			if (retval < G->header->z_min) retval = G->header->z_min;
			else if (retval > G->header->z_max) retval = G->header->z_max;
		}
		return (retval);
	}
	return (GMT->session.d_NaN);
}

double GMT_get_bcr_z (struct GMT_CTRL *GMT, struct GMT_GRID *G, double xx, double yy)
{
	/* Given xx, yy in user's grid file (in non-normalized units)
	   this routine returns the desired interpolated value (nearest-neighbor, bilinear
	   B-spline or bicubic) at xx, yy. */

	unsigned int i, j;
	uint64_t ij, node;
	double retval, wsum, wx[4], wy[4], w;

	/* First check that xx,yy are not Nan or outside domain - if so return NaN */

	if (gmt_bcr_reject (G->header, &xx, &yy)) return (GMT->session.d_NaN);	/* NaNs or outside */

	/* Determine nearest node ij and set weights wx, wy */

	ij = gmt_bcr_prep (G->header, xx, yy, wx, wy);

	retval = wsum = 0.0;
	for (j = 0; j < G->header->bcr_n; j++) {
		for (i = 0; i < G->header->bcr_n; i++) {
			/* assure that index is inside bounds of the array G->data: */
			node = ij + i;
			assert (node < G->header->size);
			if (!GMT_is_fnan (G->data[node])) {
				w = wx[i] * wy[j];
				retval += G->data[node] * w;
				wsum += w;
			}
		}
		ij += G->header->mx;
	}
	if ((wsum + GMT_CONV8_LIMIT - G->header->bcr_threshold) > 0.0) {
		retval /= wsum;
		if (GMT->common.n.truncate) {
			if (retval < G->header->z_min) retval = G->header->z_min;
			else if (retval > G->header->z_max) retval = G->header->z_max;
		}
		return (retval);
	}
	return (GMT->session.d_NaN);
}

int GMT_get_bcr_img (struct GMT_CTRL *GMT, struct GMT_IMAGE *G, double xx, double yy, unsigned char *z)
{
	/* Given xx, yy in user's image file (in non-normalized units)
	   this routine returns the desired interpolated image value (nearest-neighbor, bilinear
	   B-spline or bicubic) at xx, yy. 8-bit components is assumed per band.  */

	unsigned int i, j, b, nb = G->header->n_bands;
	uint64_t ij;
	double retval[4], wsum, wx[4], wy[4], w;

	/* First check that xx,yy are not Nan or outside domain - if so return NaN */

	if (gmt_bcr_reject (G->header, &xx, &yy)) return (1);	/* NaNs or outside */

	/* Determine nearest node ij and set weights wx wy */

	ij = gmt_bcr_prep (G->header, xx, yy, wx, wy);

	GMT_memset (retval, 4, double);
	wsum = 0.0;
	for (j = 0; j < G->header->bcr_n; j++) {
		for (i = 0; i < G->header->bcr_n; i++) {
			w = wx[i] * wy[j];
			wsum += w;
			for (b = 0; b < nb; b++) retval[b] += G->data[nb*(ij+i)+b] * w;
		}
		ij += G->header->mx;
	}
	if ((wsum + GMT_CONV8_LIMIT - G->header->bcr_threshold) > 0.0) {	/* OK to evaluate result */
		for (b = 0; b < nb; b++) {
			retval[b] /= wsum;
			z[b] = (unsigned char) lrint (GMT_0_255_truncate (retval[b]));
		}
	}
	else
		for (b = 0; b < nb; b++) z[b] = GMT_u255 (GMT->current.setting.color_patch[GMT_NAN][b]);
	return (0);
}
