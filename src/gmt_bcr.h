/*--------------------------------------------------------------------
 *	$Id: gmt_bcr.h,v 1.25 2011-04-23 02:14:12 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
   Version:	5 API

   This include file defines structures and functions used.
*/

#ifndef _GMT_BCR_H
#define _GMT_BCR_H

#define BCR_NEARNEIGHBOR	0
#define BCR_BILINEAR		1
#define BCR_BSPLINE		2
#define BCR_BICUBIC		3

struct GMT_BCR {	/* Used mostly in gmt_bcr.c, but also in gmt_map.c, grdsample_func.c, grdtrack_func.c, grdview_func.c */
	double rx_inc;		/* 1.0 / grd.x_inc  */
	double ry_inc;		/* 1.0 / grd.y_inc  */
	double offset;		/* 0 or 0.5 for grid or pixel registration  */
	double threshold;	/* sum of cardinals must >= threshold in bilinear; else NaN */
	GMT_LONG interpolant;	/* Interpolation function used (0, 1, 2, 3) */
	GMT_LONG n;		/* Width of the interpolation function */
};
#endif /* _GMT_BCR_H */
