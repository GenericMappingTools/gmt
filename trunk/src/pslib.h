/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2009-2012 by P. Wessel and R. Scharroo
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
 *--------------------------------------------------------------------*/
/*
 * This include file must be included by all programs using pslib.a
 *
 * Authors:	Paul Wessel, Dept. of Geology and Geophysics, SOEST, U Hawaii
 *			   pwessel@hawaii.edu
 *		Remko Scharroo, Altimetrics
 *			   remko@altimetrics.com
 * Version:	5.0 [64-bit enabled API edition]
 * Date:	15-OCT-2009
 */

#ifndef _PSLIB_H
#define _PSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#include <stdio.h>
#include <stdlib.h>

/* Declaration of type PSL_LONG */
#ifdef _WIN64
typedef __int64 PSL_LONG;	/* A signed 8-byte integer */
#define PSL_LL "ll"
#else
typedef long PSL_LONG;		/* A signed 4 (or 8-byte for 64-bit) integer */
#define PSL_LL "l"
#endif

/* Number of PostScript points in one inch */

#define PSL_POINTS_PER_INCH	72.0
#define PSL_DOTS_PER_INCH	1200.0	/* Effective dots per inch resolution */
#define PSL_ALL_CLIP		INT_MAX	/* Terminates all clipping */

/* PSL codes for geometric symbols as expected by PSL_plotsymbol */

#define PSL_STAR		((PSL_LONG)'a')
#define PSL_CIRCLE		((PSL_LONG)'c')
#define PSL_DIAMOND		((PSL_LONG)'d')
#define PSL_ELLIPSE		((PSL_LONG)'e')
#define PSL_HEXAGON		((PSL_LONG)'h')
#define PSL_OCTAGON		((PSL_LONG)'g')
#define PSL_INVTRIANGLE		((PSL_LONG)'i')
#define PSL_ROTRECT		((PSL_LONG)'j')
#define PSL_MARC		((PSL_LONG)'m')
#define PSL_PENTAGON		((PSL_LONG)'n')
#define PSL_DOT			((PSL_LONG)'p')
#define PSL_RECT		((PSL_LONG)'r')
#define PSL_RNDRECT		((PSL_LONG)'R')
#define PSL_SQUARE		((PSL_LONG)'s')
#define PSL_TRIANGLE		((PSL_LONG)'t')
#define PSL_VECTOR		((PSL_LONG)'v')
#define PSL_WEDGE		((PSL_LONG)'w')
#define PSL_CROSS		((PSL_LONG)'x')
#define PSL_YDASH		((PSL_LONG)'y')
#define PSL_PLUS		((PSL_LONG)'+')
#define PSL_XDASH		((PSL_LONG)'-')

/* PSL codes for vector attributes - mirroring similar codes and macros in GMT */

enum PSL_enum_vecattr {PSL_VEC_LEFT = 1,	/* Only draw left half of vector head */
	PSL_VEC_RIGHT		= 2,		/* Only draw right half of vector head */
	PSL_VEC_BEGIN		= 4,		/* Place vector head at beginning of vector */
	PSL_VEC_END		= 8,		/* Place vector head at end of vector */
	PSL_VEC_JUST_B		= 0,		/* Align vector beginning at (x,y) */
	PSL_VEC_JUST_C		= 16,		/* Align vector center at (x,y) */
	PSL_VEC_JUST_E		= 32,		/* Align vector end at (x,y) */
	PSL_VEC_JUST_S		= 64,		/* Align vector center at (x,y) */
	PSL_VEC_OUTLINE		= 128,		/* Draw vector head outline using default pen */
	PSL_VEC_FILL		= 512,		/* Fill vector head using default fill */
	PSL_VEC_MARC90		= 2048};	/* Matharc only: if angles subtend 90, draw straight angle symbol */

#define PSL_vec_justify(status) ((status>>4)&3)			/* Return justification as 0-3 */
#define PSL_vec_head(status) ((status>>2)&3)			/* Return head selection as 0-3 */
#define PSL_vec_side(status) ((status&3) ? 2*(status&3)-3 : 0)	/* Return side selection as 0,-1,+1 */

/* PSL codes for arguments of PSL_beginplot and other routines */

enum PSL_enum_const {PSL_CM	= 0,
	PSL_INCH		= 1,
	PSL_METER		= 2,
	PSL_PT			= 3,
	PSL_FINALIZE		= 1,
	PSL_OVERLAY		= 1,
	PSL_INIT		= 0,
	PSL_LANDSCAPE		= 0,
	PSL_PORTRAIT		= 1,
	PSL_ASCII85		= 0,
	PSL_HEX			= 1,
	PSL_NONE		= 0,
	PSL_RLE			= 1,
	PSL_LZW			= 2,
	PSL_NO			= 0,
	PSL_YES			= 1,
	PSL_FWD			= 0,
	PSL_INV			= 1,
	PSL_OUTLINE		= 1,
	PSL_MAX_EPS_FONTS	= 6,
	PSL_MAX_DIMS		= 8,		/* Max number of dim arguments to PSL_plot_symbol */
	PSL_N_PATTERNS		= 91,		/* Current number of predefined patterns + 1, # 91 is user-supplied */
	PSL_BUFSIZ		= 4096};	/* To match GMT_BUFSIZ and be consistent across all platforms */

/* PSL codes for pen movements (used by PSL_plotpoint, PSL_plotline, PSL_plotarc) */

enum PSL_enum_move {PSL_DRAW	= 0,
	PSL_MOVE		= 1,
	PSL_STROKE		= 2,
	PSL_REL			= 4,
	PSL_CLOSE		= 8};

/* PSL codes for text and paragraph justification */

enum PSL_enum_just {PSL_BL	= 1,
	PSL_BC			= 2,
	PSL_BR			= 3,
	PSL_ML			= 5,
	PSL_MC			= 6,
	PSL_MR			= 7,
	PSL_TL			= 9,
	PSL_TC			= 10,
	PSL_TR			= 11,
	PSL_JUST		= 4};

/* PSL code for rectangle shapes */

enum PSL_enum_rect {PSL_RECT_STRAIGHT	= 0,
	PSL_RECT_ROUNDED,
	PSL_RECT_CONVEX,
	PSL_RECT_CONCAVE};

/* PSL codes for line settings */

enum PSL_enum_line {PSL_BUTT_CAP	= 0,
	PSL_ROUND_CAP			= 1,
	PSL_SQUARE_CAP			= 2,
	PSL_MITER_JOIN			= 0,
	PSL_ROUND_JOIN			= 1,
	PSL_BEVEL_JOIN			= 2,
	PSL_MITER_DEFAULT		= 35};

/* Verbosity levels */

enum PSL_enum_verbose {PSL_MSG_SILENCE = 0,	/* No messages whatsoever */
	PSL_MSG_FATAL,		/* Fatal messages */
	PSL_MSG_NORMAL,		/* Warnings level -V */
	PSL_MSG_VERBOSE,	/* Longer verbose, -Vl in some programs */
	PSL_MSG_DEBUG};		/* Debug messages for developers mostly */

/* Color spaces */

enum PSL_enum_color {PSL_RGB = 0,
	PSL_CMYK,
	PSL_HSV,
	PSL_GRAY};

/* Color types */

enum PSL_enum_fill {PSL_IS_STROKE = 0,
	PSL_IS_FILL,
	PSL_IS_FONT};

#if 0
/* Positioning types for origin */

#define PSL_ABS			1
#endif

/* PSL error codes */

enum PSL_enum_err {PSL_BAD_VALUE = -99,	/* Bad value */
	PSL_BAD_JUST,		/* Bad text or paragraph justification */
	PSL_READ_FAILURE,	/* Less than 3 points */
	PSL_NO_POLYGON,		/* Less than 3 points */
	PSL_BAD_TEXT,		/* Text is too long */
	PSL_BAD_WIDTH,		/* Negative line width */
	PSL_NO_PATH,		/* Less than 2 points given as path */
	PSL_BAD_SYMBOL,		/* Unknown symbol type */
	PSL_BAD_SIZE,		/* Size is negative */
	PSL_BAD_RANGE,		/* Range defined by min/max exceeds limit */
	PSL_BAD_FLAG,		/* A flag is outside required range */
	PSL_NO_SESSION,		/* No active session */
	PSL_NO_ERROR = 0};	/* No errors, all is OK */

/*--------------------------------------------------------------------
 *			PSL PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

struct PSL_CTRL {
	struct INIT {	/* Parameters set by user via PSL_beginplot() */
		FILE *err;			/* Error stream (NULL means stderr)		*/
		char *encoding;			/* The encoding name. e.g. ISO-8859-1		*/
		char *session;			/* The session name (NULL)			*/
		PSL_LONG unit;			/* 0 = cm, 1 = inch, 2 = meter			*/
		PSL_LONG copies;		/* Number of copies for this plot		*/
		double page_rgb[4];		/* RGB color for background paper [white]	*/
		double page_size[2];		/* Width and height of paper used in points	*/
		double magnify[2];		/* Global scale values [1/1]			*/
	} init;
	struct CURRENT {	/* Variables and settings that changes via PSL_* calls */
		char string[PSL_BUFSIZ];	/* Last text string plotted			*/
		char style[512];		/* Current setdash pattern			*/
		char bw_format[8];		/* Format used for grayshade value		*/
		char rgb_format[64];		/* Same, for RGB color triplets			*/
		char hsv_format[64];		/* Same, for HSV color triplets	(HSB in PS)	*/
		char cmyk_format[64];		/* Same, for CMYK color quadruples		*/
		char transparency_mode[16];	/* PDF transparency mode			*/
		double linewidth;		/* Current pen thickness			*/
		double rgb[3][4];		/* Current stroke, fill, and fs fill rgb	*/
		double offset;			/* Current setdash offset			*/
		double fontsize;		/* Current font size				*/
		PSL_LONG nclip;			/* Clip depth 					*/
		PSL_LONG font_no;		/* Current font number				*/
		PSL_LONG outline;		/* Current outline				*/
	} current;
	struct INTERNAL {	/* Variables used internally only */
		char *SHAREDIR;			/* Pointer to path of directory with pslib subdirectory */
		char *USERDIR;			/* Pointer to path of directory with user definitions (~/.gmt) */
		char *user_image[PSL_N_PATTERNS];	/* Name of user patterns		*/
		char origin[2];			/* 'r', 'a', 'f', 'c' depending on reference for new origin x and y coordinate */
		double offset[2];		/* Origin offset [1/1]				*/
		double p_width;			/* Paper width in points, set in PSL_beginplot();	*/
		double p_height;		/* Paper height in points, set in PSL_beginplot();	*/
		double dpu;			/* PS dots per unit.  Must be set through PSL_beginplot();		*/
		double dpp;			/* PS dots per point.  Must be set through PSL_beginplot();		*/
		double x2ix;			/* Scales user x to PS dots			*/
		double y2iy;			/* Scales user y to PS dots			*/
		double p2u;			/* Scales dimensions in points (e.g., fonts, linewidths) to user units (e.g. inch)		*/
		double axis_limit[4];		/* The current xmin, xmax, ymin, ymax settings for axes */
		double axis_pos[2];		/* Lower left placement for axes		*/
		double axis_dim[2];		/* Lengths of axes 				*/
		PSL_LONG verbose;		/* Verbosity level (0-4): see PSL_MSG_*	*/
		PSL_LONG comments;		/* TRUE for writing comments to output, FALSE strips all comments */
		PSL_LONG overlay;		/* TRUE if overlay (-O)				*/
		PSL_LONG landscape;		/* TRUE = Landscape, FALSE = Portrait		*/
		PSL_LONG text_init;		/* TRUE after PSL_text.ps has been loaded	*/
		PSL_LONG image_format;		/* 0 writes images in ascii, 2 uses binary	*/
		PSL_LONG N_FONTS;		/* Total no of fonts;  To add more, modify the file CUSTOM_font_info.d */
		PSL_LONG compress;		/* Compresses images with (1) RLE or (2) LZW or (0) None */
		PSL_LONG color_mode;		/* 0 = rgb, 1 = cmyk, 2 = hsv (only 1-2 for images)	*/
		PSL_LONG line_cap;		/* 0, 1, or 2 for butt, round, or square [butt]	*/
		PSL_LONG line_join;		/* 0, 1, or 2 for miter, arc, or bevel [miter]	*/
		PSL_LONG miter_limit;		/* Acute angle threshold 0-180; 0 means PS default [0] */
		PSL_LONG ix, iy;		/* Absolute coordinates of last point		*/
		PSL_LONG length;		/* Image row output byte counter		*/
		PSL_LONG n_userimages;		/* Number of specified custom patterns		*/
		PSL_LONG x0, y0;		/* x,y PS offsets				*/
		FILE *fp;			/* PS output file pointer. NULL = stdout	*/
		struct PSL_FONT {
			double height;		/* Height of A for unit fontsize */
			char *name;		/* Name of this font */
			PSL_LONG encoded;	/* TRUE if we never should reencode this font (e.g. symbols) */
						/* This is also changed to TRUE after we do reencode a font */
		} *font;	/* Pointer to array of font structures 		*/
		struct PSL_PATTERN {
			PSL_LONG nx, ny;
			PSL_LONG status, depth, dpi;
			double f_rgb[4], b_rgb[4];
		} pattern[PSL_N_PATTERNS*2];
	} internal;
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
#define RT_EPS		4		/* Encapsulated PostScript format */
#define RMT_NONE	0		/* maplength is expected to be 0 */
#define RMT_EQUAL_RGB	1		/* red[maplength/3], green[], blue[] follow */

/* Public functions */

EXTERN_MSC struct PSL_CTRL *New_PSL_Ctrl (char *session);
EXTERN_MSC PSL_LONG PSL_beginaxes (struct PSL_CTRL *P, double llx, double lly, double width, double height, double x0, double y0, double x1, double y1);
EXTERN_MSC PSL_LONG PSL_beginclipping (struct PSL_CTRL *P, double *x, double *y, PSL_LONG n, double rgb[], PSL_LONG flag);
EXTERN_MSC PSL_LONG PSL_beginplot (struct PSL_CTRL *P, FILE *fp, PSL_LONG orientation, PSL_LONG overlay, PSL_LONG color_mode, char origin[], double offset[], double page_size[], char *title, PSL_LONG font_no[]);
EXTERN_MSC PSL_LONG PSL_beginsession (struct PSL_CTRL *PSL);
EXTERN_MSC PSL_LONG PSL_endaxes (struct PSL_CTRL *PSL);
EXTERN_MSC PSL_LONG PSL_endclipping (struct PSL_CTRL *P, PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_endplot (struct PSL_CTRL *P, PSL_LONG lastpage);
EXTERN_MSC PSL_LONG PSL_endsession (struct PSL_CTRL *P);
EXTERN_MSC PSL_LONG PSL_plotarc (struct PSL_CTRL *P, double x, double y, double radius, double az1, double az2, PSL_LONG type);
EXTERN_MSC PSL_LONG PSL_plotaxis (struct PSL_CTRL *P, double annotation_int, char *label, double annotfontsize, PSL_LONG side);
EXTERN_MSC PSL_LONG PSL_plotbitimage (struct PSL_CTRL *P, double x, double y, double xsize, double ysize, PSL_LONG justify, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, double f_rgb[], double b_rgb[]);
EXTERN_MSC PSL_LONG PSL_plotcolorimage (struct PSL_CTRL *P, double x, double y, double xsize, double ysize, PSL_LONG justify, unsigned char *buffer, PSL_LONG nx, PSL_LONG ny, PSL_LONG nbits);
EXTERN_MSC PSL_LONG PSL_plotepsimage (struct PSL_CTRL *P, double x, double y, double xsize, double ysize, PSL_LONG justify, unsigned char *buffer, PSL_LONG size, PSL_LONG nx, PSL_LONG ny, PSL_LONG ox, PSL_LONG oy);
EXTERN_MSC PSL_LONG PSL_plotline (struct PSL_CTRL *P, double *x, double *y, PSL_LONG n, PSL_LONG type);
EXTERN_MSC PSL_LONG PSL_plotparagraph (struct PSL_CTRL *P, double x, double y, double fontsize, char *paragraph, double angle, PSL_LONG justify);
EXTERN_MSC PSL_LONG PSL_plotparagraphbox (struct PSL_CTRL *P, double x, double y, double fontsize, char *paragraph, double angle, PSL_LONG justify, double offset[], PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_plotpoint (struct PSL_CTRL *P, double x, double y, PSL_LONG pen);
EXTERN_MSC PSL_LONG PSL_plotbox (struct PSL_CTRL *P, double x0, double y0, double x1, double y1);
EXTERN_MSC PSL_LONG PSL_plotpolygon (struct PSL_CTRL *P, double *x, double *y, PSL_LONG n);
EXTERN_MSC PSL_LONG PSL_plotsegment (struct PSL_CTRL *P, double x0, double y0, double x1, double y1);
EXTERN_MSC PSL_LONG PSL_plotsymbol (struct PSL_CTRL *P, double x, double y, double param[], PSL_LONG symbol);
EXTERN_MSC PSL_LONG PSL_plottext (struct PSL_CTRL *P, double x, double y, double fontsize, char *text, double angle, PSL_LONG justify, PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_plottextbox (struct PSL_CTRL *P, double x, double y, double fontsize, char *text, double angle, PSL_LONG justify, double offset[], PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_plottextclip (struct PSL_CTRL *P, double x[], double y[], PSL_LONG m, double fontsize, char *label[], double angle[], PSL_LONG justify, double offset[], PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_plottextpath (struct PSL_CTRL *P, double x[], double y[], PSL_LONG n, PSL_LONG node[], double fontsize, char *label[], PSL_LONG m, double angle[], PSL_LONG justify, double offset[], PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_loadimage (struct PSL_CTRL *P, char *file, struct imageinfo *header, unsigned char **image);
EXTERN_MSC PSL_LONG PSL_setcolor (struct PSL_CTRL *P, double rgb[], PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_setdefaults (struct PSL_CTRL *P, double xyscales[], double page_rgb[], char *encoding);
EXTERN_MSC PSL_LONG PSL_setdash (struct PSL_CTRL *P, char *pattern, double offset);
EXTERN_MSC PSL_LONG PSL_setfill (struct PSL_CTRL *P, double rgb[], PSL_LONG outline);
EXTERN_MSC PSL_LONG PSL_setfont (struct PSL_CTRL *P, PSL_LONG font_no);
EXTERN_MSC PSL_LONG PSL_setformat (struct PSL_CTRL *P, PSL_LONG n_decimals);
EXTERN_MSC PSL_LONG PSL_setlinecap (struct PSL_CTRL *P, PSL_LONG cap);
EXTERN_MSC PSL_LONG PSL_setlinejoin (struct PSL_CTRL *P, PSL_LONG join);
EXTERN_MSC PSL_LONG PSL_setlinewidth (struct PSL_CTRL *P, double linewidth);
EXTERN_MSC PSL_LONG PSL_setmiterlimit (struct PSL_CTRL *P, PSL_LONG limit);
EXTERN_MSC PSL_LONG PSL_setorigin (struct PSL_CTRL *P, double x, double y, double angle, PSL_LONG mode);
EXTERN_MSC PSL_LONG PSL_setparagraph (struct PSL_CTRL *P, double line_space, double par_width, PSL_LONG par_just);
EXTERN_MSC PSL_LONG PSL_setpattern (struct PSL_CTRL *P, PSL_LONG image_no, char *imagefile, PSL_LONG image_dpi, double f_rgb[], double b_rgb[]);
EXTERN_MSC PSL_LONG PSL_settransparencymode (struct PSL_CTRL *PSL, const char *mode);
EXTERN_MSC PSL_LONG PSL_definteger (struct PSL_CTRL *P, const char *param, PSL_LONG value);
EXTERN_MSC PSL_LONG PSL_defpen (struct PSL_CTRL *P, const char *param, double width, char *style, double offset, double rgb[]);
EXTERN_MSC PSL_LONG PSL_defpoints (struct PSL_CTRL *P, const char *param, double fontsize);
EXTERN_MSC PSL_LONG PSL_defcolor (struct PSL_CTRL *P, const char *param, double rgb[]);
EXTERN_MSC PSL_LONG PSL_deftextdim (struct PSL_CTRL *P, const char *dim, double fontsize, char *text);
EXTERN_MSC PSL_LONG PSL_defunits (struct PSL_CTRL *P, const char *param, double value);
EXTERN_MSC unsigned char *psl_gray_encode (struct PSL_CTRL *PSL, PSL_LONG *nbytes, unsigned char *input);

/* Other deep level routines that could be useful */
EXTERN_MSC PSL_LONG psl_ix (struct PSL_CTRL *P, double value);
EXTERN_MSC PSL_LONG psl_iy (struct PSL_CTRL *P, double value);
EXTERN_MSC PSL_LONG psl_iz (struct PSL_CTRL *P, double value);
EXTERN_MSC PSL_LONG psl_ip (struct PSL_CTRL *P, double value);

/* Used indirectly by macro PSL_free and FORTRAN wrapper PSL_free_ . */
EXTERN_MSC PSL_LONG PSL_free_nonmacro (void *addr);

/* Definition for printing a message. When DEBUG is on, also print source file and line number.
 * Use this for various progress statements, debugging to see certain variables, and even fatal
 * error messages. */
/* For FORTRAN there is PSL_command_ that only accepts one text argument */
EXTERN_MSC int PSL_command (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_comment (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_initerr (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_message (struct PSL_CTRL *C, PSL_LONG level, const char *format, ...);
EXTERN_MSC FILE *PSL_fopen (char *file, char *mode);

#define PSL_free(ptr) (PSL_free_nonmacro(ptr),(ptr)=NULL) /* Cleanly set the freed pointer to NULL */

#ifdef __cplusplus
}
#endif

#endif /* _PSLIB_H */
