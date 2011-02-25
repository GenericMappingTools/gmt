/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.c,v 1.138 2011-02-25 15:33:52 jluis Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be u237seful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *	G M T _ G R D I O . C   R O U T I N E S
 *
 * Generic routines that take care of all gridfile input/output.
 * These are the only PUBLIC grd io functions to be used by developers
 *
 * Author:	Paul Wessel
 * Date:	9-SEP-1992
 * Modified:	06-DEC-2001
 * Version:	4
 * 64-bit Compliant: Yes
 *
 * Functions include:
 *
 *	GMT_grd_get_format :	Get format id, scale, offset and missing value for grdfile
 *
 *	GMT_read_grd_info :	Read header from file
 *	GMT_read_grd :		Read data set from file (must be preceded by GMT_read_grd_info)
 *	GMT_update_grd_info :	Update header in existing file (must be preceded by GMT_read_grd_info)
 *	GMT_write_grd_info :	Write header to new file
 *	GMT_write_grd :		Write header and data set to new file
 *
 *	For programs that must access on row at the time, you must use:
 *	GMT_open_grd :		Opens the grdfile for reading or writing
 *	GMT_read_grd_row :	Reads a single row of data from grdfile
 *	GMT_write_grd_row :	Writes a single row of data from grdfile
 *	GMT_close_grd :		Close the grdfile
 *
 * Additional supporting grid routines:
 *
 *	GMT_grd_init 		Initialize grd header structure
 *	GMT_grd_shift 		Rotates grdfiles in x-direction
 *	GMT_grd_setregion 	Determines subset coordinates for grdfiles
 *	GMT_grd_is_global	Determine whether grid is "global", i.e. longitudes are periodic
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define GMT_WITH_NO_PS
#include "gmt.h"
#include <sys/types.h>
#include <sys/stat.h>
#if defined(WIN32) || defined(__EMX__)  /* Some definitions and includes are different under Windows or OS/2 */
#define STAT _stat
#else                                   /* Here for Unix, Linux, Cygwin, Interix, etc */
#define STAT stat
#endif

GMT_LONG GMT_grdformats[GMT_N_GRD_FORMATS][2] = {
#include "gmt_grdformats.h"
};

void GMT_grd_do_scaling (float *grid, GMT_LONG nm, double scale, double offset);
void GMT_grd_get_units (struct GRD_HEADER *header);
void GMT_grd_set_units (struct GRD_HEADER *header);
GMT_LONG GMT_is_nc_grid (struct GRD_HEADER *header);
GMT_LONG GMT_is_native_grid (struct GRD_HEADER *header);
GMT_LONG GMT_is_ras_grid (struct GRD_HEADER *header);
GMT_LONG GMT_is_srf_grid (struct GRD_HEADER *header);
GMT_LONG GMT_is_mgg2_grid (struct GRD_HEADER *header);
GMT_LONG GMT_is_agc_grid (struct GRD_HEADER *header);

/* GENERIC I/O FUNCTIONS FOR GRIDDED DATA FILES */

GMT_LONG GMT_read_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 */

	GMT_LONG err;
	double scale, offset, nan_value;

	/* Initialize grid information */
	GMT_grd_init (header, 0, (char **)VNULL, FALSE);

	/* Save parameters on file name suffix before issuing GMT_io_readinfo */
 	GMT_err_trap (GMT_grd_get_format (file, header, TRUE));
	scale = header->z_scale_factor, offset = header->z_add_offset, nan_value = header->nan_value;

	GMT_err_trap ((*GMT_io_readinfo[header->type]) (header));
	GMT_grd_get_units (header);
	if (!GMT_is_dnan(scale)) header->z_scale_factor = scale, header->z_add_offset = offset;
	if (!GMT_is_dnan(nan_value)) header->nan_value = nan_value;
	if (header->z_scale_factor == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	GMT_err_pass (GMT_grd_RI_verify (header, 0), file);

	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
	header->xy_off = 0.5 * header->node_offset;

	return (GMT_NOERROR);
}

GMT_LONG GMT_write_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 */

	GMT_LONG err;

 	GMT_err_trap (GMT_grd_get_format (file, header, FALSE));

	if (GMT_is_dnan(header->z_scale_factor))
		header->z_scale_factor = 1.0;
	else if (header->z_scale_factor == 0.0) {
		header->z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	GMT_grd_set_units (header);
	return ((*GMT_io_writeinfo[header->type]) (header));
}

GMT_LONG GMT_update_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 */

	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	GMT_grd_set_units (header);
	return ((*GMT_io_updateinfo[header->type]) (header));
}

GMT_LONG GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	GMT_LONG err;
	GMT_LONG nm;

	GMT_err_trap ((*GMT_io_readgrd[header->type]) (header, grid, w, e, s, n, pad, complex));
	if (header->z_scale_factor == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	nm = ((GMT_LONG)(header->nx + pad[0] + pad[1])) * ((GMT_LONG)(header->ny + pad[2] + pad[3]));
	GMT_grd_do_scaling (grid, nm, header->z_scale_factor, header->z_add_offset);
	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
	header->xy_off = 0.5 * header->node_offset;
	return (GMT_NOERROR);
}

GMT_LONG GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex)
{	/* file:	File name
	 * header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	GMT_LONG err;
	GMT_LONG nm;

	GMT_err_trap (GMT_grd_get_format (file, header, FALSE));
	if (GMT_is_dnan(header->z_scale_factor))
		header->z_scale_factor = 1.0;
	else if (header->z_scale_factor == 0.0) {
		header->z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	GMT_grd_set_units (header);
	nm = ((GMT_LONG)header->nx) * ((GMT_LONG)header->ny);
	GMT_grd_do_scaling (grid, nm, 1.0/header->z_scale_factor, -header->z_add_offset/header->z_scale_factor);
	return ((*GMT_io_writegrd[header->type]) (header, grid, w, e, s, n, pad, complex));
}

/* Routines to see if a particular grd file format is specified as part of filename. */

void GMT_expand_filename (char *file, char *fname)
{
	GMT_LONG i, length, f_length, found, start;

	if (gmtdefs.gridfile_shorthand) {	/* Look for matches */
		f_length = (GMT_LONG) strlen (file);
		for (i = found = 0; !found && i < GMT_n_file_suffix; i++) {
			length = (GMT_LONG) strlen (GMT_file_suffix[i]);
			start = f_length - length;
			found = (start < 0) ? FALSE : !strncmp (&file[start], GMT_file_suffix[i], (size_t)length);
		}
		if (found) {	/* file ended in a recognized shorthand extension */
			i--;
			sprintf (fname, "%s=%ld/%g/%g/%g", file, GMT_file_id[i], GMT_file_scale[i], GMT_file_offset[i], GMT_file_nan[i]);
		}
		else
			strcpy (fname, file);
	}
	else	/* Simply copy the full name */
		strcpy (fname, file);
}

GMT_LONG GMT_grd_get_format (char *file, struct GRD_HEADER *header, GMT_LONG magic)
{
	/* This functions does a couple of things:
	 * 1. It tries to determine what kind of grid file this is. If a file is openeed for
	 *    reading we see if (a) a particular format has been specified with
	 *    the =<code> suffix, or (b) we are able to guess the format based on known
	 *    characteristics of various formats, or (c) assume the default grid format.
	 *    If a file is opened for writing, only option (a) and (c) apply.
	 *    If we cannot obtain the format we return an error.
	 * 2. We strip the suffix off. The relevant info is stored in the header struct.
	 * 3. In case of netCDF grids, the optional ?<varname> is stripped off as well.
	 *    The info is stored in header->varname.
	 * 4. If a file is open for reading, we set header->name to the full path of the file
	 *    by seaching in current dir and the various GMT_*DIR paths.
	 */
	
	GMT_LONG i = 0, val, j;
	char code[GMT_TEXT_LEN], tmp[BUFSIZ];

	GMT_expand_filename (file, header->name);	/* May append a suffix to header->name */

	/* Set default values */
	header->z_scale_factor = GMT_d_NaN, header->z_add_offset = 0.0, header->nan_value = GMT_d_NaN;

	while (header->name[i] && header->name[i] != '=') i++;

	if (header->name[i]) {	/* Reading or writing when =suffix is present: get format type, scale, offset and missing value */
		i++;
		sscanf (&header->name[i], "%[^/]/%lf/%lf/%lf", code, &header->z_scale_factor, &header->z_add_offset, &header->nan_value);
		val = GMT_grd_format_decoder (code);
		if (val < 0) return (val);
		header->type = val;
		j = (i == 1) ? i : i - 1;
		header->name[j] = 0;
		sscanf (header->name, "%[^?]?%s", tmp, header->varname);    /* Strip off variable name */
		if (magic) {	/* Reading: possibly prepend a path from GMT_[GRID|DATA|IMG]DIR */
			if (!GMT_getdatapath (tmp, header->name)) return (GMT_GRDIO_FILE_NOT_FOUND);	/* Reading: possibly prepended a path from GMT_[GRID|DATA|IMG]DIR */
		}
		else		/* Writing: store truncated pathname */
			strcpy (header->name, tmp);
	}
	else if (magic) {	/* Reading: determine file format automatically based on grid content */
		sscanf (header->name, "%[^?]?%s", tmp, header->varname);    /* Strip off variable name */
		if (!GMT_getdatapath (tmp, header->name)) return (GMT_GRDIO_FILE_NOT_FOUND);	/* Possibly prepended a path from GMT_[GRID|DATA|IMG]DIR */
		/* First check if we have a netCDF grid. This MUST be first, because ?var needs to be stripped off. */
		if ((val = GMT_is_nc_grid (header)) >= 0) return (GMT_NOERROR);
		/* Continue only when file was a pipe or when nc_open didn't like the file. */
		if (val != GMT_GRDIO_NC_NO_PIPE && val != GMT_GRDIO_OPEN_FAILED) return (val);
		/* Then check for native GMT grid */
		if (GMT_is_native_grid (header) >= 0) return (GMT_NOERROR);
		/* Next check for Sun raster GMT grid */
		if (GMT_is_ras_grid (header) >= 0) return (GMT_NOERROR);
		/* Then check for Golden Software surfer grid */
		if (GMT_is_srf_grid (header) >= 0) return (GMT_NOERROR);
		/* Then check for NGDC GRD98 GMT grid */
		if (GMT_is_mgg2_grid (header) >= 0) return (GMT_NOERROR);
		/* Then check for AGC GMT grid */
		if (GMT_is_agc_grid (header) >= 0) return (GMT_NOERROR);
		return (GMT_GRDIO_UNKNOWN_FORMAT);	/* No supported format found */
	}
	else {			/* Writing: get format type, scale, offset and missing value from gmtdefs.gridfile_format */
		if (sscanf (header->name, "%[^?]?%s", tmp, header->varname) > 1) strcpy (header->name, tmp);    /* Strip off variable name */
		sscanf (gmtdefs.gridfile_format, "%[^/]/%lf/%lf/%lf", code, &header->z_scale_factor, &header->z_add_offset, &header->nan_value);
		val = GMT_grd_format_decoder (code);
		if (val < 0) return (val);
		header->type = val;
	}
	if (header->type == GMT_grd_format_decoder ("af")) header->nan_value = 0.0;	/* 0 is NaN in the AGC format */
	return (GMT_NOERROR);
}

GMT_LONG GMT_grd_data_size (GMT_LONG format, double *nan_value)
{
	/* Determine size of data type and set NaN value, if not yet done so (integers only) */

	switch (GMT_grdformats[format][1]) {
		case 'b':
			if (GMT_is_dnan (*nan_value)) *nan_value = CHAR_MIN;
			return (sizeof(char));
			break;
		case 's':
			if (GMT_is_dnan (*nan_value)) *nan_value = SHRT_MIN;
			return (sizeof(short int));
			break;
		case 'i':
			if (GMT_is_dnan (*nan_value)) *nan_value = INT_MIN;
		case 'm':
			return (sizeof(int));
			break;
		case 'f':
			return (sizeof(float));
			break;
		case 'd':
			return (sizeof(double));
			break;
		default:
			fprintf (stderr, "Unknown grid data type: %c\n", (int)GMT_grdformats[format][1]);
			return (GMT_GRDIO_UNKNOWN_TYPE);
	}
}

GMT_LONG GMT_grd_format_decoder (const char *code)
{
	/* Returns the integer grid format ID that goes with the specified 2-character code */

	GMT_LONG id;

	if (isdigit ((int)code[0])) {	/* File format number given, convert directly */
		id = atoi (code);
 		if (id < 0 || id >= GMT_N_GRD_FORMATS) return (GMT_GRDIO_UNKNOWN_ID);
	}
	else {	/* Character code given */
		GMT_LONG i, group;
		for (i = group = 0, id = -1; id < 0 && i < GMT_N_GRD_FORMATS; i++) {
			if (GMT_grdformats[i][0] == (short)code[0]) {
				group = code[0];
				if (GMT_grdformats[i][1] == (short)code[1]) id = i;
			}
		}

		if (id == -1) return (GMT_GRDIO_UNKNOWN_ID);
	}

	return (id);
}

void GMT_grd_do_scaling (float *grid, GMT_LONG nm, double scale, double offset)
{
	/* Routine that scales and offsets the data if specified */
	GMT_LONG i;

	if (scale == 1.0 && offset == 0.0) return;

	if (scale == 1.0)
		for (i = 0; i < nm; i++) grid[i] += (float)offset;
	else if (offset == 0.0)
		for (i = 0; i < nm; i++) grid[i] *= (float)scale;
	else
		for (i = 0; i < nm; i++) grid[i] = grid[i] * ((float)scale) + (float)offset;
}

/* gmt_grd_RI_verify -- routine to check grd R and I compatibility
 *
 * Author:	W H F Smith
 * Date:	20 April 1998
 */

GMT_LONG GMT_grd_RI_verify (struct GRD_HEADER *h, GMT_LONG mode)
{
	/* mode - 0 means we are checking an existing grid, mode = 1 means we test a new -R -I combination */

	GMT_LONG error = 0;

	if (!strcmp (GMT_program, "grdedit")) return (GMT_NOERROR);	/* Separate handling in grdedit to allow grdedit -A */

	switch (GMT_minmaxinc_verify (h->x_min, h->x_max, h->x_inc, GMT_SMALL)) {
		case 3:
			(void) fprintf (stderr, "%s: GMT ERROR: grid x increment <= 0.0\n", GMT_program);
			error++;
			break;
		case 2:
			(void) fprintf (stderr, "%s: GMT ERROR: grid x range <= 0.0\n", GMT_program);
			error++;
			break;
		case 1:
			(void) fprintf (stderr, "%s: GMT ERROR: (x_max-x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
			error++;
		default:
			/* Everything is OK */
			break;
	}

	switch (GMT_minmaxinc_verify (h->y_min, h->y_max, h->y_inc, GMT_SMALL)) {
		case 3:
			(void) fprintf (stderr, "%s: GMT ERROR: grid y increment <= 0.0\n", GMT_program);
			error++;
			break;
		case 2:
			(void) fprintf (stderr, "%s: GMT ERROR: grid y range <= 0.0\n", GMT_program);
			error++;
			break;
		case 1:
			(void) fprintf (stderr, "%s: GMT ERROR: (y_max-y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
			error++;
		default:
			/* Everything is OK */
			break;
	}
	if (error) return ((mode == 0) ? GMT_GRDIO_RI_OLDBAD : GMT_GRDIO_RI_NEWBAD);
	return (GMT_NOERROR);
}

GMT_LONG GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, GMT_LONG *width, GMT_LONG *height, GMT_LONG *first_col, GMT_LONG *last_col, GMT_LONG *first_row, GMT_LONG *last_row, GMT_LONG **index)
{
	/* Determines which rows and columns to extract to extract from a grid, based on w,e,s,n.
	 * This routine first rounds the w,e,s,n boundaries to the nearest gridlines or pixels,
	 * then determines the first and last columns and rows, and the width and height of the subset (in cells).
	 * The routine also returns and array of the x-indices in the source grid to be used in the target (subset) grid.
	 */

	GMT_LONG one_or_zero, i, *k;
	GMT_LONG geo = FALSE;
	double small = 0.1, half_or_zero, x;

	half_or_zero = (header->node_offset) ? 0.5 : 0.0;

	if (*w == 0.0 && *e == 0.0) {	/* Get entire file */
		*width  = header->nx;
		*height = header->ny;
		*first_col = *first_row = 0;
		*last_col  = header->nx - 1;
		*last_row  = header->ny - 1;
		*w = header->x_min;	*e = header->x_max;
		*s = header->y_min;	*n = header->y_max;
	}
	else {				/* Must deal with a subregion */
		if (GMT_io.in_col_type[0] == GMT_IS_LON)
			geo = TRUE;	/* Geographic data for sure */
		else if (*w < header->x_min || *e > header->x_max)
			geo = TRUE;	/* Probably dealing with periodic grid */

		if (*s < header->y_min || *n > header->y_max) return (GMT_GRDIO_DOMAIN_VIOLATION);	/* Calling program goofed... */

		one_or_zero = (header->node_offset) ? 0 : 1;

		/* Make sure w,e,s,n are proper multiples of x_inc,y_inc away from x_min,y_min */

		GMT_err_pass (GMT_adjust_loose_wesn (w, e, s, n, header), header->name);

		/* Get dimension of subregion */

		*width  = irint ((*e - *w) / header->x_inc) + one_or_zero;
		*height = irint ((*n - *s) / header->y_inc) + one_or_zero;

		/* Get first and last row and column numbers */

		*first_col = (GMT_LONG)floor ((*w - header->x_min) / header->x_inc + small);
		*last_col  = (GMT_LONG)ceil  ((*e - header->x_min) / header->x_inc - small) - 1 + one_or_zero;
		*first_row = (GMT_LONG)floor ((header->y_max - *n) / header->y_inc + small);
		*last_row  = (GMT_LONG)ceil  ((header->y_max - *s) / header->y_inc - small) - 1 + one_or_zero;
#if 0
		if ((*last_col - *first_col + 1) > *width) (*last_col)--;
		if ((*last_row - *first_row + 1) > *height) (*last_row)--;
		if ((*last_col - *first_col + 1) > *width) (*first_col)++;
		if ((*last_row - *first_row + 1) > *height) (*first_row)++;
#endif
	}

	k = (GMT_LONG *) GMT_memory (VNULL, *width, sizeof (GMT_LONG), "GMT_grd_prep_io");
	if (geo) {
		small = 0.1 * header->x_inc;
		for (i = 0; i < (*width); i++) {
			x = GMT_i_to_x (i, *w, *e, header->x_inc, half_or_zero, *width);
			if (header->x_min - x > small)
				x += 360.0;
			else if (x - header->x_max > small)
				x -= 360.0;
			k[i] = GMT_x_to_i (x, header->x_min, header->x_inc, half_or_zero, header->nx);
		}
	}
	else {	/* Normal ordering */
		for (i = 0; i < (*width); i++) k[i] = (*first_col) + i;
	}

	*index = k;
	
	return (GMT_NOERROR);
}

void GMT_decode_grd_h_info (char *input, struct GRD_HEADER *h) {

/*	Given input string, copy elements into string portions of h.
	By default use "/" as the field separator. However, if the first and
	last character of the input string is the same AND that character
	is non-alpha-numeric, use the first character as a separator. This
	is to allow "/" as part of the fields.
	If a field has an equals sign, skip it.
	This routine is usually called if -D<input> was given by user,
	and after GMT_grd_init() has been called.
*/
	char ptr[BUFSIZ], sep[] = "/";
	GMT_LONG entry = 0, pos = 0;

	if (input[0] != input[strlen(input)-1]) {}
	else if (input[0] == '=') {}
	else if (input[0] >= 'A' && input[0] <= 'Z') {}
	else if (input[0] >= 'a' && input[0] <= 'z') {}
	else if (input[0] >= '0' && input[0] <= '9') {}
	else {
		sep[0] = input[0];
		pos = 1;
	}

	while ((GMT_strtok (input, sep, &pos, ptr))) {
		if (ptr[0] != '=') {
			switch (entry) {
				case 0:
					memset ( (void *)h->x_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: X unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->x_units, ptr, (size_t)GRD_UNIT_LEN);
					break;
				case 1:
					memset ( (void *)h->y_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: Y unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->y_units, ptr, (size_t)GRD_UNIT_LEN);
					break;
				case 2:
					memset ( (void *)h->z_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: Z unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->z_units, ptr, (size_t)GRD_UNIT_LEN);
					break;
				case 3:
					h->z_scale_factor = atof (ptr);
					break;
				case 4:
					h->z_add_offset = atof (ptr);
					break;
				case 5:
					if (strlen(ptr) >= GRD_TITLE_LEN) fprintf (stderr, "%s: GMT WARNING: Title string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_TITLE_LEN);
					strncpy (h->title, ptr, (size_t)GRD_TITLE_LEN);
					break;
				case 6:
					if (strlen(ptr) >= GRD_REMARK_LEN) fprintf (stderr, "%s: GMT WARNING: Remark string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_REMARK_LEN);
					strncpy (h->remark, ptr, (size_t)GRD_REMARK_LEN);
					break;
				default:
					break;
			}
		}
		entry++;
	}
	return;
}

GMT_LONG GMT_open_grd (char *file, struct GMT_GRDFILE *G, char mode)
{
	/* Assumes header contents is already known.  For writing we
	 * assume that the header has already been written.  We fill
	 * the GRD_FILE structure with all the required information.
	 * mode can be w or r.  Upper case W or R refers to headerless
	 * grdraster-type files.
	 */

	GMT_LONG r_w, err;
	int cdf_mode[3] = { NC_NOWRITE, NC_WRITE, NC_WRITE};
	char *bin_mode[3] = { "rb", "rb+", "wb"};
	GMT_LONG header = TRUE, magic = TRUE;
	EXTERN_MSC GMT_LONG GMT_nc_grd_info (struct GRD_HEADER *header, char job);

	if (mode == 'r' || mode == 'R') {	/* Open file for reading */
		if (mode == 'R') header = FALSE;
		r_w = 0;
	}
	else if (mode == 'W') {
		r_w = 2;
		header = magic = FALSE;
	}
	else
		r_w = 1;
 	GMT_err_trap (GMT_grd_get_format (file, &G->header, magic));
	if (GMT_is_dnan(G->header.z_scale_factor))
		G->header.z_scale_factor = 1.0;
	else if (G->header.z_scale_factor == 0.0) {
		G->header.z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	if (GMT_grdformats[G->header.type][0] == 'c') {		/* Open netCDF file, old format */
		GMT_err_trap (nc_open (G->header.name, cdf_mode[r_w], &G->fid));
		if (header) GMT_nc_grd_info (&G->header, mode);
		G->edge[0] = G->header.nx;
		G->start[0] = G->start[1] = G->edge[1] = 0;
	}
	else if (GMT_grdformats[G->header.type][0] == 'n') {		/* Open netCDF file, COARDS-compliant format */
		GMT_err_trap (nc_open (G->header.name, cdf_mode[r_w], &G->fid));
		if (header) GMT_nc_grd_info (&G->header, mode);
		G->edge[0] = 1;
		G->edge[1] = G->header.nx;
		G->start[0] = G->header.ny-1;
		G->start[1] = 0;
	}
	else {				/* Regular binary file with/w.o standard GMT header */
		if (r_w == 0 && (G->fp = GMT_fopen (G->header.name, bin_mode[0])) == NULL)
			return (GMT_GRDIO_OPEN_FAILED);
		else if ((G->fp = GMT_fopen (G->header.name, bin_mode[r_w])) == NULL)
			return (GMT_GRDIO_CREATE_FAILED);
		if (header && GMT_fseek (G->fp, (long)GRD_HEADER_SIZE, SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
	}

	G->size = GMT_grd_data_size (G->header.type, &G->header.nan_value);
	G->check = !GMT_is_dnan (G->header.nan_value);
	G->scale = G->header.z_scale_factor, G->offset = G->header.z_add_offset;

	if (GMT_grdformats[G->header.type][1] == 'm')	/* Bit mask */
		G->n_byte = irint (ceil (G->header.nx / 32.0)) * G->size;
	else if (GMT_grdformats[G->header.type][0] == 'r' && GMT_grdformats[G->header.type][1] == 'b')	/* Sun Raster */
		G->n_byte = irint (ceil (G->header.nx / 2.0)) * 2 * G->size;
	else	/* All other */
		G->n_byte = G->header.nx * G->size;

	G->v_row = (void *) GMT_memory (VNULL, G->n_byte, (size_t)1, GMT_program);

	G->row = 0;
	G->auto_advance = TRUE;	/* Default is to read sequential rows */
	return (GMT_NOERROR);
}

void GMT_close_grd (struct GMT_GRDFILE *G)
{
	GMT_free ((void *)G->v_row);
	if (GMT_grdformats[G->header.type][0] == 'c' || GMT_grdformats[G->header.type][0] == 'n')
		nc_close (G->fid);
	else
		GMT_fclose (G->fp);
}

GMT_LONG GMT_read_grd_row (struct GMT_GRDFILE *G, GMT_LONG row_no, float *row)
{	/* Reads the entire row vector form the grdfile
	 * If row_no is negative it is interpreted to mean that we want to
	 * fseek to the start of the abs(row_no) record and no reading takes place.
	 */

	GMT_LONG i, err;

	if (GMT_grdformats[G->header.type][0] == 'c') {		/* Get one NetCDF row, old format */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = GMT_abs (row_no);
			G->start[0] = G->row * G->edge[0];
			return (GMT_NOERROR);
		}
		GMT_err_trap (nc_get_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
		if (G->auto_advance) G->start[0] += G->edge[0];
	}
	else if (GMT_grdformats[G->header.type][0] == 'n') {	/* Get one NetCDF row, COARDS-compliant format */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = GMT_abs (row_no);
			G->start[0] = G->header.ny - 1 - G->row;
			return (GMT_NOERROR);
		}
		GMT_err_trap (nc_get_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
		if (G->auto_advance) G->start[0] --;
	}
	else {			/* Get a binary row */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = GMT_abs (row_no);
			if (GMT_fseek (G->fp, (long)(GRD_HEADER_SIZE + G->row * G->n_byte), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
			return (GMT_NOERROR);
		}
		if (!G->auto_advance && GMT_fseek (G->fp, (long)(GRD_HEADER_SIZE + G->row * G->n_byte), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);

		if (GMT_fread (G->v_row, (size_t)G->size, (size_t)G->header.nx, G->fp) != (size_t)G->header.nx)  return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		for (i = 0; i < G->header.nx; i++) {
			row[i] = GMT_decode (G->v_row, i, GMT_grdformats[G->header.type][1]);	/* Convert whatever to float */
			if (G->check && row[i] == G->header.nan_value) row[i] = GMT_f_NaN;
		}
	}
	GMT_grd_do_scaling (row, (GMT_LONG)G->header.nx, G->scale, G->offset);
	G->row++;
	return (GMT_NOERROR);
}

GMT_LONG GMT_write_grd_row (struct GMT_GRDFILE *G, GMT_LONG row_no, float *row)
{	/* Writes the entire row vector to the grdfile */

	GMT_LONG i, size, err;
	void *tmp;

	size = GMT_grd_data_size (G->header.type, &G->header.nan_value);

	tmp = (void *) GMT_memory (VNULL, (GMT_LONG)G->header.nx, (size_t)size, "GMT_write_grd_row");

	GMT_grd_do_scaling (row, (GMT_LONG)G->header.nx, G->scale, G->offset);
	for (i = 0; i < G->header.nx; i++) if (GMT_is_fnan (row[i]) && G->check) row[i] = (float)G->header.nan_value;

	switch (GMT_grdformats[G->header.type][0]) {
		case 'c':
			GMT_err_trap (nc_put_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
			if (G->auto_advance) G->start[0] += G->edge[0];
			break;
		case 'n':
			GMT_err_trap (nc_put_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
			if (G->auto_advance) G->start[0] --;
			break;
		default:
			for (i = 0; i < G->header.nx; i++) GMT_encode (tmp, i, row[i], GMT_grdformats[G->header.type][1]);
			if (GMT_fwrite (tmp, (size_t)size, (size_t)G->header.nx, G->fp) < (size_t)G->header.nx) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_free (tmp);
	return (GMT_NOERROR);
}

void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, GMT_LONG update)
{	/* GMT_grd_init initializes a grd header to default values and copies the
	 * command line to the header variable command.
	 * update = TRUE if we only want to update command line */
	GMT_LONG i, len;

	if (update)	/* Only clean the command history */
		memset ((void *)header->command, 0, (size_t)GRD_COMMAND_LEN);
	else {		/* Wipe the slate clean */
		memset ((void *)header, 0, (size_t)sizeof(struct GRD_HEADER));

		/* Set the variables that are not initialized to 0/FALSE/NULL */
		header->z_scale_factor		= 1.0;
		header->type			= -1;
		header->y_order			= 1;
		header->z_id			= -1;
		header->z_min			= GMT_d_NaN;
		header->z_max			= GMT_d_NaN;
		header->nan_value		= GMT_d_NaN;
		strcpy (header->x_units, "x");
		strcpy (header->y_units, "y");
		strcpy (header->z_units, "z");
		for (i = 0; i < 3; i++) header->t_index[i] = -1;
	}

	/* Always update command line history */

	if (argc > 0) {
		strcpy (header->command, argv[0]);
		len = strlen (header->command);
		for (i = 1; len < GRD_COMMAND_LEN && i < argc; i++) {
			len += strlen (argv[i]) + 1;
			if (len > GRD_COMMAND_LEN) continue;
			strcat (header->command, " ");
			strcat (header->command, argv[i]);
		}
		header->command[len] = 0;
	}
}

void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift)
{
	/* Rotate geographical, global grid in e-w direction
	 * This function will shift a grid by shift degrees */

	GMT_LONG i, j, k, ij, nc, n_shift, width, n_warn = 0;
	float *tmp;

	n_shift = irint (shift / header->x_inc);
	width = irint (360.0 / header->x_inc);
	if (width > header->nx) {
		fprintf (stderr, "%s: Error: can not rotate grid, too small\n", GMT_program);
		return;
	}

	tmp = (float *) GMT_memory (VNULL, (GMT_LONG)header->nx, sizeof (float), "GMT_grd_shift");

	nc = header->nx * sizeof (float);

	for (j = ij = 0; j < header->ny; j++, ij += header->nx) {
		if (width < header->nx && grd[ij] != grd[ij+width]) n_warn++;
		for (i = 0; i < header->nx; i++) {
			k = (i - n_shift) % width;
			if (k < 0) k += width;
			tmp[k] = grd[ij+i];
		}
		memcpy ((void *)&grd[ij], (void *)tmp, (size_t)nc);
	}

	/* Shift boundaries */

	header->x_min += shift;
	header->x_max += shift;
	if (header->x_max < 0.0) {
		header->x_min += 360.0;
		header->x_max += 360.0;
	}
	else if (header->x_max > 360.0) {
		header->x_min -= 360.0;
		header->x_max -= 360.0;
	}

	if (n_warn) fprintf (stderr, "%s: Gridline-registered global grid has inconsistent values at repeated node for %ld rows\n", GMT_program, n_warn);

	GMT_free ((void *) tmp);
}

GMT_LONG GMT_grd_is_global (struct GRD_HEADER *h)
{	/* Determine if grid could be global */

	if (GMT_io.in_col_type[0] == GMT_IS_LON || project_info.degree[0]) {
		if (fabs (h->x_max - h->x_min - 360.0) < GMT_SMALL) {
			if (gmtdefs.verbose) fprintf (stderr, "GMT_grd_is_global: yes, longitudes span exactly 360\n");
			return (TRUE);
		}
		else if (fabs (h->nx * h->x_inc - 360.0) < GMT_SMALL) {
			if (gmtdefs.verbose) fprintf (stderr, "GMT_grd_is_global: yes, longitude cells span exactly 360\n");
			return (TRUE);
		}
		else if (h->x_max - h->x_min > 360.0) {
			if (gmtdefs.verbose) fprintf (stderr, "GMT_grd_is_global: yes, longitudes span more than 360\n");
			return (TRUE);
		}
	}
	else if (h->y_min >= -90.0 && h->y_max <= 90.0) {
		if (fabs (h->x_max - h->x_min - 360.0) < GMT_SMALL) {
			fprintf (stderr, "GMT_grd_is_global: probably, x spans exactly 360 and -90 <= y <= 90\n");
			fprintf (stderr, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (FALSE);
		}
		else if (fabs (h->nx * h->x_inc - 360.0) < GMT_SMALL) {
			fprintf (stderr, "GMT_grd_is_global: probably, x cells span exactly 360 and -90 <= y <= 90\n");
			fprintf (stderr, "     To make sure the grid is recognized as geographical and global, use the -fg option\n");
			return (FALSE);
		}
	}
	if (gmtdefs.verbose) fprintf (stderr, "GMT_grd_is_global: no!\n");
	return (FALSE);
}

#define GMT_region_is_global ((fabs (project_info.e - project_info.w - 360.0) < GMT_SMALL && project_info.degree[0]))

GMT_LONG GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax, GMT_LONG interpolant)
{
	/* GMT_grd_setregion determines what w,e,s,n should be passed to GMT_read_grd.
	 * It does so by using project_info.w,e,s,n which have been set correctly by map_setup.
	 * Use interpolant to indicate if (and how) the grid is interpolated after this call.
	 * This determines possible extension of the grid to allow interpolation (without padding).
	 *
	 * Here are some considerations about the boundary we need to match, assuming the grid is gridline oriented:
	 * - When the output is to become pixels, the outermost point has to be beyond 0.5 cells inside the region
	 * - When linear interpolation is needed afterwards, the outermost point needs to be on the region edge or beyond
	 * - When the grid is pixel oriented, the limits need to go outward by another 0.5 cells
	 * - When the region is global, do not extend the longitudes outward (otherwise you create wrap-around issues)
	 * So to determine the boundary, we go inward from there.
	 */

	GMT_LONG grid_global;
	double shift_x, x_range, off;

	/* First make an educated guess whether the grid and region are geographical and global */
	grid_global = GMT_grd_is_global (h);

	switch (interpolant) {
		case BCR_BILINEAR:
			off = 0.0;
			break;
		case BCR_BSPLINE:
		case BCR_BICUBIC:
			off = 1.5;
			break;
		default:
			off = -0.5;
			break;
	}
	if (h->node_offset) off += 0.5;
	*ymin = project_info.s - off * h->y_inc, *ymax = project_info.n + off * h->y_inc;
	if (GMT_region_is_global) off = 0.0;
	*xmin = project_info.w - off * h->x_inc, *xmax = project_info.e + off * h->x_inc;

	if (!project_info.region && !GMT_IS_RECT_GRATICULE) {	/* Used -R... with oblique boundaries - return entire grid */
		if (*xmax < h->x_min)	/* Make adjustments so project_info.[w,e] jives with h->x_min|x_max */
			shift_x = 360.0;
		else if (*xmin > h->x_max)
			shift_x = -360.0;
		else
			shift_x = 0.0;

		*xmin = h->x_min + irint ((*xmin - h->x_min + shift_x) / h->x_inc) * h->x_inc;
		*xmax = h->x_max + irint ((*xmax - h->x_min + shift_x) / h->x_inc) * h->x_inc;
		*ymin = h->y_min + irint ((*ymin - h->y_min) / h->y_inc) * h->y_inc;
		*ymax = h->y_max + irint ((*ymax - h->y_min) / h->y_inc) * h->y_inc;

		/* Make sure we do not exceed grid domain (which can happen if project_info.w|e exceeds the grid limits) */
		if (*xmin < h->x_min && !grid_global) *xmin = h->x_min;
		if (*xmax > h->x_max && !grid_global) *xmax = h->x_max;
		if (*ymin < h->y_min) *ymin = h->y_min;
		if (*ymax > h->y_max) *ymax = h->y_max;

		/* If North or South pole are within the map boundary, we need all longitudes but restrict latitudes */
		if (!GMT_outside(0.0, +90.0)) *xmin = h->x_min, *xmax = h->x_max, *ymax = h->y_max;
		if (!GMT_outside(0.0, -90.0)) *xmin = h->x_min, *xmax = h->x_max, *ymin = h->y_min;
		return (0);
	}

	/* First set and check latitudes since they have no complications */
	*ymin = MAX (h->y_min, h->y_min + floor ((*ymin - h->y_min) / h->y_inc + GMT_SMALL) * h->y_inc);
	*ymax = MIN (h->y_max, h->y_min + ceil  ((*ymax - h->y_min) / h->y_inc - GMT_SMALL) * h->y_inc);

	if (*ymax <= *ymin) {	/* Grid must be outside chosen -R */
		if (gmtdefs.verbose) fprintf (stderr, "%s: Your grid y's or latitudes appear to be outside the map region and will be skipped.\n", GMT_program);
		return (1);
	}

	/* Periodic grid with 360 degree range is easy */

	if (grid_global) {
		*xmin = h->x_min + floor ((*xmin - h->x_min) / h->x_inc + GMT_SMALL) * h->x_inc;
		*xmax = h->x_min + ceil  ((*xmax - h->x_min) / h->x_inc - GMT_SMALL) * h->x_inc;
		/* For the odd chance that xmin or xmax are outside the region: bring them in */
		if (*xmax - *xmin >= 360.0) {
			while (*xmin < project_info.w) *xmin += h->x_inc;
			while (*xmax > project_info.e) *xmax -= h->x_inc;
		}
		return (0);
	}

	/* Shift a geographic grid 360 degrees up or down to maximize the amount of longitude range */

	if (GMT_io.in_col_type[0] == GMT_IS_LON) {
		x_range = MIN (*xmin, h->x_max) - MAX (*xmax, h->x_min);
		if (MIN (*xmin, h->x_max + 360.0) - MAX (*xmax, h->x_min + 360.0) > x_range)
			shift_x = 360.0;
		else if (MIN (*xmin, h->x_max - 360.0) - MAX (*xmax, h->x_min - 360.0) > x_range)
			shift_x = -360.0;
		else
			shift_x = 0.0;
		h->x_min += shift_x;
		h->x_max += shift_x;
	}

	*xmin = MAX (h->x_min, h->x_min + floor ((*xmin - h->x_min) / h->x_inc + GMT_SMALL) * h->x_inc);
	*xmax = MIN (h->x_max, h->x_min + ceil  ((*xmax - h->x_min) / h->x_inc - GMT_SMALL) * h->x_inc);

	if (*xmax <= *xmin) {	/* Grid is outside chosen -R in longitude */
		if (gmtdefs.verbose) fprintf (stderr, "%s: Your grid x's or longitudes appear to be outside the map region and will be skipped.\n", GMT_program);
		return (1);
	}
	return (0);
}

GMT_LONG GMT_adjust_loose_wesn (double *w, double *e, double *s, double *n, struct GRD_HEADER *header)
{
	/* Used to ensure that sloppy w,e,s,n values are rounded to the gridlines or pixels in the referenced grid.
	 * Upon entry, the boundaries w,e,s,n are given as a rough approximation of the actual subset needed.
	 * The routine will limit the boundaries to the grids region and round w,e,s,n to the nearest gridline or
	 * pixel boundaries (depending on the grid orientation).
	 * Warnings are produced when the w,e,s,n boundaries are adjusted, so this routine is currently not
	 * intended to throw just any values at it (although one could).
	 */
	
	GMT_LONG global, error = FALSE;
	double half_or_zero, val, dx, small;
	
	half_or_zero = (header->node_offset) ? 0.5 : 0.0;

	switch (GMT_minmaxinc_verify (*w, *e, header->x_inc, GMT_SMALL)) {	/* Check if range is compatible with x_inc */
		case 3:
			return (GMT_GRDIO_BAD_XINC);
			break;
		case 2:
			return (GMT_GRDIO_BAD_XRANGE);
			break;
		default:
			/* Everything is seemingly OK */
			break;
	}
	switch (GMT_minmaxinc_verify (*s, *n, header->y_inc, GMT_SMALL)) {	/* Check if range is compatible with y_inc */
		case 3:
			return (GMT_GRDIO_BAD_YINC);
			break;
		case 2:
			return (GMT_GRDIO_BAD_YRANGE);
			break;
		default:
			/* Everything is OK */
			break;
	}
	global = GMT_grd_is_global (header);

	if (!global) {
		if (*w < header->x_min) { *w = header->x_min; error = TRUE; }
		if (*e > header->x_max) { *e = header->x_max; error = TRUE; }
	}
	if (*s < header->y_min) { *s = header->y_min; error = TRUE; }
	if (*n > header->y_max) { *n = header->y_max; error = TRUE; }
	if (error) fprintf (stderr, "%s: Warning: Subset exceeds data domain. Subset reduced to common region.\n", GMT_program);
	error = FALSE;

	if (!(GMT_io.in_col_type[0] == GMT_IS_LON && GMT_360_RANGE (*w, *e) && global)) {    /* Do this unless a 360 longitude wrap */
		small = GMT_SMALL * header->x_inc;

		val = header->x_min + irint((*w - header->x_min) / header->x_inc) * header->x_inc;
		dx = fabs (*w - val);
		if (GMT_io.in_col_type[0] == GMT_IS_LON) dx = fmod (dx, 360.0);
		if (dx > small) {
			*w = val;
			(void) fprintf (stderr, "%s: GMT WARNING: (w - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
			(void) fprintf (stderr, "%s: GMT WARNING: w reset to %g\n", GMT_program, *w);
		}

		val = header->x_min + irint((*e - header->x_min) / header->x_inc) * header->x_inc;
		dx = fabs (*e - val);
		if (GMT_io.in_col_type[0] == GMT_IS_LON) dx = fmod (dx, 360.0);
		if (dx > GMT_SMALL) {
			*e = val;
			(void) fprintf (stderr, "%s: GMT WARNING: (e - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
			(void) fprintf (stderr, "%s: GMT WARNING: e reset to %g\n", GMT_program, *e);
		}
	}

	/* Check if s,n are a multiple of y_inc offset from y_min - if not adjust s, n */
	small = GMT_SMALL * header->y_inc;

	val = header->y_min + irint((*s - header->y_min) / header->y_inc) * header->y_inc;
	if (fabs (*s - val) > small) {
		*s = val;
		(void) fprintf (stderr, "%s: GMT WARNING: (s - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
		(void) fprintf (stderr, "%s: GMT WARNING: s reset to %g\n", GMT_program, *s);
	}

	val = header->y_min + irint((*n - header->y_min) / header->y_inc) * header->y_inc;
	if (fabs (*n - val) > small) {
		*n = val;
		(void) fprintf (stderr, "%s: GMT WARNING: (n - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %g.\n", GMT_program, GMT_SMALL);
		(void) fprintf (stderr, "%s: GMT WARNING: n reset to %g\n", GMT_program, *n);
	}
	return (GMT_NOERROR);
}

void GMT_grd_set_units (struct GRD_HEADER *header)
{
	/* Set unit strings for grid coordinates x, y and z based on
	   output data types for columns 0, 1, and 2.
	*/
	GMT_LONG i;
	char *string[3], unit[GRD_UNIT_LEN], date[GMT_CALSTRING_LENGTH], clock[GMT_CALSTRING_LENGTH];

	/* Copy pointers to unit strings */
	string[0] = header->x_units;
	string[1] = header->y_units;
	string[2] = header->z_units;

	/* Use input data type as backup fr output data type */
	for (i = 0; i < 3; i++) 
		if (GMT_io.out_col_type[i] == GMT_IS_UNKNOWN) GMT_io.out_col_type[i] = GMT_io.in_col_type[i];

	/* Catch some anomalies */
	if (GMT_io.out_col_type[0] == GMT_IS_LAT) {
		fprintf (stderr, "%s: Output type for X-coordinate of grid %s is LAT. Replaced by LON.\n", GMT_program, header->name);
		GMT_io.out_col_type[0] = GMT_IS_LON;
	}
	if (GMT_io.out_col_type[1] == GMT_IS_LON) {
		fprintf (stderr, "%s: Output type for Y-coordinate of grid %s is LON. Replaced by LAT.\n", GMT_program, header->name);
		GMT_io.out_col_type[1] = GMT_IS_LAT;
	}

	/* Set unit strings one by one based on output type */
	for (i = 0; i < 3; i++) {
		switch (GMT_io.out_col_type[i]) {
		case GMT_IS_LON:
			strcpy (string[i], "longitude [degrees_east]"); break;
		case GMT_IS_LAT:
			strcpy (string[i], "latitude [degrees_north]"); break;
		case GMT_IS_ABSTIME:
		case GMT_IS_RELTIME:
		case GMT_IS_RATIME:
			/* Determine time unit */
			switch (gmtdefs.time_system.unit) {
			case 'y':
				strcpy (unit, "years"); break;
			case 'o':
				strcpy (unit, "months"); break;
			case 'd':
				strcpy (unit, "days"); break;
			case 'h':
				strcpy (unit, "hours"); break;
			case 'm':
				strcpy (unit, "minutes"); break;
			default:
				strcpy (unit, "seconds"); break;
			}
			GMT_format_calendar (date, clock, &GMT_io.date_output, &GMT_io.clock_output, FALSE, 1, 0.0);
			sprintf (string[i], "time [%s since %s %s]", unit, date, clock);
			/* Warning for non-double grids */
			if (i == 2 && GMT_grdformats[header->type][1] != 'd') fprintf (stderr, "%s: Warning: Use double precision output grid to avoid loss of significance of time coordinate.\n", GMT_program);
			break;
		}
	}
}

void GMT_grd_get_units (struct GRD_HEADER *header)
{
	/* Set input data types for columns 0, 1 and 2 based on unit strings for
	   grid coordinates x, y and z.
	   When "Time": transform the data scale and offset to match the current time system.
	*/
	GMT_LONG i;
	char string[3][GMT_LONG_TEXT], *units;
	double scale = 1.0, offset = 0.0;
	struct GMT_TIME_SYSTEM time_system;

	/* Copy unit strings */
	strcpy (string[0], header->x_units);
	strcpy (string[1], header->y_units);
	strcpy (string[2], header->z_units);

	/* Parse the unit strings one by one */
	for (i = 0; i < 3; i++) {
		/* Skip parsing when input data type is already set */
		if (GMT_io.in_col_type[i] & GMT_IS_GEO) {
			project_info.degree[i] = TRUE;
			continue;
		}
		else if (GMT_io.in_col_type[i] & GMT_IS_RATIME) {
			project_info.xyz_projection[i] = GMT_TIME;
			continue;
		}

		/* Change name of variable and unit to lower case for comparison */
		GMT_str_tolower (string[i]);

		if ((!strncmp (string[i], "longitude", (size_t)9) || strstr (string[i], "degrees_e")) && (header->x_min > -360.0 && header->x_max <= 360.0)) {
			/* Input data type is longitude */
			GMT_io.in_col_type[i] = GMT_IS_LON;
			project_info.degree[i] = TRUE;
		}
		else if ((!strncmp (string[i], "latitude", (size_t)8) || strstr (string[i], "degrees_n")) && (header->y_min >= -90.0 && header->y_max <= 90.0)) {
			/* Input data type is latitude */
			GMT_io.in_col_type[i] = GMT_IS_LAT;
			project_info.degree[i] = TRUE;
		}
		else if (!strcmp (string[i], "time") || !strncmp (string[i], "time [", (size_t)6)) {
			/* Input data type is time */
			GMT_io.in_col_type[i] = GMT_IS_RELTIME;
			project_info.xyz_projection[i] = GMT_TIME;

			/* Determine coordinates epoch and units (default is internal system) */
			memcpy ((void *)&time_system, (void *)&gmtdefs.time_system, sizeof (struct GMT_TIME_SYSTEM));
			units = strchr (string[i], '[') + 1;
			if (!units || GMT_get_time_system (units, &time_system) || GMT_init_time_system_structure (&time_system))
				fprintf (stderr, "%s: Warning: Time units [%s] in grid not recognised, defaulting to gmtdefaults.\n", GMT_program, units);

			/* Determine scale between grid and internal time system, as well as the offset (in internal units) */
			scale = time_system.scale * gmtdefs.time_system.i_scale;
			offset = (time_system.rata_die - gmtdefs.time_system.rata_die) + (time_system.epoch_t0 - gmtdefs.time_system.epoch_t0);
			offset *= GMT_DAY2SEC_F * gmtdefs.time_system.i_scale;

			/* Scale data scale and extremes based on scale and offset */
			if (i == 0) {
				header->x_min = header->x_min * scale + offset;
				header->x_max = header->x_max * scale + offset;
				header->x_inc *= scale;
			}
			else if (i == 1) {
				header->y_min = header->y_min * scale + offset;
				header->y_max = header->y_max * scale + offset;
				header->y_inc *= scale;
			}
			else {
				header->z_add_offset = header->z_add_offset * scale + offset;
				header->z_scale_factor *= scale;
			}
		}
	}
}

GMT_LONG GMT_read_img (char *imgfile, struct GRD_HEADER *grd, float **grid, double w, double e, double s, double n, double scale, GMT_LONG mode, double lat, GMT_LONG init)
{
	/* Function that reads an entire Sandwell/Smith Mercator grid and stores it like a regular
	 * GMT grid.  If init is TRUE we also initialize the Mercator projection.  Lat should be 0.0
	 * if we are dealing with standard 72 or 80 img latitude; else it must be specified.
	 */

	GMT_LONG min, i, j, k, ij, mx, my, first_i, n_skip, n_cols;
	short int *i2;
	char file[BUFSIZ];
	struct STAT buf;
	FILE *fp;

	if (!GMT_getdatapath (imgfile, file)) return (GMT_GRDIO_FILE_NOT_FOUND);
	if (STAT (file, &buf)) return (GMT_GRDIO_STAT_FAILED);	/* Inquiry about file failed somehow */

	switch (buf.st_size) {	/* Known sizes are 1 or 2 min at lat_max = 72 or 80 */
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_80*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_80;
		case GMT_IMG_NLON_1M*GMT_IMG_NLAT_1M_72*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_72;
			min = 1;
			break;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_80;
		case GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_72*GMT_IMG_ITEMSIZE:
			if (lat == 0.0) lat = GMT_IMG_MAXLAT_72;
			min = 2;
			break;
		default:
			if (lat == 0.0) return (GMT_GRDIO_BAD_IMG_LAT);
			min = (buf.st_size > GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE) ? 1 : 2;
			fprintf (stderr, "%s: img file %s has unusual size - grid increment defaults to %ld min\n", GMT_program, file, min);
			break;
	}

	if (w == e && s == n) {	/* Default is entire grid */
		w = GMT_IMG_MINLON;	e = GMT_IMG_MAXLON;
		s = -lat;	n = lat;
	}

	GMT_grd_init (grd, 0, NULL, FALSE);
	grd->x_inc = grd->y_inc = min / 60.0;

	if ((fp = GMT_fopen (file, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
	if (init) {
		/* Select plain Mercator on a sphere with -Jm1 -R0/360/-lat/+lat */
		gmtdefs.ellipsoid = GMT_N_ELLIPSOIDS - 1;
		project_info.units_pr_degree = TRUE;
		project_info.pars[0] = 180.0;
		project_info.pars[1] = 0.0;
		project_info.pars[2] = 1.0;
		project_info.projection = GMT_MERCATOR;
		project_info.degree[0] = project_info.degree[1] = TRUE;

		GMT_err_pass (GMT_map_setup (GMT_IMG_MINLON, GMT_IMG_MAXLON, -lat, +lat), file);
	}

	if (w < 0.0 && e < 0.0) w += 360.0, e += 360.0;

	GMT_geo_to_xy (w, s, &grd->x_min, &grd->y_min);
	GMT_geo_to_xy (e, n, &grd->x_max, &grd->y_max);

	grd->x_min = MAX (GMT_IMG_MINLON, floor (grd->x_min / grd->x_inc) * grd->x_inc);
	grd->x_max = MIN (GMT_IMG_MAXLON, ceil (grd->x_max / grd->x_inc) * grd->x_inc);
	if (grd->x_min > grd->x_max) grd->x_min -= 360.0;
	grd->y_min = MAX (0.0, floor (grd->y_min / grd->y_inc) * grd->y_inc);
	grd->y_max = MIN (project_info.ymax, ceil (grd->y_max / grd->y_inc) * grd->y_inc);
	/* Allocate grid memory */

	grd->node_offset = 1;	/* These are always pixel grids */
	grd->nx = GMT_get_n (grd->x_min, grd->x_max, grd->x_inc, grd->node_offset);
	grd->ny = GMT_get_n (grd->y_min, grd->y_max, grd->y_inc, grd->node_offset);
	mx = grd->nx + GMT_pad[0] + GMT_pad[2];	my = grd->ny + GMT_pad[1] + GMT_pad[3];
	*grid = (float *) GMT_memory (VNULL, (mx * my), sizeof (float), GMT_program);
	grd->xy_off = 0.5;

	n_cols = (min == 1) ? GMT_IMG_NLON_1M : GMT_IMG_NLON_2M;		/* Number of columns (10800 or 21600) */
	first_i = (GMT_LONG)floor (grd->x_min / grd->x_inc);				/* first tile partly or fully inside region */
	if (first_i < 0) first_i += n_cols;
	n_skip = (GMT_LONG)floor ((project_info.ymax - grd->y_max) / grd->y_inc);	/* Number of rows clearly above y_max */
	if (GMT_fseek (fp, (long)(n_skip * n_cols * GMT_IMG_ITEMSIZE), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);

	i2 = (short int *) GMT_memory (VNULL, n_cols, sizeof (short int), GMT_program);
	for (j = 0; j < grd->ny; j++) {	/* Read all the rows, offset by 2 boundary rows and cols */
		ij = (j + GMT_pad[3]) * mx + GMT_pad[0];
		if (GMT_fread ((void *)i2, sizeof (short int), (size_t)n_cols, fp) != (size_t)n_cols)  return (GMT_GRDIO_READ_FAILED);	/* Get one row */
#if defined(_WIN32) || WORDS_BIGENDIAN == 0
		for (i = 0; i < n_cols; i++) i2[i] = GMT_swab2 (i2[i]);
#endif
		for (i = 0, k = first_i; i < grd->nx; i++) {	/* Process this row's values */
			switch (mode) {
				case 0:	/* No encoded track flags, do nothing */
					break;
				case 1:	/* Remove the track flag on odd (constrained) points */
					if (i2[k]%2) i2[k]--;
					break;
				case 2:	/* Remove the track flag on odd (constrained) points and set unconstrained to NaN */
					i2[k] = (i2[k]%2) ? i2[k] - 1 : SHRT_MIN;
					break;
				case 3:	/* Set odd (constrained) points to 1 and set unconstrained to 0 */
					i2[k] %= 2;
					break;
			}
			(*grid)[ij+i] = (float)((mode == 3) ? i2[k] : (i2[k] * scale));
			k++;
			if (k == n_cols) k = 0;	/* Wrapped around 360 */
		}
	}
	GMT_free ((void *)i2);
	GMT_fclose (fp);
	return (GMT_NOERROR);
}

struct GMT_GRID *GMT_create_grid (char *arg)
{	/* Allocates space for a new grid container.  No space allocated for the float grid itself */
	struct GMT_GRID * G;

	G = (struct GMT_GRID *) GMT_memory (VNULL, (GMT_LONG)1, sizeof (struct GMT_GRID), arg);
	G->header = (struct GRD_HEADER *) GMT_memory (VNULL, (GMT_LONG)1, sizeof (struct GRD_HEADER), arg);

	return (G);
}

void GMT_destroy_grid (struct GMT_GRID *G, GMT_LONG free_grid)
{
	if (!G) return;	/* Nothing to deallocate */
	if (G->data && free_grid) GMT_free ((void *)G->data);
	if (G->header) GMT_free ((void *)G->header);
	GMT_free ((void *)G);
}
