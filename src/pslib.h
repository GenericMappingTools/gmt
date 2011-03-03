/*--------------------------------------------------------------------
 *	$Id: pslib.h,v 1.64 2011-03-03 21:02:51 guru Exp $
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
 *
 * This include file must be included by all programs using pslib.a
 *
 * Author:	Paul Wessel
 * Version:	4.3 [64-bit enabled edition]
 * Date:	20-MAR-2008
 */

#ifndef _PSLIB_H
#define _PSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Declaration of type PSL_LONG */

#ifdef _WIN64
typedef __int64 PSL_LONG;		/* A signed 8-byte integer */
#define PSL_LL "ll"
#else
typedef long PSL_LONG;			/* A signed 4 (or 8-byte for 64-bit) integer */
#define PSL_LL "l"
#endif

/* Declaration modifiers for DLL support (MSC et al) */

#if defined(DLL_PSL)		/* define when library is a DLL */
#if defined(DLL_EXPORT)		/* define when building the library */
#define MSC_EXTRA_PSL __declspec(dllexport)
#else
#define MSC_EXTRA_PSL __declspec(dllimport)
#endif
#else
#define MSC_EXTRA_PSL
#endif				/* defined(DLL_PSL) */

/* Unless DLL_PSL is defined, EXTERN_MSC is simply extern */

#ifndef EXTERN_MSC
#define EXTERN_MSC extern MSC_EXTRA_PSL
#endif

/* Macro for exit since this should be returned when called from Matlab */

#ifdef DO_NOT_EXIT
#define PS_exit(code) return(code)
#else
#define PS_exit(code) exit(code)
#endif

#define PSL_PEN_MOVE		3
#define PSL_PEN_DRAW		+2
#define PSL_PEN_DRAW_AND_STROKE	-2
#define PSL_ARC_BEGIN		1
#define PSL_ARC_END		2
#define PSL_ARC_DRAW		3

#define PSL_MAX_EPS_FONTS	6

struct EPS {    /* Holds info for eps files */
/* For Encapsulated PostScript Headers:

	You will need to supply a pointer to an EPS structure in order to
	get correct information in the EPS header.  If you pass a NULL pointer
	instead you will get default values for the BoundingBox plus no
	info is provided about the users name, document title, and fonts used.
	To fill in the structure you must:

	- Determine the extreme dimensions of your plot in points (1/72 inch).
	- Supply the user's name (or NULL)
	- Supply the document's title (or NULL)
	- Set the font values to the ids of the fonts used  First unused font must be set
 	  to -1.  E.g., if 4 fonts are used, font[0], font[1], font[2], and
	  font[3] must contain the integer ID of these fonts; font[4] = -1
*/
	double x0, x1, y0, y1;		/* Bounding box values in points */
	int portrait;			/* TRUE if start of plot was portrait */
	int clip_level;			/* Add/sub 1 as we clip/unclip - should end at 0 */
	int fontno[PSL_MAX_EPS_FONTS];	/* Array with font ids used (skip if -1). 6 is max fonts used in GMT anot/labels */
	char *name;			/* User name */
	char *title;			/* Plot title */
};

struct imageinfo {
	int magic;		/* magic number */
	int width;		/* width (pixels) of image */
	int height;		/* height (pixels) of image */
	int depth;		/* depth (1, 8, or 24 bits) of pixel; 0 for EPS */
	int length;		/* length (bytes) of image */
	int type;		/* type of file; see RT_* below */
	int maptype;	/* type of colormap; see RMT_* below */
	int maplength;	/* length (bytes) of following map */
	int xorigin;	/* x-coordinate of origin (EPS only) */
	int yorigin;	/* y-coordinate of origin (EPS only) */
	/* color map follows for maplength bytes, followed by image */
};

#define	RAS_MAGIC	0x59a66a95	/* Magic number for Sun rasterfile */
#define EPS_MAGIC	0x25215053	/* Magic number for EPS file */
#define RT_OLD		0		/* Old-style, unencoded Sun rasterfile */
#define RT_STANDARD	1		/* Standard, unencoded Sun rasterfile */
#define RT_BYTE_ENCODED	2		/* Run-length-encoded Sun rasterfile */
#define RT_FORMAT_RGB	3		/* [X]RGB instead of [X]BGR Sun rasterfile */
#define RMT_NONE	0		/* maplength is expected to be 0 */
#define RMT_EQUAL_RGB	1		/* red[maplength/3], green[], blue[] follow */

/* Public functions */

EXTERN_MSC void ps_arc (double x, double y, double radius, double az1, double az2, PSL_LONG status);
EXTERN_MSC void ps_axis (double x, double y, double length, double val0, double val1, double annotation_int, char *label, double annotpointsize, PSL_LONG side);
EXTERN_MSC void ps_bitimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG invert, int f_rgb[], int b_rgb[]);
EXTERN_MSC void ps_circle (double x, double y, double size, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_clipoff (void);
EXTERN_MSC void ps_clipon (double *x, double *y, PSL_LONG n, int rgb[], PSL_LONG flag);
EXTERN_MSC void ps_colorimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
EXTERN_MSC void ps_colortiles (double x0, double y0, double xsize, double ysize, unsigned char *image, PSL_LONG nx, PSL_LONG ny);
EXTERN_MSC void ps_command (char *text);
EXTERN_MSC void ps_comment (char *text);
EXTERN_MSC void ps_cross (double x, double y, double size);
EXTERN_MSC void ps_encode_font (PSL_LONG font_no);
EXTERN_MSC void ps_dash (double x0, double y0, double x1, double y1);
EXTERN_MSC void ps_diamond (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_ellipse (double x, double y, double angle, double major, double minor, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_epsimage (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG size, PSL_LONG nx, PSL_LONG ny, PSL_LONG ox, PSL_LONG oy); 
EXTERN_MSC void ps_flush (void);
EXTERN_MSC void *ps_memory (void *prev_addr, PSL_LONG nelem, size_t size);
EXTERN_MSC void ps_free (void *addr);
EXTERN_MSC void ps_hexagon (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_image (double x, double y, double xsize, double ysize, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
EXTERN_MSC PSL_LONG ps_line (double *x, double *y, PSL_LONG n, PSL_LONG type, PSL_LONG close);
EXTERN_MSC void ps_itriangle (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_matharc (double x, double y, double radius, double az1, double az2, double shape, PSL_LONG status);
EXTERN_MSC void ps_octagon (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_pentagon (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC PSL_LONG ps_pattern (PSL_LONG image_no, char *imagefile, PSL_LONG invert, PSL_LONG image_dpi, PSL_LONG outline, int f_rgb[], int b_rgb[]);
EXTERN_MSC void ps_pie (double x, double y, double radius, double az1, double az2, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_plot (double x, double y, int pen);
EXTERN_MSC PSL_LONG ps_plotinit (char *plotfile, PSL_LONG overlay, PSL_LONG mode, double xoff, double yoff, double xscl, double yscl, PSL_LONG ncopies, PSL_LONG dpi, PSL_LONG unit, PSL_LONG *page_size, int *rgb, const char *encoding, struct EPS *eps);
EXTERN_MSC PSL_LONG ps_plotinit_hires (char *plotfile, PSL_LONG overlay, PSL_LONG mode, double xoff, double yoff, double xscl, double yscl, PSL_LONG ncopies, PSL_LONG dpi, PSL_LONG unit, double *page_size, int *rgb, const char *encoding, struct EPS *eps);
EXTERN_MSC void ps_plotend (PSL_LONG lastpage);
EXTERN_MSC void ps_plotr (double x, double y, int pen);
EXTERN_MSC void ps_plus (double x, double y, double size);
EXTERN_MSC void ps_point (double x, double y, double diameter);
EXTERN_MSC void ps_polygon (double *x, double *y, PSL_LONG n, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_rect (double x1, double y1, double x2, double y2, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_rotaterect (double x, double y, double angle, double x_len, double y_len, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_patch (double *x, double *y, PSL_LONG np, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_rotatetrans (double x, double y, double angle);
EXTERN_MSC void ps_segment (double x0, double y0, double x1, double y1);
EXTERN_MSC void ps_setdash (char *pattern, PSL_LONG offset);
EXTERN_MSC void ps_setfill (int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_setfont (PSL_LONG font_no);
EXTERN_MSC void ps_setformat (PSL_LONG n_decimals);
EXTERN_MSC void ps_setline (PSL_LONG linewidth);
EXTERN_MSC void ps_setlinecap (PSL_LONG cap);
EXTERN_MSC void ps_setlinejoin (PSL_LONG join);
EXTERN_MSC void ps_setmiterlimit (PSL_LONG limit);
EXTERN_MSC void ps_setpaint (int rgb[]);
EXTERN_MSC void ps_square (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_star (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_text (double x, double y, double pointsize, char *text, double angle, PSL_LONG justify, PSL_LONG form);
EXTERN_MSC void ps_textbox (double x, double y, double pointsize, char *text, double angle, PSL_LONG justify, PSL_LONG outline, double dx, double dy, int rgb[]);
EXTERN_MSC void ps_textpath (double x[], double y[], PSL_LONG n, PSL_LONG node[], double angle[], char *label[], PSL_LONG m, double pointsize, double offset[], PSL_LONG justify, PSL_LONG form);
EXTERN_MSC void ps_textclip (double x[], double y[], PSL_LONG m, double angle[], char *label[], double pointsize, double offset[], PSL_LONG justify, PSL_LONG key);
EXTERN_MSC void ps_transrotate (double x, double y, double angle);
EXTERN_MSC void ps_triangle (double x, double y, double side, int rgb[], PSL_LONG outline);
EXTERN_MSC void ps_vector (double xtail, double ytail, double xtip, double ytip, double tailwidth, double headlength, double headwidth, double headshape, int rgb[], PSL_LONG outline);
EXTERN_MSC unsigned char *ps_load_image (char *file, struct imageinfo *header);
EXTERN_MSC void ps_words (double x, double y, char **text, PSL_LONG n_words, double line_space, double par_width, PSL_LONG par_just, PSL_LONG font, double font_size, double angle, int rgb[3], PSL_LONG justify, PSL_LONG draw_box, double x_off, double y_off, double x_gap, double y_gap, PSL_LONG boxpen_width, char *boxpen_texture, PSL_LONG boxpen_offset, int boxpen_rgb[], PSL_LONG vecpen_width, char *vecpen_texture, PSL_LONG vecpen_offset, int vecpen_rgb[], int boxfill_rgb[3]);
EXTERN_MSC void ps_setline (PSL_LONG linewidth);
EXTERN_MSC void ps_textdim (char *dim, double pointsize, PSL_LONG font, char *text);
EXTERN_MSC void ps_set_length (char *param, double value);
EXTERN_MSC void ps_set_height (char *param, double fontsize);
EXTERN_MSC void ps_define_rgb (char *param, int rgb[]);
EXTERN_MSC void ps_define_pen (char *param, PSL_LONG width, char *texture, PSL_LONG offset, int rgb[]);
EXTERN_MSC void ps_rgb_to_mono (unsigned char *buffer, struct imageinfo *h);
EXTERN_MSC PSL_LONG ps_read_rasheader  (FILE *fp, struct imageinfo *h, PSL_LONG i0, PSL_LONG i1);
EXTERN_MSC PSL_LONG ps_write_rasheader (FILE *fp, struct imageinfo *h, PSL_LONG i0, PSL_LONG i1);

#ifdef __cplusplus
}
#endif

#endif	/* _PSLIB_H */
