/*--------------------------------------------------------------------
 *	$Id: gmt_io.c,v 1.45 2002-02-23 03:39:58 pwessel Exp $
 *
 *	Copyright (c) 1991-2002 by P. Wessel and W. H. F. Smith
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
int GMT_ascii_input (FILE *fp, int *n, double **ptr);		/* Decode ASCII input records */
int GMT_bin_double_input (FILE *fp, int *n, double **ptr);	/* Decode binary double input records */
int GMT_bin_float_input (FILE *fp, int *n, double **ptr);	/* Decode binary float input records */
int GMT_ascii_output (FILE *fp, int n, double *ptr);		/* Write ASCII output records */
int GMT_bin_double_output (FILE *fp, int n, double *ptr);	/* Write binary double output records */
int GMT_bin_float_output (FILE *fp, int n, double *ptr);	/* Write binary float output records */
int GMT_ascii_output_one (FILE *fp, double x, int col);		/* Writes one item to output in ascii format */
void GMT_adjust_periodic ();					/* Add/sub 360 as appropriate */
void GMT_decode_calclock_formats ();
void GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S, int mode);
void GMT_date_C_format (char *template, struct GMT_DATE_IO *S, int mode);
void GMT_clock_C_format (char *template, struct GMT_CLOCK_IO *S, int mode);
void GMT_geo_C_format (char *template, struct GMT_GEO_IO *S);
void GMT_plot_C_format (char *template, struct GMT_GEO_IO *S);
void GMT_get_dms_order (char *text, struct GMT_GEO_IO *S);

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
	/* No need to init the structure as this is done in gmt_init.h directory */

	int i;
	
	GMT_input  = GMT_input_ascii = GMT_ascii_input;
	GMT_output = GMT_ascii_output;

	GMT_io.give_report = TRUE;
	
	GMT_io.skip_if_NaN = (BOOLEAN *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (BOOLEAN), GMT_program);
	GMT_io.in_col_type  = (int *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (int), GMT_program);
	GMT_io.out_col_type = (int *)GMT_memory (VNULL, (size_t)BUFSIZ, sizeof (int), GMT_program);
	for (i = 0; i < 2; i++) GMT_io.skip_if_NaN[i] = TRUE;						/* x/y must be non-NaN */
	for (i = 0; i < 2; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = GMT_IS_UNKNOWN;	/* Must be told [or find out] what x/y are */
	for (i = 2; i < BUFSIZ; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = GMT_IS_FLOAT;	/* Other columns default to floats */

	/* Set the Y2K conversion parameters once */
	
	GMT_Y2K_fix.y2_cutoff = abs (gmtdefs.Y2K_offset_year) % 100;
	GMT_Y2K_fix.y100 = gmtdefs.Y2K_offset_year - GMT_Y2K_fix.y2_cutoff;
	GMT_Y2K_fix.y200 = GMT_Y2K_fix.y100 + 100;

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
		strcpy (GMT_io.a_mode, "ab+");
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
		if (line[i] == '\r') i--;	/* DOS CR/LF, replace with LF only */
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
			if (GMT_io.give_report && (GMT_io.n_bad_records == 1)) {	/* Report 1st occurrence */
				fprintf (stderr, "%s: Encountered first invalid record near/at line # %d\n", GMT_program, GMT_io.rec_no);
				fprintf (stderr, "%s: Likely causes:\n", GMT_program);
				fprintf (stderr, "%s: (1) Invalid x and/or y values, i.e. NaNs or garbage in text strings.\n", GMT_program);
				fprintf (stderr, "%s: (2) Incorrect data type assumed if -J, -f are not set or set incorrectly.\n", GMT_program);
				fprintf (stderr, "%s: (3) The -: switch is implied but not set.\n", GMT_program);
				fprintf (stderr, "%s: (4) Input file in multiple segment format but the -M switch is not set.\n", GMT_program);
			}
		}
		else
			done = TRUE;
	}
	*ptr = GMT_data;
	GMT_io.status = (col_no == *n || *n == BUFSIZ) ? 0 : GMT_IO_MISMATCH;
	if (*n == BUFSIZ) *n = col_no;

	if (gmtdefs.xy_toggle) d_swap (GMT_data[0], GMT_data[1]);	/* Got lat/lon instead of lon/lat */
	if (GMT_io.in_col_type[0] & GMT_IS_GEO) GMT_adjust_periodic ();	/* Must account for periodicity in 360 */
	
	return (col_no);
}

char *GMT_fgets (char *record, int maxlength, FILE *fp)
{
	return (fgets (record, maxlength, fp));
}


int GMT_bin_double_input (FILE *fp, int *n, double **ptr)
{
	int n_read, i;

	GMT_io.status = 0;
	if ((n_read = fread ((void *) GMT_data, sizeof (double), (size_t)(*n), fp)) != (*n)) {
		GMT_io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
	}

	for (i = 0; i < (*n); i++) if (GMT_io.in_col_type[i] == GMT_IS_RELTIME) GMT_data[i] = GMT_dt_from_usert (GMT_data[i]);
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
	if (GMT_io.in_col_type[0] & GMT_IS_GEO) GMT_adjust_periodic ();	/* Must account for periodicity in 360 */

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
		for (i = 0; i < n_read; i++) GMT_data[i] = (double)((GMT_io.in_col_type[i] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double)GMT_f[i]) : GMT_f[i]);
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
	if (GMT_io.in_col_type[0] & GMT_IS_GEO) GMT_adjust_periodic ();	/* Must account for periodicity in 360 */

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
	int i, last, e = 0, wn = 0;
	
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);		/* Write lat/lon instead of lon/lat */
	last = n - 1;						/* Last record, need to output linefeed instead of delimiter */

	for (i = 0; i < n && e >= 0; i++) {			/* Keep writing all fields unless there is a read error (e == -1) */
		e = GMT_ascii_output_one (fp, ptr[i], i);	/* Write one item without any separator at the end */

		if (i == last)					/* This is the last field, must add newline */
			putc ('\n', fp);
		else if (gmtdefs.field_delimiter[0])		/* Not last field, and a separator is required */
			fprintf (fp, "%s", gmtdefs.field_delimiter);
			
		wn += e;
	}
	return ((e < 0) ? e : wn);
}

void GMT_ascii_format_one (char *text, double x, int type)
{
	
	if (GMT_is_dnan (x)) {
		sprintf (text, "NaN");
		return;
	}
	switch (type) {
		case GMT_IS_FLOAT:
		case GMT_IS_UNKNOWN:
			sprintf (text, gmtdefs.d_format, x);
			break;
		case GMT_IS_LON:
			GMT_format_geo_output (FALSE, x, text);
			break;
		case GMT_IS_LAT:
			GMT_format_geo_output (TRUE, x, text);
			break;
		case GMT_IS_RELTIME:
			sprintf (text, gmtdefs.d_format, GMT_usert_from_dt ( (GMT_dtime) x));
			break;
		case GMT_IS_ABSTIME:
			GMT_format_abstime_output ((GMT_dtime) x, text);
			break;
	}
}

int GMT_ascii_output_one (FILE *fp, double x, int col)
{
	char text[32];
	
	GMT_ascii_format_one (text, x, GMT_io.out_col_type[col]);
	return (fprintf (fp, "%s", text));
}

void GMT_lon_range_adjust (int range, double *lon)
{
	switch (range) {	/* Adjust to the desired range */
		case 0:
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) >= 360.0) (*lon) -= 360.0;
			break;
		case 1:
			while ((*lon) <= -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		default:
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) > 180.0) (*lon) -= 360.0;
			break;
	}
}

void GMT_format_geo_output (BOOLEAN is_lat, double geo, char *text)
{
	int d, m, s, m_sec;
	char letter;
	BOOLEAN seconds;
	
	if (!is_lat) GMT_lon_range_adjust (GMT_io.geo.range, &geo);
	if (GMT_io.geo.decimal) {	/* Easy */
		sprintf (text, gmtdefs.d_format, geo);
		return;
	}
	
	if (GMT_io.geo.wesn) {	/* Trailing WESN */
		if (is_lat)
			letter = (fabs (geo) < GMT_CONV_LIMIT) ? 0 : ((geo < 0.0) ? 'S' : 'N');
		else
			letter = (fabs (geo) < GMT_CONV_LIMIT || fabs (geo - 180.0) < GMT_CONV_LIMIT) ? 0 : ((geo < 0.0) ? 'W' : 'E');
		geo = fabs (geo);
	}
	else	/* No letter means we print the NULL character */
		letter = 0;
		
	seconds = (GMT_io.geo.order[2] > 0);		/* Are we doing dd:mm:ss */
	GMT_geo_to_dms (geo, seconds, GMT_io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
	if (GMT_io.geo.n_sec_decimals) {		/* Wanted fraction printed */
		if (seconds)
			sprintf (text, GMT_io.geo.y_format, d, m, s, m_sec, letter);
		else
			sprintf (text, GMT_io.geo.y_format, d, m, m_sec, letter);
	}
	else if (seconds)
		sprintf (text, GMT_io.geo.y_format, d, m, s, letter);
	else
		sprintf (text, GMT_io.geo.y_format, d, m, letter);
}

void GMT_geo_to_dms (double val, BOOLEAN seconds, double fact, int *d, int *m,  int *s,  int *ix)
{
	/* Convert floating point degrees to dd:mm[:ss][.xxx] */
	BOOLEAN minus;
	int isec, imin;
	double sec, fsec, min, fmin, step;
	
	minus = (val < 0.0);
	step = (fact == 0.0) ? 0.0 : 0.5 / fact;  		/* Precision desired in seconds (or minutes) */
	
	if (seconds) {		/* Want dd:mm:ss[.xxx] format */
		sec = GMT_DEG2SEC_F * fabs (val);		/* Convert to seconds */
		isec = (int)floor (sec + step);			/* Integer seconds */
		fsec = sec - (double)isec;  			/* Leftover fractional second */
		*d = isec / GMT_DEG2SEC_I;			/* Integer degrees */
		isec -= ((*d) * GMT_DEG2SEC_I);			/* Left-over seconds in the last degree */
		*m = isec / GMT_MIN2SEC_I;			/* Integer minutes */
		isec -= ((*m) * GMT_MIN2SEC_I);			/* Leftover seconds in the last minute */
		*s = isec;					/* Integer seconds */
		*ix = irint (fsec * fact);			/* fractional seconds scaled to integer */
	}
	else {		/* Want dd:mm[.xx] format */
		min = GMT_DEG2MIN_F * fabs (val);		/* Convert to minutes */
		imin = (int)floor (min + step);			/* Integer minutes */
		fmin = min - (double)imin;  			/* Leftover fractional minute */
		*d = imin / GMT_DEG2MIN_I;			/* Integer degrees */
		imin -= ((*d) * GMT_DEG2MIN_I);			/* Left-over seconds in the last degree */
		*m = imin;					/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = irint (fmin * fact);			/* Fractional minutes scaled to integer */
	}
	if (minus) *d = -(*d);
}

void GMT_format_abstime_output (GMT_dtime dt, char *text)
{
	char date[GMT_CALSTRING_LENGTH], clock[GMT_CALSTRING_LENGTH];

	GMT_format_calendar (date, clock, &GMT_io.date_output, &GMT_io.clock_output, FALSE, 0, dt);
	sprintf (text, "%sT%s\0", date, clock);
}

int GMT_bin_double_output (FILE *fp, int n, double *ptr)
{
	int i;
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);	/* Write lat/lon instead of lon/lat */
	for (i = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_RELTIME) ptr[i] = GMT_usert_from_dt ((GMT_dtime) ptr[i]);
		if (GMT_io.out_col_type[i] == GMT_IS_LON) GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
	}

	return (fwrite ((void *) ptr, sizeof (double), (size_t)n, fp));
}
	
int GMT_bin_float_output (FILE *fp, int n, double *ptr)
{
	int i;
	static float GMT_f[BUFSIZ];
	
	if (gmtdefs.xy_toggle) d_swap (ptr[0], ptr[1]);	/* Write lat/lon instead of lon/lat */
	for (i = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_RELTIME)
			GMT_f[i] = (float) GMT_usert_from_dt ((GMT_dtime) ptr[i]);
		else if (GMT_io.out_col_type[i] == GMT_IS_LON) {
			GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
			GMT_f[i] = (float) ptr[i];
		}
		else
			GMT_f[i] = (float) ptr[i];
	}
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

			/* Set this if file is periodic, is grid registered, but repeating column or row is missing from input */

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
		strcpy (GMT_io.a_mode, "ab+");
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

/* NOTE: In the following we check GMT_io.in_col_type[2] and GMT_io.out_col_type[2] for formatting help for the first column.
 * We use column 3 ([2]) instead of the first ([0]) since we really are dealing with the z in z (x,y) here
 * and the x,y are implicit from the -R -I arguments.
 */
 
int GMT_a_read (FILE *fp, double *d)
{
	char line[64];
	if (fgets (line, 64, fp)) {	/* Read was successful */
		line[strlen(line)-1] = '\0';	/* Chop off the '\n' at end of line */
		GMT_scanf (line, GMT_io.in_col_type[2], d);	/* Convert whatever it is to double */
		return (1);
	}
	return (0);	/* Upon failure to read the line */
}

int GMT_c_read (FILE *fp, double *d)
{
	char c;
	if (fread ((void *)&c, sizeof (char), 1, fp)) {
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) c) : (double) c;
		return (1);
	}
	return (0);
}

int GMT_u_read (FILE *fp, double *d)
{
	unsigned char u;
	if (fread ((void *)&u, sizeof (unsigned char), 1, fp)) {
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) u) : (double) u;
		return (1);
	}
	return (0);
}

int GMT_h_read (FILE *fp, double *d)
{
	short int h;
	if (fread ((void *)&h, sizeof (short int), 1, fp)) {
		if (GMT_do_swab) h = GMT_swab2 (h);
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) h) : (double) h;
		return (1);
	}
	return (0);
}

int GMT_H_read (FILE *fp, double *d)
{
	unsigned short int h;
	if (fread ((void *)&h, sizeof (unsigned short int), 1, fp)) {
		if (GMT_do_swab) h = GMT_swab2 (h);
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) h) : (double) h;
		return (1);
	}
	return (0);
}

int GMT_i_read (FILE *fp, double *d)
{
	int i;
	if (fread ((void *)&i, sizeof (int), 1, fp)) {
		if (GMT_do_swab) i = GMT_swab4 (i);
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) i) : (double) i;
		return (1);
	}
	return (0);
}

int GMT_I_read (FILE *fp, double *d)
{
	unsigned int i;
	if (fread ((void *)&i, sizeof (unsigned int), 1, fp)) {
		if (GMT_do_swab) i = GMT_swab4 (i);
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) i) : (double) i;
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
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) l) : (double) l;
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
		*d = (GMT_io.in_col_type[2] == GMT_IS_RELTIME) ? GMT_dt_from_usert ((double) f) : (double) f;
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
		if (GMT_io.in_col_type[2] == GMT_IS_RELTIME) *d = GMT_dt_from_usert (*d);
		return (1);
	}
	return (0);
}

int GMT_a_write (FILE *fp, double d)
{
	int n = 0;
	n = GMT_ascii_output_one (fp, d, 2);
	fprintf (fp, "\n"); 
	return (n);
}

int GMT_c_write (FILE *fp, double d)
{
	char c;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	c = (char) d;
	return (fwrite ((void *)&c, sizeof (char), (size_t)1, fp));
}

int GMT_u_write (FILE *fp, double d)
{
	unsigned char u;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	u = (unsigned char) d;
	return (fwrite ((void *)&u, sizeof (unsigned char), (size_t)1, fp));
}

int GMT_h_write (FILE *fp, double d)
{
	short int h;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	h = (short int) d;
	return (fwrite ((void *)&h, sizeof (short int), (size_t)1, fp));
}

int GMT_H_write (FILE *fp, double d)
{
	unsigned short int h;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	h = (unsigned short int) d;
	return (fwrite ((void *)&h, sizeof (unsigned short int), (size_t)1, fp));
}

int GMT_i_write (FILE *fp, double d)
{
	int i;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	i = (int) d;
	return (fwrite ((void *)&i, sizeof (int), (size_t)1, fp));
}

int GMT_I_write (FILE *fp, double d)
{
	unsigned int i;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	i = (unsigned int) d;
	return (fwrite ((void *)&i, sizeof (unsigned int), (size_t)1, fp));
}

int GMT_l_write (FILE *fp, double d)
{
	long int l;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	l = (long int) d;
	return (fwrite ((void *)&l, sizeof (long int), (size_t)1, fp));
}

int GMT_f_write (FILE *fp, double d)
{
	float f;
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
	f = (float) d;
	return (fwrite ((void *)&f, sizeof (float), (size_t)1, fp));
}

int GMT_d_write (FILE *fp, double d)
{
	if (GMT_io.out_col_type[2] == GMT_IS_RELTIME) d = GMT_usert_from_dt ( (GMT_dtime) d);
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

void GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S, int mode)
{	/* Reads a YYYY-MM-DD or YYYYMMDD-like string and determines order.
	 * order[0] is the order of the year, [1] is month, etc.
	 * Items not encountered are left as -1. mode is 0 for text i/o
	 * and 1 for plot format.
	 */

	int i, j, order, n_y, n_m, n_d, n_j, n_w, n_delim, last, error = 0;
	
	for (i = 0; i < 4; i++) S->item_order[i] = S->item_pos[i] = -1;	/* Meaning not encountered yet */
	
	n_y = n_m = n_d = n_j = n_w = n_delim = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	
	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = TRUE;
		i++;
	}
	for (order = 0; i < strlen (text); i++) {
		switch (text[i]) {
			case 'y':	/* Year */
				if (S->item_pos[0] < 0)		/* First time we encounter a y */
					S->item_pos[0] = order++;
				else if (text[i-1] != 'y')	/* Done it before, previous char must be y */
					error++;
				n_y++;
				break;
			case 'm':	/* Month */
				if (S->item_pos[1] < 0)		/* First time we encounter a m */
					S->item_pos[1] = order++;
				else if (text[i-1] != 'm')	/* Done it before, previous char must be m */
					error++;
				n_m++;
				break;
			case 'o':	/* Month name (plot output only) */
				if (S->item_pos[1] < 0)		/* First time we encounter an o */
					S->item_pos[1] = order++;
				else				/* Done it before is error here */
					error++;
				S->mw_text = TRUE;
				n_m = 2;
				break;
				
			case 'W':	/* ISO Week flag */
				S->iso_calendar = TRUE;
				break;
			case 'w':	/* Iso Week */
				if (S->item_pos[1] < 0) {		/* First time we encounter a w */
					S->item_pos[1] = order++;
					if (text[i-1] != 'W') error++;	/* Must have the format W just before */
				}
				else if (text[i-1] != 'w')	/* Done it before, previous char must be w */
					error++;
				n_w++;
				break;
			case 'u':	/* Iso Week name ("Week 04") (plot output only) */
				S->iso_calendar = TRUE;
				if (S->item_pos[1] < 0) {		/* First time we encounter a u */
					S->item_pos[1] = order++;
				}
				else 				/* Done it before is an error */
					error++;
				S->mw_text = TRUE;
				n_w = 2;
				break;
			case 'd':	/* Day of month */
				if (S->item_pos[2] < 0)		/* First time we encounter a d */
					S->item_pos[2] = order++;
				else if (text[i-1] != 'd')	/* Done it before, previous char must be d */
					error++;
				n_d++;
				break;
			case 'j':	/* Day of year  */
				S->day_of_year = TRUE;
				if (S->item_pos[3] < 0)		/* First time we encounter a j */
					S->item_pos[3] = order++;
				else if (text[i-1] != 'j')	/* Done it before, previous char must be j */
					error++;
				n_j++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
				break;
		}
	}
	
	/* Then get the actual order by inverting table */
	
	for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) if (S->item_pos[j] == i) S->item_order[i] = j;
	S->Y2K_year = (n_y == 2);		/* Must supply the century when reading and take it out when writing */
	S->truncated_cal_is_ok = TRUE;		/* May change in the next loop */
	for (i = 1, last = S->item_order[0]; S->truncated_cal_is_ok && i < 4; i++) {
		if (S->item_order[i] == -1) continue;
		if (S->item_order[i] < last) S->truncated_cal_is_ok = FALSE;
		last = S->item_order[i];
	}
	if (S->mw_text && mode < 2) error = TRUE;
	last = (n_y > 0) + (n_m > 0) + (n_w > 0) + (n_d > 0) + (n_j > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);				/* If there are delimiters, must be one less than the items */
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
	
	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */
	sequence[0] = sequence[1] = sequence[2] = -1;
	
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	n_h = n_m = n_s = n_x = n_dec = n_delim = 0;
	
	/* Determine if we do 12-hour clock (and what form of am/pm suffix) or 24-hour clock */
	
	if ((p = strstr (text, "am"))) {	/* Want 12 hour clock with am/pm */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "am");
		strcpy (S->ampm_suffix[1], "pm");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "AM"))) {	/* Want 12 hour clock with AM/PM */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "AM");
		strcpy (S->ampm_suffix[1], "PM");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "a.m."))) {	/* Want 12 hour clock with a.m./p.m. */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "a.m.");
		strcpy (S->ampm_suffix[1], "p.m.");
		off = (int)(p) - (int)text;
	}
	else if ((p = strstr (text, "A.M."))) {	/* Want 12 hour clock with A.M./P.M. */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "A.M.");
		strcpy (S->ampm_suffix[1], "P.M.");
		off = (int)(p) - (int)text;
	}
	else
		off = strlen (text);
	
	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = TRUE;
		i++;
	}
	for (order = 0; i < off; i++) {
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
				else {	/* Must be a delimiter */
					if (n_delim == 2)
						error++;
					else
						S->delimiter[n_delim++][0] = text[i];
				}
				break;
			case 'x':	/* Fraction of seconds */
				if (n_x > 0 && text[i-1] != 'x')	/* Must follow a previous x */
					error++;
				n_x++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
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
	error += (n_delim && (last - 1) != n_delim);	/* If there are delimiters, must be one less than the items */
	error += (!(n_h == 0 || n_h == 2) || !(n_m == 0 || n_m == 2) || !(n_s == 0 || n_s == 2));	/* h, m, s are all either 2 or 0 */
	error += (n_s > n_m || n_m > n_h);		/* Cannot have secs without m etc */
	error += (n_x && n_dec != 1);			/* .xxx is the proper form */
	error += (n_x == 0 && n_dec);			/* Period by itself and not delimiter? */
	error += (n_dec > 1);				/* Only one period with xxx */
	S->n_sec_decimals = n_x;
	S->f_sec_to_int = rint (pow (10.0, (double)S->n_sec_decimals));			/* To scale fractional seconds to an integer form */
	if (error) {
		fprintf (stderr, "%s: ERROR: Unacceptable clock template %s\n", GMT_program, text);
		exit (EXIT_FAILURE);
	}
}

void GMT_get_dms_order (char *text, struct GMT_GEO_IO *S)
{	/* Reads a ddd:mm:ss-like string and determines order.
	 * order[0] is the order of the degree, [1] is minutes, etc.
	 * Order is checked since we only allow d, m, s in that order.
	 * Items not encountered are left as -1.
	 */

	int i1, i, j, order, n_d, n_m, n_s, n_x, n_dec, sequence[3], n_delim, last, error = 0;
	BOOLEAN big_to_small;
	
	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */
	
	n_d = n_m = n_s = n_x = n_dec = n_delim = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	sequence[0] = sequence[1] = sequence[2] = -1;
	
	S->range = 2;			/* -80/+180 range, may be overwritten below by + or - */
	S->decimal = S->wesn = S->no_sign = FALSE;
	
	i1 = strlen (text) - 1;
	for (i = order = 0; i <= i1; i++) {
		switch (text[i]) {
			case '+':	/* Want [0-360> range [Default] */
				S->range = 0;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case '-':	/* Want <-360-0] range [i.e., western longitudes] */
				S->range = 1;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case 'D':	/* Want to use decimal degrees using D_FORMAT [Default] */
				S->decimal = TRUE;
				if (i > 1) error++;		/* Only valid as first or second flag */
				break;
			case 'F':	/* Want to use WESN to encode sign */
				S->wesn = TRUE;
				if (i != i1 || S->no_sign) error++;		/* Only valid as last flag */
				break;
			case 'A':	/* Want no sign in plot string */
				S->no_sign = TRUE;
				if (i != i1 || S->wesn) error++;		/* Only valid as last flag */
				break;
			case 'd':	/* Degree */
				if (S->order[0] < 0)		/* First time we encounter a d */
					S->order[0] = order++;
				else if (text[i-1] != 'd')	/* Done it before, previous char must be y */
					error++;
				n_d++;
				break;
			case 'm':	/* Minute */
				if (S->order[1] < 0)		/* First time we encounter a m */
					S->order[1] = order++;
				else if (text[i-1] != 'm')	/* Done it before, previous char must be m */
					error++;
				n_m++;
				break;
			case 's':	/* Seconds */
				if (S->order[2] < 0) {		/* First time we encounter a s */
					S->order[2] = order++;
				}
				else if (text[i-1] != 's')	/* Done it before, previous char must be s */
					error++;
				n_s++;
				break;
			case '.':	/* Decimal point for seconds? */
				if (text[i+1] == 'x')
					n_dec++;
				else {	/* Must be a delimiter */
					if (n_delim == 2)
						error++;
					else
						S->delimiter[n_delim++][0] = text[i];
				}
				break;
			case 'x':	/* Fraction of seconds */
				if (n_x > 0 && text[i-1] != 'x')	/* Must follow a previous x */
					error++;
				n_x++;
				break;
			default:	/* Delimiter of some kind */
				if (n_delim == 2)
					error++;
				else
					S->delimiter[n_delim++][0] = text[i];
				break;
		}
	}
	
	if (S->decimal) return;	/* Easy formatting choice */
		
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
	last = (n_d > 0) + (n_m > 0) + (n_s > 0);	/* This is the number of items to read */
	error += (n_delim && (last - 1) != n_delim);	/* If there are delimiters, must be one less than the items */
	error += (!(n_d == 0 || n_d == 3) || !(n_m == 0 || n_m == 2) || !(n_s == 0 || n_s == 2));	/* d, m, s are all either 2(3) or 0 */
	error += (n_s > n_m || n_m > n_d);		/* Cannot have secs without m etc */
	error += (n_x && n_dec != 1);			/* .xxx is the proper form */
	error += (n_x == 0 && n_dec);			/* Period by itself and not delimiter? */
	error += (n_dec > 1);				/* Only one period with xxx */
	S->n_sec_decimals = n_x;
	S->f_sec_to_int = rint (pow (10.0, (double)S->n_sec_decimals));			/* To scale fractional seconds to an integer form */
	if (error) {
		fprintf (stderr, "%s: ERROR: Unacceptable dmmss template %s\n", GMT_program, text);
		exit (EXIT_FAILURE);
	}
}

void GMT_decode_calclock_formats ()
{
	GMT_date_C_format (gmtdefs.input_date_format, &GMT_io.date_input, 0);
	GMT_date_C_format (gmtdefs.output_date_format, &GMT_io.date_output, 1);
	GMT_date_C_format (gmtdefs.plot_date_format, &GMT_plot_calclock.date, 2);
	GMT_clock_C_format (gmtdefs.input_clock_format, &GMT_io.clock_input, 0);
	GMT_clock_C_format (gmtdefs.output_clock_format, &GMT_io.clock_output, 1);
	GMT_clock_C_format (gmtdefs.plot_clock_format, &GMT_plot_calclock.clock, 2);
	GMT_geo_C_format (gmtdefs.output_degree_format, &GMT_io.geo);
	GMT_plot_C_format (gmtdefs.plot_degree_format, &GMT_plot_calclock.geo);
}

void GMT_clock_C_format (char *template, struct GMT_CLOCK_IO *S, int mode)
{
	/* Determine the order of H, M, S in input and output clock strings,
	 * as well as the number of decimals in output seconds (if any), and
	 * if a 12- or 24-hour clock is used.
	 * mode is 0 for input and 1 for output format
	 */
	 
	/* Get the order of year, month, day or day-of-year in input/output formats for dates */
	
	GMT_get_hms_order (template, S);
	
	/* Craft the actual C-format to use for input/output clock strings */
		
	if (S->order[0] >= 0) {	/* OK, at least hours is needed */
		char fmt[32];
		if (S->compact)
			sprintf (S->format, "%%d\0");
		else
			(mode) ? sprintf (S->format, "%%2.2d\0") : sprintf (S->format, "%%2d\0");
		if (S->order[1] >= 0) {	/* Need minutes too*/
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			if (S->compact)
				sprintf (fmt, "%%d\0");
			else
				(mode) ? sprintf (fmt, "%%2.2d\0") : sprintf (fmt, "%%2d\0");
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* .. and seconds */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				if (mode) {	/* Output format */
					(S->compact) ? sprintf (fmt, "%%d\0") : sprintf (fmt, "%%2.2d\0");
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
		if (mode && S->twelve_hr_clock) {	/* Finally add %s for the am, pm string */
			sprintf (fmt, "%%s\0");
			strcat (S->format, fmt);
		}
	}
}

void GMT_date_C_format (char *template, struct GMT_DATE_IO *S, int mode)
{
	/* Determine the order of Y, M, D, J in input and output date strings.
	* mode is 0 for input, 1 for output, and 2 for plot output.
	 */
	 
	char fmt[32];
	int k;
	
	/* Get the order of year, month, day or day-of-year in input/output formats for dates */
	
	GMT_get_ymdj_order (template, S, mode);
		
	/* Craft the actual C-format to use for i/o date strings */

	if (S->item_order[0] >= 0 && S->iso_calendar) {	/* ISO Calendar string: At least Ione item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? 4 : 2;
		if (S->mw_text && S->item_order[0] == 1)	/* Prepare for "Week ##" format */
			sprintf (S->format, "%%s %%2.2d\0");
		else if (S->compact)			/* Numerical formatting of week or year without leading zeros */
			sprintf (S->format, "%%d\0");
		else					/* Numerical formatting of week or year  */
			(mode) ? sprintf (S->format, "%%%d.%dd\0", k, k) : sprintf (S->format, "%%%dd\0", k);
		if (S->item_order[1] >= 0) {	/* Need another item */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			if (S->mw_text && S->item_order[0] == 1) {	/* Prepare for "Week ##" format */
				sprintf (fmt, "%%s \0");
				strcat (S->format, fmt);
			}
			else
				strcat (S->format, "W");
			if (S->compact)
				sprintf (fmt, "%%d\0");
			else
				(mode) ? sprintf (fmt, "%%2.2d\0") : sprintf (fmt, "%%2d\0");
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* and ISO day of week */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				sprintf (fmt, "%%1d\0");
				strcat (S->format, fmt);
			}
		}
	}
	else if (S->item_order[0] >= 0) {			/* Gregorian Calendar string: At least one item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? 4 : 2;
		if (S->item_order[0] == 3) k = 3;	/* Day of year */
		if (S->mw_text && S->item_order[0] == 1)	/* Prepare for "Monthname" format */
			sprintf (S->format, "%%s\0");
		else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
			sprintf (S->format, "%%d\0");
		else					/* Numerical formatting of month or year */
			(mode) ? sprintf (S->format, "%%%d.%dd\0", k, k) : sprintf (S->format, "%%%dd\0", k);
		if (S->item_order[1] >= 0) {	/* Need more items */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			k = (S->item_order[1] == 0 && !S->Y2K_year) ? 4 : 2;
			if (S->item_order[1] == 3) k = 3;	/* Day of year */
			if (S->mw_text && S->item_order[1] == 1)	/* Prepare for "Monthname" format */
				sprintf (fmt, "%%s");
			else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
				sprintf (fmt, "%%d\0");
			else
				(mode) ? sprintf (fmt, "%%%d.%dd\0", k, k) : sprintf (fmt, "%%%dd\0", k);
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* .. and even more */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				k = (S->item_order[2] == 0 && !S->Y2K_year) ? 4 : 2;
				if (S->mw_text && S->item_order[2] == 1)	/* Prepare for "Monthname" format */
					sprintf (fmt, "%%s");
				else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
					sprintf (fmt, "%%d\0");
				else
					(mode) ? sprintf (fmt, "%%%d.%dd\0", k, k) : sprintf (fmt, "%%%dd\0", k);
				strcat (S->format, fmt);
			}
		}
	}
}

void GMT_geo_C_format (char *template, struct GMT_GEO_IO *S)
{
	/* Determine the output of geographic location formats. */
	 	
	GMT_get_dms_order (template, S);	/* Get the order of degree, min, sec in output formats */
	
	if (S->no_sign) {
		fprintf (stderr, "%s: ERROR: Unacceptable PLOT_DEGREE_FORMAT template %s. A not allowed\n", GMT_program, template);
		exit (EXIT_FAILURE);
	}
	
	if (S->decimal) {	/* Plain decimal degrees */
		sprintf (S->x_format, "%s\0", gmtdefs.d_format);
		sprintf (S->y_format, "%s\0", gmtdefs.d_format);
	}
	else {			/* Some form of dd:mm:ss */
		char fmt[32];
		sprintf (S->x_format, "%%3.3d\0");
		sprintf (S->y_format, "%%2.2d\0");
		if (S->order[1] >= 0) {	/* Need minutes too */
			strcat (S->x_format, S->delimiter[0]);
			strcat (S->y_format, S->delimiter[0]);
			sprintf (fmt, "%%2.2d\0");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->order[2] >= 0) {	/* .. and seconds */
			strcat (S->x_format, S->delimiter[1]);
			strcat (S->y_format, S->delimiter[1]);
			sprintf (fmt, "%%2.2d\0");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->n_sec_decimals) {	/* even add format for fractions of second (or minutes or degrees) */
			sprintf (fmt, ".%%%d.%dd\0", S->n_sec_decimals, S->n_sec_decimals);
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		/* Finally add %c for the W,E,S,N char (or NULL) */
		sprintf (fmt, "%%c\0");
		strcat (S->x_format, fmt);
		strcat (S->y_format, fmt);
	}
}

void GMT_plot_C_format (char *template, struct GMT_GEO_IO *S)
{
	int i, j;
	
	/* Determine the plot geographic location formats. */
	
	GMT_get_dms_order (template, S);	/* Get the order of degree, min, sec in output formats */
	
	if (S->decimal) {	/* Plain decimal degrees */
		int len;
		len = sprintf (S->x_format, "%s", gmtdefs.d_format);
		      sprintf (S->y_format, "%s", gmtdefs.d_format);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			S->x_format[len] = gmtdefs.encoding.code[gmtdefs.degree_symbol];
			S->y_format[len] = gmtdefs.encoding.code[gmtdefs.degree_symbol];
			S->x_format[len+1] = S->y_format[len+1] = '\0';
		}
		strcat (S->x_format, "%c");
		strcat (S->y_format, "%c");
	}
	else {			/* Must cover all the 6 forms of dd[:mm[:ss]][.xxx] */
		char fmt[32];
		
		for (i = 0; i < 3; i++)
		  for (j = 0; j < 2; j++)
		    GMT_plot_format[i][j] = GMT_memory (VNULL, 32, sizeof (char), GMT_program);
		
		/* Level 0: degrees only. index 0 is integer degrees, index 1 is [possibly] fractional degrees */
		
		sprintf (GMT_plot_format[0][0], "%%d");		/* ddd */
		if (S->order[1] == -1 && S->n_sec_decimals > 0) /* ddd.xxx format */
			sprintf (GMT_plot_format[0][1], "%%d.%%%d.%dd\0", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd format */
			sprintf (GMT_plot_format[0][1], "%%d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			sprintf (fmt, "%c", gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[0][0], fmt);
			strcat (GMT_plot_format[0][1], fmt);
		}
		
		/* Level 1: degrees and minutes only. index 0 is integer minutes, index 1 is [possibly] fractional minutes  */
		
		sprintf (GMT_plot_format[1][0], "%%d");	/* ddd */
		sprintf (GMT_plot_format[1][1], "%%d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[1][0], fmt);
			strcat (GMT_plot_format[1][1], fmt);
		}
		strcat (GMT_plot_format[1][0], "%2.2d");
		if (S->order[2] == -1 && S->n_sec_decimals > 0) /* ddd:mm.xxx format */
			sprintf (fmt, "%%d.%%%d.%dd", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm format */
			sprintf (fmt, "%%2.2d");
		strcat (GMT_plot_format[1][1], fmt);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_squote]);
			strcat (GMT_plot_format[1][0], fmt);
			strcat (GMT_plot_format[1][1], fmt);
		}

		/* Level 2: degrees, minutes, and seconds. index 0 is integer seconds, index 1 is [possibly] fractional seconds  */
		
		sprintf (GMT_plot_format[2][0], "%%d");
		sprintf (GMT_plot_format[2][1], "%%d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}
		strcat (GMT_plot_format[2][0], "%2.2d");
		strcat (GMT_plot_format[2][1], "%2.2d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_squote]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}
		strcat (GMT_plot_format[2][0], "%2.2d");
		if (S->n_sec_decimals > 0)			 /* ddd:mm:ss.xxx format */
			sprintf (fmt, "%%d.%%%d.%dd\0", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm:ss format */
			sprintf (fmt, "%%2.2d\0");
		strcat (GMT_plot_format[2][1], fmt);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the second symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", gmtdefs.encoding.code[gmt_dquote]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}
		
		/* Finally add %c for the W,E,S,N char (or NULL) */
	
		for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) strcat (GMT_plot_format[i][j], "%c\0");
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
			case 'x':	/* Longitude coordinates */
				code = GMT_IS_LON;
				break;
			case 'y':	/* Latitude coordinates */
				code = GMT_IS_LAT;
				break;
			case 'g':	/* Geographical coordinates */
				code = GMT_IS_GEO;
				break;
			case 'f':	/* Plain floating point coordinates */
				code = GMT_IS_FLOAT;
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

int	GMT_scanf_clock (char *s, double *val)
{
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
	
int	GMT_scanf_calendar (char *s, GMT_cal_rd *rd)
{
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

int	GMT_scanf_g_calendar (char *s, GMT_cal_rd *rd)
{
	/* Return -1 on failure.  Set rd and return 0 on success.  
	
	For gregorian calendars.  */
	
	int	k, ival[3];
	
	if (GMT_io.date_input.day_of_year) {
		/* Calendar uses year and day of year format.  */
		if ( (k = sscanf (s, GMT_io.date_input.format,
			&ival[GMT_io.date_input.item_order[0]],
			&ival[GMT_io.date_input.item_order[3]]) ) == 0) return (-1);
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
		&ival[GMT_io.date_input.item_order[0]],
		&ival[GMT_io.date_input.item_order[1]],
		&ival[GMT_io.date_input.item_order[2]]) ) == 0) return (-1);
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
	
	if (GMT_g_ymd_is_bad (ival[0], ival[1], ival[2]) ) return (-1);
	
	*rd = GMT_rd_from_gymd (ival[0], ival[1], ival[2]);
	return (0);
}
		

int	GMT_scanf_geo (char *s, double *val)
{
	/* Try to read a character string token stored in s,
	knowing that it should be a geographical variable.  
	If successful, stores value in val and returns one of
	GMT_IS_FLOAT, GMT_IS_GEO, GMT_IS_LAT, GMT_IS_LON, 
	whichever can be determined from the format of s.
	If unsuccessful, does not store anything in val and
	returns GMT_IS_NAN.  
	This should have essentially the same functionality
	as the GMT3.4 GMT_scanf, except that the expectation is
	now used and returned, and this also permits a double
	precision format in the minutes or seconds, and does
	more error checking.  However, this is not optimized
	for speed (yet).  WHFS, 16 Aug 2001
	
	note:  mismatch handling (e.g. this routine finds a lon
	but calling routine expected a lat) is not done here.

	*/
	
	char	scopy[64], suffix, *p, *p2;
	double	dd, dm, ds;
	int	retval = GMT_IS_FLOAT;
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
			case '.':	/* Decimal point without decimals, e.g., 123. */
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
	ncolons = 0;
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
	
	if (ncolons && retval == GMT_IS_FLOAT) retval = GMT_IS_GEO;
	
	switch (ncolons) {
		case 0:
			if ( (sscanf(scopy, "%lf", &dd) ) != 1) return (GMT_IS_NAN);
			break;
		case 1:
			if ( (sscanf(scopy, "%d:%lf", &id, &dm) ) != 2) return (GMT_IS_NAN);
			dd = (id < 0) ? id - dm * GMT_MIN2DEG : id + dm * GMT_MIN2DEG;
			break;
		case 2:
			if ( (sscanf(scopy, "%d:%d:%lf", &id, &im, &ds) ) != 3) return (GMT_IS_NAN);
			dd = im * GMT_MIN2DEG + ds * GMT_SEC2DEG;
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

		
int	GMT_scanf_float (char *s, double *val)
{
	/* Try to decode a value from s and store
	in val.  s should not have any special format
	(neither geographical, with suffixes or
	separating colons, nor calendar nor clock).
	However, D and d are permitted to map to e
	if this would result in a success.  This
	allows Fortran Double Precision to be readable.
	
	On success, return GMT_IS_FLOAT and store val.
	On failure, return GMT_IS_NAN and do not touch val.
	*/

	char	scopy[64], *p;
	double	x;
	int	j,k;
	
	x = strtod (s, &p);
	if (p[0] == 0) {
		/* Success (non-Fortran).  */
		*val = x;
		return (GMT_IS_FLOAT);
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
	return (GMT_IS_FLOAT);
}

int	GMT_scanf (char *s, int expectation, double *val)
{
	/* Called with s pointing to a char string, expectation
	indicating what is known/required/expected about the
	format of the string.  Attempts to decode the string to
	find a double value.  Upon success, loads val and 
	returns type found.  Upon failure, does not touch val,
	and returns GMT_IS_NAN.  Expectations permitted on call
	are
		GMT_IS_FLOAT	we expect an uncomplicated float.
	*/
	
	char	calstring[64], clockstring[64], *p;
	double	x;
	int	callen, clocklen;
	GMT_cal_rd rd;
	
	
	if (expectation & GMT_IS_GEO) {
		/* True if either a lat or a lon is expected  */
		return (GMT_scanf_geo (s, val));
	}
	
	if (expectation == GMT_IS_FLOAT) {
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
	
	if (expectation & GMT_IS_UNKNOWN) {
		/* True if we dont know but must try both geographic or float formats  */
		return (GMT_scanf_geo (s, val));
	}

	fprintf (stderr, "GMT_LOGIC_BUG:  GMT_scanf() called with invalid expectation.\n");
	return (GMT_IS_NAN);
}

int	GMT_scanf_argtime (char *s, GMT_dtime *t)
{
	/* s is a string from a command-line argument.
		The argument is known to refer to a time
		variable.  For example, the argument is
		a token from -R<t_min>/<t_max>/a/b[/c/d].
		However, we will permit it to be in EITHER
			-generic floating point format,
			in which case we interpret it as
			time relative to user units and epoch;
		OR
			-absolute time in a restricted format.
		
		The absolute format must be restricted because
		we cannot use '/' as a delimiter in an arg
		string, but we might allow the user to use that
		in a data file (in gmtdefs.[in/out]put_date_format.
		Therefore we cannot use the user's date format
		string here, and we hard-wire something here.
		
		The relative format must be decodable by GMT_scanf_float().
		
		The absolute format must have a T.  If it has a clock
		string then it must be of the form 
		<complete_calstring>T<clockstring>
		or just T<clockstring>.  If it has no clockstring then
		it must be of the form 
		<partial or complete calstring>T.
		
		A <clockstring> may be partial (e.g. hh or hh:mm) or
		complete (hh:mm:ss[.xxx]) but it must use ':' for a
		delimiter and it must be readable with "%2d:%2d:%lf".
		Also, it must be a 24 hour clock (00:00:00 to 23:59:59.xxx,
		or 60.xxx on a leap second); no am/pm suffixes allowed.
		
		A <calstring> must be of the form
		[-]yyyy[-mm[-dd]]T readable after first '-' with "%4d-%2d-%2dT" (Gregorian year,month,day)
		[-]yyyy[-jjj]T readable after first '-' with "%4d-%3dT" (Gregorian year, day-of-year)
		yyyy[-Www[-d]]T (ISO week calendar)
			
	Upon failure, returns GMT_IS_NAN.  Upon success, sets
	t and returns GMT_IS_ABSTIME.
	We could have it return either ABSTIME or RELTIME to indicate
	which one it thinks it decoded; however, this is inconsistent
	with the use of GMT_scanf when RELTIME is expected and ABSTIME
	conversion is done internal to the routine, as it is here.
	That is, we always return an absolute time if we haven't failed,
	so that is the returned value.
	*/
	
	double	ss, x;
	char 	*pw, *pt;
	int	hh, mm, j, k, i, dash, ival[3];
	BOOLEAN negate_year = FALSE, got_yd = FALSE;
	
	if ( (pt = strchr (s, (int)'T') ) == CNULL) {
		/* There is no T.  This must decode with GMT_scanf_float()
			or we die.  */
		if ( ( GMT_scanf_float (s, &x) ) == GMT_IS_NAN) return (GMT_IS_NAN);
		*t = GMT_dt_from_usert (x);
		return (GMT_IS_ABSTIME);
	}
	x = 0.0;	/* x will be the seconds since start of today.  */
	if (pt[1]) {	/* There is a string following the T:  Decode a clock:  */
		k = sscanf (&pt[1], "%2d:%2d:%lf", &hh, &mm, &ss);
		if (k == 0) return (GMT_IS_NAN);
		if (hh < 0 || hh >= 24) return (GMT_IS_NAN);
		x = GMT_HR2SEC_F * hh;
		if (k > 1) {
			if (mm < 0 || mm > 59) return (GMT_IS_NAN);
			x += GMT_MIN2SEC_F * mm;
		}
		if (k > 2) {
			if (ss < 0.0 || ss >= 61.0) return (GMT_IS_NAN);
			x += ss;
		}
	}
	
	k = 0;
	while (s[k] && s[k] == ' ') k++;
	if (s[k] == '-') negate_year = TRUE;
	if (s[k] == 'T') {
		/* There is no calendar.  Set day to 1 and use that:  */
		*t = GMT_rdc2dt ( (GMT_cal_rd)1, x);
		return (GMT_IS_ABSTIME);
	}
	
	if (!(isdigit ( (int)s[k]) ) ) return (GMT_IS_NAN);	/* Bad format */
	
	if ( (pw = strchr (s, (int)'W') ) ) {
		/* There is a W.  ISO calendar or junk.  */
		if (strlen(pw) <= strlen(pt)) {
			/* The W is after the T.  Wrong format.  */
			return (GMT_IS_NAN);
		}
		if (negate_year) {
			/* negative years not allowed in ISO calendar  */
			return (GMT_IS_NAN);
		}
		if ( (j = sscanf(&s[k], "%4d-W%2d-%1d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
		if (ival[1] < 1 || ival[1] > 53) return (GMT_IS_NAN);
		if (ival[2] < 1 || ival[2] > 7)  return (GMT_IS_NAN);
		*t = GMT_rdc2dt ( GMT_rd_from_iywd (ival[0], ival[1], ival[2]), x);
		return (GMT_IS_ABSTIME);
	}
	
	for (i = negate_year; s[k+i] && s[k+i] != '-'; i++);;	/* Goes to first - between yyyy and jjj or yyyy and mm */
	dash = ++i;				/* Position of first character after the first dash (could be end of string if no dash) */
	while (s[k+i] && !(s[k+i] == '-' || s[k+i] == 'T')) i++;	/* Goto the ending T character or get stuck on a second - */
	got_yd = ((i - dash) == 3 && s[k+i] == 'T');		/* Must have a field of 3-characters between - and T to constitute a valid day-of-year format */

	if (got_yd) {	/* Gregorian yyyy-jjj calendar:  */
		if ( (j = sscanf(&s[k], "%4d-%3d", &ival[0], &ival[1]) ) != 2) return (GMT_IS_NAN);
		ival[2] = 1;
	}
	else {	/* Gregorian yyyy-mm-dd calendar:  */
		if ( (j = sscanf(&s[k], "%4d-%2d-%2d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
	}
	if (negate_year) ival[0] = -ival[0];
	if (got_yd) {
		if (ival[1] < 1 || ival[1] > 366)  return (GMT_IS_NAN);	/* Simple range check on day-of-year (1-366) */
		*t = GMT_rdc2dt (GMT_rd_from_gymd (ival[0], 1, 1) + ival[1] - 1, x);
	}
	else {
		if (GMT_g_ymd_is_bad (ival[0], ival[1], ival[2]) ) return (GMT_IS_NAN);
		*t = GMT_rdc2dt (GMT_rd_from_gymd (ival[0], ival[1], ival[2]), x);
	}
	
	return (GMT_IS_ABSTIME);
}

int	GMT_scanf_arg (char *s, int expectation, double *val)
{
	/* Version of GMT_scanf used for command line arguments only (not data records).
	 * It differs from GMT_scanf in that if the expectation is GMT_IS_UNKNOWN it will
	 * check to see if the argument is (1) an absolute time string, (2) a geographical
	 * location string, or if not (3) a floating point string.  To ensure backward
	 * compatibility: if we encounter geographic data it will also set the GMT_io.type[]
	 * variable accordingly so that data i/o will work as in 3.4
	 */
	
	char c;
	
	if (expectation == GMT_IS_UNKNOWN) {	/* Expectation for this column not set - must be determined if possible */
		if (strchr (s, (int)'T')) {				/* Found a T in the argument - assume Absolute time */
			expectation = GMT_IS_ABSTIME;
		}
		else if ((c = s[strlen(s)-1]) == 'W' || c == 'E') {	/* Found trailing W or E - assume Geographic longitudes */
			expectation = GMT_IS_LON;
		}
		else if ((c = s[strlen(s)-1]) == 'S' || c == 'N') {	/* Found trailing S or N - assume Geographic latitudes */
			expectation = GMT_IS_LON;
		}
		else if (strchr (s, (int)':')) {			/* Found a : in the argument - assume Geographic coordinates */
			expectation = GMT_IS_GEO;
		}
		else {							/* Found nothing - assume floating point */
			expectation = GMT_IS_FLOAT;
		}
	}
	
	/* OK, here we have an expectation, now call GMT_scanf */
	
	return (GMT_scanf (s, expectation, val));
}

