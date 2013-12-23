/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#ifndef _GMT_ENUMS_H
#define _GMT_ENUMS_H

/* Enums related to detecting data gaps which should be treated as segment boundaries */
enum GMT_enum_gaps {GMT_NEGGAP_IN_COL = 0,	/* Check if previous minus current column value exceeds <gap> */
	GMT_POSGAP_IN_COL,			/* Check if current minus previous column value exceeds <gap> */
	GMT_ABSGAP_IN_COL,			/* Check if |current minus previous column value| exceeds <gap> */
	GMT_NEGGAP_IN_MAP_COL,			/* Check if previous minus current column value exceeds <gap> after map projection */
	GMT_POSGAP_IN_MAP_COL,			/* Check if current minus previous column value exceeds <gap> after map projection */
	GMT_ABSGAP_IN_MAP_COL,			/* Check if |current minus previous column value| exceeds <gap> after map projection */
	GMT_GAP_IN_GDIST,			/* Check if great-circle distance between successive points exceeds <gap> (in km,m,nm, etc)*/
	GMT_GAP_IN_CDIST,			/* Check if Cartesian distance between successive points exceeds <gap> */
	GMT_GAP_IN_PDIST,			/* Check if Cartesian distance between successive points exceeds <gap> after map projection */
	GMT_GAP_IN_DDIST,			/* Check if great-circle distance between successive points exceeds <gap> (in arc degrees,min,sec) */
	GMT_N_GAP_METHODS};

/* Various allocation-length parameters */
enum GMT_enum_length {
	GMT_TINY_CHUNK  = 8U,
	GMT_SMALL_CHUNK = 64U,
	GMT_CHUNK       = 2048U,
	GMT_BIG_CHUNK   = 65536U,
	GMT_LEN16	= 16U,		/* All strings used to format date/clock output must be this length */
	GMT_LEN32  = 32U,          /* Small length of texts */
	GMT_LEN64  = 64U,          /* Intermediate length of texts */
	GMT_LEN128 = 128U,         /* Double of 64 */
	GMT_LEN256 = 256U,         /* Max size of some text items */
	GMT_MAX_COLUMNS = 4096U,        /* Limit on number of columns in data tables (not grids) */
	GMT_BUFSIZ      = 4096U,        /* Size of char record for i/o */
	GMT_MIN_MEMINC  = 1U,        /* E.g., 16 kb of 8-byte doubles */
	GMT_MAX_MEMINC  = 67108864U};   /* E.g., 512 Mb of 8-byte doubles */

/* The four plot length units [m just used internally] */
enum GMT_enum_unit {
	GMT_CM = 0,
	GMT_INCH,
	GMT_M,
	GMT_PT};

/* Handling of swap/no swap in i/o */
enum GMT_swap_direction {
	k_swap_none = 0,
	k_swap_in,
	k_swap_out};

/* Various options for FFT calculations [Default is 0] */
enum FFT_implementations {
	k_fft_auto = 0,    /* Automatically select best FFT algorithm */
	k_fft_accelerate,  /* Select Accelerate Framework vDSP FFT [OS X only] */
	k_fft_fftw,        /* Select FFTW */
	k_fft_kiss,        /* Select Kiss FFT (always available) */
	k_fft_brenner,     /* Select Brenner FFT (Legacy*/
	k_n_fft_algorithms /* Number of FFT implementations available in GMT */
};

/* Various algorithms for triangulations */
enum GMT_enum_tri {
	GMT_TRIANGLE_WATSON = 0, /* Select Watson's algorithm */
	GMT_TRIANGLE_SHEWCHUK};  /* Select Shewchuk's algorithm */

/* Various 1-D interpolation modes */
enum GMT_enum_spline {
	GMT_SPLINE_LINEAR = 0, /* Linear spline */
	GMT_SPLINE_AKIMA,      /* Akima spline */
	GMT_SPLINE_CUBIC,      /* Cubic spline */
	GMT_SPLINE_NONE};      /* No spline set */

enum GMT_enum_extrap {
	GMT_EXTRAPOLATE_NONE = 0,   /* No extrapolation; set to NaN outside bounds */
	GMT_EXTRAPOLATE_SPLINE,     /* Let spline extrapolate beyond bounds */
	GMT_EXTRAPOLATE_CONSTANT};  /* Set extrapolation beyond bound to specifiec constant */

/* Various line/grid/image interpolation modes */
enum GMT_enum_track {
	GMT_TRACK_FILL = 0,	/* Normal fix_up_path behavior: Keep all (x,y) points but add intermediate if gap > cutoff */
	GMT_TRACK_FILL_M,	/* Fill in, but navigate via meridians (y), then parallels (x) */
	GMT_TRACK_FILL_P,	/* Fill in, but navigate via parallels (x), then meridians (y) */
	GMT_TRACK_SAMPLE_FIX,	/* Resample the track at equidistant points; old points may be lost. Use given spacing */
	GMT_TRACK_SAMPLE_ADJ};	/* Resample the track at equidistant points; old points may be lost. Adjust spacing to fit length of track exactly */

enum GMT_enum_bcr {
	BCR_NEARNEIGHBOR = 0, /* Nearest neighbor algorithm */
	BCR_BILINEAR,         /* Bilinear spline */
	BCR_BSPLINE,          /* B-spline */
	BCR_BICUBIC};         /* Bicubic spline */

/* Various grid/image boundary conditions */
enum GMT_enum_bc {
	GMT_BC_IS_NOTSET = 0, /* BC not yet set */
	GMT_BC_IS_NATURAL,    /* Use natural BC */
	GMT_BC_IS_PERIODIC,   /* Use periodic BC */
	GMT_BC_IS_GEO,        /* Geographic BC condition */
	GMT_BC_IS_DATA};      /* Fill in BC with actual data */

enum GMT_enum_radius {	/* Various "average" radii for an ellipsoid with axes a,a,b */
	GMT_RADIUS_MEAN = 0,	/* Mean radius IUGG R_1 = (2*a+b)/3 = a (1 - f/3) */
	GMT_RADIUS_AUTHALIC,	/* Authalic radius 4*pi*r^2 = surface area of ellipsoid, R_2 = sqrt (0.5a^2 + 0.5b^2 (tanh^-1 e)/e) */
	GMT_RADIUS_VOLUMETRIC,	/* Volumetric radius 3/4*pi*r^3 = volume of ellipsoid, R_3 = (a*a*b)^(1/3) */
	GMT_RADIUS_MERIDIONAL,	/* Meridional radius, M_r = [(a^3/2 + b^3/2)/2]^2/3 */
	GMT_RADIUS_QUADRATIC};	/* Quadratic radius, Q_r = 1/2 sqrt (3a^2 + b^2) */

enum GMT_enum_latswap {GMT_LATSWAP_NONE = -1,	/* Deactivate latswapping */
	GMT_LATSWAP_G2A = 0,	/* input = geodetic;   output = authalic   */
	GMT_LATSWAP_A2G,	/* input = authalic;   output = geodetic   */
	GMT_LATSWAP_G2C,	/* input = geodetic;   output = conformal  */
	GMT_LATSWAP_C2G,	/* input = conformal;  output = geodetic   */
	GMT_LATSWAP_G2M,	/* input = geodetic;   output = meridional */
	GMT_LATSWAP_M2G,	/* input = meridional; output = geodetic   */
	GMT_LATSWAP_G2O,	/* input = geodetic;   output = geocentric */
	GMT_LATSWAP_O2G,	/* input = geocentric; output = geodetic   */
	GMT_LATSWAP_G2P,	/* input = geodetic;   output = parametric */
	GMT_LATSWAP_P2G,	/* input = parametric; output = geodetic   */
	GMT_LATSWAP_O2P,	/* input = geocentric; output = parametric */
	GMT_LATSWAP_P2O,	/* input = parametric; output = geocentric */
	GMT_LATSWAP_N};		/* number of defined swaps  */

/* Various settings for contour label placements at crossing lines */
enum GMT_enum_contline {
	GMT_CONTOUR_NONE = 0,	/* No contour/line crossing  */
	GMT_CONTOUR_XLINE,	/* Contour labels where crossing straight lines (via key points) */
	GMT_CONTOUR_XCURVE};	/* Contour labels where crossing arbitrary lines (via file) */

/* Grid i/o error codes */
enum Gmt_error_code {
	GMT_NOERROR_UNUSED=0,	/* The real GMT_NOERROR is declared in gmt_resources.h and is part of API */
	GMT_GRDIO_NONUNIQUE_FORMAT,
	GMT_GRDIO_UNKNOWN_FORMAT,
	GMT_GRDIO_UNKNOWN_TYPE,
	GMT_GRDIO_UNKNOWN_ID,
	GMT_GRDIO_PIPE_CODECHECK,
	GMT_GRDIO_DOMAIN_VIOLATION,
	GMT_GRDIO_OPEN_FAILED,
	GMT_GRDIO_CREATE_FAILED,
	GMT_GRDIO_READ_FAILED,
	GMT_GRDIO_WRITE_FAILED,
	GMT_GRDIO_STAT_FAILED,
	GMT_GRDIO_SEEK_FAILED,
	GMT_GRDIO_FILE_NOT_FOUND,
	GMT_GRDIO_BAD_VAL,
	GMT_GRDIO_BAD_XINC,
	GMT_GRDIO_BAD_XRANGE,
	GMT_GRDIO_BAD_YINC,
	GMT_GRDIO_BAD_YRANGE,
	GMT_GRDIO_BAD_IMG_LAT,
	GMT_GRDIO_NO_2DVAR,
	GMT_GRDIO_NO_VAR,
	GMT_GRDIO_BAD_DIM,
	GMT_GRDIO_NC_NO_PIPE,
	GMT_GRDIO_NOT_RAS,
	GMT_GRDIO_NOT_8BIT_RAS,
	GMT_GRDIO_NOT_SURFER,
	GMT_GRDIO_SURF7_UNSUPPORTED,
	GMT_GRDIO_GRD98_XINC,
	GMT_GRDIO_GRD98_YINC,
	GMT_GRDIO_GRD98_BADMAGIC,
	GMT_GRDIO_GRD98_BADLENGTH,
	GMT_GRDIO_ESRI_NONSQUARE,
	GMT_GRDIO_RI_OLDBAD,
	GMT_GRDIO_RI_NEWBAD,
	GMT_GRDIO_RI_NOREPEAT,
	GMT_IO_BAD_PLOT_DEGREE_FORMAT,
	GMT_CHEBYSHEV_NEG_ORDER,
	GMT_CHEBYSHEV_BAD_DOMAIN,
	GMT_MAP_EXCEEDS_360,
	GMT_MAP_BAD_ELEVATION_MIN,
	GMT_MAP_BAD_ELEVATION_MAX,
	GMT_MAP_BAD_LAT_MIN,
	GMT_MAP_BAD_LAT_MAX,
	GMT_MAP_NO_REGION,
	GMT_MAP_NO_PROJECTION,
	GMT_MAP_BAD_DIST_FLAG,
	GMT_MAP_BAD_MEASURE_UNIT
};

/* Enums for gmt_fft use */
enum GMT_FFT_EXTEND {
	GMT_FFT_EXTEND_POINT_SYMMETRY = 0,
	GMT_FFT_EXTEND_MIRROR_SYMMETRY,
	GMT_FFT_EXTEND_NONE,
	GMT_FFT_EXTEND_NOT_SET};
	
enum GMT_FFT_TREND {
	GMT_FFT_REMOVE_NOT_SET = -1,
	GMT_FFT_REMOVE_NOTHING = 0,
	GMT_FFT_REMOVE_MEAN,
	GMT_FFT_REMOVE_MID,
	GMT_FFT_REMOVE_TREND};
	
enum GMT_FFT_KCHOICE {
	GMT_FFT_K_IS_KX = 0,
	GMT_FFT_K_IS_KY,
	GMT_FFT_K_IS_KR,
	};
	
enum GMT_FFT_DIMSET {
	GMT_FFT_EXTEND = 0,
	GMT_FFT_FORCE,
	GMT_FFT_SET,
	GMT_FFT_LIST,
	GMT_FFT_QUERY};

/* Enums for grid use */
enum GMT_enum_grdtype {
	/* Special cases of geographic grids with periodicity */
	GMT_GRID_CARTESIAN=0,			/* Cartesian data, no periodicity involved */
	GMT_GRID_GEOGRAPHIC_LESS360,		/* x is longitude, but range is < 360 degrees */
	GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT,	/* x is longitude, range is 360 degrees, no repeat node */
	GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT,	/* x is longitude, range is 360 degrees, gridline registered and repeat node at 360*/
	GMT_GRID_GEOGRAPHIC_MORE360		/* x is longitude, and range exceeds 360 degrees */
};

enum GMT_enum_grdlayout {	/* Grid layout for complex grids */
	GMT_GRID_IS_SERIAL = 0,		/* Grid is RRRRRR...[IIIIII...] */
	GMT_GRID_IS_INTERLEAVED = 1};	/* Grid is RIRIRIRI... - required layout for FFT */

/* The array wesn in the header has a name that indicates the order (west, east, south, north).
 * However, to avoid using confusing indices 0-5 we define very brief constants
 * XLO, XHI, YLO, YHI, ZLO, ZHI that should be used instead: */
enum GMT_enum_wesnIDs {
	XLO = 0, /* Index for west or xmin value */
	XHI,     /* Index for east or xmax value */
	YLO,     /* Index for south or ymin value */
	YHI,     /* Index for north or ymax value */
	ZLO,     /* Index for zmin value */
	ZHI      /* Index for zmax value */
};

enum GMT_enum_img {
	GMT_IMG_NLON_1M    = 21600U, /* At 1 min resolution */
	GMT_IMG_NLON_2M    = 10800U, /* At 2 min resolution */
	GMT_IMG_NLAT_1M_72 = 12672U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_72 = 6336U,  /* At 2 min resolution */
	GMT_IMG_NLAT_1M_80 = 17280U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_80 = 8640U,  /* At 2 min resolution */
	GMT_IMG_NLAT_1M_85 = 21600U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_85 = 10800U, /* At 2 min resolution */
	GMT_IMG_ITEMSIZE   = 2U      /* Size of 2 byte short ints */
};

/* Special grid format IDs */

enum Gmt_grid_id {
	/* DO NOT change the order because id values have grown historically.
	 * Append newly introduced id's at the end. */
	k_grd_unknown_fmt = 0, /* if grid format cannot be auto-detected */
	GMT_GRID_IS_BF,         /* GMT native, C-binary format (32-bit float) */
	GMT_GRID_IS_BS,         /* GMT native, C-binary format (16-bit integer) */
	GMT_GRID_IS_RB,         /* SUN rasterfile format (8-bit standard) */
	GMT_GRID_IS_BB,         /* GMT native, C-binary format (8-bit integer) */
	GMT_GRID_IS_BM,         /* GMT native, C-binary format (bit-mask) */
	GMT_GRID_IS_SF,         /* Golden Software Surfer format 6 (32-bit float) */
	GMT_GRID_IS_CB,         /* GMT netCDF format (8-bit integer) */
	GMT_GRID_IS_CS,         /* GMT netCDF format (16-bit integer) */
	GMT_GRID_IS_CI,         /* GMT netCDF format (32-bit integer) */
	GMT_GRID_IS_CF,         /* GMT netCDF format (32-bit float) */
	GMT_GRID_IS_CD,         /* GMT netCDF format (64-bit float) */
	GMT_GRID_IS_RF,         /* GEODAS grid format GRD98 (NGDC) */
	GMT_GRID_IS_BI,         /* GMT native, C-binary format (32-bit integer) */
	GMT_GRID_IS_BD,         /* GMT native, C-binary format (64-bit float) */
	GMT_GRID_IS_NB,         /* GMT netCDF format (8-bit integer) */
	GMT_GRID_IS_NS,         /* GMT netCDF format (16-bit integer) */
	GMT_GRID_IS_NI,         /* GMT netCDF format (32-bit integer) */
	GMT_GRID_IS_NF,         /* GMT netCDF format (32-bit float) */
	GMT_GRID_IS_ND,         /* GMT netCDF format (64-bit float) */
	GMT_GRID_IS_SD,         /* Golden Software Surfer format 7 (64-bit float, read-only) */
	GMT_GRID_IS_AF,         /* Atlantic Geoscience Center format AGC (32-bit float) */
	GMT_GRID_IS_GD,         /* Import through GDAL */
	GMT_GRID_IS_EI,         /* ESRI Arc/Info ASCII Grid Interchange format (ASCII integer) */
	GMT_GRID_IS_EF          /* ESRI Arc/Info ASCII Grid Interchange format (ASCII float, write-only) */
};

enum GMT_enum_cplx {GMT_RE = 0, GMT_IM = 1};	/* Real and imaginary indices */

/* gmt_io.c enums */
/* Three different i/o status: unused, actively using, or used */
enum GMT_enum_status {
	GMT_IS_UNUSED = 0,	/* We have not yet read from/written to this resource */
	GMT_IS_USING,		/* Means we have started reading from/writing to this file */
	GMT_IS_USED};		/* Means we are done reading from/writing to this file */

/* THere are three GMT/OGR status values */
enum GMT_ogr_status {
	GMT_OGR_UNKNOWN = -1,	/* We have not parsed enough records to know yet */
	GMT_OGR_FALSE,		/* This is NOT a GMT/OGR file */
	GMT_OGR_TRUE};		/* This is a GMT/OGR file */

/* Specific feature geometries as obtained from OGR */
/* Note: As far as registering or reading data, GMT only needs to know if data type is POINT, LINE, or POLY */

enum GMT_enum_ogr {
	GMT_IS_LINESTRING = 2,
	GMT_IS_POLYGON,
	GMT_IS_MULTIPOINT,
	GMT_IS_MULTILINESTRING,
	GMT_IS_MULTIPOLYGON};

/* Codes for aspatial assocation with segment header options: */

enum GMT_enum_segopt {
	GMT_IS_D = -1,	/* -D */
	GMT_IS_G = -2,			/* -G */
	GMT_IS_I = -3,			/* -I */
	GMT_IS_L = -4,			/* -L */
	GMT_IS_T = -5,			/* -T */
	GMT_IS_W = -6,			/* -W */
	GMT_IS_Z = -7};			/* -Z */

/* Types of possible column entries in a file: */

enum GMT_col_enum {
	GMT_IS_NAN   =   0,	/* Returned by GMT_scanf routines when read fails */
	GMT_IS_FLOAT		=   1,	/* Generic (double) data type, no special format */
	GMT_IS_LAT		=   2,
	GMT_IS_LON		=   4,
	GMT_IS_GEO		=   6,	/* data type is either Lat or Lon */
	GMT_IS_RELTIME		=   8,	/* For I/O of data in user units */
	GMT_IS_ABSTIME		=  16,	/* For I/O of data in calendar+clock units */
	GMT_IS_RATIME		=  24,	/* To see if time is either Relative or Absolute */
	GMT_IS_ARGTIME		=  32,	/* To invoke GMT_scanf_argtime()  */
	GMT_IS_DIMENSION	=  64,	/* A float with [optional] unit suffix, e.g., 7.5c, 0.4i; convert to inch  */
	GMT_IS_GEOANGLE		= 128,	/* An angle to be converted via map projection to angle on map  */
	GMT_IS_STRING		= 256,	/* An text argument [internally used, not via -f]  */
	GMT_IS_UNKNOWN		= 512};	/* Input type is not knowable without -f */

/* Various ways to report longitudes */

enum GMT_lon_enum {
	GMT_IS_GIVEN_RANGE 			= 0,	/* Report lon as is */
	GMT_IS_0_TO_P360_RANGE			= 1,	/* Report 0 <= lon <= 360 */
	GMT_IS_0_TO_P360			= 2,	/* Report 0 <= lon < 360 */
	GMT_IS_M360_TO_0_RANGE			= 3,	/* Report -360 <= lon <= 0 */
	GMT_IS_M360_TO_0			= 4,	/* Report -360 < lon <= 0 */
	GMT_IS_M180_TO_P180_RANGE		= 5,	/* Report -180 <= lon <= +180 */
	GMT_IS_M180_TO_P180			= 6,	/* Report -180 <= lon < +180 */
	GMT_IS_M180_TO_P270_RANGE		= 7};	/* Report -180 <= lon < +270 [GSHHS only] */

/* How to handle NaNs in records */

enum GMT_io_nan_enum {
	GMT_IO_NAN_OK = 0,	/* NaNs are fine; just ouput the record as is */
	GMT_IO_NAN_SKIP,	/* -s[cols]	: Skip records with z == NaN in selected cols [z-col only] */
	GMT_IO_NAN_KEEP,	/* -sr		: Skip records with z != NaN */
	GMT_IO_NAN_ONE};	/* -sa		: Skip records with at least one NaN */

/* Byteswap widths used with gmt_byteswap_file */
typedef enum {
	Int16len = 2,
	Int32len = 4,
	Int64len = 8
} SwapWidth;

/* gmt_map.c enums */
enum GMT_enum_coord {GMT_GEOGRAPHIC = 0,	/* Means coordinates are lon,lat : compute spherical distances */
	GMT_CARTESIAN,	/* Means coordinates are Cartesian x,y : compute Cartesian distances */
	GMT_GEO2CART,	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */
	GMT_CART2GEO};	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */

enum GMT_enum_dist {GMT_MAP_DIST = 0,	/* Distance in the map */
	GMT_CONT_DIST,		/* Distance along a contour or line in dist units */
	GMT_LABEL_DIST};	/* Distance along a contour or line in dist label units */

enum GMT_enum_path {GMT_RESAMPLE_PATH = 0,	/* Default: Resample geographic paths based in a max gap allowed (path_step) */
	GMT_LEAVE_PATH};	/* Options like -A can turn of this resampling, where available */

enum GMT_enum_cdist {GMT_CARTESIAN_DIST	 = 0,	/* Cartesian 2-D x,y data, r = hypot */
	GMT_CARTESIAN_DIST2,		/* Cartesian 2-D x,y data, return r^2 to avoid hypot */
	GMT_CARTESIAN_DIST_PROJ,	/* Project lon,lat to Cartesian 2-D x,y data, then get distance */
	GMT_CARTESIAN_DIST_PROJ2,	/* Same as --"-- but return r^2 to avoid hypot */
	GMT_CARTESIAN_DIST_PROJ_INV};	/* Project Cartesian 2-D x,y data to lon,lat, then get distance */
enum GMT_enum_mdist {GMT_FLATEARTH = 1,	/* Compute Flat Earth distances */
	GMT_GREATCIRCLE,	/* Compute great circle distances */
	GMT_GEODESIC,		/* Compute geodesic distances */
	GMT_LOXODROME};		/* Compute loxodrome distances (otherwise same as great circle machinery) */
enum GMT_enum_sph {GMT_DIST_M = 10,	/* 2-D lon, lat data, convert distance to meter */
	GMT_DIST_DEG = 20,	/* 2-D lon, lat data, convert distance to spherical degree */
	GMT_DIST_COS = 30};	/* 2-D lon, lat data, convert distance to cos of spherical degree */

/* gmt_memory.c enums */
enum GMT_enum_mem_alloc {	/* Initial memory for 2 double columns is 32 Mb */
	GMT_INITIAL_MEM_COL_ALLOC	= 2U,
	GMT_INITIAL_MEM_ROW_ALLOC	= 2097152U	/* 2^21 */	
};

/* gmt_plot.c enums */
/* FRONT symbols */

enum GMT_enum_front {GMT_FRONT_FAULT = 0,
	GMT_FRONT_TRIANGLE,
	GMT_FRONT_SLIP,
	GMT_FRONT_CIRCLE,
	GMT_FRONT_BOX};

/* Direction of FRONT symbols: */

enum GMT_enum_frontdir {GMT_FRONT_RIGHT = -1,
	GMT_FRONT_CENTERED,
	GMT_FRONT_LEFT};

/* Note: If changes are made to GMT_enum_vecattr you must also change pslib.h: PSL_enum_vecattr */
enum GMT_enum_vecattr {GMT_VEC_LEFT = 1,	/* Only draw left half of vector head */
	GMT_VEC_RIGHT		= 2,		/* Only draw right half of vector head */
	GMT_VEC_BEGIN		= 4,		/* Place vector head at beginning of vector */
	GMT_VEC_END		= 8,		/* Place vector head at end of vector */
	GMT_VEC_HEADS		= 12,		/* Mask for either head end */
	GMT_VEC_JUST_B		= 0,		/* Align vector beginning at (x,y) */
	GMT_VEC_JUST_C		= 16,		/* Align vector center at (x,y) */
	GMT_VEC_JUST_E		= 32,		/* Align vector end at (x,y) */
	GMT_VEC_JUST_S		= 64,		/* Align vector center at (x,y) */
	GMT_VEC_ANGLES		= 128,		/* Got start/stop angles instead of az, length */
	GMT_VEC_POLE		= 256,		/* Got pole of small/great circle */
	GMT_VEC_OUTLINE		= 512,		/* Draw vector head outline using default pen */
	GMT_VEC_OUTLINE2	= 1024,		/* Draw vector head outline using supplied v_pen */
	GMT_VEC_FILL		= 2048,		/* Fill vector head using default fill */
	GMT_VEC_FILL2		= 4096,		/* Fill vector head using supplied v_fill) */
	GMT_VEC_MARC90		= 8192,		/* Matharc only: if angles subtend 90, draw straight angle symbol */
	GMT_VEC_SCALE		= 32768};	/* Not needed in pslib: If not set we determine the required inch-to-degree scale */

/* gmt_proj.c enums */
enum GMT_enum_annot {GMT_LINEAR = 0,
	GMT_LOG10,	/* These numbers are only used for GMT->current.proj.xyz_projection[3], */
	GMT_POW,	/* while GMT->current.proj.projection = 0 */
	GMT_TIME,
	GMT_ANNOT_CPT,
	GMT_CUSTOM};

enum GMT_enum_cyl {GMT_MERCATOR = 100,
	GMT_CYL_EQ,
	GMT_CYL_EQDIST,
	GMT_CYL_STEREO,
	GMT_MILLER,
	GMT_TM,
	GMT_UTM,
	GMT_CASSINI,
	GMT_OBLIQUE_MERC = 150,
	GMT_OBLIQUE_MERC_POLE};

enum GMT_enum_conic {GMT_ALBERS = 200,
	GMT_ECONIC,
	GMT_POLYCONIC,
	GMT_LAMBERT = 250};

enum GMT_enum_azim {GMT_STEREO = 300,
	GMT_LAMB_AZ_EQ,
	GMT_ORTHO,
	GMT_AZ_EQDIST,
	GMT_GNOMONIC,
	GMT_GENPER,
	GMT_POLAR = 350};

enum GMT_enum_misc {GMT_MOLLWEIDE = 400,
	GMT_HAMMER,
	GMT_SINUSOIDAL,
	GMT_VANGRINTEN,
	GMT_ROBINSON,
	GMT_ECKERT4,
	GMT_ECKERT6,
	GMT_WINKEL};

/* The various GMT measurement units */
enum GMT_enum_units {GMT_IS_METER = 0,
	GMT_IS_KM,
	GMT_IS_MILE,
	GMT_IS_NAUTICAL_MILE,
	GMT_IS_INCH,
	GMT_IS_CM,
	GMT_IS_PT,
	GMT_IS_FOOT,
	GMT_IS_SURVEY_FOOT,
	GMT_N_UNITS,
	GMT_IS_NOUNIT = -1};

enum GMT_enum_frame {GMT_IS_PLAIN = 0,	/* Plain baseframe */
	GMT_IS_INSIDE	= 1,	/* Plain frame ticks/annotations on the inside of boundary */
	GMT_IS_GRAPH	= 2,	/* Plain fram with arrow extensions on axes */
	GMT_IS_FANCY	= 4,	/* Fancy baseframe */
	GMT_IS_ROUNDED	= 12};	/* Fancy baseframe, rounded */

/* Define the 6 axis items that each axis can have (some are mutually exclusive: only one ANNOT/INTV for upper and lower) */

enum GMT_enum_tick {GMT_ANNOT_UPPER = 0,	/* Tick annotations closest to the axis */
	GMT_ANNOT_LOWER,	/* Tick annotations farthest from the axis*/
	GMT_TICK_UPPER,		/* Frame tick marks closest to the axis */
	GMT_TICK_LOWER,		/* Frame tick marks closest to the axis */
	GMT_GRID_UPPER,		/* Gridline spacing */
	GMT_GRID_LOWER};	/* Gridline spacing */

/* Return codes from GMT_inonout */
enum GMT_enum_inside {
	GMT_OUTSIDE = 0,
	GMT_ONEDGE,
	GMT_INSIDE};


#endif /* _GMT_ENUMS_H */
