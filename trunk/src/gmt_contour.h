/*--------------------------------------------------------------------
 *	$Id: gmt_contour.h,v 1.8 2004-05-25 04:59:47 pwessel Exp $
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

struct GMT_LABEL {	/* Contains information on contour/lineation labels */
	double *x, *y;		/* Either 1 point where label goes OR a path along which to place label */
	int n;			/* Length of path or 1 */
	double angle;		/* Angle of text unless curved text */
	double line_angle;	/* ANgle of line at label unless curved text */
	double dist;
	int node;
	char label[32];
	struct GMT_LABEL *next_label, *prev_label;
};

struct GMT_CONTOUR {
	char option[BUFSIZ];		/* Copy of the option string */
	char label[BUFSIZ];		/* Fixed label */
	char flag;			/* Char for the option key */
	BOOLEAN annot;			/* TRUE if we want labels */
	BOOLEAN spacing;		/* TRUE if we have spacing constraints to apply */
	double label_dist_spacing;	/* Min distance between labels */
	int dist_kind;			/* What kind of distance [0 = xy, 1 = map ] */
	PFD dist_func;			/* Pointer to function that calculates distances */
	double d_scale;			/* Scale to yield correct units */
	int proj_type;			/* type of scaling */
	int half_width;			/* Number of points to use in smoothing the angle [10/2] */
	
	BOOLEAN number;			/* TRUE if we have constraints on the number of labels to apply */
	int number_placement;		/* How the n_cont labels are distributed */
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
	double label_font_size;		/* Font size for labels */
	double label_angle;		/* For fixed-angle labels only */
	double clearance[2];		/* Spacing between text and textbox */
	int box;			/* Textbox [0 = transparent, 1 = filled, 2 = filled + outline */
	BOOLEAN curved_text;		/* TRUE for text to follow curved lines */
	int rgb[3];			/* Opaque box color */
	struct GMT_PEN pen;		/* Pen for drawing box */
	struct GMT_LABEL *anchor, *old_label;	/* Linked list of contours */
	struct GMT_LABEL **L;		/* Pointers to sorted list of labels */
	int n_label;			/* Length of list */
	char unit[32];			/* Unit for labels */
	int just;			/* Label justification */
	int end_just[2];		/* Justification for end of lines */
	int angle_type;			/* 0 = contour-parallel, 1 = contour-normal, 2 = fixed angle */
	BOOLEAN no_gap;			/* Clip contour or not depends on label placement */
	int label_type;			/* 0 = what is passed, 1 = fixed label above , 2 = multiseg header, 3 = distances */
	double z_level;			/* When plotted in 3-D we must have z = z_level (i.e., all points have fixed z) */
};

EXTERN_MSC int GMT_contlabel_info (char flag, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_init (struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_specs (char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_prep (struct GMT_CONTOUR *G, double xyz[2][3]);
EXTERN_MSC void GMT_contlabel_angle (double x[], double y[], int start, int stop, double cangle, int n, struct GMT_LABEL *L, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_draw (double x[], double y[], double d[], int n, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_plot (struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_draw_contour (double *xx, double *yy, int *pen, int nn, char *label, char ctype, double cangle, int closed, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_code_to_lonlat (char *code, double *lon, double *lat);
EXTERN_MSC void GMT_x_free (struct GMT_XOVER *X);
EXTERN_MSC struct GMT_XSEGMENT *GMT_init_track (double x[], double y[], int n);
EXTERN_MSC int GMT_crossover (double xa[], double ya[], int sa[], struct GMT_XSEGMENT A[], int na, double xb[], double yb[], int sb[], struct GMT_XSEGMENT B[], int nb, BOOLEAN internal, struct GMT_XOVER *X);

#endif /* _GMT_CONTOUR_H */
