/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

/* Various settings for contour label placements at crossing lines */
enum GMT_enum_contline {
	GMT_CONTOUR_NONE = 0,	/* No contour/line crossing  */
	GMT_CONTOUR_XLINE,	/* Contour labels where crossing straight lines (via key points) */
	GMT_CONTOUR_XCURVE};	/* Contour labels where crossing arbitrary lines (via file) */

/* Various settings for quoted line/contour label types */
enum GMT_enum_label {
	GMT_LABEL_IS_NONE = 0,	/* No contour/line crossing  */
	GMT_LABEL_IS_CONSTANT,	/* Label is constant, given by +l<label> */
	GMT_LABEL_IS_HEADER,	/* Label is taken from header, given by +Lh */
	GMT_LABEL_IS_PDIST,	/* Label is computed from plot distances, via +Ld */
	GMT_LABEL_IS_MDIST,	/* Label is computed from map distances, via +LD */
	GMT_LABEL_IS_FFILE,	/* Label is taken from 3rd column in given file, via +Lf */
	GMT_LABEL_IS_XFILE,	/* Label is taken from segment header in crossing file, via +Lx */
	GMT_LABEL_IS_SEG,	/* Label is computed from segment number, via +Ln */
	GMT_LABEL_IS_FSEG};	/* Label is computed from file and segment number, via +LN */

struct GMT_XOVER {		/* Structure with info on all track cross-over */
	double *x;		/* x or Longitude */
	double *y;		/* y or Latitude */
	double *xnode[2];	/* Decimal Node index at cross-over along track 1 and 2 */
};

struct GMT_XSEGMENT {
	uint64_t start;		/* y-array index for minimum y endpoint */
	uint64_t stop;		/* y-array index for maximum y endpoint */
};

struct GMT_LABEL {	/* Contains information on contour/lineation labels */
	double x, y;		/* Point where label goes */
	double angle;		/* Angle of text unless curved text */
	double line_angle;	/* Angle of line at label unless curved text */
	double dist;
	uint64_t node;		/* Node of label on the line */
	int end;		/* If N is used then -1 is start, +1 is end label */
	char *label;
};

struct GMT_CONTOUR_LINE {
	uint64_t n;			/* Length of the contour */
	unsigned int n_labels;		/* Number of labels; if 0 we just have a line segment */
	bool annot;			/* true if we want labels */
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
	uint64_t current_seg_no;	/* Number (0->) of current segment in current data file */
	unsigned int current_file_no;	/* Number (0->) of current input data file */
	unsigned int line_type;	/* Kind of line: contour (1) or line (0) */
	unsigned int dist_kind;	/* What kind of distance [0 = xy, 1 = map ] */
	unsigned int dist_unit;	/* Units for labelled distances along tracks [cip] */
	unsigned int proj_type;	/* type of scaling */
	unsigned int L_proj_type;	/* type of scaling for label content only */
	unsigned int half_width;	/* Number of points to use in smoothing the angle [10/2] */
	unsigned int n_cont;		/* Number of labels per segment */
	enum GMT_enum_contline crossing;	/* 1 for crossing simple lines, 2 for file with crossing lines */
	enum GMT_enum_label label_type;	/* 0-7; see above for enums */
	unsigned int nx;		/* Number of crossovers at any time */
	unsigned int f_n;			/* Number of such points */
	unsigned int clearance_flag;	/* 1 if spacing given in % of labelfont size, 0 otherwise */
	unsigned int nudge_flag;	/* 0 if off, 1 if nudging relative to x/y axis, 2 if following local line coordinate system */
	unsigned int box;		/* Textbox bits [1 = outline, 2 = rect box shape, 4 = rounded rect shape] */
	unsigned int n_label;		/* Length of list */
	unsigned int just;		/* Label justification */
	unsigned int end_just[2];	/* Justification for end of lines */
	unsigned int angle_type;	/* 0 = contour-parallel, 1 = contour-normal, 2 = fixed angle */
	unsigned int n_segments;		/* The number of segments */
	unsigned int save_labels;	/* 1 if we wish to save label locations to a text file, 2 if we wish to include label angles [1 = no angles] */
	int number_placement;	/* How the n_cont labels are distributed [-1/0/+1]*/
	int hill_label;		/* -1/+1 = make label readable when looking down/up gradient, 0 = no special treatment  */
	bool annot;			/* true if we want labels */
	bool isolate;		/* true if we have a limit on how close labels may appear (see below) */
	bool spacing;		/* true if we have spacing constraints to apply */
	bool number;			/* true if we have constraints on the number of labels to apply */
	bool do_interpolate;		/* true if we must resample the crossing lines */
	bool fixed;			/* true if we chose fixed positions */
	bool transparent;		/* true for transparent textbox, false for opaque */
	bool curved_text;		/* true for text to follow curved lines */
	bool no_gap;		/* Clip contour or not depends on label placement */
	bool data_col;		/* true if there is data in the zz arrays passed, false if they are NULL */
	bool debug;			/* true of we want to draw helper lines/points */
	bool delay;			/* true of we want to delay the actual annotation plotting until later */
	size_t n_alloc;			/* How many allocated so far */
	char file[GMT_BUFSIZ];		/* File with crossing lines, if specified */
	char option[GMT_BUFSIZ];	/* Copy of the option string */
	char label[GMT_BUFSIZ];		/* Fixed label */
	char label_file[GMT_BUFSIZ];	/* Output files for text dump of label locations */
	char unit[GMT_LEN64];	/* Unit for labels */
	char prefix[GMT_LEN64];	/* prefix for labels */
	char line_name[16];		/* Name of line: contour or line */
	char flag;			/* Char for the option key */
	char **f_label;			/* Array for fixed labels */
	struct GMT_FONT font_label;	/* Which font */
	struct GMT_DATATABLE *xp;		/* Table with list of structures with crossing-line coordinates */
	struct GMT_XSEGMENT *ylist_XP;	/* Sorted y-segments for crossing-lines */
	struct GMT_XSEGMENT *ylist;	/* y-indices sorted in increasing order */
	struct GMT_XOVER XC;		/* Structure with resulting crossovers */
	struct GMT_PEN pen;		/* Pen for drawing textbox outline */
	struct GMT_PEN line_pen;	/* Pen for drawing the contour line */
	struct GMT_LABEL **L;		/* Pointers to sorted list of labels */
	FILE *fp;			/* File pointer for writing labels and positions to text file */
	/* Contour line section */
	
	struct GMT_CONTOUR_LINE **segment;	/* Array of segments */
};

#endif /* _GMT_CONTOUR_H */
