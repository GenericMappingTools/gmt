/*--------------------------------------------------------------------
 *	$Id: gmt.h,v 1.6 2001-03-09 21:14:37 pwessel Exp $
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
 * gmt.h is the main include file for GMT.  It contains definitions
 * for several of the structures and parameters used by all programs.
 *
 * Author:	Paul Wessel
 * Date:	01-MAR-1991
 * Revised:	22-FEB-2001
 * Version:	3.4
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
#include <stddef.h>
#ifdef __MACHTEN__
/* Kludge to fix a Macthen POSIX bug */
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

#define GMT_CONV_LIMIT	1.0e-8	/* Fairly tight convergence limit or "close to zero" limit */
#define SMALL		1.0e-4	/* Needed when results aren't exactly zero but close */
#define GMT_CHUNK	2000
#define GMT_SMALL_CHUNK	50
#define GMT_TINY_CHUNK	5
#define GMT_VERSION	"3.4b"
#define CNULL		((char *)NULL)
#define VNULL		((void *)NULL)
#define GMT_CM		0
#define GMT_INCH	1
#define GMT_M		2
#define GMT_PT		3

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

/* Macros for byte-order swapping (From John M. Kuhn, NOAA) */

#define GMT_swab2(data) ((((data) & 0xff) << 8) | ((unsigned short) (data) >> 8))
#define GMT_swab4(data) \
	(((data) << 24) | (((data) << 8) & 0x00ff0000) | \
	(((data) >> 8) & 0x0000ff00) | ((unsigned long)(data) >> 24))

/*--------------------------------------------------------------------
 *			GMT TYPEDEF DEFINITIONS
 *--------------------------------------------------------------------*/

typedef int BOOLEAN;		/* BOOLEAN used for logical variables */
typedef void (*PFV) ();		/* PFV declares a pointer to a function returning void */
typedef int (*PFI) ();		/* PFI declares a pointer to a function returning an int */
typedef BOOLEAN (*PFB) ();	/* PFB declares a pointer to a function returning a BOOLEAN */
typedef double (*PFD) ();	/* PFD declares a pointer to a function returning a double */

/*--------------------------------------------------------------------
 *			GMT PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

#define N_UNIQUE 59		/* Number of unique options */
#define N_KEYS 58		/* Number of gmt defaults */
#define N_FONTS 39		/* Number of fonts in the PS_font_names.h include file */
#define GMT_N_MEDIA 29		/* Number of standard paper formats in the GMT_media_names.h include file */
#define HASH_SIZE 61		/* Used in get_gmtdefaults, should be ~> N_KEYS */

/* This structure contains default parameters for the GMT system */

#define N_ELLIPSOIDS 14

#define GMT_PEN_LEN	128
#define GMT_PENWIDTH	0.25	/* Default pen width in points */

struct GMT_PEN {	/* Holds pen attributes */
	double width;	/* In points */
	double offset;	/* In points */
	int rgb[3];
	char texture[GMT_PEN_LEN];	/* In points */
};

struct GMTDEFAULTS {
	double anot_min_angle;		/* If angle between map boundary and anotation is less, no anot is drawn [20] */
	double anot_min_spacing;	/* If an anotation is closer that this to an older anotation, the anot is skipped [0.0] */
	int anot_font;			/* Font for anotations [Helvetica] */
	int anot_font_size;		/* Font size for anotations in points [14] */
	double anot_offset;		/* Distance between anotation and tickmarks [0.075] */
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
	double grid_cross_size;		/* Size of gridcrosses.  0 means draw continuous gridlines */
	struct GMT_PEN grid_pen;	/* Pen attributes for gridlines [1] */
	BOOLEAN gridfile_shorthand;	/* Use shorthand suffix notation for embedded formats [FALSE] */
	int header_font;		/* Font for headers [Helvetica] */
	int header_font_size;		/* Font size for headers in points [36] */
	double hsv_min_saturation;	/* For smallest or most negative intensity [1.0] */
	double hsv_max_saturation;	/* For largest or most positive intensity [0.1] */
	double hsv_min_value;		/* For smallest or most negative intensity [0.3] */
	double hsv_max_value;		/* For largest or most positive intensity [1.0] */
	int interpolant;		/* Choose between 0 (Linear), 1 (Akima), or 2 (Cubic spline) */
	BOOLEAN io_header;		/* Input data has header records [FALSE] */
	int n_header_recs;		/* number of header records [0] */
	int label_font;			/* Font for labels [Helvetica] */
	int label_font_size;		/* Font size for labels in points [24] */
	BOOLEAN last_page;		/* If TRUE, terminate plot system when done [TRUE] */
	double line_step;		/* Maximum straight linesegment length for arcuate lines */
	double map_scale_factor;	/* Central mapscale factor, typically 0.9996 */
	double map_scale_height;	/* Height of map scale drawn on a map [0.075] */
	int measure_unit;		/* Choose 0 (cm), 1 (inch), 2 (m) or 3 (point) [1] */
	int media;			/* Default paper media [25(Letter)] */
	int n_copies;			/* Number of copies pr plot [1] */
	int n_lat_nodes;		/* No of points to use for drawing a latitudal line [50] */
	int n_lon_nodes;		/* No of points to use for drawing a longitudal line [50] */
	double dlon, dlat;		/* Corresponding increment in lon/lat */
	int oblique_anotation;		/* Controls anotations and tick angles etc. [0] */
	BOOLEAN overlay;		/* Make plot in overlay mode [FALSE] */
	int page_rgb[3];		/* Color of the page [255/255/255 white] */
	int page_orientation;		/* Orientation of page [0 = Landscape, 1 = Portrait] */
	int paper_width[2];		/* Width and height of paper to plot on in points [Letter or A4] */
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
	BOOLEAN xy_toggle;		/* TRUE means read/write I/O as lat/lon instead of lon/lat [FALSE] */
	int y_axis_type;		/* Select y-axis with horizontal (0) or vertical (1) annotations  [0] */
	struct ELLIPSOID {	/* Information about a particular ellipsoid */
		/* Table taken from Snyder "Map projection - a working manual", p 12 Table 1 */
		char name[32];
		int date;
		double eq_radius;
		double pol_radius;
		double flattening;
	} ellipse[N_ELLIPSOIDS];	/* Ellipsoid parameters */
	
};

struct GMT_HASH {	/* Used to related keywords to gmtdefaults entry */
	struct GMT_HASH *next;
	int id;
	char *key;
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
	int width;	/* Width in points */
	int height;	/* Height in points */
};

/*  Moved from gmt_plot.c */
struct XINGS {
        double xx[2], yy[2];    /* Cartesian coordinates of intersection with map boundary */
        double angle[2];        /* Angles of intersection */
        int sides[2];           /* Side id of intersection */
        int nx;                 /* Number of intersections (1 or 2) */
};

struct BCR {	/* Used mostly in gmt_support.c */
	double	nodal_value[4][4];	/* z, dz/dx, dz/dy, d2z/dxdy at 4 corners  */
	double	bcr_basis[4][4];	/* multiply on nodal vals, yields z at point */
	double	bl_basis[4];		/* bilinear basis functions  */
	double	rx_inc;			/* 1.0 / grd.x_inc  */
	double	ry_inc;			/* 1.0 / grd.y_inc  */
	double	offset;			/* 0 or 0.5 for grid or pixel registration  */
/* If we later want to estimate of dz/dx or dz/dy, we will need [4][4] basis for these  */
	int	ij_move[4];		/* add to ij of zero vertex to get other vertex ij  */
	int	i;			/* Location of current nodal_values  */
	int	j;			/* Ditto.   */
	int	bilinear;		/* T/F use bilinear instead of bicubic  */
	int	nan_condition;		/* T/F we cannot evaluate; return z = NaN  */
	int	ioff;			/* Padding on west side of array  */
	int	joff;			/* Padding on north side of array  */
	int	mx;			/* Padded array dimension  */
	int	my;			/* Ditto  */
};


/*--------------------------------------------------------------------*/
/*	External variables for misc purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct GMTDEFAULTS gmtdefs;

EXTERN_MSC char *GMTHOME;			/* Points to the GMT home directory with lib subdir */
EXTERN_MSC char *GMT_unit_names[];
EXTERN_MSC double GMT_u2u[4][4];		/* measure unit translation matrix 4 x 4*/
EXTERN_MSC char *GMT_font_name[];
EXTERN_MSC double GMT_font_height[];
EXTERN_MSC char *GMT_unique_option[];
EXTERN_MSC char *GMT_keywords[];
EXTERN_MSC char *GMT_media_name[];
EXTERN_MSC struct GMT_MEDIA GMT_media[];
EXTERN_MSC char **GMT_user_media_name;
EXTERN_MSC struct GMT_MEDIA *GMT_user_media;
EXTERN_MSC int GMT_n_user_media;

EXTERN_MSC float GMT_f_NaN;		/* Holds IEEE not-a-number float */
EXTERN_MSC double GMT_d_NaN;		/* Holds IEEE not-a-number double */
EXTERN_MSC BOOLEAN GMT_quick;		/* TRUE if short usage message is desired (must say program - ) */
EXTERN_MSC char *GMT_program;		/* Name of current GMT program */
EXTERN_MSC int GMT_no_rgb[];
EXTERN_MSC int GMT_oldargc;
EXTERN_MSC char *GMT_oldargv[];		/* Pointers to old common arguments */
EXTERN_MSC char *GMT_degree_symbol[2];	/* Contains the two octal codes for small and large degree symbols */

/*--------------------------------------------------------------------*/
/*	For i/o purposes */
/*--------------------------------------------------------------------*/


EXTERN_MSC FILE *GMT_stdin, *GMT_stdout;
EXTERN_MSC PFI GMT_input, GMT_output;
EXTERN_MSC BOOLEAN GMT_geographic_in;	/*TRUE if input data is long/lat */
EXTERN_MSC BOOLEAN GMT_geographic_out;	/*TRUE if output data is long/lat */
EXTERN_MSC double GMT_data[BUFSIZ];
EXTERN_MSC double GMT_grd_in_nan_value, GMT_grd_out_nan_value;
EXTERN_MSC int GMT_n_file_suffix;
EXTERN_MSC int *GMT_file_id;
EXTERN_MSC double *GMT_file_scale, *GMT_file_offset, *GMT_file_nan;
EXTERN_MSC char **GMT_file_suffix;
EXTERN_MSC int GMT_fd_history;	/* File descriptor for .gmtcommands */
EXTERN_MSC BOOLEAN GMT_lock;	/*T/F for advisory file locking */
EXTERN_MSC int GMT_pad[4];

/*--------------------------------------------------------------------*/
/*	For plotting purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct MAP_FRAME frame_info;
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

/*--------------------------------------------------------------------*/
/*	For projection purposes */
/*--------------------------------------------------------------------*/

EXTERN_MSC struct MAP_PROJECTIONS project_info;
EXTERN_MSC struct THREE_D z_project;
EXTERN_MSC PFI GMT_forward, GMT_inverse;	/*	Pointers to the selected mapping functions */
EXTERN_MSC PFI GMT_x_forward, GMT_x_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFI GMT_y_forward, GMT_y_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFI GMT_z_forward, GMT_z_inverse;	/*	Pointers to the selected linear functions */
EXTERN_MSC PFD GMT_scan_time_string;		/*	pointer to functions that converts timestring to secs */

#include "gmt_math.h"		/* Machine-dependent macros for non-POSIX math functions */
#ifdef WIN32
#define GMT_make_fnan(x) (((unsigned int *) &x)[0] = 0x7fffffff)
#define GMT_make_dnan(x) (((unsigned int *) &x)[0] = 0xffffffff, ((unsigned int *) &x)[1] = 0x7fffffff)
#else
#include "gmt_nan.h"		/* Machine-dependent macros for making and testing NaNs */
#endif
#include "gmt_project.h"        /* Define project_info and frame_info structures */
#include "gmt_grd.h"            /* Define grd file header structure */
#include "gmt_io.h"		/* Defines structurs and macros for table i/o */
#include "gmt_colors.h"         /* Defines color/shading global structure */
#include "gmt_grdio.h"          /* Defines function pointers for grd i/o operations */
#include "pslib.h"		/* Defines pslib function prototypes */
#include "gmt_shore.h"		/* Defines structures used when reading shore database */
#include "gmt_funcnames.h"      /* List of functions */
#include "gmt_boundcond.h"	/* Boundary conditions for grids */
#include "gmt_bcr.h"		/* Grid resampling functions */

#endif  /* _GMT_H */
