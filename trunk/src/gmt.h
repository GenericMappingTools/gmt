/*--------------------------------------------------------------------
 *	$Id: gmt.h,v 1.87 2004-11-04 03:07:06 pwessel Exp $
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
 * gmt.h is the main include file for GMT.  It contains definitions
 * for several of the structures and parameters used by all programs.
 *
 * Author:	Paul Wessel
 * Date:	01-MAR-1991
 * Revised:	15-SEP-2004
 * Version:	4.0
 */

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
#include <floatingpoint.h>
#endif

#include "gmt_notunix.h"	/* Stuff for Windows, OS/2 etc */
#ifndef WIN32
#include <pwd.h>
#include <unistd.h>
#endif

/*--------------------------------------------------------------------
 *			GMT CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

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

#define GMT_CONV_LIMIT	1.0e-8	/* Fairly tight convergence limit or "close to zero" limit */
#define SMALL		1.0e-4	/* Needed when results aren't exactly zero but close */
#define GMT_CHUNK	2000
#define GMT_SMALL_CHUNK	50
#define GMT_TINY_CHUNK	5
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
#define GMT_CLIP_ON	1024
#define GMT_CLIP_OFF	2048

/*--------------------------------------------------------------------
 *			GMT FUNCTION MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

/* Safe math macros that check arguments */

#define d_sqrt(x) ((x) < 0.0 ? 0.0 : sqrt (x))
#define d_acos(x) (fabs (x) >= 1.0 ? ((x) < 0.0 ? M_PI : 0.0) : acos (x))
#define d_asin(x) (fabs (x) >= 1.0 ? copysign (M_PI_2, (x)) : asin (x))
#define d_atan2(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2 (y, x))
#define d_log(x) ((x) <= 0.0 ? GMT_d_NaN : log (x))
#define d_log10(x) ((x) <= 0.0 ? GMT_d_NaN : log10 (x))
#define d_log1p(x) ((x) <= -1.0 ? GMT_d_NaN : log1p (x))
#define d_log101p(x) ((x) <= -1.0 ? GMT_d_NaN : log10 (1.0+(x)))

/* Macros for degree-based trig */

#define sind(x) sin ((x) * D2R)
#define cosd(x) cos ((x) * D2R)
#define tand(x) tan ((x) * D2R)

/* Macros for swapping misc data types */

#define i_swap(x, y) {int tmp; tmp = x, x = y, y = tmp;}
#define d_swap(x, y) {double tmp; tmp = x, x = y, y = tmp;}
#define f_swap(x, y) {float tmp; tmp = x, x = y, y = tmp;}

/* Macros for byte-order swapping 2- and 4-byte values(From John M. Kuhn, NOAA) */

#define GMT_swab2(data) ((((data) & 0xff) << 8) | ((unsigned short) (data) >> 8))
#define GMT_swab4(data) \
	(((data) << 24) | (((data) << 8) & 0x00ff0000) | \
	(((data) >> 8) & 0x0000ff00) | ((unsigned int)(data) >> 24))

/* Macro to determine if the grd format is netCDF */

#define GRD_IS_CDF(id) ((id) == 0 || ((id) >= 6 && (id) <= 11))

/*--------------------------------------------------------------------
 *			GMT TYPEDEF DEFINITIONS
 *--------------------------------------------------------------------*/

typedef int BOOLEAN;		/* BOOLEAN used for logical variables */
typedef void (*PFV) ();		/* PFV declares a pointer to a function returning void */
typedef int (*PFI) ();		/* PFI declares a pointer to a function returning an int */
typedef BOOLEAN (*PFB) ();	/* PFB declares a pointer to a function returning a BOOLEAN */
typedef double (*PFD) ();	/* PFD declares a pointer to a function returning a double */
typedef double GMT_dtime;	/* GMT internal time representation */

/*--------------------------------------------------------------------
 *			GMT PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

#define N_UNIQUE 59		/* Number of unique options */
#define N_KEYS 113		/* Number of gmt defaults */
#define GMT_N_MEDIA 29		/* Number of standard paper formats in the GMT_media_names.h include file */
#define HASH_SIZE 113		/* Used in get_gmtdefaults, should be ~> N_KEYS */
#define GMT_N_SYSTEMS 6		/* Number of time systems in gmt_time_systems.h */
/* This structure contains default parameters for the GMT system */

#define N_ELLIPSOIDS	63
#define N_DATUMS	223

#define GMT_PEN_LEN	128
#define GMT_PENWIDTH	0.25	/* Default pen width in points */
#define GMT_N_PEN_NAMES	11

struct GMT_PEN {	/* Holds pen attributes */
	double width;	/* In points */
	double offset;	/* In points */
	int rgb[3];
	char texture[GMT_PEN_LEN];	/* In points */
};

struct GMT_PEN_NAME {	/* Names of pens and their thicknesses */
	char name[16];
	double width;
};

struct GMTDEFAULTS {
	double annot_min_angle;		/* If angle between map boundary and annotation is less, no annotation is drawn [20] */
	double annot_min_spacing;	/* If an annotation is closer that this to an older annotation, the annotation is skipped [0.0] */
	int annot_font[2];		/* Font for primary and secondary annotations [Helvetica] */
	double annot_font_size[2];	/* Font size for primary and secondary annotations in points [14,16] */
	double annot_offset[2];		/* Distance between primary or secondary annotation and tickmarks [0.075] */
	char basemap_axes[5];		/* Which axes to draw and annotate ["WESN"]  */
	int basemap_frame_rgb[3];	/* Frame color rgb [(0,0,0) = black] */
	int basemap_type;		/* Fancy (0) or plain (1) [0] */
	int background_rgb[3];		/* Color of background [0/0/0] */
	int foreground_rgb[3];		/* Color of foreground [255/255/255] */
	int nan_rgb[3];			/* Color of NaNs [0/0/0] */
	int color_image;		/* 0 = Adobe's colorimage, 1 = Tiles, 2 = RGB-separation */
	int color_model;		/* 0 = RGB, 1 = HSV [0] */
	char d_format[32];		/* Default double output format [%lg] */
	int degree_format;		/* 0 = <0/360/-90/90>, 1 = <-180/180/-90/90>, 2 = <0/180/0/90>
					   3 = 0E/180E/180W/0W/90S/90N> */
	int dpi;			/* Dots pr. inch plotter resolution [300] */
	int ellipsoid;			/* Which ellipsoid to use [0 = GRS 80] */
	struct GMT_PEN frame_pen;	/* Pen attributes for map boundary [5] */
	double frame_width;		/* Thickness of fancy map frame [0.075] */
	double global_x_scale;		/* Scaling of x just before plotting [1] */
	double global_y_scale;		/* Scaling of y just before plotting [1] */
	double grid_cross_size[2];	/* Size of primary & secondary gridcrosses.  0 means draw continuous gridlines */
	struct GMT_PEN grid_pen[2];	/* Pen attributes for primary and secondary gridlines [1] */
	BOOLEAN gridfile_shorthand;	/* Use shorthand suffix notation for embedded formats [FALSE] */
	int header_font;		/* Font for headers [Helvetica] */
	double header_font_size;	/* Font size for headers in points [36] */
	double header_offset;		/* Distance between lowermost annotation (or label) and base of plot title [0.1875] */
	double hsv_min_saturation;	/* For smallest or most negative intensity [1.0] */
	double hsv_max_saturation;	/* For largest or most positive intensity [0.1] */
	double hsv_min_value;		/* For smallest or most negative intensity [0.3] */
	double hsv_max_value;		/* For largest or most positive intensity [1.0] */
	int interpolant;		/* Choose between 0 (Linear), 1 (Akima), or 2 (Cubic spline) */
	BOOLEAN io_header[2];		/* Input & Output data has header records [FALSE, FALSE] */
	int n_header_recs;		/* number of header records [0] */
	int label_font;			/* Font for labels [Helvetica] */
	double label_font_size;		/* Font size for labels in points [24] */
	double label_offset;		/* Distance between lowermost annotation and top of label [0.1125] */
	BOOLEAN last_page;		/* If TRUE, terminate plot system when done [TRUE] */
	double line_step;		/* Maximum straight linesegment length for arcuate lines */
	double map_scale_factor;	/* Central mapscale factor, typically 0.9996-1 (or -1 for default action) */
	double map_scale_height;	/* Height of map scale drawn on a map [0.075] */
	int measure_unit;		/* Choose 0 (cm), 1 (inch), 2 (m) or 3 (point) [1] */
	int media;			/* Default paper media [25(Letter)] */
	int n_copies;			/* Number of copies pr plot [1] */
	int n_lat_nodes;		/* No of points to use for drawing a latitudinal line [50] */
	int n_lon_nodes;		/* No of points to use for drawing a longitudinal line [50] */
	double dlon, dlat;		/* Corresponding increment in lon/lat */
	int oblique_annotation;		/* Controls annotations and tick angles etc. [0] */
	BOOLEAN overlay;		/* Make plot in overlay mode [FALSE] */
	int page_rgb[3];		/* Color of the page [255/255/255 white] */
	int page_orientation;		/* Orientation of page [0 = Landscape, 1 = Portrait] */
	int paper_width[2];		/* Width and height of paper to plot on in points [Letter or A4] */
	BOOLEAN ps_cmykmode;		/* TRUE writes CMYK in PostScript, FALSE uses RGB [FALSE] */
	int ps_compress;		/* Compression of PostScript images: 0 = no, 1 = RLE, 2 = LZW [0] */
	BOOLEAN ps_heximage;		/* TRUE gives hex ps output image, FALSE gives binary image [TRUE] */
	double tick_length;		/* Length of tickmarks [0.075] */
	struct GMT_PEN tick_pen;	/* Pen attributes for tickmarks [2] */
	BOOLEAN unix_time;		/* Plot time and map projection on map [FALSE] */
	char unix_time_label[BUFSIZ];	/* Label to plot after time-stamp instead of command line */
	double unix_time_pos[2];	/* Where to plot timestamp relative to origin */
	double vector_shape;		/* 0.0 = straight vectorhead, 1.0 = arrowshape, with continuous range in between */
	BOOLEAN verbose;		/* Give info during execution [FALSE] */
	BOOLEAN want_euro_font;		/* Include re-encoding for European characters [TRUE] */
	double x_axis_length;		/* Length of x-axis if no scale is given [8] */
	double y_axis_length;		/* Length of y-axis if no scale is given [5] */
	double x_origin;		/* x-origin of plot, i.e. where lower left corner plots on paper [1] */
	double y_origin;		/* y-origin of plot, i.e. where lower left corner plots on paper [1] */
	BOOLEAN xy_toggle[2];		/* TRUE means read/write I/O as lat/lon instead of lon/lat [FALSE,FALSE] */
	int y_axis_type;		/* Select y-axis with horizontal (0) or vertical (1) annotations  [0] */
	struct ELLIPSOID {	/* Information about a particular ellipsoid */
		/* Table taken from Snyder "Map projection - a working manual", p 12 Table 1 */
		char name[64];
		int date;
		double eq_radius;
		double pol_radius;
		double flattening;
	} ref_ellipsoid[N_ELLIPSOIDS];	/* Ellipsoid parameters */
	struct DATUM {	/* Information about a particular datum */
		char name[64];		/* Datum name */
		char ellipsoid[64];	/* Ellipsoid GMT ID name */
		char region[256];	/* Region of use */
		double xyz[3];		/* Coordinate shifts in meter for x, y, and z */
	} datum[N_DATUMS];	/* Datum parameters */
	char input_clock_format[32];	/* How to decode an incoming clock string [hh:mm:ss] */
	char input_date_format[32];	/* How to decode an incoming date string [yyyy-mm-dd] */
	char output_clock_format[32];	/* Controls how clocks are written on output [hh:mm:ss] */
	char output_date_format[32];	/* Controls how dates are written on output [yyyy-mm-dd] */
	char output_degree_format[32];	/* Controls how degrees are written on output [000 = dd.xxxx] */
	char plot_clock_format[32];	/* Controls how clocks are plotted on maps [hh:mm:ss] */
	char plot_date_format[32];	/* Controls how dates are plotted on maps [yyyy-mm-dd] */
	char plot_degree_format[32];	/* Controls how degrees are plotted on maps [020 = dd:mm:ss as in old DEGREE_FORMAT = 0] */
	char time_format[2][32];	/* Controls annotation format for Months/Weeks/Weekdays for primary and secondary axes */
	BOOLEAN time_is_interval;	/* Does a time given as a month (or year or day) mean the middle of the interval? */
	double time_interval_fraction;	/* How much of a partial interval is needed in order to annotate it */
	BOOLEAN want_leap_seconds;	/* Do we need to worry about leap seconds? */
	char time_epoch[32];		/* User-defined epoch for time */
	char time_unit;			/* User-defined time unit */
	int time_system;		/* Which time system is in effect */
	int time_week_start;		/* Which day (Sun = 0, Sat = 7) is start of week */
	char time_language[32];		/* Language file for time support */
	int Y2K_offset_year;		/* Cutoff for making 4-digit years from 2-digit years (1900 vs 2000) */
	char field_delimiter[8];	/* Separator between output ascii data columns [tab] */
	enum gmt_symbol { gmt_none = -1, gmt_ring, gmt_degree, gmt_colon, gmt_squote, gmt_dquote, gmt_lastsym } degree_symbol;
	struct gmt_encoding
	{
		char *name;
		int code[gmt_lastsym]; /* Codes for symbols we print. */
	} encoding;
};

struct GMT_HASH {	/* Used to related keywords to gmtdefaults entry */
	struct GMT_HASH *next;
	int id;
	char *key;
};

struct GMT_TIME_SYSTEM {
	char name[32];		/* Name of system */
	char epoch[32];		/* Epoch time string */
	char unit;		/* Time unit */
	double epoch_t0;	/* Internal time representation of epoch in seconds*/
	double scale;		/* Converts user units to seconds */
	double i_scale;		/* Converts seconds to user units (1.0/scale) */
};

struct GMT_TIME_LANGUAGE {	/* Language-specific text strings for calendars */
	char month_name[3][12][16];	/* Full, short, and 1-char month names */
	char day_name[3][7][16];	/* Full, short, and 1-char weekday names */
	char week_name[3][16];		/* Full, short, and 1-char versions of the word Week */
};

struct GMT_FILL {	/* Holds fill attributes */
	BOOLEAN use_pattern;	/* TRUE if pattern rather than rgb is set */
	int rgb[3];		/* Chosen color if no pattern */
	int pattern_no;		/* Number of predefined pattern, if set */
	int dpi;		/* Desired dpi of image building-block */
	BOOLEAN inverse;	/* TRUE if -bit pattern should be reversed */
	BOOLEAN colorize;	/* TRUE if 1-bit pattern should -> 24bit */
	int f_rgb[3], b_rgb[3];	/* Color when using a 1-bit colorize image */
	char pattern[BUFSIZ];	/* Full filename of user-define raster */
};

struct GMT_MEDIA {	/* Holds information about paper sizes in points */
	int width;		/* Width in points */
	int height;		/* Height in points */
};

/*  Moved from gmt_plot.c */
struct XINGS {
        double xx[2], yy[2];    /* Cartesian coordinates of intersection with map boundary */
        double angle[2];        /* Angles of intersection */
        int sides[2];           /* Side id of intersection */
        int nx;                 /* Number of intersections (1 or 2) */
};

struct MAP_SCALE {	/* Used to plot a map scale in psbasemap and pscoast */
	double lon, lat;	/* Location of top/mid point of scale on the map in lon/lat space */
	double x0, y0;		/* Location of top/mid point of scale on the map in inches x/y */
	double scale_lon;	/* Point where scale should apply */
	double scale_lat;	/* Point where scale should apply */
	double length;		/* How long the scale is in measure units */
	BOOLEAN boxdraw;	/* TRUE if we want to plot a rectangle behind the scale */
	BOOLEAN boxfill;	/* TRUE if we want to paint/fill a rectangle behind the scale */
	BOOLEAN plot;		/* TRUE if we want to draw the scale */
	BOOLEAN fancy;		/* TRUE for a fancy map scale */
	BOOLEAN gave_xy;	/* TRUE if x0, y0 was given in cartesian map coordinates and not lon/lat */
	char measure;		/* The unit, i.e., m (miles), n (nautical miles), or k (kilometers) */
	char justify;		/* Placement of label: t(op), b(ottom), l(eft), r(ight), u(nit) */
	char label[64];		/* Alternative user-specified label */
	struct GMT_FILL fill;	/* Fill to use for background rectangle */
	struct GMT_PEN pen;	/* Pen to use for background rectangle */
};

struct MAP_ROSE {	/* Used to plot a map direction "rose" in psbasemap and pscoast */
	double lon, lat;	/* Location of center point of rose on the map in lon/lat space */
	double x0, y0;		/* Location of center point of scale on the map in inches x/y */
	double size;		/* Diameter of the rose in measure units */
	double declination;	/* Magnetic declination if needed */
	double a_int[2];	/* Annotation interval for geographic and magnetic directions */
	double f_int[2];	/* Tick (large) interval for geographic and magnetic directions */
	double g_int[2];	/* Tick (small) interval for geographic and magnetic directions */
	BOOLEAN plot;		/* TRUE if we want to draw the rose */
	BOOLEAN fancy;		/* TRUE for a fancy map rose */
	BOOLEAN gave_xy;	/* TRUE if x0, y0 was given in cartesian map coordinates and not lon/lat */
	int kind;		/* 0 : 90 degrees, 1 : 45 degrees, 2 : 22.5 degrees between points */
	char label[4][64];	/* User-changable labels for W, E, S, N point */
	char dlabel[128];	/* Magnetic declination label */
};

struct GMT_FONT {		/* Information for each font */
	char *name;			/* Name of the font */
	double height;			/* Height of letter "A" for unit fontsize */
};

struct GMT_LINES {		/* For holding multisegment lines in memory */
	double *lon, *lat;	/* Coordinates x,y */
	double dist;		/* Distance from a point to this feature */
	int *seg;		/* Segment number information */
	int np;			/* Number of points in this segment */
	int polar;		/* TRUE if a polygon and enclosing N or S pole */
	char *label;		/* Label string (if applicable) */
};


/*--------------------------------------------------------------------*/
/*	External variables for misc purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct GMTDEFAULTS gmtdefs;

EXTERN_MSC int N_FONTS;				/* Number of fonts loaded from share/pslib */
EXTERN_MSC char *GMTHOME;			/* Points to the GMT home directory with lib subdir */
EXTERN_MSC char *GMT_DATADIR;			/* Points to the GMT misc directory [if set] */
EXTERN_MSC char *GMT_GRIDDIR;			/* Points to the GMT grids directory [if set] */
EXTERN_MSC char *GMT_IMGDIR;			/* Points to the GMT img directory [if set] */
EXTERN_MSC char *GMT_unit_names[];
EXTERN_MSC double GMT_u2u[4][4];		/* measure unit translation matrix 4 x 4*/
EXTERN_MSC struct GMT_FONT *GMT_font;
EXTERN_MSC char *GMT_unique_option[];
EXTERN_MSC char *GMT_keywords[];
EXTERN_MSC char *GMT_media_name[];
EXTERN_MSC struct GMT_MEDIA GMT_media[];
EXTERN_MSC char **GMT_user_media_name;
EXTERN_MSC struct GMT_MEDIA *GMT_user_media;
EXTERN_MSC int GMT_n_user_media;
EXTERN_MSC char *GMT_weekdays[];
EXTERN_MSC struct GMT_HASH GMT_month_hashnode[12];
EXTERN_MSC struct GMT_TIME_SYSTEM GMT_time_system[];
EXTERN_MSC struct GMT_TIME_LANGUAGE GMT_time_language;
EXTERN_MSC struct GMT_PEN_NAME GMT_penname[];

EXTERN_MSC float GMT_f_NaN;		/* Holds IEEE not-a-number float */
EXTERN_MSC double GMT_d_NaN;		/* Holds IEEE not-a-number double */
EXTERN_MSC BOOLEAN GMT_quick;		/* TRUE if short usage message is desired (must say program - ) */
EXTERN_MSC char *GMT_program;		/* Name of current GMT program */
EXTERN_MSC int GMT_no_rgb[];
EXTERN_MSC int GMT_oldargc;
EXTERN_MSC char *GMT_oldargv[];		/* Pointers to old common arguments */
EXTERN_MSC char *GMT_degree_symbol[4][3];	/* Contains the two octal codes for small and large degree symbols, for each char encoding */
EXTERN_MSC char *GMT_minute_symbol[4][2];	/* Standard encoding minute symbol */
EXTERN_MSC char *GMT_second_symbol[4][2];	/* Standard encoding second symbol */
EXTERN_MSC BOOLEAN GMT_processed_option[N_UNIQUE];	/* TRUE if option has been procssed */

/*--------------------------------------------------------------------*/
/*	For i/o purposes */
/*--------------------------------------------------------------------*/


EXTERN_MSC FILE *GMT_stdin, *GMT_stdout;
EXTERN_MSC PFI GMT_input, GMT_output, GMT_input_ascii;
EXTERN_MSC BOOLEAN GMT_geographic_in;	/*TRUE if input data is long/lat */
EXTERN_MSC BOOLEAN GMT_geographic_out;	/*TRUE if output data is long/lat */
EXTERN_MSC double GMT_data[BUFSIZ];
EXTERN_MSC double GMT_grd_in_nan_value, GMT_grd_out_nan_value;
EXTERN_MSC int GMT_n_file_suffix;
EXTERN_MSC int *GMT_file_id;
EXTERN_MSC double *GMT_file_scale, *GMT_file_offset, *GMT_file_nan;
EXTERN_MSC char **GMT_file_suffix;
EXTERN_MSC int GMT_fd_history;	/* File descriptor for .gmtcommands4 */
EXTERN_MSC BOOLEAN GMT_lock;	/*T/F for advisory file locking */
EXTERN_MSC int GMT_pad[4];

/*--------------------------------------------------------------------*/
/*	For plotting purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct PLOT_FRAME frame_info;	/* Boundary info for linear plots and maps */
EXTERN_MSC struct GMT_TRUNCATE_TIME GMT_truncate_time;	/* Used to round off times to mid-interval */
EXTERN_MSC double *GMT_x_plot;			/* Holds the x/y (inches) of a line to be plotted */
EXTERN_MSC double *GMT_y_plot;
EXTERN_MSC int *GMT_pen;			/* Pen (3 = up, 2 = down) for these points */
EXTERN_MSC int GMT_n_plot;			/* Number of such points */
EXTERN_MSC int GMT_n_alloc;			/* Current size of allocated arrays */
EXTERN_MSC int GMT_x_status_new;		/* Tells us what quadrant old and new points are in */
EXTERN_MSC int GMT_y_status_new;
EXTERN_MSC int GMT_x_status_old;
EXTERN_MSC int GMT_y_status_old;
EXTERN_MSC int GMT_corner;
EXTERN_MSC BOOLEAN GMT_world_map;		/* TRUE if map has 360 degrees of longitude range */
EXTERN_MSC BOOLEAN GMT_world_map_tm;		/* TRUE if TM map is global? */
EXTERN_MSC BOOLEAN GMT_on_border_is_outside;	/* TRUE if point exactly on the map border should be considered outside */
EXTERN_MSC double GMT_map_width;		/* Full width of this world map */
EXTERN_MSC double GMT_map_height;		/* Full height of this world map */
EXTERN_MSC double GMT_half_map_size;		/* Half width of this world map */
EXTERN_MSC double GMT_half_map_height;		/* Half height of this world map */
EXTERN_MSC PFI GMT_outside;			/* pointer to function checking if a lon/lat point is outside map */
EXTERN_MSC PFI GMT_crossing;			/* pointer to functions returning crossover point at boundary */
EXTERN_MSC PFI GMT_overlap;			/* pointer to function checking for overlap between 2 regions */
EXTERN_MSC PFI GMT_map_clip;			/* pointer to functions that clip a polygon to fit inside map */
EXTERN_MSC PFD GMT_left_edge, GMT_right_edge;	/* pointer to functions that returns the left,right edge of map */
EXTERN_MSC PFD GMT_distance_func;		/* pointer to function returning distance between two points points */
EXTERN_MSC BOOLEAN GMT_z_periodic;		/* TRUE if grid values are 0-360 degrees (phases etc) */
EXTERN_MSC PFI GMT_wrap_around_check;		/* Does x or y wrap checks */
EXTERN_MSC PFI GMT_map_jump;			/* TRUE if we jump in x or y */
EXTERN_MSC PFB GMT_will_it_wrap;		/* TRUE if consecutive points indicate wrap */
EXTERN_MSC PFB GMT_this_point_wraps;		/* Used in above */
EXTERN_MSC PFV GMT_get_crossings;		/* Returns map crossings in x or y */
EXTERN_MSC PFI GMT_truncate;			/* Truncate polygons agains boundaries */
EXTERN_MSC BOOLEAN GMT_meridian_straight;	/* TRUE if meridians plot as straight lines */
EXTERN_MSC BOOLEAN GMT_parallel_straight;	/* TRUE if parallels plot as straight lines */
EXTERN_MSC int GMT_3D_mode;			/* Determines if we draw fore and/or back 3-D box lines */
EXTERN_MSC char *GMT_plot_format[3][2];		/* Keeps the 6 formats for dd:mm:ss plot output */

/*--------------------------------------------------------------------*/
/*	For projection purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct MAP_PROJECTIONS project_info;
EXTERN_MSC struct THREE_D z_project;
EXTERN_MSC struct GMT_DATUM_CONV GMT_datum;	/*	For datum conversions */
EXTERN_MSC PFI GMT_forward, GMT_inverse;	/*	Pointers to the selected mapping functions */
EXTERN_MSC PFI GMT_x_forward, GMT_x_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFI GMT_y_forward, GMT_y_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFI GMT_z_forward, GMT_z_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFD GMT_scan_time_string;		/*	pointer to functions that converts timestring to secs */
EXTERN_MSC double DEG2M;			/*	Scale converting degrees to meters on spheroid	*/
EXTERN_MSC double DEG2KM;			/*	Scale converting degrees to km on spheroid	*/

#include "gmt_math.h"		/* Machine-dependent macros for non-POSIX math functions */
#ifdef WIN32
#define GMT_make_fnan(x) (((unsigned int *) &x)[0] = 0x7fffffff)
#define GMT_make_dnan(x) (((unsigned int *) &x)[0] = 0xffffffff, ((unsigned int *) &x)[1] = 0x7fffffff)
#else
#include "gmt_nan.h"		/* Machine-dependent macros for making and testing NaNs */
#endif
#include "gmt_version.h"        /* Only contains the current GMT version number */
#include "gmt_project.h"        /* Define project_info and frame_info structures */
#include "gmt_grd.h"            /* Define grd file header structure */
#include "gmt_io.h"		/* Defines structures and macros for table i/o */
#include "gmt_colors.h"         /* Defines color/shading global structure */
#include "gmt_grdio.h"          /* Defines function pointers for grd i/o operations */
#include "pslib.h"		/* Defines pslib function prototypes */
#include "gmt_shore.h"		/* Defines structures used when reading shore database */
#include "gmt_boundcond.h"	/* Boundary conditions for grids */
#include "gmt_bcr.h"		/* Grid resampling functions */
#include "gmt_calclock.h"	/* Calendar/time functions */
#include "gmt_symbol.h"		/* Custom symbol functions */
#include "gmt_contour.h"	/* Contour label structure and functions */
#include "gmt_funcnames.h"      /* List of functions */

#endif  /* _GMT_H */
