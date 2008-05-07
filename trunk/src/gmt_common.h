/*--------------------------------------------------------------------
 *	$Id: gmt_common.h,v 1.9 2008-05-07 07:35:08 guru Exp $
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
 
/*
 * Holds current selections for the family of common GMT options.
 *
 * Author: Paul Wessel
 * Date:	16-APR-2006
 * Version:	5
 */
 
#ifndef _GMT_COMMON_H
#define _GMT_COMMON_H

struct GMT_COMMON {
	/* Structure with all information given via the 16 common GMT command-line options -R -J ..
	 * THe users's command line choices will affect the settings of these options.  For the
	 * stand-along GMT applications they are only set once and then the program runs and exits.
	 * For use via the API, when several GMT layers may be done by the same applications, we
	 * must deactive options that normally should only apply once.  For each option below we
	 * describe what will happen during a multi-layer execution. */
	
	struct synopsis {	/* [0]	-\0 (zero) */
		/* Since the calling program will exit/return when this option is selected
		 * we do not need to do anything special at the end. */
		BOOLEAN active;		/* TRUE if we should just display the synopsis and exit */
	} synopsis;
	
	struct B {	/* [1]  -B<params> */
		/* This option will be deactivated after first use.  The arg string will remain unchanged
		 * and can only be changed with another -B option (which again reactivates the option) */
		BOOLEAN active;		/* TRUE if the -B option has been set */
		char *arg;		/* Copy of the arguments given to -B */
	} B;
	
	struct H {	/* [2]  -H[i][<nrecs>] */
		/* Once set, this option remains in effect unless -H is issued again. <nrecs> = 0 turns it to FALSE. */
		BOOLEAN active;		/* TRUE if the -H option has been set */
		char *arg;		/* Copy of the arguments given to -H */
	} H;	
	
	struct J {	/* [3-4]  -J<params> */
		/* Once set, this option remains in effect unless changed by a later -J call */
		BOOLEAN active;		/* TRUE if the -J option has been set */
		char *arg;		/* Copy of the arguments given to -J */
	} J;
		
	struct K {	/* [5]  -K */
		/* Once set, this option remains in effect until the entire process ends */
		BOOLEAN active;		/* TRUE if we plan to append more PostScript later */
	} K;
	
	struct O {	/* [6]  -O */
		/* Once set, this option remains in effect until the entire process ends */
		BOOLEAN active;		/* TRUE if we are preparing a PostScript overlay */
	} O;
	
	struct P {	/* [7]  -P */
		/* Once set, this option remains in effect unless changed by a later -P call.
		 * It is only used when a new plot is initialized and not by any overlay. */
		BOOLEAN active;		/* TRUE if the current plot has Portrait orientation */
	} P;
	
	struct R {	/* [8]  -Rw/e/s/n[/z0/z1][r] */
		/* Once set, this option remains in effect unless changed by a later -R call */
		BOOLEAN active;		/* TRUE if the -R option has been set */
		char *arg;		/* Copy of the arguments given to -R */
	} R;
	
	struct U {	/* [9]  -U */
		/* This option will be deactivated after first use.  The label information will remain unchanged
		 * and can only be changed with another -U option (which again reactivates the option) */
		BOOLEAN active;		/* TRUE if the -U option has been set */
		char *arg;		/* Copy of the arguments given to -U */
	} U;
	
	struct V {	/* [10]  -V */
		/* Once set, this option remains in effect unless changed by a later -V call */
		BOOLEAN active;		/* TRUE if the -V option has been set */
		char *arg;		/* Copy of the arguments given to -V */
	} V;
	
	struct X {	/* [11]  -X */
		/* This option will be deactivated after first use and must be reissued for each intended x-shift */
		BOOLEAN active;		/* TRUE if the -X option has been set */
		char *arg;		/* Copy of the arguments given to -X */
	} X;
	
	struct Y {	/* [12] -Y */
		/* This option will be deactivated after first use and must be reissued for each intended y-shift */
		BOOLEAN active;		/* TRUE if the -Y option has been set */
		char *arg;		/* Copy of the arguments given to -Y */
	} Y;
	
	struct c {	/* [13]  -c */
		/* Once set, this option remains in effect unless changed by a later -c call.
		 * It is only used when a new plot is initialized and not by any overlay. */
		BOOLEAN active;		/* TRUE if the -c option has been set */
		char *arg;		/* Copy of the arguments given to -c */
	} c;
	
	struct t {	/* [14]  -:[i|o] */
		/* Once set, this option remains in effect unless -: is issued again. */
		BOOLEAN active;		/* TRUE if the -: option has been set */
		char *arg;		/* Copy of the arguments given to -: */
	} t;
	
	struct b {	/* -b[i|o][<n>][s|S|d|D] */
		/* Once set, this option remains in effect unless -b is issued again. <n> = 0 turns binary to FALSE */
		BOOLEAN active;		/* TRUE if the -b option has been set */
		char *arg;		/* Copy of the arguments given to -b */
	} b;
	
	struct f {	/* [15]  -f[i|o]<col>|<colrange>[t|T|g],.. */
		/* Once set, this option remains in effect unless -f is issued again. */
		BOOLEAN active;		/* TRUE if the -f option has been set */
		char *arg;		/* Copy of the arguments given to -f */
	} f;
};

struct GMT_PROJ {		/* Holds all projection-related parameters */
	struct GMT_MAP_PROJECTIONS project_info;
	struct GMT_THREE_D z_project;
	struct GMT_DATUM_CONV GMT_datum;	/* For datum conversions */
	PFI GMT_forward, GMT_inverse;		/* Pointers to the selected mapping functions */
	PFI GMT_x_forward, GMT_x_inverse;	/* Pointers to the selected linear functions */
	PFI GMT_y_forward, GMT_y_inverse;	/* Pointers to the selected linear functions */
	PFI GMT_z_forward, GMT_z_inverse;	/* Pointers to the selected linear functions */
};

struct GMT_MAP {		/* Holds all map-related parameters */
	struct GMT_PLOT_FRAME frame_info;
	int GMT_x_status_new;				/* Tells us what quadrant old and new points are in */
	int GMT_y_status_new;
	int GMT_x_status_old;
	int GMT_y_status_old;
	int GMT_corner = 0;
	BOOLEAN GMT_on_border_is_outside = FALSE;	/* TRUE if a point exactly on the map border shoud be considered outside the map */
	BOOLEAN GMT_world_map = FALSE;			/* TRUE if map has 360 degrees of longitude range */
	BOOLEAN GMT_world_map_tm = FALSE;		/* TRUE if GMT_TM map is global? */
	double GMT_map_width;				/* Full width in inches of this world map */
	double GMT_map_height;				/* Full height in inches of this world map */
	double GMT_half_map_size;			/* Half width in inches of this world map */
	double GMT_half_map_height;			/* Half height of this world map */
	PFI GMT_outside;				/* pointer to function checking if a lon/lat point is outside map */
	PFI GMT_crossing;				/* pointer to functions returning crossover point at boundary */
	PFI GMT_overlap;				/* pointer to function checking for overlap between 2 regions */
	PFI GMT_map_clip;				/* pointer to functions that clip a polygon to fit inside map */
	PFD GMT_left_edge, GMT_right_edge;		/* pointers to functions that return left/right edge of map */
	PFD GMT_distance_func;				/* pointer to function returning distance between two points points */
	BOOLEAN GMT_z_periodic = FALSE;			/* TRUE if grid values are 0-360 degrees (phases etc) */
	PFI GMT_wrap_around_check;			/* Does x or y wrap checks */
	PFI GMT_map_jump;				/* TRUE if we jump in x or y */
	PFB GMT_will_it_wrap;				/* TRUE if consecutive points indicate wrap */
	PFB GMT_this_point_wraps;			/* Used in above */
	PFV GMT_get_crossings;				/* Returns map crossings in x or y */
	PFI GMT_truncate;				/* Truncate polygons agains boundaries */
	BOOLEAN GMT_meridian_straight = FALSE;		/* TRUE if meridians plot as straight lines */
	BOOLEAN GMT_parallel_straight = FALSE;		/* TRUE if parallels plot as straight lines */
	GMT_LONG GMT_n_lon_nodes = 360;			/* Somewhat arbitrary # of nodes for lines in longitude (may be reset in gmt_map.c) */
	GMT_LONG GMT_n_lat_nodes = 180;			/* Somewhat arbitrary # of nodes for lines in latitude (may be reset in gmt_map.c) */
	double GMT_dlon = 0.0;				/* Steps taken in longitude along gridlines (gets reset in gmt_init.c) */
	double GMT_dlat = 0.0;				/* Steps taken in latitude along gridlines (gets reset in gmt_init.c) */

};

struct GMT_IO {		/* Holds all map-related parameters */
	FILE *GMT_stdin;			/* Pointer for standard input */
	FILE *GMT_stdout;			/* Pointer for standard output */
	PFI GMT_input;				/* Pointer to function reading ascii or binary tables */
	PFI GMT_input_ascii;			/* Pointer to function reading ascii tables only */
	PFI GMT_output;				/* Pointer to function writing ascii or binary tables */
	PFI GMT_output_ascii;			/* Pointer to function writing ascii tables only */
	struct GMT_IO GMT_io = {
	int GMT_pad[4] = {0, 0, 0, 0};
	int GMT_inc_code[2] = {0, 0};
	double GMT_data[BUFSIZ];
};

struct GMT_TIME {		/* Holds all time-related parameters */
	struct GMT_PLOT_CALCLOCK GMT_plot_calclock;
	struct GMT_TRUNCATE_TIME GMT_truncate_time;
	struct GMT_Y2K_FIX GMT_Y2K_fix;		/* Used to convert 2-digit years to 4-digit years */
	struct GMT_TIME_LANGUAGE GMT_time_language;	/* For time axis */
};

struct GMT_INFO {		/* Holds misc run-time parameters */
	char *GMT_program;	/* Name of current GMT program */
	int GMT_oldargc;
	char *GMT_oldargv[GMT_N_UNIQUE];	/* Pointers to old common arguments */
};

struct GMT_PLOT {		/* Holds all projection-related parameters */
	double *GMT_x_plot = 0;				/* Holds the x/y (inches) of a line to be plotted */
	double *GMT_y_plot = 0;
	int *GMT_pen = 0;				/* Pen (3 = up, 2 = down) for these points */
	GMT_LONG GMT_n_plot = 0;				/* Number of such points */
	GMT_LONG GMT_n_alloc = 0;				/* Size of allocated plot arrays */
	int GMT_3D_mode = 3;				/* Determines if we draw fore and/or back 3-D box lines [Default is both] */
	char *GMT_plot_format[3][2];			/* Keeps the 6 formats for dd:mm:ss plot output */

};

struct GMT_TEXTURE {		/* Holds all pen, color, and fill-related parameters */
	struct GMT_LUT *GMT_lut;		/* CPT lookup table read by GMT_read_cpt */
	struct GMT_BFN_COLOR GMT_bfn[3];	/* Structures with back/fore/nan colors */
	int GMT_n_colors = 0;			/* Number of colors in CPT lookup table */
	int GMT_cpt_flags = 0;			/* Flags controling use of BFN colors */
	BOOLEAN GMT_gray;			/* TRUE if only grayshades are needed */
	BOOLEAN GMT_b_and_w;			/* TRUE if only black and white are needed */
	BOOLEAN GMT_continuous;			/* TRUE if continuous color tables have been given */
	BOOLEAN GMT_cpt_pattern = FALSE;	/* TRUE if cpt file contains any patterns */
	BOOLEAN GMT_cpt_skip = FALSE;		/* TRUE if current z-slice is to be skipped */
	#ifdef GMT_CPT2	
	BOOLEAN GMT_categorical = FALSE;	/* TRUE if CPT applies to categorical data */
	#endif
};

struct GMT_INIT {
	/* These are internal parameters that need to be passed around between
	 * many GMT functions.  These values are initialized ONCE and remain
	 * unchanged during the life of the GMT session. */
	int GMT_N_FONTS;			/* Total number of fonts returned by GMT_init_fonts */
	struct GMT_FONT *font;		/* Array with font names and heights */
	struct GMT_HASH rgb_hashnode[GMT_N_COLOR_NAMES];/* Used to translate colornames to r/g/b */
	struct GMT_HASH month_hashnode[12];		/* Used to translate months to 1-12 */
	int GMT_n_file_suffix;
	PFI GMT_io_readinfo[GMT_N_GRD_FORMATS];
	PFI GMT_io_updateinfo[GMT_N_GRD_FORMATS];
	PFI GMT_io_writeinfo[GMT_N_GRD_FORMATS];
	PFI GMT_io_readgrd[GMT_N_GRD_FORMATS];
	PFI GMT_io_writegrd[GMT_N_GRD_FORMATS];
	double *GMT_file_scale, *GMT_file_offset, *GMT_file_nan;
	int *GMT_file_id;
	char **GMT_file_suffix;
	char *GMT_SHAREDIR = CNULL;
	char *GMT_HOMEDIR = CNULL;
	char *GMT_USERDIR = CNULL;
	char *GMT_DATADIR = CNULL;
	char *GMT_GRIDDIR = CNULL;
	char *GMT_IMGDIR = CNULL;
	char *GMT_TMPDIR = CNULL;
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
};
	
struct GMT_CURRENT {
	/* These are internal parameters that need to be passed around between
	 * many GMT functions.  These values may change during execution. */
	struct GMT_DEFAULTS def;	/* Holds all GMT defaults parameters */
	struct GMT_IO io;		/* Holds all i/o-related parameters */
	struct GMT_MAP map;		/* Holds all projection-related parameters */
	struct GMT_PLOT plot;		/* HOlds all plotting-related parameters */
	struct GMT_COLOR color;		/* Holds all colors, pen, and fill-related parameters */
	struct GMT_PS ps;		/* Hold parameters related to PS setup */

};

struct GMT_CTRL {
	/* Master structure for a GMT invokation.  All internal settings for GMT is accessed here */
	struct GMT_INIT *init;		/* Structure with all the GMT constants that never change once set */
	struct GMT_CURRENT *current;	/* Structure with all the GMT items that can change during execution, such as defaults settings (pens, colors, fonts.. ) */
	struct GMT_INTERNAL *hidden;	/* Internal global variables that are not to be changed directly by users */
};

/* For custom symbol plotting in psxy[z].  These should only be used by those two programs does not have to be in this structure */

int GMT_n_custom_symbols = 0;
struct GMT_CUSTOM_SYMBOL **GMT_custom_symbol;


#endif /* _GMT_COMMON_H */
