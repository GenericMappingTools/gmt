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
/*
 * gmt_resources.h contains the definitions for the GMT 5 resources
 * GMT_GRID, GMT_DATASET, GMT_TEXTSET, GMT_PALETTE, and GMT_IMAGE
 * as well as named constants.
 *
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef _GMT_RESOURCES_H
#define _GMT_RESOURCES_H

/*============================================================ */
/*=============== Constants Public Declaration =============== */
/*============================================================ */

/* Here are structure definitions and constants (enums) needed by the
 * public GMT API. Note: As this is C, some structures will contain
 * members we do not disclose in the API documentation but of course
 * the variables are accessible just like the "public" ones.
 */

/* These are the 6 methods for i/o */

enum GMT_enum_methods {
	GMT_IS_FILE = 0, 	/* Entity is a filename */
	GMT_IS_STREAM,		/* Entity is an open stream */
	GMT_IS_FDESC,           /* Entity is an open file descriptor */
	GMT_IS_COPY,            /* Entity is a memory location that should be duplicated */
	GMT_IS_REF,             /* Entity is a memory location and we just pass the ref (no copying) */
	GMT_IS_READONLY,        /* As GMT_IS_REF, but we are not allowed to change the data in any way. */
	GMT_N_METHODS};         /* Number of methods we recognize */

/* But Grid can come from a GMT grid OR User Matrix, and Data can come from DATASET or via Vectors|Matrix, and Text from TEXTSET or Matrix */

enum GMT_enum_via {
	GMT_VIA_VECTOR = 100,	/* Data passed via user matrix */
	GMT_VIA_MATRIX = 200};	/* Data passed via user vectors */

/* These are the 5 families of data types */
enum GMT_enum_families {
	GMT_IS_DATASET = 0,	/* Entity is data table */
	GMT_IS_TEXTSET,		/* Entity is a Text table */
	GMT_IS_GRID,		/* Entity is a GMT grid */
	GMT_IS_CPT,		/* Entity is a CPT table */
	GMT_IS_IMAGE,		/* Entity is a 1- or 3-layer unsigned char image */
	GMT_IS_VECTOR,		/* to hande interfacing with user data types: */
	GMT_IS_MATRIX,		/* Entity is user vectors */
	GMT_N_FAMILIES};	/* Entity is user matrix */

/* Array indices for input/output/stderr variables */

enum GMT_io_enum {
	GMT_IN = 0,	/* stdin */
	GMT_OUT,	/* stdout */
	GMT_ERR};	/* stderr */

/* There are 3 named columns */
enum GMT_enum_dimensions {
	GMT_X,		/* x or lon is in 0th column */
	GMT_Y,		/* y or lat is in 1st column */
	GMT_Z};		/* z is in 2nd column */

enum GMT_enum_freg {
	GMT_REG_FILES_IF_NONE = 1,	/* Tell GMT_Init_IO we conditionally want to register all input files in the option list if nothing else is registered */
	GMT_REG_FILES_ALWAYS = 2,		/* Tell GMT_Init_IO to always register all input files in the option list */
	GMT_REG_STD_IF_NONE = 4,		/* Tell GMT_Init_IO we conditionally want to register std(in|out) if nothing else has been registered */
	GMT_REG_STD_ALWAYS = 8,			/* Tell GMT_Init_IO to always register std(in|out) */
	GMT_REG_DEFAULT = 5};			/* Tell GMT_Init_IO to register files, and if none are found then std(in|out), but only if nothing was registered before this call */

enum GMT_enum_read {
	GMT_READ_DOUBLE = 0,	/* Read ASCII data record and return double array */
	GMT_READ_NORMAL = 0,	/* Normal read mode [Default] */
	GMT_READ_TEXT = 1,			/* Read ASCII data record and return text string */
	GMT_READ_MIXED = 2,			/* Read ASCII data record and return double array but tolerate conversion errors */
	GMT_FILE_BREAK = 4};			/* Add to mode to indicate we want to know when each file end is reached [continuous] */

enum GMT_enum_write {
	GMT_WRITE_DOUBLE = 0,	/* Write double array to output */
	GMT_WRITE_TEXT,		/* Write ASCII current record to output */
	GMT_WRITE_SEGHEADER,	/* Write segment header record to output */
	GMT_WRITE_TBLHEADER,	/* Write current record as table header to output */
	GMT_WRITE_NOLF = 16};	/* Do not write LF at end of ascii record, and not increment output rec number */

enum GMT_enum_dest {
	GMT_WRITE_OGR = -1,	/* Output OGR/GMT format [Requires proper -a setting] */
	GMT_WRITE_SET,			/* Write all output tables and all their segments to one destination [Default] */
	GMT_WRITE_TABLES,		/* Write each output table and all their segments to separate destinations */
	GMT_WRITE_SEGMENTS,		/* Write all output tables' segments to separate destinations */
	GMT_WRITE_TABLE_SEGMENTS};	/* Same as 2 but if no filenames we use both tbl and seg with format */

enum GMT_enum_alloc {
	GMT_ALLOCATED = 0, /* Item was allocated so GMT_* modules should free when GMT_Destroy_Data is called */
	GMT_REFERENCE,     /* Item was not allocated so GMT_* modules should NOT free when GMT_Destroy_Data is called, but may realloc if needed */
	GMT_READONLY,      /* Item was not allocated so GMT_* modules should NOT free when GMT_Destroy_Data is called . Consider read-only data */
	GMT_CLOBBER};      /* Free item no matter what its allocation status */

enum GMT_enum_shape {
	GMT_ALLOC_NORMAL = 0,	/* Normal allocation of new dataset based on shape of input dataset */
	GMT_ALLOC_VERTICAL,			/* Allocate a single table for data set to hold all input tables by vertical concatenation */
	GMT_ALLOC_HORIZONTAL};			/* Alocate a single table for data set to hold all input tables by horizontal (paste) concatenations */

enum GMT_enum_out {
	GMT_WRITE_NORMAL = 0,	/* Write header and contents of this entity (table or segment) */
	GMT_WRITE_HEADER,		/* Only write header and not the contents of this entity (table or segment) */
	GMT_WRITE_SKIP};		/* Entirely skip this entity on output (table or segment) */

/*============================================================ */
/*===============+ GMT_GRID Public Declaration =============== */
/*============================================================ */

enum GMT_enum_reg {	/* Public constants for grid registration */
	GMT_GRIDLINE_REG = 0,
	GMT_PIXEL_REG};

enum GMT_enum_gridio {
	GMT_GRID_ALL = 0,	/* Read|write both grid header and the entire grid (no subset) */
	GMT_GRID_HEADER = 1,		/* Just read|write the grid header */
	GMT_GRID_DATA = 2,		/* Read|write the grid array given w/e/s/n set in the header */
	GMT_GRID_ALL2 = 3,		/* The 1|2 flags together, same meaning as GMT_GRID_ALL */
	GMT_GRID_REAL = 0,		/* Read|write a normal real-valued grid */
	GMT_GRID_COMPLEX_REAL = 4,	/* Read|write the real component to/from a complex grid */
	GMT_GRID_COMPLEX_IMAG = 8,	/* Read|write the imaginary component to/from a complex grid */
	GMT_GRID_COMPLEX_MASK = 12,	/* To mask out the rea|imag flags */
	GMT_GRID_NO_HEADER = 16};	/* Write a native grid without the leading grid header */

/* These lengths (except GMT_GRID_VARNAME_LEN80) must NOT be changed as they are part of grd definition */
enum GMT_enum_grdlen {
	GMT_GRID_UNIT_LEN80     = 80U,
	GMT_GRID_TITLE_LEN80    = 80U,
	GMT_GRID_VARNAME_LEN80  = 80U,
	GMT_GRID_COMMAND_LEN320 = 320U,
	GMT_GRID_REMARK_LEN160  = 160U,
	GMT_GRID_NAME_LEN256	= 256U,
	GMT_GRID_HEADER_SIZE    = 892U};

/* Note: GMT_GRID_HEADER_SIZE is 4 less than sizeof (struct GRD_HEADER) for 64 bit systems due to alignment.
   Since the GRD_HEADER was designed during 32-bit era its sizeof was 892.  Bof backwards compatibility
   we continue to enforce this header size by not writing the structure components separately. */

struct GRD_HEADER {
/* Variables we document for the API: */
/* ===== Do not change the first three items. They are copied verbatim to the native grid header */
	unsigned int nx;                /* Number of columns */
	unsigned int ny;                /* Number of rows */
	unsigned int registration;      /* GMT_GRIDLINE_REG (0) for node grids, GMT_PIXEL_REG (1) for pixel grids */
	
	/* ---- Variables "hidden" from the API ---- */
/* This section is flexible. It is not copied to any grid header or stored in the file. It is considered private */
	unsigned int type;               /* Grid format */
	unsigned int bits;               /* Bits per data value (e.g., 32 for ints/floats; 8 for bytes) */
	unsigned int complex_mode;       /* 0 = normal, GMT_GRID_COMPLEX_REAL = real part of complex grid, GMT_GRID_COMPLEX_IMAG = imag part of complex grid */
	unsigned int mx, my;             /* Actual dimensions of the grid in memory, allowing for the padding */
	unsigned int BB_mx, BB_my;       /* Actual dimensions of a mosaicked grid, allowing for the padding */
	size_t nm;                       /* Number of data items in this grid (nx * ny) [padding is excluded] */
	size_t size;                     /* Actual number of items (not bytes) required to hold this grid (= mx * my) */
	unsigned int n_bands;            /* Number of bands [1]. Used with IMAGE containers and macros to get ij index from row,col, band */
	unsigned int pad[4];             /* Padding on west, east, south, north sides [2,2,2,2] */
	unsigned int BC[4];              /* Boundary condition applied on each side via pad [0 = not set, 1 = natural, 2 = periodic, 3 = data] */
	unsigned int grdtype;            /* 0 for Cartesian, > 0 for geographic and depends on 360 periodicity [see GMT_enum_grdtype above] */
	char name[GMT_GRID_NAME_LEN256];     /* Actual name of the file after any ?<varname> and =<stuff> has been removed */
	char varname[GMT_GRID_VARNAME_LEN80];/* NetCDF: variable name */
	int row_order;                   /* NetCDF: k_nc_start_south if S->N, k_nc_start_north if N->S */
	int z_id;                        /* NetCDF: id of z field */
	int ncid;                        /* NetCDF: file ID */
	int xy_dim[2];                   /* NetCDF: dimension order of x and y; normally {1, 0} */
	size_t t_index[3];               /* NetCDF: index of higher coordinates */
	size_t data_offset;              /* NetCDF: distance from the beginning of the in-memory grid */
	size_t stride;                   /* NetCDF: distance between two rows in the in-memory grid */
	double nan_value;                /* Missing value as stored in grid file */
	double xy_off;                   /* 0.0 (registration == GMT_GRIDLINE_REG) or 0.5 ( == GMT_PIXEL_REG) */
	double r_inc[2];                 /* Reciprocal incs, i.e. 1/inc */
	char flags[4];                   /* Flags used for ESRI grids */
	char *pocket;                    /* GDAL: A working variable handy to transmit info between funcs e.g. +b<band_info> to gdalread */
	double bcr_threshold;            /* sum of cardinals must >= threshold in bilinear; else NaN */
	unsigned int bcr_interpolant;    /* Interpolation function used (0, 1, 2, 3) */
	unsigned int bcr_n;              /* Width of the interpolation function */
	unsigned int nxp;                /* if X periodic, nxp > 0 is the period in pixels  */
	unsigned int nyp;                /* if Y periodic, nxp > 0 is the period in pixels  */
	unsigned int no_BC;              /* If true we skip BC stuff entirely */
	unsigned int gn;                 /* true if top    edge will be set as N pole  */
	unsigned int gs;                 /* true if bottom edge will be set as S pole  */
	unsigned int is_netcdf4;         /* true if netCDF-4/HDF5 format */
	unsigned int z_chunksize[2];     /* chunk size (lat,lon) */
	unsigned int z_shuffle;          /* if shuffle filter is turned on */
	unsigned int z_deflate_level;    /* if deflate filter is in use */
	unsigned int z_scale_autoadust;  /* if z_scale_factor should be auto-detected */
	unsigned int z_offset_autoadust; /* if z_add_offset should be auto-detected */

/* Variables we document for the API: */
/* ===== The following elements must not be changed. They are copied verbatim to the native grid header */
	double wesn[4];                   /* Min/max x and y coordinates */
	double z_min;                     /* Minimum z value */
	double z_max;                     /* Maximum z value */
	double inc[2];                    /* x and y increment */
	double z_scale_factor;            /* grd values must be multiplied by this */
	double z_add_offset;              /* After scaling, add this */
	char x_units[GMT_GRID_UNIT_LEN80];     /* units in x-direction */
	char y_units[GMT_GRID_UNIT_LEN80];     /* units in y-direction */
	char z_units[GMT_GRID_UNIT_LEN80];     /* grid value units */
	char title[GMT_GRID_TITLE_LEN80];      /* name of data set */
	char command[GMT_GRID_COMMAND_LEN320]; /* name of generating command */
	char remark[GMT_GRID_REMARK_LEN160];   /* comments re this data set */
};
/* grd is stored in rows going from west (xmin) to east (xmax)
 * first row in file has yvalue = north (ymax).  
 * This is SCANLINE orientation.*/

/*-----------------------------------------------------------------------------------------
 *	Notes on registration:

	Assume x_min = y_min = 0 and x_max = y_max = 10 and x_inc = y_inc = 1.
	For a normal node grid we have:
		(1) nx = (x_max - x_min) / x_inc + 1 = 11
		    ny = (y_max - y_min) / y_inc + 1 = 11
		(2) node # 0 is at (x,y) = (x_min, y_max) = (0,10) and represents the surface
		    value in a box with dimensions (1,1) centered on the node.
	For a pixel grid we have:
		(1) nx = (x_max - x_min) / x_inc = 10
		    ny = (y_max - y_min) / y_inc = 10
		(2) node # 0 is at (x,y) = (x_min + 0.5*x_inc, y_max - 0.5*y_inc) = (0.5, 9.5)
		    and represents the surface value in a box with dimensions (1,1)
		    centered on the node.
-------------------------------------------------------------------------------------------*/

/*============================================================ */
/*============== GMT_DATASET Public Declaration ============== */
/*============================================================ */

/* GIS geometries, with GMT_IS_TEXT as 0 for no such thing */
enum GMT_enum_geometries {
	GMT_IS_TEXT = 0,
	GMT_IS_ANY = 0,
	GMT_IS_POINT = 1,
	GMT_IS_LINE,
	GMT_IS_POLY,
	GMT_IS_SURFACE,
	GMT_N_GEOMETRIES};

/* These are two polygon modes */
enum GMT_enum_pol {
	GMT_IS_PERIMETER = 0,
	GMT_IS_HOLE};

/* Return codes for GMT_ascii_input: */

enum GMT_enum_ascii_input_return {
	GMT_IO_TBL_HEADER = 1,	/* Read a table header */
	GMT_IO_SEG_HEADER	=  2,		/* Read a segment header */
	GMT_IO_MISMATCH		=  4,		/* Read incorrect number of columns */
	GMT_IO_EOF		=  8,		/* Read end-of-file */
	GMT_IO_NAN		= 16,		/* Read a NaN record */
	GMT_IO_GAP		= 32,		/* Determined a gap should occur before this record */
	GMT_IO_NEXT_FILE	= 64};		/* Like EOF except for an individual file (with more files to follow) */

/* Here are the GMT data types used for tables */

struct GMT_OGR {	/* Struct with all things GMT/OGR for a table */
	/* The first parameters are usually set once per data set and do not change */
	unsigned int geometry;	/* @G: The geometry of this data set, if known [0 otherwise] */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	char *region;			/* @R: The region textstring [NULL if not set] */
	char *proj[4];			/* @J: The 1-4 projection strings [NULL if not set] */
	unsigned int *type;		/* @T: The data types of the aspatial fields [NULL if not set]  */
	char **name;			/* @N The names of the aspatial fields [NULL if not set]  */
	/* The following are for OGR data only. It is filled during parsing (current segment) but is then copied to the segment header so it can be accessed later */
	unsigned int pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	char **value;			/* @D: The text values of the current aspatial fields */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_OGR_SEG {	/* Struct with GMT/OGR aspatial data for a segment*/
	unsigned int pol_mode;	/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	unsigned int n_aspatial;	/* @T: The number of aspatial fields */
	char **value;			/* @D: The values of the current aspatial fields (uses GMT_OGR's n_aspatial as length) */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_LINE_SEGMENT {		/* For holding segment lines in memory */
	/* Variables we document for the API: */
	uint64_t n_rows;		/* Number of points in this segment */
	unsigned int n_columns;		/* Number of fields in each record (>= 2) */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	double **coord;			/* Coordinates x,y, and possibly other columns */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Segment header (if applicable) */
/* ---- Variables "hidden" from the API ---- */
	unsigned int mode;		/* 0 = output segment, 1 = output header only, 2 = skip segment */
	unsigned int pol_mode;		/* Either GMT_IS_PERIMETER  [-Pp] or GMT_IS_HOLE [-Ph] (for polygons only) */
	uint64_t id;			/* The internal number of the segment */
	size_t n_alloc;			/* The current allocation length of each coord */
	int range;			/* 0 = use default lon adjustment, -1 = negative longs, +1 = positive lons */
	int pole;			/* Spherical polygons only: If it encloses the S (-1) or N (+1) pole, or none (0) */
	double dist;			/* Distance from a point to this feature */
	double lat_limit;		/* For polar caps: the latitude of the point closest to the pole */
	struct GMT_OGR_SEG *ogr;	/* NULL unless OGR/GMT metadata exist for this segment */
	struct GMT_LINE_SEGMENT *next;	/* NULL unless polygon and has holes and pointing to next hole */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

struct GMT_TABLE {	/* To hold an array of line segment structures and header information in one container */
	/* Variables we document for the API: */
	unsigned int n_headers;	/* Number of file header records (0 if no header) */
	unsigned int n_columns;	/* Number of columns (fields) in each record */
	uint64_t n_segments;	/* Number of segments in the array */
	uint64_t n_records;	/* Total number of data records across all segments */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	char **header;			/* Array with all file header records, if any) */
	struct GMT_LINE_SEGMENT **segment;	/* Pointer to array of segments */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the table */
	unsigned int mode;		/* 0 = output table, 1 = output header only, 2 = skip table */
	size_t n_alloc;			/* The current allocation length of segments */
	struct GMT_OGR *ogr;		/* Pointer to struct with all things GMT/OGR (if MULTI-geometry and not MULTIPOINT) */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

/* The main GMT Data Containers used in the API: */

struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
	/* Variables we document for the API: */
	unsigned int n_tables;		/* The total number of tables (files) contained */
	unsigned int n_columns;		/* The number of data columns */
	uint64_t n_segments;		/* The total number of segments across all tables */
	uint64_t n_records;		/* The total number of data records across all tables */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	struct GMT_TABLE **table;	/* Pointer to array of tables */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the data set */
	size_t n_alloc;			/* The current allocation length of tables */
	enum GMT_enum_dest io_mode;	/* -1 means write OGR format (requires proper -a),
					 * 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments.
					 * 3 is same as 2 but with no filenames we create filenames from tbl and seg numbers */
	enum GMT_enum_alloc alloc_mode;	/* Allocation info [0-3] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

/*============================================================ */
/*============== GMT_TEXTSET Public Declaration ============== */
/*============================================================ */

struct GMT_TEXT_SEGMENT {		/* For holding segment text records in memory */
	/* Variables we document for the API: */
	uint64_t n_rows;		/* Number of rows in this segment */
	char **record;			/* Array of text records */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Segment header (if applicable) */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the table */
	unsigned int mode;		/* 0 = output segment, 1 = output header only, 2 = skip segment */
	size_t n_alloc;			/* Number of rows allocated for this segment */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	char **a_value;			/* The values of the OGR/GMT aspatial fields */	
};

struct GMT_TEXT_TABLE {	/* To hold an array of text segment structures and header information in one container */
	/* Variables we document for the API: */
	unsigned int n_headers;	/* Number of file header records (0 if no header) */
	uint64_t n_segments;	/* Number of segments in the array */
	uint64_t n_records;	/* Total number of data records across all segments */
	char **header;			/* Array with all file header records, if any) */
	struct GMT_TEXT_SEGMENT **segment;	/* Pointer to array of segments */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the table */
	unsigned int mode;		/* 0 = output table, 1 = output header only, 2 = skip table */
	size_t n_alloc;			/* The current allocation length of segments */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
};

struct GMT_TEXTSET {	/* Single container for an array of GMT text tables (files) */
	/* Variables we document for the API: */
	unsigned int n_tables;		/* The total number of tables (files) contained */
	uint64_t n_segments;		/* The total number of segments across all tables */
	uint64_t n_records;		/* The total number of data records across all tables */
	struct GMT_TEXT_TABLE **table;	/* Pointer to array of tables */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;			/* The internal number of the data set */
	size_t n_alloc;			/* The current allocation length of tables */
	enum GMT_enum_dest io_mode;	/* -1 means write OGR format (requires proper -a),
					 * 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments.
					 * 3 is same as 2 but with no filenames we create filenames from tbl and seg numbers */
	enum GMT_enum_alloc alloc_mode;	/* Allocation info [0] */
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

enum GMT_enum_cptflags {GMT_CPT_NO_BNF = 1, GMT_CPT_EXTEND_BNF = 2};

/* Here is the definition of the GMT_PALETTE structure that is used in programs
 * that deals with coloring of items as a function of z-lookup.  Note that rgb
 * arrays have 4 items as the 4th value could be a non-zero transparency (when supported).
 */
 
struct GMT_LUT {
	double z_low, z_high, i_dz;
	double rgb_low[4], rgb_high[4], rgb_diff[4];
	double hsv_low[4], hsv_high[4], hsv_diff[4];
	unsigned int annot;		/* 1 for Lower, 2 for Upper, 3 for Both */
	unsigned int skip;		/* true means skip this slice */
	struct GMT_FILL *fill;	/* Use by grdview */			/* Content not counted by sizeof (struct) */
	char *label;		/* For non-number labels */		/* Content not counted by sizeof (struct) */
};

struct GMT_BFN_COLOR {		/* For back-, fore-, and nan-colors */
	double rgb[4];
	double hsv[4];
	unsigned int skip;		/* true means skip this slice */
	struct GMT_FILL *fill;						/* Content not counted by sizeof (struct) */
};

struct GMT_PALETTE {		/* Holds all pen, color, and fill-related parameters */
	/* Variables we document for the API: */
	unsigned int n_headers;		/* Number of CPT file header records (0 if no header) */
	unsigned int n_colors;		/* Number of colors in CPT lookup table */
	unsigned int cpt_flags;		/* Flags controling use of BFN colors */
	struct GMT_LUT *range;		/* CPT lookup table read by GMT_read_cpt */
	struct GMT_BFN_COLOR patch[3];	/* Structures with back/fore/nan colors */
	char **header;			/* Array with all CPT file header records, if any) */		/* Content not counted by sizeof (struct) */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the data set */
	unsigned int alloc_mode;	/* Allocation info [0] */
	unsigned int model;		/* RGB, HSV, CMYK */
	unsigned int is_gray;			/* true if only grayshades are needed */
	unsigned int is_bw;			/* true if only black and white are needed */
	unsigned int is_continuous;		/* true if continuous color tables have been given */
	unsigned int has_pattern;		/* true if cpt file contains any patterns */
	unsigned int skip;			/* true if current z-slice is to be skipped */
	unsigned int categorical;		/* true if CPT applies to categorical data */
};

/*============================================================ */
/*=============== GMT_IMAGE Public Declaration =============== */
/*============================================================ */

/* The GMT_IMAGE container is used to pass user images in from the GDAL bridge */

struct GMT_IMAGE {	/* Single container for a user image of data */
	/* Variables we document for the API: */
	enum GMT_enum_type type;	/* Data type, e.g. GMT_FLOAT */
	int		*ColorMap;
	struct GRD_HEADER *header;	/* Pointer to full GMT header for the image */
	unsigned char *data;		/* Pointer to actual image */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the data set */
	enum GMT_enum_alloc alloc_mode;	/* Allocation info [0] */
	const char	*ProjRefPROJ4;
	const char	*ProjRefWKT;
	const char	*ColorInterp;
};

/*============================================================ */
/*============= GMT_UNIVECTOR Public Declaration ============= */
/*============================================================ */

/* This union is used to hold any type of array */
union GMT_UNIVECTOR {
	/* Universal vector or any data type can be held here */
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
	unsigned int n_columns;		/* Number of vectors */
	uint64_t n_rows;		/* Number of rows in each vector */
	enum GMT_enum_type *type;	/* Array of data types (type of each uni-vector, e.g. GMT_FLOAT */
	union GMT_UNIVECTOR *data;	/* Array of uni-vectors */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;			/* The internal number of the data set */
	enum GMT_enum_alloc alloc_mode;	/* Allocation info [0 = allocated, 1 = allocate as needed] */
};

/*============================================================ */
/*=============== GMT_MATRIX Public Declaration ============== */
/*============================================================ */

/* These containers are used to pass user vectors and matrices in/out of GMT */

struct GMT_MATRIX {	/* Single container for a user matrix of data */
	/* Variables we document for the API: */
	unsigned int n_rows;		/* Number of rows in this matrix */
	unsigned int n_columns;	/* Number of columns in this matrix */
	unsigned int n_layers;	/* Number of layers in a 3-D matrix [1] */
	unsigned int shape;		/* 0 = C (rows) and 1 = Fortran (cols) */
	unsigned int registration;	/* 0 for gridline and 1 for pixel registration  */
	size_t dim;			/* Allocated length of longest C or Fortran dim */
	size_t size;			/* Byte length of data */
	enum GMT_enum_type type;	/* Data type, e.g. GMT_FLOAT */
	double limit[6];		/* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
	union GMT_UNIVECTOR data;	/* Union with pointer to actual matrix of the chosen type */
/* ---- Variables "hidden" from the API ---- */
	unsigned int id;		/* The internal number of the data set */
	enum GMT_enum_alloc alloc_mode;	/* Allocation info [0] */
};

#endif /* _GMT_RESOURCES_H */
