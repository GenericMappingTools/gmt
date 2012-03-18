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
/*
 * Miscellaneous definitions and structures related to color.
 *
 * Author: Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef _GMT_COLORS_H
#define _GMT_COLORS_H

enum GMT_enum_color {GMT_RGB	= 0,
	GMT_CMYK		= 1,
	GMT_HSV			= 2,
	GMT_COLORINT		= 4,
	GMT_NO_COLORNAMES	= 8};

enum GMT_enum_bfn {GMT_BGD, GMT_FGD, GMT_NAN};

/* Copy two RGB[T] arrays (a = b) */
#define GMT_rgb_copy(a,b) memcpy (a, b, (size_t)(4 * sizeof(double)))

/* To compare is two colors are ~ the same */
#define GMT_eq(a,b) (fabs((a)-(b)) < GMT_SMALL)
#define GMT_same_rgb(a,b) (GMT_eq(a[0],b[0]) && GMT_eq(a[1],b[1]) && GMT_eq(a[2],b[2]) && GMT_eq(a[3],b[3]))

/* Macros for conversion of RGB in 0-1 range to 0-255 range */
#define GMT_s255(s) ((s) * 255.0)
#define GMT_t255(t) GMT_q(GMT_s255(t[0])),GMT_q(GMT_s255(t[1])),GMT_q(GMT_s255(t[2]))
#define GMT_u255(s) ((unsigned char)rint(GMT_s255(s)))

/* Macros for conversion of RGB in 0-255 range to 0-1 range */
#define GMT_is255(s) ((s) / 255.0)
#define GMT_it255(t) GMT_is255(t[0]),GMT_is255(t[1]),GMT_is255(t[2])

/* Macro to avoid small numbers in color codes */
#define GMT_q(s) ((s) < 1e-5 ? 0.0 : (s))

/* How B/W TV's convert RGB to Gray */
#define GMT_YIQ(rgb) (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2])

/* Determine if a RGB combination is grayshade */
#define GMT_is_gray(rgb) (GMT_eq(rgb[0],rgb[1]) && GMT_eq(rgb[1],rgb[2]))

/* Determine if a RGB combination is in fact B/W */
#define GMT_is_bw(rgb) (GMT_is_gray(rgb) && (GMT_eq(rgb[0],0.0) || GMT_eq(rgb[0],1.0)))

/* Force component to be in 0 <= s <= 255 range */
#define GMT_0_255_truncate(s) ((s < 0) ? 0 : ((s > 255) ? 255 : s))	/* Truncate to allowable 0-255 range */

/* Here is the definition of the GMT_PALETTE structure that is used in programs
 * that deals with coloring of items as a function of z-lookup.  Note that rgb
 * arrays have 4 items as the 4th value could be a non-zero transparency (when supported).
 */
 
struct GMT_LUT {
	double z_low, z_high, i_dz;
	double rgb_low[4], rgb_high[4], rgb_diff[4];
	double hsv_low[4], hsv_high[4], hsv_diff[4];
	GMT_LONG annot;
	GMT_LONG skip;
	struct GMT_FILL *fill;	/* Use by grdview */			/* Content not counted by sizeof (struct) */
	char *label;		/* For non-number labels */		/* Content not counted by sizeof (struct) */
};

struct GMT_BFN_COLOR {		/* For back-, fore-, and nan-colors */
	double rgb[4];
	double hsv[4];
	GMT_LONG skip;
	struct GMT_FILL *fill;						/* Content not counted by sizeof (struct) */
};

struct GMT_PALETTE {		/* Holds all pen, color, and fill-related parameters */
	GMT_LONG n_headers;		/* Number of CPT file header records (0 if no header) */
	struct GMT_LUT *range;		/* CPT lookup table read by GMT_read_cpt */
	struct GMT_BFN_COLOR patch[3];	/* Structures with back/fore/nan colors */
	GMT_LONG n_colors;		/* Number of colors in CPT lookup table */
	GMT_LONG cpt_flags;		/* Flags controling use of BFN colors */
	GMT_LONG alloc_mode;		/* Allocation info [0] */
	GMT_LONG model;			/* RGB, HSV, CMYK */
	GMT_LONG is_gray;		/* TRUE if only grayshades are needed */
	GMT_LONG is_bw;			/* TRUE if only black and white are needed */
	GMT_LONG is_continuous;		/* TRUE if continuous color tables have been given */
	GMT_LONG has_pattern;		/* TRUE if cpt file contains any patterns */
	GMT_LONG skip;			/* TRUE if current z-slice is to be skipped */
	GMT_LONG categorical;		/* TRUE if CPT applies to categorical data */
	char **header;			/* Array with all CPT ile header records, if any) */		/* Content not counted by sizeof (struct) */
};

#endif /* _GMT_COLORS_H */
