/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Table input/output in GMT can be either ASCII or binary (where supported)
 * and ASCII tables may consist of single or multiple segments.  When the
 * latter is the case usually there is a -M option to signal this case.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ASCII tables.
 *
 * Author:	Paul Wessel
 * Date:	15-NOV-2009
 * Version:	6 API
 *
 */

/*!
 * \file gmt_io.h
 * \brief
 */

#ifndef GMT_IO_H
#define GMT_IO_H

#ifdef HAVE_SETLOCALE
#	include <locale.h>
#endif

static inline const char* __gmt_token_separators (unsigned int skip_comma) {
	static const char separators[] = ",; \t";
#ifdef HAVE_SETLOCALE
	struct lconv *lc = localeconv();
	if (skip_comma || (strcmp (lc->decimal_point, ",") == 0) )
		return separators + 1; /* Omit comma */
#endif
	return separators;
}
#define GMT_TOKEN_SEPARATORS __gmt_token_separators(0) /* Data columns may be separated by any of these characters */
#define GMT_TOKEN_SEPARATORS_PSTEXT __gmt_token_separators(1) /* No comma if pstext and fonts are in input records */

/* Must add M, m, E, Z, and/or S to the common option processing list */
#define GMT_OPT(opt) opt

/*! Three different i/o status: unused, actively using, or used */
enum GMT_enum_status {
	GMT_IS_UNUSED = 0,	/* We have not yet read from/written to this resource */
	GMT_IS_USING,		/* Means we have started reading from/writing to this file */
	GMT_IS_USED};		/* Means we are done reading from/writing to this file */

/*! There are three GMT/OGR status values */
enum GMT_ogr_status {
	GMT_OGR_UNKNOWN = -1,	/* We have not parsed enough records to know yet */
	GMT_OGR_FALSE,		/* This is NOT a GMT/OGR file */
	GMT_OGR_TRUE};		/* This is a GMT/OGR file */

#define gmt_M_polygon_is_hole(S) (S->pol_mode == GMT_IS_HOLE || (S->ogr && S->ogr->pol_mode == GMT_IS_HOLE))

/*! Codes for aspatial association with segment header options: */
enum GMT_enum_segopt {
	/*! -D */	GMT_IS_D = -1,
	/*! -G */	GMT_IS_G = -2,
	/*! -I */	GMT_IS_I = -3,
	/*! -L */	GMT_IS_L = -4,
	/*! -T */	GMT_IS_T = -5,
	/*! -W */	GMT_IS_W = -6,
	/*! -Z */	GMT_IS_Z = -7};

/* Macros to simplify check for return status */
#define gmt_M_rec_is_table_header(C)	(C->current.io.status & GMT_IO_TABLE_HEADER)
#define gmt_M_rec_is_segment_header(C)	(C->current.io.status & GMT_IO_SEGMENT_HEADER)
#define gmt_M_rec_is_any_header(C)	(C->current.io.status & GMT_IO_ANY_HEADER)
#define gmt_M_rec_is_error(C)		(C->current.io.status & GMT_IO_MISMATCH)
#define gmt_M_rec_is_eof(C)		(C->current.io.status & GMT_IO_EOF)
#define gmt_M_rec_is_nan(C)		(C->current.io.status & GMT_IO_NAN)
#define gmt_M_rec_is_gap(C)		(C->current.io.status & GMT_IO_GAP)
#define gmt_M_rec_is_new_segment(C)	(C->current.io.status & GMT_IO_NEW_SEGMENT)
#define gmt_M_rec_is_line_break(C)	(C->current.io.status & GMT_IO_LINE_BREAK)
#define gmt_M_rec_is_file_break(C)	(C->current.io.status & GMT_IO_NEXT_FILE)
#define gmt_M_rec_is_data(C)		(C->current.io.status == 0 || C->current.io.status == GMT_IO_NAN)

/* Get current setting for in/out columns */

/*! Types of possible column entries in a file: */
enum gmt_col_enum {
	GMT_IS_NAN   		=   0,	/* Returned by gmt_scanf routines when read fails */
	GMT_IS_FLOAT		=   1,	/* Generic (double) data type, no special format */
	GMT_IS_LAT		=    2,
	GMT_IS_LON		=    4,
	GMT_IS_GEO		=    6,	/* data type is either Lat or Lon */
	GMT_IS_RELTIME		=    8,	/* For I/O of data in user units */
	GMT_IS_ABSTIME		=   16,	/* For I/O of data in calendar+clock units */
	GMT_IS_RATIME		=   24,	/* To see if time is either Relative or Absolute */
	GMT_IS_ARGTIME		=   32,	/* To invoke gmt_scanf_argtime()  */
	GMT_IS_DURATION		=   64,	/* For elapsed time */
	GMT_IS_DIMENSION	=  128,	/* A float with [optional] unit suffix, e.g., 7.5c, 0.4i; convert to inch  */
	GMT_IS_GEODIMENSION	=  256,	/* A float with [optional] geo-distance unit suffix, e.g., 7.5n, 0.4d; convert to km  */
	GMT_IS_AZIMUTH		=  512,	/* An angle to be converted via map projection to angle on map  */
	GMT_IS_ANGLE		= 1024,	/* An angle to be used as is  */
	GMT_IS_STRING		= 2048,	/* An text argument [internally used, not via -f]  */
	GMT_IS_UNKNOWN		= 4096};	/* Input type is not knowable without -f */

/*! Various ways to report longitudes */
enum GMT_lon_enum {
	GMT_IS_GIVEN_RANGE 			= 0,	/* Report lon as is */
	GMT_IS_0_TO_P360_RANGE		= 1,	/* Report 0 <= lon <= 360 */
	GMT_IS_0_TO_P360			= 2,	/* Report 0 <= lon < 360 */
	GMT_IS_M360_TO_0_RANGE		= 3,	/* Report -360 <= lon <= 0 */
	GMT_IS_M360_TO_0			= 4,	/* Report -360 < lon <= 0 */
	GMT_IS_M180_TO_P180_RANGE	= 5,	/* Report -180 <= lon <= +180 */
	GMT_IS_M180_TO_P180			= 6,	/* Report -180 <= lon < +180 */
	GMT_IS_M180_TO_P270_RANGE	= 7,	/* Report -180 <= lon < +270 [GSHHG only] */
	GMT_IGNORE_RANGE	= 99};		/* Do not adjust longitudes at all */

/*! How to handle NaNs in records */
enum GMT_io_nan_enum {
	GMT_IO_NAN_OK = 0,	/* NaNs are fine; just output the record as is */
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

#define gmt_M_fputs(line,fp) fputs(line,fp)
#define gmt_M_fread(ptr,size,nmemb,stream) fread(ptr,size,nmemb,stream)
#define gmt_M_fwrite(ptr,size,nmemb,stream) fwrite(ptr,size,nmemb,stream)
#define gmt_M_rewind(stream) rewind(stream)

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
	char ampm_suffix[2][GMT_LEN8];	/* Holds the strings to append am or pm */
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

/*! For selecting row ranges via -q */
struct GMT_ROW_RANGE {
	int64_t first, last, inc;
};

/*! For selecting data ranges via -q */
struct GMT_DATA_RANGE {
	double first, last;
};

struct GMT_IO {				/* Used to process input data records */
	void * (*input) (struct GMT_CTRL *, FILE *, uint64_t *, int *);	/* Pointer to function reading ASCII or binary tables */
	int (*output) (struct GMT_CTRL *, FILE *, uint64_t, double *, char *);	/* Pointer to function writing ASCII or binary tables */
	int (*read_item) (struct GMT_CTRL *, FILE *, uint64_t, double *);		/* Pointer to function reading 1-col z tables in grd2xyz */
	int (*write_item) (struct GMT_CTRL *, FILE *, uint64_t, double *);		/* Pointer to function writing 1-col z tables in xyz2grd */
	bool (*ogr_parser) (struct GMT_CTRL *, char *);				/* Set to handle either header or data OGR records */
	const char *scan_separators;	/* List of characters that separates columns in ascii records */

	unsigned int pad[4];		/* pad[0] = west, pad[1] = east, pad[2] = south, pad[3] = north */
	unsigned int inc_code[2];
	double curr_rec[GMT_MAX_COLUMNS];	/* The most recently processed data record */
	double prev_rec[GMT_MAX_COLUMNS];	/* The previous data record */

	bool multi_segments[2];	/* true if current ASCII input/output file has multiple segments */
	bool skip_headers_on_outout;	/* true when gmtconvert -T is set [or possibly other similar actions in the future] */
	bool skip_bad_records;	/* true if records where x and/or y are NaN or Inf */
	bool give_report;		/* true if functions should report how many bad records were skipped */
	bool skip_duplicates;	/* true if we should ignore duplicate x,y records */
	bool variable_in_columns;	/* true if we are reading ASCII records with variable numbers of columns */
	bool need_previous;		/* true if when parsing a record we need access to previous record values (e.g., for gap or duplicate checking) */
	bool has_previous_rec;		/* true if we have the previous record for this segment */
	bool warn_geo_as_cartesion;	/* true if we should warn if we read a record with geographic data while the expected format has not been set (i.e., no -J or -fg) */
	bool first_rec;			/* true when reading very first data record in a dataset */
	bool trailing_text[2];	/* Default is to process training text unless turned off via -i, -o */
	bool hash_refreshed;		/* true after calling the hash_refresh function the first time */
	bool internet_error;		/* true after failing to get hash table due to time-out */
	uint64_t seg_no;		/* Number of current multi-segment in entire data set */
	uint64_t seg_in_tbl_no;		/* Number of current multi-segment in current table */
	uint64_t n_clean_rec;		/* Number of clean records read (not including skipped records or comments or blanks) */
	uint64_t n_bad_records;		/* Number of bad records encountered during i/o */
	unsigned int tbl_no;		/* Number of current table in entire data set */
	unsigned int io_nan_ncols;	/* Number of columns to consider for -s option */
	unsigned int record_type[2];	/* Either GMT_READ|WRITE_DATA (0), GMT_READ|WRITE_TEXT (1), or GMT_READ|WRITE_MIXED (2) (for input and output) */
	unsigned int n_numerical_cols;	/* As it says */
	unsigned int max_cols_to_read;	/* For ascii input [all] */
	unsigned int n_row_ranges[2];	/* How many row ranges given in -q */
	enum GMT_ogr_status ogr;	/* Tells us if current input source has OGR/GMT metadata (GMT_OGR_TRUE) or not (GMT_OGR_FALSE) or not set (GMT_OGR_UNKNOWN) */
	unsigned int status;		/* 0	All is ok
					   1	Current record is segment header
					   2	Mismatch between actual and expected fields
					   4	EOF
					   8	NaNs encountered in first 2/3 cols */
	uint64_t rec_no;		/* Number of current records (counts headers etc) in entire data set */
	uint64_t rec_in_tbl_no;		/* Number of current record (counts headers etc) in current table */
	uint64_t data_record_number_in_set[2];	/* Number of current valid data record number in the whole dataset, for input and output. Headers not counted.  */
	uint64_t data_record_number_in_tbl[2];	/* Number of current valid data record number in the current table, for input and output. Headers not counted.  */
	uint64_t data_record_number_in_seg[2];	/* Number of current valid data record number in the current segment, for input and output. Headers not counted.  */
	int64_t curr_pos[2][4];		/* Keep track of current input/output table, segment, row, and table headers (for rec-by-rec action) */
	char r_mode[4];			/* Current file opening mode for reading (r or rb) */
	char w_mode[4];			/* Current file opening mode for writing (w or wb) */
	char a_mode[4];			/* Current file append mode for writing (a+ or ab+) */
	char curr_text[GMT_BUFSIZ];	/* Current ASCII record as it was read */
	char curr_trailing_text[GMT_BUFSIZ];	/* Current text portion of current record (or NULL) */
	char segment_header[GMT_BUFSIZ];	/* Current ASCII segment header */
	char filename[2][PATH_MAX];	/* Current filenames (or <stdin>/<stdout>) */
#ifdef HAVE_GDAL
	char tempfile[PATH_MAX];	/* Temporary file used to read - should be removed when closed */
#endif
	char col_set[2][GMT_MAX_COLUMNS];	/* Keeps track of which columns have had their type set */
	char *o_format[GMT_MAX_COLUMNS];	/* Custom output ASCII format to overrule format_float_out */
	int ncid;			/* NetCDF file ID (when opening netCDF file) */
	int nvars;			/* Number of requested variables in netCDF file */
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
	struct GMT_ROW_RANGE row_range[2][GMT_MAX_RANGES];		/* One or more ranges for input or output rows */
	struct GMT_DATA_RANGE data_range[2][GMT_MAX_RANGES];		/* One or more ranges for input or output times */
	struct GMT_OGR *OGR;		/* Pointer to GMT/OGR info used during reading */
	struct GMT_RECORD record;	/* Current record with pointers to data columns and text */
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
	unsigned int x_period;	/* length of a row in the input data ( <= n_columns, see x_missing) */
	unsigned int y_period;	/* length of a col in the input data ( <= n_rows, see y_missing) */
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

/* For the GMT_GRID container, see gmt_grdio.h */

#endif /* GMT_IO_H */
