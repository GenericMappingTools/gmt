/*--------------------------------------------------------------------
 *	$Id: gmt_io.h,v 1.117 2011-07-04 19:14:29 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Table input/output in GMT can be either ascii or binary (where supported)
 * and ASCII tables may consist of single or multiple segments.  When the
 * latter is the case usually there is a -M option to signal this case.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ascii tables.
 *
 * Author:	Paul Wessel
 * Date:	15-NOV-2009
 * Version:	5 API
 *
 */

#ifndef _GMT_IO_H
#define _GMT_IO_H

#ifdef GMT_COMPAT
/* Must add M, m, E, Z, and/or S to the common option processing list */
#define GMT_OPT(opt) opt
#else
#define GMT_OPT(opt) ""
#endif

#define GMT_COLUMN_FORMAT	1	/* 2-D grid is Fortran-style with columns */
#define GMT_ROW_FORMAT		2	/* 2-D grid is C-style with rows */

/* Two different i/o mode: GMT_Put|Get_Data vs GMT_Put|Get_Record */
#define GMT_BY_SET		0	/* Default is to read the entire set */
#define GMT_BY_REC		1	/* Means we will access the registere files on a record-by-record basis */

/* These are the 6 methods for i/o */
#ifdef DEBUG	/* Allow these to be integers so ddd can resolve them: This is to aid our debugging */
enum GMT_methods {GMT_IS_FILE, GMT_IS_STREAM, GMT_IS_FDESC, GMT_IS_COPY, GMT_IS_REF, GMT_IS_READONLY};
#else
#define GMT_IS_FILE		0	/* Entity is a filename */
#define GMT_IS_STREAM		1	/* Entity is an open stream */
#define GMT_IS_FDESC		2	/* Entity is an open file descriptor */
#define GMT_IS_COPY		3	/* Entity is a memory location that should be duplicated */
#define GMT_IS_REF		4	/* Entity is a memory location and we just pass the ref (no copying) */
#define GMT_IS_READONLY		5	/* As GMT_IS_REF, but we are not allowed to change the data in any way. */
#endif
#define GMT_N_METHODS		6	/* Number of methods we recognize */

/* But Grid can come from a GMT grid OR User Matrix, and Data can come from DATASET or via Vectors|Matrix, and Text from TEXTSET or Matrix */

#define GMT_VIA_VECTOR		100
#define GMT_VIA_MATRIX		200

/* These are the 5 families of data types */
#ifdef DEBUG	/* Allow these to be integers so ddd can resolve them: This is to aid our debugging */
enum GMT_families {GMT_IS_DATASET, GMT_IS_TEXTSET, GMT_IS_GRID, GMT_IS_CPT, GMT_IS_IMAGE, GMT_IS_VECTOR, GMT_IS_MATRIX};
#else
#define GMT_IS_DATASET		0	/* Entity is data table */
#define GMT_IS_TEXTSET		1	/* Entity is a Text table */
#define GMT_IS_GRID		2	/* Entity is a GMT grid */
#define GMT_IS_CPT		3	/* Entity is a CPT table */
#define GMT_IS_IMAGE		4	/* Entity is a 1- or 3-layer unsigned char image */
/* These are used internally to hande interfacing with user data types */
#define GMT_IS_VECTOR		5	/* Entity is user vectors */
#define GMT_IS_MATRIX		6	/* Entity is user matrix */
#endif
#define GMT_N_FAMILIES		5	/* Number of families we recognize */

#ifdef DEBUG	/* Allow these to be integers so ddd can resolve them: This is to aid our debugging */
enum GMT_dimensions {GMT_X, GMT_Y, GMT_Z};
#else
#define GMT_X			0	/* x or lon is in 0th column */
#define GMT_Y			1	/* y or lat is in 1st column */
#define GMT_Z			2	/* z is in 2nd column */
#endif
#ifdef DEBUG	/* Allow these to be integers so ddd can resolve them: This is to aid our debugging */
enum GMT_geometries {GMT_IS_TEXT, GMT_IS_POINT, GMT_IS_LINE, GMT_IS_POLY, GMT_IS_SURFACE};
#else
#define GMT_IS_TEXT		0	/* GIS geometries (text here refers to just a line of text with no coordinates) */
#define GMT_IS_POINT		1
#define GMT_IS_LINE		2
#define GMT_IS_POLY		3
#define GMT_IS_SURFACE		4
#endif

/* These are polygon modes */
#define GMT_IS_PERIMETER	0
#define GMT_IS_HOLE		1

#define GMT_polygon_is_hole(S) (S->pol_mode == GMT_IS_HOLE || (S->ogr && S->ogr->pol_mode == GMT_IS_HOLE))

/* Specific feature geometries as obtained from OGR */
/* Note: As far as registering or reading data, GMT only needs to know if data type is POINT, LINE, or POLY */

#define GMT_IS_LINESTRING	2
#define GMT_IS_POLYGON		3
#define GMT_IS_MULTIPOINT	4
#define GMT_IS_MULTILINESTRING	5
#define GMT_IS_MULTIPOLYGON	6

#define GMT_REG_FILES_IF_NONE	1	/* Tell GMT_Init_IO we conditionally want to register all input files in the option list if nothing else is registered */
#define GMT_REG_FILES_ALWAYS	2	/* Tell GMT_Init_IO to always register all input files in the option list */
#define GMT_REG_STD_IF_NONE	4	/* Tell GMT_Init_IO we conditionally want to register std(in|out) if nothing else has been registered */
#define GMT_REG_STD_ALWAYS	8	/* Tell GMT_Init_IO to always register std(in|out) */
#define GMT_REG_DEFAULT		5	/* Tell GMT_Init_IO to register files, and if none are found then std(in|out), but only if nothing was registered before this call */

#define GMT_IO_DONE		0	/* Tell GMT_End_IO we are done but nothing special is to be done. */
#define GMT_IO_ASCII		512	/* Force ASCII mode for reading (ignoring current io settings). */
#define GMT_IO_RESET		32768	/* Tell GMT_End_IO that accessed resources should be made read/write-able again. */
#define GMT_IO_UNREG		16384	/* Tell GMT_End_IO to unregister all accessed resources. */

#define GMT_GRID_ALL		0	/* Read|write both grid header and the entire grid (no subset) */
#define GMT_GRID_HEADER		1	/* Just read|write the grid header */
#define GMT_GRID_DATA		2	/* Read|write the grid array given w/e/s/n set in the header */
#define GMT_GRID_COMPLEX_REAL	4	/* Read|write the real component to/from a complex grid */
#define GMT_GRID_COMPLEX_IMAG	8	/* Read|write the imaginary component to/from a complex grid */
#define GMT_GRID_NO_HEADER	16	/* Write a native grid without the leading grid header */

#define GMT_ALLOCATED		0	/* Item was allocated so GMT_* modules should free when GMT_Destroy_Data is called */
#define GMT_REFERENCE		1	/* Item was not allocated so GMT_* modules should NOT free when GMT_Destroy_Data is called, but may realloc if needed */
#define GMT_READONLY		2	/* Item was not allocated so GMT_* modules should NOT free when GMT_Destroy_Data is called . Consider read-only data */
#define GMT_CLOBBER		3	/* Free item no matter what its allocation status */

#define GMT_READ_DOUBLE		0	/* Read ASCII data record and return double array */
#define GMT_READ_TEXT		1	/* Read ASCII data record and return text string */
#define GMT_READ_MIXED		2	/* Read ASCII data record and return double array but tolerate conversion errors */
#define GMT_FILE_BREAK		4	/* Add to mode to indicate we want to know when each file end is reached [continuous] */

#define GMT_WRITE_DOUBLE	0	/* Write double array to output */
#define GMT_WRITE_TEXT		1	/* Write ASCII current record to output */
#define GMT_WRITE_SEGHEADER	2	/* Write segment header record to output */
#define GMT_WRITE_TBLHEADER	3	/* Write current record as table header to output */

#define GMT_WRITE_OGR		-1	/* Output OGR/GMT format [Requires proper -a setting] */
#define GMT_WRITE_DATASET	0	/* Write all output tables and all their segments to one destination [Default] */
#define GMT_WRITE_TABLES	1	/* Write each output table and all their segments to separate destinations */
#define GMT_WRITE_SEGMENTS	2	/* Write all output tables' segments to separate destinations */
#define GMT_WRITE_TABLE_SEGMENTS	3	/* Same as 2 but if no filenames we use both tbl and seg with format */

#define GMT_ALLOC_NORMAL	0	/* Normal allocation of new dataset based on shape of input dataset */
#define GMT_ALLOC_VERTICAL	1	/* Allocate a single table for data set to hold all input tables by vertical concatenation */
#define GMT_ALLOC_HORIZONTAL	2	/* Alocate a single table for data set to hold all input tables by horizontal (paste) concatenations */

#define GMT_WRITE_NORMAL	0	/* Write header and contents of this entity (table or segment) */
#define GMT_WRITE_HEADER	1	/* Only write header and not the contents of this entity (table or segment) */
#define GMT_WRITE_SKIP		2	/* Entirely skip this entity on output (table or segment) */

/* Codes for aspatial assocation with segment header options -D -G -I -L -T -W -Z */

#define GMT_IS_D	-1
#define GMT_IS_G	-2
#define GMT_IS_I	-3
#define GMT_IS_L	-4
#define GMT_IS_T	-5
#define GMT_IS_W	-6
#define GMT_IS_Z	-7

/* Error return codes */

#define GMT_IO_TBL_HEADER	1	/* Return codes for GMT_ascii_input */
#define GMT_IO_SEG_HEADER	2
#define GMT_IO_MISMATCH		4
#define GMT_IO_EOF		8
#define GMT_IO_NAN		16
#define GMT_IO_GAP		32
#define GMT_IO_NEXT_FILE	64	/* Like EOF except for an individual file (with more files to follow) */

#define GMT_REC_IS_TBL_HEADER(C)	(C->current.io.status & GMT_IO_TBL_HEADER)
#define GMT_REC_IS_SEG_HEADER(C)	(C->current.io.status & GMT_IO_SEG_HEADER)
#define GMT_REC_IS_ANY_HEADER(C)	(C->current.io.status & (GMT_IO_TBL_HEADER | GMT_IO_SEG_HEADER))
#define GMT_REC_IS_ERROR(C)		(C->current.io.status & GMT_IO_MISMATCH)
#define GMT_REC_IS_EOF(C)		(C->current.io.status & GMT_IO_EOF)
#define GMT_REC_IS_NAN(C)		(C->current.io.status & GMT_IO_NAN)
#define GMT_REC_IS_GAP(C)		(C->current.io.status & GMT_IO_GAP)
#define GMT_REC_IS_NEW_SEGMENT(C)	(C->current.io.status & (GMT_IO_SEG_HEADER | GMT_IO_NAN))
#define GMT_REC_IS_LINE_BREAK(C)	(C->current.io.status & (GMT_IO_SEG_HEADER | GMT_IO_EOF | GMT_IO_NAN | GMT_IO_GAP))
#define GMT_REC_IS_FILE_BREAK(C)	(C->current.io.status & GMT_IO_NEXT_FILE)
#define GMT_REC_IS_DATA(C)		(C->current.io.status == 0 || C->current.io.status == GMT_IO_NAN)

/* Array indices for input/output/stderr variables */

#ifdef DEBUG	/* Allow these to be integers so ddd can resolve them: This is to aid our debugging only */
enum GMT_io {GMT_IN, GMT_OUT, GMT_ERR};
#else
#define GMT_IN	0
#define GMT_OUT	1
#define GMT_ERR	2
#endif

/* Get current setting for in/out columns */

#define GMT_get_cols(C,direction) (C->common.b.ncol[direction])

/* Determine if current binary table has header */
#define GMT_binary_header(GMT,dir) (GMT->common.b.active[dir] && GMT->current.io.io_header[dir] && GMT->current.io.io_n_header_items)

/* Types of possible column entries in a file: */

#define GMT_IS_NAN		0	/* Returned by GMT_scanf routines when read fails */
#define GMT_IS_FLOAT		1	/* Generic (double) data type, no special format */
#define GMT_IS_LAT		2
#define GMT_IS_LON		4
#define GMT_IS_GEO		6	/* data type is either Lat or Lon */
#define GMT_IS_RELTIME		8	/* For I/O of data in user units */
#define GMT_IS_ABSTIME		16	/* For I/O of data in calendar+clock units */
#define GMT_IS_RATIME		24	/* To see if time is either Relative or Absolute */
#define GMT_IS_ARGTIME		32	/* To invoke GMT_scanf_argtime()  */
#define GMT_IS_DIMENSION	64	/* A float with [optional] unit suffix, e.g., 7.5c, 0.4i; convert to inch  */
#define GMT_IS_UNKNOWN		128	/* Input type is not knowable without -f */

/* Various ways to report longitudes */

#define GMT_IS_0_TO_P360_RANGE		1	/* Report 0 <= lon <= 360 */
#define GMT_IS_0_TO_P360		2	/* Report 0 <= lon < 360 */
#define GMT_IS_M360_TO_0_RANGE		3	/* Report -360 <= lon <= 0 */
#define GMT_IS_M360_TO_0		4	/* Report -360 < lon <= 0 */
#define GMT_IS_M180_TO_P180_RANGE	5	/* Report -180 <= lon <= +180 */
#define GMT_IS_M180_TO_P180		6	/* Report -180 <= lon < +180 */
#define GMT_IS_M180_TO_P270_RANGE	7	/* Report -180 <= lon < +270 [GSHHS only] */

#ifdef WIN32
/* Functions we have written to handle DLL troubles */
EXTERN_MSC FILE *GMT_fdopen (int handle, const char *mode);
EXTERN_MSC int GMT_fgetc (FILE *stream);
EXTERN_MSC int GMT_ungetc (int c, FILE *stream);
EXTERN_MSC int GMT_fputs (const char *line, FILE *fp);
EXTERN_MSC int GMT_fseek (FILE *stream, long offset, int whence);
EXTERN_MSC long GMT_ftell (FILE *stream);
EXTERN_MSC size_t GMT_fread (void * ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN_MSC size_t GMT_fwrite (const void * ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN_MSC void GMT_rewind (FILE *stream);
EXTERN_MSC int GMT_fprintf (FILE *fp, char *format, ...);
#if 0
EXTERN_MSC int GMT_fscanf (FILE *fp, char *format, ...);
#endif
#else
/* No need for functions; just use text macros */
#define GMT_fdopen(handle, mode) fdopen(handle, mode)
#define GMT_fgetc(stream) fgetc(stream)
#define GMT_ungetc(c, stream) ungetc(c, stream)
#define GMT_fputs(line,fp) fputs(line,fp)
#define GMT_fseek(stream,offset,whence) fseek(stream,offset,whence)
#define GMT_ftell(stream) ftell(stream)
#define GMT_fread(ptr,size,nmemb,stream) fread(ptr,size,nmemb,stream)
#define GMT_fwrite(ptr,size,nmemb,stream) fwrite(ptr,size,nmemb,stream)
#define GMT_rewind(stream) rewind(stream)
#define GMT_fprintf(stream,...) fprintf(stream,__VA_ARGS__)
#if 0
#define GMT_fscanf(stream,...) fscanf(stream,__VA_ARGS__)
#endif
#endif

/* Low-level structures used internally */

struct GMT_QUAD {	/* Counting parameters needed to determine proper longitude min/max range */
	GMT_LONG quad[4];		/* Keeps track if a longitude fell in these quadrants */
	GMT_LONG range[2];		/* The format for reporting longitude */
	double min[2], max[2];		/* Min/max values in either -180/180 or 0/360 counting */
};

struct GMT_CLOCK_IO {
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	GMT_LONG order[3];		/* The relative order of hour, mn, sec in input clock string */
	GMT_LONG n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	GMT_LONG compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	GMT_LONG twelve_hr_clock;	/* TRUE if we are doing am/pm on output */
	char ampm_suffix[2][8];		/* Holds the strings to append am or pm */
	char format[GMT_TEXT_LEN64];	/* Actual C format used to output clock */
	char delimiter[2][2];		/* Delimiter strings in clock, e.g. ":" */
};

struct GMT_DATE_IO {
	GMT_LONG item_order[4];		/* The sequence year, month, day, day-of-year in input calendar string */
	GMT_LONG item_pos[4];		/* Which position year, month, day, day-of-year has in calendar string */
	GMT_LONG Y2K_year;		/* TRUE if we have 2-digit years */
	GMT_LONG truncated_cal_is_ok;	/* TRUE if we have YMD or YJ order so smallest unit is to the right */
	GMT_LONG iso_calendar;		/* TRUE if we do ISO week calendar */
	GMT_LONG day_of_year;		/* TRUE if we do day-of-year rather than month/day */
	GMT_LONG mw_text;		/* TRUE if we must plot the month name or Week rather than a numeral */
	GMT_LONG compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	char format[GMT_TEXT_LEN64];	/* Actual C format used to input/output date */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_GEO_IO {			/* For geographic output and plotting */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	GMT_LONG order[3];		/* The relative order of degree, minute, seconds in form */
	GMT_LONG range;			/* 0 for 0/360, 1 for -360/0, 2 for -180/+180 */
	GMT_LONG decimal;		/* TRUE if we want to use the D_FORMAT for decimal degrees only */
	GMT_LONG wesn;			/* TRUE if we want sign encoded with suffix W, E, S, N */
	GMT_LONG no_sign;		/* TRUE if we want absolute values (plot only) */
	GMT_LONG n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	char x_format[GMT_TEXT_LEN64];	/* Actual C format used to plot/output longitude */
	char y_format[GMT_TEXT_LEN64];	/* Actual C format used to plot/output latitude */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_OGR {	/* Struct with all things GMT/OGR for a table*/
	/* The first parameters are usually set once per data set and do not change */
	GMT_LONG geometry;		/* @G: The geometry of this data set, if known [0 otherwise] */
	GMT_LONG n_aspatial;		/* @T: The number of aspatial fields */
	char *region;			/* @R: The region textstring [NULL if not set] */
	char *proj[4];			/* @J: The 1-4 projection strings [NULL if not set] */
	GMT_LONG *type;			/* @T: The data types of the aspatial fields [NULL if not set]  */
	char **name;			/* @N The names of the aspatial fields [NULL if not set]  */
	/* The following are for OGR data only. It is filled during parsing (current segment) but is then copied to the segment header so it can be accessed later */
	GMT_LONG pol_mode;		/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	char **value;			/* @D: The text values of the current aspatial fields */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_OGR_SEG {	/* Struct with GMT/OGR aspatial data for a segment*/
	GMT_LONG pol_mode;		/* @P: Either GMT_IS_PERIMETER or GMT_IS_HOLE (for polygons only) */
	GMT_LONG n_aspatial;		/* @T: The number of aspatial fields */
	char **value;			/* @D: The values of the current aspatial fields (uses GMT_OGR's n_aspatial as length) */
	double *dvalue;			/* @D: Same but converted to double (assumed possible) */
};

struct GMT_COL_INFO {	/* Used by -i and input parsing */
	GMT_LONG col;		/* The column number in the order requested via -i */
	GMT_LONG order;		/* The initial order (0,1,...) but this will be sorted on col */
	GMT_LONG convert;	/* TRUE if we must convert the data by log10, scale, offset */
	double scale;		/* Multiplier for raw in value */
	double offset;		/* Offset applied after multiplier */ 
};

struct GMT_COL_TYPE {	/* Used by -b for binary formatting */
	GMT_LONG type;		/* Data type e.g., GMT_FLOAT_TYPE */
	GMT_LONG skip;		/* Rather than read/write an item, jump skip bytes */
	PFL io;			/* Pointer to the correct read or write function given type/swab */
};

struct GMT_IO {				/* Used to process input data records */
	
	PFL input;			/* Pointer to function reading ascii or binary tables */
	PFL output;			/* Pointer to function writing ascii or binary tables */
	PFL read_item;			/* Pointer to function reading 1-col z tables in grd2xyz */
	PFL write_item;			/* Pointer to function writing 1-col z tables in xyz2grd */
	PFL ogr_parser;			/* Set to handle either header or data OGR records */

	GMT_LONG pad[4];		/* pad[0] = west, pad[1] = east, pad[2] = south, pad[3] = north */
	GMT_LONG inc_code[2];
	double curr_rec[GMT_MAX_COLUMNS];	/* The most recently processed data record */
	double prev_rec[GMT_MAX_COLUMNS];	/* The previous data record */
	struct GMT_GRD_INFO grd_info;

	GMT_LONG multi_segments[2];	/* TRUE if current Ascii input/output file has multiple segments */
	GMT_LONG io_header[2];		/* TRUE if input/output data has header records */
	GMT_LONG skip_bad_records;	/* TRUE if records where x and/or y are NaN or Inf */
	GMT_LONG give_report;		/* TRUE if functions should report how many bad records were skipped */
	GMT_LONG skip_duplicates;	/* TRUE if we should ignore duplicate x,y records */
	GMT_LONG io_nan_ncols;		/* Number of columns to consider for -s option */

	GMT_LONG file_no;		/* Number of current file */
	GMT_LONG io_n_header_items;	/* number of header records (ascii) or bytes (binary) [0] */
	GMT_LONG seg_no;		/* Number of current multi-segment in entire data set */
	GMT_LONG rec_no;		/* Number of current records (counts headers etc) in entire data set */
	GMT_LONG tbl_no;		/* Number of current table in entire data set */
	GMT_LONG seg_in_tbl_no;		/* Number of current multi-segment in current table */
	GMT_LONG rec_in_tbl_no;		/* Number of current record (counts headers etc) in current table */
	GMT_LONG pt_no;			/* Number of current valid points in a row  */
	GMT_LONG curr_pos[2][3];	/* Keep track of current input/output table, segment, and row (for rec-by-rec action) */
	GMT_LONG n_clean_rec;		/* Number of clean records read (not including skipped records or comments or blanks) */
	GMT_LONG n_bad_records;		/* Number of bad records encountered during i/o */
	GMT_LONG ogr;			/* Tells us if current input source has OGR/GMT metadata (1) or not (0) or not set (-1) */
	GMT_LONG status;		/* 0	All is ok
					   1	Current record is segment header
					   2	Mismatch between actual and expected fields
					   4	EOF
					   8	NaNs encountered in first 2/3 cols */
	char r_mode[4];			/* Current file opening mode for reading (r or rb) */
	char w_mode[4];			/* Current file opening mode for writing (w or wb) */
	char a_mode[4];			/* Current file append mode for writing (a+ or ab+) */
	char current_record[GMT_BUFSIZ];	/* Current ascii record */
	char segment_header[GMT_BUFSIZ];	/* Current ascii segment header */
	char current_filename[2][GMT_BUFSIZ];	/* Current filenames (or <stdin>/<stdout>) */
	char *o_format[GMT_MAX_COLUMNS];	/* Custom output ascii format to overrule format_float_out */
	int ncid;			/* NetCDF file ID (when opening netCDF file) */
	int nvars;			/* Number of requested variables in netCDF file */
	size_t ndim;			/* Length of the column dimension */
	size_t nrec;			/* Record count */
	struct GMT_DATE_IO date_input;	/* Has all info on how to decode input dates */
	struct GMT_DATE_IO date_output;	/* Has all info on how to write output dates */
	struct GMT_CLOCK_IO clock_input;	/* Has all info on how to decode input clocks */
	struct GMT_CLOCK_IO clock_output;	/* Has all info on how to write output clocks */
	struct GMT_GEO_IO geo;		/* Has all the info on how to write geographic coordinates */
	GMT_LONG skip_if_NaN[GMT_MAX_COLUMNS];	/* TRUE if column j cannot be NaN and we must skip the record */
	GMT_LONG col_type[2][GMT_MAX_COLUMNS];	/* Type of column on input and output: Time, geographic, etc, see GMT_IS_<TYPE> */
	GMT_LONG col_skip[GMT_MAX_COLUMNS];	/* TRUE of input column is to be ignored [Default reads all columns, but see -i] */
	GMT_LONG io_nan_col[GMT_MAX_COLUMNS];	/* Array of columns to consider for -s option ir TRUE */
	struct GMT_COL_INFO col[2][GMT_MAX_COLUMNS];	/* Order of columns on input and output unless 0,1,2,3,... */
	struct GMT_COL_TYPE fmt[2][GMT_MAX_COLUMNS];	/* Formatting information for binary data */
	struct GMT_OGR *OGR;		/* Pointer to GMT/OGR info used during reading */
	/* The remainder are just pointers to memory allocated elsewhere */
	int *varid;			/* Array of variable IDs */
	double *scale_factor;		/* Array of scale factors */
	double *add_offset;		/* Array of offsets */
	double *missing_value;		/* Array of missing values */
};

struct GMT_Z_IO {		/* Used when processing z(x,y) table input when (x,y) is implicit */
	GMT_LONG swab;		/* TRUE if we must swap byte-order */
	GMT_LONG x_missing;	/* 1 if a periodic (right) column is implicit (i.e., not stored) */
	GMT_LONG y_missing;	/* 1 if a periodic (top) row is implicit (i.e., not stored) */
	GMT_LONG binary;		/* TRUE if we are reading/writing binary data */
	GMT_LONG input;		/* TRUE if we are reading, FALSE if we are writing */
	GMT_LONG format;	/* Either GMT_COLUMN_FORMAT or GMT_ROW_FORMAT */
	GMT_LONG x_step;	/* +1 if logical x values increase to right, else -1 */
	GMT_LONG y_step;	/* +1 if logical y values increase upwards, else -1 */
	GMT_LONG skip;		/* Number of bytes to skip before reading data */
	GMT_LONG x_period;	/* length of a row in the input data ( <= nx, see x_missing) */
	GMT_LONG y_period;	/* length of a col in the input data ( <= ny, see y_missing) */
	GMT_LONG start_col;	/* First logical column in file */
	GMT_LONG start_row;	/* First logical row in file */
	GMT_LONG n_expected;	/* Number of data element expected to be read */
	GMT_LONG gmt_i;		/* Current column number in the GMT registered grid */
	GMT_LONG gmt_j;		/* Current row number in the GMT registered grid */
	PFL get_gmt_ij;		/* Pointer to function that converts running number to GMT ij */
};

struct GMT_PARSE_Z_IO {	/* -Z[<flags>] */
	GMT_LONG active;
	GMT_LONG swab;
	GMT_LONG repeat[2];
	GMT_LONG skip;
	char type;
	char format[2];
};

struct GMT_PLOT_CALCLOCK {
	struct GMT_DATE_IO date;
	struct GMT_CLOCK_IO clock;
	struct GMT_GEO_IO geo;
};

/* Here are the GMT data types used for tables */

struct GMT_LINE_SEGMENT {		/* For holding segment lines in memory */
	GMT_LONG id;			/* The internal number of the table */
	GMT_LONG n_rows;		/* Number of points in this segment */
	GMT_LONG n_columns;		/* Number of fields in each record (>= 2) */
	GMT_LONG pole;			/* Spherical polygons only: If it encloses the S (-1) or N (+1) pole, or none (0) */
	GMT_LONG mode;			/* 0 = output segment, 1 = output header only, 2 = skip segment */
	GMT_LONG range;			/* 0 = use default lon adjustment, -1 = negative longs, +1 = positive lons */
	GMT_LONG pol_mode;		/* Either GMT_IS_PERIMETER  [-Pp] or GMT_IS_HOLE [-Ph] (for polygons only) */
	GMT_LONG n_alloc;		/* The current allocation length of each coord */
	double dist;			/* Distance from a point to this feature */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	double **coord;			/* Coordinates x,y, and possibly other columns */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Segment header (if applicable) */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	struct GMT_OGR_SEG *ogr;	/* NULL unless OGR/GMT metadata exist for this segment */
	struct GMT_LINE_SEGMENT *next;	/* NULL unless polygon and has holes and pointing to next hole */
};

struct GMT_TABLE {	/* To hold an array of line segment structures and header information in one container */
	GMT_LONG id;			/* The internal number of the table */
	GMT_LONG n_headers;		/* Number of file header records (0 if no header) */
	GMT_LONG n_segments;		/* Number of segments in the array */
	GMT_LONG n_records;		/* Total number of data records across all segments */
	GMT_LONG n_columns;		/* Number of columns (fields) in each record */
	GMT_LONG mode;			/* 0 = output table, 1 = output header only, 2 = skip table */
	GMT_LONG n_alloc;		/* The current allocation length of segments */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	char **header;			/* Array with all file header records, if any) */
	struct GMT_LINE_SEGMENT **segment;	/* Pointer to array of segments */
	struct GMT_OGR *ogr;		/* Pointer to struct with all things GMT/OGR (if MULTI-geometry and not MULTIPOINT) */
};

struct GMT_TEXT_SEGMENT {		/* For holding segment text records in memory */
	GMT_LONG id;			/* The internal number of the table */
	GMT_LONG n_rows;		/* Number of rows in this segment */
	GMT_LONG n_alloc;		/* Number of rows allocated for this segment */
	GMT_LONG mode;			/* 0 = output segment, 1 = output header only, 2 = skip segment */
	char **record;			/* Array of text records */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Segment header (if applicable) */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	char **a_value;			/* The values of the OGR/GMT aspatial fields */	
};

struct GMT_TEXT_TABLE {	/* To hold an array of text segment structures and header information in one container */
	GMT_LONG id;			/* The internal number of the table */
	GMT_LONG n_headers;		/* Number of file header records (0 if no header) */
	GMT_LONG n_segments;		/* Number of segments in the array */
	GMT_LONG n_records;		/* Total number of data records across all segments */
	GMT_LONG mode;			/* 0 = output table, 1 = output header only, 2 = skip table */
	GMT_LONG n_alloc;		/* The current allocation length of segments */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	char **header;			/* Array with all file header records, if any) */
	struct GMT_TEXT_SEGMENT **segment;	/* Pointer to array of segments */
};

struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
	GMT_LONG id;			/* The internal number of the data set */
	GMT_LONG n_tables;		/* The total number of tables (files) contained */
	GMT_LONG n_segments;		/* The total number of segments across all tables */
	GMT_LONG n_records;		/* The total number of data records across all tables */
	GMT_LONG n_columns;		/* The number of data columns */
	GMT_LONG io_mode;		/* 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments. */
	GMT_LONG n_alloc;		/* The current allocation length of tables */
	GMT_LONG alloc_mode;		/* Allocation info [0] */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	struct GMT_TABLE **table;	/* Pointer to array of tables */
};

struct GMT_TEXTSET {	/* Single container for an array of GMT text tables (files) */
	GMT_LONG id;			/* The internal number of the data set */
	GMT_LONG n_tables;		/* The total number of tables (files) contained */
	GMT_LONG n_segments;		/* The total number of segments across all tables */
	GMT_LONG n_records;		/* The total number of data records across all tables */
	GMT_LONG io_mode;		/* 0 means write everything to one destination [Default],
					 * 1 means use table->file[GMT_OUT] to write separate table,
					 * 2 means use segment->file[GMT_OUT] to write separate segments. */
	GMT_LONG n_alloc;		/* The current allocation length of tables */
	GMT_LONG alloc_mode;		/* Allocation info [0] */
	char *file[2];			/* Name of file or source [0 = in, 1 = out] */
	struct GMT_TEXT_TABLE **table;	/* Pointer to array of tables */
};


/* These container is used to pass user images in from the GDAL bridge */

struct GMT_IMAGE {	/* Single container for a user image of data */
	GMT_LONG id;			/* The internal number of the data set */
	GMT_LONG n_rows;		/* Number of rows in this image */
	GMT_LONG n_columns;		/* Number of columns in this image */
	GMT_LONG n_bands;		/* Number of bands in a 1,3|4-D image [1] */
	GMT_LONG dim;			/* Allocated length of longest C or Fortran dim */
	GMT_LONG shape;			/* 0 = C (rows) and 1 = Fortran (cols) */
	GMT_LONG type;			/* Data type, e.g. GMTAPI_FLOAT */
	GMT_LONG registration;     	/* 0 for gridline and 1 for pixel registration  */
	GMT_LONG size;			/* Byte length of data */
	GMT_LONG alloc_mode;		/* Allocation info [0] */
	int		*ColorMap;
	const char	*ProjRefPROJ4;
	const char	*ProjRefWKT;
	const char	*ColorInterp;
	struct GRD_HEADER *header;	/* Pointer to full GMT header for the image */
	double limit[6];		/* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
	unsigned char *data;		/* Pointer to actual image */
};

/* These containers are used to pass user vectors and matrices in/out of GMT */

struct GMT_MATRIX {	/* Single container for a user matrix of data */
	GMT_LONG id;			/* The internal number of the data set */
	GMT_LONG n_rows;		/* Number of rows in this matrix */
	GMT_LONG n_columns;		/* Number of columns in this matrix */
	GMT_LONG n_layers;		/* Number of layers in a 3-D matrix [1] */
	GMT_LONG dim;			/* Allocated length of longest C or Fortran dim */
	GMT_LONG shape;			/* 0 = C (rows) and 1 = Fortran (cols) */
	GMT_LONG type;			/* Data type, e.g. GMTAPI_FLOAT */
	GMT_LONG registration;     	/* 0 for gridline and 1 for pixel registration  */
	GMT_LONG size;			/* Byte length of data */
	GMT_LONG alloc_mode;		/* Allocation info [0] */
	double limit[6];		/* Contains xmin/xmax/ymin/ymax[/zmin/zmax] */
	void *data;			/* Opaque pointer to actual matrix */
};

struct GMT_VECTOR {	/* Single container for user vector(s) of data */
	GMT_LONG id;			/* The internal number of the data set */
	GMT_LONG n_rows;		/* Number of rows in each vector */
	GMT_LONG n_columns;		/* Number of vectors */
	GMT_LONG *type;			/* Data type of each vector, e.g. GMTAPI_FLOAT */
	GMT_LONG alloc_mode;		/* Allocation info [0 = allocated, 1 = allocate as needed] */
	void **data;			/* Opaque pointers to actual vectors */
};
	
#endif /* _GMT_IO_H */
