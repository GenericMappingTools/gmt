/*--------------------------------------------------------------------
 *	$Id: gmt_io.h,v 1.4 2001-08-16 19:12:23 pwessel Exp $
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

#define GMT_IS_NAN		0
#define GMT_IS_UNKNOWN		1
#define GMT_IS_LAT		2
#define GMT_IS_LON		4
#define GMT_IS_GEO		6
#define GMT_IS_RELTIME		8
#define GMT_IS_ABSTIME		16
#define GMT_IS_TIME		24

EXTERN_MSC FILE *GMT_fopen (const char* filename, const char* mode);	/* fopen wrapper */
EXTERN_MSC int GMT_fclose (FILE *stream);				/* fclose wrapper */
EXTERN_MSC void GMT_io_init (void);					/* Initialize pointers */
EXTERN_MSC int GMT_io_selection (char *text);				/* Decode -b option and set parameters */
EXTERN_MSC int GMT_decode_coltype (char *text);				/* Decode -i option and set parameters */
EXTERN_MSC void GMT_multisegment (char *text);				/* Decode -M option */
EXTERN_MSC void GMT_write_segmentheader (FILE *fp, int n);		/* Write multisegment header back out */
EXTERN_MSC int GMT_scanf_old (char *p, double *val);			/* Convert text (incl dd:mm:ss) to double number */
EXTERN_MSC char *GMT_fgets (char *record, int maxlength, FILE *fp);	/* Does a fscanf from inside gmt_io to keep DLLs working */


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
	int n_sec_decimals;		/* Number of digits in decimal seconds (0 for whole seconds) */
	double f_sec_to_int;		/* Scale to convert 0.xxx seconds to integer xxx (used for formatting) */
	BOOLEAN twelwe_hr_clock;	/* TRUE if we are doing am/pm on output */
	BOOLEAN iso_calendar;		/* TRUE if we do ISO week calendar */
	BOOLEAN day_of_year;		/* TRUE if we do day-of-year rather than month/day */
	char ampm_suffix[2][8];		/* Holds the strings to append am or pm */
	int ymdj_input_order[4];	/* The relative order of year, month, day, day-of-year in input calendar string */
	int ymdj_output_order[4];	/* The relative order of year, month, day, day-of-year in output calendar string */
	int hms_output_order[3];	/* The relative order of hour, mn, sec in output clock string */
	char output_clock_format[32];	/* Actual C format used to output clock */
};

EXTERN_MSC struct GMT_IO GMT_io;

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

EXTERN_MSC void GMT_init_z_io (struct GMT_Z_IO *r, BOOLEAN input);
EXTERN_MSC int GMT_parse_z_io (char *txt, struct GMT_Z_IO *r, BOOLEAN input);
EXTERN_MSC void GMT_set_z_io (struct GMT_Z_IO *r, struct GRD_HEADER *h);
EXTERN_MSC void GMT_check_z_io (struct GMT_Z_IO *r, float *a);
