/*--------------------------------------------------------------------
 *	$Id: gmt_io.h,v 1.2 2001-03-01 22:08:26 pwessel Exp $
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

EXTERN_MSC FILE *GMT_fopen (const char* filename, const char* mode);	/* fopen wrapper */
EXTERN_MSC int GMT_fclose (FILE *stream);				/* fclose wrapper */
EXTERN_MSC void GMT_io_init (void);					/* Initialize pointers */
EXTERN_MSC int GMT_io_selection (char *text);				/* Decode -b option and set parameters */
EXTERN_MSC void GMT_multisegment (char *text);				/* Decode -M option */
EXTERN_MSC void GMT_write_segmentheader (FILE *fp, int n);		/* Write multisegment header back out */
EXTERN_MSC int GMT_scanf (char *p, double *val);			/* Convert text (incl dd:mm:ss) to double number */
EXTERN_MSC char *GMT_fgets (char *record, int maxlength, FILE *fp);	/* Does a fscanf from inside gmt_io to keep DLLs working */


struct GMT_IO {
	BOOLEAN multi_segments;		/* TRUE if current Ascii input file has multiple segments */
	BOOLEAN single_precision[2];	/* TRUE if current binary input(0) or output(1) is in single precision
					   [Default is double] */
	BOOLEAN binary[2];		/* TRUE if current input(0) or output(1) is in binary format */
	BOOLEAN skip_bad_records;	/* TRUE if records where x and/or y are NaN or Inf */
	BOOLEAN give_report;		/* TRUE if functions should report how many bad records were skipped */

	int ncol[2];			/* Number of expected columns of input(0) and output(1)
					   0 means it will be determined by program */
	int rec_no;			/* Number of current records */
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
};

EXTERN_MSC struct GMT_IO GMT_io;

struct GMT_Z_IO {	/* Used when processing z table input */
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
