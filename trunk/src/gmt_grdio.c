/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.c,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
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
 *	Contact info: www.soest.hawaii.edu/gmt
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
 * Modified:	27-JUN-2000
 * Version:	3.3.6
 *
 * Functions include:
 *
 *	GMT_grd_get_i_format :	Get format id for input grdfile
 *	GMT_grd_get_o_format :	Get format id for output grdfile
 *
 *	GMT_read_grd_info :	Read header from file
 *	GMT_read_grd :		Read header and data set from file
 *	GMT_write_grd_info :	Update header in existing file
 *	GMT_write_grd :		Write header and data set to new file
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt.h"

void GMT_grd_do_scaling (float *grid, int nm, double scale, double offset);

/* GENERIC I/O FUNCTIONS FOR GRIDDED DATA FILES */

int GMT_read_grd_info (char *file, struct GRD_HEADER *header)
{
	int status;
	char fname[BUFSIZ];
	double scale = GMT_d_NaN, offset = 0.0;

	GMT_grd_i_format = GMT_grd_get_i_format (file, fname, &scale, &offset);
	status = (*GMT_io_readinfo[GMT_grd_i_format]) (fname, header);
	if (GMT_is_dnan(scale))
		scale = header->z_scale_factor, offset = header->z_add_offset;
	else
		header->z_scale_factor = scale, header->z_add_offset = offset;
	if (scale == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	GMT_grd_RI_verify (header, 0);

	header->z_min = header->z_min * scale + offset;
	header->z_max = header->z_max * scale + offset;

	return (status);
}

int GMT_write_grd_info (char *file, struct GRD_HEADER *header)
{
	int status;
	char fname[BUFSIZ];
	double scale = header->z_scale_factor, offset = header->z_add_offset;
	
	GMT_grd_o_format = GMT_grd_get_o_format (file, fname, &scale, &offset);
	header->z_scale_factor = scale, header->z_add_offset = offset;
	header->z_min = (header->z_min - offset) / scale;
	header->z_max = (header->z_max - offset) / scale;
	status = (*GMT_io_writeinfo[GMT_grd_o_format]) (fname, header);
	return (status);
}

int GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int status;
	char fname[BUFSIZ];
	double scale = GMT_d_NaN, offset = 0.0;

	GMT_grd_i_format = GMT_grd_get_i_format (file, fname, &scale, &offset);
	status = (*GMT_io_readgrd[GMT_grd_i_format]) (fname, header, grid, w, e, s, n, pad, complex);
	if (GMT_is_dnan(scale))
		scale = header->z_scale_factor, offset = header->z_add_offset;
	else
		header->z_scale_factor = scale, header->z_add_offset = offset;
	if (scale == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	GMT_grd_do_scaling (grid, ((header->nx + pad[0] + pad[1]) * (header->ny + pad[2] + pad[3])), scale, offset);
	header->z_min = header->z_min * scale + offset;
	header->z_max = header->z_max * scale + offset;
	return (status);
}

int GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int status;
	char fname[BUFSIZ];
	double scale = header->z_scale_factor, offset = header->z_add_offset;

	GMT_grd_o_format = GMT_grd_get_o_format (file, fname, &scale, &offset);
	header->z_scale_factor = scale, header->z_add_offset = offset;
	GMT_grd_do_scaling (grid, (header->nx * header->ny), 1.0/scale, -offset/scale);
	status = (*GMT_io_writegrd[GMT_grd_o_format]) (fname, header, grid, w, e, s, n, pad, complex);
	return (status);
}

/* Routines to see if a particular grd file format is specified as part of filename. */

void GMT_expand_filename (char *file, char *fname)
{
	int i, length, f_length, found, start;

	if (gmtdefs.gridfile_shorthand) {	/* Look for matches */
		f_length = (int) strlen (file);
		for (i = found = 0; !found && i < GMT_n_file_suffix; i++) {
			length = (int) strlen (GMT_file_suffix[i]);
			start = f_length - length;
			found = (start < 0) ? FALSE : !strncmp (&file[start], GMT_file_suffix[i], (size_t)length);
		}
		if (found) {
			i--;
			sprintf (fname, "%s=%d/%lg/%lg/%lg\0", file, GMT_file_id[i], GMT_file_scale[i], GMT_file_offset[i], GMT_file_nan[i]);
		}
		else
			strcpy (fname, file);
	}
	else	/* Simply copy the full name */
		strcpy (fname, file);
}

int GMT_grd_get_i_format (char *file, char *fname, double *scale, double *offset)
{
	int i = 0, j, n, id = 0;

	GMT_expand_filename (file, fname);
	
	while (fname[i] && fname[i] != '=') i++;
	
	if (fname[i]) {	/* Check format id */
		i++;
		n = sscanf (&fname[i], "%d/%lf/%lf/%lf", &id, scale, offset, &GMT_grd_in_nan_value);
/*		if (n <= 1) *scale = GMT_d_NaN, *offset = 0.0;
		else if (n == 4) GMT_grd_in_nan_value = (GMT_grd_in_nan_value - *offset) / *scale; */
 		if (id < 0 || id >= N_GRD_FORMATS) {
			fprintf (stderr, "GMT Warning: grdfile format option (%d) unknown, reset to 0\n", id);
			id = 0;
		}
		j = (i == 1) ? i : i - 1;
		fname[j] = 0;
	}
	return (id);
}

int GMT_grd_get_o_format (char *file, char *fname, double *scale, double *offset)
{
	int i = 0, j, n, id = 0;

	GMT_expand_filename (file, fname);
	
	while (fname[i] && fname[i] != '=') i++;
	
	if (fname[i]) {	/* Check format id */
		i++;
		n = sscanf (&fname[i], "%d/%lf/%lf/%lf", &id, scale, offset, &GMT_grd_out_nan_value);
	/*	if (n == 4) GMT_grd_out_nan_value = (GMT_grd_out_nan_value - *offset) / *scale; */
		if (id < 0 || id >= N_GRD_FORMATS) {
			fprintf (stderr, "GMT Warning: grdfile format option (%d) unknown, reset to 0\n", id);
			id = 0;
		}
                j = (i == 1) ? i : i - 1;
                fname[j] = 0;
	}
	if (*scale == 0.0) {
		*scale = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0, reset to 1.\n");
 	}
	return (id);
}

/* Routine that scales and offsets the data if specified */

void GMT_grd_do_scaling (float *grid, int nm, double scale, double offset)
{
	int i;
	
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

void GMT_grd_RI_verify (struct GRD_HEADER *h, int mode)
{
	/* mode - 0 means we are checking an existing grid, mode = 1 means we test a new -R -I combination */
	
	int error = 0;
	
	if (!strcmp (GMT_program, "grdedit")) return;	/* Separate handling in grdedit to allow grdedit -A */

	switch (GMT_minmaxinc_verify (h->x_min, h->x_max, h->x_inc, SMALL)) {
		case 3:
			(void) fprintf (stderr, "%s: GMT ERROR: grid x increment <= 0.0\n", GMT_program);
			error++;
			break;
		case 2:
			(void) fprintf (stderr, "%s: GMT ERROR: grid x range <= 0.0\n", GMT_program);
			error++;
			break;
		case 1:
			(void) fprintf (stderr, "%s: GMT ERROR: (x_max-x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= %lg.\n", GMT_program, SMALL);
			error++;
		default:
			/* Everything is OK */
			break;
	}
		
	switch (GMT_minmaxinc_verify (h->y_min, h->y_max, h->y_inc, SMALL)) {
		case 3:
			(void) fprintf (stderr, "%s: GMT ERROR: grid y increment <= 0.0\n", GMT_program);
			error++;
			break;
		case 2:
			(void) fprintf (stderr, "%s: GMT ERROR: grid y range <= 0.0\n", GMT_program);
			error++;
			break;
		case 1:
			(void) fprintf (stderr, "%s: GMT ERROR: (y_max-y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= %lg.\n", GMT_program, SMALL);
			error++;
		default:
			/* Everything is OK */
			break;
	}
	if (error) {
		if (mode == 0)
			(void) fprintf (stderr, "%s: GMT ERROR: Use grdedit -A on your gridfile to make it compatible.\n", GMT_program);
		else
			(void) fprintf (stderr, "%s: GMT ERROR: Please select compatible -R and -I values.\n", GMT_program);
		exit (EXIT_FAILURE);
	}
}

int *GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, int *width, int *height, int *first_col, int *last_col, int *first_row, int *last_row)
{
	/* Determines which rows and colums to extract, and if it is
	 * a grid that is periodic and wraps around and returns indeces. */

	int one_or_zero, i, *k;
	BOOLEAN geo = FALSE;
	double small,off, half_or_zero, x;


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

		if (*w < header->x_min || *e > header->x_max) geo = TRUE;	/* Dealing with periodic grid */

		if (*s < header->y_min || *n > header->y_max) {	/* Calling program goofed... */
			fprintf (stderr, "GMT ERROR: Trying to read beyond grid domain - abort!!\n", GMT_program);
			exit (EXIT_FAILURE);
		}
		one_or_zero = (header->node_offset) ? 0 : 1;

		/* Get dimension of subregion */
	
		*width  = irint ((*e - *w) / header->x_inc) + one_or_zero;
		*height = irint ((*n - *s) / header->y_inc) + one_or_zero;
	
		/* Get first and last row and column numbers */
	
		small = 0.1 * header->x_inc;
		*first_col = (int)floor ((*w - header->x_min + small) / header->x_inc);
		*last_col  = (int)ceil  ((*e - header->x_min - small) / header->x_inc) - 1 + one_or_zero;
		small = 0.1 * header->y_inc;
		*first_row = (int)floor ((header->y_max - *n + small) / header->y_inc);
		*last_row  = (int)ceil  ((header->y_max - *s - small) / header->y_inc) - 1 + one_or_zero;

		if ((*last_col - *first_col + 1) > *width) (*last_col)--;
		if ((*last_row - *first_row + 1) > *height) (*last_row)--;
		if ((*last_col - *first_col + 1) > *width) (*first_col)++;
		if ((*last_row - *first_row + 1) > *height) (*first_row)++;
	}

	k = (int *) GMT_memory (VNULL, (size_t)(*width), sizeof (int), "GMT_bin_write_grd");
	if (geo) {
		off = (header->node_offset) ? 0.0 : 0.5;
		half_or_zero = (header->node_offset) ? 0.5 : 0.0;
		small = 0.1 * header->x_inc;	/* Anything smaller than 0.5 dx will do */
		for (i = 0; i < (*width); i++) {
			x = *w + (i + half_or_zero) * header->x_inc;
			if ((header->x_min - x) > small)
				x += 360.0;
			else if ((x - header->x_max) > small)
				x -= 360.0;
			k[i] = (int) floor (((x - header->x_min) / header->x_inc) + off);
		}
	}
	else {	/* Normal ordering */
		for (i = 0; i < (*width); i++) k[i] = (*first_col) + i;
	}

	return (k);
}

void GMT_decode_grd_h_info (char *input, struct GRD_HEADER *h) {

/*	Given input string, copy elements into string portions of h.
	Replace underscores by blanks.  Use "/" as the field separator.
	If a field has an equals sign, skip it.
	This routine is usually called if -D<input> was given by user,
	and after GMT_grd_init() has been called.
*/
	char	*ptr;
	int	entry;
	
	/* Replace blanks  */	
	for (entry = 0; input[entry]; entry++) if (input[entry] == '_') input[entry] = ' ';
	
	ptr = strtok (input, "/");
	entry = 0;
	while (ptr) {
		if (ptr[0] != '=') {
			switch (entry) {
				case 0:
					memset ( (void *)h->x_units, 0, (size_t)80);
					strcpy (h->x_units, ptr);
					break;
				case 1:
					memset ( (void *)h->y_units, 0, (size_t)80);
					strcpy (h->y_units, ptr);
					break;
				case 2:
					memset ( (void *)h->z_units, 0, (size_t)80);
					strcpy (h->z_units, ptr);
					break;
				case 3:
					h->z_scale_factor = atof(ptr);
					break;
				case 4:
					h->z_add_offset = atof(ptr);
					break;
				case 5:
					strcpy (h->title, ptr);
					break;
				case 6:
					strcpy (h->remark, ptr);
					break;
				default:
					break;
			}
		}
		ptr = strtok (CNULL, "/");
		entry++;
	}
	return;
}
