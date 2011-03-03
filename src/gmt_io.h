/*--------------------------------------------------------------------
 *	$Id: gmt_io.h,v 1.91 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
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
 * Date:	24-JAN-2000
 * Version:	4.1.x
 *
 */

#define GMT_COLUMN_FORMAT	1
#define GMT_ROW_FORMAT		2

#define GMT_IS_FILE		0
#define GMT_IS_STREAM		1
#define GMT_IS_FDESC		2
#define GMT_IS_ARRAY		3
#define GMT_IS_GRIDFILE		4
#define GMT_IS_GRID		5
#define GMT_IS_GMTGRID		6

#define GMT_X			0	/* x or lon is in 0th column */
#define GMT_Y			1	/* y or lat is in 1st column */
#define GMT_Z			2	/* z is in 2nd column */

/* Error return codes */

#define GMT_IO_SEGMENT_HEADER	1
#define GMT_IO_MISMATCH		2
#define GMT_IO_EOF		4
#define GMT_IO_NAN		8
#define GMT_IO_GAP		16

#define GMT_REC_IS_SEG_HEADER		(GMT_io.status & GMT_IO_SEGMENT_HEADER)
#define GMT_REC_IS_ERROR		(GMT_io.status & GMT_IO_MISMATCH)
#define GMT_REC_IS_EOF			(GMT_io.status & GMT_IO_EOF)
#define GMT_REC_IS_NAN			(GMT_io.status & GMT_IO_NAN)
#define GMT_REC_IS_GAP			(GMT_io.status & GMT_IO_GAP)
#define GMT_REC_IS_NEW_SEGMENT		(GMT_io.status & (GMT_IO_SEGMENT_HEADER | GMT_IO_NAN))
#define GMT_REC_IS_LINE_BREAK		(GMT_io.status & (GMT_IO_SEGMENT_HEADER | GMT_IO_EOF | GMT_IO_NAN | GMT_IO_GAP))

/* Array indices for input/output variables */

#define GMT_IN	0
#define GMT_OUT	1

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

#ifdef WIN32
EXTERN_MSC FILE *GMT_fdopen (int handle, const char *mode);
EXTERN_MSC int GMT_fgetc (FILE *stream);
EXTERN_MSC int GMT_ungetc (int c, FILE *stream);
EXTERN_MSC int GMT_fputs (const char *line, FILE *fp);
EXTERN_MSC int GMT_fputc (const int c, FILE *fp);
EXTERN_MSC int GMT_fseek (FILE *stream, long offset, int whence);
EXTERN_MSC long GMT_ftell (FILE *stream);
EXTERN_MSC size_t GMT_fread (void * ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN_MSC size_t GMT_fwrite (const void * ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN_MSC void GMT_rewind (FILE *stream);
#else
#define GMT_fdopen(handle, mode) fdopen(handle, mode)
#define GMT_fgetc(stream) fgetc(stream)
#define GMT_ungetc(c, stream) ungetc(c, stream)
#define GMT_fputs(line,fp) fputs(line,fp)
#define GMT_fputc(c,fp) fputc(c,fp)
#define GMT_fseek(stream,offset,whence) fseek(stream,offset,whence)
#define GMT_ftell(stream) ftell(stream)
#define GMT_fread(ptr,size,nmemb,stream) fread(ptr,size,nmemb,stream)
#define GMT_fwrite(ptr,size,nmemb,stream) fwrite(ptr,size,nmemb,stream)
#define GMT_rewind(stream) rewind(stream)
#endif

EXTERN_MSC char *GMT_fgets (char *line, GMT_LONG size, FILE *fp);
EXTERN_MSC int GMT_fclose (FILE *stream);
EXTERN_MSC FILE *GMT_fopen (const char* filename, const char *mode);
EXTERN_MSC char *GMT_getuserpath (const char *stem, char *path);	/* Look for user file */
EXTERN_MSC char *GMT_getdatapath (const char *stem, char *path);	/* Look for data file */
EXTERN_MSC char *GMT_getsharepath (const char *subdir, const char *stem, const char *suffix, char *path);	/* Look for shared file */
EXTERN_MSC int GMT_access (const char *filename, int mode);		/* access wrapper */
EXTERN_MSC void GMT_io_init (void);					/* Initialize pointers */
EXTERN_MSC GMT_LONG GMT_parse_b_option (char *text);				/* Decode -b option and set parameters */
EXTERN_MSC GMT_LONG GMT_parse_f_option (char *text);				/* Decode -f option and set parameters */
EXTERN_MSC void GMT_parse_m_option (char *text);				/* Decode -m option */
EXTERN_MSC void GMT_write_segmentheader (FILE *fp, GMT_LONG n);		/* Write multisegment header back out */
EXTERN_MSC GMT_LONG GMT_scanf (char *p, GMT_LONG expectation, double *val);	/* Convert strings to double, handling special formats [Data records only ] */
EXTERN_MSC GMT_LONG GMT_scanf_arg (char *p, GMT_LONG expectation, double *val);	/* Convert strings to double, handling special formats [ command line only ] */
EXTERN_MSC GMT_LONG	GMT_scanf_argtime (char *s, double *val);		/* Convert an argument token to a time  */
EXTERN_MSC void GMT_format_abstime_output (double dt, char *text);	/* Generate formatted textstring for absolute calendar time */
EXTERN_MSC void GMT_format_geo_output (GMT_LONG is_lat, double geo, char *text);	/* Generate formatted textstring for geographic coordinate */
EXTERN_MSC GMT_LONG GMT_ascii_output_one (FILE *fp, double x, GMT_LONG col);
EXTERN_MSC void GMT_ascii_format_one (char *text, double x, GMT_LONG type);
EXTERN_MSC void GMT_lon_range_adjust (GMT_LONG range, double *lon);		/* Adjust the longitude given the desired range */
EXTERN_MSC GMT_LONG GMT_points_are_antipodal (double lonA, double latA, double lonB, double latB);
EXTERN_MSC GMT_LONG GMT_geo_to_dms (double val, GMT_LONG n_items, double fact, GMT_LONG *d, GMT_LONG *m,  GMT_LONG *s,  GMT_LONG *ix);
EXTERN_MSC GMT_LONG GMT_not_numeric (char *text);				/* Rules out _some_ text as possible numerics */
EXTERN_MSC GMT_LONG GMT_is_a_blank_line (char *line);	/* Checks if line is a blank line or comment */
EXTERN_MSC GMT_LONG GMT_nc_get_att_text (int ncid, int varid, char *name, char *text, size_t textlen);
EXTERN_MSC GMT_LONG GMT_parse_segment_item (char *in_string, char *pattern, char *out_string);

/* DO NOT REARRANGE THE ORDER OF ITEMS IN THESE STRUCTURES UNLESS YOU MATCH IT IN gmt_globals.h */

struct GMT_CLOCK_IO {
	GMT_LONG order[3];		/* The relative order of hour, mn, sec in input clock string */
	GMT_LONG n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	GMT_LONG compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	GMT_LONG twelve_hr_clock;	/* TRUE if we are doing am/pm on output */
	char ampm_suffix[2][8];		/* Holds the strings to append am or pm */
	char format[GMT_TEXT_LEN];	/* Actual C format used to output clock */
	char delimiter[2][2];		/* Delimiter strings in clock, e.g. ":" */
};

struct GMT_DATE_IO {
	GMT_LONG item_order[4];		/* The sequence year, month, day, day-of-year in input calendar string */
	GMT_LONG item_pos[4];		/* Which position year, month, day, day-of-year has in calendar string */
	GMT_LONG Y2K_year;		/* TRUE if we have 2-digit years */
	GMT_LONG truncated_cal_is_ok;	/* TRUE if we have YMD or YJ order so smallest unit is to the right */
	char format[GMT_TEXT_LEN];	/* Actual C format used to input/output date */
	GMT_LONG iso_calendar;		/* TRUE if we do ISO week calendar */
	GMT_LONG day_of_year;		/* TRUE if we do day-of-year rather than month/day */
	GMT_LONG mw_text;		/* TRUE if we must plot the month name or Week rather than a numeral */
	GMT_LONG compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_GEO_IO {			/* For geographic output and plotting */
	GMT_LONG order[3];		/* The relative order of degree, minute, seconds in form */
	GMT_LONG range;			/* 0 for 0/360, 1 for -360/0, 2 for -180/+180 */
	GMT_LONG decimal;		/* TRUE if we want to use the D_FORMAT for decimal degrees only */
	GMT_LONG wesn;			/* TRUE if we want sign encoded with suffix W, E, S, N */
	GMT_LONG no_sign;		/* TRUE if we want absolute values (plot only) */
	GMT_LONG n_sec_decimals;	/* Number of digits in decimal seconds (0 for whole seconds) */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	char x_format[GMT_TEXT_LEN];	/* Actual C format used to plot/output longitude */
	char y_format[GMT_TEXT_LEN];	/* Actual C format used to plot/output latitude */
	char delimiter[2][2];		/* Delimiter strings in date, e.g. "-" */
};

struct GMT_IO {				/* Used to process input data records */
	
	GMT_LONG multi_segments[2];	/* TRUE if current Ascii input/output file has multiple segments */
	GMT_LONG single_precision[2];	/* TRUE if current binary input/output is in single precision [double] */
	GMT_LONG swab[2];		/* TRUE if current binary input/output must be byte-swapped */
	GMT_LONG binary[2];		/* TRUE if current input/output is in native binary format */
	GMT_LONG netcdf[2];		/* TRUE if current input/output is in netCDF format */
	GMT_LONG io_header[2];		/* TRUE if input/output data has header records */
	GMT_LONG skip_bad_records;	/* TRUE if records where x and/or y are NaN or Inf */
	GMT_LONG give_report;		/* TRUE if functions should report how many bad records were skipped */
	GMT_LONG skip_duplicates;	/* TRUE if we should ignore duplicate x,y records */

	GMT_LONG file_no;		/* Number of current file */
	GMT_LONG ncol[2];		/* Number of expected columns of input/output
					   0 means it will be determined by program */
	GMT_LONG n_header_recs;		/* number of header records [0] */
	GMT_LONG seg_no;		/* Number of current multi-segment */
	GMT_LONG rec_no;		/* Number of current records (counts headers etc) */
	GMT_LONG pt_no;			/* Number of current valid points in a row  */
	GMT_LONG n_clean_rec;		/* Number of clean records read (not including skipped records or comments or blanks) */
	GMT_LONG n_bad_records;		/* Number of bad records encountered during i/o */
	GMT_LONG status;		/* 0	All is ok
					   1	Current record is segment header
					   2	Mismatch between actual and expected fields
					   4	EOF
					   8	NaNs encountered in first 2/3 cols */
	char EOF_flag[2];		/* Character signaling start of new segment in input/output Ascii table */
	char current_record[BUFSIZ];	/* Current ascii record */
	char segment_header[BUFSIZ];	/* Current ascii segment header */
	char varnames[BUFSIZ];		/* List of variable names to be input/output in netCDF mode */
	char r_mode[4];			/* Current file opening mode for reading (r or rb) */
	char w_mode[4];			/* Current file opening mode for writing (w or wb) */
	char a_mode[4];			/* Current file append mode for writing (a+ or ab+) */
	GMT_LONG *skip_if_NaN;		/* TRUE if column j cannot be NaN and we must skip the record */
	GMT_LONG *in_col_type;		/* Type of column on input: Time, geographic, etc, see GMT_IS_<TYPE> */
	GMT_LONG *out_col_type;		/* Type of column on output: Time, geographic, etc, see GMT_IS_<TYPE> */
	int ncid;			/* NetCDF file ID (when opening netCDF file) */
	int nvars;			/* Number of requested variables in netCDF file */
	size_t ndim;			/* Length of the column dimension */
	size_t nrec;			/* Record count */
	int *varid;			/* Array of variable IDs */
	double *scale_factor;		/* Array of scale factors */
	double *add_offset;		/* Array of offsets */
	double *missing_value;		/* Array of missing values */
	struct GMT_DATE_IO date_input;	/* Has all info on how to decode input dates */
	struct GMT_DATE_IO date_output;	/* Has all info on how to write output dates */
	struct GMT_CLOCK_IO clock_input;	/* Has all info on how to decode input clocks */
	struct GMT_CLOCK_IO clock_output;	/* Has all info on how to write output clocks */
	struct GMT_GEO_IO geo;		/* Has all the info on how to write geographic coordinates */
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
	GMT_LONG nx, ny;	/* Dimensions of final gridded data */
	GMT_LONG gmt_i;		/* Current column number in the GMT registered grid */
	GMT_LONG gmt_j;		/* Current row number in the GMT registered grid */
	PFL read_item;		/* Pointer to function that will read 1 data point from file */
	PFL write_item;		/* Pointer to function that will write 1 data point from file */
	PFV get_gmt_ij;		/* Pointer to function that converts running number to GMT ij */
};

struct GMT_PLOT_CALCLOCK {
	struct GMT_DATE_IO date;
	struct GMT_CLOCK_IO clock;
	struct GMT_GEO_IO geo;
};

struct GMT_LINE_SEGMENT {		/* For holding multisegment lines in memory */
	char *label;			/* Label string (if applicable) */
	char *header;			/* Multisegment header (if applicable) */
	GMT_LONG n_rows;		/* Number of points in this segment */
	GMT_LONG n_columns;		/* Number of fields in each record (>= 2) */
	GMT_LONG pole;			/* Spherical polygons only: If it encloses the S (-1) or N (+1) pole, or none (0) */
	double dist;			/* Distance from a point to this feature */
	double *min;			/* Minimum coordinate for each column */
	double *max;			/* Maximum coordinate for each column */
	double **coord;			/* Coordinates x,y, and possibly other columns */
};

struct GMT_TABLE {	/* To hold an array of line segment structures and header information in one container */
	GMT_LONG n_headers;			/* Number of file header records (0 if no header) */
	char *file;				/* Name of file header or source */
	char **header;				/* Array with all file header records, if any) */
	GMT_LONG n_segments;			/* Number of segments in the array */
	GMT_LONG n_records;			/* Total number of data records across all segments */
	GMT_LONG n_columns;			/* Number of columns (fields) in each record */
	double *min;				/* Minimum coordinate for each column */
	double *max;				/* Maximum coordinate for each column */
	struct GMT_LINE_SEGMENT **segment;	/* Pointer to array of segments */
};

struct GMT_DATASET {	/* Single container for an array of GMT tables (files) */
	GMT_LONG n_tables;		/* The total number of tables (files) contained */
	GMT_LONG n_segments;		/* The total number of segments across all tables */
	GMT_LONG n_records;		/* The total number of data records across all tables */
	struct GMT_TABLE **table;	/* Pointer to array of tables */
};

EXTERN_MSC struct GMT_IO GMT_io;
EXTERN_MSC struct GMT_PLOT_CALCLOCK GMT_plot_calclock;	/* Formatting information for time axis plotting */

EXTERN_MSC GMT_LONG GMT_init_z_io (char format[], GMT_LONG repeat[], GMT_LONG swab, GMT_LONG skip, char type, struct GMT_Z_IO *r);
EXTERN_MSC GMT_LONG GMT_set_z_io (struct GMT_Z_IO *r, struct GRD_HEADER *h);
EXTERN_MSC void GMT_check_z_io (struct GMT_Z_IO *r, float *a);
EXTERN_MSC GMT_LONG GMT_import_table (void *source, GMT_LONG source_type, struct GMT_TABLE **T, double dist, GMT_LONG greenwich, GMT_LONG poly, GMT_LONG use_GMT_io);
EXTERN_MSC GMT_LONG GMT_export_table (void *dest, GMT_LONG dest_type, struct GMT_TABLE *T, GMT_LONG use_GMT_io);
EXTERN_MSC void GMT_alloc_segment (struct GMT_LINE_SEGMENT *S, GMT_LONG n_rows, GMT_LONG n_columns, GMT_LONG first);
EXTERN_MSC void GMT_free_segment (struct GMT_LINE_SEGMENT *segment);
EXTERN_MSC void GMT_free_table (struct GMT_TABLE *table);
EXTERN_MSC void GMT_free_dataset (struct GMT_DATASET *data);
EXTERN_MSC void GMT_date_C_format (char *form, struct GMT_DATE_IO *S, GMT_LONG mode);
EXTERN_MSC void GMT_clock_C_format (char *form, struct GMT_CLOCK_IO *S, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_geo_C_format (char *form, struct GMT_GEO_IO *S);
EXTERN_MSC void GMT_plot_C_format (char *form, struct GMT_GEO_IO *S);
