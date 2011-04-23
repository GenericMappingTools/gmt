/*--------------------------------------------------------------------
 *	$Id: gmt_symbol.h,v 1.33 2011-04-23 00:56:08 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 
/*
 * Miscellaneous definitions and structures related to:
 * 1. Compass symbols used by pscbasemap and pscoast
 * 2. Custom symbols used by psxy and psxyz.
 * 3. Definitions for vector attributes
 *
 * Author: Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef _GMT_SYMBOLS_H
#define _GMT_SYMBOLS_H

/* VECTOR attributes are used by psxy, psxyz, psrose, grdvector */
#define VECTOR_LINE_WIDTH	2.0	/* Default vector attributes in points */
#define VECTOR_HEAD_WIDTH	7.0
#define VECTOR_HEAD_LENGTH	9.0

struct GMT_CUSTOM_SYMBOL_ITEM {
	double x, y, p[3], const_val[2];
	GMT_LONG action, conditional, operator, negate, var;
	struct GMT_FILL *fill;
	struct GMT_PEN *pen;
	struct GMT_CUSTOM_SYMBOL_ITEM *next;
	char *string;
};

struct GMT_CUSTOM_SYMBOL {
	char name[GMT_TEXT_LEN64];
	char *PS_macro;		/* Contains all the PS commands if PS is TRUE */
	GMT_LONG n_required;	/* Number of additional columns necessary to decode chosen symbol */
	GMT_LONG PS;		/* TRUE if a PSL symbol */
	struct GMT_CUSTOM_SYMBOL_ITEM *first;
};

struct GMT_MAP_SCALE {	/* Used to plot a map scale in psbasemap and pscoast */
	double lon, lat;	/* Location of top/mid point of scale on the map in lon/lat space */
	double x0, y0;		/* Location of top/mid point of scale on the map in inches x/y */
	double scale_lon;	/* Point where scale should apply */
	double scale_lat;	/* Point where scale should apply */
	double length;		/* How long the scale is in measure units */
	GMT_LONG boxdraw;	/* TRUE if we want to plot a rectangle behind the scale */
	GMT_LONG boxfill;	/* TRUE if we want to paint/fill a rectangle behind the scale */
	GMT_LONG plot;		/* TRUE if we want to draw the scale */
	GMT_LONG fancy;		/* TRUE for a fancy map scale */
	GMT_LONG gave_xy;	/* TRUE if x0, y0 was given in cartesian map coordinates and not lon/lat */
	GMT_LONG unit;		/* TRUE if we should append distance unit to all annotations along the scale */
	GMT_LONG do_label;	/* TRUE if we should plot a label for the scale */
	char measure;		/* The unit, i.e., m (miles), n (nautical miles), or k (kilometers) */
	char justify;		/* Placement of label: t(op), b(ottom), l(eft), r(ight) */
	char label[GMT_TEXT_LEN64];	/* Alternative user-specified label */
	struct GMT_FILL fill;	/* Fill to use for background rectangle */
	struct GMT_PEN pen;	/* Pen to use for background rectangle */
};

struct GMT_MAP_ROSE {	/* Used to plot a map direction "rose" in psbasemap and pscoast */
	double lon, lat;	/* Location of center point of rose on the map in lon/lat space */
	double x0, y0;		/* Location of center point of scale on the map in inches x/y */
	double size;		/* Diameter of the rose in measure units */
	double declination;	/* Magnetic declination if needed */
	double a_int[2];	/* Annotation interval for geographic and magnetic directions */
	double f_int[2];	/* Tick (large) interval for geographic and magnetic directions */
	double g_int[2];	/* Tick (small) interval for geographic and magnetic directions */
	GMT_LONG plot;		/* TRUE if we want to draw the rose */
	GMT_LONG fancy;		/* TRUE for a fancy map rose */
	GMT_LONG gave_xy;	/* TRUE if x0, y0 was given in cartesian map coordinates and not lon/lat */
	GMT_LONG kind;		/* 0 : 90 degrees, 1 : 45 degrees, 2 : 22.5 degrees between points */
	char label[4][GMT_TEXT_LEN64];	/* User-changable labels for W, E, S, N point */
	char dlabel[GMT_TEXT_LEN256];	/* Magnetic declination label */
};

#endif	/* _GMT_SYMBOLS_H */
