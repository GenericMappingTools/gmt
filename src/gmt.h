/*--------------------------------------------------------------------
 *	$Id: gmt.h,v 1.203 2011-03-03 21:02:50 guru Exp $
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
 * gmt.h is the main include file for GMT.  It contains definitions
 * for several of the structures and parameters used by all programs.
 *
 * Author:	Paul Wessel
 * Date:	01-MAR-1991
 * Revised:	22-MAR-2006
 * Version:	4.x
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _GMT_H
#define _GMT_H

/*  GMT is POSIX.1 COMPLIANT  */

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE 1
#endif

/* Declaration modifiers for DLL support (MSC et al) */

#if defined(DLL_GMT)		/* define when library is a DLL */
#if defined(DLL_EXPORT)		/* define when building the library */
#define MSC_EXTRA_GMT __declspec(dllexport)
#else
#define MSC_EXTRA_GMT __declspec(dllimport)
#endif
#else
#define MSC_EXTRA_GMT
#endif				/* defined(DLL_GMT) */

#define EXTERN_MSC extern MSC_EXTRA_GMT

/* So unless DLL_GMT is defined, EXTERN_MSC is simply extern */

/*--------------------------------------------------------------------
 *			SYSTEM HEADER FILES
 *--------------------------------------------------------------------*/

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <stddef.h>
#include <sys/stat.h>
#ifdef __MACHTEN__
/* Kludge to fix a Machten POSIX bug */
#include <sys/types.h>
#endif
#if defined(__ultrix__) && defined(__mips)
/* Needed to get isnan[fd] macros */
#include <ieeefp.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __FreeBSD__
#ifdef _i386_
#include <floatingpoint.h>
#endif
#endif

#include "gmt_notunix.h"	/* Stuff for Windows, OS/2 etc */
#ifndef WIN32
#include <pwd.h>
#include <unistd.h>
#endif

#if defined(__LP64__)
#define GMT_abs(n) labs(n)
#elif defined(_WIN64)
#define GMT_abs(n) _abs64(n)
#else
#define GMT_abs(n) abs(n)
#endif
/*--------------------------------------------------------------------
 *			GMT CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#define BOOLEAN GMT_LONG
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TWO_PI
#define TWO_PI        6.28318530717958647692
#endif
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4          0.78539816339744830962
#endif
#ifndef M_E
#define	M_E		2.7182818284590452354
#endif
#ifndef M_SQRT2
#define	M_SQRT2		1.41421356237309504880
#endif
#ifndef M_LN2_INV
#define	M_LN2_INV	(1.0 / 0.69314718055994530942)
#endif
#ifndef M_EULER
#define M_EULER 0.577215664901532860606512	/* Euler's constant (gamma) */
#endif

#define GMT_CONV_LIMIT	1.0e-8	/* Fairly tight convergence limit or "close to zero" limit */
#define GMT_SMALL	1.0e-4	/* Needed when results aren't exactly zero but close */
#define GMT_MIN_MEMINC	2048		/* E.g., 16 kb of 8-byte doubles */
#define GMT_MAX_MEMINC	67108864	/* E.g., 512 Mb of 8-byte doubles */

#define GMT_CHUNK	2048
#define GMT_SMALL_CHUNK	64
#define GMT_TINY_CHUNK	8
#define GMT_TEXT_LEN	64
#define GMT_LONG_TEXT	256
#define GMT_MAX_COLUMNS	4096		/* Limit on number of columns in data tables (not grids) */
#define GMT_LEVEL	4
#define CNULL		((char *)NULL)
#define VNULL		((void *)NULL)
#define GMT_CM		0
#define GMT_INCH	1
#define GMT_M		2
#define GMT_PT		3

#define GMT_DEG2SEC_F	3600.0
#define GMT_DEG2SEC_I	3600
#define GMT_SEC2DEG	(1.0 / GMT_DEG2SEC_F)
#define GMT_DEG2MIN_F	60.0
#define GMT_DEG2MIN_I	60
#define GMT_MIN2DEG	(1.0 / GMT_DEG2MIN_F)
#define GMT_MIN2SEC_F	60.0
#define GMT_MIN2SEC_I	60
#define GMT_SEC2MIN	(1.0 / GMT_MIN2SEC_F)
#define GMT_DAY2HR_F	24.0
#define GMT_DAY2HR_I	24
#define GMT_HR2DAY	(1.0 / GMT_DAY2HR_F)
#define GMT_DAY2MIN_F	1440.0
#define GMT_DAY2MIN_I	1440
#define GMT_MIN2DAY	(1.0 / GMT_DAY2MIN_F)
#define GMT_DAY2SEC_F	86400.0
#define GMT_DAY2SEC_I	86400
#define GMT_SEC2DAY	(1.0 / GMT_DAY2SEC_F)
#define GMT_HR2SEC_F	3600.0
#define GMT_HR2SEC_I	3600
#define GMT_SEC2HR	(1.0 / GMT_HR2SEC_F)
#define GMT_HR2MIN_F	60.0
#define GMT_HR2MIN_I	60
#define GMT_MIN2HR	(1.0 / GMT_HR2MIN_F)

#define GMT_YR2SEC_F	(365.2425 * 86400.0)
#define GMT_MON2SEC_F	(365.2425 * 86400.0 / 12.0)

#define GMT_DEC_SIZE	0.54	/* Size of a decimal number compared to point size */
#define GMT_PER_SIZE	0.30	/* Size of a decimal point compared to point size */

/*--------------------------------------------------------------------
 *			GMT FUNCTION MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MOD			/* Knuth-style modulo function (remainder after floored division) */
#define MOD(x, y) (x - y * floor((double)(x)/(double)(y)))
#endif

/* Safe math macros that check arguments */

#ifdef __APPLE__
#ifdef __x86_64__
/* Temorarily bypass Apple 64-bit log10 bug on Leopard 10.5.2 */
/* #define LOG2_10_SCALE 0x1.34413509F79FFp-2 */
#define LOG2_10_SCALE 0.30102999566398114250631579125183634459972381591796875
#define d_log10(x) ((x) <= 0.0 ? GMT_d_NaN : LOG2_10_SCALE * log2 (x))
#define d_log101p(x) ((x) <= -1.0 ? GMT_d_NaN : LOG2_10_SCALE * log2 (1.0+(x)))
#endif
#endif
#ifndef LOG2_10_SCALE
#define d_log10(x) ((x) <= 0.0 ? GMT_d_NaN : log10 (x))
#define d_log101p(x) ((x) <= -1.0 ? GMT_d_NaN : log10 (1.0+(x)))
#endif
#define d_sqrt(x) ((x) < 0.0 ? 0.0 : sqrt (x))
#define d_acos(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? M_PI : 0.0) : acos(x))
#define d_asin(x) (fabs(x) >= 1.0 ? copysign (M_PI_2, (x)) : asin(x))
#define d_atan2(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2(y, x))
#define d_log(x) ((x) <= 0.0 ? GMT_d_NaN : log (x))
#define d_log1p(x) ((x) <= -1.0 ? GMT_d_NaN : log1p (x))

/* Macros for degree-based trig */

#define sind(x) sin((x) * D2R)
#define cosd(x) cos((x) * D2R)
#define tand(x) tan((x) * D2R)
#define sincosd(x,s,c) sincos((x) * D2R,s,c)
#define asind(x) (asin(x) * R2D)
#define acosd(x) (acos(x) * R2D)
#define atand(x) (atan(x) * R2D)
#define atan2d(y,x) (atan2(y,x) * R2D)

/* Safe versions of the degree-based trig */

#define d_acosd(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? 180.0 : 0.0) : acosd(x))
#define d_asind(x) (fabs(x) >= 1.0 ? copysign (90.0, (x)) : asind(x))
#define d_atan2d(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2d(y,x))

/* Macros for swapping misc data types */

#define l_swap(x, y) {GMT_LONG tmp; tmp = x, x = y, y = tmp;}
#define i_swap(x, y) {int tmp; tmp = x, x = y, y = tmp;}
#define d_swap(x, y) {double tmp; tmp = x, x = y, y = tmp;}
#define f_swap(x, y) {float tmp; tmp = x, x = y, y = tmp;}

/* Macros for byte-order swapping 2- and 4-byte values(From John M. Kuhn, NOAA) */

#define GMT_swab2(data) ((((data) & 0xff) << 8) | ((unsigned short) (data) >> 8))
#define GMT_swab4(data) \
	(((data) << 24) | (((data) << 8) & 0x00ff0000) | \
	(((data) >> 8) & 0x0000ff00) | ((unsigned int)(data) >> 24))

/* Macro for exit since this should be returned when called from Matlab */

#ifdef DO_NOT_EXIT
#define GMT_exit(code) return(code)
#else
#define GMT_exit(code) exit(code)
#endif

/*--------------------------------------------------------------------
 *			GMT TYPEDEF DEFINITIONS
 *--------------------------------------------------------------------*/

/* Note: Under Windows 64-bit a 64-bit integer is __int64 and when used
 * with scanf the format must be %lld.  This is not exactly what we call
 * POSIX-clean where %ld is expected.  Thus, in places where such 64-bit
 * variables are processed we let the compiler build the actual format
 * using the GMT_LL string which is either "l" or "ll"
 */
#ifdef _WIN64
typedef __int64 GMT_LONG;	/* A signed 8-byte integer */
#define GMT_LL "ll"
#else
typedef long GMT_LONG;		/* A signed 4 (or 8-byte for 64-bit) integer */
#define GMT_LL "l"
#endif
typedef void (*PFV) ();		/* PFV declares a pointer to a function returning void */
typedef GMT_LONG (*PFL) ();	/* PFI declares a pointer to a function returning an GMT_LONG */
typedef int (*PFI) ();		/* PFI declares a pointer to a function returning an int */
typedef GMT_LONG (*PFB) ();	/* PFB declares a pointer to a function returning a GMT_LONG */
typedef double (*PFD) ();	/* PFD declares a pointer to a function returning a double */
/*--------------------------------------------------------------------
 *			GMT PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

/* This structure contains default parameters for the GMT system */

#include "gmt_dimensions.h"
#define GMT_PEN_LEN	128
#define GMT_PENWIDTH	0.25	/* Default pen width in points */

struct GMT_PEN {	/* Holds pen attributes */
	double width;	/* In points */
	double offset;	/* In points */
	int rgb[3];
	char texture[GMT_PEN_LEN];	/* In points */
};

struct GMT_TIME_SYSTEM {
	char epoch[GMT_TEXT_LEN];	/* User-defined epoch for time */
	char unit;			/* User-defined time unit */
	GMT_LONG rata_die;		/* Rata die number of epoch */
	double epoch_t0;		/* Rata_die fraction (in days since epoch, 0 <= t0 < 1) */
	double scale;			/* Converts user units to seconds */
	double i_scale;			/* Converts seconds to user units (= 1.0/scale) */
};

struct GMT_TIME_LANGUAGE {		/* Language-specific text strings for calendars */
	char month_name[3][12][16];	/* Full, short, and 1-char month names */
	char day_name[3][7][16];	/* Full, short, and 1-char weekday names */
	char week_name[3][16];		/* Full, short, and 1-char versions of the word Week */
};

struct GMT_DEFAULTS {
	/* DO NOT MAKE CHANGES HERE WITHOUT CORRESPONDING CHANGES TO gmt_globals.h !!!!!!!!!!!!!!!!!!! */
	double annot_min_angle;		/* If angle between map boundary and annotation is less, no annotation is drawn [20] */
	double annot_min_spacing;	/* If an annotation is closer that this to an older annotation, the annotation is skipped [0.0] */
	GMT_LONG annot_font[2];		/* Font for primary and secondary annotations [Helvetica] */
	double annot_font_size[2];	/* Font size for primary and secondary annotations in points [14,16] */
	double annot_offset[2];		/* Distance between primary or secondary annotation and tickmarks [0.075] */
	char basemap_axes[5];		/* Which axes to draw and annotate ["WESN"]  */
	int basemap_frame_rgb[3];	/* Frame color rgb [(0,0,0) = black] */
	GMT_LONG basemap_type;		/* Fancy (0) or plain (1) [0] */
	int background_rgb[3];		/* Color of background [0/0/0] */
	int foreground_rgb[3];		/* Color of foreground [255/255/255] */
	int nan_rgb[3];			/* Color of NaNs [0/0/0] */
	GMT_LONG color_image;		/* 0 = Adobe's colorimage, 1 = Tiles, 2 = RGB-separation */
	GMT_LONG color_model;		/* 1 = read RGB, 2 = use RGB, 4 = read HSV, 8 = use HSV, 16 = read CMYK, 32 = use CMYK [1+2] */
	char d_format[GMT_TEXT_LEN];	/* Default double output format [%g] */
	GMT_LONG degree_format;		/* 0 = <0/360/-90/90>, 1 = <-180/180/-90/90>, 2 = <0/180/0/90>
					   3 = 0E/180E/180W/0W/90S/90N> */
	GMT_LONG dpi;			/* Dots pr. inch plotter resolution [300] */
	GMT_LONG ellipsoid;		/* Which ellipsoid to use [0 = GRS 80] */
	struct GMT_PEN frame_pen;	/* Pen attributes for map boundary [5] */
	double frame_width;		/* Thickness of fancy map frame [0.075] */
	double global_x_scale;		/* Scaling of x just before plotting [1] */
	double global_y_scale;		/* Scaling of y just before plotting [1] */
	double grid_cross_size[2];	/* Size of primary & secondary gridcrosses.  0 means draw continuous gridlines */
	char gridfile_format[GMT_TEXT_LEN];	/* Default grid file format */
	struct GMT_PEN grid_pen[2];	/* Pen attributes for primary and secondary gridlines [1] */
	GMT_LONG gridfile_shorthand;	/* Use shorthand suffix notation for embedded grid file formats [FALSE] */
	GMT_LONG header_font;		/* Font for headers [Helvetica] */
	double header_font_size;	/* Font size for headers in points [36] */
	double header_offset;		/* Distance between lowermost annotation (or label) and base of plot title [0.1875] */
	double hsv_min_saturation;	/* For smallest or most negative intensity [1.0] */
	double hsv_max_saturation;	/* For largest or most positive intensity [0.1] */
	double hsv_min_value;		/* For smallest or most negative intensity [0.3] */
	double hsv_max_value;		/* For largest or most positive intensity [1.0] */
	GMT_LONG interpolant;		/* Choose between 0 (Linear), 1 (Akima), or 2 (Cubic spline) */
	GMT_LONG io_header[2];		/* Input & Output data has header records [FALSE, FALSE] */
	GMT_LONG n_header_recs;		/* number of header records [0] */
	GMT_LONG label_font;		/* Font for labels [Helvetica] */
	double label_font_size;		/* Font size for labels in points [24] */
	double label_offset;		/* Distance between lowermost annotation and top of label [0.1125] */
	double line_step;		/* Maximum straight linesegment length for arcuate lines */
	double map_scale_factor;	/* Central mapscale factor, typically 0.9996-1 (or -1 for default action) */
	double map_scale_height;	/* Height of map scale drawn on a map [0.075] */
	GMT_LONG measure_unit;		/* Choose 0 (cm), 1 (inch), 2 (m) or 3 (point) [1] */
	GMT_LONG media;			/* Default paper media [25(Letter)] */
	GMT_LONG nan_is_gap;		/* Determines what NaNs in input records should mean (beyond skipping the record) */
	GMT_LONG n_copies;		/* Number of copies pr plot [1] */
	GMT_LONG oblique_annotation;	/* Controls annotations and tick angles etc. [0] */
	int page_rgb[3];		/* Color of the page [255/255/255 white] */
	GMT_LONG portrait;		/* Orientation of page [FALSE = Landscape, TRUE = Portrait] */
	double paper_width[2];		/* Width and height of paper to plot on in points [Letter or A4] */
	double polar_cap[2];		/* Latitude of polar cap and delta_lon for gridline spacing [85/90] */
	GMT_LONG ps_colormode;		/* 2 writes HSV in PostScript, 1 writes CMYK, 0 uses RGB [0] */
	GMT_LONG ps_compress;		/* Compression of PostScript images: 0 = no, 1 = RLE, 2 = LZW [0] */
	GMT_LONG ps_heximage;		/* TRUE gives hex ps output image, FALSE gives binary image [TRUE] */
	GMT_LONG ps_line_cap;		/* butt|round|square [butt] */
	GMT_LONG ps_line_join;		/* miter|arc|bevel [miter] */
	GMT_LONG ps_miter_limit;	/* acute angle (degrees) beyond which we do a bevel join [-] */
	GMT_LONG ps_verbose;		/* TRUE writes comments in ps output, FALSE gives no comments [TRUE] */
	double tick_length;		/* Length of tickmarks [0.075] */
	struct GMT_PEN tick_pen;	/* Pen attributes for tickmarks [2] */
	GMT_LONG unix_time;		/* Plot time and map projection on map [FALSE] */
	GMT_LONG unix_time_just;	/* Justification of the GMT timestamp box [1 (BL)] */
	double unix_time_pos[2];	/* Where to plot timestamp relative to origin */
	char unix_time_format[GMT_LONG_TEXT];	/* Specify the format for writing time stamps (see strftime) */
	double vector_shape;		/* 0.0 = straight vectorhead, 1.0 = arrowshape, with continuous range in between */
	GMT_LONG verbose;		/* Give info during execution [FALSE] */
	GMT_LONG want_euro_font;		/* Include re-encoding for European characters [TRUE] */
	double x_axis_length;		/* Length of x-axis if no scale is given [8] */
	double y_axis_length;		/* Length of y-axis if no scale is given [5] */
	double x_origin;		/* x-origin of plot, i.e. where lower left corner plots on paper [1] */
	double y_origin;		/* y-origin of plot, i.e. where lower left corner plots on paper [1] */
	GMT_LONG xy_toggle[2];		/* TRUE means read/write I/O as lat/lon instead of lon/lat [FALSE,FALSE] */
	GMT_LONG y_axis_type;		/* Select y-axis with horizontal (0) or vertical (1) annotations  [0] */
	struct ELLIPSOID {	/* Information about a particular ellipsoid */
		/* Table taken from Snyder "Map projection - a working manual", p 12 Table 1 */
		char name[GMT_TEXT_LEN];
		GMT_LONG date;
		double eq_radius;
		double flattening;
	} ref_ellipsoid[GMT_N_ELLIPSOIDS];	/* Ellipsoid parameters */
	struct DATUM {	/* Information about a particular datum */
		char name[GMT_TEXT_LEN];	/* Datum name */
		char ellipsoid[GMT_TEXT_LEN];	/* Ellipsoid GMT ID name */
		char region[GMT_LONG_TEXT];	/* Region of use */
		double xyz[3];		/* Coordinate shifts in meter for x, y, and z */
	} datum[GMT_N_DATUMS];	/* Datum parameters */
	char input_clock_format[GMT_TEXT_LEN];	/* How to decode an incoming clock string [hh:mm:ss] */
	char input_date_format[GMT_TEXT_LEN];	/* How to decode an incoming date string [yyyy-mm-dd] */
	char output_clock_format[GMT_TEXT_LEN];	/* Controls how clocks are written on output [hh:mm:ss] */
	char output_date_format[GMT_TEXT_LEN];	/* Controls how dates are written on output [yyyy-mm-dd] */
	char output_degree_format[GMT_TEXT_LEN];/* Controls how degrees are written on output [000 = dd.xxxx] */
	char plot_clock_format[GMT_TEXT_LEN];	/* Controls how clocks are plotted on maps [hh:mm:ss] */
	char plot_date_format[GMT_TEXT_LEN];	/* Controls how dates are plotted on maps [yyyy-mm-dd] */
	char plot_degree_format[GMT_TEXT_LEN];	/* Controls how degrees are plotted on maps [020 = dd:mm:ss as in old DEGREE_FORMAT = 0] */
	char time_format[2][GMT_TEXT_LEN];	/* Controls annotation format for Months/Weeks/Weekdays for primary and secondary axes */
	GMT_LONG time_is_interval;	/* Does a time given as a month (or year or day) mean the middle of the interval? */
	double time_interval_fraction;	/* How much of a partial interval is needed in order to annotate it */
	GMT_LONG want_leap_seconds;	/* Do we need to worry about leap seconds? */
	struct GMT_TIME_SYSTEM time_system;	/* All the information about the selected time system */
	GMT_LONG time_week_start;		/* Which day (Sun = 0, Sat = 7) is start of week */
	char time_language[GMT_TEXT_LEN];	/* Language file for time support */
	GMT_LONG Y2K_offset_year;		/* Cutoff for making 4-digit years from 2-digit years (1900 vs 2000) */
	char field_delimiter[8];	/* Separator between output ascii data columns [tab] */
	enum gmt_symbol { gmt_none = -1, gmt_ring, gmt_degree, gmt_colon, gmt_squote, gmt_dquote, gmt_lastsym } degree_symbol;
	struct gmt_encoding
	{
		char name[GMT_TEXT_LEN];
		GMT_LONG code[gmt_lastsym]; /* Codes for symbols we print. */
	} encoding;
	GMT_LONG history;		/* TRUE to pass information via .gmtdefaults4 files */
	GMT_LONG transparency[2];	/* Transparency level for strokes and fill */
};

struct GMT_PS {	/* Holds the current settings that affect PS generation */
	/* A structure pointer is passed to GMT_plotinit which calls ps_plotinit */
	GMT_LONG portrait;			/* TRUE for portrait, FALSE for landscape */
	GMT_LONG verbose;			/* TRUE to give verbose feedback from pslib routines [FALSE] */
	GMT_LONG heximage;			/* TRUE to write images in HEX, FALSE in BIN [TRUE] */
	GMT_LONG absolute;			/* TRUE if -X, -Y was absolute [FALSE] */
	GMT_LONG last_page;			/* Result of not -K [TRUE] */
	GMT_LONG overlay;			/* Result of -O [FALSE] */
	GMT_LONG unix_time;			/* Result of -U [gmtdefs.unix_time] */
	GMT_LONG comments;			/* TRUE to write comments to PS file [FALSE] */
	GMT_LONG clip;				/* +1 if clipping will extend beyond current process, -1 if we terminate clipping */
	GMT_LONG n_copies;			/* Result of -c [gmtdefs.n_copies] */
	GMT_LONG colormode;			/* 0 (RGB), 1 (CMYK), 2 (HSV) */
	GMT_LONG compress;			/* 0 (none), 1 (RLE), 2 (LZW) */
	GMT_LONG line_cap;			/* 0 (butt), 1 (round), 2 (square) */
	GMT_LONG line_join;			/* 0 (miter), 1 (round), 2 (bevel) */
	GMT_LONG miter_limit;			/* 0-180 degrees as whole integer */
	GMT_LONG dpi;				/* Plotter resolution in dots-per-inch */
	int page_rgb[3];			/* Array with Color of page (paper) */
	GMT_LONG unix_time_just;		/* Justification of the GMT time stamp box */
	double paper_width[2];			/* Physical width and height of paper used in points */
	double x_origin, y_origin;		/* Result of -X -Y [gmtdefs.x|y_origin] */
	double x_scale, y_scale;		/* Copy of gmtdefs.global_x|y_scale */
	double unix_time_pos[2];		/* Result of -U [gmtdefs.unix_time_pos] */
	char *encoding_name;			/* Pointer to font encoding used */
	char unix_time_label[BUFSIZ];		/* Label added to GMT time stamp generated by -U */
};

struct GMT_HASH {	/* Used to related keywords to gmtdefaults entry */
	struct GMT_HASH *next;
	GMT_LONG id;
	char *key;
};

struct GMT_FILL {	/* Holds fill attributes */
	GMT_LONG use_pattern;	/* TRUE if pattern rather than rgb is set */
	int rgb[3];		/* Chosen color if no pattern */
	GMT_LONG pattern_no;	/* Number of predefined pattern, if set */
	GMT_LONG dpi;		/* Desired dpi of image building-block */
	GMT_LONG inverse;	/* TRUE if 1-bit pattern should be reversed */
	int f_rgb[3], b_rgb[3];	/* Colors applied to unset and set bits in 1-bit image */
	char pattern[BUFSIZ];	/* Full filename of user-define raster */
};

/*  Moved from gmt_plot.c */
struct GMT_XINGS {
        double xx[2], yy[2];    /* Cartesian coordinates of intersection with map boundary */
        double angle[2];        /* Angles of intersection */
        GMT_LONG sides[2];	/* Side id of intersection */
        GMT_LONG nx;		/* Number of intersections (1 or 2) */
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
	char label[GMT_TEXT_LEN];	/* Alternative user-specified label */
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
	char label[4][GMT_TEXT_LEN];	/* User-changable labels for W, E, S, N point */
	char dlabel[GMT_LONG_TEXT];	/* Magnetic declination label */
};

struct GMT_FONT {		/* Information for each font */
	char *name;			/* Name of the font */
	double height;			/* Height of letter "A" for unit fontsize */
};

/*--------------------------------------------------------------------*/
/*	External variables for misc purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct GMT_CTRL *GMT;

EXTERN_MSC struct GMT_DEFAULTS gmtdefs;

EXTERN_MSC GMT_LONG GMT_N_FONTS;		/* Number of fonts loaded from $GMT_SHAREDIR/pslib */
EXTERN_MSC char *GMT_SHAREDIR;			/* Points to the GMT the shared data files */
EXTERN_MSC char *GMT_HOMEDIR;			/* Points to the GMT user home directory [~] */
EXTERN_MSC char *GMT_USERDIR;			/* Points to the GMT user .gmt directory [~/.gmt] */
EXTERN_MSC char *GMT_DATADIR;			/* Points to the GMT misc directory [if set] */
EXTERN_MSC char *GMT_GRIDDIR;			/* Points to the GMT grids directory [if set] */
EXTERN_MSC char *GMT_IMGDIR;			/* Points to the GMT img directory [if set] */
EXTERN_MSC char *GMT_TMPDIR;			/* Points to the GMT temporary directory [if set] */
EXTERN_MSC char *GMT_unit_names[];
EXTERN_MSC double GMT_u2u[4][4];		/* measure unit translation matrix 4 x 4*/
EXTERN_MSC struct GMT_FONT *GMT_font;
EXTERN_MSC struct GMT_HASH GMT_month_hashnode[12];
EXTERN_MSC struct GMT_TIME_LANGUAGE GMT_time_language;

EXTERN_MSC float GMT_f_NaN;		/* Holds IEEE not-a-number float */
EXTERN_MSC double GMT_d_NaN;		/* Holds IEEE not-a-number double */
EXTERN_MSC GMT_LONG GMT_give_synopsis_and_exit;		/* TRUE if short usage message is desired (must say program - ) */
EXTERN_MSC char *GMT_program;		/* Name of current GMT program */
EXTERN_MSC int GMT_oldargc;
EXTERN_MSC char *GMT_oldargv[];		/* Pointers to old common arguments */
EXTERN_MSC int GMT_no_rgb[];

/*--------------------------------------------------------------------*/
/*	For i/o purposes */
/*--------------------------------------------------------------------*/


EXTERN_MSC FILE *GMT_stdin, *GMT_stdout;
EXTERN_MSC PFL GMT_input, GMT_output, GMT_input_ascii, GMT_output_ascii;
EXTERN_MSC double GMT_curr_rec[GMT_MAX_COLUMNS], GMT_prev_rec[GMT_MAX_COLUMNS];
EXTERN_MSC GMT_LONG GMT_n_file_suffix;
EXTERN_MSC GMT_LONG *GMT_file_id;
EXTERN_MSC double *GMT_file_scale, *GMT_file_offset, *GMT_file_nan;
EXTERN_MSC char **GMT_file_suffix;
EXTERN_MSC GMT_LONG GMT_pad[4];
EXTERN_MSC GMT_LONG GMT_inc_code[2];	/* For adjusting -R -I */
EXTERN_MSC void nc_nopipe (char *file);

/*--------------------------------------------------------------------*/
/*	For plotting purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct GMT_PLOT_FRAME frame_info;	/* Boundary info for linear plots and maps */
EXTERN_MSC struct GMT_TRUNCATE_TIME GMT_truncate_time;	/* Used to round off times to mid-interval */
EXTERN_MSC double *GMT_x_plot;			/* Holds the x/y (inches) of a line to be plotted */
EXTERN_MSC double *GMT_y_plot;
EXTERN_MSC int *GMT_pen;			/* Pen (3 = up, 2 = down) for these points */
EXTERN_MSC GMT_LONG GMT_n_plot;			/* Number of such points */
EXTERN_MSC GMT_LONG GMT_n_alloc;		/* Current size of allocated arrays */
EXTERN_MSC GMT_LONG GMT_x_status_new;		/* Tells us what quadrant old and new points are in */
EXTERN_MSC GMT_LONG GMT_y_status_new;
EXTERN_MSC GMT_LONG GMT_x_status_old;
EXTERN_MSC GMT_LONG GMT_y_status_old;
EXTERN_MSC GMT_LONG GMT_corner;
EXTERN_MSC GMT_LONG GMT_world_map;		/* TRUE if map has 360 degrees of longitude range */
EXTERN_MSC GMT_LONG GMT_world_map_tm;		/* TRUE if GMT_TM map is global? */
EXTERN_MSC GMT_LONG GMT_lon_wrap;		/* TRUE if wrapping of longitudes over 360 degrees is allowed */
EXTERN_MSC GMT_LONG GMT_on_border_is_outside;	/* TRUE if point exactly on the map border should be considered outside */
EXTERN_MSC double GMT_map_width;		/* Full width of this world map */
EXTERN_MSC double GMT_map_height;		/* Full height of this world map */
EXTERN_MSC double GMT_half_map_size;		/* Half width of this world map */
EXTERN_MSC double GMT_half_map_height;		/* Half height of this world map */
EXTERN_MSC PFL GMT_outside;			/* pointer to function checking if a lon/lat point is outside map */
EXTERN_MSC PFL GMT_crossing;			/* pointer to functions returning crossover point at boundary */
EXTERN_MSC PFL GMT_overlap;			/* pointer to function checking for overlap between 2 regions */
EXTERN_MSC PFL GMT_map_clip;			/* pointer to functions that clip a polygon to fit inside map */
EXTERN_MSC PFD GMT_left_edge, GMT_right_edge;	/* pointer to functions that returns the left,right edge of map */
EXTERN_MSC PFD GMT_distance_func;		/* pointer to function returning distance between two points points */
EXTERN_MSC GMT_LONG GMT_z_periodic;		/* TRUE if grid values are 0-360 degrees (phases etc) */
EXTERN_MSC PFL GMT_wrap_around_check;		/* Does x or y wrap checks */
EXTERN_MSC PFL GMT_map_jump;			/* TRUE if we jump in x or y */
EXTERN_MSC PFB GMT_will_it_wrap;		/* TRUE if consecutive points indicate wrap */
EXTERN_MSC PFB GMT_this_point_wraps;		/* Used in above */
EXTERN_MSC PFV GMT_get_crossings;		/* Returns map crossings in x or y */
EXTERN_MSC GMT_LONG GMT_meridian_straight;	/* TRUE if meridians plot as straight lines */
EXTERN_MSC GMT_LONG GMT_parallel_straight;	/* TRUE if parallels plot as straight lines */
EXTERN_MSC GMT_LONG GMT_3D_mode;		/* Determines if we draw fore and/or back 3-D box lines */
EXTERN_MSC char *GMT_plot_format[3][2];		/* Keeps the 6 formats for dd:mm:ss plot output */
EXTERN_MSC GMT_LONG GMT_n_lon_nodes;		/* Somewhat arbitrary # of nodes for lines in longitude (may be reset in gmt_map.c) */
EXTERN_MSC GMT_LONG GMT_n_lat_nodes;		/* Somewhat arbitrary # of nodes for lines in latitude (may be reset in gmt_map.c) */
EXTERN_MSC double GMT_dlon;			/* Steps taken in longitude along gridlines (gets reset in gmt_init.c) */
EXTERN_MSC double GMT_dlat;			/* Steps taken in latitude along gridlines (gets reset in gmt_init.c) */

/*--------------------------------------------------------------------*/
/*	For projection purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct GMT_MAP_PROJECTIONS project_info;
EXTERN_MSC struct GMT_THREE_D z_project;
EXTERN_MSC struct GMT_DATUM_CONV GMT_datum;	/*	For datum conversions */
EXTERN_MSC PFL GMT_forward, GMT_inverse;	/*	Pointers to the selected mapping functions */
EXTERN_MSC PFL GMT_x_forward, GMT_x_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFL GMT_y_forward, GMT_y_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFL GMT_z_forward, GMT_z_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFD GMT_scan_time_string;		/*	pointer to functions that converts timestring to secs */

/* Experimental GDAL support */
#ifdef USE_GDAL
#include "gmt_gdalread.h"
#endif
#include "gmt_common.h"		/* For holding the GMT common option settings */
#include "gmt_math.h"		/* Machine-dependent macros for non-POSIX math functions */
#include "gmt_nan.h"		/* Machine-dependent macros for making and testing NaNs */
#include "gmt_error.h"       	/* Only contains error codes */
#include "gmt_synopsis.h"       /* Only contains macros for synopsis lines */
#include "gmt_version.h"        /* Only contains the current GMT version number */
#include "gmt_project.h"        /* Define project_info and frame_info structures */
#include "gmt_grd.h"            /* Define grd file header structure */
#include "gmt_io.h"		/* Defines structures and macros for table i/o */
#include "gmt_colors.h"         /* Defines color/shading global structure */
#include "gmt_grdio.h"          /* Defines function pointers for grd i/o operations */
#include "gmt_shore.h"		/* Defines structures used when reading shore database */
#include "gmt_boundcond.h"	/* Boundary conditions for grids */
#include "gmt_bcr.h"		/* Grid resampling functions */
#include "gmt_calclock.h"	/* Calendar/time functions */
#include "gmt_symbol.h"		/* Custom symbol functions */
#include "gmt_contour.h"	/* Contour label structure and functions */
#include "gmt_map.h"      	/* extern functions defined in gmt_map.c */
#include "gmt_plot.h"      	/* extern functions defined in gmt_plot.c */
#include "gmt_init.h"      	/* extern functions defined in gmt_init.c */
#include "gmt_stat.h"      	/* extern functions defined in gmt_stat.c */
#include "gmt_support.h"      	/* extern functions defined in gmt_support.c */
#include "gmt_vector.h"      	/* extern functions defined in gmt_vector.c */

/* For memory allocation.  These are the initial allocation and max increment used.
   In between we allocate 50% more than what we have, subject to the max limit.
*/

EXTERN_MSC GMT_LONG GMT_min_meminc, GMT_max_meminc;

#ifdef DEBUG
#define GMT_alloc_memory(ptr,n,n_alloc,element_size,module) GMT_alloc_memory_func(ptr,n,n_alloc,element_size,module,__FILE__,__LINE__)
#define GMT_memory(array,n,size,program) GMT_memory_func(array,n,size,program,__FILE__,__LINE__)
#define GMT_free(array) GMT_free_func(array,__FILE__,__LINE__)
extern struct MEMORY_TRACKER *GMT_mem_keeper;
#endif

#endif  /* _GMT_H */
#ifdef __cplusplus
}
#endif
