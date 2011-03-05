/*--------------------------------------------------------------------
 *	$Id: gmt_io.c,v 1.225 2011-03-05 21:24:28 guru Exp $
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
 * latter is the case usually there is a -m option to signal this case.
 * The structure GMT_IO holds parameters that are used during the reading
 * and processing of ascii tables.  For compliance with a wide variety of
 * binary data formats for grids and their internal nesting the GMT_Z_IO
 * structure and associated functions are used (in xyz2grd and grd2xyz)
 *
 * The following functions are here:
 *
 *	GMT_getuserpath		Get pathname of file in "user directories" (GMT_TMPDIR, CWD, HOME, GMT_USERDIR)
 *	GMT_getdatapath		Get pathname of file in "data directories" (CWD, GMT_{USER,DATA,GRID,IMG}DIR)
 *	GMT_getsharepath	Get pathname of file in "share directries" (CWD, GMT_USERDIR, GMT_SHAREDIR tree)
 *	GMT_fopen:		Open a file using GMT_getdatapath
 *	GMT_fclose:		Close a file
 *	GMT_io_init:		Init GMT_IO structure
 *	GMT_parse_b_option:	Decode the -b switch
 *	GMT_parse_m_option:	Decode the -m switch
 *	GMT_write_segmentheader	Write header record for multisegment files
 *	GMT_ascii_input:	Decode ascii input record
 *	GMT_scanf:		Robust scanf function with optional dd:mm:ss conversion
 *	GMT_bin_double_input:	Decode binary double precision record
 *	GMT_bin_double_input_swab:	Decode binary double precision record followed by byte-swabbing
 *	GMT_bin_float_input:	Decode binary single precision record
 *	GMT_bin_float_input_swab:	Decode binary single precision record followed by byte-swabbing
 *	GMT_nc_input:		Decode one record of netCDF column-oriented data
 *	GMT_ascii_output:	Write ascii record
 *	GMT_bin_double_output:	Write binary double precision record
 *	GMT_bin_double_output_swab:	Write binary double precision record after first swabbing
 *	GMT_bin_float_output:	Write binary single precision record
 *	GMT_bin_float_output_swab:	Write binary single precision record after first swabbing
 *	GMT_set_z_io:		Set GMT_Z_IO structure based on -Z
 *	GMT_check_z_io:		Fill in implied missing row/column
 *	GMT_A_read:		Read the next ascii item from input stream (may be more than one per line) z must be regular float
 *	GMT_a_read:		Read 1 ascii item per input record
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
 * Version:	4.1.x
 * Now 64-bit enabled.
 */

#define GMT_WITH_NO_PS
#include "gmt.h"

GMT_LONG GMT_do_swab = FALSE;	/* Used to indicate swab'ing during binary read */
PFB GMT_read_binary;		/* Set to handle double|float w/wo swab */
GMT_LONG GMT_A_read (FILE *fp, double *d);
GMT_LONG GMT_a_read (FILE *fp, double *d);
GMT_LONG GMT_c_read (FILE *fp, double *d);
GMT_LONG GMT_u_read (FILE *fp, double *d);
GMT_LONG GMT_h_read (FILE *fp, double *d);
GMT_LONG GMT_H_read (FILE *fp, double *d);
GMT_LONG GMT_i_read (FILE *fp, double *d);
GMT_LONG GMT_I_read (FILE *fp, double *d);
GMT_LONG GMT_l_read (FILE *fp, double *d);
GMT_LONG GMT_f_read (FILE *fp, double *d);
GMT_LONG GMT_d_read (FILE *fp, double *d);
GMT_LONG GMT_a_write (FILE *fp, double d);
GMT_LONG GMT_c_write (FILE *fp, double d);
GMT_LONG GMT_u_write (FILE *fp, double d);
GMT_LONG GMT_h_write (FILE *fp, double d);
GMT_LONG GMT_H_write (FILE *fp, double d);
GMT_LONG GMT_i_write (FILE *fp, double d);
GMT_LONG GMT_I_write (FILE *fp, double d);
GMT_LONG GMT_l_write (FILE *fp, double d);
GMT_LONG GMT_f_write (FILE *fp, double d);
GMT_LONG GMT_d_write (FILE *fp, double d);
void GMT_col_ij (struct GMT_Z_IO *r, GMT_LONG ij, GMT_LONG *gmt_ij);
void GMT_row_ij (struct GMT_Z_IO *r, GMT_LONG ij, GMT_LONG *gmt_ij);
GMT_LONG GMT_ascii_input (FILE *fp, GMT_LONG *n, double **ptr);		/* Decode ASCII input records */
GMT_LONG GMT_bin_input (FILE *fp, GMT_LONG *n, double **ptr);		/* Decode binary input records */
GMT_LONG GMT_bin_double_input (FILE *fp, GMT_LONG *n, double **ptr);	/* Decode binary double input records */
GMT_LONG GMT_bin_double_input_swab (FILE *fp, GMT_LONG *n, double **ptr);	/* Decode binary double input records */
GMT_LONG GMT_bin_float_input (FILE *fp, GMT_LONG *n, double **ptr);	/* Decode binary float input records */
GMT_LONG GMT_bin_float_input_swab (FILE *fp, GMT_LONG *n, double **ptr);	/* Decode binary float input records */
GMT_LONG GMT_ascii_output (FILE *fp, GMT_LONG n, double *ptr);		/* Write ASCII output records */
GMT_LONG GMT_bin_double_output (FILE *fp, GMT_LONG n, double *ptr);	/* Write binary double output records */
GMT_LONG GMT_bin_double_output_swab (FILE *fp, GMT_LONG n, double *ptr);	/* Write binary double output records */
GMT_LONG GMT_bin_float_output (FILE *fp, GMT_LONG n, double *ptr);	/* Write binary float output records */
GMT_LONG GMT_bin_float_output_swab (FILE *fp, GMT_LONG n, double *ptr);	/* Write binary float output records */
GMT_LONG GMT_ascii_output_one (FILE *fp, double x, GMT_LONG col);		/* Writes one item to output in ascii format */
void GMT_adjust_periodic ();					/* Add/sub 360 as appropriate */
void GMT_decode_calclock_formats ();
GMT_LONG GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S, GMT_LONG mode);
GMT_LONG GMT_get_dms_order (char *text, struct GMT_GEO_IO *S);
GMT_LONG GMT_get_hms_order (char *text, struct GMT_CLOCK_IO *S);

GMT_LONG GMT_scanf_clock (char *s, double *val);
GMT_LONG GMT_scanf_calendar (char *s, GMT_cal_rd *rd);
GMT_LONG GMT_scanf_ISO_calendar (char *s, GMT_cal_rd *rd);
GMT_LONG GMT_scanf_g_calendar (char *s, GMT_cal_rd *rd);
GMT_LONG GMT_scanf_geo (char *s, double *val);
GMT_LONG GMT_scanf_float (char *s, double *val);
GMT_LONG GMT_n_segment_points (struct GMT_LINE_SEGMENT *S, GMT_LONG n_segments);

FILE *GMT_nc_fopen (const char *filename, const char *mode);
GMT_LONG GMT_nc_input (FILE *fp, GMT_LONG *n, double **ptr);
GMT_LONG GMT_process_binary_input (GMT_LONG n_read);
GMT_LONG GMT_get_binary_d_input (FILE *fp, GMT_LONG n);
GMT_LONG GMT_get_binary_d_input_swab (FILE *fp, GMT_LONG n);
GMT_LONG GMT_get_binary_f_input (FILE *fp, GMT_LONG n);
GMT_LONG GMT_get_binary_f_input_swab (FILE *fp, GMT_LONG n);
GMT_LONG GMT_read_binary_f_input (FILE *fp, float *GMT_f, GMT_LONG n);
GMT_LONG GMT_n_cols_needed_for_gaps (GMT_LONG n);
GMT_LONG GMT_set_gap ();

/* Library functions needed for Windows DLL to work properly.
 * THese are only compiled under Windows - under other OS the
 * macros in gmt_io.h will kick in instead.
 */

#ifdef WIN32
FILE *GMT_fdopen (int handle, const char *mode)
{	/* Wrapper for fdopen */
	FILE *fp;

	if ((fp = fdopen (handle, mode))) return (fp);
	return (NULL);
}

int GMT_fgetc (FILE *stream)
{
	return (fgetc (stream));
}

int GMT_ungetc (int c, FILE *stream)
{
	return (ungetc (c, stream));
}

int GMT_fputs (const char *str, FILE *stream)
{
	return (fputs (str, stream));
}

int GMT_fputc (const int c, FILE *stream)
{
	return (fputc (c, stream));
}

int GMT_fseek (FILE *stream, long offset, int whence)
{
	return (fseek(stream, offset, whence));
}

long GMT_ftell (FILE *stream)
{
	return (ftell(stream));
}

size_t GMT_fread (void *ptr, size_t size, size_t nmemb, FILE * stream)
{
	return (fread (ptr, size, nmemb, stream));
}

size_t GMT_fwrite (const void *ptr, size_t size, size_t nmemb, FILE * stream)
{
	return (fwrite (ptr, size, nmemb, stream));
}

void GMT_rewind (FILE *stream)
{
	rewind (stream);
}

void GMT_fflush (FILE *stream) { fflush (stream); }

#endif

/* This version of fgets will check for input record truncation, that is
 * the input record is longer than the given size.  Since calls to GMT_fgets
 * ASSUME they get a logical record, we will give a warning if truncation
 * occurs and read until we have consumed the linefeed, thus making the
 * i/o machinery ready for the next logical record.
 */

char *GMT_fgets (char *str, GMT_LONG size, FILE *stream)
{
	char *result;
	
	memset ((void *)str, 0, (size_t)(size * sizeof (char)));
	if (!(result = fgets (str, (int)size, stream))) return (NULL);	/* Got nothing */
	/* fgets will always set str[size-1] = '\0'.  Thus, we examine
	 * str[size-2].  If this is neither '\0' or '\n' then we only
	 * read a portion of a logical record longer than size.
	 */
	if (!(str[size-2] == '\n' || str[size-2] == '\0')) {	/* Only got part of a record */
		int c, n = 0;
		while ((c = fgetc (stream)) != '\n') n++;	/* Read char-by-char until newline is consumed */
		fprintf (stderr, "%s: Long input record (%ld bytes) was truncated to first %ld bytes!\n", GMT_program, size+n, size);
	}
	return (result);
}

int GMT_fclose (FILE *stream)
{
	if (GMT_io.ncid) {
		nc_close (GMT_io.ncid);
		GMT_free (GMT_io.varid);
		GMT_free (GMT_io.add_offset);
		GMT_free (GMT_io.scale_factor);
		GMT_free (GMT_io.missing_value);
		GMT_io.ncid = GMT_io.nvars = 0;
		GMT_io.ndim = GMT_io.nrec = 0;
		GMT_input = GMT_input_ascii;
		return (0);
	}
	else
		return (fclose (stream));
}

FILE *GMT_fopen (const char *filename, const char *mode)
{
	char path[BUFSIZ];

	if (mode[0] == 'r') {
		if (GMT_io.netcdf[GMT_IN] || strchr (filename, '?')) return (GMT_nc_fopen (filename, mode));
		return (fopen (GMT_getdatapath(filename, path), mode));
	}
	else
		return (fopen (filename, mode));
}

FILE *GMT_nc_fopen (const char *filename, const char *mode)
/* Open a netCDF file for column I/O. Append ?var1/var2/... to indicate the requested
 * columns.
 * Currently only reading is supported.
 * The routine returns a fake file pointer (in fact the netCDF file ID), but stores
 * all the relevant information in the GMT_io struct (ncid, ndim, nrec, varid, add_offset,
 * scale_factor, missing_value). Some of these are allocated here, and have to be
 * deallocated upon GMT_fclose.
 * Also asigns GMT_io.in_col_type based on the variable attributes.
 */
{
	char file[BUFSIZ], path[BUFSIZ];
	int i, j, nvars;
	size_t n;
	GMT_LONG tmp_pointer;	/* To avoid 64-bit warnings */
	char varnm[10][GMT_TEXT_LEN], long_name[GMT_LONG_TEXT], units[GMT_LONG_TEXT], varname[GMT_TEXT_LEN];
	struct GMT_TIME_SYSTEM time_system;

	if (mode[0] != 'r') {
		fprintf (stderr, "%s: GMT_fopen does not support netCDF writing mode\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}

	nvars = sscanf (filename, "%[^?]?%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]", file, varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9]) - 1;
	if (nc_open (GMT_getdatapath(file, path), NC_NOWRITE, &GMT_io.ncid)) return (NULL);
	if (nvars <= 0) nvars = sscanf (GMT_io.varnames, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]", varnm[0], varnm[1], varnm[2], varnm[3], varnm[4], varnm[5], varnm[6], varnm[7], varnm[8], varnm[9]);
	if (nvars <= 0)
		nc_inq_nvars (GMT_io.ncid, &GMT_io.nvars);
	else
		GMT_io.nvars = nvars;
	GMT_io.varid = (int *)GMT_memory (VNULL, (size_t)GMT_io.nvars, sizeof (int), GMT_program);
	GMT_io.scale_factor = (double *)GMT_memory (VNULL, (size_t)GMT_io.nvars, sizeof (double), GMT_program);
	GMT_io.add_offset = (double *)GMT_memory (VNULL, (size_t)GMT_io.nvars, sizeof (double), GMT_program);
	GMT_io.missing_value = (double *)GMT_memory (VNULL, (size_t)GMT_io.nvars, sizeof (double), GMT_program);
	GMT_io.ndim = GMT_io.nrec = 0;

	for (i = 0; i < GMT_io.nvars; i++) {
		/* Get variable ID and variable name */
		if (nvars <= 0)
			GMT_io.varid[i] = i;
		else
			GMT_err_fail (nc_inq_varid (GMT_io.ncid, varnm[i], &GMT_io.varid[i]), file);
		nc_inq_varname (GMT_io.ncid, GMT_io.varid[i], varname);

		/* Check column size */
		nc_inq_varndims (GMT_io.ncid, GMT_io.varid[i], &j);
		if (j != 1) {
			fprintf (stderr, "%s: NetCDF variable %s is not 1-dimensional\n", GMT_program, varname);
			GMT_exit (EXIT_FAILURE);
		}
		nc_inq_vardimid(GMT_io.ncid, GMT_io.varid[i], &j);
		nc_inq_dimlen(GMT_io.ncid, j, &n);
		if (GMT_io.ndim != 0 && GMT_io.ndim != n) {
			fprintf (stderr, "%s: NetCDF variable %s has different dimension (%ld) from others (%ld)\n", GMT_program, varname, (GMT_LONG)n, (GMT_LONG)GMT_io.ndim);
			GMT_exit (EXIT_FAILURE);
		}
		GMT_io.ndim = n;

		/* Get scales, offsets and missing values */
		if (nc_get_att_double(GMT_io.ncid, GMT_io.varid[i], "scale_factor", &GMT_io.scale_factor[i])) GMT_io.scale_factor[i] = 1.0;
		if (nc_get_att_double(GMT_io.ncid, GMT_io.varid[i], "add_offset", &GMT_io.add_offset[i])) GMT_io.add_offset[i] = 0.0;
		if (nc_get_att_double (GMT_io.ncid, GMT_io.varid[i], "_FillValue", &GMT_io.missing_value[i]) &&
		    nc_get_att_double (GMT_io.ncid, GMT_io.varid[i], "missing_value", &GMT_io.missing_value[i])) GMT_io.missing_value[i] = GMT_d_NaN;

		/* Scan for geographical or time units */
		if (GMT_nc_get_att_text (GMT_io.ncid, GMT_io.varid[i], "long_name", long_name, (size_t)GMT_LONG_TEXT)) long_name[0] = 0;
		if (GMT_nc_get_att_text (GMT_io.ncid, GMT_io.varid[i], "units", units, (size_t)GMT_LONG_TEXT)) units[0] = 0;
		GMT_str_tolower (long_name); GMT_str_tolower (units);

		if (!strcmp (long_name, "longitude") || strstr (units, "degrees_e"))
			GMT_io.in_col_type[i] = GMT_IS_LON;
		else if (!strcmp (long_name, "latitude") || strstr (units, "degrees_n"))
			GMT_io.in_col_type[i] = GMT_IS_LAT;
		else if (!strcmp (long_name, "time") || !strcmp (varname, "time")) {
			GMT_io.in_col_type[i] = GMT_IS_RELTIME;
			memcpy ((void *)&time_system, (void *)&gmtdefs.time_system, sizeof (struct GMT_TIME_SYSTEM));
			if (GMT_get_time_system (units, &time_system) || GMT_init_time_system_structure (&time_system))
				fprintf (stderr, "%s: Warning: Time units [%s] in NetCDF file not recognised, defaulting to gmtdefaults.\n", GMT_program, units);
			/* Determine scale between data and internal time system, as well as the offset (in internal units) */
			GMT_io.scale_factor[i] = GMT_io.scale_factor[i] * time_system.scale * gmtdefs.time_system.i_scale;
			GMT_io.add_offset[i] *= time_system.scale;	/* Offset in seconds */
			GMT_io.add_offset[i] += GMT_DAY2SEC_F * ((time_system.rata_die - gmtdefs.time_system.rata_die) + (time_system.epoch_t0 - gmtdefs.time_system.epoch_t0));
			GMT_io.add_offset[i] *= gmtdefs.time_system.i_scale;	/* Offset in internal time units */
		}
		else if (GMT_io.in_col_type[i] == GMT_IS_UNKNOWN)
			GMT_io.in_col_type[i] = GMT_IS_FLOAT;
	}

	GMT_input = GMT_nc_input;
	tmp_pointer = (GMT_LONG)GMT_io.ncid;
	return ((FILE *)tmp_pointer);
}

GMT_LONG GMT_nc_get_att_text (int ncid, int varid, char *name, char *text, size_t textlen)
{	/* This function is a replacement for nc_get_att_text that avoids overflow of text
	 * ncid, varid, name, text	: as in nc_get_att_text
	 * textlen			: maximum number of characters to copy to string text
	 */
	GMT_LONG err;
	size_t attlen;
	char *att;

	GMT_err_trap (nc_inq_attlen (ncid, varid, name, &attlen));
	att = (char *) GMT_memory (VNULL, attlen, sizeof (char), "GMT_nc_get_att_text");
	nc_get_att_text (ncid, varid, name, att);
	attlen = MIN (attlen, textlen-1);	/* Truncate text to one less than textlen (to keep space for NUL terminator) */
	memcpy (text, att, attlen);		/* Copy att to text */
	memset (&text[attlen], 0, textlen - attlen);	/* Fill rest of text with zeros */
	GMT_free (att);
	return (GMT_NOERROR);
}

/* Table I/O routines for ascii and binary io */

char *GMT_getuserpath (const char *stem, char *path)
{
	/* stem is the name of the file, e.g., .gmtdefaults4 or .gmtdefaults
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in the temporary directory (if defined),
	 * current directory, home directory and $GMT_USERDIR (default ~/.gmt)
	 */

	/* In isolation mode (when GMT_TMPDIR is defined), we first look there */

	if (GMT_TMPDIR) {
		sprintf (path, "%s%c%s", GMT_TMPDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}

	/* Then look in the current working directory */

	if (!access (stem, R_OK)) {	/* Yes, found it */
		strcpy (path, stem);
		return (path);
	}

	/* If still not found, see if there is a file in the GMT_{HOME,USER}DIR directories */

	if (GMT_HOMEDIR) {
		sprintf (path, "%s%c%s", GMT_HOMEDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}
	if (GMT_USERDIR) {
		sprintf (path, "%s%c%s", GMT_USERDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}

	return (NULL);	/* No file found, give up */
}

char *GMT_getdatapath (const char *stem, char *path)
{
	/* stem is the name of the file, e.g., grid.img
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory and $GMT_{USER,DATA,GRID,IMG}DIR
	 */

	/* First look in the current working directory */

	if (!access (stem, R_OK)) {	/* Yes, found it */
		strcpy (path, stem);
		return (path);
	}

	/* Not found, see if there is a file in the GMT_{USER,DATA,GRID,IMG}DIR directories */

	if (GMT_USERDIR) {
		sprintf (path, "%s%c%s", GMT_USERDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}
	if (GMT_DATADIR) {	/* Examine all directories given in that order */
		char dir[BUFSIZ];
		GMT_LONG pos = 0; 
		while ((GMT_strtok (GMT_DATADIR, PATH_SEPARATOR, &pos, dir))) {
			sprintf (path, "%s%c%s", dir, DIR_DELIM, stem);
			if (!access (path, R_OK)) return (path);
		}
	}
	if (GMT_GRIDDIR) {
		sprintf (path, "%s%c%s", GMT_GRIDDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}
	if (GMT_IMGDIR) {
		sprintf (path, "%s%c%s", GMT_IMGDIR, DIR_DELIM, stem);
		if (!access (path, R_OK)) return (path);
	}

	return (NULL);	/* No file found, give up */
}

char *GMT_getsharepath (const char *subdir, const char *stem, const char *suffix, char *path)
{
	/* stem is the prefix of the file, e.g., gmt_cpt for gmt_cpt.conf
	 * subdir is an optional subdirectory name in the $GMT_SHAREDIR directory.
	 * suffix is an optional suffix to append to name
	 * path is the full path to the file in question
	 * Returns full pathname if a workable path was found
	 * Looks for file stem in current directory, $GMT_USERDIR (default ~/.gmt) and $GMT_SHAREDIR[/subdir]
	 */

	/* First look in the current working directory */

	sprintf (path, "%s%s", stem, suffix);
	if (!access (path, R_OK)) return (path);	/* Yes, found it in current directory */

	/* Do not continue when full pathname is given */

#ifdef WIN32
	if (stem[0] == '\\' || stem[1] == ':') return (NULL);
#else
	if (stem[0] == '/') return (NULL);
#endif

	/* Not found, see if there is a file in the user's GMT_USERDIR (~/.gmt) directory */

	if (GMT_USERDIR) {
		sprintf (path, "%s%c%s%s", GMT_USERDIR, DIR_DELIM, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Try to get file from $GMT_SHAREDIR/subdir */

	if (subdir) {
		sprintf (path, "%s%c%s%c%s%s", GMT_SHAREDIR, DIR_DELIM, subdir, DIR_DELIM, stem, suffix);
		if (!access (path, R_OK)) return (path);
	}

	/* Finally try file in $GMT_SHAREDIR (for backward compatibility) */

	sprintf (path, "%s%c%s%s", GMT_SHAREDIR, DIR_DELIM, stem, suffix);
	if (!access (path, R_OK)) return (path);

	return (NULL);	/* No file found, give up */
}

int GMT_access (const char* filename, int mode)
{	/* Like access but also checks the GMT_*DIR places */
	char path[BUFSIZ];

	if (!filename || !filename[0]) return (-1);	/* No file given */
	if (mode == R_OK || mode == F_OK) {
		/* Look in special directories when reading or just accessing */
		if (GMT_getdatapath (filename, path)) return (0);
	}
	else {
		/* When writing, only look in current directory */
		if (!(access (filename, mode))) return (0);
	}
	return (-1);
}

void GMT_io_init (void)
{
	/* No need to init the structure as this is done in gmt_init.h directory */

	GMT_LONG i;

	GMT_input  = GMT_input_ascii = GMT_ascii_input;
	GMT_output = GMT_ascii_output;

	GMT_io.give_report = TRUE;

	GMT_io.skip_if_NaN = (GMT_LONG *)GMT_memory (VNULL, (size_t)GMT_MAX_COLUMNS, sizeof (GMT_LONG), GMT_program);
	GMT_io.in_col_type  = (GMT_LONG *)GMT_memory (VNULL, (size_t)GMT_MAX_COLUMNS, sizeof (GMT_LONG), GMT_program);
	GMT_io.out_col_type = (GMT_LONG *)GMT_memory (VNULL, (size_t)GMT_MAX_COLUMNS, sizeof (GMT_LONG), GMT_program);
	for (i = 0; i < 2; i++) GMT_io.skip_if_NaN[i] = TRUE;						/* x/y must be non-NaN */
	for (i = 0; i < 2; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = GMT_IS_UNKNOWN;	/* Must be told [or find out] what x/y are */
	for (i = 2; i < GMT_MAX_COLUMNS; i++) GMT_io.in_col_type[i] = GMT_io.out_col_type[i] = GMT_IS_FLOAT;	/* Other columns default to floats */
	GMT_io.n_header_recs = gmtdefs.n_header_recs;
	memcpy ((void *)GMT_io.io_header, (void *)gmtdefs.io_header, 2*sizeof(GMT_LONG));

	/* Set the Y2K conversion parameters once */

	GMT_Y2K_fix.y2_cutoff = GMT_abs (gmtdefs.Y2K_offset_year) % 100;
	GMT_Y2K_fix.y100 = gmtdefs.Y2K_offset_year - GMT_Y2K_fix.y2_cutoff;
	GMT_Y2K_fix.y200 = GMT_Y2K_fix.y100 + 100;

	GMT_decode_calclock_formats ();
}

GMT_LONG GMT_parse_b_option (char *text)
{
	/* Syntax:	-b[i][o][s|S][d|D][#cols][cvar1/var2/...] */

	GMT_LONG i, id = GMT_IN;
	GMT_LONG i_or_o = FALSE, done = FALSE, error = FALSE;

	for (i = 0; !done && text[i]; i++) {

		switch (text[i]) {

			case 'i':	/* Settings apply to input */
				id = GMT_IN;
				i_or_o = TRUE;
				GMT_io.binary[id] = TRUE;
				break;
			case 'o':	/* Settings apply to output */
				id = GMT_OUT;
				i_or_o = TRUE;
				GMT_io.binary[id] = TRUE;
				break;
			case 'c':	/* I/O is netCDF */
				GMT_io.binary[id] = FALSE;
				GMT_io.netcdf[id] = TRUE;
				strcpy (GMT_io.varnames, &text[i+1]);
				done = TRUE;
				break;
			case 'S':	/* Single Precision but needs byte swap */
				GMT_io.swab[id] = TRUE;
				GMT_io.single_precision[id] = TRUE;
				GMT_io.binary[id] = TRUE;
			case 's':	/* Single Precision */
				GMT_io.single_precision[id] = TRUE;
				GMT_io.binary[id] = TRUE;
				break;
			case 'D':	/* Double Precision but needs byte swap */
				GMT_io.swab[id] = TRUE;
				GMT_io.binary[id] = TRUE;
			case 'd':	/* Double Precision */
				GMT_io.binary[id] = TRUE;
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
		GMT_io.binary[GMT_OUT] = GMT_io.binary[GMT_IN];
		GMT_io.single_precision[GMT_OUT] = GMT_io.single_precision[GMT_IN];
		GMT_io.swab[GMT_OUT] = GMT_io.swab[GMT_IN];
		GMT_io.ncol[GMT_OUT] = GMT_io.ncol[GMT_IN];
	}

	if (GMT_io.binary[GMT_IN]) {
		GMT_input = GMT_bin_input;
		if (GMT_io.swab[GMT_IN])
			GMT_read_binary  = (GMT_io.single_precision[GMT_IN]) ? GMT_get_binary_f_input_swab  : GMT_get_binary_d_input_swab;
		else
			GMT_read_binary  = (GMT_io.single_precision[GMT_IN]) ? GMT_get_binary_f_input  : GMT_get_binary_d_input;
		strcpy (GMT_io.r_mode, "rb");
	}

	if (GMT_io.binary[GMT_OUT]) {
		if (GMT_io.swab[GMT_OUT])
			GMT_output = (GMT_io.single_precision[GMT_OUT]) ? GMT_bin_float_output_swab : GMT_bin_double_output_swab;
		else
			GMT_output = (GMT_io.single_precision[GMT_OUT]) ? GMT_bin_float_output : GMT_bin_double_output;
		strcpy (GMT_io.w_mode, "wb");
		strcpy (GMT_io.a_mode, "ab+");
	}

	return (error);
}

void GMT_parse_m_option (char *text)
{
	/* Turns multisegment on and sets flag, if given.
	 * flag is only used for ASCII data sets.
	 * GMT 4.1.2: Added possibility separate settings for input and output */

	switch (text[0]) {
		case 'i':	/* -m for input files only */
			GMT_io.multi_segments[GMT_IN] = TRUE;
			if (text[1]) GMT_io.EOF_flag[GMT_IN] = text[1];
			break;
		case 'o':	/* -m for output files only */
			GMT_io.multi_segments[GMT_OUT] = TRUE;
			if (text[1]) GMT_io.EOF_flag[GMT_OUT] = text[1];
			break;
		default:	/* Applies to both input and output */
			GMT_io.multi_segments[GMT_IN] = GMT_io.multi_segments[GMT_OUT] = TRUE;
			if (text[0]) GMT_io.EOF_flag[GMT_IN] = GMT_io.EOF_flag[GMT_OUT] = text[0];
			break;
	}
}

GMT_LONG GMT_ascii_input (FILE *fp, GMT_LONG *n, double **ptr)
{
	char line[BUFSIZ], *p, token[BUFSIZ];
	GMT_LONG i, pos, col_no, len, n_convert, n_use;
	GMT_LONG done = FALSE, bad_record, set_nan_flag;
	double val;

	/* GMT_ascii_input will skip blank lines and cshell comment lines which start
	 * with # except when -m# is used of course.  Fields may be separated by
	 * spaces, tabs, or commas.  The routine returns the actual
	 * number of items read [or 0 for segment header and -1 for EOF]
	 * If *n is passed as BUFSIZ it will be reset to the actual number of fields.
	 * If gap checking is in effect and one of the checks involves a column beyond
	 * the ones otherwise needed by the program we extend the reading so we may
	 * examin the column needed in the gap test.
	 */

	while (!done) {	/* Done becomes TRUE when we successfully have read a data record */

		/* First read until we get a non-blank, non-comment record, or reach EOF */

		GMT_io.rec_no++;	/* Counts up, regardless of what this record is (data, junk, multisegment header, etc) */
		while ((p = GMT_fgets (line, BUFSIZ, fp)) && GMT_is_a_blank_line(line)) GMT_io.rec_no++;	/* Skip comments and blank lines */

		if (!p) {	/* Ran out of records */
			GMT_io.status = GMT_IO_EOF;
			if (GMT_io.give_report && GMT_io.n_bad_records) {	/* Report summary and reset */
				fprintf (stderr, "%s: This file had %ld records with invalid x and/or y values\n", GMT_program, GMT_io.n_bad_records);
				GMT_io.n_bad_records = GMT_io.rec_no = GMT_io.pt_no = GMT_io.n_clean_rec = 0;
			}
			return (-1);
		}

		if (line[0] == GMT_io.EOF_flag[GMT_IN]) {	/* Got a multisegment header, take action and return */
			if (GMT_io.multi_segments[GMT_IN]) {
				GMT_io.status = GMT_IO_SEGMENT_HEADER;
				GMT_io.seg_no++;
				strcpy (GMT_io.segment_header, line);
				return (0);
			}
			else {
				fprintf (stderr, "%s: ERROR: Input file seems to be in multiple segment format but the -m switch is not set.\n", GMT_program);
				GMT_exit (EXIT_FAILURE);
			}
		}

		/* Normal data record */

		n_use = GMT_n_cols_needed_for_gaps (*n);	/* Gives is the actual columns we need (which may > *n if gap checking is active; if gap check we also update prev_rec) */

		/* First chop off trailing whitespace and commas */

		len = strlen (line);
#ifndef _WIN32
		if (len >= (BUFSIZ-1)) {
			fprintf (stderr, "%s: This file appears to be in DOS format - reformat with dos2unix\n", GMT_program);
			GMT_exit (EXIT_FAILURE);
		}
#endif

		for (i = len - 1; i >= 0 && strchr (" \t,\r\n", (int)line[i]); i--);
		line[++i] = '\n';	line[++i] = '\0';	/* Now have clean C string with \n\0 at end */

		bad_record = set_nan_flag = FALSE;
		strcpy (GMT_io.current_record, line);
		line[i-1] = '\0';		/* Chop off newline at end of string */
		col_no = pos = 0;
		while (!bad_record && col_no < n_use && (GMT_strtok (line, " \t,", &pos, token))) {	/* Get each field in turn */
			if ((n_convert = GMT_scanf (token, GMT_io.in_col_type[col_no], &val)) == GMT_IS_NAN) {	/* Got NaN or it failed to decode */
				if (gmtdefs.nan_is_gap || !GMT_io.skip_if_NaN[col_no])	/* This field (or all fields) can be NaN so we pass it on */
					GMT_curr_rec[col_no] = GMT_d_NaN;
				else	/* Cannot have NaN in this column, set flag */
					bad_record = TRUE;
				if (GMT_io.skip_if_NaN[col_no]) set_nan_flag = TRUE;
			}
			else					/* Successful decode, assign to array */
				GMT_curr_rec[col_no] = val;
			col_no++;		/* Goto next field */
		}
		if (bad_record) {
			GMT_io.n_bad_records++;
			if (GMT_io.give_report && (GMT_io.n_bad_records == 1)) {	/* Report 1st occurrence */
				fprintf (stderr, "%s: Encountered first invalid record near/at line # %ld\n", GMT_program, GMT_io.rec_no);
				fprintf (stderr, "%s: Likely causes:\n", GMT_program);
				fprintf (stderr, "%s: (1) Invalid x and/or y values, i.e. NaNs or garbage in text strings.\n", GMT_program);
				fprintf (stderr, "%s: (2) Incorrect data type assumed if -J, -f are not set or set incorrectly.\n", GMT_program);
				fprintf (stderr, "%s: (3) The -: switch is implied but not set.\n", GMT_program);
				fprintf (stderr, "%s: (4) Input file in multiple segment format but the -m switch is not set.\n", GMT_program);
			}
		}
		else if (GMT_io.skip_duplicates && GMT_io.pt_no) {	/* Skip duplicate records with same x,y */
			done = !(GMT_curr_rec[GMT_X] == GMT_prev_rec[GMT_X] && GMT_curr_rec[GMT_Y] == GMT_prev_rec[GMT_Y]);
		}
		else
			done = TRUE;
	}
	*ptr = GMT_curr_rec;
	GMT_io.status = (col_no == n_use || *n == GMT_MAX_COLUMNS) ? 0 : GMT_IO_MISMATCH;
	if (set_nan_flag) GMT_io.status |= GMT_IO_NAN;
	if (*n == GMT_MAX_COLUMNS) *n = col_no;

	if (gmtdefs.xy_toggle[GMT_IN]) d_swap (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (GMT_io.in_col_type[GMT_X] & GMT_IS_GEO) GMT_adjust_periodic ();			/* Must account for periodicity in 360 */

	if (GMT_gap_detected()) return (GMT_set_gap());

	GMT_io.pt_no++;	/* Got a valid data record */
	return (col_no);
}

GMT_LONG GMT_set_gap () {	/* Data gaps are special since there is no multiple-segment header flagging the gap; thus next time the record is already read */
	GMT_io.status = GMT_IO_GAP;
	GMT_io.seg_no++;
	sprintf (GMT_io.segment_header, "%c Data gap detected\n", GMT_io.EOF_flag[GMT_IN]);
	return (0);
}

GMT_LONG GMT_is_a_blank_line (char *line) {
	/* Returns TRUE if we should skip this line (because it is blank or has comments) */
	GMT_LONG i = 0;
	if (line[i] == '#' && GMT_io.EOF_flag[GMT_IN] != '#') return (TRUE);	/* Comment */
	while (line[i] && (line[i] == ' ' || line[i] == '\t')) i++;	/* Wind past leading whitespace */
	if (line[i] == '\n' || line[i] == '\r') return (TRUE);
	return (FALSE);
}

GMT_LONG GMT_n_cols_needed_for_gaps (GMT_LONG n) {
	GMT_LONG n_use;
	/* Return the actual items needed (which may be more than n if gap testing demands it) and update previous record */
	n_use = MAX (n, GMT->common->g.n_col);
	memcpy ((void *)GMT_prev_rec, (void *)GMT_curr_rec, (size_t)(n_use*sizeof (double)));
	if (!GMT->common->g.active) return (n);	/* No gap checking, n it is */
	return (n_use);
}

GMT_LONG GMT_nc_input (FILE *fp, GMT_LONG *n, double **ptr)
{
	GMT_LONG i, status, n_use;

	GMT_io.status = 0;
	if (*n == GMT_MAX_COLUMNS)
		*n = GMT_io.nvars;
	else if (*n > GMT_io.nvars) {
		fprintf (stderr, "%s: GMT_nc_input is asking for %ld columns, but file has only %d\n", GMT_program, *n, GMT_io.nvars);
		GMT_io.status = GMT_IO_MISMATCH;
	}
	do {	/* Keep reading until (1) EOF, (2) got a multisegment record, or (3) a valid data record */

		n_use = GMT_n_cols_needed_for_gaps (*n);
		if (GMT_io.nrec == GMT_io.ndim) {
			GMT_io.status = GMT_IO_EOF;
			return (-1);
		}
		for (i = 0; i < GMT_io.nvars && i < n_use; i++) {
			nc_get_var1_double (GMT_io.ncid, GMT_io.varid[i], &GMT_io.nrec, &GMT_curr_rec[i]);
			if (GMT_curr_rec[i] == GMT_io.missing_value[i])
				GMT_curr_rec[i] = GMT_d_NaN;
			else
				GMT_curr_rec[i] = GMT_curr_rec[i] * GMT_io.scale_factor[i] + GMT_io.add_offset[i];
		}
		GMT_io.nrec++;
		GMT_io.rec_no++;
		status = GMT_process_binary_input (n_use);
		if (status == 1) return (0);		/* A multisegment header */
	} while (status == 2);	/* Continue reading when record is to be skipped */
	*ptr = GMT_curr_rec;
	if (GMT_gap_detected()) return (GMT_set_gap());
	GMT_io.pt_no++;
	return (*n);
}

GMT_LONG GMT_bin_input (FILE *fp, GMT_LONG *n, double **ptr)
{	/* General binary read function which calls function pointed to by GMT_read_binary to handle actual reading (and possbily swabbing) */
	GMT_LONG status, n_use;

	GMT_io.status = 0;
	do {	/* Keep reading until (1) EOF, (2) got a multisegment record, or (3) a valid data record */
		n_use = GMT_n_cols_needed_for_gaps (*n);
		if ((*GMT_read_binary) (fp, n_use)) return (-1);	/* EOF */
		GMT_io.rec_no++;
		status = GMT_process_binary_input (n_use);
		if (status == 1) return (0);		/* A multisegment header */
	} while (status == 2);	/* Continue reading when record is to be skipped */
	*ptr = GMT_curr_rec;
	if (GMT_gap_detected()) return (GMT_set_gap());
	GMT_io.pt_no++;

	return (*n);
}

/* Sub functions for GMT_bin_input */

GMT_LONG GMT_get_binary_d_input (FILE *fp, GMT_LONG n) {
	/* Reads the n binary doubles from input */
	GMT_LONG n_read;
	if ((n_read = GMT_fread ((void *) GMT_curr_rec, sizeof (double), (size_t)n, fp)) != n) {	/* EOF or came up short */
		GMT_io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
		if (GMT_io.give_report && GMT_io.n_bad_records) {	/* Report summary and reset */
			fprintf (stderr, "%s: This file had %ld records with invalid x and/or y values\n", GMT_program, GMT_io.n_bad_records);
			GMT_io.n_bad_records = GMT_io.rec_no = GMT_io.pt_no = GMT_io.n_clean_rec = 0;
		}
		return (TRUE);	/* Done with this file */
	}
	return (FALSE);	/* OK so far */
}

GMT_LONG GMT_get_binary_d_input_swab (FILE *fp, GMT_LONG n) {
	/* Reads and swabs the n binary doubles from input */
	GMT_LONG i;
	unsigned int *ii, jj;
	if (GMT_get_binary_d_input (fp, n)) return (TRUE);	/* Return immediately if EOF */
	/* Swab the bytes for each double */
	for (i = 0; i < n; i++) {
		ii = (unsigned int *)&GMT_curr_rec[i];	/* These 4 lines do the swab */
		jj = GMT_swab4 (ii[0]);
		ii[0] = GMT_swab4 (ii[1]);
		ii[1] = jj;
	}
	return (FALSE);
}

GMT_LONG GMT_get_binary_f_input (FILE *fp, GMT_LONG n) {
	/* Reads the n binary floats, then converts them to doubles */
	GMT_LONG i;
	static float GMT_f[GMT_MAX_COLUMNS];
	if (GMT_read_binary_f_input (fp, GMT_f, n)) return (TRUE);	/* EOF or came up short */
	for (i = 0; i < n; i++) GMT_curr_rec[i] = (double)GMT_f[i];
	return (FALSE);	/* OK so far */
}

GMT_LONG GMT_get_binary_f_input_swab (FILE *fp, GMT_LONG n) {
	/* Reads the n binary floats, byte-swabs them, then converts the result to doubles */
	GMT_LONG i;
	unsigned int *ii;
	static float GMT_f[GMT_MAX_COLUMNS];
	
	if (GMT_read_binary_f_input (fp, GMT_f, n)) return (TRUE);	/* EOF or came up short */
	for (i = 0; i < n; i++) {	/* Do the float swab, then assign to the double */
		ii = (unsigned int *)&GMT_f[i];	/* These 2 lines do the swab */
		*ii = GMT_swab4 (*ii);
		GMT_curr_rec[i] = (double)GMT_f[i];
	}
	return (FALSE);	/* OK so far */
}

GMT_LONG GMT_read_binary_f_input (FILE *fp, float *GMT_f, GMT_LONG n) {
	/* Reads the n floats */
	GMT_LONG n_read;
	if ((n_read = GMT_fread ((void *) GMT_f, sizeof (float), (size_t)n, fp)) != n) {	/* EOF or came up short */
		GMT_io.status = (feof (fp)) ? GMT_IO_EOF : GMT_IO_MISMATCH;
		if (GMT_io.give_report && GMT_io.n_bad_records) {	/* Report summary and reset */
			fprintf (stderr, "%s: This file had %ld records with invalid x and/or y values\n", GMT_program, GMT_io.n_bad_records);
			GMT_io.n_bad_records = GMT_io.rec_no = GMT_io.pt_no = GMT_io.n_clean_rec = 0;
		}
		return (TRUE);	/* Done with this file since we got EOF */
	}
	return (FALSE);	/* OK so far */
}

GMT_LONG GMT_process_binary_input (GMT_LONG n_read) {
	/* Process a binary record to determine what kind of record it is. Return values:
	 * 0 = regular record; 1 = segment header (all NaNs); 2 = skip this record
	*/
	GMT_LONG col_no, n_NaN;
	GMT_LONG bad_record = FALSE, set_nan_flag = FALSE;
	/* Here, GMT_curr_rec has been filled in by fread */
	
	/* Determine if this was a multisegment header, and if so return */
	for (col_no = n_NaN = 0; col_no < n_read; col_no++) {
		if (!GMT_is_dnan (GMT_curr_rec[col_no])) continue;	/* Clean data, do nothing */
		/* We end up here if we found a NaN */
		if (!gmtdefs.nan_is_gap && GMT_io.skip_if_NaN[col_no]) bad_record = TRUE;	/* This field is not allowed to be NaN */
		if (GMT_io.skip_if_NaN[col_no]) set_nan_flag = TRUE;
		n_NaN++;
	}
	if (!GMT_io.status && GMT_io.multi_segments[GMT_IN]) {	/* Must have n_read NaNs and -m set to qualify as segment header */
		if (n_NaN == n_read) {
			GMT_io.status = GMT_IO_SEGMENT_HEADER;
			strcpy (GMT_io.segment_header, "> Binary multisegment header\n");
			GMT_io.seg_no++;
			GMT_io.pt_no = 0;
			return (1);	/* 1 means segment header */
		}
	}
	if (bad_record) {
		GMT_io.n_bad_records++;
		if (GMT_io.give_report && (GMT_io.n_bad_records == 1)) {	/* Report 1st occurrence */
			fprintf (stderr, "%s: Encountered first invalid binary record near/at line # %ld\n", GMT_program, GMT_io.rec_no);
			fprintf (stderr, "%s: Likely causes:\n", GMT_program);
			fprintf (stderr, "%s: (1) Invalid x and/or y values, i.e. NaNs.\n", GMT_program);
			fprintf (stderr, "%s: (2) Input file in multiple segment format but the -m switch is not set.\n", GMT_program);
		}
		return (2);	/* 2 means skip this record and try again */
	}
	else if (GMT_io.skip_duplicates && GMT_io.pt_no) {	/* Skip duplicate records with same x,y */
		if (GMT_curr_rec[GMT_X] == GMT_prev_rec[GMT_X] && GMT_curr_rec[GMT_Y] == GMT_prev_rec[GMT_Y]) return (2);
	}
	if (gmtdefs.xy_toggle[GMT_IN]) d_swap (GMT_curr_rec[GMT_X], GMT_curr_rec[GMT_Y]);	/* Got lat/lon instead of lon/lat */
	if (GMT_io.in_col_type[GMT_X] & GMT_IS_GEO) GMT_adjust_periodic ();		/* Must account for periodicity in 360 */
	if (set_nan_flag) GMT_io.status |= GMT_IO_NAN;
	return (0);	/* 0 means OK regular record */
}

void GMT_adjust_periodic (void) {
	/* while (GMT_curr_rec[GMT_X] > project_info.e) GMT_curr_rec[GMT_X] -= 360.0;
	while (GMT_curr_rec[GMT_X] < project_info.w) GMT_curr_rec[GMT_X] += 360.0; */
	while (GMT_curr_rec[GMT_X] > project_info.e && (GMT_curr_rec[GMT_X] - 360.0) >= project_info.w) GMT_curr_rec[GMT_X] -= 360.0;
	while (GMT_curr_rec[GMT_X] < project_info.w && (GMT_curr_rec[GMT_X] + 360.0) <= project_info.w) GMT_curr_rec[GMT_X] += 360.0;
	/* If data is not inside the given range it will satisfy (lon > east) */
	/* Now it will be outside the region on the same side it started out at */
}

GMT_LONG GMT_ascii_output (FILE *fp, GMT_LONG n, double *ptr)
{
	GMT_LONG i, col, last, e = 0, wn = 0;

	last = n - 1;						/* Last record, need to output linefeed instead of delimiter */

	for (i = 0; i < n && e >= 0; i++) {			/* Keep writing all fields unless there is a read error (e == -1) */
		col = (gmtdefs.xy_toggle[GMT_OUT] && i < 2) ? 1 - i : i;	/* Write lat/lon instead of lon/lat */
		e = GMT_ascii_output_one (fp, ptr[col], col);	/* Write one item without any separator at the end */

		if (i == last)					/* This is the last field, must add newline */
			putc ('\n', fp);
		else if (gmtdefs.field_delimiter[0])		/* Not last field, and a separator is required */
			fprintf (fp, "%s", gmtdefs.field_delimiter);

		wn += e;
	}
	return ((e < 0) ? e : wn);
}

void GMT_ascii_format_one (char *text, double x, GMT_LONG type)
{

	if (GMT_is_dnan (x)) {
		sprintf (text, "NaN");
		return;
	}
	switch (type) {
		case GMT_IS_LON:
			GMT_format_geo_output (FALSE, x, text);
			break;
		case GMT_IS_LAT:
			GMT_format_geo_output (TRUE, x, text);
			break;
		case GMT_IS_ABSTIME:
			GMT_format_abstime_output (x, text);
			break;
		default:
			sprintf (text, gmtdefs.d_format, x);
			break;
	}
}

GMT_LONG GMT_ascii_output_one (FILE *fp, double x, GMT_LONG col)
{
	char text[GMT_LONG_TEXT];

	GMT_ascii_format_one (text, x, GMT_io.out_col_type[col]);
	return (fprintf (fp, "%s", text));
}

void GMT_lon_range_adjust (GMT_LONG range, double *lon)
{
	switch (range) {	/* Adjust to the desired range */
		case 0:		/* Make 0 <= lon < 360 */
			while ((*lon) < 0.0) (*lon) += 360.0;
			while ((*lon) >= 360.0) (*lon) -= 360.0;
			break;
		case 1:		/* Make -360 < lon <= 0 */
			while ((*lon) <= -360.0) (*lon) += 360.0;
			while ((*lon) > 0) (*lon) -= 360.0;
			break;
		default:	/* Make -180 < lon < +180 */
			while ((*lon) < -180.0) (*lon) += 360.0;
			while ((*lon) > 180.0) (*lon) -= 360.0;
			break;
	}
}

GMT_LONG GMT_points_are_antipodal (double lonA, double latA, double lonB, double latB)
/* Returns TRUE if the points are antipodal, FALSE otherwise */
{
	double dellon;
	GMT_LONG antipodal = FALSE;

	if (latA == -latB) {
		dellon = lonA - lonB;
		GMT_lon_range_adjust (2, &dellon);
		if (dellon > +180.0) dellon -= 360.0;
		if (dellon < -180.0) dellon += 360.0;
		if (dellon == +180.0 || dellon == -180.0) antipodal = TRUE;
	}
	return (antipodal);
}

void GMT_format_geo_output (GMT_LONG is_lat, double geo, char *text)
{
	GMT_LONG k, n_items, d, m, s, m_sec;
	char letter;
	GMT_LONG minus;

	if (!is_lat) GMT_lon_range_adjust (GMT_io.geo.range, &geo);
	if (GMT_io.geo.decimal) {	/* Easy */
		sprintf (text, gmtdefs.d_format, geo);
		return;
	}

	if (GMT_io.geo.wesn) {	/* Trailing WESN */
		if (is_lat)
			letter = (GMT_IS_ZERO (geo)) ? 0 : ((geo < 0.0) ? 'S' : 'N');
		else
			letter = (GMT_IS_ZERO (geo) || GMT_IS_ZERO (geo - 180.0)) ? 0 : ((geo < 0.0) ? 'W' : 'E');
		geo = fabs (geo);
	}
	else	/* No letter means we print the NULL character */
		letter = 0;

	for (k = n_items = 0; k < 3; k++) if (GMT_io.geo.order[k] >= 0) n_items++;	/* How many of d, m, and s are requested as integers */
	minus = GMT_geo_to_dms (geo, n_items, GMT_io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
	if (minus) text[0] = '-';	/* Must manually insert leading minus sign when degree == 0 */
	if (GMT_io.geo.n_sec_decimals) {		/* Wanted fraction printed */
		if (n_items == 3)
			sprintf (&text[minus], GMT_io.geo.y_format, d, m, s, m_sec, letter);
		else if (n_items == 2)
			sprintf (&text[minus], GMT_io.geo.y_format, d, m, m_sec, letter);
		else
			sprintf (&text[minus], GMT_io.geo.y_format, d, m_sec, letter);
	}
	else if (n_items == 3)
		sprintf (&text[minus], GMT_io.geo.y_format, d, m, s, letter);
	else if (n_items == 2)
		sprintf (&text[minus], GMT_io.geo.y_format, d, m, letter);
	else
		sprintf (&text[minus], GMT_io.geo.y_format, d, letter);
}

GMT_LONG GMT_geo_to_dms (double val, GMT_LONG n_items, double fact, GMT_LONG *d, GMT_LONG *m,  GMT_LONG *s,  GMT_LONG *ix)
{
	/* Convert floating point degrees to dd:mm[:ss][.xxx].  Returns TRUE if d = 0 and val is negative */
	GMT_LONG minus;
	GMT_LONG isec, imin;
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
	else if (n_items == 2) {		/* Want dd:mm[.xx] format */
		min = GMT_DEG2MIN_F * fabs (val) + step;	/* Convert to minutes */
		imin = (GMT_LONG)floor (min);			/* Integer minutes */
		fmin = min - (double)imin;  			/* Leftover fractional minute */
		*d = imin / GMT_DEG2MIN_I;			/* Integer degrees */
		imin -= ((*d) * GMT_DEG2MIN_I);			/* Left-over seconds in the last degree */
		*m = imin;					/* Integer minutes */
		*s = 0;						/* No seconds */
		*ix = (GMT_LONG)floor (fmin * fact);		/* Fractional minutes scaled to integer */
	}
	else {		/* Want dd[.xx] format */
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

void GMT_format_abstime_output (double dt, char *text)
{
	char date[GMT_CALSTRING_LENGTH], clock[GMT_CALSTRING_LENGTH];

	GMT_format_calendar (date, clock, &GMT_io.date_output, &GMT_io.clock_output, FALSE, 1, dt);
	sprintf (text, "%sT%s", date, clock);
}

GMT_LONG GMT_bin_double_output (FILE *fp, GMT_LONG n, double *ptr)
{
	GMT_LONG i;
	if (gmtdefs.xy_toggle[GMT_OUT]) d_swap (ptr[GMT_X], ptr[GMT_Y]);	/* Write lat/lon instead of lon/lat */
	for (i = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_LON) GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
	}

	return (GMT_fwrite ((void *) ptr, sizeof (double), (size_t)n, fp));
}

GMT_LONG GMT_bin_double_output_swab (FILE *fp, GMT_LONG n, double *ptr)
{	/* Binary output after swabing the data.  Use temp variable d so we dont modify the original data */
	GMT_LONG i, k;
	unsigned int *ii, jj;
	double d;
	void *vptr;

	if (gmtdefs.xy_toggle[GMT_OUT]) d_swap (ptr[GMT_X], ptr[GMT_Y]);	/* Write lat/lon instead of lon/lat */
	for (i = k = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_LON) GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
		/* Do the 8-byte swabbing */
		d = ptr[i];
		vptr = (void *)&d;
		ii = (unsigned int *)vptr;
		jj = GMT_swab4 (ii[0]);
		ii[0] = GMT_swab4 (ii[1]);
		ii[1] = jj;
		k += GMT_fwrite ((void *) &d, sizeof (double), (size_t)1, fp);
	}

	return (k);
}

GMT_LONG GMT_bin_float_output (FILE *fp, GMT_LONG n, double *ptr)
{
	GMT_LONG i;
	static float GMT_f[GMT_MAX_COLUMNS];

	if (gmtdefs.xy_toggle[GMT_OUT]) d_swap (ptr[GMT_X], ptr[GMT_Y]);	/* Write lat/lon instead of lon/lat */
	for (i = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_LON) GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
		GMT_f[i] = (float) ptr[i];
	}
	return (GMT_fwrite ((void *) GMT_f, sizeof (float), (size_t)n, fp));
}

GMT_LONG GMT_bin_float_output_swab (FILE *fp, GMT_LONG n, double *ptr)
{	/* Binary output after swabing the data. */
	GMT_LONG i, k;
	unsigned int *ii;
	static float GMT_f[GMT_MAX_COLUMNS];

	if (gmtdefs.xy_toggle[GMT_OUT]) d_swap (ptr[GMT_X], ptr[GMT_Y]);	/* Write lat/lon instead of lon/lat */
	for (i = k = 0; i < n; i++) {
		if (GMT_io.out_col_type[i] == GMT_IS_LON) GMT_lon_range_adjust (GMT_io.geo.range, &ptr[i]);
		GMT_f[i] = (float) ptr[i];
		ii = (unsigned int *)&GMT_f[i];
		*ii = GMT_swab4 (*ii);
		k += GMT_fwrite ((void *) &GMT_f[i], sizeof (float), (size_t)1, fp);
	}
	return (k);
}

void GMT_write_segmentheader (FILE *fp, GMT_LONG n)
{
	/* Output ASCII or binary multisegment header.
	 * ASCII header is expected to contain newline (\n) */

	GMT_LONG i;
	if (!GMT_io.multi_segments[GMT_OUT]) return;	/* No output segments requested */
	if (GMT_io.binary[GMT_OUT])
		for (i = 0; i < n; i++) GMT_output (fp, 1, &GMT_d_NaN);
	else if (GMT_io.segment_header[0] == '\0')	/* Most likely binary input with NaN-headers */
		fprintf (fp, "%c\n", GMT_io.EOF_flag[GMT_OUT]);
	else
		fprintf (fp, "%s", GMT_io.segment_header);
}

GMT_LONG GMT_init_z_io (char format[], GMT_LONG repeat[], GMT_LONG swab, GMT_LONG skip, char type, struct GMT_Z_IO *r)
{
	GMT_LONG first = TRUE;
	GMT_LONG k;

	memset ((void *)r, 0, sizeof (struct GMT_Z_IO));

	for (k = 0; k < 2; k++) {	/* Loop over the two format flags */

		switch (format[k]) {

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
			default:
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Z: %c not a valid format specifier!\n", GMT_program, format[k]);
				GMT_exit (EXIT_FAILURE);
				break;

		}
	}

	r->x_missing = repeat[GMT_X];
	r->y_missing = repeat[GMT_Y];
	r->skip = skip;
	r->swab = swab;

	switch (type) {	/* Set read pointer depending on data format */
		case 'A':	/* ASCII with more than one per record */
			r->read_item = GMT_A_read;	r->write_item = GMT_a_write;
			r->binary = FALSE;
			break;

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
			fprintf (stderr, "%s: GMT SYNTAX ERROR -Z: %c not a valid data type!\n", GMT_program, type);
			GMT_exit (EXIT_FAILURE);
			break;
	}

	if (r->binary) {
		strcpy (GMT_io.r_mode, "rb");
		strcpy (GMT_io.w_mode, "wb");
		strcpy (GMT_io.a_mode, "ab+");
	}

	return (GMT_NOERROR);
}

GMT_LONG GMT_set_z_io (struct GMT_Z_IO *r, struct GRD_HEADER *h)
{
	if ((r->x_missing || r->y_missing) && h->node_offset == 1) return (GMT_GRDIO_RI_NOREPEAT);

	r->start_col = (GMT_LONG)((r->x_step == 1) ? 0 : h->nx - 1 - r->x_missing);
	r->start_row = (GMT_LONG)((r->y_step == 1) ? r->y_missing : h->ny - 1);
	r->get_gmt_ij = (r->format == GMT_COLUMN_FORMAT) ? GMT_col_ij : GMT_row_ij;
	r->nx = (GMT_LONG)h->nx;
	r->ny = (GMT_LONG)h->ny;
	r->x_period = r->nx - r->x_missing;
	r->y_period = r->ny - r->y_missing;
	r->n_expected = r->x_period * r->y_period;
	GMT_do_swab = r->swab;
	return (GMT_NOERROR);
}

void GMT_check_z_io (struct GMT_Z_IO *r, float *a)
{
	/* Routine to fill in the implied periodic row or column that was missing */

	GMT_LONG i, j, k;

	if (r->x_missing) for (j = 0; j < r->ny; j++) a[(j+1)*r->nx-1] = a[j*r->nx];
	if (r->y_missing) for (i = 0, k = (r->ny-1)*r->nx; i < r->nx; i++) a[i] = a[k+i];
}

/* NOTE: In the following we check GMT_io.in_col_type[2] and GMT_io.out_col_type[2] for formatting help for the first column.
 * We use column 3 ([2]) instead of the first ([0]) since we really are dealing with the z in z (x,y) here
 * and the x,y are implicit from the -R -I arguments.
 */

GMT_LONG GMT_A_read (FILE *fp, double *d)
{	/* Can read one or more items from input records. Limitation is that they must be floating point values (no dates or ddd:mm:ss) */
	GMT_LONG i;
	if ((i = fscanf (fp, "%lg", d)) <= 0) return (0);	/* Read was unsuccessful */
	return (1);
}

GMT_LONG GMT_a_read (FILE *fp, double *d)
{
	GMT_LONG i;
	char line[GMT_TEXT_LEN];
	if (!fgets (line, GMT_TEXT_LEN, fp)) return (0);	/* Read was unsuccessful */
	for (i = strlen(line) - 1; i >= 0 && strchr (" \t,\r\n", (int)line[i]); i--);	/* Take out trailing whitespace */
	line[++i] = '\0';
	GMT_scanf (line, GMT_io.in_col_type[2], d);	/* Convert whatever it is to double */
	return (1);
}

GMT_LONG GMT_c_read (FILE *fp, double *d)
{
	char c;
	if (!GMT_fread ((void *)&c, sizeof (char), (size_t)1, fp)) return (0);
	*d = (double) c;
	return (1);
}

GMT_LONG GMT_u_read (FILE *fp, double *d)
{
	unsigned char u;
	if (!GMT_fread ((void *)&u, sizeof (unsigned char), (size_t)1, fp)) return (0);
	*d = (double) u;
	return (1);
}

GMT_LONG GMT_h_read (FILE *fp, double *d)
{
	short int h;
	if (!GMT_fread ((void *)&h, sizeof (short int), (size_t)1, fp)) return (0);
	if (GMT_do_swab) h = GMT_swab2 (h);
	*d = (double) h;
	return (1);
}

GMT_LONG GMT_H_read (FILE *fp, double *d)
{
	unsigned short int h;
	if (!GMT_fread ((void *)&h, sizeof (unsigned short int), (size_t)1, fp)) return (0);
	if (GMT_do_swab) h = GMT_swab2 (h);
	*d = (double) h;
	return (1);
}

GMT_LONG GMT_i_read (FILE *fp, double *d)
{
	int i;
	if (!GMT_fread ((void *)&i, sizeof (int), (size_t)1, fp)) return (0);
	if (GMT_do_swab) i = GMT_swab4 (i);
	*d = (double) i;
	return (1);
}

GMT_LONG GMT_I_read (FILE *fp, double *d)
{
	unsigned int i;
	if (!GMT_fread ((void *)&i, sizeof (unsigned int), (size_t)1, fp)) return (0);
	if (GMT_do_swab) i = GMT_swab4 (i);
	*d = (double) i;
	return (1);
}

GMT_LONG GMT_l_read (FILE *fp, double *d)
{
	long int l;

	if (!GMT_fread ((void *)&l, sizeof (long int), (size_t)1, fp)) return (0);
	if (GMT_do_swab) {
		unsigned int *i, k;
		void *vptr;
		vptr = (void *)&l;
		i = (unsigned int *)vptr;
		for (k = 0; k < sizeof (long int)/4; k++) i[k] = GMT_swab4 (i[k]);
	}
	*d = (double) l;
	return (1);
}

GMT_LONG GMT_f_read (FILE *fp, double *d)
{
	float f;
	if (!GMT_fread ((void *)&f, sizeof (float), (size_t)1, fp)) return (0);
	if (GMT_do_swab) {
		unsigned int *i;
		void *vptr;
		vptr = (void *)&f;
		i = (unsigned int *)vptr;
		*i = GMT_swab4 (*i);
	}
	*d = (double) f;
	return (1);
}

GMT_LONG GMT_d_read (FILE *fp, double *d)
{
	if (!GMT_fread ((void *)d, sizeof (double), (size_t)1, fp)) return (0);
	if (GMT_do_swab) {
		unsigned int *i, j;
		i = (unsigned int *)d;
		j = GMT_swab4 (i[0]);
		i[0] = GMT_swab4 (i[1]);
		i[1] = j;
	}
	return (1);
}

GMT_LONG GMT_a_write (FILE *fp, double d)
{
	GMT_LONG n = 0;
	n = GMT_ascii_output_one (fp, d, 2);
	fprintf (fp, "\n");
	return (n);
}

GMT_LONG GMT_c_write (FILE *fp, double d)
{
	char c;
	c = (char) d;
	return (GMT_fwrite ((void *)&c, sizeof (char), (size_t)1, fp));
}

GMT_LONG GMT_u_write (FILE *fp, double d)
{
	unsigned char u;
	u = (unsigned char) d;
	return (GMT_fwrite ((void *)&u, sizeof (unsigned char), (size_t)1, fp));
}

GMT_LONG GMT_h_write (FILE *fp, double d)
{
	short int h;
	h = (short int) d;
	return (GMT_fwrite ((void *)&h, sizeof (short int), (size_t)1, fp));
}

GMT_LONG GMT_H_write (FILE *fp, double d)
{
	unsigned short int h;
	h = (unsigned short int) d;
	return (GMT_fwrite ((void *)&h, sizeof (unsigned short int), (size_t)1, fp));
}

GMT_LONG GMT_i_write (FILE *fp, double d)
{
	int i;
	i = (int) d;
	return (GMT_fwrite ((void *)&i, sizeof (int), (size_t)1, fp));
}

GMT_LONG GMT_I_write (FILE *fp, double d)
{
	unsigned int i;
	i = (unsigned int) d;
	return (GMT_fwrite ((void *)&i, sizeof (unsigned int), (size_t)1, fp));
}

GMT_LONG GMT_l_write (FILE *fp, double d)
{
	long int l;
	l = (long int) d;
	return (GMT_fwrite ((void *)&l, sizeof (long int), (size_t)1, fp));
}

GMT_LONG GMT_f_write (FILE *fp, double d)
{
	float f;
	f = (float) d;
	return (GMT_fwrite ((void *)&f, sizeof (float), (size_t)1, fp));
}

GMT_LONG GMT_d_write (FILE *fp, double d)
{
	return (GMT_fwrite ((void *)&d, sizeof (double), (size_t)1, fp));
}

void GMT_col_ij (struct GMT_Z_IO *r, GMT_LONG ij, GMT_LONG *gmt_ij)
{
	/* Translates incoming ij to gmt_ij for column-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij % r->y_period);
	r->gmt_i = r->start_col + r->x_step * (ij / r->y_period);

	*gmt_ij = r->gmt_j * r->nx + r->gmt_i;
}

void GMT_row_ij (struct GMT_Z_IO *r, GMT_LONG ij, GMT_LONG *gmt_ij)
{

	/* Translates incoming ij to gmt_ij for row-structured data */

	r->gmt_j = r->start_row + r->y_step * (ij / r->x_period);
	r->gmt_i = r->start_col + r->x_step * (ij % r->x_period);

	*gmt_ij = r->gmt_j * r->nx + r->gmt_i;
}

GMT_LONG GMT_get_ymdj_order (char *text, struct GMT_DATE_IO *S, GMT_LONG mode)
{	/* Reads a YYYY-MM-DD or YYYYMMDD-like string and determines order.
	 * order[0] is the order of the year, [1] is month, etc.
	 * Items not encountered are left as -1. mode is 0 for text i/o
	 * and 1 for plot format.
	 */

	GMT_LONG i, j, order, n_y, n_m, n_d, n_j, n_w, n_delim, last, error = 0;

	for (i = 0; i < 4; i++) S->item_order[i] = S->item_pos[i] = -1;	/* Meaning not encountered yet */

	n_y = n_m = n_d = n_j = n_w = n_delim = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;

	i = 0;
	if (text[i] == '-') {	/* Leading hyphen means use %d and not %x.xd for integer formats */
		S->compact = TRUE;
		i++;
	}
	for (order = 0; i < (int)strlen (text); i++) {
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
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_get_hms_order (char *text, struct GMT_CLOCK_IO *S)
{	/* Reads a HH:MM:SS or HHMMSS-like string and determines order.
	 * hms_order[0] is the order of the hour, [1] is min, etc.
	 * Items not encountered are left as -1.
	 */

	GMT_LONG i, j, order, n_delim, sequence[3], last, n_h, n_m, n_s, n_x, n_dec, error = 0;
	GMT_LONG big_to_small;
	char *p;
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
		fprintf (stderr, "%s: ERROR: Unacceptable clock template %s\n", GMT_program, text);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_get_dms_order (char *text, struct GMT_GEO_IO *S)
{	/* Reads a ddd:mm:ss-like string and determines order.
	 * order[0] is the order of the degree, [1] is minutes, etc.
	 * Order is checked since we only allow d, m, s in that order.
	 * Items not encountered are left as -1.
	 */

	GMT_LONG i1, i, j, order, n_d, n_m, n_s, n_x, n_dec, sequence[3], n_delim, last, error = 0;
	GMT_LONG big_to_small;

	for (i = 0; i < 3; i++) S->order[i] = -1;	/* Meaning not encountered yet */

	n_d = n_m = n_s = n_x = n_dec = n_delim = 0;
	S->delimiter[0][0] = S->delimiter[0][1] = S->delimiter[1][0] = S->delimiter[1][1] = 0;
	sequence[0] = sequence[1] = sequence[2] = -1;

	S->range = 2;			/* -180/+180 range, may be overwritten below by + or - */
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
		fprintf (stderr, "%s: ERROR: Unacceptable dmmss template %s\n", GMT_program, text);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
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

void GMT_clock_C_format (char *form, struct GMT_CLOCK_IO *S, GMT_LONG mode)
{
	/* Determine the order of H, M, S in input and output clock strings,
	 * as well as the number of decimals in output seconds (if any), and
	 * if a 12- or 24-hour clock is used.
	 * mode is 0 for input and 1 for output format
	 */

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	GMT_get_hms_order (form, S);

	/* Craft the actual C-format to use for input/output clock strings */

	if (S->order[0] >= 0) {	/* OK, at least hours is needed */
		char fmt[GMT_LONG_TEXT];
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

void GMT_date_C_format (char *form, struct GMT_DATE_IO *S, GMT_LONG mode)
{
	/* Determine the order of Y, M, D, J in input and output date strings.
	* mode is 0 for input, 1 for output, and 2 for plot output.
	 */

	char fmt[GMT_LONG_TEXT];
	GMT_LONG k, ywidth;
	GMT_LONG no_delim;

	/* Get the order of year, month, day or day-of-year in input/output formats for dates */

	GMT_get_ymdj_order (form, S, mode);

	/* Craft the actual C-format to use for i/o date strings */

	no_delim = !(S->delimiter[0][0] || S->delimiter[1][0]);	/* TRUE for things like yyyymmdd */
	ywidth = (no_delim) ? 4 : 4+(!mode);			/* 4 or 5, depending on values */
	if (S->item_order[0] >= 0 && S->iso_calendar) {	/* ISO Calendar string: At least Ione item is needed */
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

GMT_LONG GMT_geo_C_format (char *form, struct GMT_GEO_IO *S)
{
	/* Determine the output of geographic location formats. */

	GMT_get_dms_order (form, S);	/* Get the order of degree, min, sec in output formats */

	if (S->no_sign) return (GMT_IO_BAD_PLOT_DEGREE_FORMAT);

	if (S->decimal) {	/* Plain decimal degrees */
		sprintf (S->x_format, "%s", gmtdefs.d_format);
		sprintf (S->y_format, "%s", gmtdefs.d_format);
	}
	else {			/* Some form of dd:mm:ss */
		char fmt[GMT_LONG_TEXT];
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
		sprintf (fmt, "%%c");
		strcat (S->x_format, fmt);
		strcat (S->y_format, fmt);
	}
	return (GMT_NOERROR);
}

void GMT_plot_C_format (char *form, struct GMT_GEO_IO *S)
{
	GMT_LONG i, j;

	/* Determine the plot geographic location formats. */

	for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) GMT_plot_format[i][j] = CNULL;

	GMT_get_dms_order (form, S);	/* Get the order of degree, min, sec in output formats */

	if (S->decimal) {	/* Plain decimal degrees */
		GMT_LONG len;
		len = sprintf (S->x_format, "%s", gmtdefs.d_format);
		      sprintf (S->y_format, "%s", gmtdefs.d_format);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			S->x_format[len] = (char)gmtdefs.encoding.code[gmtdefs.degree_symbol];
			S->y_format[len] = (char)gmtdefs.encoding.code[gmtdefs.degree_symbol];
			S->x_format[len+1] = S->y_format[len+1] = '\0';
		}
		strcat (S->x_format, "%c");
		strcat (S->y_format, "%c");
	}
	else {			/* Must cover all the 6 forms of dd[:mm[:ss]][.xxx] */
		char fmt[GMT_LONG_TEXT];

		for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) GMT_plot_format[i][j] = GMT_memory (VNULL, GMT_LONG_TEXT, sizeof (char), GMT_program);

		/* Level 0: degrees only. index 0 is integer degrees, index 1 is [possibly] fractional degrees */

		sprintf (GMT_plot_format[0][0], "%%d");		/* ddd */
		if (S->order[1] == -1 && S->n_sec_decimals > 0) /* ddd.xxx format */
			sprintf (GMT_plot_format[0][1], "%%" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd format */
			sprintf (GMT_plot_format[0][1], "%%" GMT_LL "d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* But we want the degree symbol appended */
			sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[0][0], fmt);
			strcat (GMT_plot_format[0][1], fmt);
		}

		/* Level 1: degrees and minutes only. index 0 is integer minutes, index 1 is [possibly] fractional minutes  */

		sprintf (GMT_plot_format[1][0], "%%" GMT_LL "d");	/* ddd */
		sprintf (GMT_plot_format[1][1], "%%" GMT_LL "d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[1][0], fmt);
			strcat (GMT_plot_format[1][1], fmt);
		}
		strcat (GMT_plot_format[1][0], "%2.2" GMT_LL "d");
		if (S->order[2] == -1 && S->n_sec_decimals > 0) /* ddd:mm.xxx format */
			sprintf (fmt, "%%2.2" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm format */
			sprintf (fmt, "%%2.2" GMT_LL "d");
		strcat (GMT_plot_format[1][1], fmt);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_squote]);
			strcat (GMT_plot_format[1][0], fmt);
			strcat (GMT_plot_format[1][1], fmt);
		}

		/* Level 2: degrees, minutes, and seconds. index 0 is integer seconds, index 1 is [possibly] fractional seconds  */

		sprintf (GMT_plot_format[2][0], "%%" GMT_LL "d");
		sprintf (GMT_plot_format[2][1], "%%" GMT_LL "d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the degree symbol appended */
			sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmtdefs.degree_symbol]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}
		strcat (GMT_plot_format[2][0], "%2.2" GMT_LL "d");
		strcat (GMT_plot_format[2][1], "%2.2" GMT_LL "d");
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the minute symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_squote]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}
		strcat (GMT_plot_format[2][0], "%2.2" GMT_LL "d");
		if (S->n_sec_decimals > 0)			 /* ddd:mm:ss.xxx format */
			sprintf (fmt, "%%" GMT_LL "d.%%%" GMT_LL "d.%" GMT_LL "d" GMT_LL "d", S->n_sec_decimals, S->n_sec_decimals);
		else						/* ddd:mm:ss format */
			sprintf (fmt, "%%2.2" GMT_LL "d");
		strcat (GMT_plot_format[2][1], fmt);
		if (gmtdefs.degree_symbol != gmt_none)
		{	/* We want the second symbol appended */
			if (gmtdefs.degree_symbol == gmt_colon)
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_colon]);
			else
				sprintf (fmt, "%c", (int)gmtdefs.encoding.code[gmt_dquote]);
			strcat (GMT_plot_format[2][0], fmt);
			strcat (GMT_plot_format[2][1], fmt);
		}

		/* Finally add %c for the W,E,S,N char (or NULL) */

		for (i = 0; i < 3; i++) for (j = 0; j < 2; j++) strcat (GMT_plot_format[i][j], "%c");
	}
}

GMT_LONG GMT_parse_f_option (char *arg)
{
	/* Routine will decode the -f[i|o]<col>|<colrange>[t|T|g],... arguments */

	char copy[BUFSIZ], p[BUFSIZ], *c;
	GMT_LONG i, k = 1, ic, pos = 0, code, *col = VNULL;
	GMT_LONG both_i_and_o = FALSE, start = -1, stop = -1;

	if (arg[0] == 'i')	/* Apply to input columns only */
		col = GMT_io.in_col_type;
	else if (arg[0] == 'o')	/* Apply to output columns only */
		col = GMT_io.out_col_type;
	else {			/* Apply to both input and output columns */
		both_i_and_o = TRUE;
		k = 0;
	}

	memset ((void *)copy, 0, (size_t)BUFSIZ);	/* Clean the copy */
	strncpy (copy, &arg[k], (size_t)BUFSIZ);	/* arg should NOT have a leading i|o part */

	if (copy[0] == 'g') {	/* Got -f[i|o]g which is shorthand for -f[i|o]0x,1y */
		if (both_i_and_o) {
			GMT_io.in_col_type[GMT_X] = GMT_io.out_col_type[GMT_X] = GMT_IS_LON;
			GMT_io.in_col_type[GMT_Y] = GMT_io.out_col_type[GMT_Y] = GMT_IS_LAT;
		}
		else {
			col[GMT_X] = GMT_IS_LON;
			col[GMT_Y] = GMT_IS_LAT;
		}
		return (0);
	}

	while ((GMT_strtok (copy, ",", &pos, p))) {	/* While it is not empty, process it */
		if ((c = strchr (p, '-')))	/* Range of columns given. e.g., 7-9T */
			sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
		else if (isdigit ((int)p[0]))	/* Just a single column, e.g., 3t */
			start = stop = atoi (p);
		else				/* Just assume it goes column by column */
			start++, stop++;

		ic = (int) p[strlen(p)-1];	/* Last char in p is the potential code T, t, or g */
		switch (ic) {
			case 'T':	/* Absolute calendar time */
				code = GMT_IS_ABSTIME;
				break;
			case 't':	/* Relative time (units since epoch) */
				code = GMT_IS_RELTIME;
				break;
			case 'x':	/* Longitude coordinates */
				code = GMT_IS_LON;
				break;
			case 'y':	/* Latitude coordinates */
				code = GMT_IS_LAT;
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
	}
	return (0);
}

GMT_LONG	GMT_scanf_clock (char *s, double *val)
{
	/* On failure, return -1.  On success, set val and return 0.

	Looks for apAP, but doesn't discover a failure if called with "11:13:15 Hello, Walter",
	because it will find an a.

	Doesn't check whether use of a or p matches stated intent to use twelve_hour_clock.

	ISO standard allows 24:00:00, so 86400 is not too big.
	If the day of this clock might be a day with a leap second, (this routine doesn't know that)
	then we should also allow 86401.  A value exceeding 86401 is an error.
	*/

	GMT_LONG	k, hh, mm, add_noon = 0;
	GMT_LONG	hh_limit = 24;	/* ISO std allows 24:00:00  */
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

GMT_LONG	GMT_scanf_calendar (char *s, GMT_cal_rd *rd)
{
	/* On failure, return -1.  On success, set rd and return 0 */
	if (GMT_io.date_input.iso_calendar) {
		return (GMT_scanf_ISO_calendar (s, rd));
	}
	return (GMT_scanf_g_calendar (s, rd));
}

GMT_LONG	GMT_scanf_ISO_calendar (char *s, GMT_cal_rd *rd) {

	/* On failure, return -1.  On success, set rd and return 0.
	Assumes that year, week of year, day of week appear in that
	order only, and that the format string can handle the W.
	Assumes also that it is always OK to fill in missing bits.  */

	GMT_LONG	k, n, ival[3];

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

GMT_LONG	GMT_scanf_g_calendar (char *s, GMT_cal_rd *rd)
{
	/* Return -1 on failure.  Set rd and return 0 on success.

	For gregorian calendars.  */

	GMT_LONG	k, ival[4];
	char month[16];

	if (GMT_io.date_input.day_of_year) {
		/* Calendar uses year and day of year format.  */
		if ( (k = sscanf (s, GMT_io.date_input.format,
			&ival[GMT_io.date_input.item_order[0]],
			&ival[GMT_io.date_input.item_order[1]]) ) == 0) return (-1);
		if (k < 2) {
			if (!GMT_io.date_input.truncated_cal_is_ok) return (-1);
			ival[1] = 1;	/* Set first day of year  */
		}
		if (GMT_io.date_input.Y2K_year) {
			if (ival[0] < 0 || ival[0] > 99) return (-1);
			ival[0] = GMT_y2_to_y4_yearfix (ival[0]);
		}
		k = (GMT_is_gleap(ival[0])) ? 366 : 365;
		if (ival[3] < 1 || ival[3] > k) return (-1);
		*rd = GMT_rd_from_gymd (ival[0], 1, 1) + ival[3] - 1;
		return (0);
	}

	/* Get here when calendar type has months and days of months.  */

	if (GMT_io.date_input.mw_text) {	/* Have month name abbreviation in data format */
		switch (GMT_io.date_input.item_pos[1]) {	/* Order of month in data string */
			case 0:	/* e.g., JAN-24-1987 or JAN-1987-24 */
				k = sscanf (s, GMT_io.date_input.format, month, &ival[GMT_io.date_input.item_order[1]], &ival[GMT_io.date_input.item_order[2]]);
				GMT_str_toupper (month);
				ival[1] = GMT_hash_lookup (month, GMT_month_hashnode, 12, 12) + 1;
				break;
			case 1:	/* e.g., 24-JAN-1987 or 1987-JAN-24 */
				k = sscanf (s, GMT_io.date_input.format, &ival[GMT_io.date_input.item_order[0]], month, &ival[GMT_io.date_input.item_order[2]]);
				GMT_str_toupper (month);
				ival[1] = GMT_hash_lookup (month, GMT_month_hashnode, 12, 12) + 1;
				break;
			case 2:	/* e.g., JAN-24-1987 ? */
				k = sscanf (s, GMT_io.date_input.format, month, &ival[GMT_io.date_input.item_order[1]], &ival[GMT_io.date_input.item_order[2]]);
				GMT_str_toupper (month);
				ival[1] = GMT_hash_lookup (month, GMT_month_hashnode, 12, 12) + 1;
				break;
			default:
				k = 0;
				break;
		}
		if (k == 0) return (-1);
	}
	else if ( (k = sscanf (s, GMT_io.date_input.format,
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


GMT_LONG	GMT_scanf_geo (char *s, double *val)
{
	/* Try to read a character string token stored in s, knowing that it should be a geographical variable.
	If successful, stores value in val and returns one of GMT_IS_FLOAT, GMT_IS_GEO, GMT_IS_LAT, GMT_IS_LON,
	whichever can be determined from the format of s.
	If unsuccessful, does not store anything in val and returns GMT_IS_NAN.
	This should have essentially the same functionality as the GMT3.4 GMT_scanf, except that the expectation
	is now used and returned, and this also permits a double precision format in the minutes or seconds,
	and does more error checking.  However, this is not optimized for speed (yet).  WHFS, 16 Aug 2001

	Note:  Mismatch handling (e.g. this routine finds a lon but calling routine expected a lat) is not
	done here.
	*/

	char	scopy[GMT_TEXT_LEN], suffix, *p, *p2;
	double	dd, dm, ds;
	GMT_LONG	retval = GMT_IS_FLOAT, k, ncolons, negate = FALSE, id, im;

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
			case 'G':
			case 'g':
			case 'D':
			case 'd':
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
	if (k >= GMT_TEXT_LEN) return (GMT_IS_NAN);
	strncpy (scopy, s, (size_t)k);				/* Copy all but the suffix  */
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
			if ( (sscanf(scopy, "%" GMT_LL "d:%lf", &id, &dm) ) != 2) return (GMT_IS_NAN);
			dd = dm * GMT_MIN2DEG;
			if (id < 0) {	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			}
			else if (id > 0) {	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			}
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
		case 2:
			if ( (sscanf(scopy, "%" GMT_LL "d:%" GMT_LL "d:%lf", &id, &im, &ds) ) != 3) return (GMT_IS_NAN);
			dd = im * GMT_MIN2DEG + ds * GMT_SEC2DEG;
			if (id < 0) {	/* Negative degrees present, subtract the fractional part */
				dd = id - dd;
			}
			else if (id > 0) {	/* Positive degrees present, add the fractional part */
				dd = id + dd;
			}
			else {			/* degree part is 0; check if a leading sign is present */
				if (scopy[0] == '-') dd = -dd;	/* Make fraction negative */
			}
			break;
	}
	*val = (negate) ? -dd : dd;
	return (retval);
}


GMT_LONG	GMT_scanf_float (char *s, double *val)
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

	char	scopy[GMT_TEXT_LEN], *p;
	double	x;
	GMT_LONG	j,k;

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
	if (j > GMT_TEXT_LEN) return (GMT_IS_NAN);
	j -= k;
	strncpy (scopy, s, (size_t)j );
	scopy[j] = 'e';
	strcpy (&scopy[j+1], &p[1]);
	x = strtod(scopy, &p);
	if (p[0] != 0) return (GMT_IS_NAN);
	*val = x;
	return (GMT_IS_FLOAT);
}

GMT_LONG	GMT_scanf_dim (char *s, double *val)
{
	/* Try to decode a value from s and store
	in val.  s is a regular float with optional
	unit info, e.g., 8.5i or 7.5c.  If a valid unit
	is found we convert the number to inch.

	We return GMT_IS_FLOAT and pass val.
	*/

	if (isalpha ((int)s[0]))	/* Probably a symbol character; just return 0 */
		*val = 0.0;
	else
		*val = GMT_convert_units (s, GMT_INCH);
	return (GMT_IS_FLOAT);
}

GMT_LONG	GMT_scanf (char *s, GMT_LONG expectation, double *val)
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

	char	calstring[GMT_TEXT_LEN], clockstring[GMT_TEXT_LEN], *p;
	double	x;
	GMT_LONG	callen, clocklen;
	GMT_cal_rd rd;


	if (expectation & GMT_IS_GEO) {
		/* True if either a lat or a lon is expected  */
		return (GMT_scanf_geo (s, val));
	}

	else if (expectation == GMT_IS_FLOAT) {
		/* True if no special format is expected or allowed  */
		return (GMT_scanf_float (s, val));
	}

	else if (expectation == GMT_IS_DIMENSION) {
		/* True if units might be appended, e.g. 8.4i  */
		return (GMT_scanf_dim (s, val));
	}

	else if (expectation == GMT_IS_RELTIME) {
		/* True if we expect to read a float with no special
		formatting (except for an optional trailing 't'), and then
		assume it is relative time in user's units since epoch.  */
		callen = strlen (s) - 1;
		if (s[callen] == 't') s[callen] = '\0';
		if ((GMT_scanf_float (s, val)) == GMT_IS_NAN) return (GMT_IS_NAN);
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

		if ( (p = strchr ( s, (int)('T') ) ) == NULL) {
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
		if (clocklen && GMT_scanf_clock (clockstring, &x)) return (GMT_IS_NAN);
		rd = 1;
		if (callen && GMT_scanf_calendar (calstring, &rd)) return (GMT_IS_NAN);
		*val = GMT_rdc2dt (rd, x);
		if (gmtdefs.time_is_interval) {	/* Must truncate and center on time interval */
			GMT_moment_interval (&GMT_truncate_time.T, *val, TRUE);	/* Get the current interval */
			if (GMT_truncate_time.direction) {	/* Actually need midpoint of previous interval... */
				x = GMT_truncate_time.T.dt[0] - 0.5 * (GMT_truncate_time.T.dt[1] - GMT_truncate_time.T.dt[0]);
				GMT_moment_interval (&GMT_truncate_time.T, x, TRUE);	/* Get the current interval */
			}
			/* Now get half-point of interval */
			*val = 0.5 * (GMT_truncate_time.T.dt[1] + GMT_truncate_time.T.dt[0]);
		}
		return (GMT_IS_ABSTIME);
	}

	else if (expectation == GMT_IS_ARGTIME) {
		return (GMT_scanf_argtime (s, val));
	}

	else if (expectation & GMT_IS_UNKNOWN) {
		/* True if we dont know but must try both geographic or float formats  */
		return (GMT_scanf_geo (s, val));
	}

	else {
		fprintf (stderr, "GMT_LOGIC_BUG:  GMT_scanf() called with invalid expectation.\n");
		return (GMT_IS_NAN);
	}
}

GMT_LONG	GMT_scanf_argtime (char *s, double *t)
{
	/* s is a string from a command-line argument.
		The argument is known to refer to a time variable.  For example, the argument is
		a token from -R<t_min>/<t_max>/a/b[/c/d].  However, we will permit it to be in EITHER
		-- generic floating point format, in which case we interpret it as relative time
		   in user units since epoch;
		OR
		-- absolute time in a restricted format, which is to be converted to relative time.

		The absolute format must be restricted because we cannot use '/' as a delimiter in an arg
		string, but we might allow the user to use that in a data file (in gmtdefs.[in/out]put_date_format.
		Therefore we cannot use the user's date format string here, and we hard-wire something here.

		The relative format must be decodable by GMT_scanf_float().  It may optionally end in 't'
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

	double	ss, x;
	char 	*pw, *pt;
	GMT_LONG hh, mm, j, k, i, dash, ival[3], negate_year = FALSE, got_yd = FALSE;

	i = strlen(s)-1;
	if (s[i] == 't') s[i] = '\0';
	if ( (pt = strchr (s, (int)'T') ) == CNULL) {
		/* There is no T.  This must decode with GMT_scanf_float() or we die.  */
		if ( ( GMT_scanf_float (s, t) ) == GMT_IS_NAN) return (GMT_IS_NAN);
		return (GMT_IS_RELTIME);
	}
	x = 0.0;	/* x will be the seconds since start of today.  */
	if (pt[1]) {	/* There is a string following the T:  Decode a clock:  */
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
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-W%2" GMT_LL "d-%1" GMT_LL "d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
		if (GMT_iso_ywd_is_bad ((GMT_LONG)ival[0], (GMT_LONG)ival[1], (GMT_LONG)ival[2]) ) return (GMT_IS_NAN);
		*t = GMT_rdc2dt ( GMT_rd_from_iywd ((GMT_LONG)ival[0], (GMT_LONG)ival[1], (GMT_LONG)ival[2]), x);
		return (GMT_IS_ABSTIME);
	}

	for (i = negate_year; s[k+i] && s[k+i] != '-'; i++);;	/* Goes to first - between yyyy and jjj or yyyy and mm */
	dash = ++i;				/* Position of first character after the first dash (could be end of string if no dash) */
	while (s[k+i] && !(s[k+i] == '-' || s[k+i] == 'T')) i++;	/* Goto the ending T character or get stuck on a second - */
	got_yd = ((i - dash) == 3 && s[k+i] == 'T');		/* Must have a field of 3-characters between - and T to constitute a valid day-of-year format */

	if (got_yd) {	/* Gregorian yyyy-jjj calendar:  */
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-%3" GMT_LL "d", &ival[0], &ival[1]) ) != 2) return (GMT_IS_NAN);
		ival[2] = 1;
	}
	else {	/* Gregorian yyyy-mm-dd calendar:  */
		if ( (j = sscanf(&s[k], "%4" GMT_LL "d-%2" GMT_LL "d-%2" GMT_LL "d", &ival[0], &ival[1], &ival[2]) ) == 0) return (GMT_IS_NAN);
		for (k = j; k < 3; k++) ival[k] = 1;
	}
	if (negate_year) ival[0] = -ival[0];
	if (got_yd) {
		if (ival[1] < 1 || ival[1] > 366)  return (GMT_IS_NAN);	/* Simple range check on day-of-year (1-366) */
		*t = GMT_rdc2dt (GMT_rd_from_gymd ((GMT_LONG)ival[0], 1, 1) + (GMT_LONG)ival[1] - 1, x);
	}
	else {
		if (GMT_g_ymd_is_bad ((GMT_LONG)ival[0], (GMT_LONG)ival[1], (GMT_LONG)ival[2]) ) return (GMT_IS_NAN);
		*t = GMT_rdc2dt (GMT_rd_from_gymd ((GMT_LONG)ival[0], (GMT_LONG)ival[1], (GMT_LONG)ival[2]), x);
	}

	return (GMT_IS_ABSTIME);
}

GMT_LONG	GMT_scanf_arg (char *s, GMT_LONG expectation, double *val)
{
	/* Version of GMT_scanf used for cpt & command line arguments only (not data records).
	 * It differs from GMT_scanf in that if the expectation is GMT_IS_UNKNOWN it will
	 * check to see if the argument is (1) an absolute time string, (2) a geographical
	 * location string, or if not (3) a floating point string.  To ensure backward
	 * compatibility: if we encounter geographic data it will also set the GMT_io.type[]
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

	return (GMT_scanf (s, expectation, val));
}

GMT_LONG GMT_import_table (void *source, GMT_LONG source_type, struct GMT_TABLE **table, double dist, GMT_LONG greenwich, GMT_LONG poly, GMT_LONG use_GMT_io)
{
	/* Reads an entire multisegment data set into memory */

	char open_mode[4], file[BUFSIZ];
	GMT_LONG save, ascii, close_file = FALSE, no_segments;
	GMT_LONG n_fields, n_expected_fields, k, cdf = 0;
	GMT_LONG n_read = 0, seg = -1, n_row_alloc = GMT_CHUNK, n_seg_alloc = GMT_CHUNK, row = 0, col;
	double d, *in;
	FILE *fp;
	struct GMT_TABLE *T;
	PFL psave = NULL;

	if (use_GMT_io) {	/* Use GMT_io settings to determine if input is ascii/binary, else it defaults to ascii */
		n_expected_fields = (GMT_io.binary[GMT_IN]) ? GMT_io.ncol[GMT_IN] : GMT_MAX_COLUMNS;
		strcpy (open_mode, GMT_io.r_mode);
		ascii = !GMT_io.binary[GMT_IN];
	}
	else {			/* Force ASCII mode */
		n_expected_fields = GMT_MAX_COLUMNS;	/* GMT_input will return the number of columns */
		strcpy (open_mode, "r");
		ascii = TRUE;
		psave = GMT_input;		/* Save the previous pointer since we need to change it back at the end */
		GMT_input = GMT_input_ascii;	/* Override and use ascii mode */
		cdf = GMT_io.netcdf[GMT_IN];	/* Save any netcdf setting */
		GMT_io.netcdf[GMT_IN] = 0;	/* Wipe netcdf setting since GMT_fopen checks for it */
	}

	if (source_type == GMT_IS_FILE) {	/* source is a file name */
		strcpy (file, (char *)source);
		if ((fp = GMT_fopen (file, open_mode)) == NULL) {
			fprintf (stderr, "%s: Cannot open file %s\n", GMT_program, file);
			if (!use_GMT_io) {GMT_input = psave; GMT_io.netcdf[GMT_IN] = cdf;}	/* Restore previous setting */
			GMT_exit (EXIT_FAILURE);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (source_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)source;
		if (fp == GMT_stdin)
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input stream>");
	}
	else if (source_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd;
		fd = (int *)source;
		if ((fp = fdopen (*fd, open_mode)) == NULL) {
			fprintf (stderr, "%s: Cannot convert file descriptor %d to stream in GMT_import_table\n", GMT_program, *fd);
			if (!use_GMT_io) {GMT_input = psave; GMT_io.netcdf[GMT_IN] = cdf;}	/* Restore previous setting */
			GMT_exit (EXIT_FAILURE);
		}
		if (fp == GMT_stdin)
			strcpy (file, "<stdin>");
		else
			strcpy (file, "<input file descriptor>");
	}
	else {
		fprintf (stderr, "%s: Unrecognized source type %ld in GMT_import_table\n", GMT_program, source_type);
		GMT_exit (EXIT_FAILURE);
	}

	save = GMT_io.multi_segments[GMT_IN];	/* Must set this to TRUE temporarily since GMT_input uses GMT_io.multi_segments when reading */
	GMT_io.multi_segments[GMT_IN] = TRUE;

	n_fields = GMT_input (fp, &n_expected_fields, &in);
	if (GMT_io.status & GMT_IO_EOF) {
		if (gmtdefs.verbose) fprintf (stderr, "%s: File %s is empty!\n", GMT_program, file);
		GMT_io.multi_segments[GMT_IN] = save;
		if (!use_GMT_io) {GMT_input = psave; GMT_io.netcdf[GMT_IN] = cdf;}	/* Restore previous setting */
		return (GMT_IO_EOF);
	}
	/* Allocate the Table structure */

	T = (struct GMT_TABLE *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_TABLE), GMT_program);
	T->file = strdup (file);
	T->segment = (struct GMT_LINE_SEGMENT **) GMT_memory (VNULL, (size_t)n_seg_alloc, sizeof (struct GMT_LINE_SEGMENT *), GMT_program);

	no_segments = (!(GMT_io.status & GMT_IO_SEGMENT_HEADER));	/* Not a multi-segment file.  We then assume file has only one segment */

	while (n_fields >= 0 && !(GMT_io.status & GMT_IO_EOF)) {	/* Not yet EOF */
		while (no_segments || (GMT_io.status & GMT_IO_SEGMENT_HEADER && !(GMT_io.status & GMT_IO_EOF))) {
			/* To use different line-distances for each segment, place the distance in the segment header */
			if (seg == -1 || T->segment[seg]->n_rows > 0) {
				seg++;	/* Only advance segment if last had any points or was the first one */
				T->segment[seg] = (struct GMT_LINE_SEGMENT *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_LINE_SEGMENT), GMT_program);
			}
			n_read++;
			if (!no_segments) {	/* No heder to process */
				if (ascii) {	/* Only ascii files can have info stored in multi-seg header record */
					k = sscanf (&GMT_io.segment_header[1], "%lg", &d);		/* See if we find a number in the header */
					T->segment[seg]->dist = (k == 1 && dist == 0.0) ? d : dist;	/* If so, assign it to dist, else go with default */
				}
				else
					T->segment[seg]->dist = dist;					/* For binary files dist must be passed via arguments */
			}
			/* Segment initialization */
			n_row_alloc = GMT_CHUNK;
			row = 0;
			if (!no_segments) n_fields = GMT_input (fp, &n_expected_fields, &in);	/* Don't read if we didnt read a segment header up front */
			T->segment[seg]->n_columns = n_expected_fields;
			no_segments = FALSE;	/* This has now served its purpose */
		}
		if ((GMT_io.status & GMT_IO_EOF)) continue;	/* At EOF; get out of this loop */
		if (ascii) {	/* Only ascii files can have info stored in multi-seg header record */
			char buffer[BUFSIZ];
			memset ((void *)buffer, 0, BUFSIZ);
			if (GMT_parse_segment_item (GMT_io.segment_header, "-L", buffer)) T->segment[seg]->label = strdup (buffer);
			if (strlen (GMT_io.segment_header)) T->segment[seg]->header = strdup (GMT_io.segment_header);
		}

		if (poly && T->segment[seg]->n_columns < 2) {
			fprintf (stderr, "%s: File %s does not have at least 2 columns required for polygons (found %ld)\n", GMT_program, file, T->segment[seg]->n_columns);
			GMT_exit (EXIT_FAILURE);
		}

		GMT_alloc_segment (T->segment[seg], n_row_alloc, T->segment[seg]->n_columns, TRUE);

		while (! (GMT_io.status & (GMT_IO_SEGMENT_HEADER | GMT_IO_EOF))) {	/* Keep going until FALSE or find a new segment header */
			n_read++;

			if (GMT_io.status & GMT_IO_MISMATCH) {
				fprintf (stderr, "%s: Mismatch between actual (%ld) and expected (%ld) fields near line %ld (skipped)\n", GMT_program, n_fields, n_expected_fields, n_read);
				continue;
			}

			if (n_expected_fields < 2) {
				fprintf (stderr, "%s: Failure to read file %s near line %ld\n", GMT_program, file, n_read);
				GMT_exit (EXIT_FAILURE);
			}
			if (GMT_io.in_col_type[GMT_X] & GMT_IS_GEO) {
				if (greenwich && T->segment[seg]->coord[GMT_X][row] > 180.0) T->segment[seg]->coord[GMT_X][row] -= 360.0;
				if (!greenwich && T->segment[seg]->coord[GMT_X][row] < 0.0)  T->segment[seg]->coord[GMT_X][row] += 360.0;
			}
			for (k = 0; k < T->segment[seg]->n_columns; k++) {
				T->segment[seg]->coord[k][row] = in[k];
				if (T->segment[seg]->coord[k][row] < T->segment[seg]->min[k]) T->segment[seg]->min[k] = T->segment[seg]->coord[k][row];
				if (T->segment[seg]->coord[k][row] > T->segment[seg]->max[k]) T->segment[seg]->max[k] = T->segment[seg]->coord[k][row];
			}

			row++;
			if (row == (n_row_alloc-1)) {	/* -1 because we may have to close the polygon and hence need 1 more cell */
				n_row_alloc <<= 1;
				GMT_alloc_segment (T->segment[seg], n_row_alloc, T->segment[seg]->n_columns, FALSE);
			}
			n_fields = GMT_input (fp, &n_expected_fields, &in);
		}
		T->segment[seg]->n_rows = row;	/* Number of records in this segment */
		T->n_records += row;		/* Total number of records so far */

		/* If file is a polygon and we must close it if needed */

		if (poly) {
			if (GMT_io.in_col_type[GMT_X] & GMT_IS_GEO) {	/* Must check for polar cap */
				double dlon, lon_sum = 0.0, lat_sum = 0.0;
				dlon = T->segment[seg]->coord[GMT_X][0] - T->segment[seg]->coord[GMT_X][row-1];
				if (!((fabs (dlon) == 0.0 || fabs (dlon) == 360.0) && T->segment[seg]->coord[GMT_Y][0] == T->segment[seg]->coord[GMT_Y][row-1])) {
					T->segment[seg]->coord[GMT_X][row] = T->segment[seg]->coord[GMT_X][0];
					T->segment[seg]->coord[GMT_Y][row] = T->segment[seg]->coord[GMT_Y][0];
					T->segment[seg]->n_rows++;
				}
				for (row = 0; row < T->segment[seg]->n_rows - 1; row++) {
					dlon = T->segment[seg]->coord[GMT_X][row+1] - T->segment[seg]->coord[GMT_X][row];
					if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);	/* Crossed Greenwich or Dateline, pick the shortest distance */
					lon_sum += dlon;
					lat_sum += T->segment[seg]->coord[GMT_Y][row];
				}
				if (GMT_360_RANGE (lon_sum, 0.0)) {	/* TRUE if contains a pole */
					T->segment[seg]->pole = irint (copysign (1.0, lat_sum));	/* So, 0 means not polar */
				}
			}
			else if (GMT_polygon_is_open (T->segment[seg]->coord[GMT_X], T->segment[seg]->coord[GMT_Y], row)) {	/* Cartesian closure */
				T->segment[seg]->coord[GMT_X][row] = T->segment[seg]->coord[GMT_X][0];
				T->segment[seg]->coord[GMT_Y][row] = T->segment[seg]->coord[GMT_Y][0];
				T->segment[seg]->n_rows++;
			}
		}

		/* Reallocate to free up some memory */

		GMT_alloc_segment (T->segment[seg], T->segment[seg]->n_rows, T->segment[seg]->n_columns, FALSE);

		if (T->segment[seg]->n_rows == 0) {	/* Empty segment; we delete to avoid problems downstream in applications */
			GMT_free ((void *)T->segment[seg]);
			seg--;	/* Go back to where we were */
		}

		if (seg == (n_seg_alloc-1)) {
			n_seg_alloc <<= 1;
			T->segment = (struct GMT_LINE_SEGMENT **) GMT_memory ((void *)T->segment, (size_t)n_seg_alloc, sizeof (struct GMT_LINE_SEGMENT *), GMT_program);
		}
	}
	if (close_file) GMT_fclose (fp);
	GMT_io.multi_segments[GMT_IN] = save;
	if (!use_GMT_io) {GMT_input = psave; GMT_io.netcdf[GMT_IN] = cdf;}	/* Restore previous setting */

	if (T->segment[seg]->n_rows == 0)	/* Last segment was empty; we delete to avoid problems downstream in applications */
		GMT_free ((void *)T->segment[seg]);
	else
		seg++;
	T->segment = (struct GMT_LINE_SEGMENT **) GMT_memory ((void *)T->segment, (size_t)seg, sizeof (struct GMT_LINE_SEGMENT *), GMT_program);
	T->n_segments = seg;
	T->n_columns = T->segment[0]->n_columns;
	/* Determine table min,max values */
	T->min = (double *) GMT_memory (VNULL, T->n_columns, sizeof (double), GMT_program);
	T->max = (double *) GMT_memory (VNULL, T->n_columns, sizeof (double), GMT_program);
	for (col = 0; col < T->n_columns; col++) {T->min[col] = DBL_MAX; T->max[col] = -DBL_MAX;}
	for (seg = 0; seg < T->n_segments; seg++) {
		for (col = 0; col < T->n_columns; col++) {
			T->min[col] = MIN (T->min[col], T->segment[seg]->min[col]);
			T->max[col] = MAX (T->max[col], T->segment[seg]->max[col]);
		}
		if (T->segment[seg]->pole) {T->min[GMT_X] = 0.0; T->max[GMT_X] = 360.0;}
	}

	*table = T;

	return (0);
}

GMT_LONG GMT_parse_segment_item (char *in_string, char *pattern, char *out_string)
{
	/* Scans the in_string for the occurence of an option switch (e.g, -L) and
	 * if found, extracts the argument and returns it via out_string.  Function
	 * return TRUE if the pattern was found and FALSE otherwise.
	 * out_string must be allocated and have space for the copying */
	char *t;
	GMT_LONG i, k;
	if (!in_string || !pattern) return (FALSE);	/* No string or pattern passed */
	if (!(t = strstr (in_string, pattern))) return (FALSE);	/* Option not present */
	if ((i = (GMT_LONG)t - (GMT_LONG)in_string - 1) < 0) return (FALSE);	/* No leading space/tab possible */
	if (!(in_string[i] == ' ' || in_string[i] == '\t')) return (FALSE);	/* No leading space/tab present */
	i += (GMT_LONG)(strlen (pattern) + 1);	/* Position of argument */
	if (in_string[i] == '\"') {	/* Quoted argument, must find terminal quote */
		i++;	/* Skip passed first quote */
		for (k = i; k < (GMT_LONG)strlen (in_string) && in_string[k] != '\"'; k++);	/* Find next quote */
		strncpy (out_string, &in_string[i], (size_t)(k - i));
		out_string[k-i] = '\0';	/* Terminate string */
	}
	else	/* No quote, just one word */
		sscanf (&in_string[i], "%s", out_string);
	return (TRUE);
}

GMT_LONG GMT_export_table (void *dest, GMT_LONG dest_type, struct GMT_TABLE *table, GMT_LONG use_GMT_io)
{
	/* Writes an entire multisegment data set to file or wherever */

	char open_mode[4], file[BUFSIZ];
	GMT_LONG ascii, close_file = FALSE;
	GMT_LONG row = 0, seg, col;
	double *out;
	FILE *fp;
	PFL psave = NULL;

	if (use_GMT_io) {	/* Use GMT_io settings to determine if input is ascii/binary, else it defaults to ascii */
		strcpy (open_mode, GMT_io.w_mode);
		ascii = !GMT_io.binary[GMT_OUT];
	}
	else {			/* Force ASCII mode */
		strcpy (open_mode, "w");
		ascii = TRUE;
		psave = GMT_output;		/* Save the previous pointer since we need to change it back at the end */
		GMT_output = GMT_output_ascii;	/* Override and use ascii mode */
	}

	if (dest_type == GMT_IS_FILE) {	/* dest is a file name */
		strcpy (file, (char *)dest);
		if ((fp = GMT_fopen (file, open_mode)) == NULL) {
			fprintf (stderr, "%s: Cannot open file %s\n", GMT_program, file);
			GMT_exit (EXIT_FAILURE);
		}
		close_file = TRUE;	/* We only close files we have opened here */
	}
	else if (dest_type == GMT_IS_STREAM) {	/* Open file pointer given, just copy */
		fp = (FILE *)dest;
		if (fp == GMT_stdout)
			strcpy (file, "<stdout>");
		else
			strcpy (file, "<output stream>");
	}
	else if (dest_type == GMT_IS_FDESC) {		/* Open file descriptor given, just convert to file pointer */
		int *fd;
		fd = (int *)dest;
		if ((fp = fdopen (*fd, open_mode)) == NULL) {
			fprintf (stderr, "%s: Cannot convert file descriptor %d to stream in GMT_export_table\n", GMT_program, *fd);
			GMT_exit (EXIT_FAILURE);
		}
		if (fp == GMT_stdout)
			strcpy (file, "<stdout>");
		else
			strcpy (file, "<output file descriptor>");
	}
	else {
		fprintf (stderr, "%s: Unrecognized source type %ld in GMT_export_table\n", GMT_program, dest_type);
		GMT_exit (EXIT_FAILURE);
	}

	out = (double *) GMT_memory (VNULL, (size_t)table->n_columns, sizeof (double), "GMT_export_table");

	for (seg = 0; seg < table->n_segments; seg++) {
		if (GMT_io.multi_segments[GMT_OUT]) {	/* Want to write segment headers */
			if (table->segment[seg]->header) strcpy (GMT_io.segment_header, table->segment[seg]->header);
			GMT_write_segmentheader (fp, table->segment[seg]->n_columns);
		}
		for (row = 0; row < table->segment[seg]->n_rows; row++) {
			for (col = 0; col < table->segment[seg]->n_columns; col++) out[col] = table->segment[seg]->coord[col][row];
			GMT_output (fp, table->segment[seg]->n_columns, out);
		}
	}

	if (close_file) GMT_fclose (fp);	/* Close the file since we opened it */
	GMT_free ((void *)out);			/* Free up allocated memory */
	if (!use_GMT_io) GMT_output = psave;	/* Restore former pointers and values */

	return (0);	/* OK status */
}

void GMT_alloc_segment (struct GMT_LINE_SEGMENT *S, GMT_LONG n_rows, GMT_LONG n_columns, GMT_LONG first)
{	/* (re)allocates memory for a segment of given dimensions */
	GMT_LONG col;
	if (first) {	/* First time we allocate the number of columns needed */
		S->coord = (double **) GMT_memory (VNULL, (size_t)n_columns, sizeof (double *), GMT_program);
		S->min = (double *) GMT_memory (VNULL, (size_t)n_columns, sizeof (double), GMT_program);
		S->max = (double *) GMT_memory (VNULL, (size_t)n_columns, sizeof (double), GMT_program);
		for (col = 0; col < n_columns; col++) {	/* Initialize the min/max array */
			S->min[col] = +DBL_MAX;
			S->max[col] = -DBL_MAX;
		}
	}
	for (col = 0; col < n_columns; col++) S->coord[col] = (double *) GMT_memory ((void *)S->coord[col], (size_t)n_rows, sizeof (double), GMT_program);
}

GMT_LONG GMT_n_segment_points (struct GMT_LINE_SEGMENT *S, GMT_LONG n_segments)
{	/* Returns the total number of data records for all segments */
	GMT_LONG seg, n_records = 0;
	for (seg = 0; seg < n_segments; seg++) n_records += S[seg].n_rows;
	return (n_records);
}

void GMT_free_dataset (struct GMT_DATASET *data)
{
	GMT_LONG tbl;
	for (tbl = 0; tbl < data->n_tables; tbl++) {
		GMT_free_table (data->table[tbl]);
	}
	GMT_free ((void *)data);
}

void GMT_free_table (struct GMT_TABLE *table)
{
	GMT_LONG seg;
	if (!table) return;	/* Do not try to free NULL pointer */
	for (seg = 0; seg < table->n_segments; seg++) GMT_free_segment (table->segment[seg]);
	GMT_free ((void *)table->segment);
	GMT_free ((void *)table->min);
	GMT_free ((void *)table->max);
	free ((void *)table->file);
	GMT_free ((void *)table);
}

void GMT_free_segment (struct GMT_LINE_SEGMENT *segment)
{
	/* Free memory allocated by GMT_import_table */

	GMT_LONG col;
	if (!segment) return;	/* Do not try to free NULL pointer */
	for (col = 0; col < segment->n_columns; col++) GMT_free ((void *) segment->coord[col]);
	GMT_free ((void *) segment->coord);
	if (segment->min) GMT_free ((void *) segment->min);
	if (segment->max) GMT_free ((void *) segment->max);
	if (segment->label) free ((void *) segment->label);
	if (segment->header) free ((void *) segment->header);
	GMT_free ((void *)segment);
}

GMT_LONG GMT_not_numeric (char *text)
{
	/* TRUE if text does not represent a valid number  However,
	 * FALSE does not therefore mean we have a valid number because
	 * <date>T<clock> representations may use all kinds
	 * of punctuations or letters according to the various format
	 * settings in .gmtdefaults4.  Here we just rule out things
	 * that we are sure of. */

	GMT_LONG i, k, n_digits = 0, n_period = 0, period = 0, n_plus = 0, n_minus = 0;

	for (i = 0; text[i]; i++) {	/* Check each character */
		/* First check for ASCII values that never appear in any number */
		if (text[i] < 43) return (TRUE);	/* ASCII 0-42 */
		if (text[i] == '\'' || text[i] == '/') return (TRUE);
		if (text[i] > ':' && text[i] < 'D') return (TRUE);
		if (text[i] > 'E' && text[i] < 'N') return (TRUE);
		if (text[i] > 'N' && text[i] < 'S') return (TRUE);
		if (text[i] > 'S' && text[i] < 'W') return (TRUE);
		if (text[i] > 'W' && text[i] < 'd') return (TRUE);
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
	return (FALSE);	/* THis may in fact be numeric */
}
