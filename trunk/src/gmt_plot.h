/*--------------------------------------------------------------------
 *	$Id: gmt_plot.h,v 1.29 2011-03-03 21:02:50 guru Exp $
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

#ifndef _GMT_PLOT_H
#define _GMT_PLOT_H

/* GMT symbol identifiers */

#define GMT_SYMBOL_NONE		99
#define GMT_SYMBOL_NOT_SET	999

#define GMT_SYMBOL_LINE		0
#define GMT_SYMBOL_BARX		1
#define GMT_SYMBOL_BARY		2
#define GMT_SYMBOL_CROSS	3
#define GMT_SYMBOL_POINT	4
#define GMT_SYMBOL_CIRCLE	5
#define GMT_SYMBOL_SQUARE	6
#define GMT_SYMBOL_TRIANGLE	7
#define GMT_SYMBOL_DIAMOND	8
#define GMT_SYMBOL_STAR		9
#define GMT_SYMBOL_HEXAGON	10
#define GMT_SYMBOL_ITRIANGLE	11
#define GMT_SYMBOL_CUBE		12
#define GMT_SYMBOL_COLUMN	13
#define GMT_SYMBOL_ELLIPSE	14
#define GMT_SYMBOL_VECTOR	15
#define GMT_SYMBOL_VECTOR2	16
#define GMT_SYMBOL_PIE		17
#define GMT_SYMBOL_RECT		18
#define GMT_SYMBOL_XDASH	19
#define GMT_SYMBOL_YDASH	20
#define GMT_SYMBOL_ZDASH	21
#define GMT_SYMBOL_TEXT		22
#define GMT_SYMBOL_PENTAGON	23
#define GMT_SYMBOL_OCTAGON	24
#define GMT_SYMBOL_CUSTOM	25
#define GMT_SYMBOL_ROTATERECT	26
#define GMT_SYMBOL_PLUS		27
#define GMT_SYMBOL_MANGLE	28

#define GMT_SYMBOL_FRONT	-100
#define GMT_SYMBOL_QUOTED_LINE	-200

#define GMT_POINT_SIZE 0.005	/* Size of a "dot" on a GMT PS map */

/* FRONT symbols */

#define GMT_FRONT_FAULT		0
#define GMT_FRONT_TRIANGLE	1
#define GMT_FRONT_SLIP		2
#define GMT_FRONT_CIRCLE	3
#define GMT_FRONT_BOX		4

/* Direction of FRONT symbols: */

#define GMT_FRONT_LEFT		+1
#define GMT_FRONT_CENTERED	0
#define GMT_FRONT_RIGHT		-1

struct GMT_FRONTLINE {		/* A sub-symbol for symbols along a front */
	double f_gap;		/* Gap between front symbols in inches */
	double f_len;		/* Length of front symbols in inches */
	double f_off;		/* Offset of first symbol from start of front in inches */
	GMT_LONG f_sense;	/* Draw symbols to left (+1), centered (0), or right (-1) of line */
	GMT_LONG f_symbol;	/* Which symbol to draw along the front line */
};

struct GMT_SYMBOL {
	GMT_LONG symbol;	/* Symbol id */
	GMT_LONG n_required;	/* Number of additional columns necessary to decode chosen symbol */
	GMT_LONG u;		/* Measure unit id (0 = cm, 1 = inch, 2 = m, 3 = point */
	GMT_LONG u_set;		/* TRUE if a unit was specified */
	double size_x;		/* Current symbol size in x */
	double size_y;		/* Current symbol size in y */
	double size_x2;		/* Half size in x */
	double size_y2;		/* Half size in y */
	double given_size_x;	/* Symbol size read from file or command line */
	double given_size_y;	/* Symbol size read from file or command line */
	GMT_LONG equal_area;	/* Symbol should be scaled to give same area as circle */
	GMT_LONG read_size;	/* TRUE when we must read symbol size from file */
	GMT_LONG shade3D;	/* TRUE when we should simulate shading of 3D symbols cube and column */
	GMT_LONG font_no;	/* font to use for the -Sl symbol */
	GMT_LONG n_nondim;	/* Number of columns that has angles or km (and not dimensions with units) */
	GMT_LONG nondim_col[6];	/* Which columns has angles or km for this symbol */

	/* These apply to bar symbols */

	double base;		/* From what level to draw the bar */
	GMT_LONG user_unit;	/* if TRUE */
	GMT_LONG base_set;	/* TRUE if user provided a custom base [otherwise default to bottom axis */

	/* These apply to vectors */

	GMT_LONG convert_angles;	/* If 2, convert azimuth to angle on map, 1 special case for -JX, 0 plain case */
	GMT_LONG read_vector;	/* if TRUE must read vector attributes */
	GMT_LONG shrink;		/* If TRUE, shrink vector attributes for small lengths */
	double v_norm;		/* shrink when lengths are smaller than this */
	double v_shrink;	/* Required scale factor */
	double v_width;		/* Width of vector stem in inches */
	double h_length;	/* Length of vector head in inches */
	double h_width;		/* Width of vector head in inches */
	GMT_LONG v_just;		/* How to justify vector: head point given (3), head (2), center(1), tail (0 - Default) */
	GMT_LONG v_double_heads;		/* If TRUE, Add 8 (|= 8) to outline to specify double-headed vector (FALSE is single-headed) */

	char string[GMT_TEXT_LEN];	/* Character code to plot (could be octal) */

	struct GMT_FRONTLINE f;	/* parameters needed for a front */
	struct GMT_CUSTOM_SYMBOL *custom;	/* pointer to a custom symbol */

	struct GMT_CONTOUR G;	/* For labelled lines */
};

EXTERN_MSC struct GMT_PS GMT_ps;

EXTERN_MSC void GMT_color_image (double x0, double y0, double x_side, double y_side, unsigned char *image, GMT_LONG nx, GMT_LONG ny, GMT_LONG depth);
EXTERN_MSC void GMT_draw_map_rose (struct GMT_MAP_ROSE *mr);
EXTERN_MSC void GMT_draw_map_scale (struct GMT_MAP_SCALE *ms);
EXTERN_MSC void GMT_echo_command (int argc, char **argv);
EXTERN_MSC void GMT_fill (double x[], double y[], GMT_LONG n, struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_fill_polygon (double *lon, double *lat, double z, GMT_LONG n, struct GMT_FILL *F, GMT_LONG outline);
EXTERN_MSC void GMT_plot_ellipse (double lon, double lat, double z, double major, double minor, double azimuth, struct GMT_FILL fill, GMT_LONG outline);
EXTERN_MSC void GMT_plot_rectangle (double lon, double lat, double z, double width, double height, double azimuth, struct GMT_FILL fill, GMT_LONG outline);
EXTERN_MSC void GMT_draw_fence (double x[], double y[], double z, GMT_LONG n, struct GMT_FRONTLINE *f, struct GMT_FILL *g, GMT_LONG outline);
EXTERN_MSC void GMT_grid_clip_off (void);
EXTERN_MSC void GMT_grid_clip_on (struct GRD_HEADER *h, int rgb[], GMT_LONG flag);
EXTERN_MSC void GMT_linearx_grid (double w, double e, double s, double n, double dval);
EXTERN_MSC void GMT_map_basemap (void);
EXTERN_MSC void GMT_map_clip_off (void);
EXTERN_MSC void GMT_map_clip_on (int rgb[], GMT_LONG flag);
EXTERN_MSC void GMT_plot_line (double *x, double *y, int *pen, GMT_LONG n);
EXTERN_MSC void GMT_setpen (struct GMT_PEN *pen);
EXTERN_MSC void GMT_text3D (double x, double y, double z, double fsize, GMT_LONG fontno, char *text, double angle, GMT_LONG justify, GMT_LONG form);
EXTERN_MSC void GMT_textbox3D (double x, double y, double z, double size, GMT_LONG font, char *label, double angle, GMT_LONG just, GMT_LONG outline, double dx, double dy, int rgb[]);
EXTERN_MSC void GMT_timestamp (double x, double y, GMT_LONG justify, char *U_label);
EXTERN_MSC void GMT_vertical_axis (GMT_LONG mode);
EXTERN_MSC void GMT_xy_axis (double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate);
EXTERN_MSC struct EPS *GMT_epsinfo (char *program);
EXTERN_MSC GMT_LONG *GMT_split_line (double **xx, double **yy, GMT_LONG *nn, GMT_LONG add_crossings);
EXTERN_MSC GMT_LONG GMT_plotend (void);
EXTERN_MSC GMT_LONG GMT_plotinit (int argc, char *argv[]);
EXTERN_MSC void GMT_square (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_circle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_triangle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_itriangle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_diamond (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_hexagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_pentagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_octagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_star (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_plus (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_cross (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_rect (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_ellipse (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_pie (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_rotrect (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_vector (double x0, double y0, double x1, double y1, double z0, double tailwidth, double headlength, double headwidth, double shape, struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_matharc (double x, double y, double z, double size[], double shape, struct GMT_PEN *pen, GMT_LONG status);
EXTERN_MSC char *GMT_export2proj4(char *pStrOut);
#endif /* _GMT_PLOT_H */
