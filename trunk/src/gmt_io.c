/*--------------------------------------------------------------------
 *	$Id: gmt_io.c,v 1.11 2001-08-17 21:25:48 pwessel Exp $
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
 * and processing of ascii tables.  For compliance with a wide variety of
 * binary data formats for grids and their internal nesting the GMT_Z_IO
 * structure and associated functions are used (in xyz2grd and grd2xyz)
 *
 * The following functions are here:
 *
 *	GMT_fopen:		Open a file
 *	GMT_fclose:		Close a file
 *	GMT_io_init:		Init GMT_IO structure
 *	GMT_io_selection:	Decode the -b switch
 *	GMT_multisegment:	Decode the -M switch
 *	GMT_write_segmentheader	Write header record for multisegment files
 *	GMT_ascii_input:	Decode ascii input record
 *	GMT_scanf:		Robust scanf function with optional dd:mm:ss conversion
 *	GMT_bin_double_input:	Decode binary double precision record
 *	GMT_bin_float_input:	Decode binary single precision record
 *	GMT_ascii_output:	Write ascii record
 *	GMT_bin_double_output:	Write binary double precision record
 *	GMT_bin_float_output:	Write binary single precision record
 *	GMT_init_z_io:		Initialize GMT_Z_IO structure
 *	GMT_parse_z_io:		Parse the -Z switch
 *	GMT_set_z_io:		Set GMT_Z_IO structure based on -Z
 *	GMT_check_z_io:		Fill in implied missing row/column
 *	GMT_a_read:		Read 1 ascii item
 *	GMT_c_read:		Read 1 binary char item
 *	GMT_u_read:		Read 1 binary unsigned char item
 *	GMT_h_read:		Read 1 binary short int item
 *	GMT_H_read:		Read 1 binary unsigned short int item
 *	GMT_i_read:		Read 1 binary int item
 *	GMT_I_read:		Read 1 binary unsigned int item
 *	GMT_l_read:		Read 1 binary long int item
 *	GMT_f_read:		Read 1 binary float item
 *	GMT_d_read:		Read 1 binary double item
 *	GMT_a_write:		Write 1 ascii item
 *	GMT_c_write:		Write 1 binary char item
 *	GMT_u_write:		Write 1 binary unsigned char item
 *	GMT_h_write:		Write 1 binary short int item
 *	GMT_H_write:		Write 1 binary unsigned short int item
 *	GMT_i_write:		Write 1 binary int item
 *	GMT_I_write:		Write 1 binary unsigned int item
 *	GMT_l_write:		Write 1 binary long int item
 *	GMT_f_write:		Write 1 binary float item
 *	GMT_d_write:		Write 1 binary double item
 *	GMT_col_ij:		Convert index to column format
 *	GMT_row_ij:		Convert index to row format
 *
 * Author:	Paul Wessel
 * Date:	14-JUL-2000
 * Version:	3.4
 */
 
#include "gmt.h"

#define GMT_I_60	(1.0 / 60.0)	/* Convert minutes to degrees */
#define GMT_I_3600	(1.0 / 3600.0)	/* Convert seconds to degrees */

BOOLEAN GMT_do_swab = FALSE;	/* Used to indicate swab'ing during binary read */

int GMT_a_read (FILE *fp, double *d);
int GMT_c_read (FILE *fp, double *d);
int GMT_u_read (FILE *fp, double *d);
int GMT_h_read (FILE *fp, double *d);
int GMT_H_read (FILE *fp, double *d);
int GMT_i_read (FILE *fp, double *d);
int GMT_I_read (FILE *fp, double *d);
int GMT_l_read (FILE *fp, double *d);
int GMT_f_read (FILE *fp, double *d);
int GMT_d_read (FILE *fp, double *d);
int GMT_a_write (FILE *fp, double d);
int GMT_c_write (FILE *fp, double d);
int GMT_u_write (FILE *fp, double d);
int GMT_h_write (FILE *fp, double d);
int GMT_H_write (FILE *fp, double d);
int GMT_i_write (FILE *fp, double d);
int GMT_I_write (FILE *fp, double d);
int GMT_l_write (FILE *fp, double d);
int GMT_f_write (FILE *fp, double d);
int GMT_d_write (FILE *fp, double d);
void GMT_col_ij (struct GMT_Z_IO *r, int ij, int *gmt_ij);
void GMT_row_ij (struct GMT_Z_IO *r, int ij, int *gmt_ij);
int GMT_ascii_input (FILE *fp, int *n, double **ptr);		/* Decode Ascii input records */
int GMT_bin_double_input (FILE *fp, int *n, double **ptr);	/* Decode binary double input records */
int GMT_bin_float_input (FILE *fp, int *n, double **ptr);	/* Decode binary float input records */
int GMT_ascii_output (FILE *fp, int n, double *ptr);		/* Write Ascii output records */
int GMT_bin_double_output (FILE *fp, int n, double *ptr);	/* Write binary double output records */
int GMT_bin_float_output (FILE *fp, int n, double *ptr);	/* Write binary float output records */
void GMT_adjust_periodic ();					/* Add/sub 360 as appropriate */
void GMT_decode_calclock_formats ();
void GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S);
void GMT_date_C_format (char *template, struct GMT_DATE_IO *S, int mode);
void GMT_clock_C_format (char *template, struct GMT_CLOCK_IO *S, int mode);

int	GMT_scanf_clock (char *s, double *val);
int	GMT_scanf_calendar (char *s, GMT_cal_rd *rd);
int	GMT_scanf_ISO_calendar (char *s, GMT_cal_rd *rd);
int	GMT_scanf_g_calendar (char *s, GMT_cal_rd *rd);
int	GMT_scanf_geo (char *s, double *val);
int	GMT_scanf_float (char *s, double *val);


/* Table I/O routines for ascii and binary io */

FILE *GMT_fopen (const char* filename, const char* mode)
{
        return (fopen (filename, mode));
}

int GMT_fclose (FILE *stream)
{
        return (fclose (stream));
}

void GMT_io_init (void)
{
	/* No need to init the structure as this is done in gmt_init.h directoy */

	int i;
	
	GMT_input  = GMT_ascii_input;
	GMT_output = GMT_ascii_output;

	GMT_io.give_report = TRUE;
	
	GMT_io.skip_if_NaN = (BOOLEAN *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (BOOLEAN), GMT_program);
	GMT_io.in_col_type  = (int *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (int), GMT_program);
	GMT_io.out_col_type = (int *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (int), GMT_program);
	for (i = 0; i < BUFSIZ; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = GMT_IS_UNKNOWN;

	GMT_decode_calclock_formats ();
}

int GMT_io_selection (char *text)
{
	/* Syntax:	-b[i][o][s][d][#cols] */

	int i, id = 0;
	BOOLEAN i_or_o = FALSE, ok = TRUE, error = FALSE;

	for (i = 0; ok && text[i]; i++) {

		switch (text[i]) {

			case 'i':	/* Settings apply to input */
				id = 0;
				GMT_io.binary[id] = i_or_o = TRUE;
				break;
			case 'o':	/* Settings apply to output */
				id = 1;
				GMT_io.binary[id] = i_or_o = TRUE;
				break;
			case 's':	/* Single Precision */
				GMT_io.single_precision[id] = TRUE;
				break;
			case 'd':	/* Double Precision */
				GMT_io.single_precision[id] = FALSE;
				break;
			case '0':	/* Number of columns */
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				GMT_io.ncol[id] = atoi (&text[i]);
				while (text[i] && isdigit ((int)text[i])) i++;
				i--;
				break;

			default:	/* Stop scanning */
				error = TRUE;
				fprintf (stderr, "%s: GMT Error: Malformed -b argument [%s]\n", GMT_program, text);
				break;
		}
	}

	if (!i_or_o) {	/* Specified neither i or o so let settings apply to both */
		GMT_io.binary[0] = GMT_io.binary[1] = TRUE;
		GMT_io.single_precision[1] = GMT_io.single_precision[0];
		GMT_io.ncol[1] = GMT_io.ncol[0];
	}

	if (GMT_io.binary[0]) {
		GMT_input  = (GMT_io.single_precision[0]) ? GMT_bin_float_input  : GMT_bin_double_input;
		strcpy (GMT_io.r_mode, "rb");
	}

	if (GMT_io.binary[1]) {
		GMT_output = (GMT_io.single_precision[1]) ? GMT_bin_float_output : GMT_bin_double_output;
		strcpy (GMT_io.w_mode, "wb");
	}

	return (error);
}


void GMT_multisegment (char *text)
{
	/* Turns multisegment on and sets flag, if given.
	 * flag is only used for ASCII data sets */

	GMT_io.multi_segments = TRUE;
	if (text[0]) GMT_io.EOF_flag = text[0];
}

int GMT_ascii_input (FILE *fp, int *n, double **ptr)
{
	char line[BUFSIZ], *p;
	int i, col_no, len, n_convert;
	BOOLEAN done = FALSE, bad_record;
	double val;
	
	/* GMT_ascii_input will skip blank lines and cshell comment lines which start
	 * with # except when -M# is used of course.  Fields may be separated by
	 * spaces, tabs, or commas.  The routine returns the actual
	 * number of items read [or 0 for segment header and -1 for EOF]
	 * If *n is passed as BUFSIZ it will be reset to the actual number of fields */

	while (!done) {	/* Done becomes TRUE when we successfully have read a data record */

		/* First read until we get a non-blank, non-comment record, or reach EOF */

		GMT_io.rec_no++;
		while ((p = fgets (line, BUFSIZ, fp)) && (line[0] == '\n' || (line[0] == '#' && GMT_io.EOF_flag != '#'))) GMT_io.rec_no++;

		if (!p) {
			GMT_io.status = GMT_IO_EOF;
			if (GMT_io.give_report && GMT_io.n_bad_records) {	/* Report summary and reset */
				fprintf (stderr, "%s: This file had %d records with invalid x and/or y values\n", GMT_program, GMT_io.n_bad_records);
				GMT_io.n_bad_records = GMT_io.rec_no = GMT_io.n_clean_rec = 0;
			}
			return (-1);
		}

		if (GMT_io.multi_segments && line[0] == GMT_io.EOF_flag) {	/* Got a multisegment header, take action and return */
			GMT_io.status = GMT_IO_SEGMENT_HEADER;
			strcpy (GMT_io.segment_header, line);
			return (0);
		}

		/* Normal data record */

		/* First chop off trailing whitespace and commas */

		len = strlen (line);
#ifndef _WIN32
		if (len >= (BUFSIZ-1)) {
			fprintf (stderr, "%s: This file appears to be in DOS format - reformat with dos2unix\n", GMT_program);
			exit (EXIT_FAILURE);
		}
#endif
			
		for (i = len - 1; i >= 0 && strchr (" \t,\n", (int)line[i]); i--);
		line[++i] = '\n';	line[++i] = '\0';
 
		bad_record = FALSE;
		strcpy (GMT_io.current_record, line);
		line[i-1] = '\0';		/* Chop off newline at end of string */
		p = strtok(line, " \t,");	/* Get first field and keep going until done */
		col_no = 0;
		while (!bad_record && p && col_no < *n) {
			if ((n_convert = GMT_scanf (p, GMT_io.in_col_type[col_no], &val)) == GMT_IS_NAN) {	/* Got NaN or it failed to decode */
				if (GMT_io.skip_if_NaN[col_no])	/* This field cannot be NaN so we must skip the entire record */
					bad_record = TRUE;
				else				/* OK to have NaN in this field, continue processing of record */
					GMT_data[col_no] = GMT_d_NaN;
			}
			else {					/* Successful decode, assign to array */
				GMT_data[col_no] = val;
			} 
			p = strtok(CNULL, " \t,");		/* Goto next field */
			col_no++;
		}
		if (bad_record) {
			GMT_io.n_bad_records++;
			if (GMT_io.give_report && (GMT_io.n_bad_records == 1)) {	/* Report 1st occurence */
				fprintf (stderr, "%s: Encountered first invalid record near/at line # %d\n", GMT_program, GMT_io.rec_no);
				fprintf (stderr, "%s: Likely cause: Invalid x and/or y values or missing -: switch\n", GMT_program);
			}
		}
		else
			done = TRUE;
	}
	*ptr = GMT_data;
	GMT_io.status = (col_no == *n || *n == BUFSIZ) ? 0 : GMT_IO_MISMATCH;
	if (*n == BUFSIZ) *n = col_no;

	if (gmtdefs.xy_toggle) d_swap (GMT_data[0], GMT_data[1]);	/* Got lat/lon instead of lon/lat */
	if (GMT_geographic_in) {
		GMT_adjust_periodic ();	/* Must account for periodicity in 360 */
	}
	
	return (col_no);
}

int GMT_scanf_old (char *p, double *val)
{
	/* Converts text to double if certain conditions are met:
	   p must be of the form [+|-][0-9][.][0-9][E|e|D|e][+|-][0-9] OR [+|-][dd:mm[:ss][WESN]
	   If it is the former we look for d or D which is FORTRASH
	   for exponent and convert to e and then call atof.  If it
	   is the latter we do a straight conversion.
	*/
	int i, k, c, colons, suffix;
	BOOLEAN error, period, exponent, sign, flip_sign;
	double degree, minute, second;

	for (i = colons = 0; p[i]; i++) if (p[i] == ':') colons++;	/* Colons indicate dd:mm:ss format */
	k = --i;
	c = (int)p[k];		/* Last character in string */
	suffix = toupper (c);	/* Last character in string, forced upper case */
	
	if (colons == 0) {	/* Regular ASCII representation of an integer or floating point number */
		if (suffix == 'W' || suffix == 'S') {	/* Sign was given implicitly as -ve */
			p[k] = 0;	/* Temporarily hide the suffix - will restore below */
			flip_sign = TRUE;
		}
		else if (suffix == 'E' || suffix == 'N') {	/* Sign was given implicitly as +ve */
			p[k] = 0;	/* Temporarily hide the suffix - will restore below */
			flip_sign = FALSE;
		}
		else	/* No suffix or unrecognized suffix */
			flip_sign = FALSE;
			
		i = 0;					/* Reset to 1st character */
		while (p[i] == ' ') i++;		/* Skip leading blanks */
		if (p[i] == '-' || p[i] == '+') i++;	/* Leading sign is OK */
		error = period = exponent = sign = FALSE;
		while (p[i] && !error) {
			if (p[i] == '.') {	/* One period is OK */
				if (period) error = TRUE;
				period = TRUE;
			}
			else if (p[i] == 'D' || p[i] == 'd') {	/* Fortran Double Precision Fix */
				p[i] = 'e';	/* 'd' is not understood outside Fortran i/o */
				if (exponent) error = TRUE;
				exponent = TRUE;
			}
			else if (p[i] == 'E' || p[i] == 'e') {	/* Normal exponentional notation */
				if (exponent) error = TRUE;
				exponent = TRUE;
			}
			else if (p[i] == '-' || p[i] == '+') {	/* One more sign only ok after exponential */
				if (sign || !exponent) error = TRUE;
				sign = TRUE;
			}
			else if (p[i] < '0' || p[i] > '9')	/* Other non-digits are not allowed */
				error = TRUE;
			i++;
		}
		if (error) return (0);	/* Failed format check */
		*val = atof (p);	/* Safe to convert */
		if (flip_sign) {	/* Flip sign and restore missing suffix in text string */
			*val = -(*val);
			p[k] = suffix;
		}
			
		return (1);
	}

	/* Here we know we need to deal with dd:mm[:ss] strings */
	
	if (colons == 2) {	/* dd:mm:ss format */
		sscanf (p, "%lf:%lf:%lf", &degree, &minute, &second);
		if (suffix == 'W' || suffix == 'w' || suffix == 'S' || suffix == 's') degree = -degree;	/* Sign was given implicitly */
		*val = degree + copysign (minute * GMT_I_60 + second * GMT_I_3600, degree);
	}
	else if (colons == 1) {	/* dd:mm format */
		sscanf (p, "%lf:%lf", &degree, &minute);
		if (suffix == 'W' || suffix == 'w' || suffix == 'S' || suffix == 's') degree = -degree;	/* Sign was given implicitly */
		*val = degree + copysign (minute * GMT_I_60, degree);
	}
	else	/* Unrecognized */
		return (0);

	return (1);
}

char *GMT_fgets (char *record, int maxlength, FILE *fp)
{
	return (fgets (record, maxlength, fp));
}


int GMT_bin_double_input (FILE *fp, int *n, double **ptr)
{
	int n_read;

	GMT_io.status = 0;
	if ((n_read = fread ((void *) GMT_data, sizeof (double), (size_t)(*n), fp)) != (*n)) {
		GMT_io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
	}

	*ptr = GMT_data;

	/* Read ok, how about multisegment? */

	if (!GMT_io.status && GMT_io.multi_segments) {	/* Must have n_read NaNs */
		int i;
		BOOLEAN is_bad = TRUE;
		for (i = 0; i < n_read && is_bad; i++) is_bad = GMT_is_dnan (GMT_data[i]);
		if (is_bad) {
			GMT_io.status = GMT_IO_SEGMENT_HEADER;
			strcpy (GMT_io.segment_header, "> Binary multisegment header\n");
			return (0);
		}
	}
	if (gmtdefs.xy_toggle) d_swap (GMT_data[0], GMT_data[1]);	/* Got lat/lon instead of lon/lat */
	if (GMT_geographic_in) {
		GMT_adjust_periodic ();	/* Must account for periodicity in 360 */
	}

	return (n_read);
}
	

int GMT_bin_float_input (FILE *fp, int *n, double **ptr)
{
	int i, n_read;
	static float GMT_f[BUFSIZ];
	
	GMT_io.status = 0;
	if ((n_read = fread ((void *) GMT_f, sizeof (float), (size_t)(*n), fp)) != (*n)) {
		GMT_io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
	}
	else {
		for (i = 0; i < n_read; i++) GMT_data[i] = (double)GMT_f[i];
	}

	*ptr = GMT_data;

	/* Read ok, how about multisegment? */

	if (!GMT_io.status && GMT_io.multi_segments) {	/* Must have n_read NaNs */
		BOOLEAN is_bad = TRUE;
		for (i = 0; i < n_read && is_bad; i++) is_bad = GMT_is_dnan (GMT_data[i]);
		if (is_bad) {
			GMT_io.status = GMT_IO_SEGMENT_HEADER;
			strcpy (GMT_io.segment_header, "> Binary multisegment header\n");
			return (0);
		}
	}
	if (gmtdefs.xy_toggle) d_swap (GMT_data[0], GMT_data[1]);	/* Got lat/lon instead of lon/lat */
	if (GMT_geographic_in) {
		GMT_adjust_periodic ();	/* Must account for periodicity in 360 */
	}

	return (n_read);
}

void GMT_adjust_periodic (void) {
	/* while (GMT_data[0] > project_info.e) GMT_data[0] -= 360.0;
	while (GMT_data[0] < project_info.w) GMT_data[0] += 360.0; */
	while (GMT_data[0] > project_info.e && (GMT_data[0] - 360.0) >= project_info.w) GMT_data[0] -= 360.0;
	while (GMT_data[0] < project_info.w && (GMT_data[0] + 360.0) <= project_info.w) GMT_data[0] += 360.0;
	/* If data is not inside the given range it will satisfy (lon > east) */
	/* Now it will be outside the region on the same side it started out at */
}
	
int GMT_ascii_output (FILE *fp, int n, double *ptr)
{
	int i, e = 0, wn = 0;
	
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);	/* Write lat/lon instead of lon/lat */
	n--;
	for (i = 0; i < n && e >= 0; i++) {
		(GMT_is_dnan (ptr[i])) ? (e = fprintf (fp, "NaN\t")) : (e = fprintf (fp, gmtdefs.d_format, ptr[i]), putc ('\t', fp));
		wn += e;
	}
	(GMT_is_dnan (ptr[n])) ? (e = fprintf (fp, "NaN\n")) : (e = fprintf (fp, gmtdefs.d_format, ptr[n]), putc ('\n', fp));
	wn += e;
	return ((e < 0) ? e : wn);
}

int GMT_bin_double_output (FILE *fp, int n, double *ptr)
{
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);	/* Write lat/lon instead of lon/lat */
	return (fwrite ((void *) ptr, sizeof (double), (size_t)n, fp));
}
	
int GMT_bin_float_output (FILE *fp, int n, double *ptr)
{
	int i;
	static float GMT_f[BUFSIZ];
	
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);	/* Write lat/lon instead of lon/lat */
	for (i = 0; i < n; i++) GMT_f[i] = (float)ptr[i];
	return (fwrite ((void *) GMT_f, sizeof (float), (size_t)n, fp));
}

void GMT_write_segmentheader (FILE *fp, int n)
{
	/* Output ASCII or binary multisegment header */

	int i;
	if (GMT_io.binary[1])
		for (i = 0; i < n; i++) GMT_output (fp, 1, &GMT_d_NaN);
	else
		fprintf (fp, "%s", GMT_io.segment_header);
}
		
void GMT_init_z_io (struct GMT_Z_IO *r, BOOLEAN input)
{
	memset ((void *) r, 0, sizeof (struct GMT_Z_IO));

	r->input = input;

	/* Set default format if no arguments are given to be TLf */

	if (input)
		r->read_item = GMT_a_read;
	else
		r->write_item = GMT_a_write;
	r->binary = FALSE;
	r->format = GMT_ROW_FORMAT;
	r->y_step = r->x_step = 1;
}

int GMT_parse_z_io (char *txt, struct GMT_Z_IO *r, BOOLEAN input)
{
	BOOLEAN first = TRUE;
	int i;

	/* BOOLEAN input:  currently unused */

	for (i = 0; txt[i]; i++) {	/* Loop over flags */

		switch (txt[i]) {

			/* These 4 cases will set the format orientation for input */

			case 'T':
				if (first) r->format = GMT_ROW_FORMAT;
				r->y_step = 1;
				first = FALSE;
				break;

			case 'B':
				if (first) r->format = GMT_ROW_FORMAT;
				r->y_step = -1;
				first = FALSE;
				break;

			case 'L':
				if (first)r->format = GMT_COLUMN_FORMAT;
				r->x_step = 1;
				first = FALSE;
				break;

			case 'R':
				if (first)r->format = GMT_COLUMN_FORMAT;
				r->x_step = -1;
				first = FALSE;
				break;

			/* Set this if file is periodic, is grid registrered, but repeating column or row is missing from input */

			case 'x':
				r->x_missing = 1;
				break;

			case 'y':
				r->y_missing = 1;
				break;

			/* Optionally skip the given number of bytes before reading data */

			case 's':
				i++;
				if (txt[i]) {
					r->skip = atoi (&txt[i]);
					while (txt[i] && isdigit ((int)txt[i])) i++;
					i--;
				}
				break;

			case 'w':
				r->swab = TRUE;
				break;

			/* Set read pointer depending on data format */

			case 'a':	/* ASCII */
				r->read_item = GMT_a_read;	r->write_item = GMT_a_write;
				r->binary = FALSE;
				break;

			case 'c':	/* Binary signed char */
				r->read_item = GMT_c_read; 	r->write_item = GMT_c_write;
				r->binary = TRUE;
				break;

			case 'u':	/* Binary unsigned char */
				r->read_item = GMT_u_read; 	r->write_item = GMT_u_write;
				r->binary = TRUE;
				break;

			case 'h':	/* Binary short 2-byte integer */
				r->read_item = GMT_h_read; 	r->write_item = GMT_h_write;
				r->binary = TRUE;
				break;

			case 'H':	/* Binary unsigned short 2-byte integer */
				r->read_item = GMT_H_read;	r->write_item = GMT_H_write;
				r->binary = TRUE;
				break;

			case 'i':	/* Binary 4-byte integer */
				r->read_item = GMT_i_read;	r->write_item = GMT_i_write;
				r->binary = TRUE;
				break;

			case 'I':	/* Binary 4-byte unsigned integer */
				r->read_item = GMT_I_read;	r->write_item = GMT_I_write;
				r->binary = TRUE;
				break;

			case 'l':	/* Binary 4(or8)-byte integer, machine dependent! */
				r->read_item = GMT_l_read;	r->write_item = GMT_l_write;
				r->binary = TRUE;
				break;

			case 'f':	/* Binary 4-byte float */
				r->read_item = GMT_f_read;	r->write_item = GMT_f_write;
				r->binary = TRUE;
				break;

			case 'd':	/* Binary 8-byte double */
				r->read_item = GMT_d_read;	r->write_item = GMT_d_write;
				r->binary = TRUE;
				break;


			default:
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Z: %c not a valid modifier!\n", GMT_program, txt[i]);
				exit (EXIT_FAILURE);
				break;
		}
	}

	if (r->binary) {
		strcpy (GMT_io.r_mode, "rb");
		strcpy (GMT_io.w_mode, "wb");
	}

	return (FALSE);
}

void GMT_set_z_io (struct GMT_Z_IO *r, struct GRD_HEADER *h)
{
	if ((r->x_missing || r->y_missing) && h->node_offset == 1) {
		fprintf (stderr, "%s: Pixel format grids do not have repeating rows or columns!\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	r->start_col = (r->x_step == 1) ? 0 : h->nx - 1 - r->x_missing;
	r->start_row = (r->y_step == 1) ? r->y_missing : h->ny - 1;
	r->get_gmt_ij = (r->format == GMT_COLUMN_FORMAT) ? GMT_col_ij : GMT_row_ij;
	r->nx = h->nx;
	r->ny = h->ny;
	r->x_period = h->nx - r->x_missing;
	r->y_period = h->ny - r->y_missing;
	r->n_expected = r->x_period * r->y_period;
	GMT_do_swab = r->swab;
}

void GMT_check_z_io (struct GMT_Z_IO *r, float *a)
{
	/* Routine to fill in the implied periodic row or column that was missing */

	int i, j, k;

	if (r->x_missing) for (j = 0; j < r->ny; j++) a[(j+1)*r->nx-1] = a[j*r->nx];
	if (r->y_missing) for (i = 0, k = (r->ny-1)*r->nx; i < r->nx; i++) a[i] = a[k+i];
}

int GMT_a_read (FILE *fp, double *d)
{
	char line[64];
	if (fgets (line, 64, fp)) {
		*d = (line[0] == 'N' || line[0] == 'n') ? GMT_d_NaN : atof (line);
		return (1);
	}
	return (0);
}

int GMT_c_read (FILE *fp, double *d)
{
	char c;
	if (fread ((void *)&c, sizeof (char), 1, fp)) {
		*d = (double) c;
		return (1);
	}
	return (0);
}

int GMT_u_read (FILE *fp, double *d)
{
	unsigned char u;
	if (fread ((void *)&u, sizeof (unsigned char), 1, fp)) {
		*d = (double) u;
		return (1);
	}
	return (0);
}

int GMT_h_read (FILE *fp, double *d)
{
	short int h;
	if (fread ((void *)&h, sizeof (short int), 1, fp)) {
		if (GMT_do_swab) h = GMT_swab2 (h);
		*d = (double) h;
		return (1);
	}
	return (0);
}

int GMT_H_read (FILE *fp, double *d)
{
	unsigned short int h;
	if (fread ((void *)&h, sizeof (unsigned short int), 1, fp)) {
		*d = (double) ((GMT_do_swab) ? GMT_swab2 (h) : h);
		return (1);
	}
	return (0);
}

int GMT_i_read (FILE *fp, double *d)
{
	int i;
	if (fread ((void *)&i, sizeof (int), 1, fp)) {
		if (GMT_do_swab) i = GMT_swab4 (i);
		*d = (double) i;
		return (1);
	}
	return (0);
}

int GMT_I_read (FILE *fp, double *d)
{
	unsigned int i;
	if (fread ((void *)&i, sizeof (unsigned int), 1, fp)) {
		*d = (double) ((GMT_do_swab) ? GMT_swab4 (i) : i);
		return (1);
	}
	return (0);
}

int GMT_l_read (FILE *fp, double *d)
{
	long int l;
	if (fread ((void *)&l, sizeof (long int), 1, fp)) {
		if (GMT_do_swab) {
			unsigned int *i, k;
			i = (unsigned int *)&l;
			for (k = 0; k < sizeof (long int)/4; k++) i[k] = GMT_swab4 (i[k]);
		}
		*d = (double) l;
		return (1);
	}
	return (0);
}

int GMT_f_read (FILE *fp, double *d)
{
	float f;
	if (fread ((void *)&f, sizeof (float), 1, fp)) {
		if (GMT_do_swab) {
			unsigned int *i;
			i = (unsigned int *)&f;
			*i = GMT_swab4 (*i);
		}
		*d = (double) f;
		return (1);
	}
	return (0);
}

int GMT_d_read (FILE *fp, double *d)
{
	if (fread ((void *)d, sizeof (double), 1, fp)) {
		if (GMT_do_swab) {
			unsigned int *i, j;
			i = (unsigned int *)d;
			j = GMT_swab4 (i[0]);
			i[0] = GMT_swab4 (i[1]);
			i[1] = j;
		}
		return (1);
	}
	return (0);
}

int GMT_a_write (FILE *fp, double d)
{
	int n = 0;
	n = (GMT_is_dnan (d)) ? fprintf (fp, "NaN") : fprintf (fp, gmtdefs.d_format, d);
	fprintf (fp, "\n"); 
	return (n);
}

int GMT_c_write (FILE *fp, double d)
{
	char c;
	c = (char) d;
	return (fwrite ((void *)&c, sizeof (char), (size_t)1, fp));
}

int GMT_u_write (FILE *fp, double d)
{
	unsigned char u;
	u = (unsigned char) d;
	return (fwrite ((void *)&u, sizeof (unsigned char), (size_t)1, fp));
}

int GMT_h_write (FILE *fp, double d)
{
	short int h;
	h = (short int) d;
	return (fwrite ((void *)&h, sizeof (short int), (size_t)1, fp));
}

int GMT_H_write (FILE *fp, double d)
{
	unsigned short int h;
	h = (unsigned short int) d;
	return (fwrite ((void *)&h, sizeof (unsigned short int), (size_t)1, fp));
}

int GMT_i_write (FILE *fp, double d)
{
	int i;
	i = (int) d;
	return (fwrite ((void *)&i, sizeof (int), (size_t)1, fp));
}

int GMT_I_write (FILE *fp, double d)
{
	unsigned int i;
	i = (unsigned int) d;
	return (fwrite ((void *)&i, sizeof (unsigned int), (size_t)1, fp));
}

int GMT_l_write (FILE *fp, double d)
{
	long int l;
	l = (long int) d;
	return (fwrite ((void *)&l, sizeof (long int), (size_t)1, fp));
}

int GMT_f_write (FILE *fp, double d)
{
	float f;
	f = (float) d;
	return (fwrite ((void *)&f, sizeof (float), (size_t)1, fp));
}

int GMT_d_write (FILE *fp, double d)
{
	return (fwrite ((void *)&d, sizeof (double), (size_t)1, fp));
}

void GMT_col_ij (struct GMT_Z_IO *r, int ij, int *gmt_ij)
{
	/* Translates incoming ij to gmt_ij for column-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij % r->y_period);
	r->gmt_i = r->start_col + r->x_step * (ij / r->y_period);

	*gmt_ij = r->gmt_j * r->nx + r->gmt_i;
}

void GMT_row_ij (struct GMT_Z_IO *r, int ij, int *gmt_ij)
{

	/* Translates incoming ij to gmt_ij for row-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij / r->x_period);
	r->gmt_i = r->start_col + r->x_step * (ij % r->x_period);

	*gmt_ij = r->gmt_j * r->nx + r->gmt_i;
}

void GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S)
{	/* Reads a YYYY-MM-DD or YYYYMMDD-like string and determines order.
	 * order[0] is the order of the year, [1] is month, etc.
	 * Items not encountered are left as -1.
	 */

	int i, j, order, n_y, n_m, n_d, n_j, n_w, sequence[4], n_delim, last, error = 0;
	
	/* S->order is initialized to {-1, -1, -1, -1} */
	
	n_y = n_m = n_d = n_j = n_w = n_delim = 0;
	S->delimeter[0][0] = S->delimeter[0][1] = S->delimeter[1][0] = S->delimeter[1][1] = 0;
	sequence[0] = sequence[1] = sequence[2] = sequence[3] = -1;
	
	for (i = order = 0; i < strlen (text); i++) {
		switch (text[i]) {
			case 'y':	/* Year */
				if (S->order[0] < 0)		/* First time we encounter a y */
					S->order[0] = order++;
				else if (text[i-1] != 'y')	/* Done it before, previous char must be y */
					error++;
				n_y++;
				break;
			case 'm':	/* Month */
				if (S->order[1] < 0)		/* First time we encounter a m */
					S->order[1] = order++;
				else if (text[i-1] != 'm')	/* Done it before, previous char must be m */
					error++;
				n_m++;
				break;
			case 'W':	/* ISO Week flag */
				S->iso_calendar = TRUE;
				break;
			case 'w':	/* Iso Week */
				if (S->order[1] < 0) {		/* First time we encounter a w */
					S->order[1] = order++;
					if (text[i-1] != 'W') error++;	/* Must have the format W just before */
				}
				else if (text[i-1] != 'w')	/* Done it before, previous char must be w */
					error++;
				n_w++;
				break;
			case 'd':	/* Day of month */
				if (S->order[2] < 0)		/* First time we encounter a d */
					S->order[2] = order++;
				else if (text[i-1] != 'd')	/* Done it before, previous char must be d */
					error++;
				n_d++;
				break;
			case 'j':	/* Day of year  */
				if (S->order[3] < 0)		/* First time we encounter a j */
					S->order[3] = order++;
				else if (text[i-1] != 'j')	/* Done it before, previous char must be j */
					error++;
				n_j++;
				break;
			default:	/* Delimeter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimeter[n_delim++][0] = text[i];
				break;
		}
	}
	
	/* Then get the actual order by inverting table */
	
	for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) if (S->order[j] == i) sequence[i] = j;
	for (i = 0; i < 4; i++) S->order[i] = sequence[i];
	S->Y2K_year = (n_y == 2);		/* Must supply the century when reading */
	S->truncated_cal_is_ok = TRUE;		/* May change in the next loop */
	for (i = 1, last = S->order[0]; S->truncated_cal_is_ok && i < 4; i++) {
		if (S->order[i] == -1) continue;
		if (S->order[i] < last) S->truncated_cal_is_ok = FALSE;
		last = S->order[i];
	}
	last = (n_y > 0) + (n_m > 0) + (n_w > 0) + (n_d > 0) + (n_j > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);				/* If there are delimeters, must be one less than the items */
	if (S->iso_calendar) {		/* Check if ISO Week format is ok */
		error += (!S->truncated_cal_is_ok);
		error += (n_w != 2);			/* Gotta have 2 ww */
		error += !(n_d == 1 || n_d == 0);	/* Gotta have 1 d if present */
	}
	else {				/* Check if Gregorian format is ok */
		error += (n_w != 0);	/* Should have no w */
		error += (n_j == 3 && !(n_m == 0 && n_d == 0));	/* day of year should have m = d = 0 */
		error += (n_j == 0 && !((n_m == 2 || n_m == 0) && (n_d == 2 || n_d == 0) && n_d <= n_m));	/* mm/dd must have jjj = 0 and m >= d and m,d 0 or 2 */
	}
	if (error) {
		fprintf (stderr, "%s: ERROR: Unacceptable date template %s\n", GMT_program, text);
		exit (EXIT_FAILURE);
	}
}

void GMT_get_hms_order (char *text, struct GMT_CLOCK_IO *S)
{	/* Reads a HH:MM:SS or HHMMSS-like string and determines order.
	 * hms_order[0] is the order of the hour, [1] is min, etc.
	 * Items not encountered are left as -1.
	 */

	int i, j, order, n_delim, sequence[3], last, n_h, n_m, n_s, n_x, n_dec, off, error = 0;
	BOOLEAN big_to_small;
	char *p;
	
	/* hms_order is initialized to {-1, -1, -1} */
	sequence[0] = sequence[1] = sequence[2] = -1;
	
	S->delimeter[0][0] = S->delimeter[0][1] = S->delimeter[1][0] = S->delimeter[1][1] = 0;
	n_h = n_m = n_s = n_x = n_dec = n_delim = 0;
	
	/* Determine if we do 12-hour clock (and what form of am/pm suffix) or 24-hour clock */
	
	if ((p = strstr (text, "am"))) {	/* Want 12 hour clock with am/pm */
		S->twelwe_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "am");
		strcpy (S->ampm_suffix[1], "pm");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "AM"))) {	/* Want 12 hour clock with AM/PM */
		S->twelwe_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "AM");
		strcpy (S->ampm_suffix[1], "PM");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "a.m."))) {	/* Want 12 hour clock with a.m./p.m. */
		S->twelwe_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "a.m.");
		strcpy (S->ampm_suffix[1], "p.m.");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "A.M."))) {	/* Want 12 hour clock with A.M./P.M. */
		S->twelwe_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "A.M.");
		strcpy (S->ampm_suffix[1], "P.M.");
		off = (int)(p) - (int)text;
	}
	else
		off = strlen (text);
	
	for (i = order = 0; i < off; i++) {
		switch (text[i]) {
			case 'h':	/* Hour */
				if (S->order[0] < 0)		/* First time we encountered a h */
					S->order[0] = order++;
				else if (text[i-1] != 'h')	/* Must follow a previous h */
					error++;
				n_h++;
				break;
			case 'm':	/* Minute */
				if (S->order[1] < 0)		/* First time we encountered a m */
					S->order[1] = order++;
				else if (text[i-1] != 'm')	/* Must follow a previous m */
					error++;
				n_m++;
				break;
			case 's':	/* Seconds */
				if (S->order[2] < 0)		/* First time we encountered a s */
					S->order[2] = order++;
				else if (text[i-1] != 's')	/* Must follow a previous s */
					error++;
				n_s++;
				break;
			case '.':	/* Decimal point for seconds? */
				if (text[i+1] == 'x')
					n_dec++;
				else {	/* Must be a delimeter */
					if (n_delim == 2)
						error++;
					else
						S->delimeter[n_delim++][0] = text[i];
				}
				break;
			case 'x':	/* Fraction of seconds */
				if (n_x > 0 && text[i-1] != 'x')	/* Must follow a previous x */
					error++;
				n_x++;
				break;
			default:	/* Delimeter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimeter[n_delim++][0] = text[i];
				break;
		}
	}
	
	/* Then get the actual order by inverting table */
	
	for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) if (S->order[j] == i) sequence[i] = j;
	for (i = 0; i < 3; i++) S->order[i] = sequence[i];
	big_to_small = TRUE;		/* May change in the next loop */
	for (i = 1, last = S->order[0]; big_to_small && i < 3; i++) {
		if (S->order[i] == -1) continue;
		if (S->order[i] < last) big_to_small = FALSE;
		last = S->order[i];
	}
	if (!big_to_small) error++;
	last = (n_h > 0) + (n_m > 0) + (n_s > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);	/* If there are delimeters, must be one less than the items */
	error += (!(n_h == 0 || n_h == 2) || !(n_m == 0 || n_m == 2) || !(n_s == 0 || n_s == 2));	/* h, m, s are all either 2 or 0 */
	error += (n_s > n_m || n_m > n_h);		/* Cannot have secs without m etc */
	error += (n_x && n_dec != 1);			/* .xxx is the proper form */
	error += (n_x == 0 && n_dec);			/* Period by itself and not delimeter? */
	error += (n_dec > 1);				/* Only one period with xxx */
	S->n_sec_decimals = n_x;
	S->f_sec_to_int = rint (pow (10.0, (double)S->n_sec_decimals));			/* To scale fracional seconds to an integer form */
	if (error) {
		fprintf (stderr, "%s: ERROR: Unacceptable clock template %s\n", GMT_program, text);
		exit (EXIT_FAILURE);
	}
}

void GMT_decode_calclock_formats ()
{
	GMT_date_C_format (gmtdefs.input_date_format, &GMT_io.date_input, 0);
	GMT_date_C_format (gmtdefs.output_date_format, &GMT_io.date_output, 1);
	GMT_clock_C_format (gmtdefs.input_clock_format, &GMT_io.clock_input, 0);
	GMT_clock_C_format (gmtdefs.output_clock_format, &GMT_io.clock_output, 1);
}

void GMT_clock_C_format (char *template, struct GMT_CLOCK_IO *S, int mode)
{
	/* Determine the order of H, M, S in input and output clock strings,
	 * as well as the number of decimals in output seconds (if any), and
	 * if a 12- or 24-hour clock is used.
	 * mode is 0 for input and 1 for output format
	 */
	 
	char *c;
	
	/* Get the order of year, month, day or day-of-year in input/output formats for dates */
	
	GMT_get_hms_order (template, S);
	
	/* Craft the actual C-format to use for input/output clock strings */
		
	if (S->order[0] >= 0) {	/* OK, at least hours is needed */
		char fmt[32];
		(mode) ? sprintf (S->format, "%%2.2d\0") : sprintf (S->format, "%%2d\0");
		if (S->order[1] >= 0) {	/* Need minutes too*/
			if (S->delimeter[0][0]) strcat (S->format, S->delimeter[0]);
			(mode) ? sprintf (fmt, "%%2.2d\0") : sprintf (fmt, "%%2d\0");
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* .. and seconds */
				if (S->delimeter[1][0]) strcat (S->format, S->delimeter[1]);
				if (mode) {	/* Output format */
					sprintf (fmt, "%%2.2d\0");
					strcat (S->format, fmt);
					if (S->n_sec_decimals) {	/* even add format for fractions of second */
						sprintf (fmt, ".%%%d.%dd\0", S->n_sec_decimals, S->n_sec_decimals);
						strcat (S->format, fmt);
					}
				}
				else {		/* Input format */
					sprintf (fmt, "%%lf\0");
					strcat (S->format, fmt);
				}
			}
		}
		if (mode && S->twelwe_hr_clock) {	/* Finally add %s for the am, pm string */
			sprintf (fmt, "%%s\0");
			strcat (S->format, fmt);
		}
	}
}

void GMT_date_C_format (char *template, struct GMT_DATE_IO *S, int mode)
{
	/* Determine the order of Y, M, D, J in input and output date strings.
	* mode is 0 for input and 1 for output
	 */
	 
	char *c, fmt[32];
	int k;
	
	/* Get the order of year, month, day or day-of-year in input/output formats for dates */
	
	GMT_get_ymdj_order (template, S);
		
	/* Craft the actual C-format to use for i/o date strings */

	if (S->order[0] >= 0 && S->iso_calendar) {	/* OK, at least ISO year is needed */
		k = (S->Y2K_year) ? 2 : 4;
		(mode) ? sprintf (S->format, "%%%d.%dd\0", k, k) : sprintf (S->format, "%%%dd\0", k);
		if (S->order[1] >= 0) {	/* Need ISO week */
			if (S->delimeter[0][0]) strcat (S->format, S->delimeter[0]);
			strcat (S->format, "W");
			(mode) ? sprintf (fmt, "%%2.2d\0") : sprintf (fmt, "%%2d\0");
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* and ISO day of week */
				if (S->delimeter[1][0]) strcat (S->format, S->delimeter[1]);
				sprintf (fmt, "%%1d\0");
				strcat (S->format, fmt);
			}
		}
	}
	else if (S->order[0] >= 0) {	/* OK, at least one item is needed */
		k = (S->order[0] == 0 && !S->Y2K_year) ? 4 : 2;
		(mode) ? sprintf (S->format, "%%%d.%dd\0", k, k) : sprintf (S->format, "%%%dd\0", k);
		if (S->order[1] >= 0) {	/* Need more items */
			if (S->delimeter[0][0]) strcat (S->format, S->delimeter[0]);
			k = (S->order[1] == 0 && !S->Y2K_year) ? 4 : 2;
			(mode) ? sprintf (fmt, "%%%d.%dd\0", k, k) : sprintf (fmt, "%%%dd\0", k);
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* .. and even more */
				if (S->delimeter[1][0]) strcat (S->format, S->delimeter[1]);
				k = (S->order[2] == 0 && !S->Y2K_year) ? 4 : 2;
				(mode) ? sprintf (fmt, "%%%d.%dd\0", k, k) : sprintf (fmt, "%%%dd\0", k);
				strcat (S->format, fmt);
			}
		}
	}
}

int GMT_decode_coltype (char *arg)
{
	/* Routine will decode the -f[i|o]<col>|<colrange>[t|T|g],... arguments */
	
	char copy[BUFSIZ], *p, *c;
	int i, k = 1, start, stop, ic, code, *col;
	BOOLEAN both_i_and_o = FALSE;

	if (arg[0] == 'i')	/* Apply to input columns only */
		col = GMT_io.in_col_type;
	else if (arg[0] == 'o')	/* Apply to output columns only */
		col = GMT_io.out_col_type;
	else {			/* Apply to both input and output columns */
		both_i_and_o = TRUE;
		k = 0;
	}
	
	strncpy (copy, &arg[k], BUFSIZ);	/* arg should NOT have a leading i|o part */
	
	p = strtok (copy, ",");		/* Get first token */
	while (p) {			/* While it is not empty, process it */
		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9T */
			sscanf (p, "%d-%d", &start, &stop);
		else				/* Just a single column, e.g., 3t */
			start = stop = atoi (p);

		ic = (int) p[strlen(p)-1];	/* Last char in p is the potential code T, t, or g */
		switch (ic) {
			case 'T':	/* Absolute calendar time */
				code = GMT_IS_ABSTIME;
				break;
			case 't':	/* Relative calendar time (need epoch) */
				code = GMT_IS_RELTIME;
				break;
			case 'g':	/* Geographical coordinates */
				code = GMT_IS_GEO;
				break;
			default:	/* No suffix, consider it an error */
				fprintf (stderr, "%s: GMT Error: Malformed -i argument [%s]\n", GMT_program, arg);
				return 1;
				break;
		}
		
		/* Now set the code for these columns */
			
		if (both_i_and_o)
			for (i = start; i <= stop; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = code;
		else
			for (i = start; i <= stop; i++) col[i] = code;
		
		p = strtok (NULL, ",");	/* Next entry */
	}
	return (0);
}

int	GMT_scanf_clock (char *s, double *val) {

	/* On failure, return -1.  On success, set val and return 0.
	
	Looks for apAP, but doesn't discover a failure if called
	with "11:13:15 Hello, Walter", because it will find an a. 
	
	Doesn't check whether use of a or p matches stated intent
	to use twelve_hour_clock.
	
	ISO standard allows 24:00:00, so 86400 is not too big.
	If the day of this clock might be a day with a leap second, 
	(this routine doesn't know that) then we should also allow
	86401.  A value exceeding 86401 is an error.
	*/
	
	int	k, hh, mm, add_noon = 0;
	int	hh_limit = 24;	/* ISO std allows 24:00:00  */
	double	ss, x;
	char	*p;
	
	if ( (p = strpbrk (s, "apAP") ) ) {
		switch (p[0]) {
			case 'a':
			case 'A':
				add_noon = 0;
				hh_limit = 12;
				break;
			case 'p':
			case 'P':
				add_noon = 43200;
				hh_limit = 12;
				break;
			default:
				return (-1);
				break;
		}
	}
	
	k = sscanf(s, GMT_io.clock_input.format, &hh, &mm, &ss);
	if (k == 0) return (-1);
	if (hh < 0 || hh > hh_limit) return (-1);
	
	x = add_noon + 3600*hh;	
	if (k > 1) {
		if (mm < 0 || mm > 59) return (-1);
		x += 60*mm;
	}
	if (k > 2) {
		x += ss;
		if (x > 86401.0) return (-1);
	}
	*val = x;
	return (0);
}

	
	
int	GMT_scanf_calendar (char *s, GMT_cal_rd *rd) {

	/* On failure, return -1.  On success, set rd and return 0 */
	if (GMT_io.date_input.iso_calendar) {
		return (GMT_scanf_ISO_calendar (s, rd));
	}
	return (GMT_scanf_g_calendar (s, rd));
}

int	GMT_scanf_ISO_calendar (char *s, GMT_cal_rd *rd) {

	/* On failure, return -1.  On success, set rd and return 0.
	Assumes that year, week of year, day of week appear in that
	order only, and that the format string can handle the W.
	Assumes also that it is always OK to fill in missing bits.  */

	int	k, n, ival[3];
	
	if ( (n = sscanf(s, GMT_io.date_input.format, &ival[0], &ival[1], &ival[2]) ) == 0) return (-1);
	
	/* Handle possible missing bits:  */
	for (k = n; k < 3; k++) ival[k] = 1;
	
	if (ival[1] < 1 || ival[1] > 53) return (-1);
	if (ival[2] < 1 || ival[2] > 7) return (-1);
	if (GMT_io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = GMT_y2_to_y4_yearfix (ival[0]);
	}
	*rd = GMT_rd_from_iywd (ival[0], ival[1], ival[2]);
	return (0);
}

int	GMT_scanf_g_calendar (char *s, GMT_cal_rd *rd) {

	/* Return -1 on failure.  Set rd and return 0 on success.  
	
	For gregorian calendars.  */
	
	int	i, k, ival[3];
	
	if (GMT_io.date_input.day_of_year) {
		/* Calendar uses year and day of year format.  */
		if ( (k = sscanf (s, GMT_io.date_input.format,
			&ival[GMT_io.date_input.order[0]],
			&ival[GMT_io.date_input.order[3]]) ) == 0) return (-1);
		if (k < 2) {
			if (GMT_io.date_input.truncated_cal_is_ok) {
				ival[1] = 1;	/* Set first day of year  */
			}
			else {
				return (-1);
			}
		}
		if (GMT_io.date_input.Y2K_year) {
			if (ival[0] < 0 || ival[0] > 99) return (-1);
			ival[0] = GMT_y2_to_y4_yearfix (ival[0]);
		}
		k = (GMT_is_gleap(ival[0])) ? 366 : 365;
		if (ival[1] < 1 || ival[1] > k) return (-1);
		*rd = GMT_rd_from_gymd (ival[0], 1, 1) + ival[1] - 1;
		return (0);
	}
	
	/* Get here when calendar type has months and days of months.  */
	if ( (k = sscanf (s, GMT_io.date_input.format,
		&ival[GMT_io.date_input.order[0]],
		&ival[GMT_io.date_input.order[1]],
		&ival[GMT_io.date_input.order[2]]) ) == 0) return (-1);
	if (k < 3) {
		if (GMT_io.date_input.truncated_cal_is_ok) {
			ival[2] = 1;	/* Set first day of month  */
			if (k == 1) ival[1] = 1;	/* Set first month of year */
		}
		else {
			return (-1);
		}
	}
	if (GMT_io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = GMT_y2_to_y4_yearfix (ival[0]);
	}
	if (ival[1] < 1 || ival[1] > 12 || ival[2] < 1) return (-1);
	if (ival[1] == 2) {
		k = (GMT_is_gleap (ival[0]) ) ? 29 : 28;
		if (ival[2] > k) return (-1);
	}
	else {
		i = ival[1]%2;
		if (ival[1] < 8) {
			k = 30 + i;
		}
		else {
			k = 31 - i;
		}
		if (ival[2] > k) return (-1);
	}
	*rd = GMT_rd_from_gymd (ival[0], ival[1], ival[2]);
	return (0);
}
		

int	GMT_scanf_geo (char *s, double *val) {

	/* Try to read a character string token stored in s,
	knowing that it should be a geographical variable.  
	If successful, stores value in val and returns one of
	GMT_IS_UNKNOWN, GMT_IS_GEO, GMT_IS_LAT, GMT_IS_LON, 
	whichever can be determined from the format of s.
	If unsuccessful, does not store anything in val and
	returns GMT_IS_NAN.  
	This should have essentially the same functionality
	as GMT_scanf_old(), except that the expectation is
	now used and returned, and this also permits a double
	precision format in the minutes or seconds, and does
	more error checking.  However, this is not optimized
	for speed (yet).  WHFS, 16 Aug 2001
	
	note:  mismatch handling (e.g. this routine finds a lon
	but calling routine expected a lat) is not done here.

	*/
	
	char	scopy[64], suffix, *p, *p2;
	double	dd, dm, ds;
	int	retval = GMT_IS_UNKNOWN;
	int	k, id, im, ncolons;
	BOOLEAN	negate = FALSE;

	k = strlen(s);
	if (k == 0) return (GMT_IS_NAN);
	if (!(isdigit ( (int)s[k-1]) ) ) {
		suffix = s[k-1];
		switch (suffix) {
			case 'W':
			case 'w':
				negate = TRUE;
				retval = GMT_IS_LON;
				break;
			case 'E':
			case 'e':
				retval = GMT_IS_LON;
				break;
			case 'S':
			case 's':
				negate = TRUE;
				retval = GMT_IS_LAT;
				break;
			case 'N':
			case 'n':
				retval = GMT_IS_LAT;
				break;
			default:
				return (GMT_IS_NAN);
				break;
		}
		k--;
	}
	if (k >= 64) return (GMT_IS_NAN);
	strncpy (scopy, s, k);	/* Copy all but the suffix  */
	scopy[k] = 0;
	if ( (p = strpbrk (scopy, "dD")) ) {
		/* We found a D or d.  */
		if (strlen(p) < 1 || (strpbrk (&p[1], "dD:") ) ){
			/* It is at the end, or followed by a 
				colon or another d or D.  */
			return (GMT_IS_NAN);
		}
		/* Map it to an e, permitting FORTRAN Double
			Precision formats.  */
		p[0] = 'e';
	}
	p = scopy;
	while ( (p2 = strpbrk (p, ":")) ) {
		if (strlen(p2) < 1) {
			/* Shouldn't end with a colon  */
			return (GMT_IS_NAN);
		}
		ncolons++;
		if (ncolons > 2) return (GMT_IS_NAN);
		p = &p2[1];
	}
	
	if (ncolons && retval == GMT_IS_UNKNOWN) retval = GMT_IS_GEO;
	
	switch (ncolons) {
		case 0:
			if ( (sscanf(scopy, "%lf", &dd) ) != 1) return (GMT_IS_NAN);
			break;
		case 1:
			if ( (sscanf(scopy, "%d:%lf", &id, &dm) ) != 2) return (GMT_IS_NAN);
			dd = (id < 0) ? id + dm * GMT_I_60 : id - dm * GMT_I_60;
			break;
		case 2:
			if ( (sscanf(scopy, "%d:%d:%lf", &id, &im, &ds) ) != 3) return (GMT_IS_NAN);
			dd = im * GMT_I_60 + ds * GMT_I_3600;
			if (id < 0) {
				dd = id - dd;
			}
			else {
				dd = id + dd;
			}
			break;
	}
	*val = (negate) ? -dd : dd;
	return (retval);
}

		
int	GMT_scanf_float (char *s, double *val) {

	/* Try to decode a value from s and store
	in val.  S should not have any special format
	(neither geographical, with suffixes or
	separating colons, nor calendar nor clock).
	However, D and d are permitted to map to e
	if this would result in a success.  This
	allows Fortran Double Precision to be readable.
	
	On success, return GMT_IS_UNKNOWN and store val.
	On failure, return GMT_IS_NAN and do not touch val.
	*/

	char	scopy[64], *p;
	double	x;
	int	j,k;
	
	x = strtod (s, &p);
	if (p[0] == 0) {
		/* Success (non-Fortran).  */
		*val = x;
		return (GMT_IS_UNKNOWN);
	}
	if (p[0] != 'D' && p[0] != 'd') return (GMT_IS_NAN);
	k = strlen(p);
	if (k == 1) {
		/* A string ending in e would be invalid  */
		return (GMT_IS_NAN);
	}
	/* Make a copy of s in scopy, mapping the d or D to an e:  */
	j = strlen(s);
	if (j > 64) return (GMT_IS_NAN);
	j -= k;
	strncpy (scopy, s, (size_t)j );
	scopy[j] = 'e';
	strcpy (&scopy[j+1], &p[1]);
	x = strtod(scopy, &p);
	if (p[0] != 0) return (GMT_IS_NAN);
	*val = x;
	return (GMT_IS_UNKNOWN);
}

int	GMT_scanf (char *s, int expectation, double *val) {

	/* Called with s pointing to a char string, expectation
	indicating what is known/required/expected about the
	format of the string.  Attempts to decode the string to
	find a double value.  Upon success, loads val and 
	returns type found.  Upon failure, does not touch val,
	and returns GMT_IS_NAN.  Expectations permitted on call
	are
		GMT_IS_UNKNOWN	we expect an uncomplicated float.
	*/
	
	char	calstring[64], clockstring[64], *p;
	double	x;
	int	callen, clocklen;
	GMT_cal_rd rd;
	
	
	if (expectation & GMT_IS_GEO) {
		/* True if either a lat or a lon is expected  */
		return (GMT_scanf_geo (s, val));
	}
	
	if (expectation == GMT_IS_UNKNOWN) {
		/* True if no special format is expected or allowed  */
		return (GMT_scanf_float (s, val));
	}
	
	if (expectation == GMT_IS_RELTIME) {
		/* True if we expect to read a float with no special
		formatting, and then convert that float to our time
		based on user's epoch and units.  */
		if ( ( GMT_scanf_float (s, &x) ) == GMT_IS_NAN) return (GMT_IS_NAN);
		*val = GMT_dt_from_usert (x);
		return (GMT_IS_ABSTIME);
	}
	
	if (expectation == GMT_IS_ABSTIME) {
		/* True when we expect to read calendar and/or
		clock strings in user-specified formats.  If both 
		are present, they must be in the form
		<calendar_string>T<clock_string>.
		If only a calendar string is present, then either
		<calendar_string> or <calendar_string>T are valid.
		If only a clock string is present, then it must
		be preceded by a T:  T<clock_string>, and the time
		will be treated as if on day one of our calendar.  */
		
		callen = strlen (s);
		if (callen < 2) return (GMT_IS_NAN);	/* Maybe should be more than 2  */
		
		if ( (p = strchr ( s, (int)('T') ) ) == NULL) {
			/* There is no T.  Put all of s in calstring.  */
			clocklen = 0;
			strcpy (calstring, s);
		}
		else {
			clocklen = strlen(p);
			callen -= clocklen;
			strncpy (calstring, s, callen);
			strcpy (clockstring, &p[1]);
			clocklen--;
		}
		x = 0.0;
		if (clocklen && GMT_scanf_clock (clockstring, &x) ) {
			return (GMT_IS_NAN);
		} 
		rd = 1;
		if (callen && GMT_scanf_calendar (calstring, &rd) ) {
			return (GMT_IS_NAN);
		}
		*val = GMT_rdc2dt (rd, x);
		return (GMT_IS_ABSTIME);
	}
	
	if (expectation == GMT_IS_ARGTIME) {
		return (GMT_scanf_argtime (s, val));
	}
	
	fprintf (stderr, "GMT_LOGIC_BUG:  GMT_scanf() called with invalid expectation.\n");
	return (GMT_IS_NAN);
}

int	GMT_scanf_argtime (char *s, double *val) {

	/* s points to a string which may be absolute or relative
	time, but must be interpreted as time, and must be */
		
	fprintf (stderr, "GMT_scanf_argtime is a dummy routine.\n");
	return (GMT_IS_NAN);
}

