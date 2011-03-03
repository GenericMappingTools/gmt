/*--------------------------------------------------------------------
 *	$Id: gmt_contour.h,v 1.51 2011-03-03 21:02:50 guru Exp $
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
	GMT_LONG start;	/* y-array index for minimum y endpoint */
	GMT_LONG stop;	/* y-array index for maximum y endpoint */
};

struct GMT_LABEL {	/* Contains information on contour/lineation labels */
	double x, y;		/* Point where label goes */
	double angle;		/* Angle of text unless curved text */
	double line_angle;	/* Angle of line at label unless curved text */
	double dist;
	GMT_LONG node;
	GMT_LONG end;		/* If N is used then -1 is start, +1 is end label */
	char *label;
};

struct GMT_CONTOUR_LINE {
	double *x, *y;			/* Coordinates of the contour */
	double z;			/* Datum of this contour (z-value) */
	GMT_LONG n;			/* Length of the contour */
	GMT_LONG annot;			/* TRUE if we want labels */
	char *name;			/* Contour name */
	struct GMT_PEN pen;		/* Pen for drawing contour */
	int font_rgb[3];		/* Font color */
	struct GMT_LABEL *L;		/* Pointer to array of structures with labels */
	GMT_LONG n_labels;		/* Number of labels; if 0 we just have a line segment */
};

struct GMT_CONTOUR {
	/* Control section */
	char option[BUFSIZ];		/* Copy of the option string */
	char label[BUFSIZ];		/* Fixed label */
	char line_name[16];		/* Name of line: contour or line */
	char flag;			/* Char for the option key */
	GMT_LONG current_file_no;	/* Number (0->) of current input data file */
	GMT_LONG current_seg_no;	/* Number (0->) of current segment in current data file */
	GMT_LONG annot;			/* TRUE if we want labels */
	GMT_LONG isolate;		/* TRUE if we have a limit on how close labels may appear (see below) */
	double label_isolation;		/* Only have one label inside a circle of this radius */
	GMT_LONG spacing;		/* TRUE if we have spacing constraints to apply */
	double label_dist_spacing;	/* Min distance between labels */
	double label_dist_frac;		/* Fraction of Min distance between labels offset for closed labels [0.25] */
	GMT_LONG line_type;		/* Kind of line: contour (1) or line (0) */
	GMT_LONG label_font;		/* Which font */
	GMT_LONG dist_kind;		/* What kind of distance [0 = xy, 1 = map ] */
	GMT_LONG dist_unit;		/* Units for labelled distances along tracks [cimp] */
	PFD dist_func;			/* Pointer to function that calculates distances */
	double d_scale;			/* Scale to yield correct units */
	GMT_LONG proj_type;		/* type of scaling */
	PFD L_dist_func;		/* Pointer to function that calculates distances for label content only */
	double L_d_scale;		/* Scale to yield correct units for label content only*/
	GMT_LONG L_proj_type;		/* type of scaling for label content only */
	GMT_LONG half_width;		/* Number of points to use in smoothing the angle [10/2] */
	double min_radius;		/* Do not place labels if the radius of curvature drops below this value [0] */
	double min_dist;		/* Do not place labels closer than this value [0] */
	GMT_LONG number;			/* TRUE if we have constraints on the number of labels to apply */
	GMT_LONG number_placement;	/* How the n_cont labels are distributed */
	GMT_LONG n_cont;		/* Number of labels per segment */
	char file[BUFSIZ];		/* File with crossing lines, if specified */
	GMT_LONG do_interpolate;		/* TRUE if we must resample the crossing lines */
	GMT_LONG crossing;		/* 1 for crossing simple lines, 2 for file with crossing lines */
	struct GMT_TABLE *xp;		/* Table with list of structures with crossing-line coordinates */
	struct GMT_XSEGMENT *ylist_XP;	/* Sorted y-segments for crossing-lines */
	struct GMT_XSEGMENT *ylist;	/* y-indices sorted in increasing order */
	struct GMT_XOVER XC;		/* Structure with resulting crossovers */
	GMT_LONG nx;			/* Number of crossovers at any time */
	GMT_LONG fixed;			/* TRUE if we chose fixed positions */
	double slop;			/* slop distance in being close to points */
	double *f_xy[2];		/* Array for fixed points */
	char **f_label;			/* Array for fixed labels */
	GMT_LONG f_n;			/* Number of such points */
	double label_font_size;		/* Font size for labels */
	double label_angle;		/* For fixed-angle labels only */
	double clearance[2];		/* Spacing between text and textbox */
	GMT_LONG clearance_flag;	/* 1 if spacing given in % of labelfont size, 0 otherwise */
	double nudge[2];		/* Shift between calculated and desired text placement */
	GMT_LONG nudge_flag;		/* 0 if off, 1 if nudging relative to x/y axis, 2 if following local line coordinate system */
	GMT_LONG transparent;		/* TRUE for transparent textbox, FALSE for opaque */
	GMT_LONG box;			/* Textbox bits [1 = outline, 2 = rect box shape, 4 = rounded rect shape] */
	GMT_LONG curved_text;		/* TRUE for text to follow curved lines */
	int rgb[3];			/* Opaque box color */
	int font_rgb[3];		/* Font color */
	GMT_LONG got_font_rgb;		/* TRUE if +k was specified */
	struct GMT_PEN pen;		/* Pen for drawing textbox outline */
	struct GMT_PEN line_pen;	/* Pen for drawing the contour line */
	struct GMT_LABEL **L;		/* Pointers to sorted list of labels */
	GMT_LONG n_label;		/* Length of list */
	char unit[GMT_TEXT_LEN];	/* Unit for labels */
	char prefix[GMT_TEXT_LEN];	/* prefix for labels */
	GMT_LONG just;			/* Label justification */
	GMT_LONG end_just[2];		/* Justification for end of lines */
	GMT_LONG angle_type;		/* 0 = contour-parallel, 1 = contour-normal, 2 = fixed angle */
	GMT_LONG hill_label;		/* -1/+1 = make label readable when looking down/up gradient, 0 = no special treatment  */
	GMT_LONG no_gap;			/* Clip contour or not depends on label placement */
	GMT_LONG label_type;		/* 0 = what is passed, 1 = fixed label above , 2 = multiseg header, 3 = distances */
	double z_level;			/* When plotted in 3-D we must have z = z_level (i.e., all points have fixed z) */
	GMT_LONG data_col;		/* TRUE if there is data in the zz arrays passed, FALSE if they are NULL */
	GMT_LONG debug;			/* TRUE of we want to draw helper lines/points */
	/* Contour line section */
	
	struct GMT_CONTOUR_LINE **segment;	/* Array of segments */
	GMT_LONG n_segments;			/* The number of segments */
	GMT_LONG n_alloc;			/* How many allocated so far */
};

EXTERN_MSC GMT_LONG GMT_contlabel_info (char flag, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_init (struct GMT_CONTOUR *G, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_contlabel_specs (char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC GMT_LONG GMT_contlabel_prep (struct GMT_CONTOUR *G, double xyz[2][3]);
EXTERN_MSC void GMT_contlabel_angle (double x[], double y[], GMT_LONG start, GMT_LONG stop, double cangle, GMT_LONG n, struct GMT_LABEL *L, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_draw (double x[], double y[], double d[], GMT_LONG n, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_plot (struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_free (struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_hold_contour (double **xx, double **yy, GMT_LONG nn, double zval, char *label, char ctype, double cangle, GMT_LONG closed, struct GMT_CONTOUR *G);
EXTERN_MSC GMT_LONG GMT_code_to_lonlat (char *code, double *lon, double *lat);
EXTERN_MSC void GMT_x_free (struct GMT_XOVER *X);
EXTERN_MSC GMT_LONG GMT_init_track (double y[], GMT_LONG n, struct GMT_XSEGMENT **S);
EXTERN_MSC GMT_LONG GMT_crossover (double xa[], double ya[], GMT_LONG sa[], struct GMT_XSEGMENT A[], GMT_LONG na, double xb[], double yb[], GMT_LONG sb[], struct GMT_XSEGMENT B[], GMT_LONG nb, GMT_LONG internal, struct GMT_XOVER *X);

#endif /* _GMT_CONTOUR_H */
