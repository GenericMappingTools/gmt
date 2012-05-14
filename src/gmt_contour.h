/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
/* gmt_contour.h - structures and variables needed for contour labels.

   Author:	Paul Wessel
   Date:	1 JAN 2010
   Version:	5 API

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
	COUNTER_LARGE start;	/* y-array index for minimum y endpoint */
	COUNTER_LARGE stop;	/* y-array index for maximum y endpoint */
};

struct GMT_LABEL {	/* Contains information on contour/lineation labels */
	double x, y;		/* Point where label goes */
	double angle;		/* Angle of text unless curved text */
	double line_angle;	/* Angle of line at label unless curved text */
	double dist;
	COUNTER_LARGE node;		/* Node of label on the line */
	GMT_LONG end;		/* If N is used then -1 is start, +1 is end label */
	char *label;
};

struct GMT_CONTOUR_LINE {
	COUNTER_LARGE n;			/* Length of the contour */
	COUNTER_MEDIUM annot;			/* TRUE if we want labels */
	COUNTER_MEDIUM n_labels;		/* Number of labels; if 0 we just have a line segment */
	double z;			/* Datum of this contour (z-value) */
	double rgb[4];			/* Box rgb */
	double *x, *y;			/* Coordinates of the contour */
	struct GMT_PEN pen;		/* Pen for drawing contour */
	struct GMT_LABEL *L;		/* Pointer to array of structures with labels */
	char *name;			/* Contour name */
};

struct GMT_CONTOUR {
	/* Control section */
	double label_isolation;		/* Only have one label inside a circle of this radius */
	double label_dist_spacing;	/* Min distance between labels */
	double label_dist_frac;		/* Fraction of Min distance between labels offset for closed labels [0.25] */
	double d_scale;			/* Scale to yield correct units */
	double L_d_scale;		/* Scale to yield correct units for label content only*/
	double min_radius;		/* Do not place labels if the radius of curvature drops below this value [0] */
	double min_dist;		/* Do not place labels closer than this value [0] */
	double slop;			/* slop distance in being close to points */
	double *f_xy[2];		/* Array for fixed points */
	double label_angle;		/* For fixed-angle labels only */
	double clearance[2];		/* Spacing between text and textbox */
	double nudge[2];		/* Shift between calculated and desired text placement */
	double rgb[4];			/* Opaque box fill */
	COUNTER_MEDIUM current_file_no;	/* Number (0->) of current input data file */
	COUNTER_LARGE current_seg_no;	/* Number (0->) of current segment in current data file */
	BOOLEAN annot;			/* TRUE if we want labels */
	BOOLEAN isolate;		/* TRUE if we have a limit on how close labels may appear (see below) */
	BOOLEAN spacing;		/* TRUE if we have spacing constraints to apply */
	COUNTER_MEDIUM line_type;	/* Kind of line: contour (1) or line (0) */
	COUNTER_MEDIUM dist_kind;	/* What kind of distance [0 = xy, 1 = map ] */
	COUNTER_MEDIUM dist_unit;	/* Units for labelled distances along tracks [cip] */
	COUNTER_MEDIUM proj_type;	/* type of scaling */
	COUNTER_MEDIUM L_proj_type;	/* type of scaling for label content only */
	COUNTER_MEDIUM half_width;	/* Number of points to use in smoothing the angle [10/2] */
	BOOLEAN number;			/* TRUE if we have constraints on the number of labels to apply */
	GMT_LONG number_placement;	/* How the n_cont labels are distributed [-1/0/+1]*/
	COUNTER_MEDIUM n_cont;		/* Number of labels per segment */
	BOOLEAN do_interpolate;		/* TRUE if we must resample the crossing lines */
	COUNTER_MEDIUM crossing;	/* 1 for crossing simple lines, 2 for file with crossing lines */
	COUNTER_MEDIUM nx;		/* Number of crossovers at any time */
	BOOLEAN fixed;			/* TRUE if we chose fixed positions */
	COUNTER_MEDIUM f_n;			/* Number of such points */
	COUNTER_MEDIUM clearance_flag;	/* 1 if spacing given in % of labelfont size, 0 otherwise */
	COUNTER_MEDIUM nudge_flag;	/* 0 if off, 1 if nudging relative to x/y axis, 2 if following local line coordinate system */
	BOOLEAN transparent;		/* TRUE for transparent textbox, FALSE for opaque */
	COUNTER_MEDIUM box;		/* Textbox bits [1 = outline, 2 = rect box shape, 4 = rounded rect shape] */
	BOOLEAN curved_text;		/* TRUE for text to follow curved lines */
	COUNTER_MEDIUM n_label;		/* Length of list */
	COUNTER_MEDIUM just;		/* Label justification */
	COUNTER_MEDIUM end_just[2];	/* Justification for end of lines */
	COUNTER_MEDIUM angle_type;	/* 0 = contour-parallel, 1 = contour-normal, 2 = fixed angle */
	GMT_LONG hill_label;		/* -1/+1 = make label readable when looking down/up gradient, 0 = no special treatment  */
	GMT_LONG no_gap;		/* Clip contour or not depends on label placement */
	COUNTER_MEDIUM label_type;	/* 0 = what is passed, 1 = fixed label above , 2 = multiseg header, 3 = distances */
	BOOLEAN save_labels;		/* TRUE if we wish to save label locations to a text file */
	BOOLEAN data_col;		/* TRUE if there is data in the zz arrays passed, FALSE if they are NULL */
	BOOLEAN debug;			/* TRUE of we want to draw helper lines/points */
	BOOLEAN delay;			/* TRUE of we want to delay the actual annotation plotting until later */
	COUNTER_MEDIUM n_segments;		/* The number of segments */
	size_t n_alloc;			/* How many allocated so far */
	char file[GMT_BUFSIZ];		/* File with crossing lines, if specified */
	char option[GMT_BUFSIZ];	/* Copy of the option string */
	char label[GMT_BUFSIZ];		/* Fixed label */
	char label_file[GMT_BUFSIZ];	/* Output files for text dump of label locations */
	char unit[GMT_TEXT_LEN64];	/* Unit for labels */
	char prefix[GMT_TEXT_LEN64];	/* prefix for labels */
	char line_name[16];		/* Name of line: contour or line */
	char flag;			/* Char for the option key */
	char **f_label;			/* Array for fixed labels */
	p_func_d dist_func;		/* Pointer to function that calculates distances */
	p_func_d L_dist_func;		/* Pointer to function that calculates distances for label content only */
	struct GMT_FONT font_label;	/* Which font */
	struct GMT_TABLE *xp;		/* Table with list of structures with crossing-line coordinates */
	struct GMT_XSEGMENT *ylist_XP;	/* Sorted y-segments for crossing-lines */
	struct GMT_XSEGMENT *ylist;	/* y-indices sorted in increasing order */
	struct GMT_XOVER XC;		/* Structure with resulting crossovers */
	struct GMT_PEN pen;		/* Pen for drawing textbox outline */
	struct GMT_PEN line_pen;	/* Pen for drawing the contour line */
	struct GMT_LABEL **L;		/* Pointers to sorted list of labels */
	/* Contour line section */
	
	struct GMT_CONTOUR_LINE **segment;	/* Array of segments */
};

#endif /* _GMT_CONTOUR_H */
