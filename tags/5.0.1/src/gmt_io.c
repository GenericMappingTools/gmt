/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Table input/output in GMT can be either ascii, binary, or netCDF COARDS files
 * and may consist of single or multiple segments.  These files are accessed
 * via the C->current.io.input function pointer which either points to the
 * ASCII read (gmt_ascii_input), the binary read (gmt_bin_input), or the
 * netCDF read (gmt_nc_input) functions, depending on the -bi setting.
 * Similarly, writing of such tables are done via the C->current.io.output
 * function pointer which is set to either GMT_ascii_output, gmt_bin_output,
 * or GMT_nc_output [not implemented yet].
 * For special processing of z-data by xyz2grd & grd2xyz we also use the
 * C->current.io.input/output functions but these are reset in those two
 * programs to the corresponding read/write functions pointed to by the
 * GMT_Z_IO member ->read_item or ->write_item as these can handle additional
 * binary data types such as char, int, short etc.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ascii tables.  For compliance with a wide variety of
 * binary data formats for grids and their internal nesting the GMT_Z_IO
 * structure and associated functions are used (in xyz2grd and grd2xyz)
 *
 * The following functions are here:
 *
 *	GMT_getuserpath		Get pathname of file in "user directories" (C->session.TMPDIR, CWD, HOME, C->session.USERDIR)
 *	GMT_getdatapath		Get pathname of file in "data directories" (CWD, GMT_{USER,DATA,GRID,IMG}DIR)
 *	GMT_getsharepath	Get pathname of file in "share directries" (CWD, C->session.USERDIR, C->session.SHAREDIR tree)
 *	GMT_fopen:		Open a file using GMT_getdatapath
 *	GMT_fclose:		Close a file
 *	GMT_io_init:		Init GMT_IO structure
 *	GMT_write_segmentheader	Write header record for multisegment files
 *	gmt_ascii_input:	Decode ascii input record
 *	GMT_scanf:		Robust scanf function with optional dd:mm:ss conversion
 *	GMT_bin_double_input:	Decode binary double precision record
 *	GMT_bin_double_input_swab:	Decode binary double precision record followed by byte-swabbing
 *	GMT_bin_float_input:	Decode binary single precision record
 *	GMT_bin_float_input_swab:	Decode binary single precision record followed by byte-swabbing
 *	gmt_nc_input:		Decode one record of netCDF column-oriented data
 *	GMT_ascii_output:	Write ascii record
 *	GMT_bin_double_output:	Write binary double precision record
 *	GMT_bin_double_output_swab:	Write binary double precision record after first swabbing
 *	GMT_bin_float_output:	Write binary single precision record
 *	GMT_bin_float_output_swab:	Write binary single precision record after first swabbing
 *	GMT_set_z_io:		Set GMT_Z_IO structure based on -Z
 *	GMT_check_z_io:		Fill in implied missing row/column
 *	gmt_A_read:		Read the next ascii item from input stream (may be more than one per line) z must be regular float
 *	gmt_a_read:		Read 1 ascii item per input record
 *	gmt_c_read:		Read 1 binary char item
 *	gmt_u_read:		Read 1 binary unsigned char item
 *	gmt_h_read:		Read 1 binary short int item
 *	gmt_H_read:		Read 1 binary unsigned short int item
 *	gmt_i_read:		Read 1 binary int item
 *	gmt_I_read:		Read 1 binary unsigned int item
 *	gmt_l_read:		Read 1 binary long int item
 *	gmt_f_read:		Read 1 binary float item
 *	gmt_d_read:		Read 1 binary double item
 *	gmt_a_write:		Write 1 ascii item
 *	gmt_c_write:		Write 1 binary char item
 *	gmt_u_write:		Write 1 binary unsigned char item
 *	gmt_h_write:		Write 1 binary short int item
 *	gmt_H_write:		Write 1 binary unsigned short int item
 *	gmt_i_write:		Write 1 binary int item
 *	gmt_I_write:		Write 1 binary unsigned int item
 *	gmt_l_write:		Write 1 binary long int item
 *	gmt_f_write:		Write 1 binary float item
 *	gmt_d_write:		Write 1 binary double item
 *	gmt_col_ij:		Convert index to column format
 *	gmt_row_ij:		Convert index to row format
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 * Now 64-bit enabled.
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

EXTERN_MSC GMT_LONG GMTAPI_n_items (struct GMTAPI_CTRL *API, GMT_LONG family, GMT_LONG direction, GMT_LONG *first_ID);
EXTERN_MSC GMT_LONG GMTAPI_Unregister_IO (struct GMTAPI_CTRL *API, GMT_LONG object_ID, GMT_LONG direction);
EXTERN_MSC GMT_LONG GMTAPI_Validate_ID (struct GMTAPI_CTRL *API, GMT_LONG family, GMT_LONG object_ID, GMT_LONG direction, GMT_LONG *item_no);

#ifdef HAVE_DIRENT_H_
#	include <dirent.h>
#endif

#ifdef HAVE_SYS_DIR_H_
#	include <sys/dir.h>
#endif

#ifndef DT_DIR
#	define DT_DIR 4
#endif

/* Macro to apply columns log/scale/offset conversion on the fly */
#define gmt_convert_col(S,x) {if (S.convert) x = ((S.convert == 2) ? log10 (x) : x) * S.scale + S.offset;}

static const char *GMT_type[GMTAPI_N_TYPES] = {"byte", "byte", "integer", "integer", "integer", "integer", "integer", "integer", "double", "double", "string", "datetime"};

/* Library functions needed for Windows DLL to work properly.
 * These are only compiled under Windows - under other OS the
 * macros in gmt_io.h will kick in instead.  The reason these
 * are needed has to do with how Windows DDL differs from the
 * typical Linux shared libraries: Pointers created inside a
 * DLL can only be freed inside the DLL.
 */

#ifdef WIN32
FILE *GMT_fdopen (int handle, const char *mode) { return (fdopen (handle, mode)); }
int GMT_fgetc (FILE *stream) { return (fgetc (stream)); }
int GMT_ungetc (int c, FILE *stream) { return (ungetc (c, stream)); }
int GMT_fputs (const char *str, FILE *stream) { return (fputs (str, stream)); }
int GMT_fseek (FILE *stream, long offset, int whence) { return (fseek(stream, offset, whence)); }
long GMT_ftell (FILE *stream) { return (ftell(stream)); }
size_t GMT_fread (void *ptr, size_t size, size_t nmemb, FILE * stream) { return (fread (ptr, size, nmemb, stream)); }
size_t GMT_fwrite (const void *ptr, size_t size, size_t nmemb, FILE * stream) { return (fwrite (ptr, size, nmemb, stream)); }
void GMT_rewind (FILE *stream) { rewind (stream); }
#endif

/* This version of fgets will check for input record truncation, that is,
 * the input record is longer than the given size.  Since calls to GMT_fgets
 * ASSUME they get a logical record, we will give a warning if truncation
 * occurs and read until we have consumed the linefeed, thus making the
 * i/o machinery ready for the next logical record.
 */

char *GMT_fgets (struct GMT_CTRL *C, char *str, int size, FILE *stream)
{
	str[size-2] = '\0'; /* Set last but one record to 0 */
	if (!fgets (str, size, stream))
		return (NULL); /* Got nothing */

	/* fgets will always set str[size-1] = '\0' if more data than str can handle is found.
	 * Thus, we examine str[size-2].  If this is neither '\0' nor '\n' then we have only
	 * read a portion of a logical record that is longer than size.
	 */
	if (!(str[size-2] == '\0' || str[size-2] == '\n')) {
		/* Only got part of a record */
		int c, n = 0;
		/* Read char-by-char until newline is consumed */
		while ((c = fgetc (stream)) != '\n' && c != EOF)
			++n;
		if (c == '\n')
			/* We expect fgets to retain '\n', so add it */
			str[size-2] = '\n';
		else
			/* EOF without '\n' */
			--n;
		/* This will report wrong lengths if last line has no '\n' but we don't care */
		GMT_report (C, GMT_MSG_FATAL, "Long input record (%d bytes) was truncated to first %d bytes!\n", size+n, size-2);
	}
	return (str);
}

char *GMT_fgets_chop (struct GMT_CTRL *C, char *str, int size, FILE *stream)
{
	char *p;

	/* fgets will always set str[size-1] = '\0' if more data than str can handle is found.
	 * Thus, we examine str[size-2].  If this is neither '\0' nor '\n' then we have only
	 * read a portion of a logical record that is longer than size.
	 */
	str[size-2] = '\0'; /* Set last but one record to 0 */
	if (!fgets (str, size, stream))
		return (NULL); /* Got nothing */

	p = strpbrk (str, "\r\n");
	if ( p == NULL || ((p-str+2 == size) && *p != '\n') ) {
		/* If CR or LF not found, or last but one record not \n,
		 * then only got part of a record */
		int c, n = 0;
		/* Read char-by-char until newline is consumed */
		while ((c = fgetc (stream)) != '\n' && c != EOF)
			(void) (isspace(c) || ++n); /* Do not count whitespace */
		if (n)
			GMT_report (C, GMT_MSG_FATAL, "Long input record (%d bytes) was truncated to first %d bytes!\n", size+n-1, size);
	}
	if (p)
		/* Overwrite 1st CR or LF with terminate string */
		*p = '\0';

	return (str);
}

int GMT_fclose (struct GMT_CTRL *C, FILE *stream)
{
	if (!stream || stream == NULL)  return (0);
	/* First skip any stream related to the three Unix i/o descriptors */
	if (stream == C->session.std[GMT_IN])  return (0);
	if (stream == C->session.std[GMT_OUT]) return (0);
	if (stream == C->session.std[GMT_ERR]) return (0);
	if ((GMT_LONG)stream == -C->current.io.ncid) {	/* Special treatment for netCDF files */
		nc_close (C->current.io.ncid);
		GMT_free (C, C->current.io.varid);
		GMT_free (C, C->current.io.add_offset);
		GMT_free (C, C->current.io.scale_factor);
		GMT_free (C, C->current.io.missing_value);
		C->current.io.ncid = C->current.io.nvars = 0;	/* Split in two too shut up a compiler warning */
		C->current.io.ndim = C->current.io.nrec = 0;
		C->current.io.input = C->session.input_ascii;
		return (0);
	}
	/* Regular file */
	return (fclose (stream));
}

void GMT_skip_xy_duplicates (struct GMT_CTRL *C, GMT_LONG mode)
{	/* Changes the status of the skip_duplicates setting */
	/* PW: This is needed as some algorithms testing if a point is
	 * inside or outside a polygon have trouble if there are
	 * duplicate vertices in the polygon.  This option can
	 * be set to TRUE and such points will be skipped during
	 * the data reading step. Mode = TRUE or FALSE */
	C->current.io.skip_duplicates = mode;
}

GMT_LONG gmt_n_cols_needed_for_gaps (struct GMT_CTRL *C, GMT_LONG n) {
	GMT_LONG n_use = MAX (n, C->common.g.n_col);
	/* Return the actual items needed (which may be more than n if gap testing demands it) and update previous record */
	GMT_memcpy (C->current.io.prev_rec, C->current.io.curr_rec, n_use, double);
	if (!C->common.g.active) return (n);	/* No gap checking, n it is */
	return (n_use);
}

GMT_LONG gmt_gap_detected (struct GMT_CTRL *C)
{	/* Determine if two points are "far enough apart" to constitude a data gap and thus "pen up" */
	GMT_LONG i;

	if (!C->common.g.active || C->current.io.pt_no == 0) return (FALSE);	/* Not active or on first point in a segment */
	/* Here we must determine if any or all of the selected gap criteria [see gmt_set_gap_param] are met */
	for (i = 0; i < C->common.g.n_methods; i++) {	/* Go through each criterion */
		if ((C->common.g.get_dist[i] (C, C->common.g.col[i]) > C->common.g.gap[i]) != C->common.g.match_all) return (!C->common.g.match_all);
	}
	return (C->common.g.match_all);
}

GMT_LONG gmt_set_gap (struct GMT_CTRL *C) {	/* Data gaps are special since there is no multiple-segment header flagging the gap; thus next time the record is already read */
	C->current.io.status = GMT_IO_GAP;
	C->current.io.seg_no++;
	GMT_report (C, GMT_MSG_VERBOSE, "Data gap detected via -g; Segment header inserted near/at line # %ld\n", C->current.io.rec_no);
	sprintf (C->current.io.segment_header, "Data gap detected via -g; Segment header inserted");
	return (0);
}

void gmt_adjust_periodic (struct GMT_CTRL *C) {
	while (C->current.io.curr_rec[GMT_X] > C->common.R.wesn[XHI] && (C->current.io.curr_rec[GMT_X] - 360.0) >= C->common.R.wesn[XLO]) C->current.io.curr_rec[GMT_X] -= 360.0;
	while (C->current.io.curr_rec[GMT_X] < C->common.R.wesn[XLO] && (C->current.io.curr_rec[GMT_X] + 360.0) <= C->common.R.wesn[XLO]) C->current.io.curr_rec[GMT_X] += 360.0;
	/* If data is not inside the given range it will satisfy (lon > east) */
	/* Now it will be outside the region on the same side it started out at */
}

void GMT_set_segmentheader (struct GMT_CTRL *C, GMT_LONG direction, GMT_LONG true_false)
{	/* Enable/Disable multi-segment headers for either input or output */
		
	C->current.io.multi_segments[direction] = true_false;
}

GMT_LONG gmt_process_binary_input (struct GMT_CTRL *C, GMT_LONG n_read) {
	/* Process a binary record to determine what kind of record it is. Return values:
	 * 0 = regular record; 1 = segment header (all NaNs); 2 = skip this record
	*/
	GMT_LONG col_no, n_NaN, bad_record = FALSE, set_nan_flag = FALSE;
	/* Here, C->current.io.curr_rec has been filled in by fread */

	/* Determine if this was a segment header, and if so return */
	for (col_no = n_NaN = 0; col_no < n_read; col_no++) {
		if (!GMT_is_dnan (C->current.io.curr_rec[col_no])) continue;	/* Clean data, do nothing */
		/* We end up here if we found a NaN */
		if (!C->current.setting.io_nan_records && C->current.io.skip_if_NaN[col_no]) bad_record = TRUE;	/* This field is not allowed to be NaN */
		if (C->current.io.skip_if_NaN[col_no]) set_nan_flag = TRUE;
		n_NaN++;
	}
	if (!C->current.io.status) {	/* Must have n_read NaNs to qualify as segment header */
		if (n_NaN == n_read) {
			GMT_report (C, GMT_MSG_VERBOSE, "Detected binary segment header near/at line # %ld\n", C->current.io.rec_no);
			C->current.io.status = GMT_IO_SEG_HEADER;
			C->current.io.segment_header[0] = '\0';
			GMT_set_segmentheader (C, GMT_OUT, TRUE);	/* Turn on "-mo" */
			C->current.io.seg_no++;
			C->current.io.pt_no = 0;
			return (1);	/* 1 means segment header */
		}
	}
	if (bad_record) {
		C->current.io.n_bad_records++;
		if (C->current.io.give_report && C->current.io.n_bad_records == 1) {	/* Report 1st occurrence */
			GMT_report (C, GMT_MSG_FATAL, "Encountered first invalid binary record near/at line # %ld\n", C->current.io.rec_no);
			GMT_report (C, GMT_MSG_FATAL, "Likely causes:\n");
			GMT_report (C, GMT_MSG_FATAL, "(1) Invalid x and/or y values, i.e. NaNs.\n");
		}
		return (2);	/* 2 means skip this record and try again */
	}
	else if (C->current.io.skip_duplicates && C->current.io.pt_no) {	/* Test to determine if we should skip duplicate records with same x,y */
		if (C->current.io.curr_rec[GMT_X] == C->current.io.prev_rec[GMT_X] && C->current.io.curr_rec[GMT_Y] == C->current.io.prev_rec[GMT_Y]) return (2);	/* Yes, duplicate */
	}
	if (C->current.setting.io_lonlat_toggle[GMT_IN] && n_read >= 2) d_swap (C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) gmt_adjust_periodic (C);	/* Must account for periodicity in 360 */
	if (set_nan_flag) C->current.io.status |= GMT_IO_NAN;
	return (0);	/* 0 means OK regular record */
}

void * gmt_nc_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, GMT_LONG *retval)
{
	GMT_LONG i, status, n_use;

	C->current.io.status = 0;
	if (*n == GMT_MAX_COLUMNS)
		*n = C->current.io.nvars;
	else if (*n > C->current.io.nvars) {
		GMT_report (C, GMT_MSG_FATAL, "gmt_nc_input is asking for %ld columns, but file has only %d\n", *n, C->current.io.nvars);
		C->current.io.status = GMT_IO_MISMATCH;
	}
	do {	/* Keep reading until (1) EOF, (2) got a segment record, or (3) a valid data record */
		n_use = gmt_n_cols_needed_for_gaps (C, *n);

		if (C->current.io.nrec == C->current.io.ndim) {
			C->current.io.status = GMT_IO_EOF;
			*retval = -1;
			return (NULL);
		}
		for (i = 0; i < C->current.io.nvars && i < n_use; i++) {
			nc_get_var1_double (C->current.io.ncid, C->current.io.varid[i], &C->current.io.nrec, &C->current.io.curr_rec[i]);
			if (C->current.io.curr_rec[i] == C->current.io.missing_value[i])
				C->current.io.curr_rec[i] = C->session.d_NaN;
			else
				C->current.io.curr_rec[i] = C->current.io.curr_rec[i] * C->current.io.scale_factor[i] + C->current.io.add_offset[i];
				gmt_convert_col (C->current.io.col[GMT_IN][i], C->current.io.curr_rec[i]);
		}
		C->current.io.nrec++;
		C->current.io.rec_no++;
		status = gmt_process_binary_input (C, n_use);
		if (status == 1) { *retval = 0; return (NULL); }		/* A segment header */
	} while (status == 2);	/* Continue reading when record is to be skipped */
	if (gmt_gap_detected (C)) {
		*retval = gmt_set_gap (C);
		return (C->current.io.curr_rec);
	}
	C->current.io.pt_no++;
	*retval = *n;
	return (C->current.io.curr_rec);
}

GMT_LONG GMT_nc_get_att_text (struct GMT_CTRL *C, int ncid, int varid, char *name, char *text, size_t textlen)
{	/* This function is a replacement for nc_get_att_text that avoids overflow of text
	 * ncid, varid, name, text	: as in nc_get_att_text
	 * textlen			: maximum number of characters to copy to string text
	 */
	GMT_LONG err;
	size_t attlen;
	char *att = NULL;

	GMT_err_trap (nc_inq_attlen (ncid, varid, name, &attlen));
	att = GMT_memory (C, NULL, attlen, char);
	nc_get_att_text (ncid, varid, name, att);
	attlen = MIN (attlen, textlen-1);	/* Truncate text to one less than textlen (to keep space for NUL terminator) */
	GMT_memcpy (text, att, attlen, char);		/* Copy att to text */
	GMT_memset (&text[attlen], textlen - attlen, char);	/* Fill rest of text with zeros */
	GMT_free (C, att);
	return (GMT_NOERROR);
}

FILE *gmt_nc_fopen (struct GMT_CTRL *C, const char *filename, const char *mode)
/* Open a netCDF file for column I/O. Append ?var1/var2/... to indicate the requested columns.
 * Currently only reading is supported.
 * The routine returns a fake file pointer (in fact the netCDF file ID), but stores
 * all the relevant information in the C->current.io struct (ncid, ndim, nrec, varid, add_offset,
 * scale_factor, missing_value). Some of these are allocated here, and have to be
 * deallocated upon GMT_fclose.
 * Also asigns C->current.io.col_type[GMT_IN] based on the variable attributes.
 */
{
	char file[GMT_BUFSIZ], path[GMT_BUFSIZ];
	int i, j, nvars;
	size_t n;
	GMT_LONG tmp_pointer;	/* To avoid 64-bit warnings */
	char varnm[20][GMT_TEXT_LEN64], long_name[GMT_TEXT_LEN256], units[GMT_TEXT_LEN256], varname[GMT_TEXT_LEN64];
	struct GMT_TIME_SYSTEM time_system;

	if (mode[0] != 'r') {
		GMT_report (C, GMT_MSG_FATAL, "GMT_fopen does not support netCDF writing mode\n");
		GMT_exit (EXIT_FAILURE);
	}

	nvars = sscanf (filename,
		"%[^?]?%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]",
		file, varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9], varnm[10],
		varnm[11], varnm[12], varnm[13], varnm[14], varnm[15], varnm[16], varnm[17], varnm[18], varnm[19]) - 1;
	if (nc_open (GMT_getdatapath (C, file, path), NC_NOWRITE, &C->current.io.ncid)) return (NULL);
#ifdef GMT_COMPAT
	if (nvars <= 0) nvars = sscanf (C->common.b.varnames,
		"%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]",
		varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9], varnm[10],
		varnm[11], varnm[12], varnm[13], varnm[14], varnm[15], varnm[16], varnm[17], varnm[18], varnm[19]);
#endif
	if (nvars <= 0)
		nc_inq_nvars (C->current.io.ncid, &C->current.io.nvars);
	else
		C->current.io.nvars = nvars;
	C->current.io.varid = GMT_memory (C, NULL, C->current.io.nvars, int);
	C->current.io.scale_factor = GMT_memory (C, NULL, C->current.io.nvars, double);
	C->current.io.add_offset = GMT_memory (C, NULL, C->current.io.nvars, double);
	C->current.io.missing_value = GMT_memory (C, NULL, C->current.io.nvars, double);
	C->current.io.ndim = C->current.io.nrec = 0;

	for (i = 0; i < C->current.io.nvars; i++) {
		/* Get variable ID and variable name */
		if (nvars <= 0)
			C->current.io.varid[i] = i;
		else
			GMT_err_fail (C, nc_inq_varid (C->current.io.ncid, varnm[i], &C->current.io.varid[i]), file);
		nc_inq_varname (C->current.io.ncid, C->current.io.varid[i], varname);

		/* Check column size */
		nc_inq_varndims (C->current.io.ncid, C->current.io.varid[i], &j);
		if (j != 1) {
			GMT_report (C, GMT_MSG_FATAL, "NetCDF variable %s is not 1-dimensional\n", varname);
			GMT_exit (EXIT_FAILURE);
		}
		nc_inq_vardimid(C->current.io.ncid, C->current.io.varid[i], &j);
		nc_inq_dimlen(C->current.io.ncid, j, &n);
		if (C->current.io.ndim != 0 && C->current.io.ndim != n) {
			GMT_report (C, GMT_MSG_FATAL, "NetCDF variable %s has different dimension (%ld) from others (%ld)\n", varname, (GMT_LONG)n, (GMT_LONG)C->current.io.ndim);
			GMT_exit (EXIT_FAILURE);
		}
		C->current.io.ndim = n;

		/* Get scales, offsets and missing values */
		if (nc_get_att_double(C->current.io.ncid, C->current.io.varid[i], "scale_factor", &C->current.io.scale_factor[i])) C->current.io.scale_factor[i] = 1.0;
		if (nc_get_att_double(C->current.io.ncid, C->current.io.varid[i], "add_offset", &C->current.io.add_offset[i])) C->current.io.add_offset[i] = 0.0;
		if (nc_get_att_double (C->current.io.ncid, C->current.io.varid[i], "_FillValue", &C->current.io.missing_value[i]) &&
		    nc_get_att_double (C->current.io.ncid, C->current.io.varid[i], "missing_value", &C->current.io.missing_value[i])) C->current.io.missing_value[i] = C->session.d_NaN;

		/* Scan for geographical or time units */
		if (GMT_nc_get_att_text (C, C->current.io.ncid, C->current.io.varid[i], "long_name", long_name, (size_t)GMT_TEXT_LEN256)) long_name[0] = 0;
		if (GMT_nc_get_att_text (C, C->current.io.ncid, C->current.io.varid[i], "units", units, (size_t)GMT_TEXT_LEN256)) units[0] = 0;
		GMT_str_tolower (long_name); GMT_str_tolower (units);

		if (!strcmp (long_name, "longitude") || strstr (units, "degrees_e"))
			C->current.io.col_type[GMT_IN][i] = GMT_IS_LON;
		else if (!strcmp (long_name, "latitude") || strstr (units, "degrees_n"))
			C->current.io.col_type[GMT_IN][i] = GMT_IS_LAT;
		else if (!strcmp (long_name, "time") || !strcmp (varname, "time")) {
			C->current.io.col_type[GMT_IN][i] = GMT_IS_RELTIME;
			GMT_memcpy (&time_system, &C->current.setting.time_system, 1, struct GMT_TIME_SYSTEM);
			if (GMT_get_time_system (C, units, &time_system) || GMT_init_time_system_structure (C, &time_system))
				GMT_report (C, GMT_MSG_FATAL, "Warning: Time units [%s] in NetCDF file not recognised, defaulting to gmt.conf.\n", units);
			/* Determine scale between data and internal time system, as well as the offset (in internal units) */
			C->current.io.scale_factor[i] = C->current.io.scale_factor[i] * time_system.scale * C->current.setting.time_system.i_scale;
			C->current.io.add_offset[i] *= time_system.scale;	/* Offset in seconds */
			C->current.io.add_offset[i] += GMT_DAY2SEC_F * ((time_system.rata_die - C->current.setting.time_system.rata_die) + (time_system.epoch_t0 - C->current.setting.time_system.epoch_t0));
			C->current.io.add_offset[i] *= C->current.setting.time_system.i_scale;	/* Offset in internal time units */
		}
		else if (C->current.io.col_type[GMT_IN][i] == GMT_IS_UNKNOWN)
			C->current.io.col_type[GMT_IN][i] = GMT_IS_FLOAT;
	}

	C->current.io.input = gmt_nc_input;
	tmp_pointer = (GMT_LONG)(-C->current.io.ncid);
	return ((FILE *)tmp_pointer);
}

FILE *GMT_fopen (struct GMT_CTRL *C, const char *filename, const char *mode)
{
	char path[GMT_BUFSIZ];
	FILE *fd = NULL;

	if (mode[0] != 'r')	/* Open file for writing (no netCDF) */
		return (fopen (filename, mode));
	else if (C->common.b.active[GMT_IN])	/* Definitely not netCDF */
		return (fopen (GMT_getdatapath(C, filename, path), mode));
#ifdef GMT_COMPAT
	else if (C->common.b.varnames[0])	/* Definitely netCDF */
		return (gmt_nc_fopen (C, filename, mode));
#endif
	else if (strchr (filename, '?'))	/* Definitely netCDF */
		return (gmt_nc_fopen (C, filename, mode));
#ifdef WIN32
	else if (!strcmp (filename, "NUL"))	/* Special case of /dev/null under Windows */
#else
	else if (!strcmp (filename, "/dev/null"))	/* The Unix null device; catch here to avoid gmt_nc_fopen */
#endif
	{
		return (fopen (GMT_getdatapath(C, filename, path), mode));
	}
	else {	/* Maybe netCDF */
		fd = gmt_nc_fopen (C, filename, mode);
		if (!fd) {
			char *c;
			if ((c = GMT_getdatapath(C, filename, path)) != NULL) fd = fopen(c, mode);
		}
		return (fd);
	}
}

/* Table I/O routines for ascii and binary io */

GMT_LONG GMT_set_cols (struct GMT_CTRL *C, GMT_LONG direction, GMT_LONG expected)
{	/* Initializes the internal GMT->common.b.ncol[] settings.
	 * direction is either GMT_IN or GMT_OUT.
	 * expected is the expected or known number of columns.  Use 0 if not known.
	 * For binary input or output the number of columns must be specified.
	 * For ascii output the number of columns must also be specified.
	 * For ascii input the i/o machinery will set this automatically so expected is ignored.
	 * Programs that need to read an input record in order to determine how
	 * many columns on output should call this function after returning the
	 * first data record; otherwise, call it before registering the resource.
	 */
	static char *mode[2] = {"input", "output"};

	if (! (direction == GMT_IN || direction == GMT_OUT)) return (GMT_NOT_A_VALID_DIRECTION);
	if (C->common.b.ncol[direction]) return (GMT_OK);	/* Already set by -b */

	if (expected == 0 && (direction == GMT_OUT || C->common.b.active[direction])) {
		GMT_report (C, GMT_MSG_FATAL, "Number of %s columns has not been set\n", mode[direction]);
		return (GMT_N_COLS_NOT_SET);
	}
	/* Here we may set the number of data columns */
	if (C->common.b.active[direction]) {	/* Must set uninitialized input/output pointers */
		GMT_LONG col;
		char type = (C->common.b.type[direction]) ? C->common.b.type[direction] : 'd';
		for (col = 0; col < expected; col++) {
			C->current.io.fmt[direction][col].io = GMT_get_io_ptr (C, direction, C->common.b.swab[direction], type);
			C->current.io.fmt[direction][col].type = GMT_get_io_type (C, type);
		}
		C->common.b.ncol[direction] = expected;
	}
	else
		C->common.b.ncol[direction] = (direction == GMT_IN && expected == 0) ? GMT_MAX_COLUMNS : expected;
	return (GMT_OK);
}

char *GMT_getuserpath (struct GMT_CTRL *C, const char *stem, char *path)
{
	/* stem is the name of the file, e.g., gmt.conf
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in the temporary directory (if defined),
	 * current directory, home directory and $C->session.USERDIR (default ~/.gmt)
	 */

	/* If a full path is given, we only look for that file directly */

#ifdef WIN32
	if (stem[0] == '/' || stem[1] == ':')
#else
	if (stem[0] == '/')
#endif
	{
		if (!access (stem, R_OK)) return (strcpy (path, stem));	/* Yes, found it */
		return (NULL);	/* No file found, give up */
	}

	/* In isolation mode (when C->session.TMPDIR is defined), we first look there */

	if (C->session.TMPDIR) {
		sprintf (path, "%s/%s", C->session.TMPDIR, stem);
		if (!access (path, R_OK)) return (path);
	}

	/* Then look in the current working directory */

	if (!access (stem, R_OK)) return (strcpy (path, stem));	/* Yes, found it */

	/* If still not found, see if there is a file in the GMT_{HOME,USER}DIR directories */

	if (C->session.HOMEDIR) {
		sprintf (path, "%s/%s", C->session.HOMEDIR, stem);
		if (!access (path, R_OK)) return (path);
	}
	if (C->session.USERDIR) {
		sprintf (path, "%s/%s", C->session.USERDIR, stem);
		if (!access (path, R_OK)) return (path);
	}

	return (NULL);	/* No file found, give up */
}

char *GMT_getdatapath (struct GMT_CTRL *C, const char *stem, char *path)
{
	/* stem is the name of the file, e.g., grid.img
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory and $GMT_{USER,DATA}DIR
	 * If the dir ends in / we traverse recursively [not under Windows].
	 */
	GMT_LONG d, pos, L, found;
	char *udir[2] = {C->session.USERDIR, C->session.DATADIR}, dir[GMT_BUFSIZ];
	char path_separator[2] = {PATH_SEPARATOR, '\0'};
#ifdef HAVE_DIRENT_H_
	GMT_LONG N;
	GMT_LONG gmt_traverse_dir (const char *file, char *path);
#endif /* HAVE_DIRENT_H_ */
	GMT_LONG gmt_file_is_readable (struct GMT_CTRL *C, char *path);

	/* First look in the current working directory */

	if (!access (stem, F_OK)) {	/* Yes, found it */
		if (gmt_file_is_readable (C, (char *)stem)) {	/* Yes, can read it */
			strcpy (path, stem);
			return (path);
		}
		return (NULL);	/* Cannot read, give up */
	}

	/* If we got here and a full path is given, we give up ... unless it is one of those /vsi.../ files */
	if (stem[0] == '/') {
#ifdef USE_GDAL
		if (GMT_check_url_name ((char *)stem))
			return ((char *)stem);			/* With GDAL all the /vsi-stuff is given existence credit */
		else
			return (NULL);
#else
		return (NULL);
#endif
	}

#ifdef WIN32
	if (stem[1] == ':') return (NULL);
#endif

	/* Not found, see if there is a file in the GMT_{USER,DATA}DIR directories [if set] */

	for (d = 0; d < 2; d++) {	/* Loop over USER and DATA dirs */
		if (!udir[d]) continue;	/* This directory was not set */
		found = pos = 0;
		while (!found && (GMT_strtok (udir[d], path_separator, &pos, dir))) {
			L = strlen (dir);
#ifdef HAVE_DIRENT_H_
#ifdef GMT_COMPAT
			if (dir[L-1] == '*' || dir[L-1] == '/') {	/* Must search recursively from this dir */
				N = (dir[L-1] == '/') ? L - 1 : L - 2;
#else
			if (dir[L-1] == '/') {	/* Must search recursively from this dir */
				N = L - 1;
#endif /* GMT_COMPAT */
				strncpy (path, dir, N);	path[N] = 0;
				found = gmt_traverse_dir (stem, path);
			}
			else {
#endif /* HAVE_DIRENT_H_ */
				sprintf (path, "%s/%s", dir, stem);
				found = (!access (path, F_OK));
#ifdef HAVE_DIRENT_H_
			}
#endif /* HAVE_DIRENT_H_ */
		}
		if (found && gmt_file_is_readable (C, path)) return (path);	/* Yes, can read it */
	}

	return (NULL);	/* No file found, give up */
}

GMT_LONG gmt_file_is_readable (struct GMT_CTRL *C, char *path)
{	/* Returns TRUE if readable, otherwise give error and return FALSE */
	if (!access (path, R_OK)) return (TRUE);	/* Readable */
	/* Get here when found, but not readable */
	GMT_report (C, GMT_MSG_FATAL, "Unable to read %s (permissions?)\n", path);
	return (FALSE);	/* Cannot read, give up */
}

#ifdef HAVE_DIRENT_H_
GMT_LONG gmt_traverse_dir (const char *file, char *path) {
	/* Look for file in the directory pointed to by path, recursively */
	DIR *D = NULL;
	struct dirent *F = NULL;
	int len, d_namlen;
	GMT_LONG ok = FALSE;
	char savedpath[GMT_BUFSIZ];

 	if ((D = opendir (path)) == NULL) return (0);	/* Unable to open directory listing */
	len = strlen (file);
	strcpy (savedpath, path);	/* Make copy of current directory path */

	while (!ok && (F = readdir (D)) != NULL) {	/* For each directory entry until end or ok becomes TRUE */
		d_namlen = strlen (F->d_name);
		if (d_namlen == 1 && F->d_name[0] == '.') continue;				/* Skip current dir */
		if (d_namlen == 2 && F->d_name[0] == '.' && F->d_name[1] == '.') continue;	/* Skip parent dir */
#ifdef HAVE_SYS_DIR_H_
		if (F->d_type == DT_DIR) {	/* Entry is a directory; must search this directory recursively */
			sprintf (path, "%s/%s", savedpath, F->d_name);
			ok = gmt_traverse_dir (file, path);
		}
		else if (d_namlen == len && !strcmp (F->d_name, file)) {	/* Found the file in this dir (i.e., F_OK) */
			sprintf (path, "%s/%s", savedpath, file);
			ok = TRUE;
		}
#endif /* HAVE_SYS_DIR_H_ */
	}
	(void)closedir (D);
	return (ok);	/* did or did not find file */
}
#endif /* HAVE_DIRENT_H_ */

char *GMT_getsharepath (struct GMT_CTRL *C, const char *subdir, const char *stem, const char *suffix, char *path)
{
	/* stem is the prefix of the file, e.g., gmt_cpt for gmt_cpt.conf
	 * subdir is an optional subdirectory name in the $GMT_SHAREDIR directory.
	 * suffix is an optional suffix to append to name
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory, $GMT_USERDIR (default ~/.gmt) and $GMT_SHAREDIR/subdir
	 */

	/* First look in the current working directory */

	sprintf (path, "%s%s", stem, suffix);
	if (!access (path, R_OK)) return (path);	/* Yes, found it in current directory */

	/* Do not continue when full pathname is given */

	if (stem[0] == '/') return (NULL);
#ifdef WIN32
	if (stem[0] && stem[1] == ':') return (NULL);
#endif

	/* Not found, see if there is a file in the user's GMT_USERDIR (~/.gmt) directory */

	if (C->session.USERDIR) {
		/* Try to get file from $GMT_USERDIR */
		sprintf (path, "%s/%s%s", C->session.USERDIR, stem, suffix);
		if (!access (path, R_OK)) return (path);
		/* Try to get file from $GMT_USERDIR/subdir */
		sprintf (path, "%s/%s/%s%s", C->session.USERDIR, subdir, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Try to get file from $GMT_SHAREDIR/subdir */

	if (subdir) {
		sprintf (path, "%s/%s/%s%s", C->session.SHAREDIR, subdir, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	return (NULL);	/* No file found, give up */
}

int GMT_access (struct GMT_CTRL *C, const char* filename, int mode)
{	/* Like access but also checks the GMT_*DIR places */
	char file[GMT_BUFSIZ];

	file[0] = '\0';		/* 'Initialize' it so we can test if it's still 'empty' after the sscanf below */
	if (!filename || !filename[0])
		return (-1);	/* No file given */
	sscanf (filename, "%[^=?]", file);		/* Exclude netcdf 3/-D grid extensions to make sure we get a valid file name */
	if (file[0] == '\0')
		return(-1);		/* It happens for example when parsing grdmath args and it finds an isolated  "=" */

	if (mode == W_OK)
		return (access (file, mode));	/* When writing, only look in current directory */
	if (mode == R_OK || mode == F_OK) {	/* Look in special directories when reading or just checking for existance */
		char path[GMT_BUFSIZ];
		return (GMT_getdatapath (C, file, path) ? 0 : -1);
	}
	/* If we get here then mode is bad (X_OK)? */
	fprintf (stderr, "GMT: Bad mode (%d) passed to GMT_access\n", mode);
	return (-1);
}

double gmt_convert_aspatial_value (struct GMT_CTRL *C, GMT_LONG type, char *V)
{
	/* Return the value associated with the aspatial values given for this column col as a double */

	double value;

	switch (type) {
		case GMTAPI_DOUBLE:
		case GMTAPI_FLOAT:
		case GMTAPI_ULONG:
		case GMTAPI_LONG:
		case GMTAPI_UINT:
		case GMTAPI_INT:
		case GMTAPI_USHORT:
		case GMTAPI_SHORT:
		case GMTAPI_CHAR:
		case GMTAPI_UCHAR:
			value = atof (V);
			break;
		case GMTAPI_TIME:
			GMT_scanf_arg (C, V, GMT_IS_ABSTIME, &value);
			break;
		default:	/* Give NaN */
			value = C->session.d_NaN;
			break;
	}
	return (value);
}

GMT_LONG gmt_ogr_decode_aspatial_values (struct GMT_CTRL *C, char *record, struct GMT_OGR *S)
{	/* Parse @D<vals> aspatial values; this is done once per feature (segment).  We store
 	 * both the text representation (value) and attempt to convert to double in dvalue.
 	 * We use S->n_aspatial to know how many values there are .*/
	GMT_LONG pos = 0, col = 0;
	char buffer[GMT_BUFSIZ], p[GMT_BUFSIZ];

	if (S->n_aspatial == 0) return (0);	/* Nothing to do */
	if (S->value == NULL) {			/* First time, allocate space */
		S->value  = GMT_memory (C, S->value,  S->n_aspatial, char *);
		S->dvalue = GMT_memory (C, S->dvalue, S->n_aspatial, double);
	}
	strcpy (buffer, record);
	while ((GMT_strtok1 (buffer, '|', &pos, p))) {
		if (S->value[col]) free (S->value[col]);	/* Free previous item */
		S->value[col]  = strdup (p);
		S->dvalue[col] = gmt_convert_aspatial_value (C, S->type[col], p);
		col++;
	}
	if (col == (S->n_aspatial-1)) {	/* Last item was blank */
		S->value[col] = strdup ("");	/* Allocate space for blank string */
	}
	return (col);
}

void gmt_copy_and_truncate (char *out, char *in)
{	/* Duplicate in to out, then find the first space not inside quotes and truncate string there */
	GMT_LONG quote = FALSE;
	while (*in && (quote || *in != ' ')) {
		*out++ = *in;	/* Copy char */
		if (*in++ == ' ') quote = !quote;	/* Wind to next space except skip if inside double quotes */
	}
	*out = '\0';	/* Terminate string */
}

GMT_LONG gmt_ogr_decode_aspatial_types (struct GMT_CTRL *C, char *record, struct GMT_OGR *S)
{	/* Parse aspatial types; this is done once per dataset */
	GMT_LONG pos = 0, col = 0, n_alloc;
	char buffer[GMT_BUFSIZ], p[GMT_BUFSIZ];

	n_alloc = (S->type) ? GMT_BUFSIZ : 0;
	gmt_copy_and_truncate (buffer, record);
	while ((GMT_strtok (buffer, "|", &pos, p))) {
		if (col == n_alloc) S->type = GMT_memory (C, S->type, n_alloc += GMT_TINY_CHUNK, GMT_LONG);
		S->type[col++] = gmt_ogr_get_type (p);
	}
	if (n_alloc < GMT_BUFSIZ && col < n_alloc) S->type = GMT_memory (C, S->type, col, GMT_LONG);
	return (col);
}

GMT_LONG gmt_ogr_decode_aspatial_names (struct GMT_CTRL *C, char *record, struct GMT_OGR *S)
{	/* Decode aspatial names; this is done once per dataset */
	GMT_LONG pos = 0, col = 0, n_alloc;
	char buffer[GMT_BUFSIZ], p[GMT_BUFSIZ];

	n_alloc = (S->type) ? GMT_BUFSIZ : 0;
	gmt_copy_and_truncate (buffer, record);
	while ((GMT_strtok (buffer, "|", &pos, p))) {
		if (col == n_alloc) S->name = GMT_memory (C, S->name, n_alloc += GMT_TINY_CHUNK, char *);
		S->name[col++] = strdup (p);
	}
	if (n_alloc < GMT_BUFSIZ && col < n_alloc) S->name = GMT_memory (C, S->name, col, char *);
	return (col);
}

GMT_LONG GMT_append_ogr_item (struct GMT_CTRL *C, char *name, GMT_LONG type, struct GMT_OGR *S)
{
	/* Adds one more metadata item to this OGR structure */
	S->n_aspatial++;
	S->name = GMT_memory (C, S->name, S->n_aspatial, char *);
	S->name[S->n_aspatial-1] = strdup (name);
	S->type = GMT_memory (C, S->type, S->n_aspatial, GMT_LONG);
	S->type[S->n_aspatial-1] = type;
	return (GMT_NOERROR);
}

GMT_LONG gmt_ogr_parser (struct GMT_CTRL *C, char *record)
{	/* Parsing of the GMT/OGR vector specification (v 1.0). See Appendix R */
	return (C->current.io.ogr_parser (C, record));
}

GMT_LONG gmt_ogr_data_parser (struct GMT_CTRL *C, char *record)
{	/* Parsing of the GMT/OGR vector specification (v 1.0) for data feature records.
 	 * We KNOW C->current.io.ogr == +1, i.e., current file is a GMT/OGR file.
	 * We also KNOW that C->current.io.OGR has been allocated by gmt_ogr_header_parser.
	 * For GMT/OGR files we must parse and store the metadata in C->current.io.OGR,
	 * from where higher-level functions can access it.  GMT_End_IO will free the structure.
	 * This function returns TRUE if we parsed a GMT/OGR record and FALSE otherwise.
	 * If we encounter a parsing error we stop parsing any further by setting C->current.io.ogr = 0.
	 * We loop until all @<info> tags have been processed on this record.
	 */

	GMT_LONG n_aspatial, quote;
	char *p = NULL;
	struct GMT_OGR *S = NULL;

	if (record[0] != '#') return (FALSE);			/* Not a comment record so no point looking further */
	if (!(p = strchr (record, '@'))) return (FALSE);	/* Not an OGR/GMT record since @ was not found */

	/* Here we are reasonably sure that @? strings are OGR/GMT feature specifications */

	GMT_chop (record);	/* Get rid of linefeed etc */

	S = C->current.io.OGR;	/* Set S shorthand */
	quote = FALSE;

	while (*p == '@') {
		++p;	/* Move to first char after @ */
		switch (p[0]) {	/* These are the feature tags only: @D, @P, @H */
			case 'D':	/* Aspatial data values, store in segment header  */
				if (!S->geometry) { GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @D given but no geometry set\n"); return (0);}
				n_aspatial = gmt_ogr_decode_aspatial_values (C, &p[1], S);
				if (S->n_aspatial != n_aspatial) {
					GMT_report (C, GMT_MSG_VERBOSE, "OGR/GMT: Some @D items not specified (set to NULL)\n");
				}
				break;

			case 'P':	/* Polygon perimeter, store in segment header  */
				if (!(S->geometry == GMT_IS_POLYGON || S->geometry == GMT_IS_MULTIPOLYGON)) {
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @P only valid for polygons\n");
					C->current.io.ogr = 0;
					return (FALSE);
				}
				S->pol_mode = GMT_IS_PERIMETER;
				break;

			case 'H':	/* Polygon hole, store in segment header  */
				if (!(S->geometry == GMT_IS_POLYGON || S->geometry == GMT_IS_MULTIPOLYGON)) {
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @H only valid for polygons\n");
					C->current.io.ogr = 0;
					return (0);
				}
				S->pol_mode = GMT_IS_HOLE;
				break;

			default:	/* Bad OGR record? */
				GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: Cannot have @%c after FEATURE_DATA\n", p[0]);
				C->current.io.ogr = 0;
				break;
		}
		while (*p && (quote || *p != '@')) if (*p++ == '\"') quote = !quote;	/* Wind to next @ except skip if inside double quotes */
	}
	return (TRUE);
}

GMT_LONG gmt_get_ogr_id (struct GMT_OGR *G, char *name)
{
	GMT_LONG k;
	for (k = 0; k < G->n_aspatial; k++) if (!strcmp (name, G->name[k])) return (k);
	return (GMTAPI_NOTSET);
}

void gmt_align_ogr_values (struct GMT_CTRL *C)
{
	GMT_LONG k, id;
	if (!C->common.a.active) return;	/* Nothing selected with -a */
	for (k = 0; k < C->common.a.n_aspatial; k++) {	/* Process the requested columns */
		id = gmt_get_ogr_id (C->current.io.OGR, C->common.a.name[k]);	/* See what order in the OGR struct this -a column appear */
		C->common.a.ogr[k] = id;
	}
}

GMT_LONG gmt_ogr_header_parser (struct GMT_CTRL *C, char *record)
{	/* Parsing of the GMT/OGR vector specification (v 1.0).
 	 * C->current.io.ogr can have three states:
	 *	-1 if not yet set [this is how it is initialized].
	 *	 0 if file has been determined NOT to be a GMT/OGR file.
	 *	+1 if it has met the criteria and is a GMT/OGR file.
	 * For GMT/OGR files we must parse and store the metadata in C->current.io.OGR,
	 * from where higher-level functions can access it.  GMT_End_IO will free the structure.
	 * This function returns TRUE if we parsed a GMT/OGR record and FALSE otherwise.
	 * If we encounter a parsing error we stop parsing any further by setting C->current.io.ogr = 0.
	 * We loop until all @<info> tags have been processed on this record.
	 * gmt_ogr_parser will point to this function until the header has been parsed, then it is
	 * set to point to gmt_ogr_data_parser instead, to speed up data record processing.
	 */

	GMT_LONG geometry = 0, n_aspatial, quote, k;
	char *p = NULL;
	struct GMT_OGR *S = NULL;

	if (!C->current.io.ogr) return (FALSE);			/* No point parsing further if we KNOW it is not OGR */
	if (record[0] != '#') return (FALSE);			/* Not a comment record so no point looking any further */
	if (C->current.io.ogr == 1 && !strncmp (record, "# FEATURE_DATA", 14)) {	/* It IS an OGR file and we found end of OGR header section and start of feature data */
		C->current.io.ogr_parser = gmt_ogr_data_parser;	/* From now on only parse for feature tags */
		gmt_align_ogr_values (C);	/* Simplify copy from aspatial values to input columns as per -a option */
		return (TRUE);
	}
	if (!(p = strchr (record, '@'))) return (FALSE);	/* Not an OGR/GMT record since @ was not found */
	
	if (C->current.io.ogr == -1 && !strncmp (p, "@VGMT", 5)) {	/* Found the OGR version identifier, look for @G if on the same record */
		if (C->common.a.output) {	/* Cannot read OGR files when -a is used to define output */
			GMT_report (C, GMT_MSG_FATAL, "Cannot read OGR/GMT files when -a is used to define output format\n");
			GMT_exit (EXIT_FAILURE);
		}
		C->current.io.ogr = 1;				/* File is a GMT/OGR geospatial file */
		if (!(p = strchr (&p[5], '@'))) return (TRUE);	/* No more @ codes; goto next record */
	}
	if (C->current.io.ogr != 1) return (FALSE);		/* No point parsing further since file is not GMT/OGR (yet) */

	/* Here we are reasonably sure that @? strings are OGR/GMT header specifications */

	GMT_chop (record);	/* Get rid of linefeed etc */

	/* Allocate S the first time we get here */
	
	if (!C->current.io.OGR) C->current.io.OGR = GMT_memory (C, NULL, 1, struct GMT_OGR);
	S = C->current.io.OGR;
	quote = FALSE;

	while (*p == '@') {
		++p;	/* Move to first char after @ */

		switch (p[0]) {	/* These are the header tags */
		
			case 'G':	/* Geometry */
				if (!strncmp (&p[1], "LINESTRING", 10))
					geometry = GMT_IS_LINESTRING;
				else if (p[1] == 'P') {
					if (!strncmp (&p[2], "OLYGON", 6))
						geometry = GMT_IS_POLYGON;
					else if (!strncmp (&p[2], "OINT", 4))
						geometry = GMT_IS_POINT;
				}
				else if (!strncmp (&p[1], "MULTI", 5)) {
					if (!strncmp (&p[6], "POINT", 5))
						geometry = GMT_IS_MULTIPOINT;
					else if (!strncmp (&p[6], "LINESTRING", 10))
						geometry = GMT_IS_MULTILINESTRING;
					else if (!strncmp (&p[6], "POLYGON", 7))
						geometry = GMT_IS_MULTIPOLYGON;
					else {
						GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @G unrecognized geometry\n");
						C->current.io.ogr = 0;
						return (FALSE);
					}
				}
				if (!S->geometry)
					S->geometry = geometry;
				else if (S->geometry != geometry) {
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @G cannot have different geometries\n");
					C->current.io.ogr = 0;
				}
				break;

			case 'N':	/* Aspatial name fields, store in table header */
				if (!S->geometry) { GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @N given but no geometry set\n"); return (0);}
				if (S->name) {	/* Already set */
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @N Cannot have more than one per segment\n");
					C->current.io.ogr = 0;
					return (FALSE);
				}
				n_aspatial = gmt_ogr_decode_aspatial_names (C, &p[1], S);
				if (S->n_aspatial == 0)
					S->n_aspatial = n_aspatial;
				else if (S->n_aspatial != n_aspatial) {
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @N number of items vary\n");
					C->current.io.ogr = 0;
				}
				break;

			case 'J':	/* Dataset projection strings (one of 4 kinds) */
				switch (p[1]) {
					case 'e': k = 0;	break;	/* EPSG code */
					case 'g': k = 1;	break;	/* GMT proj code */
					case 'p': k = 2;	break;	/* Proj.4 code */
					case 'w': k = 3;	break;	/* OGR WKT representation */
					default:
						GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @J given unknown format (%c)\n", (int)p[1]);
						C->current.io.ogr = 0;
						return (FALSE);
				}
				S->proj[k] = strdup (&p[2]);
				break;

			case 'R':	/* Dataset region */
				if (S->region) { /* Already set */
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @R can only appear once\n");
					C->current.io.ogr = 0;
					return (FALSE);
				}
				S->region = strdup (&p[1]);
				break;

			case 'T':	/* Aspatial field types, store in table header  */
				if (!S->geometry) { GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @T given but no geometry set\n"); return (0);}
				if (S->type) {	/* Already set */
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @T Cannot have more than one per segment\n");
					C->current.io.ogr = 0;
					return (FALSE);
				}
				n_aspatial = gmt_ogr_decode_aspatial_types (C, &p[1], S);
				if (S->n_aspatial == 0)
					S->n_aspatial = n_aspatial;
				else if (S->n_aspatial != n_aspatial) {
					GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @T number of items vary\n");
					C->current.io.ogr = 0;
				}
				break;

			default:	/* Just record, probably means this is NOT a GMT/OGR file after all */
				GMT_report (C, GMT_MSG_FATAL, "Bad OGR/GMT: @%c not allowed before FEATURE_DATA\n", (int)p[0]);
				C->current.io.ogr = 0;
				break;
		}

		while (*p && (quote || *p != '@')) if (*p++ == '\"') quote = !quote;	/* Wind to next @ except skip if inside double quotes */
	}
	return (TRUE);
}

#define gmt_convert_col(S,x) {if (S.convert) x = ((S.convert == 2) ? log10 (x) : x) * S.scale + S.offset;}

GMT_LONG gmt_assign_aspatial_cols (struct GMT_CTRL *C)
{	/* This function will load input columns with aspatial data as requested by -a.
 	 * It will then handle any possible -i scalings/offsets as well for those columns */

	GMT_LONG k, n;
	double value;
	if (C->current.io.ogr != 1) return (0);		/* No point checking further since file is not GMT/OGR */
	for (k = n = 0; k < C->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (C->common.a.col[k] < 0) continue;	/* Not meant for data columns */
		value = C->current.io.OGR->dvalue[C->common.a.ogr[k]];
		gmt_convert_col (C->current.io.col[GMT_IN][C->common.a.col[k]], value);
		C->current.io.curr_rec[C->common.a.col[k]] = value;
		n++;
	}
	return (n);
}

char *GMT_trim_segheader (struct GMT_CTRL *C, char *line) {
	/* Trim trailing junk and return pointer to first non-space/tab/> part of segment header
	 * Do not try to free the returned pointer!
	 */
	GMT_strstrip (line, FALSE); /* Strip trailing whitespace */
	/* Skip over leading whitespace and segment marker */
	while (*line && (isspace(*line) || *line == C->current.setting.io_seg_marker[GMT_IN]))
		++line;
	/* Return header string */
	return (line);
}

GMT_LONG GMT_is_a_NaN_line (struct GMT_CTRL *C, char *line)
{	/* Returns TRUE if record is NaN NaN [NaN NaN] etc */
	GMT_LONG pos = 0;
	char p[GMT_TEXT_LEN256];
	
	while ((GMT_strtok (line, " \t,", &pos, p))) {
		GMT_str_tolower (p);
		if (strncmp (p, "nan", (size_t)3)) return (FALSE);
	}
	return (TRUE);
}

GMT_LONG GMT_is_segment_header (struct GMT_CTRL *C, char *line)
{	/* Returns 1 if this record is a GMT segment header;
	 * Returns 2 if this record is a segment breaker;
	 * Otherwise returns 0 */
	if (C->current.setting.io_blankline[GMT_IN] && GMT_is_a_blank_line (line)) return (2);	/* Treat blank line as segment break */
	if (C->current.setting.io_nanline[GMT_IN] && GMT_is_a_NaN_line (C, line)) return (2);	/* Treat NaN-records as segment break */
	if (line[0] == C->current.setting.io_seg_marker[GMT_IN]) return (1);	/* Got a regular GMT segment header */
	return (0);	/* Not a segment header */
}

/* This is the lowest-most input function in GMT.  All ASCII table data are read via
 * gmt_ascii_input.  Changes here affect all programs that read such data. */

void * gmt_ascii_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, GMT_LONG *status)
{
	GMT_LONG pos, col_no = 0, in_col, col_pos, n_convert, n_ok = 0, kind, add, n_use = 0;
	GMT_LONG done = FALSE, bad_record, set_nan_flag = FALSE;
	char line[GMT_BUFSIZ], *p = NULL, token[GMT_BUFSIZ];
	double val;

	/* gmt_ascii_input will skip blank lines and shell comment lines which start
	 * with #.  Fields may be separated by spaces, tabs, or commas.  The routine returns
	 * the actual number of items read [or 0 for segment header and -1 for EOF]
	 * If *n is passed as GMT_BUFSIZ it will be reset to the actual number of fields.
	 * If gap checking is in effect and one of the checks involves a column beyond
	 * the ones otherwise needed by the program we extend the reading so we may
	 * examin the column needed in the gap test.
	 */

	while (!done) {	/* Done becomes TRUE when we successfully have read a data record */

		/* First read until we get a non-blank, non-comment record, or reach EOF */

		C->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		C->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		if (C->current.io.io_header[GMT_IN] && C->current.io.rec_in_tbl_no < C->current.io.io_n_header_items) {	/* Must treat first io_n_header_items as headers */
			p = GMT_fgets (C, line, GMT_BUFSIZ, fp);	/* Get the line */
			strcpy (C->current.io.current_record, line);
			C->current.io.status = GMT_IO_TBL_HEADER;
			C->current.io.io_header[GMT_OUT] = TRUE;	/* Turn on table headers on output */
			*status = 0;
			return (NULL);
		}
		/* Here we are done with any header records implied by -h */
		if (C->current.setting.io_blankline[GMT_IN]) {	/* Treat blank lines as segment markers, so only read one line */
			p = GMT_fgets (C, line, GMT_BUFSIZ, fp);
			C->current.io.rec_no++, C->current.io.rec_in_tbl_no++;
		}
		else {	/* Skip all blank lines until we get something else */
			while ((p = GMT_fgets (C, line, GMT_BUFSIZ, fp)) && GMT_is_a_blank_line (line)) C->current.io.rec_no++, C->current.io.rec_in_tbl_no++;
		}
		if (gmt_ogr_parser (C, line)) continue;	/* If we parsed a GMT/OGR record we go up to top of loop and get the next record */
		if (line[0] == '#') {	/* Got a file header, copy it and return */
			strcpy (C->current.io.current_record, line);
			C->current.io.status = GMT_IO_TBL_HEADER;
			*status = 0;
			return (NULL);
		}
		if (!p) {	/* Ran out of records, which can happen if file ends in a comment record */
			C->current.io.status = GMT_IO_EOF;
			if (C->current.io.give_report && C->current.io.n_bad_records) {	/* Report summary and reset counters */
				GMT_report (C, GMT_MSG_FATAL, "This file had %ld records with invalid x and/or y values\n", C->current.io.n_bad_records);
				C->current.io.n_bad_records = C->current.io.pt_no = C->current.io.n_clean_rec = 0;
				C->current.io.rec_no = C->current.io.rec_in_tbl_no = -1;
			}
			*status = -1;
			return (NULL);
		}

		if ((kind = GMT_is_segment_header (C, line))) {	/* Got a segment header, take action and return */
			C->current.io.status = GMT_IO_SEG_HEADER;
			GMT_set_segmentheader (C, GMT_OUT, TRUE);	/* Turn on segment headers on output */
			C->current.io.seg_no++;
			if (kind == 1) {
				/* Just save the header content, not the marker and leading whitespace */
				strcpy (C->current.io.segment_header, GMT_trim_segheader (C, line));
			}
			else	/* Got a segment break instead - set header to NULL */
				C->current.io.segment_header[0] = '\0';
			*status = 0;
			return (NULL);
		}

		/* Here we know we are processing a data record */

		if (C->common.a.active && !C->current.io.ogr) {	/* Cannot give -a but not reading an OGR/GMT file */
			GMT_report (C, GMT_MSG_FATAL, "Aspatial associations set with -a but file is not in OGR/GMT format!\n");
			GMT_exit (EXIT_FAILURE);
		}

		n_use = gmt_n_cols_needed_for_gaps (C, *n);	/* Gives is the actual columns we need (which may > *n if gap checking is active; if gap check we also update prev_rec) */

		/* First chop off trailing whitespace and commas */

		GMT_strstrip (line, FALSE); /* Eliminate DOS endings and trailing white space, add linefeed */

		bad_record = set_nan_flag = FALSE;		/* Initialize flags */
		strcpy (C->current.io.current_record, line);	/* Keep copy of current record around */
		col_no = pos = n_ok = 0;			/* Initialize counters */
		in_col = -1;					/* Since we will increment right away inside the loop */

		while (!bad_record && col_no < n_use && (GMT_strtok (line, " \t,", &pos, token))) {	/* Get one field at the time until we run out or have issues */
			in_col++;	/* This is the actual column number in the input file */
			if (C->common.i.active) {	/* Must do special column-based processing since the -i option was set */
				if (C->current.io.col_skip[in_col]) continue;		/* Just skip and not even count this column */
				col_pos = C->current.io.col[GMT_IN][col_no].order;	/* Which data column will receive this value */
			}
			else				/* Default column order */
				col_pos = col_no;
			if ((n_convert = GMT_scanf (C, token, C->current.io.col_type[GMT_IN][col_pos], &val)) == GMT_IS_NAN) {	/* Got a NaN or it failed to decode the string */
				if (C->current.setting.io_nan_records || !C->current.io.skip_if_NaN[col_pos]) {	/* This field (or all fields) can be NaN so we pass it on */
					C->current.io.curr_rec[col_pos] = C->session.d_NaN;
					n_ok++;	/* Since NaN is considered an OK result */
				}
				else	/* Cannot have NaN in this column, flag record as bad */
					bad_record = TRUE;
				if (C->current.io.skip_if_NaN[col_pos]) set_nan_flag = TRUE;	/* Flag that we found NaN in a column that means we should skip */
			}
			else {					/* Successful decode, assign the value to the input array */
				gmt_convert_col (C->current.io.col[GMT_IN][col_no], val);
				C->current.io.curr_rec[col_pos] = val;
				n_ok++;
			}
			col_no++;		/* Count up number of columns found */
		}
		if ((add = gmt_assign_aspatial_cols (C))) {	/* We appended <add> columns given via aspatial OGR/GMT values */
			col_no += add;
			n_ok += add;
		}
		if (bad_record) {	/* This record failed our test and had NaNs */
			C->current.io.n_bad_records++;
			if (C->current.io.give_report && (C->current.io.n_bad_records == 1)) {	/* Report 1st occurrence of bad record */
				GMT_report (C, GMT_MSG_FATAL, "Encountered first invalid record near/at line # %ld\n", C->current.io.rec_no);
				GMT_report (C, GMT_MSG_FATAL, "Likely causes:\n");
				GMT_report (C, GMT_MSG_FATAL, "(1) Invalid x and/or y values, i.e. NaNs or garbage in text strings.\n");
				GMT_report (C, GMT_MSG_FATAL, "(2) Incorrect data type assumed if -J, -f are not set or set incorrectly.\n");
				GMT_report (C, GMT_MSG_FATAL, "(3) The -: switch is implied but not set.\n");
			}
		}
		else if (C->current.io.skip_duplicates && C->current.io.pt_no) {	/* Test to determine if we should skip repeated duplicate records with same x,y */
			done = !(C->current.io.curr_rec[GMT_X] == C->current.io.prev_rec[GMT_X] && C->current.io.curr_rec[GMT_Y] == C->current.io.prev_rec[GMT_Y]);	/* Yes, duplicate */
		}
		else
			done = TRUE;	/* Success, we can get out of this loop and return what we got */
	}
	C->current.io.status = (n_ok == n_use || *n == GMT_MAX_COLUMNS) ? 0 : GMT_IO_MISMATCH;	/* Hopefully set status to 0 (OK) */
	if (*n == GMT_MAX_COLUMNS) *n = n_ok;							/* Update the number of expected fields */
	if (GMT_REC_IS_ERROR (C)) GMT_report (C, GMT_MSG_FATAL, "Mismatch between actual (%ld) and expected (%ld) fields near line %ld\n", col_no, *n, C->current.io.rec_no);

	if (C->current.setting.io_lonlat_toggle[GMT_IN] && col_no >= 2) d_swap (C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) gmt_adjust_periodic (C);	/* Must account for periodicity in 360 as per current rule*/

	if (gmt_gap_detected (C)) {*status = gmt_set_gap (C); return (C->current.io.curr_rec); }	/* A gap between this an previous record was detected (see -g) so we set status and return 0 */

	C->current.io.pt_no++;			/* Got a valid data record (which is true even if it was a gap) */
	*status = n_ok;				/* Return the number of fields successfully read */
	if (set_nan_flag) {
		C->current.io.status |= GMT_IO_NAN;	/* Say we found NaNs */
		return (C->current.io.curr_rec);	/* Pass back pointer to data array */
	}
	return ((C->current.io.status) ? NULL : C->current.io.curr_rec);	/* Pass back pointer to data array */
}

char * GMT_ascii_textinput (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, GMT_LONG *status)
{
	char line[GMT_BUFSIZ], *p = NULL;

	/* GMT_ascii_textinput will read one text line and return it, setting
	 * header or segment flags in the process.
	 */

	/* First read until we get a non-blank, non-comment record, or reach EOF */

	C->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
	C->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
	while ((p = GMT_fgets (C, line, GMT_BUFSIZ, fp)) && gmt_ogr_parser (C, line)) {	/* Exits loop when we successfully have read a data record */
		C->current.io.rec_no++;		/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
		C->current.io.rec_in_tbl_no++;	/* Counts up, regardless of what this record is (data, junk, segment header, etc) */
	}
	/* Here we come once any OGR headers have been parsed and we have a real (non-OGR header) record */
	if (C->current.io.io_header[GMT_IN] && C->current.io.rec_in_tbl_no < C->current.io.io_n_header_items) {	/* Must treat first io_n_header_items as headers */
		strcpy (C->current.io.current_record, line);
		C->current.io.status = GMT_IO_TBL_HEADER;
		*status = 0;
		return (NULL);
	}
	if (!p) {	/* Ran out of records */
		C->current.io.status = GMT_IO_EOF;
		*n = 0;
		*status = -1;
		return (NULL);
	}
	if (line[0] == '#') {	/* Got a file header, take action and return */
		strcpy (C->current.io.current_record, line);
		C->current.io.status = GMT_IO_TBL_HEADER;
		*n = 1;
		*status = 0;
		return (NULL);
	}

	if (line[0] == C->current.setting.io_seg_marker[GMT_IN]) {	/* Got a segment header, take action and return */
		C->current.io.status = GMT_IO_SEG_HEADER;
		GMT_set_segmentheader (C, GMT_OUT, TRUE);	/* Turn on segment headers on output */
		C->current.io.seg_no++;
		/* Just save the header content, not the marker and leading whitespace */
		strcpy (C->current.io.segment_header, GMT_trim_segheader (C, line));
		*n = 1;
		*status = 0;
		return (NULL);
	}

	/* Normal data record */

	/* First chop off trailing whitespace and commas */

	GMT_strstrip (line, FALSE); /* Eliminate DOS endings and trailing white space */

	strcpy (C->current.io.current_record, line);

	C->current.io.status = 0;
	C->current.io.pt_no++;	/* Got a valid text record */
	*n = 1;			/* We always return 1 item as there are no columns */
	*status = 1;
	return (C->current.io.current_record);
}

GMT_LONG GMT_is_a_blank_line (char *line) {
	/* Returns TRUE if we should skip this line (because it is blank) */
	GMT_LONG i = 0;
	while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;	/* Wind past leading whitespace or tabs */
	if (line[i] == '\n' || line[i] == '\r') return (TRUE);
	return (FALSE);
}

GMT_LONG gmt_bin_colselect (struct GMT_CTRL *C)
{	/* When -i<cols> is used we must pull out and reset the current record */
	GMT_LONG col;
	static double tmp[GMT_BUFSIZ];
	for (col = 0; col < C->common.i.n_cols; col++) {
		tmp[C->current.io.col[GMT_IN][col].order] = C->current.io.curr_rec[C->current.io.col[GMT_IN][col].col];
		gmt_convert_col (C->current.io.col[GMT_IN][col], tmp[C->current.io.col[GMT_IN][col].order]);
	}
	GMT_memcpy (C->current.io.curr_rec, tmp, C->common.i.n_cols, double);
	return (C->common.i.n_cols);
	
}

/* Sub functions for gmt_bin_input */

GMT_LONG gmt_x_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG n)
{	/* Used to skip (*n) bytes; no reading takes place */
	if (GMT_fseek (fp, (long)n, SEEK_CUR)) {
		C->current.io.status = GMT_IO_EOF;
		return (-1);
	}
	return (1);
}

GMT_LONG gmt_get_binary_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG n) {
	/* Reads the n binary doubles from input and saves to C->current.io.curr_rec[] */
	GMT_LONG i, one = 1;
	
	if (n > GMT_MAX_COLUMNS) {
		GMT_report (C, GMT_MSG_FATAL, "Number of data columns (%ld) exceeds limit (GMT_MAX_COLUMS = %d)\n", n, GMT_MAX_COLUMNS);
		return (TRUE);	/* Done with this file */
	}
	for (i = 0; i < n; i++) {
		if (C->current.io.fmt[GMT_IN][i].skip < 0) gmt_x_read (C, fp, -C->current.io.fmt[GMT_IN][i].skip);	/* Pre-skip */
		if (C->current.io.fmt[GMT_IN][i].io (C, fp, &one, &C->current.io.curr_rec[i]) != 1) {	/* EOF or came up short */
			C->current.io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
			if (C->current.io.give_report && C->current.io.n_bad_records) {	/* Report summary and reset */
				GMT_report (C, GMT_MSG_FATAL, "This file had %ld records with invalid x and/or y values\n", C->current.io.n_bad_records);
				C->current.io.n_bad_records = C->current.io.rec_no = C->current.io.pt_no = C->current.io.n_clean_rec = 0;
			}
			return (TRUE);	/* Done with this file */
		}
		if (C->current.io.fmt[GMT_IN][i].skip > 0) gmt_x_read (C, fp, C->current.io.fmt[GMT_IN][i].skip);	/* Post-skip */
	}
	return (FALSE);	/* OK so far */
}

void * gmt_bin_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, GMT_LONG *retval)
{	/* General binary read function which calls function pointed to by C->current.io.read_binary to handle actual reading (and possbily swabbing) */
	GMT_LONG status, n_use;

	C->current.io.status = 0;
	do {	/* Keep reading until (1) EOF, (2) got a segment record, or (3) a valid data record */
		n_use = gmt_n_cols_needed_for_gaps (C, *n);
		if (gmt_get_binary_input (C, fp, n_use)) { *retval = -1; return (NULL); }	/* EOF */
		C->current.io.rec_no++;
		status = gmt_process_binary_input (C, n_use);
		if (status == 1) { *retval = 0; return (NULL); }		/* A segment header */
	} while (status == 2);	/* Continue reading when record is to be skipped */
	if (C->common.i.active) *n = gmt_bin_colselect (C);
	
	if (gmt_gap_detected (C)) { *retval = gmt_set_gap (C); return (C->current.io.curr_rec); }
	C->current.io.pt_no++;

	*retval = *n;
	return (C->current.io.curr_rec);
}

GMT_LONG gmt_skip_output (struct GMT_CTRL *C, double *cols, GMT_LONG n_cols)
{	/* Consult the -s[<cols>[r] setting and the cols values to determine if this record should be output */
	GMT_LONG c, n_nan;
	
	if (n_cols > GMT_MAX_COLUMNS) {
		GMT_report (C, GMT_MSG_FATAL, "Number of output data columns (%ld) exceeds limit (GMT_MAX_COLUMS = %d)\n", n_cols, GMT_MAX_COLUMNS);
		return (TRUE);	/* Skip record since we cannot access that many columns */
	}
	if (!C->current.setting.io_nan_mode) return (FALSE);				/* Normal case; output the record */
	if (C->current.setting.io_nan_mode == 3) {	/* -sa: Skip records if any NaNs are found */
		for (c = 0; c < n_cols; c++) if (GMT_is_dnan (cols[c]))  return (TRUE);	/* Found a NaN so we skip */
		return (FALSE);	/* No NaNs, output record */
	}
	for (c = n_nan = 0; c < C->current.io.io_nan_ncols; c++) {			/* Check each of the specified columns set via -s */
		if (C->current.io.io_nan_col[c] >= n_cols) continue;			/* Input record does not have this column */
		if (GMT_is_dnan (cols[C->current.io.io_nan_col[c]])) n_nan++;		/* Count the nan columns found */
	}
	if (n_nan < C->current.io.io_nan_ncols  && C->current.setting.io_nan_mode == 2) return (TRUE);	/* Skip records if -sr and not enough NaNs found */
	if (n_nan == C->current.io.io_nan_ncols && C->current.setting.io_nan_mode == 1) return (TRUE);	/* Skip records if -s and NaNs in specified columns */
	return (FALSE);	/* No match, output record */
}

GMT_LONG gmt_x_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n)
{	/* Used to write n bytes of space for filler on binary output */
	char c = ' ';
	GMT_LONG i;
	for (i = 0; i < n; i++) if (GMT_fwrite (&c, sizeof (char), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	return (GMT_NOERROR);
}

GMT_LONG gmt_bin_output (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *ptr)
{
	GMT_LONG i, k, n_out, col_pos;
	
	if (gmt_skip_output (C, ptr, n)) return (0);	/* Record was skipped via -s[r] */
	if (C->current.setting.io_lonlat_toggle[GMT_OUT]) d_swap (ptr[GMT_X], ptr[GMT_Y]);	/* Write lat/lon instead of lon/lat */
	n_out = (C->common.o.active) ? C->common.o.n_cols : n;
	for (i = k = 0; i < n_out; i++) {
		col_pos = (C->common.o.active) ? C->current.io.col[GMT_OUT][i].col : i;	/* Which data column to pick */
		if (C->current.io.col_type[GMT_OUT][col_pos] == GMT_IS_LON) GMT_lon_range_adjust (C->current.io.geo.range, &ptr[col_pos]);
		if (C->current.io.fmt[GMT_OUT][i].skip < 0) gmt_x_write (C, fp, -C->current.io.fmt[GMT_OUT][i].skip);	/* Pre-fill */
		k += C->current.io.fmt[GMT_OUT][i].io (C, fp, 1, &ptr[col_pos]);
		if (C->current.io.fmt[GMT_OUT][i].skip > 0) gmt_x_write (C, fp, C->current.io.fmt[GMT_OUT][i].skip);	/* Post-fill */
	}
	return (k);
}

void GMT_set_bin_input (struct GMT_CTRL *C) 
{	/* Make sure we point to binary input functions after processing -b option */
	if (C->common.b.active[GMT_IN]) {
		C->current.io.input = gmt_bin_input;
		strcpy (C->current.io.r_mode, "rb");
	}
	if (C->common.b.active[GMT_OUT]) {
		C->current.io.output = gmt_bin_output;
		strcpy (C->current.io.w_mode, "wb");
		strcpy (C->current.io.a_mode, "ab+");
	}
}

GMT_LONG GMT_ascii_output_col (struct GMT_CTRL *C, FILE *fp, double x, GMT_LONG col)
{	/* Formats x according to to output column number */
	char text[GMT_TEXT_LEN256];

	GMT_ascii_format_col (C, text, x, col);
	return (fprintf (fp, "%s", text));
}

GMT_LONG GMT_ascii_output (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *ptr)
{
	GMT_LONG i, col, last, e = 0, wn = 0, n_out;

	if (gmt_skip_output (C, ptr, n)) return (0);	/* Record was skipped via -s[r] */
	n_out = (C->common.o.active) ? C->common.o.n_cols : n;

	last = n_out - 1;				/* Last filed, need to output linefeed instead of delimiter */

	for (i = 0; i < n_out && e >= 0; i++) {		/* Keep writing all fields unless there is a read error (e == -1) */
		if (C->common.o.active)	/* Which data column to pick */
			col = C->current.io.col[GMT_OUT][i].col;
		else if (C->current.setting.io_lonlat_toggle[GMT_OUT] && i < 2)
			col = 1 - i;	/* Write lat/lon instead of lon/lat */
		else
			col = i;	/* Just goto next column */
		e = GMT_ascii_output_col (C, fp, ptr[col], col);	/* Write one item without any separator at the end */

		if (i == last)					/* This is the last field, must add newline */
			putc ('\n', fp);
		else if (C->current.setting.io_col_separator[0])		/* Not last field, and a separator is required */
			fprintf (fp, "%s", C->current.setting.io_col_separator);

		wn += e;
	}
	return ((e < 0) ? e : wn);
}

void gmt_format_geo_output (struct GMT_CTRL *C, GMT_LONG is_lat, double geo, char *text)
{
	GMT_LONG k, n_items, d, m, s, m_sec, minus, h_pos = 0;
	char hemi[3], *f;

	if (!is_lat) GMT_lon_range_adjust (C->current.io.geo.range, &geo);
	if (C->current.io.geo.decimal) {	/* Easy */
		f = (C->current.io.o_format[is_lat]) ? C->current.io.o_format[is_lat] : C->current.setting.format_float_out;
		sprintf (text, f, geo);
		return;
	}

	GMT_memset (hemi, 3, char);
	if (C->current.io.geo.wesn) {	/* Trailing WESN */
		if (C->current.io.geo.wesn == 2) hemi[h_pos++] = ' ';	/* Want space between numbers and hemisphere letter */
		if (is_lat)
			hemi[h_pos] = (GMT_IS_ZERO (geo)) ? 0 : ((geo < 0.0) ? 'S' : 'N');
		else
			hemi[h_pos] = (GMT_IS_ZERO (geo) || doubleAlmostEqual (geo, 180.0)) ? 0 : ((geo < 0.0) ? 'W' : 'E');
		geo = fabs (geo);
		if (hemi[h_pos] == 0) hemi[0] = 0;
	}

	for (k = n_items = 0; k < 3; k++) if (C->current.io.geo.order[k] >= 0) n_items++;	/* How many of d, m, and s are requested as integers */
	minus = GMT_geo_to_dms (geo, n_items, C->current.io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
	if (minus) text[0] = '-';	/* Must manually insert leading minus sign when degree == 0 */
	if (C->current.io.geo.n_sec_decimals) {		/* Wanted fraction printed */
		if (n_items == 3)
			sprintf (&text[minus], C->current.io.geo.y_format, d, m, s, m_sec, hemi);
		else if (n_items == 2)
			sprintf (&text[minus], C->current.io.geo.y_format, d, m, m_sec, hemi);
		else
			sprintf (&text[minus], C->current.io.geo.y_format, d, m_sec, hemi);
	}
	else if (n_items == 3)
		sprintf (&text[minus], C->current.io.geo.y_format, d, m, s, hemi);
	else if (n_items == 2)
		sprintf (&text[minus], C->current.io.geo.y_format, d, m, hemi);
	else
		sprintf (&text[minus], C->current.io.geo.y_format, d, hemi);
}

void gmt_format_abstime_output (struct GMT_CTRL *C, double dt, char *text)
{
	char date[GMT_CALSTRING_LENGTH], clock[GMT_CALSTRING_LENGTH];

	GMT_format_calendar (C, date, clock, &C->current.io.date_output, &C->current.io.clock_output, FALSE, 1, dt);
	sprintf (text, "%sT%s", date, clock);
}

void GMT_ascii_format_col (struct GMT_CTRL *C, char *text, double x, GMT_LONG col)
{	/* Format based on column position */
	if (GMT_is_dnan (x)) {	/* NaN, just write it as a string */
		sprintf (text, "NaN");
		return;
	}
	switch (C->current.io.col_type[GMT_OUT][col]) {
		case GMT_IS_LON:
			gmt_format_geo_output (C, FALSE, x, text);
			break;
		case GMT_IS_LAT:
			gmt_format_geo_output (C, TRUE, x, text);
			break;
		case GMT_IS_ABSTIME:
			gmt_format_abstime_output (C, x, text);
			break;
		default:	/* Floating point */
			if (C->current.io.o_format[col])	/* Specific to this column */
				sprintf (text, C->current.io.o_format[col], x);
			else	/* Use the general float format */
				sprintf (text, C->current.setting.format_float_out, x);
			break;
	}
}

void GMT_io_init (struct GMT_CTRL *C)
{
	/* No need to memset the structure to NULL as this is done in gmt_init.h directory.
	 * The assignments here are done once per GMT session as GMT_io_init is called
	 * from GMT_begin.  Some variables may change later due to --PAR=value parsing.
	 * GMT_io_init must be called before parsing of defaults. */

	GMT_LONG i;

	/* Pointer assignment for default ASCII input functions */

	C->current.io.input  = C->session.input_ascii = gmt_ascii_input;
	C->current.io.output = GMT_ascii_output;

	C->current.io.ogr_parser = gmt_ogr_header_parser;		/* Parse OGR header records to start with */

	/* Assign non-zero/NULL initial values */

	C->current.io.give_report = TRUE;
	C->current.io.seg_no = C->current.io.rec_no = C->current.io.rec_in_tbl_no = -1;	/* These gets incremented the first time */
	C->current.setting.io_seg_marker[GMT_IN] = C->current.setting.io_seg_marker[GMT_OUT] = '>';
	strcpy (C->current.io.r_mode, "r");
	strcpy (C->current.io.w_mode, "w");
	strcpy (C->current.io.a_mode, "a+");
	for (i = 0; i < 4; i++) {
		C->current.io.date_input.item_order[i] = C->current.io.date_input.item_pos[i] = -1;
		C->current.io.date_output.item_order[i] = C->current.io.date_output.item_pos[i] = -1;
	}
	for (i = 0; i < 3; i++) {
		C->current.io.clock_input.order[i] = C->current.io.clock_output.order[i] = C->current.io.geo.order[i] = -1;
	}
	strcpy (C->current.io.clock_input.ampm_suffix[0],  "am");
	strcpy (C->current.io.clock_output.ampm_suffix[0], "am");
	strcpy (C->current.io.clock_input.ampm_suffix[1],  "pm");
	strcpy (C->current.io.clock_output.ampm_suffix[1], "pm");

	for (i = 0; i < GMT_MAX_COLUMNS; i++) C->current.io.col[GMT_IN][i].col = C->current.io.col[GMT_IN][i].order = C->current.io.col[GMT_OUT][i].col = C->current.io.col[GMT_OUT][i].order = i;	/* Default order */
	for (i = 0; i < 2; i++) C->current.io.skip_if_NaN[i] = TRUE;								/* x/y must be non-NaN */
	for (i = 0; i < 2; i++) C->current.io.col_type[GMT_IN][i] = C->current.io.col_type[GMT_OUT][i] = GMT_IS_UNKNOWN;	/* Must be told [or find out] what x/y are */
	for (i = 2; i < GMT_MAX_COLUMNS; i++) C->current.io.col_type[GMT_IN][i] = C->current.io.col_type[GMT_OUT][i] = GMT_IS_FLOAT;	/* Other columns default to floats */
	C->current.io.io_n_header_items = C->current.setting.io_n_header_items;
	GMT_memcpy (C->current.io.io_header, C->current.setting.io_header, 2, GMT_LONG);
}

void GMT_lon_range_adjust (GMT_LONG range, double *lon)
{
	switch (range) {	/* Adjust to the desired range */
		case GMT_IS_0_TO_P360_RANGE:		/* Make 0 <= lon <= 360 */
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) > 360.0) (*lon) -= 360.0;
			break;
		case GMT_IS_0_TO_P360:		/* Make 0 <= lon < 360 */
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) >= 360.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M360_TO_0_RANGE:		/* Make -360 <= lon <= 0 */
			while ((*lon) < -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		case GMT_IS_M360_TO_0:		/* Make -360 < lon <= 0 */
			while ((*lon) <= -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P180_RANGE:	/* Make -180 <= lon <= +180 */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) > 180.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P180:	/* Make -180 <= lon < +180 [Special case where +180 is not desired] */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) >= 180.0) (*lon) -= 360.0;
			break;
		case GMT_IS_M180_TO_P270_RANGE:	/* Make -180 <= lon < +270 [Special case for GSHHS only] */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) >= 270.0) (*lon) -= 360.0;
			break;
	}
}

void GMT_quad_reset (struct GMT_CTRL *C, struct GMT_QUAD *Q, GMT_LONG n_items)
{	/* Allocate and initialize the QUAD struct needed to find min/max of a set of longitudes */
	GMT_LONG i;
	
	GMT_memset (Q, n_items, struct GMT_QUAD);	/* Set all to NULL/0 */
	for (i = 0; i < n_items; i++) {
		Q[i].min[0] = Q[i].min[1] = +DBL_MAX;
		Q[i].max[0] = Q[i].max[1] = -DBL_MAX;
		Q[i].range[0] = GMT_IS_M180_TO_P180_RANGE;
		Q[i].range[1] = GMT_IS_0_TO_P360_RANGE;
	}
}

struct GMT_QUAD * GMT_quad_init (struct GMT_CTRL *C, GMT_LONG n_items)
{	/* Allocate an initialize the QUAD struct needed to find min/max of longitudes */
	struct GMT_QUAD *Q = GMT_memory (C, NULL, n_items, struct GMT_QUAD);
	
	GMT_quad_reset (C, Q, n_items);
	
	return (Q);
}

void GMT_quad_add (struct GMT_CTRL *C, struct GMT_QUAD *Q, double x)
{	/* Update quad array for this longitude x */
	GMT_LONG way, quad_no;
	for (way = 0; way < 2; way++) {
		GMT_lon_range_adjust (Q->range[way], &x);	/* Set -180/180, then 0-360 range */
		Q->min[way] = MIN (x, Q->min[way]);
		Q->max[way] = MAX (x, Q->max[way]);
	}
	quad_no = (GMT_LONG)floor (x / 90.0);	/* Now x is 0-360; this yields quadrants 0-3 */
	if (quad_no == 4) quad_no = 0;		/* When x == 360.0 */
	Q->quad[quad_no] = TRUE;		/* OUr x fell in this quadrant */
}

GMT_LONG GMT_quad_finalize (struct GMT_CTRL *C, struct GMT_QUAD *Q)
{
	/* Finalize longitude range settings */
	GMT_LONG way, n_quad;
	
	n_quad = Q->quad[0] + Q->quad[1] + Q->quad[2] + Q->quad[3];		/* How many quadrants had data */
	if (Q->quad[0] && Q->quad[3])		/* Longitudes on either side of Greenwich only, must use -180/+180 notation */
		way = 0;
	else if (Q->quad[1] && Q->quad[2])	/* Longitudes on either side of the date line, must user 0/360 notation */
		way = 1;
	else if (n_quad == 2 && ((Q->quad[0] && Q->quad[2]) || (Q->quad[1] && Q->quad[3])))	/* Funny quadrant gap, pick shortest longitude extent */
		way = ((Q->max[0] - Q->min[0]) < (Q->max[1] - Q->min[1])) ? 0 : 1;
	else					/* Either will do, use default settings */
		way = (C->current.io.geo.range == GMT_IS_0_TO_P360_RANGE) ? 1 : 0;
	/* Final adjustments */
	if (Q->min[way] > Q->max[way]) Q->min[way] -= 360.0;
	if (Q->min[way] < 0.0 && Q->max[way] < 0.0) Q->min[way] += 360.0, Q->max[way] += 360.0;
	return (way);
}

void GMT_get_lon_minmax (struct GMT_CTRL *C, double *lon, GMT_LONG n, double *min, double *max)
{	/* Return the min/max longitude in array lon using clever quadrant checking. */
	GMT_LONG way, row;
	struct GMT_QUAD *Q = GMT_quad_init (C, 1);	/* Allocate and initialize one QUAD structure */

	/* We must keep separate min/max for both Dateline and Greenwich conventions */
	for (row = 0; row < n; row++) GMT_quad_add (C, Q, lon[row]);

	/* Finalize longitude range settings */
	way = GMT_quad_finalize (C, Q);
	*min = Q->min[way];		*max = Q->max[way];
	GMT_free (C, Q);
}

void GMT_set_seg_polar (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Must check if polygon is a polar cap */
	GMT_LONG row;
	double dlon, lon_sum = 0.0, lat_sum = 0.0;
	
	if (GMT_polygon_is_open (C, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows)) {
		GMT_report (C, GMT_MSG_FATAL, "Error: Cannot call GMT_set_seg_polar on an open polygon\n");
		return;
	}
	for (row = 0; row < S->n_rows - 1; row++) {
		dlon = S->coord[GMT_X][row+1] - S->coord[GMT_X][row];
		if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);	/* Crossed Greenwich or Dateline, pick the shortest distance */
		lon_sum += dlon;
		lat_sum += S->coord[GMT_Y][row];
	}
	if (GMT_360_RANGE (lon_sum, 0.0)) {	/* TRUE if contains a pole */
		S->pole = irint (copysign (1.0, lat_sum));	/* So, 0 means not polar */
		S->min[GMT_X] = 0.0;	S->max[GMT_X] = 360.0;
		if (S->pole == -1) S->min[GMT_Y] = -90.0;
		if (S->pole == +1) S->max[GMT_Y] = +90.0;
	}
}

GMT_LONG GMT_geo_to_dms (double val, GMT_LONG n_items, double fact, GMT_LONG *d, GMT_LONG *m,  GMT_LONG *s,  GMT_LONG *ix)
{
	/* Convert floating point degrees to dd:mm[:ss][.xxx].  Returns TRUE if d = 0 and val is negative */
	GMT_LONG minus, isec, imin;
	double sec, fsec, min, fmin, step;

	minus = (val < 0.0);
	step = (fact == 0.0) ? GMT_CONV_LIMIT : 0.5 / fact;  	/* Precision desired in seconds (or minutes); else just deal with roundoff */

	if (n_items == 3) {		/* Want dd:mm:ss[.xxx] format */
		sec = GMT_DEG2SEC_F * fabs (val) + step;	/* Convert to seconds */
		isec = (GMT_LONG)floor (sec);			/* Integer seconds */
		fsec = sec - (double)isec;  			/* Leftover fractional second */
		*d = isec / GMT_DEG2SEC_I;			/* Integer degrees */
		isec -= ((*d) * GMT_DEG2SEC_I);			/* Left-over seconds in the last degree */
		*m = isec / GMT_MIN2SEC_I;			/* Integer minutes */
		isec -= ((*m) * GMT_MIN2SEC_I);			/* Leftover seconds in the last minute */
		*s = isec;					/* Integer seconds */
		*ix = (GMT_LONG)floor (fsec * fact);		/* Fractional seconds scaled to integer */
	}
	else if (n_items == 2) {		/* Want dd:mm[.xxx] format */
		min = GMT_DEG2MIN_F * fabs (val) + step;	/* Convert to minutes */
		imin = (GMT_LONG)floor (min);			/* Integer minutes */
		fmin = min - (double)imin;  			/* Leftover fractional minute */
		*d = imin / GMT_DEG2MIN_I;			/* Integer degrees */
		imin -= ((*d) * GMT_DEG2MIN_I);			/* Left-over seconds in the last degree */
		*m = imin;					/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = (GMT_LONG)floor (fmin * fact);		/* Fractional minutes scaled to integer */
	}
	else {		/* Want dd[.xxx] format */
		min = fabs (val) + step;			/* Convert to degrees */
		imin = (GMT_LONG)floor (min);			/* Integer degrees */
		fmin = min - (double)imin;  			/* Leftover fractional degree */
		*d = imin;					/* Integer degrees */
		*m = 0;						/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = (GMT_LONG)floor (fmin * fact);		/* Fractional degrees scaled to integer */
	}
	if (minus) {	/* OK, change sign, but watch for *d = 0 */
		if (*d)	/* Non-zero degree term is easy */
			*d = -(*d);
		else	/* Cannot change 0 to -0, so pass flag back to calling function */
			return (TRUE);
	}
	return (FALSE);
}

void GMT_add_to_record (struct GMT_CTRL *C, char *record, double val, GMT_LONG col, GMT_LONG sep)
{	/* formats and appends val to the record texts string.
	 * If sep is 1 we prepend col separator.
	 * If sep is 2 we append col separator
	 * If sep is 1|2 do both [0 means no separator].
	 */
	char word[GMT_TEXT_LEN64];
	GMT_ascii_format_col (C, word, val, col);
	if (sep & 1) strcat (record, C->current.setting.io_col_separator);
	strcat (record, word);
	if (sep & 2) strcat (record, C->current.setting.io_col_separator);
}

void GMT_write_segmentheader (struct GMT_CTRL *C, FILE *fp, GMT_LONG n)
{
	/* Output ASCII or binary segment header.
	 * ASCII header is expected to contain newline (\n) */

	GMT_LONG i;
	
	if (!C->current.io.multi_segments[GMT_OUT]) return;	/* No output segments requested */
	if (C->common.b.active[GMT_OUT]) {			/* Binary native file uses all NaNs */
		for (i = 0; i < n; i++) C->current.io.output (C, fp, 1, &C->session.d_NaN);
		return;
	}
	/* Here we are doing ASCII */
	if (C->current.setting.io_blankline[GMT_OUT])	/* Write blank line to indicate segment break */
		fprintf (fp, "\n");
	else if (C->current.setting.io_nanline[GMT_OUT]) {	/* Write NaN record to indicate segment break */
		for (i = 1 ; i < MIN (2,n); i++) fprintf (fp, "NaN%s", C->current.setting.io_col_separator);
		fprintf (fp, "NaN\n");
	}
	else if (!C->current.io.segment_header[0])		/* No header; perhaps via binary input with NaN-headers */
		fprintf (fp, "%c\n", C->current.setting.io_seg_marker[GMT_OUT]);
	else
		fprintf (fp, "%c %s\n", C->current.setting.io_seg_marker[GMT_OUT], C->current.io.segment_header);
}

void GMT_io_binary_header (struct GMT_CTRL *C, FILE *fp, GMT_LONG dir)
{
	GMT_LONG k;
	char c = ' ';
	if (dir == GMT_IN) {	/* Use fread since we dont know if input is a stream or a file */
		for (k = 0; k < C->current.io.io_n_header_items; k++) GMT_fread (&c, sizeof (char), (size_t)1, fp);
	}
	else {
		for (k = 0; k < C->current.io.io_n_header_items; k++) GMT_fwrite (&c, sizeof (char), (size_t)1, fp);
	}
}

void GMT_write_tableheader (struct GMT_CTRL *C, FILE *fp, char *txt)
{
	/* Output ASCII segment header; skip if mode is binary.
	 * We append a newline (\n) if not is present */

	if (!C->current.io.io_header[GMT_OUT]) return;	/* No output headers requested */
	if (GMT_binary_header (C, GMT_OUT))		/* Must write a binary header */
		GMT_io_binary_header (C, fp, GMT_OUT);
	else if (!txt || !txt[0])				/* Blank header */
		fprintf (fp, "#\n");
	else {
		if (txt[0] != '#') fputc ('#', fp);	/* Make sure we have # at start */
		fprintf (fp, "%s", txt);
		if (txt[strlen(txt)-1] != '\n') fputc ('\n', fp);	/* Make sure we have \n at end */
	}
}

void GMT_write_textrecord (struct GMT_CTRL *C, FILE *fp, char *txt)
{
	/* Output ASCII segment header; skip if mode is binary.
	 * We append a newline (\n) if not is present */

	if (C->common.b.active[GMT_OUT]) return;		/* Cannot write text records if binary output */
	if (!txt || !txt[0]) return;				/* Skip blank lines */
	fprintf (fp, "%s", txt);				/* May or may not have \n at end */
	if (txt[strlen(txt)-1] != '\n') fputc ('\n', fp);	/* Make sure we have \n at end */
}

/* Various functions to support {grd2xyz,xyz2grd}_func.c */

/* NOTE: In the following we check C->current.io.col_type[GMT_IN][2] and C->current.io.col_type[GMT_OUT][2] for formatting help for the first column.
 * We use column 3 ([2] or GMT_Z) instead of the first ([0]) since we really are dealing with the z in z (x,y) here
 * and the x,y are implicit from the -R -I arguments.
 */

GMT_LONG gmt_A_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{	/* Can read one or more items from input records. Limitation is that they must be floating point values (no dates or ddd:mm:ss) */
	GMT_LONG i, k;
	for (i = 0; i < *n; i++) if ((k = fscanf (fp, "%lg", &d[i])) <= 0) return (-1);	/* Read was unsuccessful */
	return (1);
}

GMT_LONG gmt_a_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{	/* Only reads one item regardless of *n */
	GMT_LONG i;
	char line[GMT_TEXT_LEN64];
	*n = 0;
	if (!fgets (line, GMT_TEXT_LEN64, fp)) {	/* Read was unsuccessful */
		C->current.io.status = GMT_IO_EOF;
		return (-1);
	}
	for (i = strlen (line) - 1; i >= 0 && strchr (" \t,\r\n", (int)line[i]); i--);	/* Take out trailing whitespace */
	line[++i] = '\0';
	GMT_scanf (C, line, C->current.io.col_type[GMT_IN][GMT_Z], d);	/* Convert whatever it is to double */
	*n = 1;
	return (1);
}

GMT_LONG gmt_c_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	char c;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&c, sizeof (char), (size_t)1, fp)) {	/* Read was unsuccessful */
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) c;
	}
	return (1);
}

GMT_LONG gmt_u_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned char u;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&u, sizeof (unsigned char), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) u;
	}
	return (1);
}

GMT_LONG gmt_h_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	short int h;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&h, sizeof (short int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) h;
	}
	return (1);
}

GMT_LONG gmt_h_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	short int h;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&h, sizeof (short int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (short int) GMT_swab2 (h);
	}
	return (1);
}

GMT_LONG gmt_H_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned short int h;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&h, sizeof (unsigned short int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) h;
	}
	return (1);
}

GMT_LONG gmt_H_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned short int h;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&h, sizeof (unsigned short int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (unsigned short int) GMT_swab2 (h);
	}
	return (1);
}

GMT_LONG gmt_i_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	int i4;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&i4, sizeof (int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) i4;
	}
	return (1);
}

GMT_LONG gmt_i_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	int i4;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&i4, sizeof (int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (int) GMT_swab4 (i4);
	}
	return (1);
}

GMT_LONG gmt_I_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned int i4;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&i4, sizeof (unsigned int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) i4;
	}
	return (1);
}

GMT_LONG gmt_I_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned int i4;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&i4, sizeof (unsigned int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (unsigned int) GMT_swab4 (i4);
	}
	return (1);
}

GMT_LONG gmt_l_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	long int l;

	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&l, sizeof (long int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) l;
	}
	return (1);
}

GMT_LONG gmt_l_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	long int l;
	unsigned int *i4, k;
	void *vptr = &l;

	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&l, sizeof (long int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		i4 = (unsigned int *)vptr;
		for (k = 0; k < 2; k++) i4[k] = GMT_swab4 (i4[k]);
		d[i] = (double) l;
	}
	return (1);
}

GMT_LONG gmt_L_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned long int l;

	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&l, sizeof (unsigned long int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) l;
	}
	return (1);
}

GMT_LONG gmt_L_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned long int l;
	unsigned int *i4, k;
	void *vptr = &l;

	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&l, sizeof (unsigned long int), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		i4 = (unsigned int *)vptr;
		for (k = 0; k < 2; k++) i4[k] = GMT_swab4 (i4[k]);
		d[i] = (double) l;
	}
	return (1);
}

GMT_LONG gmt_f_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	float f;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&f, sizeof (float), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		d[i] = (double) f;
	}
	return (1);
}

GMT_LONG gmt_f_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	float f;
	unsigned int *i4;
	void *vptr = &f;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&f, sizeof (float), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		i4 = (unsigned int *)vptr;
		*i4 = GMT_swab4 (*i4);
		d[i] = (double) f;
	}
	return (1);
}

GMT_LONG gmt_d_read (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&d[i], sizeof (double), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
	}
	return (1);
}

GMT_LONG gmt_d_read_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double *d)
{
	GMT_LONG i;
	unsigned int j4, *i4;
	for (i = 0; i < *n; i++) {
		if (!GMT_fread (&d[i], sizeof (double), (size_t)1, fp)) {
			C->current.io.status = GMT_IO_EOF;
			return (-1);
		}
		i4 = (unsigned int *)&d[i];
		j4 = GMT_swab4 (i4[0]);
		i4[0] = GMT_swab4 (i4[1]);
		i4[1] = j4;
	}
	return (1);
}

GMT_LONG gmt_a_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{
	GMT_LONG i, n1 = n - 1;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		GMT_ascii_output_col (C, fp, d[i], GMT_Z);
		if (i < n1) fprintf (fp, "\t");
	}
	fprintf (fp, "\n");
	return (n);
}

GMT_LONG gmt_c_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{
	GMT_LONG i;
	char c;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		c = (char) d[i];
		if (GMT_fwrite (&c, sizeof (char), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_u_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{
	GMT_LONG i;
	unsigned char u;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		u = (unsigned char) d[i];
		if (GMT_fwrite (&u, sizeof (unsigned char), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_h_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{
	GMT_LONG i;
	short int h;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		h = (short int) d[i];
		if (GMT_fwrite (&h, sizeof (short int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_h_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{
	GMT_LONG i;
	short int h;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		h = GMT_swab2 ((short int) d[i]);
		if (GMT_fwrite (&h, sizeof (short int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_H_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned short int h;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		h = (unsigned short int) d[i];
		if (GMT_fwrite (&h, sizeof (unsigned short int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_H_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned short int h;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		h = GMT_swab2 ((unsigned short int) d[i]);
		if (GMT_fwrite (&h, sizeof (unsigned short int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_i_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	int i4;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		i4 = (int) d[i];
		if (GMT_fwrite (&i4, sizeof (int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_i_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	int i4;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		i4 = GMT_swab4 ((int) d[i]);
		if (GMT_fwrite (&i4, sizeof (int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_I_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned int i4;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		i4 = (unsigned int) d[i];
		if (GMT_fwrite (&i4, sizeof (unsigned int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_I_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned int i4;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		i4 = GMT_swab4 ((unsigned int) d[i]);
		if (GMT_fwrite (&i4, sizeof (unsigned int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_l_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	long int l;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		l = (long int) d[i];
		if (GMT_fwrite (&l, sizeof (long int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_l_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	long int l;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		l = GMT_swab4 ((long int) d[i]);
		if (GMT_fwrite (&l, sizeof (long int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_L_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned long int l;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		l = (unsigned long int) d[i];
		if (GMT_fwrite (&l, sizeof (unsigned long int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_L_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned long int l;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		l = GMT_swab4 ((unsigned long int) d[i]);
		if (GMT_fwrite (&l, sizeof (unsigned long int), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_f_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		float f;
		f = (float) d[i];
		if (GMT_fwrite (&f, sizeof (float), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_f_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	float f;
	void *vptr = &f;
	unsigned int *i4 = (unsigned int *)vptr;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		f = (float) d[i];
		f = (float) GMT_swab4 (*i4);
		if (GMT_fwrite (&f, sizeof (float), (size_t)1, fp) != 1) return (GMT_DATA_WRITE_ERROR);
	}
	return (n);
}

GMT_LONG gmt_d_write (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	return (GMT_fwrite (d, sizeof (double), (size_t)n, fp));
}

GMT_LONG gmt_d_write_swab (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, double *d)
{	/* Input n is ignored since it is always 1 */
	GMT_LONG i;
	unsigned int j4, *i4;
	if (gmt_skip_output (C, d, n)) return (0);	/* Record was skipped via -s[r] */
	for (i = 0; i < n; i++) {
		i4 = (unsigned int *)&d[i];
		j4 = GMT_swab4 (i4[0]);
		i4[0] = GMT_swab4 (i4[1]);
		i4[1] = j4;
		if (!GMT_fwrite (&d[i], sizeof (double), (size_t)1, fp))  return (GMT_DATA_WRITE_ERROR);
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_col_ij (struct GMT_Z_IO *r, struct GMT_GRID *G, GMT_LONG ij)
{
	/* Translates incoming ij (no padding) to gmt_ij (includes padding) for column-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij % r->y_period);
	r->gmt_i = r->start_col + r->x_step * (ij / r->y_period);

	return (GMT_IJP (G->header, r->gmt_j, r->gmt_i));
}

GMT_LONG gmt_row_ij (struct GMT_Z_IO *r, struct GMT_GRID *G, GMT_LONG ij)
{
	/* Translates incoming ij (no padding) to gmt_ij (includes padding) for row-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij / r->x_period);
	r->gmt_i = r->start_col + r->x_step * (ij % r->x_period);

	return (GMT_IJP (G->header, r->gmt_j, r->gmt_i));
}

GMT_LONG GMT_parse_z_io (struct GMT_CTRL *C, char *txt, struct GMT_PARSE_Z_IO *z)
{
	GMT_LONG i, k = 0;

	if (!txt) return (EXIT_FAILURE);	/* Must give a non-NULL argument */
	if (!txt[0]) return (0);		/* Default -ZTLa */

	for (i = 0; txt[i] && i < 2; i++) {	/* Loop over the first 2 flags */

		switch (txt[i]) {

			/* These 4 cases will set the format orientation for input */

			case 'T':
			case 'B':
			case 'L':
			case 'R':
				if (k > 2) {
					GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: Choose format from [TBLR][TBLR]!\n");
					return (EXIT_FAILURE);
				}
				z->format[k++] = txt[i];
				break;
			default:
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: Must begin with [TBLR][TBLR]!\n");
				return (EXIT_FAILURE);
				break;
		}
	}
	
	for (i = 2; txt[i]; i++) {	/* Loop over flags */

		switch (txt[i]) {

			/* Set this if file is periodic, is grid registered, but repeating column or row is missing from input */

			case 'x':
				z->repeat[GMT_X] = 1;	break;
			case 'y':
				z->repeat[GMT_Y] = 1;	break;

			/* Optionally skip the given number of bytes before reading data */

			case 's':
				i++;
				if (txt[i]) {	/* Read the byte count for skipping */
					z->skip = atoi (&txt[i]);
					while (txt[i] && isdigit ((int)txt[i])) i++;
					i--;
				}
				break;

			case 'w':
				z->swab = TRUE; 	break;

			/* Set read pointer depending on data format */

			case 'l':	/* Binary 8-byte integer, 64-bit mode only */
			case 'L':	/* Binary 8-byte integer, 64-bit mode only */
				if (sizeof (GMT_LONG) == 4) {
					GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: Cannot specify %c in 32-bit mode\n", (int)txt[i]);
					return (EXIT_FAILURE);
				}
			case 'A':	/* ASCII (next regular float (%lg) from the stream) */
			case 'a':	/* ASCII (1 per record) */
			case 'c':	/* Binary signed char */
			case 'u':	/* Binary unsigned char */
			case 'h':	/* Binary signed 2-byte integer */
			case 'H':	/* Binary unsigned 2-byte integer */
			case 'i':	/* Binary signed 4-byte integer */
			case 'I':	/* Binary unsigned 4-byte integer */
			case 'f':	/* Binary 4-byte float */
			case 'd':	/* Binary 8-byte double */
				z->type = txt[i];
				break;

			default:
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: %c not a valid modifier!\n", txt[i]);
				return (EXIT_FAILURE);
				break;
		}
	}

	return (0);
}

GMT_LONG GMT_get_io_type (struct GMT_CTRL *C, char type)
{
	GMT_LONG t = -1;
	switch (type) {	/* Set read pointer depending on data format */
		case 'a': case 'A': t = -1; break;		/* ASCII */
		case 'c': t = GMTAPI_CHAR; break;	/* Binary signed char */
		case 'u': t = GMTAPI_UCHAR; break;	/* Binary unsigned char */
		case 'h': t = GMTAPI_SHORT; break;	/* Binary 2-byte signed integer */
		case 'H': t = GMTAPI_USHORT; break;	/* Binary 2-byte unsigned integer */
		case 'i': t = GMTAPI_INT; break;	/* Binary 4-byte signed integer */
		case 'I': t = GMTAPI_UINT; break;	/* Binary 4-byte unsigned integer */
		case 'l': t = GMTAPI_LONG; break;	/* Binary 8-byte signed integer, 64-bit mode only */
		case 'L': t = GMTAPI_ULONG; break;	/* Binary 8-byte unsigned integer, 64-bit mode only */
		case 'f': t = GMTAPI_FLOAT; break;		/* Binary 4-byte float */
		case 'd': t = GMTAPI_DOUBLE; break;		/* Binary 8-byte double */
		default:
			GMT_report (C, GMT_MSG_FATAL, "%c not a valid data type!\n", type);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	return (t);
}

PFL GMT_get_io_ptr (struct GMT_CTRL *C, GMT_LONG direction, GMT_LONG swap, char type)
{	/* Return pointer to read or write function for this data type */
	PFL p = NULL;
	
	switch (type) {	/* Set read pointer depending on data format */
		case 'A':	/* ASCII with more than one per record */
			p = (direction == GMT_IN) ? (PFL)gmt_A_read : (PFL)gmt_a_write;	break;
		case 'a':	/* ASCII */
			p = (direction == GMT_IN) ? (PFL)gmt_a_read : (PFL)gmt_a_write;	break;
		case 'c':	/* Binary signed char */
			p = (direction == GMT_IN) ? (PFL)gmt_c_read : (PFL)gmt_c_write;	break;
		case 'u':	/* Binary unsigned char */
			p = (direction == GMT_IN) ? (PFL)gmt_u_read : (PFL)gmt_u_write;	break;
		case 'h':	/* Binary short 2-byte integer */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_h_read_swab : (PFL)gmt_h_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_h_read : (PFL)gmt_h_write;
			break;
		case 'H':	/* Binary unsigned short 2-byte integer */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_H_read_swab : (PFL)gmt_H_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_H_read : (PFL)gmt_H_write;
			break;
		case 'i':	/* Binary 4-byte integer */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_i_read_swab : (PFL)gmt_i_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_i_read : (PFL)gmt_i_write;
			break;
		case 'I':	/* Binary 4-byte unsigned integer */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_I_read_swab : (PFL)gmt_I_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_I_read : (PFL)gmt_I_write;
			break;
		case 'l':	/* Binary 8-byte integer, 64-bit mode only */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_l_read_swab : (PFL)gmt_l_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_l_read : (PFL)gmt_l_write;
			break;
		case 'L':	/* Binary 8-byte unsigned integer, 64-bit mode only */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_L_read_swab : (PFL)gmt_L_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_L_read : (PFL)gmt_L_write;
			break;
		case 'f':	/* Binary 4-byte float */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_f_read_swab : (PFL)gmt_f_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_f_read : (PFL)gmt_f_write;
			break;
		case 'd':	/* Binary 8-byte double */
			if (swap)
				p = (direction == GMT_IN) ? (PFL)gmt_d_read_swab : (PFL)gmt_d_write_swab;
			else
				p = (direction == GMT_IN) ? (PFL)gmt_d_read : (PFL)gmt_d_write;
			break;
		case 'x': break;	/* Binary skip */

		default:
			GMT_report (C, GMT_MSG_FATAL, "%c not a valid data type!\n", type);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	return (p);
}

GMT_LONG GMT_init_z_io (struct GMT_CTRL *C, char format[], GMT_LONG repeat[], GMT_LONG swab, GMT_LONG skip, char type, struct GMT_Z_IO *r)
{
	GMT_LONG first = TRUE, k;

	GMT_memset (r, 1, struct GMT_Z_IO);

	for (k = 0; k < 2; k++) {	/* Loop over the two format flags */
		switch (format[k]) {
			/* These 4 cases will set the format orientation for input */
			case 'T':
				if (first) r->format = GMT_ROW_FORMAT;
				r->y_step = 1;	first = FALSE;	break;
			case 'B':
				if (first) r->format = GMT_ROW_FORMAT;
				r->y_step = -1;	first = FALSE;	break;
			case 'L':
				if (first) r->format = GMT_COLUMN_FORMAT;
				r->x_step = 1;	first = FALSE;	break;
			case 'R':
				if (first) r->format = GMT_COLUMN_FORMAT;
				r->x_step = -1;	first = FALSE;	break;
			default:
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: %c not a valid format specifier!\n", format[k]);
				GMT_exit (EXIT_FAILURE);
				break;
		}
	}

	if (!strchr ("AacuhHiIlLfd", type)) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error -Z: %c not a valid data type!\n", type);
		GMT_exit (EXIT_FAILURE);
		
	}

	r->x_missing = repeat[GMT_X];	r->y_missing = repeat[GMT_Y];
	r->skip = skip;			r->swab = swab;
	r->binary = (strchr ("Aa", type)) ? FALSE : TRUE;
	C->current.io.read_item  = GMT_get_io_ptr (C, GMT_IN,  swab, type);	/* Set read pointer depending on data format */
	C->current.io.write_item = GMT_get_io_ptr (C, GMT_OUT, swab, type);	/* Set write pointer depending on data format */
	
	if (r->binary) {	/* Use the binary modes (which only matters under Windoze)  */
		strcpy (C->current.io.r_mode, "rb");
		strcpy (C->current.io.w_mode, "wb");
		strcpy (C->current.io.a_mode, "ab+");
	}

	return (GMT_NOERROR);
}

/* GMT_z_input and GMT_z_output are used in grd2xyz/xyz2grd to fascilitate reading of one-col items via the general i/o machinery */
void * GMT_z_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, GMT_LONG *status)
{
	if ((*status = C->current.io.read_item (C, fp, n, C->current.io.curr_rec)) == -1) return (NULL);
	return (C->current.io.curr_rec);
}
	
GMT_LONG GMT_z_output (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, void *data)
{
	double *ptr = data;
	return (C->current.io.write_item (C, fp, n, ptr));
}
	
GMT_LONG GMT_set_z_io (struct GMT_CTRL *C, struct GMT_Z_IO *r, struct GMT_GRID *G)
{
	/* THIS SHOULD NOT BE FATAL!
	if ((r->x_missing || r->y_missing) && G->header->registration == GMT_PIXEL_REG) return (GMT_GRDIO_RI_NOREPEAT);
	*/
	r->start_col = ((r->x_step == 1) ? 0 : G->header->nx - 1 - r->x_missing);
	r->start_row = ((r->y_step == 1) ? r->y_missing : G->header->ny - 1);
	r->get_gmt_ij = (r->format == GMT_COLUMN_FORMAT) ? gmt_col_ij : gmt_row_ij;
	r->x_period = G->header->nx - r->x_missing;
	r->y_period = G->header->ny - r->y_missing;
	r->n_expected = r->x_period * r->y_period;
	return (GMT_NOERROR);
}

void GMT_check_z_io (struct GMT_CTRL *C, struct GMT_Z_IO *r, struct GMT_GRID *G)
{
	/* Routine to fill in the implied periodic row or column that was missing.
	 * We must allow for padding in G->data */

	GMT_LONG col, row, k, k_top, k_bot;

	if (r->x_missing) for (row = 0, k = GMT_IJP (G->header, row, 0); row < G->header->ny; row++, k += G->header->mx) G->data[k+G->header->nx-1] = G->data[k];
	if (r->y_missing) for (col = 0, k_top = GMT_IJP (G->header, 0, 0), k_bot = GMT_IJP (G->header, G->header->ny-1, 0); col < G->header->nx; col++) G->data[col+k_top] = G->data[col+k_bot];
}

GMT_LONG gmt_get_ymdj_order (struct GMT_CTRL *C, char *text, struct GMT_DATE_IO *S)
{	/* Reads a YYYY-MM-DD or YYYYMMDD-like string and determines order.
	 * order[0] is the order of the year, [1] is month, etc.
	 * Items not encountered are left as -1.
	 */

	GMT_LONG i, j, order, n_y, n_m, n_d, n_j, n_w, n_delim, last, error = 0;

	GMT_memset (S, 1, struct GMT_DATE_IO);
	for (i = 0; i < 4; i++) S->item_order[i] = S->item_pos[i] = -1;	/* Meaning not encountered yet */

	n_y = n_m = n_d = n_j = n_w = n_delim = 0;

	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = TRUE;
		i++;
	}
	for (order = 0; i < (GMT_LONG)strlen (text); i++) {
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
			case 'w':	/* ISO Week */
				if (S->item_pos[1] < 0) {		/* First time we encounter a w */
					S->item_pos[1] = order++;
					if (text[i-1] != 'W') error++;	/* Must have the format W just before */
				}
				else if (text[i-1] != 'w')	/* Done it before, previous char must be w */
					error++;
				n_w++;
				break;
			case 'u':	/* ISO Week name ("Week 04") (plot output only) */
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
		GMT_report (C, GMT_MSG_FATAL, "Error: Unacceptable date template %s\n", text);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_get_hms_order (struct GMT_CTRL *C, char *text, struct GMT_CLOCK_IO *S)
{	/* Reads a HH:MM:SS or HHMMSS-like string and determines order.
	 * hms_order[0] is the order of the hour, [1] is min, etc.
	 * Items not encountered are left as -1.
	 */

	GMT_LONG i, j, order, n_delim, sequence[3], last, n_h, n_m, n_s, n_x, n_dec, error = 0, big_to_small;
	char *p = NULL;
	ptrdiff_t off;

	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */
	sequence[0] = sequence[1] = sequence[2] = -1;

	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	n_h = n_m = n_s = n_x = n_dec = n_delim = 0;

	/* Determine if we do 12-hour clock (and what form of am/pm suffix) or 24-hour clock */

	if ((p = strstr (text, "am"))) {	/* Want 12 hour clock with am/pm */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "am");
		strcpy (S->ampm_suffix[1], "pm");
		off = p - text;
	}
	else if ((p = strstr (text, "AM"))) {	/* Want 12 hour clock with AM/PM */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "AM");
		strcpy (S->ampm_suffix[1], "PM");
		off = p - text;
	}
	else if ((p = strstr (text, "a.m."))) {	/* Want 12 hour clock with a.m./p.m. */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "a.m.");
		strcpy (S->ampm_suffix[1], "p.m.");
		off = p - text;
	}
	else if ((p = strstr (text, "A.M."))) {	/* Want 12 hour clock with A.M./P.M. */
		S->twelve_hr_clock = TRUE;
		strcpy (S->ampm_suffix[0], "A.M.");
		strcpy (S->ampm_suffix[1], "P.M.");
		off = p - text;
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
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Unacceptable clock template %s\n", text);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG gmt_get_dms_order (struct GMT_CTRL *C, char *text, struct GMT_GEO_IO *S)
{	/* Reads a ddd:mm:ss-like string and determines order.
	 * order[0] is the order of the degree, [1] is minutes, etc.
	 * Order is checked since we only allow d, m, s in that order.
	 * Items not encountered are left as -1.
	 */

	GMT_LONG i1, i, j, order, n_d, n_m, n_s, n_x, n_dec, sequence[3], n_delim, last, error = 0, big_to_small;

	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */

	n_d = n_m = n_s = n_x = n_dec = n_delim = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	sequence[0] = sequence[1] = sequence[2] = -1;

	S->range = GMT_IS_M180_TO_P180_RANGE;			/* -180/+180 range, may be overwritten below by + or - */
	S->decimal = S->wesn = S->no_sign = FALSE;

	i1 = strlen (text) - 1;
	for (i = order = 0; i <= i1; i++) {
		switch (text[i]) {
			case '+':	/* Want [0-360 range [Default] */
				S->range = GMT_IS_0_TO_P360_RANGE;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case '-':	/* Want [-360-0] range [i.e., western longitudes] */
				S->range = GMT_IS_M360_TO_0_RANGE;
				if (i != 0) error++;		/* Only valid as first flag */
				break;
			case 'D':	/* Want to use decimal degrees using D_FORMAT [Default] */
				S->decimal = TRUE;
				if (i > 1) error++;		/* Only valid as first or second flag */
				break;
			case 'F':	/* Want to use WESN to encode sign */
				S->wesn = 1;
				if (i != i1 || S->no_sign) error++;		/* Only valid as last flag */
				break;
			case 'G':	/* Want to use WESN to encode sign but have leading space */
				S->wesn = 2;
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

	if (S->decimal) return (GMT_NOERROR);	/* Easy formatting choice */

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
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Unacceptable dmmss template %s\n", text);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

void gmt_clock_C_format (struct GMT_CTRL *C, char *form, struct GMT_CLOCK_IO *S, GMT_LONG mode)
{
	/* Determine the order of H, M, S in input and output clock strings,
	 * as well as the number of decimals in output seconds (if any), and
	 * if a 12- or 24-hour clock is used.
	 * mode is 0 for input and 1 for output format
	 */

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	gmt_get_hms_order (C, form, S);

	/* Craft the actual C-format to use for input/output clock strings */

	if (S->order[0] >= 0) {	/* OK, at least hours is needed */
		char fmt[GMT_TEXT_LEN256];
		if (S->compact)
			sprintf (S->format, "%%" GMT_LL "d");
		else
			(mode) ? sprintf (S->format, "%%2.2" GMT_LL "d") : sprintf (S->format, "%%2" GMT_LL "d");
		if (S->order[1] >= 0) {	/* Need minutes too*/
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			(mode) ? sprintf (fmt, "%%2.2" GMT_LL "d") : sprintf (fmt, "%%2" GMT_LL "d");
			strcat (S->format, fmt);
			if (S->order[2] >= 0) {	/* .. and seconds */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				if (mode) {	/* Output format */
					sprintf (fmt, "%%2.2" GMT_LL "d");
					strcat (S->format, fmt);
					if (S->n_sec_decimals) {	/* even add format for fractions of second */
						sprintf (fmt, ".%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
						strcat (S->format, fmt);
					}
				}
				else {		/* Input format */
					sprintf (fmt, "%%lf");
					strcat (S->format, fmt);
				}
			}
		}
		if (mode && S->twelve_hr_clock) {	/* Finally add %s for the am, pm string */
			sprintf (fmt, "%%s");
			strcat (S->format, fmt);
		}
	}
}

void gmt_date_C_format (struct GMT_CTRL *C, char *form, struct GMT_DATE_IO *S, GMT_LONG mode)
{
	/* Determine the order of Y, M, D, J in input and output date strings.
	* mode is 0 for input, 1 for output, and 2 for plot output.
	 */

	GMT_LONG k, ywidth, no_delim;
	char fmt[GMT_TEXT_LEN256];

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	gmt_get_ymdj_order (C, form, S);

	/* Craft the actual C-format to use for i/o date strings */

	no_delim = !(S->delimiter[0][0] || S->delimiter[1][0]);	/* TRUE for things like yyyymmdd */
	ywidth = (no_delim) ? 4 : 4+(!mode);			/* 4 or 5, depending on values */
	if (S->item_order[0] >= 0 && S->iso_calendar) {	/* ISO Calendar string: At least one item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? ywidth : 2;
		if (S->mw_text && S->item_order[0] == 1)	/* Prepare for "Week ##" format */
			sprintf (S->format, "%%s %%2.2" GMT_LL "d");
		else if (S->compact)			/* Numerical formatting of week or year without leading zeros */
			sprintf (S->format, "%%" GMT_LL "d");
		else					/* Numerical formatting of week or year  */
			(mode) ? sprintf (S->format, "%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", k, k) : sprintf (S->format, "%%%" GMT_LL "d" GMT_LL "d", k);
		if (S->item_order[1] >= 0) {	/* Need another item */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			if (S->mw_text && S->item_order[0] == 1) {	/* Prepare for "Week ##" format */
				sprintf (fmt, "%%s ");
				strcat (S->format, fmt);
			}
			else
				strcat (S->format, "W");
			if (S->compact)
				sprintf (fmt, "%%" GMT_LL "d");
			else
				(mode) ? sprintf (fmt, "%%2.2" GMT_LL "d") : sprintf (fmt, "%%2" GMT_LL "d");
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* and ISO day of week */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				sprintf (fmt, "%%1" GMT_LL "d");
				strcat (S->format, fmt);
			}
		}
	}
	else if (S->item_order[0] >= 0) {			/* Gregorian Calendar string: At least one item is needed */
		k = (S->item_order[0] == 0 && !S->Y2K_year) ? ywidth : 2;
		if (S->item_order[0] == 3) k = 3;	/* Day of year */
		if (S->mw_text && S->item_order[0] == 1)	/* Prepare for "Monthname" format */
			(mode == 0) ? sprintf (S->format, "%%[^%s]", S->delimiter[0]) : sprintf (S->format, "%%s");
		else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
			sprintf (S->format, "%%" GMT_LL "d");
		else					/* Numerical formatting of month or year */
			(mode) ? sprintf (S->format, "%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", k, k) : sprintf (S->format, "%%%" GMT_LL "d" GMT_LL "d", k);
		if (S->item_order[1] >= 0) {	/* Need more items */
			if (S->delimiter[0][0]) strcat (S->format, S->delimiter[0]);
			k = (S->item_order[1] == 0 && !S->Y2K_year) ? ywidth : 2;
			if (S->item_order[1] == 3) k = 3;	/* Day of year */
			if (S->mw_text && S->item_order[1] == 1)	/* Prepare for "Monthname" format */
				(mode == 0) ? sprintf (fmt, "%%[^%s]", S->delimiter[1]) : sprintf (fmt, "%%s");
			else if (S->compact && !S->Y2K_year)		/* Numerical formatting of month or 4-digit year w/o leading zeros */
				sprintf (fmt, "%%" GMT_LL "d");
			else
				(mode) ? sprintf (fmt, "%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", k, k) : sprintf (fmt, "%%%" GMT_LL "d" GMT_LL "d", k);
			strcat (S->format, fmt);
			if (S->item_order[2] >= 0) {	/* .. and even more */
				if (S->delimiter[1][0]) strcat (S->format, S->delimiter[1]);
				k = (S->item_order[2] == 0 && !S->Y2K_year) ? ywidth : 2;
				if (S->mw_text && S->item_order[2] == 1)	/* Prepare for "Monthname" format */
					sprintf (fmt, "%%s");
				else if (S->compact)			/* Numerical formatting of month or year w/o leading zeros */
					sprintf (fmt, "%%" GMT_LL "d");
				else
					(mode) ? sprintf (fmt, "%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", k, k) : sprintf (fmt, "%%%" GMT_LL "d" GMT_LL "d", k);
				strcat (S->format, fmt);
			}
		}
	}
}

GMT_LONG gmt_geo_C_format (struct GMT_CTRL *C)
{
	/* Determine the output of geographic location formats. */

	struct GMT_GEO_IO *S = &C->current.io.geo;

	gmt_get_dms_order (C, C->current.setting.format_geo_out, S);	/* Get the order of degree, min, sec in output formats */

	if (S->no_sign) return (GMT_IO_BAD_PLOT_DEGREE_FORMAT);

	if (S->decimal) {	/* Plain decimal degrees */
		 /* here we depend on FORMAT_FLOAT_OUT begin set.  This will not be true when FORMAT_GEO_MAP is parsed but will be
		  * handled at the end of GMT_begin.  For gmtset and --PAR later we will be OK as well. */
		if (!C->current.setting.format_float_out[0]) return (GMT_NOERROR); /* Quietly return and deal with this later in GMT_begin */
		sprintf (S->x_format, "%s", C->current.setting.format_float_out);
		sprintf (S->y_format, "%s", C->current.setting.format_float_out);
	}
	else {			/* Some form of dd:mm:ss */
		char fmt[GMT_TEXT_LEN256];
		sprintf (S->x_format, "%%3.3" GMT_LL "d");
		sprintf (S->y_format, "%%2.2" GMT_LL "d");
		if (S->order[1] >= 0) {	/* Need minutes too */
			strcat (S->x_format, S->delimiter[0]);
			strcat (S->y_format, S->delimiter[0]);
			sprintf (fmt, "%%2.2" GMT_LL "d");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->order[2] >= 0) {	/* .. and seconds */
			strcat (S->x_format, S->delimiter[1]);
			strcat (S->y_format, S->delimiter[1]);
			sprintf (fmt, "%%2.2" GMT_LL "d");
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		if (S->n_sec_decimals) {	/* even add format for fractions of second (or minutes or degrees) */
			sprintf (fmt, ".%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
			strcat (S->x_format, fmt);
			strcat (S->y_format, fmt);
		}
		/* Finally add %c for the W,E,S,N char (or NULL) */
		sprintf (fmt, "%%s");
		strcat (S->x_format, fmt);
		strcat (S->y_format, fmt);
	}
	return (GMT_NOERROR);
}

void gmt_plot_C_format (struct GMT_CTRL *C)
{
	GMT_LONG i, j;
	struct GMT_GEO_IO *S = &C->current.plot.calclock.geo;

	/* Determine the plot geographic location formats. */

	for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) GMT_memset (C->current.plot.format[i][j], GMT_TEXT_LEN256, char);

	gmt_get_dms_order (C, C->current.setting.format_geo_map, S);	/* Get the order of degree, min, sec in output formats */

	if (S->decimal) {	/* Plain decimal degrees */
		GMT_LONG len;
		 /* here we depend on FORMAT_FLOAT_OUT begin set.  This will not be true when FORMAT_GEO_MAP is parsed but will be
		  * handled at the end of GMT_begin.  For gmtset and --PAR later we will be OK as well. */
		if (!C->current.setting.format_float_out[0]) return; /* Quietly return and deal with this later in GMT_begin */

		len = sprintf (S->x_format, "%s", C->current.setting.format_float_out);
		      sprintf (S->y_format, "%s", C->current.setting.format_float_out);
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			S->x_format[len] = (char)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol];
			S->y_format[len] = (char)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol];
			S->x_format[len+1] = S->y_format[len+1] = '\0';
		}
		strcat (S->x_format, "%s");
		strcat (S->y_format, "%s");
	}
	else {			/* Must cover all the 6 forms of dd[:mm[:ss]][.xxx] */
		char fmt[GMT_TEXT_LEN256];

		/* Level 0: degrees only. index 0 is integer degrees, index 1 is [possibly] fractional degrees */

		sprintf (C->current.plot.format[0][0], "%%d");		/* ddd */
		if (S->order[1] == -1 && S->n_sec_decimals > 0) /* ddd.xxx format */
			sprintf (C->current.plot.format[0][1], "%%" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd format */
			sprintf (C->current.plot.format[0][1], "%%" GMT_LL "d");
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol]);
			strcat (C->current.plot.format[0][0], fmt);
			strcat (C->current.plot.format[0][1], fmt);
		}

		/* Level 1: degrees and minutes only. index 0 is integer minutes, index 1 is [possibly] fractional minutes  */

		sprintf (C->current.plot.format[1][0], "%%" GMT_LL "d");	/* ddd */
		sprintf (C->current.plot.format[1][1], "%%" GMT_LL "d");
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol]);
			strcat (C->current.plot.format[1][0], fmt);
			strcat (C->current.plot.format[1][1], fmt);
		}
		strcat (C->current.plot.format[1][0], "%2.2" GMT_LL "d");
		if (S->order[2] == -1 && S->n_sec_decimals > 0) /* ddd:mm.xxx format */
			sprintf (fmt, "%%2.2" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm format */
			sprintf (fmt, "%%2.2" GMT_LL "d");
		strcat (C->current.plot.format[1][1], fmt);
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (C->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_squote]);
			strcat (C->current.plot.format[1][0], fmt);
			strcat (C->current.plot.format[1][1], fmt);
		}

		/* Level 2: degrees, minutes, and seconds. index 0 is integer seconds, index 1 is [possibly] fractional seconds  */

		sprintf (C->current.plot.format[2][0], "%%" GMT_LL "d");
		sprintf (C->current.plot.format[2][1], "%%" GMT_LL "d");
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[C->current.setting.map_degree_symbol]);
			strcat (C->current.plot.format[2][0], fmt);
			strcat (C->current.plot.format[2][1], fmt);
		}
		strcat (C->current.plot.format[2][0], "%2.2" GMT_LL "d");
		strcat (C->current.plot.format[2][1], "%2.2" GMT_LL "d");
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (C->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_squote]);
			strcat (C->current.plot.format[2][0], fmt);
			strcat (C->current.plot.format[2][1], fmt);
		}
		strcat (C->current.plot.format[2][0], "%2.2" GMT_LL "d");
		if (S->n_sec_decimals > 0)			 /* ddd:mm:ss.xxx format */
			sprintf (fmt, "%%" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm:ss format */
			sprintf (fmt, "%%2.2" GMT_LL "d");
		strcat (C->current.plot.format[2][1], fmt);
		if (C->current.setting.map_degree_symbol != gmt_none)
		{	/* We want the second symbol appended */
			if (C->current.setting.map_degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)C->current.setting.ps_encoding.code[gmt_dquote]);
			strcat (C->current.plot.format[2][0], fmt);
			strcat (C->current.plot.format[2][1], fmt);
		}

		/* Finally add %s for the [leading space]W,E,S,N char (or NULL) */

		for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) strcat (C->current.plot.format[i][j], "%s");
	}
}

GMT_LONG gmt_scanf_clock (struct GMT_CTRL *C, char *s, double *val)
{
	/* On failure, return -1.  On success, set val and return 0.

	Looks for apAP, but doesn't discover a failure if called with "11:13:15 Hello, Walter",
	because it will find an a.

	Doesn't check whether use of a or p matches stated intent to use twelve_hour_clock.

	ISO standard allows 24:00:00, so 86400 is not too big.
	If the day of this clock might be a day with a leap second, (this routine doesn't know that)
	then we should also allow 86401.  A value exceeding 86401 is an error.
	*/

	GMT_LONG k, hh, mm, add_noon = 0, hh_limit = 24;	/* ISO std allows 24:00:00  */
	double ss, x;
	char *p = NULL;

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

	k = sscanf (s, C->current.io.clock_input.format, &hh, &mm, &ss);
	if (k == 0) return (-1);
	if (hh < 0 || hh > hh_limit) return (-1);

	x = (double)(add_noon + 3600*hh);
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

GMT_LONG gmt_scanf_ISO_calendar (struct GMT_CTRL *C, char *s, GMT_LONG *rd) {

	/* On failure, return -1.  On success, set rd and return 0.
	Assumes that year, week of year, day of week appear in that
	order only, and that the format string can handle the W.
	Assumes also that it is always OK to fill in missing bits.  */

	GMT_LONG k, n, ival[3];

	if ((n = sscanf (s, C->current.io.date_input.format, &ival[0], &ival[1], &ival[2])) == 0) return (-1);

	/* Handle possible missing bits */
	for (k = n; k < 3; k++) ival[k] = 1;

	if (ival[1] < 1 || ival[1] > 53) return (-1);
	if (ival[2] < 1 || ival[2] > 7) return (-1);
	if (C->current.io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = GMT_y2_to_y4_yearfix (C, ival[0]);
	}
	*rd = GMT_rd_from_iywd (C, ival[0], ival[1], ival[2]);
	return (0);
}

GMT_LONG gmt_scanf_g_calendar (struct GMT_CTRL *C, char *s, GMT_LONG *rd)
{
	/* Return -1 on failure.  Set rd and return 0 on success.

	For gregorian calendars.  */

	GMT_LONG i, k, ival[4];
	char month[16];

	if (C->current.io.date_input.day_of_year) {
		/* Calendar uses year and day of year format.  */
		if ( (k = sscanf (s, C->current.io.date_input.format,
			&ival[C->current.io.date_input.item_order[0]],
			&ival[C->current.io.date_input.item_order[1]]) ) == 0) return (-1);
		if (k < 2) {
			if (!C->current.io.date_input.truncated_cal_is_ok) return (-1);
			ival[1] = 1;	/* Set first day of year  */
		}
		if (C->current.io.date_input.Y2K_year) {
			if (ival[0] < 0 || ival[0] > 99) return (-1);
			ival[0] = GMT_y2_to_y4_yearfix (C, ival[0]);
		}
		k = (GMT_is_gleap (ival[0])) ? 366 : 365;
		if (ival[3] < 1 || ival[3] > k) return (-1);
		*rd = GMT_rd_from_gymd (C, ival[0], 1, 1) + ival[3] - 1;
		return (0);
	}

	/* Get here when calendar type has months and days of months.  */

	if (C->current.io.date_input.mw_text) {	/* Have month name abbreviation in data format */
		switch (C->current.io.date_input.item_pos[1]) {	/* Order of month in data string */
			case 0:	/* e.g., JAN-24-1987 or JAN-1987-24 */
				k = sscanf (s, C->current.io.date_input.format, month, &ival[C->current.io.date_input.item_order[1]], &ival[C->current.io.date_input.item_order[2]]);
				break;
			case 1:	/* e.g., 24-JAN-1987 or 1987-JAN-24 */
				k = sscanf (s, C->current.io.date_input.format, &ival[C->current.io.date_input.item_order[0]], month, &ival[C->current.io.date_input.item_order[2]]);
				break;
			case 2:	/* e.g., JAN-24-1987 ? */
				k = sscanf (s, C->current.io.date_input.format, month, &ival[C->current.io.date_input.item_order[1]], &ival[C->current.io.date_input.item_order[2]]);
				break;
			default:
				k = 0;
				return (-1);
				break;
		}
		GMT_str_toupper (month);
		for (i = ival[1] = 0; i < 12 && ival[1] == 0; i++) {
			if (!strcmp (month, C->current.time.language.month_name[3][i])) ival[1] = i + 1;
		}
		if (ival[1] == 0) return (-1);	/* No match for month name */
	}
	else if ((k = sscanf (s, C->current.io.date_input.format, &ival[C->current.io.date_input.item_order[0]], &ival[C->current.io.date_input.item_order[1]], &ival[C->current.io.date_input.item_order[2]])) == 0)
		return (-1);
	if (k < 3) {
		if (C->current.io.date_input.truncated_cal_is_ok) {
			ival[2] = 1;	/* Set first day of month  */
			if (k == 1) ival[1] = 1;	/* Set first month of year */
		}
		else
			return (-1);
	}
	if (C->current.io.date_input.Y2K_year) {
		if (ival[0] < 0 || ival[0] > 99) return (-1);
		ival[0] = GMT_y2_to_y4_yearfix (C, ival[0]);
	}

	if (GMT_g_ymd_is_bad (ival[0], ival[1], ival[2]) ) return (-1);

	*rd = GMT_rd_from_gymd (C, ival[0], ival[1], ival[2]);
	return (0);
}

GMT_LONG gmt_scanf_calendar (struct GMT_CTRL *C, char *s, GMT_LONG *rd)
{
	/* On failure, return -1.  On success, set rd and return 0 */
	if (C->current.io.date_input.iso_calendar) return (gmt_scanf_ISO_calendar (C, s, rd));
	return (gmt_scanf_g_calendar (C, s, rd));
}

GMT_LONG gmt_scanf_geo (char *s, double *val)
{
	/* Try to read a character string token stored in s, knowing that it should be a geographical variable.
	If successful, stores value in val and returns one of GMT_IS_FLOAT, GMT_IS_GEO, GMT_IS_LAT, GMT_IS_LON,
	whichever can be determined from the format of s.
	If unsuccessful, does not store anything in val and returns GMT_IS_NAN.
	This should have essentially the same functionality as the GMT3.4 GMT_scanf, except that the expectation
	is now used and returned, and this also permits a double precision format in the minutes or seconds,
	and does more error checking.  However, this is not optimized for speed (yet).  WHFS, 16 Aug 2001

	Note: Mismatch handling (e.g. this routine finds a lon but calling routine expected a lat) is not
	done here.
	*/

	GMT_LONG retval = GMT_IS_FLOAT, k, id, im, ncolons, negate = FALSE;
	char scopy[GMT_TEXT_LEN64], suffix, *p = NULL, *p2 = NULL;
	double dd, dm, ds;

	k = (GMT_LONG)strlen (s);
	if (k == 0) return (GMT_IS_NAN);
	if (!(isdigit ((int)s[k-1]))) {
		suffix = s[k-1];
		switch (suffix) {
			case 'W': case 'w':
				negate = TRUE;
				retval = GMT_IS_LON;
				break;
			case 'E': case 'e':
				retval = GMT_IS_LON;
				break;
			case 'S': case 's':
				negate = TRUE;
				retval = GMT_IS_LAT;
				break;
			case 'N': case 'n':
				retval = GMT_IS_LAT;
				break;
			case 'G': case 'g': case 'D': case 'd':
				retval = GMT_IS_GEO;
				break;
			case '.':	/* Decimal point without decimals, e.g., 123. */
				break;
			default:
				return (GMT_IS_NAN);
				break;
		}
		k--;
	}
	if (k >= GMT_TEXT_LEN64) return (GMT_IS_NAN);
	strncpy (scopy, s, (size_t)k);				/* Copy all but the suffix  */
	scopy[k] = 0;
	ncolons = 0;
	if ((p = strpbrk (scopy, "dD"))) {
		/* We found a D or d.  */
		if (strlen (p) < 1 || (strpbrk (&p[1], "dD:") ) ){
			/* It is at the end, or followed by a colon or another d or D.  */
			return (GMT_IS_NAN);
		}
		/* Map it to an e, permitting FORTRAN Double Precision formats.  */
		p[0] = 'e';
	}
	p = scopy;
	while ((p2 = strpbrk (p, ":"))) {
		if (strlen (p2) < 1) return (GMT_IS_NAN);	/* Shouldn't end with a colon  */
		ncolons++;
		if (ncolons > 2) return (GMT_IS_NAN);
		p = &p2[1];
	}

	if (ncolons && retval == GMT_IS_FLOAT) retval = GMT_IS_GEO;

	dd = 0.0;
	switch (ncolons) {
		case 0:
			if ((sscanf (scopy, "%lf", &dd)) != 1) return (GMT_IS_NAN);
			break;
		case 1:
			if ((sscanf (scopy, "%" GMT_LL "d:%lf", &id, &dm)) != 2) return (GMT_IS_NAN);
			dd = dm * GMT_MIN2DEG;
			if (id < 0)	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			else if (id > 0)	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
		case 2:
			if ((sscanf (scopy, "%" GMT_LL "d:%" GMT_LL "d:%lf", &id, &im, &ds)) != 3) return (GMT_IS_NAN);
			dd = im * GMT_MIN2DEG + ds * GMT_SEC2DEG;
			if (id < 0)	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			else if (id > 0)	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
	}
	*val = (negate) ? -dd : dd;
	return (retval);
}


GMT_LONG gmt_scanf_float (char *s, double *val)
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

	char scopy[GMT_TEXT_LEN64], *p = NULL;
	double x;
	GMT_LONG j, k;

	x = strtod (s, &p);
	if (p[0] == 0) {	/* Success (non-Fortran).  */
		*val = x;
		return (GMT_IS_FLOAT);
	}
	if (p[0] != 'D' && p[0] != 'd') return (GMT_IS_NAN);
	k = strlen(p);
	if (k == 1) return (GMT_IS_NAN);	/* A string ending in e would be invalid  */
	/* Make a copy of s in scopy, mapping the d or D to an e */
	j = (GMT_LONG)strlen (s);
	if (j > GMT_TEXT_LEN64) return (GMT_IS_NAN);
	j -= k;
	strncpy (scopy, s, (size_t)j );
	scopy[j] = 'e';
	strcpy (&scopy[j+1], &p[1]);
	x = strtod (scopy, &p);
	if (p[0] != 0) return (GMT_IS_NAN);
	*val = x;
	return (GMT_IS_FLOAT);
}

GMT_LONG gmt_scanf_dim (struct GMT_CTRL *C, char *s, double *val)
{
	/* Try to decode a value from s and store
	in val.  s is a regular float with optional
	unit info, e.g., 8.5i or 7.5c.  If a valid unit
	is found we convert the number to inch.
	We also skip any trailing modifiers like +<mods>, e.g.
	vector specifications like 0.5i+jc+b+s

	We return GMT_IS_FLOAT and pass val.
	*/

	if (isalpha ((int)s[0]) || (s[1] == 0 && (s[0] == '-' || s[0] == '+')))	/* Probably a symbol character; quietly return 0 */
		*val = 0.0;
	else {	/* Probably a dimension with optional unit.  First check if there are modifiers to ignore here */
		char *p = NULL;
		if ((p = strchr (s, '+'))) { /* Found trailing +mod args */
			*p = 0;	/* Chop off modifier */
			*val = GMT_to_inch (C, s);	/* Get dimension */
			*p = '+';	/* Restore modifier */
		}
		else
			*val = GMT_to_inch (C, s);
	}
	return (GMT_IS_FLOAT);
}

GMT_LONG gmt_scanf_argtime (struct GMT_CTRL *C, char *s, double *t)
{
	/* s is a string from a command-line argument.
	   The argument is known to refer to a time variable.  For example, the argument is
	   a token from -R<t_min>/<t_max>/a/b[/c/d].  However, we will permit it to be in EITHER
	   -- generic floating point format, in which case we interpret it as relative time
	      in user units since epoch;
	   OR
	   -- absolute time in a restricted format, which is to be converted to relative time.

	   The absolute format must be restricted because we cannot use '/' as a delimiter in an arg
	   string, but we might allow the user to use that in a data file (in C->current.setting.[in/out]put_date_format.
	   Therefore we cannot use the user's date format string here, and we hard-wire something here.

	   The relative format must be decodable by gmt_scanf_float().  It may optionally end in 't'
	   (which will be stripped off by this routine).

	   The absolute format must have a T.  If it has a clock string then it must be of the form
	   <complete_calstring>T<clockstring> or just T<clockstring>.  If it has no clockstring then
	   it must be of the form <partial or complete calstring>T.

	   A <clockstring> may be partial (e.g. hh or hh:mm) or complete (hh:mm:ss[.xxx]) but it must use
	   ':' for a delimiter and it must be readable with "%2d:%2d:%lf".
	   Also, it must be a 24 hour clock (00:00:00 to 23:59:59.xxx,
	   or 60.xxx on a leap second); no am/pm suffixes allowed.

	   A <calstring> must be of the form
	   [-]yyyy[-mm[-dd]]T readable after first '-' with "%4d-%2d-%2dT" (Gregorian year,month,day)
	   [-]yyyy[-jjj]T readable after first '-' with "%4d-%3dT" (Gregorian year, day-of-year)
	   yyyy[-Www[-d]]T (ISO week calendar)

	Upon failure, returns GMT_IS_NAN.  Upon success, sets t and returns GMT_IS_ABSTIME.
	We have it return either ABSTIME or RELTIME to indicate which one it thinks it decoded.
	This is inconsistent with the use of GMT_scanf which always returns ABSTIME, even when RELTIME is
	expected and ABSTIME conversion is done internal to the routine, as it is here.
	The reason for returning RELTIME instead is that the -R option needs
	to know which was decoded and hence which is expected as column input.
	*/

	GMT_LONG hh, mm, j, k, i, dash, ival[3], negate_year = FALSE, got_yd = FALSE;
	double ss, x;
	char *pw = NULL, *pt = NULL;

	i = (GMT_LONG)strlen (s) - 1;
	if (s[i] == 't') s[i] = '\0';
	if ( (pt = strchr (s, (int)'T') ) == CNULL) {
		/* There is no T.  This must decode with gmt_scanf_float() or we die.  */
		if ((gmt_scanf_float (s, t)) == GMT_IS_NAN) return (GMT_IS_NAN);
		return (GMT_IS_RELTIME);
	}
	x = 0.0;	/* x will be the seconds since start of today.  */
	if (pt[1]) {	/* There is a string following the T:  Decode a clock */
		k = sscanf (&pt[1], "%2" GMT_LL "d:%2" GMT_LL "d:%lf", &hh, &mm, &ss);
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
	if (s[k] == 'T') {	/* There is no calendar.  Set day to 1 and use that */
		*t = GMT_rdc2dt (C, (GMT_LONG)1, x);
		return (GMT_IS_ABSTIME);
	}

	if (!(isdigit ((int)s[k]))) return (GMT_IS_NAN);	/* Bad format */

	if ((pw = strchr (s, (int)'W'))) {
		/* There is a W.  ISO calendar or junk.  */
		if (strlen (pw) <= strlen (pt)) return (GMT_IS_NAN);	/* The W is after the T.  Wrong format.  */
		if (negate_year) return (GMT_IS_NAN);			/* negative years not allowed in ISO calendar  */
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-W%2" GMT_LL "d-%1" GMT_LL "d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
		if (GMT_iso_ywd_is_bad (ival[0], ival[1], ival[2]) ) return (GMT_IS_NAN);
		*t = GMT_rdc2dt (C, GMT_rd_from_iywd (C, ival[0], ival[1], ival[2]), x);
		return (GMT_IS_ABSTIME);
	}

	for (i = negate_year; s[k+i] && s[k+i] != '-'; i++);;	/* Goes to first - between yyyy and jjj or yyyy and mm */
	dash = ++i;				/* Position of first character after the first dash (could be end of string if no dash) */
	while (s[k+i] && !(s[k+i] == '-' || s[k+i] == 'T')) i++;	/* Goto the ending T character or get stuck on a second - */
	got_yd = ((i - dash) == 3 && s[k+i] == 'T');		/* Must have a field of 3-characters between - and T to constitute a valid day-of-year format */

	if (got_yd) {	/* Gregorian yyyy-jjj calendar */
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-%3" GMT_LL "d", &ival[0], &ival[1]) ) != 2) return (GMT_IS_NAN);
		ival[2] = 1;
	}
	else {	/* Gregorian yyyy-mm-dd calendar */
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-%2" GMT_LL "d-%2" GMT_LL "d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
	}
	if (negate_year) ival[0] = -ival[0];
	if (got_yd) {
		if (ival[1] < 1 || ival[1] > 366)  return (GMT_IS_NAN);	/* Simple range check on day-of-year (1-366) */
		*t = GMT_rdc2dt (C, GMT_rd_from_gymd (C, ival[0], 1, 1) + ival[1] - 1, x);
	}
	else {
		if (GMT_g_ymd_is_bad (ival[0], ival[1], ival[2]) ) return (GMT_IS_NAN);
		*t = GMT_rdc2dt (C, GMT_rd_from_gymd (C, ival[0], ival[1], ival[2]), x);
	}

	return (GMT_IS_ABSTIME);
}

GMT_LONG GMT_scanf (struct GMT_CTRL *C, char *s, GMT_LONG expectation, double *val)
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

	char calstring[GMT_TEXT_LEN64], clockstring[GMT_TEXT_LEN64], *p = NULL;
	double x;
	GMT_LONG callen, clocklen, rd;

	if (s[0] == 'T') {	/* Numbers cannot start with letters except for clocks, e.g., T07:0 */
		if (!isdigit ((int)s[1])) return (GMT_IS_NAN);	/* Clocks must have T followed by digit, e.g., T07:0 otherwise junk*/
	}
	else if (isalpha ((int)s[0])) return (GMT_IS_NAN);	/* Numbers cannot start with letters */

	if (expectation & GMT_IS_GEO) {
		/* True if either a lat or a lon is expected  */
		return (gmt_scanf_geo (s, val));
	}

	else if (expectation == GMT_IS_FLOAT) {
		/* True if no special format is expected or allowed  */
		return (gmt_scanf_float (s, val));
	}

	else if (expectation == GMT_IS_DIMENSION) {
		/* True if units might be appended, e.g. 8.4i  */
		return (gmt_scanf_dim (C, s, val));
	}

	else if (expectation == GMT_IS_RELTIME) {
		/* True if we expect to read a float with no special
		formatting (except for an optional trailing 't'), and then
		assume it is relative time in user's units since epoch.  */
		callen = strlen (s) - 1;
		if (s[callen] == 't') s[callen] = '\0';
		if ((gmt_scanf_float (s, val)) == GMT_IS_NAN) return (GMT_IS_NAN);
		return (GMT_IS_ABSTIME);
	}

	else if (expectation == GMT_IS_ABSTIME) {
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

		if ((p = strchr ( s, (int)('T'))) == NULL) {
			/* There is no T.  Put all of s in calstring.  */
			clocklen = 0;
			strcpy (calstring, s);
		}
		else {
			clocklen = strlen(p);
			callen -= clocklen;
			strncpy (calstring, s, (size_t)callen);
			calstring[callen] = 0;
			strcpy (clockstring, &p[1]);
			clocklen--;
		}
		x = 0.0;
		if (clocklen && gmt_scanf_clock (C, clockstring, &x)) return (GMT_IS_NAN);
		rd = 1;
		if (callen && gmt_scanf_calendar (C, calstring, &rd)) return (GMT_IS_NAN);
		*val = GMT_rdc2dt (C, rd, x);
		if (C->current.setting.time_is_interval) {	/* Must truncate and center on time interval */
			GMT_moment_interval (C, &C->current.time.truncate.T, *val, TRUE);	/* Get the current interval */
			if (C->current.time.truncate.direction) {	/* Actually need midpoint of previous interval... */
				x = C->current.time.truncate.T.dt[0] - 0.5 * (C->current.time.truncate.T.dt[1] - C->current.time.truncate.T.dt[0]);
				GMT_moment_interval (C, &C->current.time.truncate.T, x, TRUE);	/* Get the current interval */
			}
			/* Now get half-point of interval */
			*val = 0.5 * (C->current.time.truncate.T.dt[1] + C->current.time.truncate.T.dt[0]);
		}
		return (GMT_IS_ABSTIME);
	}

	else if (expectation == GMT_IS_ARGTIME) {
		return (gmt_scanf_argtime (C, s, val));
	}

	else if (expectation & GMT_IS_UNKNOWN) {
		/* True if we dont know but must try both geographic or float formats  */
		return (gmt_scanf_geo (s, val));
	}

	else {
		GMT_report (C, GMT_MSG_FATAL, "GMT_LOGIC_BUG: GMT_scanf() called with invalid expectation.\n");
		return (GMT_IS_NAN);
	}
}

GMT_LONG GMT_scanf_arg (struct GMT_CTRL *C, char *s, GMT_LONG expectation, double *val)
{
	/* Version of GMT_scanf used for cpt & command line arguments only (not data records).
	 * It differs from GMT_scanf in that if the expectation is GMT_IS_UNKNOWN it will
	 * check to see if the argument is (1) an absolute time string, (2) a geographical
	 * location string, or if not (3) a floating point string.  To ensure backward
	 * compatibility: if we encounter geographic data it will also set the C->current.io.type[]
	 * variable accordingly so that data i/o will work as in 3.4
	 */

	char c;

	if (expectation == GMT_IS_UNKNOWN) {		/* Expectation for this column not set - must be determined if possible */
		c = s[strlen(s)-1];
		if (strchr (s, (int)'T'))		/* Found a T in the argument - assume Absolute time */
			expectation = GMT_IS_ARGTIME;
		else if (c == 't')			/* Found trailing t - assume Relative time */
			expectation = GMT_IS_ARGTIME;
		else if (strchr ("WwEe", (int)c))	/* Found trailing W or E - assume Geographic longitudes */
			expectation = GMT_IS_LON;
		else if (strchr ("SsNn", (int)c))	/* Found trailing S or N - assume Geographic latitudes */
			expectation = GMT_IS_LAT;
		else if (strchr ("DdGg", (int)c))	/* Found trailing G or D - assume Geographic coordinate */
			expectation = GMT_IS_GEO;
		else if (strchr (s, (int)':'))		/* Found a : in the argument - assume Geographic coordinates */
			expectation = GMT_IS_GEO;
		else 					/* Found nothing - assume floating point */
			expectation = GMT_IS_FLOAT;
	}

	/* OK, here we have an expectation, now call GMT_scanf */

	return (GMT_scanf (C, s, expectation, val));
}

struct GMT_TEXT_TABLE * GMT_read_texttable (struct GMT_CTRL *C, void *source, GMT_LONG source_type)
{
	/* Reads an entire segment text data set into memory */

	GMT_LONG close_file = FALSE, header = TRUE, no_segments;
	GMT_LONG n_fields, n_read = 0, seg = -1, row = 0, ncol = 0;
	GMT_LONG n_row_alloc = GMT_CHUNK, n_seg_alloc = GMT_CHUNK, n_head_alloc = GMT_TINY_CHUNK;
	char file[GMT_BUFSIZ], *in = NULL;
	FILE *fp = NULL;
	struct GMT_TEXT_TABLE *T = NULL;
	PFP psave = NULL;

	/* Determine input source */

	if (source_type == GMT_IS_FILE) {	/* source is a file name */
		strcpy (file, source);
		if ((fp = GMT_fopen (C, file, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", file);
			return (NULL);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (source_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)source;
		if (fp == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input stream>");
	}
	else if (source_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd = source;
		if (fd && (fp = fdopen (*fd, "r")) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_read_texttable\n", *fd);
			return (NULL);
		}
		if (fd == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input file descriptor>");
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_read_texttable\n", source_type);
		return (NULL);
	}

	in = GMT_ascii_textinput (C, fp, &ncol, &n_fields);	/* Get first record */
	n_read++;
	if (GMT_REC_IS_EOF (C)) {
		GMT_report (C, GMT_MSG_NORMAL, "File %s is empty!\n", file);
		return (NULL);
	}

	/* Allocate the Table structure */

	T = GMT_memory (C, NULL, 1, struct GMT_TEXT_TABLE);
	T->file[GMT_IN] = strdup (file);
	T->segment = GMT_memory (C, NULL, n_seg_alloc, struct GMT_TEXT_SEGMENT *);
	T->header  = GMT_memory (C, NULL, n_head_alloc, char *);

	while (n_fields >= 0 && !GMT_REC_IS_EOF (C)) {	/* Not yet EOF */
		while (header && ((C->current.io.io_header[GMT_IN] && n_read <= C->current.io.io_n_header_items) || GMT_REC_IS_TBL_HEADER (C))) { /* Process headers */
			T->header[T->n_headers] = strdup (C->current.io.current_record);
			T->n_headers++;
			if (T->n_headers == n_head_alloc) {
				n_head_alloc <<= 1;
				T->header = GMT_memory (C, T->header, n_head_alloc, char *);
			}
			in = GMT_ascii_textinput (C, fp, &ncol, &n_fields);
			n_read++;
		}
		header = FALSE;	/* Done processing header block; other comments are GIS/OGR encoded comments */

		if (T->n_headers)
			T->header = GMT_memory (C, T->header, T->n_headers, char *);
		else {	/* No header records found */
			GMT_free (C, T->header);
			T->header = NULL;
		}

		no_segments = !GMT_REC_IS_SEG_HEADER (C);	/* Not a multi-segment file.  We then assume file has only one segment */

		while (no_segments || (GMT_REC_IS_SEG_HEADER (C) && !GMT_REC_IS_EOF (C))) {
			/* PW: This will need to change to allow OGR comments to follow segment header */
			/* To use different line-distances for each segment, place the distance in the segment header */
			if (seg == -1 || T->segment[seg]->n_rows > 0) {
				seg++;	/* Only advance segment if last had any points or was the first one */
				T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_TEXT_SEGMENT);
			}
			n_read++;
			/* Segment initialization */
			n_row_alloc = GMT_CHUNK;
			row = 0;
			if (!no_segments) {
				in = GMT_ascii_textinput (C, fp, &ncol, &n_fields);	/* Don't read if we didnt read a segment header up front */
				n_read++;
			}
			no_segments = FALSE;	/* This has now served its purpose */
		}
		if (GMT_REC_IS_EOF (C)) continue;	/* At EOF; get out of this loop */
		if (!no_segments) {			/* Handle info stored in multi-seg header record */
			char buffer[GMT_BUFSIZ];
			GMT_memset (buffer, GMT_BUFSIZ, char);
			if (GMT_parse_segment_item (C, C->current.io.segment_header, "-L", buffer)) T->segment[seg]->label = strdup (buffer);
			if (strlen (C->current.io.segment_header)) T->segment[seg]->header = strdup (C->current.io.segment_header);
		}

		T->segment[seg]->record = GMT_memory (C, NULL, n_row_alloc, char *);

		while (!(C->current.io.status & (GMT_IO_SEG_HEADER | GMT_IO_EOF))) {	/* Keep going until FALSE or find a new segment header */

			T->segment[seg]->record[row++] = strdup (in);

			if (row == n_row_alloc) {
				n_row_alloc <<= 1;
				T->segment[seg]->record = GMT_memory (C, T->segment[seg]->record, n_row_alloc, char *);
			}
			in = GMT_ascii_textinput (C, fp, &ncol, &n_fields);
			n_read++;
		}
		T->segment[seg]->n_rows = row;	/* Number of records in this segment */
		T->n_records += row;		/* Total number of records so far */
		T->segment[seg]->id = seg;	/* Internal segment number */

		/* Reallocate to free up some memory */

		T->segment[seg]->record = GMT_memory (C, T->segment[seg]->record, T->segment[seg]->n_rows, char *);

		if (T->segment[seg]->n_rows == 0) {	/* Empty segment; we delete to avoid problems downstream in applications */
			GMT_free (C, T->segment[seg]);
			seg--;	/* Go back to where we were */
		}

		if (seg == (n_seg_alloc-1)) {
			GMT_LONG n_old_alloc = n_seg_alloc;
			n_seg_alloc <<= 1;
			T->segment = GMT_memory (C, T->segment, n_seg_alloc, struct GMT_TEXT_SEGMENT *);
			GMT_memset (&(T->segment[n_old_alloc]), n_seg_alloc - n_old_alloc, struct GMT_TEXT_SEGMENT *);	/* Set to NULL */
		}
	}
	if (close_file) GMT_fclose (C, fp);
	C->current.io.input = psave;	/* Restore former pointer */

	if (T->segment[seg]->n_rows == 0)	/* Last segment was empty; we delete to avoid problems downstream in applications */
		GMT_free (C, T->segment[seg]);
	else
		seg++;
	if (seg < n_seg_alloc) T->segment = GMT_memory (C, T->segment, seg, struct GMT_TEXT_SEGMENT *);
	T->n_segments = seg;

	return (T);
}

void GMT_set_seg_minmax (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Determine the min/max values for each column in the segment */
	GMT_LONG col, row;

	for (col = 0; col < S->n_columns; col++) {
		if (C->current.io.col_type[GMT_IN][col] == GMT_IS_LON) /* Requires separate quandrant assessment */
			GMT_get_lon_minmax (C, S->coord[col], S->n_rows, &(S->min[col]), &(S->max[col]));
		else {	/* Simple Cartesian-like arrangement */
			S->min[col] = S->max[col] = S->coord[col][0];
			for (row = 1; row < S->n_rows; row++) {
				if (S->coord[col][row] < S->min[col]) S->min[col] = S->coord[col][row];
				if (S->coord[col][row] > S->max[col]) S->max[col] = S->coord[col][row];
			}
		}
	}
}

void GMT_set_tbl_minmax (struct GMT_CTRL *C, struct GMT_TABLE *T)
{	/* Update the min/max of all segments and the entire table */
	GMT_LONG k, seg;
	struct GMT_LINE_SEGMENT *S = NULL;

	if (!T) return;	/* No table given */
	if (!T->n_columns) return;	/* No columns given */
	if (!T->min) T->min = GMT_memory (C, NULL, T->n_columns, double);
	if (!T->max) T->max = GMT_memory (C, NULL, T->n_columns, double);
	for (k = 0; k < T->n_columns; k++) {	/* Initialize */
		T->min[k] = DBL_MAX;
		T->max[k] = -DBL_MAX;
	}
	for (seg = 0; seg < T->n_segments; seg++) {
		S = T->segment[seg];
		GMT_set_seg_minmax (C, S);
		for (k = 0; k < T->n_columns; k++) {
			if (S->min[k] < T->min[k]) T->min[k] = S->min[k];
			if (S->max[k] > T->max[k]) T->max[k] = S->max[k];
		}
	}
}

GMT_LONG GMT_parse_segment_header (struct GMT_CTRL *C, char *header, struct GMT_PALETTE *P, GMT_LONG *use_fill, struct GMT_FILL *fill, struct GMT_FILL def_fill,  GMT_LONG *use_pen, struct GMT_PEN *pen, struct GMT_PEN def_pen, GMT_LONG def_outline, struct GMT_OGR_SEG *G)
{
	/* Scan header for occurrences of valid GMT options.
	 * The possibilities are:
	 * Fill: -G<fill>	Use the new fill and turn filling ON
	 *	 -G-		Turn filling OFF
	 *	 -G		Revert to default fill [none if not set on command line]
	 * Pens: -W<pen>	Use the new pen and turn outline ON
	 *	 -W-		Turn outline OFF
	 *	 -W		Revert to default pen [current.map_default_pen if not set on command line]
	 * z:	-Z<zval>	Obtain fill via cpt lookup using this z value
	 *	-ZNaN		Get the NaN color from the cpt file
	 *
	 * header is the text string to process
	 * P is the color palette used for the -Z option
	 * use_fill is set to TRUE, FALSE or left alone if no change
	 * fill is the fill structure to use after this function returns
	 * def_fill holds the default fill (if any) to use if -G is found
	 * use_pen is set to TRUE, FALSE or left alone if no change
	 * pen is the pen structure to use after this function returns
	 * def_pen holds the default pen (if any) to use if -W is found
	 * def_outline holds the default outline setting (TRUE/FALSE)
	 *
	 * The function returns the sum of the following return codes:
	 * 0 = No fill, no outline
	 * 1 = Revert to default fill or successfully parsed a -G<fill> option
	 * 2 = Encountered a -Z option to set the fill based on a CPT file
	 * 4 = Encountered a -W<pen> option
	 * For OGR/GMT files similar parsing occurs in that aspatial values assigned to the magic
	 * columns D, G, L, W, Z are used in the same fashion.
	 */

	GMT_LONG processed = 0, change = 0, k;
	char line[GMT_BUFSIZ], *txt = NULL;
	double z;
	struct GMT_FILL test_fill;
	struct GMT_PEN test_pen;

	if (C->common.a.active) {	/* Use aspatial data instead */
		for (k = 0; k < C->common.a.n_aspatial; k++) {
			if (C->common.a.col[k] >= 0) continue;	/* Skip regular data column fillers */
			txt = (G) ? G->value[k] : C->current.io.OGR->value[k];
			z = (G) ? G->dvalue[k] : C->current.io.OGR->dvalue[k];
			switch (C->common.a.col[k]) {
				case GMT_IS_G:
					GMT_getfill (C, txt, fill);
					*use_fill = TRUE;
					change = 1;
					processed++;	/* Processed one option */
					break;
				case GMT_IS_W:
					GMT_getpen (C, txt, pen);
					*use_pen = TRUE;
					change |= 4;
					break;
				case GMT_IS_Z:
					GMT_get_fill_from_z (C, P, z, fill);
					*use_fill = TRUE;
					change |= 2;
					processed++;	/* Processed one option */
					break;
			}
		}
		return (change);
	}
	
	/* Standard GMT multisegment parsing */
	
	if (!header || !header[0]) return (0);

	if (GMT_parse_segment_item (C, header, "-G", line)) {	/* Found a potential -G option */
		test_fill = def_fill;
		if (line[0] == '-') {	/* Turn fill OFF */
			fill->rgb[0] = fill->rgb[1] = fill->rgb[2] = -1.0, fill->use_pattern = FALSE;
			*use_fill = FALSE;
			processed++;	/* Processed one option */
		}
		else if (!line[0] || line[0] == '+') {	/* Revert to default fill */
			*fill = def_fill;
			*use_fill = (def_fill.use_pattern || def_fill.rgb[0] != -1.0);
			if (*use_fill) change = 1;
			processed++;	/* Processed one option */
		}
		else if (!GMT_getfill (C, line, &test_fill)) {	/* Successfully processed a -G<fill> option */
			*fill = test_fill;
			*use_fill = TRUE;
			change = 1;
			processed++;	/* Processed one option */
		}
		/* Failure is OK since -Gjunk may appear in text strings - we then do nothing (hence no else clause) */
	}
	if (P && GMT_parse_segment_item (C, header, "-Z", line)) {	/* Found a potential -Z option to set symbol r/g/b via cpt-lookup */
		if(!strncmp (line, "NaN", (size_t)3))	{	/* Got -ZNaN */
			GMT_get_fill_from_z (C, P, C->session.d_NaN, fill);
			*use_fill = TRUE;
			change |= 2;
			processed++;	/* Processed one option */
		}
		else if (sscanf (line, "%lg", &z) == 1) {
			GMT_get_fill_from_z (C, P, z, fill);
			*use_fill = TRUE;
			change |= 2;
			processed++;	/* Processed one option */
		}
		/* Failure is OK since -Zjunk may appear in text strings - we then do nothing (hence no else clause) */
	}

	if (processed == 2) GMT_report (C, GMT_MSG_FATAL, "Warning: segment header has both -G and -Z options\n");	/* Giving both -G and -Z is a problem */

	if (GMT_parse_segment_item (C, header, "-W", line)) {	/* Found a potential -W option */
		test_pen = def_pen;	/* Set test pen to the default, may be overruled later */
		if (line[0] == '-') {	/* Turn outline OFF */
			*pen = def_pen;	/* Set pen to default */
			*use_pen = FALSE;
		}
		else if (!line[0] || line[0] == '+') {	/* Revert to default pen/outline */
			*pen = def_pen;	/* Set pen to default */
			*use_pen = def_outline;
			if (def_outline) change |= 4;
		}
		else if (!GMT_getpen (C, line, &test_pen)) {
			*pen = test_pen;
			*use_pen = TRUE;
			change |= 4;
		}
		/* Failure is OK since -W may appear in text strings (hence no else clause) */
	}
	return (change);
}

void GMT_extract_label (struct GMT_CTRL *C, char *line, char *label)
{	/* Pull out the label in a -L<label> option in a segment header w./w.o. quotes */
	GMT_LONG i = 0, k, j, j0, done;
	char *p = NULL, q[2] = {'\"', '\''};

	if (GMT_parse_segment_item (C, line, "-L", label)) return;	/* Found -L */

	label[0] = '\0';	/* Remove previous label */
	if (!line || !line[0]) return;	/* Line is empty */
	while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;	/* Bypass whitespace */

	for (k = done = 0; k < 2; k++) {
		if ((p = strchr (&line[i], q[k]))) {	/* Gave several double/single-quoted words as label */
			for (j0 = j = i + 1; line[j] != q[k]; j++);
			if (line[j] == q[k]) {	/* Found the matching quote */
				strncpy (label, &line[j0], (size_t)(j-j0));
				label[j-j0] = '\0';
				done = TRUE;
			}
			else {			/* Missing the matching quote */
				sscanf (&line[i], "%s", label);
				GMT_report (C, GMT_MSG_FATAL, "Warning: Label (%s) not terminated by matching quote\n", label);
			}
		}
	}
	if (!done) sscanf (&line[i], "%s", label);
}

GMT_LONG GMT_parse_segment_item (struct GMT_CTRL *C, char *in_string, char *pattern, char *out_string)
{
	/* Scans the in_string for the occurrence of an option switch (e.g, -L) and
	 * if found, extracts the argument and returns it via out_string.  Function
	 * return TRUE if the pattern was found and FALSE otherwise.
	 * out_string must be allocated and have space for the copying */
	char *t = NULL;
	GMT_LONG k;
	if (!in_string || !pattern) return (FALSE);	/* No string or pattern passed */
	if (!(t = strstr (in_string, pattern))) return (FALSE);	/* Option not present */
	if (!out_string) return (TRUE);	/* If NULL is passed as out_string then we just return TRUE if we find the option */
	out_string[0] = '\0';	/* Reset string to empty before we try to set it below */
	k = (GMT_LONG)t - (GMT_LONG)in_string;	/* Position of pattern in in_string */
	if (k && !(in_string[k-1] == ' ' || in_string[k-1] == '\t')) return (FALSE);	/* Option not first or preceeded by whitespace */
	t += 2;	/* Position of the argument */
	if (t[0] == '\"')	/* Double quoted argument, must scan from next character until terminal quote */
		sscanf (++t, "%[^\"]", out_string);
	else if (t[0] == '\'')	/* Single quoted argument, must scan from next character until terminal quote */
		sscanf (++t, "%[^\']", out_string);
	else	/* Scan until next white space; stop also when there is leading white space, indicating no argument at all! */
		sscanf (t, "%[^ \t]", out_string);
	return (TRUE);
}

void GMT_write_ogr_header (FILE *fp, struct GMT_OGR *G)
{	/* Write out table-level OGR/GMT header metadata */
	GMT_LONG k;
	char *flavor = "egpw";

	fprintf (fp, "# @VGMT1.0 @G");
	if (G->geometry > GMT_IS_POLYGON) fprintf (fp, "MULTI");
	if (G->geometry == GMT_IS_POINT || G->geometry == GMT_IS_MULTIPOINT) fprintf (fp, "POINT\n");
	if (G->geometry == GMT_IS_LINESTRING || G->geometry == GMT_IS_MULTILINESTRING) fprintf (fp, "LINESTRING\n");
	if (G->geometry == GMT_IS_POLYGON || G->geometry == GMT_IS_MULTIPOLYGON) fprintf (fp, "POLYGON\n");
	fprintf (fp, "# @R%s\n", G->region);
	for (k = 0; k < 4; k++) {
		if (G->proj[k]) fprintf (fp, "# @J%c%s\n", flavor[k], G->proj[k]);
	}
	if (G->n_aspatial) {
		fprintf (fp, "# @N%s", G->name[0]);
		for (k = 1; k < G->n_aspatial; k++) fprintf (fp, "|%s", G->name[k]);
		fprintf (fp, "\n# @T%s", GMT_type[G->type[0]]);
		for (k = 1; k < G->n_aspatial; k++) fprintf (fp, "|%s", GMT_type[G->type[k]]);
		fprintf (fp, "\n");
	}
	fprintf (fp, "# FEATURE_DATA\n");
}

void gmt_write_formatted_ogr_value (struct GMT_CTRL *C, FILE *fp, GMT_LONG col, GMT_LONG type, struct GMT_OGR_SEG *G)
{
	char text[GMT_TEXT_LEN64];
	
	switch (type) {
		case GMTAPI_TEXT:
			fprintf (fp, "%s", G->value[col]);
			break;
		case GMTAPI_DOUBLE:
		case GMTAPI_FLOAT:
			GMT_ascii_format_col (C, text, G->dvalue[col], GMT_Z);
			fprintf (fp, "%s", text);
			break;
		case GMTAPI_CHAR:
		case GMTAPI_UCHAR:
		case GMTAPI_INT:
		case GMTAPI_UINT:
		case GMTAPI_LONG:
		case GMTAPI_ULONG:
			fprintf (fp, "%ld", (GMT_LONG)irint (G->dvalue[col]));
			break;
		case GMTAPI_TIME:
			gmt_format_abstime_output (C, G->dvalue[col], text);
			fprintf (fp, "%s", text);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Bad type passed to gmt_write_formatted_ogr_value - assumed to be double\n");
			GMT_ascii_format_col (C, text, G->dvalue[col], GMT_Z);
			fprintf (fp, "%s", text);
			break;
	}
}

void gmt_write_ogr_segheader (struct GMT_CTRL *C, FILE *fp, struct GMT_LINE_SEGMENT *S)
{	/* Write out segment-level OGR/GMT header metadata */
	GMT_LONG k, col;
	char *kind = "PH";
	char *sflag[7] = {"-D", "-G", "-I", "-L", "-T", "-W", "-Z"}, *quote[7] = {"", "", "\"", "\"", "\"", "", ""};
	char buffer[GMT_BUFSIZ];

	if (C->common.a.geometry == GMT_IS_POLYGON || C->common.a.geometry == GMT_IS_MULTIPOLYGON) fprintf (fp, "# @%c\n", (int)kind[S->ogr->pol_mode]);
	if (C->common.a.n_aspatial) {
		fprintf (fp, "# @D");
		for (k = 0; k < C->common.a.n_aspatial; k++) {
			if (k) fprintf (fp, "|");
			switch (C->common.a.col[k]) {
				case GMT_IS_D:	/* Pick up from -D<distance> */
				case GMT_IS_G:	/* Pick up from -G<fill> */
				case GMT_IS_I:	/* Pick up from -I<ID> */
				case GMT_IS_T:	/* Pick up from -T<text> */
				case GMT_IS_W:	/* Pick up from -W<pen> */
				case GMT_IS_Z:	/* Pick up from -Z<value> */
					col = -C->common.a.col[k] - 1;	/* So -3 becomes 2 etc */
					if (GMT_parse_segment_item (C, C->current.io.segment_header, sflag[col], buffer)) fprintf (fp, "%s%s%s", quote[col], buffer, quote[col]);
					break;
				case GMT_IS_L:	/* Pick up from -L<value> */
					col = -C->common.a.col[k] - 1;	/* So -3 becomes 2 etc */
					if (S->label) fprintf (fp, "%s%s%s", quote[col], S->label, quote[col]);
					else if (GMT_parse_segment_item (C, C->current.io.segment_header, sflag[col], buffer)) fprintf (fp, "%s%s%s", quote[col], buffer, quote[col]);
					break;
				default:	/* Regular column cases */
					if (S->ogr) gmt_write_formatted_ogr_value (C, fp, k, C->common.a.type[k], S->ogr);
					break;
			}
		}
		fprintf (fp, "\n");
	}
}

void gmt_build_segheader_from_ogr (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Build segment-level OGR/GMT header metadata */
	GMT_LONG k, col, n, space = FALSE;
	char *sflag[7] = {"-D", "-G", "-I", "-L", "-T", "-W", "-Z"};
	char buffer[GMT_BUFSIZ];

	if (C->common.a.output) return;		/* Input was not OGR (but output will be) */
	n = (S->ogr && S->ogr->n_aspatial) ? S->ogr->n_aspatial : C->common.a.n_aspatial;
	if (n == 0) return;	/* Either input was not OGR or there are no aspatial fields */
	buffer[0] = 0;
	for (k = 0; k < n; k++) {
		switch (C->common.a.col[k]) {
			case GMT_IS_D:	/* Format -D<distance> */
			case GMT_IS_G:	/* Format -G<fill> */
			case GMT_IS_I:	/* Format -I<ID> */
			case GMT_IS_T:	/* Format -T<text> */
			case GMT_IS_W:	/* Format -W<pen> */
			case GMT_IS_Z:	/* Format -Z<value> */
				col = -C->common.a.col[k] - 1;	/* So -3 becomes 2 etc */
				if (space) strcat (buffer, " ");
				strcat (buffer, sflag[col]);
				strcat (buffer, S->ogr->value[C->common.a.ogr[k]]);
				space = TRUE;
				break;
			default:	/* Regular column cases are skipped */
				break;
		}
	}
	if (GMT_polygon_is_hole (S)) {		/* Indicate this is a polygon hole [Default is perimeter] */
		if (space) strcat (buffer, " ");
		strcat (buffer, "-Ph");
	}
	if (S->header) { strcat (buffer, " "); strcat (buffer, S->header); }	/* Append rest of previous header */
	free (S->header);
	S->header = strdup (buffer);
}

void gmt_alloc_ogr_seg (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, GMT_LONG n_aspatial)
{	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	if (S->ogr) return;	/* Already allocated */
	S->ogr = GMT_memory (C, NULL, 1, struct GMT_OGR_SEG);
	S->ogr->n_aspatial = n_aspatial;
	if (n_aspatial) {
		S->ogr->value = GMT_memory (C, NULL, n_aspatial, char *);
		S->ogr->dvalue = GMT_memory (C, NULL, n_aspatial, double);
	}
}

void gmt_copy_ogr_seg (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, struct GMT_OGR *G)
{	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	GMT_LONG k;
	
	gmt_alloc_ogr_seg (C, S, G->n_aspatial);
	for (k = 0; k < G->n_aspatial; k++) {
		if (G->value[k]) S->ogr->value[k] = strdup (G->value[k]);
		S->ogr->dvalue[k] = G->dvalue[k];
	}
	S->ogr->pol_mode = G->pol_mode;
}

void GMT_duplicate_ogr_seg (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S_to, struct GMT_LINE_SEGMENT *S_from)
{	/* Allocates the OGR structure for a given segment and copies current values from table OGR segment */
	GMT_LONG k;
	
	if (!S_from->ogr) return;	/* No data */
	gmt_alloc_ogr_seg (C, S_to, S_from->ogr->n_aspatial);
	for (k = 0; k < S_from->ogr->n_aspatial; k++) {
		if (S_from->ogr->value[k]) S_to->ogr->value[k] = strdup (S_from->ogr->value[k]);
		S_to->ogr->dvalue[k] = S_from->ogr->dvalue[k];
	}
	S_to->ogr->pol_mode = S_from->ogr->pol_mode;
}

GMT_LONG gmt_prep_ogr_output (struct GMT_CTRL *C, struct GMT_DATASET *D) {

	GMT_LONG object_ID, seg1, seg2, k, row, col, seg, stop, n_reg, item, error = 0;
	char buffer[GMT_BUFSIZ], in_string[GMTAPI_STRLEN], out_string[GMTAPI_STRLEN];
	struct GMT_TABLE *T = NULL;
	struct GMT_DATASET *M = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct GMTAPI_DATA_OBJECT O;
	
	/* When this functions is called we have already registered the output destination.  This will normally
	 * prevent us from register the data set separately in order to call GMT_minmax.  We must temporarily
	 * unregister the output, do our thing, then reregister again. */

	n_reg = GMTAPI_n_items (C->parent, GMT_IS_DATASET, GMT_OUT, &object_ID);	/* Are there outputs registered already? */
	if (n_reg == 1) {	/* Yes, must save and unregister, then reregister later */
		if ((error = GMTAPI_Validate_ID (C->parent, GMT_IS_DATASET, object_ID, GMT_OUT, &item)) != GMT_OK) return (GMT_Report_Error (C->parent, error));
		GMT_memcpy (&O, C->parent->object[item], 1, struct GMTAPI_DATA_OBJECT);
		GMTAPI_Unregister_IO (C->parent, object_ID, GMT_OUT);
	}

	/* Determine w/e/s/n via GMT_minmax */

	/* Create option list, register D as input source via ref */
	if ((object_ID = GMT_Register_IO (C->parent, GMT_IS_DATASET, GMT_IS_REF, GMT_IS_POINT, GMT_IN, D, NULL)) == GMTAPI_NOTSET) {
		return (C->parent->error);
	}
	if (GMT_Encode_ID (C->parent, in_string, object_ID) != GMT_OK) {
		return (C->parent->error);	/* Make filename with embedded object ID */
	}
	if ((object_ID = GMT_Register_IO (C->parent, GMT_IS_DATASET, GMT_IS_COPY, GMT_IS_POINT, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
		return (C->parent->error);
	}
	if (GMT_Encode_ID (C->parent, out_string, object_ID)) {
		return (C->parent->error);	/* Make filename with embedded object ID */
	}
	sprintf (buffer, "-C -fg -<%s ->%s", in_string, out_string);
	if (GMT_minmax (C->parent, 0, buffer) != GMT_OK) {	/* Get the extent via minmax */
		return (C->parent->error);
	}
	if ((M = GMT_Retrieve_Data (C->parent, object_ID)) == NULL) {
		return (C->parent->error);
	}
	
	/* Time to reregister the original destination */
	
	if ((object_ID = GMT_Register_IO (C->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, D, NULL)) == GMTAPI_NOTSET) {
		return (C->parent->error);
	}
	if ((error = GMTAPI_Validate_ID (C->parent, GMT_IS_DATASET, object_ID, GMT_OUT, &item)) != GMT_OK) {
		return (GMT_Report_Error (C->parent, error));
	}
	GMT_memcpy (C->parent->object[item], &O, 1, struct GMTAPI_DATA_OBJECT);	/* Restore what we had before */
	
	T = D->table[0];
	T->ogr = GMT_memory (C, NULL, 1, struct GMT_OGR);
	sprintf (buffer, "%.8g/%.8g/%.8g/%.8g", M->table[0]->segment[0]->coord[0][0], M->table[0]->segment[0]->coord[1][0], M->table[0]->segment[0]->coord[2][0], M->table[0]->segment[0]->coord[3][0]);
	GMT_free_dataset (C, &M);
	T->ogr->region = strdup (buffer);
	T->ogr->proj[1] = strdup ("\"-Jx1d --PROJ_ELLIPSOID=WGS84\"");
	T->ogr->proj[2] = strdup ("\"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs\"");
	T->ogr->proj[3] = strdup ("\"GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]\"");
	T->ogr->geometry = C->common.a.geometry;
	T->ogr->n_aspatial = C->common.a.n_aspatial;
	if (T->ogr->n_aspatial) {	/* Copy over the command-line settings */
		T->ogr->name = GMT_memory (C, NULL, T->ogr->n_aspatial, char *);
		T->ogr->type = GMT_memory (C, NULL, T->ogr->n_aspatial, GMT_LONG);
		T->ogr->dvalue = GMT_memory (C, NULL, T->ogr->n_aspatial, double);
		for (k = 0; k < T->ogr->n_aspatial; k++) {
			T->ogr->name[k] = strdup (C->common.a.name[k]);
			T->ogr->type[k] = C->common.a.type[k];
		}
		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			S = T->segment[seg];
			gmt_alloc_ogr_seg (C, S, T->ogr->n_aspatial);	/* Copy over any feature-specific values */
			for (k = 0; k < T->ogr->n_aspatial; k++) {	/* For each column to turn into a constant aspatial value */
				col = C->common.a.col[k];
				if (col < 0) continue;	/* Multisegment header entry instead */
				for (row = 1, stop = FALSE; !stop && row < S->n_rows; ++row) {
					if (!doubleAlmostEqualZero (S->coord[col][row], S->coord[col][row-1]))
						stop = TRUE;
				}
				if (stop) {
					GMT_report (C, GMT_MSG_FATAL, "The -a option specified a constant column but its contents vary!\n");
					return (GMT_RUNTIME_ERROR);
				}
				else
					S->ogr->dvalue[k] = S->coord[col][0];
			}
		}
		/* OK, successfully passed the constant column tests, if any */
		for (seg = col = 0; seg < T->n_segments; seg++) {	/* Free up columns now stored as aspatial values */
			S = T->segment[seg];
			for (k = 0; k < T->ogr->n_aspatial; k++) if (C->common.a.col[k] > 0) GMT_free (C, S->coord[C->common.a.col[k]]);
			for (col = k = 0; k < T->n_columns; k++) {
				while (!S->coord[k]) k++;	/* Next available column */
				S->coord[col++] = S->coord[k];	/* Update pointers */
			}
			S->n_columns = col;	/* May have lost some columns now */
		}
		T->n_columns = D->n_columns = col;	/* May have lost some columns now */
	}
	if (T->ogr->geometry == GMT_IS_POLYGON || T->ogr->geometry == GMT_IS_MULTIPOLYGON) {	/* Must check consistency */
		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			if ((T->ogr->geometry == GMT_IS_POLYGON || T->ogr->geometry == GMT_IS_MULTIPOLYGON) && GMT_polygon_is_open (C, T->segment[seg]->coord[GMT_X], T->segment[seg]->coord[GMT_Y], T->segment[seg]->n_rows)) {
				GMT_report (C, GMT_MSG_FATAL, "The -a option specified [M]POLY but open segments were detected!\n");
				GMT_Destroy_Data (C->parent, GMT_ALLOCATED, &D[GMT_OUT]);
				return (GMT_RUNTIME_ERROR);
			}
			gmt_alloc_ogr_seg (C, T->segment[seg], T->ogr->n_aspatial);	/* Copy over any feature-specific values */
			T->segment[seg]->ogr->pol_mode = GMT_IS_PERIMETER;
			GMT_set_seg_minmax (C, T->segment[seg]);	/* Make sure min/max are set per polygon */

		}
		/* OK, they are all polygons.  Determine any polygon holes: if a point is fully inside another polygon (not on the edge) */
		for (seg1 = 0; seg1 < T->n_segments; seg1++) {	/* For each segment in the table */
			for (seg2 = seg1 + 1; seg2 < T->n_segments; seg2++) {	/* For each segment in the table */
				if (GMT_inonout (C, T->segment[seg1]->coord[GMT_X][0], T->segment[seg1]->coord[GMT_Y][0], T->segment[seg2]) == GMT_INSIDE) T->segment[seg1]->ogr->pol_mode = GMT_IS_HOLE;
				if (GMT_inonout (C, T->segment[seg2]->coord[GMT_X][0], T->segment[seg2]->coord[GMT_Y][0], T->segment[seg1]) == GMT_INSIDE) T->segment[seg2]->ogr->pol_mode = GMT_IS_HOLE;
			}
		}
	}
	if (T->ogr->geometry > GMT_IS_POINT && T->ogr->geometry != GMT_IS_MULTIPOINT) {	/* Must check for Dateline crossings */
		GMT_LONG n_split, n_segs = T->n_segments;
		struct GMT_LINE_SEGMENT **L = NULL;
		
		for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment in the table */
			if (!GMT_crossing_dateline (C, T->segment[seg])) continue;	/* GIS-safe feature! */
			GMT_report (C, GMT_MSG_VERBOSE, "Feature %ld crosses the Dateline\n", seg);
			if (!C->common.a.clip) continue;	/* Not asked to clip */
			/* Here we must split into east and west part(s) */
			if (T->ogr->geometry == GMT_IS_POLYGON || T->ogr->geometry == GMT_IS_MULTIPOLYGON) {	/* Clipping must add dateline segments */
				/* Clip into two closed polygons.  Eventually, perhaps return more (eliminate bridges) */
				n_split = GMT_split_poly_at_dateline (C, T->segment[seg], &L);
			}
			else {	/* Clipping just needs to add crossing points */
				/* Truncate into two or more line segments */
				n_split = GMT_split_line_at_dateline (C, T->segment[seg], &L);
			}
			T->segment = GMT_memory (C, T->segment, n_segs + n_split - 1, struct GMT_LINE_SEGMENT *);	/* Allow more space for new segments */
			GMT_free_segment (C, T->segment[seg]);	/* Delete the old one */
			T->segment[seg] = L[0];			/* Hook in the first replacement */
			for (k = 1; k < n_split; k++) T->segment[n_segs++] = L[k];	/* Add the remaining segments to the end */
			GMT_free (C, L);
		}
		D->n_segments = T->n_segments = n_segs;	/* Update number of segments */
		
	}
	C->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Select the -180/180 output range format */
	return (0);
}

GMT_LONG GMT_write_table (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TABLE *table, GMT_LONG use_GMT_io, GMT_LONG io_mode)
{
	/* Writes an entire segment data set to file or wherever.
	 * Specify io_mode == GMT_WRITE_SEGMENTS or GMT_WRITE_TABLE_SEGMENTS to write segments to individual files.
	 * If dist is NULL we choose stdout. */

	GMT_LONG ascii, close_file = FALSE, append, save = 0, row = 0, seg, col, k;
	int *fd = NULL;
	char open_mode[4], file[GMT_BUFSIZ], tmpfile[GMT_BUFSIZ], *out_file = tmpfile;
	double *out = NULL;
	FILE *fp = NULL;
	PFL psave = NULL;

	if (table->mode == GMT_WRITE_SKIP) return (0);	/* Skip this table */

	append = (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>');	/* Want to append to existing file */

	if (use_GMT_io) {	/* Use C->current.io.info settings to determine if input is ascii/binary, else it defaults to ascii */
		strcpy (open_mode, (append) ? C->current.io.a_mode : C->current.io.w_mode);
		ascii = !C->common.b.active[GMT_OUT];
	}
	else {			/* Force ASCII mode */
		strcpy (open_mode, (append) ? "a" : "w");
		ascii = TRUE;
		psave = C->current.io.output;		/* Save the previous pointer since we need to change it back at the end */
		C->current.io.output = C->session.output_ascii;	/* Override and use ascii mode */
	}

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			strcpy (file, dest);
			if (io_mode < GMT_WRITE_SEGMENTS) {	/* Only require one destination */
				if ((fp = GMT_fopen (C, &file[append], open_mode)) == NULL) {
					GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", &file[append]);
					GMT_exit (EXIT_FAILURE);
				}
				close_file = TRUE;	/* We only close files we have opened here */
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_write_table\n", *fd);
				GMT_exit (EXIT_FAILURE);
			}
			if (fd == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_write_table\n", dest_type);
			GMT_exit (EXIT_FAILURE);
			break;
	}
	if (io_mode < GMT_WRITE_SEGMENTS) {
		if (ascii && C->current.io.io_header[GMT_OUT]) {
			for (k = 0; k < table->n_headers; k++) GMT_write_tableheader (C, fp, table->header[k]);
		}
		if (table->ogr) GMT_write_ogr_header (fp, table->ogr);	/* Must write OGR/GMT header */
	}

	out = GMT_memory (C, NULL, table->n_columns, double);
	for (seg = 0; seg < table->n_segments; seg++) {
		if (table->segment[seg]->mode == GMT_WRITE_SKIP) continue;	/* Skip this segment */
		if (io_mode >= GMT_WRITE_SEGMENTS) {	/* Create separate file for each segment */
			if (table->segment[seg]->file[GMT_OUT])
				out_file = table->segment[seg]->file[GMT_OUT];
			else if (io_mode == GMT_WRITE_TABLE_SEGMENTS)	/* Build name with table id and seg # */
				sprintf (tmpfile, file, table->id, seg);
			else					/* Build name with seg ids */
				sprintf (tmpfile, file, table->segment[seg]->id);
			if ((fp = GMT_fopen (C, out_file, open_mode)) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", out_file);
				GMT_exit (EXIT_FAILURE);
			}
			GMT_report (C, GMT_MSG_NORMAL, "Writing data segment to file %s\n", out_file);
			if (ascii && C->current.io.io_header[GMT_OUT]) for (k = 0; k < table->n_headers; k++) GMT_write_tableheader (C, fp, table->header[k]);
		}
		if (C->current.io.multi_segments[GMT_OUT]) {	/* Want to write segment headers */
			if (table->segment[seg]->ogr) gmt_build_segheader_from_ogr (C, table->segment[seg]);	/* We have access to OGR metadata */
			if (table->segment[seg]->header) strcpy (C->current.io.segment_header, table->segment[seg]->header);
			GMT_write_segmentheader (C, fp, table->segment[seg]->n_columns);
			if (table->segment[seg]->ogr && C->common.a.output) gmt_write_ogr_segheader (C, fp, table->segment[seg]);
		}
		if (table->segment[seg]->mode == GMT_WRITE_HEADER) continue;	/* Skip after writing segment header */
		if (table->segment[seg]->range) {save = C->current.io.geo.range; C->current.io.geo.range = table->segment[seg]->range; }	/* Segment-specific formatting */
		for (row = 0; row < table->segment[seg]->n_rows; row++) {
			for (col = 0; col < table->segment[seg]->n_columns; col++) out[col] = table->segment[seg]->coord[col][row];
			C->current.io.output (C, fp, table->segment[seg]->n_columns, out);
		}
		if (table->segment[seg]->range) C->current.io.geo.range = save; 	/* Restore formatting */
		if (io_mode == GMT_WRITE_SEGMENTS) GMT_fclose (C, fp);	/* Close the segment file */
	}

	if (close_file) GMT_fclose (C, fp);	/* Close the file since we opened it */
	GMT_free (C, out);			/* Free up allocated memory */
	if (!use_GMT_io) C->current.io.output = psave;	/* Restore former pointers and values */

	return (0);	/* OK status */
}

GMT_LONG GMT_write_dataset (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_DATASET *D, GMT_LONG use_GMT_io, GMT_LONG table)
{	/* Writes an entire data set to file or stream */
	GMT_LONG tbl, error, append = 0, close_file = FALSE;
	int *fd = NULL;
	char file[GMT_BUFSIZ], tmpfile[GMT_BUFSIZ], open_mode[4], *out_file = tmpfile;
	FILE *fp = NULL;

	if (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>') append = 1;	/* Want to append to existing file */
	if (use_GMT_io)	/* Use C->current.io.info settings to determine if input is ascii/binary, else it defaults to ascii */
		strcpy (open_mode, (append) ? C->current.io.a_mode : C->current.io.w_mode);
	else			/* Force ASCII mode */
		strcpy (open_mode, (append) ? "a" : "w");

	/* Convert any destination type to stream */

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			strcpy (file, dest);
			if (D->io_mode < GMT_WRITE_TABLES) {	/* Only need one destination */
				if ((fp = GMT_fopen (C, &file[append], open_mode)) == NULL) {
					GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", &file[append]);
					return (EXIT_FAILURE);
				}
				close_file = TRUE;	/* We only close files we have opened here */
				GMT_report (C, GMT_MSG_NORMAL, "Write Data Table to file %s\n", &file[append]);
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			GMT_report (C, GMT_MSG_NORMAL, "Write Data Table to %s\n", file);
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_write_table\n", *fd);
				return (EXIT_FAILURE);
			}
			if (fd == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			GMT_report (C, GMT_MSG_NORMAL, "Write Data Table to %s\n", file);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_write_table\n", dest_type);
			return (EXIT_FAILURE);
			break;
	}

	if (D->io_mode == GMT_WRITE_OGR && gmt_prep_ogr_output (C, D)) {	/* Must preprocess aspatial information and set metadata */
		GMT_report (C, GMT_MSG_FATAL, "Failed to prepare for OGR output formatting\n");
		return (EXIT_FAILURE);
	}	
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		if (table != GMTAPI_NOTSET && table != tbl) continue;	/* Selected a specific table */
		if (D->io_mode > GMT_WRITE_TABLES) {	/* Write segments to separate files; must pass original file name in case a template */
			if ((error = GMT_write_table (C, dest, GMT_IS_FILE, D->table[tbl], use_GMT_io, D->io_mode))) return (error);
		}
		else if (D->io_mode == GMT_WRITE_TABLES) {	/* Must write this table a its own file */
			if (D->table[tbl]->file[GMT_OUT])
				out_file = D->table[tbl]->file[GMT_OUT];
			else
				sprintf (tmpfile, file, D->table[tbl]->id);
			GMT_report (C, GMT_MSG_NORMAL, "Write Data Table to %s\n", out_file);
			if ((error = GMT_write_table (C, out_file, GMT_IS_FILE, D->table[tbl], use_GMT_io, D->io_mode))) return (error);
		}
		else {	/* Write to stream we set up earlier */
			if ((error = GMT_write_table (C, fp, GMT_IS_STREAM, D->table[tbl], use_GMT_io, D->io_mode))) return (error);
		}
	}

	if (close_file) GMT_fclose (C, fp);

	return (0);	/* OK status */
}

GMT_LONG gmt_write_texttable (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TEXT_TABLE *table, GMT_LONG io_mode)
{
	/* Writes an entire segment text data set to file or wherever.
	 * Specify io_mode == GMT_WRITE_SEGMENTS or GMT_WRITE_TABLE_SEGMENTS to write segments to individual files.
	 * If dist is NULL we choose stdout. */

	GMT_LONG close_file = FALSE, row = 0, append, seg, k;
	int *fd = NULL;	/* Must be int, not GMT_LONG */
	char file[GMT_BUFSIZ], tmpfile[GMT_BUFSIZ], *out_file = tmpfile;
	FILE *fp = NULL;

	if (table->mode == GMT_WRITE_SKIP) return (0);	/* Skip this table */

	append = (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>');	/* Want to append to existing file */

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			strcpy (file, dest);
			if (io_mode < GMT_WRITE_SEGMENTS) {	/* Only require one destination */
				if ((fp = GMT_fopen (C, &file[append], (append) ? "a" : "w")) == NULL) {
					GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s in gmt_write_texttable\n", &file[append]);
					GMT_exit (EXIT_FAILURE);
				}
				close_file = TRUE;	/* We only close files we have opened here */
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, "w")) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in gmt_write_texttable\n", *fd);
				GMT_exit (EXIT_FAILURE);
			}
			if (fd == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in gmt_write_texttable\n", dest_type);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	if (io_mode < GMT_WRITE_SEGMENTS) {
		if (C->current.io.io_header[GMT_OUT]) {
			for (k = 0; k < table->n_headers; k++) GMT_write_tableheader (C, fp, table->header[k]);
		}
	}
	for (seg = 0; seg < table->n_segments; seg++) {
		if (table->segment[seg]->mode == GMT_WRITE_SKIP) continue;	/* Skip this segment */
		if (io_mode >= GMT_WRITE_SEGMENTS) {	/* Create separate file for each segment */
			if (table->segment[seg]->file[GMT_OUT])
				out_file = table->segment[seg]->file[GMT_OUT];
			else if (io_mode == GMT_WRITE_TABLE_SEGMENTS)	/* Build name with table id and seg # */
				sprintf (tmpfile, file, table->id, seg);
			else					/* Build name with seg ids */
				sprintf (tmpfile, file, table->segment[seg]->id);
			if ((fp = GMT_fopen (C, out_file, "w")) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", out_file);
				GMT_exit (EXIT_FAILURE);
			}
			GMT_report (C, GMT_MSG_NORMAL, "Writing Text Table segment to file %s\n", out_file);
			if (C->current.io.io_header[GMT_OUT]) for (k = 0; k < table->n_headers; k++) GMT_write_tableheader (C, fp, table->header[k]);
		}
		if (C->current.io.multi_segments[GMT_OUT]) {	/* Want to write segment headers */
			if (table->segment[seg]->header) strcpy (C->current.io.segment_header, table->segment[seg]->header);
			GMT_write_segmentheader (C, fp, 0);
		}
		if (table->segment[seg]->mode == GMT_WRITE_HEADER) continue;	/* Skip after writing segment header */
		for (row = 0; row < table->segment[seg]->n_rows; row++) {
			GMT_fputs (table->segment[seg]->record[row], fp);
			GMT_fputs ("\n", fp);
		}
		if (io_mode == GMT_WRITE_SEGMENTS) GMT_fclose (C, fp);	/* Close the segment file */
	}

	if (close_file) GMT_fclose (C, fp);	/* Close the file since we opened it */

	return (0);	/* OK status */
}

GMT_LONG GMT_write_textset (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TEXTSET *D, GMT_LONG table)
{	/* Writes an entire text set to file or stream */
	GMT_LONG tbl, error, append = 0, close_file = FALSE;
	int *fd = NULL;	/* Must be int, not GMT_LONG */
	char file[GMT_BUFSIZ], tmpfile[GMT_BUFSIZ], *out_file = tmpfile;
	FILE *fp = NULL;

	/* Convert any destination type to stream */

	if (dest_type == GMT_IS_FILE && dest && ((char *)dest)[0] == '>') append = 1;	/* Want to append to existing file */

	switch (dest_type) {
		case GMT_IS_FILE:	/* dest is a file name */
			strcpy (file, dest);
			if (D->io_mode < GMT_WRITE_TABLES) {	/* Only need one destination */
				if ((fp = GMT_fopen (C, &file[append], (append) ? "a" : "w")) == NULL) {
					GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", &file[append]);
					return (EXIT_FAILURE);
				}
				close_file = TRUE;	/* We only close files we have opened here */
				GMT_report (C, GMT_MSG_NORMAL, "Write Text Table to file %s\n", &file[append]);
			}
			break;
		case GMT_IS_STREAM:	/* Open file pointer given, just copy */
			fp = (FILE *)dest;
			if (fp == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output stream>");
			GMT_report (C, GMT_MSG_NORMAL, "Write Text Table to %s\n", file);
			break;
		case GMT_IS_FDESC:		/* Open file descriptor given, just convert to file pointer */
			fd = dest;
			if (fd && (fp = fdopen (*fd, "w")) == NULL) {
				GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_write_textset\n", *fd);
				return (EXIT_FAILURE);
			}
			if (fd == NULL) fp = C->session.std[GMT_OUT];	/* Default destination */
			if (fp == C->session.std[GMT_OUT])
				strcpy (file, "<stdout>");
			else
				strcpy (file, "<output file descriptor>");
			GMT_report (C, GMT_MSG_NORMAL, "Write Text Table to %s\n", file);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_write_textset\n", dest_type);
			return (EXIT_FAILURE);
			break;
	}

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		if (table != GMTAPI_NOTSET && table != tbl) continue;	/* Selected a specific table */
		if (D->io_mode > GMT_WRITE_TABLES) {	/* Must pass original file name in case a template */
			if ((error = gmt_write_texttable (C, dest, GMT_IS_FILE, D->table[tbl], D->io_mode))) return (error);
		}
		else if (D->io_mode == GMT_WRITE_TABLES) {	/* Must write this table a its own file */
			if (D->table[tbl]->file[GMT_OUT])
				out_file = D->table[tbl]->file[GMT_OUT];
			else
				sprintf (tmpfile, file, D->table[tbl]->id);
			GMT_report (C, GMT_MSG_NORMAL, "Write Text Table to file %s\n", out_file);
			if ((error = gmt_write_texttable (C, out_file, GMT_IS_FILE, D->table[tbl], D->io_mode))) return (error);
		}
		else {	/* Write to stream we set up earlier */
			if ((error = gmt_write_texttable (C, fp, GMT_IS_STREAM, D->table[tbl], D->io_mode))) return (error);
		}
	}

	if (close_file) GMT_fclose (C, fp);

	return (0);	/* OK status */
}

void gmt_adjust_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, GMT_LONG n_columns)
{	/* Change the number of columns in this segment to n_columns (free or allocate as needed) */
	GMT_LONG col;
	for (col = n_columns; col < S->n_columns; col++) GMT_free (C, S->coord[col]);	/* Free up if n_columns < S->columns */
	S->coord = GMT_memory (C, S->coord, n_columns, double *);
	S->min = GMT_memory (C, S->min, n_columns, double);
	S->max = GMT_memory (C, S->max, n_columns, double);
	for (col = S->n_columns; col < n_columns; col++) {	/* Allocate new columns and initialize the min/max arrays */
		S->min[col] = +DBL_MAX;
		S->max[col] = -DBL_MAX;
		S->coord[col] = GMT_memory (C, NULL, S->n_rows, double);
	}
	S->n_columns = n_columns;
}

void gmt_adjust_table (struct GMT_CTRL *C, struct GMT_TABLE *T, GMT_LONG n_columns)
{
	/* Let table have n_columns (so either deallocate or allocate columns). */
	GMT_LONG seg;

	T->min = GMT_memory (C, T->min, n_columns, double);
	T->max = GMT_memory (C, T->max, n_columns, double);
	for (seg = 0; seg < T->n_segments; seg++) gmt_adjust_segment (C, T->segment[seg], n_columns);
	T->n_columns = n_columns;	/* New number of n_columns */
}

void GMT_adjust_dataset (struct GMT_CTRL *C, struct GMT_DATASET *D, GMT_LONG n_columns)
{
	/* Adjust existing data set structure to have n_columns instead.  This may
	 * involve shrinking (deallocation) or growing (allocation) of columns.
	 */
	GMT_LONG tbl;

	for (tbl = 0; tbl < D->n_tables; tbl++) gmt_adjust_table (C, D->table[tbl], n_columns);
	D->n_columns = n_columns;
}

struct GMT_TEXTSET * GMT_create_textset (struct GMT_CTRL *C, GMT_LONG n_tables, GMT_LONG n_segments, GMT_LONG n_rows)
{	/* Create an empty text set structure with the required number of empty tables, all set to hold n_segments with n_rows */
	/* Allocate the new textset structure given the specified dimensions.
	 * If n_segments or n_rows are negative we use the abs value to allocate
	 * and set the n_alloc values but we do NOT set the corresponding
	 * counters (i.e., n_rows, n_segments).  */
	GMT_LONG tbl, seg;
	struct GMT_TEXT_TABLE *T = NULL;
	struct GMT_TEXTSET *D = NULL;
	
	D = GMT_memory (C, NULL, 1, struct GMT_TEXTSET);
	D->table = GMT_memory (C, NULL, n_tables, struct GMT_TEXT_TABLE *);
	D->n_tables = D->n_alloc = n_tables;
	if (n_segments > 0) D->n_segments = n_tables * n_segments;
	for (tbl = 0; tbl < n_tables; tbl++) {
		D->table[tbl] = GMT_memory (C, NULL, 1, struct GMT_TEXT_TABLE);
		T = D->table[tbl];
		T->n_alloc = GMT_abs (n_segments);
		T->segment = GMT_memory (C, NULL, T->n_alloc, struct GMT_TEXT_SEGMENT *);
		if (n_segments > 0) T->n_segments = n_segments;
		for (seg = 0; seg < T->n_segments; seg++) {
			T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_TEXT_SEGMENT);
			T->segment[seg]->record = GMT_memory (C, NULL, n_rows, char *);
			T->segment[seg]->n_alloc = n_rows;
		}
	}
	D->alloc_mode = GMT_ALLOCATED;	/* So GMT_* modules can free this memory. */
	
	return (D);
}

struct GMT_TEXT_TABLE * gmt_alloc_texttable (struct GMT_CTRL *C, struct GMT_TEXT_TABLE *Tin)
{
	/* Allocate the new Text Table structure with same # of segments and rows/segment as input table. */
	GMT_LONG seg, hdr;
	struct GMT_TEXT_TABLE *T = GMT_memory (C, NULL, 1, struct GMT_TEXT_TABLE);
	
	T->n_segments = T->n_alloc = Tin->n_segments;	/* Same number of segments as input table */
	T->n_records  = Tin->n_records;		/* Same number of records as input table */
	T->n_headers  = Tin->n_headers;
	if (T->n_headers) {
		T->header = GMT_memory (C, NULL, Tin->n_headers, char *);
		for (hdr = 0; hdr < T->n_headers; hdr++) T->header[hdr] = strdup (Tin->header[hdr]);
	}
	T->segment = GMT_memory (C, NULL, Tin->n_segments, struct GMT_TEXT_SEGMENT *);
	for (seg = 0; seg < T->n_segments; seg++) {
		T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_TEXT_SEGMENT);
		T->segment[seg]->record = GMT_memory (C, NULL, Tin->segment[seg]->n_rows, char *);
		T->segment[seg]->n_rows = T->segment[seg]->n_alloc = Tin->segment[seg]->n_rows;
		if (Tin->segment[seg]->header) T->segment[seg]->header = strdup (Tin->segment[seg]->header);
	}
	return (T);
}

struct GMT_TEXTSET * GMT_alloc_textset (struct GMT_CTRL *C, struct GMT_TEXTSET *Din, GMT_LONG mode)
{
	/* Allocate new textset structure with same # of tables, segments and rows/segment as input data set.
	 * We copy over headers and segment headers.
	 * mode controls how the new dataset is to be allocated;
	 * mode = GMT_ALLOC_NORMAL means we replicate the number of tables and the layout of the Din dataset
	 * mode = GMT_ALLOC_VERTICAL means we concatenate all the tables in Din into a single table for Dout
	 * mode = GMT_ALLOC_HORIZONTAL means we base the Dout size only on the first Din table
	 *	(# of segments, # of rows/segment) because tables will be pasted horizontally and not vertically.
	 */
	GMT_LONG tbl, seg, hdr, n_seg, seg_in_tbl;
	size_t len;
	struct GMT_TEXTSET *D = GMT_memory (C, NULL, 1, struct GMT_TEXTSET);
	
	if (mode) {	/* Pack everything into a single table */
		D->n_tables = D->n_alloc = 1;
		if (mode == 1)
			for (tbl = n_seg = 0; tbl < Din->n_tables; tbl++) n_seg += Din->table[tbl]->n_segments;
		else
			n_seg = Din->table[0]->n_segments;
		D->table = GMT_memory (C, NULL, 1, struct GMT_TEXT_TABLE *);
		D->table[0] = GMT_memory (C, NULL, 1, struct GMT_TEXT_TABLE);

		/* As for file headers we concatenate the headers from all tables */
		D->table[0]->n_headers  = Din->table[0]->n_headers;
		if (D->table[0]->n_headers) D->table[0]->header = GMT_memory (C, NULL, D->table[0]->n_headers, char *);
		for (hdr = 0; hdr < D->table[0]->n_headers; hdr++) {	/* Concatenate headers */
			for (tbl = len = 0; tbl < Din->n_tables; tbl++) len += (strlen (Din->table[tbl]->header[hdr]) + 2);
			D->table[0]->header[hdr] = calloc (len, sizeof (char));
			strcpy (D->table[0]->header[hdr], Din->table[0]->header[hdr]);
			if (Din->n_tables > 1) GMT_chop (D->table[0]->header[hdr]);	/* Remove newline */
			for (tbl = 1; tbl < Din->n_tables; tbl++) {	/* Now go across tables to paste */
				if (tbl < (Din->n_tables - 1)) GMT_chop (Din->table[tbl]->header[hdr]);
				strcat (D->table[0]->header[hdr], "\t");
				strcat (D->table[0]->header[hdr], Din->table[tbl]->header[hdr]);
			}
		}

		D->n_segments = D->table[0]->n_segments = D->table[0]->n_alloc = n_seg;
		D->table[0]->segment = GMT_memory (C, NULL, n_seg, struct GMT_TEXT_SEGMENT *);
		for (seg = tbl = seg_in_tbl = 0; seg < D->n_segments; seg++) {
			if (seg == Din->table[tbl]->n_segments) { tbl++; seg_in_tbl = 0; }	/* Go to next table */
			D->table[0]->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_TEXT_SEGMENT);
			D->table[0]->segment[seg]->n_rows = Din->table[tbl]->segment[seg_in_tbl]->n_rows;
			D->table[0]->segment[seg]->record = GMT_memory (C, NULL, D->table[0]->segment[seg]->n_rows, char *);
			if (mode == 1 && Din->table[tbl]->segment[seg_in_tbl]->header) D->table[0]->segment[seg]->header = strdup (Din->table[tbl]->segment[seg_in_tbl]->header);
			seg_in_tbl++;
		}
	}
	else {	/* Just copy over the same dataset layout except for columns */
		D->n_tables = D->n_alloc = Din->n_tables;		/* Same number of tables as input dataset */
		D->n_segments  = Din->n_segments;	/* Same number of segments as input dataset */
		D->n_records  = Din->n_records;		/* Same number of records as input dataset */
		D->table = GMT_memory (C, NULL, D->n_tables, struct GMT_TEXT_TABLE *);
		for (tbl = 0; tbl < D->n_tables; tbl++) D->table[tbl] = gmt_alloc_texttable (C, Din->table[tbl]);
	}
	return (D);
}

struct GMT_TEXTSET * GMT_duplicate_textset (struct GMT_CTRL *C, struct GMT_TEXTSET *Din, GMT_LONG mode)
{
	GMT_LONG tbl, seg, row;
	struct GMT_TEXTSET *D = NULL;
	D = GMT_alloc_textset (C, Din, mode);
	for (tbl = 0; tbl < Din->n_tables; tbl++) for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
		for (row = 0; row < Din->table[tbl]->segment[seg]->n_rows; row++) D->table[tbl]->segment[seg]->record[row] = strdup (Din->table[tbl]->segment[seg]->record[row]);
	}
	return (D);
}

struct GMT_TABLE * gmt_alloc_table (struct GMT_CTRL *C, struct GMT_TABLE *Tin, GMT_LONG n_columns, GMT_LONG n_rows)
{
	/* Allocate the new Table structure with same # of segments and rows/segment as input table.
	 * However, n_columns is given separately and could differ.
	 * If n_rows is > 0 we well override the Tin rows counts by using n_rows instead.  */
	GMT_LONG seg, hdr, nr;
	struct GMT_TABLE *T = GMT_memory (C, NULL, 1, struct GMT_TABLE);
	
	T->n_segments = T->n_alloc = Tin->n_segments;	/* Same number of segments as input table */
	T->n_headers  = Tin->n_headers;
	T->n_columns  = n_columns;		/* Separately specified n_columns */
	T->min = GMT_memory (C, NULL, n_columns, double);
	T->max = GMT_memory (C, NULL, n_columns, double);
	if (T->n_headers) {
		T->header = GMT_memory (C, NULL, Tin->n_headers, char *);
		for (hdr = 0; hdr < T->n_headers; hdr++) T->header[hdr] = strdup (Tin->header[hdr]);
	}
	T->segment = GMT_memory (C, NULL, Tin->n_segments, struct GMT_LINE_SEGMENT *);
	for (seg = 0; seg < T->n_segments; seg++) {
		T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
		nr = (n_rows) ? n_rows : Tin->segment[seg]->n_rows;
		GMT_alloc_segment (C, T->segment[seg], nr, n_columns, TRUE);
		T->segment[seg]->n_rows = nr;
		T->segment[seg]->n_columns = n_columns;
		T->n_records += nr;
		if (Tin->segment[seg]->header) T->segment[seg]->header = strdup (Tin->segment[seg]->header);
	}
	return (T);
}

GMT_LONG GMT_alloc_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, GMT_LONG n_rows, GMT_LONG n_columns, GMT_LONG first)
{	/* (re)allocates memory for a segment of given dimensions.
 	 * If n_rows is negative then we do not set S->n_rows.  */
	GMT_LONG col;
	if (first && n_columns) {	/* First time we allocate the number of columns needed */
		S->coord = GMT_memory (C, NULL, n_columns, double *);
		S->min = GMT_memory (C, NULL, n_columns, double);
		S->max = GMT_memory (C, NULL, n_columns, double);
		S->n_columns = n_columns;
		for (col = 0; col < n_columns; col++) {	/* Initialize the min/max array */
			S->min[col] = +DBL_MAX;
			S->max[col] = -DBL_MAX;
		}
	}
	if (n_rows > 0) S->n_rows = n_rows;
	n_rows = GMT_abs (n_rows);
	S->n_alloc = n_rows;
	if (n_rows) for (col = 0; col < n_columns; col++) S->coord[col] = GMT_memory (C, S->coord[col], n_rows, double);
	return (GMT_OK);
}

struct GMT_TABLE * GMT_create_table (struct GMT_CTRL *C, GMT_LONG n_segments, GMT_LONG n_columns, GMT_LONG n_rows)
{
	/* Allocate the new Table structure given the specified dimensions.
	 * If n_columns == 0 it means we don't know that dimension yet.
	 * If n_segments or n_rows are negative we use the abs value to allocate
	 * and set the n_alloc values but we do NOT set the corresponding
	 * counters (i.e., n_rows, n_segments).  */
	GMT_LONG seg;
	struct GMT_TABLE *T = NULL;
	
	T = GMT_memory (C, NULL, 1, struct GMT_TABLE);
	if (n_segments > 0) T->n_segments = n_segments;
	n_segments = GMT_abs (n_segments);
	T->n_alloc = n_segments;
	T->n_columns = n_columns;
	if (n_columns) {
		T->min = GMT_memory (C, NULL, n_columns, double);
		T->max = GMT_memory (C, NULL, n_columns, double);
	}
	if (n_segments) {
		T->segment = GMT_memory (C, NULL, n_segments, struct GMT_LINE_SEGMENT *);
		for (seg = 0; n_columns && seg < n_segments; seg++) {
			T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
			if (GMT_alloc_segment (C, T->segment[seg], n_rows, n_columns, TRUE)) return (NULL);
		}
	}
	
	return (T);
}

struct GMT_DATASET * GMT_create_dataset (struct GMT_CTRL *C, GMT_LONG n_tables, GMT_LONG n_segments, GMT_LONG n_columns, GMT_LONG n_rows)
{	/* Create an empty data set structure with the required number of empty tables, all set to hold n_segments with n_columns */
	GMT_LONG tbl;
	struct GMT_DATASET *D = NULL;
	
	D = GMT_memory (C, NULL, 1, struct GMT_DATASET);
	D->n_columns = n_columns;
	if (n_columns) {
		D->min = GMT_memory (C, NULL, n_columns, double);
		D->max = GMT_memory (C, NULL, n_columns, double);
	}
	D->table = GMT_memory (C, NULL, n_tables, struct GMT_TABLE *);
	D->n_tables = D->n_alloc = n_tables;
	if (n_segments > 0) D->n_segments = D->n_tables * n_segments;
	if (n_segments > 0) D->n_records = D->n_segments * n_rows;
	for (tbl = 0; tbl < n_tables; tbl++) if ((D->table[tbl] = GMT_create_table (C, n_segments, n_columns, n_rows)) == NULL) return (NULL);
	D->alloc_mode = GMT_ALLOCATED;	/* So GMT_* modules can free this memory. */
	
	return (D);
}

struct GMT_TABLE * GMT_read_table (struct GMT_CTRL *C, void *source, GMT_LONG source_type, GMT_LONG greenwich, GMT_LONG poly, GMT_LONG use_GMT_io)
{
	/* Reads an entire data set into a single table memory with any number of segments */

	GMT_LONG ascii, close_file = FALSE, header = TRUE, no_segments, n_head_alloc = GMT_TINY_CHUNK;
	GMT_LONG n_fields, n_expected_fields, k, n_read = 0, seg = (GMT_LONG)(-1), row = 0, col, n_row_alloc;
	char open_mode[4], file[GMT_BUFSIZ], line[GMT_TEXT_LEN64];
	double d, *in = NULL;
	FILE *fp = NULL;
	struct GMT_TABLE *T = NULL;
	PFP psave = NULL;

	if (use_GMT_io) {	/* Use C->current.io.info settings to determine if input is ascii/binary, else it defaults to ascii */
		n_expected_fields = C->common.b.active[GMT_IN] ? C->common.b.ncol[GMT_IN] : GMT_MAX_COLUMNS;
		strcpy (open_mode, C->current.io.r_mode);
		ascii = !C->common.b.active[GMT_IN];
	}
	else {			/* Force ASCII mode */
		n_expected_fields = GMT_MAX_COLUMNS;	/* C->current.io.input will return the number of columns */
		strcpy (open_mode, "r");
		ascii = TRUE;
		psave = C->current.io.input;			/* Save the previous pointer since we need to change it back at the end */
		C->current.io.input = C->session.input_ascii;	/* Override and use ascii mode */
	}

#ifdef SET_IO_MODE
	if (!ascii) GMT_setmode (C, GMT_IN);
#endif

	/* Determine input source */

	if (source_type == GMT_IS_FILE) {	/* source is a file name */
		strcpy (file, source);
		if ((fp = GMT_fopen (C, file, open_mode)) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot open file %s\n", file);
			if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (source_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)source;
		if (fp == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input stream>");
	}
	else if (source_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd = source;
		if (fd && (fp = fdopen (*fd, open_mode)) == NULL) {
			GMT_report (C, GMT_MSG_FATAL, "Cannot convert file descriptor %d to stream in GMT_read_table\n", *fd);
			if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}
		if (fd == NULL) fp = C->session.std[GMT_IN];	/* Default input */
		if (fp == C->session.std[GMT_IN])
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input file descriptor>");
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "Unrecognized source type %ld in GMT_read_table\n", source_type);
		if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
		return (NULL);
	}

	in = C->current.io.input (C, fp, &n_expected_fields, &n_fields);	/* Get first record */
	n_read++;
	if (GMT_REC_IS_EOF(C)) {
		GMT_report (C, GMT_MSG_NORMAL, "File %s is empty!\n", file);
		if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
		return (NULL);
	}
	/* Allocate the Table structure with GMT_CHUNK segments, but none has any rows or columns */

	T = GMT_create_table (C, GMT_CHUNK, 0, 0);

	T->file[GMT_IN] = strdup (file);
	T->header = GMT_memory (C, NULL, n_head_alloc, char *);
	n_row_alloc = GMT_CHUNK;	/* Initial space allocated for rows in the current segment. Since allcoation/reallocation is
					 * expensive we will set n_row_alloc to the size of the previous segment */

	while (n_fields >= 0 && !GMT_REC_IS_EOF (C)) {	/* Not yet EOF */
		while (header && ((C->current.io.io_header[GMT_IN] && n_read <= C->current.io.io_n_header_items) || GMT_REC_IS_TBL_HEADER (C))) { /* Process headers */
			T->header[T->n_headers] = strdup (C->current.io.current_record);
			T->n_headers++;
			if (T->n_headers == n_head_alloc) {
				n_head_alloc <<= 1;
				T->header = GMT_memory (C, T->header, n_head_alloc, char *);
			}
			in = C->current.io.input (C, fp, &n_expected_fields, &n_fields);
			n_read++;
		}
		header = FALSE;	/* Done processing header block; other comments are GIS/OGR encoded comments */

		if (T->n_headers)
			T->header = GMT_memory (C, T->header, T->n_headers, char *);
		else {	/* No header records found */
			GMT_free (C, T->header);
			T->header = NULL;
		}
		if (GMT_REC_IS_EOF (C)) continue;	/* Got EOF after headers */

		no_segments = !GMT_REC_IS_SEG_HEADER (C);	/* Not a multi-segment file.  We then assume file has only one segment */

		while (no_segments || (GMT_REC_IS_SEG_HEADER (C) && !GMT_REC_IS_EOF (C))) {
			/* To use different line-distances for each segment, place the distance in the segment header */
			if (seg == -1 || T->segment[seg]->n_rows > 0) {
				seg++;	/* Only advance segment if last had any points or was the first one */
				T->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
			}
			n_read++;
			if (ascii && !no_segments) {	/* Only ascii files can have info stored in multi-seg header records */
				if (GMT_parse_segment_item (C, C->current.io.segment_header, "-D", line)) {	/* Found a potential -D<dist> option in the header */
					if (sscanf (line, "%lg", &d) == 1) T->segment[seg]->dist = d;	/* If readable, assign it to dist, else leave as zero */
				}
			}
			/* Segment initialization */
			row = 0;
			if (!no_segments) {	/* Read data if we read a segment header up front, but guard against headers which sets in = NULL */
				while (!GMT_REC_IS_EOF (C) && (in = C->current.io.input (C, fp, &n_expected_fields, &n_fields)) == NULL) n_read++;
			}
			T->segment[seg]->n_columns = n_expected_fields;
			no_segments = FALSE;	/* This has now served its purpose */
		}
		if (GMT_REC_IS_EOF (C)) continue;	/* At EOF; get out of this loop */
		if (ascii && !no_segments) {	/* Only ascii files can have info stored in multi-seg header record */
			char buffer[GMT_BUFSIZ];
			if (strlen (C->current.io.segment_header)) {
				GMT_memset (buffer, GMT_BUFSIZ, char);
				T->segment[seg]->header = strdup (C->current.io.segment_header);
				if (GMT_parse_segment_item (C, C->current.io.segment_header, "-L", buffer)) T->segment[seg]->label = strdup (buffer);
			}
			if (C->current.io.ogr == 1) gmt_copy_ogr_seg (C, T->segment[seg], C->current.io.OGR);	/* Copy over any feature-specific values */
		}

		if (poly && T->segment[seg]->n_columns < 2) {
			GMT_report (C, GMT_MSG_FATAL, "File %s does not have at least 2 columns required for polygons (found %ld)\n", file, T->segment[seg]->n_columns);
			if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
			return (NULL);
		}
		GMT_alloc_segment (C, T->segment[seg], n_row_alloc, T->segment[seg]->n_columns, TRUE);	/* Alloc space for this segment with n_row_alloc rows */

		while (! (C->current.io.status & (GMT_IO_SEG_HEADER | GMT_IO_GAP | GMT_IO_EOF))) {	/* Keep going until FALSE or find a new segment header */
			if (C->current.io.status & GMT_IO_MISMATCH) {
				GMT_report (C, GMT_MSG_FATAL, "Mismatch between actual (%ld) and expected (%ld) fields near line %ld\n", n_fields, n_expected_fields, n_read);
				if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */
				return (NULL);
			}

			if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) {
				if (greenwich && T->segment[seg]->coord[GMT_X][row] > 180.0) T->segment[seg]->coord[GMT_X][row] -= 360.0;
				if (!greenwich && T->segment[seg]->coord[GMT_X][row] < 0.0)  T->segment[seg]->coord[GMT_X][row] += 360.0;
			}
			for (k = 0; k < T->segment[seg]->n_columns; k++) {
				T->segment[seg]->coord[k][row] = in[k];
				if (T->segment[seg]->coord[k][row] < T->segment[seg]->min[k]) T->segment[seg]->min[k] = T->segment[seg]->coord[k][row];
				if (T->segment[seg]->coord[k][row] > T->segment[seg]->max[k]) T->segment[seg]->max[k] = T->segment[seg]->coord[k][row];
			}

			row++;
			if (row == (T->segment[seg]->n_alloc-1)) {	/* -1 because we may have to close the polygon and hence need 1 more cell */
				T->segment[seg]->n_alloc <<= 1;
				GMT_alloc_segment (C, T->segment[seg], T->segment[seg]->n_alloc, T->segment[seg]->n_columns, FALSE);
			}
			in = C->current.io.input (C, fp, &n_expected_fields, &n_fields);
			while (GMT_REC_IS_TBL_HEADER (C)) in = C->current.io.input (C, fp, &n_expected_fields, &n_fields);	/* Just wind past other comments */
			n_read++;
		}
		T->segment[seg]->n_rows = row;	/* Number of records in this segment */
		T->n_records += row;		/* Total number of records so far */
		T->segment[seg]->id = seg;	/* Internal segment number */

		GMT_set_seg_minmax (C, T->segment[seg]);
		if (poly) {	/* If file contains a polygon then we must close it if needed */
			if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) {	/* Must check for polar cap */
				double dlon;
				dlon = T->segment[seg]->coord[GMT_X][0] - T->segment[seg]->coord[GMT_X][row-1];
				if (!((fabs (dlon) == 0.0 || fabs (dlon) == 360.0) && T->segment[seg]->coord[GMT_Y][0] == T->segment[seg]->coord[GMT_Y][row-1])) {
					for (k = 0; k < T->segment[seg]->n_columns; k++) T->segment[seg]->coord[k][row] = T->segment[seg]->coord[k][0];
					T->segment[seg]->n_rows++;	/* Explicitly close polygon */
				}
				GMT_set_seg_polar (C, T->segment[seg]);
			}
			else if (GMT_polygon_is_open (C, T->segment[seg]->coord[GMT_X], T->segment[seg]->coord[GMT_Y], row)) {	/* Cartesian closure */
				for (k = 0; k < T->segment[seg]->n_columns; k++) T->segment[seg]->coord[k][row] = T->segment[seg]->coord[k][0];
				T->segment[seg]->n_rows++;
			}
			if (GMT_parse_segment_item (C, T->segment[seg]->header, "-Ph", NULL)) T->segment[seg]->pol_mode = GMT_IS_HOLE;
			/* If this is a hole then set link from previous segment to this one */
			if (seg && GMT_polygon_is_hole (T->segment[seg])) T->segment[seg-1]->next = T->segment[seg];
		}
		
		/* Reallocate to free up some memory */

		if (n_row_alloc > T->segment[seg]->n_rows) GMT_alloc_segment (C, T->segment[seg], T->segment[seg]->n_rows, T->segment[seg]->n_columns, FALSE);
		n_row_alloc = MAX (2, T->segment[seg]->n_rows);	/* Reset initial allocation size to match last segment, except no smaller than 2 due to test on T->n_alloc -1 below */
		if (T->segment[seg]->n_rows == 0) {	/* Empty segment; we delete to avoid problems downstream in applications */
			GMT_free (C, T->segment[seg]);
			seg--;	/* Go back to where we were */
		}

		if (seg == (T->n_alloc-1)) {	/* Need to allocate more segments */
			GMT_LONG n_old_alloc = T->n_alloc;
			T->n_alloc <<= 1;
			T->segment = GMT_memory (C, T->segment, T->n_alloc, struct GMT_LINE_SEGMENT *);
			GMT_memset (&(T->segment[n_old_alloc]), T->n_alloc - n_old_alloc, struct GMT_LINE_SEGMENT *);	/* Set to NULL */
		}

		/* If a gap was detected, forget about it now, so we can use the data for the next segment */

		C->current.io.status -= (C->current.io.status & GMT_IO_GAP);
	}
	if (close_file) GMT_fclose (C, fp);
	if (!use_GMT_io) C->current.io.input = psave;	/* Restore previous setting */

	if (seg == -1) {	/* Never saw any segment or data records */
		GMT_free_table (C, T);
		return (NULL);
	}
	if (T->segment[seg]->n_rows == 0)	/* Last segment was empty; we delete to avoid problems downstream in applications */
		GMT_free (C, T->segment[seg]);
	else
		seg++;
	T->segment = GMT_memory (C, T->segment, seg, struct GMT_LINE_SEGMENT *);
	T->n_segments = seg;
	T->n_columns = T->segment[0]->n_columns;
	/* Determine table min,max values */
	T->min = GMT_memory (C, NULL, T->n_columns, double);
	T->max = GMT_memory (C, NULL, T->n_columns, double);
	for (col = 0; col < T->n_columns; col++) {T->min[col] = DBL_MAX; T->max[col] = -DBL_MAX;}
	for (seg = 0; seg < T->n_segments; seg++) {
		for (col = 0; col < T->n_columns; col++) {
			T->min[col] = MIN (T->min[col], T->segment[seg]->min[col]);
			T->max[col] = MAX (T->max[col], T->segment[seg]->max[col]);
		}
		if (T->segment[seg]->pole) {T->min[GMT_X] = 0.0; T->max[GMT_X] = 360.0;}
	}

	return (T);
}

void GMT_copy_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *Sout, struct GMT_LINE_SEGMENT *Sin)
{	/* Duplicates the segment */
	GMT_LONG col;
	for (col = 0; col < Sin->n_columns; col++) GMT_memcpy (Sout->coord[col], Sin->coord[col], Sin->n_rows, double);
	GMT_memcpy (Sout->min, Sin->min, Sin->n_columns, double);
	GMT_memcpy (Sout->max, Sin->max, Sin->n_columns, double);
	Sout->n_rows = Sin->n_rows;
}

struct GMT_LINE_SEGMENT * GMT_duplicate_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *Sin)
{	/* Duplicates the segment */
	GMT_LONG col;
	struct GMT_LINE_SEGMENT *Sout = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
	GMT_alloc_segment (C, Sout, Sin->n_rows, Sin->n_columns, TRUE);
	for (col = 0; col < Sin->n_columns; col++) GMT_memcpy (Sout->coord[col], Sin->coord[col], Sin->n_rows, double);
	Sout->n_rows = Sin->n_rows;
	return (Sout);
}

struct GMT_DATASET * GMT_alloc_dataset (struct GMT_CTRL *C, struct GMT_DATASET *Din, GMT_LONG n_columns, GMT_LONG n_rows, GMT_LONG mode)
{
	/* Allocate new dataset structure with same # of tables, segments and rows/segment as input data set.
	 * However, n_columns is given separately and could differ.  Also, if n_rows > 0 we let that override the segment row counts.
	 * We copy over headers and segment headers.
	 * mode controls how the new dataset is to be allocated;
	 * mode = GMT_ALLOC_NORMAL means we replicate the number of tables and the layout of the Din dataset
	 * mode = GMT_ALLOC_VERTICAL means we concatenate all the tables in Din into a single table for Dout
	 * mode = GMT_ALLOC_HORIZONTAL means we base the Dout size only on the first Din table
	 *	(# of segments, # of rows/segment) because tables will be pasted horizontally and not vertically.
	 */
	GMT_LONG tbl, seg, hdr, nr, n_seg, seg_in_tbl;
	size_t len;
	struct GMT_DATASET *D = GMT_memory (C, NULL, 1, struct GMT_DATASET);
	
	D->n_columns = (n_columns) ? n_columns : Din->n_columns;
	D->min = GMT_memory (C, NULL, D->n_columns, double);
	D->max = GMT_memory (C, NULL, D->n_columns, double);
	if (mode) {	/* Pack everything into a single table */
		D->n_tables = D->n_alloc = 1;
		if (mode == 1)
			for (tbl = n_seg = 0; tbl < Din->n_tables; tbl++) n_seg += Din->table[tbl]->n_segments;
		else
			n_seg = Din->table[0]->n_segments;
		D->table = GMT_memory (C, NULL, 1, struct GMT_TABLE *);
		D->table[0] = GMT_memory (C, NULL, 1, struct GMT_TABLE);

		/* As for file headers we concatenate the headers from all tables */
		D->table[0]->n_headers  = Din->table[0]->n_headers;
		if (D->table[0]->n_headers) D->table[0]->header = GMT_memory (C, NULL, D->table[0]->n_headers, char *);
		for (hdr = 0; hdr < D->table[0]->n_headers; hdr++) {	/* Concatenate headers */
			for (tbl = len = 0; tbl < Din->n_tables; tbl++) len += (strlen (Din->table[tbl]->header[hdr]) + 2);
			D->table[0]->header[hdr] = calloc (len, sizeof (char));
			strcpy (D->table[0]->header[hdr], Din->table[0]->header[hdr]);
			if (Din->n_tables > 1) GMT_chop (D->table[0]->header[hdr]);	/* Remove newline */
			for (tbl = 1; tbl < Din->n_tables; tbl++) {	/* Now go across tables to paste */
				if (tbl < (Din->n_tables - 1)) GMT_chop (Din->table[tbl]->header[hdr]);
				strcat (D->table[0]->header[hdr], "\t");
				strcat (D->table[0]->header[hdr], Din->table[tbl]->header[hdr]);
			}
		}

		D->n_segments = D->table[0]->n_segments = D->table[0]->n_alloc = n_seg;
		D->table[0]->n_columns = D->n_columns;
		D->table[0]->segment = GMT_memory (C, NULL, n_seg, struct GMT_LINE_SEGMENT *);
		for (seg = tbl = seg_in_tbl = 0; seg < D->n_segments; seg++) {
			if (seg == Din->table[tbl]->n_segments) { tbl++; seg_in_tbl = 0; }	/* Go to next table */
			D->table[0]->segment[seg] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
			nr = (n_rows) ? n_rows : Din->table[tbl]->segment[seg_in_tbl]->n_rows;
			D->table[0]->segment[seg]->n_rows = nr;
			GMT_alloc_segment (C, D->table[0]->segment[seg], nr, D->n_columns, TRUE);
			D->table[0]->segment[seg]->n_columns = D->n_columns;
			if (mode == 1 && Din->table[tbl]->segment[seg_in_tbl]->header) D->table[0]->segment[seg]->header = strdup (Din->table[tbl]->segment[seg_in_tbl]->header);
			D->n_records += nr;
			seg_in_tbl++;
		}
	}
	else {	/* Just copy over the same dataset layout except for columns */
		D->n_tables = D->n_alloc  = Din->n_tables;		/* Same number of tables as input dataset */
		D->n_segments  = Din->n_segments;	/* Same number of segments as input dataset */
		D->n_records  = Din->n_records;		/* Same number of records as input dataset */
		D->table = GMT_memory (C, NULL, D->n_tables, struct GMT_TABLE *);
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			D->table[tbl] = gmt_alloc_table (C, Din->table[tbl], D->n_columns, n_rows);
		}
	}
	return (D);
}

struct GMT_DATASET * GMT_duplicate_dataset (struct GMT_CTRL *C, struct GMT_DATASET *Din, GMT_LONG n_columns, GMT_LONG mode)
{	/* Make an exact replica */
	GMT_LONG tbl, seg;
	struct GMT_DATASET *D = NULL;
	D = GMT_alloc_dataset (C, Din, n_columns, 0, mode);
	GMT_memcpy (D->min, Din->min, Din->n_columns, double);
	GMT_memcpy (D->max, Din->max, Din->n_columns, double);
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			GMT_copy_segment (C, D->table[tbl]->segment[seg], Din->table[tbl]->segment[seg]);
		}
		GMT_memcpy (D->table[tbl]->min, Din->table[tbl]->min, Din->table[tbl]->n_columns, double);
		GMT_memcpy (D->table[tbl]->max, Din->table[tbl]->max, Din->table[tbl]->n_columns, double);
	}
	return (D);
}

void gmt_free_ogr_seg (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Frees the OGR structure for a given segment */
	GMT_LONG k, n;
	n = (C->current.io.OGR) ? C->current.io.OGR->n_aspatial : C->common.a.n_aspatial; 
	if (n) {
		for (k = 0; S->ogr->value && k < n; k++) if (S->ogr->value[k]) free (S->ogr->value[k]);
		GMT_free (C, S->ogr->value);
		GMT_free (C, S->ogr->dvalue);
	}
	GMT_free (C, S->ogr);
}

void GMT_free_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *segment)
{
	/* Free memory allocated by GMT_read_table */

	GMT_LONG col, k;
	if (!segment) return;	/* Do not try to free NULL pointer */
	for (col = 0; col < segment->n_columns; col++) GMT_free (C, segment->coord[col]);
	GMT_free (C, segment->coord);
	if (segment->min) GMT_free (C, segment->min);
	if (segment->max) GMT_free (C, segment->max);
	if (segment->label) free ( segment->label);
	if (segment->header) free ( segment->header);
	for (k = 0; k < 2; k++) if (segment->file[k]) free (segment->file[k]);
	if (segment->ogr) gmt_free_ogr_seg (C, segment);	/* OGR metadata */
	GMT_free (C, segment);
}

void GMT_free_table (struct GMT_CTRL *C, struct GMT_TABLE *table)
{
	GMT_LONG k;
	if (!table) return;		/* Do not try to free NULL pointer */
	for (k = 0; k < table->n_headers; k++) free (table->header[k]);
	if (table->n_headers) GMT_free (C, table->header);
	GMT_free (C, table->min);
	GMT_free (C, table->max);
	for (k = 0; k < 2; k++) if (table->file[k]) free (table->file[k]);
	GMT_free_ogr (C, &(table->ogr), 1);
	if (table->segment) {	/* Free segments */
		for (k = 0; k < table->n_segments; k++) GMT_free_segment (C, table->segment[k]);
		GMT_free (C, table->segment);
	}
	GMT_free (C, table);
}

void GMT_free_dataset_ptr (struct GMT_CTRL *C, struct GMT_DATASET *data)
{	/* This takes pointer to data array and thus can return it as NULL */
	GMT_LONG tbl, k;
	if (!data) return;	/* Do not try to free NULL pointer */
	if (!data->table) return;	/* Do not try to free NULL pointer of tables */
	for (tbl = 0; tbl < data->n_tables; tbl++) {
		GMT_free_table (C, data->table[tbl]);
	}
	GMT_free (C, data->min);
	GMT_free (C, data->max);
	GMT_free (C, data->table);
	for (k = 0; k < 2; k++) if (data->file[k]) free (data->file[k]);
}

void GMT_free_dataset (struct GMT_CTRL *C, struct GMT_DATASET **data)
{	/* This takes pointer to data array and thus can return it as NULL */
	GMT_free_dataset_ptr (C, *data);
	GMT_free (C, *data);
}

void gmt_free_textsegment (struct GMT_CTRL *C, struct GMT_TEXT_SEGMENT *segment)
{
	/* Free memory allocated by GMT_read_texttable */

	GMT_LONG k;
	if (!segment) return;	/* Do not try to free NULL pointer */
	for (k = 0; k < segment->n_rows; k++) if (segment->record[k]) free (segment->record[k]);
	GMT_free (C, segment->record);
	if (segment->label) free ( segment->label);
	if (segment->header) free ( segment->header);
	for (k = 0; k < 2; k++) if (segment->file[k]) free (segment->file[k]);
	GMT_free (C, segment);
}

void gmt_free_texttable (struct GMT_CTRL *C, struct GMT_TEXT_TABLE *table)
{
	GMT_LONG k;
	if (!table) return;	/* Do not try to free NULL pointer */
	for (k = 0; k < table->n_segments; k++) gmt_free_textsegment (C, table->segment[k]);
	for (k = 0; k < table->n_headers; k++) free (table->header[k]);
	if (table->n_headers) GMT_free (C, table->header);
	GMT_free (C, table->segment);
	for (k = 0; k < 2; k++) if (table->file[k]) free (table->file[k]);
	GMT_free (C, table);
}

void GMT_free_textset_ptr (struct GMT_CTRL *C, struct GMT_TEXTSET *data)
{	/* This takes pointer to data array and thus can return it as NULL */

	GMT_LONG tbl, k;
	for (tbl = 0; tbl < data->n_tables; tbl++) gmt_free_texttable (C, data->table[tbl]);
	GMT_free (C, data->table);
	for (k = 0; k < 2; k++) if (data->file[k]) free (data->file[k]);
}

void GMT_free_textset (struct GMT_CTRL *C, struct GMT_TEXTSET **data)
{	/* This takes pointer to data array and thus can return it as NULL */

	GMT_free_textset_ptr (C, *data);
	GMT_free (C, *data);
}

#ifdef USE_GDAL
struct GMT_IMAGE *GMT_create_image (struct GMT_CTRL *C)
{	/* Allocates space for a new image container. */
	struct GMT_IMAGE *I = GMT_memory (C, NULL, 1, struct GMT_IMAGE);
	I->header = GMT_memory (C, NULL, 1, struct GRD_HEADER);
	GMT_grd_setpad (C, I->header, C->current.io.pad);	/* Use the system pad setting by default */
	I->header->pocket = NULL;			/* Char pointer to hold whatever we may temporarilly need to store */
	return (I);
}

void GMT_free_image_ptr (struct GMT_CTRL *C, struct GMT_IMAGE *I, GMT_LONG free_image)
{	/* Free contents of image pointer */
	if (!I) return;	/* Nothing to deallocate */
	if (I->data && free_image) GMT_free (C, I->data);
	if (I->header) GMT_free (C, I->header);
	if (I->ColorMap) GMT_free (C, I->ColorMap);
}

void GMT_free_image (struct GMT_CTRL *C, struct GMT_IMAGE **I, GMT_LONG free_image)
{	/* By taking a reference to the image pointer we can set it to NULL when done */
	GMT_free_image_ptr (C, *I, free_image);
	GMT_free (C, *I);
}
#endif

void GMT_free_univector (struct GMT_CTRL *C, union GMT_UNIVECTOR *u, GMT_LONG type)
{	/* By taking a reference to the vector pointer we can set it to NULL when done */
	/* free_vector = FALSE means the vectors are not to be freed but the data array itself will be */
	if (!u) return;	/* Nothing to deallocate */
	switch (type) {
		case GMTAPI_UCHAR:	GMT_free (C, u->uc1); break;
		case GMTAPI_CHAR:	GMT_free (C, u->sc1); break;
		case GMTAPI_USHORT:	GMT_free (C, u->ui2); break;
		case GMTAPI_SHORT:	GMT_free (C, u->si2); break;
		case GMTAPI_UINT:	GMT_free (C, u->ui4); break;
		case GMTAPI_INT:	GMT_free (C, u->si4); break;
		case GMTAPI_ULONG:	GMT_free (C, u->ui8); break;
		case GMTAPI_LONG:	GMT_free (C, u->si8); break;
		case GMTAPI_FLOAT:	GMT_free (C, u->f4);  break;
		case GMTAPI_DOUBLE:	GMT_free (C, u->f8);  break;
	}
}

struct GMT_VECTOR * GMT_create_vector (struct GMT_CTRL *C, GMT_LONG n_columns)
{	/* Allocates space for a new vector container.  No space allocated for the vectors themselves */
	struct GMT_VECTOR *V = NULL;
	
	V = GMT_memory (C, NULL, 1, struct GMT_VECTOR);
	V->data = GMT_memory (C, NULL, n_columns, union GMT_UNIVECTOR);
	V->type = GMT_memory (C, NULL, n_columns, GMT_LONG);
	V->n_columns = n_columns;
	V->alloc_mode = GMT_ALLOCATED;	/* So GMT_* modules can free this memory. */
	
	return (V);
}

GMT_LONG GMT_alloc_univector (struct GMT_CTRL *C, union GMT_UNIVECTOR *u, GMT_LONG type, GMT_LONG n_rows)
{	/* Allocate space for one univector according to data type */
	
	switch (type) {
		case GMTAPI_UCHAR:  u->uc1 = GMT_memory (C, u->uc1, n_rows, unsigned char);		break;
		case GMTAPI_CHAR:   u->sc1 = GMT_memory (C, u->sc1, n_rows, char);			break;
		case GMTAPI_USHORT: u->ui2 = GMT_memory (C, u->ui2, n_rows, unsigned short int);	break;
		case GMTAPI_SHORT:  u->si2 = GMT_memory (C, u->si2, n_rows, short int);			break;
		case GMTAPI_UINT:   u->ui4 = GMT_memory (C, u->ui4, n_rows, unsigned int);		break;
		case GMTAPI_INT:    u->si4 = GMT_memory (C, u->si4, n_rows, int);			break;
		case GMTAPI_ULONG:  u->ui8 = GMT_memory (C, u->ui8, n_rows, GMT_ULONG);			break;
		case GMTAPI_LONG:   u->si8 = GMT_memory (C, u->si8, n_rows, GMT_LONG);			break;
		case GMTAPI_FLOAT:  u->f4  = GMT_memory (C, u->f4,  n_rows, float);			break;
		case GMTAPI_DOUBLE: u->f8  = GMT_memory (C, u->f8,  n_rows, double);			break;
	}
	return (GMT_OK);
}

GMT_LONG GMT_duplicate_univector (struct GMT_CTRL *C, union GMT_UNIVECTOR *u_out, union GMT_UNIVECTOR *u_in, GMT_LONG type, GMT_LONG n_rows)
{	/* Allocate space for one univector according to data type */
	
	switch (type) {
		case GMTAPI_UCHAR:  GMT_memcpy (u_out->uc1, u_in->uc1, n_rows, unsigned char);		break;
		case GMTAPI_CHAR:   GMT_memcpy (u_out->sc1, u_in->sc1, n_rows, char);			break;
		case GMTAPI_USHORT: GMT_memcpy (u_out->ui2, u_in->ui2, n_rows, unsigned short int);	break;
		case GMTAPI_SHORT:  GMT_memcpy (u_out->si2, u_in->si2, n_rows, short int);		break;
		case GMTAPI_UINT:   GMT_memcpy (u_out->ui4, u_in->ui4, n_rows, unsigned int);		break;
		case GMTAPI_INT:    GMT_memcpy (u_out->si4, u_in->si4, n_rows, int);			break;
		case GMTAPI_ULONG:  GMT_memcpy (u_out->ui8, u_in->ui8, n_rows, GMT_ULONG);		break;
		case GMTAPI_LONG:   GMT_memcpy (u_out->si8, u_in->si8, n_rows, GMT_LONG);		break;
		case GMTAPI_FLOAT:  GMT_memcpy (u_out->f4,  u_in->f4,  n_rows, float);			break;
		case GMTAPI_DOUBLE: GMT_memcpy (u_out->f8,  u_in->f8,  n_rows, double);			break;
	}
	return (GMT_OK);
}

GMT_LONG GMT_alloc_vectors (struct GMT_CTRL *C, struct GMT_VECTOR *V, GMT_LONG n_rows)
{	/* Allocate space for each column according to data type */
	GMT_LONG col;
	
	for (col = 0; col < V->n_columns; col++) {
		if (GMT_alloc_univector (C, &V->data[col], V->type[col], n_rows) != GMT_OK) return (GMT_MEMORY_ERROR);
	}
	return (GMT_OK);
}

void GMT_free_vector_ptr (struct GMT_CTRL *C, struct GMT_VECTOR *V, GMT_LONG free_vector)
{	/* By taking a reference to the vector pointer we can set it to NULL when done */
	/* free_vector = FALSE means the vectors are not to be freed but the data array itself will be */
	if (!V) return;	/* Nothing to deallocate */
	if (V->data && free_vector) {
		GMT_LONG col;
		for (col = 0; col < V->n_columns; col++) GMT_free_univector (C, &(V->data[col]), V->type[col]);
	}
	GMT_free (C, V->data);
	GMT_free (C, V->type);
}

void GMT_free_vector (struct GMT_CTRL *C, struct GMT_VECTOR **V, GMT_LONG free_vector)
{	/* By taking a reference to the vector pointer we can set it to NULL when done */
	/* free_vector = FALSE means the vectors are not to be freed but the data array itself will be */
	GMT_free_vector_ptr (C, *V, free_vector);
	GMT_free (C, *V);
}

struct GMT_VECTOR * GMT_duplicate_vector (struct GMT_CTRL *C, struct GMT_VECTOR *V_in, GMT_LONG duplicate_data)
{	/* Duplicates a vector contrainer - optionally duplicates data arrays */
	struct GMT_VECTOR *V = NULL;
	
	V = GMT_memory (C, NULL, 1, struct GMT_VECTOR);
	GMT_memcpy (V, V_in, 1, struct GMT_MATRIX);
	V->data = GMT_memory (C, NULL, V_in->n_columns, union GMT_UNIVECTOR);
	V->type = GMT_memory (C, NULL, V_in->n_columns, GMT_LONG);
	if (duplicate_data) {
		GMT_LONG col;
		for (col = 0; col < V_in->n_columns; col++) {
			GMT_alloc_univector (C, &V->data[col], V->type[col], V->n_rows);
			GMT_duplicate_univector (C, &V->data[col], &V_in->data[col], V->type[col], V->n_rows);
		}
	}
	return (V);
}

struct GMT_MATRIX * GMT_create_matrix (struct GMT_CTRL *C)
{	/* Allocates space for a new matrix container. */
	struct GMT_MATRIX *M = NULL;
	M = GMT_memory (C, NULL, 1, struct GMT_MATRIX);
	M->alloc_mode = GMT_ALLOCATED;	/* So GMT_* modules can free this memory. */
	return (M);
}

struct GMT_MATRIX * GMT_duplicate_matrix (struct GMT_CTRL *C, struct GMT_MATRIX *M_in, GMT_LONG duplicate_data)
{	/* Duplicates a matrix container - optionally duplicates the data array */
	struct GMT_MATRIX *M = NULL;
	M = GMT_memory (C, NULL, 1, struct GMT_MATRIX);
	GMT_memcpy (M, M_in, 1, struct GMT_MATRIX);
	GMT_memset (&M->data, 1, union GMT_UNIVECTOR);
	if (duplicate_data) {
		GMT_LONG size = M->n_rows * M->n_columns;
		GMT_alloc_univector (C, &(M->data), M->type, size);
		GMT_duplicate_univector (C, &M->data, &M_in->data, M->type, size);
	}
	return (M);
}

void GMT_free_matrix_ptr (struct GMT_CTRL *C, struct GMT_MATRIX *M, GMT_LONG free_matrix)
{	/* Free everything but the struct itself  */
	if (!M) return;	/* Nothing to deallocate */
	if (free_matrix) GMT_free_univector (C, &(M->data), M->type);
}

void GMT_free_matrix (struct GMT_CTRL *C, struct GMT_MATRIX **M, GMT_LONG free_matrix)
{	/* By taking a reference to the matrix pointer we can set it to NULL when done */
	GMT_free_matrix_ptr (C, *M, free_matrix);
	GMT_free (C, *M);
}

GMT_LONG GMT_not_numeric (struct GMT_CTRL *C, char *text)
{
	/* TRUE if text cannot represent a valid number  However,
	 * FALSE does not therefore mean we have a valid number because
	 * <date>T<clock> representations may use all kinds
	 * of punctuations or letters according to the various format
	 * settings in gmt.conf.  Here we just rule out things
	 * that we are sure of. */

	GMT_LONG i, k, n_digits = 0, n_period = 0, period = 0, n_plus = 0, n_minus = 0, len;
	static char *valid = "0123456789-+.:WESNT" GMT_LEN_UNITS GMT_DIM_UNITS;
	if (!text) return (TRUE);	/* NULL pointer */
	if (!(len = strlen (text)))  return (TRUE);	/* Blank string */
	if (isalpha ((int)text[0])) return (TRUE);	/* Numbers cannot start with letters */
	if (!(text[0] == '+' || text[0] == '-' || text[0] == '.' || isdigit ((int)text[0]))) return (TRUE);	/* Numbers must be [+|-][.][<digits>] */
	for (i = 0; text[i]; i++) {	/* Check each character */
		/* First check for ASCII values that should never appear in any number */
		if (!strchr (valid, text[i])) return (TRUE);	/* Found a char not among valid letters */
		if (isdigit ((int)text[i])) n_digits++;
		if (text[i] == '.') {
			n_period++;
			period = i;
		}
		if (text[i] == '+') n_plus++;
		if (text[i] == '-') n_minus++;
	}
	if (n_digits == 0 || n_period > 1 || (n_plus + n_minus) > 2) return (TRUE);
	if (n_period) {	/* Check if we have filename.ext with ext having no numbers */
		for (i = period + 1, n_digits = k = 0; text[i]; i++, k++) if (isdigit ((int)text[i])) n_digits++;
		if (k > 0 && n_digits == 0) return (TRUE);	/* Probably a file */
	}
	return (FALSE);	/* This may in fact be numeric */
}

#if 0
GMT_LONG GMT_not_numeric_old (struct GMT_CTRL *C, char *text)
{
	/* TRUE if text cannot represent a valid number  However,
	 * FALSE does not therefore mean we have a valid number because
	 * <date>T<clock> representations may use all kinds
	 * of punctuations or letters according to the various format
	 * settings in gmt.conf.  Here we just rule out things
	 * that we are sure of. */

	GMT_LONG i, k, n_digits = 0, n_period = 0, period = 0, n_plus = 0, n_minus = 0, len;
	if (!text) return (TRUE);	/* NULL pointer */
	if (!(len = strlen (text)))  return (TRUE);	/* Blank string */
	if (isalpha ((int)text[0])) return (TRUE);	/* Numbers cannot start with letters */
	if (!(text[0] == '+' || text[0] == '-' || text[0] == '.' || isdigit ((int)text[0]))) return (TRUE);	/* Numbers must be [+|-][.][<digits>] */
	for (i = 0; text[i]; i++) {	/* Check each character */
		/* First check for ASCII values that should never appear in any number */
		if (text[i] < 43) return (TRUE);	/* ASCII 0-42 */
		if (text[i] == '\'' || text[i] == '/') return (TRUE);
		if (text[i] > ':' && text[i] < 'D') return (TRUE);
		if (text[i] > 'E' && text[i] < 'N') return (TRUE);
		if (text[i] > 'N' && text[i] < 'S') return (TRUE);
		if (text[i] > 'S' && text[i] < 'W') return (TRUE);
		if (text[i] > 'W' && text[i] < 'c') return (TRUE);
		if (text[i] > 'e') return (TRUE);
		if (isdigit ((int)text[i])) n_digits++;
		if (text[i] == '.') {
			n_period++;
			period = i;
		}
		if (text[i] == '+') n_plus++;
		if (text[i] == '-') n_minus++;
	}
	if (n_digits == 0 || n_period > 1 || (n_plus + n_minus) > 2) return (TRUE);
	if (n_period) {	/* Check if we have filename.ext with ext having no numbers */
		for (i = period + 1, n_digits = k = 0; text[i]; i++, k++) if (isdigit ((int)text[i])) n_digits++;
		if (k > 0 && n_digits == 0) return (TRUE);	/* Probably a file */
	}
	return (FALSE);	/* This may in fact be numeric */
}
#endif

GMT_LONG GMT_conv_intext2dbl (struct GMT_CTRL *C, char *record, GMT_LONG ncols)
{
	/* Used when we read records from GMT_TEXTSETs and need to obtain doubles */
	/* Convert the first ncols fields in the record string to numbers that we
	 * store in C->current.io.curr_rec, which is what normal GMT_DATASET processing do.
	 * We stop if we run out of fields and ignore conversion errors.  */

	GMT_LONG k = 0, pos = 0;
	char p[GMT_BUFSIZ];

	while (k < ncols && GMT_strtok (record, " \t,", &pos, p)) {	/* Get each field in turn and bail when done */
		if (!(p[0] == '+' || p[0] == '-' || p[0] == '.' || isdigit ((int)p[0]))) continue;	/* Numbers must be [+|-][.][<digits>] */
		GMT_scanf (C, p, C->current.io.col_type[GMT_IN][k], &C->current.io.curr_rec[k]);	/* Be tolerant of errors */
		k++;
	}
	if (C->current.setting.io_lonlat_toggle[GMT_IN] && k >= 2) d_swap (C->current.io.curr_rec[GMT_X], C->current.io.curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (C->current.io.col_type[GMT_IN][GMT_X] & GMT_IS_GEO) gmt_adjust_periodic (C);			/* Must account for periodicity in 360 */
	return (0);
}

GMT_LONG gmt_ogr_get_type (char *item)
{
	if (!strcmp (item, "double") || !strcmp (item, "DOUBLE")) return (GMTAPI_DOUBLE);
	if (!strcmp (item, "float") || !strcmp (item, "FLOAT")) return (GMTAPI_FLOAT);
	if (!strcmp (item, "integer") || !strcmp (item, "INTEGER")) return (GMTAPI_INT);
	if (!strcmp (item, "char") || !strcmp (item, "CHAR")) return (GMTAPI_CHAR);
	if (!strcmp (item, "string") || !strcmp (item, "STRING")) return (GMTAPI_TEXT);
	if (!strcmp (item, "datetime") || !strcmp (item, "DATETIME")) return (GMTAPI_TIME);
	if (!strcmp (item, "logical") || !strcmp (item, "LOGICAL")) return (GMTAPI_UCHAR);
	return (GMTAPI_NOTSET);
}

GMT_LONG gmt_ogr_get_geometry (char *item)
{
	if (!strcmp (item, "point") || !strcmp (item, "POINT")) return (GMT_IS_POINT);
	if (!strcmp (item, "mpoint") || !strcmp (item, "MPOINT")) return (GMT_IS_MULTIPOINT);
	if (!strcmp (item, "line") || !strcmp (item, "LINE")) return (GMT_IS_LINESTRING);
	if (!strcmp (item, "mline") || !strcmp (item, "MLINE")) return (GMT_IS_MULTILINESTRING);
	if (!strcmp (item, "poly") || !strcmp (item, "POLY")) return (GMT_IS_POLYGON);
	if (!strcmp (item, "mpoly") || !strcmp (item, "MPOLY")) return (GMT_IS_MULTIPOLYGON);
	return (GMTAPI_NOTSET);
}

void GMT_free_ogr (struct GMT_CTRL *C, struct GMT_OGR **G, GMT_LONG mode)
{	/* Free up GMT/OGR structure, if used */
	GMT_LONG k;
	if (!(*G)) return;	/* Nothing to do */
	/* mode = 0 only frees the aspatial data value array, while mode = 1 frees the entire struct and contents */
	for (k = 0; k < (*G)->n_aspatial; k++) {
		if (mode == 1 && (*G)->name && (*G)->name[k]) free ((*G)->name[k]);
		if ((*G)->value && (*G)->value[k]) free ((*G)->value[k]);
	}
	GMT_free (C, (*G)->value);
	GMT_free (C, (*G)->dvalue);
	if (mode == 0) return;	/* That's all we do for now */
	/* Here we free up everything */
	GMT_free (C, (*G)->name);
	GMT_free (C, (*G)->type);
	if ((*G)->region) free ((*G)->region);
	for (k = 0; k < 4; k++) free ((*G)->proj[k]);
	GMT_free (C, (*G));
	return;
}

struct GMT_OGR * GMT_duplicate_ogr (struct GMT_CTRL *C, struct GMT_OGR *G)
{	/* Duplicate GMT/OGR structure, if used */
	GMT_LONG k;
	struct GMT_OGR *G_dup = NULL;
	if (!G) return (NULL);	/* Nothing to do */
	G_dup = GMT_memory (C, NULL, 1, struct GMT_OGR);
	if (G->region) G_dup->region = strdup (G->region);
	for (k = 0; k < 4; k++) if (G->proj[k]) G_dup->proj[k] = strdup (G->proj[k]);
	G_dup->geometry = G->geometry;
	if (G->n_aspatial) {
		G_dup->n_aspatial = G->n_aspatial;
		G_dup->name = GMT_memory (C, NULL, G->n_aspatial, char *);
		for (k = 0; k < G->n_aspatial; k++) if (G->name[k]) G_dup->name[k] = strdup (G->name[k]);
		G_dup->type = GMT_memory (C, NULL, G->n_aspatial, GMT_LONG);
		GMT_memcpy (G_dup->type, G->type, G->n_aspatial, GMT_LONG);
	}
	return (G_dup);
}

/* NOT USED ??? */
GMT_LONG GMT_validate_aspatial (struct GMT_CTRL *C, struct GMT_OGR *G)
{
	GMT_LONG k;
	if (C->current.io.ogr != 1) return (GMT_OK);	/* No point checking further since file is not GMT/OGR */
	for (k = 0; k < C->common.a.n_aspatial; k++) if (gmt_get_ogr_id (G, C->common.a.name[k])) return (-1);
	return (GMT_OK);
}

/* NOT USED ??? */
GMT_LONG GMT_load_aspatial_values (struct GMT_CTRL *C, struct GMT_OGR *G)
{
	/* Uses the info in -a and OGR to replace values in the curr_rec array with aspatial values */

	GMT_LONG k, id, n;
	for (k = n = 0; k < C->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		id = gmt_get_ogr_id (G, C->common.a.name[k]);
		switch (G->type[id]) {
			case GMTAPI_DOUBLE:
			case GMTAPI_FLOAT:
			case GMTAPI_ULONG:
			case GMTAPI_LONG:
			case GMTAPI_UINT:
			case GMTAPI_INT:
			case GMTAPI_USHORT:
			case GMTAPI_SHORT:
			case GMTAPI_UCHAR:
			case GMTAPI_CHAR:
				C->current.io.curr_rec[C->common.a.col[k]] = atof (G->value[id]);
				break;
			case GMTAPI_TIME:
				GMT_scanf_arg (C, G->value[id], GMT_IS_ABSTIME, &C->current.io.curr_rec[C->common.a.col[k]]);
				break;
			default:	/* Do nothing (string) */
				break;
		}
		n++;
	}
	return (n);
}

double GMT_get_aspatial_value (struct GMT_CTRL *C, GMT_LONG col, struct GMT_LINE_SEGMENT *S)
{
	/* Return the value associated with the aspatial values given for this column col */

	GMT_LONG k, id;
	char *V;
	for (k = 0; k < C->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (col != C->common.a.col[k]) continue;	/* Not the column we want */
		id = gmt_get_ogr_id (C->current.io.OGR, C->common.a.name[k]);	/* Get the ID */
		V = (S && S->ogr) ? S->ogr->value[id] : C->current.io.OGR->value[id];	/* Either from table or from segment (multi) */
		return (gmt_convert_aspatial_value (C, C->current.io.OGR->type[id], V));
	}
	GMT_report (C, GMT_MSG_FATAL, "Warning: No aspatial value found for column %ld [Return NaN]\n", col);
	return (C->session.d_NaN);
}

GMT_LONG GMT_load_aspatial_string (struct GMT_CTRL *C, struct GMT_OGR *G, GMT_LONG col, char out[GMT_BUFSIZ])
{
	/* Uses the info in -a and OGR to retrieve the requested aspatial string */

	GMT_LONG k, id = GMTAPI_NOTSET, len;
	if (C->current.io.ogr != 1) return (0);		/* No point checking further since file is not GMT/OGR */
	for (k = 0; k < C->common.a.n_aspatial; k++) {	/* For each item specified in -a */
		if (C->common.a.col[k] == col) id = k;			/* ..that matches the given column */
	}
	if (id == GMTAPI_NOTSET) return (0);
	id = gmt_get_ogr_id (G, C->common.a.name[id]);
	if (id == GMTAPI_NOTSET) return (0);
	len = strlen (G->value[id]);
	GMT_memset (out, GMT_BUFSIZ, char);
	if (G->value[id][0] == '\"' && G->value[id][len-1] == '\"')	/* Skip opening and closing quotes */
		strncpy (out, &G->value[id][1], len-2);
	else
		strcpy (out, G->value[id]);
	return (1);
}
