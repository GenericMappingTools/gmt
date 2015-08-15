/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * gmt_types.h contains definitions of special types used by GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

/*!
 * \file gmt_types.h
 * \brief Definitions of special types used by GMT.
 */

#ifndef _GMT_TYPES_H
#define _GMT_TYPES_H
#include <stdbool.h>
#include <stdint.h>

/*--------------------------------------------------------------------
 * GMT TYPE DEFINITIONS
 *--------------------------------------------------------------------*/

/*! Definition of MATH_MACRO used by grdmath and gmtmath */
struct MATH_MACRO {
	unsigned int n_arg;	/* How many commands this macro represents */
	char *name;	/* The macro name */
	char **arg;	/* List of those commands */
};

/*! Definition of structure use for finding optimal nx.ny for surface */
struct GMT_SURFACE_SUGGESTION {	/* Used to find top ten list of faster grid dimensions  */
	unsigned int nx;
	unsigned int ny;
	double factor;	/* Speed up by a factor of factor  */
};

/*! Definition of structure used for holding information of integer items to be selected */
struct GMT_INT_SELECTION {	/* Used to hold array with items (0-n) that have been selected */
	uint64_t *item;		/* Array with item numbers given (0 is first), sorted into ascending order */
	uint64_t n;		/* Number of items */
	uint64_t current;	/* Current item in item array */
	bool invert;		/* Instead select the items NOT listed in item[] */
};

/*! Definition of structure used for holding information of text items to be selected */
struct GMT_TEXT_SELECTION {	/* Used to hold array with items (0-n) that have been selected */
	char **pattern;		/* Array with text items given, sorted into lexical order */
	int ogr_item;		/* Used if ogr_match is true */
	uint64_t n;		/* Number of items */
	bool invert;		/* Instead select the items NOT listed in item[] */
	bool *regexp;		/* Item is a regex expression */
	bool *caseless;		/* Treat as caseless */
	bool ogr_match;		/* Compare pattern to an OGR item */
};

/*! For weighted mean/mode */
struct GMT_OBSERVATION {
	float value;
	float weight;
};

/*! For trend-fitting models */
struct GMT_MODEL_TERM {	/* A single model term */
	unsigned int kind;	/* GMT_POLYNOMIAL | GMT_COSINE | GMT_SINE | GMT_FOURIER */
	unsigned int order[2];	/* Polygon or Fourier order */
	unsigned int type;	/* 0-7 for which kind of sin/cos combination */
};

struct GMT_MODEL {	/* A model consists of n_terms */
	bool robust;		/* True for L1 fitting [L2] */
	bool chebyshev;		/* True if given polynomial of order n */
	bool intercept;		/* True if given model has intercept */
	bool got_origin[2];	/* True if we got origin(s) */
	bool got_period[2];	/* True if we got periods(s) */
	unsigned int dim;	/* 1 or 2 */
	unsigned int type;	/* 1 = poly, 2 = Fourier, 3 = both */
	unsigned int n_terms;	/* Terms in model */
	double origin[2];	/* x (or t) and y origins */
	double period[2];	/* x (or t) and y periods */
	struct GMT_MODEL_TERM term[GMT_N_MAX_MODEL];
};

struct GMT_DIST {	/* Holds info for a particular distance calculation */
	bool init;	/* true if we have initialized settings for this type via GMT_init_distaz */
	bool arc;	/* true if distances are in deg/min/sec or arc; otherwise they are e|f|k|M|n or Cartesian */
	double (*func) (struct GMT_CTRL *, double, double, double, double);	/* pointer to function returning distance between two points points */
	double scale;	/* Scale to convert function output to desired unit */
};

struct GMT_MAP {		/* Holds all map-related parameters */
	struct GMT_PLOT_FRAME frame;		/* Everything about the frame parameters */
	int this_x_status;			/* Tells us what quadrant old and new points are in (-4/4) */
	int this_y_status;
	int prev_x_status;
	int prev_y_status;
	int corner;			/* Tells us which corner 1-4 or -1 if not a corner */
	bool coastline;			/* true if we are currently plotting the coastline data in pscoast */
	bool on_border_is_outside;		/* true if a point exactly on the map border shoud be considered outside the map */
	bool is_world;			/* true if map has 360 degrees of longitude range */
	bool is_world_tm;			/* true if GMT_TM map is global? */
	bool lon_wrap;			/* true when longitude wrapping over 360 degrees is allowed */
	bool z_periodic;			/* true if grid values are 0-360 degrees (phases etc) */
	bool loxodrome;				/* true if we are computing loxodrome distances */
	unsigned int meridian_straight;		/* 1 if meridians plot as straight lines, 2 for special case */
	unsigned int parallel_straight;		/* 1 if parallels plot as straight lines, 2 for special case */
	unsigned int n_lon_nodes;		/* Somewhat arbitrary # of nodes for lines in longitude (may be reset in gmt_map.c) */
	unsigned int n_lat_nodes;		/* Somewhat arbitrary # of nodes for lines in latitude (may be reset in gmt_map.c) */
	unsigned int path_mode;		/* 0 if we should call GMT_fix_up_path to resample across gaps > path_step, 1 to leave alone */
	double width;				/* Full width in inches of this world map */
	double height;				/* Full height in inches of this world map */
	double half_width;			/* Half width in inches of this world map */
	double half_height;			/* Half height of this world map */
	double dlon;				/* Steps taken in longitude along gridlines (gets reset in gmt_init.c) */
	double dlat;				/* Steps taken in latitude along gridlines (gets reset in gmt_init.c) */
	double path_step;			/* Sampling interval if resampling of paths should be done */
	bool (*outside) (struct GMT_CTRL *, double, double);	/* Pointer to function checking if a lon/lat point is outside map */
	bool (*overlap) (struct GMT_CTRL *, double, double, double, double);	/* Pointer to function checking for overlap between 2 regions */
	bool (*will_it_wrap) (struct GMT_CTRL *, double *, double *, uint64_t, uint64_t *);	/* true if consecutive points indicate wrap */
	int (*jump) (struct GMT_CTRL *, double, double, double, double);	/* true if we jump in x or y */
	unsigned int (*crossing) (struct GMT_CTRL *, double, double, double, double, double *, double *, double *, double *, unsigned int *);	/* Pointer to functions returning crossover point at boundary */
	uint64_t (*clip) (struct GMT_CTRL *, double *, double *, uint64_t, double **, double **, uint64_t *);	/* Pointer to functions that clip a polygon to fit inside map */
	double (*left_edge) (struct GMT_CTRL *, double);	/* Pointers to functions that return left edge of map */
	double (*right_edge) (struct GMT_CTRL *, double);	/* Pointers to functions that return right edge of map */
	struct GMT_DIST dist[3];		/* struct with pointers to functions/scales returning distance between two points points */
	bool (*near_lines_func) (struct GMT_CTRL *, double, double, struct GMT_DATATABLE *, unsigned int, double *, double *, double *);	/* Pointer to function returning distance to nearest line among a set of lines */
	bool (*near_a_line_func) (struct GMT_CTRL *, double, double, uint64_t, struct GMT_DATASEGMENT *, unsigned int, double *, double *, double *);	/* Pointer to function returning distance to line */
	bool (*near_point_func) (struct GMT_CTRL *, double, double, struct GMT_DATATABLE *, double);	/* Pointer to function returning distance to nearest point */
	unsigned int (*wrap_around_check) (struct GMT_CTRL *, double *, double, double, double, double, double *, double *, unsigned int *);	/* Does x or y wrap checks */
	double (*azimuth_func) (struct GMT_CTRL *, double, double, double, double, bool);	/* Pointer to function returning azimuth between two points points */
	void (*get_crossings) (struct GMT_CTRL *, double *, double *, double, double, double, double);	/* Returns map crossings in x or y */
	double (*geodesic_meter) (struct GMT_CTRL *, double, double, double, double);	/* pointer to geodesic function returning distance between two points points in meter */
	double (*geodesic_az_backaz) (struct GMT_CTRL *, double, double, double, double, bool);	/* pointer to geodesic function returning azimuth or backazimuth between two points points */
};

struct GMT_GCAL {	/* (proleptic) Gregorian calendar  */
	int year;		/* signed; negative and 0 allowed  */
	unsigned int month;	/* Always between 1 and 12  */
	unsigned int day_m;	/* Day of month; always in 1 - 31  */
	unsigned int day_y;	/* Day of year; 1 thru 366  */
	unsigned int day_w;	/* Day of week; 0 (Sun) thru 6 (Sat)  */
	int iso_y;		/* ISO year; not necessarily == year */
	unsigned int iso_w;	/* ISO week of iso_y; must be in 1 -- 53  */
	unsigned int iso_d;	/* ISO day of iso_w; uses 1 (Mon) thru 7 (Sun)  */
	unsigned int hour;	/* 00 through 23  */
	unsigned int min;	/* 00 through 59  */
	double sec;		/* 00 through 59.xxxx; leap not yet handled  */
};

struct GMT_Y2K_FIX {	/* The issue that refuses to go away... */
	unsigned int y2_cutoff;	/* The 2-digit offset year.  If y2 >= y2_cuttoff, add y100 else add y200 */
	int y100;	/* The multiple of 100 to add to the 2-digit year if we are above the time_Y2K_offset_year */
	int y200;	/* The multiple of 100 to add to the 2-digit year if we are below the time_Y2K_offset_year */
};

struct GMT_MOMENT_INTERVAL {
	struct GMT_GCAL	cc[2];		
	double dt[2];		
	double sd[2];		/* Seconds since the start of the day.  */
	int64_t rd[2];
	unsigned int step;
	char unit;
};

struct GMT_TRUNCATE_TIME {		/* Used when TIME_IS_INTERVAL is not OFF */
	struct GMT_MOMENT_INTERVAL T;
	unsigned int direction;		/* 0 [+] to center on next interval, 1 [-] for previous interval */
};

struct GMT_TIME_CONV {		/* Holds all time-related parameters */
	struct GMT_TRUNCATE_TIME truncate;
	struct GMT_Y2K_FIX Y2K_fix;		/* Used to convert 2-digit years to 4-digit years */
	time_t tic;				/* Last system time marker */
	int64_t today_rata_die;			/* The rata die of current day at start of program */
};

struct GMT_LANGUAGE {		/* Language-specific text strings for calendars, map annotations, etc. */
	char month_name[4][12][GMT_LEN16];	/* Full, short, 1-char, and short (upper case) month names */
	char day_name[3][7][GMT_LEN16];	/* Full, short, and 1-char weekday names */
	char week_name[3][GMT_LEN16];	/* Full, short, and 1-char versions of the word Week */
	char cardinal_name[3][4][GMT_LEN16];	/* Full, and abbreviated (map annot., direction) versions of compass directions */
};

struct GMT_INIT { /* Holds misc run-time parameters */
	unsigned int n_custom_symbols;
	const char *module_name;      /* Name of current module or NULL if not set */
	const char *module_lib;       /* Name of current shared library or NULL if not set */
	/* The rest of the struct contains pointers that may point to memory not included by this struct */
	char *runtime_bindir;         /* Directory that contains the main exe at run-time */
	char *runtime_libdir;         /* Directory that contains the main shared lib at run-time */
	char *runtime_plugindir;      /* Directory that contains the main supplemental plugins at run-time */
	char *history[GMT_N_UNIQUE];  /* The internal gmt.history information */
	struct GMT_CUSTOM_SYMBOL **custom_symbol; /* For custom symbol plotting in psxy[z]. */
};

struct GMT_PLOT {		/* Holds all plotting-related parameters */
	uint64_t n;			/* Number of such points */
	size_t n_alloc;			/* Size of allocated plot arrays */
	bool r_theta_annot;		/* true for special r-theta map annotation (see GMT_get_annot_label) */
	unsigned int mode_3D;		/* Determines if we draw fore and/or back 3-D box lines [Default is both] */
	unsigned int *pen;		/* Pen (PSL_MOVE = up, PSL_DRAW = down) for these points */
	struct GMT_PLOT_CALCLOCK calclock;
	/* The rest of the struct contains pointers that may point to memory not included by this struct */
	double *x;			/* Holds the x/y (inches) of a line to be plotted */
	double *y;
	char format[3][2][GMT_LEN256];	/* Keeps the 6 formats for dd:mm:ss plot output */
};

struct GMT_CURRENT {
	/* These are internal parameters that need to be passed around between
	 * many GMT functions.  These values may change by user interaction. */
	struct GMT_DEFAULTS setting;	/* Holds all GMT defaults parameters */
	struct GMT_IO io;		/* Holds all i/o-related parameters */
	struct GMT_PROJ proj;		/* Holds all projection-related parameters */
	struct GMT_MAP map;		/* Holds all projection-related parameters */
	struct GMT_PLOT plot;		/* Holds all plotting-related parameters */
	struct GMT_TIME_CONV time;	/* Holds all time-related parameters */
	struct GMT_LANGUAGE language;	/* Holds all language-related parameters */
	struct GMT_PS ps;		/* Hold parameters related to PS setup */
	struct GMT_OPTION *options;	/* Pointer to current program's options */
	struct GMT_FFT_HIDDEN fft;	/* Structure with info that must survive between FFT calls */
	struct GMT_GDALREAD_IN_CTRL  gdal_read_in;  /* Hold parameters related to options transmitted to gdalread */ 
	struct GMT_GDALREAD_OUT_CTRL gdal_read_out; /* Hold parameters related to options transmitted from gdalread */ 
	struct GMT_GDALWRITE_CTRL    gdal_write;    /* Hold parameters related to options transmitted to gdalwrite */ 
};

struct GMT_INTERNAL {
	/* These are internal parameters that need to be passed around between
	 * many GMT functions.  These may change during execution but are not
	 * modified directly by user interaction. */
	unsigned int func_level;	/* Keeps track of what level in a nested GMT_func calling GMT_func etc we are.  0 is top function */
	size_t mem_cols;		/* Current number of allocated columns for temp memory */
	size_t mem_rows;		/* Current number of allocated rows for temp memory */
	double **mem_coord;		/* Columns of temp memory */
	struct MEMORY_TRACKER *mem_keeper;	/* Only filled when #ifdef MEMDEBUG  */
};

struct GMT_SHORTHAND {	/* Holds information for each grid extension shorthand read from the user's .gmtio file */
	char *suffix; /* suffix of file */
	char *format; /* format: ff/scale/offset/invalid */
};

struct GMT_SESSION {
	/* These are parameters that is set once at the start of a GMT session and
	 * are essentially read-only constants for the duration of the session */
	FILE *std[3];			/* Pointers for standard input, output, and error */
	void * (*input_ascii) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to function reading ASCII tables only */
	int (*output_ascii) (struct GMT_CTRL *, FILE *, uint64_t, double *);	/* Pointer to function writing ASCII tables only */
	unsigned int n_fonts;		/* Total number of fonts returned by GMT_init_fonts */
	unsigned int n_user_media;	/* Total number of user media returned by gmt_load_user_media */
	size_t min_meminc;		/* with -DMEMDEBUG, sets min/max memory increments */
	size_t max_meminc;
	float f_NaN;			/* Holds the IEEE NaN for floats */
	double d_NaN;			/* Holds the IEEE NaN for doubles */
	double no_rgb[4];		/* To hold {-1, -1, -1, 0} when needed */
	double u2u[4][4];		/* u2u is the 4x4 conversion matrix for cm, inch, m, pt */
	char unit_name[4][8];		/* Full name of the 4 units cm, inch, m, pt */
	struct GMT_HASH rgb_hashnode[GMT_N_COLOR_NAMES];/* Used to translate colornames to r/g/b */
	bool rgb_hashnode_init;		/* true once the rgb_hashnode array has been loaded; false otherwise */
	unsigned int n_shorthands;			/* Length of arrray with shorthand information */
	char *grdformat[GMT_N_GRD_FORMATS];	/* Type and description of grid format */
	int (*readinfo[GMT_N_GRD_FORMATS]) (struct GMT_CTRL *, struct GMT_GRID_HEADER *);	/* Pointers to grid read header functions */
	int (*updateinfo[GMT_N_GRD_FORMATS]) (struct GMT_CTRL *, struct GMT_GRID_HEADER *);	/* Pointers to grid update header functions */
	int (*writeinfo[GMT_N_GRD_FORMATS]) (struct GMT_CTRL *, struct GMT_GRID_HEADER *);	/* Pointers to grid write header functions */
	int (*readgrd[GMT_N_GRD_FORMATS]) (struct GMT_CTRL *, struct GMT_GRID_HEADER *, float *, double *, unsigned int *, unsigned int);	/* Pointers to grid read functions */
	int (*writegrd[GMT_N_GRD_FORMATS]) (struct GMT_CTRL *, struct GMT_GRID_HEADER *, float *, double *, unsigned int *, unsigned int);	/* Pointers to grid read functions */
	int (*fft1d[k_n_fft_algorithms]) (struct GMT_CTRL *, float *, unsigned int, int, unsigned int);	/* Pointers to available 1-D FFT functions (or NULL if not configured) */
	int (*fft2d[k_n_fft_algorithms]) (struct GMT_CTRL *, float *, unsigned int, unsigned int, int, unsigned int);	/* Pointers to available 2-D FFT functions (or NULL if not configured) */
	/* This part contains pointers that may point to additional memory outside this struct */
	char *DCWDIR;			/* Path to the DCW directory */
	char *GSHHGDIR;			/* Path to the GSHHG directory */
	char *SHAREDIR;			/* Path to the GMT share directory */
	char *HOMEDIR;			/* Path to the user's home directory */
	char *USERDIR;			/* Path to the user's GMT settings directory */
	char *DATADIR;			/* Path to one or more directories with data sets */
	char *TMPDIR;			/* Path to the directory directory for isolation mode */
	char *CUSTOM_LIBS;		/* Names of one or more comma-separated GMT-compatible shared libraries */
	char **user_media_name;		/* Length of array with custom media dimensions */
	struct GMT_FONTSPEC *font;		/* Array with font names and height specification */
	struct GMT_MEDIA *user_media;		/* Array with custom media dimensions */
	struct GMT_SHORTHAND *shorthand;	/* Array with info about shorthand file extension magic */
};

struct GMT_CTRL {
	/* Master structure for a GMT invokation.  All internal settings for GMT is accessed here */
	struct GMT_SESSION session;	/* Structure with all values that do not change throughout a session */
	struct GMT_INIT init;		/* Structure with all values that do not change in a GMT_func call */
	struct GMT_COMMON common;	/* Structure with all the common GMT command settings (-R -J ..) */
	struct GMT_CURRENT current;	/* Structure with all the GMT items that can change during execution, such as defaults settings (pens, colors, fonts.. ) */
	struct GMT_INTERNAL hidden;	/* Internal global variables that are not to be changed directly by users */
	struct PSL_CTRL *PSL;		/* Pointer to the PSL structure [or NULL] */
	struct GMTAPI_CTRL *parent;	/* Owner of this structure [or NULL]; gives access to the API from functions being passed *GMT only */
};

/* p_to_io_func is used as a pointer to functions such as GMT_read_d in assignments
 * and is used to declare GMT_get_io_ptr in gmt_io.c and gmt_prototypes.h */
typedef int (*p_to_io_func) (struct GMT_CTRL *, FILE *, uint64_t, double *);

/* Exit or return:  For some environments (e.g., Matlab) we do not
   wish to call the system "Exit" as it brings down Matlab as well.  In those cases
   we instead call return and let Matlab client deal with any follow-up.  This
   decision is set in GMT_Create_Session via its flags.  While exit always returns
   an integer code, the return functions may have to return other types, hence we
   let GMT_exit possibly call exit, else it does nothing.  Thus, calls to GMT_exit
   must be followed by return <type> so that we return where we said we would. */

/* If GMT is not set or no_not_exit is false then we call system exit, else we move along */
static inline void GMT_exit (struct GMT_CTRL *GMT, int code) {
	if (GMT == NULL || GMT->parent == NULL || GMT->parent->do_not_exit == false)
		exit (code);
}

#endif  /* _GMT_TYPES_H */
