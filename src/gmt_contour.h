/*--------------------------------------------------------------------
 *	$Id: gmt_contour.h,v 1.2 2004-05-18 22:29:08 pwessel Exp $
 *
 *	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
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
/* gmt_contour.h - structures and variables needed for contour labels.

   Author:	Paul Wessel
   Date:	18 May 2004
   Version:	4

   This include file defines structures and functions used.
*/

#ifndef _GMT_CONTOUR_H
#define _GMT_CONTOUR_H

#define GMT_CONTOUR_XLINE	1
#define GMT_CONTOUR_XCURVE	2

struct GMT_XOVER {		/* Structure with info on all track cross-over */
	double *x;		/* x or Longitude */
	double *y;		/* y or Latitude */
	double *xnode[2];	/* Decimal Node index at cross-over along track 1 and 2 */
};

struct GMT_XSEGMENT {
	int start;	/* y-array index for minimum y endpoint */
	int stop;	/* y-array index for maximum y endpoint */
};

struct GMT_CONTOUR {
	char option[BUFSIZ];		/* Copy of the option string */
	char flag;			/* Char for the option key */
	BOOLEAN spacing;		/* TRUE if we have spacing constraints to apply */
	double label_dist_spacing;	/* Min distance between labels */
	int half_width;			/* Number of points to use in smoothing the angle [10/2] */
	
	BOOLEAN number;			/* TRUE if we have constraints on the number of labels to apply */
	int n_cont;			/* Number of labels per segment */
	char file[BUFSIZ];		/* File with crossing lines, if specified */
	BOOLEAN do_interpolate;		/* TRUE if we must resample the crossing lines */
	int crossing;			/* 1 for crossing simple lines, 2 for file with crossing lines */
	struct GMT_LINES *xp;		/* List of structures with crossing-line coordinates */
	int n_xp;			/* Number of such lines */
	struct GMT_XSEGMENT *ylist_XP;	/* Sorted y-segments for crossing-lines */
	struct GMT_XSEGMENT *ylist;	/* y-indices sorted in increasing order */
	struct GMT_XOVER XC;		/* Structure with resulting crossovers */
	int nx;				/* Number of crossovers at any time */
};

EXTERN_MSC int GMT_contlabel_info (char flag, char *txt, struct GMT_CONTOUR *L);
EXTERN_MSC int GMT_contlabel_init (struct GMT_CONTOUR *G);
EXTERN_MSC double GMT_contlabel_angle (double x[], double y[], double x0, double y0, int start, int stop, int width, int n);
EXTERN_MSC int GMT_code_to_lonlat (char *code, double *lon, double *lat);
EXTERN_MSC void GMT_x_free (struct GMT_XOVER *X);
EXTERN_MSC struct GMT_XSEGMENT *GMT_init_track (double x[], double y[], int n);
EXTERN_MSC int GMT_crossover (double xa[], double ya[], int sa[], struct GMT_XSEGMENT A[], int na, double xb[], double yb[], int sb[], struct GMT_XSEGMENT B[], int nb, BOOLEAN internal, struct GMT_XOVER *X);

#endif /* _GMT_CONTOUR_H */
