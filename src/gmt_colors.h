/*--------------------------------------------------------------------
 *	$Id: gmt_colors.h,v 1.38 2011-03-03 21:02:50 guru Exp $
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
 
/*
 * Miscellaneous definitions and structures related to color.
 *
 * Author: Paul Wessel
 * Date:	10-AUG-1995
 * Revised:	10-MAY-1998
 *		29-APR-2000 PW: Sped-up calculations of colors
 * Version:	4.1.x
 */

#ifndef _GMT_COLORS_H
#define _GMT_COLORS_H

#define GMT_READ_RGB	1
#define GMT_USE_RGB	2
#define GMT_RGB 	3
#define GMT_READ_HSV	4
#define GMT_USE_HSV	8
#define GMT_HSV		12
#define GMT_READ_CMYK	16
#define GMT_USE_CMYK	32
#define GMT_CMYK	48

#define GMT_BGD 0
#define GMT_FGD 1
#define GMT_NAN 2

/* How B/W TV's convert RGB to Gray */
#define GMT_YIQ(rgb) ((int)irint (0.299 * (rgb[0]) + 0.587 * (rgb[1]) + 0.114 * (rgb[2])))

/* Determine if a RGB combination is grayshade */
#define GMT_is_gray(r,g,b) ((r) == (g) && (r) == (b))

/* Determine if a R(=G=B) gray combination is in fact B/W */
#define GMT_is_bw(r) ((r) == 0 || (r) == 255)

/* Determine if there is any fill at all */
#define GMT_is_nofill(F) (F->rgb[0] == -1 && !F->use_pattern)

/* Here is the definition of the GMT_lut structure that is used in programs
 * that deals with coloring of grids.
 */
 
struct GMT_LUT {
	double z_low, z_high, i_dz;
	int rgb_low[3], rgb_high[3], rgb_diff[3];
	double hsv_low[3], hsv_high[3], hsv_diff[3];
	GMT_LONG annot;
	GMT_LONG skip;
	struct GMT_FILL *fill;	/* Use by grdview */
	char *label;	/* For non-number labels */
};

struct GMT_BFN_COLOR {	/* For back-, fore-, and nan-colors */
	int rgb[3];
	double hsv[3];
	GMT_LONG skip;
	struct GMT_FILL *fill;
};

EXTERN_MSC struct GMT_HASH GMT_rgb_hashnode[GMT_N_COLOR_NAMES];	/* Used to translate colornames to r/g/b */

EXTERN_MSC struct GMT_LUT *GMT_lut;
EXTERN_MSC struct GMT_BFN_COLOR GMT_bfn[3];
EXTERN_MSC GMT_LONG GMT_n_colors;
EXTERN_MSC GMT_LONG GMT_cpt_flags;
EXTERN_MSC GMT_LONG GMT_gray;		/* TRUE if only grayshades are used */
EXTERN_MSC GMT_LONG GMT_b_and_w;		/* TRUE if only black OR white is used */
EXTERN_MSC GMT_LONG GMT_continuous;	/* TRUE if colors change continuously within slice */
EXTERN_MSC GMT_LONG GMT_cpt_pattern;	/* TRUE if cpt file contains any patterns */
EXTERN_MSC GMT_LONG GMT_cpt_skip;	/* TRUE if current z-slice is to be skipped */
#ifdef GMT_CPT2	
EXTERN_MSC GMT_LONG GMT_categorical;	/* TRUE if the CPT applies to categorical data */
#endif
EXTERN_MSC void GMT_sample_cpt (double z[], GMT_LONG nz, GMT_LONG continuous, GMT_LONG reverse, GMT_LONG log_mode);

#endif /* _GMT_COLORS_H */
