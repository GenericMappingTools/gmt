/*--------------------------------------------------------------------
 *	$Id: gmt_bcr.c,v 1.12 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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
 * Version:	4
 * Now 64-bit enabled.
 *
 * Modules in this file:
 *
 *	GMT_bcr_init		Initialize structure for convolution interpolation
 *	GMT_get_bcr_z		Get interpolated value by convolution
 */

#define GMT_WITH_NO_PS
#include "gmt.h"

void GMT_bcr_init (struct GRD_HEADER *grd, GMT_LONG *pad, GMT_LONG interpolant, double threshold, struct GMT_BCR *bcr)
{
	/* Initialize interpolant and threshold */
	bcr->interpolant = interpolant;
	bcr->threshold = threshold;
	if (interpolant == BCR_NEARNEIGHBOR)
		bcr->n = 1;
	else if (interpolant == BCR_BILINEAR)
		bcr->n = 2;
	else
		bcr->n = 4;

	/* Initialize ioff, joff, mx, my according to grd and pad:  */
	bcr->ioff = pad[0];
	bcr->joff = pad[3];
	bcr->mx = (GMT_LONG)(grd->nx + pad[0] + pad[1]);
	bcr->my = (GMT_LONG)(grd->ny + pad[2] + pad[3]);

	/* Initialize rx_inc, ry_inc, and offset:  */
	bcr->rx_inc = 1.0 / grd->x_inc;
	bcr->ry_inc = 1.0 / grd->y_inc;
	bcr->offset = (grd->node_offset) ? 0.5 : 0.0;
}

double GMT_get_bcr_z (struct GRD_HEADER *grd, double xx, double yy, float *data, struct GMT_EDGEINFO *edgeinfo, struct GMT_BCR *bcr)
{
	/* Given xx, yy in user's grid file (in non-normalized units)
	   this routine returns the desired interpolated value (nearest-neighbor, bilinear
	   B-spline or bicubic) at xx, yy. */

	GMT_LONG i, j, ij;
	double	x, y, retval, wsum, wx[4], wy[4], w, wp, wq;

	/* First check that xx,yy are not Nan - if so return NaN */
	
	if (GMT_is_dnan (xx) || GMT_is_dnan (yy)) return (GMT_d_NaN);
	
	/* First check if the xx and yy are within the grid.
	   16-Sep-2007: Added some slack (GMT_SMALL) here to avoid setting to NaN points
	   that are really on the edge but because of rounding errors are regarded outside.
	   Remember that we have padded the grid with 2 extra values, so this should not be
	   a problem. */

	if (xx < grd->x_min - GMT_SMALL || xx > grd->x_max + GMT_SMALL) return (GMT_d_NaN);
	if (yy < grd->y_min - GMT_SMALL || yy > grd->y_max + GMT_SMALL) return (GMT_d_NaN);

	/* Compute the normalized real indices (x,y) of the point (xx,yy) within the grid.
	   Note that the y axis points down from the upper left corner of the grid. */

	x = (xx - grd->x_min) * bcr->rx_inc - bcr->offset;
	y = (grd->y_max - yy) * bcr->ry_inc - bcr->offset;

	if (bcr->interpolant == BCR_NEARNEIGHBOR) {
		/* Find the indices (i,j) of the closest node. */
		i = irint(x);
		j = irint(y);
	}
	else {
		/* Find the indices (i,j) of the node to the upper left of that.
	   	   Because of padding, i and j can be on the edge. */
		i = (GMT_LONG)floor(x);
		j = (GMT_LONG)floor(y);

		/* Determine the offset of (x,y) with respect to (i,j). */
		x -= (double)i;
		y -= (double)j;

		/* For 4x4 interpolants, move over one more cell to the upper left corner */
		if (bcr->n == 4) { i--; j--; }
	}

	/* Normally, one would expect here a check on the value (i,j) to make sure that the
	   corners of the convolution kernel, (i,j) and (i+bcr->n-1,j+bcr->n-1), are both within
	   the padded grid. However, the check on (xx, yy) above, even with the slack, ensures
	   that the corner points are between (-2,-2) and (grd->nx+1,grd->ny+1), the corners
	   of the padding.

	if (i < -2 || j < -2 || i+bcr->n > grd_nx+2 || j+bcr->n > grd_ny+2) return (GMT_d_NaN);
	*/

	/* Save the location of the upper left corner point of the convolution kernel */
	ij = (j + bcr->joff) * bcr->mx + (i + bcr->ioff);

	/* Build weights */

	switch (bcr->interpolant) {
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

	retval = wsum = 0.0;
	for (j = 0; j < bcr->n; j++) {
		for (i = 0; i < bcr->n; i++) {
			if (!GMT_is_fnan(data[ij+i])) {
				w = wx[i] * wy[j];
				retval += data[ij+i] * w;
				wsum += w;
			}
		}
		ij += bcr->mx;
	}
	return ( ((wsum + GMT_CONV_LIMIT - bcr->threshold) > 0.0) ? retval / wsum : GMT_d_NaN);
}
