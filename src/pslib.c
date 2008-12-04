/*--------------------------------------------------------------------
 *	$Id: pslib.c,v 1.179 2008-12-04 22:38:36 remko Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
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
 *	ps_cross		: Plots a +
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

#include "pslib_inc.h"
#include "pslib.h"

/* Special macros and structure for ps_words */

#define NO_SPACE	0
#define ONE_SPACE	1
#define COMPOSITE_1	8
#define COMPOSITE_2	16
#define SYMBOL		12
#define PSL_CHUNK	2048

struct GMT_WORD {
	int font_no;
	int rgb[3];
	int flag;
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
	int ncolors;
	unsigned char colors[MAX_COLORS][3];
} *colormap_t;

typedef struct
{
	unsigned char *buffer;
	colormap_t colormap;
} *indexed_image_t;

typedef struct {
	PS_LONG nbytes;
	int depth;
	unsigned char *buffer;
} *byte_stream_t;

/* Define support functions called inside pslib functions */

char *ps_prepare_text (char *text);
void def_font_encoding (void);
void init_font_encoding (struct EPS *eps);
void get_uppercase(char *new, char *old);
void ps_rle_decode (struct imageinfo *h, unsigned char **in);
unsigned char *ps_cmyk_encode (PS_LONG *nbytes, unsigned char *input);
unsigned char *ps_rle_encode (PS_LONG *nbytes, unsigned char *input);
unsigned char *ps_lzw_encode (PS_LONG *nbytes, unsigned char *input);
byte_stream_t ps_lzw_putcode (byte_stream_t stream, short int incode);
void ps_stream_dump (unsigned char *buffer, int nx, int ny, int depth, int compress, int encode, int mask);
void ps_a85_encode (unsigned char quad[], PS_LONG nbytes);
void *ps_memory (void *prev_addr, size_t nelem, size_t size);
PS_LONG ps_shorten_path (double *x, double *y, PS_LONG n, PS_LONG *ix, PS_LONG *iy);
int ps_comp_int_asc (const void *p1, const void *p2);
static void ps_bulkcopy (const char *fname, const char *version);
static void ps_init_fonts (int *n_fonts, int *n_GMT_fonts);
int ps_pattern_init(int image_no, char *imagefile);
void ps_rgb_to_cmyk_char (unsigned char rgb[], unsigned char cmyk[]);
void ps_rgb_to_cmyk_int (int rgb[], int cmyk[]);
void ps_rgb_to_cmyk (int rgb[], double cmyk[]);
void ps_rgb_to_hsv (int rgb[], double hsv[]);
void ps_cmyk_to_rgb (int rgb[], double cmyk[]);
int ps_place_color (int rgb[]);
void ps_place_setdash (char *pattern, int offset);
void ps_set_length_array (char *param, double *array, PS_LONG n);
PS_LONG ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, PS_LONG *node, PS_LONG n, PS_LONG m);
void ps_set_txt_array (char *param, char *array[], PS_LONG n);
void ps_set_integer (char *param, PS_LONG value);
void ps_set_real_array (char *param, double *array, PS_LONG n);
indexed_image_t ps_makecolormap (unsigned char *buffer, int nx, int ny, int nbits);
int ps_bitreduce (unsigned char *buffer, int nx, int ny, int ncolors);
int ps_bitimage_cmap (int f_rgb[], int b_rgb[]);
void ps_colorimage_rgb (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits);
void ps_colorimage_cmap (double x, double y, double xsize, double ysize, indexed_image_t image, int nx, int ny, int nbits);
unsigned char *ps_load_raster (FILE *fp, struct imageinfo *header);
unsigned char *ps_load_eps (FILE *fp, struct imageinfo *header);
int ps_get_boundingbox (FILE *fp, PS_LONG *llx, PS_LONG *lly, PS_LONG *trx, PS_LONG *try);
char *ps_getsharepath (const char *subdir, const char *stem, const char *suffix, char *path);
int ps_pattern (int image_no, char *imagefile, int invert, int image_dpi, int outline, int f_rgb[], int b_rgb[]);


/*------------------- PUBLIC PSLIB FUNCTIONS--------------------- */


void ps_arc (double x, double y, double radius, double az1, double az2, int status)
{	/* 1 = set anchor, 2 = set end, 3 = both */
	PS_LONG ix, iy, ir;

	ix = (PS_LONG)irint (x * PSL->internal.scale);
	iy = (PS_LONG)irint (y * PSL->internal.scale);
	ir = (PS_LONG)irint (radius * PSL->internal.scale);
	if (fabs (az1 - az2) > 360.0) az1 = 0.0, az2 = 360.0;
	if (status%2) {	/* Beginning of new segment */
		fprintf (PSL->internal.fp, "S ");
		PSL->internal.npath = 0;
	}
	else
		PSL->internal.npath++;
	if (az1 < az2)	/* Forward positive arc */
		fprintf (PSL->internal.fp, "%ld %ld %ld %g %g arc", ix ,iy, ir, az1, az2);
	else	/* Negative arc */
		fprintf (PSL->internal.fp, "%ld %ld %ld %g %g arcn", ix ,iy, ir, az1, az2);
	if (status > 1)	fprintf (PSL->internal.fp, " S");
	fprintf (PSL->internal.fp, "\n");
}

/* fortran interface */
void ps_arc_ (double *x, double *y, double *radius, double *az1, double *az2, int *status)
{
	 ps_arc (*x, *y, *radius, *az1, *az2, *status);
}

void ps_axis (double x, double y, double length, double val0, double val1, double annotation_int, char *label, double annotpointsize, int side)
{
	int annot_justify, label_justify, i, j, ndig = 0;
	int left = FALSE;
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
		sprintf (format, "%%.%df", ndig);
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
void ps_axis_ (double *x, double *y, double *length, double *val0, double *val1, double *annotation_int, char *label, double *annotpointsize, int *side, int nlen)
{
	ps_axis (*x, *y, *length, *val0, *val1, *annotation_int, label, *annotpointsize, *side);
}

void ps_bitimage (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int invert, int f_rgb[], int b_rgb[])
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
	PS_LONG lx, ly;
	int inv;
	char *kind[2] = {"Binary", "Ascii"};

	lx = (PS_LONG)irint (xsize * PSL->internal.scale);
	ly = (PS_LONG)irint (ysize * PSL->internal.scale);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe 1-bit image\n", kind[PSL->internal.ascii]);
	fprintf (PSL->internal.fp, "V N %g %g T %ld %ld scale", x * PSL->internal.scale, y * PSL->internal.scale, lx, ly);
	inv = (ps_bitimage_cmap (f_rgb, b_rgb) + invert) % 2;
	fprintf (PSL->internal.fp, "\n<< /ImageType 1 /Decode [%d %d] ", inv, 1-inv);
	ps_stream_dump (buffer, nx, ny, 1, PSL->internal.compress, PSL->internal.ascii, (int)(f_rgb[0] < 0 || b_rgb[0] < 0));

	fprintf (PSL->internal.fp, "U\n");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of %s Abobe 1-bit image\n", kind[PSL->internal.ascii]);
}

/* fortran interface */
void ps_bitimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, int *nx, int *ny, int *invert, int *f_rgb, int *b_rgb)
{
	ps_bitimage (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *invert, f_rgb, b_rgb);
}

void ps_circle (double x, double y, double size, int rgb[], int outline)
{
	PS_LONG ix, iy, ir;
	int pmode;

	/* size is assumed to be diameter */

	ix = (PS_LONG)irint (x * PSL->internal.scale);
	iy = (PS_LONG)irint (y * PSL->internal.scale);
	ir = (PS_LONG)irint (0.5 * size * PSL->internal.scale);
	fprintf (PSL->internal.fp, "N ");
	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld C%c\n", ix, iy, ir, PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_circle_ (double *x, double *y, double *size, int *rgb, int *outline)
{
	 ps_circle (*x, *y, *size, rgb, *outline);
}

void ps_clipoff (void) {
	fprintf (PSL->internal.fp, "S U\n");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Clipping is currently OFF\n");
	PSL->internal.npath = PSL->internal.clip_path_length = 0;
	PSL->current.rgb[0] = PSL->current.rgb[1] = PSL->current.rgb[2] = -1;	/* Reset to -1 so ps_setpaint will update the current paint */
	PSL->current.linewidth = -1;			/* Reset to -1 so ps_setline will update the current width */
	PSL->current.offset = -1;				/* Reset to -1 so ps_setdash will update the current pattern */
}

/* fortran interface */
void ps_clipoff_ (void) {
	ps_clipoff ();
}

void ps_clipon (double *x, double *y, PS_LONG n, int rgb[], int flag)
{
	/* Path length */
	/* Optional paint (-1 to avoid paint) */
	/* combo of 1 | 2. 1 = Start, 2 = end */
	/* Any plotting outside the path defined by x,y will be clipped.
	   use clipoff to restore the original clipping path. */

	PS_LONG used;
	int pmode;
	char move[7];

	if (flag & 1) {	/* First segment in (possibly multi-segmented) clip-path */
		strcpy (move, "M");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of clip path\n");
		fprintf (PSL->internal.fp, "S V\n");
		PSL->internal.npath = 0;
	}
	else
		strcpy (move, "m");

	used = 0;
	if (n > 0) {
		PSL->internal.ix = (PS_LONG)irint (x[0]*PSL->internal.scale);
		PSL->internal.iy = (PS_LONG)irint (y[0]*PSL->internal.scale);
		PSL->internal.npath++;
		used++;
		fprintf (PSL->internal.fp, "%ld %ld %s\n", PSL->internal.ix, PSL->internal.iy, move);
		used += ps_line (&x[1], &y[1], n-1, 0, FALSE, FALSE);	/* Must pass close = FALSE since first point not given ! */
		fprintf (PSL->internal.fp, "P\n");
	}
	PSL->internal.clip_path_length += used;
	PSL->internal.max_path_length = MAX (PSL->internal.clip_path_length, PSL->internal.max_path_length);

	if (flag & 2) {	/* End path and [optionally] fill */
		if (rgb[0] >= 0) {	/* fill is desired */
			fprintf (PSL->internal.fp, "V ");
			pmode = ps_place_color (rgb);
			fprintf (PSL->internal.fp, "%c eofill U ", PSL->internal.paint_code[pmode]);
		}
		if (flag & 4)
			fprintf (PSL->internal.fp, "eoclip\n");
		else
			fprintf (PSL->internal.fp, "eoclip N\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of clip path.  Clipping is currently ON\n");
		PSL->internal.npath = 0;
	}
}

/* fortran interface */
void ps_clipon_ (double *x, double *y, PS_LONG *n, int *rgb, int *flag)
{
	ps_clipon (x, y, *n, rgb, *flag);
}

void ps_colorimage (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits)
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
	 * nx < 0	: 24-bit image contains a color mask (first 3 bytes)
	 * nbits < 0	: "Hardware" interpolation requested
	 */
	PS_LONG lx, ly;
	int id, it;
	char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	char *kind[2] = {"Binary", "Ascii"};				/* What encoding to use */
	char *type[3] = {"1", "4 /MaskColor[0]", "1 /Interpolate true"};
	indexed_image_t image;

	lx = (PS_LONG)irint (xsize * PSL->internal.scale);
	ly = (PS_LONG)irint (ysize * PSL->internal.scale);
	id = ((PSL->internal.color_mode & PSL_CMYK) && abs(nbits) == 24) ? 2 : ((abs(nbits) == 24) ? 1 : 0);
	it = (nx < 0 && abs(nbits) == 24) ? 1 : (nbits < 0 ? 2 : 0);	/* Colormask or interpolate */

	if ((image = ps_makecolormap (buffer, nx, ny, nbits))) {
		/* Creation of colormap was successful */
		nbits = ps_bitreduce (image->buffer, nx, ny, image->colormap->ncolors);

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe Indexed %s image [%d bit]\n", kind[PSL->internal.ascii], colorspace[id], nbits);
		fprintf (PSL->internal.fp, "V N %g %g T %ld %ld scale [/Indexed /Device%s %d <\n", x * PSL->internal.scale, y * PSL->internal.scale, lx, ly, colorspace[id], image->colormap->ncolors - 1);
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
		/* Export full RGB or CMYK image */
		nbits = abs(nbits);

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Start of %s Adobe %s image [%d bit]\n", kind[PSL->internal.ascii], colorspace[id], nbits);
		fprintf (PSL->internal.fp, "V N %g %g T %ld %ld scale /Device%s setcolorspace", x * PSL->internal.scale, y * PSL->internal.scale, lx, ly, colorspace[id]);

		if (it == 1) {	/* Do PS Level 3 image type 4 with colormask */
			fprintf (PSL->internal.fp, "\n<< /ImageType 4 /MaskColor[%d %d %d]", (int)buffer[0], (int)buffer[1], (int)buffer[2]);
			buffer += 3;
		}
		else		/* Do PS Level 2 image, optionally with interpolation */
			fprintf (PSL->internal.fp, "\n<< /ImageType %s", type[it]);

		fprintf (PSL->internal.fp, " /Decode [%s] ", decode[id]);
		ps_stream_dump (buffer, nx, ny, nbits, PSL->internal.compress, PSL->internal.ascii, 0);
		fprintf (PSL->internal.fp, "U\n\n");
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of %s Adobe %s image\n", kind[PSL->internal.ascii], colorspace[id]);
	}
}

/* fortran interface */
void ps_colorimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, int *nx, int *ny, int *nbits, int nlen)
{
	ps_colorimage (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *nbits);
}

void ps_colortiles (double x0, double y0, double xsize, double ysize, unsigned char *image, int nx, int ny)
{
	/* Plots image as many colored rectangles
	 * x0, y0	: Lower left corner in inches
	 * xsize, ysize	: Size of image in inches
	 * image	: color image with rgb triplets per pixel
	 * nx, ny	: image size in pixels
	 */
	int i, j, k, rgb[3];
	double x1, x2, y1, y2, dx, dy, noise, noise2;

	nx = abs(nx);
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
void ps_colortiles_ (double *x0, double *y0, double *xsize, double *ysize, unsigned char *image, int *nx, int *ny, int nlen)
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

void ps_cross (double x, double y, double diameter)
{	/* Fit inside circle of given diameter; draw using current color */
	fprintf (PSL->internal.fp, "%ld %ld %ld X\n", (PS_LONG) irint (diameter * PSL->internal.scale), (PS_LONG) irint ((x - 0.5 * diameter) * PSL->internal.scale), (PS_LONG ) irint (y * PSL->internal.scale));
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_cross_ (double *x, double *y, double *diameter)
{
	ps_cross (*x, *y, *diameter);
}

void ps_point (double x, double y, double diameter)
{     /* Fit inside circle of given diameter; draw using current color */
	fprintf (PSL->internal.fp, "%ld %ld %ld O\n", (PS_LONG)irint (diameter * PSL->internal.scale), (PS_LONG)irint (x * PSL->internal.scale), (PS_LONG)irint (y * PSL->internal.scale));
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_point_ (double *x, double *y, double *diameter)
{
	ps_point (*x, *y, *diameter);
}

void ps_diamond (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld D%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_diamond_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_diamond (*x, *y, *diameter, rgb, *outline);
}

void ps_segment (double x0, double y0, double x1, double y1)
{	/* Short line segment */
	PS_LONG ix, iy, dx, dy;

	ix = (PS_LONG)irint (x0 * PSL->internal.scale);
	iy = (PS_LONG)irint (y0 * PSL->internal.scale);
	dx = (PS_LONG)irint (x1 * PSL->internal.scale) - ix;
	dy = (PS_LONG)irint (y1 * PSL->internal.scale) - iy;
	fprintf (PSL->internal.fp, "%ld %ld M %ld %ld D S\n", ix, iy, dx, dy);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_segment_ (double *x0, double *y0, double *x1, double *y1)
{
	 ps_segment (*x0, *y0, *x1, *y1);
}

void ps_star (double x, double y, double diameter, int rgb[], int outline)
{	/* Fit inside circle of given diameter */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld A%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_star_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_star (*x, *y, *diameter, rgb, *outline);
}

void ps_square (double x, double y, double diameter, int rgb[], int outline)
{	/* give diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld S%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_square_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_square (*x, *y, *diameter, rgb, *outline);
}

void ps_triangle (double x, double y, double diameter, int rgb[], int outline)
{	/* Give diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld T%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_triangle_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_triangle (*x, *y, *diameter, rgb, *outline);
}

void ps_itriangle (double x, double y, double diameter, int rgb[], int outline)	/* Inverted triangle */
{	/* Give diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld I%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_itriangle_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_itriangle (*x, *y, *diameter, rgb, *outline);
}

void ps_hexagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld H%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_hexagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_hexagon (*x, *y, *diameter, rgb, *outline);
}

void ps_pentagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld N%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_pentagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_pentagon (*x, *y, *diameter, rgb, *outline);
}

void ps_octagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int pmode;

	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld O%c\n", (PS_LONG)irint(0.5 * diameter * PSL->internal.scale), (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_octagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_octagon (*x, *y, *diameter, rgb, *outline);
}

void ps_pie (double x, double y, double radius, double az1, double az2, int rgb[], int outline)
{
	PS_LONG ix, iy, ir;
	int pmode;

	ix = (PS_LONG)irint (x * PSL->internal.scale);
	iy = (PS_LONG)irint (y * PSL->internal.scale);
	ir = (PS_LONG)irint (radius * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld %ld M ", ix, iy);
	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %ld %g %g W%c\n", ix, iy, ir, az1, az2, PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_pie_ (double *x, double *y, double *radius, double *az1, double *az2, int *rgb, int *outline)
{
	 ps_pie (*x, *y, *radius, *az1, *az2, rgb, *outline);
}

void ps_ellipse (double x, double y, double angle, double major, double minor, int rgb[], int outline)
{
	PS_LONG ix, iy, ir;
	int pmode;
	double aspect;

	/* Feature: Pen thickness also affected by aspect ratio */

	ix = (PS_LONG)irint (x * PSL->internal.scale);
	iy = (PS_LONG)irint (y * PSL->internal.scale);
	fprintf (PSL->internal.fp, "V %ld %ld T", ix, iy);
	if (angle != 0.0) fprintf (PSL->internal.fp, " %g R", angle);
	aspect = minor / major;
	fprintf (PSL->internal.fp, " 1 %g scale\n", aspect);
	ir = (PS_LONG)irint (major * PSL->internal.scale);
	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "0 0 %ld C%c U\n", ir, PSL->internal.paint_code[pmode+outline]);
}

/* fortran interface */
void ps_ellipse_ (double *x, double *y, double *angle, double *major, double *minor, int *rgb, int *outline)
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

void ps_image (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits)
{	/* Backwards compatibility */
	ps_colorimage (x, y, xsize, ysize, buffer, nx, ny, nbits);
}

/* fortran interface */
void ps_image_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, int *nx, int *ny, int *nbits, int nlen)
{
	ps_image (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *nbits);
}

void ps_pattern_cleanup (void) {
	int image_no;

	for (image_no = 0; image_no < PSL_N_PATTERNS * 2; image_no++) {
		if (PSL->internal.pattern[image_no].status) {
			fprintf (PSL->internal.fp, "currentdict /image%d undef\n", image_no);
			fprintf (PSL->internal.fp, "currentdict /pattern%d undef\n", image_no);
		}
	}
}

/* fortran interface */
int ps_pattern_ (int *image_no, char *imagefile, int *invert, int *image_dpi, int *outline, int *f_rgb, int *b_rgb, int nlen)
{
	 return (ps_pattern (*image_no, imagefile, *invert, *image_dpi, *outline, f_rgb, b_rgb));
}

int ps_pattern (int image_no, char *imagefile, int invert, int image_dpi, int outline, int f_rgb[], int b_rgb[])
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

	BOOLEAN found;
	int i, id, inv, refresh;
	PS_LONG nx, ny;
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

	id = (PSL->internal.color_mode & PSL_CMYK) ? 2 : 1;
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

		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Setup %s fill using pattern %d\n", name, image_no);
		if (image_dpi) {	/* Use given DPI */
			nx = (PS_LONG)irint (nx * PSL->internal.scale / image_dpi);
			ny = (PS_LONG)irint (ny * PSL->internal.scale / image_dpi);
		}
		fprintf (PSL->internal.fp, "/pattern%d {V %ld %ld scale", image_no, nx, ny);
		fprintf (PSL->internal.fp, "\n<< /PaintType 1 /PatternType 1 /TilingType 1 /BBox [0 0 1 1] /XStep 1 /YStep 1 /PaintProc\n   {begin");

		if (PSL->internal.pattern[image_no].depth == 1) {	/* 1-bit bitmap basis */
			inv = (ps_bitimage_cmap (f_rgb, b_rgb) + invert) % 2;
			fprintf (PSL->internal.fp, "\n<< /ImageType 1 /Decode [%d %d]", inv, 1-inv);
		}
		else
			fprintf (PSL->internal.fp, " /Device%s setcolorspace\n<< /ImageType 1 /Decode [%s]", colorspace[id], decode[id]);
		fprintf (PSL->internal.fp, " /Width %ld /Height %ld /BitsPerComponent %d", PSL->internal.pattern[image_no].nx, PSL->internal.pattern[image_no].ny, MIN(PSL->internal.pattern[image_no].depth,8));
		fprintf (PSL->internal.fp, "\n   /ImageMatrix [%ld 0 0 %ld 0 %ld] /DataSource image%d\n>> %s end}\n>> matrix makepattern U} def\n", PSL->internal.pattern[image_no].nx, -PSL->internal.pattern[image_no].ny, PSL->internal.pattern[image_no].ny, image_no, name);

		PSL->internal.pattern[image_no].dpi = image_dpi;
		for (i = 0; i < 3; i++) {
			PSL->internal.pattern[image_no].f_rgb[i] = (invert) ? b_rgb[i] : f_rgb[i];
			PSL->internal.pattern[image_no].b_rgb[i] = (invert) ? f_rgb[i] : b_rgb[i];
		}
	}

	return (image_no);
}

int ps_pattern_init (int image_no, char *imagefile)
{
	int i;
	char name[BUFSIZ], file[BUFSIZ];
	unsigned char *picture;
	struct imageinfo h;
	BOOLEAN found;

	if ((image_no >= 0 && image_no < PSL_N_PATTERNS) && PSL->internal.pattern[image_no].status) return (image_no);	/* Already done this */

	if ((image_no >= 0 && image_no < PSL_N_PATTERNS)) {	/* Premade pattern yet not used */
		sprintf (name, "ps_pattern_%2.2d", image_no);
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

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%%\n%% Define pattern %d\n%%\n", image_no);

	fprintf (PSL->internal.fp, "/image%d {<~\n", image_no);
	ps_stream_dump (picture, h.width, h.height, h.depth, PSL->internal.compress, 1, 2);
	fprintf (PSL->internal.fp, "} def\n");

	ps_free ((void *)picture);

	return (image_no);
}

/* fortran interface */
void ps_epsimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, PS_LONG size, int *nx, int *ny, PS_LONG *ox, PS_LONG *oy, int nlen)
{
	ps_epsimage (*x, *y, *xsize, *ysize, buffer, size, *nx, *ny, *ox, *oy);
}

void ps_epsimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PS_LONG size, int nx, int ny, PS_LONG ox, PS_LONG oy)
{
	/* Plots an EPS image
	 * x,y:		Position of image (in inches)
	 * xsize,ysize:	Size of image (in inches)
	 * buffer:	EPS file (buffered)
	 * size:	Number of bytes in buffer
	 * nx,ny:	Size of image (in pixels)
	 * ox,oy:	Coordinates of lower left corner (in pixels)
	 */

	 fprintf (PSL->internal.fp, "V N %g %g T %g %g scale\n", x * PSL->internal.scale, y * PSL->internal.scale, xsize * PSL->internal.scale / nx, ysize * PSL->internal.scale / ny);
	 fprintf (PSL->internal.fp, "%ld %ld T\n", -ox, -oy);
	 fprintf (PSL->internal.fp, "N %ld %ld m %ld %ld L %ld %ld L %ld %ld L P clip N\n", ox, oy, ox+nx, oy, ox+nx, oy+ny, ox, oy+ny);
	 fprintf (PSL->internal.fp, "countdictstack\nmark\n/showpage {} def\n");
	 if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Start of imported EPS file\n");
	 fprintf (PSL->internal.fp, "%%%%BeginDocument: psimage.eps\n");
	 fwrite (buffer, (size_t)1, (size_t)size, PSL->internal.fp);
	 fprintf (PSL->internal.fp, "%%%%EndDocument\n");
	 if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% End of imported EPS file\n");
	 fprintf (PSL->internal.fp, "cleartomark\ncountdictstack exch sub { end } repeat\ngrestore\n");
}

PS_LONG ps_line (double *x, double *y, PS_LONG n, int type, int close, int split)
{
	/* type:  1 means new anchor point, 2 means stroke line, 3 = both */
	/* close: TRUE if a closed polygon */
	/* split: TRUE if we can split line segment into several sections */
	PS_LONG i, *ix, *iy;
	int trim = FALSE;
	char move = 'M';

	PSL->internal.split = 0;	/* No splitting yet... */

	/* First remove unnecessary points that have zero curvature */

	ix = (PS_LONG *) ps_memory (VNULL, (size_t)n, sizeof (PS_LONG));
	iy = (PS_LONG *) ps_memory (VNULL, (size_t)n, sizeof (PS_LONG));

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
		PSL->internal.npath = 1;
	}
	else if (ix[0] != PSL->internal.ix || iy[0] != PSL->internal.iy)
		fprintf (PSL->internal.fp, "%ld %ld D\n", ix[0] - PSL->internal.ix, iy[0] - PSL->internal.iy);
	PSL->internal.ix = ix[0];
	PSL->internal.iy = iy[0];

	if (!split) PSL->internal.max_path_length = MAX ((n + PSL->internal.clip_path_length), PSL->internal.max_path_length);

	for (i = 1; i < n; i++) {
		if (ix[i] != PSL->internal.ix || iy[i] != PSL->internal.iy) fprintf (PSL->internal.fp, "%ld %ld D\n", ix[i] - PSL->internal.ix, iy[i] - PSL->internal.iy);
		PSL->internal.ix = ix[i];
		PSL->internal.iy = iy[i];
		PSL->internal.npath++;
		if ((PSL->internal.npath + PSL->internal.clip_path_length) > PSL_MAX_L1_PATH && split) {
			fprintf (PSL->internal.fp, "S %ld %ld M\n", PSL->internal.ix, PSL->internal.iy);
			PSL->internal.npath = 1;
			PSL->internal.split = 1;
			close = FALSE;
			if (trim) {	/* Restore the duplicate point since close no longer is TRUE */
				n++;
				trim = FALSE;
			}
		}
	}
	if (type > 1) {
		if (close)
			fprintf (PSL->internal.fp, "P S\n");	/* Close and stroke the path */
		else
			fprintf (PSL->internal.fp, "S\n");	/* Stroke the path */
		PSL->internal.npath = 0;
	}
	else if (close)
		fprintf (PSL->internal.fp, "P\n");		/* Close the path */

	ps_free ((void *)ix);
	ps_free ((void *)iy);

	return (n);
}

/* fortran interface */
void ps_line_ (double *x, double *y, PS_LONG *n, int *type, int *close, int *split)
{
	ps_line (x, y, *n, *type, *close, *split);
}

PS_LONG ps_shorten_path (double *x, double *y, PS_LONG n, PS_LONG *ix, PS_LONG *iy)
{
	/* Simplifies the (x,y) array by converting it to pixel coordinates (ix,iy)
	 * and eliminating repeating points and intermediate points along straight
	 * line segments.  The result is the fewest points needed to draw the path
	 * and still look exactly like the original path. */

	double old_slope = 1.0e200, new_slope;
	PS_LONG i, k, dx, dy;
	int old_dir = 0, new_dir;
	/* These seeds for old_slope and old_dir make sure that first point gets saved */

	if (n < 2) return (n);	/* Not a path to start with */

	for (i = 0; i < n; i++) {	/* Convert all coordinates to integers at current scale */
		ix[i] = (PS_LONG)irint (x[i] * PSL->internal.scale);
		iy[i] = (PS_LONG)irint (y[i] * PSL->internal.scale);
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
	if (k < 1) return ((PS_LONG)1);

	/* Last point (k cannot be < 1 so k-1 >= 0) */
	if (x[k-1] != ix[n-1] || iy[k-1] != iy[n-1]) {	/* Do not do slope check on last point since we must end there */
		ix[k] = ix[n-1];
		iy[k] = iy[n-1];
		k++;
	}
	return (k);
}

/* fortran interface */
void ps_shorten_path_ (double *x, double *y, PS_LONG *n, PS_LONG *ix, PS_LONG *iy)
{
	ps_shorten_path (x, y, *n, ix, iy);
}

void ps_plot (double x, double y, int pen)
{
	PS_LONG ix, iy, idx, idy;

	ix = (PS_LONG)irint (x*PSL->internal.scale);
	iy = (PS_LONG)irint (y*PSL->internal.scale);
	if (abs (pen) == 2) {	/* Convert absolute draw to relative draw */
		idx = ix - PSL->internal.ix;
		idy = iy - PSL->internal.iy;
		if (idx == 0 && idy == 0) return;
		fprintf (PSL->internal.fp, "%ld %ld D\n", idx, idy);
		PSL->internal.npath++;
	}
	else {
		idx = ix;
		idy = iy;
		fprintf (PSL->internal.fp, "%ld %ld M\n", idx, idy);
		PSL->internal.npath = 1;
	}
	if (pen == PSL_PEN_DRAW_AND_STROKE) fprintf (PSL->internal.fp, "S\n");
	PSL->internal.ix = ix;
	PSL->internal.iy = iy;
	if ((PSL->internal.npath + PSL->internal.clip_path_length) > PSL_MAX_L1_PATH) {
		fprintf (PSL->internal.fp, "S %ld %ld M\n", ix, iy);
		PSL->internal.npath = 1;
	}
}

/* fortran interface */
void ps_plot_ (double *x, double *y, int *pen)
{
	ps_plot (*x, *y, *pen);
}

void ps_plotend (int lastpage)
{
	int i;

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
			fprintf (PSL->internal.fp, "%%%%BoundingBox: %d %d %d %d\n", irint(x0), irint(y0), irint(x1), irint(y1));
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
void ps_plotend_ (int *lastpage)
{
	ps_plotend (*lastpage);
}

int ps_plotinit_hires (char *plotfile, int overlay, int mode, double xoff, double yoff, double xscl, double yscl, int ncopies, int dpi, int unit, double *page_size, int *rgb, const char *encoding, struct EPS *eps)
/* plotfile:	Name of output file or NULL for standard output
   xoff, yoff:	Sets a new origin relative to old
   xscl, yscl:	Global scaling, usually left to 1,1
   page_size:	Physical width and height of paper used in points
   overlay:	FALSE means print headers and macros first
   mode:	     bit 0 : 0 = Landscape, 1 = Portrait,
		     bit 1 : 0 = be silent, 1 = be verbose
		     bit 2 : 0 = bin image, 1 = hex image
		     bit 3 : 0 = rel positions, 1 = abs positions
		  bit 9-10 : 0 = RGB color, 1 = CMYK color, 2 = HSV color
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
	int i, pmode, manual = FALSE, n_GMT_fonts;
	int no_rgb[3] = {-1, -1, -1};
	time_t right_now;
	char openmode[2], *this;
	char paint_code[12] = {'Q', 'Q', 'A', 'a', 'C', 'c', 'K', 'k', 'H', 'h', 'I', 'i'};
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
	memcpy ((void *)PSL->init.eps, (void *)eps,sizeof(struct EPS));
	PSL->init.eps->name = (char *) ps_memory (VNULL, (size_t)(strlen (eps->name) + 1), sizeof (char));
	strcpy (PSL->init.eps->name, eps->name);
	PSL->init.eps->title = (char *) ps_memory (VNULL, (size_t)(strlen (eps->title) + 1), sizeof (char));
	strcpy (PSL->init.eps->title, eps->title);
		
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
	strcpy (PSL->current.bw_format, "%.3lg ");			/* Default format used for grayshade value */
	strcpy (PSL->current.rgb_format, "%.3lg %.3lg %.3lg ");		/* Same, for RGB triplets */
	strcpy (PSL->current.hsv_format, "%.3lg %.3lg %.3lg ");		/* Same, for HSV triplets */
	strcpy (PSL->current.cmyk_format, "%.3lg %.3lg %.3lg %.3lg ");	/* Same, for CMYK quadruples */
	memcpy ((void *)PSL->internal.paint_code, (void *)paint_code, 12L);

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
		fprintf (PSL->internal.fp, "%%%%LanguageLevel: 1\n");
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
		ps_bulkcopy ("PSL_prologue", "v 1.17 ");
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
		if (!PSL->internal.eps_format && ncopies > 1) fprintf (PSL->internal.fp, "/#copies %d def\n", ncopies);
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
			pmode = ps_place_color (rgb);
			fprintf (PSL->internal.fp, "%c F N\n", PSL->internal.paint_code[pmode]);
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

/* Original ps_ploitinit used ints for paper size */

int ps_plotinit (char *plotfile, int overlay, int mode, double xoff, double yoff, double xscl, double yscl, int ncopies, int dpi, int unit, int *page_size, int *rgb, const char *encoding, struct EPS *eps)
{
	double d_page_size[2];
	d_page_size[0] = (double)page_size[0];
	d_page_size[1] = (double)page_size[1];
	return (ps_plotinit_hires (plotfile, overlay, mode, xoff, yoff, xscl, yscl, ncopies, dpi, unit, d_page_size, rgb, encoding, eps));
}

/* fortran interface */
void ps_plotinit_ (char *plotfile, int *overlay, int *mode, double *xoff, double *yoff, double *xscl, double *yscl, int *ncopies, int *dpi, int *unit, int *page_size, int *rgb, const char *encoding, int nlen1, int nlen2)
{
	 ps_plotinit (plotfile, *overlay, *mode, *xoff, *yoff, *xscl, *yscl, *ncopies, *dpi, *unit, page_size, rgb, encoding, (struct EPS *)NULL);
}

void ps_setlinecap (int cap)
{
	fprintf (PSL->internal.fp, "%d setlinecap\n", cap);
}

/* fortran interface */
void ps_setlinecap_ (int *cap)
{
	ps_setlinecap (*cap);
}

void ps_setlinejoin (int join)
{
	fprintf (PSL->internal.fp, "%d setlinejoin\n", join);
}

/* fortran interface */
void ps_setlinejoin_ (int *join)
{
	ps_setlinejoin (*join);
}

void ps_setmiterlimit (int limit)
{
	double miter;
	miter = (limit == 0) ? 10.0 : 1.0 / sin (0.5 * limit * D2R);
	fprintf (PSL->internal.fp, "%g setmiterlimit\n", miter);
}

/* fortran interface */
void ps_setmiterlimit_ (int *limit)
{
	ps_setmiterlimit (*limit);
}

void ps_plotr (double x, double y, int pen)
{
	PS_LONG ix, iy;

	ix = (PS_LONG)irint (x * PSL->internal.scale);
	iy = (PS_LONG)irint (y * PSL->internal.scale);
	if (ix == 0 && iy == 0) return;
	PSL->internal.npath++;
	if (abs (pen) == 2)
		fprintf (PSL->internal.fp, "%ld %ld D\n", ix, iy);
	else {
		fprintf (PSL->internal.fp, "%ld %ld G\n", ix, iy);
		PSL->internal.npath = 1;
	}
	if (pen == PSL_PEN_DRAW_AND_STROKE) fprintf (PSL->internal.fp, "S\n");
	PSL->internal.ix += ix;	/* Update absolute position */
	PSL->internal.iy += iy;
}

/* fortran interface */
void ps_plotr_ (double *x, double *y, int *pen)
{
	ps_plotr (*x, *y, *pen);
}

void ps_polygon (double *x, double *y, PS_LONG n, int rgb[], int outline)
{
	/* Draw and optionally fill polygons. */
	int split, pmode;

	split = (rgb[0] == -1);	/* Can only split if we need outline only */
	if (outline >= 0) ps_line (x, y, n, 1, FALSE, split);	/* No stroke or close path yet */
	PSL->internal.npath = 0;

	PSL->internal.max_path_length = MAX ((n + PSL->internal.clip_path_length), PSL->internal.max_path_length);

	if (split && PSL->internal.split == 1)	/* Outline only */
		fprintf (PSL->internal.fp, "S\n");
	else {
		pmode = ps_place_color (rgb);
		if (outline > 0) pmode += outline;
		fprintf (PSL->internal.fp, "Q%c\n", PSL->internal.paint_code[pmode]);
	}
	if (outline < 0) {
		if (outline == -1) {
			fprintf (PSL->internal.fp, "N U\n");
			if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% Clipping is currently OFF\n");
		}
		PSL->internal.clip_path_length = 0;
	}
}

/* fortran interface */
void ps_polygon_ (double *x, double *y, PS_LONG *n, int *rgb, int *outline)
{
	ps_polygon (x, y, *n, rgb, *outline);
}

void ps_patch (double *x, double *y, PS_LONG np, int rgb[], int outline)
{
	/* Like ps_polygon but intended for small polygons (< 20 points).  No checking for
	 * shorter path by calling ps_shorten_path as in ps_polygon.
	 *
	 * Thus, the usage is (with xi,yi being absolute coordinate for point i and dxi
	 * the increment)
	 * from point i to i+1, and r,g,b in the range 0.0-1.0.  Here, n = np-1.
	 *
	 *	        dx_n dy_n ... n x0 y0 qQ	(If rgb[0] < 0 then outline only)
	 *	      r dx_n dy_n ... n x0 y0 qA	(gray shade; use qa for outline)
	 *	  r g b dx_n dy_n ... n x0 y0 qC	(rgb; use qc for outline)
	 *	c m y k dx_n dy_n ... n x0 y0 qK	(cmyk; use qk for outline)
	 *	  h s v dx_n dy_n ... n x0 y0 qH	(hsv; use qh for outline)
	 *	pattern dx_n dy_n ... n x0 y0 qI	(pattern fill; use qi for outline)
	 */

	PS_LONG ix[20], iy[20], i, n, n1;
 	int pmode;

	if (np > 20) {	/* Must call ps_polygon instead */
		ps_polygon (x, y, np, rgb, outline);
		return;
	}

	ix[0] = (PS_LONG)irint (x[0] * PSL->internal.scale);	/* Convert inch to absolute pixel position for start of quadrilateral */
	iy[0] = (PS_LONG)irint (y[0] * PSL->internal.scale);

	for (i = n = 1, n1 = 0; i < (int)np; i++) {	/* Same but check if new point represent a different pixel */
		ix[n] = (PS_LONG)irint (x[i] * PSL->internal.scale);
		iy[n] = (PS_LONG)irint (y[i] * PSL->internal.scale);
		if (ix[n] != ix[n1] || iy[n] != iy[n1]) n++, n1++;
	}
	if (ix[0] == ix[n1] && iy[0] == iy[n1]) n--, n1--;	/* Closepath will do this automatically */

	if (n < 3) return;	/* 2 points or less don't make a polygon */

	pmode = ps_place_color (rgb) + outline;	/* Returns 0-11 */

	n--;
	n1 = n;
	for (i = n - 1; i != -1; i--, n--) fprintf (PSL->internal.fp, "%ld %ld ", ix[n] - ix[i], iy[n] - iy[i]);
	fprintf (PSL->internal.fp, "%ld %ld %ld q%c\n", n1, ix[0], iy[0], PSL->internal.paint_code[pmode]);
}

/* fortran interface */

void ps_patch_ (double *x, double *y, PS_LONG *n, int *rgb, int *outline)
{
	ps_patch (x, y, *n, rgb, *outline);
}

void ps_rect (double x1, double y1, double x2, double y2, int rgb[], int outline)
{
	int pmode;
	PS_LONG xll, yll;
	pmode = ps_place_color (rgb);
	xll = (PS_LONG)irint (x1 * PSL->internal.scale);	/* Get lower left point with minimum round-off */
	yll = (PS_LONG)irint (y1 * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld %ld %ld %ld B%c\n", (PS_LONG)irint(y2 * PSL->internal.scale) - yll, (PS_LONG)irint(x2 * PSL->internal.scale) - xll, xll, yll, PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_rect_ (double *x1, double *y1, double *x2, double *y2, int *rgb, int *outline)
{
	ps_rect (*x1, *y1, *x2, *y2, rgb, *outline);
}

void ps_rotaterect (double x, double y, double angle, double x_len, double y_len, int rgb[], int outline)
{
	int pmode;
	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%ld %ld %g %ld %ld R%c\n", (PS_LONG)irint(y_len * PSL->internal.scale), (PS_LONG)irint(x_len * PSL->internal.scale), angle, (PS_LONG)irint(x * PSL->internal.scale), (PS_LONG)irint(y * PSL->internal.scale), PSL->internal.paint_code[pmode+outline]);
	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_rotaterect_ (double *x1, double *y1, double *angle, double *x2, double *y2, int *rgb, int *outline)
{
	ps_rotaterect (*x1, *y1, *angle, *x2, *y2, rgb, *outline);
}

void ps_rotatetrans (double x, double y, double angle)
{
	int go = FALSE;

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

void ps_setdash (char *pattern, int offset)
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
	PSL->internal.npath = 0;
}

void ps_place_setdash (char *pattern, int offset)
{
	int place_space;
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
		fprintf (PSL->internal.fp, "] %d B", offset);
	}
	else
		fprintf (PSL->internal.fp, "[] 0 B");	/* Reset to continuous line */
}

/* fortran interface */
void ps_setdash_ (char *pattern, int *offset, int nlen)
{
	ps_setdash (pattern, *offset);
}

void ps_setfont (int font_no)
{
	if (font_no < 0 || font_no >= PSL->internal.N_FONTS)
		fprintf (stderr, "pslib: Selected font out of range (%d), ignored\n", font_no);
	else
		PSL->current.font_no = font_no;
}

/* fortran interface */
void ps_setfont_ (int *font_no)
{
	ps_setfont (*font_no);
}

void ps_setformat (int n_decimals)
{
	/* Sets nmber of decimals used for rgb/gray specifications [3] */
	if (n_decimals < 1 || n_decimals > 3)
		fprintf (stderr, "pslib: Selected decimals for color out of range (%d), ignored\n", n_decimals);
	else {
		sprintf (PSL->current.bw_format, "%%.%df ", n_decimals);
		sprintf (PSL->current.rgb_format, "%%.%df %%.%df %%.%df ", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.hsv_format, "%%.%df %%.%df %%.%df ", n_decimals, n_decimals, n_decimals);
		sprintf (PSL->current.cmyk_format, "%%.%df %%.%df %%.%df %%.%df ", n_decimals, n_decimals, n_decimals, n_decimals);
	}
}

/* fortran interface */
void ps_setformat_ (int *n_decimals)
{
	ps_setformat (*n_decimals);
}

void ps_setline (int linewidth)
{
	if (linewidth < 0) {
		fprintf (stderr, "pslib: Selected linewidth is negative (%d), ignored\n", linewidth);
		return;
	}
	if (linewidth == PSL->current.linewidth) return;

	fprintf (PSL->internal.fp, "S %g W\n", (double)(linewidth * 72.0 / PSL->internal.points_pr_unit));
	PSL->current.linewidth = linewidth;
}

/* fortran interface */
void ps_setline_ (int *linewidth)
{
	 ps_setline (*linewidth);
}

void ps_setpaint (int rgb[])
{
	int pmode;

	if (rgb[0] < 0) return;	/* Some rgb's indicate no fill */
	if (rgb[0] == PSL->current.rgb[0] && rgb[1] == PSL->current.rgb[1] && rgb[2] == PSL->current.rgb[2]) return;	/* Same color as already set */

	fprintf (PSL->internal.fp, "S ");
	pmode = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%c\n", PSL->internal.paint_code[pmode]);

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

void ps_textbox (double x, double y, double pointsize, char *text, double angle, int justify, int outline, double dx, double dy, int rgb[])
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
	int i = 0, pmode, j, h_just, v_just, rounded;

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
	ps_textdim ("PSL_dimx", "PSL_dimy", fabs (pointsize), PSL->current.font_no, &text[i], 1);			/* Set the string BB dimensions in PS */
	if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	ps_set_length ("PSL_dx", dx);
	ps_set_length ("PSL_dy", dy);
	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	/* Got to anchor point */

	if (pointsize > 0.0) {	/* Set a new anchor point */
		PSL->internal.ix = (PS_LONG)irint (x * PSL->internal.scale);
		PSL->internal.iy = (PS_LONG)irint (y * PSL->internal.scale);
		fprintf (PSL->internal.fp, "V %ld %ld T ", PSL->internal.ix, PSL->internal.iy);
	}
	else
		fprintf (PSL->internal.fp, "V PSL_save_x PSL_save_y T ");

	if (angle != 0.0) fprintf (PSL->internal.fp, "%.3g R ", angle);
	if (justify > 1) {	/* Move the new origin so (0,0) is lower left of box */
		h_just = (justify + 3) % 4;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (PSL->internal.fp, "PSL_dimx_ur PSL_dimx_ll sub %s ", align[h_just]) : fprintf (PSL->internal.fp, "0 ");
		(v_just) ? fprintf (PSL->internal.fp, "PSL_dimy_ur PSL_dimy_ll sub %s ", align[v_just]) : fprintf (PSL->internal.fp, "0 ");
		fprintf (PSL->internal.fp, "T ");
	}
	/* Here, (0,0) is lower point on textbox with no clearance yet */
	if (rounded) {
		fprintf (PSL->internal.fp, "\n/PSL_r %ld def\n", (PS_LONG)irint (MIN (dx, dy) * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_dx2 %ld def\n", (PS_LONG)irint ((dx - MIN (dx, dy)) * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_dy2 %ld def\n", (PS_LONG)irint ((dy - MIN (dx, dy)) * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_x_side PSL_dimx_ur PSL_dimx_ll sub PSL_dx2 2 mul add def\n");
		fprintf (PSL->internal.fp, "/PSL_y_side PSL_dimy_ur PSL_dimy_ll sub PSL_dy2 2 mul add def\n");
		fprintf (PSL->internal.fp, "/PSL_bx0 PSL_dimx_ll PSL_dx2 sub def\n");
		fprintf (PSL->internal.fp, "/PSL_by0 PSL_dimy_ll PSL_dy2 sub def\n");
		fprintf (PSL->internal.fp, "PSL_dimx_ll PSL_dx2 sub PSL_dimy_ll PSL_dy sub M PSL_x_side 0 D\n");
		fprintf (PSL->internal.fp, "PSL_bx0 PSL_x_side add PSL_by0 PSL_r 270 360 arc\n");
		fprintf (PSL->internal.fp, "0 PSL_y_side D PSL_bx0 PSL_x_side add PSL_by0 PSL_y_side add PSL_r 0 90 arc\n");
		fprintf (PSL->internal.fp, "PSL_x_side neg 0 D PSL_bx0 PSL_by0 PSL_y_side add PSL_r 90 180 arc\n");
		fprintf (PSL->internal.fp, "0 PSL_y_side neg D PSL_bx0 PSL_by0 PSL_r 180 270 arc P\n");
	}
	else {
		fprintf (PSL->internal.fp, "\n/PSL_x_side PSL_dimx_ur PSL_dimx_ll sub PSL_dx 2 mul add def\n");
		fprintf (PSL->internal.fp, "/PSL_y_side PSL_dimy_ur PSL_dimy_ll sub PSL_dy 2 mul add def\n");
		fprintf (PSL->internal.fp, "PSL_dimx_ll PSL_dx sub PSL_dimy_ll PSL_dy sub M PSL_x_side 0 D 0 PSL_y_side D PSL_x_side neg 0 D 0 PSL_y_side neg D P\n");
	}
	if (rgb[0] >= 0) {	/* Paint the textbox */
		fprintf (PSL->internal.fp, "V ");
		pmode = ps_place_color (rgb);
		fprintf (PSL->internal.fp, "%c F U ", PSL->internal.paint_code[pmode]);
	}
	(outline) ? fprintf (PSL->internal.fp, "S U\n") : fprintf (PSL->internal.fp, "N U\n");
	fprintf (PSL->internal.fp, "U\n");
	if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% ps_textbox end:\n\n");

	ps_free ((void *)string);
}

/* fortran interface */
void ps_textbox_ (double *x, double *y, double *pointsize, char *text, double *angle, int *justify, int *outline, double *dx, double *dy, int *rgb, int nlen)
{
	 ps_textbox (*x, *y, *pointsize, text, *angle, *justify, *outline, *dx, *dy, rgb);
}

void ps_textdim (char *xdim, char *ydim, double pointsize, int in_font, char *text, int key)
{
	/* key = 0: Will calculate the exact dimensions (xdim, ydim) of the given text string.
	 * Because of possible escape sequences we need to examine the string
	 * carefully.  The dimensions will be set in PostScript and can be
	 * used by addressing the variables xdim and ydim.
	 * key = 1: Will return bounding box of the text string instead.  Will append
	 * _ll and _ur to the xdim and ydim strings and initialize 4 variables in PS.
	 */

	char *tempstring, *piece, *piece2, *ptr, *string;
	int i = 0, font;
	int sub, super, small, old_font;
	double height, small_size, size, scap_size;

	if (strlen (text) >= (BUFSIZ-1)) {
		fprintf (stderr, "pslib: text_item > %d long!\n", BUFSIZ);
		return;
	}

	ps_setfont (in_font);			/* Switch to the selected font */

	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	height = pointsize / PSL->internal.points_pr_unit;

	if (!strchr (string, '@')) {	/* Plain text string */
		if (key == 0)
			fprintf (PSL->internal.fp, "0 0 M %ld F%d (%s) E /%s exch def YP /%s exch def\n", (PS_LONG) irint (height * PSL->internal.scale), PSL->current.font_no, string, xdim, ydim);
		else
			fprintf (PSL->internal.fp, "0 0 M %ld F%d (%s) FP pathbbox N /%s_ur exch def /%s_ur exch def /%s_ll exch def /%s_ll exch def\n" , (PS_LONG) irint (height * PSL->internal.scale), PSL->current.font_no, string, ydim, xdim, ydim, xdim);
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
		fprintf (PSL->internal.fp, "%ld F%d (%s) FP ", (PS_LONG)irint (size*PSL->internal.scale), font, ptr);
		ptr = strtok ((char *)NULL, "@");
	}

	while (ptr) {
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
			while (*ptr != '%') ptr++;
			ptr++;
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
				while (*ptr != ':') ptr++;
			}
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == ';') {	/* Color change */
			ptr++;
			while (*ptr != ';') ptr++;
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '_') {	/* Small caps toggle */
			ptr++;
			strcpy (piece, ptr);
		}
		else	/* Not recognized or @@ for a single @ */
			strcpy (piece, ptr);
		if (strlen (piece) > 0) fprintf (PSL->internal.fp, "%ld F%d (%s) FP ", (PS_LONG)irint (size*PSL->internal.scale), font, piece);
		ptr = strtok ((char *)NULL, "@");
	}

	fprintf (PSL->internal.fp, "pathbbox N ");
	if (key == 0)
		fprintf (PSL->internal.fp, "exch 2 {3 1 roll sub abs} repeat /%s exch def /%s exch def\n", xdim, ydim);
	else
		fprintf (PSL->internal.fp, "/%s_ur exch def /%s_ur exch def /%s_ll exch def /%s_ll exch def\n", ydim, xdim, ydim, xdim);

	ps_free ((void *)tempstring);
	ps_free ((void *)piece);
	ps_free ((void *)piece2);
	ps_free ((void *)string);
}

void ps_text (double x, double y, double pointsize, char *text, double angle, int justify, int form)
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
	int dy, i = 0, j, font, v_just, h_just, upen, ugap;
	int sub, super, small, old_font, n_uline, start_uline, stop_uline;
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
		ps_textdim ("PSL_dimx", "PSL_dimy", fabs (pointsize), PSL->current.font_no, &text[i], 0);			/* Set the string dimensions in PS */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	}

	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */

	height = fabs (pointsize) / PSL->internal.points_pr_unit;

	PSL->internal.npath = 0;

	if (pointsize > 0.0) {	/* Set a new anchor point */
		PSL->internal.ix = (PS_LONG)irint (x * PSL->internal.scale);
		PSL->internal.iy = (PS_LONG)irint (y * PSL->internal.scale);
		fprintf (PSL->internal.fp, "%ld %ld M ", PSL->internal.ix, PSL->internal.iy);
	}

	if (angle != 0.0) fprintf (PSL->internal.fp, "V %.3g R ", angle);
	if (justify > 1) {
		h_just = (justify % 4) - 1;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (PSL->internal.fp, "PSL_dimx %s ", align[h_just]) : fprintf (PSL->internal.fp, "0 ");
		(v_just) ? fprintf (PSL->internal.fp, "PSL_dimy %s ", align[v_just]) : fprintf (PSL->internal.fp, "0 ");
		fprintf (PSL->internal.fp, "G ");
	}

	if (!strchr (string, '@')) {	/* Plain text string - do things simply and exit */
		fprintf (PSL->internal.fp, "%ld F%d (%s) ", (PS_LONG) irint (height * PSL->internal.scale), PSL->current.font_no, string);
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
		fprintf (PSL->internal.fp, "%ld F%d (%s) %s\n", (PS_LONG)irint (size*PSL->internal.scale), font, ptr, op);
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
			fprintf (PSL->internal.fp, "%ld F%d (%s) dup stringwidth pop exch %s -2 div dup 0 G\n", (PS_LONG)irint (size*PSL->internal.scale), font, piece2, op);
			fprintf (PSL->internal.fp, "%ld F%d (%s) E -2 div dup 0 G exch %s sub neg dup 0 lt {pop 0} if 0 G\n", (PS_LONG)irint (size*PSL->internal.scale), font, piece, op);
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
			fprintf (PSL->internal.fp, "0 %d G\n", dy);
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '+') {	/* Superscript */
			super = !super;
			size = (super) ? small_size : height;
			dy = (super) ? (int)irint (ustep*PSL->internal.scale) : (int)irint (-ustep*PSL->internal.scale);
			fprintf (PSL->internal.fp, "0 %d G\n", dy);
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
			int pmode, n_scan, rgb[3], error = FALSE;
			ptr++;
			if (ptr[0] == ';') {	/* Reset color */
				pmode = ps_place_color (PSL->current.rgb);
				fprintf (PSL->internal.fp, "%c ", PSL->internal.paint_code[pmode]);
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
					pmode = ps_place_color (rgb);
					fprintf (PSL->internal.fp, "%c ", PSL->internal.paint_code[pmode]);
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
		if (stop_uline) fprintf (PSL->internal.fp, "V %d W currentpoint pop /x1_u exch def x0_u y0_u %d sub m x1_u x0_u sub 0 D S x1_u y0_u m U\n", upen, ugap);
		start_uline = stop_uline = FALSE;
		if (strlen (piece) > 0) fprintf (PSL->internal.fp, "%ld F%d (%s) %s\n", (PS_LONG)irint (size*PSL->internal.scale), font, piece, op);
		ptr = strtok ((char *)NULL, "@");
	}
	if (form == 1) fprintf (PSL->internal.fp, "S\n");
	if (angle != 0.0) fprintf (PSL->internal.fp, "U\n");

	ps_free ((void *)piece);
	ps_free ((void *)piece2);
	ps_free ((void *)string);
}

/* fortran interface */
void ps_text_ (double *x, double *y, double *pointsize, char *text, double *angle, int *justify, int *form, int nlen)
{
	ps_text (*x, *y, *pointsize, text, *angle, *justify, *form);
}

void ps_textpath (double x[], double y[], PS_LONG n, PS_LONG node[], double angle[], char *label[], PS_LONG m, double pointsize, double offset[], int justify, int form)
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

	PS_LONG i = 0, j, k;
	int first;

	if (form & 8) {		/* If 8 bit is set we already have placed the info */
		form -= 8;		/* Knock off the 8 flag */
		fprintf (PSL->internal.fp, "%d PSL_curved_text_labels\n", form);
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
	justify =  abs (justify);

	if (first) {	/* Do this only once */
		ps_set_integer ("PSL_just", (PS_LONG)justify);
		ps_set_length ("PSL_gap_x", offset[0]);
		ps_set_length ("PSL_gap_y", offset[1]);
		if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
			if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
			ps_textdim ("PSL_dimx", "PSL_height", fabs (pointsize), PSL->current.font_no, label[0], 0);			/* Set the string dimensions in PS */
			if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
		}
		fprintf (PSL->internal.fp, "%ld F%d\n", (PS_LONG) irint ((fabs (pointsize) / PSL->internal.points_pr_unit) * PSL->internal.scale), PSL->current.font_no);	/* Set font */
	}

	/* Set these each time */

	n = ps_set_xyn_arrays ("PSL_x", "PSL_y", "PSL_node", x, y, node, n, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
	ps_set_integer ("PSL_n", n);
	ps_set_integer ("PSL_m", m);

	fprintf (PSL->internal.fp, "%d PSL_curved_text_labels\n", form);

	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_textpath_ (double x[], double y[], PS_LONG *n, PS_LONG node[], double angle[], char *label[], PS_LONG *m, double *pointsize, double offset[], int *justify, int *form, int len)
{
	ps_textpath (x, y, *n, node, angle, label, *m, *pointsize, offset, *justify, *form);
}

void ps_textclip (double x[], double y[], PS_LONG m, double angle[], char *label[], double pointsize, double offset[], int justify, int key)
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

	PS_LONG i = 0, j, k;

	if (key & 2) {	/* Flag to terminate clipping */
		if (PSL->internal.comments) fprintf (PSL->internal.fp, "%% If clipping is active, terminate it\n");
		fprintf (PSL->internal.fp, "PSL_clip_on {U /PSL_clip_on false def} if\n");
		return;
	}
	if (key & 8) {		/* Flag to place text already define in PSL arrays */
		fprintf (PSL->internal.fp, "%d PSL_straight_text_labels\n", key);
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
	justify =  abs (justify);

	/* fprintf (PSL->internal.fp, "gsave\n"); */
	ps_set_integer ("PSL_m", m);
	ps_set_length_array ("PSL_txt_x", x, m);
	ps_set_length_array ("PSL_txt_y", y, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
	ps_set_integer ("PSL_just", (PS_LONG)justify);
	ps_set_length ("PSL_gap_x", offset[0]);
	ps_set_length ("PSL_gap_y", offset[1]);

	if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "currentpoint /PSL_save_y exch def /PSL_save_x exch def\n");	/* Must save the current point since ps_textdim will destroy it */
		ps_textdim ("PSL_dimx", "PSL_height", fabs (pointsize), PSL->current.font_no, label[0], 0);			/* Set the string dimensions in PS */
		if (pointsize < 0.0) fprintf (PSL->internal.fp, "PSL_save_x PSL_save_y m\n");					/* Reset to the saved current point */
	}

	fprintf (PSL->internal.fp, "%ld F%d\n", (PS_LONG) irint ((fabs (pointsize) / PSL->internal.points_pr_unit) * PSL->internal.scale), PSL->current.font_no);	/* Set font */
	fprintf (PSL->internal.fp, "%d PSL_straight_text_labels\n", key);

	PSL->internal.npath = 0;
}

/* fortran interface */
void ps_textclip_ (double x[], double y[], PS_LONG *m, double angle[], char *label[], double *pointsize, double offset[], int *justify, int *key, int len)
{
	ps_textclip (x, y, *m, angle, label, *pointsize, offset, *justify, *key);
}

void ps_transrotate (double x, double y, double angle)
{
	int go = FALSE;

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

void ps_vector (double xtail, double ytail, double xtip, double ytip, double tailwidth, double headlength, double headwidth, double headshape, int rgb[], int outline)
{
	/* Will make sure that arrow has a finite width in PS coordinates */

	double angle;
	PS_LONG w2, length, hw, hl, hl2, hw2, l2;
	int pmode;

	length = (PS_LONG)irint (hypot ((xtail-xtip), (ytail-ytip)) * PSL->internal.scale);	/* Vector length in PS units */
	if (length == 0) return;					/* NULL vector */

	angle = atan2 ((ytip-ytail),(xtip-xtail)) * R2D;					/* Angle vector makes with horizontal, in radians */
	fprintf (PSL->internal.fp, "V %ld %ld T ", (PS_LONG)irint (xtail * PSL->internal.scale), (PS_LONG)irint (ytail * PSL->internal.scale));	/* Temporarily set tail point the local origin (0, 0) */
	if (angle != 0.0) fprintf (PSL->internal.fp, "%g R ", angle);					/* Rotate so vector is horizontal in local coordinate system */
	w2 = (PS_LONG)irint (0.5 * tailwidth * PSL->internal.scale);	if (w2 == 0) w2 = 1;			/* Half-width of vector tail */
	hw = (PS_LONG)irint (headwidth * PSL->internal.scale);	if (hw == 0) hw = 1;				/* Width of vector head */
	hl = (PS_LONG)irint (headlength * PSL->internal.scale);							/* Length of vector head */
	hl2 = (PS_LONG)irint (0.5 * headshape * headlength * PSL->internal.scale);					/* Cut-in distance due to slanted back-side of arrow head */
	hw2 = hw - w2;										/* Distance from tail side to head side (vertically) */
	if (outline & 8) {	/* Double-headed vector */
		outline -= 8;	/* Remove the flag */
		l2 = length - 2 * hl + 2 * hl2;							/* Inside length between start of heads */
		pmode = ps_place_color (rgb);
		fprintf (PSL->internal.fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld v%c U\n",
				hl2, hw2, -l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -hl2, hw2, hl, -hw, PSL->internal.paint_code[pmode+outline]);
	}
	else {			/* Single-headed vector */
		l2 = length - hl + hl2;								/* Length from tail to start of slanted head */
		pmode = ps_place_color (rgb);
		fprintf (PSL->internal.fp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld V%c U\n",
			-l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -w2, PSL->internal.paint_code[pmode+outline]);
	}
}

/* fortran interface */
void ps_vector_ (double *xtail, double *ytail, double *xtip, double *ytip, double *tailwidth, double *headlength, double *headwidth, double *headshape, int *rgb, int *outline)
{
	 ps_vector (*xtail, *ytail, *xtip, *ytip, *tailwidth, *headlength, *headwidth, *headshape, rgb, *outline);
}

void ps_words (double x, double y, char **text, PS_LONG n_words, double line_space, double par_width, int par_just, int font, double font_size, double angle, int rgb[3], int justify, int draw_box, double x_off, double y_off, double x_gap, double y_gap, int boxpen_width, char *boxpen_texture, int boxpen_offset, int boxpen_rgb[], int vecpen_width, char *vecpen_texture, int vecpen_offset, int vecpen_rgb[], int boxfill_rgb[3])
{
	PS_LONG i, i1, i0, j, k, n, pj;
	PS_LONG n_scan, color, found, last_k = -1;
	int pmode, error = 0, last_font, after, last_rgb[3];
	int *rgb_list, *rgb_unique, n_rgb_unique;
	int *font_list, *font_unique, n_font_unique;
	size_t n_alloc, n_items;
	BOOLEAN sub, super, small, plain_word = FALSE, under, escape;
	char *c, *clean, test_char;
	double last_size;
	struct GMT_WORD **word;
	struct GMT_WORD *add_word_part (char *word, PS_LONG length, int fontno, double font_size, BOOLEAN sub, BOOLEAN super, BOOLEAN small, BOOLEAN under, int space, int rgb[]);

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
			i1 = (PS_LONG) (c - clean);

			if (i1 > i0) word[k++] = add_word_part (&clean[i0], i1 - i0, font, font_size, sub, super, small, under, NO_SPACE, rgb);
			if ((size_t)k == n_alloc) {
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
							word[k++] = add_word_part (&clean[i1], (PS_LONG)4, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k++] = add_word_part (&clean[i1], (PS_LONG)1, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1++;
						}
						if ((size_t)k == n_alloc) {
							n_alloc <<= 1;
							word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
						}
						if (clean[i1] == '\\') { /* 2nd char is Octal code character */
							word[k] = add_word_part (&clean[i1], (PS_LONG)4, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k] = add_word_part (&clean[i1], (PS_LONG)1, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1++;
						}
						if (!clean[i1]) word[k]->flag++;	/* New word after this composite */
						k++;
						if ((size_t)k == n_alloc) {
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
						if ((size_t)k == n_alloc) {
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
			word[k++] = add_word_part (clean, (PS_LONG)0, font, font_size, sub, super, small, under, ONE_SPACE, rgb);
			if ((size_t)k == n_alloc) {
				n_alloc <<= 1;
				word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
			}
		}

		ps_free ((void *)clean);	/* Reclaim this memory */

	} /* End of word loop */

	k--;
	while (k && !word[k]->txt) k--;	/* Skip any blank lines at end */
	n_items = k + 1;

	for (i0 = 0, i1 = 1 ; i1 < (PS_LONG)n_items-1; i1++, i0++) {	/* Loop for periods ending sentences and indicate 2 spaces to follow */
		if (isupper ((int)word[i1]->txt[0]) && word[i0]->txt[strlen(word[i0]->txt)-1] == '.') {
			word[i0]->flag &= 60;	/* Sets bits 1 & 2 to zero */
			word[i0]->flag |= 2;	/* Specify 2 spaces */
		}
		if (!word[i1]->txt[0]) {	/* No space at end of paragraph */
			word[i0]->flag &= 60;
			word[i1]->flag &= 60;
		}
	}
	if (i1 >= (PS_LONG)n_items) i1 = (PS_LONG)n_items - 1;	/* one-word fix */
	word[i1]->flag &= 60;	/* Last word not followed by anything */

	/* Determine list of unique colors */

	rgb_list = (int *) ps_memory (VNULL, n_items, sizeof (int));
	rgb_unique = (int *) ps_memory (VNULL, n_items, sizeof (int));

	for (i = 0; i < (PS_LONG)n_items; i++) rgb_list[i] = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
	qsort ((void *)rgb_list, (size_t) n_items, sizeof (int), ps_comp_int_asc);
	rgb_unique[0] = rgb_list[0];
	n_rgb_unique = 1;
	k = 0;
	for (i = 1; i < (PS_LONG)n_items; i++) {
		if (rgb_list[i] != rgb_list[k]) {	/* New color */
			rgb_unique[n_rgb_unique++] = rgb_list[i];
			k = i;
		}
	}
	ps_free ((void *)rgb_list);

	/* Replace each word's red value with the index of the corresponding unique color entry */

	for (i = 0; i < (PS_LONG)n_items; i++) {
		color = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
		for (j = 0, found = -1; found < 0 && j < n_rgb_unique; j++) if (color == rgb_unique[j]) found = j;
		word[i]->rgb[0] = found;
	}

	/* Determine list of unique fonts */

	font_list = (int *) ps_memory (VNULL, n_items, sizeof (int));
	font_unique = (int *) ps_memory (VNULL, n_items, sizeof (int));

	for (i = 0; i < (PS_LONG)n_items; i++) font_list[i] = word[i]->font_no;
	qsort ((void *)font_list, (size_t) n_items, sizeof (int), ps_comp_int_asc);
	font_unique[0] = font_list[0];
	n_font_unique = 1;
	k = 0;
	for (i = 1; i < (PS_LONG)n_items; i++) {
		if (font_list[i] != font_list[k]) {	/* New font */
			font_unique[n_font_unique++] = font_list[i];
			k = i;
		}
	}
	ps_free ((void *)font_list);

	/* Replace each word's font with the index of the corresponding unique font entry */

	for (i = 0; i < (PS_LONG)n_items; i++) {
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
	fprintf (PSL->internal.fp, "%d array astore def\n", n_font_unique);
	ps_free ((void *)font_unique);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Initialize variables:\n\n");
	fprintf (PSL->internal.fp, "/PSL_n %ld def\n", (PS_LONG)n_items);
	fprintf (PSL->internal.fp, "/PSL_n1 %ld def\n", (PS_LONG)n_items - 1);
	fprintf (PSL->internal.fp, "/PSL_y0 %ld def\n", (PS_LONG)irint (y * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_linespace %ld def\n", (PS_LONG)irint (line_space * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_parwidth %ld def\n", (PS_LONG)irint (par_width * PSL->internal.scale));
	fprintf (PSL->internal.fp, "/PSL_parjust %ld def\n", pj);
	fprintf (PSL->internal.fp, "/PSL_spaces [() ( ) (  ) ] def\n");
	(draw_box & 1) ? fprintf (PSL->internal.fp, "/PSL_drawbox true def\n") : fprintf (PSL->internal.fp, "/PSL_drawbox false def\n");
	(draw_box & 2) ? fprintf (PSL->internal.fp, "/PSL_fillbox true def\n") : fprintf (PSL->internal.fp, "/PSL_fillbox false def\n");
	fprintf (PSL->internal.fp, "/PSL_boxshape %d def\n", draw_box & 4);
	fprintf (PSL->internal.fp, "/PSL_lastfn -1 def\n/PSL_lastfs -1 def\n/PSL_lastfc -1 def\n");
	fprintf (PSL->internal.fp, "/PSL_UL 0 def\n/PSL_show {ashow} def\n");

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of words:\n");
	fprintf (PSL->internal.fp, "/PSL_word\n");
	for (i = n = 0 ; i < (PS_LONG)n_items; i++) {
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
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word font numbers:\n");
	fprintf (PSL->internal.fp, "/PSL_fnt\n");
	for (i = 0 ; i < (PS_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%d", word[i]->font_no);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word fontsizes:\n");
	fprintf (PSL->internal.fp, "/PSL_size\n");
	for (i = 0 ; i < (PS_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%.2f", word[i]->font_size);
		(!((i+1)%20)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%20)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word spaces to follow:\n");
	fprintf (PSL->internal.fp, "/PSL_flag\n");
	for (i = 0 ; i < (PS_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%d", word[i]->flag);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word baseline shifts:\n");
	fprintf (PSL->internal.fp, "/PSL_bshift\n");
	for (i = 0 ; i < (PS_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%g", word[i]->baseshift);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word colors indices:\n");
	fprintf (PSL->internal.fp, "/PSL_color\n");
	for (i = 0 ; i < (PS_LONG)n_items; i++) {
		fprintf (PSL->internal.fp, "%d", word[i]->rgb[0]);
		(!((i+1)%25)) ? fputc ('\n', PSL->internal.fp) : fputc (' ', PSL->internal.fp);
	}
	if ((i%25)) fputc ('\n', PSL->internal.fp);
	fprintf (PSL->internal.fp, "%ld array astore def\n", (PS_LONG)n_items);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word colors:\n");
	fprintf (PSL->internal.fp, "/PSL_rgb\n");
	for (i = 0 ; i < n_rgb_unique; i++) fprintf (PSL->internal.fp, "%.3g %.3g %.3g\n", PSL_INV_255 * (rgb_unique[i] >> 16), PSL_INV_255 * ((rgb_unique[i] >> 8) & 0xFF), PSL_INV_255 * (rgb_unique[i] & 0xFF));
	fprintf (PSL->internal.fp, "%d array astore def\n", 3 * n_rgb_unique);
	ps_free ((void *)rgb_unique);

	if (PSL->internal.comments) fprintf (PSL->internal.fp, "\n%% Define array of word widths:\n\n");
	fprintf (PSL->internal.fp, "/PSL_width %ld array def\n", (PS_LONG)n_items);
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
	fprintf (PSL->internal.fp, "/PSL_count %ld array def\n", (PS_LONG)n_items);
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
		fprintf (PSL->internal.fp, "0 0 M %ld %ld D S\n", (PS_LONG)irint (x_off * PSL->internal.scale), (PS_LONG)irint (y_off * PSL->internal.scale));
		if (vecpen_texture) ps_setdash (CNULL, 0);
	}

	ps_transrotate (x_off, y_off, 0.0);	/* Adjust for shift */

	/* Do the relative horizontal justification */

	fprintf (PSL->internal.fp, "0 0 M\n\n0 PSL_textjustifier");
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Just get paragraph height\n") : fprintf (PSL->internal.fp, "\n");

	/* Adjust origin for box justification */

	fprintf (PSL->internal.fp, "/PSL_x0 %ld def\n", -(PS_LONG)irint (0.5 * ((justify - 1) % 4) * par_width * PSL->internal.scale));
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
		fprintf (PSL->internal.fp, "/PSL_xgap %ld def\n", (PS_LONG)irint (x_gap * PSL->internal.scale));
		fprintf (PSL->internal.fp, "/PSL_ygap %ld def\n", (PS_LONG)irint (y_gap * PSL->internal.scale));
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
			fprintf (PSL->internal.fp, "/PSL_r %ld def\n", (PS_LONG)irint (MIN (x_gap, y_gap) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dx %ld def\n", (PS_LONG)irint (MAX (x_gap-y_gap, 0.0) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dx %ld def\n", (PS_LONG)irint (MAX (x_gap-y_gap, 0.0) * PSL->internal.scale));
			fprintf (PSL->internal.fp, "/PSL_dy %ld def\n", (PS_LONG)irint (MAX (y_gap-x_gap, 0.0) * PSL->internal.scale));
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
			pmode = ps_place_color (boxfill_rgb);
			fprintf (PSL->internal.fp, "%c F U ", PSL->internal.paint_code[pmode]);
		}
		if (draw_box & 1) {	/* Stroke */
			pmode = ps_place_color (boxpen_rgb);
			fprintf (PSL->internal.fp, "%c ", PSL->internal.paint_code[pmode]);
			fprintf (PSL->internal.fp, "S\n");
		}
		else
			fprintf (PSL->internal.fp, "N\n");
		if (boxpen_texture) ps_setdash (CNULL, 0);
		/* Because inside gsave/grestore we must reset PSL->pen and PSL->current.rgb so that they are set next time */
		PSL->current.rgb[0] = PSL->current.rgb[1] = PSL->current.rgb[2] = PSL->current.linewidth = -1;
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
void ps_words_ (double *x, double *y, char **text, PS_LONG *n_words, double *line_space, double *par_width, int *par_just, int* font, double *font_size, double *angle, int *rgb, int *justify, int *draw_box, double *x_off, double *y_off, double *x_gap, double *y_gap, int *boxpen_width, char *boxpen_texture, int *boxpen_offset, int *boxpen_rgb, int *vecpen_width, char *vecpen_texture, int *vecpen_offset, int *vecpen_rgb, int *boxfill_rgb, int n1, int n2, int n3) {

	ps_words (*x, *y, text, *n_words, *line_space, *par_width, *par_just, *font, *font_size, *angle, rgb, *justify, *draw_box, *x_off, *y_off, *x_gap, *y_gap, *boxpen_width, boxpen_texture, *boxpen_offset, boxpen_rgb, *vecpen_width, vecpen_texture, *vecpen_offset, vecpen_rgb, boxfill_rgb);

}

struct GMT_WORD *add_word_part (char *word, PS_LONG length, int fontno, double font_size, BOOLEAN sub, BOOLEAN super, BOOLEAN small, BOOLEAN under, int space, int rgb[])
{
	/* For flag: bits 1 and 2 give number of spaces to follow (0, 1, or 2)
	 * bit 3 == 1 means leading TAB
	 * bit 4 == 1 means Composite 1 character
	 * bit 5 == 1 means Composite 2 character
	 * bit 6 == 1 means underline word
	 */

	PS_LONG i = 0;
	int c;
	BOOLEAN tab = FALSE;
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
	PS_LONG i = 0;
	int c;
	while (old[i]) {
		c = (int)old[i];
		new[i++] = toupper (c);
	}
	new[i] = 0;
}

void ps_encode_font (int font_no)
{
	if (PSL->init.encoding == 0) return;		/* Already have StandardEncoding by default */
	if (PSL->internal.font[font_no].encoded) return;	/* Already reencoded or should not be reencoded ever */

	/* Reencode fonts with Standard+ or ISOLatin1[+] encodings */
	fprintf (PSL->internal.fp, "PSL_font_encode %d get 0 eq {%s_Encoding /%s /%s PSL_reencode PSL_font_encode %d 1 put} if", font_no, PSL->init.encoding, PSL->internal.font[font_no].name, PSL->internal.font[font_no].name, font_no);
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Set this font\n") : fprintf (PSL->internal.fp, "\n");
	PSL->internal.font[font_no].encoded = TRUE;
}

void init_font_encoding (struct EPS *eps)
{	/* Reencode all the fonts that we know may be used: the ones listed in eps */

	int i;

	if (eps)
		for (i = 0; i < 6 && PSL->init.eps->fontno[i] != -1; i++) ps_encode_font (PSL->init.eps->fontno[i]);
	else	/* Must output all */
		for (i = 0; i < PSL->internal.N_FONTS; i++) ps_encode_font (i);
}

void def_font_encoding (void)
{
	/* Initialize book-keeping for font encoding and write font macros */

	int i;

	/* Initialize T/F array for font reencoding so that we only do it once
	 * for each font that is used */

	fprintf (PSL->internal.fp, "/PSL_font_encode ");
	for (i = 0; i < PSL->internal.N_FONTS; i++) fprintf (PSL->internal.fp, "0 ");
	fprintf (PSL->internal.fp, "%d array astore def", PSL->internal.N_FONTS);
	(PSL->internal.comments) ? fprintf (PSL->internal.fp, "\t%% Initially zero\n") : fprintf (PSL->internal.fp, "\n");

	/* Define font macros (see pslib.h for details on how to add fonts) */

	for (i = 0; i < PSL->internal.N_FONTS; i++) fprintf (PSL->internal.fp, "/F%d {/%s Y}!\n", i, PSL->internal.font[i].name);
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
	int i=0, j=0, font;
	int he = 0;		/* GMT Historical Encoding (if any) */

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

	PS_LONG n, p, llx, lly, trx, try, BLOCKSIZE=4096;
	unsigned char *buffer;

	llx=0; lly=0; trx=720; try=720;

	/* Scan for BoundingBox */

	ps_get_boundingbox (fp, &llx, &lly, &trx, &try);

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
	h->width = trx - llx;
	h->height = try - lly;
	h->depth = 0;
	h->length = n;
	h->type = 4;
	h->maptype = RMT_NONE;
	h->maplength = 0;
	h->xorigin = llx;
	h->yorigin = lly;

	return (buffer);
}

unsigned char *ps_load_raster (FILE *fp, struct imageinfo *header)
{
	/* ps_load_raster reads a Sun standard rasterfile of depth 1, 8, 24, or 32 into memory */

	PS_LONG mx_in, mx, j, k, i, ij, n = 0, ny, get, odd, oddlength, r_off, b_off;
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
		mx_in = (PS_LONG) (2 * ceil (header->width / 16.0));	/* Because Sun images are written in multiples of 2 bytes */
		mx = (PS_LONG) (ceil (header->width / 8.0));		/* However, PS wants only the bytes that matters, so mx may be one less */
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
	else if (header->depth == (size_t)8 && header->maplength == (size_t)0) {	/* 8-bit without color table (implicit grayramp) */
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->length, fp) != (size_t)header->length) {
			fprintf (stderr, "pslib: Trouble reading 8-bit Sun rasterfile!\n");
			PS_exit (EXIT_FAILURE);
		}
		if (header->type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
	}
	else if (header->depth == 8) {	/* 8-bit with color table */
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
		odd = (PS_LONG)header->width%2;
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
	else if (header->depth == (size_t)24 && header->maplength == (size_t)0) {	/* 24-bit raster, no colormap */
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
	else if (header->depth == (size_t)32 && header->maplength == (size_t)0) {	/* 32-bit raster, no colormap */
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

int ps_read_rasheader (FILE *fp, struct imageinfo *h, int i0, int i1)
{
	/* Reads the header of a Sun rasterfile (or any other).
	   Since the byte order is defined as Big Endian, the bytes are read
	   byte by byte to ensure portability onto Little Endian platforms.
	 */

	unsigned char byte[4];
	int i, j, value, in[4];

	for (i = i0; i <= i1; i++) {

		if (fread ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) {
			fprintf (stderr, "pslib: Error reading rasterfile header\n");
			return (-1);
		}
		for (j = 0; j < 4; j++) in[j] = (int)byte[j];

		value = (in[0] << 24) + (in[1] << 16) + (in[2] << 8) + in[3];

		switch (i) {
			case 0:
				h->magic = value;
				break;
			case 1:
				h->width = value;
				break;
			case 2:
				h->height = value;
				break;
			case 3:
				h->depth = value;
				break;
			case 4:
				h->length = value;
				break;
			case 5:
				h->type = value;
				break;
			case 6:
				h->maptype = value;
				break;
			case 7:
				h->maplength = value;
				break;
		}
	}

	if (h->type == RT_OLD && h->length == 0) h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;

	return (0);
}

int ps_write_rasheader (FILE *fp, struct imageinfo *h, int i0, int i1)
{
	/* Writes the header of a Sun rasterfile.
	   Since the byte order is defined as Big Endian, the bytes are read
	   byte by byte to ensure portability onto Little Endian platforms.
	 */

	unsigned char byte[4];
	int i, j, value, in[4];

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

indexed_image_t ps_makecolormap (unsigned char *buffer, int nx, int ny, int nbits)
{
	/* When image consists of less than MAX_COLORS colors, the image can be
	 * indexed to safe a significant amount of space.
	 * The image and colormap are returned as a struct indexed_image_t.
	 *
	 * It is important that the first RGB tuple is mapped to index 0.
	 * This is used for color masked images.
	 */
	int i, j, npixels;
	colormap_t colormap;
	indexed_image_t image;

	if (abs(nbits) != 24) return (NULL);		/* We only index into the RGB colorspace. */

	npixels = abs(nx) * ny;

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
				image->buffer[i] = j;
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
			image->buffer[i] = j;
			colormap->colors[j][0] = buffer[0];
			colormap->colors[j][1] = buffer[1];
			colormap->colors[j][2] = buffer[2];
			colormap->ncolors++;
		}
		buffer += 3;
	}
	if (PSL->internal.verbose) fprintf (stderr, "pslib: Colormap of %d colors created\n", colormap->ncolors);
	return (image);
}

void ps_stream_dump (unsigned char *buffer, int nx, int ny, int nbits, int compress, int encode, int mask)
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
	PS_LONG nbytes, i;
	unsigned char *buffer1, *buffer2;
	char *kind_compress[3] = {"", "/RunLengthDecode filter", "/LZWDecode filter"};
	char *kind_mask[2] = {"", "mask"};

	nx = abs (nx);
	nbytes = ((((PS_LONG)nbits) * ((PS_LONG)(nx))) + 7) / ((PS_LONG)8) * ((PS_LONG)ny);
	PSL->internal.length = 0;

	/* Transform RGB stream to CMYK stream */
	if ((PSL->internal.color_mode & PSL_CMYK) && nbits == 24)
		buffer1 = ps_cmyk_encode (&nbytes, buffer);
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
		fprintf (PSL->internal.fp, "/Width %d /Height %d /BitsPerComponent %d\n", nx, ny, MIN(nbits,8));
		fprintf (PSL->internal.fp, "   /ImageMatrix [%d 0 0 %d 0 %d] /DataSource currentfile ", nx, -ny, ny);
		if (PSL->internal.ascii) fprintf (PSL->internal.fp, "/ASCII85Decode filter ");
		fprintf (PSL->internal.fp, "%s\n>> image%s\n", kind_compress[compress], kind_mask[mask]);
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
		fwrite ((void *)buffer, sizeof (unsigned char), (size_t)nbytes, PSL->internal.fp);
	}
	if (mask == 2) fprintf (PSL->internal.fp, "%s", kind_compress[compress]);

	/* Clear newly created buffers, but maintain original */
	if (buffer2 != buffer1) ps_free(buffer2);
	if (buffer1 != buffer ) ps_free(buffer1);
}

void ps_a85_encode (unsigned char quad[], PS_LONG nbytes)
{
	/* Encode 4-byte binary to 5-byte ASCII
	 * Special cases:	#00000000 is encoded as z
	 *			When n < 4, output only n+1 bytes */
	int j;
	unsigned int n;	/* Was size_t but that fails under 64-bit mode */
	unsigned char c[5];

	if (nbytes < 1) return;		/* Ignore empty input */
	nbytes = MIN (4, nbytes);	/* Limit to first four bytes */

	for (j = nbytes; j < 4; j++) quad[j] = 0;	/* Set truncated bytes to 0 */

	n = (quad[0] << 24) + (quad[1] << 16) + (quad[2] << 8) + quad[3];

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

	PS_LONG i, j, col, width, len;
	int odd = FALSE, count;
	unsigned char mask_table[] = {0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
	unsigned char mask, *out, value = 0;

	i = j = col = count = 0;

	width = (PS_LONG)irint (ceil (h->width * h->depth / 8.0));	/* Scanline width in bytes */
	if (width%2) odd = TRUE, width++;	/* To ensure 16-bit words */
	mask = mask_table[h->width%8];	/* Padding for 1-bit images */

	len = width * ((PS_LONG)h->height);		/* Length of output image */
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
			if (odd) out[i++] = count = 0;
			col = 0;
		}
	}

	if (i != len) fprintf (stderr, "pslib: ps_rle_decode has wrong # of outbytes (%ld versus expected %ld)\n", i, len);

	ps_free ((void *)*in);
	*in = out;
}

unsigned char *ps_cmyk_encode (PS_LONG *nbytes, unsigned char *input)
{
	/* Recode RGB stream as CMYK stream */

	PS_LONG in, out, nout;
	unsigned char *output;

	nout = *nbytes / 3 * 4;
	output = (unsigned char *)ps_memory (VNULL, (size_t)nout, sizeof (unsigned char));
	out = 0;

	for (in = 0; in < *nbytes; in += 3) {
		ps_rgb_to_cmyk_char (&input[in], &output[out]);
		out += 4;
	}
	*nbytes = nout;
	return (output);
}

unsigned char *ps_rle_encode (PS_LONG *nbytes, unsigned char *input)
{
	/* Run Length Encode a buffer of nbytes. */

	PS_LONG count = 0, out = 0, in = 0, i;
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
		if (PSL->internal.verbose) fprintf (stderr, "pslib: RLE inflated %ld to %ld bytes (aborted)\n", in, out);
		ps_free (output);
		return (NULL);
	}

	/* Return number of output bytes and output buffer */
	if (PSL->internal.verbose) fprintf (stderr, "pslib: RLE compressed %ld to %ld bytes\n", in, out);
	*nbytes = out;
	return (output);
}

unsigned char *ps_lzw_encode (PS_LONG *nbytes, unsigned char *input)
{
	/* LZW compress a buffer of nbytes. */

	static PS_LONG ncode = 4096*256;
	PS_LONG i, index, in = 0;
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
		if (PSL->internal.verbose) fprintf (stderr, "pslib: LZW inflated %ld to %ld bytes (aborted)\n", in, output->nbytes);
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
		stream->buffer[stream->nbytes] = bit_buffer >> 24;
		stream->nbytes++;
		bit_buffer <<= 8;
		bit_count -= 8;
	}
	if (incode == eod) {	/* Flush buffer */
		stream->buffer[stream->nbytes] = bit_buffer >> 24;
		stream->nbytes++;
		bit_buffer = 0;
		bit_count = 0;
	}
	return (stream);
}

int ps_bitimage_cmap (int f_rgb[], int b_rgb[])
{
	/* Print colormap for 1-bit image or imagemask. Returns value of "polarity":
	 * 0 = Paint 0 bits foreground color, leave 1 bits transparent
	 * 1 = Paint 1 bits background color, leave 0 bits transparent
	 * 2 = Paint 0 bits foreground color, paint 1 bits background color
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
		if (f_rgb[0] == 0 && f_rgb[1] == 0 && f_rgb[2] == 0)
			polarity = 4;
		else if (PSL->internal.color_mode & PSL_CMYK) {
			ps_rgb_to_cmyk_int (f_rgb, f_cmyk);
			fprintf (PSL->internal.fp, " [/Indexed /DeviceCMYK 0 <%02X%02X%02X%02X>] setcolorspace", f_cmyk[0], f_cmyk[1], f_cmyk[2], f_cmyk[3]);
		}
		else
			fprintf (PSL->internal.fp, " [/Indexed /DeviceRGB 0 <%02X%02X%02X>] setcolorspace", f_rgb[0], f_rgb[1], f_rgb[2]);
	}
	else if (f_rgb[0] < 0) {
		/* Foreground is transparent */
		polarity = 1;
		if (b_rgb[0] == 0 && b_rgb[1] == 0 && b_rgb[2] == 0)
			polarity = 5;
		else if (PSL->internal.color_mode & PSL_CMYK) {
			ps_rgb_to_cmyk_int (b_rgb, b_cmyk);
			fprintf (PSL->internal.fp, " [/Indexed /DeviceCMYK 0 <%02X%02X%02X%02X>] setcolorspace", b_cmyk[0], b_cmyk[1], b_cmyk[2], b_cmyk[3]);
		}
		else
			fprintf (PSL->internal.fp, " [/Indexed /DeviceRGB 0 <%02X%02X%02X>] setcolorspace", b_rgb[0], b_rgb[1], b_rgb[2]);
	}
	else if (b_rgb[0] == 0 && b_rgb[1] == 0 && b_rgb[2] == 0 && f_rgb[0] == 255 && f_rgb[1] == 255 && f_rgb[1] == 255) {
		/* 0 = White; 1 = Black */
		polarity = 3;
	}
	else if (f_rgb[0] == 0 && f_rgb[1] == 0 && f_rgb[2] == 0 && b_rgb[0] == 255 && b_rgb[1] == 255 && b_rgb[1] == 255) {
		/* 0 = Black; 1 = White */
		polarity = 6;
	}
	else {
		/* Colored foreground and background */
		polarity = 2;
		if (PSL->internal.color_mode & PSL_CMYK) {
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
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, (PS_LONG)irint (value * PSL->internal.scale));
}

void ps_set_height (char *param, double fontsize)
{
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, (PS_LONG)irint (fontsize * PSL->internal.scale / PSL->internal.points_pr_unit));
}

void ps_set_integer (char *param, PS_LONG value)
{
	fprintf (PSL->internal.fp, "/%s %ld def\n", param, value);
}

void ps_define_pen (char *param, int width, char *texture, int offset, int rgb[])
{
	int k;
	/* Function to set line pen attributes */
	fprintf (PSL->internal.fp, "/%s {", param);
	k = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%c %d W ", PSL->internal.paint_code[k], width);
	ps_place_setdash (texture, offset);
	fprintf (PSL->internal.fp, "} def\n");
}

void ps_define_rgb (char *param, int rgb[])
{
	int k;
	fprintf (PSL->internal.fp, "/%s {", param);
	k = ps_place_color (rgb);
	fprintf (PSL->internal.fp, "%c} def\n", PSL->internal.paint_code[k]);
}

void ps_set_length_array (char *param, double *array, PS_LONG n)
{	/* These are scaled by psscale */
	PS_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "%.2f\n", array[i] * PSL->internal.scale);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

PS_LONG ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, PS_LONG *node, PS_LONG n, PS_LONG m)
{	/* These are scaled by psscale.  We make sure there are no point pairs that would yield dx = dy = 0 (repeat point)
	 * at the resolution we are using (0.01 DPI units), hence a new n (possibly shorter) is returned. */
	PS_LONG i, j, k, this_i, this_j, last_i, last_j, n_skipped;
	char *use;

	use = (char *) ps_memory (VNULL, (size_t)n, sizeof (char));
	this_i = this_j = INT_MAX;
	for (i = j = k = n_skipped = 0; i < n; i++) {
		last_i = this_i;	last_j = this_j;
		this_i = (PS_LONG)irint (x[i] * PSL->internal.scale * 100.0);	/* Simulates the digits written by a %.2lf format */
		this_j = (PS_LONG)irint (y[i] * PSL->internal.scale * 100.0);
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

void ps_set_real_array (char *param, double *array, PS_LONG n)
{	/* These are raw and not scaled */
	PS_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "%.2f\n", array[i]);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

void ps_set_txt_array (char *param, char *array[], PS_LONG n)
{
	PS_LONG i;
	fprintf (PSL->internal.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (PSL->internal.fp, "(%s)\n", array[i]);
	fprintf (PSL->internal.fp, "%ld array astore def\n", n);
}

void *ps_memory (void *prev_addr, size_t nelem, size_t size)
{
	void *tmp;

	if (nelem == 0) return (VNULL); /* Take care of n = 0 */

	if (prev_addr) {
		if ((tmp = realloc ((void *) prev_addr, (nelem * size))) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not reallocate more memory, n = ");
			PRINT_SIZE_T (stderr, nelem);	fprintf (stderr, "\n");
			PS_exit (EXIT_FAILURE);
		}
	}
	else {
		if ((tmp = calloc ((size_t) nelem, size)) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not allocate memory, n = \n");
			PRINT_SIZE_T (stderr, nelem);	fprintf (stderr, "\n");
			PS_exit (EXIT_FAILURE);
		}
	}
	return (tmp);
}

void ps_free (void *addr)
{
	free (addr);
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
	int i, j;
	BOOLEAN first = TRUE;

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

static void ps_init_fonts (int *n_fonts, int *n_GMT_fonts)
{
	FILE *in;
	int i = 0, n_alloc = 64;
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
		if (sscanf (buf, "%s %lf %d", fullname, &PSL->internal.font[i].height, &PSL->internal.font[i].encoded) != 3) {
			fprintf (stderr, "PSL Fatal Error: Trouble decoding font info for font %d\n", i);
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
			if (sscanf (buf, "%s %lf %d", PSL->internal.font[i].name, &PSL->internal.font[i].height, &PSL->internal.font[i].encoded) != 3) {
				fprintf (stderr, "PSL Fatal Error: Trouble decoding custom font info for font %d\n", i - *n_GMT_fonts);
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

int ps_place_color (int rgb[])
{
	int pmode;

	if (rgb[0] == -1) {
		/* Outline only, no color set */
		pmode = 0;
	}
	else if (rgb[0] == -3) {
		/* Pattern fill */
		fprintf (PSL->internal.fp, "pattern%d ", rgb[1]);
		pmode = 10;
	}
	else if (!PSL_iscolor (rgb)) {
		/* Grey scale */
		fprintf (PSL->internal.fp, PSL->current.bw_format, rgb[0] * PSL_INV_255);
		pmode = 2;
	}
	else if (PSL->internal.color_mode == PSL_RGB) {
		/* Full color, RGB mode */
		fprintf (PSL->internal.fp, PSL->current.rgb_format, rgb[0] * PSL_INV_255, rgb[1] * PSL_INV_255, rgb[2] * PSL_INV_255);
		pmode = 4;
	}
	else if (PSL->internal.color_mode & PSL_CMYK) {
		/* CMYK mode */
		double cmyk[4];
		ps_rgb_to_cmyk (rgb, cmyk);
		fprintf (PSL->internal.fp, PSL->current.cmyk_format, cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
		pmode = 6;
	}
	else {
		/* HSV mode */
		double hsv[3];
		ps_rgb_to_hsv (rgb, hsv);
		fprintf (PSL->internal.fp, PSL->current.hsv_format, hsv[0], hsv[1], hsv[2]);
		pmode = 8;
	}
	return (pmode);
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
	double xr, xg, xb, r_dist, g_dist, b_dist, max_v, min_v, diff, idiff;

	xr = rgb[0] * PSL_INV_255;
	xg = rgb[1] * PSL_INV_255;
	xb = rgb[2] * PSL_INV_255;
	max_v = MAX (MAX (xr, xg), xb);
	min_v = MIN (MIN (xr, xg), xb);
	diff = max_v - min_v;
	hsv[0] = 0.0;
	hsv[1] = (max_v == 0.0) ? 0.0 : diff / max_v;
	hsv[2] = max_v;
	if (hsv[1] == 0.0) return;	/* Hue is undefined */
	idiff = 1.0 / diff;
	r_dist = (max_v - xr) * idiff;
	g_dist = (max_v - xg) * idiff;
	b_dist = (max_v - xb) * idiff;
	if (xr == max_v)
		hsv[0] = b_dist - g_dist;
	else if (xg == max_v)
		hsv[0] = 2.0 + r_dist - b_dist;
	else
		hsv[0] = 4.0 + g_dist - r_dist;
	hsv[0] *= 60.0;
	if (hsv[0] < 0.0) hsv[0] += 360.0;
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
	unsigned char rgb[3];

	if (h->depth == 24) {
		for (i = j = 0; i < h->width * h->height; i++, j += 3)
		{
			memcpy ((void *)rgb, (void *)&buffer[j], 3 * sizeof(unsigned char));
			buffer[i] = (unsigned char) PSL_YIQ (rgb);
		}
		h->depth = 8;
	}
}

int ps_bitreduce (unsigned char *buffer, int nx, int ny, int ncolors)
{
	/* Reduce an 8-bit stream to 1-, 2- or 4-bit stream */
	int in, out, i, j, nout, nbits;

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
	nx = abs(nx);
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
	if (PSL->internal.verbose) fprintf (stderr, "pslib: Image depth reduced to %d bits\n", nbits);
	return (nbits);
}

int ps_get_boundingbox (FILE *fp, PS_LONG *llx, PS_LONG *lly, PS_LONG *trx, PS_LONG *try)
{
	PS_LONG nested;
	char buf[BUFSIZ];

	nested = 0; *llx = 1; *trx = 0;
	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (!nested && !strncmp(buf, "%%BoundingBox:", (size_t)14)) {
			if (!strstr(buf, "(atend)")) {
				if (sscanf(strchr(buf, ':') + 1, "%ld %ld %ld %ld", llx, lly, trx, try) < 4) return 1;
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

	if (*llx >= *trx || *lly >= *try) {
		*llx = 0; *trx = 720; *lly = 0; *try = 720;
		fprintf(stderr, "No proper BoundingBox, defaults assumed: %ld %ld %ld %ld\n", *llx, *lly, *trx, *try);
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
