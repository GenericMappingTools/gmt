/*--------------------------------------------------------------------
 *	$Id: gmt_bcr.h,v 1.23 2011-03-03 21:02:50 guru Exp $
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
/* bcr.h  --  stuff for implementing bi-dimensional convolution interpolation.

   Authors:	Walter H F Smith and Remko Scharroo
   Date:	23-SEP-1993 and 11-SEP-2007
   Version:	4.1.x

   This include file defines structures and functions used.
*/

#ifndef _GMT_BCR_H
#define _GMT_BCR_H

#define BCR_NEARNEIGHBOR	0
#define BCR_BILINEAR		1
#define BCR_BSPLINE		2
#define BCR_BICUBIC		3

struct GMT_BCR {	/* Used mostly in gmt_support.c */
	double rx_inc;			/* 1.0 / grd.x_inc  */
	double ry_inc;			/* 1.0 / grd.y_inc  */
	double offset;			/* 0 or 0.5 for grid or pixel registration  */
	double threshold;		/* sum of cardinals must >= threshold in bilinear; else NaN */
	GMT_LONG interpolant;		/* Interpolation function used (0, 1, 2, 3) */
	GMT_LONG n;			/* Width of the interpolation function */
	GMT_LONG	ioff;		/* Padding on west side of array  */
	GMT_LONG	joff;		/* Padding on north side of array  */
	GMT_LONG	mx;		/* Padded array dimension  */
	GMT_LONG	my;		/* Ditto  */
};

EXTERN_MSC void GMT_bcr_init (struct GRD_HEADER *grd, GMT_LONG *pad, GMT_LONG bilinear, double threshold, struct GMT_BCR *bcr);
EXTERN_MSC double GMT_get_bcr_z (struct GRD_HEADER *grd, double xx, double yy, float *data,  struct GMT_EDGEINFO *edgeinfo, struct GMT_BCR *bcr);		/* Compute z(x,y) from bcr structure  */
#endif /* _GMT_BCR_H */
