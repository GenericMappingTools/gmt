/*--------------------------------------------------------------------
 *	$Id: gmt_io.h,v 1.14 2001-09-26 02:07:39 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 * Table input/output in GMT can be either ascii or binary (where supported)
 * and ASCII tables may consist of single or multiple segments.  When the
 * latter is the case usually there is a -M option to signal this case.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ascii tables.
 *
 * Author:	Paul Wessel
 * Date:	24-JAN-2000
 * Version:	3.4
 *
 */

#define GMT_IO_SEGMENT_HEADER	1
#define GMT_IO_MISMATCH		2
#define GMT_IO_EOF		4

#define GMT_COLUMN_FORMAT	1
#define GMT_ROW_FORMAT		2

/* Types of possible column entries in a file: */

#define GMT_IS_NAN		0	/* Returned by GMT_scanf routines when read fails */
#define GMT_IS_UNKNOWN		1	/* Generic (double) data type, no special format */
#define GMT_IS_LAT		2
#define GMT_IS_LON		4
#define GMT_IS_GEO		6	/* data type is either Lat or Lon */
#define GMT_IS_RELTIME		8	/* For I/O of data in user units */
#define GMT_IS_ABSTIME		16	/* For I/O of data in calendar+clock units */
#define GMT_IS_RATIME		24	/* To see if time is either Relative or Absolute */
#define GMT_IS_ARGTIME		32	/* To invoke GMT_scanf_argtime()  */

EXTERN_MSC FILE *GMT_fopen (const char* filename, const char* mode);	/* fopen wrapper */
EXTERN_MSC int GMT_fclose (FILE *stream);				/* fclose wrapper */
EXTERN_MSC void GMT_io_init (void);					/* Initialize pointers */
EXTERN_MSC int GMT_io_selection (char *text);				/* Decode -b option and set parameters */
EXTERN_MSC int GMT_decode_coltype (char *text);				/* Decode -i option and set parameters */
EXTERN_MSC void GMT_multisegment (char *text);				/* Decode -M option */
EXTERN_MSC void GMT_write_segmentheader (FILE *fp, int n);		/* Write multisegment header back out */
EXTERN_MSC char *GMT_fgets (char *record, int maxlength, FILE *fp);	/* Does a fscanf from inside gmt_io to keep DLLs working */
EXTERN_MSC int GMT_scanf (char *p, int expectation, double *val);	/* Convert strings to double, handling special formats */
EXTERN_MSC int	GMT_scanf_argtime (char *s, double *val);		/* Convert an argument token to a time  */
EXTERN_MSC void GMT_format_abstime_output (GMT_dtime dt, char *text);	/* Generate formatted textstring for absolute calendar time */
EXTERN_MSC void GMT_format_geo_output (BOOLEAN is_lat, double geo, char *text);	/* Generate formatted textstring for geographic coordinate */

struct GMT_CLOCK_IO {
	int order[3];			/* The relative order of hour, mn, sec in input clock string */
	int n_sec_decimals;		/* Number of digits in decimal seconds (0 for whole seconds) */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	BOOLEAN compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	BOOLEAN twelwe_hr_clock;	/* TRUE if we are doing am/pm on output */
	char ampm_suffix[2][8];		/* Holds the strings to append am or pm */
	char format[32];		/* Actual C format used to output clock */
	char delimeter[2][2];		/* Delimeter strings in clock, e.g. ":" */
};

struct GMT_DATE_IO {
	int item_order[4];		/* The sequence year, month, day, day-of-year in input calendar string */
	int item_pos[4];		/* Which position year, month, day, day-of-year has in calendar string */
	BOOLEAN Y2K_year;		/* TRUE if we have 2-digit years */
	BOOLEAN truncated_cal_is_ok;	/* TRUE if we have YMD or YJ order so smallest unit is to the right */
	char format[32];		/* Actual C format used to input/output date */
	BOOLEAN iso_calendar;		/* TRUE if we do ISO week calendar */
	BOOLEAN day_of_year;		/* TRUE if we do day-of-year rather than month/day */
	BOOLEAN mw_text;		/* TRUE if we must plot the month name or Week rather than a numeral */
	BOOLEAN compact;		/* TRUE if we do not want leading zeros in items (e.g., 03) */
	char delimeter[2][2];		/* Delimeter strings in date, e.g. "-" */
};

struct GMT_GEO_IO {	/* For geographic output and plotting */
	int order[3];			/* The relative order of degree, minute, seconds in template */
	int range;			/* 0 for 0/360, 1 for -360/0, 2 for -180/+180 */
	BOOLEAN decimal;		/* TRUE if we want to use the D_FORMAT for decimal degrees only */
	BOOLEAN wesn;			/* TRUE if we want sign encoded with suffix W, E, S, N */
	BOOLEAN no_sign;		/* TRUE if we want absolute values (plot only) */
	int n_sec_decimals;		/* Number of digits in decimal seconds (0 for whole seconds) */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	char x_format[32];		/* Actual C format used to plot/output longitude */
	char y_format[32];		/* Actual C format used to plot/output latitude */
	char delimeter[2][2];		/* Delimeter strings in date, e.g. "-" */
};

struct GMT_IO {	/* Used to process input data records */
	
	BOOLEAN multi_segments;		/* TRUE if current Ascii input file has multiple segments */
	BOOLEAN single_precision[2];	/* TRUE if current binary input(0) or output(1) is in single precision
					   [Default is double] */
	BOOLEAN binary[2];		/* TRUE if current input(0) or output(1) is in binary format */
	BOOLEAN skip_bad_records;	/* TRUE if records where x and/or y are NaN or Inf */
	BOOLEAN give_report;		/* TRUE if functions should report how many bad records were skipped */

	int ncol[2];			/* Number of expected columns of input(0) and output(1)
					   0 means it will be determined by program */
	int rec_no;			/* Number of current records */
	int n_clean_rec;		/* Number of clean records read (not including skipped records or comments or blanks) */
	int n_bad_records;		/* Number of bad records encountered during i/o */
	unsigned int status;		/* 0	All is ok
					   1	Current record is segment header
					   2	Mismatch between actual and expected fields
					   4	EOF */
	char EOF_flag;			/* Character signaling start of new segment in Ascii table */
	char current_record[BUFSIZ];	/* Current ascii record */
	char segment_header[BUFSIZ];	/* Current ascii segment header */
	char r_mode[3];			/* Current file opening mode for reading (r or rb) */
	char w_mode[3];			/* Current file opening mode for writing (w or wb) */
	BOOLEAN *skip_if_NaN;		/* TRUE if column j cannot be NaN and we must skip the record */
	int *in_col_type;		/* Type of column on input: Time, geographic, etc, see GMT_IS_<TYPE> */
	int *out_col_type;		/* Type of column on output: Time, geographic, etc, see GMT_IS_<TYPE> */
	struct GMT_DATE_IO date_input;	/* Has all info on how to decode input dates */
	struct GMT_DATE_IO date_output;	/* Has all info on how to write output dates */
	struct GMT_CLOCK_IO clock_input;	/* Has all info on how to decode input clocks */
	struct GMT_CLOCK_IO clock_output;	/* Has all info on how to write output clocks */
	struct GMT_GEO_IO geo;		/* Has all the info on how to write geographic coordinates */
};

struct GMT_Z_IO {	/* Used when processing z(x,y) table input when (x,y) is implicit */
	int binary;	/* TRUE if we are reading/writing binary data */
	int input;	/* TRUE if we are reading, FALSE if we are writing */
	int format;	/* Either GMT_COLUMN_FORMAT or GMT_ROW_FORMAT */
	int skip;	/* Number of bytes to skip before reading data */
	BOOLEAN swab;	/* TRUE if we must swap byte-order */
	int x_step;	/* +1 if logical x values increase to right, else -1 */
	int y_step;	/* +1 if logical y values increase upwards, else -1 */
	int x_missing;	/* 1 if a periodic (right) column is implicit (i.e., not stored) */
	int y_missing;	/* 1 if a periodic (top) row is implicit (i.e., not stored) */
	int n_expected;	/* Number of data element expected to be read */
	int start_col;	/* First logical column in file */
	int start_row;	/* First logical row in file */
	int nx, ny;	/* Dimensions of final gridded data */
	int x_period;	/* length of a row in the input data ( <= nx, see x_missing) */
	int y_period;	/* length of a col in the input data ( <= ny, see y_missing) */
	int gmt_i;	/* Current column number in the GMT registered grid */
	int gmt_j;	/* Current row number in the GMT registered grid */
	PFI read_item;	/* Pointer to function that will read 1 data point from file */
	PFI write_item;	/* Pointer to function that will write 1 data point from file */
	PFV get_gmt_ij;	/* Pointer to function that converts running number to GMT ij */
};

struct GMT_PLOT_CALCLOCK {
	struct GMT_DATE_IO date;
	struct GMT_CLOCK_IO clock;
	struct GMT_GEO_IO geo;
};

EXTERN_MSC struct GMT_IO GMT_io;
EXTERN_MSC struct GMT_PLOT_CALCLOCK GMT_plot_calclock;	/* Formatting information for time axis plotting */

EXTERN_MSC void GMT_init_z_io (struct GMT_Z_IO *r, BOOLEAN input);
EXTERN_MSC int GMT_parse_z_io (char *txt, struct GMT_Z_IO *r, BOOLEAN input);
EXTERN_MSC void GMT_set_z_io (struct GMT_Z_IO *r, struct GRD_HEADER *h);
EXTERN_MSC void GMT_check_z_io (struct GMT_Z_IO *r, float *a);
