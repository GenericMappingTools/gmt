/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2012-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_resources.h contains the definitions for the six GMT resources
 * GMT_DATASET, GMT_GRID, GMT_IMAGE, GMT_PALETTE, GMT_POSTSCRIPT, GMT_TEXTSET
 * as well as the two auxiliary resources GMT_MATRIX and GMT_VECTOR,
 * as well as all named enums.
 *
 * Author:	Paul Wessel
 * Date:	13-JUN-2016
 * Version:	5 API
 */

/*!
 * \file gmt_resources.h
 * \brief Definitions for the GMT resources (GMT_GRID, GMT_DATASET, etc...)
 */

#ifndef _GMT_RESOURCES_H
#define _GMT_RESOURCES_H

#define GMT_BACKWARDS_API	/* Try to be backwards compatible with API naming for now */

/*============================================================ */
/*=============== Constants Public Declaration =============== */
/*============================================================ */

/* Here are structure definitions and constants (enums) needed by the
 * public GMT API. Note: As this is C, some structures will contain
 * members we do not disclose in the API documentation but of course
 * the variables are accessible just like the "public" ones.  There
 * is no guarantee that the "private" members cannot change over time.
 */

/*! Session modes for GMT_Create_Session */
enum GMT_enum_session {
	GMT_SESSION_NORMAL   = 0,	/* Typical mode to GMT_Create_Session */
	GMT_SESSION_NOEXIT   = 1,	/* Call return and not exit when error */
	GMT_SESSION_EXTERNAL = 2,	/* Called by an external API (e.g., MATLAB, Python). */
	GMT_SESSION_COLMAJOR = 4,	/* External API uses column-major formats (e.g., MATLAB, FORTRAN). [Row-major format] */
	GMT_SESSION_RUNMODE  = 8};	/* If set enable GMT's modern runmode. [Classic] */

/*! Miscellaneous settings */
enum GMT_enum_api {
	GMT_USAGE	= 0,	/* Want to report full program usage message */
	GMT_SYNOPSIS	= 1,	/* Just want the synopsis of usage */
	GMT_STR16	= 16	/* Bytes needed to hold the @GMTAPI@-###### resource names */
};

/*! These data primitive identifiers are as follows: */
enum GMT_enum_type {
	GMT_CHAR = 0,  /* int8_t, 1-byte signed integer type */
	GMT_UCHAR,     /* uint8_t, 1-byte unsigned integer type */
	GMT_SHORT,     /* int16_t, 2-byte signed integer type */
	GMT_USHORT,    /* uint16_t, 2-byte unsigned integer type */
	GMT_INT,       /* int32_t, 4-byte signed integer type */
	GMT_UINT,      /* uint32_t, 4-byte unsigned integer type */
	GMT_LONG,      /* int64_t, 8-byte signed integer type */
	GMT_ULONG,     /* uint64_t, 8-byte unsigned integer type */
	GMT_FLOAT,     /* 4-byte data float type */
	GMT_DOUBLE,    /* 8-byte data float type */
	GMT_TEXT,      /* Arbitrarily long text string [OGR/GMT use only] */
	GMT_DATETIME,  /* string with date/time info [OGR/GMT use only] */
	GMT_N_TYPES};  /* The number of supported data types above */

/*! These are the 5 methods for i/o; used as arguments in the API that expects a "method" */

enum GMT_enum_method {
	GMT_IS_FILE = 0,	/* Entity is a filename */
	GMT_IS_STREAM = 1,	/* Entity is an open stream */
	GMT_IS_FDESC = 2,	/* Entity is an open file descriptor */
	GMT_IS_DUPLICATE = 3,	/* Entity is a memory location that should be duplicated */
	GMT_IS_REFERENCE = 4,	/* Entity is a memory location and we just pass the ref (no copying) */
	GMT_IS_OUTPUT = 1024};	/* When creating a resource as a container for output */

/* A Grid can come from a grid OR User Matrix, and Data can come from DATASET or via Vectors|Matrix, and Text from TEXTSET or Matrix */

enum GMT_enum_via {
	GMT_VIA_NONE = 0,	/* No via anything */
	GMT_VIA_MODULE_INPUT = 64};	/* To flag resources destined for another module's "command-line" input */

/* We may allocate just a container, just the data (if container was allocated earlier), or both: */
enum GMT_enum_container {
	GMT_CONTAINER_AND_DATA	= 0U,    /* Create|Read|write both container and the data array */
	GMT_CONTAINER_ONLY	= 1U,    /* Create|read|write the container but no data array */
	GMT_DATA_ONLY		= 2U};   /* Create|Read|write the container's array only */

/*! These are the 6 families of data types, + a coordinate array + 3 help containers for vector, matrix, and coordinates */
enum GMT_enum_family {
	GMT_IS_DATASET = 0,	/* Entity is a data table */
	GMT_IS_GRID,		/* Entity is a grid */
	GMT_IS_IMAGE,		/* Entity is a 1- or 3-layer unsigned char image */
	GMT_IS_PALETTE,		/* Entity is a color palette table */
	GMT_IS_POSTSCRIPT,	/* Entity is a PostScript content struct */
	GMT_IS_TEXTSET,		/* Entity is a text table */
	GMT_IS_MATRIX,		/* Entity is user matrix */
	GMT_IS_VECTOR,		/* Entity is set of user vectors */
	GMT_IS_COORD,		/* Entity is a double coordinate array */
	GMT_N_FAMILIES};	/* Total number of families [API Developers only]  */

#define GMT_IS_CPT	GMT_IS_PALETTE		/* Backwards compatibility for < 5.3.3; */

/*! These are modes for handling comments */
enum GMT_enum_comment {
	GMT_COMMENT_IS_TEXT	= 0,        /* Comment is a text string */
	GMT_COMMENT_IS_OPTION	= 1U,   /* Comment is a linked list of GMT_OPTION structures */
	GMT_COMMENT_IS_COMMAND	= 2U,   /* Comment replaces header->command */
	GMT_COMMENT_IS_REMARK	= 4U,   /* Comment replaces header->remark */
	GMT_COMMENT_IS_TITLE	= 8U,   /* Comment replaces header->title */
	GMT_COMMENT_IS_NAME_X	= 16U,  /* Comment replaces header->x_units [grids only] */
	GMT_COMMENT_IS_NAME_Y	= 32U,  /* Comment replaces header->y_units [grids only] */
	GMT_COMMENT_IS_NAME_Z	= 64U,  /* Comment replaces header->z_units [grids only] */
	GMT_COMMENT_IS_COLNAMES	= 128U, /* Comment replaces header->colnames [tables only] */
	GMT_COMMENT_IS_RESET	= 256U, /* Wipe existing header first [append] */
	GMT_COMMENT_IS_MULTISEG = 512U};/* Comment is in fact a multisegment record */

enum GMT_enum_apierr {
	GMT_NOTSET  = -1,	/* When something is not set */
	GMT_NOERROR = 0};	/* Return code when all is well */

enum GMT_enum_module {
	GMT_MODULE_USAGE	= -6,	/* What GMT_Call_Module returns if told to print usage only */
	GMT_MODULE_SYNOPSIS	= -5,	/* What GMT_Call_Module returns if told to print synopsis only */
	GMT_MODULE_LIST		= -4,	/* mode for GMT_Call_Module to print list of all modules */
	GMT_MODULE_EXIST	= -3,	/* mode for GMT_Call_Module to return 0 if it exists */
	GMT_MODULE_PURPOSE	= -2,	/* mode for GMT_Call_Module to print purpose of module, or all modules */
	GMT_MODULE_OPT		= -1,	/* Gave linked list of option structures to GMT_Call_Module */
	GMT_MODULE_CMD		=  0};	/* Gave an array of text strings (argv[]) to GMT_Call_Module */

/* Array indices for input/output/stderr variables */

enum GMT_enum_std {
	GMT_IN = 0,	/* stdin */
	GMT_OUT,	/* stdout */
	GMT_ERR};	/* stderr */

/* There are 3 named columns */
enum GMT_enum_dimensions {
	GMT_X = 0,	/* x or lon is in 0th column */
	GMT_Y,		/* y or lat is in 1st column */
	GMT_Z};		/* z is in 2nd column */

enum GMT_enum_freg {
	GMT_ADD_FILES_IF_NONE = 1,	/* Tell GMT_Init_IO we conditionally want to register all input files in the option list if nothing else is registered */
	GMT_ADD_FILES_ALWAYS = 2,	/* Tell GMT_Init_IO to always register all input files in the option list */
	GMT_ADD_STDIO_IF_NONE = 4,	/* Tell GMT_Init_IO we conditionally want to register std(in|out) if nothing else has been registered */
	GMT_ADD_STDIO_ALWAYS = 8,	/* Tell GMT_Init_IO to always register std(in|out) */
	GMT_ADD_EXISTING = 16,		/* Tell GMT_Init_IO to only use already registered resources */
	GMT_ADD_DEFAULT = 6};		/* Tell GMT_Init_IO to register files, and if none are found then std(in|out), but only if nothing was registered before this call */

enum GMT_enum_ioset {
	GMT_IO_DONE = 0,		/* Tell GMT_End_IO we are done but nothing special is to be done. */
	GMT_IO_ASCII = 512,		/* Force ASCII mode for reading (ignoring current io settings). */
	GMT_IO_RESET = 32768,		/* Tell GMT_End_IO that accessed resources should be made read/write-able again. */
	GMT_IO_UNREG = 16384};		/* Tell GMT_End_IO to unregister all accessed resources. */

enum GMT_enum_read {
	GMT_READ_DATA = 0,		/* Read ASCII data record and return double array */
	GMT_READ_DOUBLE = 0,		/* Read ASCII data record and return double array [obsolete, replaced by GMT_READ_DATA for clarity] */
	GMT_READ_NORMAL = 0,		/* Normal read mode [Default] */
	GMT_READ_TEXT = 1,		/* Read ASCII data record and return text string */
	GMT_READ_MIXED = 2,		/* Read ASCII data record and return double array but tolerate conversion errors */
	GMT_READ_FILEBREAK = 4};	/* Add to mode to indicate we want to know when each file end is reached [continuous] */

enum GMT_enum_write {
	GMT_WRITE_DATA = 0,		/* Write double array to output */
	GMT_WRITE_DOUBLE = 0,		/* Write double array to output [obsolete, replaced by GMT_WRITE_DATA for clarity] */
	GMT_WRITE_TEXT = 1,		/* Write ASCII current record to output */
	GMT_WRITE_SEGMENT_HEADER = 2,	/* Write segment header record to output */
	GMT_WRITE_TABLE_HEADER = 3,	/* Write current record as table header to output */
	GMT_WRITE_TABLE_START = 4,	/* Write common header block to output (optional title + command line) */
	GMT_WRITE_SET = 0,		/* Write all output tables and all their segments to one destination [Default] */
	GMT_WRITE_OGR = 1,		/* Output OGR/GMT format [Requires proper -a setting] */
	GMT_WRITE_TABLE = 2,		/* Write each output table and all their segments to separate destinations */
	GMT_WRITE_SEGMENT = 3,		/* Write all output tables' segments to separate destinations */
	GMT_WRITE_TABLE_SEGMENT = 4,	/* Same as 2 but if no filenames we use both tbl and seg with format */
	GMT_WRITE_NORMAL = 0,		/* Write header and contents of this entity (table or segment) */
	GMT_WRITE_HEADER = 1,		/* Only write header and not the contents of this entity (table or segment) */
	GMT_WRITE_SKIP = 2,		/* Entirely skip this entity on output (table or segment) */
	GMT_WRITE_NOLF = 16,		/* Do not write LF at end of ASCII record, and not increment output rec number */
	GMT_STRICT_CONVERSION = 1024,	/* Do not convert text to double unless is is viable */
	GMT_LAX_CONVERSION = 2048};	/* Convert text to double if at least one field can be converted */

enum GMT_enum_header {
	GMT_HEADER_OFF = 0,		/* Disable header blocks out as default */
	GMT_HEADER_ON};			/* Enable header blocks out as default */

enum GMT_enum_alloc {
	GMT_ALLOC_EXTERNALLY = 0,	/* Allocated outside of GMT: We cannot reallocate or free this memory */
	GMT_ALLOC_INTERNALLY = 1,	/* Allocated by GMT: We may reallocate as needed and free when no longer needed */
	GMT_ALLOC_NORMAL = 0,		/* Normal allocation of new dataset based on shape of input dataset */
	GMT_ALLOC_VERTICAL = 4,		/* Allocate a single table for data set to hold all input tables by vertical concatenation */
	GMT_ALLOC_HORIZONTAL = 8};	/* Allocate a single table for data set to hold all input tables by horizontal (paste) concatenations */

enum GMT_enum_duplicate {
	GMT_DUPLICATE_NONE = 0,		/* Duplicate data set structure but no allocate&copy of data records|grid|image */
	GMT_DUPLICATE_ALLOC,		/* Duplicate data set structure and allocate space for data records|grid|image, but no copy */
	GMT_DUPLICATE_DATA,		/* Duplicate data set structure, allocate space for data records|grid|image, and copy */
	GMT_DUPLICATE_RESET = 4};	/* During duplicate, reset copy to normal system grid pad if original is different */

/* Various directions and modes to call the FFT */
enum GMT_enum_FFT {
	GMT_FFT_FWD     = 0U,		/* forward Fourier transform */
	GMT_FFT_INV     = 1U,		/* inverse Fourier transform */
	GMT_FFT_REAL    = 0U,		/* real-input FT (currently unsupported) */
	GMT_FFT_COMPLEX = 1U};		/* complex-input Fourier transform */

/* Various modes to select time in GMT_Message */
enum GMT_enum_time {
	GMT_TIME_NONE    = 0U,		/* Do not report time */
	GMT_TIME_CLOCK   = 1U,		/* Report absolute time formatted via FORMAT_TIME_STAMP */
	GMT_TIME_ELAPSED = 2U,		/* Report elapsed time since last time mark reset */
	GMT_TIME_RESET   = 4U};		/* Reset time mark */

/* Verbosity levels */
enum GMT_enum_verbose {GMT_MSG_QUIET = 0,  /* No messages whatsoever */
	GMT_MSG_NORMAL,                        /* Default output, e.g., warnings and errors only */
	GMT_MSG_TICTOC,                        /* To print a tic-toc elapsed time message */
	GMT_MSG_COMPAT,                        /* Compatibility warnings */
	GMT_MSG_VERBOSE,                       /* Verbose level */
	GMT_MSG_LONG_VERBOSE,                  /* Longer verbose, -Vl in some programs */
	GMT_MSG_DEBUG};                        /* Debug messages for developers mostly */

/*============================================================ */
/*===============+ GMT_GRID Public Declaration =============== */
/*============================================================ */

enum GMT_enum_reg {	/* Public constants for grid registration */
	GMT_GRID_NODE_REG	= 0U,
	GMT_GRID_PIXEL_REG	= 1U,
	GMT_GRID_DEFAULT_REG	= 1024U};	/* Means select whatever is implied via -r */

enum GMT_enum_gridindex {
        GMT_XLO = 0U,	/* Index for west or xmin value */
        GMT_XHI,	/* Index for east or xmax value */
        GMT_YLO,	/* Index for south or ymin value */
        GMT_YHI,	/* Index for north or ymax value */
        GMT_ZLO,	/* Index for zmin value */
        GMT_ZHI		/* Index for zmax value */
};

enum GMT_enum_dimindex {
        GMT_TBL = 0U,	/* Index for number of tables in dimension array */
        GMT_SEG,	/* Index for number of segments in dimension array */
        GMT_ROW,	/* Index for number of rows in dimension array */
        GMT_COL		/* Index for number of columns in dimension array [DATASET only] */
};

enum GMT_enum_gridio {
	GMT_GRID_IS_CARTESIAN	   = 0U,    /* Grid is not geographic but Cartesian */
	GMT_GRID_IS_REAL	   = 0U,    /* Read|write a normal real-valued grid */
	GMT_GRID_IS_COMPLEX_REAL   = 4U,    /* Read|write the real component to/from a complex grid */
	GMT_GRID_IS_COMPLEX_IMAG   = 8U,    /* Read|write the imaginary component to/from a complex grid */
	GMT_GRID_IS_COMPLEX_MASK   = 12U,   /* To mask out the real|imag flags */
	GMT_GRID_NO_HEADER	   = 16U,   /* Write a native grid without the leading grid header */
	GMT_GRID_ROW_BY_ROW	   = 32U,   /* Read|write the grid array one row at the time sequentially */
	GMT_GRID_ROW_BY_ROW_MANUAL = 64U,   /* Read|write the grid array one row at the time in any order */
	GMT_GRID_XY		   = 128U,  /* Allocate and initialize x,y vectors */
	GMT_GRID_IS_GEO		   = 256U}; /* Grid is a geographic grid, not Cartesian */

#define GMT_GRID_ALL		0U   /* Backwards compatibility for < 5.3.3; See GMT_CONTAINER_AND_DATA */
#define GMT_GRID_HEADER_ONLY	1U   /* Backwards compatibility for < 5.3.3; See GMT_CONTAINER_ONLY */
#define GMT_GRID_DATA_ONLY	2U   /* Backwards compatibility for < 5.3.3; See GMT_DATA_ONLY */

/* These lengths (except GMT_GRID_VARNAME_LEN80) must NOT be changed as they are part of grd definition */
enum GMT_enum_grdlen {
	GMT_GRID_UNIT_LEN80     = 80U,
	GMT_GRID_TITLE_LEN80    = 80U,
	GMT_GRID_VARNAME_LEN80  = 80U,
	GMT_GRID_COMMAND_LEN320 = 320U,
	GMT_GRID_REMARK_LEN160  = 160U,
	GMT_GRID_NAME_LEN256	= 256U,
	GMT_GRID_HEADER_SIZE    = 892U};

/* Note: GMT_GRID_HEADER_SIZE is 4 less than sizeof (struct GMT_GRID_HEADER) on
 * 64 bit systems due to alignment.  Since the GMT_GRID_HEADER was designed
 * during the 32-bit era its size in files is 892.  For backwards compatibility
 * we continue to enforce this header size by writing the structure components
 * separately. */

struct GMT_GRID_HEADER {
	/* Variables we document for the API:
	 * == Do not change the type of the following three items.
	 * == They are copied verbatim to the native grid header and must be 4-byte unsigned ints. */
	uint32_t n_columns;              /* Number of columns */
	uint32_t n_rows;                 /* Number of rows */
	uint32_t registration;           /* GMT_GRID_NODE_REG (0) for node grids, GMT_GRID_PIXEL_REG (1) for pixel grids */

	/* -- Here is the possible location for data structure padding:
	 *    A double is 8-byte aligned on Windows. */

	/* == The types of the following 12 elements must not be changed.
	 * == They are also copied verbatim to the native grid header. */
	double wesn[4];                        /* Min/max x and y coordinates */
	double z_min;                          /* Minimum z value */
	double z_max;                          /* Maximum z value */
	double inc[2];                         /* x and y increment */
	double z_scale_factor;                 /* grd values must be multiplied by this */
	double z_add_offset;                   /* After scaling, add this */
	char x_units[GMT_GRID_UNIT_LEN80];     /* units in x-direction */
	char y_units[GMT_GRID_UNIT_LEN80];     /* units in y-direction */
	char z_units[GMT_GRID_UNIT_LEN80];     /* grid value units */
	char title[GMT_GRID_TITLE_LEN80];      /* name of data set */
	char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
	char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
	/* == End of "untouchable" header. */

	/* ---- Variables "hidden" from the API ----
	 * This section is flexible.  It is not copied to any grid header
	 * or stored in any file.  It is considered private */
	unsigned int type;               /* Grid format */
	unsigned int bits;               /* Bits per data value (e.g., 32 for ints/floats; 8 for bytes) */
	unsigned int complex_mode;       /* 0 = normal, GMT_GRID_IS_COMPLEX_REAL = real part of complex grid, GMT_GRID_IS_COMPLEX_IMAG = imag part of complex grid */
	unsigned int mx, my;             /* Actual dimensions of the grid in memory, allowing for the padding */
	size_t nm;                       /* Number of data items in this grid (n_columns * n_rows) [padding is excluded] */
	size_t size;                     /* Actual number of items (not bytes) required to hold this grid (= mx * my) */
	size_t n_alloc;                  /* Bytes allcoated for this grid */
	unsigned int trendmode;          /* Holds status for detrending of grids.  0 if not detrended, 1 if mean, 2 if mid-value, and 3 if LS plane removed */
	unsigned int arrangement;        /* Holds status for complex grid as how the read/imag is placed in the grid (interleaved, R only, etc.) */
	unsigned int n_bands;            /* Number of bands [1]. Used with IMAGE containers and macros to get ij index from row,col, band */
	unsigned int pad[4];             /* Padding on west, east, south, north sides [2,2,2,2] */
	unsigned int BC[4];              /* Boundary condition applied on each side via pad [0 = not set, 1 = natural, 2 = periodic, 3 = data] */
	unsigned int grdtype;            /* 0 for Cartesian, > 0 for geographic and depends on 360 periodicity [see GMT_enum_grdtype in gmt_grd.h] */
	unsigned int reset_pad;          /* true in cases where we need a subset from a memory grid and must compute node index separately */
	char name[GMT_GRID_NAME_LEN256]; /* Actual name of the file after any ?<varname> and =<stuff> has been removed */
	char varname[GMT_GRID_VARNAME_LEN80];/* NetCDF: variable name */
	char  *ProjRefPROJ4;             /* To store a referencing system string in PROJ.4 format */
	char  *ProjRefWKT;               /* To store a referencing system string in WKT format */
	int row_order;                   /* NetCDF: k_nc_start_south if S->N, k_nc_start_north if N->S */
	int z_id;                        /* NetCDF: id of z field */
	int ncid;                        /* NetCDF: file ID */
	int xy_dim[2];                   /* NetCDF: dimension order of x and y; normally {1, 0} */
	size_t t_index[3];               /* NetCDF: index of higher coordinates */
	size_t data_offset;              /* NetCDF: distance from the beginning of the in-memory grid */
	unsigned int stride;             /* NetCDF: distance between two rows in the in-memory grid */
	float  nan_value;                /* Missing value as stored in grid file */
	double xy_off;                   /* 0.0 (registration == GMT_GRID_NODE_REG) or 0.5 ( == GMT_GRID_PIXEL_REG) */
	double r_inc[2];                 /* Reciprocal incs, i.e. 1/inc */
	char   flags[4];                 /* Flags used for ESRI grids */
	char  *pocket;                   /* GDAL: A working variable handy to transmit info between funcs e.g. +b<band_info> to gdalread */
	char   mem_layout[4];            /* Three or Four char codes T|B R|C S|R|S (grd) or B|L|P + A|a (img) describing array layout in mem and interleaving */
	double bcr_threshold;            /* sum of cardinals must >= threshold in bilinear; else NaN */
	unsigned int has_NaNs;           /* Is 2 if the grid contains any NaNs, 1 if it does not, and 0 if no check has yet happened */
	unsigned int bcr_interpolant;    /* Interpolation function used (0, 1, 2, 3) */
	unsigned int bcr_n;              /* Width of the interpolation function */
	unsigned int nxp;                /* if X periodic, nxp > 0 is the period in pixels  */
	unsigned int nyp;                /* if Y periodic, nxp > 0 is the period in pixels  */
	unsigned int no_BC;              /* If true we skip BC stuff entirely */
	unsigned int gn;                 /* true if top    edge will be set as N pole  */
	unsigned int gs;                 /* true if bottom edge will be set as S pole  */
	unsigned int is_netcdf4;         /* true if netCDF-4/HDF5 format */
	size_t z_chunksize[2];           /* chunk size (lat,lon) */
	unsigned int z_shuffle;          /* if shuffle filter is turned on */
	unsigned int z_deflate_level;    /* if deflate filter is in use */
	unsigned int z_scale_autoadjust; /* if z_scale_factor should be auto-detected */
	unsigned int z_offset_autoadjust;/* if z_add_offset should be auto-detected */
					 /* xy_*[] is separate settings for GMT_IN and GMT_OUT */
	unsigned int xy_adjust[2];	 /* 1 if +u<unit> was parsed and scale set, 3 if xy has been adjusted, 0 otherwise */
	unsigned int xy_mode[2];	 /* 1 if +U<unit> was parsed, 0 otherwise */
	unsigned int xy_unit[2];	 /* Unit enum specified via +u<unit> */
	double xy_unit_to_meter[2];	 /* Scale, given xy_unit, to convert xy from <unit> to meters */
	uint64_t (*index_function) (struct GMT_GRID_HEADER *, uint64_t, uint64_t, uint64_t);	/* Pointer to index function (for images only) */
#ifdef GMT_BACKWARDS_API
	uint32_t nx;
	uint32_t ny;
#endif
};

/* grd is stored in rows going from west (xmin) to east (xmax)
 * first row in file has yvalue = north (ymax).  
 * This is SCANLINE orientation.*/

/*-----------------------------------------------------------------------------------------
 *	Notes on registration:

	Assume x_min = y_min = 0 and x_max = y_max = 10 and x_inc = y_inc = 1.
	For a normal node grid we have:
		(1) n_columns = (x_max - x_min) / x_inc + 1 = 11
		    n_rows = (y_max - y_min) / y_inc + 1 = 11
		(2) node # 0 is at (x,y) = (x_min, y_max) = (0,10) and represents the surface
		    value in a box with dimensions (1,1) centered on the node.
	For a pixel grid we have:
		(1) n_columns = (x_max - x_min) / x_inc = 10
		    n_rows = (y_max - y_min) / y_inc = 10
		(2) node # 0 is at (x,y) = (x_min + 0.5*x_inc, y_max - 0.5*y_inc) = (0.5, 9.5)
		    and represents the surface value in a box with dimensions (1,1)
		    centered on the node.
-------------------------------------------------------------------------------------------*/

struct GMT_GRID {	/* To hold a GMT float grid and its header in one container */
	struct GMT_GRID_HEADER *header;	/* Pointer to full GMT header for the grid */
	float *data;                    /* Pointer to the float grid */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;                /* The internal number of the grid */
	unsigned int alloc_level;       /* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode; /* Allocation mode [GMT_ALLOC_INTERNALLY] */
	double  *x, *y;                 /* Vector of coordinates */
	void    *extra;                 /* Row-by-row machinery information [NULL] */
};

/*============================================================ */
/*============== GMT_DATASET Public Declaration ============== */
/*============================================================ */

/* GIS geometries, with GMT_IS_NONE as 16 for no such thing */
enum GMT_enum_geometry {
	GMT_IS_POINT	= 1U,
	GMT_IS_LINE	= 2U,
	GMT_IS_POLY	= 4U,
	GMT_IS_PLP	= 7U,	/* Could be any one of POINT, LINE, POLY */
	GMT_IS_SURFACE	= 8U,
	GMT_IS_NONE	= 16U};	/* Non-geographical items like color palettes and textsets */

/* These are two polygon modes */
enum GMT_enum_pol {
	GMT_IS_PERIMETER = 0,
	GMT_IS_HOLE = 1U};

/* Modes for setting output columns for rec-by-rec output */
enum GMT_enum_columns {
	GMT_COL_FIX = 0,
	GMT_COL_ADD,
	GMT_COL_SUB};

/* Return codes for GMT_Get_Record: */

enum GMT_enum_ascii_input_return {	/* Bit flag related to record i/o */
	GMT_IO_DATA_RECORD    =  0,		/* Read a data record and had no issues */
	GMT_IO_TABLE_HEADER   =  1U,	/* Read a table header */
	GMT_IO_SEGMENT_HEADER =  2U,	/* Read a segment header */
	GMT_IO_ANY_HEADER     =  3U,	/* Read either table or segment header */
	GMT_IO_MISMATCH       =  4U,	/* Read incorrect number of columns */
	GMT_IO_EOF            =  8U,	/* Read end-of-file */
	GMT_IO_NAN            = 16U,	/* Read a NaN record */
	GMT_IO_NEW_SEGMENT    = 18U,	/* Read either segment header or NaN-record */
	GMT_IO_GAP            = 32U,	/* Determined a gap should occur before this record */
	GMT_IO_LINE_BREAK     = 58U,	/* Segment break caused by seg header, gap, nan, or EOF */
	GMT_IO_NEXT_FILE      = 64U};	/* Like EOF except for an individual file (with more files to follow) */

/* Here are the GMT data types used for tables */

struct GMT_OGR {	/* Struct with all things GMT/OGR for a table */
	/* The first parameters are usually set once per data set and do not change */
	unsigned int geometry;		/* @G: The geometry of this data set, if known [0 otherwise] */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	char *region;			/* @R: The region textstring [NULL if not set] */
	char *proj[4];			/* @J: The 1-4 projection strings [NULL if not set] */
	unsigned int *type;		/* @T: The data types of the aspatial fields [NULL if not set]  */
	char **name;			/* @N The names of the aspatial fields [NULL if not set]  */
	/* The following are for OGR data only. It is filled during parsing (current segment) but is then copied to the segment header so it can be accessed later */
	enum GMT_enum_pol pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	char **tvalue;			/* @D: The text values of the current aspatial fields */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_OGR_SEG {	/* Struct with GMT/OGR aspatial data for a segment */
	enum GMT_enum_pol pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	char **tvalue;			/* @D: The values of the current aspatial fields (uses GMT_OGR's n_aspatial as length) */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_DATASEGMENT {    /* For holding segment lines in memory */
	/* Variables we document for the API: */
	uint64_t n_rows;        /* Number of points in this segment */
	uint64_t n_columns;     /* Number of fields in each record (>= 2) */
	double *min;            /* Minimum coordinate for each column */
	double *max;            /* Maximum coordinate for each column */
	double **data;          /* Data x,y, and possibly other columns */
	char *label;            /* Label string (if applicable) */
	char *header;           /* Segment header (if applicable) */
	char **text;            /* text beyond the data */
/* ---- Variables "hidden" from the API ---- */
	enum GMT_enum_write mode;	/* 0 = output segment, 1 = output header only, 2 = skip segment */
	enum GMT_enum_pol pol_mode;	/* Either GMT_IS_PERIMETER  [-Pp] or GMT_IS_HOLE [-Ph] (for polygons only) */
	uint64_t id;            /* The internal number of the segment */
	size_t n_alloc;         /* The current allocation length of each data column */
	unsigned int range;     /* Longitude reporting scheme, e.g. GMT_IS_GIVEN_RANGE [0] */
	int pole;               /* Spherical polygons only: If it encloses the S (-1) or N (+1) pole, or none (0) */
	double dist;            /* Distance from a point to this feature */
	double lat_limit;       /* For polar caps: the latitude of the point closest to the pole */
	struct GMT_OGR_SEG *ogr;/* NULL unless OGR/GMT metadata exist for this segment */
	struct GMT_DATASEGMENT *next;	/* NULL unless polygon and has holes and pointing to next hole */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	char *file[2];          /* Name of file or source [0 = in, 1 = out] */
#ifdef GMT_BACKWARDS_API
	double **coord;
#endif
};

struct GMT_DATATABLE {	/* To hold an array of line segment structures and header information in one container */
	/* Variables we document for the API: */
	unsigned int n_headers; /* Number of file header records (0 if no header) */
	uint64_t n_columns;     /* Number of columns (fields) in each record */
	uint64_t n_segments;    /* Number of segments in the array */
	uint64_t n_records;     /* Total number of data records across all segments */
	double *min;            /* Minimum coordinate for each column */
	double *max;            /* Maximum coordinate for each column */
	char **header;          /* Array with all file header records, if any) */
	struct GMT_DATASEGMENT **segment;	/* Pointer to array of segments */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;            /* The internal number of the table */
	size_t n_alloc;         /* The current allocation length of segments */
	double dist;            /* Distance from a point to this feature */
	enum GMT_enum_write mode;	/* 0 = output table, 1 = output header only, 2 = skip table */
	struct GMT_OGR *ogr;    /* Pointer to struct with all things GMT/OGR (if MULTI-geometry and not MULTIPOINT) */
	char *file[2];          /* Name of file or source [0 = in, 1 = out] */
};

/* The main GMT Data Containers used in the API: */

struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
	/* Variables we document for the API: */
	uint64_t n_tables;		/* The total number of tables (files) contained */
	uint64_t n_columns;		/* The number of data columns */
	uint64_t n_segments;	/* The total number of segments across all tables */
	uint64_t n_records;		/* The total number of data records across all tables */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	struct GMT_DATATABLE **table;	/* Pointer to array of tables */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the data set */
	size_t n_alloc;			/* The current allocation length of tables */
	uint64_t dim[4];		/* Only used by GMT_Duplicate_Data to override dimensions */
	unsigned int geometry;	/* The geometry of this dataset */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_write io_mode;	/* -1 means write OGR format (requires proper -a),
					 * 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments.
					 * 3 is same as 2 but with no filenames we create filenames from tbl and seg numbers */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

/*============================================================ */
/*============== GMT_TEXTSET Public Declaration ============== */
/*============================================================ */

struct GMT_TEXTSEGMENT {		/* For holding segment text records in memory */
	/* Variables we document for the API: */
	uint64_t n_rows;		/* Number of rows in this segment */
	char **data;			/* Array of text records */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Segment header (if applicable) */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the table */
	enum GMT_enum_write mode;	/* 0 = output segment, 1 = output header only, 2 = skip segment */
	size_t n_alloc;			/* Number of rows allocated for this segment */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	char **tvalue;			/* The values of the OGR/GMT aspatial fields */	
#ifdef GMT_BACKWARDS_API
	char **record;
#endif
};

struct GMT_TEXTTABLE {	/* To hold an array of text segment structures and header information in one container */
	/* Variables we document for the API: */
	unsigned int n_headers;		/* Number of file header records (0 if no header) */
	uint64_t n_segments;		/* Number of segments in the array */
	uint64_t n_records;		/* Total number of data records across all segments */
	char **header;			/* Array with all file header records, if any) */
	struct GMT_TEXTSEGMENT **segment;	/* Pointer to array of segments */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the table */
	size_t n_alloc;			/* The current allocation length of segments */
	enum GMT_enum_write mode;	/* 0 = output table, 1 = output header only, 2 = skip table */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

struct GMT_TEXTSET {	/* Single container for an array of GMT text tables (files) */
	/* Variables we document for the API: */
	uint64_t n_tables;		/* The total number of tables (files) contained */
	uint64_t n_segments;		/* The total number of segments across all tables */
	uint64_t n_records;		/* The total number of data records across all tables */
	struct GMT_TEXTTABLE **table;	/* Pointer to array of tables */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the data set */
	size_t n_alloc;			/* The current allocation length of tables */
	unsigned int geometry;		/* The geometry of this dataset */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_write io_mode;	/* -1 means write OGR format (requires proper -a),
					 * 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments.
					 * 3 is same as 2 but with no filenames we create filenames from tbl and seg numbers */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

/*============================================================ */
/*============== GMT_PALETTE Public Declaration ============== */
/*============================================================ */

enum GMT_enum_color {GMT_RGB	= 0,
	GMT_CMYK		= 1,
	GMT_HSV			= 2,
	GMT_COLORINT		= 4,
	GMT_NO_COLORNAMES	= 8};

enum GMT_enum_bfn {GMT_BGD, GMT_FGD, GMT_NAN};

enum GMT_enum_cpt {GMT_CPT_REQUIRED, GMT_CPT_OPTIONAL};

enum GMT_enum_cptflags {GMT_CPT_NO_BNF = 1, GMT_CPT_EXTEND_BNF = 2, GMT_CPT_HINGED = 4};

/* Here is the definition of the GMT_PALETTE structure that is used in programs
 * that deals with coloring of items as a function of z-lookup.  Note that rgb
 * arrays have 4 items as the 4th value could be a non-zero transparency (when supported).
 */
 
struct GMT_LUT {
	double z_low, z_high, i_dz;
	double rgb_low[4], rgb_high[4], rgb_diff[4];
	double hsv_low[4], hsv_high[4], hsv_diff[4];
	unsigned int annot;	/* 1 for Lower, 2 for Upper, 3 for Both */
	unsigned int skip;	/* true means skip this slice */
	struct GMT_FILL *fill;	/* For patterns instead of color */
	char *label;		/* For non-number labels */
};

struct GMT_BFN {		/* For back-, fore-, and nan-colors */
	double rgb[4];		/* Red, green, blue, and alpha */
	double hsv[4];		/* Hue, saturation, value, alpha */
	unsigned int skip;	/* true means skip this slice */
	struct GMT_FILL *fill;	/* For patterns instead of color */
};

struct GMT_PALETTE {		/* Holds all pen, color, and fill-related parameters */
	/* Variables we document for the API: */
	unsigned int n_headers;		/* Number of CPT header records (0 if no header) */
	unsigned int n_colors;		/* Number of colors in CPT lookup table */
	unsigned int mode;		/* Flags controlling use of BFN colors */
	struct GMT_LUT *data;		/* CPT lookup table read by gmtlib_read_cpt */
	struct GMT_BFN bfn[3];		/* Structures with back/fore/nan colors */
	char **header;			/* Array with all CPT header records, if any) */		/* Content not counted by sizeof (struct) */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the data set */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	unsigned int alloc_level;	/* The level it was allocated at */
	unsigned int auto_scale;	/* If 1 then we must resample to fit actual data range */
	unsigned int model;		/* RGB, HSV, CMYK */
	unsigned int is_wrapping;	/* If 1 then we must wrap around to find color - can never be F or B */
	unsigned int is_gray;		/* true if only grayshades are needed */
	unsigned int is_bw;		/* true if only black and white are needed */
	unsigned int is_continuous;	/* true if continuous color tables have been given */
	unsigned int has_pattern;	/* true if CPT contains any patterns */
	unsigned int has_hinge;		/* true if CPT is hinged at hinge (below) */
	unsigned int has_range;		/* true if CPT has a natural range (minmax below) */
	unsigned int skip;		/* true if current z-slice is to be skipped */
	unsigned int categorical;	/* true if CPT applies to categorical data */
	unsigned int z_adjust[2];	/* 1 if +u<unit> was parsed and scale set, 3 if z has been adjusted, 0 otherwise */
	unsigned int z_mode[2];	 	/* 1 if +U<unit> was parsed, 0 otherwise */
	unsigned int z_unit[2];	 	/* Unit enum specified via +u<unit> */
	double z_unit_to_meter[2];	/* Scale, given z_unit, to convert z from <unit> to meters */
	double minmax[2];		/* Min/max z-value for a default range, if given */
	double hinge;			/* z-value for hinged CPTs */
	double wrap_length;		/* z-length of active CPT */
#ifdef GMT_BACKWARDS_API
	struct GMT_LUT *range;
	struct GMT_BFN *patch;
	unsigned int cpt_flags;
#endif
};

/*============================================================ */
/*=============== GMT_IMAGE Public Declaration =============== */
/*============================================================ */

/* The GMT_IMAGE container is used to pass user images in from the GDAL bridge */

struct GMT_IMAGE {	/* Single container for a user image of data */
	/* Variables we document for the API: */
	enum    GMT_enum_type type;     /* Data type, e.g. GMT_FLOAT */
	int    *colormap;               /* Array with color lookup values */
	int     n_indexed_colors;       /* Number of colors in a paletted image */
	struct  GMT_GRID_HEADER *header;/* Pointer to full GMT header for the image */
	unsigned char *data;            /* Pointer to actual image */
	unsigned char *alpha;           /* Pointer to an optional transparency layer stored in a separate variable */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;                    /* The internal number of the data set */
	unsigned int alloc_level;       /* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
	const char *color_interp;
	double  *x, *y;                 /* Vector of coordinates */
#ifdef GMT_BACKWARDS_API
	int *ColorMap; 
	int nIndexedColors; 
#endif
};

/*==================================================================== */
/*================= GMT_POSTSCRIPT Public Declaration ================ */
/*==================================================================== */

/* The GMT_POSTSCRIPT container is used to pass PostScript objects */

enum GMT_enum_ps {
	GMT_PS_EMPTY = 0,
	GMT_PS_HEADER,
	GMT_PS_TRAILER,
	GMT_PS_COMPLETE
};

struct GMT_POSTSCRIPT {	/* Single container for a chunk of PostScript */
	/* Variables we document for the API: */
	size_t n_alloc;             /* Memory allocated so far */
	size_t n_bytes;             /* Length of data array */
	unsigned int mode;          /* GMT_PS_HEADER = Has header, GMT_PS_TRAILER = Has trailer, GMT_PS_COMPLETE = Has both */
	unsigned int n_headers;     /* Number of PS header records (0 if no header) */
	char *data;		    /* Pointer to actual PS text */
	char **header;		    /* Array with all PS header records, if any) */		/* Content not counted by sizeof (struct) */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;                /* The internal number of the data set */
	unsigned int alloc_level;   /* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

/*============================================================ */
/*============= GMT_UNIVECTOR Public Declaration ============= */
/*============================================================ */

/* This union is used to hold any type of array */
union GMT_UNIVECTOR {
	/* Universal vector of any data type can be held here */
	uint8_t  *uc1; /* Unsigned 1-byte int */
	int8_t   *sc1; /* Signed 1-byte int */
	uint16_t *ui2; /* Unsigned 2-byte int */
	int16_t  *si2; /* Signed 2-byte int */
	uint32_t *ui4; /* Unsigned 4-byte int */
	int32_t  *si4; /* Signed 4-byte int */
	uint64_t *ui8; /* Unsigned 8-byte int */
	int64_t  *si8; /* Signed 8-byte int */
	float    *f4;  /* 4-byte float */
	double   *f8;  /* 8-byte float */
};

/*============================================================ */
/*=============== GMT_VECTOR Public Declaration ============== */
/*============================================================ */

struct GMT_VECTOR {	/* Single container for user vector(s) of data */
	/* Variables we document for the API: */
	uint64_t n_columns;		/* Number of vectors */
	uint64_t n_rows;		/* Number of rows in each vector */
	enum GMT_enum_reg registration;	/* 0 for gridline and 1 for pixel registration  */
	enum GMT_enum_type *type;	/* Array of data types (type of each uni-vector, e.g. GMT_FLOAT */
	union GMT_UNIVECTOR *data;	/* Array of uni-vectors */
	double range[2];		/* Contains tmin/tmax (or 0/0 if not equidistant) */
	char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
	char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the data set */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

/*============================================================ */
/*=============== GMT_MATRIX Public Declaration ============== */
/*============================================================ */

enum GMT_enum_fmt {
	GMT_IS_ROW_FORMAT	= 0,	/* 2-D grid is C-style with rows: as index increase we move across rows */
	GMT_IS_COL_FORMAT	= 1};	/* 2-D grid is Fortran-style with columns: as index increase we move down columns  */

/* These containers are used to pass user vectors and matrices in/out of GMT */

struct GMT_MATRIX {	/* Single container for a user matrix of data */
	/* Variables we document for the API: */
	uint64_t n_rows;		/* Number of rows in this matrix */
	uint64_t n_columns;		/* Number of columns in this matrix */
	uint64_t n_layers;		/* Number of layers in a 3-D matrix [1] */
	enum GMT_enum_fmt shape;	/* 0 = C (rows) and 1 = Fortran (cols) */
	enum GMT_enum_reg registration;	/* 0 for gridline and 1 for pixel registration  */
	size_t dim;			/* Allocated length of longest C or Fortran dim */
	size_t size;			/* Byte length of data */
	enum GMT_enum_type type;	/* Data type, e.g. GMT_FLOAT */
	double range[6];		/* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
	union GMT_UNIVECTOR data;	/* Union with pointer to actual matrix of the chosen type */
	char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
	char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
/* ---- Variables "hidden" from the API ---- */
	uint64_t id;			/* The internal number of the data set */
	unsigned int alloc_level;	/* The level it was allocated at */
	enum GMT_enum_alloc alloc_mode;	/* Allocation mode [GMT_ALLOC_INTERNALLY] */
};

/*============================================================ */
/*=============== GMT_OPTION Public Declaration ============== */
/*============================================================ */

enum GMT_enum_opt {
	GMT_OPT_USAGE =     '?',	/* Command-line option for full usage */
	GMT_OPT_SYNOPSIS =  '^',	/* Command-line option for synopsis */
	GMT_OPT_PARAMETER = '-',	/* Command-line option for GMT defaults parameter */
	GMT_OPT_INFILE =    '<',	/* Command-line option for input file */
	GMT_OPT_OUTFILE =   '>'};	/* Command-line option for output file */

/* This struct is used to pass program options in/out of GMT modules */

struct GMT_OPTION {              /* Structure for a single GMT command option */
	char option;                 /* 1-char command line -<option> (e.g. D in -D) identifying the option (* if file) */
	char *arg;                   /* If not NULL, contains the argument for this option */
	struct GMT_OPTION *next;     /* Pointer to next option in a linked list */
	struct GMT_OPTION *previous; /* Pointer to previous option in a linked list */
};

/*============================================================ */
/*  struct GMT_RESOURCE is for API Developers only   ========= */
/*============================================================ */

struct GMT_RESOURCE {	/* Information related to passing resources between GMT and external APIs */
	enum GMT_enum_family family;	/* GMT data family, i.e., GMT_IS_DATASET, GMT_IS_GRID, etc. */
	enum GMT_enum_geometry geometry;/* One of the recognized GMT geometries */
	enum GMT_enum_std direction;	/* Either GMT_IN or GMT_OUT */
	struct GMT_OPTION *option;	/* Pointer to the corresponding module option */
	char name[GMT_STR16];		/* Virtual file name for resource */
	int pos;			/* Corresponding index into external object in|out arrays */
	int mode;			/* Either primary (0) or secondary (1) resource */
	void *object;			/* Pointer to the actual GMT object */
};

#endif /* _GMT_RESOURCES_H */
