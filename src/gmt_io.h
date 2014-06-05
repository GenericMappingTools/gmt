/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

/* Must add M, m, E, Z, and/or S to the common option processing list */
#define GMT_OPT(opt) opt

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

#define GMT_polygon_is_hole(S) (S->pol_mode == GMT_IS_HOLE || (S->ogr && S->ogr->pol_mode == GMT_IS_HOLE))

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

/* Macros to simplify check for return status */
#define GMT_REC_IS_TABLE_HEADER(C)	(C->current.io.status & GMT_IO_TABLE_HEADER)
#define GMT_REC_IS_SEGMENT_HEADER(C)	(C->current.io.status & GMT_IO_SEGMENT_HEADER)
#define GMT_REC_IS_ANY_HEADER(C)	(C->current.io.status & GMT_IO_ANY_HEADER)
#define GMT_REC_IS_ERROR(C)		(C->current.io.status & GMT_IO_MISMATCH)
#define GMT_REC_IS_EOF(C)		(C->current.io.status & GMT_IO_EOF)
#define GMT_REC_IS_NAN(C)		(C->current.io.status & GMT_IO_NAN)
#define GMT_REC_IS_GAP(C)		(C->current.io.status & GMT_IO_GAP)
#define GMT_REC_IS_NEW_SEGMENT(C)	(C->current.io.status & GMT_IO_NEW_SEGMENT)
#define GMT_REC_IS_LINE_BREAK(C)	(C->current.io.status & GMT_IO_LINE_BREAK)
#define GMT_REC_IS_FILE_BREAK(C)	(C->current.io.status & GMT_IO_NEXT_FILE)
#define GMT_REC_IS_DATA(C)		(C->current.io.status == 0 || C->current.io.status == GMT_IO_NAN)

/* Get current setting for in/out columns */

/* Determine if current binary table has header */
#define GMT_binary_header(GMT,dir) (GMT->common.b.active[dir] && GMT->current.setting.io_header[dir] && GMT->current.setting.io_n_header_items)

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
	GMT_IS_M180_TO_P270_RANGE		= 7};	/* Report -180 <= lon < +270 [GSHHG only] */

/* How to handle NaNs in records */

enum GMT_io_nan_enum {
	GMT_IO_NAN_OK = 0,	/* NaNs are fine; just ouput the record as is */
	GMT_IO_NAN_SKIP,	/* -s[cols]	: Skip records with z == NaN in selected cols [z-col only] */
	GMT_IO_NAN_KEEP,	/* -sr		: Skip records with z != NaN */
	GMT_IO_NAN_ONE};	/* -sa		: Skip records with at least one NaN */

/* Use POSIX functions ftello() and fseeko(), which represent the
 * position using the off_t type: */
#ifdef HAVE_FSEEKO
#	define fseek fseeko
#endif

#ifdef HAVE_FTELLO
#	define ftell ftello
#endif

/* Windows 64-bit file access */
#if defined HAVE__FSEEKI64 && defined HAVE__FTELLI64
#	define fseek _fseeki64
#	define ftell _ftelli64
#	ifndef SIZEOF_OFF_T
		typedef __int64 off_t;
#	else
#		define off_t __int64
#	endif /* SIZEOF_OFF_T */
#elif !defined SIZEOF_OFF_T /* HAVE__FSEEKI64 && HAVE__FTELLI64 */
	typedef long off_t;
#endif /* HAVE__FSEEKI64 && HAVE__FTELLI64 */

#define GMT_fdopen(handle, mode) fdopen(handle, mode)
#define GMT_fgetc(stream) fgetc(stream)
#define GMT_ungetc(c, stream) ungetc(c, stream)
#define GMT_fputs(line,fp) fputs(line,fp)
#define GMT_fread(ptr,size,nmemb,stream) fread(ptr,size,nmemb,stream)
#define GMT_fwrite(ptr,size,nmemb,stream) fwrite(ptr,size,nmemb,stream)
#define GMT_rewind(stream) rewind(stream)

/* Low-level structures used internally */

struct GMT_QUAD {	/* Counting parameters needed to determine proper longitude min/max range */
	uint64_t quad[4];		/* Keeps track if a longitude fell in these quadrants */
	unsigned int range[2];	/* The format for reporting longitude */
	double min[2], max[2];		/* Min/max values in either -180/180 or 0/360 counting */
};

struct GMT_CLOCK_IO {
	bool skip;			/* Only true if a format string was pass as NULL */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	int order[3];		/* The relative order of hour, mn, sec in input clock string (-ve if unused) */
	unsigned int n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	bool compact;		/* true if we do not want leading zeros in items (e.g., 03) */
	bool twelve_hr_clock;	/* true if we are doing am/pm on output */
	char ampm_suffix[2][8];		/* Holds the strings to append am or pm */
	char format[GMT_LEN64];	/* Actual C format used to output clock */
	char delimiter[2][2];		/* Delimiter strings in clock, e.g. ":" */
};

struct GMT_DATE_IO {
	bool skip;			/* Only true if a format string was pass as NULL */
	bool watch;			/* Only true if input format has month last and is monthname */
	int item_order[4];		/* The sequence year, month, day, day-of-year in input calendar string (-ve if unused) */
	int item_pos[4];		/* Which position year, month, day, day-of-year has in calendar string (-ve if unused) */
	bool Y2K_year;		/* true if we have 2-digit years */
	bool truncated_cal_is_ok;	/* true if we have YMD or YJ order so smallest unit is to the right */
	bool iso_calendar;		/* true if we do ISO week calendar */
	bool day_of_year;		/* true if we do day-of-year rather than month/day */
	bool mw_text;		/* true if we must plot the month name or Week rather than a numeral */
	bool compact;		/* true if we do not want leading zeros in items (e.g., 03) */
	char format[GMT_LEN64];	/* Actual C format used to input/output date */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_GEO_IO {			/* For geographic output and plotting */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	unsigned int n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	unsigned int range;		/* 0 for 0/360, 1 for -360/0, 2 for -180/+180 */
	unsigned int wesn;		/* 1 if we want sign encoded with suffix W, E, S, N, 2 if also want space before letter */
	int order[3];			/* The relative order of degree, minute, seconds in form (-ve if unused) */
	bool decimal;			/* true if we want to use the D_FORMAT for decimal degrees only */
	bool no_sign;			/* true if we want absolute values (plot only) */
	char x_format[GMT_LEN64];	/* Actual C format used to plot/output longitude */
	char y_format[GMT_LEN64];	/* Actual C format used to plot/output latitude */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_COL_INFO {	/* Used by -i and input parsing */
	unsigned int col;	/* The column number in the order requested via -i */
	unsigned int order;	/* The initial order (0,1,...) but this will be sorted on col */
	unsigned int convert;	/* 2 if we must convert the data by log10, 1 if scale, offset */
	double scale;		/* Multiplier for raw in value */
	double offset;		/* Offset applied after multiplier */ 
};

struct GMT_COL_TYPE {	/* Used by -b for binary formatting */
	unsigned int type;	/* Data type e.g., GMT_FLOAT */
	off_t skip;		/* Rather than read/write an item, jump |skip| bytes before (-ve) or after (+ve) read/write */
	int (*io) (struct GMT_CTRL *, FILE *, uint64_t, double *);	/* Pointer to the correct read or write function given type/swab */
};

struct GMT_IO {				/* Used to process input data records */
	void * (*input) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to function reading ascii or binary tables */
	int (*output) (struct GMT_CTRL *, FILE *, uint64_t, double *);	/* Pointer to function writing ascii or binary tables */
	int (*read_item) (struct GMT_CTRL *, FILE *, uint64_t, double *);		/* Pointer to function reading 1-col z tables in grd2xyz */
	int (*write_item) (struct GMT_CTRL *, FILE *, uint64_t, double *);		/* Pointer to function writing 1-col z tables in xyz2grd */
	bool (*ogr_parser) (struct GMT_CTRL *, char *);				/* Set to handle either header or data OGR records */

	unsigned int pad[4];		/* pad[0] = west, pad[1] = east, pad[2] = south, pad[3] = north */
	unsigned int inc_code[2];
	double curr_rec[GMT_MAX_COLUMNS];	/* The most recently processed data record */
	double prev_rec[GMT_MAX_COLUMNS];	/* The previous data record */
	struct GMT_GRID_INFO grd_info;

	bool multi_segments[2];	/* true if current Ascii input/output file has multiple segments */
	bool skip_bad_records;	/* true if records where x and/or y are NaN or Inf */
	bool give_report;		/* true if functions should report how many bad records were skipped */
	bool skip_duplicates;	/* true if we should ignore duplicate x,y records */
	bool read_mixed;		/* true if we are reading ascii x y [z] [variable numbers of text] */
	bool need_previous;		/* true if when parsing a record we need access to previous record values (e.g., for gap or duplicate checking) */
	bool warn_geo_as_cartesion;	/* true if we should warn if we read a record with geographic data while the expected format has not been set (i.e., no -J or -fg) */

	uint64_t seg_no;		/* Number of current multi-segment in entire data set */
	uint64_t seg_in_tbl_no;		/* Number of current multi-segment in current table */
	uint64_t n_clean_rec;		/* Number of clean records read (not including skipped records or comments or blanks) */
	uint64_t n_bad_records;		/* Number of bad records encountered during i/o */
	unsigned int tbl_no;		/* Number of current table in entire data set */
	unsigned int io_nan_ncols;	/* Number of columns to consider for -s option */
	enum GMT_ogr_status ogr;	/* Tells us if current input source has OGR/GMT metadata (GMT_OGR_TRUE) or not (GMT_OGR_FALSE) or not set (GMT_OGR_UNKNOWN) */
	unsigned int status;		/* 0	All is ok
					   1	Current record is segment header
					   2	Mismatch between actual and expected fields
					   4	EOF
					   8	NaNs encountered in first 2/3 cols */
	uint64_t rec_no;		/* Number of current records (counts headers etc) in entire data set */
	uint64_t rec_in_tbl_no;		/* Number of current record (counts headers etc) in current table */
	uint64_t pt_no;			/* Number of current valid points in a row  */
	uint64_t curr_pos[2][3];	/* Keep track of current input/output table, segment, and row (for rec-by-rec action) */
	char r_mode[4];			/* Current file opening mode for reading (r or rb) */
	char w_mode[4];			/* Current file opening mode for writing (w or wb) */
	char a_mode[4];			/* Current file append mode for writing (a+ or ab+) */
	char current_record[GMT_BUFSIZ];	/* Current ascii record */
	char segment_header[GMT_BUFSIZ];	/* Current ascii segment header */
	char current_filename[2][GMT_BUFSIZ];	/* Current filenames (or <stdin>/<stdout>) */
	char *o_format[GMT_MAX_COLUMNS];	/* Custom output ascii format to overrule format_float_out */
	int ncid;			/* NetCDF file ID (when opening netCDF file) */
	int nvars;			/* Number of requested variablesin netCDF file */
	uint64_t ncols;			/* Number of total columns in netCDF file */
	size_t t_index[GMT_MAX_COLUMNS][5];		/* Indices for cross-sections (netCDF only) */
	size_t count[GMT_MAX_COLUMNS][5];		/* Count used for cross-sections (netCDF only) */
	size_t ndim;			/* Length of the column dimension */
	size_t nrec;			/* Record count */
	struct GMT_DATE_IO date_input;	/* Has all info on how to decode input dates */
	struct GMT_DATE_IO date_output;	/* Has all info on how to write output dates */
	struct GMT_CLOCK_IO clock_input;	/* Has all info on how to decode input clocks */
	struct GMT_CLOCK_IO clock_output;	/* Has all info on how to write output clocks */
	struct GMT_GEO_IO geo;		/* Has all the info on how to write geographic coordinates */
	bool skip_if_NaN[GMT_MAX_COLUMNS];	/* true if column j cannot be NaN and we must skip the record */
	bool col_skip[GMT_MAX_COLUMNS];	/* true of input column is to be ignored [Default reads all columns, but see -i] */
	unsigned int col_type[2][GMT_MAX_COLUMNS];	/* Type of column on input and output: Time, geographic, etc, see GMT_IS_<TYPE> */
	unsigned int io_nan_col[GMT_MAX_COLUMNS];	/* Array of columns to consider for -s option ir true */
	struct GMT_COL_INFO col[2][GMT_MAX_COLUMNS];	/* Order of columns on input and output unless 0,1,2,3,... */
	struct GMT_COL_TYPE fmt[2][GMT_MAX_COLUMNS];	/* Formatting information for binary data */
	struct GMT_OGR *OGR;		/* Pointer to GMT/OGR info used during reading */
	/* The remainder are just pointers to memory allocated elsewhere */
	int *varid;			/* Array of variable IDs (netCDF only) */
	double *scale_factor;		/* Array of scale factors (netCDF only) */
	double *add_offset;		/* Array of offsets (netCDF only) */
	double *missing_value;		/* Array of missing values (netCDF only) */
};

struct GMT_Z_IO {		/* Used when processing z(x,y) table input when (x,y) is implicit */
	bool swab;		/* true if we must swap byte-order */
	bool binary;		/* true if we are reading/writing binary data */
	bool input;		/* true if we are reading, false if we are writing */
	int x_step;	/* +1 if logical x values increase to right, else -1 */
	int y_step;	/* +1 if logical y values increase upwards, else -1 */
	unsigned int x_missing;	/* 1 if a periodic (right) column is implicit (i.e., not stored) */
	unsigned int y_missing;	/* 1 if a periodic (top) row is implicit (i.e., not stored) */
	unsigned int format;	/* Either GMT_IS_COL_FORMAT or GMT_IS_ROW_FORMAT */
	unsigned int x_period;	/* length of a row in the input data ( <= nx, see x_missing) */
	unsigned int y_period;	/* length of a col in the input data ( <= ny, see y_missing) */
	unsigned int start_col;	/* First logical column in file */
	unsigned int start_row;	/* First logical row in file */
	unsigned int gmt_i;		/* Current column number in the GMT registered grid */
	unsigned int gmt_j;		/* Current row number in the GMT registered grid */
	uint64_t n_expected;	/* Number of data element expected to be read */
	off_t skip;		/* Number of bytes to skip before reading data */
	uint64_t (*get_gmt_ij) (struct GMT_Z_IO *, struct GMT_GRID *, uint64_t);	/* Pointer to function that converts running number to GMT ij */
};

struct GMT_PARSE_Z_IO {	/* -Z[<flags>] */
	bool active;		/* true if selected */
	bool not_grid;		/* false if binary data file is a grid so organization matters */
	bool repeat[2];		/* true if periodic in x|y and repeating row/col is missing */
	enum GMT_swap_direction swab;	/* k_swap_none = no byte swapping, k_swap_inswaps input, k_swap_out swaps output, combine to swap both */
	off_t skip;		/* Initial bytes to skip before reading */
	char type;		/* Data type flag A|a|c|u|h|H|i|I|l|L|f|d */
	char format[2];		/* 2-char code describing row/col organization for grids */
};

struct GMT_PLOT_CALCLOCK {
	struct GMT_DATE_IO date;
	struct GMT_CLOCK_IO clock;
	struct GMT_GEO_IO geo;
};

/* Byteswap widths used with gmt_byteswap_file */
typedef enum {
	Int16len = 2,
	Int32len = 4,
	Int64len = 8
} SwapWidth;

/* For the GMT_GRID container, see gmt_grdio.h */

#endif /* _GMT_IO_H */
