/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * gmt_defaults.h contains definition of the structure with default settings.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	6 API
 */

/*!
 * \file gmt_defaults.h
 * \brief Definition of the structure with default settings.
 */

#ifndef GMT_DEFAULTS_H
#define GMT_DEFAULTS_H

/*--------------------------------------------------------------------
 *			GMT DEFAULTS STRUCTURE DEFINITION
 *--------------------------------------------------------------------*/

/*! Information about a particular ellipsoid */
struct ELLIPSOID {
	/* Table taken from Snyder "Map projection - a working manual", p 12 Table 1 */
	char name[GMT_LEN64];
	int date;
	double eq_radius;
	double flattening;
};

/*! Information about a particular datum */
struct DATUM {
	char name[GMT_LEN64];	/* Datum name */
	char ellipsoid[GMT_LEN64];	/* Ellipsoid GMT ID name */
	char region[GMT_LEN256];	/* Region of use */
	double xyz[3];		/* Coordinate shifts in meter for x, y, and z */
};

struct GMT_ENCODING {
	char name[GMT_LEN64];
	int code[GMT_LEN8]; /* Codes for 7 symbols we plot but whose code depends on character set (ring, degree, colon, squote, dquote, minus, hyphen). */
};

/*! Holds all variables directly controlled by GMT Default parameters */
struct GMT_DEFAULTS {
	/* COLOR group [sorted by type to optimize storage] */
	unsigned int color_model;		/* 1 = read RGB, 2 = use RGB, 4 = read HSV, 8 = use HSV, 16 = read CMYK, 32 = use CMYK [1+2]
									 * Add 128 to disallow output of color names */
	double color_patch[3][4];		/* Color of background, foreground, nan [black,white,127.5] */
	double color_hsv_min_s;			/* For smallest or most negative intensity [1.0] */
	double color_hsv_max_s;			/* For largest or most positive intensity [0.1] */
	double color_hsv_min_v;			/* For smallest or most negative intensity [0.3] */
	double color_hsv_max_v;			/* For largest or most positive intensity [1.0] */
	/* DIR group */
	/* FONT group */
	struct GMT_FONT font_annot[2];		/* Font for primary and secondary annotations [12p|14p,Helvetica,black] */
	struct GMT_FONT font_heading;		/* Font for headings above subplots [32p,Helvetica,black] */
	struct GMT_FONT font_label;		/* Font for labels [16p,Helvetica,black] */
	struct GMT_FONT font_logo;		/* Font for GMT logo [8p,Helvetica,black] */
	struct GMT_FONT font_tag;		/* Font for plot figure tags, e.g., a) [20p,Helvetica,black] */
	struct GMT_FONT font_title;		/* Font for plot titles [24p,Helvetica,black] */
	/* FORMAT group */
	char format_clock_in[GMT_LEN64];	/* How to decode an incoming clock string [hh:mm:ss] */
	char format_clock_out[GMT_LEN64];	/* Controls how clocks are written on output [hh:mm:ss] */
	char format_clock_map[GMT_LEN64];	/* Controls how clocks are plotted on maps [hh:mm:ss] */
	char format_date_in[GMT_LEN64];	/* How to decode an incoming date string [yyyy-mm-dd] */
	char format_date_out[GMT_LEN64];	/* Controls how dates are written on output [yyyy-mm-dd] */
	char format_date_map[GMT_LEN64];	/* Controls how dates are plotted on maps [yyyy-mm-dd] */
	char format_geo_out[GMT_LEN64];	/* Controls how degrees are written on output [000 = dd.xxxx] */
	char format_geo_map[GMT_LEN64];	/* Controls how degrees are plotted on maps [020 = dd:mm:ss as in old DEGREE_FORMAT = 0] */
	char format_float_out[GMT_LEN64];	/* Default double output format [%g] */
	char format_float_out_orig[GMT_LEN256];	/* User-specified format for all columns */
	char format_float_map[GMT_LEN64];	/* Default double plot format [%g] */
	char format_time[2][GMT_LEN64];	/* Controls annotation format for Months/Weeks/Weekdays for primary and secondary axes */
	char format_time_stamp[GMT_LEN256];	/* Specify the format for writing time stamps (see strftime) */
	/* GMT group */
	size_t url_size_limit;
	unsigned int compatibility; /* Choose between 4 (GMT4) and up to latest version (5 for now) */
	unsigned int auto_download;   /* 0 (GMT_NO_DOWNLOAD) or 1 (GMT_YES_DOWNLOAD): For auto-downlaod of known files */
	unsigned int interpolant; /* Choose between 0 (Linear), 1 (Akima), or 2 (Cubic spline) */
	unsigned int triangulate; /* 0 for Watson [Default], 1 for Shewchuk (if configured) */
	unsigned int verbose;     /* Level of verbosity 0-4 [1] */
	unsigned int fft;         /* Any of FFT_implementations: k_fft_auto, k_fft_accelerate, k_fft_fftw3, k_fft_kiss, k_fft_brenner */
	unsigned int fftw_plan;   /* Only accessed if HAVE_FFTW3F is defined: Any of FFTW_planner_flags: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
	unsigned int run_mode;     /* Either classic [0] or modern [1] */
	bool use_modern_name;     /* true if we should use the modern name in usage message */
	double extrapolate_val[2];/* Choose between [0] = 0, 1D extrapolated vals are NaN, = 1 -> extrapolate, = 2 -> set to const stored in [1] */
	bool fftwf_threads;   /* Only accessed if HAVE_FFTW3F_THREADS is defined: Any of FFTW_planner_flags: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
	unsigned int history;     /* mode to pass information via history file gmt.history (GMT_HISTORY_OFF, GMT_HISTORY_READ, GMT_HISTORY_WRITE) */
	unsigned int history_orig;     /* Copy of history */
	unsigned int export_type;     /* What data type to export to external APIs [GMT_DOUBLE] */
	unsigned graphics_format;	/* The default graphics format in modern mode [GMT_SESSION_FORMAT] */
	/* IO group */
	uint64_t n_bin_header_cols;		/* Minimum number of columns in a binary file for which the all cols == NaN means segment header [2] */
	unsigned int io_n_header_items;		/* Number of header records expected when -h is used [1]; else 0 */
	unsigned int io_nan_mode;		/* -s: 1 means skip NaN (x,y) records on output, 2 = inverse (only output nan-records; -sr), 0 reports all records */
	size_t io_nc4_chunksize[2];         /* NetCDF chunk size (lat,lon) on output [0] */
	unsigned int io_nc4_deflation_level;/* NetCDF deflation level on output [0] */
	unsigned int io_first_header;       /* 2 means never write single-segment header, 1 means we always write segment header, 0 means write if needed [0] */
	bool io_gridfile_shorthand;         /* Use shorthand suffix notation for embedded grid file formats [false] */
	bool io_header[2];                  /* Input & Output data has header records [false, false] */
	bool io_header_orig;                /* Copy of Input io_header */
	bool io_nan_records;                /* Determines what NaNs in input records should mean (beyond skipping the record) */
	bool io_lonlat_toggle[2];           /* true means read/write I/O as lat/lon instead of lon/lat [false,false] */
	bool io_blankline[2];               /* true means blank lines should be treated as segment breaks [false,false] */
	bool io_nanline[2];                 /* true means lines with all NaNs should be treated as segment breaks [false,false] */
	char io_col_separator[GMT_LEN8];    /* Separator between output ASCII data columns [tab] */
	char io_gridfile_format[GMT_LEN64]; /* Default grid file format */
	char io_seg_marker[2];              /* Character used to recognize and write segment headers [>,>] */
	char io_head_marker[2];             /* Character used to recognize and write header records [#,#] */
	/* MAP group */
	double map_annot_offset[2];		/* Distance between primary or secondary annotation and tickmarks [5p/5p] */
	double map_annot_min_angle;		/* If angle between map boundary and annotation is less, no annotation is drawn [20] */
	double map_annot_min_spacing;	/* If an annotation is closer that this to an older annotation, the annotation is skipped [0.0] */
	double map_frame_width;			/* Thickness of fancy map frame [5p] */
	double map_grid_cross_size[2];	/* Size of primary & secondary gridcrosses.  0 means draw continuous gridlines */
	double map_heading_offset;		/* Distance between top of panel title and base of subplot heading [18p] */
	double map_label_offset;		/* Distance between lowermost annotation and top of label [8p] */
	double map_line_step;			/* Maximum straight linesegment length for arcuate lines [0.75p] */
	double map_logo_pos[2];			/* Where to plot timestamp relative to origin [BL/-54p/-54p] */
	double map_origin[2];			/* x- and y-origin of plot, i.e. where lower left corner plots on paper [1i/1i] */
	double map_polar_cap[2];		/* Latitude of polar cap and delta_lon for gridline spacing [85/90] */
	double map_scale_height;		/* Height of map scale drawn on a map [0.075] */
	double map_tick_length[4];		/* Length of primary and secondary major and minor tickmarks [5p/2.5p/15p/3.75p] */
	double map_title_offset;		/* Distance between lowermost annotation (or label) and base of plot title [14p] */
	double map_vector_shape;		/* 0.0 = straight vectorhead, 1.0 = arrowshape, with continuous range in between */
	double map_graph_extension;		/* If mapframetype is graph, how must longer to make axis length. [7.5%] */
	unsigned int map_annot_oblique;	/* Controls annotations and tick angles etc. [GMT_OBL_ANNOT_ANYWHERE] */
	unsigned int map_logo_justify;		/* Justification of the GMT timestamp box [1 (BL)] */
	unsigned int map_frame_type;		/* Fancy (0), plain (1), or graph (2) [0] */
	unsigned int map_graph_extension_unit;	/* If mapframetype is graph, the unit is GMT_CM, GMT_INCH, GMT_PT [%] */
	bool map_logo;			/* Plot time and map projection on map [false] */
	struct GMT_PEN map_default_pen;		/* Default pen for most pens [0.25p] */
	struct GMT_PEN map_frame_pen;		/* Pen attributes for map boundary [1.25p] */
	struct GMT_PEN map_grid_pen[2];		/* Pen attributes for primary and secondary gridlines [default,black/thinner,black] */
	struct GMT_PEN map_tick_pen[2];		/* Pen attributes for primary and secondary tickmarks [thinner,black] */
	char map_frame_axes[6];			/* Which axes to draw and annotate ["WESNZ"]  */
	char map_annot_ortho[6];		/* Which axes have orthogonal annotations in linear projections ["we"] */
	enum GMT_enum_symbol { gmt_none = -1, gmt_ring, gmt_degree, gmt_colon, gmt_squote, gmt_dquote, gmt_minus, gmt_hyphen, gmt_lastsym } map_degree_symbol;
	/* PROJ group */
	double proj_scale_factor;		/* Central mapscale factor, typically 0.9996-1 (or -1 for default action) */
	unsigned int proj_ellipsoid;		/* Which ellipsoid to use [0 = GRS 80] */
	unsigned int proj_length_unit;	/* Choose 0 (cm), 1 (inch), 2 (m) or 3 (point) [1] */
	enum GMT_enum_radius proj_mean_radius;	/* Choose 0 (mean), 1 (authalic), 2 (volumetric), 3 (meridional) or 4 (quadratic) [0] */
	enum GMT_enum_latswap proj_aux_latitude;	/* Choose GMT_LATSWAP_NONE, GMT_LATSWAP_G2A, GMT_LATSWAP_G2C, GMT_LATSWAP_G2M, GMT_LATSWAP_G2O, GMT_LATSWAP_G2P [GMT_LATSWAP_NONE] */
	enum GMT_enum_geodesic proj_geodesic;	/* Choose GMT_GEODESIC_EXACT, GMT_GEODESIC_APPROXIMATE, GMT_GEODESIC_VINCENTY, GMT_GEODESIC_RUDOE, GMT_GEODESIC_ANDOYER [GMT_GEODESIC_EXACT] */
	struct DATUM proj_datum[GMT_N_DATUMS];	/* Datum parameters */
	struct ELLIPSOID ref_ellipsoid[GMT_N_ELLIPSOIDS];	/* Ellipsoid parameters */
	/* PS group [These are arguments to pass to PSL_beginsession and PSL_setdefaults] */
	/* [All other internal PSL settings are set directly when parsing PSL settings ] */
	double ps_page_size[2];			/* Width and height of paper to plot on in points [Letter or A4] */
	double ps_page_rgb[4];			/* Default paper color [white] */
	double ps_magnify[2];			/* Width and height of paper to plot on in points [Letter or A4] */
	double ps_transparency;			/* Later transparency [0] */
	double ps_penwidth;			/* Current pen width */
	unsigned int ps_color_mode;		/* Postscript encoding of color [PSL_RGB | PSL_CMYK | PSL_HSV | PSL_GRAY] */
	unsigned int ps_copies;		/* How man copies of each plot [>=1] [GMT4 COMPATIBILITY ONLY] */
	int ps_media;			/* Default paper media [25(Letter)]; negative if custom size */
	bool ps_orientation;			/* Orientation of page [false = Landscape, true = Portrait] */
	bool ps_comments;			/* true if we write comments in the PS file */
	char ps_transpmode[GMT_LEN16];		/* Transparency mode for PDF only */
	char ps_convert[GMT_LEN256];		/* Arguments for implicit psconvert calls under modern mode [""] */
	struct GMT_ENCODING ps_encoding;
	/* TIME group */
	enum GMT_enum_tictoc timer_mode;	/* Choose GMT_NO_TIMER, GMT_ABS_TIMER, GMT_ELAPSED_TIMER [GMT_NO_TIMER] */
	double time_interval_fraction;		/* How much of a partial interval is needed in order to annotate it */
	bool time_is_interval;		/* Does a time given as a month (or year or day) mean the middle of the interval? */
	bool time_leap_seconds;		/* Do we need to worry about leap seconds? */
	unsigned int time_week_start;		/* Which day (Sun = 0, Sat = 7) is start of week */
	unsigned int time_Y2K_offset_year;	/* Cutoff for making 4-digit years from 2-digit years (1900 vs 2000) */
	struct GMT_TIME_SYSTEM time_system;	/* All the information about the selected time system */
	char language[GMT_LEN64];	/* Language file for localization support */
	/* Related parameters */
	char given_unit[GMT_N_KEYS];		/* Unit given or implied for each setting */
};

#endif  /* GMT_DEFAULTS_H */
