/*--------------------------------------------------------------------
 *	$Id: gmt_contour.h,v 1.22 2004-06-22 20:00:28 pwessel Exp $
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
	double x, y;		/* Point where label goes */
	double angle;		/* Angle of text unless curved text */
	double line_angle;	/* Angle of line at label unless curved text */
	double dist;
	int node;
	char *label;
};

struct GMT_CONTOUR_LINE {
	double *x, *y;			/* Coordinates of the contour */
	int n;				/* Length of the contour */
	BOOLEAN annot;			/* TRUE if we want labels */
	char *name;			/* Contour name */
	struct GMT_PEN pen;		/* Pen for drawing contour */
	int font_rgb[3];		/* Font color */
	struct GMT_LABEL *L;		/* Pointer to array of strictures with labels */
	int n_labels;			/* Number of labels; if 0 we just have a line segment */
};

struct GMT_CONTOUR {
	/* Control section */
	char option[BUFSIZ];		/* Copy of the option string */
	char label[BUFSIZ];		/* Fixed label */
	char line_name[16];		/* Name of line: contour or line */
	char flag;			/* Char for the option key */
	int current_file_no;		/* Number (0->) of current input data file */
	int current_seg_no;		/* Number (0->) of current segment in current data file */
	BOOLEAN annot;			/* TRUE if we want labels */
	BOOLEAN spacing;		/* TRUE if we have spacing constraints to apply */
	double label_dist_spacing;	/* Min distance between labels */
	double label_dist_frac;		/* Fraction of Min distance between labels offset for closed labels [0.25] */
	int line_type;			/* Kind of line: contour (1) or line (0) */
	int label_font;			/* Which font */
	int dist_kind;			/* What kind of distance [0 = xy, 1 = map ] */
	int dist_unit;			/* Units for labelled distances along tracks [cimp] */
	PFD dist_func;			/* Pointer to function that calculates distances */
	double d_scale;			/* Scale to yield correct units */
	int proj_type;			/* type of scaling */
	PFD L_dist_func;		/* Pointer to function that calculates distances for label content only */
	double L_d_scale;		/* Scale to yield correct units for label content only*/
	int L_proj_type;		/* type of scaling for label content only */
	int half_width;			/* Number of points to use in smoothing the angle [10/2] */
	double min_radius;		/* Do not place labels if the radius of curvature drops below this value [0] */
	double min_dist;		/* Do not place labels closer than this value [0] */
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
	BOOLEAN fixed;			/* TRUE if we chose fixed positions */
	double slop;			/* slop distance in being close to points */
	double *f_xy[2];		/* Array for fixed points */
	char **f_label;			/* Array for fixed labels */
	int f_n;			/* Number of such points */
	double label_font_size;		/* Font size for labels */
	double label_angle;		/* For fixed-angle labels only */
	double clearance[2];		/* Spacing between text and textbox */
	int clearance_flag;		/* 1 if spacing given in % of labelfont size, 0 otherwise */
	BOOLEAN transparent;		/* TRUE for transparent textbox, FALSE for opaque */
	int box;			/* Textbox bits [1 = outline, 2 = rect box shape, 4 = rounded rect shape] */
	BOOLEAN curved_text;		/* TRUE for text to follow curved lines */
	int rgb[3];			/* Opaque box color */
	int font_rgb[3];		/* Font color */
	BOOLEAN got_font_rgb;		/* TRUE if +k was specified */
	struct GMT_PEN pen;		/* Pen for drawing textbox outline */
	struct GMT_PEN line_pen;	/* Pen for drawing the contour line */
	struct GMT_LABEL **L;		/* Pointers to sorted list of labels */
	int n_label;			/* Length of list */
	char unit[32];			/* Unit for labels */
	char prefix[32];		/* prefix for labels */
	int just;			/* Label justification */
	int end_just[2];		/* Justification for end of lines */
	int angle_type;			/* 0 = contour-parallel, 1 = contour-normal, 2 = fixed angle */
	BOOLEAN no_gap;			/* Clip contour or not depends on label placement */
	int label_type;			/* 0 = what is passed, 1 = fixed label above , 2 = multiseg header, 3 = distances */
	double z_level;			/* When plotted in 3-D we must have z = z_level (i.e., all points have fixed z) */
	BOOLEAN data_col;		/* TRUE if there is data in the zz arrays passed, FALSE if they are NULL */
	BOOLEAN debug;			/* TRUE of we want to draw helper lines/points */
	/* Contour line section */
	
	struct GMT_CONTOUR_LINE **segment;	/* Array of segments */
	int n_segments;				/* The number of segments */
	size_t n_alloc;				/* How many allocated so far */
};

EXTERN_MSC int GMT_contlabel_info (char flag, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_init (struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_specs (char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_prep (struct GMT_CONTOUR *G, double xyz[2][3], int mode);
EXTERN_MSC void GMT_contlabel_angle (double x[], double y[], int start, int stop, double cangle, int n, struct GMT_LABEL *L, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_draw (double x[], double y[], double d[], int n, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_plot (struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_hold_contour (double **xx, double **yy, int nn, char *label, char ctype, double cangle, int closed, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_code_to_lonlat (char *code, double *lon, double *lat);
EXTERN_MSC void GMT_x_free (struct GMT_XOVER *X);
EXTERN_MSC struct GMT_XSEGMENT *GMT_init_track (double x[], double y[], int n);
EXTERN_MSC int GMT_crossover (double xa[], double ya[], int sa[], struct GMT_XSEGMENT A[], int na, double xb[], double yb[], int sb[], struct GMT_XSEGMENT B[], int nb, BOOLEAN internal, struct GMT_XOVER *X);

#endif /* _GMT_CONTOUR_H */
