/*--------------------------------------------------------------------
 *	$Id: gmt_colors.h,v 1.11 2004-04-07 20:15:45 pwessel Exp $
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
 
/*
 * Miscellaneous definitions and structures related to color.
 *
 * Author: Paul Wessel
 * Date:	10-AUG-1995
 * Revised:	10-MAY-1998
 *		29-APR-2000 PW: Sped-up calculations of colors
 * Version:	3.4
 */

#ifndef _GMT_COLORS_H
#define _GMT_COLORS_H

#define GMT_RGB 0
#define GMT_HSV 1
#define GMT_CMYK 2

#define GMT_BGD 0
#define GMT_FGD 1
#define GMT_NAN 2

#define GMT_N_COLOR_NAMES 663

/* How B/W TV's convert RGB to Gray */
#define YIQ(rgb) irint (0.299 * (rgb[0]) + 0.587 * (rgb[1]) + 0.114 * (rgb[2]))

/* Determine if a RGB combination is grayshade */
#define GMT_is_gray(r,g,b) ((r) == (g) && (r) == (b))

/* Determine if a R(=G=B) gray combination is in fact B/W */
#define GMT_is_bw(r) ((r) == 0 || (r) == 255)

/* Here is the definition of the GMT_lut structure that is used in programs
 * that deals with coloring of grids.
 */
 
struct GMT_LUT {
	double z_low, z_high, i_dz;
	int rgb_low[3], rgb_high[3], rgb_diff[3];
	int annot;
	BOOLEAN skip;
	struct GMT_FILL *fill;	/* Use by grdview */
};

struct GMT_BFN_COLOR {	/* For back-, fore-, and nan-colors */
	int rgb[3];
	int skip;
	struct GMT_FILL *fill;
};

EXTERN_MSC char *GMT_color_name[GMT_N_COLOR_NAMES];		/* Names of X11 color names */
EXTERN_MSC int   GMT_color_rgb[GMT_N_COLOR_NAMES][3];		/* Corresponding r/g/b values */
EXTERN_MSC struct GMT_HASH GMT_rgb_hashnode[GMT_N_COLOR_NAMES];	/* Used to translate colornames to r/g/b */

EXTERN_MSC struct GMT_LUT *GMT_lut;
EXTERN_MSC struct GMT_BFN_COLOR GMT_bfn[3];
EXTERN_MSC int GMT_n_colors;
EXTERN_MSC BOOLEAN GMT_gray;		/* TRUE if only grayshades are used */
EXTERN_MSC BOOLEAN GMT_b_and_w;		/* TRUE if only black OR white is used */
EXTERN_MSC BOOLEAN GMT_continuous;	/* TRUE if colors change continuously within slice */
EXTERN_MSC BOOLEAN GMT_cpt_pattern;	/* TRUE if cpt file contains any patterns */
EXTERN_MSC BOOLEAN GMT_cpt_skip;	/* TRUE if current z-slice is to be skipped */

EXTERN_MSC void GMT_sample_cpt (double z[], int nz, BOOLEAN continuous, BOOLEAN reverse, int log_mode);

#endif /* _GMT_COLORS_H */
