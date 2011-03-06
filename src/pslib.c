/*--------------------------------------------------------------------
 *	$Id: pslib.c,v 1.239 2011-03-06 01:43:05 guru Exp $
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
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * pslib is a library of plot functions that create PostScript.
 * All the routines write their output to the same plotting file,
 * which can be dumped to a Postscript output device (laserwriters).
 * pslib can handle and mix text, line-drawings, and bit-map graphics
 * in both black/white and color.
 *
 * pslib conforms to the Encapsulated PostScript Files Specification V 3.0,
 * and pslib documents have successfully been used to make Encapsulated
 * PostScript files on a Macintosh.
 *
 * C considerations:
 *	All floating point data are assumed to be of type double.
 *	All integer data are assumed to be of type int.
 *	All logical data are assumed to be of type int (1 = TRUE, 0 = FALSE).
 *
 * Updated May, 1992 by J. Goff, WHOI to include FORTRAN interfaces
 * Updated July, 1993 by P. Wessel to include more symbols and binary image output
 * Updated August, 1993 by P. Wessel to only warn about path length problems.
 * Updated August, 1998 by P. Wessel for GMT3.1 release.
 * Updated November, 1998 by P. Wessel to add paper Media support (ps_plotinit changed arguments).
 * Updated March 26, 1999 by P. Wessel to add ps_imagemask routine.
 * Updated April 07, 1999 by P. Wessel to allow DOS delimiters and drive letters in pattern names.
 * Updated May 04, 1999 by P. Wessel to add ps_words: typesetting of paragraphs.
 * Updated June 17, 1999 by P. Wessel to remove all references to GMT functions and add ps_memory for internal use.
 * Updated June 16, 2000 by P. Wessel to add more encoded special characters.
 * Updated July 5, 2000 by P. Wessel to ensure that implementation limit on string length in images is not exceeded.
 * Updated April 24, 2001 by P. Wessel to ensure setpagedevice is only used with PS Level 2 or higher.
 * Updated January 4, 2002 by P. Wessel to make all font size variables double instead of int.
 * Updated December 22, 2003 by P. Wessel to add pentagon symbol.
 * Updated January 12, 2004 by P. Wessel to add octagon symbol.
 * Updated June 2, 2004 by P. Wessel to add contour/line clipping & labeling machinery (PSL_label.ps).
 * Updated October 25, 2004 by R. Scharroo and L. Parkes to add image compression tricks.
 * Updated March 6, 2006 by P. Wessel to skip output of PS comments unless PSL->comments is TRUE.
 * Updated May 18, 2007 by P. Wessel to allow @;, @:, and @_ also for ps_text.
 *
 * FORTRAN considerations:
 *	All floating point data are assumed to be DOUBLE PRECISION
 *	All integer data are assumed to be INTEGER, i.e. INTEGER*4
 *	All LOGICAL/int data are assumed to be of type INTEGER*4 (1 = TRUE, 0 = FALSE).
 *
 *	When passing (from FORTRAN to C) a fixed-length character variable which has
 *	blanks at the end, append '\0' (null character) after the last non-blank
 *	character.  This is so that C will know where the character string ends.
 *	It is NOT sufficient to pass, for example, "string(1:string_length)".

 *
 * List of functions:
 *	ps_arc			: Draws a circular arc
 *	ps_axis			: Plots an axis with tickmarks and annotation/label
 *	ps_bitimage		: Plots a 1-bit image or imagemask
 *	ps_circle		: Plots circle and [optionally] fills it
 *	ps_clipoff		: Restores previous clipping path
 *	ps_clipon		: Clips plot outside the specified polygon
 *	ps_colorimage		: Plots a 24-bit 2-D image using the colorimage operator
 *	ps_colortiles		: Plots a 24-bit 2-D image using tiling
 *	ps_command		: Writes a given PostScript statement to the plot file
 *	ps_comment		: Writes a comment statement to the plot file
 *	ps_cross		: Plots a cross (x)
 *	ps_dash			: Plots a short horizontal line segment (dash)
 *	ps_diamond		: Plots a diamond and [optionally] fills it
 *	ps_ellipse		: Plots an ellipse and [optionally] fills it
 *	ps_encode_font		: Reencode a font with a different encoding vector
 *	ps_epsimage		: Inserts EPS image
 *	ps_flush		: Flushes the output buffer
 *	ps_hexagon		: Plots a hexagon and {optionally] fills it
 *	ps_image		: (deprecated: use ps_colorimage)
 *	ps_itriangle		: Plots an inverted triangle and [optionally] fills it
 *	ps_line			: Plots a line
 *	ps_load_eps		: Read EPS 'image'
 *	ps_load_image		: Read image file of of supported type
 *	ps_load_raster		: Read image from a Sun rasterfile
 *	ps_octagon		: Plots an octagon and {optionally] fills it
 *	ps_patch		: Special case of ps_polygon:  Short polygons only (< 20 points, no path-shortening)
 *	ps_pentagon		: Plots a pentagon and {optionally] fills it
 *	ps_pie			: Plots a sector of a circle and [optionally] fills it
 *	ps_plot			: Absolute move to new position (pen up or down)
 *	ps_plotend		: Close plotfile
 *	ps_plotinit		: Initialize parameters/open plotfile etc.
 *	ps_plotr		: Relative move to a new position (pen up or down)
 *      ps_plus			: Plots a plus (+)
 *	ps_polygon		: Creates a polygon and optionally fills it
 *	ps_read_rasheader	: Portable reading of Sun rasterfile headers
 *	ps_write_rasheader	: Portable writing of Sun rasterfile headers
 *	ps_rect			: Draws a rectangle and [optionally] fills it
 *	ps_rotatetrans		: Rotates, then translates the coordinate system
 *	ps_setdash		: Specify pattern for dashed line
 *	ps_setfont		: Changes current font
 *	ps_setformat		: Changes # of decimals used in color and gray specs [3]
 *	ps_setline		: Sets linewidth
 *	ps_setpaint		: Sets the current r/g/b for fill
 *	ps_square		: Plots square and [optionally] shades it
 *	ps_star			: Plots a star and {optionally] fills it
 *	ps_text			: Plots textstring
 *	ps_textbox		: Draw a filled box around a textstring
 *	ps_textclip		: Place clippaths to protect areas where labels will print
 *	ps_textpath		: --"-- for curved text following lines - also places labels
 *	ps_transrotate		: Translates and rotates the coordinate system
 *	ps_triangle		: Plots a triangle and [optionally] fills it
 *	ps_vector		: Draws an vector as specified
 *	ps_words		: Plots a text paragraph
 *
 *
 *
 * For information about usage, syntax etc, see the pslib.l manual pages
 *
 * Author:	Paul Wessel, Dept. of Geology and Geophysics
 *		School of Ocean and Earth Science and Technology
 *		1680 East-West Road, Honolulu, HI 96822
 *		pwessel@hawaii.edu
 * Date:	20-MAR-2008
 * Version:	4.3 [64-bit enabled edition]
 *
 */

/*  PSL is POSIX COMPLIANT  */

#define _POSIX_SOURCE 1

/*--------------------------------------------------------------------
 *			SYSTEM HEADER FILES
 *--------------------------------------------------------------------*/

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#ifdef __MACHTEN__
/* Kludge to fix a Machten POSIX bug */
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pslib.h"
#include "gmt_notunix.h"
#include "gmt_math.h"

#ifndef WIN32
#include <unistd.h>
#endif

/*--------------------------------------------------------------------
 *		     STANDARD CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif
#ifndef R2D
#define R2D (180.0/M_PI)
#endif
#ifndef D2R
#define D2R (M_PI/180.0)
#endif
#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif
#define VNULL		((void *)NULL)
#ifndef CNULL
#define CNULL (char *)NULL
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef irint
#define irint(x) ((int)rint(x))
#endif

/*--------------------------------------------------------------------
 *			PSL CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#define PSL_Version		"4.2"
#define PSL_SMALL		1.0e-10
#define PSL_MAX_L1_PATH		1000 	/* Max path length in Level 1 implementations */
#define PSL_INV_255		(1.0 / 255.0)
#define PSL_N_PATTERNS		91	/* Current number of predefined patterns + 1, # 91 is user-supplied */
#define PSL_PAGE_HEIGHT_IN_PTS	842	/* A4 height */
#define PSL_RGB			0
#define PSL_CMYK		1
#define PSL_HSV			2
#define PSL_GRAY		3
#define PS_LANGUAGE_LEVEL	2

/*--------------------------------------------------------------------
 *			PSL FUNCTION MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#define PSL_YIQ(rgb) irint (0.299 * (rgb[0]) + 0.587 * (rgb[1]) + 0.114 * (rgb[2]))	/* How B/W TV's convert RGB to Gray */
#define PSL_iscolor(rgb) (rgb[0] != rgb[1] || rgb[1] != rgb[2])

#if defined(__LP64__)
#define PSL_abs(n) labs(n)
#elif defined(_WIN64)
#define PSL_abs(n) _abs64(n)
#else
#define PSL_abs(n) abs(n)
#endif

/*--------------------------------------------------------------------
 *			PSL PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

/* Single, global structure used internally by pslib */

struct PSL {
	struct INIT {	/* Parameters set by user via ps_plotinit() */
		char *file;			/* Name of output file (NULL means stdout)	*/
		char *encoding;			/* The encoding name. e.g. ISO-8859-1		*/
		PSL_LONG overlay;		/* TRUE skips writing the PS header section	*/
		PSL_LONG mode;			/* 32 bit-flags, used as follows:
			bit 0 : 0 = Landscape, 1 = Portrait,
			bit 1 : 0 = be silent, 1 = be verbose
			bit 2 : 0 = bin image, 1 = hex image
			bit 3 : 0 = rel positions, 1 = abs positions
			bit 9-10 : 0 = RGB color, 1 = CMYK color, 2 = HSV color
			bits 12-13 : 0 = no compression, 1 = RLE compression, 2 = LZW compression
			bits 14-15 : (0,1,2) sets the line cap setting
			bits 16-17 : (0,1,2) sets the line miter setting
			bits 18-25 : (8 bits) sets the miter limit
			bit 31 : 0 = write no comments, 1 = write PS comments to PS file	*/
		PSL_LONG unit;			/* 0 = cm, 1 = inch, 2 = meter			*/
		PSL_LONG copies;			/* Number of copies for this plot		*/
		int page_rgb[3];		/* RGB color for background paper [white]	*/
		double page_size[2];		/* Width and height of paper used in points	*/
		PSL_LONG dpi;			/* Selected dots per inch			*/
		double magnify[2];		/* Global scale values [1/1]			*/
		double origin[2];		/* Origin offset [1/1]				*/
		struct EPS *eps;		/* structure with Document info			*/
	} init;
	struct CURRENT {	/* Variables and settings that changes via ps_* calls */
		char texture[512];		/* Current setdash pattern			*/
		char bw_format[8];		/* Format used for grayshade value		*/
		char rgb_format[64];		/* Same, for RGB color triplets			*/
		char hsv_format[64];		/* Same, for HSV color triplets	(HSB in PS)	*/
		char cmyk_format[64];		/* Same, for CMYK color quadruples		*/
		PSL_LONG font_no;		/* Current font number				*/
		PSL_LONG linewidth;		/* Current pen thickness			*/
		int rgb[3];			/* Current paint				*/
		int fill_rgb[3];		/* Current fill					*/
		PSL_LONG outline;		/* Current outline				*/
		PSL_LONG offset;			/* Current setdash offset			*/
	} current;
	struct INTERNAL {	/* Variables used internally only */
		char *SHAREDIR;			/* Pointer to path of directory with pslib subdirectory */
		char *USERDIR;			/* Pointer to path of directory with user definitions (~/.gmt) */
		char *user_image[PSL_N_PATTERNS];	/* Name of user patterns		*/
		PSL_LONG verbose;		/* TRUE for verbose output, FALSE remains quiet	*/
		PSL_LONG comments;		/* TRUE for writing comments to output, FALSE strips all comments */
		PSL_LONG landscape;		/* TRUE = Landscape, FALSE = Portrait		*/
		PSL_LONG text_init;		/* TRUE after PSL_text.ps has been loaded	*/
		PSL_LONG ascii;			/* TRUE writes images in ascii, FALSE uses binary	*/
		PSL_LONG absolute;		/* TRUE will reset origin, FALSE means relative position	*/
		PSL_LONG eps_format;		/* TRUE makes EPS file, FALSE means PS file	*/
		PSL_LONG N_FONTS;		/* Total no of fonts;  To add more, modify the file CUSTOM_font_info.d */
		PSL_LONG compress;		/* Compresses images with (1) RLE or (2) LZW or (0) None */
		PSL_LONG color_mode;		/* 0 = rgb, 1 = cmyk, 2 = hsv (only 1-2 for images)	*/
		PSL_LONG line_cap;		/* 0, 1, or 2 for butt, round, or square [butt] */
		PSL_LONG line_join;		/* 0, 1, or 2 for miter, arc, or bevel [miter] */
		PSL_LONG miter_limit;		/* Acute angle threshold 0-180; 0 means PS default [0] */
		double bb[4];			/* Boundingbox arguments			*/
		PSL_LONG ix, iy;			/* Absolute coordinates of last point		*/
		double p_width;			/* Paper width in points, set in plotinit();	*/
		double p_height;		/* Paper height in points, set in plotinit();	*/
		PSL_LONG length;			/* Image row output byte counter		*/
		PSL_LONG n_userimages;		/* Number of specified custom patterns		*/
		double scale;			/* Must be set through plotinit();		*/
		double points_pr_unit;		/* # of points pr measure unit (e.g., 72/inch	*/
		FILE *fp;			/* PS output file pointer. NULL = stdout	*/
		struct PSL_FONT {
			char *name;		/* Name of this font */
			double height;		/* Height of A for unit fontsize */
			PSL_LONG encoded;	/* TRUE if we never should reencode this font (e.g. symbols) */
						/* This is also changed to TRUE after we do reencode a font */
		} *font;	/* Pointer to array of font structures 		*/
		struct PSL_PATTERN {
			PSL_LONG nx, ny;
			PSL_LONG status, depth, dpi;
			int f_rgb[3], b_rgb[3];
		} pattern[PSL_N_PATTERNS*2];
	} internal;
} *PSL;

/* Special macros and structure for ps_words */

#define NO_SPACE	0
#define ONE_SPACE	1
#define COMPOSITE_1	8
#define COMPOSITE_2	16
#define SYMBOL		12
#define PSL_CHUNK	2048

struct GMT_WORD {
	PSL_LONG font_no;
	int rgb[3];
	PSL_LONG flag;
	double font_size;
	double baseshift;
	char *txt;
};

/* Special macros and structure for color(sic) maPSL-> */

#define INDEX_BITS 8	/* PostScript indices may be 12 bit */
			/* But we only do 8 bits for now. */
#define MAX_COLORS (1<<INDEX_BITS)

typedef struct
{
	PSL_LONG ncolors;
	unsigned char colors[MAX_COLORS][3];
} *colormap_t;

typedef struct
{
	unsigned char *buffer;
	colormap_t colormap;
} *indexed_image_t;

typedef struct {
	PSL_LONG nbytes;
	int depth;
	unsigned char *buffer;
} *byte_stream_t;

/* Define support functions called inside pslib functions */

char *ps_prepare_text (char *text);
void def_font_encoding (void);
void init_font_encoding (struct EPS *eps);
void get_uppercase(char *new, char *old);
void ps_rle_decode (struct imageinfo *h, unsigned char **in);
unsigned char *ps_cmyk_encode (PSL_LONG *nbytes, unsigned char *input);
unsigned char *ps_gray_encode (PSL_LONG *nbytes, unsigned char *input);
unsigned char *ps_rle_encode (PSL_LONG *nbytes, unsigned char *input);
unsigned char *ps_lzw_encode (PSL_LONG *nbytes, unsigned char *input);
byte_stream_t ps_lzw_putcode (byte_stream_t stream, short int incode);
void ps_stream_dump (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG depth, PSL_LONG compress, PSL_LONG encode, PSL_LONG mask);
void ps_a85_encode (unsigned char quad[], PSL_LONG nbytes);
PSL_LONG ps_shorten_path (double *x, double *y, PSL_LONG n, PSL_LONG *ix, PSL_LONG *iy);
int ps_comp_int_asc (const void *p1, const void *p2);
int ps_comp_long_asc (const void *p1, const void *p2);
static void ps_bulkcopy (const char *fname, const char *version);
static void ps_init_fonts (PSL_LONG *n_fonts, PSL_LONG *n_GMT_fonts);
PSL_LONG ps_pattern_init(PSL_LONG image_no, char *imagefile);
void ps_rgb_to_cmyk_char (unsigned char rgb[], unsigned char cmyk[]);
void ps_rgb_to_cmyk_int (int rgb[], int cmyk[]);
void ps_rgb_to_cmyk (int rgb[], double cmyk[]);
void ps_rgb_to_hsv (int rgb[], double hsv[]);
void ps_cmyk_to_rgb (int rgb[], double cmyk[]);
void ps_place_color (int rgb[]);
void ps_place_setdash (char *pattern, PSL_LONG offset);
void ps_set_length_array (char *param, double *array, PSL_LONG n);
PSL_LONG ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, PSL_LONG *node, PSL_LONG n, PSL_LONG m);
void ps_set_txt_array (char *param, char *array[], PSL_LONG n);
void ps_set_integer (char *param, PSL_LONG value);
void ps_set_real_array (char *param, double *array, PSL_LONG n);
indexed_image_t ps_makecolormap (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
PSL_LONG ps_bitreduce (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG ncolors);
PSL_LONG ps_bitimage_cmap (int f_rgb[], int b_rgb[]);
void ps_colorimage_rgb (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
void ps_colorimage_cmap (double x, double y, double xsize, double ysize, indexed_image_t image, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
unsigned char *ps_load_raster (FILE *fp, struct imageinfo *header);
unsigned char *ps_load_eps (FILE *fp, struct imageinfo *header);
PSL_LONG ps_get_boundingbox (FILE *fp, PSL_LONG *llx, PSL_LONG *lly, PSL_LONG *trx, PSL_LONG *try_);
char *ps_getsharepath (const char *subdir, const char *stem, const char *suffix, char *path);
PSL_LONG ps_pattern (PSL_LONG image_no, char *imagefile, PSL_LONG invert, PSL_LONG image_dpi, PSL_LONG outline, int f_rgb[], int b_rgb[]);
void get_origin (double xt, double yt, double xr, double yr, double r, double *xo, double *yo, double *b1, double *b2);

#ifdef GMT_QSORT
/* Need to replace OS X's qsort with one that works for 64-bit data */
void GMT_qsort(void *a, size_t n, size_t es, int (*cmp) (const void *, const void *));
#endif

/*------------------- PUBLIC PSLIB FUNCTIONS--------------------- */


void ps_arc (double x, double y, double radius, double az1, double az2, PSL_LONG status)
{	/* 1 = set anchor, 2 = set end, 3 = both */
	PSL_LONG ix, iy, ir;

	ix = (PSL_LONG)irint (x * PSL->internal.scale);
	iy = (PSL_LONG)irint (y * PSL->internal.scale);
	ir = (PSL_LONG)irint (radius * PSL->internal.scale);
	if (fabs (az1 - az2) > 360.0) az1 = 0.0, az2 = 360.0;
	if (status%2)	/* Beginning of new segment */
		fprintf (PSL->internal.fp, "S ");
	if (az1 < az2)	/* Forward positive arc */
		fprintf (PSL->internal.fp, "%ld %ld %ld %g %g arc", ix ,iy, ir, az1, az2);
	else	/* Negative arc */
		fprintf (PSL->internal.fp, "%ld %ld %ld %g %g arcn", ix ,iy, ir, az1, az2);
	if (status > 1)	fprintf (PSL->internal.fp, " S");
	fprintf (PSL->internal.fp, "\n");
}

/* fortran interface */
void ps_arc_ (double *x, double *y, double *radius, double *az1, double *az2, PSL_LONG *status)
{
	 ps_arc (*x, *y, *radius, *az1, *az2, *status);
}

void ps_axis (double x, double y, double length, double val0, double val1, double annotation_int, char *label, double annotpointsize, PSL_LONG side)
{
	PSL_LONG annot_justify, label_justify, i, j, ndig = 0;
	PSL_LONG left = FALSE;
	double angle, dy, scl, val, annot_off, label_off, xx, sign;
	char text[256], format[256];

	if (annotation_int < 0.0) left = TRUE;
	annotation_int = fabs (annotation_int);
	sprintf (text, "%g", annotation_int);
	for (i = 0; text[i] && text[i] != '.'; i++);
	if (text[i]) {	/* Found a decimal point */
		for (j = i + 1; text[j]; j++);
		ndig = j - i - 1;
	}
	if (ndig > 0)
		sprintf (format, "%%.%ldf", ndig);
	else
		strcpy (format, "%g");

	angle = (side%2) ? 90.0 : 0.0;
	sign = (side < 2) ? -1.0 : 1.0;
	annot_justify = label_justify = (side < 2) ? -10 : -2;
	dy = sign * annotpointsize / PSL->internal.points_pr_unit;

	fprintf (PSL->internal.fp, "\nV %g %g T %g R\n", x * PSL->internal.scale, y * PSL->internal.scale, angle);
	ps_segment (0.0, 0.0, length, 0.0);
	if ((val1 - val0) == 0.0) {
		fprintf (stderr, "pslib: ERROR: Axis val0 == val1!\n");
		return;
	}
	scl = length / (val1 - val0);
	annot_off = dy;
	label_off = 2.5 * dy;
	dy *= 0.5;

	i = 0;
	val = val0;
	while (val <= (val1+PSL_SMALL)) {
		i++;
		xx = (val - val0) * scl;
		if (left) xx = length - xx;
		ps_segment (xx, 0.0, xx, dy);
		sprintf( text, format, val);
		ps_text (xx, annot_off, annotpointsize, text, 0.0, annot_justify, 0);
		val = val0 + i * annotation_int;
	}
	ps_text (0.5*length, label_off, annotpointsize*1.5, label, 0.0, label_justify, 0);
	fprintf (PSL->internal.fp, "U\n\n");
}

/* fortran interface */
void ps_axis_ (double *x, double *y, double *length, double *val0, double *val1, double *annotation_int, char *label, double *annotpointsize, PSL_LONG *side, int nlen)
{
	ps_axis (*x, *y, *length, *val0, *val1, *annotation_int, label, *annotpointsize, *side);
}

void ps_bitimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG invert, int f_rgb[], int b_rgb[])
{
	/* Plots a 1-bit image or imagemask.
	 * x,y:		Position of image (in inches)
	 * xsize,ysize:	Size of image (in inches)
	 * buffer:	Image bit buffer
	 * nx,ny:	Size of image (in pixels)
	 * invert:	If TRUE: invert bits (0<->1)
	 * f_rgb:	Foreground color for 1 bits (if f_rgb[0] < 0, make transparent)
	 * b_rgb:	Background color for 0 bits (if b_rgb[0] < 0, make transparent)
	 */
	PSL_LONG lx, ly;
	PSL_LONG inv;
	char *kind[2] = {"Binary", "Ascii"};

	lx = (PSL_LONG)irint (xsize * PSL->internal.scale);
	ly = (PSL_LONG)irint (ysize * PSL->internal.scale);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe 1-bit image\n", kind[PSL->internal.ascii]);
	fprintf (PSL->internal.fp, "V N %g %g T %ld %ld scale", x * PSL->internal.scale, y * PSL->internal.scale, lx, ly);
	inv = (ps_bitimage_cmap (f_rgb, b_rgb) + invert) % 2;
	fprintf (PSL->internal.fp, "\n<< /ImageType 1 /Decode [%ld %ld] ", inv, 1-inv);
	ps_stream_dump (buffer, nx, ny, 1, PSL->internal.compress, PSL->internal.ascii, (int)(f_rgb[0] < 0 || b_rgb[0] < 0));

	fprintf (PSL->internal.fp, "U\n");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of %s Abobe 1-bit image\n", kind[PSL->internal.ascii]);
}

/* fortran interface */
void ps_bitimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, PSL_LONG *nx, PSL_LONG *ny, PSL_LONG *invert, int *f_rgb, int *b_rgb)
{
	ps_bitimage (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *invert, f_rgb, b_rgb);
}

void ps_circle (double x, double y, double size, int rgb[], PSL_LONG outline)
{
	/* size is assumed to be diameter */

	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SC\n", (PSL_LONG)irint (0.5 * size * PSL->internal.scale), (PSL_LONG)irint (x * PSL->internal.scale), (PSL_LONG)irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_circle_ (double *x, double *y, double *size, int *rgb, PSL_LONG *outline)
{
	 ps_circle (*x, *y, *size, rgb, *outline);
}

void ps_clipoff (void)
{
	/* Return to original clipping path */
	fprintf (PSL->internal.fp, "S U\n");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Clipping is currently OFF\n");
	PSL->current.rgb[0] = PSL->current.rgb[1] = PSL->current.rgb[2] = -1;	/* Reset to -1 so ps_setpaint will update the current paint */
	PSL->current.linewidth = -1;			/* Reset to -1 so ps_setline will update the current width */
	PSL->current.offset = -1;			/* Reset to -1 so ps_setdash will update the current pattern */
}

/* fortran interface */
void ps_clipoff_ (void) {
	ps_clipoff ();
}

void ps_clipon (double *x, double *y, PSL_LONG n, int rgb[], PSL_LONG flag)
{
	/* Any plotting outside the path defined by x,y will be clipped.
	 * use ps_clipoff to restore the original clipping path.
	 * n    : number of x,y pairs (i.e. path length)
	 * rgb  : optional paint (use rgb[0] < 0 to avoid paint)
	 * flag : 1 = start new clipping path (more follows)
	 *        2 = end clipping path (this is the last segment)
	 *        3 = this is the complete clipping path (start to end)
	 */

	char move[7];

	if (flag & 1) {	/* First segment in (possibly multi-segmented) clip-path */
		strcpy (move, "M");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of clip path\n");
		fprintf (PSL->internal.fp, "S V\n");
	}
	else
		strcpy (move, "m");

	if (n > 0) {
		PSL->internal.ix = (PSL_LONG)irint (x[0]*PSL->internal.scale);
		PSL->internal.iy = (PSL_LONG)irint (y[0]*PSL->internal.scale);
		fprintf (PSL->internal.fp, "%ld %ld %s\n", PSL->internal.ix, PSL->internal.iy, move);
		ps_line (&x[1], &y[1], n-1, 0, FALSE);	/* Must pass close = FALSE since first point not given ! */
	}

	if (flag & 2) {	/* End path and [optionally] fill */
		if (rgb[0] >= 0) {	/* fill is desired */
			fprintf (PSL->internal.fp, "V ");
			ps_place_color (rgb);
			fprintf (PSL->internal.fp, " eofill U ");
		}
		if (flag & 4)
			fprintf (PSL->internal.fp, "eoclip\n");
		else
			fprintf (PSL->internal.fp, "eoclip N\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of clip path.  Clipping is currently ON\n");
	}
}

/* fortran interface */
void ps_clipon_ (double *x, double *y, PSL_LONG *n, int *rgb, PSL_LONG *flag)
{
	ps_clipon (x, y, *n, rgb, *flag);
}

void ps_colorimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits)
{
	/* Plots a 24-bit color image in Grayscale, RGB or CMYK mode.
	 * When the number of unique colors does not exceed MAX_COLORS, the routine will index
	 * 24-bit RGB images and then attempt to reduce the depth of the indexed image to 1, 2 or 4 bits.
	 *
	 * x, y		: lower left position of image in inches
	 * xsize, ysize	: image size in inches
	 * buffer	: contains the bytes for the image
	 * nx, ny	: pixel dimension
	 * nbits	: number of bits per pixel (1, 2, 4, 8, 24)
	 *
	 * Special cases:
	 * nx < 0	: 8- or 24-bit image contains a color mask (first 1 or 3 bytes)
	 * nbits < 0	: "Hardware" interpolation requested
	 */
	PSL_LONG llx, lly, urx, ury;
	PSL_LONG id, it;
	char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	char *kind[2] = {"Binary", "Ascii"};				/* What encoding to use */
	char *type[3] = {"1", "4 /MaskColor[0]", "1 /Interpolate true"};
	indexed_image_t image;

	/* Convert lower left and upper right coordinates to integers.
	   This ensures that the image is located in the same place as a box drawn with the same coordinates. */
	llx = (PSL_LONG)irint (x * PSL->internal.scale);
	lly = (PSL_LONG)irint (y * PSL->internal.scale);
	urx = (PSL_LONG)irint ((x + xsize) * PSL->internal.scale);
	ury = (PSL_LONG)irint ((y + ysize) * PSL->internal.scale);

	/* Gray scale, CMYK or RGB encoding/colorspace */
	id = (PSL->internal.color_mode == PSL_GRAY || PSL_abs (nbits) < 24) ? 0 : (PSL->internal.color_mode == PSL_CMYK ? 2 : 1);
	/* Colormask or interpolate */
	it = nx < 0 ? 1 : (nbits < 0 ? 2 : 0);

	if (PSL->internal.color_mode != PSL_GRAY && (image = ps_makecolormap (buffer, nx, ny, nbits))) {
		/* Creation of colormap was successful */
		nbits = ps_bitreduce (image->buffer, nx, ny, image->colormap->ncolors);

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe Indexed %s image [%ld bit]\n", kind[PSL->internal.ascii], colorspace[id], nbits);
		fprintf (PSL->internal.fp, "V N %ld %ld T %ld %ld scale [/Indexed /Device%s %ld <\n", llx, lly, urx-llx, ury-lly, colorspace[id], image->colormap->ncolors - 1);
		ps_stream_dump (&image->colormap->colors[0][0], image->colormap->ncolors, 1, 24, 0, 2, 2);
		fprintf (PSL->internal.fp, ">] setcolorspace\n<< /ImageType %s /Decode [0 %d] ", type[it], (1<<nbits)-1);
		ps_stream_dump (image->buffer, nx, ny, nbits, PSL->internal.compress, PSL->internal.ascii, 0);
		fprintf (PSL->internal.fp, "U\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of %s Adobe Indexed %s image\n", kind[PSL->internal.ascii], colorspace[id]);

		/* Clear the newly created image buffer and colormap */
		ps_free (image->buffer);
		ps_free (image->colormap);
		ps_free (image);
	}
	else {
		/* Export full gray scale, RGB or CMYK image */
		nbits = PSL_abs (nbits);

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe %s image [%ld bit]\n", kind[PSL->internal.ascii], colorspace[id], nbits);
		fprintf (PSL->internal.fp, "V N %ld %ld T %ld %ld scale /Device%s setcolorspace", llx, lly, urx-llx, ury-lly, colorspace[id]);

		if (it == 1 && nbits == 24) {	/* Do PS Level 3 image type 4 with colormask */
			fprintf (PSL->internal.fp, "\n<< /ImageType 4 /MaskColor[%d %d %d]", (int)buffer[0], (int)buffer[1], (int)buffer[2]);
			buffer += 3;
		}
		else if (it == 1 && nbits == 8) {	/* Do PS Level 3 image type 4 with colormask */
			fprintf (PSL->internal.fp, "\n<< /ImageType 4 /MaskColor[%d]", (int)buffer[0]);
			buffer++;
		}
		else		/* Do PS Level 2 image, optionally with interpolation */
			fprintf (PSL->internal.fp, "\n<< /ImageType %s", type[it]);

		fprintf (PSL->internal.fp, " /Decode [%s] ", decode[id]);
		ps_stream_dump (buffer, nx, ny, nbits, PSL->internal.compress, PSL->internal.ascii, 0);
		fprintf (PSL->internal.fp, "U\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of %s Adobe %s image\n", kind[PSL->internal.ascii], colorspace[id]);
	}
}

/* fortran interface */
void ps_colorimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, PSL_LONG *nx, PSL_LONG *ny, PSL_LONG *nbits, int nlen)
{
	ps_colorimage (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *nbits);
}

void ps_colortiles (double x0, double y0, double xsize, double ysize, unsigned char *image, PSL_LONG nx, PSL_LONG ny)
{
	/* Plots image as many colored rectangles
	 * x0, y0	: Lower left corner in inches
	 * xsize, ysize	: Size of image in inches
	 * image	: color image with rgb triplets per pixel
	 * nx, ny	: image size in pixels
	 */
	PSL_LONG i, j, k;
	int rgb[3];
	double x1, x2, y1, y2, dx, dy, noise, noise2;

	nx = PSL_abs (nx);
	noise = 2.0 / PSL->internal.scale;
	noise2 = 2.0 * noise;
	dx = xsize / nx;
	dy = ysize / ny;

	ps_transrotate (x0, y0, 0.0);
	y2 = ny * dy + 0.5 * noise;
	for (j = k = 0; j < ny; j++) {
		y1 = (ny - j - 1) * dy - 0.5 * noise;
		x1 = -noise;
		for (i = 0; i < nx; i++) {
			x2 = (i + 1) * dx + noise;
			rgb[0] = image[k++];
			rgb[1] = image[k++];
			rgb[2] = image[k++];
			ps_rect (x1, y1, x2, y2, rgb, FALSE);
			x1 = x2 - noise2;
		}
		y2 = y1 + noise2;
	}
	ps_rotatetrans (-x0, -y0, 0.0);
}

/* fortran interface */
void ps_colortiles_ (double *x0, double *y0, double *xsize, double *ysize, unsigned char *image, PSL_LONG *nx, PSL_LONG *ny, int nlen)
{
	 ps_colortiles (*x0, *y0, *xsize, *ysize, image, *nx, *ny);
}

void ps_command (char *text)
{
	fprintf (PSL->internal.fp, "%s\n", text);
}

/* fortran interface */
void ps_command_ (char *text, int nlen)
{
	ps_command (text);
}

void ps_comment (char *text)
{
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%%\n%% %s\n%%\n", text);
}

/* fortran interface */
void ps_comment_ (char *text, int nlen)
{
	ps_comment (text);
}

void ps_matharc (double x, double y, double radius, double az1, double az2, double shape, PSL_LONG status)
{	/* 1 = add arrowhead at az1, 2 = add arrowhead at az2, 3 = at both, 0 no arrows */
	PSL_LONG ix, iy, i;
	double p, arc_length, half_width, da, xt, yt, s, c, xr, yr, xl, yl, xo, yo;
	double angle[2], A, bo1, bo2, xi, yi, bi1, bi2, xv, yv, sign[2] = {+1.0, -1.0};

	ix = (PSL_LONG)irint (x * PSL->internal.scale);
	iy = (PSL_LONG)irint (y * PSL->internal.scale);
	fprintf (PSL->internal.fp, "V %ld %ld T\n", ix ,iy);
	angle[0] = az1;	angle[1] = az2;
	p = (double)(PSL->current.linewidth) / PSL->init.dpi;	/* Line width in inches */
	arc_length = 8.0 * p;
	half_width = 2.5 * p;
	da = arc_length * 180.0 / (M_PI * radius);	/* Angle corresponding to the arc length */

	for (i = 0; i < 2; i++) {
		if (status & (i+1)) {	/* Add arrow head at this angle */
			ps_setfill (PSL->current.rgb, FALSE);
			A = D2R * angle[i];
			s = sin (A);	c = cos (A);
			xt = radius * c;	yt = radius * s;
			A = D2R * (angle[i] + sign[i] * da);
			s = sin (A);	c = cos (A);
			xr = (radius + half_width) * c;	yr = (radius + half_width) * s;
			xl = (radius - half_width) * c;	yl = (radius - half_width) * s;
			get_origin (xt, yt, xr, yr, radius, &xo, &yo, &bo1, &bo2);
			ps_arc (xo, yo, radius, bo1, bo2, 1);		/* Draw the arrow arc from tip to outside flank */
			get_origin (xt, yt, xl, yl, radius, &xi, &yi, &bi1, &bi2);
			ps_arc (xi, yi, radius, bi2, bi1, 0);		/* Draw the arrow arc from tip to outside flank */
			A = D2R * (angle[i]+sign[i]*da*(1.0-0.5*shape));
			s = sin (A);	c = cos (A);
			xv = radius * c - xl;	yv = radius * s - yl;
			ps_plotr (xv, yv, PSL_PEN_DRAW);
			ps_command ("P fs os");
			angle[i] += 0.5 * sign[i] * da;
		}
	}
	ps_arc (0.0, 0.0, radius, angle[0], angle[1], 3);		/* Draw the (possibly shortened) arc */
	fprintf (PSL->internal.fp, "U \n");
}

void get_origin (double xt, double yt, double xr, double yr, double r, double *xo, double *yo, double *b1, double *b2)
{ /* finds origin so that distance is r to the two points given */
	double a0, b0, c0, A, B, C, q, sx1, sx2, sy1, sy2, r1, r2;

	a0 = (xt - xr) / (yr - yt);
	b0 = 0.5 * (xr*xr + yr*yr - xt*xt - yt*yt)/(yr - yt);
	c0 = b0 - yt;
	A = 1 + a0*a0;
	B = 2*(c0*a0 - xt);
	C = xt*xt - r*r + c0*c0;
	q = sqrt (B*B - 4*A*C);
	sx1 = 0.5* (-B + q)/A;
	sx2 = 0.5* (-B - q)/A;
	sy1 = b0 + a0 * sx1;
	sy2 = b0 + a0 * sx2;

	r1 = hypot (sx1, sy1);
	r2 = hypot (sx2, sy2);
	if (r1 < r) {
	    *xo = sx1;
	    *yo = sy1;
	}
	else {
	    *xo = sx2;
	    *yo = sy2;
	}
	*b1 = R2D * atan2 (yr - *yo, xr - *xo);
	*b2 = R2D * atan2 (yt - *yo, xt - *xo);
}

void ps_plus (double x, double y, double diameter)
{	/* Draw plus sign using current color. Fit inside circle of given diameter. */
	fprintf (PSL->internal.fp, "%ld %ld %ld x\n", (PSL_LONG) irint (0.5 * diameter * PSL->internal.scale), (PSL_LONG) irint (x * PSL->internal.scale), (PSL_LONG) irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_plus_ (double *x, double *y, double *diameter)
{
	ps_plus (*x, *y, *diameter);
}

void ps_cross (double x, double y, double diameter)
{	/* Draw cross sign using current color. Fit inside circle of given diameter. */
	fprintf (PSL->internal.fp, "%ld %ld %ld X\n", (PSL_LONG) irint (0.5 * diameter * PSL->internal.scale), (PSL_LONG) irint (x * PSL->internal.scale), (PSL_LONG) irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_cross_ (double *x, double *y, double *diameter)
{
	ps_cross (*x, *y, *diameter);
}

void ps_point (double x, double y, double diameter)
{     /* Fit inside circle of given diameter; draw using current color */
	fprintf (PSL->internal.fp, "%ld %ld %ld O\n", (PSL_LONG)irint (0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint (x * PSL->internal.scale), (PSL_LONG)irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_point_ (double *x, double *y, double *diameter)
{
	ps_point (*x, *y, *diameter);
}

void ps_diamond (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* diameter is diameter of circumscribing circle */

	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SD\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_diamond_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	 ps_diamond (*x, *y, *diameter, rgb, *outline);
}

void ps_segment (double x0, double y0, double x1, double y1)
{	/* Short line segment */
	PSL_LONG ix, iy;

	ix = (PSL_LONG)irint (x0 * PSL->internal.scale);
	iy = (PSL_LONG)irint (y0 * PSL->internal.scale);
	PSL->internal.ix = (PSL_LONG)irint (x1 * PSL->internal.scale);
	PSL->internal.iy = (PSL_LONG)irint (y1 * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld %ld M %ld %ld D S\n", ix, iy, PSL->internal.ix - ix, PSL->internal.iy - iy);
}

/* fortran interface */
void ps_segment_ (double *x0, double *y0, double *x1, double *y1)
{
	 ps_segment (*x0, *y0, *x1, *y1);
}

void ps_setfill (int rgb[], PSL_LONG outline)
{
	if (rgb[0] == -2)
		{ /* Skipped, no fill specified */ }
	else if (PSL->current.fill_rgb[0] == rgb[0] && PSL->current.fill_rgb[1] == rgb[1] && PSL->current.fill_rgb[2] == rgb[2])
		{ /* Skipped, fill already set */ }
	else {
		if (rgb[0] == -1)
			ps_command ("FQ");
		else {
			fprintf (PSL->internal.fp, "{");
			ps_place_color (rgb);
			fprintf (PSL->internal.fp, "} FS\n");
		}
	}

	if (outline == -1) outline = 0;
	if (outline >= 0 && PSL->current.outline != outline) fprintf (PSL->internal.fp, "O%ld\n", outline);

	PSL->current.fill_rgb[0] = rgb[0];
	PSL->current.fill_rgb[1] = rgb[1];
	PSL->current.fill_rgb[2] = rgb[2];
	PSL->current.outline = outline;
}

/* fortran interface */
void ps_setfill_ (int *rgb, PSL_LONG *outline)
{
	 ps_setfill (rgb, *outline);
}

void ps_star (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* Fit inside circle of given diameter */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SA\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_star_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	 ps_star (*x, *y, *diameter, rgb, *outline);
}

void ps_square (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* give diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SS\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_square_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	ps_square (*x, *y, *diameter, rgb, *outline);
}

void ps_triangle (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* Give diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld ST\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_triangle_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	ps_triangle (*x, *y, *diameter, rgb, *outline);
}

void ps_itriangle (double x, double y, double diameter, int rgb[], PSL_LONG outline)	/* Inverted triangle */
{	/* Give diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SI\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_itriangle_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	ps_itriangle (*x, *y, *diameter, rgb, *outline);
}

void ps_hexagon (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* diameter is diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SH\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_hexagon_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	 ps_hexagon (*x, *y, *diameter, rgb, *outline);
}

void ps_pentagon (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* diameter is diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SN\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_pentagon_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	 ps_pentagon (*x, *y, *diameter, rgb, *outline);
}

void ps_octagon (double x, double y, double diameter, int rgb[], PSL_LONG outline)
{	/* diameter is diameter of circumscribing circle */
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %ld SO\n", (PSL_LONG)irint(0.5 * diameter * PSL->internal.scale), (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_octagon_ (double *x, double *y, double *diameter, int *rgb, PSL_LONG *outline)
{
	 ps_octagon (*x, *y, *diameter, rgb, *outline);
}

void ps_pie (double x, double y, double radius, double az1, double az2, int rgb[], PSL_LONG outline)
{
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %g %g %ld %ld SW\n", (PSL_LONG)irint (radius * PSL->internal.scale), az1, az2, (PSL_LONG)irint (x * PSL->internal.scale), (PSL_LONG)irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_pie_ (double *x, double *y, double *radius, double *az1, double *az2, int *rgb, PSL_LONG *outline)
{
	 ps_pie (*x, *y, *radius, *az1, *az2, rgb, *outline);
}

void ps_ellipse (double x, double y, double angle, double major, double minor, int rgb[], PSL_LONG outline)
{
	/* Feature: Pen thickness also affected by aspect ratio */

	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %g %ld %ld SE\n", (PSL_LONG)irint (0.5 * major * PSL->internal.scale), (PSL_LONG)irint (0.5 * minor * PSL->internal.scale), angle, (PSL_LONG)irint (x * PSL->internal.scale), (PSL_LONG)irint (y * PSL->internal.scale));
}

/* fortran interface */
void ps_ellipse_ (double *x, double *y, double *angle, double *major, double *minor, int *rgb, PSL_LONG *outline)
{
	 ps_ellipse (*x, *y, *angle, *major, *minor, rgb, *outline);
}

void ps_flush ()
{
	/* Simply flushes the output buffer */
	fflush (PSL->internal.fp);
}

/* fortran interface */
void ps_flush_ ()
{
	ps_flush();
}

void ps_image (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits)
{	/* Backwards compatibility */
	ps_colorimage (x, y, xsize, ysize, buffer, nx, ny, nbits);
}

/* fortran interface */
void ps_image_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, PSL_LONG *nx, PSL_LONG *ny, PSL_LONG *nbits, int nlen)
{
	ps_image (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *nbits);
}

void ps_pattern_cleanup (void) {
	PSL_LONG image_no;

	for (image_no = 0; image_no < PSL_N_PATTERNS * 2; image_no++) {
		if (PSL->internal.pattern[image_no].status) {
			fprintf (PSL->internal.fp, "currentdict /image%ld undef\n", image_no);
			fprintf (PSL->internal.fp, "currentdict /pattern%ld undef\n", image_no);
		}
	}
}

/* fortran interface */
PSL_LONG ps_pattern_ (PSL_LONG *image_no, char *imagefile, PSL_LONG *invert, PSL_LONG *image_dpi, PSL_LONG *outline, int *f_rgb, int *b_rgb, int nlen)
{
	 return (ps_pattern (*image_no, imagefile, *invert, *image_dpi, *outline, f_rgb, b_rgb));
}

PSL_LONG ps_pattern (PSL_LONG image_no, char *imagefile, PSL_LONG invert, PSL_LONG image_dpi, PSL_LONG outline, int f_rgb[], int b_rgb[])
{
	/* Set up pattern fill, either by using image number or imagefile name
	 * image_no:	Number of the standard GMT fill pattern (use negative when file name used instead)
	 * imagefile:	Name of image file
	 * invert:	If TRUE exchange set and unset pixels (1-bit only)
	 * image_dpi:	Resolution of image on the page
	 * outline:	TRUE will draw outline, -1 means clippath already in place
	 * f_rgb:	Foreground color used for set bits (1) (1-bit only)
	 * b_rgb:	Background color used for unset bits (0) (1-bit only)
	 * Returns image number
	 */

	PSL_LONG found;
	PSL_LONG i, id, inv, refresh;
	PSL_LONG nx, ny;
	char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	char *name;

	/* Determine if image was used before */

	if ((image_no >= 0 && image_no < PSL_N_PATTERNS) && !PSL->internal.pattern[image_no].status)	/* Unused predefined */
		image_no = ps_pattern_init (image_no, imagefile);
	else if (image_no < 0) {	/* User image, check if already used */
		for (i = 0, found = FALSE; !found && i < PSL->internal.n_userimages; i++) found = !strcmp (PSL->internal.user_image[i], imagefile);
		if (!found)	/* Not found or no previous user images loaded */
			image_no = ps_pattern_init (image_no, imagefile);
		else
			image_no = PSL_N_PATTERNS + i - 1;
	}
	nx = PSL->internal.pattern[image_no].nx;
	ny = PSL->internal.pattern[image_no].ny;

	id = (PSL->internal.color_mode == PSL_CMYK) ? 2 : 1;
	name = (PSL->internal.pattern[image_no].depth == 1 && (f_rgb[0] < 0 || b_rgb[0] < 0)) ? "imagemask" : "image";

	/* When DPI or colors have changed, the /pattern procedure needs to be rewritten */

	refresh = 0;
	if (PSL->internal.pattern[image_no].dpi != image_dpi) refresh++;
	for (i = 0; !refresh && i < 3; i++) {
		if (invert) {
			if (PSL->internal.pattern[image_no].f_rgb[i] != b_rgb[i]) refresh++;
			if (PSL->internal.pattern[image_no].b_rgb[i] != f_rgb[i]) refresh++;
		}
		else {
			if (PSL->internal.pattern[image_no].f_rgb[i] != f_rgb[i]) refresh++;
			if (PSL->internal.pattern[image_no].b_rgb[i] != b_rgb[i]) refresh++;
		}
	}

	if (refresh) {

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Setup %s fill using pattern %ld\n", name, image_no);
		if (image_dpi) {	/* Use given DPI */
			nx = (PSL_LONG)irint (nx * PSL->internal.scale / image_dpi);
			ny = (PSL_LONG)irint (ny * PSL->internal.scale / image_dpi);
		}
		fprintf (PSL->internal.fp, "/pattern%ld {V %ld %ld scale", image_no, nx, ny);
		fprintf (PSL->internal.fp, "\n<< /PaintType 1 /PatternType 1 /TilingType 1 /BBox [0 0 1 1] /XStep 1 /YStep 1 /PaintProc\n   {begin");

		if (PSL->internal.pattern[image_no].depth == 1) {	/* 1-bit bitmap basis */
			inv = (ps_bitimage_cmap (f_rgb, b_rgb) + invert) % 2;
			fprintf (PSL->internal.fp, "\n<< /ImageType 1 /Decode [%ld %ld]", inv, 1-inv);
		}
		else
			fprintf (PSL->internal.fp, " /Device%s setcolorspace\n<< /ImageType 1 /Decode [%s]", colorspace[id], decode[id]);
		fprintf (PSL->internal.fp, " /Width %ld /Height %ld /BitsPerComponent %ld", PSL->internal.pattern[image_no].nx, PSL->internal.pattern[image_no].ny, MIN(PSL->internal.pattern[image_no].depth,8));
		fprintf (PSL->internal.fp, "\n   /ImageMatrix [%ld 0 0 %ld 0 %ld] /DataSource image%ld\n>> %s end}\n>> matrix makepattern U} def\n", PSL->internal.pattern[image_no].nx, -PSL->internal.pattern[image_no].ny, PSL->internal.pattern[image_no].ny, image_no, name);

		PSL->internal.pattern[image_no].dpi = image_dpi;
		for (i = 0; i < 3; i++) {
			PSL->internal.pattern[image_no].f_rgb[i] = (invert) ? b_rgb[i] : f_rgb[i];
			PSL->internal.pattern[image_no].b_rgb[i] = (invert) ? f_rgb[i] : b_rgb[i];
		}
	}

	return (image_no);
}

PSL_LONG ps_pattern_init (PSL_LONG image_no, char *imagefile)
{
	PSL_LONG i;
	char name[BUFSIZ], file[BUFSIZ];
	unsigned char *picture;
	struct imageinfo h;
	PSL_LONG found;

	if ((image_no >= 0 && image_no < PSL_N_PATTERNS) && PSL->internal.pattern[image_no].status) return (image_no);	/* Already done this */

	if ((image_no >= 0 && image_no < PSL_N_PATTERNS)) {	/* Premade pattern yet not used */
		sprintf (name, "ps_pattern_%2.2ld", image_no);
		ps_getsharepath ("pattern", name, ".ras", file);
	}
	else {	/* User image, check to see if already used */

		for (i = 0, found = FALSE; !found && i < PSL->internal.n_userimages; i++) found = !strcmp (PSL->internal.user_image[i], imagefile);
		if (found) return (PSL_N_PATTERNS + i - 1);
		ps_getsharepath (CNULL, imagefile, "", file);
		PSL->internal.user_image[PSL->internal.n_userimages] = (char *) ps_memory (VNULL, (size_t)(strlen (imagefile)+1), sizeof (char));
		strcpy (PSL->internal.user_image[PSL->internal.n_userimages], imagefile);
		image_no = PSL_N_PATTERNS + PSL->internal.n_userimages;
		PSL->internal.n_userimages++;
	}

	/* Load image file. Store size, depth and bogus DPI setting */

	picture = ps_load_image (file, &h);

	PSL->internal.pattern[image_no].status = 1;
	PSL->internal.pattern[image_no].nx = h.width;
	PSL->internal.pattern[image_no].ny = h.height;
	PSL->internal.pattern[image_no].depth = h.depth;
	PSL->internal.pattern[image_no].dpi = -999;

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%%\n%% Define pattern %ld\n%%\n", image_no);

	fprintf (PSL->internal.fp, "/image%ld {<~\n", image_no);
	ps_stream_dump (picture, h.width, h.height, h.depth, PSL->internal.compress, 1, 2);
	fprintf (PSL->internal.fp, "} def\n");

	ps_free ((void *)picture);

	return (image_no);
}

/* fortran interface */
void ps_epsimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, PSL_LONG size, PSL_LONG *nx, PSL_LONG *ny, PSL_LONG *ox, PSL_LONG *oy, int nlen)
{
	ps_epsimage (*x, *y, *xsize, *ysize, buffer, size, *nx, *ny, *ox, *oy);
}

void ps_epsimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG size, PSL_LONG nx, PSL_LONG ny, PSL_LONG ox, PSL_LONG oy)
{
	/* Plots an EPS image
	 * x,y:		Position of image (in inches)
	 * xsize,ysize:	Size of image (in inches)
	 * buffer:	EPS file (buffered)
	 * size:	Number of bytes in buffer
	 * nx,ny:	Size of image (in pixels)
	 * ox,oy:	Coordinates of lower left corner (in pixels)
	 */
	int unused = 0;
	fprintf (PSL->internal.fp, "PSL_eps_begin\n");
	fprintf (PSL->internal.fp, "%g %g T %g %g scale\n", x * PSL->internal.scale, y * PSL->internal.scale, xsize * PSL->internal.scale / nx, ysize * PSL->internal.scale / ny);
	fprintf (PSL->internal.fp, "%ld %ld T\n", -ox, -oy);
	fprintf (PSL->internal.fp, "N %ld %ld m %ld %ld L %ld %ld L %ld %ld L P clip N\n", ox, oy, ox+nx, oy, ox+nx, oy+ny, ox, oy+ny);
	fprintf (PSL->internal.fp, "%%%%BeginDocument: psimage.eps\n");
	unused = (int)fwrite (buffer, (size_t)1, (size_t)size, PSL->internal.fp);
	fprintf (PSL->internal.fp, "%%%%EndDocument\n");
	fprintf (PSL->internal.fp, "PSL_eps_end\n");
}

PSL_LONG ps_line (double *x, double *y, PSL_LONG n, PSL_LONG type, PSL_LONG close)
{
	/* type:  1 means new anchor point, 2 means stroke line, 3 = both */
	/* close: TRUE if a closed polygon */
	PSL_LONG i, *ix, *iy;
	PSL_LONG trim = FALSE;
	char move = 'M';

	/* First remove unnecessary points that have zero curvature */

	ix = (PSL_LONG *) ps_memory (VNULL, (size_t)n, sizeof (PSL_LONG));
	iy = (PSL_LONG *) ps_memory (VNULL, (size_t)n, sizeof (PSL_LONG));

	if ((n = ps_shorten_path (x, y, n, ix, iy)) < 2) {
		ps_free ((void *)ix);
		ps_free ((void *)iy);
		return (0);
	}

	if (close && ix[0] == ix[n-1] && iy[0] == iy[n-1]) {
		trim = TRUE;
		n--;
	}

	if (type < 0) {	/* Do not stroke before moveto */
		type = -type;
		move = 'm';
	}

	if (type%2) {
		fprintf (PSL->internal.fp, "%ld %ld %c\n", ix[0], iy[0], move);
	}
	else if (ix[0] != PSL->internal.ix || iy[0] != PSL->internal.iy)
		fprintf (PSL->internal.fp, "%ld %ld D\n", ix[0] - PSL->internal.ix, iy[0] - PSL->internal.iy);
	PSL->internal.ix = ix[0];
	PSL->internal.iy = iy[0];

	for (i = 1; i < n; i++) {
		if (ix[i] != PSL->internal.ix || iy[i] != PSL->internal.iy) fprintf (PSL->internal.fp, "%ld %ld D\n", ix[i] - PSL->internal.ix, iy[i] - PSL->internal.iy);
		PSL->internal.ix = ix[i];
		PSL->internal.iy = iy[i];
	}
	if (type > 1) {
		if (close)
			fprintf (PSL->internal.fp, "P S\n");	/* Close and stroke the path */
		else
			fprintf (PSL->internal.fp, "S\n");	/* Stroke the path */
	}
	else if (close)
		fprintf (PSL->internal.fp, "P\n");		/* Close the path */

	ps_free ((void *)ix);
	ps_free ((void *)iy);

	return (n);
}

/* fortran interface */
void ps_line_ (double *x, double *y, PSL_LONG *n, PSL_LONG *type, PSL_LONG *close)
{
	ps_line (x, y, *n, *type, *close);
}

PSL_LONG ps_shorten_path (double *x, double *y, PSL_LONG n, PSL_LONG *ix, PSL_LONG *iy)
{
	/* Simplifies the (x,y) array by converting it to pixel coordinates (ix,iy)
	 * and eliminating repeating points and intermediate points along straight
	 * line segments.  The result is the fewest points needed to draw the path
	 * and still look exactly like the original path. */

	double old_slope = 1.0e200, new_slope;
	PSL_LONG i, k, dx, dy;
	PSL_LONG old_dir = 0, new_dir;
	/* These seeds for old_slope and old_dir make sure that first point gets saved */

	if (n < 2) return (n);	/* Not a path to start with */

	for (i = 0; i < n; i++) {	/* Convert all coordinates to integers at current scale */
		ix[i] = (PSL_LONG)irint (x[i] * PSL->internal.scale);
		iy[i] = (PSL_LONG)irint (y[i] * PSL->internal.scale);
	}

	/* The only truly unique point is the starting point; all else must show increments
	 * relative to the previous point */
	
	/* First point is the anchor. We will find at least one point, unless all points are the same */
	for (i = k = 0; i < n-1; i++) {
		dx = ix[i+1] - ix[i];
		dy = iy[i+1] - iy[i];
		if (dx == 0 && dy == 0) continue;	/* Skip duplicates */
		new_slope = (dx == 0) ? copysign (1.0e100, (double)dy) : ((double)dy) / ((double)dx);
		new_dir = (dx >= 0) ? 1 : -1;
		if (new_slope != old_slope || new_dir != old_dir) {
			ix[k] = ix[i];
			iy[k] = iy[i];
			k++;
			old_slope = new_slope;
			old_dir = new_dir;
		}
	}

	/* If all points are the same, we get here with k = 0, so we can exit here now with 1 point */
	if (k < 1) return ((PSL_LONG)1);

	/* Last point (k cannot be < 1 so k-1 >= 0) */
	if (x[k-1] != ix[n-1] || iy[k-1] != iy[n-1]) {	/* Do not do slope check on last point since we must end there */
		ix[k] = ix[n-1];
		iy[k] = iy[n-1];
		k++;
	}
	return (k);
}

/* fortran interface */
void ps_shorten_path_ (double *x, double *y, PSL_LONG *n, PSL_LONG *ix, PSL_LONG *iy)
{
	ps_shorten_path (x, y, *n, ix, iy);
}

void ps_plot (double x, double y, int pen)
{
	PSL_LONG ix, iy, idx, idy;

	/* Convert user coordinates to dots */
	ix = (PSL_LONG)irint (x * PSL->internal.scale);
	iy = (PSL_LONG)irint (y * PSL->internal.scale);

	/* Absolute move or absolute draw converted to relative */
	idx = ix - PSL->internal.ix;
	idy = iy - PSL->internal.iy;

	if (pen == PSL_PEN_DRAW_AND_STROKE) {
		/* Always draw-stroke even when displacement is 0 */
		fprintf (PSL->internal.fp, "%ld %ld D S\n", idx, idy);
	}
	else if (pen == PSL_PEN_MOVE) {
		/* Do this always, even if idx = idy = 0, just to be sure we are where we are supposed to be */
		fprintf (PSL->internal.fp, "%ld %ld M\n", ix, iy);
	}
	else if (idx == 0 && idy == 0)
		return;
	else {
		/* Convert to relative draw to have smaller numbers */
		fprintf (PSL->internal.fp, "%ld %ld D\n", idx, idy);
	}
	PSL->internal.ix = ix;	/* Update absolute position */
	PSL->internal.iy = iy;
}

/* fortran interface */
void ps_plot_ (double *x, double *y, int *pen)
{
	ps_plot (*x, *y, *pen);
}

void ps_plotend (PSL_LONG lastpage)
{
	PSL_LONG i;

	ps_pattern_cleanup ();
	ps_setdash (CNULL, 0);

	if (lastpage) {
		if (!PSL->internal.eps_format)
			fprintf (PSL->internal.fp, "%%%%PageTrailer\n");
		else {
			double x0, y0, x1, y1;
			x0 = MAX (PSL->init.magnify[0] * PSL->internal.bb[0], 0.0);
			y0 = MAX (PSL->init.magnify[1] * PSL->internal.bb[1], 0.0);
			x1 = PSL->init.magnify[0] * PSL->internal.bb[2];
			y1 = PSL->init.magnify[1] * PSL->internal.bb[3];
			fprintf (PSL->internal.fp, "%%%%Trailer\n");
			fprintf (PSL->internal.fp, "%%%%BoundingBox: %d %d %d %d\n", (int)floor(x0), (int)floor(y0), (int)ceil(x1), (int)ceil(y1));
			fprintf (PSL->internal.fp, "%%%%HiResBoundingBox: %g %g %g %g\n", x0, y0, x1, y1);
		}
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Reset translations and scale and call showpage\n");
		fprintf (PSL->internal.fp, "S %g %g T", -(PSL->init.origin[0] * PSL->internal.scale), -(PSL->init.origin[1] * PSL->internal.scale));
		fprintf (PSL->internal.fp, " %g %g scale",
			PSL->internal.scale/(PSL->internal.points_pr_unit * PSL->init.magnify[0]), PSL->internal.scale/(PSL->internal.points_pr_unit * PSL->init.magnify[1]));
		if (PSL->internal.landscape) fprintf (PSL->internal.fp, " -90 R %g 0 T", -PSL->internal.p_width);
		fprintf (PSL->internal.fp, " 0 A\nshowpage\n");
		if (!PSL->internal.eps_format) fprintf (PSL->internal.fp, "\n%%%%Trailer\n");
		fprintf (PSL->internal.fp, "\nend\n");
		if (!PSL->internal.eps_format) fprintf (PSL->internal.fp, "%%%%EOF\n");
	}
	else if (PSL->internal.absolute)
		fprintf (PSL->internal.fp, "S %g %g T 0 A\n", -(PSL->init.origin[0] * PSL->internal.scale), -(PSL->init.origin[1] * PSL->internal.scale));
	else
		fprintf (PSL->internal.fp, "S 0 A\n");
	if (PSL->internal.fp != stdout) fclose (PSL->internal.fp);

	/* Free up memory used by the PSL control structure */

	for (i = 0; i < PSL->internal.N_FONTS; i++) if (PSL->internal.font[i].name) ps_free ((void *)PSL->internal.font[i].name);
	ps_free ((void *)PSL->internal.font);
	for (i = 0; i < PSL->internal.n_userimages; i++) if (PSL->internal.user_image[i]) ps_free (PSL->internal.user_image[i]);
	if (PSL->init.file) ps_free ((void *)PSL->init.file);
	if (PSL->init.encoding) ps_free ((void *)PSL->init.encoding);
	if (PSL->init.eps->name) ps_free ((void *)PSL->init.eps->name);
	if (PSL->init.eps->title) ps_free ((void *)PSL->init.eps->title);
	if (PSL->init.eps) ps_free ((void *)PSL->init.eps);
	if (PSL->internal.SHAREDIR) ps_free ((void *)PSL->internal.SHAREDIR);
	if (PSL->internal.USERDIR) ps_free ((void *)PSL->internal.USERDIR);
	ps_free ((void *)PSL);
}

/* fortran interface */
void ps_plotend_ (PSL_LONG *lastpage)
{
	ps_plotend (*lastpage);
}

PSL_LONG ps_plotinit_hires (char *plotfile, PSL_LONG overlay, PSL_LONG mode, double xoff, double yoff, double xscl, double yscl, PSL_LONG ncopies, PSL_LONG dpi, PSL_LONG unit, double *page_size, int *rgb, const char *encoding, struct EPS *eps)
/* plotfile:	Name of output file or NULL for standard output
   xoff, yoff:	Sets a new origin relative to old
   xscl, yscl:	Global scaling, usually left to 1,1
   page_size:	Physical width and height of paper used in points
   overlay:	FALSE means print headers and macros first
   mode:	     bit 0 : 0 = Landscape, 1 = Portrait,
		     bit 1 : 0 = be silent, 1 = be verbose
		     bit 2 : 0 = bin image, 1 = hex image
		     bit 3 : 0 = rel positions, 1 = abs positions
		  bit 9-10 : 0 = RGB color, 1 = CMYK color, 2 = HSV color, 3 = Gray scale
		bits 12-13 : 0 = no compression, 1 = RLE compression, 2 = LZW compression
		bits 14-15 : (0,1,2) sets the line cap setting
		bits 16-17 : (0,1,2) sets the line miter setting
		bits 18-25 : (8 bits) sets the miter limit
		    bit 31 : 0 = write no comments, 1 = write PS comments to PS file
   ncopies:	Number of copies for this plot
   dpi:		Plotter resolution in dots-per-inch
   unit:	0 = cm, 1 = inch, 2 = meter
   rgb:		array with Color of page (paper)
   encoding:	Font encoding used
   eps:		structure with Document info.  !! Fortran version (ps_plotinit_) does not have this argument !!
*/
{
	PSL_LONG i, manual = FALSE, n_GMT_fonts;
	int no_rgb[3] = {-1, -1, -1};
	time_t right_now;
	char openmode[2], *this;
	double scl;

	/* Allocate PSL control structure */

	if ((PSL = (struct PSL *) ps_memory (VNULL, 1L, sizeof (struct PSL))) == NULL) {
		fprintf (stderr, "PSL Fatal Error: Could not allocate PSL control structure!\n");
		PS_exit (EXIT_FAILURE);
	}

	/* Save original initialization settings */

	PSL->init.file = (plotfile == NULL || plotfile[0] == 0) ? NULL : strdup (plotfile);
	PSL->init.encoding = strdup (encoding);
	PSL->init.overlay = overlay;
	PSL->init.mode = mode;
	PSL->init.unit = unit;
	PSL->init.dpi = dpi;
	memcpy ((void *)PSL->init.page_rgb, (void *)rgb, 3*sizeof(int));
	memcpy ((void *)PSL->init.page_size, (void *)page_size, 2*sizeof(double));
	PSL->init.origin[0] = xoff;	PSL->init.origin[1] = yoff;
	PSL->init.magnify[0] = xscl;	PSL->init.magnify[1] = yscl;
	/* Duplicate entire contents of EPS structure - to be freed by ps_plotend() */
	PSL->init.eps = (struct EPS *) ps_memory (VNULL, 1L, sizeof (struct EPS));
	if (eps) {	/* Copy over user's settings */
		memcpy ((void *)PSL->init.eps, (void *)eps,sizeof(struct EPS));
		PSL->init.eps->name = (char *) ps_memory (VNULL, (size_t)(strlen (eps->name) + 1), sizeof (char));
		strcpy (PSL->init.eps->name, eps->name);
		PSL->init.eps->title = (char *) ps_memory (VNULL, (size_t)(strlen (eps->title) + 1), sizeof (char));
		strcpy (PSL->init.eps->title, eps->title);
	}
		
	/* Determine SHAREDIR (directory containing pslib and pattern subdirectories) */

	if ((this = getenv ("GMT_SHAREDIR")) != CNULL) {	/* GMT_SHAREDIR was set */
		PSL->internal.SHAREDIR = (char *) ps_memory (VNULL, (size_t)(strlen (this) + 1), sizeof (char));
		strcpy (PSL->internal.SHAREDIR, this);
	}
	else {	/* Default is GMT_SHARE_PATH */
		PSL->internal.SHAREDIR = (char *) ps_memory (VNULL, (size_t)(strlen (GMT_SHARE_PATH) + 1), sizeof (char));
		strcpy (PSL->internal.SHAREDIR, GMT_SHARE_PATH);
	}

	/* Determine USERDIR (directory containing user replacements contents in SHAREDIR) */

	if ((this = getenv ("GMT_USERDIR")) != CNULL) {	/* GMT_USERDIR was set */
		PSL->internal.USERDIR = (char *) ps_memory (VNULL, (size_t)(strlen (this) + 1), sizeof (char));
		strcpy (PSL->internal.USERDIR, this);
	}
	else if ((this = getenv ("HOME")) != CNULL) {	/* HOME was set: use HOME/.gmt */
		PSL->internal.USERDIR = (char *) ps_memory (VNULL, (size_t)(strlen (this) + 6), sizeof (char));
		sprintf (PSL->internal.USERDIR, "%s%c%s", this, DIR_DELIM, ".gmt");
	}
	else {
#ifdef WIN32
		/* Set USERDIR to C:\.gmt under Windows */
		PSL->internal.USERDIR = (char *) ps_memory (VNULL, (size_t)8, sizeof (char));
		sprintf (PSL->internal.USERDIR, "C:%c%s", DIR_DELIM, ".gmt");
#else
		fprintf (stderr, "GMT Warning: Could not determine home directory!\n");
#endif
	}
	if (access(PSL->internal.USERDIR,R_OK)) PSL->internal.USERDIR = CNULL;

	ps_init_fonts (&PSL->internal.N_FONTS, &n_GMT_fonts);	/* Load the available font information */

	PSL->internal.verbose = (mode & 2) ? TRUE : FALSE;
	PSL->internal.ascii = (mode & 4) ? TRUE : FALSE;
	PSL->internal.color_mode = (mode >> 9) & 3;
	PSL->internal.compress = (mode >> 12) & 3;
	PSL->internal.absolute = (mode & 8) ? TRUE : FALSE;
	PSL->internal.line_cap = (mode >> 14) & 3;
	PSL->internal.line_join = (mode >> 16) & 3;
	PSL->internal.miter_limit = (mode >> 18) & 255;
	PSL->internal.comments = (mode >> 30) & 1;
	if (page_size[0] < 0.0) {		/* Want Manual Request for paper */
		PSL->internal.p_width  = fabs (page_size[0]);
		manual = TRUE;
	}
	else
		PSL->internal.p_width = page_size[0];
	if (page_size[1] < 0.0) {		/* Want EPS format */
		page_size[1] = -page_size[1];
		PSL->internal.eps_format = TRUE;
	}

	PSL->internal.p_height = page_size[1];
	PSL->current.linewidth = -1;	/* Will be changed by ps_setline */
	PSL->current.rgb[0] = PSL->current.rgb[1] = PSL->current.rgb[2] = -1;	/* Will be changed by ps_setpaint */
	PSL->current.fill_rgb[0] = PSL->current.fill_rgb[1] = PSL->current.fill_rgb[2] = (int)-2;	/* Will be changed by ps_setfill */
	PSL->current.outline = -2;
	PSL->internal.scale = (double)dpi;	/* Dots pr. unit resolution of output device */
	PSL->internal.points_pr_unit = 72.0;
	if (unit == 0) PSL->internal.points_pr_unit /= 2.54;
	if (unit == 2) PSL->internal.points_pr_unit /= 0.0254;
	mode &= 1;							/* Get rid of other flags */
	if (plotfile == NULL || plotfile[0] == 0)
		PSL->internal.fp = stdout;
	else {
		(overlay) ? strcpy (openmode, "a") : strcpy (openmode, "w");
		if ((PSL->internal.fp = fopen (plotfile, openmode)) == NULL) {
			fprintf (stderr, "pslib: Cannot create/open file : %s\n", plotfile);
			return (-1);
		}
	}

#ifdef WIN32
	/*
	 * Diomidis Spinellis, December 2001
	 * Set binary mode to avoid corrupting binary color images.
	 */
	setmode(fileno(PSL->internal.fp), O_BINARY);
#elif __EMX__	/* PW: Same for OS/2 with EMX support */
	_fsetmode (PSL->internal.fp, "b");
#endif

	right_now = time ((time_t *)0);
	PSL->internal.landscape = !(overlay || mode);	/* Only rotate if not overlay and not Portrait */
	PSL->init.magnify[0] = xscl;
	PSL->init.origin[0] = xoff;
	PSL->init.magnify[1] = yscl;
	PSL->init.origin[1] = yoff;
	/* Initialize global variables */
	strcpy (PSL->current.bw_format, "%.3lg A");			/* Default format used for grayshade value */
	strcpy (PSL->current.rgb_format, "%.3lg %.3lg %.3lg C");	/* Same, for RGB triplets */
	strcpy (PSL->current.hsv_format, "%.3lg %.3lg %.3lg H");	/* Same, for HSV triplets */
	strcpy (PSL->current.cmyk_format, "%.3lg %.3lg %.3lg %.3lg K");	/* Same, for CMYK quadruples */

	/* In case this is the last overlay, set the Bounding box coordinates to be used atend */

	if (eps) {	/* Document info is available */
		if (PSL->init.eps->portrait) {	/* Plot originated as Portrait */
			PSL->internal.bb[0] = PSL->init.eps->x0;
			PSL->internal.bb[1] = PSL->init.eps->y0;
			PSL->internal.bb[2] = PSL->init.eps->x1;
			PSL->internal.bb[3] = PSL->init.eps->y1;
		}
		else {			/* Plot originated as Landscape */
			PSL->internal.bb[0] = PSL->internal.p_width - PSL->init.eps->y1;
			PSL->internal.bb[1] = PSL->init.eps->x0;
			PSL->internal.bb[2] = PSL->internal.p_width - PSL->init.eps->y0;
			PSL->internal.bb[3] = PSL->init.eps->x1;
		}
	}
	else {		/* No info is available, default to Current Media Size */
		PSL->internal.bb[0] = PSL->internal.bb[1] = 0.0;
		PSL->internal.bb[2] = PSL->internal.p_width;
		PSL->internal.bb[3] = (fabs (PSL->internal.p_height) < PSL_SMALL) ? PSL_PAGE_HEIGHT_IN_PTS : PSL->internal.p_height;
	}

	if (!overlay) {

		if (PSL->internal.eps_format)
			fprintf (PSL->internal.fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
		else
			fprintf (PSL->internal.fp, "%%!PS-Adobe-3.0\n");

		/* Write definitions of macros to plotfile */

		if (PSL->internal.eps_format) {
			fprintf (PSL->internal.fp, "%%%%BoundingBox: (atend)\n");
			fprintf (PSL->internal.fp, "%%%%HiResBoundingBox: (atend)\n");
		}
		else {
			fprintf (PSL->internal.fp, "%%%%BoundingBox: 0 0 %d %d\n", irint (PSL->internal.p_width), irint (PSL->internal.p_height));
			fprintf (PSL->internal.fp, "%%%%HiResBoundingBox: 0 0 %g %g\n", PSL->internal.p_width, PSL->internal.p_height);
		}
		if (eps) {	/* Document info is available */
			fprintf (PSL->internal.fp, "%%%%Title: %s\n", PSL->init.eps->title);
			fprintf (PSL->internal.fp, "%%%%Creator: GMT\n");
			fprintf (PSL->internal.fp, "%%%%For: %s\n", PSL->init.eps->name);
			fprintf (PSL->internal.fp, "%%%%DocumentNeededResources: font");
			for (i = 0; i < PSL_MAX_EPS_FONTS && PSL->init.eps->fontno[i] != -1; i++) fprintf (PSL->internal.fp, " %s", PSL->internal.font[PSL->init.eps->fontno[i]].name);
			fprintf (PSL->internal.fp, "\n");
		}
		else {
			fprintf (PSL->internal.fp, "%%%%Title: pslib v%s document\n", PSL_Version);
			fprintf (PSL->internal.fp, "%%%%Creator: pslib\n");
		}

		fprintf (PSL->internal.fp, "%%%%CreationDate: %s", ctime(&right_now));
		fprintf (PSL->internal.fp, "%%%%LanguageLevel: %d\n", PS_LANGUAGE_LEVEL);
		if (PSL->internal.ascii)
			fprintf (PSL->internal.fp, "%%%%DocumentData: Clean7Bit\n");
		else
			fprintf (PSL->internal.fp, "%%%%DocumentData: Binary\n");
		if (PSL->internal.landscape)
			fprintf (PSL->internal.fp, "%%%%Orientation: Landscape\n");
		else
			fprintf (PSL->internal.fp, "%%%%Orientation: Portrait\n");
		if (!PSL->internal.eps_format) fprintf (PSL->internal.fp, "%%%%Pages: 1\n");
		fprintf (PSL->internal.fp, "%%%%EndComments\n\n");

		fprintf (PSL->internal.fp, "%%%%BeginProlog\n");
		ps_bulkcopy ("PSL_prologue", "v 1.27 ");	/* Version number should match that of PSL_prologue.ps */
		ps_bulkcopy (PSL->init.encoding, "");

		def_font_encoding ();		/* Initialize book-keeping for font encoding and write font macros */

		ps_bulkcopy ("PSL_label", "v 1.14 ");		/* Place code for label line annotations and clipping */
		fprintf (PSL->internal.fp, "%%%%EndProlog\n\n");

		fprintf (PSL->internal.fp, "%%%%BeginSetup\n");
		fprintf (PSL->internal.fp, "/PSLevel /languagelevel where {pop languagelevel} {1} ifelse def\n");
		if (manual)	/* Manual media feed requested */
			fprintf (PSL->internal.fp, "PSLevel 1 gt { << /ManualFeed true >> setpagedevice } if\n");
		else if (!PSL->internal.eps_format && PSL->internal.p_width > 0.0 && PSL->internal.p_height > 0.0)	/* Specific media selected */
			fprintf (PSL->internal.fp, "PSLevel 1 gt { << /PageSize [%g %g] /ImagingBBox null >> setpagedevice } if\n", PSL->internal.p_width, PSL->internal.p_height);
		if (!PSL->internal.eps_format && ncopies > 1) fprintf (PSL->internal.fp, "/#copies %ld def\n", ncopies);
		fprintf (PSL->internal.fp, "%%%%EndSetup\n\n");

		if (!PSL->internal.eps_format) fprintf (PSL->internal.fp, "%%%%Page: 1 1\n\n");

		fprintf (PSL->internal.fp, "%%%%BeginPageSetup\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Init coordinate system and scales\n");
		scl = PSL->internal.points_pr_unit / PSL->internal.scale;
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Scale is originally set to %g, which means that\n", scl);
		if (unit == 0) {	/* CM used as unit */
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% 1 cm on the paper equals %g Postscript units\n", PSL->internal.scale);
		}
		else if (unit == 1) {	/* INCH used as unit */
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% 1 inch on the paper equals %g Postscript units\n", PSL->internal.scale);
		}
		else if (unit == 2) {	/* M used as unit */
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% 1 m on the paper equals %g Postscript units\n", PSL->internal.scale);
		}
		else {
			fprintf (stderr, "pslib: Measure unit not valid!\n");
			PS_exit (EXIT_FAILURE);
		}

		xscl *= scl;
		yscl *= scl;
		if (PSL->internal.landscape) fprintf (PSL->internal.fp, "%g 0 T 90 R\n", PSL->internal.p_width);
		fprintf (PSL->internal.fp, "%g %g scale\n", xscl, yscl);
		fprintf (PSL->internal.fp, "%%%%EndPageSetup\n\n");

		if (!(rgb[0] == rgb[1] && rgb[1] == rgb[2] && rgb[0] == 255)) {	/* Change background color */
			fprintf (PSL->internal.fp, "clippath ");
			ps_place_color (rgb);
			fprintf (PSL->internal.fp, " F N\n");
		}
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of pslib header\n\n");
	}
	init_font_encoding (eps);	/* Reencode fonts if necessary */

	/* Set line-handling attributes */
	ps_setlinecap (PSL->internal.line_cap);
	ps_setlinejoin (PSL->internal.line_join);
	ps_setmiterlimit (PSL->internal.miter_limit);
	ps_setpaint (no_rgb);
	if (!(xoff == 0.0 && yoff == 0.0)) fprintf (PSL->internal.fp, "%g %g T\n", xoff*PSL->internal.scale, yoff*PSL->internal.scale);

	return (0);
}

/* fortran interface */
void ps_plotinit_hires_ (char *plotfile, PSL_LONG *overlay, PSL_LONG *mode, double *xoff, double *yoff, double *xscl, double *yscl, PSL_LONG *ncopies, PSL_LONG *dpi, PSL_LONG *unit, double *page_size, int *rgb, const char *encoding, int nlen1, int nlen2)
{
	 ps_plotinit_hires (plotfile, *overlay, *mode, *xoff, *yoff, *xscl, *yscl, *ncopies, *dpi, *unit, page_size, rgb, encoding, (struct EPS *)NULL);
}

/* Original ps_ploitinit used ints for paper size */

PSL_LONG ps_plotinit (char *plotfile, PSL_LONG overlay, PSL_LONG mode, double xoff, double yoff, double xscl, double yscl, PSL_LONG ncopies, PSL_LONG dpi, PSL_LONG unit, PSL_LONG *page_size, int *rgb, const char *encoding, struct EPS *eps)
{
	double d_page_size[2];
	d_page_size[0] = (double)page_size[0];
	d_page_size[1] = (double)page_size[1];
	return (ps_plotinit_hires (plotfile, overlay, mode, xoff, yoff, xscl, yscl, ncopies, dpi, unit, d_page_size, rgb, encoding, eps));
}

/* fortran interface */
void ps_plotinit_ (char *plotfile, PSL_LONG *overlay, PSL_LONG *mode, double *xoff, double *yoff, double *xscl, double *yscl, PSL_LONG *ncopies, PSL_LONG *dpi, PSL_LONG *unit, PSL_LONG *page_size, int *rgb, const char *encoding, int nlen1, int nlen2)
{
	 ps_plotinit (plotfile, *overlay, *mode, *xoff, *yoff, *xscl, *yscl, *ncopies, *dpi, *unit, page_size, rgb, encoding, (struct EPS *)NULL);
}

void ps_setlinecap (PSL_LONG cap)
{
	fprintf (PSL->internal.fp, "%ld setlinecap\n", cap);
}

/* fortran interface */
void ps_setlinecap_ (PSL_LONG *cap)
{
	ps_setlinecap (*cap);
}

void ps_setlinejoin (PSL_LONG join)
{
	fprintf (PSL->internal.fp, "%ld setlinejoin\n", join);
}

/* fortran interface */
void ps_setlinejoin_ (PSL_LONG *join)
{
	ps_setlinejoin (*join);
}

void ps_setmiterlimit (PSL_LONG limit)
{
	double miter;
	miter = (limit == 0) ? 10.0 : 1.0 / sin (0.5 * limit * D2R);
	fprintf (PSL->internal.fp, "%g setmiterlimit\n", miter);
}

/* fortran interface */
void ps_setmiterlimit_ (PSL_LONG *limit)
{
	ps_setmiterlimit (*limit);
}

void ps_plotr (double x, double y, int pen)
{
	PSL_LONG ix, iy;

	/* Convert user coordinates to dots */
	ix = (PSL_LONG)irint (x * PSL->internal.scale);
	iy = (PSL_LONG)irint (y * PSL->internal.scale);
	
	/* Relative move or relative draw */
	if (pen == PSL_PEN_DRAW_AND_STROKE) {
		/* Always draw-stroke even when displacement is 0 */
		fprintf (PSL->internal.fp, "%ld %ld D S\n", ix, iy);
	}
	else if (ix == 0 && iy == 0)
		return;
	else if (pen == PSL_PEN_MOVE)
		fprintf (PSL->internal.fp, "%ld %ld G\n", ix, iy);
	else
		fprintf (PSL->internal.fp, "%ld %ld D\n", ix, iy);
	PSL->internal.ix += ix;	/* Update absolute position */
	PSL->internal.iy += iy;
}

/* fortran interface */
void ps_plotr_ (double *x, double *y, int *pen)
{
	ps_plotr (*x, *y, *pen);
}

void ps_polygon (double *x, double *y, PSL_LONG n, int rgb[], PSL_LONG outline)
{
	/* Draw and optionally fill polygons. */

	ps_setfill (rgb, outline);
	if (outline >= 0) ps_line (x, y, n, 1, FALSE);	/* No stroke or close path yet */
	ps_command ("P fs os");

	if (outline == -1) {
		fprintf (PSL->internal.fp, "U\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Clipping is currently OFF\n");
	}
}

/* fortran interface */
void ps_polygon_ (double *x, double *y, PSL_LONG *n, int *rgb, PSL_LONG *outline)
{
	ps_polygon (x, y, *n, rgb, *outline);
}

void ps_patch (double *x, double *y, PSL_LONG np, int rgb[], PSL_LONG outline)
{
	/* Like ps_polygon but intended for small polygons (< 20 points).  No checking for
	 * shorter path by calling ps_shorten_path as in ps_polygon.
	 */

	PSL_LONG ix[20], iy[20], i, n, n1;

	if (np > 20) {	/* Must call ps_polygon instead */
		ps_polygon (x, y, np, rgb, outline);
		return;
	}

	ix[0] = (PSL_LONG)irint (x[0] * PSL->internal.scale);	/* Convert inch to absolute pixel position for start of quadrilateral */
	iy[0] = (PSL_LONG)irint (y[0] * PSL->internal.scale);

	for (i = n = 1, n1 = 0; i < np; i++) {	/* Same but check if new point represent a different pixel */
		ix[n] = (PSL_LONG)irint (x[i] * PSL->internal.scale);
		iy[n] = (PSL_LONG)irint (y[i] * PSL->internal.scale);
		if (ix[n] != ix[n1] || iy[n] != iy[n1]) n++, n1++;
	}
	if (ix[0] == ix[n1] && iy[0] == iy[n1]) n--, n1--;	/* Closepath will do this automatically */

	if (n < 3) return;	/* 2 points or less don't make a polygon */

	ps_setfill (rgb, outline);

	n--;
	n1 = n;
	for (i = n - 1; i != -1; i--, n--) fprintf (PSL->internal.fp, "%ld %ld ", ix[n] - ix[i], iy[n] - iy[i]);
	fprintf (PSL->internal.fp, "%ld %ld %ld SP\n", n1, ix[0], iy[0]);
}

/* fortran interface */

void ps_patch_ (double *x, double *y, PSL_LONG *n, int *rgb, PSL_LONG *outline)
{
	ps_patch (x, y, *n, rgb, *outline);
}

void ps_rect (double x1, double y1, double x2, double y2, int rgb[], PSL_LONG outline)
{
	PSL_LONG xll, yll;
	ps_setfill (rgb, outline);
	xll = (PSL_LONG)irint (x1 * PSL->internal.scale);	/* Get lower left point with minimum round-off */
	yll = (PSL_LONG)irint (y1 * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld %ld %ld %ld SB\n", (PSL_LONG)irint(y2 * PSL->internal.scale) - yll, (PSL_LONG)irint(x2 * PSL->internal.scale) - xll, xll, yll);
}

/* fortran interface */
void ps_rect_ (double *x1, double *y1, double *x2, double *y2, int *rgb, PSL_LONG *outline)
{
	ps_rect (*x1, *y1, *x2, *y2, rgb, *outline);
}

void ps_rotaterect (double x, double y, double angle, double x_len, double y_len, int rgb[], PSL_LONG outline)
{
	ps_setfill (rgb, outline);
	fprintf (PSL->internal.fp, "%ld %ld %g %ld %ld SR\n", (PSL_LONG)irint(y_len * PSL->internal.scale), (PSL_LONG)irint(x_len * PSL->internal.scale), angle, (PSL_LONG)irint(x * PSL->internal.scale), (PSL_LONG)irint(y * PSL->internal.scale));
}

/* fortran interface */
void ps_rotaterect_ (double *x1, double *y1, double *angle, double *x2, double *y2, int *rgb, PSL_LONG *outline)
{
	ps_rotaterect (*x1, *y1, *angle, *x2, *y2, rgb, *outline);
}

void ps_rotatetrans (double x, double y, double angle)
{
	PSL_LONG go = FALSE;

	if (fabs(angle) < 1e-9) angle = 0.0;
	if (angle != 0.0) {
		fprintf (PSL->internal.fp, "%g R", angle);
		go = TRUE;
	}
	if (fabs(x) < 1e-9) x = 0.0;
	if (fabs(y) < 1e-9) y = 0.0;
	if (x != 0.0 || y != 0.0) {
		if (go) fputc (' ', PSL->internal.fp);
		fprintf (PSL->internal.fp, "%g %g T", x * PSL->internal.scale, y * PSL->internal.scale);
		go = TRUE;
	}
	if (go) fputc ('\n', PSL->internal.fp);
}

/* fortran interface */
void ps_rotatetrans_ (double *x, double *y, double *angle)
{
	 ps_rotatetrans (*x, *y, *angle);
}

void ps_setdash (char *pattern, PSL_LONG offset)
{
	/* Line structure in Postscript units
	 * offset from plotpoint in PS units
	 * Examples:
	 * pattern = "4 4", offset = 0:
	 *   4 units of line, 4 units of space, start at current point
	 * pattern = "5 3 1 3", offset = 2:
	 *   5 units line, 3 units space, 1 unit line, 3 units space, start
	 *    2 units from curr. point.
	 */

	if (offset == PSL->current.offset && ((pattern && !strcmp (pattern, PSL->current.texture)) || (!pattern && PSL->current.texture[0] == '\0'))) return;
	PSL->current.offset = offset;
	if (pattern)
		strncpy (PSL->current.texture, pattern, 512L);
	else
		memset (PSL->current.texture, 0, 512L);
	fputs ("S ", PSL->internal.fp);
	ps_place_setdash (pattern, offset);
	fputs ("\n", PSL->internal.fp);
}

void ps_place_setdash (char *pattern, PSL_LONG offset)
{
	PSL_LONG place_space;
	if (pattern) {
		fputs ("[", PSL->internal.fp);
		place_space = 0;
		while (*pattern) {
			if (place_space) fputc (' ', PSL->internal.fp);
			fprintf (PSL->internal.fp, "%g", (atoi(pattern) * 72.0 / PSL->internal.points_pr_unit));
			while (*pattern && *pattern != ' ') pattern++;
			while (*pattern && *pattern == ' ') pattern++;
			place_space = 1;
		}
		fprintf (PSL->internal.fp, "] %ld B", offset);
	}
	else
		fprintf (PSL->internal.fp, "[] 0 B");	/* Reset to continuous line */
}

/* fortran interface */
void ps_setdash_ (char *pattern, PSL_LONG *offset, int nlen)
{
	ps_setdash (pattern, *offset);
}

void ps_setfont (PSL_LONG font_no)
{
	if (font_no < 0 || font_no >= PSL->internal.N_FONTS)
		fprintf (stderr, "pslib: Selected font out of range (%ld), ignored\n", font_no);
	else
		PSL->current.font_no = font_no;
}

/* fortran interface */
void ps_setfont_ (PSL_LONG *font_no)
{
	ps_setfont (*font_no);
}

void ps_setformat (PSL_LONG n_decimals)
{
	/* Sets nmber of decimals used for rgb/gray specifications [3] */
	if (n_decimals < 1 || n_decimals > 3)
		fprintf (stderr, "pslib: Selected decimals for color out of range (%ld), ignored\n", n_decimals);
	else {
		sprintf (PSL->current.bw_format, "%%.%ldf A", n_decimals);
		sprintf (PSL->current.rgb_format, "%%.%ldf %%.%ldf %%.%ldf C", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.hsv_format, "%%.%ldf %%.%ldf %%.%ldf H", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.cmyk_format, "%%.%ldf %%.%ldf %%.%ldf %%.%ldf K", n_decimals, n_decimals, n_decimals, n_decimals);
	}
}

/* fortran interface */
void ps_setformat_ (PSL_LONG *n_decimals)
{
	ps_setformat (*n_decimals);
}

void ps_setline (PSL_LONG linewidth)
{
	if (linewidth < 0) {
		fprintf (stderr, "pslib: Selected linewidth is negative (%ld), ignored\n", linewidth);
		return;
	}
	if (linewidth == PSL->current.linewidth) return;

	fprintf (PSL->internal.fp, "S %g W\n", (double)(linewidth * 72.0 / PSL->internal.points_pr_unit));
	PSL->current.linewidth = linewidth;
}

/* fortran interface */
void ps_setline_ (PSL_LONG *linewidth)
{
	 ps_setline (*linewidth);
}

void ps_setpaint (int rgb[])
{
	if (rgb[0] < 0) return;	/* Some rgb's indicate no fill */
	if (rgb[0] == PSL->current.rgb[0] && rgb[1] == PSL->current.rgb[1] && rgb[2] == PSL->current.rgb[2]) return;	/* Same color as already set */

	fprintf (PSL->internal.fp, "S ");
	ps_place_color (rgb);
	fprintf (PSL->internal.fp, "\n");

	/* Update the current color information */

	PSL->current.rgb[0] = rgb[0];
	PSL->current.rgb[1] = rgb[1];
	PSL->current.rgb[2] = rgb[2];
}

/* fortran interface */
void ps_setpaint_ (int *rgb)
{
	 ps_setpaint (rgb);
}

void ps_textbox (double x, double y, double pointsize, char *text, double angle, PSL_LONG justify, PSL_LONG outline, double dx, double dy, int rgb[])
{
	/* x,y = location of string
	 * pointsize = fontsize in points
	 * text = text to be boxed in
	 * angle = angle with baseline (horizontal)
	 * justify indicates what x,y refers to, see fig below
	 * outline = TRUE if we should draw box outline
	 * dx, dy = Space between box border and text, in inches
	 * rgb = fill color
	 *
	 *
	 *   9       10      11
	 *   |----------------|
	 *   5  <textstring>  7
	 *   |----------------|
	 *   1       2        3
	 */
	char *string, align[3][10] = {"0", "2 div neg", "neg"};
	PSL_LONG i = 0, j, h_just, v_just, rounded;

	if (pointsize == 0.0) return;	/* Nothing to do if text has zero size */

	if (strlen (text) >= (BUFSIZ-1)) {
		fprintf (stderr, "pslib: text_item > %d long!\n", BUFSIZ);
		return;
	}

	rounded = (outline & 4 && dx > 0.0 && dy > 0.0);	/* Want rounded label boxes, assuming there is clearance */
	outline &= 3;	/* Turn off the 4 */
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% ps_textbox begin:");
	fprintf (PSL->internal.fp, "\nV\n");

	if (justify < 0)  {	/* Strip leading and trailing blanks */
		for (i = 0; text[i] == ' '; i++);
		for (j = strlen (text) - 1; text[j] == ' '; j--) text[j] = 0;
		justify = -justify;
	}

	if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
	ps_textdim ("PSL_dim", fabs (pointsize), PSL->current.font_no, &text[i]);	/* Set the string dimensions in PS */
	if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	ps_set_length ("PSL_dx", dx);
	ps_set_length ("PSL_dy", dy);
	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	/* Got to anchor point */

	if (pointsize > 0.0) {	/* Set a new anchor point */
		PSL->internal.ix = (PSL_LONG)irint (x * PSL->internal.scale);
		PSL->internal.iy = (PSL_LONG)irint (y * PSL->internal.scale);
		fprintf (PSL->internal.fp, "V %ld %ld T ", PSL->internal.ix, PSL->internal.iy);
	}
	else
		fprintf (PSL->internal.fp, "V PSL_save_x PSL_save_y T ");

	if (angle != 0.0) fprintf (PSL->internal.fp, "%.3g R ", angle);
	if (justify > 1) {	/* Move the new origin so (0,0) is lower left of box */
		h_just = (justify + 3) % 4;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (PSL->internal.fp, "PSL_dim_w %s ", align[h_just]) : fprintf (PSL->internal.fp, "0 ");
		(v_just) ? fprintf (PSL->internal.fp, "PSL_dim_h %s ", align[v_just]) : fprintf (PSL->internal.fp, "0 ");
		fprintf (PSL->internal.fp, "T ");
	}
	/* Here, (0,0) is lower point of textbox with no clearance yet */
	if (rounded) {
		fprintf (PSL->internal.fp, "\n/PSL_r %ld def\n", (PSL_LONG)irint (MIN (dx, dy) * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_dx2 %ld def\n", (PSL_LONG)irint ((dx - MIN (dx, dy)) * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_dy2 %ld def\n", (PSL_LONG)irint ((dy - MIN (dx, dy)) * PSL->internal.scale));
		ps_command ("/PSL_x_side PSL_dim_w PSL_dx2 2 mul add def");
		ps_command ("/PSL_y_side PSL_dim_h PSL_dim_d sub PSL_dy2 2 mul add def");
		ps_command ("/PSL_bx0 PSL_dx2 neg def");
		ps_command ("/PSL_by0 PSL_dim_d PSL_dy2 sub def");
		ps_command ("PSL_dx2 neg PSL_dim_d PSL_dy sub M PSL_x_side 0 D");
		ps_command ("PSL_bx0 PSL_x_side add PSL_by0 PSL_r 270 360 arc");
		ps_command ("0 PSL_y_side D PSL_bx0 PSL_x_side add PSL_by0 PSL_y_side add PSL_r 0 90 arc");
		ps_command ("PSL_x_side neg 0 D PSL_bx0 PSL_by0 PSL_y_side add PSL_r 90 180 arc");
		ps_command ("0 PSL_y_side neg D PSL_bx0 PSL_by0 PSL_r 180 270 arc P");
	}
	else {
		ps_command ("\n/PSL_x_side PSL_dim_w PSL_dx 2 mul add def");
		ps_command ("/PSL_y_side PSL_dim_h PSL_dim_d sub PSL_dy 2 mul add def");
		ps_command ("PSL_dx neg PSL_dim_d PSL_dy sub M PSL_x_side 0 D 0 PSL_y_side D PSL_x_side neg 0 D 0 PSL_y_side neg D P");
	}
	if (rgb[0] >= 0) {	/* Paint the textbox */
		fprintf (PSL->internal.fp, "V ");
		ps_place_color (rgb);
		fprintf (PSL->internal.fp, " F U ");
	}
	(outline) ? ps_command ("S U") : ps_command("N U");
	ps_command ("U");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% ps_textbox end:\n\n");

	ps_free ((void *)string);
}

/* fortran interface */
void ps_textbox_ (double *x, double *y, double *pointsize, char *text, double *angle, PSL_LONG *justify, PSL_LONG *outline, double *dx, double *dy, int *rgb, int nlen)
{
	 ps_textbox (*x, *y, *pointsize, text, *angle, *justify, *outline, *dx, *dy, rgb);
}

void ps_textdim (char *dim, double pointsize, PSL_LONG in_font, char *text)
{
	/* Will calculate the dimension of the given text string.
	 * Because of possible escape sequences we need to examine the string
	 * carefully.  The dimensions will be set in PostScript as dim_w, dim_h, dim_d
	 * The width (dim_w) is determined by "stringwidth" and includes some whitespace, making,
	 * for example, all numerical digits the same width (which we want). The height (dim_h)
	 * is measured from the baseline and does not include any depth (below the baseline).
	 * Finally, dim_d is the (negative) depth.
	 * We try to produce the "stringwidth" result also when the string includes
	 * escape sequences.
	 */

	char *tempstring, *piece, *piece2, *ptr, *string;
	PSL_LONG i = 0, font, sub, super, small, old_font, error = 0;
	double height, small_size, size, scap_size;

	if (strlen (text) >= (BUFSIZ-1)) {
		fprintf (stderr, "pslib: text_item > %d long!\n", BUFSIZ);
		return;
	}

	ps_setfont (in_font);			/* Switch to the selected font */

	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	height = pointsize / PSL->internal.points_pr_unit;

	if (!strchr (string, '@')) {	/* Plain text string */
		fprintf (PSL->internal.fp, "0 0 M %ld F%ld (%s) E /%s_w exch def FP pathbbox N /%s_h exch def pop /%s_d exch def pop\n" , (PSL_LONG) irint (height * PSL->internal.scale), PSL->current.font_no, string, dim, dim, dim);
		ps_free ((void *)string);
		return;
	}

	/* Here, we have special request for Symbol font and sub/superscript
	 * @~ toggles between Symbol font and default font
	 * @%<fontno>% switches font number <fontno>; give @%% to reset
	 * @- toggles between subscript and normal text
	 * @+ toggles between superscript and normal text
	 * @# toggles between Small caps and normal text
	 * @! will make a composite character of next two characters
	 * Use @@ to print a single @
	 */

	piece  = ps_memory (VNULL, (size_t)(2 * BUFSIZ), sizeof (char));
	piece2 = ps_memory (VNULL, (size_t)BUFSIZ, sizeof (char));

	font = old_font = PSL->current.font_no;
	size = height;
	small_size = height * 0.7;
	scap_size = height * 0.85;
	sub = super = small = FALSE;

	tempstring = ps_memory (VNULL, (size_t)(strlen(string)+1), sizeof (char));	/* Since strtok steps on it */
	strcpy (tempstring, string);
	ptr = strtok (tempstring, "@");
	fprintf (PSL->internal.fp, "N 0 0 m ");	/* Initialize currentpoint */
	if(string[0] != '@') {
		fprintf (PSL->internal.fp, "%ld F%ld (%s) FP ", (PSL_LONG)irint (size*PSL->internal.scale), font, ptr);
		ptr = strtok ((char *)NULL, "@");
	}

	while (ptr && !error) {
		if (ptr[0] == '!') {	/* Composite character */
			ptr++;
			if (ptr[0] == '\\')	/* Octal code */
				ptr += 4;
			else
				ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '~') {	/* Symbol font toggle */
			font = (font == 12) ? PSL->current.font_no : 12;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '%') {	/* Switch font option */
			ptr++;
			if (ptr[0] == '%')
				font = old_font;
			else {
				old_font = font;
				font = atoi (ptr);
			}
			while (*ptr != '\0' && *ptr != '%') ptr++;
			if (ptr[0] != '%') error++; else ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '-') {	/* Subscript toggle  */
			sub = !sub;
			size = (sub) ? small_size : height;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '+') {	/* Superscript toggle */
			super = !super;
			size = (super) ? small_size : height;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '#') {	/* Small caps toggle */
			small = !small;
			size = (small) ? scap_size : height;
			ptr++;
			(small) ? get_uppercase (piece, ptr) : (void) strcpy (piece, ptr);
		}
		else if (ptr[0] == ':') {	/* Font size change */
			ptr++;
			if (ptr[0] == ':')
				size = height;
			else {
				i = atoi (ptr);
				size = (double)i / PSL->internal.points_pr_unit;
				while (*ptr != '\0' && *ptr != ':') ptr++;
			}
			if (ptr[0] != ':') error++; else ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == ';') {	/* Color change */
			ptr++;
			while (*ptr != '\0' && *ptr != ';') ptr++;
			if (ptr[0] != ';') error++; else ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '_') {	/* Small caps toggle */
			ptr++;
			strcpy (piece, ptr);
		}
		else	/* Not recognized or @@ for a single @ */
			strcpy (piece, ptr);
		if (strlen (piece) > 0) fprintf (PSL->internal.fp, "%ld F%ld (%s) FP ", (PSL_LONG)irint (size*PSL->internal.scale), font, piece);
		ptr = strtok ((char *)NULL, "@");
	}
	if (error) {
		fprintf (stderr, "pslib: text_item %s has incomplete escape sequence - aborting.\n", text);
		PS_exit (EXIT_FAILURE);
	}
	fprintf (PSL->internal.fp, "pathbbox N ");
	fprintf (PSL->internal.fp, "/%s_h exch def exch /%s_d exch def add /%s_w exch def\n", dim, dim, dim);

	ps_free ((void *)tempstring);
	ps_free ((void *)piece);
	ps_free ((void *)piece2);
	ps_free ((void *)string);
}

void ps_text (double x, double y, double pointsize, char *text, double angle, PSL_LONG justify, PSL_LONG form)
{
	/* General purpose text plotter for single line of text.  For paragraphs, see ps_words.
	* ps_text positions and justifies the text string according to the parameters given.
	* The adjustments requires knowledge of font metrics and characteristics; hence all such
	* adjustments are passed on to the PostScript interpreter who will calculate the offsets.
	* The arguments to ps_text are as follows:
	*
	* x,y:		location of string
	* pointsize:	fontsize in points.  If negative, assume currentpoint is already set,
	*		else we use x, y to set a new currentpoint.
	* text:		text string to be plotted
	* angle:	angle between text baseline and the horizontal.
	* justify:	indicates where on the textstring the x,y point refers to, see fig below.
	*		If negative then we string leading and trailing blanks from the text.
	*		0 means no justification (already done separately).
	*
	*   9	    10      11
	*   |----------------|
	*   5       6        7
	*   |----------------|
	*   1	    2	     3
	* form:		0 = normal text, 1 = outline of text only
	*/

	char *piece, *piece2, *ptr, *string, op[16], align[3][10] = {"0", "2 div neg", "neg"};
	PSL_LONG dy, i = 0, j, font, v_just, h_just, upen, ugap;
	PSL_LONG sub, super, small, old_font, n_uline, start_uline, stop_uline;
	double height, small_size, size, scap_size, ustep, dstep;

	if (pointsize == 0.0) return;	/* Nothing to do if text has zero size */

	if (strlen (text) >= (BUFSIZ-1)) {	/* We gotta have some limit on how long a single string can be... */
		fprintf (stderr, "pslib: text_item > %d long - text not plotted!\n", BUFSIZ);
		return;
	}

	if (justify < 0)  {	/* Strip leading and trailing blanks */
		for (i = 0; text[i] == ' '; i++);
		for (j = strlen (text) - 1; text[j] == ' '; j--) text[j] = 0;
		justify = -justify;
	}

	if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
		ps_textdim ("PSL_dim", fabs (pointsize), PSL->current.font_no, &text[i]);			/* Set the string dimensions in PS */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	}

	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	height = fabs (pointsize) / PSL->internal.points_pr_unit;

	if (pointsize > 0.0) {	/* Set a new anchor point */
		PSL->internal.ix = (PSL_LONG)irint (x * PSL->internal.scale);
		PSL->internal.iy = (PSL_LONG)irint (y * PSL->internal.scale);
		fprintf (PSL->internal.fp, "%ld %ld M ", PSL->internal.ix, PSL->internal.iy);
	}

	if (angle != 0.0) fprintf (PSL->internal.fp, "V %.3g R ", angle);
	if (justify > 1) {
		h_just = (justify % 4) - 1;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (PSL->internal.fp, "PSL_dim_w %s ", align[h_just]) : fprintf (PSL->internal.fp, "0 ");
		(v_just) ? fprintf (PSL->internal.fp, "PSL_dim_h %s ", align[v_just]) : fprintf (PSL->internal.fp, "0 ");
		fprintf (PSL->internal.fp, "G ");
	}

	if (!strchr (string, '@')) {	/* Plain text string - do things simply and exit */
		fprintf (PSL->internal.fp, "%ld F%ld (%s) ", (PSL_LONG) irint (height * PSL->internal.scale), PSL->current.font_no, string);
		(form == 0) ? fprintf (PSL->internal.fp, "Z") : fprintf (PSL->internal.fp, "false charpath S");
		(angle != 0.0) ? fprintf (PSL->internal.fp, " U\n") : fprintf (PSL->internal.fp, "\n");
		ps_free ((void *)string);
		return;
	}

	/* Here, we have special request for Symbol font and sub/superscript
	 * @~ toggles between Symbol font and default font
	 * @%<fontno>% switches font number <fontno>; give @%% to reset
	 * @- toggles between subscript and normal text
	 * @+ toggles between superscript and normal text
	 * @# toggles between Small caps and normal text
	 * @! will make a composite character of next two characters
	 * Use @@ to print a single @
	 */

	piece  = ps_memory (VNULL, (size_t)(2 * BUFSIZ), sizeof (char));
	piece2 = ps_memory (VNULL, (size_t)BUFSIZ, sizeof (char));

	/* Now we can start printing text items */

	font = old_font = PSL->current.font_no;
	(form == 0) ? strcpy (op, "Z") : strcpy (op, "false charpath");
	sub = super = small = FALSE;
	size = height;
	small_size = height * 0.7;
	scap_size = height * 0.85;
	ustep = 0.35 * height;
	dstep = 0.25 * height;
	upen = (int)irint (0.025 * height * PSL->internal.scale);	/* Underline pen thickness */
	ugap = (int)irint (0.075 * height * PSL->internal.scale);	/* Underline shift */
	start_uline = stop_uline = n_uline = 0;

	ptr = strtok (string, "@");
	if(string[0] != '@') {	/* String has @ but not at start - must deal with first piece explicitly */
		fprintf (PSL->internal.fp, "%ld F%ld (%s) %s\n", (PSL_LONG)irint (size*PSL->internal.scale), font, ptr, op);
		ptr = strtok ((char *)NULL, "@");
	}

	while (ptr) {	/* Loop over all the sub-text items separated by escape characters */
		if (ptr[0] == '!') {	/* Composite character */
			ptr++;
			if (ptr[0] == '\\') {	/* Octal code */
				strncpy (piece, ptr, (size_t)4);
				piece[4] = 0;
				ptr += 4;
			}
			else {
				piece[0] = ptr[0];	piece[1] = 0;
				ptr++;
			}
			if (ptr[0] == '\\') {	/* Octal code again */
				strncpy (piece2, ptr, (size_t)4);
				piece2[4] = 0;
				ptr += 4;
			}
			else {
				piece2[0] = ptr[0];	piece2[1] = 0;
				ptr++;
			}
			/* Try to center justify these two character to make a composite character - may not be right */
			fprintf (PSL->internal.fp, "%ld F%ld (%s) dup stringwidth pop exch %s -2 div dup 0 G\n", (PSL_LONG)irint (size*PSL->internal.scale), font, piece2, op);
			fprintf (PSL->internal.fp, "%ld F%ld (%s) E -2 div dup 0 G exch %s sub neg dup 0 lt {pop 0} if 0 G\n", (PSL_LONG)irint (size*PSL->internal.scale), font, piece, op);
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '~') {	/* Symbol font */
			font = (font == 12) ? PSL->current.font_no : 12;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '%') {	/* Switch font option */
			ptr++;
			if (*ptr == '%')
				font = old_font;
			else {
				old_font = font;
				font = atoi (ptr);
			}
			while (*ptr != '%') ptr++;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '-') {	/* Subscript */
			sub = !sub;
			size = (sub) ? small_size : height;
			dy = (sub) ? (int)irint (-dstep*PSL->internal.scale) : (int)irint (dstep*PSL->internal.scale);
			fprintf (PSL->internal.fp, "0 %ld G\n", dy);
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '+') {	/* Superscript */
			super = !super;
			size = (super) ? small_size : height;
			dy = (super) ? (int)irint (ustep*PSL->internal.scale) : (int)irint (-ustep*PSL->internal.scale);
			fprintf (PSL->internal.fp, "0 %ld G\n", dy);
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '#') {	/* Small caps */
			small = !small;
			size = (small) ? scap_size : height;
			ptr++;
			(small) ? get_uppercase (piece, ptr) : (void) strcpy (piece, ptr);
		}
		else if (ptr[0] == ':') {	/* Font size change */
			ptr++;
			if (ptr[0] == ':')	/* Reset size */
				size = height;
			else {
				i = atoi (ptr);
				size = (double)i / PSL->internal.points_pr_unit;
				while (*ptr != ':') ptr++;
			}
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == ';') {	/* Font color change */
			int n_scan, rgb[3], error = FALSE;
			ptr++;
			if (ptr[0] == ';') {	/* Reset color */
				ps_place_color (PSL->current.rgb);
				fprintf (PSL->internal.fp, " ");
			}
			else {
				j = 0;
				while (ptr[j] != ';') j++;
				ptr[j] = 0;
				n_scan = sscanf (ptr, "%d/%d/%d", &rgb[0], &rgb[1], &rgb[2]);
				if (n_scan == 1) {	/* Got gray shade */
					rgb[1] = rgb[2] = rgb[0];
					if (rgb[0] < 0 || rgb[0] > 255) error++;
				}
				else if (n_scan == 3) {	/* Got r/g/b */
					if (rgb[0] < 0 || rgb[0] > 255) error++;
					if (rgb[1] < 0 || rgb[1] > 255) error++;
					if (rgb[2] < 0 || rgb[2] > 255) error++;
				}
				else {	/* Got crap */
					fprintf (stderr, "%s: Bad color change (%s) - ignored\n", "pslib", ptr);
					error = TRUE;
				}

				ptr[j] = ';';
				while (*ptr != ';') ptr++;
				if (!error) {
					ps_place_color (rgb);
					fprintf (PSL->internal.fp, " ");
				}
			}
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '_') {	/* Toggle underline */
			n_uline++;
			if (n_uline%2)
				start_uline = TRUE;
			else
				stop_uline = TRUE;
			ptr++;
			strcpy (piece, ptr);
		}
		else
			strcpy (piece, ptr);
		if (start_uline) fprintf (PSL->internal.fp, "currentpoint /y0_u exch def /x0_u exch def\n");
		if (stop_uline) fprintf (PSL->internal.fp, "V %ld W currentpoint pop /x1_u exch def x0_u y0_u %ld sub m x1_u x0_u sub 0 D S x1_u y0_u m U\n", upen, ugap);
		start_uline = stop_uline = FALSE;
		if (strlen (piece) > 0) fprintf (PSL->internal.fp, "%ld F%ld (%s) %s\n", (PSL_LONG)irint (size*PSL->internal.scale), font, piece, op);
		ptr = strtok ((char *)NULL, "@");
	}
	if (form == 1) fprintf (PSL->internal.fp, "S\n");
	if (angle != 0.0) fprintf (PSL->internal.fp, "U\n");

	ps_free ((void *)piece);
	ps_free ((void *)piece2);
	ps_free ((void *)string);
}

/* fortran interface */
void ps_text_ (double *x, double *y, double *pointsize, char *text, double *angle, PSL_LONG *justify, PSL_LONG *form, int nlen)
{
	ps_text (*x, *y, *pointsize, text, *angle, *justify, *form);
}

void ps_textpath (double x[], double y[], PSL_LONG n, PSL_LONG node[], double angle[], char *label[], PSL_LONG m, double pointsize, double offset[], PSL_LONG justify, PSL_LONG form)
{
	/* x,y		Array containing the label path
	 * n		Length of label path
	 * node		Index into x/y array of label plot positions
	 * angle	Text angle for each label
	 * label	Array of text labels
	 * m		Number of labels
	 * pointsize	Pointsize of label text
	 * offset	Clearances between text and textbox
	 * just		Justification of text relative to label coordinates
	 * form		bits: 1 = clip path, 2 = just place gap, 4 = draw line,
	 *		      8 = just call labelline and reuse last set of parameters
	 *		      32 = first time called, 64 = final time called, 128 = fill box, 256 = draw box
	 */

	PSL_LONG i = 0, j, k;
	PSL_LONG first;

	if (form & 8) {		/* If 8 bit is set we already have placed the info */
		form -= 8;		/* Knock off the 8 flag */
		fprintf (PSL->internal.fp, "%ld PSL_curved_text_labels\n", form);
		return;
	}

	if (m <= 0) return;	/* Nothing to do yet */
	if (pointsize == 0.0) return;	/* Nothing to do if text has zero size */

	first = (form & 32);

	for (i = 0; i < m; i++) {
		if (justify < 0)  {	/* Strip leading and trailing blanks */
			for (k = 0; label[i][k] == ' '; k++);	/* Count # of leading blanks */
			if (k > 0) {	/* Shift text to start, eliminating spaces */
				j = 0;
				while (label[i][k]) {
					label[i][j] = label[i][j+k];
					j++;
				}
				label[i][j] = 0;
			}
			/* Then strip off trailing blanks, if any */
			for (j = strlen (label[i]) - 1; label[i][j] == ' '; j--) label[i][j] = 0;
		}
	}
	justify = PSL_abs (justify);

	if (first) {	/* Do this only once */
		ps_set_integer ("PSL_just", (PSL_LONG)justify);
		ps_set_length ("PSL_gap_x", offset[0]);
		ps_set_length ("PSL_gap_y", offset[1]);
		if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
			if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
			ps_textdim ("PSL_dim", fabs (pointsize), PSL->current.font_no, label[0]);			/* Set the string dimensions in PS */
			fprintf (PSL->internal.fp, "PSL_dim_h PSL_dim_d sub /PSL_height exch def\n");
			if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
		}
		fprintf (PSL->internal.fp, "%ld F%ld\n", (PSL_LONG) irint ((fabs (pointsize) / PSL->internal.points_pr_unit) * PSL->internal.scale), PSL->current.font_no);	/* Set font */
	}

	/* Set these each time */

	n = ps_set_xyn_arrays ("PSL_x", "PSL_y", "PSL_node", x, y, node, n, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
	ps_set_integer ("PSL_n", n);
	ps_set_integer ("PSL_m", m);

	fprintf (PSL->internal.fp, "%ld PSL_curved_text_labels\n", form);
}

/* fortran interface */
void ps_textpath_ (double x[], double y[], PSL_LONG *n, PSL_LONG node[], double angle[], char *label[], PSL_LONG *m, double *pointsize, double offset[], PSL_LONG *justify, PSL_LONG *form, int len)
{
	ps_textpath (x, y, *n, node, angle, label, *m, *pointsize, offset, *justify, *form);
}

void ps_textclip (double x[], double y[], PSL_LONG m, double angle[], char *label[], double pointsize, double offset[], PSL_LONG justify, PSL_LONG key)
{
	/* x,y		Array containing the locations where labels will go
	 * m		Number of labels
	 * angle	Text angle for each label
	 * label	Array of text labels
	 * pointsize	Pointsize of label text
	 * offset	Gaps between text and textbox
	 * just		Justification of text relative to label coordinates
	 * key		bits: 0 = lay down clip path, 1 = Just place text, 2 turn off clipping,
	 *		8 = reuse pars, 16 = rounded box, 128 fill box, 256 draw box
	 */

	PSL_LONG i = 0, j, k;

	if (key & 2) {	/* Flag to terminate clipping */
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% If clipping is active, terminate it\n");
		fprintf (PSL->internal.fp, "PSL_clip_on {U /PSL_clip_on false def} if\n");
		return;
	}
	if (key & 8) {		/* Flag to place text already define in PSL arrays */
		fprintf (PSL->internal.fp, "%ld PSL_straight_text_labels\n", key);
		return;
	}

	/* Here key == 0 (or 4) which means we plan to create labeltext clip paths (and paint them) */

	if (m <= 0) return;	/* Nothing to do yet */
	if (pointsize == 0.0) return;	/* Nothing to do if text has zero size */

	for (i = 0; i < m; i++) {
		if (justify < 0)  {	/* Strip leading and trailing blanks */
			for (k = 0; label[i][k] == ' '; k++);	/* Count # of leading blanks */
			if (k > 0) {	/* Shift text to start, eliminating spaces */
				j = 0;
				while (label[i][k]) {
					label[i][j] = label[i][j+k];
					j++;
				}
				label[i][j] = 0;
			}
			/* Then strip off trailing blanks, if any */
			for (j = strlen (label[i]) - 1; label[i][j] == ' '; j--) label[i][j] = 0;
		}
	}
	justify = PSL_abs (justify);

	/* fprintf (PSL->internal.fp, "gsave\n"); */
	ps_set_integer ("PSL_m", m);
	ps_set_length_array ("PSL_txt_x", x, m);
	ps_set_length_array ("PSL_txt_y", y, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
	ps_set_integer ("PSL_just", (PSL_LONG)justify);
	ps_set_length ("PSL_gap_x", offset[0]);
	ps_set_length ("PSL_gap_y", offset[1]);

	if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
		ps_textdim ("PSL_dim", fabs (pointsize), PSL->current.font_no, label[0]);			/* Set the string dimensions in PS */
		fprintf (PSL->internal.fp, "PSL_dim_h PSL_dim_d sub /PSL_height exch def\n");
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	}

	fprintf (PSL->internal.fp, "%ld F%ld\n", (PSL_LONG) irint ((fabs (pointsize) / PSL->internal.points_pr_unit) * PSL->internal.scale), PSL->current.font_no);	/* Set font */
	fprintf (PSL->internal.fp, "%ld PSL_straight_text_labels\n", key);
}

/* fortran interface */
void ps_textclip_ (double x[], double y[], PSL_LONG *m, double angle[], char *label[], double *pointsize, double offset[], PSL_LONG *justify, PSL_LONG *key, int len)
{
	ps_textclip (x, y, *m, angle, label, *pointsize, offset, *justify, *key);
}

void ps_transrotate (double x, double y, double angle)
{
	PSL_LONG go = FALSE;

	if (fabs(x) < 1e-9) x = 0.0;
	if (fabs(y) < 1e-9) y = 0.0;
	if (x != 0.0 || y != 0.0) {
		fprintf (PSL->internal.fp, "%g %g T", x * PSL->internal.scale, y * PSL->internal.scale);
		go = TRUE;
	}
	if (fabs(angle) < 1e-9) angle = 0.0;
	if (angle != 0.0) {
		if (go) fputc (' ', PSL->internal.fp);
		fprintf (PSL->internal.fp, "%g R", angle);
		go = TRUE;
	}
	if (go) fputc ('\n', PSL->internal.fp);
}

/* fortran interface */
void ps_transrotate_ (double *x, double *y, double *angle)
{
	ps_transrotate (*x, *y, *angle);
}

void ps_vector (double xtail, double ytail, double xtip, double ytip, double tailwidth, double headlength, double headwidth, double headshape, int rgb[], PSL_LONG outline)
{
	/* Will make sure that arrow has a finite width in PS coordinates */

	double angle;
	PSL_LONG w2, length, hw, hl, hl2, hw2, l2;

	length = (PSL_LONG)irint (hypot ((xtail-xtip), (ytail-ytip)) * PSL->internal.scale);	/* Vector length in PS units */
	if (length == 0) return;					/* NULL vector */

	if (outline & 8)
		ps_setfill (rgb, outline - 8);
	else
		ps_setfill (rgb, outline);
	angle = atan2 ((ytip-ytail),(xtip-xtail)) * R2D;					/* Angle vector makes with horizontal, in radians */
	fprintf (PSL->internal.fp, "V %ld %ld T ", (PSL_LONG)irint (xtail * PSL->internal.scale), (PSL_LONG)irint (ytail * PSL->internal.scale));	/* Temporarily set tail point the local origin (0, 0) */
	if (angle != 0.0) fprintf (PSL->internal.fp, "%g R ", angle);					/* Rotate so vector is horizontal in local coordinate system */
	w2 = (PSL_LONG)irint (0.5 * tailwidth * PSL->internal.scale);	if (w2 == 0) w2 = 1;			/* Half-width of vector tail */
	hw = (PSL_LONG)irint (headwidth * PSL->internal.scale);	if (hw == 0) hw = 1;				/* Width of vector head */
	hl = (PSL_LONG)irint (headlength * PSL->internal.scale);							/* Length of vector head */
	hl2 = (PSL_LONG)irint (0.5 * headshape * headlength * PSL->internal.scale);					/* Cut-in distance due to slanted back-side of arrow head */
	hw2 = hw - w2;										/* Distance from tail side to head side (vertically) */
	if (outline & 8) {	/* Double-headed vector */
		l2 = length - 2 * hl + 2 * hl2;							/* Inside length between start of heads */
		fprintf (PSL->internal.fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld Sv U\n",
				hl2, hw2, -l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -hl2, hw2, hl, -hw);
	}
	else {			/* Single-headed vector */
		l2 = length - hl + hl2;								/* Length from tail to start of slanted head */
		fprintf (PSL->internal.fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld SV U\n",
			-l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -w2);
	}
}

/* fortran interface */
void ps_vector_ (double *xtail, double *ytail, double *xtip, double *ytip, double *tailwidth, double *headlength, double *headwidth, double *headshape, int *rgb, PSL_LONG *outline)
{
	 ps_vector (*xtail, *ytail, *xtip, *ytip, *tailwidth, *headlength, *headwidth, *headshape, rgb, *outline);
}

void ps_words (double x, double y, char **text, PSL_LONG n_words, double line_space, double par_width, PSL_LONG par_just, PSL_LONG font, double font_size, double angle, int rgb[3], PSL_LONG justify, PSL_LONG draw_box, double x_off, double y_off, double x_gap, double y_gap, PSL_LONG boxpen_width, char *boxpen_texture, PSL_LONG boxpen_offset, int boxpen_rgb[], PSL_LONG vecpen_width, char *vecpen_texture, PSL_LONG vecpen_offset, int vecpen_rgb[], int boxfill_rgb[3])
{
	PSL_LONG i, i1, i0, j, k, n, pj;
	PSL_LONG n_scan, color, found, last_k = -1;
	PSL_LONG error = 0, last_font, after;
	PSL_LONG *font_list, *font_unique, n_font_unique, n_rgb_unique;
	PSL_LONG n_alloc, n_items;
	int last_rgb[3], *rgb_list, *rgb_unique;
	PSL_LONG sub, super, small, plain_word = FALSE, under, escape;
	char *c, *clean, test_char;
	double last_size;
	struct GMT_WORD **word;
	struct GMT_WORD *add_word_part (char *word, PSL_LONG length, PSL_LONG fontno, double font_size, PSL_LONG sub, PSL_LONG super, PSL_LONG small, PSL_LONG under, PSL_LONG space, int rgb[]);

	if (font_size == 0.0) return;	/* Nothing to do if text has zero size */

	sub = super = small = under = FALSE;
	if (draw_box & 64) {	/* Smart offsets follow justification */
		if ((justify & 3) == 3)  x_off = -x_off;
		if ((justify & 2) == 2)  x_off = 0.0;
		if ((justify >> 2) == 2) y_off = -y_off;
		if ((justify & 4) == 4) y_off = 0.0;
	}

	n_alloc = PSL_CHUNK;
	last_font = font;
	last_size = font_size;

	word = (struct GMT_WORD **) ps_memory (VNULL, n_alloc, sizeof (struct GMT_WORD *));

	for (i = k = 0; i < n_words; i++) {

		clean = ps_prepare_text (text[i]);	/* Escape special characters and European character shorthands */

		if ((c = strchr (clean, '@'))) {	/* Found a @ escape command */
			i0 = 0;
			i1 = (PSL_LONG) (c - clean);

			if (i1 > i0) word[k++] = add_word_part (&clean[i0], i1 - i0, font, font_size, sub, super, small, under, NO_SPACE, rgb);
			if (k == n_alloc) {
				n_alloc <<= 1;
				word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
			}

			i1++;	/* Skip the @ */

			while (clean[i1]) {

				escape = (clean[i1-1] == '@');	/* i1 char is an escape argument */
				test_char = (escape) ? clean[i1] : 'A';		/* Only use clean[i1] if it is an escape modifier */
				plain_word = FALSE;

				switch (test_char) {

					case '!':	/* 2 Composite characters */
						i1++;
						if (clean[i1] == '\\') { /* First char is Octal code character */
							word[k++] = add_word_part (&clean[i1], (PSL_LONG)4, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k++] = add_word_part (&clean[i1], (PSL_LONG)1, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1++;
						}
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
						}
						if (clean[i1] == '\\') { /* 2nd char is Octal code character */
							word[k] = add_word_part (&clean[i1], (PSL_LONG)4, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k] = add_word_part (&clean[i1], (PSL_LONG)1, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1++;
						}
						if (!clean[i1]) word[k]->flag++;	/* New word after this composite */
						k++;
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
						}
						break;

					case '~':	/* Toggle symbol font */
						font = (font == SYMBOL) ? last_font : SYMBOL;
						i1++;
						break;

					case '%':	/* Switch font option */
						i1++;
						if (clean[i1] == '%') {
							font = last_font;
							i1++;
						}
						else {
							last_font = font;
							font = atoi (&clean[i1]);
							while (clean[i1] != '%') i1++;
							i1++;
						}
						break;

					case '_':	/* Toggle Underline */
						i1++;
						under = !under;
						break;

					case '-':	/* Toggle Subscript */
						i1++;
						sub = !sub;
						break;

					case '+':	/* Toggle Subscript */
						i1++;
						super = !super;
						break;

					case '#':	/* Toggle Small caps */
						i1++;
						small = !small;
						break;

					case ':':	/* Change font size */
						i1++;
						if (clean[i1] == ':') {
							font_size = last_size;
							i1++;
						}
						else {
							font_size = atof (&clean[i1]);
							while (clean[i1] != ':') i1++;
							i1++;
						}
						break;

					case ';':	/* Change font color */
						i1++;
						if (clean[i1] == ';') {
							memcpy ((void *)rgb, (void *)last_rgb, (size_t)(3 * sizeof (int)));
							i1++;
						}
						else {
							memcpy ((void *)last_rgb, (void *)rgb, (size_t)(3 * sizeof (int)));
							j = i1;
							while (clean[j] != ';') j++;
							clean[j] = 0;
							n_scan = sscanf (&clean[i1], "%d/%d/%d", &rgb[0], &rgb[1], &rgb[2]);
							if (n_scan == 1) {	/* Got gray shade */
								rgb[1] = rgb[2] = rgb[0];
								if (rgb[0] < 0 || rgb[0] > 255) error++;
							}
							else if (n_scan == 3) {	/* Got r/g/b */
								if (rgb[0] < 0 || rgb[0] > 255) error++;
								if (rgb[1] < 0 || rgb[1] > 255) error++;
								if (rgb[2] < 0 || rgb[2] > 255) error++;
							}
							else	/* Got crap */
								error++;

							clean[j] = ';';
							i1 = j + 1;
						}
						break;

					default:	/* Regular text to copy */

						j = i1;
						while (clean[j] && clean[j] != '@') j++;
						after = (clean[j]) ? NO_SPACE : 1;
						plain_word = TRUE;
						word[k++] = add_word_part (&clean[i1], j-i1, font, font_size, sub, super, small, under, after, rgb);
						if (k == n_alloc) {
							n_alloc <<= 1;
							word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
						}
						i1 = (clean[j]) ? j + 1 : j;
						break;
				}
				while (clean[i1] == '@') i1++;	/* SKip @ character */

			} /* End loop over word with @ in it */

			last_k = k - 1;
			if (!plain_word && (last_k = k - 1) >= 0) {	/* Allow space if text ends with @ commands only */
				word[last_k]->flag &= 60;
				word[last_k]->flag |= 1;
			}
		}
		else {	/* Plain word, no worries */
			word[k++] = add_word_part (clean, (PSL_LONG)0, font, font_size, sub, super, small, under, ONE_SPACE, rgb);
			if (k == n_alloc) {
				n_alloc <<= 1;
				word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
			}
		}

		ps_free ((void *)clean);	/* Reclaim this memory */

	} /* End of word loop */

	k--;
	while (k && !word[k]->txt) k--;	/* Skip any blank lines at end */
	n_items = k + 1;

	for (i0 = 0, i1 = 1 ; i1 < (PSL_LONG)n_items-1; i1++, i0++) {	/* Loop for periods ending sentences and indicate 2 spaces to follow */
		if (isupper ((int)word[i1]->txt[0]) && word[i0]->txt[strlen(word[i0]->txt)-1] == '.') {
			word[i0]->flag &= 60;	/* Sets bits 1 & 2 to zero */
			word[i0]->flag |= 2;	/* Specify 2 spaces */
		}
		if (!word[i1]->txt[0]) {	/* No space at end of paragraph */
			word[i0]->flag &= 60;
			word[i1]->flag &= 60;
		}
	}
	if (i1 >= (PSL_LONG)n_items) i1 = (PSL_LONG)n_items - 1;	/* one-word fix */
	word[i1]->flag &= 60;	/* Last word not followed by anything */

	/* Determine list of unique colors */

	rgb_list = (int *) ps_memory (VNULL, n_items, sizeof (int));
	rgb_unique = (int *) ps_memory (VNULL, n_items, sizeof (int));

	for (i = 0; i < (PSL_LONG)n_items; i++) rgb_list[i] = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
	qsort ((void *)rgb_list, (size_t) n_items, sizeof (int), ps_comp_int_asc);
	rgb_unique[0] = rgb_list[0];
	n_rgb_unique = 1;
	k = 0;
	for (i = 1; i < (PSL_LONG)n_items; i++) {
		if (rgb_list[i] != rgb_list[k]) {	/* New color */
			rgb_unique[n_rgb_unique++] = rgb_list[i];
			k = i;
		}
	}
	ps_free ((void *)rgb_list);

	/* Replace each word's red value with the index of the corresponding unique color entry */

	for (i = 0; i < (PSL_LONG)n_items; i++) {
		color = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
		for (j = 0, found = -1; found < 0 && j < n_rgb_unique; j++) if (color == rgb_unique[j]) found = j;
		word[i]->rgb[0] = (int)found;
	}

	/* Determine list of unique fonts */

	font_list = (PSL_LONG *) ps_memory (VNULL, n_items, sizeof (PSL_LONG));
	font_unique = (PSL_LONG *) ps_memory (VNULL, n_items, sizeof (PSL_LONG));

	for (i = 0; i < n_items; i++) font_list[i] = word[i]->font_no;
	qsort ((void *)font_list, (size_t) n_items, sizeof (PSL_LONG), ps_comp_long_asc);
	font_unique[0] = font_list[0];
	n_font_unique = 1;
	k = 0;
	for (i = 1; i < n_items; i++) {
		if (font_list[i] != font_list[k]) {	/* New font */
			font_unique[n_font_unique++] = font_list[i];
			k = i;
		}
	}
	ps_free ((void *)font_list);

	/* Replace each word's font with the index of the corresponding unique font entry */

	for (i = 0; i < n_items; i++) {
		for (j = 0, found = -1; found < 0 && j < n_font_unique; j++) if (word[i]->font_no == font_unique[j]) found = j;
		word[i]->font_no = found;
	}

	switch (par_just) {
		case 'l':
		case 'L':
			pj = 1;
			break;
		case 'c':
		case 'C':
			pj = 2;
			break;
		case 'r':
		case 'R':
			pj = 3;
			break;
		case 'j':
		case 'J':
			pj = 4;
			break;
		default:
			fprintf (stderr, "%s: Bad paragraph justification (%c) - Exiting\n", "pslib", (int)par_just);
			PS_exit (EXIT_FAILURE);
	}

	/* Time to write out to PS file */

	/* Load PSL_text procedures from file for now */

	if (!PSL->internal.text_init) {
		ps_bulkcopy ("PSL_text", "v 1.10 ");
		PSL->internal.text_init = TRUE;
	}

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% ps_words begin:\n");
	fprintf (PSL->internal.fp, "\nV\n");

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of fonts:\n");
	fprintf (PSL->internal.fp, "/PSL_fontname\n");
	for (i = 0 ; i < n_font_unique; i++) fprintf (PSL->internal.fp, "/%s\n", PSL->internal.font[font_unique[i]].name);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n_font_unique);
	ps_free ((void *)font_unique);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Initialize variables:\n\n");
	fprintf (PSL->internal.fp, "/PSL_n %ld def\n", (PSL_LONG)n_items);
	fprintf (PSL->internal.fp, "/PSL_n1 %ld def\n", (PSL_LONG)n_items - 1);
	fprintf (PSL->internal.fp, "/PSL_y0 %ld def\n", (PSL_LONG)irint (y * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_linespace %ld def\n", (PSL_LONG)irint (line_space * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_parwidth %ld def\n", (PSL_LONG)irint (par_width * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_parjust %ld def\n", pj);
	fprintf (PSL->internal.fp, "/PSL_spaces [() ( ) (  ) ] def\n");
	(draw_box & 1) ? fprintf (PSL->internal.fp, "/PSL_drawbox true def\n") : fprintf (PSL->internal.fp, "/PSL_drawbox false def\n");
	(draw_box & 2) ? fprintf (PSL->internal.fp, "/PSL_fillbox true def\n") : fprintf (PSL->internal.fp, "/PSL_fillbox false def\n");
	fprintf (PSL->internal.fp, "/PSL_boxshape %ld def\n", draw_box & 4);
	fprintf (PSL->internal.fp, "/PSL_lastfn -1 def\n/PSL_lastfs -1 def\n/PSL_lastfc -1 def\n");
	fprintf (PSL->internal.fp, "/PSL_UL 0 def\n/PSL_show {ashow} def\n");

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of words:\n");
	fprintf (PSL->internal.fp, "/PSL_word\n");
	for (i = n = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "(%s)", word[i]->txt);
		n += strlen (word[i]->txt) + 1;
		if (n < 60)
			fputc (' ', PSL->internal.fp);
		else {
			n = 0;
			fputc ('\n', PSL->internal.fp);
		}
	}
	if (n) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word font numbers:\n");
	fprintf (PSL->internal.fp, "/PSL_fnt\n");
	for (i = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%ld", word[i]->font_no);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word fontsizes:\n");
	fprintf (PSL->internal.fp, "/PSL_size\n");
	for (i = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%.2f", word[i]->font_size);
		(!((i+1)%20)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%20)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word spaces to follow:\n");
	fprintf (PSL->internal.fp, "/PSL_flag\n");
	for (i = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%ld", word[i]->flag);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word baseline shifts:\n");
	fprintf (PSL->internal.fp, "/PSL_bshift\n");
	for (i = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%g", word[i]->baseshift);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word colors indices:\n");
	fprintf (PSL->internal.fp, "/PSL_color\n");
	for (i = 0 ; i < (PSL_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%d", word[i]->rgb[0]);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PSL_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word colors:\n");
	fprintf (PSL->internal.fp, "/PSL_rgb\n");
	for (i = 0 ; i < n_rgb_unique; i++) fprintf (PSL->internal.fp, "%.3g %.3g %.3g\n", PSL_INV_255 * (rgb_unique[i] >> 16), PSL_INV_255 * ((rgb_unique[i] >> 8) & 0xFF), PSL_INV_255 * (rgb_unique[i] & 0xFF));
	fprintf (PSL->internal.fp, "%ld array astore def\n", 3 * n_rgb_unique);
	ps_free ((void *)rgb_unique);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word widths:\n\n");
	fprintf (PSL->internal.fp, "/PSL_width %ld array def\n", (PSL_LONG)n_items);
	fprintf (PSL->internal.fp, "0 1 PSL_n1 {");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Determine word width given the font and fontsize for each word\n") : fprintf (PSL->internal.fp, "\n");
	fprintf (PSL->internal.fp, "  /i exch def");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Loop index i\n") : fprintf (PSL->internal.fp, "\n");
	fprintf (PSL->internal.fp, "  PSL_size i get PSL_fontname PSL_fnt i get get Y");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Get and set font and size\n") : fprintf (PSL->internal.fp, "\n");
	fprintf (PSL->internal.fp, "  PSL_width i PSL_word i get stringwidth pop put");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t\t%% Calculate and store width\n") : fprintf (PSL->internal.fp, "\n");
	fprintf (PSL->internal.fp, "} for\n");

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word char counts:\n\n");
	fprintf (PSL->internal.fp, "/PSL_count %ld array def\n", (PSL_LONG)n_items);
	fprintf (PSL->internal.fp, "0 1 PSL_n1 {PSL_count exch dup PSL_word exch get length put} for\n");

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% For composite chars, set width and count to zero for 2nd char:\n\n");
	fprintf (PSL->internal.fp, "1 1 PSL_n1 {\n  /k exch def\n  PSL_flag k get 16 and 16 eq {\n");
	fprintf (PSL->internal.fp, "    /k1 k 1 sub def\n    /w1 PSL_width k1 get def\n    /w2 PSL_width k get def\n");
	fprintf (PSL->internal.fp, "    PSL_width k1 w1 w2 gt {w1} {w2} ifelse put\n    PSL_width k 0 put\n");
	fprintf (PSL->internal.fp, "    PSL_count k 0 put\n  } if\n} for\n\n");

	ps_transrotate (x, y, angle);	/* To original point */

	if (draw_box & 32) {	/* Draw line from box to point */
		ps_setline (vecpen_width);
		ps_setpaint (vecpen_rgb);
		if (vecpen_texture) ps_setdash (vecpen_texture, vecpen_offset);
		fprintf (PSL->internal.fp, "0 0 M %ld %ld D S\n", (PSL_LONG)irint (x_off * PSL->internal.scale), (PSL_LONG)irint (y_off * PSL->internal.scale));
		if (vecpen_texture) ps_setdash (CNULL, 0);
	}

	ps_transrotate (x_off, y_off, 0.0);	/* Adjust for shift */

	/* Do the relative horizontal justification */

	fprintf (PSL->internal.fp, "0 0 M\n\n0 PSL_textjustifier");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Just get paragraph height\n") : fprintf (PSL->internal.fp, "\n");

	/* Adjust origin for box justification */

	fprintf (PSL->internal.fp, "/PSL_x0 %ld def\n", -(PSL_LONG)irint (0.5 * ((justify - 1) % 4) * par_width * PSL->internal.scale));
	if (justify > 8) {	/* Top row */
		fprintf (PSL->internal.fp, "/PSL_y0 0 def\n");
	}
	else if (justify > 4) {	/* Middle row */
		fprintf (PSL->internal.fp, "/PSL_y0 PSL_parheight 2 div def\n");
	}
	else {			/* Bottom row */
		fprintf (PSL->internal.fp, "/PSL_y0 PSL_parheight def\n");
	}
	fprintf (PSL->internal.fp, "/PSL_txt_y0 PSL_top neg def\n");

	/* Make upper left textbox corner the origin */

	fprintf (PSL->internal.fp, "PSL_x0 PSL_y0 T\n\n");

	if (draw_box) {
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Start PSL box beneath text block:\n");
		ps_setline (boxpen_width);
		ps_setpaint (boxpen_rgb);
		if (boxpen_texture) ps_setdash (boxpen_texture, boxpen_offset);
		fprintf (PSL->internal.fp, "/PSL_xgap %ld def\n", (PSL_LONG)irint (x_gap * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_ygap %ld def\n", (PSL_LONG)irint (y_gap * PSL->internal.scale));
		if (draw_box & 16) {	/* Create convex box path */
			fprintf (PSL->internal.fp, "/PSL_h PSL_parheight 2 div PSL_ygap add def\n");
			fprintf (PSL->internal.fp, "/PSL_w PSL_parwidth 2 div PSL_xgap add def\n");
			fprintf (PSL->internal.fp, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
			fprintf (PSL->internal.fp, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
			fprintf (PSL->internal.fp, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
			fprintf (PSL->internal.fp, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% PSL_path:\n");
			fprintf (PSL->internal.fp, "PSL_xgap neg PSL_ygap M\n");
			fprintf (PSL->internal.fp, "PSL_ry PSL_xgap 2 mul sub PSL_parheight 2 div neg PSL_ry 180 PSL_ay sub 180 PSL_ay add arc\n");
			fprintf (PSL->internal.fp, "PSL_parwidth 2 div PSL_parheight 2 PSL_ygap mul add PSL_rx sub neg PSL_rx 270 PSL_ax sub 270 PSL_ax add arc\n");
			fprintf (PSL->internal.fp, "PSL_parwidth PSL_xgap 2 mul add PSL_ry sub PSL_parheight 2 div neg PSL_ry PSL_ay dup neg exch arc\n");
			fprintf (PSL->internal.fp, "PSL_parwidth 2 div PSL_ygap 2 mul PSL_rx sub PSL_rx 90 PSL_ax sub 90 PSL_ax add arc P\n");
		}
		else if (draw_box & 8) {	/* Create concave box path */
			fprintf (PSL->internal.fp, "/PSL_h PSL_parheight 2 div PSL_ygap 2 mul add def\n");
			fprintf (PSL->internal.fp, "/PSL_w PSL_parwidth 2 div PSL_xgap 2 mul add def\n");
			fprintf (PSL->internal.fp, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
			fprintf (PSL->internal.fp, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
			fprintf (PSL->internal.fp, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
			fprintf (PSL->internal.fp, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% PSL_path:\n");
			fprintf (PSL->internal.fp, "PSL_xgap 2 mul neg PSL_ygap 2 mul M\n");
			fprintf (PSL->internal.fp, "PSL_xgap PSL_ry add neg PSL_parheight 2 div neg PSL_ry PSL_ay dup neg arcn\n");
			fprintf (PSL->internal.fp, "PSL_parwidth 2 div PSL_parheight PSL_ygap add PSL_rx add neg PSL_rx 90 PSL_ax add 90 PSL_ax sub arcn\n");
			fprintf (PSL->internal.fp, "PSL_parwidth PSL_xgap add PSL_ry add PSL_parheight 2 div neg PSL_ry 180 PSL_ay add 180 PSL_ay sub arcn\n");
			fprintf (PSL->internal.fp, "PSL_parwidth 2 div PSL_ygap PSL_rx add PSL_rx 270 PSL_ax add 270 PSL_ax sub arcn P\n");
		}
		else if (draw_box & 4) {	/* Create rounded box path */
			fprintf (PSL->internal.fp, "/XL PSL_xgap neg def\n");
			fprintf (PSL->internal.fp, "/XR PSL_parwidth PSL_xgap add def\n");
			fprintf (PSL->internal.fp, "/YT PSL_ygap def\n");
			fprintf (PSL->internal.fp, "/YB PSL_parheight PSL_ygap add neg def\n");
			fprintf (PSL->internal.fp, "/PSL_r %ld def\n", (PSL_LONG)irint (MIN (x_gap, y_gap) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dx %ld def\n", (PSL_LONG)irint (MAX (x_gap-y_gap, 0.0) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dx %ld def\n", (PSL_LONG)irint (MAX (x_gap-y_gap, 0.0) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dy %ld def\n", (PSL_LONG)irint (MAX (y_gap-x_gap, 0.0) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/xl PSL_dx def\n");
			fprintf (PSL->internal.fp, "/xr PSL_parwidth PSL_dx add def\n");
			fprintf (PSL->internal.fp, "/yt PSL_dy def\n");
			fprintf (PSL->internal.fp, "/yb PSL_parheight PSL_dy add neg def\n");
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% PSL_path:\n");
			fprintf (PSL->internal.fp, "XL yt M XL yb L\n");
			fprintf (PSL->internal.fp, "xl yb PSL_r 180 270 arc xr YB L\n");
			fprintf (PSL->internal.fp, "xr yb PSL_r 270 360 arc XR yt L\n");
			fprintf (PSL->internal.fp, "xr yt PSL_r 0 90 arc xl YT L\n");
			fprintf (PSL->internal.fp, "xl yt PSL_r 90 180 arc P\n");
		}
		else {
			fprintf (PSL->internal.fp, "/XL PSL_xgap neg def\n");
			fprintf (PSL->internal.fp, "/XR PSL_parwidth PSL_xgap add def\n");
			fprintf (PSL->internal.fp, "/YT PSL_ygap def\n");
			fprintf (PSL->internal.fp, "/YB PSL_parheight PSL_ygap add neg def\n");
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% PSL_path:\n");
			fprintf (PSL->internal.fp, "XL YT M XL YB L XR YB L XR YT L P\n");
		}
		if (draw_box & 2) {	/* Fill */
			fprintf (PSL->internal.fp, "V ");
			ps_place_color (boxfill_rgb);
			fprintf (PSL->internal.fp, " F U ");
		}
		if (draw_box & 1) {	/* Stroke */
			ps_place_color (boxpen_rgb);
			fprintf (PSL->internal.fp, " S\n");
		}
		else
			fprintf (PSL->internal.fp, "N\n");
		if (boxpen_texture) ps_setdash (CNULL, 0);
		/* Because inside gsave/grestore we must reset PSL->pen and PSL->current.rgb so that they are set next time */
		PSL->current.rgb[0] = PSL->current.rgb[1] = PSL->current.rgb[2] = (int)-1;
		PSL->current.linewidth = -1;
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End PSL box beneath text block:\n");
	}
	/* Adjust origin so 0,0 is lower left corner of first character on baseline */

	fprintf (PSL->internal.fp, "0 PSL_txt_y0 T");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Move to col 0 on first baseline\n") : fprintf (PSL->internal.fp, "\n");
	fprintf (PSL->internal.fp, "\n0 0 M\n1 PSL_textjustifier");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Place the paragraph\n") : fprintf (PSL->internal.fp, "\n");

	fprintf (PSL->internal.fp, "U\n");

	ps_free ((void *)word);
}

/* fortran interface */
void ps_words_ (double *x, double *y, char **text, PSL_LONG *n_words, double *line_space, double *par_width, PSL_LONG *par_just, PSL_LONG* font, double *font_size, double *angle, int *rgb, PSL_LONG *justify, PSL_LONG *draw_box, double *x_off, double *y_off, double *x_gap, double *y_gap, PSL_LONG *boxpen_width, char *boxpen_texture, PSL_LONG *boxpen_offset, int *boxpen_rgb, PSL_LONG *vecpen_width, char *vecpen_texture, PSL_LONG *vecpen_offset, int *vecpen_rgb, int *boxfill_rgb, int n1, int n2, int n3) {

	ps_words (*x, *y, text, *n_words, *line_space, *par_width, *par_just, *font, *font_size, *angle, rgb, *justify, *draw_box, *x_off, *y_off, *x_gap, *y_gap, *boxpen_width, boxpen_texture, *boxpen_offset, boxpen_rgb, *vecpen_width, vecpen_texture, *vecpen_offset, vecpen_rgb, boxfill_rgb);

}

struct GMT_WORD *add_word_part (char *word, PSL_LONG length, PSL_LONG fontno, double font_size, PSL_LONG sub, PSL_LONG super, PSL_LONG small, PSL_LONG under, PSL_LONG space, int rgb[])
{
	/* For flag: bits 1 and 2 give number of spaces to follow (0, 1, or 2)
	 * bit 3 == 1 means leading TAB
	 * bit 4 == 1 means Composite 1 character
	 * bit 5 == 1 means Composite 2 character
	 * bit 6 == 1 means underline word
	 */

	PSL_LONG i = 0;
	int c;
	PSL_LONG tab = FALSE;
	double fs;
	struct GMT_WORD *new;

	if (!length) length = strlen (word);
	while (word[i] && word[i] == '\t') {	/* Leading tab(s) means indent once */
		tab = TRUE;
		i++;
		length--;
	}

	new = (struct GMT_WORD *) ps_memory (VNULL, (size_t)1, sizeof (struct GMT_WORD));
	new->txt = (char *) ps_memory (VNULL, (size_t)(length+1), sizeof (char));
	fs = font_size * PSL->internal.scale / PSL->internal.points_pr_unit;

	strncpy (new->txt, &word[i], (size_t)length);
	new->font_no = fontno;
	if (small) {	/* Small caps is on */
		new->font_size = 0.85 * fs;
		for (i = 0; new->txt[i]; i++) {
			c = (int)new->txt[i];
			new->txt[i] = (char) toupper (c);
		}
	}
	else if (super) {
		new->font_size = 0.7 * fs;
		new->baseshift = 0.35 * fs;
	}
	else if (sub) {
		new->font_size = 0.7 * fs;
		new->baseshift = -0.25 * fs;
	}
	else
		new->font_size = fs;

	new->flag = space;
	if (tab) new->flag |= 4;	/* 3rd bit indicates tab, then add space after word */
	if (under) new->flag |= 32;	/* 6rd bit indicates underline */
	memcpy ((void *)new->rgb, rgb, (3 * sizeof (int)));

	return (new);
}


/* Support functions used in ps_* functions.  No Fortran bindings needed */

void get_uppercase (char *new, char *old)
{
	PSL_LONG i = 0;
	int c;
	while (old[i]) {
		c = (int)old[i];
		new[i++] = toupper (c);
	}
	new[i] = 0;
}

void ps_encode_font (PSL_LONG font_no)
{
	if (PSL->init.encoding == 0) return;		/* Already have StandardEncoding by default */
	if (PSL->internal.font[font_no].encoded) return;	/* Already reencoded or should not be reencoded ever */

	/* Reencode fonts with Standard+ or ISOLatin1[+] encodings */
	fprintf (PSL->internal.fp, "PSL_font_encode %ld get 0 eq {%s_Encoding /%s /%s PSL_reencode PSL_font_encode %ld 1 put} if", font_no, PSL->init.encoding, PSL->internal.font[font_no].name, PSL->internal.font[font_no].name, font_no);
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Set this font\n") : fprintf (PSL->internal.fp, "\n");
	PSL->internal.font[font_no].encoded = TRUE;
}

void init_font_encoding (struct EPS *eps)
{	/* Reencode all the fonts that we know may be used: the ones listed in eps */

	PSL_LONG i;

	if (eps)
		for (i = 0; i < 6 && PSL->init.eps->fontno[i] != -1; i++) ps_encode_font (PSL->init.eps->fontno[i]);
	else	/* Must output all */
		for (i = 0; i < PSL->internal.N_FONTS; i++) ps_encode_font (i);
}

void def_font_encoding (void)
{
	/* Initialize book-keeping for font encoding and write font macros */

	PSL_LONG i;

	/* Initialize T/F array for font reencoding so that we only do it once
	 * for each font that is used */

	fprintf (PSL->internal.fp, "/PSL_font_encode ");
	for (i = 0; i < PSL->internal.N_FONTS; i++) fprintf (PSL->internal.fp, "0 ");
	fprintf (PSL->internal.fp, "%ld array astore def", PSL->internal.N_FONTS);
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Initially zero\n") : fprintf (PSL->internal.fp, "\n");

	/* Define font macros (see pslib.h for details on how to add fonts) */

	for (i = 0; i < PSL->internal.N_FONTS; i++) fprintf (PSL->internal.fp, "/F%ld {/%s Y}!\n", i, PSL->internal.font[i].name);
}

char *ps_prepare_text (char *text)

/*	Adds escapes for misc parenthesis, brackets etc.
	Will also translate to some European characters such as the @a, @e
	etc escape sequences. Calling function must REMEMBER to free memory
	allocated by string */
{
	char *psl_scandcodes[13][4] = {	/* Short-hand conversion for some European characters in both Standard [0], Standard+ [1], ISOLatin1 [2], and ISOLatin1+ [2] encoding */
		{ "AA", "\\375", "\\305", "\\305"},	/* Aring */
		{ "AE", "\\341", "\\306", "\\306"},	/* AE */
		{ "OE", "\\351", "\\330", "\\330"},	/* Oslash */
		{ "aa", "\\376", "\\345", "\\345"},	/* aring */
		{ "ae", "\\372", "\\346", "\\346"},	/* ae */
		{ "oe", "\\371", "\\370", "\\370"},	/* oslash */
		{ "C", "\\201", "\\307", "\\307"},	/* Ccedilla */
		{ "N", "\\204", "\\321", "\\321"},	/* Ntilde */
		{ "U", "\\335", "\\334", "\\334"},	/* Udieresis */
		{ "c", "\\215", "\\347", "\\347"},	/* ccedilla */
		{ "n", "\\227", "\\36", "\\361"},	/* ntilde */
		{ "\\373", "\\373", "\\337", "\\337"},	/* germandbls */
		{ "u", "\\370", "\\374", "\\374"}	/* udieresis */
	};
	char *string;
	PSL_LONG i=0, j=0, font;
	PSL_LONG he = 0;		/* GMT Historical Encoding (if any) */

	if (strcmp ("Standard", PSL->init.encoding) == 0)
		he = 1;
	if (strcmp ("Standard+", PSL->init.encoding) == 0)
		he = 2;
	/* ISOLatin1 and ISOLatin1+ are the same _here_. */
	if (strncmp ("ISOLatin1", PSL->init.encoding, (size_t)9) == 0)
		he = 3;

	string = ps_memory (NULL, (size_t)(2 * BUFSIZ), sizeof(char));
	while (text[i]) {
		if (he && text[i] == '@') {
			i++;
			switch (text[i]) {
				case 'A':
					strcat (string, psl_scandcodes[0][he-1]);
					j += strlen(psl_scandcodes[0][he-1]); i++;
					break;
				case 'E':
					strcat (string, psl_scandcodes[1][he-1]);
					j += strlen(psl_scandcodes[1][he-1]); i++;
					break;
				case 'O':
					strcat (string, psl_scandcodes[2][he-1]);
					j += strlen(psl_scandcodes[2][he-1]); i++;
					break;
				case 'a':
					strcat (string, psl_scandcodes[3][he-1]);
					j += strlen(psl_scandcodes[3][he-1]); i++;
					break;
				case 'e':
					strcat (string, psl_scandcodes[4][he-1]);
					j += strlen(psl_scandcodes[4][he-1]); i++;
					break;
				case 'o':
					strcat (string, psl_scandcodes[5][he-1]);
					j += strlen(psl_scandcodes[5][he-1]); i++;
					break;
				case 'C':
					strcat (string, psl_scandcodes[6][he-1]);
					j += strlen(psl_scandcodes[6][he-1]); i++;
					break;
				case 'N':
					strcat (string, psl_scandcodes[7][he-1]);
					j += strlen(psl_scandcodes[7][he-1]); i++;
					break;
				case 'U':
					strcat (string, psl_scandcodes[8][he-1]);
					j += strlen(psl_scandcodes[8][he-1]); i++;
					break;
				case 'c':
					strcat (string, psl_scandcodes[9][he-1]);
					j += strlen(psl_scandcodes[9][he-1]); i++;
					break;
				case 'n':
					strcat (string, psl_scandcodes[10][he-1]);
					j += strlen(psl_scandcodes[10][he-1]); i++;
					break;
				case 's':
					strcat (string, psl_scandcodes[11][he-1]);
					j += strlen(psl_scandcodes[1][he-1]); i++;
					break;
				case 'u':
					strcat (string, psl_scandcodes[12][he-1]);
					j += strlen(psl_scandcodes[12][he-1]); i++;
					break;
				case '@':
/*	Also now converts "@@" to the octal code for "@" = "\100" in both std and ISO.
	This was necessary since the system routine "strtok" gobbles up
	multiple @'s when parsing the string inside "ps_text", and thus
	didn't properly output a single "@" sign when encountering "@@".
	John L. Lillibridge: 4/6/95 [This was a problem on SGI; PW]
*/

					strcat (string, "\\100"); j += 4; i++;
					break;
				case '%':	/* Font switcher */
					if (isdigit ((int)text[i+1])) {	/* Got a font */
						font = atoi (&text[i+1]);
						ps_encode_font (font);
					}
					string[j++] = '@';
					string[j++] = text[i++];	/* Just copy over the rest */
					while (text[i] != '%') string[j++] = text[i++];
					break;
				default:
					string[j++] = '@';
					string[j++] = text[i++];
					break;
			}
		}
		else {
			switch (text[i]) {    /* NEED TO BE ESCAPED!!!! for PostScript*/
				case '{':
				case '}':
				case '[':
				case ']':
				case '(':
				case ')':
				case '<':
				case '>':
					if (j > 0 && string[MAX(j-1,0)] == '\\')	/* ALREADY ESCAPED... */
						string[j++] = text[i++];
					else {
						strcat(string, "\\"); j++;
						string[j++] = text[i++];
					}
					break;
				default:
					string[j++] = text[i++];
					break;
			}
		}
	}
	return (string);
}

unsigned char *ps_load_image (char *file, struct imageinfo *h)
{
	/* ps_load_image loads an image of any recognised type into memory
	 *
	 * Currently supported image types are:
	 * - Sun Raster File
	 * - (Encapsulated) PostScript File
	 */

	FILE *fp = NULL;

	/* Open PostScript or Sun raster file */

	if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "pslib: Cannot open image file %s!\n", file);
		PS_exit (EXIT_FAILURE);
	}

	/* Read magic number to determine image type */

	if (ps_read_rasheader (fp, h, 0, 0)) {
		fprintf (stderr, "pslib: Error reading magic number of image file %s!\n", file);
		PS_exit (EXIT_FAILURE);
	}
	fseek (fp, 0L, SEEK_SET);

	/* Which file type */

	if (h->magic == RAS_MAGIC) {
		return (ps_load_raster (fp, h));
	} else if (h->magic == EPS_MAGIC) {
		return (ps_load_eps (fp, h));
	} else {
		fprintf (stderr, "pslib: Unrecognised magic number 0x%x in file %s!\n", h->magic, file);
		PS_exit (EXIT_FAILURE);
	}

	return (0);	/* Dummy return to satisfy some compilers */
}

unsigned char *ps_load_eps (FILE *fp, struct imageinfo *h)
{
	/* ps_load_eps reads an Encapsulated PostScript file */

	PSL_LONG n, p, llx, lly, trx, try_, BLOCKSIZE=4096;
	unsigned char *buffer;

	llx=0; lly=0; trx=720; try_=720;

	/* Scan for BoundingBox */

	ps_get_boundingbox (fp, &llx, &lly, &trx, &try_);

	/* Rewind and load into buffer */

	n=0;
	fseek (fp, 0L, SEEK_SET);
	buffer = (unsigned char *) ps_memory (VNULL, (size_t)1, (size_t)BLOCKSIZE);
	while ((p = fread ((unsigned char *)buffer + n, (size_t)1, (size_t)BLOCKSIZE, fp)) == BLOCKSIZE)
	{
		n+=BLOCKSIZE;
		buffer = (unsigned char *) ps_memory ((void *)buffer, (size_t)1, (size_t)n+BLOCKSIZE);
	}
	n+=p;

	/* Fill header struct with appropriate values */
	h->magic = EPS_MAGIC;
	h->width = (int)(trx - llx);
	h->height = (int)(try_ - lly);
	h->depth = 0;
	h->length = (int)n;
	h->type = 4;
	h->maptype = RMT_NONE;
	h->maplength = 0;
	h->xorigin = (int)llx;
	h->yorigin = (int)lly;

	return (buffer);
}

unsigned char *ps_load_raster (FILE *fp, struct imageinfo *header)
{
	/* ps_load_raster reads a Sun standard rasterfile of depth 1, 8, 24, or 32 into memory */

	PSL_LONG mx_in, mx, j, k, i, ij, n = 0, ny, get, odd, oddlength, r_off, b_off;
	unsigned char *buffer, *entry, *red, *green, *blue;

	if (ps_read_rasheader (fp, header, 0, 7)) {
		fprintf (stderr, "pslib: Trouble reading Sun rasterfile header!\n");
		PS_exit (EXIT_FAILURE);
	}

	if (header->magic != RAS_MAGIC) {	/* Not a Sun rasterfile */
		fprintf (stderr, "pslib: Raster is not a Sun rasterfile (Magic # = 0x%x)!\n", header->magic);
		PS_exit (EXIT_FAILURE);
	}
	if (header->type < RT_OLD || header->type > RT_FORMAT_RGB) {
		fprintf (stderr, "pslib: Can only read Sun rasterfiles types %d - %d (your type = %d)!\n", RT_OLD, RT_FORMAT_RGB, header->type);
		PS_exit (EXIT_FAILURE);
	}

	buffer = entry = red = green = blue = (unsigned char *)NULL;

	if (header->depth == 1) {	/* 1 bit black and white image */
		mx_in = (PSL_LONG) (2 * ceil (header->width / 16.0));	/* Because Sun images are written in multiples of 2 bytes */
		mx = (PSL_LONG) (ceil (header->width / 8.0));		/* However, PS wants only the bytes that matters, so mx may be one less */
		ny = header->height;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 1-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);

		if (mx < mx_in) {	/* OK, here we must shuffle image to get rid of the superfluous last byte per line */
			for (j = k = ij = 0; j < ny; j++) {
				for (i = 0; i < mx; i++) buffer[k++] = buffer[ij++];
				ij++;	/* Skip the extra byte */
			}
		}
	}
	else if (header->depth == 8 && header->maplength) {	/* 8-bit with color table */
		get = header->maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if (n != header->maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		odd = (PSL_LONG)header->width%2;
		entry = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)entry, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 8-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &entry);
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)(3 * header->width * header->height), sizeof (unsigned char));
		for (j = k = ij = 0; j < header->height; j++) {
			for (i = 0; i < header->width; i++) {
				buffer[k++] = red[entry[ij]];
				buffer[k++] = green[entry[ij]];
				buffer[k++] = blue[entry[ij]];
				ij++;
			}
			if (odd) ij++;
		}
		header->depth = 24;
	}
	else if (header->depth == (size_t)8) {	/* 8-bit without color table (implicit grayramp) */
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 8-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
	}
	else if (header->depth == 24 && header->maplength) {	/* 24-bit raster with colormap */
		unsigned char r, b;
		get = header->maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if ((size_t)n != (size_t)header->maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 24-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		oddlength = 3 * header->width;
		odd = (3 * header->width) % 2;
		r_off = (header->type == RT_FORMAT_RGB) ? 0 : 2;
		b_off = (header->type == RT_FORMAT_RGB) ? 2 : 0;
		for (i = j = 0; i < header->length; i += 3, j += 3) {	/* BGR -> RGB */
			r =  red[buffer[i+r_off]];
			b = blue[buffer[i+b_off]];
			buffer[j] = r;
			buffer[j+1] = green[buffer[i+1]];
			buffer[j+2] = b;
			if (odd && (j+3)%oddlength == 0) i++;
		}
	}
	else if (header->depth == (size_t)24) {	/* 24-bit raster, no colormap */
		unsigned char r, b;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 24-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		oddlength = 3 * header->width;
		odd = (3 * header->width) % 2;
		r_off = (header->type == RT_FORMAT_RGB) ? 0 : 2;
		b_off = (header->type == RT_FORMAT_RGB) ? 2 : 0;
		for (i = j = 0; i < header->length; i += 3, j += 3) {	/* BGR -> RGB */
			r = buffer[i+r_off];
			b = buffer[i+b_off];
			buffer[j] = r;
			buffer[j+1] = buffer[i+1];
			buffer[j+2] = b;
			if (odd && (j+3)%oddlength == 0) i++;
		}
	}
	else if (header->depth == 32 && header->maplength) {	/* 32-bit raster with colormap */
		unsigned char b;
		get = header->maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if ((size_t)n != (size_t)header->maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 32-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		r_off = (header->type == RT_FORMAT_RGB) ? 1 : 3;
		b_off = (header->type == RT_FORMAT_RGB) ? 3 : 1;
		b = blue[buffer[b_off]];
		buffer[0] = red[buffer[r_off]];
		buffer[1] = green[buffer[2]];
		buffer[2] = b;
		for (i = 3, j = 4; j < header->length; i += 3, j += 4) {	/* _BGR -> RGB */
			buffer[i] = red[buffer[j+r_off]];
			buffer[i+1] = green[buffer[j+2]];
			buffer[i+2] = blue[buffer[j+b_off]];
		}
		header->depth = 24;
	}
	else if (header->depth == (size_t)32) {	/* 32-bit raster, no colormap */
		unsigned char b;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 32-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		r_off = (header->type == RT_FORMAT_RGB) ? 1 : 3;
		b_off = (header->type == RT_FORMAT_RGB) ? 3 : 1;
		b = buffer[b_off];
		buffer[0] = buffer[r_off];
		buffer[1] = buffer[2];
		buffer[2] = b;
		for (i = 3, j = 4; j < header->length; i += 3, j += 4) {	/* _BGR -> RGB */
			buffer[i] = buffer[j+r_off];
			buffer[i+1] = buffer[j+2];
			buffer[i+2] = buffer[j+b_off];
		}
		header->depth = 24;
	}
	else	/* Unrecognized format */
		return ((unsigned char *)NULL);

	fclose (fp);

	if (entry) ps_free ((void *)entry);
	if (red) ps_free ((void *)red);
	if (green) ps_free ((void *)green);
	if (blue) ps_free ((void *)blue);


	return (buffer);
}

PSL_LONG ps_read_rasheader (FILE *fp, struct imageinfo *h, PSL_LONG i0, PSL_LONG i1)
{
	/* Reads the header of a Sun rasterfile (or any other).
	   Since the byte order is defined as Big Endian, the bytes are read
	   byte by byte to ensure portability onto Little Endian platforms.
	 */

	unsigned char byte[4];
	PSL_LONG i, j, value, in[4];

	for (i = i0; i <= i1; i++) {

		if (fread ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) {
			fprintf (stderr, "pslib: Error reading rasterfile header\n");
			return (-1);
		}
		for (j = 0; j < 4; j++) in[j] = (int)byte[j];

		value = (in[0] << 24) + (in[1] << 16) + (in[2] << 8) + in[3];

		switch (i) {
			case 0:
				h->magic = (int)value;
				break;
			case 1:
				h->width = (int)value;
				break;
			case 2:
				h->height = (int)value;
				break;
			case 3:
				h->depth = (int)value;
				break;
			case 4:
				h->length = (int)value;
				break;
			case 5:
				h->type = (int)value;
				break;
			case 6:
				h->maptype = (int)value;
				break;
			case 7:
				h->maplength = (int)value;
				break;
		}
	}

	if (h->type == RT_OLD && h->length == 0) h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;

	return (0);
}

PSL_LONG ps_write_rasheader (FILE *fp, struct imageinfo *h, PSL_LONG i0, PSL_LONG i1)
{
	/* Writes the header of a Sun rasterfile.
	   Since the byte order is defined as Big Endian, the bytes are read
	   byte by byte to ensure portability onto Little Endian platforms.
	 */

	unsigned char byte[4];
	PSL_LONG i, j, value, in[4];

	for (i = i0; i <= i1; i++) {
		switch (i) {
			case 0:
				value = h->magic;
				break;
			case 1:
				value = h->width;
				break;
			case 2:
				value = h->height;
				break;
			case 3:
				value = h->depth;
				break;
			case 4:
				value = h->length;
				break;
			case 5:
				value = h->type;
				break;
			case 6:
				value = h->maptype;
				break;
			default:
				value = h->maplength;
				break;
		}

		in[0] = (value >> 24);
		in[1] = (value >> 16) & 255;
		in[2] = (value >> 8) & 255;
		in[3] = (value & 255);
		for (j = 0; j < 4; j++) byte[j] = (unsigned char)in[j];

		if (fwrite ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) {
			fprintf (stderr, "pslib: Error writing rasterfile header\n");
			return (-1);
		}
	}

	return (0);
}

indexed_image_t ps_makecolormap (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits)
{
	/* When image consists of less than MAX_COLORS colors, the image can be
	 * indexed to safe a significant amount of space.
	 * The image and colormap are returned as a struct indexed_image_t.
	 *
	 * It is important that the first RGB tuple is mapped to index 0.
	 * This is used for color masked images.
	 */
	PSL_LONG i, j, npixels;
	colormap_t colormap;
	indexed_image_t image;

	if (PSL_abs (nbits) != 24) return (NULL);		/* We only index into the RGB colorspace. */

	npixels = PSL_abs (nx) * ny;

	colormap = ps_memory (VNULL, (size_t)1, sizeof (*colormap));
	colormap->ncolors = 0;
	image = ps_memory (VNULL, (size_t)1, sizeof (*image));
	image->buffer = ps_memory (VNULL, (size_t)npixels, sizeof (*image->buffer));
	image->colormap = colormap;

	if (nx < 0) {
		/* Copy the colour mask value into index 0 */
		colormap->colors[0][0] = buffer[0];
		colormap->colors[0][1] = buffer[1];
		colormap->colors[0][2] = buffer[2];
		colormap->ncolors++;
		buffer += 3;		/* Skip to start of image */
	}

	for (i = 0; i < npixels; i++) {
		for (j = 0; j < colormap->ncolors; j++)
			if (colormap->colors[j][0] == buffer[0] && colormap->colors[j][1] == buffer[1] && colormap->colors[j][2] == buffer[2]) {
				image->buffer[i] = (unsigned char)j;
				break;
			}

		if (j == colormap->ncolors) {
			if (colormap->ncolors == MAX_COLORS) {	/* Too many colors to index. */
				ps_free (image->buffer);
				ps_free (image);
				ps_free (colormap);
				if (PSL->internal.verbose) fprintf (stderr, "pslib: Too many colors to make colormap - using 24-bit direct color instead.\n");
				return (NULL);
			}
			image->buffer[i] = (unsigned char)j;
			colormap->colors[j][0] = buffer[0];
			colormap->colors[j][1] = buffer[1];
			colormap->colors[j][2] = buffer[2];
			colormap->ncolors++;
		}
		buffer += 3;
	}

	/* There's no need for a color map when the number of colors is the same as the number of pixels.
	   Then you're better off with a compressed 24-bit color image instead. */
	if (colormap->ncolors >= npixels)  {
		ps_free (image->buffer);
		ps_free (image);
		ps_free (colormap);
		if (PSL->internal.verbose) fprintf (stderr, "pslib: Use of colormap is inefficient - using 24-bit direct color instead.\n");
		return (NULL);
	}

	if (PSL->internal.verbose) fprintf (stderr, "pslib: Colormap of %ld colors created\n", colormap->ncolors);
	return (image);
}

void ps_stream_dump (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits, PSL_LONG compress, PSL_LONG encode, PSL_LONG mask)
{
	/* Writes a stream of bytes in binary or ascii, performs RGB to CMYK
	 * conversion and compression.
	 * buffer	= stream of bytes
	 * nx, ny	= image dimensions in pixels
	 * nbits	= depth of image pixels in bits
	 * compress	= no (0), rle (1) or lzw (2) compression
	 * encode	= binary (0), ascii85 (1) or hex (2) encoding
	 * mask		= image (0), imagemask (1), or neither (2)
	 */
	PSL_LONG nbytes, i, unused = 0;
	unsigned char *buffer1, *buffer2;
	char *kind_compress[3] = {"", "/RunLengthDecode filter", "/LZWDecode filter"};
	char *kind_mask[2] = {"", "mask"};

	nx = PSL_abs (nx);
	nbytes = ((((PSL_LONG)nbits) * ((PSL_LONG)(nx))) + 7) / ((PSL_LONG)8) * ((PSL_LONG)ny);
	PSL->internal.length = 0;

	/* Transform RGB stream to CMYK stream */
	if (PSL->internal.color_mode == PSL_CMYK && nbits == 24)
		buffer1 = ps_cmyk_encode (&nbytes, buffer);
	else if (PSL->internal.color_mode == PSL_GRAY && nbits == 24)
		buffer1 = ps_gray_encode (&nbytes, buffer);
	else
		buffer1 = buffer;

	/* Perform selected compression method */
	if (compress == 1)
		buffer2 = ps_rle_encode (&nbytes, buffer1);
	else if (compress == 2)
		buffer2 = ps_lzw_encode (&nbytes, buffer1);
	else
		buffer2 = NULL;

	if (!buffer2)	{ /* If compression failed, or no compression requested */
		compress = 0;
		buffer2 = buffer1;
	}

	/* Output image dictionary */
	if (mask < 2) {
		fprintf (PSL->internal.fp, "/Width %ld /Height %ld /BitsPerComponent %ld\n", nx, ny, MIN(nbits,8));
		fprintf (PSL->internal.fp, "   /ImageMatrix [%ld 0 0 %ld 0 %ld] /DataSource currentfile", nx, -ny, ny);
		if (encode) fprintf (PSL->internal.fp, " /ASCII85Decode filter");
		if (compress) fprintf (PSL->internal.fp, " %s", kind_compress[compress]);
		fprintf (PSL->internal.fp, "\n>> image%s\n", kind_mask[mask]);
	}
	if (encode == 1) {
		/* Write each 4-tuple as ASCII85 5-tuple */
		for (i = 0; i < nbytes; i += 4) ps_a85_encode (&buffer2[i], nbytes-i);
		fprintf (PSL->internal.fp, "~>\n");
	}
	else if (encode == 2) {
		for (i = 0; i < nbytes; i++) {
			fprintf (PSL->internal.fp, "%02X", buffer2[i]); PSL->internal.length += 2;
			if (PSL->internal.length > 95) { fprintf (PSL->internal.fp, "\n"); PSL->internal.length = 0; }
		}
	}
	else {
		/* Plain binary dump */
		unused = fwrite ((void *)buffer, sizeof (unsigned char), (size_t)nbytes, PSL->internal.fp);
	}
	if (mask == 2) fprintf (PSL->internal.fp, "%s", kind_compress[compress]);

	/* Clear newly created buffers, but maintain original */
	if (buffer2 != buffer1) ps_free(buffer2);
	if (buffer1 != buffer ) ps_free(buffer1);
}

void ps_a85_encode (unsigned char quad[], PSL_LONG nbytes)
{
	/* Encode 4-byte binary to 5-byte ASCII
	 * Special cases:	#00000000 is encoded as z
	 *			When n < 4, output only n+1 bytes */
	PSL_LONG j;
	unsigned int n = 0;	/* Was size_t but that fails under 64-bit mode */
	unsigned char c[5];

	if (nbytes < 1) return;		/* Ignore empty input */
	nbytes = MIN (4, nbytes);	/* Limit to first four bytes */

	/* Wrap quad into a 4-byte integer */
	for (j = 0; j < nbytes; j++) n += quad[j] << (24 - 8*j);

	if (n == 0 && nbytes == 4) {	/* Set the only output byte to "z" */
		nbytes = 0;
		c[4] = 122;
	}
	else {				/* Determine output 5-tuple */
		for (j = 0; j < 4; j++) { c[j] = (n % 85) + 33; n = n / 85; }
		c[4] = n + 33 ;
	}

	/* Print 1 byte if n = 0, otherwise print nbytes+1 byte
	 * Insert newline when line exceeds 96 characters */
	for (j = 4; j >= 4-nbytes; j--) {
		fprintf (PSL->internal.fp, "%c", c[j]); PSL->internal.length++;
		if (PSL->internal.length > 95) { fprintf (PSL->internal.fp, "\n"); PSL->internal.length = 0; }
	}
}

#define ESC 128

void ps_rle_decode (struct imageinfo *h, unsigned char **in)
{
	/* Function to undo RLE encoding in Sun rasterfiles
	 *
	 * RLE consists of ESCaped pairs of bytes.  This are started
	 * when the ESC value is encountered.  The Next byte is the <count>,
	 * the following is the <value>.  We then replicate <value>
	 * the required number of times.  If count is 0 then ESC is output.
	 * If bytes are not ESCaped they are simply copied to output.
	 * This is implemented with the constraint that all scanlines must
	 * be an even number of bytes (i.e., we are using 16-bit words
	 */

	PSL_LONG i, j, col, width, len;
	PSL_LONG odd = FALSE, count;
	unsigned char mask_table[] = {0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
	unsigned char mask, *out, value = 0;

	i = j = col = count = 0;

	width = (PSL_LONG)irint (ceil (h->width * h->depth / 8.0));	/* Scanline width in bytes */
	if (width%2) odd = TRUE, width++;	/* To ensure 16-bit words */
	mask = mask_table[h->width%8];	/* Padding for 1-bit images */

	len = width * ((PSL_LONG)h->height);		/* Length of output image */
	out = (unsigned char *) ps_memory (VNULL, (size_t)len, sizeof (unsigned char));
	if (odd) width--;

	while (j < h->length || count > 0) {

		if (count) {
			out[i++] = value;
			count--;
			col++;
		}
		else {
			switch ((int)(*in)[j]) {
				case ESC:
					count = (int)(*in)[++j];
					j++;
					if (count == 0) {
						out[i++] = ESC;
						col++;
					}
					else {
						count++;
						value = (*in)[j];
						j++;
					}
					break;
				default:
					out[i++] = (*in)[j++];
					col++;
			}
		}

		if (col == width) {
			if (h->depth == 1) out[width-1] &= mask;
			if (odd) {out[i++] = 0; count = 0;}
			col = 0;
		}
	}

	if (i != len) fprintf (stderr, "pslib: ps_rle_decode has wrong # of outbytes (%ld versus expected %ld)\n", i, len);

	ps_free ((void *)*in);
	*in = out;
}

unsigned char *ps_cmyk_encode (PSL_LONG *nbytes, unsigned char *input)
{
	/* Recode RGB stream as CMYK stream */

	PSL_LONG in, out, nout;
	unsigned char *output;

	nout = *nbytes / 3 * 4;
	output = (unsigned char *)ps_memory (VNULL, (size_t)nout, sizeof (unsigned char));

	for (in = out = 0; in < *nbytes; out += 4, in += 3) ps_rgb_to_cmyk_char (&input[in], &output[out]);
	*nbytes = nout;
	return (output);
}

unsigned char *ps_gray_encode (PSL_LONG *nbytes, unsigned char *input)
{
	/* Recode RGB stream as gray-scale stream */

	PSL_LONG in, out, nout;
	unsigned char *output;

	nout = *nbytes / 3;
	output = (unsigned char *)ps_memory (VNULL, (size_t)nout, sizeof (unsigned char));

	for (in = out = 0; in < *nbytes; out++, in += 3) output[out] = (char) PSL_YIQ ((&input[in]));
	*nbytes = nout;
	return (output);
}

unsigned char *ps_rle_encode (PSL_LONG *nbytes, unsigned char *input)
{
	/* Run Length Encode a buffer of nbytes. */

	PSL_LONG count = 0, out = 0, in = 0, i;
	unsigned char pixel, *output;

	i = MAX (512, *nbytes) + 136;	/* Maximum output length */
	output = (unsigned char *)ps_memory (VNULL, (size_t)i, sizeof (unsigned char));

	/* Loop scanning all input bytes. Abort when inflating after processing at least 512 bytes */
	while (count < *nbytes && (out < in || out < 512)) {
		in = count;
		pixel = input[in++];
		while (in < *nbytes && in - count < 127 && input[in] == pixel) in++;
		if (in - count == 1) {	/* No more duplicates. How many non-duplicates were there? */
			while (in < *nbytes && (in - count) < 127 && ((input[in] != input[in-1] || in > 1) && input[in] != input[in-2])) in++;
			while (in < *nbytes && input[in] == input[in-1]) in--;
			output[out++] = (unsigned char)(in - count - 1);
			for (i = count; i < in; i++) output[out++] = input[i];
		}
		else {		/* Write out a runlength */
			output[out++] = (unsigned char)(count - in + 1);
			output[out++] = pixel;
		}
		count = in;
	}

	/* Write end of data marker */
	output[out++] = 128;

	/* Drop the compression when end result is bigger than original */
	if (out > in) {
		if (PSL->internal.verbose) fprintf (stderr, "pslib: RLE inflated %ld to %ld bytes. No compression done.\n", in, out);
		ps_free (output);
		return (NULL);
	}

	/* Return number of output bytes and output buffer */
	if (PSL->internal.verbose) fprintf (stderr, "pslib: RLE compressed %ld to %ld bytes\n", in, out);
	*nbytes = out;
	return (output);
}

unsigned char *ps_lzw_encode (PSL_LONG *nbytes, unsigned char *input)
{
	/* LZW compress a buffer of nbytes. */

	static PSL_LONG ncode = 4096*256;
	PSL_LONG i, index, in = 0;
	static short int clear = 256, eod = 257;
	short int table = 4095;	/* Initial value forces clearing of table on first byte */
	short int bmax = 0, pre, oldpre, ext, *code;
	byte_stream_t output;
	unsigned char *buffer;

	i = MAX (512, *nbytes) + 8;	/* Maximum output length */
	output = (byte_stream_t)ps_memory (VNULL, (size_t)1, sizeof (*output));
	output->buffer = (unsigned char *)ps_memory (VNULL, (size_t)i, sizeof (*output->buffer));
	code = (short int *)ps_memory (VNULL, (size_t)ncode, sizeof (short int));

	output->nbytes = 0;
	output->depth = 9;
	pre = input[in++];

	/* Loop scanning all input bytes. Abort when inflating after processing at least 512 bytes */
	while (in < *nbytes && (output->nbytes < in || output->nbytes < 512)) {
		if (table >= 4095) {	/* Refresh code table */
			output = ps_lzw_putcode (output, clear);
			for (i = 0; i < ncode; i++) code[i]=0;
			table = eod + 1;
			bmax = clear * 2;
			output->depth = 9;
		}

		ext = input[in++];
		oldpre = pre;
		index = (pre << 8) + ext;
		pre = code[index];

		if (pre == 0) {		/* Add new entry to code table */
			code[index] = table;
			table++;
			output = ps_lzw_putcode (output, oldpre);
			pre = ext;
			if (table == bmax) {
				bmax <<= 1;
				output->depth++;
			}
		}
	}

	/* Output last byte and End-of-Data */
	output = ps_lzw_putcode (output, pre);
	output = ps_lzw_putcode (output, eod);

	/* Drop the compression when end result is bigger than original */
	if (output->nbytes > in) {
		if (PSL->internal.verbose) fprintf (stderr, "pslib: LZW inflated %ld to %ld bytes. No compression done.\n", in, output->nbytes);
		ps_free (code);
		ps_free (output->buffer);
		ps_free (output);
		return (NULL);
	}

	/* Return number of output bytes and output buffer; release code table */
	if (PSL->internal.verbose) fprintf (stderr, "pslib: LZW compressed %ld to %ld bytes\n", in, output->nbytes);
	*nbytes = output->nbytes;
	buffer = output->buffer;
	ps_free (code);
	ps_free (output);
	return (buffer);
}

byte_stream_t ps_lzw_putcode (byte_stream_t stream, short int incode)
{
	static short int eod = 257;
	static size_t bit_count = 0;
	static size_t bit_buffer = 0;

	/* Add incode to buffer and output 1 or 2 bytes */
	bit_buffer |= (size_t) incode << (32 - stream->depth - bit_count);
	bit_count += stream->depth;
	while (bit_count >= 8) {
		stream->buffer[stream->nbytes] = (unsigned char)(bit_buffer >> 24);
		stream->nbytes++;
		bit_buffer <<= 8;
		bit_count -= 8;
	}
	if (incode == eod) {	/* Flush buffer */
		stream->buffer[stream->nbytes] = (unsigned char)(bit_buffer >> 24);
		stream->nbytes++;
		bit_buffer = 0;
		bit_count = 0;
	}
	return (stream);
}

PSL_LONG ps_bitimage_cmap (int f_rgb[], int b_rgb[])
{
	/* Print colormap for 1-bit image or imagemask. Returns value of "polarity":
	 * 0 = Paint 0 bits foreground color, leave 1 bits transparent
	 * 1 = Paint 1 bits background color, leave 0 bits transparent
	 * 2 = Paint 0 bits foreground color, paint 1 bits background color
	 * ! The following polarity modes were removed on 2009-01-12 since they caused
	 * ! ghostscript errors messages when the general pen color was not black.
	 * ! Thus these modes now use /DeviceGray instead.
	 * 3 = No coloring, but invert bits (i.e., 0 bits white, 1 bits black)
	 * 4 = Paint 0 bits black, leave 1 bits transparent
	 * 5 = Paint 1 bits black, leave 0 bits transparent
	 * 6 = No coloring, no inversion (i.e., 0 bits black, 1 bits white)
	 * ! Note that odd return values indicate that the bitmap has to be
	 * ! inverted before plotting, either explicitly, or through a mapping
	 * ! function in the PostScript image definition.
	 */
	int polarity, f_cmyk[4], b_cmyk[4];

	if (b_rgb[0] < 0) {
		/* Backgound is transparent */
		polarity = 0;
		if (!PSL_iscolor (f_rgb))
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 0 <%02X>] setcolorspace", f_rgb[0]);
		else if (PSL->internal.color_mode == PSL_GRAY)
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 0 <%02X>] setcolorspace", PSL_YIQ(f_rgb));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			ps_rgb_to_cmyk_int (f_rgb, f_cmyk);
			fprintf (PSL->internal.fp, " [/Indexed /DeviceCMYK 0 <%02X%02X%02X%02X>] setcolorspace", f_cmyk[0], f_cmyk[1], f_cmyk[2], f_cmyk[3]);
		}
		else
			fprintf (PSL->internal.fp, " [/Indexed /DeviceRGB 0 <%02X%02X%02X>] setcolorspace", f_rgb[0], f_rgb[1], f_rgb[2]);
	}
	else if (f_rgb[0] < 0) {
		/* Foreground is transparent */
		polarity = 1;
		if (!PSL_iscolor (b_rgb))
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 0 <%02X>] setcolorspace", b_rgb[0]);
		else if (PSL->internal.color_mode == PSL_GRAY)
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 0 <%02X>] setcolorspace", PSL_YIQ(b_rgb));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			ps_rgb_to_cmyk_int (b_rgb, b_cmyk);
			fprintf (PSL->internal.fp, " [/Indexed /DeviceCMYK 0 <%02X%02X%02X%02X>] setcolorspace", b_cmyk[0], b_cmyk[1], b_cmyk[2], b_cmyk[3]);
		}
		else
			fprintf (PSL->internal.fp, " [/Indexed /DeviceRGB 0 <%02X%02X%02X>] setcolorspace", b_rgb[0], b_rgb[1], b_rgb[2]);
	}
	else {
		/* Colored foreground and background */
		polarity = 2;
		if (!PSL_iscolor (b_rgb) && !PSL_iscolor (f_rgb))
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 1 <%02X%02X>] setcolorspace", f_rgb[0], b_rgb[0]);
		else if (PSL->internal.color_mode == PSL_GRAY)
			fprintf (PSL->internal.fp, " [/Indexed /DeviceGray 1 <%02X%02X>] setcolorspace", PSL_YIQ(f_rgb), PSL_YIQ(b_rgb));
		else if (PSL->internal.color_mode == PSL_CMYK) {
			ps_rgb_to_cmyk_int (f_rgb, f_cmyk);
			ps_rgb_to_cmyk_int (b_rgb, b_cmyk);
			fprintf (PSL->internal.fp, " [/Indexed /DeviceCMYK 1 <%02X%02X%02X%02X%02X%02X%02X%02X>] setcolorspace", f_cmyk[0], f_cmyk[1], f_cmyk[2], f_cmyk[3], b_cmyk[0], b_cmyk[1], b_cmyk[2], b_cmyk[3]);
		}
		else
			fprintf (PSL->internal.fp, " [/Indexed /DeviceRGB 1 <%02X%02X%02X%02X%02X%02X>] setcolorspace", f_rgb[0], f_rgb[1], f_rgb[2], b_rgb[0], b_rgb[1], b_rgb[2]);
	}

	return (polarity);
}

void ps_set_length (char *param, double value)
{
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, (PSL_LONG)irint (value * PSL->internal.scale));
}

void ps_set_height (char *param, double fontsize)
{
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, (PSL_LONG)irint (fontsize * PSL->internal.scale / PSL->internal.points_pr_unit));
}

void ps_set_integer (char *param, PSL_LONG value)
{
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, value);
}

void ps_define_pen (char *param, PSL_LONG width, char *texture, PSL_LONG offset, int rgb[])
{
	/* Function to set line pen attributes */
	fprintf (PSL->internal.fp, "/%s {", param);
	ps_place_color (rgb);
	fprintf (PSL->internal.fp, " %ld W ", width);
	ps_place_setdash (texture, offset);
	fprintf (PSL->internal.fp, "} def\n");
}

void ps_define_rgb (char *param, int rgb[])
{
	fprintf (PSL->internal.fp, "/%s {", param);
	ps_place_color (rgb);
	fprintf (PSL->internal.fp, "} def\n");
}

void ps_set_length_array (char *param, double *array, PSL_LONG n)
{	/* These are scaled by psscale */
	PSL_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "%.2f\n", array[i] * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

PSL_LONG ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, PSL_LONG *node, PSL_LONG n, PSL_LONG m)
{	/* These are scaled by psscale.  We make sure there are no point pairs that would yield dx = dy = 0 (repeat point)
	 * at the resolution we are using (0.01 DPI units), hence a new n (possibly shorter) is returned. */
	PSL_LONG i, j, k, this_i, this_j, last_i, last_j, n_skipped;
	char *use;

	use = (char *) ps_memory (VNULL, (size_t)n, sizeof (char));
	this_i = this_j = INT_MAX;
	for (i = j = k = n_skipped = 0; i < n; i++) {
		last_i = this_i;	last_j = this_j;
		this_i = (PSL_LONG)irint (x[i] * PSL->internal.scale * 100.0);	/* Simulates the digits written by a %.2lf format */
		this_j = (PSL_LONG)irint (y[i] * PSL->internal.scale * 100.0);
		if (this_i != last_i && this_j != last_j) {	/* Not a repeat point, use it */
			use[i] = TRUE;
			j++;
		}
		else	/* Repeat point, skip it */
			n_skipped++;
		if (k < m && node[k] == i && n_skipped) node[k++] -= n_skipped;	/* Adjust node pointer since we are removing points and upsetting the order */
	}
	fprintf (PSL->internal.fp, "/%s\n", xparam);
	for (i = 0; i < n; i++) if (use[i]) fprintf (PSL->internal.fp, "%.2f\n", x[i] * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld array astore def\n", j);
	fprintf (PSL->internal.fp, "/%s\n", yparam);
	for (i = 0; i < n; i++) if (use[i]) fprintf (PSL->internal.fp, "%.2f\n", y[i] * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld array astore def\n", j);
	fprintf (PSL->internal.fp, "/%s\n", nparam);
	for (i = 0; i < m; i++) fprintf (PSL->internal.fp, "%ld\n", node[i]);
	fprintf (PSL->internal.fp, "%ld array astore def\n", m);

	ps_free ((void *)use);
	return (j);
}

void ps_set_real_array (char *param, double *array, PSL_LONG n)
{	/* These are raw and not scaled */
	PSL_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "%.2f\n", array[i]);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

void ps_set_txt_array (char *param, char *array[], PSL_LONG n)
{
	PSL_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "(%s)\n", array[i]);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

void *ps_memory (void *prev_addr, PSL_LONG nelem, size_t size)
{
	void *tmp;

	if (nelem == 0) return (VNULL); /* Take care of n = 0 */

	if (prev_addr) {
		if ((tmp = realloc ((void *) prev_addr, (size_t)(nelem * size))) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not reallocate more memory, n = %ld\n", nelem);
			PS_exit (EXIT_FAILURE);
		}
	}
	else {
		if ((tmp = calloc ((size_t) nelem, size)) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not allocate memory, n = %ld\n", nelem);
			PS_exit (EXIT_FAILURE);
		}
	}
	return (tmp);
}

void ps_free (void *addr)
{
	if (addr) free (addr);
}

int ps_comp_int_asc (const void *p1, const void *p2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	int *point_1, *point_2;

	point_1 = (int *)p1;
	point_2 = (int *)p2;
	if ( (*point_1) < (*point_2) )
		return (-1);
	else if ( (*point_1) > (*point_2) )
		return (1);
	else
		return (0);
}

int ps_comp_long_asc (const void *p1, const void *p2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	PSL_LONG *point_1, *point_2;

	point_1 = (PSL_LONG *)p1;
	point_2 = (PSL_LONG *)p2;
	if ( (*point_1) < (*point_2) )
		return (-1);
	else if ( (*point_1) > (*point_2) )
		return (1);
	else
		return (0);
}

/* This function copies a file called $GMT_SHAREDIR/pslib/<fname>.ps
 * to the postscript output verbatim.
 * If version is not "" then the first line should contain both $Id: and
 * the requested version string.
 */
static void ps_bulkcopy (const char *fname, const char *version)
{
	FILE *in;
	char buf[BUFSIZ];
	char fullname[BUFSIZ];
	PSL_LONG i, j;
	PSL_LONG first = TRUE;

	ps_getsharepath ("pslib", fname, ".ps", fullname);
	if ((in = fopen (fullname, "r")) == NULL) {
		fprintf (stderr, "PSL Fatal Error: ");
		perror (fullname);
		PS_exit (EXIT_FAILURE);
	}

	while (fgets (buf, BUFSIZ, in)) {
		if (version[0] && first) {
			first = FALSE;
			if (!strstr (buf, "$Id:") || !strstr (buf, version)) fprintf (stderr, "Warning: PSL expects %sof %s\n", version, fullname);
		}
		else if (PSL->internal.comments) {
			/* We copy every line, including the comments, except those starting '%-' */
			if (buf[0] == '%' && buf[1] == '-') continue;
			fprintf (PSL->internal.fp, "%s", buf);
		}
		else {
			/* Here we remove the comments */
			i = 0;
			while (buf[i] && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')) i++;	/* Find first non-blank character */
			if (!buf[i]) continue;								/* Blank line, skip */
			if (buf[i] == '%' && buf[i+1] != '%') continue;					/* Comment line, skip */
			/* Output this line, but skip trailing comments (while watching for DSC %% comments) */
			/* Find the end of important stuff on the line (i.e., look for start of trailing comments) */
			for (i = 1; buf[i] && !(buf[i] == '%' && buf[i-1] != '%'); i++);
			i--;										/* buf[i] is the last character to be output */
			while (i && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')) i--;			/* Remove white-space prior to the comment */
			for (j = 0; j <= i; j++) fputc ((int)buf[j], PSL->internal.fp);
			fputc ('\n', PSL->internal.fp);
		}
	}
	fclose (in);
}

static void ps_init_fonts (PSL_LONG *n_fonts, PSL_LONG *n_GMT_fonts)
{
	FILE *in;
	PSL_LONG i = 0, n_alloc = 64;
	char buf[BUFSIZ];
	char fullname[BUFSIZ];

	/* Loads the available fonts for this installation */

	/* First the standard 35 PostScript fonts from Adobe */

	ps_getsharepath ("pslib", "PS_font_info", ".d", fullname);
	if ((in = fopen (fullname, "r")) == NULL) {
		fprintf (stderr, "PSL Fatal Error: ");
		perror (fullname);
		PS_exit (EXIT_FAILURE);
	}

	PSL->internal.font = (struct PSL_FONT *) ps_memory (VNULL, (size_t)n_alloc, sizeof (struct PSL_FONT));

	while (fgets (buf, BUFSIZ, in)) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
		if (sscanf (buf, "%s %lf %" PSL_LL "d", fullname, &PSL->internal.font[i].height, &PSL->internal.font[i].encoded) != 3) {
			fprintf (stderr, "PSL Fatal Error: Trouble decoding font info for font %ld\n", i);
			PS_exit (EXIT_FAILURE);
		}
		PSL->internal.font[i].name = (char *)ps_memory (VNULL, (size_t)(strlen (fullname)+1), sizeof (char));
		strcpy (PSL->internal.font[i].name, fullname);
		i++;
		if (i == n_alloc) {
			n_alloc <<= 1;
			PSL->internal.font = (struct PSL_FONT *) ps_memory ((void *)PSL->internal.font, (size_t)n_alloc, sizeof (struct PSL_FONT));
		}
	}
	fclose (in);
	*n_fonts = *n_GMT_fonts = i;

	/* Then any custom fonts */

	ps_getsharepath ("pslib", "CUSTOM_font_info", ".d", fullname);
	if (!access (fullname, R_OK)) {	/* Decode Custom font file */

		if ((in = fopen (fullname, "r")) == NULL)
		{
			fprintf (stderr, "PSL Fatal Error: ");
			perror (fullname);
			PS_exit (EXIT_FAILURE);
		}

		while (fgets (buf, BUFSIZ, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			PSL->internal.font[i].name = (char *)ps_memory (VNULL, strlen (buf), sizeof (char));
			if (sscanf (buf, "%s %lf %" PSL_LL "d", PSL->internal.font[i].name, &PSL->internal.font[i].height, &PSL->internal.font[i].encoded) != 3) {
				fprintf (stderr, "PSL Fatal Error: Trouble decoding custom font info for font %ld\n", i - *n_GMT_fonts);
				PS_exit (EXIT_FAILURE);
			}
			i++;
			if (i == n_alloc) {
				n_alloc <<= 1;
				PSL->internal.font = (struct PSL_FONT *) ps_memory ((void *)PSL->internal.font, (size_t)n_alloc, sizeof (struct PSL_FONT));
			}
		}
		fclose (in);
		*n_fonts = i;
	}
	PSL->internal.font = (struct PSL_FONT *) ps_memory ((void *)PSL->internal.font, (size_t)(*n_fonts), sizeof (struct PSL_FONT));
}

void ps_place_color (int rgb[])
{
	if (rgb[0] == -1) {
		/* Outline only, no color set */
	}
	else if (rgb[0] == -3) {
		/* Pattern fill */
		fprintf (PSL->internal.fp, "pattern%d I", rgb[1]);
	}
	else if (!PSL_iscolor (rgb)) {
		/* Gray scale, since R==G==B */
		fprintf (PSL->internal.fp, PSL->current.bw_format, rgb[0] * PSL_INV_255);
	}
	else if (PSL->internal.color_mode == PSL_GRAY) {
		/* Gray scale, forced by user */
		fprintf (PSL->internal.fp, PSL->current.bw_format, PSL_YIQ(rgb) * PSL_INV_255);
	}
	else if (PSL->internal.color_mode == PSL_RGB) {
		/* Full color, RGB mode */
		fprintf (PSL->internal.fp, PSL->current.rgb_format, rgb[0] * PSL_INV_255, rgb[1] * PSL_INV_255, rgb[2] * PSL_INV_255);
	}
	else if (PSL->internal.color_mode == PSL_CMYK) {
		/* CMYK mode */
		double cmyk[4];
		ps_rgb_to_cmyk (rgb, cmyk);
		fprintf (PSL->internal.fp, PSL->current.cmyk_format, cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
	}
	else {
		/* HSV mode */
		double hsv[3];
		ps_rgb_to_hsv (rgb, hsv);
		fprintf (PSL->internal.fp, PSL->current.hsv_format, hsv[0], hsv[1], hsv[2]);
	}
}

void ps_rgb_to_cmyk_char (unsigned char rgb[], unsigned char cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* RGB is in 0-255, CMYK will be in 0-255 range */

	int i;

	for (i = 0; i < 3; i++) cmyk[i] = 255 - rgb[i];
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
}

void ps_rgb_to_cmyk_int (int rgb[], int cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* RGB is in 0-255, CMYK will be in 0-255 range */

	int i;

	for (i = 0; i < 3; i++) cmyk[i] = 255 - rgb[i];
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
}

void ps_rgb_to_cmyk (int rgb[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* RGB is in 0-255, CMYK will be in 0-1 range */

	int i;

	for (i = 0; i < 3; i++) cmyk[i] = 1.0 - (rgb[i] * PSL_INV_255);
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
	for (i = 0; i < 4; i++) {
	    if (cmyk[i] < 0.0005) cmyk[i] = 0;
	}
}

void ps_rgb_to_hsv (int rgb[], double hsv[])
{
	double diff;
	int i, imax = 0, imin = 0;

	/* This had checks using rgb value in doubles (e.g. (max_v == xr)), which failed always on some compilers.
	   Changed to integer logic: 2009-02-05 by RS.
	*/
	for (i = 1; i < 3; i++) {
		if (rgb[i] > rgb[imax]) imax = i;
		if (rgb[i] < rgb[imin]) imin = i;
	}
	diff = (double)(rgb[imax] - rgb[imin]);
	hsv[0] = 0.0;
	hsv[1] = (rgb[imax] == 0) ? 0.0 : diff / rgb[imax];
	hsv[2] = rgb[imax] * PSL_INV_255;
	if (hsv[1] == 0.0) return;	/* Hue is undefined */
	hsv[0] = 120.0 * imax + 60.0 * (rgb[(imax + 1) % 3] - rgb[(imax + 2) % 3]) / diff;
	if (hsv[0] < 0.0) hsv[0] += 360.0;
	if (hsv[0] > 360.0) hsv[0] -= 360.0;
}

void ps_cmyk_to_rgb (int rgb[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	/* CMYK is in 0-1, RGB will be in 0-255 range */

	int i;

	for (i = 0; i < 3; i++) rgb[i] = (int) floor ((1.0 - cmyk[i] - cmyk[3]) * 255.999);
}

void ps_rgb_to_mono (unsigned char *buffer, struct imageinfo *h)
{
	int i, j;

	if (h->depth == 24) {
		for (i = j = 0; i < h->width * h->height; i++, j += 3)
		{
			buffer[i] = (unsigned char) PSL_YIQ ((&buffer[j]));
		}
		h->depth = 8;
	}
}

PSL_LONG ps_bitreduce (unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG ncolors)
{
	/* Reduce an 8-bit stream to 1-, 2- or 4-bit stream */
	PSL_LONG in, out, i, j, nout, nbits;

	/* Number of colors determines number of bits */
	if (ncolors <= 2)
		nbits = 1;
	else if (ncolors <= 4)
		nbits = 2;
	else if (ncolors <= 16)
		nbits = 4;
	else
		return (8);

	/* "Compress" bytes line-by-line. The number of bits per line should be multiple of 8 */
	out = 0;
	nx = PSL_abs (nx);
	nout = (nx * nbits + 7) / 8;
	for (j = 0; j < ny; j++) {
		in = j * nx;
		if (nbits == 1) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 7) + (buffer[in+1] << 6) + (buffer[in+2] << 5) + (buffer[in+3] << 4) + (buffer[in+4] << 3) + (buffer[in+5] << 2) + (buffer[in+6] << 1) + buffer[in+7];
				in += 8;
			}
		}
		else if (nbits == 2) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 6) + (buffer[in+1] << 4) + (buffer[in+2] << 2) + buffer[in+3];
				in += 4;
			}
		}
		else if (nbits == 4) {
			for (i = 0; i < nout; i++) {
				buffer[out++] = (buffer[in] << 4) + buffer[in+1];
				in += 2;
			}
		}
	}
	if (PSL->internal.verbose) fprintf (stderr, "pslib: Image depth reduced to %ld bits\n", nbits);
	return (nbits);
}

PSL_LONG ps_get_boundingbox (FILE *fp, PSL_LONG *llx, PSL_LONG *lly, PSL_LONG *trx, PSL_LONG *try_)
{
	PSL_LONG nested;
	char buf[BUFSIZ];

	nested = 0; *llx = 1; *trx = 0;
	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (!nested && !strncmp(buf, "%%BoundingBox:", (size_t)14)) {
			if (!strstr(buf, "(atend)")) {
				if (sscanf(strchr(buf, ':') + 1, "%" PSL_LL "d %" PSL_LL "d %" PSL_LL "d %" PSL_LL "d", llx, lly, trx, try_) < 4) return 1;
				break;
			}
		}
		else if (!strncmp(buf, "%%Begin", (size_t)7)) {
			++nested;
		}
		else if (nested && !strncmp(buf, "%%End", (size_t)5)) {
			--nested;
		}
	}

	if (*llx >= *trx || *lly >= *try_) {
		*llx = 0; *trx = 720; *lly = 0; *try_ = 720;
		fprintf(stderr, "No proper BoundingBox, defaults assumed: %ld %ld %ld %ld\n", *llx, *lly, *trx, *try_);
		return 1;
	}

	return 0;
}

char *ps_getsharepath (const char *subdir, const char *stem, const char *suffix, char *path)
{
	/* stem is the name of the file, e.g., CUSTOM_font_info.d
	 * subdir is an optional subdirectory name in the $GMT_SHAREDIR directory.
	 * suffix is an optional suffix to append to name
	 * path is the full path to the file in question
	 * Returns the full pathname if a workable path was found
	 * Looks for file stem in current directory, ~/.gmt and $GMT_SHAREDIR[/subdir]
	 */

	/* First look in the current working directory */

	sprintf (path, "%s%s", stem, suffix);
	if (!access (path, R_OK)) return (path);	/* Yes, found it in current directory */

	/* Do not continue when full pathname is given */

#ifdef WIN32
	if (stem[0] == '\\' || stem[1] == ':') return (NULL);
#else
	if (stem[0] == '/') return (NULL);
#endif

	/* Not found, see if there is a file in the user's GMT_USERDIR (~/.gmt) directory */

	if (PSL->internal.USERDIR) {
		sprintf (path, "%s%c%s%s", PSL->internal.USERDIR, DIR_DELIM, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Try to get file from $GMT_SHAREDIR/subdir */

	if (subdir) {
		sprintf (path, "%s%c%s%c%s%s", PSL->internal.SHAREDIR, DIR_DELIM, subdir, DIR_DELIM, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Finally try file in $GMT_SHAREDIR (for backward compatibility) */

	sprintf (path, "%s%c%s%s", PSL->internal.SHAREDIR, DIR_DELIM, stem, suffix);
	if (!access (path, R_OK)) return (path);

	return (NULL);	/* No file found, give up */
}
