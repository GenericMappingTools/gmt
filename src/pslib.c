/*--------------------------------------------------------------------
 *	$Id: pslib.c,v 1.77 2004-07-03 02:13:31 pwessel Exp $
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
 *	ps_flush		: Flushes the output buffer
 *	ps_hexagon		: Plots a hexagon and {optionally] fills it
 *	ps_image		: Plots a 1-to-8 bit 2-D image using grayshades
 *	ps_imagefill		: Fills a polygon with a repeating n x n image pattern
 *	ps_imagemask		: Uses a 1-bit image as mask to paint given color
 *	ps_itriangle		: Plots an inverted triangle and [optionally] fills it
 *	ps_line			: Plots a line
 *	ps_loadraster		: Read image from a Sun rasterfile
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
 * Date:	02-JUN-2004
 * Version:	4.1
 *
 * The environmental variable GMTHOME must be set to the directory that holds the subdirectory
 *   share/pattern where all the pattern Sun raster files are stored
 */

#include "pslib_inc.h"
#include "pslib.h"

/* Special macros and structure for ps_words */

#define NO_SPACE	 0
#define ONE_SPACE	 1
#define COMPOSITE_1	8
#define COMPOSITE_2	16
#define SYMBOL		12
#define PSL_CHUNK	2000

struct GMT_WORD {
	int font_no;
	int rgb[3];
	int flag;
	double font_size;
	double baseshift;
	char *txt;
};

int PSL_text_first = TRUE;

/* Define support functions called inside pslib functions */

char *ps_prepare_text (char *text);
void def_font_encoding (void);
void init_font_encoding (struct EPS *eps);
void get_uppercase(char *new, char *old);
unsigned char * ps_1bit_to_24bit (unsigned char *pattern, struct rasterfile *h, int f_rgb[], int b_rgb[]);
void ps_rle_decode (struct rasterfile *h, unsigned char **in);
void ps_hex_dump (unsigned char *buffer, int nx, int ny, int depth);
void ps_hex_dump_cmyk (unsigned char *buffer, int nx, int ny, int depth);
void ps_bin_dump (unsigned char *buffer, int nx, int ny, int depth);
void ps_bin_dump_cmyk (unsigned char *buffer, int nx, int ny);
void *ps_memory (void *prev_addr, size_t nelem, size_t size);
int ps_shorten_path (double *x, double *y, int n, int *ix, int *iy);
int ps_comp_int_asc (const void *p1, const void *p2);
int ps_read_rasheader (FILE *fp, struct rasterfile *h);
int ps_write_rasheader (FILE *fp, struct rasterfile *h);
static void bulkcopy (const char *);
static void ps_init_fonts (int *n_fonts, int *n_GMT_fonts);
void ps_rgb_to_cmyk (int rgb[], double cmyk[]);
void ps_cmyk_to_rgb (int rgb[], double cmyk[]);
int ps_place_color (int rgb[]);
void ps_place_setdash (char *pattern, int offset);
void ps_set_length_array (char *param, double *array, int n);
int ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, int *node, int n, int m);
void ps_set_txt_array (char *param, char *array[], int n);
void ps_set_integer (char *param, int value);
void ps_set_real_array (char *param, double *array, int n);


/*------------------- PUBLIC PSLIB FUNCTIONS--------------------- */


void ps_arc (double x, double y, double radius, double az1, double az2, int status)
{	/* 1 = set anchor, 2 = set end, 3 = both */
	int ix, iy, ir;
	
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	ir = irint (radius * ps.scale);
	if (fabs (az1 - az2) > 360.0) az1 = 0.0, az2 = 360.0; 
	if (status%2) {	/* Beginning of new segment */
		fprintf (ps.fp, "S ");
		ps.npath = 0;
	}
	else
		ps.npath++;
	if (az1 < az2)	/* Forward positive arc */
		fprintf (ps.fp, "%d %d %d %g %g arc", ix ,iy, ir, az1, az2);
	else	/* Negative arc */
		fprintf (ps.fp, "%d %d %d %g %g arcn", ix ,iy, ir, az1, az2);
	if (status > 1)	fprintf (ps.fp, " S");
	fprintf (ps.fp, "\n");
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
	char text[128], format[32];
	
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
	dy = sign * annotpointsize / ps.points_pr_unit;
			
	fprintf (ps.fp, "\nV %g %g T %g R\n", x * ps.scale, y * ps.scale, angle);
	ps_plot (0.0, 0.0, 3);
	ps_plot (length, 0.0, 2);
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
	while (val <= (val1+SMALL)) {
		i++;
		xx = (val - val0) * scl;
		if (left) xx = length - xx;
		ps_plot (xx, 0.0, 3);
		ps_plot (xx, dy, 2);
		sprintf( text, format, val);
		ps_text (xx, annot_off, annotpointsize, text, 0.0, annot_justify, 0);
		val = val0 + i * annotation_int;
	}
	ps_text (0.5*length, label_off, annotpointsize*1.5, label, 0.0, label_justify, 0);
	fprintf (ps.fp, "U\n\n");
}

/* fortran interface */
void ps_axis_ (double *x, double *y, double *length, double *val0, double *val1, double *annotation_int, char *label, double *annotpointsize, int *side, int nlen)
{
	ps_axis (*x, *y, *length, *val0, *val1, *annotation_int, label, *annotpointsize, *side);
}

void ps_circle (double x, double y, double size, int rgb[], int outline)
{
	int ix, iy, ir, pmode;

	/* size is assumed to be diameter */	

	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	ir = irint (0.5 * size * ps.scale);
	fprintf (ps.fp, "N ");
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d C%d\n", ix, iy, ir, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_circle_ (double *x, double *y, double *size, int *rgb, int *outline)
{
	 ps_circle (*x, *y, *size, rgb, *outline);
}

void ps_clipoff (void) {
	fprintf (ps.fp, "\nS U\n%%Clipping is currently OFF\n");
	ps.npath = ps.clip_path_length = 0;
}

/* fortran interface */
void ps_clipoff_ (void) {
	ps_clipoff ();
}

void ps_clipon (double *x, double *y, int n, int rgb[], int flag)
{        
      	/* Path length */
            	/* Optional paint (-1 to avoid paint) */
	        /* combo of 1 | 2. 1 = Start, 2 = end */
	/* Any plotting outside the path defined by x,y will be clipped.
	   use clipoff to restore the original clipping path. */
	
	int used, pmode;
	char move[7];
	
	if (flag & 1) {	/* First segment in (possibly multi-segmented) clip-path */
		strcpy (move, "M");
		fprintf (ps.fp, "\n%% Start of clip path\nS V\n");
		ps.npath = 0;
	}
	else
		strcpy (move, "moveto");
		
	used = 0;
	if (n > 0) {
		ps.ix = irint (x[0]*ps.scale);
		ps.iy = irint (y[0]*ps.scale);
		ps.npath++;
		used++;
		fprintf (ps.fp, "%d %d %s\n", ps.ix, ps.iy, move);
		used += ps_line (&x[1], &y[1], n-1, 0, FALSE, FALSE);	/* Must pass close = FALSE since first point not given ! */
		fprintf (ps.fp, "P\n");
	}
	ps.clip_path_length += used;
	ps.max_path_length = MAX (ps.clip_path_length, ps.max_path_length);
	
	if (flag & 2) {	/* End path and [optionally] fill */
		if (rgb[0] >= 0) {	/* fill is desired */
			fprintf (ps.fp, "V ");
			pmode = ps_place_color (rgb);
			fprintf (ps.fp, "%c eofill U ", ps_paint_code[pmode]);
		}
		if (flag & 4)
			fprintf (ps.fp, "eoclip\n%% End of clip path.  Clipping is currently ON\n");
		else
			fprintf (ps.fp, "eoclip N\n%% End of clip path.  Clipping is currently ON\n");
		ps.npath = 0;
	}
}
	  
/* fortran interface */
void ps_clipon_ (double *x, double *y, int *n, int *rgb, int *flag)
{
	ps_clipon (x, y, *n, rgb, *flag);
}
	  
void ps_colorimage (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits)
{
	/* Plots a B/W (1), gray (4|8), or color (24)  bitmapped image (nbits)
	 * x, y is lower left position of image in inches
	 * xsize, ysize is the image size in inches
	 * buffer contains the bytes for the image
	 * nx, ny is the pixel dimension
	 * nbits is the number of bits per pixel (1, 4, 8, 24)
	 */
	 
	int lx, ly;		/* x and y dimension of image in PS coordinates */
	int mx, n_channels, id, nan_rgb[3];
	BOOLEAN colormask = FALSE;
	char *colorspace[3] = {"Gray", "RGB", "CMYK"};			/* What kind of image we are writing */
	char *decode[3] = {"0 1", "0 1 0 1 0 1", "0 1 0 1 0 1 0 1"};	/* What kind of color decoding */
	char *kind[2] = {"bin", "hex"};					/* What encoding to use */
	char *read[2] = {"readstring", "readhexstring"};		/* What read function to use */
	
	lx = irint (xsize * ps.scale);
	ly = irint (ysize * ps.scale);
	id = (ps.cmyk_mode && abs(nbits) == 24) ? 2 : ((abs(nbits) == 24) ? 1 : 0);
	if (nx < 0 && abs(nbits) == 24) {		/* Colormask requested, 24-bit only */
		nx = abs (nx);
		colormask = TRUE;
		nan_rgb[0] = (int)buffer[0];
		nan_rgb[1] = (int)buffer[1];
		nan_rgb[2] = (int)buffer[2];
		buffer += 3;		/* Skip to start of image */
	}
		
	fprintf (ps.fp, "\n%% Start of %s Adobe %s image [%d bit]\n", kind[ps.hex_image], colorspace[id], abs (nbits));
	fprintf (ps.fp, "V N %g %g T %d %d scale\n", x * ps.scale, y * ps.scale, lx, ly);
	if (colormask) {	/* Do new PS Level 3 image type 4 with colormask */
		nbits = abs (nbits);
		fprintf (ps.fp, "/Device%s setcolorspace\n", colorspace[id]);
		fprintf (ps.fp, "<<\t%% Start Image dictionary\n  /ImageType 4\n  /Width %d /Height %d\n", nx, ny);
		fprintf (ps.fp, "  /BitsPerComponent %d\n", MIN (nbits, 8));
		fprintf (ps.fp, "  /Decode [%s]\n", decode[id]);
		fprintf (ps.fp, "  /ImageMatrix [%d 0 0 %d 0 %d]\n", nx, -ny, ny);
		fprintf (ps.fp, "  /DataSource currentfile");
		if (ps.hex_image) fprintf (ps.fp, " /ASCIIHexDecode filter");
		fprintf (ps.fp, "\n  /MaskColor [%d %d %d]\n>>\nimage\n", nan_rgb[0], nan_rgb[1], nan_rgb[2]);
	}
	else if (nbits < 0) {	/* Do new PS Level 2 image with interpolation */
		nbits = abs (nbits);
		fprintf (ps.fp, "/Device%s setcolorspace\n", colorspace[id]);
		fprintf (ps.fp, "<<\t%% Start Image dictionary\n  /ImageType 1\n  /Width %d /Height %d\n", nx, ny);
		fprintf (ps.fp, "  /BitsPerComponent %d\n", MIN (nbits, 8));
		fprintf (ps.fp, "  /Decode [%s]\n", decode[id]);
		fprintf (ps.fp, "  /ImageMatrix [%d 0 0 %d 0 %d]\n", nx, -ny, ny);
		fprintf (ps.fp, "  /DataSource currentfile");
		if (ps.hex_image) fprintf (ps.fp, " /ASCIIHexDecode filter");
		fprintf (ps.fp, "\n  /Interpolate true\n>>\nimage\n");
	}
	else {		/* Standard colorimage call PS Level 1 */
		n_channels = (ps.cmyk_mode) ? 4 : 3;
		fprintf (ps.fp, "%d %d 8 div mul ceiling cvi dup 65535 ge {pop 65535} if string /pstr exch def\n", nx, nbits);
		fprintf (ps.fp, "%d %d %d [%d 0 0 %d 0 %d] {currentfile pstr %s pop} ", nx, ny, MIN (nbits, 8), nx, -ny, ny, read[ps.hex_image]);
		(nbits <= 8) ? fprintf (ps.fp, "image\n") : fprintf (ps.fp, "false %d colorimage\n", n_channels);
	}

	mx = (int) ceil (nbits * nx / 8.0);
	if (ps.hex_image)
		(id == 2) ? ps_hex_dump_cmyk (buffer, nx, ny, 24) : ps_hex_dump (buffer, nx, ny, nbits);
	else
		(id == 2) ? ps_bin_dump_cmyk (buffer, nx, ny) : ps_bin_dump (buffer, nx, ny, nbits);

	fprintf (ps.fp, "U\n%% End of %s Adobe %s image\n", kind[ps.hex_image], colorspace[id]);
}

void ps_bin_dump (unsigned char *buffer, int nx, int ny, int depth)
{
	int mx;
	mx = (int) ceil (depth * nx / 8.0);
	(void)fwrite ((void *)buffer, sizeof (unsigned char), (size_t)(mx * ny), ps.fp);
}

void ps_hex_dump (unsigned char *buffer, int nx, int ny, int depth)
{
	/* Writes core of output image using hex format */

	char hex[16], pixel[61];
	int mx, i, j, kk, ij;
	
	pixel[60] = 0;
	hex[0] = '0';	hex[1] = '1';	hex[2] = '2';	hex[3] = '3';
	hex[4] = '4';	hex[5] = '5';	hex[6] = '6';	hex[7] = '7';
	hex[8] = '8';	hex[9] = '9';	hex[10] = 'A';	hex[11] = 'B';
	hex[12] = 'C';	hex[13] = 'D';	hex[14] = 'E';	hex[15] = 'F';
	
	mx = (int) ceil (depth * nx / 8.0);
	kk = 0;
	for (j = ij = 0; j < ny; j++) {
		for (i = 0; i < mx; i++) {
			pixel[kk++] = hex[buffer[ij] / 16];
			pixel[kk++] = hex[buffer[ij++] % 16];
			if (kk == 60) {
				fprintf (ps.fp, "%s\n", pixel);
				kk = 0;
			}
		}
	}
	if (kk > 0) {
		pixel[kk] = 0;
		fprintf (ps.fp, "%s\n", pixel);
	}
}

void ps_hex_dump_cmyk (unsigned char *buffer, int nx, int ny, int depth)
{
	/* Writes core of output image in CMYK using hex format */

	char hex[16], pixel[65];
	int mx, i, j, kk, ij, c, m, y, k;
	
	pixel[64] = 0;
	hex[0] = '0';	hex[1] = '1';	hex[2] = '2';	hex[3] = '3';
	hex[4] = '4';	hex[5] = '5';	hex[6] = '6';	hex[7] = '7';
	hex[8] = '8';	hex[9] = '9';	hex[10] = 'A';	hex[11] = 'B';
	hex[12] = 'C';	hex[13] = 'D';	hex[14] = 'E';	hex[15] = 'F';
	
	mx = (int) ceil (depth * nx / 8.0);
	kk = 0;
	for (j = ij = 0; j < ny; j++) {
		for (i = 0; i < nx; i++) {
			c = 255 - buffer[ij++];	m = 255 - buffer[ij++];	y = 255 - buffer[ij++];
			k = MIN (c, m);	if (y < k) k = y;
			c -= k;	m -= k;	y -= k;
			pixel[kk++] = hex[c / 16];
			pixel[kk++] = hex[c % 16];
			pixel[kk++] = hex[m / 16];
			pixel[kk++] = hex[m % 16];
			pixel[kk++] = hex[y / 16];
			pixel[kk++] = hex[y % 16];
			pixel[kk++] = hex[k / 16];
			pixel[kk++] = hex[k % 16];
			if (kk == 64) {
				fprintf (ps.fp, "%s\n", pixel);
				kk = 0;
			}
		}
	}
	if (kk > 0) {
		pixel[kk] = 0;
		fprintf (ps.fp, "%s\n", pixel);
	}
}

void ps_bin_dump_cmyk (unsigned char *buffer, int nx, int ny)
{
	/* Writes  image in CMYK using bin format */
	
	int i, ij, j;
	unsigned char cmyk[4];
	
	for (j = ij = 0; j < ny; j++) {
		for (i = 0; i < nx; i++) {
			cmyk[0] = 255 - buffer[ij++];	cmyk[1] = 255 - buffer[ij++];	cmyk[2] = 255 - buffer[ij++];
			cmyk[3] = MIN (cmyk[0], cmyk[1]);	if (cmyk[2] < cmyk[3]) cmyk[3] = cmyk[2];
			cmyk[0] -= cmyk[3];	cmyk[1] -= cmyk[3];	cmyk[2] -= cmyk[3];
			fwrite ((void *)cmyk, sizeof (unsigned char), (size_t)4, ps.fp);
		}
	}
}

/* fortran interface */
void ps_colorimage_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, int *nx, int *ny, int *nbits, int nlen)
{
	ps_colorimage (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *nbits);
}

void ps_colortiles (double x0, double y0, double xsize, double ysize, unsigned char *image, int nx, int ny)
/* x0, y0: Lower left corner in inches */
/* xsize, ysize: Size of image in inches */
/* image: color image with rgb triplets per pixel */
/* nx, ny: image size in pixels */
{
	int i, j, k, rgb[3];
	double x1, x2, y1, y2, dx, dy, noise, noise2;
	
	noise = 2.0 / ps.scale;
	noise2 = 2.0 * noise;
	dx = xsize / nx;
	dy = ysize / ny;

	ps_transrotate (x0, y0, 0.0);
	y2 = (ny - 0.5) * dy + 0.5 * noise;
	for (j = k = 0; j < ny; j++) {
		y1 = (ny - j - 1.5) * dy - 0.5 * noise;
		x1 = -0.5 * (dx + noise);
		for (i = 0; i < nx; i++) {
			x2 = (i + 0.5) * dx + noise;
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
	fprintf (ps.fp, "%s\n", text);
}

/* fortran interface */
void ps_command_ (char *text, int nlen)
{
	ps_command (text);
}

void ps_comment (char *text)
{
	fprintf (ps.fp, "%%\n%% %s\n%%\n", text);
}

/* fortran interface */
void ps_comment_ (char *text, int nlen)
{
	ps_comment (text);
}

void ps_cross (double x, double y, double diameter)
{	/* Fit inside circle of given diameter; draw using current color */
	fprintf (ps.fp, "%d %d %d X\n", (int) irint (diameter * ps.scale), (int) irint ((x - 0.5 * diameter) * ps.scale), (int ) irint (y * ps.scale));
	ps.npath = 0;
}

/* fortran interface */
void ps_cross_ (double *x, double *y, double *diameter)
{
	 ps_cross (*x, *y, *diameter);
}

void ps_diamond (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int ix, iy, ds, pmode;
	
	diameter *= 0.5;
	ds = irint (diameter * ps.scale);
	ix = irint (x * ps.scale);
	iy = irint ((y - diameter) * ps.scale);
	
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d D%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_diamond_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_diamond (*x, *y, *diameter, rgb, *outline);
}

void ps_segment (double x0, double y0, double x1, double y1)
{	/* Short line segment */
	int ix, iy, dx, dy;
	
	ix = irint (x0 * ps.scale);
	iy = irint (y0 * ps.scale);
	dx = irint (x1 * ps.scale) - ix;
	dy = irint (y1 * ps.scale) - iy;
	fprintf (ps.fp, "%d %d M %d %d D S\n", ix, iy, dx, dy);
	ps.npath = 0;
}

/* fortran interface */
void ps_segment_ (double *x0, double *y0, double *x1, double *y1)
{
	 ps_segment (*x0, *y0, *x1, *y1);
}

void ps_star (double x, double y, double diameter, int rgb[], int outline)
{	/* Fit inside circle of given diameter */
	int ix, iy, ds, pmode;

	diameter *= 0.5;
	ds = irint (diameter * ps.scale);
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d E%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_star_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_star (*x, *y, *diameter, rgb, *outline);
}

void ps_hexagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int ix, iy, ds, pmode;

	diameter *= 0.5;
	ds = irint (diameter * ps.scale);
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d H%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_hexagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_hexagon (*x, *y, *diameter, rgb, *outline);
}

void ps_pentagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int ix, iy, ds, pmode;

	diameter *= 0.5;
	ds = irint (diameter * ps.scale);
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d N%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_pentagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_pentagon (*x, *y, *diameter, rgb, *outline);
}

void ps_octagon (double x, double y, double diameter, int rgb[], int outline)
{	/* diameter is diameter of circumscribing circle */
	int ix, iy, ds, pmode;

	diameter *= 0.5;
	ds = irint (diameter * ps.scale);
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d O%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_octagon_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	 ps_octagon (*x, *y, *diameter, rgb, *outline);
}

void ps_ellipse (double x, double y, double angle, double major, double minor, int rgb[], int outline)
{
	int ix, iy, ir, pmode;
	double aspect;
	
	/* Feature: Pen thickness also affected by aspect ratio */
	
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	fprintf (ps.fp, "V %d %d T", ix, iy);
	if (angle != 0.0) fprintf (ps.fp, " %g R", angle);
	aspect = minor / major;
	fprintf (ps.fp, " 1 %g scale\n", aspect);
	ir = irint (major * ps.scale);

	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "0 0 %d C%d U\n", ir, outline + ps_outline_offset[pmode]);
}

/* fortran interface */
void ps_ellipse_ (double *x, double *y, double *angle, double *major, double *minor, int *rgb, int *outline)
{
	 ps_ellipse (*x, *y, *angle, *major, *minor, rgb, *outline);
}

void ps_flush ()
{
	/* Simply flushes the output buffer */

         fflush (ps.fp);
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

void ps_imagefill_cleanup (void) {
	int image_no;
	
	for (image_no = 0; image_no < N_PATTERNS; image_no++) {
		if (ps_pattern_status[image_no][0]) {
			fprintf (ps.fp, "currentdict /image%d undef\n", image_no);
			fprintf (ps.fp, "currentdict /fillimage%d undef\n", image_no);
		}
		if (ps_pattern_status[image_no][1]) {
			fprintf (ps.fp, "currentdict /image%di undef\n", image_no);
			fprintf (ps.fp, "currentdict /fillimage%di undef\n", image_no);
		}
	}
	for (image_no = 0; image_no < ps_n_userimages; image_no++) {
		fprintf (ps.fp, "currentdict /image%d undef\n", N_PATTERNS+image_no);
		fprintf (ps.fp, "currentdict /fillimage%d undef\n", N_PATTERNS+image_no);
	}
}

/* fortran interface */
void ps_imagefill_ (double *x, double *y, int *n, int *image_no, char *imagefile, int *invert, int *image_dpi, int *outline, BOOLEAN *colorize, int *f_rgb, int *b_rgb, int nlen)
{
	 ps_imagefill (x, y, *n, *image_no, imagefile, *invert, *image_dpi, *outline, *colorize, f_rgb, b_rgb);
}

void ps_imagefill (double *x, double *y, int n, int image_no, char *imagefile, int invert, int image_dpi, int outline, BOOLEAN colorize, int f_rgb[], int b_rgb[])
{              
	/* x,y:		Array of enclosing path */
	/* image_dpi:	Resolution of image on the page */
 	/* n:		No of points in path */
	/* imagefile:	Name of image file */
	/* invert:	If TRUE exchange white and black pixels (1-bit only)  */
	/* outline:	TRUE will draw outline, -1 means clippath already in place */
	/* colorize:	If TRUE, replace set bits with f_rgb color and unset bits with b_rgb  */
	/* f_rgb:	Foreground color */
	/* b_rgb:	Background color */

	BOOLEAN found;
	int i, j, ix, iy, nx, ny, n_times = 0;
	char op[15];
	double xx, yy, xmin, xmax, ymin, ymax, image_size_x, image_size_y;
	
	if ((image_no >= 0 && image_no < N_PATTERNS) && !ps_pattern_status[image_no][invert]) {	/* Unused predefined */
		image_no = ps_imagefill_init (image_no, imagefile, invert, image_dpi, colorize, f_rgb, b_rgb);
		nx = ps_pattern_nx[image_no][invert];
		ny = ps_pattern_ny[image_no][invert];
	}
	else if (image_no < 0) {	/* User image, check if already used */
		for (i = 0, found = FALSE; !found && i < ps_n_userimages; i++) found = !strcmp (ps_user_image[i].name, imagefile);
		if (!found)	/* Not found or no previous user images loaded */
			image_no = ps_imagefill_init (image_no, imagefile, invert, image_dpi, colorize, f_rgb, b_rgb);
		else
			image_no = N_PATTERNS + i - 1;
		nx = ps_user_image[image_no-N_PATTERNS].nx;
		ny = ps_user_image[image_no-N_PATTERNS].ny;
	}
	else {	/* Used predefined pattern */
		nx = ps_pattern_nx[image_no][invert];
		ny = ps_pattern_ny[image_no][invert];
	}

	ps_comment ("Start of user imagefill pattern");
	if (invert)
		sprintf (op, "fillimage%di", image_no);
	else
		sprintf (op, "fillimage%d", image_no);
	
	/* Print out clip-path */
	
	if (outline >= 0) ps_clipon (x, y, n, no_rgb, 3);
	
	/* Find extreme bounds for area */
	
	xmin = xmax = x[0];
	ymin = ymax = y[0];
	for (i = 1; i < n; i++) {
		xmin = MIN (xmin, x[i]);
		ymin = MIN (ymin, y[i]);
		xmax = MAX (xmax, x[i]);
		ymax = MAX (ymax, y[i]);
	}
	
	image_size_x = (image_dpi) ? (double) nx / (double) image_dpi : nx / ps.scale;	/* Use device resolution if dpi is not set */
	image_size_y = (image_dpi) ? (double) ny / (double) image_dpi : ny / ps.scale;	/* Use device resolution if dpi is not set */

	for (j = (int) floor (ymin / image_size_y); j <= (int) ceil (ymax / image_size_y); j++) {
		yy = j * image_size_y;
		for (i = (int) floor (xmin / image_size_x); i <= (int) ceil (xmax / image_size_x); i++) {
			xx = i * image_size_x;
			ix = irint (xx * ps.scale);
			iy = irint (yy * ps.scale);
			fprintf (ps.fp, "%d %d", ix, iy);
			n_times++;
			(n_times%5) ? fputc (' ', ps.fp) : fputc ('\n', ps.fp);
			/* Prevent stack from getting too full by flushing every 200 times */
                        if (!(n_times%200)) {
                        	fprintf (ps.fp, "200 {%s} repeat\n", op);
                        	n_times = 0;
                        } 
		}
	}
	if (n_times%5) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d {%s} repeat\n", n_times, op);
	if (outline > 0) fprintf (ps.fp, "clippath S\n");
	ps_clipoff ();
	ps_comment ("End of user imagefill pattern");
}

int ps_imagefill_init (int image_no, char *imagefile, int invert, int image_dpi, BOOLEAN colorize, int f_rgb[], int b_rgb[])
{

	int i, nx, ny, dx, dy, pmode, polarity, n_channels;
	char file[BUFSIZ], name[BUFSIZ];
	char *TF[2] = {"false", "true"};
	unsigned char *picture;
	struct rasterfile h;
	BOOLEAN found;
	
	if ((image_no >= 0 && image_no < N_PATTERNS) && ps_pattern_status[image_no][invert]) return (image_no);	/* Already done this */

	if ((image_no >= 0 && image_no < N_PATTERNS)) {	/* Premade pattern yet not used */
		sprintf (file, "%s%cshare%cpattern%cps_pattern_%2.2d.ras", PSHOME, DIR_DELIM, DIR_DELIM, DIR_DELIM, image_no);
		ps_pattern_status[image_no][invert] = 1;
	}
	else {	/* User image, check to see if already used */

		for (i = 0, found = FALSE; !found && i < ps_n_userimages; i++) found = !strcmp (ps_user_image[i].name, imagefile);
		if (found) return (N_PATTERNS + i - 1);
#ifdef WIN32
		if (imagefile[0] == '\\' || imagefile[1] == ':')	/* Full path name, use it as is */
#else
		if (imagefile[0] == '/')	/* Full path name, use it as is */
#endif
			strcpy (file, imagefile);
		else {
			/* First look in users current directory */
			if (!access (imagefile, R_OK))
				strcpy (file, imagefile);
			else
				sprintf (file, "%s%cshare%c%s", PSHOME, DIR_DELIM, DIR_DELIM, imagefile);
		}
		ps_user_image[ps_n_userimages].name = (char *) ps_memory (VNULL, (size_t)(strlen (imagefile)+1), sizeof (char));
		strcpy (ps_user_image[ps_n_userimages].name, imagefile);
		image_no = N_PATTERNS + ps_n_userimages;
		ps_n_userimages++;
	}

	picture = ps_loadraster (file, &h, invert, FALSE, colorize, f_rgb, b_rgb);

	nx = h.ras_width;
	ny = h.ras_height;
	if (image_no < N_PATTERNS) {
		ps_pattern_nx[image_no][invert] = nx;
		ps_pattern_ny[image_no][invert] = ny;
	}
	else {
		ps_user_image[ps_n_userimages-1].nx = nx;
		ps_user_image[ps_n_userimages-1].ny = ny;
	}

	if (image_dpi) {	/* Use given DPI */
		dx = irint (h.ras_width * ps.scale / image_dpi);
		dy = irint (h.ras_height * ps.scale / image_dpi);
	}
	else {	/* Use device resolution */
		dx = h.ras_width;
		dy = h.ras_height;
	}
	
	ps_comment ("Start of user imagefill pattern definition");

	if (invert)
		sprintf (name, "image%di", image_no);
	else
		sprintf (name, "image%d", image_no);

	fprintf (ps.fp, "/%s <\n", name);
	ps_hex_dump (picture, nx, ny, h.ras_depth);
	(ps.cmyk_mode) ? ps_hex_dump_cmyk (picture, nx, ny, h.ras_depth) : ps_hex_dump (picture, nx, ny, h.ras_depth);
	fprintf (ps.fp, "> def\n");

	if (h.ras_depth == 1) {	/* 1-bit bitmap basis */
		if (f_rgb[0] < 0 || b_rgb[0] < 0) {	/* Colorized imagemask case */
			fprintf (ps.fp, "/fill%s { V T ", name);
			if (f_rgb[0] < 0) {	/* Use background color for masks 0 bits */
				polarity = 1;
				pmode = ps_place_color (b_rgb);
			}
			else {	/* Use foreground color for masks 1 bits */
				polarity = 0;
				pmode = ps_place_color (f_rgb);
			}
			fprintf (ps.fp, "%c %d %d scale %d %d %s [%d 0 0 %d 0 %d] {%s} imagemask U} def\n", ps_paint_code[pmode], dx, dy, nx, ny, TF[polarity], nx, -ny, ny, name);
		}
		else	/* Plain b/w image */
			fprintf (ps.fp, "/fill%s { V T %d %d scale %d %d 1 [%d 0 0 %d 0 %d] {%s} image U} def\n", name, dx, dy, nx, ny, nx, -ny, ny, name);
	}
	else {
		n_channels = (ps.cmyk_mode) ? 4 : 3;
		fprintf (ps.fp, "/fill%s { V T %d %d scale %d %d 8 [%d 0 0 %d 0 %d] {%s} false %d colorimage U} def\n", name, dx, dy, nx, ny, nx, -ny, ny, name, n_channels);
	}

	ps_free ((void *)picture);
	
	ps_comment ("End of user imagefill pattern definition");

	return (image_no);
}

void ps_imagemask (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int polarity, int rgb[])
{
	/* Plots a 1-bit mask. Polarity says how to interpret the mask:
	 * If 1 is used, then maskbits == 1 will be painted with rgb.
	 * If 0 is used, then maskbits == 0 will be painted with rgb.
	 * buffer width must be an integral of 8 bits.
	 */
	int lx, ly;
	char *TF[2] = {"false", "true"}, *kind[2] = {"bin", "hex"}, *read[2] = {"readstring", "readhexstring"};
	
	lx = irint (xsize * ps.scale);
	ly = irint (ysize * ps.scale);
	fprintf (ps.fp, "\n%% Start of %s imagemask\n", kind[ps.hex_image]);
	fprintf (ps.fp, "V N %g %g T %d %d scale\n", x * ps.scale, y * ps.scale, lx, ly);
	ps_setpaint (rgb);
	memcpy ((void *)ps.rgb, (void *)no_rgb, 3 * sizeof (int));	/* So subsequent ps_setpaint calls work properly */
	fprintf (ps.fp, "%d 1 8 div mul ceiling cvi dup 65535 ge {pop 65535} if string /pstr exch def\n", nx);
	fprintf (ps.fp, "%d %d %s [%d 0 0 %d 0 %d] {currentfile pstr %s pop} imagemask\n",
		nx, ny, TF[polarity], nx, -ny, ny, read[ps.hex_image]);
	
	(ps.hex_image) ? ps_hex_dump (buffer, nx, ny, 1) : (void)fwrite ((void *)buffer, sizeof (unsigned char), (size_t)(((int) ceil (0.125 * nx)) * ny), ps.fp);


	fprintf (ps.fp, "U\n%% End of imagemask\n\n");
}

/* fortran interface */
void ps_imagemask_ (double *x, double *y, double *xsize, double *ysize, unsigned char *buffer, int *nx, int *ny, int *polarity, int *rgb, int nlen)
{
	ps_imagemask (*x, *y, *xsize, *ysize, buffer, *nx, *ny, *polarity, rgb);
}

int ps_line (double *x, double *y, int n, int type, int close, int split)
              
            	/* type: 1 means new anchor point, 2 means stroke line, 3 = both */
          	/* TRUE if a closed polygon */
           {	/* TRUE if we can split line segment into several sections */
	int i, *ix, *iy, trim = FALSE;
	char move = 'M';
	
	/* First remove unnecessary points that have zero curvature */
	
	ix = (int *) ps_memory (VNULL, (size_t)n, sizeof (int));
	iy = (int *) ps_memory (VNULL, (size_t)n, sizeof (int));
	
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
		fprintf (ps.fp, "%d %d %c\n", ix[0], iy[0], move);
		ps.npath = 1;
	}
	else
		fprintf (ps.fp, "%d %d D\n", ix[0] - ps.ix, iy[0] - ps.iy);
	ps.ix = ix[0];
	ps.iy = iy[0];
		
	if (!split) ps.max_path_length = MAX ((n + ps.clip_path_length), ps.max_path_length);
	
	for (i = 1; i < n; i++) {
		fprintf (ps.fp, "%d %d D\n", ix[i] - ps.ix, iy[i] - ps.iy);
		ps.ix = ix[i];
		ps.iy = iy[i];
		ps.npath++;
		if ((ps.npath + ps.clip_path_length) > MAX_L1_PATH && split) {
			fprintf (ps.fp, "S %d %d M\n", ps.ix, ps.iy);
			ps.npath = 1;
			close = FALSE;
			if (trim) {	/* Restore the duplicate point since close no longer is TRUE */
				n++;
				trim = FALSE;
			}
		}
	}
	if (close) fprintf (ps.fp, "P");	/* Close the path */
	if (type > 1) {
		fprintf (ps.fp, " S\n");	/* Stroke the path */
		ps.npath = 0;
	}
	else if (close)
		fprintf (ps.fp, "\n");
	
	ps_free ((void *)ix);
	ps_free ((void *)iy);
	
	return (n);
}

/* fortran interface */
void ps_line_ (double *x, double *y, int *n, int *type, int *close, int *split)
{
	ps_line (x, y, *n, *type, *close, *split);
}

int ps_shorten_path (double *x, double *y, int n, int *ix, int *iy)
{
	double old_slope, new_slope, dx, dy, old_dir, new_dir;
	int i, j, k, *xx, *yy, fixed;
	
	if (n < 2) return (0);
	
	xx = (int *) ps_memory (VNULL, (size_t)n, sizeof (int));
	yy = (int *) ps_memory (VNULL, (size_t)n, sizeof (int));
	
	xx[0] = irint (x[0] * ps.scale);
	yy[0] = irint (y[0] * ps.scale);
	
	for (i = j = 1; i < n; i++) {
		xx[j] = irint (x[i] * ps.scale);
		yy[j] = irint (y[i] * ps.scale);
		if (xx[j] != xx[j-1] || yy[j] != yy[j-1]) j++;
	}
	n = j;
	
	if (n < 2) {
		ps_free ((void *)xx);
		ps_free ((void *)yy);
		return (0);
	}
	
	ix[0] = xx[0];	iy[0] = yy[0];	k = 1;
	
	dx = xx[1] - xx[0];
	dy = yy[1] - yy[0];
	fixed = (dx == 0.0 && dy == 0.0);
	old_slope = (fixed) ? 1.01e100 : ((dx == 0) ? copysign (1.0e100, dy) : dy / dx);
	old_dir = (dx >= 0.0) ? 1 : -1;
	
	for (i = 1; i < n-1; i++) {
		dx = xx[i+1] - xx[i];
		dy = yy[i+1] - yy[i];
		fixed = (dx == 0.0 && dy == 0.0);
		new_slope = (fixed) ? 1.01e100 : ((dx == 0) ? copysign (1.0e100, dy) : dy / dx);
		if (fixed) continue;	/* Didnt move */
		
		new_dir = (dx >= 0.0) ? 1 : -1;
		if (new_slope != old_slope || new_dir != old_dir) {
			ix[k] = xx[i];
			iy[k] = yy[i];
			k++;
			old_slope = new_slope;
			old_dir = new_dir;
		}
	}
	dx = xx[n-1] - xx[n-2];
	dy = yy[n-1] - yy[n-2];
	fixed = (dx == 0.0 && dy == 0.0 && (k > 1 && ix[k-1] == xx[n-1] && iy[k-1] == yy[n-1]));	/* Didnt move */
	if (!fixed) {
		ix[k] = xx[n-1];
		iy[k] = yy[n-1];
		k++;
	}
	
	ps_free ((void *)xx);
	ps_free ((void *)yy);
	
	return (k);
}

/* fortran interface */
void ps_shorten_path_ (double *x, double *y, int *n, int *ix, int *iy)
{
	ps_shorten_path (x, y, *n, ix, iy);
}

void ps_pie (double x, double y, double radius, double az1, double az2, int rgb[], int outline)
{
	int ix, iy, ir, pmode;
	
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	ir = irint (radius * ps.scale);
	fprintf (ps.fp, "%d %d M ", ix, iy);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d %lg %lg P%d\n", ix, iy, ir, az1, az2, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_pie_ (double *x, double *y, double *radius, double *az1, double *az2, int *rgb, int *outline)
{
	 ps_pie (*x, *y, *radius, *az1, *az2, rgb, *outline);
}

void ps_plot (double x, double y, int pen)
{
	int ix, iy, idx, idy;
	
	ix = irint (x*ps.scale);
	iy = irint (y*ps.scale);
	if (abs (pen) == 2) {	/* Convert absolute draw to relative draw */
		idx = ix - ps.ix;
		idy = iy - ps.iy;
		if (idx == 0 && idy == 0) return;
		fprintf (ps.fp, "%d %d D\n", idx, idy);
		ps.npath++;
	}
	else {
		idx = ix;
		idy = iy;
		fprintf (ps.fp, "%d %d M\n", idx, idy);
		ps.npath = 1;
	}
	if (pen == -2) fprintf (ps.fp, "S\n");
	ps.ix = ix;
	ps.iy = iy;
	if ((ps.npath + ps.clip_path_length) > MAX_L1_PATH) {
		fprintf (ps.fp, "S %d %d M\n", ix, iy);
		ps.npath = 1;
	}
}

/* fortran interface */
void ps_plot_ (double *x, double *y, int *pen)
{
	ps_plot (*x, *y, *pen);
}

void ps_plotend (int lastpage)
{
	ps_imagefill_cleanup ();
	ps_setdash (CNULL, 0);

	if (lastpage) {
		if (!ps.eps_format)
			fprintf (ps.fp, "%%%%PageTrailer\n");
		else {
			int x0, y0, x1, y1;
			x0 = MAX (irint (ps.xscl * ps.bb[0]), 0);
			y0 = MAX (irint (ps.yscl * ps.bb[1]), 0);
			x1 = irint (ps.xscl * ps.bb[2]);
			y1 = irint (ps.yscl * ps.bb[3]);
			fprintf (ps.fp, "%%%%Trailer\n");
			fprintf (ps.fp, "%%%%BoundingBox: %d %d %d %d\n", x0, y0, x1, y1);
		}
		fprintf (ps.fp, "%% Reset translations and scale and call showpage\n");
		fprintf (ps.fp, "S %g %g T", -(ps.xoff * ps.scale), -(ps.yoff * ps.scale));
		fprintf (ps.fp, " %g %g scale",
			ps.scale/(ps.points_pr_unit * ps.xscl), ps.scale/(ps.points_pr_unit * ps.yscl));
		if (ps.landscape) fprintf (ps.fp, " -90 R %d 0 T", -ps.p_width);
		fprintf (ps.fp, " 0 A\nshowpage\n");
		if (!ps.eps_format) fprintf (ps.fp, "\n%%%%Trailer\n");
		fprintf (ps.fp, "\nend\n");
		if (!ps.eps_format) fprintf (ps.fp, "%%%%EOF\n");
	}
	else if (ps.absolute)
		fprintf (ps.fp, "S %g %g T 0 A\n", -(ps.xoff * ps.scale), -(ps.yoff * ps.scale));
	else
		fprintf (ps.fp, "S 0 A\n");
	if (ps.fp != stdout) fclose (ps.fp);
}

/* fortran interface */
void ps_plotend_ (int *lastpage)
{
	ps_plotend (*lastpage);
}

int ps_plotinit (char *plotfile, int overlay, int mode, double xoff, double yoff, double xscl, double yscl, int ncopies, int dpi, int unit, int *page_size, int *rgb, const char *encoding, struct EPS *eps)
/* plotfile:	Name of output file or NULL for standard output */
/* xoff, yoff:	Sets a new origin relative to old */
/* xscl, yscl:	Global scaling, usually left to 1,1 */
/* page_size:	Physical width and height of paper used in points */
/* overlay:	FALSE means print headers and macros first */
/* mode:	 1st bit 0 = Landscape, 1 = Portrait,
                 3rd bit 0 = bin image, 1 = hex image
		 4th bit 0 = rel positions, 1 = abs positions
		10th bit 0 = RGB image, 1 = CMYK image */
/* ncopies:	Number of copies for this plot */
/* dpi:		Plotter resolution in dots-per-inch */
/* unit:	0 = cm, 1 = inch, 2 = meter */
/* rgb:		array with Color of page (paper) */
/* encoding:	Font encoding used */
/* eps:		structure with Document info.  !! Fortran version (ps_plotinit_) does not have this argument !! */
{
	int i, pmode, manual = FALSE;
	time_t right_now;
	char openmode[2], *this;
	double scl;
	
	if ((this = getenv ("GMTHOME")) == NULL) {	/* Use default GMT path */
 		PSHOME = (char *) ps_memory (VNULL, (size_t)(strlen (GMT_DEFAULT_PATH) + 1), sizeof (char));
 		strcpy (PSHOME, GMT_DEFAULT_PATH);
	}
	else {	/* Set user's default path */
		PSHOME = (char *) ps_memory (VNULL, (size_t)(strlen (this) + 1), sizeof (char));
		strcpy (PSHOME, this);
	}

	ps_init_fonts (&N_FONTS, &N_GMT_FONTS);	/* Load the available font information */

	ps.eps_format = FALSE;
	ps.hex_image = (mode & 4) ? TRUE : FALSE;
	ps.cmyk_mode = (mode & 512) ? TRUE : FALSE;
	ps.absolute = (mode & 8) ? TRUE : FALSE;
	if (page_size[0] < 0) {		/* Want Manual Request for paper */
		ps.p_width  = abs (page_size[0]);
		manual = TRUE;
	}
	else
		ps.p_width = page_size[0];
	if (page_size[1] < 0) {	/* Want EPS format */
		page_size[1] = -page_size[1];
		ps.eps_format = TRUE;
	}

	ps.p_height = page_size[1];
	ps.font_no = 0;
	ps.linewidth = -1;	/* Will be changed by ps_setline */
	ps.rgb[0] = ps.rgb[1] = ps.rgb[2] = -1;	/* Will be changed by ps_setpaint */
	ps.scale = (double)dpi;	/* Dots pr. unit resolution of output device */
	ps.points_pr_unit = 72.0;
	if (unit == 0) ps.points_pr_unit /= 2.54;
	if (unit == 2) ps.points_pr_unit /= 0.0254;
	ps.encoding = ps_memory (NULL, strlen (encoding) + 1, sizeof (char));
	strcpy (ps.encoding, encoding);
	mode &= 1;							/* Get rid of other flags */
	if (plotfile == NULL || plotfile[0] == 0)
		ps.fp = stdout;
	else {
		(overlay) ? strcpy (openmode, "a") : strcpy (openmode, "w");
		if ((ps.fp = fopen (plotfile, openmode)) == NULL) {
			fprintf (stderr, "pslib: Cannot create/open file : %s\n", plotfile);
			return (-1);
		}
	}

#ifdef WIN32
	/* 
	 * Diomidis Spinellis, December 2001
	 * Set binary mode to avoid corrupting binary color images.
	 */
	setmode(fileno(ps.fp), O_BINARY);
#elif __EMX__	/* PW: Same for OS/2 with EMX support */
	_fsetmode (ps.fp, "b");
#endif

	right_now = time ((time_t *)0);
	ps.landscape = !(overlay || mode);	/* Only rotate if not overlay and not Portrait */
	ps.xscl = xscl;
	ps.xoff = xoff;
	ps.yscl = yscl;
	ps.yoff = yoff;
	strcpy (ps.bw_format, "%.3lg ");			/* Default format used for grayshade value */
	strcpy (ps.rgb_format, "%.3lg %.3lg %.3lg ");	/* Same, for color triplets */
	strcpy (ps.cmyk_format, "%.3lg %.3lg %.3lg %.3lg ");	/* Same, for CMYK quadruples */

	/* In case this is the last overlay, set the Bounding box coordinates to be used atend */

	if (eps) {	/* Document info is available */
		if (eps->portrait) {	/* Plot originated as Portrait */
			ps.bb[0] = eps->x0;
			ps.bb[1] = eps->y0;
			ps.bb[2] = eps->x1;
			ps.bb[3] = eps->y1;
		}
		else { 			/* Plot originated as Landscape */
			ps.bb[0] = ps.p_width - eps->y1; ps.bb[1] = eps->x0; ps.bb[2] = ps.p_width - eps->y0; ps.bb[3] = eps->x1;
		}
	}
	else {		/* No info is available, default to Current Media Size */
		ps.bb[0] = ps.bb[1] = 0;
		ps.bb[2] = ps.p_width;
		ps.bb[3] = (ps.p_height == 0) ? PAGE_HEIGHT_IN_PTS : ps.p_height;
	}

	if (!overlay) {

		if (ps.eps_format)
			fprintf (ps.fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
		else
			fprintf (ps.fp, "%%!PS-Adobe-3.0\n");

		/* Write definitions of macros to plotfile */
		
		fprintf (ps.fp, "%%%%BoundingBox: ");
		if (ps.eps_format)
			fprintf (ps.fp, "(atend)\n");
		else
			fprintf (ps.fp, "0 0 %d %d\n", ps.p_width, ps.p_height);
		if (eps) {	/* Document info is available */
			fprintf (ps.fp, "%%%%Title: %s\n", eps->title);
			fprintf (ps.fp, "%%%%Creator: GMT\n");
			fprintf (ps.fp, "%%%%For: %s\n", eps->name);
			fprintf (ps.fp, "%%%%DocumentNeededResources: font");
			for (i = 0; i < 6 && eps->fontno[i] != -1; i++) fprintf (ps.fp, " %s", ps.font[eps->fontno[i]].name);
			fprintf (ps.fp, "\n");
		}
		else {
			fprintf (ps.fp, "%%%%Title: pslib v%s document\n", PSL_Version);
			fprintf (ps.fp, "%%%%Creator: pslib\n");
		}

		fprintf (ps.fp, "%%%%CreationDate: %s", ctime(&right_now));
		fprintf (ps.fp, "%%%%LanguageLevel: 1\n");
		if (ps.hex_image)
			fprintf (ps.fp, "%%%%DocumentData: Clean7Bit\n");
		else
			fprintf (ps.fp, "%%%%DocumentData: Binary\n");
		if (!ps.eps_format) {
			if (ps.landscape)
				fprintf (ps.fp, "%%%%Orientation: Landscape\n");
			else
				fprintf (ps.fp, "%%%%Orientation: Portrait\n");
			fprintf (ps.fp, "%%%%Pages: 1\n");
		}
		fprintf (ps.fp, "%%%%EndComments\n\n");

		bulkcopy ("PSL_prologue");
		bulkcopy (ps.encoding);
		
		def_font_encoding ();		/* Place code for reencoding of fonts and initialize book-keeping */

		/* XXX This should be done by code in the prologue */
		/* XXX This may also be wishful thinking. */
		/* Define font macros (see pslib.h for details on how to add fonts) */
		
		
		for (i = 0; i < N_FONTS; i++) fprintf (ps.fp, "/F%d {/%s Y} bind def\n", i, ps.font[i].name);

		if (!ps.eps_format) fprintf (ps.fp, "/#copies %d def\n\n", ncopies);
		bulkcopy ("PSL_label");		/* Place code for label line annotations and clipping */
	 	fprintf (ps.fp, "%%%%EndProlog\n\n");

		fprintf (ps.fp, "%%%%BeginSetup\n\n");
		fprintf (ps.fp, "/PSLevel /languagelevel where {pop languagelevel} {1} ifelse def\n");
		if (manual)	/* Manual media feed requested */
			fprintf (ps.fp, "PSLevel 1 gt { << /ManualFeed true >> setpagedevice } if\n\n");
		else if (!ps.eps_format && ps.p_width > 0 && ps.p_height > 0)	/* Specific media selected */
			fprintf (ps.fp, "PSLevel 1 gt { << /PageSize [%d %d] /ImagingBBox null >> setpagedevice } if\n\n", ps.p_width, ps.p_height);

		fprintf (ps.fp, "%% Init coordinate system and scales\n");
		scl = ps.points_pr_unit / ps.scale;
		fprintf (ps.fp, "%% Scale is originally set to %g, which means that\n", scl);
		if (unit == 0) {	/* CM used as unit */
			fprintf (ps.fp, "%% 1 cm on the paper equals %d Postscript units\n", (int)ps.scale);
		}
		else if (unit == 1)	/* INCH used as unit */
			fprintf (ps.fp, "%% 1 inch on the paper equals %d Postscript units\n", (int)ps.scale);
		else if (unit == 2) {	/* M used as unit */
			fprintf (ps.fp, "%% 1 m on the paper equals %d Postscript units\n", (int)ps.scale);
		}
		else {
			fprintf (stderr, "pslib: Measure unit not valid!\n");
			exit (EXIT_FAILURE);
		}
		
		xscl *= scl;
		yscl *= scl;
		if (ps.landscape) fprintf (ps.fp, "%d 0 T 90 R\n", ps.p_width);
		fprintf (ps.fp, "%g %g scale\n", xscl, yscl);
	}
	if (!overlay) {
		fprintf (ps.fp, "%% End of pslib header\n");
		fprintf (ps.fp, "%%%%EndSetup\n\n");
		if (!ps.eps_format) fprintf (ps.fp, "%%%%Page: 1 1\n\n");
		if (!(rgb[0] == rgb[1] && rgb[1] == rgb[2] && rgb[0] == 255)) {	/* Change background color */
			fprintf (ps.fp, "clippath ");
			pmode = ps_place_color (rgb);
			fprintf (ps.fp, "%c F N\n", ps_paint_code[pmode]);
		}
	}
	init_font_encoding (eps);	/* Reencode fonts if necessary */

	ps_setpaint (no_rgb);
	if (!(xoff == 0.0 && yoff == 0.0)) fprintf (ps.fp, "%g %g T\n", xoff*ps.scale, yoff*ps.scale);
	
	/* Initialize global variables */
	
	ps.npath = ps.clip_path_length = ps.max_path_length = 0;
	memset ((void *) ps_pattern_status, 0, (size_t)(2 * N_PATTERNS));
	memset ((void *) ps_user_image, 0, (size_t)(sizeof (struct USERIMAGE) * N_PATTERNS));
	ps_n_userimages = 0;
	return (0);
}

/* fortran interface */
void ps_plotinit_ (char *plotfile, int *overlay, int *mode, double *xoff, double *yoff, double *xscl, double *yscl, int *ncopies, int *dpi, int *unit, int *page_size, int *rgb, const char *encoding, int nlen1, int nlen2)
{
	 ps_plotinit (plotfile, *overlay, *mode, *xoff, *yoff, *xscl, *yscl, *ncopies, *dpi, *unit, page_size, rgb, encoding, (struct EPS *)NULL);
}

void ps_plotr (double x, double y, int pen)
{
	int ix, iy;
	
	ix = irint (x * ps.scale);
	iy = irint (y * ps.scale);
	if (ix == 0 && iy == 0) return;
	ps.npath++;
	if (abs (pen) == 2)
		fprintf (ps.fp, "%d %d D\n", ix, iy);
	else {
		fprintf (ps.fp, "%d %d G\n", ix, iy);
		ps.npath = 1;
	}
	if (pen == -2) fprintf (ps.fp, "S\n");
	ps.ix += ix;	/* Update absolute position */
	ps.iy += iy;
}

/* fortran interface */
void ps_plotr_ (double *x, double *y, int *pen)
{
	ps_plotr (*x, *y, *pen);
}

void ps_polygon (double *x, double *y, int n, int rgb[], int outline)
{
	int split, pmode;
	char mode;
	
	split = (rgb[0] < 0);	/* Can only split if we need outline only */
	if (outline >= 0) ps_line (x, y, n, 1, FALSE, split);	/* No stroke or close path yet */
	ps.npath = 0;

	ps.max_path_length = MAX ((n + ps.clip_path_length), ps.max_path_length);
	
	if (split) {	/* Outline only */
		mode = 'p';
		outline = 0;
	}
	else {
		pmode = ps_place_color (rgb);
		mode = ps_paint_code[pmode] - 'A' + 'a';	/* Convert A, C, or K to a, c, or k */
	}
	if (outline > 0) mode += outline;		/* Convert a, c, or k to b, d, or l */
	fprintf (ps.fp, "%c\n", mode);
	if (outline < 0) {
		fprintf (ps.fp, "\nN U\n%%Clipping is currently OFF\n");
		ps.clip_path_length = 0;
	}
}

/* fortran interface */
void ps_polygon_ (double *x, double *y, int *n, int *rgb, int *outline)
{
	ps_polygon (x, y, *n, rgb, *outline);
}

  
void ps_patch (double *x, double *y, int np, int rgb[], int outline)
{
	/* Like ps_polygon but intended for small polygons (< 20 points).  No checking for
	 * shorter path by calling ps_shorten_path as in ps_polygon.
	 *
	 * Thus, the usage is (with xi,yi being absolute coordinate for point i and dxi the increment
	 * from point i to i+1, and r,g,b in the range 0.0-1.0.  Here, n = np-1.
	 *
	 *	dx_n dy_n ... n x0 y0 w		(If r < 0 then outline only)
	 *	r dx_n dy_n ... n x0 y0 q	(gray shade; use r for outline)
	 *	r g b dx_n dy_n ... n x0 y0 s	(rgb; use t for outline)
	 *	c m y k dx_n dy_n ... n x0 y0 u (cmyk; use v for outline)
	 */
	 
	int i, n, pmode, n1, ix[20], iy[20];
	char mode;
	
	if (np > 20) {	/* Must call ps_polygon instead */
		ps_polygon ( x, y, np, rgb, outline);
		return;
	}
	
	ix[0] = irint (x[0] * ps.scale);	/* Convert inch to absolute pixel position for start of quadrilateral */
	iy[0] = irint (y[0] * ps.scale);
	
	for (i = n = 1, n1 = 0; i < np; i++) {	/* Same but check if new point represent a different pixel */
		ix[n] = irint (x[i] * ps.scale);
		iy[n] = irint (y[i] * ps.scale);
		if (ix[n] != ix[n1] || iy[n] != iy[n1]) n++, n1++;
	}
	if (ix[0] == ix[n1] && iy[0] == iy[n1]) n--, n1--;	/* Closepath will do this automatically */
	
	if (n < 3) return;	/* 2 points or less don't make a polygon */
	
	pmode = ps_place_color (rgb);
	mode = 'q' + 2 * pmode;			/* Will give q, s, u, or w */
	if (outline && pmode < 3) mode++;	/* Will turn q -> r, s -> t, and u -> v but leave w as is */

	n--;
	n1 = n;
	for (i = n - 1; i != -1; i--, n--) fprintf (ps.fp, "%d %d ", ix[n] - ix[i], iy[n] - iy[i]);
	fprintf (ps.fp, "%d %d %d %c\n", n1, ix[0], iy[0], mode);
}

/* fortran interface */

void ps_patch_ (double *x, double *y, int *n, int *rgb, int *outline)
{
	ps_patch (x, y, *n, rgb, *outline);
}

void ps_rect (double x1, double y1, double x2, double y2, int rgb[], int outline)
{
	int ix, iy, idx, idy, pmode;
	
	ix = irint (x1 * ps.scale);
	iy = irint (y1 * ps.scale);
	idx = irint (x2 * ps.scale) - ix;
	idy = irint (y2 * ps.scale) - iy;
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d %d R%d\n", idy, idx, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_rect_ (double *x1, double *y1, double *x2, double *y2, int *rgb, int *outline)
{
	ps_rect (*x1, *y1, *x2, *y2, rgb, *outline);
}

void ps_rotatetrans (double x, double y, double angle)
{
	int go = FALSE;
	
	if (angle != 0.0) {
		if (fabs(angle) < 1e-9) angle = 0.0;
		fprintf (ps.fp, "%g R", angle);
		go = TRUE;
	}
	if (x != 0.0 || y != 0.0) {
		if (fabs(x) < 1e-9) x = 0.0;
		if (fabs(y) < 1e-9) y = 0.0;
		if (go) fputc (' ', ps.fp);
		fprintf (ps.fp, "%g %g T", x * ps.scale, y * ps.scale);
	}
	fputc ('\n', ps.fp);
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

	fputs ("S ", ps.fp);
	ps_place_setdash (pattern, offset);
	fputs ("\n", ps.fp);
	ps.npath = 0;
}

void ps_place_setdash (char *pattern, int offset)
{
	int place_space;
	if (pattern) {
		fputs ("[", ps.fp);
		place_space = 0;
		while (*pattern) {
			if (place_space) fputc (' ', ps.fp);
			fprintf (ps.fp, "%g", (atoi(pattern) * 72.0 / ps.points_pr_unit));
			while (*pattern && *pattern != ' ') pattern++;
			while (*pattern && *pattern == ' ') pattern++;
			place_space = 1;
		}		    		
		fprintf (ps.fp, "] %d B", offset);
	}
	else
		fprintf (ps.fp, "[] 0 B");	/* Reset to continuous line */
}

/* fortran interface */
void ps_setdash_ (char *pattern, int *offset, int nlen)
{
	ps_setdash (pattern, *offset);
}

void ps_setfont (int font_no)
{
	if (font_no < 0 || font_no >= N_FONTS)
		fprintf (stderr, "pslib: Selected font out of range (%d), ignored\n", font_no);
	else
		ps.font_no = font_no;
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
  		sprintf (ps.bw_format, "%%.%df ", n_decimals);
  		sprintf (ps.rgb_format, "%%.%df %%.%df %%.%df ", n_decimals, n_decimals, n_decimals);
  		sprintf (ps.cmyk_format, "%%.%df %%.%df %%.%df %%.%df ", n_decimals, n_decimals, n_decimals, n_decimals);
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
	if (linewidth == ps.linewidth) return;

	fprintf (ps.fp, "S %g W\n", (double)(linewidth * 72.0 / ps.points_pr_unit));
	ps.linewidth = linewidth;
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
	if (rgb[0] == ps.rgb[0] && rgb[1] == ps.rgb[1] && rgb[2] == ps.rgb[2]) return;	/* Same color as already set */

	fprintf (ps.fp, "S ");
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%c\n", ps_paint_code[pmode]);

	/* Update the current color information */

	ps.rgb[0] = rgb[0];
	ps.rgb[1] = rgb[1];
	ps.rgb[2] = rgb[2];
}

/* fortran interface */
void ps_setpaint_ (int *rgb)
{
	 ps_setpaint (rgb);
}

void ps_square (double x, double y, double diameter, int rgb[], int outline)
{	/* give diameter of circumscribing circle */
	int ds, ix, iy, pmode;
	
	diameter *= 0.707106781187;
	ds = irint (diameter * ps.scale);
	diameter *= 0.5;
	ix = irint ((x - diameter) * ps.scale);
	iy = irint ((y - diameter) * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d S%d\n", ds, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_square_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_square (*x, *y, *diameter, rgb, *outline);
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
 *    9	10	11
 *   |----------------|
 *   5  <textstring>  7
 *   |----------------|
 *   1	 2	 3
 */
	char *string;
	int i = 0, pmode, j, h_just, v_just, rounded;

	if (strlen (text) >= (BUFSIZ-1)) {
		fprintf (stderr, "pslib: text_item > %d long!\n", BUFSIZ);
		return;
	}
	
	rounded = (outline & 4 && dx > 0.0 && dy > 0.0);	/* Want rounded label boxes, assuming there is clearance */
	outline &= 3;	/* Turn off the 4 */
	fprintf (ps.fp, "\n%% ps_textbox begin:\nV\n");

	if (justify < 0)  {	/* Strip leading and trailing blanks */
		for (i = 0; text[i] == ' '; i++);
		for (j = strlen (text) - 1; text[j] == ' '; j--) text[j] = 0;
 		justify = -justify;
	}
	
 	if (pointsize < 0.0) ps_command ("currentpoint /PSL_save_y exch def /PSL_save_x exch def");	/* Must save the current point since ps_textdim will destroy it */
	ps_textdim ("PSL_dimx", "PSL_dimy", fabs (pointsize), ps.font_no, &text[i], 1);			/* Set the string BB dimensions in PS */
 	if (pointsize < 0.0) ps_command ("PSL_save_x PSL_save_y m");					/* Reset to the saved current point */
	ps_set_length ("PSL_dx", dx);
	ps_set_length ("PSL_dy", dy);
	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */
	
  	/* Got to anchor point */
  	
	if (pointsize > 0.0) {	/* Set a new anchor point */
		ps.ix = irint (x * ps.scale);
		ps.iy = irint (y * ps.scale);
		fprintf (ps.fp, "V %d %d T ", ps.ix, ps.iy);
	}
	else
		fprintf (ps.fp, "V PSL_save_x PSL_save_y T ");
	
	if (angle != 0.0) fprintf (ps.fp, "%.3g R ", angle);
	if (justify > 1) {	/* Move the new origin so (0,0) is lower left of box */
		h_just = (justify % 4) - 1;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (ps.fp, "0 PSL_dimx_ur PSL_dimx_ll sub %3.1f mul ", -0.5 * h_just) : fprintf (ps.fp, "0 ") ;
		(v_just) ? fprintf (ps.fp, "PSL_dimy_ur PSL_dimy_ll sub %3.1f mul ", -0.5 * v_just) : fprintf (ps.fp, "0 ");
		fprintf (ps.fp, "T ");
	}
	/* Here, (0,0) is lower point on textbox with no clearance yet */
	if (rounded) {
		fprintf (ps.fp, "/PSL_r %d def\n", irint (MIN (dx, dy) * ps.scale));
		fprintf (ps.fp, "/PSL_dx2 %d def\n", irint ((dx - MIN (dx, dy)) * ps.scale));
		fprintf (ps.fp, "/PSL_dy2 %d def\n", irint ((dy - MIN (dx, dy)) * ps.scale));
		fprintf (ps.fp, "/PSL_x_side PSL_dimx_ur PSL_dimx_ll sub PSL_dx2 2 mul add def\n");
		fprintf (ps.fp, "/PSL_y_side PSL_dimy_ur PSL_dimy_ll sub PSL_dy2 2 mul add def\n");
		fprintf (ps.fp, "/PSL_bx0 PSL_dimx_ll PSL_dx2 sub def\n");
		fprintf (ps.fp, "/PSL_by0 PSL_dimy_ll PSL_dy2 sub def\n");
		fprintf (ps.fp, "PSL_dimx_ll PSL_dx2 sub PSL_dimy_ll PSL_dy sub M PSL_x_side 0 D\n");
		fprintf (ps.fp, "PSL_bx0 PSL_x_side add PSL_by0 PSL_r 270 360 arc\n");
		fprintf (ps.fp, "0 PSL_y_side D PSL_bx0 PSL_x_side add PSL_by0 PSL_y_side add PSL_r 0 90 arc\n");
		fprintf (ps.fp, "PSL_x_side neg 0 D PSL_bx0 PSL_by0 PSL_y_side add PSL_r 90 180 arc 0 PSL_y_side neg D P \n");
	}
	else {
		fprintf (ps.fp, "/PSL_x_side PSL_dimx_ur PSL_dimx_ll sub PSL_dx 2 mul add def\n");
		fprintf (ps.fp, "/PSL_y_side PSL_dimy_ur PSL_dimy_ll sub PSL_dy 2 mul add def\n");
		fprintf (ps.fp, "PSL_dimx_ll PSL_dx sub PSL_dimy_ll PSL_dy sub M PSL_x_side 0 D 0 PSL_y_side D PSL_x_side neg 0 D 0 PSL_y_side neg D P \n");
	}
	if (rgb[0] >= 0) {	/* Paint the textbox */
		fprintf (ps.fp, "V ");
		pmode = ps_place_color (rgb);
		fprintf (ps.fp, "%c F U ", ps_paint_code[pmode]);
	}
	(outline) ? fprintf (ps.fp, "S U\n") : fprintf (ps.fp, "N U\n");
	fprintf (ps.fp, "U\n%% ps_textbox end:\n\n");
	
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
	
  	height = pointsize / ps.points_pr_unit;
  	
 	if (!strchr (string, '@')) {	/* Plain text string */
		fprintf (ps.fp, "0 0 M %d F%d (%s) true charpath flattenpath pathbbox N ", (int) irint (height * ps.scale), ps.font_no, string);
		if (key == 0)
			fprintf (ps.fp, "exch 2 {3 1 roll sub abs} repeat /%s exch def /%s exch def\n", xdim, ydim);
		else
			fprintf (ps.fp, "/%s_ur exch def /%s_ur exch def /%s_ll exch def /%s_ll exch def\n", ydim, xdim, ydim, xdim);
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
	 
	font = old_font = ps.font_no;
	size = height;
	small_size = height * 0.7;
	scap_size = height * 0.85;
	sub = super = small = FALSE;
	
	tempstring = ps_memory (VNULL, (size_t)(strlen(string)+1), sizeof (char));	/* Since strtok steps on it */
	strcpy (tempstring, string);
	ptr = strtok (tempstring, "@");
	fprintf (ps.fp, "N 0 0 m ");	/* Initialize currentpoint */
	if(string[0] != '@') {
		fprintf (ps.fp, "%d F%d (%s) true charpath flattenpath ", irint (size*ps.scale), font, ptr);
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
			font = (font == 12) ? ps.font_no : 12;
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
		else	/* Not recognized or @@ for a single @ */
			strcpy (piece, ptr);
		if (strlen (piece) > 0) fprintf (ps.fp, "%d F%d (%s) true charpath flattenpath ", irint (size*ps.scale), font, piece);
		ptr = strtok ((char *)NULL, "@");
	}

	fprintf (ps.fp, "pathbbox N ");
	if (key == 0)
		fprintf (ps.fp, "exch 2 {3 1 roll sub abs} repeat /%s exch def /%s exch def\n", xdim, ydim);
	else
		fprintf (ps.fp, "/%s_ur exch def /%s_ur exch def /%s_ll exch def /%s_ll exch def\n", ydim, xdim, ydim, xdim);
	
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

	char *piece, *piece2, *ptr, *string, op[16];
	int dy, i = 0, j, font, v_just, h_just;
	int sub, super, small, old_font;
	double height, small_size, size, scap_size, ustep, dstep;

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
 		if (pointsize < 0.0) ps_command ("currentpoint /PSL_save_y exch def /PSL_save_x exch def");	/* Must save the current point since ps_textdim will destroy it */
		ps_textdim ("PSL_dimx", "PSL_dimy", fabs (pointsize), ps.font_no, &text[i], 0);			/* Set the string dimensions in PS */
 		if (pointsize < 0.0) ps_command ("PSL_save_x PSL_save_y m");					/* Reset to the saved current point */
	}
	
	string = ps_prepare_text (&text[i]);	/* Check for escape sequences */
	
 	height = fabs (pointsize) / ps.points_pr_unit;
  	
 	ps.npath = 0;
	
	if (pointsize > 0.0) {	/* Set a new anchor point */
		ps.ix = irint (x * ps.scale);
		ps.iy = irint (y * ps.scale);
		fprintf (ps.fp, "%d %d M ", ps.ix, ps.iy);
	}
	
	if (angle != 0.0) fprintf (ps.fp, "V %.3g R ", angle);
	if (justify > 1) {
		h_just = (justify % 4) - 1;	/* Gives 0 (left justify, i.e., do nothing), 1 (center), or 2 (right justify) */
		v_just = justify / 4;		/* Gives 0 (bottom justify, i.e., do nothing), 1 (middle), or 2 (top justify) */
		(h_just) ? fprintf (ps.fp, "0 PSL_dimx %3.1f mul ", -0.5 * h_just) : fprintf (ps.fp, "0 ") ;
		(v_just) ? fprintf (ps.fp, "PSL_dimy %3.1f mul ", -0.5 * v_just) : fprintf (ps.fp, "0 ");
		fprintf (ps.fp, "G ");
	}
	
	if (!strchr (string, '@')) {	/* Plain text string - do things simply and exit */
		fprintf (ps.fp, "%d F%d (%s) ", (int) irint (height * ps.scale), ps.font_no, string);
		(form == 0) ? fprintf (ps.fp, "Z") : fprintf (ps.fp, "false charpath S");
		(angle != 0.0) ? fprintf (ps.fp, " U\n") : fprintf (ps.fp, "\n");
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
	
	font = old_font = ps.font_no;
	(form == 0) ? strcpy (op, "Z") : strcpy (op, "false charpath");
	sub = super = small = FALSE;
	size = height;
	small_size = height * 0.7;
	scap_size = height * 0.85;
	ustep = 0.35 * height;
	dstep = 0.25 * height;

	ptr = strtok (string, "@");
	if(string[0] != '@') {	/* String has @ but not at start - must deal with first piece explicitly */
		fprintf (ps.fp, "%d F%d (%s) %s\n", irint (size*ps.scale), font, ptr, op);
		ptr = strtok ((char *)NULL, "@");
	}

	while (ptr) {	/* Loop over all the sub-text items separated by escape characters */
		if (ptr[0] == '!') {	/* Composite character */
			ptr++;
			if (ptr[0] == '\\') {	/* Octal code */
				strncpy (piece, ptr, 4);
				piece[4] = 0;
				ptr += 4;
			}
			else {
				piece[0] = ptr[0];	piece[1] = 0;
				ptr++;
			}
			if (ptr[0] == '\\') {	/* Octal code again */
				strncpy (piece2, ptr, 4);
				piece2[4] = 0;
				ptr += 4;
			}
			else {
				piece2[0] = ptr[0];	piece2[1] = 0;
				ptr++;
			}
			/* Try to center justify these two character to make a composite character - may not be right */
			fprintf (ps.fp, "%d F%d (%s) dup stringwidth pop exch %s -2 div dup 0 G\n", irint (size*ps.scale), font, piece2, op);
			fprintf (ps.fp, "%d F%d (%s) E -2 div dup 0 G exch %s sub neg dup 0 lt {pop 0} if 0 G\n", irint (size*ps.scale), font, piece, op);
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '~') {	/* Symbol font */
			font = (font == 12) ? ps.font_no : 12;
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
			dy = (sub) ? irint (-dstep*ps.scale) : irint (dstep*ps.scale);
			fprintf (ps.fp, "0 %d G\n", dy);
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '+') {	/* Superscript */
			super = !super;
			size = (super) ? small_size : height;
			dy = (super) ? irint (ustep*ps.scale) : irint (-ustep*ps.scale);
			fprintf (ps.fp, "0 %d G\n", dy);
			ptr++;
			strcpy (piece, ptr);
		}
		else if (ptr[0] == '#') {	/* Small caps */
			small = !small;
			size = (small) ? scap_size : height;
			ptr++;
			(small) ? get_uppercase (piece, ptr) : (void) strcpy (piece, ptr);
		}
		else
			strcpy (piece, ptr);
		if (strlen (piece) > 0)
			fprintf (ps.fp, "%d F%d (%s) %s\n", irint (size*ps.scale), font, piece, op);
		ptr = strtok ((char *)NULL, "@");
	}
	if (form == 1) fprintf (ps.fp, "S\n");
	if (angle != 0.0) fprintf (ps.fp, "U\n");
	
	ps_free ((void *)piece);
	ps_free ((void *)piece2);
	ps_free ((void *)string);
}

/* fortran interface */
void ps_text_ (double *x, double *y, double *pointsize, char *text, double *angle, int *justify, int *form, int nlen)
{
	ps_text (*x, *y, *pointsize, text, *angle, *justify, *form);
}

void ps_textpath (double x[], double y[], int n, int node[], double angle[], char *label[], int m, double pointsize, double offset[], int justify, int form)
{
	/* x,y		Array containing the label path
	/* n		Length of label path
	 * node		Index into x/y array of label plot positions
	 * angle	Text angle for each label
	 * label	Array of text labels
	 * m		Number of labels
	 * pointsize	Pointsize of label text
	 * offset	Clearences between text and textbox
	 * just		Justification of text relative to label coordinates
	 * form		bits: 1 = clip path, 2 = just place gap, 4 = draw line,
	 *		      8 = just call labelline and reuse last set of parameters
	 *		      32 = first time called, 64 = final time called, 128 = fill box, 256 = draw box
	 */
	 
	int i = 0, j, k, first;
	
	if (form & 8) {		/* If 8 bit is set we already have placed the info */
		form -= 8;		/* Knock off the 8 flag */
		fprintf (ps.fp, "%d PSL_curved_text_labels\n", form);
		return;
	}

	if (m <= 0) return;	/* Nothing to do yet */
	
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
		ps_set_integer ("PSL_just", justify);
		ps_set_length ("PSL_gap_x", offset[0]);
		ps_set_length ("PSL_gap_y", offset[1]);
		if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
 			if (pointsize < 0.0) ps_command ("currentpoint /PSL_save_y exch def /PSL_save_x exch def");	/* Must save the current point since ps_textdim will destroy it */
			ps_textdim ("PSL_dimx", "PSL_height", fabs (pointsize), ps.font_no, label[0], 0);		/* Set the string dimensions in PS */
 			if (pointsize < 0.0) ps_command ("PSL_save_x PSL_save_y m");					/* Reset to the saved current point */
		}
		fprintf (ps.fp, "%d F%d\n", (int) irint ((fabs (pointsize) / ps.points_pr_unit) * ps.scale), ps.font_no);	/* Set font */
	}
	
	/* Set these each time */
	
	n = ps_set_xyn_arrays ("PSL_x", "PSL_y", "PSL_node", x, y, node, n, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
 	ps_set_integer ("PSL_n", n);
	ps_set_integer ("PSL_m", m);
	
	fprintf (ps.fp, "%d PSL_curved_text_labels\n", form);
	
  	ps.npath = 0;
}

/* fortran interface */
void ps_textpath_ (double x[], double y[], int *n, int node[], double angle[], char *label[], int *m, double *pointsize, double offset[], int *justify, int *form, int len)
{
	ps_textpath (x, y, *n, node, angle, label, *m, *pointsize, offset, *justify, *form);
}

void ps_textclip (double x[], double y[], int m, double angle[], char *label[], double pointsize, double offset[], int justify, int key)
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
	 
	int i = 0, j, k;
	
	if (key & 2) {	/* Flag to terminate clipping */
		fprintf (ps.fp, "PSL_clip_on\t\t%% If clipping is active, terminate it\n{\n  grestore\n  /PSL_clip_on false def\n} if\n");
		return;
	}
	if (key & 8) {		/* Flag to place text already define in PSL arrays */
		fprintf (ps.fp, "%d PSL_straight_text_labels\n", key);
		return;
	}
	
	/* Here key == 0 (or 4) which means we plan to create labeltext clip paths (and paint them) */
	
	if (m <= 0) return;	/* Nothing to do yet */
	
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

	/* fprintf (ps.fp, "gsave\n"); */
	ps_set_integer ("PSL_m", m);
	ps_set_length_array ("PSL_txt_x", x, m);
	ps_set_length_array ("PSL_txt_y", y, m);
	ps_set_real_array ("PSL_angle", angle, m);
	ps_set_txt_array ("PSL_str", label, m);
	ps_set_integer ("PSL_just", justify);
	ps_set_length ("PSL_gap_x", offset[0]);
	ps_set_length ("PSL_gap_y", offset[1]);
		
 	if (justify > 1) {	/* Only Lower Left (1) is already justified - all else must move */
 		if (pointsize < 0.0) ps_command ("currentpoint /PSL_save_y exch def /PSL_save_x exch def");	/* Must save the current point since ps_textdim will destroy it */
		ps_textdim ("PSL_dimx", "PSL_height", fabs (pointsize), ps.font_no, label[0], 0);		/* Set the string dimensions in PS */
 		if (pointsize < 0.0) ps_command ("PSL_save_x PSL_save_y m");					/* Reset to the saved current point */
	}
	
	fprintf (ps.fp, "%d F%d\n", (int) irint ((fabs (pointsize) / ps.points_pr_unit) * ps.scale), ps.font_no);	/* Set font */
	fprintf (ps.fp, "%d PSL_straight_text_labels\n", key);
	
  	ps.npath = 0;
}

/* fortran interface */
void ps_textclip_ (double x[], double y[], int *m, double angle[], char *label[], double *pointsize, double offset[], int *justify, int *key, int len)
{
	ps_textclip (x, y, *m, angle, label, *pointsize, offset, *justify, *key);
}

void ps_transrotate (double x, double y, double angle)
{
	int go = FALSE;
	
	if (x != 0.0 || y != 0.0) {
		if (fabs(x) < 1e-9) x = 0.0;
		if (fabs(y) < 1e-9) y = 0.0;
		fprintf (ps.fp, "%g %g T", x * ps.scale, y * ps.scale);
		go = TRUE;
	}
	if (angle != 0.0) {
		if (fabs(angle) < 1e-9) angle = 0.0;
		if (go) fputc (' ', ps.fp);
		fprintf (ps.fp, "%g R", angle);
	}
	fputc ('\n', ps.fp);
}

/* fortran interface */
void ps_transrotate_ (double *x, double *y, double *angle)
{
	ps_transrotate (*x, *y, *angle);
}

void ps_triangle (double x, double y, double diameter, int rgb[], int outline)
{	/* Give diameter of circumscribing circle */
	int ix, iy, is, pmode;
	
	ix = irint ((x-0.433012701892*diameter) * ps.scale);
	iy = irint ((y-0.25*diameter) * ps.scale);
	is = irint (0.866025403784 * diameter * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d T%d\n", is, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_triangle_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_triangle (*x, *y, *diameter, rgb, *outline);
}

void ps_itriangle (double x, double y, double diameter, int rgb[], int outline)	/* Inverted triangle */
{	/* Give diameter of circumscribing circle */
	int ix, iy, is, pmode;
	
	ix = irint ((x-0.433012701892*diameter) * ps.scale);
	iy = irint ((y+0.25*diameter) * ps.scale);
	is = irint (0.866025403784 * diameter * ps.scale);
	pmode = ps_place_color (rgb);
	fprintf (ps.fp, "%d %d %d I%d\n", is, ix, iy, outline + ps_outline_offset[pmode]);
	ps.npath = 0;
}

/* fortran interface */
void ps_itriangle_ (double *x, double *y, double *diameter, int *rgb, int *outline)
{
	ps_itriangle (*x, *y, *diameter, rgb, *outline);
}

void ps_vector (double xtail, double ytail, double xtip, double ytip, double tailwidth, double headlength, double headwidth, double headshape, int rgb[], int outline)
{
	/* Will make sure that arrow has a finite width in PS coordinates */
	
	double angle;
	int w2, length, hw, hl, hl2, hw2, l2, pmode;
	
	length = irint (hypot ((xtail-xtip), (ytail-ytip)) * ps.scale);	/* Vector length in PS units */
	if (length == 0) return;					/* NULL vector */

	angle = atan2 ((ytip-ytail),(xtip-xtail)) * R2D;					/* Angle vector makes with horizontal, in radians */
	fprintf (ps.fp, "V %d %d T ", irint (xtail * ps.scale), irint (ytail * ps.scale));	/* Temporarily set tail point the local origin (0, 0) */
	if (angle != 0.0) fprintf (ps.fp, "%g R ", angle);					/* Rotate so vector is horizontal in local coordinate system */
	w2 = irint (0.5 * tailwidth * ps.scale);	if (w2 == 0) w2 = 1;			/* Half-width of vector tail */
	hw = irint (headwidth * ps.scale);	if (hw == 0) hw = 1;				/* Width of vector head */
	hl = irint (headlength * ps.scale);							/* Length of vector head */
	hl2 = irint (0.5 * headshape * headlength * ps.scale);					/* Cut-in distance due to slanted back-side of arrow head */
	hw2 = hw - w2;										/* Distance from tail side to head side (vertically) */
	if (outline & 8) {	/* Double-headed vector */
		outline -= 8;	/* Remove the flag */
		l2 = length - 2 * hl + 2 * hl2;							/* Inside length between start of heads */
		pmode = ps_place_color (rgb);
		fprintf (ps.fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d a%d U\n",
				hl2, hw2, -l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -hl2, hw2, hl, -hw, outline+ps_outline_offset[pmode]);
	}
	else {			/* Single-headed vector */
		l2 = length - hl + hl2;								/* Length from tail to start of slanted head */
		pmode = ps_place_color (rgb);
		fprintf (ps.fp, "%d %d %d %d %d %d %d %d %d %d %d A%d U\n",
			-l2, hl2, -hw2, -hl, hw, hl, hw, -hl2, -hw2, l2, -w2, outline+ps_outline_offset[pmode]);
	}
}

/* fortran interface */
void ps_vector_ (double *xtail, double *ytail, double *xtip, double *ytip, double *tailwidth, double *headlength, double *headwidth, double *headshape, int *rgb, int *outline)
{
	 ps_vector (*xtail, *ytail, *xtip, *ytip, *tailwidth, *headlength, *headwidth, *headshape, rgb, *outline);
}

/* Support functions used in ps_* functions.  No Fortran bindings needed */

void get_uppercase (char *new, char *old)
{
	int i = 0, c;
	while (old[i]) {
		c = (int)old[i];
		new[i++] = toupper (c);
	}
	new[i] = 0;
}

void ps_encode_font (int font_no)
{
	if (ps.encoding == 0) return;		/* Already have StandardEncoding by default */
	if (ps.font[font_no].encoded) return;	/* Already reencoded or should not be reencoded ever */

	/* Reencode fonts with Standard+ or ISOLatin1[+] encodings */
	fprintf (ps.fp, "PSL_font_encode %d get 0 eq { %% Set this font\n", font_no);
	fprintf (ps.fp, "\t%s_Encoding /%s /%s PSL_reencode\n", ps.encoding, ps.font[font_no].name, ps.font[font_no].name);
	fprintf (ps.fp, "\tPSL_font_encode %d 1 put\n} if\n", font_no);
	ps.font[font_no].encoded = TRUE;
}

void init_font_encoding (struct EPS *eps)
{	/* Reencode all the fonts that we know may be used: the ones listed in eps */

	int i;

	if (eps)
		for (i = 0; i < 6 && eps->fontno[i] != -1; i++) ps_encode_font (eps->fontno[i]);
	else	/* Must output all */
		for (i = 0; i < N_FONTS; i++) ps_encode_font (i);
}

void def_font_encoding (void)
{	/* Place code for reencoding of fonts and initialize book-keeping array */

	int i;
	
	fprintf (ps.fp, "/PSL_reencode {\t%% To reencode one font with the provided encoding vector\n");
	fprintf (ps.fp, "\tfindfont dup length dict begin\n");
	fprintf (ps.fp, "\t{1 index /FID ne {def} {pop pop} ifelse} forall\n");
	fprintf (ps.fp, "\texch /Encoding exch def currentdict end definefont pop\n");
	fprintf (ps.fp, "} bind def\n");

	/* Initialize T/F array for font reencoding so that we only do it once
	 * for each font that is used */

	fprintf (ps.fp, "/PSL_font_encode ");
	for (i = 0; i < N_FONTS; i++) fprintf (ps.fp, "0 ");
	fprintf (ps.fp, "%d array astore def	%% Initially zero\n", N_FONTS);
}

char *ps_prepare_text (char *text)
           
/*	Adds escapes for misc parenthesis, brackets etc.
	Will also translate to some Norwegian characters from the @a, @e
	etc escape sequences. Calling function must REMEMBER to free memory
	allocated by string */
{
	char *string;
	int i=0, j=0, font;
	int he = 0;		/* GMT Historical Encoding (if any) */

	if (strcmp ("Standard", ps.encoding) == 0)
		he = 1;
	if (strcmp ("Standard+", ps.encoding) == 0)
		he = 2;
	/* ISOLatin1 and ISOLatin1+ are the same _here_. */
	if (strncmp ("ISOLatin1", ps.encoding, 9) == 0)
		he = 3;

	string = ps_memory (NULL, 2 * BUFSIZ, sizeof(char));
	while (text[i]) {
		if (he && text[i] == '@') {
			i++;
			switch (text[i]) {
				case 'A':
					strcat (string, ps_scandcodes[0][he-1]);
					j += strlen(ps_scandcodes[0][he-1]); i++;
					break;
				case 'E':
					strcat (string, ps_scandcodes[1][he-1]);
					j += strlen(ps_scandcodes[1][he-1]); i++;
					break;
				case 'O':
					strcat (string, ps_scandcodes[2][he-1]);
					j += strlen(ps_scandcodes[2][he-1]); i++;
					break;
				case 'a':
					strcat (string, ps_scandcodes[3][he-1]);
					j += strlen(ps_scandcodes[3][he-1]); i++;
					break;
				case 'e':
					strcat (string, ps_scandcodes[4][he-1]);
					j += strlen(ps_scandcodes[4][he-1]); i++;
					break;
				case 'o':
					strcat (string, ps_scandcodes[5][he-1]);
					j += strlen(ps_scandcodes[5][he-1]); i++;
					break;
				case '@':
/*    Also now converts "@@" to the octal code for "@" = "\100".
       This was necessary since the system routine "strtok" gobbles up
       multiple @'s when parsing the string inside "ps_text", and thus
       didn't properly output a single "@" sign when encountering "@@".
       John L. Lillibridge: 4/6/95 [This was a problem on SGI; PW]
*/

					strcat (string, "\\100"); j += 4; i++;
					break;
				case '%':	/* Font switcher */
					if (isdigit (text[i+1])) {	/* Got a font */
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
					if (j > 0 && string[MAX(j-1,0)] == '\\')        /* ALREADY ESCAPED... */
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

unsigned char *ps_loadraster (char *file, struct rasterfile *header, BOOLEAN invert, BOOLEAN monochrome, BOOLEAN colorize, int f_rgb[], int b_rgb[])
{
	/* ps_loadraster reads a Sun standard rasterfile of depth 1,8,24, or 32 into memory */

	int mx_in, mx, j, k, i, ij, n = 0, ny, get, odd, oddlength, r_off, b_off;
	unsigned char *buffer, *entry, *red, *green, *blue, *tmp, rgb[3];
	FILE *fp;

	if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "pslib: Cannot open rasterfile %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (ps_read_rasheader (fp, header)) {
		fprintf (stderr, "pslib: Trouble reading Sun rasterfile header!\n");
		exit (EXIT_FAILURE);
	}
	
	if (header->ras_magic != RAS_MAGIC) {	/* Not a Sun rasterfile */
		fprintf (stderr, "pslib: Raster is not a Sun rasterfile (Magic # = 0x%x)!\n", header->ras_magic);
		exit (EXIT_FAILURE);
	}
	if (header->ras_type < RT_OLD || header->ras_type > RT_FORMAT_RGB) {
		fprintf (stderr, "pslib: Can only read Sun rasterfiles types %d - %d (your type = %d)!\n", RT_OLD, RT_FORMAT_RGB, header->ras_type);
		exit (EXIT_FAILURE);
	}

	buffer = entry = red = green = blue = (unsigned char *)NULL;
	
	if (header->ras_depth == 1) {	/* 1 bit black and white image */
		mx_in = (int) (2 * ceil (header->ras_width / 16.0));	/* Because Sun images are written in multiples of 2 bytes */
		mx = (int) (ceil (header->ras_width / 8.0));		/* However, PS wants only the bytes that matters, so mx may be one less */
		ny = header->ras_height;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 1-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		
		if (mx < mx_in) {	/* OK, here we must shuffle image to get rid of the superfluous last byte per line */
			for (j = k = ij = 0; j < ny; j++) {
				for (i = 0; i < mx; i++) buffer[k++] = buffer[ij++];
				ij++;	/* Skip the extra byte */
			}
		}
		
		if (invert) {
			unsigned int int4, bit8 = 255, endmask;
			int mx1;

			n = header->ras_width % 8;
			mx1 = mx - 1;
			endmask = (n) ? ((1 << n) - 1) << (8 - n) : ~0;
			for (j = k = 0; j < ny; j++) {
				for (i = 0; i < mx; i++, k++) {
					int4 = buffer[k];
					if (i == mx1) {
						int4 = ~int4;
						int4 &= endmask;
					}
					else
						int4 = ~int4;
					buffer[k] = int4 & bit8;
				}
			}
		}
		if (colorize) {	/* Convert from 1-bit to 24-bit */
			tmp = ps_1bit_to_24bit (buffer, header, f_rgb, b_rgb);
			ps_free ((void *)buffer);
			buffer = tmp;
		}
	}
	else if (header->ras_depth == (size_t)8 && header->ras_maplength == (size_t)0) {	/* 8-bit without color table (implicit grayramp) */
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 8-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
	}
	else if (header->ras_depth == 8) {	/* 8-bit with color table */
		get = header->ras_maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if (n != header->ras_maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		odd = (int)header->ras_width%2;
		entry = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)entry, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 8-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &entry);
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)(3 * header->ras_width * header->ras_height), sizeof (unsigned char));
		for (j = k = ij = 0; j < header->ras_height; j++) {
			for (i = 0; i < header->ras_width; i++) {
				buffer[k++] = red[entry[ij]];
				buffer[k++] = green[entry[ij]];
				buffer[k++] = blue[entry[ij]];
				ij++;
			}
			if (odd) ij++;
		}
		header->ras_depth = 24;
	}
	else if (header->ras_depth == 24 && header->ras_maplength) {	/* 24-bit raster with colormap */
		unsigned char r, b;
		get = header->ras_maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if ((size_t)n != (size_t)header->ras_maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 24-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		oddlength = 3 * header->ras_width;
		odd = (3 * header->ras_width) % 2;
		r_off = (header->ras_type == RT_FORMAT_RGB) ? 0 : 2;
		b_off = (header->ras_type == RT_FORMAT_RGB) ? 2 : 0;
		for (i = j = 0; i < header->ras_length; i += 3, j += 3) {	/* BGR -> RGB */
			r =  red[buffer[i+r_off]];
			b = blue[buffer[i+b_off]];
			buffer[j] = r;
			buffer[j+1] = green[buffer[i+1]];
			buffer[j+2] = b;
			if (odd && (j+3)%oddlength == 0) i++;
		}
	}
	else if (header->ras_depth == (size_t)24 && header->ras_maplength == (size_t)0) {	/* 24-bit raster, no colormap */
		unsigned char r, b;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 24-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		oddlength = 3 * header->ras_width;
		odd = (3 * header->ras_width) % 2;
		r_off = (header->ras_type == RT_FORMAT_RGB) ? 0 : 2;
		b_off = (header->ras_type == RT_FORMAT_RGB) ? 2 : 0;
		for (i = j = 0; i < header->ras_length; i += 3, j += 3) {	/* BGR -> RGB */
			r = buffer[i+r_off];
			b = buffer[i+b_off];
			buffer[j] = r;
			buffer[j+1] = buffer[i+1];
			buffer[j+2] = b;
			if (odd && (j+3)%oddlength == 0) i++;
		}
	}
	else if (header->ras_depth == 32 && header->ras_maplength) {	/* 32-bit raster with colormap */
		unsigned char b;
		get = header->ras_maplength / 3;
		red   = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		green = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		blue  = (unsigned char *) ps_memory (VNULL, (size_t)get, sizeof (unsigned char));
		n  = fread ((void *)red,   (size_t)1, (size_t)get, fp);
		n += fread ((void *)green, (size_t)1, (size_t)get, fp);
		n += fread ((void *)blue,  (size_t)1, (size_t)get, fp);
		if ((size_t)n != (size_t)header->ras_maplength) {
			fprintf (stderr, "%s: Error reading colormap!\n", "pslib");
			return ((unsigned char *)NULL);
		}
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 32-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		r_off = (header->ras_type == RT_FORMAT_RGB) ? 1 : 3;
		b_off = (header->ras_type == RT_FORMAT_RGB) ? 3 : 1;
		b = blue[buffer[b_off]];
		buffer[0] = red[buffer[r_off]];
		buffer[1] = green[buffer[2]];
		buffer[2] = b;
		for (i = 3, j = 4; j < header->ras_length; i += 3, j += 4) {	/* _BGR -> RGB */
			buffer[i] = red[buffer[j+r_off]];
			buffer[i+1] = green[buffer[j+2]];
			buffer[i+2] = blue[buffer[j+b_off]];
		}
	}
	else if (header->ras_depth == (size_t)32 && header->ras_maplength == (size_t)0) {	/* 32-bit raster, no colormap */
		unsigned char b;
		buffer = (unsigned char *) ps_memory (VNULL, (size_t)header->ras_length, sizeof (unsigned char));
		if (fread ((void *)buffer, (size_t)1, (size_t)header->ras_length, fp) != (size_t)header->ras_length) {
			fprintf (stderr, "pslib: Trouble reading 32-bit Sun rasterfile!\n");
			exit (EXIT_FAILURE);
		}
		if (header->ras_type == RT_BYTE_ENCODED) ps_rle_decode (header, &buffer);
		r_off = (header->ras_type == RT_FORMAT_RGB) ? 1 : 3;
		b_off = (header->ras_type == RT_FORMAT_RGB) ? 3 : 1;
		b = buffer[b_off];
		buffer[0] = buffer[r_off];
		buffer[1] = buffer[2];
		buffer[2] = b;
		for (i = 3, j = 4; j < header->ras_length; i += 3, j += 4) {	/* _BGR -> RGB */
			buffer[i] = buffer[j+r_off];
			buffer[i+1] = buffer[j+2];
			buffer[i+2] = buffer[j+b_off];
		}
	}
	else	/* Unrecognized format */
		return ((unsigned char *)NULL);
		
	fclose (fp);

	if (monochrome && header->ras_depth > 1) {
		for (i = j = 0; i < header->ras_width * header->ras_height; i++, j += 3)
		{
			memcpy ((void *)rgb, (void *)&buffer[j], 3 * sizeof(unsigned char));
			buffer[i] = (unsigned char) YIQ (rgb);
		}
		header->ras_depth = 8;
	}
		
	if (entry) ps_free ((void *)entry);
	if (red) ps_free ((void *)red);
	if (green) ps_free ((void *)green);
	if (blue) ps_free ((void *)blue);

	
	return (buffer);	
}

unsigned char *ps_1bit_to_24bit (unsigned char *pattern, struct rasterfile *h, int f_rgb[], int b_rgb[])
{
	/* This routine accepts a 1-bit image and turns it into a 24-bit image using the
	   specified foreground and background colors.  Input image pattern has rows that
	   are multiples of 8 bits. */

	int color_choice[2][3], mx, extra, step, i, j, k, kk, id, nx, ny, m, my;
	unsigned int p;
	unsigned char *rgb;

	/* Set fore- and background color choices */

	color_choice[0][0] = b_rgb[0];	color_choice[0][1] = b_rgb[1];	color_choice[0][2] = b_rgb[2];
	color_choice[1][0] = f_rgb[0];	color_choice[1][1] = f_rgb[1];	color_choice[1][2] = f_rgb[2];

	nx = h->ras_width;
	ny = h->ras_height;

	rgb = (unsigned char *) ps_memory (VNULL, (size_t)(3 * nx * ny), sizeof (unsigned char));

	mx = nx / 8;			/* Number of full 8-bit byte in 1-bit image */
	extra = nx - mx * 8;		/* Remainder of bits in the last byte in 1-bit image */
	step = (extra) ? mx + 1: mx;	/* Number of bytes per row */

	for (j = kk = my = 0; j < ny; j++, my += step) {		/* For each row in image */

		for (i = 0, m = my; i < mx; i++, m++) {	/* For each chunk of full 8 bits */

			for (k = 0; k < 8; k++) {	/* Deal with each bit */

				p = (128 >> k);
				id = (((unsigned int)(pattern[m]) & p) == 0);	/* 0 = background, 1 = foreground */
				rgb[kk++] = color_choice[id][0];
				rgb[kk++] = color_choice[id][1];
				rgb[kk++] = color_choice[id][2];

			}
		}
		if (extra) {	/* Deal with remainder of bits in last short (m is already incremented) */
			for (k = 0; k < extra; k++) {
				p = (128 >> k);
				id = (((unsigned int)(pattern[m]) & p) == 0);
				rgb[kk++] = color_choice[id][0];
				rgb[kk++] = color_choice[id][1];
				rgb[kk++] = color_choice[id][2];
			}
		}
	}

	/* Fill out header structure for rasterfile */

	h->ras_depth = 24;
	h->ras_length = 3 * nx * ny;
	h->ras_maptype = RMT_NONE;
	h->ras_maplength = 0;

	return (rgb);
}

#define ESC 128

void ps_rle_decode (struct rasterfile *h, unsigned char **in)
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

	int i, j, col, count, width, len, odd = FALSE;
	unsigned char mask_table[] = {0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
	unsigned char mask, *out, value = 0;

	i = j = col = count = 0;

	width = irint (ceil (h->ras_width * h->ras_depth / 8.0));	/* Scanline width in bytes */
	if (width%2) odd = TRUE, width++;	/* To ensure 16-bit words */
	mask = mask_table[h->ras_width%8];	/* Padding for 1-bit images */

	len = width * h->ras_height;		/* Length of output image */
	out = (unsigned char *) ps_memory (VNULL, (size_t)len, sizeof (unsigned char));
	if (odd) width--;

	while (j < h->ras_length || count > 0) {

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
			if (h->ras_depth == 1) out[width-1] &= mask;
			if (odd) out[i++] = count = 0;
			col = 0;
		}
	}

	if (i != len) fprintf (stderr, "pslib: ps_rle_decode has wrong # of outbytes (%d versus expected %d)\n", i, len);

	ps_free ((void *)*in);

	*in = out;
}

int ps_read_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Reads the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	int i, j, value, in[4];

	for (i = 0; i < 8; i++) {

		if (fread ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) {
			fprintf (stderr, "pslib: Error reading rasterfile header\n");
			return (-1);
		}
		for (j = 0; j < 4; j++) in[j] = (int)byte[j];

		value = (in[0] << 24) + (in[1] << 16) + (in[2] << 8) + in[3];

		switch (i) {
			case 0:
				h->ras_magic = value;
				break;
			case 1:
				h->ras_width = value;
				break;
			case 2:
				h->ras_height = value;
				break;
			case 3:
				h->ras_depth = value;
				break;
			case 4:
				h->ras_length = value;
				break;
			case 5:
				h->ras_type = value;
				break;
			case 6:
				h->ras_maptype = value;
				break;
			case 7:
				h->ras_maplength = value;
				break;
		}
	}

	if (h->ras_type == RT_OLD && h->ras_length == 0) h->ras_length = 2 * irint (ceil (h->ras_width * h->ras_depth / 16.0)) * h->ras_height;

	return (0);
}

int ps_write_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Writes the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	int i, j, value, in[4];

	for (i = 0; i < 8; i++) {
		switch (i) {
			case 0:
				value = h->ras_magic;
				break;
			case 1:
				value = h->ras_width;
				break;
			case 2:
				value = h->ras_height;
				break;
			case 3:
				value = h->ras_depth;
				break;
			case 4:
				value = h->ras_length;
				break;
			case 5:
				value = h->ras_type;
				break;
			case 6:
				value = h->ras_maptype;
				break;
			case 7:
				value = h->ras_maplength;
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

void ps_words (double x, double y, char **text, int n_words, double line_space, double par_width, int par_just, int font, double font_size, double angle, int rgb[3], int justify, int draw_box, double x_off, double y_off, double x_gap, double y_gap, int boxpen_width, char *boxpen_texture, int boxpen_offset, int boxpen_rgb[], int vecpen_width, char *vecpen_texture, int vecpen_offset, int vecpen_rgb[], int boxfill_rgb[3])
{
	int i, i1, i0, j, k, n, pj, pmode, error = 0, last_font, last_rgb[3];
	int n_scan, after, color, found, last_k = -1;
	int *rgb_list, *rgb_unique, n_rgb_unique;
	int *font_list, *font_unique, n_font_unique;
	size_t n_alloc, n_items;
	BOOLEAN sub, super, small, plain_word = FALSE, under, escape;
	char *c, *clean, test_char;
	double last_size;
	struct GMT_WORD **word;
	struct GMT_WORD *add_word_part (char *word, int length, int fontno, double font_size, BOOLEAN sub, BOOLEAN super, BOOLEAN small, BOOLEAN under, int space, int rgb[]);

	sub = super = small = under = FALSE;
	if (draw_box & 64) {	/* Smart offsets follow justification */
		if ((justify & 3) == 3)  x_off = -x_off;
		if ((justify >> 2) == 2) y_off = -y_off;
	}

	n_alloc = n_words;
	last_font = font;
	last_size = font_size;

	word = (struct GMT_WORD **) ps_memory (VNULL, n_alloc, sizeof (struct GMT_WORD *));

	for (i = k = 0; i < n_words; i++) {

		clean = ps_prepare_text (text[i]);	/* Escape special characters and Scandinavian shorthands */

		if ((c = strchr (clean, '@'))) {	/* Found a @ escape command */
			i0 = 0;
			i1 = (int) (c - clean);

			if (i1 > i0) word[k++] = add_word_part (&clean[i0], i1 - i0, font, font_size, sub, super, small, under, NO_SPACE, rgb);
			if ((size_t)k == n_alloc) {
				n_alloc += PSL_CHUNK;
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
							word[k++] = add_word_part (&clean[i1], 4, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k++] = add_word_part (&clean[i1], 1, font, font_size, sub, super, small, under, COMPOSITE_1, rgb);
							i1++;
						}
						if ((size_t)k == n_alloc) {
							n_alloc += PSL_CHUNK;
							word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
						}
						if (clean[i1] == '\\') { /* 2nd char is Octal code character */
							word[k] = add_word_part (&clean[i1], 4, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1 += 4;
						}
						else {	/* Regular character */
							word[k] = add_word_part (&clean[i1], 1, font, font_size, sub, super, small, under, COMPOSITE_2, rgb);
							i1++;
						}
						if (!clean[i1]) word[k]->flag++;	/* New word after this composite */
						k++;
						if ((size_t)k == n_alloc) {
							n_alloc += PSL_CHUNK;
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
							n_alloc += PSL_CHUNK;
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
			word[k++] = add_word_part (clean, 0, font, font_size, sub, super, small, under, ONE_SPACE, rgb);
			if ((size_t)k == n_alloc) {
				n_alloc += PSL_CHUNK;
				word = (struct GMT_WORD **) ps_memory ((void *)word, n_alloc, sizeof (struct GMT_WORD *));
			}
		}

		ps_free ((void *)clean);	/* Reclaim this memory */

	} /* End of word loop */

	k--;
	while (k && !word[k]->txt) k--;	/* Skip any blank lines at end */
	n_items = k + 1;

	for (i0 = 0, i1 = 1 ; i1 < (int)n_items-1; i1++, i0++) {	/* Loop for periods ending sentences and indicate 2 spaces to follow */
		if (isupper ((int)word[i1]->txt[0]) && word[i0]->txt[strlen(word[i0]->txt)-1] == '.') {
			word[i0]->flag &= 60;	/* Sets bits 1 & 2 to zero */
			word[i0]->flag |= 2;	/* Specify 2 spaces */
		}
		if (!word[i1]->txt[0]) {	/* No space at end of paragraph */
			word[i0]->flag &= 60;
			word[i1]->flag &= 60;
		}
	}
	if (i1 >= (int)n_items) i1 = (int)n_items - 1;	/* one-word fix */
	word[i1]->flag &= 60;	/* Last word not followed by anything */

	/* Determine list of unique colors */

	rgb_list = (int *) ps_memory (VNULL, n_items, sizeof (int));
	rgb_unique = (int *) ps_memory (VNULL, n_items, sizeof (int));

	for (i = 0; i < (int)n_items; i++) rgb_list[i] = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
	qsort ((void *)rgb_list, (size_t) n_items, sizeof (int), ps_comp_int_asc);
	rgb_unique[0] = rgb_list[0];
	n_rgb_unique = 1;
	k = 0;
	for (i = 1; i < (int)n_items; i++) {
		if (rgb_list[i] != rgb_list[k]) {	/* New color */
			rgb_unique[n_rgb_unique++] = rgb_list[i];
			k = i;
		}
	}
	ps_free ((void *)rgb_list);

	/* Replace each word's red value with the index of the corresponding unique color entry */
	
	for (i = 0; i < (int)n_items; i++) {
		color = (word[i]->rgb[0] << 16) + (word[i]->rgb[1] << 8) + word[i]->rgb[2];
		for (j = 0, found = -1; found < 0 && j < n_rgb_unique; j++) if (color == rgb_unique[j]) found = j;
		word[i]->rgb[0] = found;
	}

	/* Determine list of unique fonts */

	font_list = (int *) ps_memory (VNULL, n_items, sizeof (int));
	font_unique = (int *) ps_memory (VNULL, n_items, sizeof (int));

	for (i = 0; i < (int)n_items; i++) font_list[i] = word[i]->font_no;
	qsort ((void *)font_list, (size_t) n_items, sizeof (int), ps_comp_int_asc);
	font_unique[0] = font_list[0];
	n_font_unique = 1;
	k = 0;
	for (i = 1; i < (int)n_items; i++) {
		if (font_list[i] != font_list[k]) {	/* New font */
			font_unique[n_font_unique++] = font_list[i];
			k = i;
		}
	}
	ps_free ((void *)font_list);

	/* Replace each word's font with the index of the corresponding unique font entry */
	
	for (i = 0; i < (int)n_items; i++) {
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
			fprintf (stderr, "%s: Bad paragraph justification (%c) - Exiting\n", "pslib", par_just);
			exit (EXIT_FAILURE);
	}

	/* Time to write out to PS file */

	/* Load PSL_text procedures from file for now */

	if (PSL_text_first) {
		bulkcopy ("PSL_text");
		PSL_text_first = FALSE;
	}

	fprintf (ps.fp, "\n%% ps_words begin:\n\ngsave\n");

	fprintf (ps.fp, "\n%% Define array of fonts:\n\n/PSL_fontname\n");
	for (i = 0 ; i < n_font_unique; i++) fprintf (ps.fp, "/%s\n", ps.font[font_unique[i]].name);
	fprintf (ps.fp, "%d array astore def\n", n_font_unique);
	ps_free ((void *)font_unique);

	fprintf (ps.fp, "\n%% Initialize variables:\n\n");
	fprintf (ps.fp, "/PSL_n %d def\n", (int)n_items);
	fprintf (ps.fp, "/PSL_n1 %d def\n", (int)n_items - 1);
	fprintf (ps.fp, "/PSL_y0 %d def\n", irint (y * ps.scale));
	fprintf (ps.fp, "/PSL_linespace %d def\n", irint (line_space * ps.scale));
	fprintf (ps.fp, "/PSL_parwidth %d def\n", irint (par_width * ps.scale));
	fprintf (ps.fp, "/PSL_parjust %d def\n", pj);
	fprintf (ps.fp, "/PSL_spaces [() ( ) (  ) ] def\n");
	(draw_box & 1) ? fprintf (ps.fp, "/PSL_drawbox true def\n") : fprintf (ps.fp, "/PSL_drawbox false def\n");
	(draw_box & 2) ? fprintf (ps.fp, "/PSL_fillbox true def\n") : fprintf (ps.fp, "/PSL_fillbox false def\n");
	fprintf (ps.fp, "/PSL_boxshape %d def\n", draw_box & 4);
	fprintf (ps.fp, "/PSL_lastfn -1 def\n/PSL_lastfs -1 def\n/PSL_lastfc -1 def\n");
	fprintf (ps.fp, "/PSL_UL 0 def\n/PSL_show {ashow} def\n");

	fprintf (ps.fp, "\n%% Define array of words:\n\n/PSL_word\n");
	for (i = n = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "(%s)", word[i]->txt);
		n += strlen (word[i]->txt) + 1;
		if (n < 60)
			fputc (' ', ps.fp);
		else {
			n = 0;
			fputc ('\n', ps.fp);
		}
	}
	if (n) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word font numbers:\n\n/PSL_fnt\n");
	for (i = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "%d", word[i]->font_no);
		(!((i+1)%25)) ? fputc ('\n', ps.fp) : fputc (' ', ps.fp);
	}
	if ((i%25)) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word fontsizes:\n\n/PSL_size\n");
	for (i = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "%.2f", word[i]->font_size);
		(!((i+1)%20)) ? fputc ('\n', ps.fp) : fputc (' ', ps.fp);
	}
	if ((i%20)) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word spaces to follow:\n\n/PSL_flag\n");
	for (i = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "%d", word[i]->flag);
		(!((i+1)%25)) ? fputc ('\n', ps.fp) : fputc (' ', ps.fp);
	}
	if ((i%25)) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word baseline shifts:\n\n/PSL_bshift\n");
	for (i = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "%g", word[i]->baseshift);
		(!((i+1)%25)) ? fputc ('\n', ps.fp) : fputc (' ', ps.fp);
	}
	if ((i%25)) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word colors indices:\n\n/PSL_color\n");
	for (i = 0 ; i < (int)n_items; i++) {
		fprintf (ps.fp, "%d", word[i]->rgb[0]);
		(!((i+1)%25)) ? fputc ('\n', ps.fp) : fputc (' ', ps.fp);
	}
	if ((i%25)) fputc ('\n', ps.fp);
	fprintf (ps.fp, "%d array astore def\n", (int)n_items);

	fprintf (ps.fp, "\n%% Define array of word colors:\n\n/PSL_rgb\n");
	for (i = 0 ; i < n_rgb_unique; i++) fprintf (ps.fp, "%.3g %.3g %.3g\n", I_255 * (rgb_unique[i] >> 16), I_255 * ((rgb_unique[i] >> 8) & 0xFF), I_255 * (rgb_unique[i] & 0xFF));
	fprintf (ps.fp, "%d array astore def\n", 3 * n_rgb_unique);
	ps_free ((void *)rgb_unique);

	fprintf (ps.fp, "\n%% Define array of word widths:\n\n");
	fprintf (ps.fp, "/PSL_width %d array def\n", (int)n_items);
	fprintf (ps.fp, "0 1 PSL_n1 {	%% Determine word width given the font and fontsize for each word\n");
	fprintf (ps.fp, "  /i exch def	%% Loop index i\n");
	fprintf (ps.fp, "  PSL_size i get PSL_fontname PSL_fnt i get get Y	%% Get and set font and size\n");
	fprintf (ps.fp, "  PSL_width i PSL_word i get stringwidth pop put		%% Calculate and store width\n");
	fprintf (ps.fp, "} for\n");

	fprintf (ps.fp, "\n%% Define array of word char counts:\n\n");
	fprintf (ps.fp, "/PSL_count %d array def\n", (int)n_items);
	fprintf (ps.fp, "0 1 PSL_n1 {PSL_count exch dup PSL_word exch get length put} for\n");

	fprintf (ps.fp, "\n%% For composite chars, set width and count to zero for 2nd char:\n\n");
	fprintf (ps.fp, "1 1 PSL_n1 {\n  /k exch def\n  PSL_flag k get 16 and 16 eq {\n");
	fprintf (ps.fp, "    /k1 k 1 sub def\n    /w1 PSL_width k1 get def\n    /w2 PSL_width k get def\n");
	fprintf (ps.fp, "    PSL_width k1 w1 w2 gt {w1} {w2} ifelse put\n    PSL_width k 0 put\n");
	fprintf (ps.fp, "    PSL_count k 0 put\n  } if\n} for\n\n");

	ps_transrotate (x, y, angle);	/* To original point */

	if (draw_box & 32) {	/* Draw line from box to point */
		ps_setline (vecpen_width);
		ps_setpaint (vecpen_rgb);
		if (vecpen_texture) ps_setdash (vecpen_texture, vecpen_offset);
		fprintf (ps.fp, "0 0 M %d %d D S\n", irint (x_off * ps.scale), irint (y_off * ps.scale));
		if (vecpen_texture) ps_setdash (CNULL, 0);
	}

	ps_transrotate (x_off, y_off, 0.0);	/* Adjust for shift */

	/* Do the relative horizontal justification */

	fprintf (ps.fp, "0 0 M\n\n0 PSL_textjustifier %% Just get paragraph height\n\n");

	/* Adjust origin for box justification */

	fprintf (ps.fp, "/PSL_x0 %d def\n", -irint (0.5 * ((justify - 1) % 4) * par_width * ps.scale));
	if (justify > 8) {	/* Top row */
		fprintf (ps.fp, "/PSL_y0 0 def\n");
	}
	else if (justify > 4) {	/* Middle row */
		fprintf (ps.fp, "/PSL_y0 PSL_parheight 0.5 mul def\n");
	}
	else {			/* Bottom row */
		fprintf (ps.fp, "/PSL_y0 PSL_parheight def\n");
	}
	fprintf (ps.fp, "/PSL_txt_y0 PSL_top neg def\n");

	/* Make upper left textbox corner the origin */

	fprintf (ps.fp, "\nPSL_x0 PSL_y0 T\n\n");

	if (draw_box) {
		fprintf (ps.fp, "%% Start PSL box beneath text block:\n");
		ps_setline (boxpen_width);
		ps_setpaint (boxpen_rgb);
		if (boxpen_texture) ps_setdash (boxpen_texture, boxpen_offset);
		fprintf (ps.fp, "/PSL_xgap %d def\n", irint (x_gap * ps.scale));
		fprintf (ps.fp, "/PSL_ygap %d def\n", irint (y_gap * ps.scale));
		if (draw_box & 16) {	/* Create convex box path */
			fprintf (ps.fp, "/PSL_h PSL_parheight 2 div PSL_ygap add def\n");
			fprintf (ps.fp, "/PSL_w PSL_parwidth 2 div PSL_xgap add def\n");
			fprintf (ps.fp, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
			fprintf (ps.fp, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
			fprintf (ps.fp, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
			fprintf (ps.fp, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
			fprintf (ps.fp, "%% PSL_path:\n");
			fprintf (ps.fp, "PSL_xgap neg PSL_ygap M\n");
			fprintf (ps.fp, "PSL_ry PSL_xgap 2 mul sub PSL_parheight 2 div neg PSL_ry 180 PSL_ay sub 180 PSL_ay add arc\n");
			fprintf (ps.fp, "PSL_parwidth 2 div PSL_parheight 2 PSL_ygap mul add PSL_rx sub neg PSL_rx 270 PSL_ax sub 270 PSL_ax add arc\n");
			fprintf (ps.fp, "PSL_parwidth PSL_xgap 2 mul add PSL_ry sub PSL_parheight 2 div neg PSL_ry PSL_ay dup neg exch arc\n");
			fprintf (ps.fp, "PSL_parwidth 2 div PSL_ygap 2 mul PSL_rx sub PSL_rx 90 PSL_ax sub 90 PSL_ax add arc P\n");
		}
		else if (draw_box & 8) {	/* Create concave box path */
			fprintf (ps.fp, "/PSL_h PSL_parheight 2 div PSL_ygap 2 mul add def\n");
			fprintf (ps.fp, "/PSL_w PSL_parwidth 2 div PSL_xgap 2 mul add def\n");
			fprintf (ps.fp, "/PSL_rx PSL_w PSL_w mul PSL_xgap PSL_xgap mul add 2 PSL_xgap mul div def\n");
			fprintf (ps.fp, "/PSL_ry PSL_h PSL_h mul PSL_ygap PSL_ygap mul add 2 PSL_ygap mul div def\n");
			fprintf (ps.fp, "/PSL_ax PSL_w PSL_rx PSL_xgap sub atan def\n");
			fprintf (ps.fp, "/PSL_ay PSL_h PSL_ry PSL_ygap sub atan def\n");
			fprintf (ps.fp, "%% PSL_path:\n");
			fprintf (ps.fp, "PSL_xgap 2 mul neg PSL_ygap 2 mul M\n");
			fprintf (ps.fp, "PSL_xgap PSL_ry add neg PSL_parheight 2 div neg PSL_ry PSL_ay dup neg arcn\n");
			fprintf (ps.fp, "PSL_parwidth 2 div PSL_parheight PSL_ygap add PSL_rx add neg PSL_rx 90 PSL_ax add 90 PSL_ax sub arcn\n");
			fprintf (ps.fp, "PSL_parwidth PSL_xgap add PSL_ry add PSL_parheight 2 div neg PSL_ry 180 PSL_ay add 180 PSL_ay sub arcn\n");
			fprintf (ps.fp, "PSL_parwidth 2 div PSL_ygap PSL_rx add PSL_rx 270 PSL_ax add 270 PSL_ax sub arcn P\n");
		}
		else if (draw_box & 4) {	/* Create rounded box path */
			fprintf (ps.fp, "/XL PSL_xgap neg def\n");
			fprintf (ps.fp, "/XR PSL_parwidth PSL_xgap add def\n");
			fprintf (ps.fp, "/YT PSL_ygap def\n");
			fprintf (ps.fp, "/YB PSL_parheight PSL_ygap add neg def\n");
			fprintf (ps.fp, "/PSL_r %d def\n", irint (MIN (x_gap, y_gap) * ps.scale));
			fprintf (ps.fp, "/PSL_dx %d def\n", irint (MAX (x_gap-y_gap, 0.0) * ps.scale));
			fprintf (ps.fp, "/PSL_dx %d def\n", irint (MAX (x_gap-y_gap, 0.0) * ps.scale));
			fprintf (ps.fp, "/PSL_dy %d def\n", irint (MAX (y_gap-x_gap, 0.0) * ps.scale));
			fprintf (ps.fp, "/xl PSL_dx def\n");
			fprintf (ps.fp, "/xr PSL_parwidth PSL_dx add def\n");
			fprintf (ps.fp, "/yt PSL_dy def\n");
			fprintf (ps.fp, "/yb PSL_parheight PSL_dy add neg def\n");
			fprintf (ps.fp, "%% PSL_path:\n");
			fprintf (ps.fp, "XL yt M XL yb L\n");
			fprintf (ps.fp, "xl yb PSL_r 180 270 arc xr YB L\n");
			fprintf (ps.fp, "xr yb PSL_r 270 360 arc XR yt L\n");
			fprintf (ps.fp, "xr yt PSL_r 0 90 arc xl YT L\n");
			fprintf (ps.fp, "xl yt PSL_r 90 180 arc P\n");
		}
		else {
			fprintf (ps.fp, "/XL PSL_xgap neg def\n");
			fprintf (ps.fp, "/XR PSL_parwidth PSL_xgap add def\n");
			fprintf (ps.fp, "/YT PSL_ygap def\n");
			fprintf (ps.fp, "/YB PSL_parheight PSL_ygap add neg def\n");
			fprintf (ps.fp, "%% PSL_path:\n");
			fprintf (ps.fp, "XL YT M XL YB L XR YB L XR YT L P\n");
		}
		if (draw_box & 2) {	/* Fill */
			fprintf (ps.fp, "V ");
			pmode = ps_place_color (boxfill_rgb);
			fprintf (ps.fp, "%c F U ", ps_paint_code[pmode]);
		}
		if (draw_box & 1) {	/* Stroke */
			pmode = ps_place_color (boxpen_rgb);
			fprintf (ps.fp, "%c ", ps_paint_code[pmode]);
			fprintf (ps.fp, "S\n");
		}
		else 
			fprintf (ps.fp, "N\n");
		if (boxpen_texture) ps_setdash (CNULL, 0);
		/* Because inside gsave/grestore we must reset ps.pen and ps.rgb so that they are set next time */
		ps.rgb[0] = ps.rgb[1] = ps.rgb[2] = ps.linewidth = -1;
		fprintf (ps.fp, "%% End PSL box beneath text block:\n");
	}
	/* Adjust origin so 0,0 is lower left corner of first character on baseline */

	fprintf (ps.fp, "0 PSL_txt_y0 T %% Move to col 0 on first baseline\n");
	fprintf (ps.fp, "\n0 0 M\n\n1 PSL_textjustifier %% Place the paragraph\n\n");

	fprintf (ps.fp, "grestore\n\n");

	ps_free ((void *)word);
}

/* fortran interface */
void ps_words_ (double *x, double *y, char **text, int *n_words, double *line_space, double *par_width, int *par_just, int* font, double *font_size, double *angle, int *rgb, int *justify, int *draw_box, double *x_off, double *y_off, double *x_gap, double *y_gap, int *boxpen_width, char *boxpen_texture, int *boxpen_offset, int *boxpen_rgb, int *vecpen_width, char *vecpen_texture, int *vecpen_offset, int *vecpen_rgb, int *boxfill_rgb, int n1, int n2, int n3) {

	ps_words (*x, *y, text, *n_words, *line_space, *par_width, *par_just, *font, *font_size, *angle, rgb, *justify, *draw_box, *x_off, *y_off, *x_gap, *y_gap, *boxpen_width, boxpen_texture, *boxpen_offset, boxpen_rgb, *vecpen_width, vecpen_texture, *vecpen_offset, vecpen_rgb, boxfill_rgb);

}

struct GMT_WORD *add_word_part (char *word, int length, int fontno, double font_size, BOOLEAN sub, BOOLEAN super, BOOLEAN small, BOOLEAN under, int space, int rgb[])
{
	/* For flag: bits 1 and 2 give number of spaces to follow (0, 1, or 2)
	 * bit 3 == 1 means leading TAB
	 * bit 4 == 1 means Composite 1 character
	 * bit 5 == 1 means Composite 2 character
	 * bit 6 == 1 means underline word
	 */

	int i = 0, c;
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
  	fs = font_size * ps.scale / ps.points_pr_unit;

	strncpy (new->txt, &word[i], length);
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

void ps_set_length (char *param, double value)
{
	fprintf (ps.fp, "/%s %d def\n", param, irint (value * ps.scale));
}

void ps_set_height (char *param, double fontsize)
{
	fprintf (ps.fp, "/%s %d def\n", param, irint (fontsize * ps.scale / ps.points_pr_unit));
}

void ps_set_integer (char *param, int value)
{
	fprintf (ps.fp, "/%s %d def\n", param, value);
}

void ps_define_pen (char *param, int width, char *texture, int offset, int rgb[])
{
	int k;
	/* Function to set line pen attributes */
	fprintf (ps.fp, "/%s { ", param);
	k = ps_place_color (rgb);
	fprintf (ps.fp, "%c %d W ", ps_paint_code[k], width);
	ps_place_setdash (texture, offset);
	fprintf (ps.fp, " } def\n");
}

void ps_define_rgb (char *param, int rgb[])
{
	int k;
	fprintf (ps.fp, "/%s { ", param);
	k = ps_place_color (rgb);
	fprintf (ps.fp, "%c } def\n", ps_paint_code[k]);
}

void ps_set_length_array (char *param, double *array, int n)
{	/* These are scaled by psscale */
	int i;
	fprintf (ps.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (ps.fp, "%.2lf\n", array[i] * ps.scale);
	fprintf (ps.fp, "%d array astore def\n", n);
}

int ps_set_xyn_arrays (char *xparam, char *yparam, char *nparam, double *x, double *y, int *node, int n, int m)
{	/* These are scaled by psscale.  We make sure there are no point pairs that would yield dx = dy = 0 (repeat point)
	 * at the resolution we are using (0.01 DPI units), hence a new n (possibly shorter) is returned. */
	int i, j, k, this_i, this_j, last_i, last_j, n_skipped;
	char *use;
	
	use = (char *) ps_memory (VNULL, (size_t)n, sizeof (char));
	this_i = this_j = INT_MAX;
	for (i = j = k = n_skipped = 0; i < n; i++) {
		last_i = this_i;	last_j = this_j;
		this_i = irint (x[i] * ps.scale * 100.0);	/* Simulates the digits written by a %.2lf format */
		this_j = irint (y[i] * ps.scale * 100.0);
		if (this_i != last_i && this_j != last_j) {	/* Not a repeat point, use it */
			use[i] = TRUE;
			j++;
		}
		else	/* Repeat point, skip it */
			n_skipped++;
		if (k < m && node[k] == i && n_skipped) node[k++] -= n_skipped;	/* Adjust node pointer since we are removing points and upsetting the order */
	}
	fprintf (ps.fp, "/%s\n", xparam);
	for (i = 0; i < n; i++) if (use[i]) fprintf (ps.fp, "%.2lf\n", x[i] * ps.scale);
	fprintf (ps.fp, "%d array astore def\n", j);
	fprintf (ps.fp, "/%s\n", yparam);
	for (i = 0; i < n; i++) if (use[i]) fprintf (ps.fp, "%.2lf\n", y[i] * ps.scale);
	fprintf (ps.fp, "%d array astore def\n", j);
	fprintf (ps.fp, "/%s\n", nparam);
	for (i = 0; i < m; i++) fprintf (ps.fp, "%d\n", node[i]);
	fprintf (ps.fp, "%d array astore def\n", m);
	
	ps_free ((void *)use);
	return (j);
}

void ps_set_real_array (char *param, double *array, int n)
{	/* These are raw and not scaled */
	int i;
	fprintf (ps.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (ps.fp, "%.2lf\n", array[i]);
	fprintf (ps.fp, "%d array astore def\n", n);
}

void ps_set_txt_array (char *param, char *array[], int n)
{
	int i;
	fprintf (ps.fp, "/%s\n", param);
	for (i = 0; i < n; i++) fprintf (ps.fp, "(%s)\n", array[i]);
	fprintf (ps.fp, "%d array astore def\n", n);
}

void *ps_memory (void *prev_addr, size_t nelem, size_t size)
{
	void *tmp;

	if (nelem == 0) return (VNULL); /* Take care of n = 0 */
	
	if (prev_addr) {
		if ((tmp = realloc ((void *) prev_addr, (size_t)(nelem * size))) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not reallocate more memory, n = %d\n", (int)nelem);
			exit (EXIT_FAILURE);
		}
	}
	else {
		if ((tmp = calloc ((size_t) nelem, (unsigned) size)) == VNULL) {
			fprintf (stderr, "PSL Fatal Error: Could not allocate memory, n = %d\n", (int)nelem);
			exit (EXIT_FAILURE);
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

/* This function copies a file called $GMTHOME/share/pslib/???.ps
 * to the postscript output verbatim.
 */
static void bulkcopy (const char *fname)
{
	FILE *in;
	size_t nread;
	char buf[80];
	char fullname[BUFSIZ];

	sprintf (fullname, "%s%cshare%cpslib%c%s.ps", PSHOME, DIR_DELIM, DIR_DELIM, DIR_DELIM, fname);

	in = fopen (fullname, "r");
	if (in == NULL)
	{
		fprintf (stderr, "PSL Fatal Error: ");
		perror (fullname);
		exit (EXIT_FAILURE);
	}
	while ((nread = fread (buf, 1, sizeof (buf), in)) > 0)
		fwrite (buf, 1, nread, ps.fp);
	fclose (in);
 }

static void ps_init_fonts (int *n_fonts, int *n_GMT_fonts)
{
	FILE *in;
	int i = 0, n_alloc = 50;
	char buf[BUFSIZ];
	char fullname[BUFSIZ];

	/* Loads the available fonts for this installation */
	
	/* First the standard 35 PostScript fonts from Adobe */
	
	sprintf (fullname, "%s%cshare%cpslib%cPS_font_info.d", PSHOME, DIR_DELIM, DIR_DELIM, DIR_DELIM);

	if ((in = fopen (fullname, "r")) == NULL)
	{
		fprintf (stderr, "PSL Fatal Error: ");
		perror (fullname);
		exit (EXIT_FAILURE);
	}
	
	ps.font = (struct PS_FONT *) ps_memory (VNULL, (size_t)n_alloc, sizeof (struct PS_FONT));
	
	while (fgets (buf, 128, in)) {
		if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
		if (sscanf (buf, "%s %lf %d", fullname, &ps.font[i].height, &ps.font[i].encoded) != 3) {
			fprintf (stderr, "PSL Fatal Error: Trouble decoding font info for font %d\n", i);
			exit (EXIT_FAILURE);
		}
		ps.font[i].name = (char *)ps_memory (VNULL, (size_t)(strlen (fullname)+1), sizeof (char));
		strcpy (ps.font[i].name, fullname);
		i++;
		if (i == n_alloc) {
			n_alloc += 50;
			ps.font = (struct PS_FONT *) ps_memory ((void *)ps.font, (size_t)n_alloc, sizeof (struct PS_FONT));
		}
	}
	fclose (in);
	*n_fonts = *n_GMT_fonts = i;
	
 	/* Then any custom fonts */
	
	sprintf (fullname, "%s%cshare%cpslib%cCUSTOM_font_info.d", PSHOME, DIR_DELIM, DIR_DELIM, DIR_DELIM);

	if (!access (fullname, R_OK)) {	/* Decode Custom font file */
	
		if ((in = fopen (fullname, "r")) == NULL)
		{
			fprintf (stderr, "PSL Fatal Error: ");
			perror (fullname);
			exit (EXIT_FAILURE);
		}
	
		while (fgets (buf, 128, in)) {
			if (buf[0] == '#' || buf[0] == '\n' || buf[0] == '\r') continue;
			ps.font[i].name = (char *)ps_memory (VNULL, strlen (buf), sizeof (char));
			if (sscanf (buf, "%s %lf %d", ps.font[i].name, &ps.font[i].height, &ps.font[i].encoded) != 3) {
				fprintf (stderr, "PSL Fatal Error: Trouble decoding custom font info for font %d\n", i - *n_GMT_fonts);
				exit (EXIT_FAILURE);
			}
			i++;
			if (i == n_alloc) {
				n_alloc += 50;
				ps.font = (struct PS_FONT *) ps_memory ((void *)ps.font, (size_t)n_alloc, sizeof (struct PS_FONT));
			}
		}
		fclose (in);
		*n_fonts = i;
	}
	ps.font = (struct PS_FONT *) ps_memory ((void *)ps.font, (size_t)(*n_fonts), sizeof (struct PS_FONT));
}

int ps_place_color (int rgb[])
{
	int pmode;
	
	if (rgb[0] < 0)	return (3);	/* Outline only, no color set */
	
	if (iscolor (rgb)) {
		if (ps.cmyk_mode) {
			double cmyk[4];
			ps_rgb_to_cmyk (rgb, cmyk);
			fprintf (ps.fp, ps.cmyk_format, cmyk[0], cmyk[1], cmyk[2], cmyk[3]);
			pmode = 2;
		}
		else {
			fprintf (ps.fp, ps.rgb_format, rgb[0] * I_255, rgb[1] * I_255, rgb[2] * I_255);
			pmode = 1;
		}
	}
	else {
		fprintf (ps.fp, ps.bw_format, rgb[0] * I_255);
		pmode = 0;
	}
	return (pmode);
}

void ps_rgb_to_cmyk (int rgb[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	
	int i;
	
	/* RGB is in 0-255, CMYK will be in 0-1 range */
	
	for (i = 0; i < 3; i++) cmyk[i] = 1.0 - (rgb[i] * I_255);
	cmyk[3] = MIN (cmyk[0], MIN (cmyk[1], cmyk[2]));	/* Black */
	for (i = 0; i < 3; i++) cmyk[i] -= cmyk[3];
}

void ps_cmyk_to_rgb (int rgb[], double cmyk[])
{
	/* Plain conversion; no undercolor removal or blackgeneration */
	
	int i;
	
	/* CMYK is in 0-1, RGB will be in 0-255 range */
	
	for (i = 0; i < 3; i++) rgb[i] = (int) floor ((1.0 - cmyk[i] - cmyk[3]) * 255.999);
}
