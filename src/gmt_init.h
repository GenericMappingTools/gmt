/*--------------------------------------------------------------------
 *	$Id: gmt_init.h,v 1.27 2002-02-14 23:53:58 pwessel Exp $
 *
 *	Copyright (c) 1991-2002 by P. Wessel and W. H. F. Smith
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
 *  This include file contains initializations for the GMT system
 *
 * Author:	Paul Wessel
 * Date:	21-AUG-1995
 * Revised:	25-FEB-2000
 * Version:	3.4
 *
 */

/*--------------------------------------------------------------------*/
/* Load parameters from include files */
/*--------------------------------------------------------------------*/

#include "gmt_keycases.h"		/* Get all the default case values */

struct GMTDEFAULTS gmtdefs = {		/* Initial default values */
#include "gmt_defaults.h"
};

char *GMT_unique_option[N_UNIQUE] = {	/* The common GMT commandline options */
#include "gmt_unique.h"
};
 
char *GMT_font_name[N_FONTS] = {	/* Name of fonts recognized by GMT */
#include "PS_font_names.h"
};

double GMT_font_height[N_FONTS] = {	/* Relative; based on the size of A divided by fontsize */
#include "PS_font_heights.h"
};

char *GMT_keywords[N_KEYS] = {		/* Names of all parameters in .gmtdefaults */
#include "gmt_keywords.h"
};

char *GMT_media_name[GMT_N_MEDIA] = {		/* Names of all paper formats */
#include "gmt_media_name.h"
};

struct GMT_MEDIA GMT_media[GMT_N_MEDIA] = {		/* Sizes in points of all paper formats */
#include "gmt_media_size.h"
};

struct GMT_TIME_SYSTEM GMT_time_system[GMT_N_SYSTEMS] = {	/* List of all used time systems */
#include "gmt_time_systems.h"
};

char *GMT_weekdays[7] = {	/* Days of the week in English */
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

char *GMT_degree_choice[4] = {  /* Users choice for degree symbol */
	"none",
	"ring",
	"degree",
	"colon",
};
/*--------------------------------------------------------------------*/
/*	For plotting purposes */
/*--------------------------------------------------------------------*/

struct PLOT_FRAME frame_info;
struct GMT_PLOT_CALCLOCK GMT_plot_calclock;
double *GMT_x_plot = 0;		/* Holds the x/y (inches) of a line to be plotted */
double *GMT_y_plot = 0;
int *GMT_pen = 0;		/* Pen (3 = up, 2 = down) for these points */
int GMT_n_plot = 0;			/* Number of such points */
int GMT_n_alloc = 0;		/* Size of allocated plot arrays */
int GMT_x_status_new;		/* Tells us what quadrant old and new points are in */
int GMT_y_status_new;
int GMT_x_status_old;
int GMT_y_status_old;
int GMT_corner = 0;
BOOLEAN GMT_on_border_is_outside = FALSE;	/* TRUE if a point exactly on the map border shoud be considered outside the map */
BOOLEAN GMT_world_map = FALSE;	/* TRUE if map has 360 degrees of longitude range */
BOOLEAN GMT_world_map_tm = FALSE;	/* TRUE if TM map is global? */
double GMT_map_width;		/* Full width in inches of this world map */
double GMT_map_height;		/* Full height in inches of this world map */
double GMT_half_map_size;	/* Half width in inches of this world map */
double GMT_half_map_height;	/* Half height of this world map */
PFI GMT_outside;			/*	pointer to function checking if a lon/lat point is outside map */
PFI GMT_crossing;			/*	pointer to functions returning crossover point at boundary */
PFI GMT_overlap;			/*	pointer to function checking for overlap between 2 regions */
PFI GMT_map_clip;			/*	pointer to functions that clip a polygon to fit inside map */
PFD GMT_left_edge, GMT_right_edge;	/*	pointers to functions that return left/right edge of map */
BOOLEAN GMT_z_periodic = FALSE;			/* TRUE if grid values are 0-360 degrees (phases etc) */
PFI GMT_wrap_around_check;	/* Does x or y wrap checks */
PFI GMT_map_jump;		/* TRUE if we jump in x or y */
PFB GMT_will_it_wrap;		/* TRUE if consecutive points indicate wrap */
PFB GMT_this_point_wraps;	/* Used in above */
PFV GMT_get_crossings;		/* Returns map crossings in x or y */
PFI GMT_truncate;		/* Truncate polygons agains boundaries */
BOOLEAN GMT_meridian_straight = FALSE;	/* TRUE if meridians plot as straight lines */
BOOLEAN GMT_parallel_straight = FALSE;	/* TRUE if parallels plot as straight lines */
int GMT_3D_mode = 3;			/* Determines if we draw fore and/or back 3-D box lines [Default is both] */
char *GMT_plot_format[3][2];		/* Keeps the 6 formats for dd:mm:ss plot output */

/*--------------------------------------------------------------------*/
/*	For color lookup purposes */
/*--------------------------------------------------------------------*/

struct GMT_LUT *GMT_lut;	/* CPT lookup table read by GMT_read_cpt */
struct GMT_BFN_COLOR GMT_bfn;	/* Structure with back/fore/nan colors */
int GMT_n_colors = 0;
BOOLEAN GMT_gray;		/* TRUE if only grayshades are needed */
BOOLEAN GMT_b_and_w;		/* TRUE if only black and white are needed */
BOOLEAN GMT_continuous;		/* TRUE if continuous color tables have been given */
BOOLEAN GMT_cpt_pattern = FALSE;	/* TRUE if cpt file contains any patterns */
BOOLEAN GMT_cpt_skip = FALSE;	/* TRUE if current z-slice is to be skipped */

/*--------------------------------------------------------------------*/
/*	For projection purposes */
/*--------------------------------------------------------------------*/

struct MAP_PROJECTIONS project_info;
struct THREE_D z_project;
PFI GMT_forward, GMT_inverse;		/*	Pointers to the selected mapping functions */
PFI GMT_x_forward, GMT_x_inverse;	/*	Pointers to the selected linear functions */
PFI GMT_y_forward, GMT_y_inverse;	/*	Pointers to the selected linear functions */
PFI GMT_z_forward, GMT_z_inverse;	/*	Pointers to the selected linear functions */

/*--------------------------------------------------------------------*/
/*	For i/o purposes */
/*--------------------------------------------------------------------*/

FILE *GMT_stdin;		/* Pointer for standard input */
FILE *GMT_stdout;		/* Pointer for standard output */
PFI GMT_input;			/* Pointer to function reading ascii or binary tables */
PFI GMT_input_ascii;		/* Pointer to function reading ascii tables only */
PFI GMT_output;			/* Pointer to function writing ascii or binary tables */
BOOLEAN GMT_geographic_in = FALSE;	/* TRUE if input data is long/lat*/
BOOLEAN GMT_geographic_out = FALSE;	/* TRUE if output data is long/lat*/
struct GMT_IO GMT_io = {
	FALSE,
	{ FALSE, FALSE },
	{ FALSE, FALSE },
	FALSE,
	FALSE,
	{ 0, 0 },
	0,
	0,
	0,
	0,
	'>',
	"",
	"",
	"r",
	"w",
	"a+",
	(BOOLEAN *)NULL,
	(int *)NULL,
	(int *)NULL,
	{ {-1, -1, -1, -1}, {-1, -1, -1, -1}, FALSE, FALSE, "", FALSE, FALSE, FALSE, FALSE, { "", ""} },
	{ {-1, -1, -1, -1}, {-1, -1, -1, -1}, FALSE, FALSE, "", FALSE, FALSE, FALSE, FALSE, { "", ""} },
	{ {-1, -1, -1}, 0, 0.0, FALSE, FALSE, { "am", "pm" }, "", { "", ""} },
	{ {-1, -1, -1}, 0, 0.0, FALSE, FALSE, { "am", "pm" }, "", { "", ""} },
	{ {-1, -1, -1}, 0, FALSE, FALSE, FALSE, 0, 0.0, "", "", { "", ""} }
};
struct GMT_Y2K_FIX GMT_Y2K_fix;		/* Used to convert 2-digit years to 4-digit years */
int GMT_grd_i_format = 0;		/* Default is GMT's cdf format */
int GMT_grd_o_format = 0;
int GMT_n_file_suffix;
PFI GMT_io_readinfo[N_GRD_FORMATS];
PFI GMT_io_updateinfo[N_GRD_FORMATS];
PFI GMT_io_writeinfo[N_GRD_FORMATS];
PFI GMT_io_readgrd[N_GRD_FORMATS];
PFI GMT_io_writegrd[N_GRD_FORMATS];
double GMT_grd_in_nan_value, GMT_grd_out_nan_value;
double *GMT_file_scale, *GMT_file_offset, *GMT_file_nan;
int *GMT_file_id;
char **GMT_file_suffix;
int GMT_fd_history = 0;	/* File descriptor for .gmtcommands */
#ifdef NO_LOCK
BOOLEAN GMT_lock = FALSE;
#else
BOOLEAN GMT_lock = TRUE;
#endif
double GMT_data[BUFSIZ];
int GMT_pad[4] = {0, 0, 0, 0};

/*--------------------------------------------------------------------*/
/*	For misc purposes */
/*--------------------------------------------------------------------*/

/* For Custom paper types */

char **GMT_user_media_name = (char **)NULL;
struct GMT_MEDIA *GMT_user_media = (struct GMT_MEDIA *)NULL;
int GMT_n_user_media = 0;

char *GMTHOME = CNULL;
char *GMT_program;	/* Name of current GMT program */

float GMT_f_NaN;
double GMT_d_NaN;

/* GMT_u2u is the conversion matrix for cm, inch, m, pt: */

double GMT_u2u[4][4] = {
	{   1.00,    1.0/2.54,    0.01,         72.0/2.54 },
	{   2.54,    1.0,         0.0254,       72.0 },
	{ 100.00,    100.0/2.54,  1.0,          72.0/0.0254 },
	{ 2.54/72.0, 1.0/72.0,    0.0254/72.0,  1.0 }
};
char *GMT_unit_names[4] = {"cm", "inch", "m", "point"};
int GMT_no_rgb[3] = {-1, -1, -1};

int GMT_oldargc;
char *GMT_oldargv[N_UNIQUE];	/* Pointers to old common arguments */
BOOLEAN GMT_quick = FALSE;

struct BCR bcr;			/* For interpolations on grid */

struct GMT_TIME_LANGUAGE GMT_time_language;	/* For time axis */

/* For custom symbol plotting in psxy[z] */

int GMT_n_custom_symbols = 0;
struct CUSTOM_SYMBOL **GMT_custom_symbol;
