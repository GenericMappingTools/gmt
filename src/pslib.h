/*--------------------------------------------------------------------
 *	$Id: pslib.h,v 1.9 2001-10-02 21:32:52 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 *
 * This include file must be included by all programs using pslib.a
 *
 * Author:	Paul Wessel
 * Version:	4.0
 * Date:	15-SEP-2001
 */

#ifndef _PSLIB_INC_H	/* User calling */

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

#ifndef EXTERN_MSC
#define EXTERN_MSC extern MSC_EXTRA_PSL
#endif

/* So unless DLL_PSL is defined, EXTERN_MSC is simply extern */

#endif	/* _PSLIB_INC_H */

struct EPS {    /* Holds info for eps files */
        int x0, x1, y0, y1;     /* Bounding box values in points */
	int portrait;		/* TRUE if start of plot was portrait */
	int clip_level;		/* Add/sub 1 as we clip/unclip - should end at 0 */
        char *font[N_FONTS];    /* Pointers to font names used */
        int fontno[N_FONTS];    /* Array with font ids */
        char *name;             /* User name */
        char *title;            /* Plot title */
};

struct rasterfile {
        int     ras_magic;              /* magic number */
        int     ras_width;              /* width (pixels) of image */
        int     ras_height;             /* height (pixels) of image */
        int     ras_depth;              /* depth (1, 8, or 24 bits) of pixel */
        int     ras_length;             /* length (bytes) of image */
        int     ras_type;               /* type of file; see RT_* below */
        int     ras_maptype;            /* type of colormap; see RMT_* below */
        int     ras_maplength;          /* length (bytes) of following map */
        /* color map follows for ras_maplength bytes, followed by image */
};

#define	RAS_MAGIC	0x59a66a95	/* Magic number for Sun rasterfile */
#define RT_OLD		0	/* Old-style, unencoded Sun rasterfile */
#define RT_STANDARD	1	/* Standard, unencoded Sun rasterfile */
#define RT_BYTE_ENCODED	2	/* Run-length-encoded Sun rasterfile */
#define RT_FORMAT_RGB	3	/* [X]RGB instead of [X]BGR Sun rasterfile */
#define RMT_NONE	0       /* ras_maplength is expected to be 0 */
#define RMT_EQUAL_RGB	1       /* red[ras_maplength/3], green[], blue[] follow */

EXTERN_MSC int ps_line(double *x, double *y, int n, int type, int close, int split);
EXTERN_MSC int ps_plotinit(char *plotfile, int overlay, int mode, double xoff, double yoff, double xscl, double yscl, int ncopies, int dpi, int unit, int *page_width, int *rgb, struct EPS *eps);
EXTERN_MSC void ps_arc(double x, double y, double radius, double az1, double az2, int status);
EXTERN_MSC void ps_axis(double x, double y, double length, double val0, double val1, double anotation_int, char *label, int anotpointsize, int side);
EXTERN_MSC void ps_circle(double x, double y, double size, int rgb[], int outline);
EXTERN_MSC void ps_clipoff(void);
EXTERN_MSC void ps_clipon(double *x, double *y, int n, int rgb[], int flag);
EXTERN_MSC void ps_colorimage(double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits);
EXTERN_MSC void ps_colortiles(double x0, double y0, double xsize, double ysize, unsigned char *image, int nx, int ny);
EXTERN_MSC void ps_command(char *text);
EXTERN_MSC void ps_comment(char *text);
EXTERN_MSC void ps_cross(double x, double y, double size);
EXTERN_MSC void ps_encode_font(int font_no);
EXTERN_MSC void ps_dash(double x0, double y0, double x1, double y1);
EXTERN_MSC void ps_diamond(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_ellipse(double x, double y, double angle, double major, double minor, int rgb[], int outline);
EXTERN_MSC void ps_flush(void);
EXTERN_MSC void ps_free(void *addr);
EXTERN_MSC void ps_hexagon(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_image(double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int nbits);
EXTERN_MSC void ps_imagefill(double *x, double *y, int n, int image_no, char *imagefile, int invert, int image_dpi, int outline, BOOLEAN colorize, int f_rgb[], int b_rgb[]);
EXTERN_MSC int ps_imagefill_init(int image_no, char *imagefile, int invert, int image_dpi, BOOLEAN colorize, int f_rgb[], int b_rgb[]);
EXTERN_MSC void ps_imagemask (double x, double y, double xsize, double ysize, unsigned char *buffer, int nx, int ny, int polarity, int rgb[]);
EXTERN_MSC void ps_itriangle(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_pie(double x, double y, double radius, double az1, double az2, int rgb[], int outline);
EXTERN_MSC void ps_plot(double x, double y, int pen);
EXTERN_MSC void ps_plotend(int lastpage);
EXTERN_MSC void ps_plotr(double x, double y, int pen);
EXTERN_MSC void ps_polygon(double *x, double *y, int n, int rgb[], int outline);
EXTERN_MSC void ps_rect(double x1, double y1, double x2, double y2, int rgb[], int outline);
EXTERN_MSC void ps_patch(double *x, double *y, int np, int rgb[], int outline);
EXTERN_MSC void ps_rotatetrans(double x, double y, double angle);
EXTERN_MSC void ps_setdash(char *pattern, int offset);
EXTERN_MSC void ps_setfont(int font_no);
EXTERN_MSC void ps_setformat(int n_decimals);
EXTERN_MSC void ps_setline(int linewidth);
EXTERN_MSC void ps_setpaint(int rgb[]);
EXTERN_MSC void ps_square(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_star(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_text(double x, double y, int pointsize, char *text, double angle, int justify, int form);
EXTERN_MSC void ps_textbox(double x, double y, int pointsize, char *text, double angle, int justify, int outline, double dx, double dy, int rgb[]);
EXTERN_MSC void ps_transrotate(double x, double y, double angle);
EXTERN_MSC void ps_triangle(double x, double y, double side, int rgb[], int outline);
EXTERN_MSC void ps_vector(double xtail, double ytail, double xtip, double ytip, double tailwidth, double headlength, double headwidth, double headshape, int rgb[], int outline);
EXTERN_MSC unsigned char *ps_loadraster (char *file, struct rasterfile *header, BOOLEAN invert, BOOLEAN monochrome, BOOLEAN colorize, int f_rgb[], int b_rgb[]);
EXTERN_MSC void ps_words (double x, double y, char **text, int n_words, double line_space, double par_width, int par_just, int font, int font_size, double angle, int rgb[3], int justify, int draw_box, double x_off, double y_off, double x_gap, double y_gap, int boxpen_width, char *boxpen_texture, int boxpen_offset, int boxpen_rgb[], int vecpen_width, char *vecpen_texture, int vecpen_offset, int vecpen_rgb[], int boxfill_rgb[3]);
EXTERN_MSC void ps_setline(int linewidth);
EXTERN_MSC void ps_textdim (char *xdim, char *ydim, int pointsize, int font, char *text, int key);

EXTERN_MSC void ps_set_length (char *param, double value);
EXTERN_MSC void ps_set_height (char *param, int fontsize);

/* For Encapsulated PostScript Headers:

   You will need to supply a pointer to an EPS structure in order to
   get correct information in the EPS header.  If you pass a NULL pointer
   instead you will get default values for the BoundingBox plus no
   info is provided about the users name, document title, and fonts used.
   To fill in the structure you must:
   
   - Determine the extreme dimensions of your plot in points (1/72 inch).
   - Supply the user's name (or NULL)
   - Supply the document's title (or NULL)
   - Set the font pointers to point to character arrays that has the full
     font name (e.g. Helvetica-Bold).  First unused pointed must be set
     to NULL.  E.g., if 4 fonts are used, font[0], font[1], font[2], and
     font[3] must point to strings with the correct name; font[4] = NULL.
*/

