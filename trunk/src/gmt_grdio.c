/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.c,v 1.86 2006-11-29 15:25:23 remko Exp $
 *
 *	Copyright (c) 1991-2006 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
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

int GMT_grdformats[GMT_N_GRD_FORMATS][2] = {
#include "gmt_grdformats.h"
};

void GMT_grd_do_scaling (float *grid, int nm, double scale, double offset);
void GMT_grd_get_units (struct GRD_HEADER *header);
void GMT_grd_set_units (struct GRD_HEADER *header);
int GMT_is_nc_grid (char *file);
int GMT_is_native_grid (char *file);
int GMT_is_ras_grid (char *file);
int GMT_is_srf_grid (char *file);
int GMT_is_mgg2_grid (char *file);
int GMT_is_agc_grid (char *file);

/* GENERIC I/O FUNCTIONS FOR GRIDDED DATA FILES */

int GMT_read_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 */

	int status;
	double scale, offset, nan_value;

	/* Initialize grid information */
	GMT_grd_init (header, 0, (char **)VNULL, FALSE);

	/* Save parameters on file name suffix before issuing GMT_io_readinfo */
	GMT_grd_get_format (file, header, TRUE);
	scale = header->z_scale_factor, offset = header->z_add_offset, nan_value = header->nan_value;

	status = (*GMT_io_readinfo[header->type]) (header);
	GMT_grd_get_units (header);
	if (!GMT_is_dnan(scale)) header->z_scale_factor = scale, header->z_add_offset = offset;
	if (!GMT_is_dnan(nan_value)) header->nan_value = nan_value;
	if (header->z_scale_factor == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	GMT_grd_RI_verify (header, 0);

	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
	header->xy_off = 0.5 * header->node_offset;

	return (status);
}

int GMT_write_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	File name
	 * header:	grid structure header
	 */

	int status;

	GMT_grd_get_format (file, header, FALSE);
	if (GMT_is_dnan(header->z_scale_factor))
		header->z_scale_factor = 1.0;
	else if (header->z_scale_factor == 0.0) {
		header->z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	GMT_grd_set_units (header);
	status = (*GMT_io_writeinfo[header->type]) (header);
	return (status);
}

int GMT_update_grd_info (char *file, struct GRD_HEADER *header)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 */

	int status;

	header->z_min = (header->z_min - header->z_add_offset) / header->z_scale_factor;
	header->z_max = (header->z_max - header->z_add_offset) / header->z_scale_factor;
	GMT_grd_set_units (header);
	status = (*GMT_io_updateinfo[header->type]) (header);
	return (status);
}

int GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	- IGNORED -
	 * header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	int status;

	status = (*GMT_io_readgrd[header->type]) (header, grid, w, e, s, n, pad, complex);
	if (header->z_scale_factor == 0.0) fprintf (stderr, "GMT Warning: scale_factor should not be 0.\n");
	GMT_grd_do_scaling (grid, ((header->nx + pad[0] + pad[1]) * (header->ny + pad[2] + pad[3])), header->z_scale_factor, header->z_add_offset);
	header->z_min = header->z_min * header->z_scale_factor + header->z_add_offset;
	header->z_max = header->z_max * header->z_scale_factor + header->z_add_offset;
	header->xy_off = 0.5 * header->node_offset;
	return (status);
}

int GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name
	 * header:	grid structure header
	 * grid:	array with final grid
	 * w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex:	TRUE if array is to hold real and imaginary parts (read in real only)
	 *		Note: The file has only real values, we simply allow space in the array
	 *		for imaginary parts when processed by grdfft etc.
	 */

	int status;

	GMT_grd_get_format (file, header, FALSE);
	if (GMT_is_dnan(header->z_scale_factor))
		header->z_scale_factor = 1.0;
	else if (header->z_scale_factor == 0.0) {
		header->z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	GMT_grd_set_units (header);
	GMT_grd_do_scaling (grid, (header->nx * header->ny), 1.0/header->z_scale_factor, -header->z_add_offset/header->z_scale_factor);
	status = (*GMT_io_writegrd[header->type]) (header, grid, w, e, s, n, pad, complex);
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
			sprintf (fname, "%s=%d/%g/%g/%g", file, GMT_file_id[i], GMT_file_scale[i], GMT_file_offset[i], GMT_file_nan[i]);
		}
		else
			strcpy (fname, file);
	}
	else	/* Simply copy the full name */
		strcpy (fname, file);
}

void GMT_grd_get_format (char *file, struct GRD_HEADER *header, BOOLEAN magic)
{
	int i = 0, j;
	char code[GMT_TEXT_LEN];

	GMT_expand_filename (file, header->name);

	/* Set default values */
	header->z_scale_factor = GMT_d_NaN, header->z_add_offset = 0.0, header->nan_value = GMT_d_NaN;

	while (header->name[i] && header->name[i] != '=') i++;

	if (header->name[i]) {	/* Get format type, scale, offset and missing value from suffix */
		i++;
		sscanf (&header->name[i], "%[^/]/%lf/%lf/%lf", code, &header->z_scale_factor, &header->z_add_offset, &header->nan_value);
		header->type = GMT_grd_format_decoder (code);
		j = (i == 1) ? i : i - 1;
		header->name[j] = 0;
	}
	else if (magic) {	/* Determine file format automatically based on grid content */

		/* First check if we have a netCDF grid */
		if ((header->type = GMT_is_nc_grid (header->name)) >= 0) return;
		/* Then check for native GMT grid */
		if ((header->type = GMT_is_native_grid (header->name)) >= 0) return;
		/* Next check for Sun raster GMT grid */
		if ((header->type = GMT_is_ras_grid (header->name)) >= 0) return;
		/* Then check for Golden Software surfer grid */
		if ((header->type = GMT_is_srf_grid (header->name)) >= 0) return;
		/* Then check for NGDC GRD98 GMT grid */
		if ((header->type = GMT_is_mgg2_grid (header->name)) >= 0) return;
		/* Then check for AGC GMT grid */
		if ((header->type = GMT_is_agc_grid (header->name)) >= 0) return;

		fprintf (stderr, "Could not determine grid type of file %s\n", header->name);
		exit (EXIT_FAILURE);
	}
	else {			/* Get format type, scale, offset and missing value from gmtdefs.grid_format */
		sscanf (gmtdefs.grid_format, "%[^/]/%lf/%lf/%lf", code, &header->z_scale_factor, &header->z_add_offset, &header->nan_value);
		header->type = GMT_grd_format_decoder (code);
	}
}

int GMT_grd_data_size (int format, double *nan_value)
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
			fprintf (stderr, "Unknown grid data type: %c\n", GMT_grdformats[format][1]);
			exit (EXIT_FAILURE);
	}
}

int GMT_grd_format_decoder (const char *code)
{
	/* Returns the integer grid format ID that goes with the specified 2-character code */

	int id;

	if (isdigit ((int)code[0])) {	/* File format number given, convert directly */
		id = atoi (code);
 		if (id < 0 || id >= GMT_N_GRD_FORMATS) {
			fprintf (stderr, "%s: GMT ERROR: grdfile format number (%d) unknown!\n", GMT_program, id);
			exit (EXIT_FAILURE);
		}
	}
	else {	/* Character code given */
		int i, group;
		for (i = group = 0, id = -1; id < 0 && i < GMT_N_GRD_FORMATS; i++) {
			if (GMT_grdformats[i][0] == (short)code[0]) {
				group = code[0];
				if (GMT_grdformats[i][1] == (short)code[1]) id = i;
			}
		}

		if (id == -1) {
			if (group) fprintf (stderr, "%s: GMT ERROR: grdfile format type (%c) for group %c is unknown!\n", GMT_program, code[1], code[0]);
			else fprintf (stderr, "%s: GMT ERROR: grdfile format code %s unknown!\n", GMT_program, code);
			exit (EXIT_FAILURE);
		}
	}

	return (id);
}

void GMT_grd_do_scaling (float *grid, int nm, double scale, double offset)
{
	/* Routine that scales and offsets the data if specified */
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
	/* Determines which rows and columns to extract, and if it is
	 * a grid that is periodic and wraps around and returns indices. */

	int one_or_zero, i, *k;
	BOOLEAN geo = FALSE;
	double small, half_or_zero, x;

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

		if (GMT_io.in_col_type[0] == GMT_IS_LON) geo = TRUE;			/* Geographic data for sure */
		if (!geo && (*w < header->x_min || *e > header->x_max)) geo = TRUE;	/* Probably dealing with periodic grid */

		if (*s < header->y_min || *n > header->y_max) {	/* Calling program goofed... */
			fprintf (stderr, "%s: GMT ERROR: Trying to read beyond grid domain - abort!!\n", GMT_program);
			exit (EXIT_FAILURE);
		}
		one_or_zero = (header->node_offset) ? 0 : 1;

		/* Make sure w,e,s,n are proper multiples of dx,dy away from w, s */

		GMT_adjust_loose_wesn (w, e, s, n, header);

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
		small = 0.1 * header->x_inc;	/* Anything smaller than 0.5 dx will do */
		for (i = 0; i < (*width); i++) {
			x = *w + (i + half_or_zero) * header->x_inc;
			if ((header->x_min - x) > small)
				x += 360.0;
			else if ((x - header->x_max) > small)
				x -= 360.0;
			k[i] = GMT_x_to_i (x, header->x_min, header->x_inc, half_or_zero, header->nx);
		}
	}
	else {	/* Normal ordering */
		for (i = 0; i < (*width); i++) k[i] = (*first_col) + i;
	}

	return (k);
}

void GMT_decode_grd_h_info (char *input, struct GRD_HEADER *h) {

/*	Given input string, copy elements into string portions of h.
	Use "/" as the field separator.  If a field has an equals sign, skip it.
	This routine is usually called if -D<input> was given by user,
	and after GMT_grd_init() has been called.
*/
	char	ptr[BUFSIZ];
	int	entry = 0, pos = 0;

	while ((GMT_strtok (input, "/", &pos, ptr))) {
		if (ptr[0] != '=') {
			switch (entry) {
				case 0:
					memset ( (void *)h->x_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: X unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->x_units, ptr, GRD_UNIT_LEN);
					break;
				case 1:
					memset ( (void *)h->y_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: Y unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->y_units, ptr, GRD_UNIT_LEN);
					break;
				case 2:
					memset ( (void *)h->z_units, 0, (size_t)GRD_UNIT_LEN);
					if (strlen(ptr) >= GRD_UNIT_LEN) fprintf (stderr, "%s: GMT WARNING: Z unit string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_UNIT_LEN);
					strncpy (h->z_units, ptr, GRD_UNIT_LEN);
					break;
				case 3:
					h->z_scale_factor = atof (ptr);
					break;
				case 4:
					h->z_add_offset = atof (ptr);
					break;
				case 5:
					if (strlen(ptr) >= GRD_TITLE_LEN) fprintf (stderr, "%s: GMT WARNING: Title string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_TITLE_LEN);
					strncpy (h->title, ptr, GRD_TITLE_LEN);
					break;
				case 6:
					if (strlen(ptr) >= GRD_REMARK_LEN) fprintf (stderr, "%s: GMT WARNING: Remark string exceeds upper length of %d characters (truncated)\n", GMT_program, GRD_REMARK_LEN);
					strncpy (h->remark, ptr, GRD_REMARK_LEN);
					break;
				default:
					break;
			}
		}
		entry++;
	}
	return;
}

void GMT_open_grd (char *file, struct GMT_GRDFILE *G, char mode)
{
	/* Assumes header contents is already known.  For writing we
	 * assume that the header has already been written.  We fill
	 * the GRD_FILE structure with all the required information.
	 * mode can be w or r.  Upper case W or R refers to headerless
	 * grdraster-type files.
	 */

	int r_w;
	int cdf_mode[3] = { NC_NOWRITE, NC_WRITE, NC_WRITE};
	char *bin_mode[3] = { "rb", "rb+", "wb"};
	char GMT_fopen_path[BUFSIZ];
	BOOLEAN header = TRUE, magic = TRUE;
	EXTERN_MSC int GMT_nc_grd_info (struct GRD_HEADER *header, char job);

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
 	GMT_grd_get_format (file, &G->header, magic);
	if (GMT_is_dnan(G->header.z_scale_factor))
		G->header.z_scale_factor = 1.0;
	else if (G->header.z_scale_factor == 0.0) {
		G->header.z_scale_factor = 1.0;
		fprintf (stderr, "GMT Warning: scale_factor should not be 0. Reset to 1.\n");
	}
	if (GMT_grdformats[G->header.type][0] == 'c') {		/* Open netCDF file, old format */
		check_nc_status (nc_open (G->header.name, cdf_mode[r_w], &G->fid));
		if (header) GMT_nc_grd_info (&G->header, mode);
		G->edge[0] = G->header.nx;
		G->start[0] = G->start[1] = G->edge[1] = 0;
	}
	else if (GMT_grdformats[G->header.type][0] == 'n') {		/* Open netCDF file, COARDS-compliant format */
		check_nc_status (nc_open (G->header.name, cdf_mode[r_w], &G->fid));
		if (header) GMT_nc_grd_info (&G->header, mode);
		G->edge[0] = 1;
		G->edge[1] = G->header.nx;
		G->start[0] = G->header.ny-1;
		G->start[1] = 0;
	}
	else {				/* Regular binary file with/w.o standard GMT header */
		if (r_w == 0 && (G->fp = GMT_fopen (G->header.name, bin_mode[0])) == NULL) {
			fprintf (stderr, "%s: Error opening file %s\n", GMT_program, G->header.name);
			exit (EXIT_FAILURE);
		}
		else if ((G->fp = fopen (G->header.name, bin_mode[r_w])) == NULL) {
			fprintf (stderr, "%s: Error opening file %s\n", GMT_program, G->header.name);
			exit (EXIT_FAILURE);
		}
		if (header) fseek (G->fp, (long)GRD_HEADER_SIZE, SEEK_SET);
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

	G->v_row = (void *) GMT_memory (VNULL, G->n_byte, 1, GMT_program);

	G->row = 0;
	G->auto_advance = TRUE;	/* Default is to read sequential rows */
}

void GMT_close_grd (struct GMT_GRDFILE *G)
{
	if (GMT_grdformats[G->header.type][0] == 'c' || GMT_grdformats[G->header.type][0] == 'n')
		check_nc_status (nc_close (G->fid));
	else
		GMT_fclose (G->fp);
}

void GMT_read_grd_row (struct GMT_GRDFILE *G, int row_no, float *row)
{	/* Reads the entire row vector form the grdfile
	 * If row_no is negative it is interpreted to mean that we want to
	 * fseek to the start of the abs(row_no) record and no reading takes place.
	 */

	int i;

	if (GMT_grdformats[G->header.type][0] == 'c') {		/* Get one NetCDF row, old format */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = abs (row_no);
			G->start[0] = G->row * G->edge[0];
			return;
		}
		check_nc_status (nc_get_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
		if (G->auto_advance) G->start[0] += G->edge[0];
	}
	else if (GMT_grdformats[G->header.type][0] == 'n') {	/* Get one NetCDF row, COARDS-compliant format */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = abs (row_no);
			G->start[0] = G->header.ny - 1 - G->row;
			return;
		}
		check_nc_status (nc_get_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
		if (G->auto_advance) G->start[0] --;
	}
	else {			/* Get a binary row */
		if (row_no < 0) {	/* Special seek instruction */
			G->row = abs (row_no);
			fseek (G->fp, (long)(GRD_HEADER_SIZE + G->row * G->n_byte), SEEK_SET);
			return;
		}
		if (!G->auto_advance) fseek (G->fp, (long)(GRD_HEADER_SIZE + G->row * G->n_byte), SEEK_SET);

		if (fread (G->v_row, G->size, (size_t)G->header.nx, G->fp) != (size_t)G->header.nx) {	/* Get one row */
			fprintf (stderr, "%s: Read error for file %s near row %d\n", GMT_program, G->header.name, G->row);
			exit (EXIT_FAILURE);
		}
		for (i = 0; i < G->header.nx; i++) {
			row[i] = GMT_decode (G->v_row, i, GMT_grdformats[G->header.type][1]);	/* Convert whatever to float */
			if (G->check && row[i] == G->header.nan_value) row[i] = GMT_f_NaN;
		}
	}
	GMT_grd_do_scaling (row, G->header.nx, G->scale, G->offset);
	G->row++;
}

void GMT_write_grd_row (struct GMT_GRDFILE *G, int row_no, float *row)
{	/* Writes the entire row vector to the grdfile */

	int i, size;
	void *tmp;

	size = GMT_grd_data_size (G->header.type, &G->header.nan_value);

	tmp = (void *) GMT_memory (VNULL, (size_t)G->header.nx, size, "GMT_write_grd_row");

	GMT_grd_do_scaling (row, G->header.nx, G->scale, G->offset);
	for (i = 0; i < G->header.nx; i++) if (GMT_is_fnan (row[i]) && G->check) row[i] = (float)G->header.nan_value;

	switch (GMT_grdformats[G->header.type][0]) {
		case 'c':
			check_nc_status (nc_put_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
			if (G->auto_advance) G->start[0] += G->edge[0];
			break;
		case 'n':
			check_nc_status (nc_put_vara_float (G->fid, G->header.z_id, G->start, G->edge, row));
			if (G->auto_advance) G->start[0] --;
			break;
		default:
			for (i = 0; i < G->header.nx; i++) GMT_encode (tmp, i, row[i], GMT_grdformats[G->header.type][1]);
			fwrite (tmp, (size_t)size, (size_t)G->header.nx, G->fp);
	}

	 GMT_free (tmp);
}

/*
 * GMT_grd_init initializes a grd header to default values and copies the
 * command line to the header variable command
 */

void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, BOOLEAN update)
{	/* TRUE if we only want to update command line */
	int i, len;

	/* Always update command line history */

	memset ((void *)header->command, 0, (size_t)GRD_COMMAND_LEN);
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

	if (update) return;	/* Leave other variables unchanged */

	/* Here we initialize the variables to default settings */

	header->x_min = header->x_max	= 0.0;
	header->y_min = header->y_max	= 0.0;
	header->z_min = header->z_max	= 0.0;
	header->x_inc = header->y_inc	= 0.0;
	header->z_scale_factor		= 1.0;
	header->z_add_offset		= 0.0;
	header->nx = header->ny		= 0;
	header->node_offset		= 0;
	header->type			= -1;
	header->y_order			= 1;
	header->z_id			= -1;
	header->nan_value		= GMT_d_NaN;
	header->xy_off			= 0.0;

	memset ((void *)header->name, 0, (size_t)GMT_LONG_TEXT);

	memset ((void *)header->x_units, 0, (size_t)GRD_UNIT_LEN);
	memset ((void *)header->y_units, 0, (size_t)GRD_UNIT_LEN);
	memset ((void *)header->z_units, 0, (size_t)GRD_UNIT_LEN);
	strcpy (header->x_units, "x");
	strcpy (header->y_units, "y");
	strcpy (header->z_units, "z");
	memset ((void *)header->title, 0, (size_t)GRD_TITLE_LEN);
	memset ((void *)header->remark, 0, (size_t)GRD_REMARK_LEN);
}

void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift)
{
	/* Rotate geographical, global grid in e-w direction */
	/* This function will shift a grid by shift degrees */

	int i, j, k, ij, nc, nx1, n_shift, width, n_warn = 0;
	float *tmp;

	tmp = (float *) GMT_memory (VNULL, (size_t)header->nx, sizeof (float), "GMT_grd_shift");

	n_shift = irint (shift / header->x_inc);
	nx1 = header->nx - 1;
	width = (header->node_offset) ? header->nx : nx1;
	nc = header->nx * sizeof (float);

	for (j = ij = 0; j < header->ny; j++, ij += header->nx) {
		if (!header->node_offset && grd[ij] != grd[ij+nx1]) n_warn++;
		for (i = 0; i < header->nx; i++) {
			k = (i - n_shift) % width;
			if (k < 0) k += width;
			tmp[k] = grd[ij+i];
		}
		if (!header->node_offset) tmp[width] = tmp[0];
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

	if (n_warn) fprintf (stderr, "%s: Gridline-registered global grid has inconsistant values at repeated node for %d rows\n", GMT_program, n_warn);

	GMT_free ((void *) tmp);
}

/* GMT_grd_setregion determines what wesn should be passed to grd_read.  It
  does so by using project_info.w,e,s,n which have been set correctly
  by map_setup. */

int GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax)
{
	BOOLEAN region_straddle, grid_straddle, global;
	double shift_x = 0.0;

	if (!project_info.region && !GMT_IS_RECT_GRATICULE) {	/* Used -R... with oblique boundaries - return entire grid */
		BOOLEAN N_outside, S_outside;
		global = (fabs (h->x_max - h->x_min - 360.0) < GMT_SMALL);	/* A global grid */
		N_outside = GMT_outside (0.0, +90.0);	/* North pole outside map boundary? */
		S_outside = GMT_outside (0.0, -90.0);	/* South pole outside map boundary? */
		/* Note: while h->xy_off might be 0.5 (pixel) or 0 (gridline), the w/e boundaries are always "gridline" hence we pass 0 as xy_off */
		if (N_outside && S_outside) {	/* No polar complications, return extreme coordinates */
			if (project_info.e < h->x_min)	/* Make adjustments so project_info.[w,e] jives with h->x_min|x_max */
				shift_x = 360.0;
			else if (project_info.w > h->x_max)
				shift_x = -360.0;
			*xmin = GMT_i_to_x (GMT_x_to_i (project_info.w + shift_x, h->x_min, h->x_inc, 0.0, h->nx), h->x_min, h->x_max, h->x_inc, 0.0, h->nx);
			*xmax = GMT_i_to_x (GMT_x_to_i (project_info.e + shift_x, h->x_min, h->x_inc, 0.0, h->nx), h->x_min, h->x_max, h->x_inc, 0.0, h->nx);
			*ymin = GMT_j_to_y (GMT_y_to_j (project_info.s, h->y_min, h->y_inc, 0.0, h->ny), h->y_min, h->y_max, h->y_inc, 0.0, h->ny);
			*ymax = GMT_j_to_y (GMT_y_to_j (project_info.n, h->y_min, h->y_inc, 0.0, h->ny), h->y_min, h->y_max, h->y_inc, 0.0, h->ny);
			/* Make sure we dont exceed grid domain (which can happen if project_info.w|e exceeds the grid limits) */
			if ((*xmin) < h->x_min && !global) *xmin = h->x_min;
			if ((*xmax) > h->x_max && !global) *xmax = h->x_max;
			if ((*ymin) < h->y_min) *ymin = h->y_min;
			if ((*ymax) > h->y_max) *ymax = h->y_max;
		}
		else if (!N_outside) {	/* North pole included, need all longitudes but restrict latitudes */
			*xmin = h->x_min;	*xmax = h->x_max;
			*ymin = GMT_j_to_y (GMT_y_to_j (project_info.s, h->y_min, h->y_inc, 0.0, h->ny), h->y_min, h->y_max, h->y_inc, 0.0, h->ny);
			*ymax = h->y_max;
			if ((*ymin) < h->y_min) *ymin = h->y_min;
		}
		else {			/* South pole included, need all longitudes but restrict latitudes */
			*xmin = h->x_min;	*xmax = h->x_max;
			*ymin = h->y_min;
			*ymax = GMT_j_to_y (GMT_y_to_j (project_info.n, h->y_min, h->y_inc, 0.0, h->ny), h->y_min, h->y_max, h->y_inc, 0.0, h->ny);
			if ((*ymax) > h->y_max) *ymax = h->y_max;
		}
		return (0);
	}

	/* Weakness in the logic below is that there is no flag in the grid to tell us it is lon/lat data.
	 * We infer the grid is global (in longitude) and geographic if w-e == 360 && |s,n| <= 90 */

	/* First set and check latitudes since they have no complications */
#ifdef SHIT
	*ymin = MAX (h->y_min, floor (project_info.s / h->y_inc) * h->y_inc);
	*ymax = MIN (h->y_max,  ceil (project_info.n / h->y_inc) * h->y_inc);
#endif
	*ymin = MAX (h->y_min, h->y_min + floor ((project_info.s - h->y_min) / h->y_inc) * h->y_inc);
	*ymax = MIN (h->y_max, h->y_min + ceil  ((project_info.n - h->y_min) / h->y_inc) * h->y_inc);

	if ((*ymax) <= (*ymin)) {	/* Grid must be outside chosen -R */
		if (gmtdefs.verbose) fprintf (stderr, "%s: Your grid y's or latitudes appear to be outside the map region and will be skipped.\n", GMT_program);
		return (1);
	}

	if (GMT_io.in_col_type[0] != GMT_IS_LON) {	/* Regular Cartesian stuff is easy... */
#ifdef SHIT
		*xmin = MAX (h->x_min, floor (project_info.w / h->x_inc) * h->x_inc);
		*xmax = MIN (h->x_max,  ceil (project_info.e / h->x_inc) * h->x_inc);
#endif
		*xmin = MAX (h->x_min, h->x_min + floor ((project_info.w - h->x_min) / h->x_inc) * h->x_inc);
		*xmax = MIN (h->x_max, h->x_min + ceil  ((project_info.e - h->x_min) / h->x_inc) * h->x_inc);
		if ((*xmax) <= (*xmin)) {	/* Grid is outside chosen -R */
			if (gmtdefs.verbose) fprintf (stderr, "%s: Your grid x-range appear to be outside the plot region and will be skipped.\n", GMT_program);
			return (1);
		}
		return (0);
	}

	/* OK, longitudes are trickier and we must make sure grid and region is on the same page as far as +-360 degrees go */

	global = (fabs (h->x_max - h->x_min - 360.0) < GMT_SMALL && h->y_min >= -90.0 && h->y_max <= +90.0);	/* We believe this indicates a global (in longitude), geographic grid */
	if (global) {	/* Periodic grid with 360 degree range is easy */
		*xmin = project_info.w;
		*xmax = project_info.e;
		return (0);
	}

	global = (fabs (project_info.e - project_info.w - 360.0) < GMT_SMALL && project_info.s >= -90.0 && project_info.n <= +90.0);	/* A global -R selected */
	if (global) {	/* Global map with full 360 degree range is easy */
		*xmin = h->x_min;
		*xmax = h->x_max;
		return (0);
	}

	/* There are 4 cases depending on whether the chosen region or the grid straddles Greenwich */

	region_straddle = (project_info.w < 0.0 && project_info.e >= 0.0) ? TRUE : FALSE;
	grid_straddle   = (h->x_min < 0.0 && h->x_max >= 0.0) ? TRUE : FALSE;

	if (! (region_straddle || grid_straddle)) {	/* Case 1: Neither -R nor grid straddles Greenwich */
		/* Here we KNOW that w/e has already been forced to be positive (0-360 range).
		 * Just make sure the grid w/e is positive too when we compare range.
		 */
		 shift_x = (h->x_min < 0.0 && h->x_max <= 0.0) ? 360.0 : 0.0;	/* Shift to SUBTRACT from w/e ... when comparing */
	}
	else if (region_straddle && grid_straddle) {	/* Case 2: Both straddle Greenwich */
		/* Here we know both mins are -ve and both max are +ve, so there should be no complications */
		shift_x = 0.0;
	}
	else if (region_straddle && !grid_straddle) {	/* Case 3a: Region straddles Greenwich but grid doesnt */
		/* Here we know w is -ve and e is +ve.
		 * Must make sure the grid w/e is in same range when comparing.
		 */
		 shift_x = (h->x_max < project_info.w) ? 360.0 : 0.0;	/* Shift to SUBTRACT from w/e ... when comparing */
	}
	else {	/* Case 3b: Grid straddles Greenwich but region doesnt */
		/* Here we KNOW that w/e has been forced to be positive (0-360 range).
		 * Make sure the grid w/e is in same range
		 */
		 shift_x = (h->x_max < project_info.w) ? 360.0 : 0.0;	/* Shift to SUBTRACT from w/e ... when comparing */
	}

	h->x_min += shift_x;
	h->x_max += shift_x;
#ifdef SHIT
	*xmin = MAX (h->x_min, floor (project_info.w / h->x_inc) * h->x_inc);
	*xmax = MIN (h->x_max, ceil  (project_info.e / h->x_inc) * h->x_inc);
#endif
	*xmin = MAX (h->x_min, h->x_min + floor ((project_info.w - h->x_min) / h->x_inc) * h->x_inc);
	*xmax = MIN (h->x_max, h->x_min + ceil  ((project_info.e - h->x_min) / h->x_inc) * h->x_inc);
	while (*xmin <= -360) (*xmin) += 360.0;
	while (*xmax <= -360) (*xmax) += 360.0;

	if ((*xmax) <= (*xmin)) {	/* Grid is outside chosen -R in longitude */
		if (gmtdefs.verbose) fprintf (stderr, "%s: Your grid longitudes appear to be outside the map region and will be skipped.\n", GMT_program);
		return (1);
	}
	return (0);
}

void GMT_grd_set_units (struct GRD_HEADER *header)
{
	/* Set unit strings for grid coordinates x, y and z based on
	   output data types for columns 0, 1, and 2.
	*/
	int i;
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
			strcpy (string[i], "Longitude [degrees_east]"); break;
		case GMT_IS_LAT:
			strcpy (string[i], "Latitude [degrees_north]"); break;
		case GMT_IS_ABSTIME:
		case GMT_IS_RELTIME:
		case GMT_IS_RATIME:
			/* Determine time unit */
			switch (GMT_time_system[gmtdefs.time_system].unit) {
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
			sprintf (string[i], "Time [%s since %s %s]", unit, date, clock);
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
	int i;
	char string[3][GRD_UNIT_LEN], *cal, *l;
	double scale = 1.0, offset = 0.0;

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

		if (!strncmp (string[i], "lon", 3) || strstr (string[i], "degrees_e")) {
			/* Input data type is longitude */
			GMT_io.in_col_type[i] = GMT_IS_LON;
			project_info.degree[i] = TRUE;
		}
		else if (!strncmp (string[i], "lat", 3) || strstr (string[i], "degrees_n")) {
			/* Input data type is latitude */
			GMT_io.in_col_type[i] = GMT_IS_LAT;
			project_info.degree[i] = TRUE;
		}
		else if (!strncmp (string[i], "time", 4)) {
			/* Input data type is time */
			GMT_io.in_col_type[i] = GMT_IS_RELTIME;
			project_info.xyz_projection[i] = GMT_TIME;
			/* Determine relative time units */
			if (strstr (string[i], "years"))
				scale = GMT_YR2SEC_F / GMT_time_system[gmtdefs.time_system].scale;
			else if (strstr (string[i], "months"))
				scale = GMT_MON2SEC_F / GMT_time_system[gmtdefs.time_system].scale;
			else if (strstr (string[i], "days"))
				scale = GMT_DAY2SEC_F / GMT_time_system[gmtdefs.time_system].scale;
			else if (strstr (string[i], "hours"))
				scale = GMT_HR2SEC_F / GMT_time_system[gmtdefs.time_system].scale;
			else if (strstr (string[i], "minutes"))
				scale = GMT_MIN2SEC_F / GMT_time_system[gmtdefs.time_system].scale;
			else if (strstr (string[i], "seconds"))
				scale = 1.0 / GMT_time_system[gmtdefs.time_system].scale;
			else
				fprintf (stderr, "%s: Warning: Time unit in grid not recognised; assumed %c.\n", GMT_program, GMT_time_system[gmtdefs.time_system].unit);
			/* Determine relative time epoch */
			if ((l = strstr (string[i], "since"))) {
				cal = l + 6;
				if ((l = strchr (cal, ' '))) *l = 'T';
				if (GMT_scanf (cal, GMT_IS_ABSTIME, &offset) == GMT_IS_NAN) fprintf (stderr, "%s: Warning: Epoch in grid not recognised; assumed %s.\n", GMT_program, GMT_time_system[gmtdefs.time_system].epoch);
			}
			else
				fprintf (stderr, "%s: Warning: No epoch for time in grid specified; assumed %s.\n", GMT_program, GMT_time_system[gmtdefs.time_system].epoch);
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

#define GMT_IMG_MINLON		0.0
#define GMT_IMG_MAXLON		360.0
#define GMT_IMG_MINLAT_72	-72.0059773539
#define GMT_IMG_MAXLAT_72	+72.0059773539
#define GMT_IMG_MINLAT_80	-80.738
#define GMT_IMG_MAXLAT_80	+80.738
#define GMT_IMG_NLON_1M		21600	/* At 1 min resolution */
#define GMT_IMG_NLON_2M		10800	/* At 2 min resolution */
#define GMT_IMG_NLAT_1M_72	12672	/* At 1 min resolution */
#define GMT_IMG_NLAT_1M_80	17280	/* At 1 min resolution */
#define GMT_IMG_NLAT_2M_72	6336	/* At 1 min resolution */
#define GMT_IMG_NLAT_2M_80	8640	/* At 1 min resolution */
#define GMT_IMG_ITEMSIZE	2	/* Size of 2 byte short ints */

void GMT_read_img (char *imgfile, struct GRD_HEADER *grd, float **grid, double w, double e, double s, double n, double scale, int mode, double lat, BOOLEAN init)
{
	/* Function that reads an entire Sandwell/Smith Mercator grid and stores it like a regular
	 * GMT grid.  If init is TRUE we also initialize the Mercator projection.  Lat should be 0.0
	 * if we are dealing with standard 72 or 80 img latitude; else it must be specified.
	 */

	int min, i, j, k, ij, mx, my, first_i, n_skip, n_cols;
	short int *i2;
	char file[BUFSIZ];
	char GMT_fopen_path[BUFSIZ];
	struct STAT buf;
	FILE *fp;

	if (!GMT_getdatapath (imgfile, file)) {
		fprintf (stderr, "%s: Unable to find file %s\n", GMT_program, imgfile);
		exit (EXIT_FAILURE);
	}
	if (STAT (file, &buf)) {	/* Inquiry about file failed somehow */
		fprintf (stderr, "%s: Unable to stat file %s\n", GMT_program, imgfile);
		exit (EXIT_FAILURE);
	}

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
			if (lat == 0.0) {
				fprintf (stderr, "%s: Must specify max latitude for img file %s\n", GMT_program, file);
				exit (EXIT_FAILURE);
			}
			min = (buf.st_size > GMT_IMG_NLON_2M*GMT_IMG_NLAT_2M_80*GMT_IMG_ITEMSIZE) ? 1 : 2;
			fprintf (stderr, "%s: img file %s has unusual size - grid increment defaults to %d min\n", GMT_program, file, min);
			break;
	}

	if (w == e && s == n) {	/* Default is entire grid */
		w = GMT_IMG_MINLON;	e = GMT_IMG_MAXLON;
		s = -lat;	n = lat;
	}

	GMT_grd_init (grd, 0, NULL, FALSE);
	grd->x_inc = grd->y_inc = min / 60.0;

	if ((fp = GMT_fopen (file, "rb")) == NULL) {
		fprintf (stderr, "%s: Error opening img file %s\n", GMT_program, file);
		exit (EXIT_FAILURE);
	}
	if (init) {
		/* Select plain Mercator on a sphere with -Jm1 -R0/360/-lat/+lat */
		gmtdefs.ellipsoid = GMT_N_ELLIPSOIDS - 1;
		project_info.units_pr_degree = TRUE;
		project_info.m_got_parallel = FALSE;
		project_info.pars[0] = 1.0;
		project_info.projection = GMT_MERCATOR;
		project_info.degree[0] = project_info.degree[1] = TRUE;

		GMT_map_setup (GMT_IMG_MINLON, GMT_IMG_MAXLON, -lat, +lat);
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
	*grid = (float *) GMT_memory (VNULL, (size_t)(mx * my), sizeof (float), GMT_program);
	grd->xy_off = 0.5;

	n_cols = (min == 1) ? GMT_IMG_NLON_1M : GMT_IMG_NLON_2M;		/* Number of columns (10800 or 21600) */
	first_i = (int)floor (grd->x_min / grd->x_inc);				/* first tile partly or fully inside region */
	if (first_i < 0) first_i += n_cols;
	n_skip = (int)floor ((project_info.ymax - grd->y_max) / grd->y_inc);	/* Number of rows clearly above y_max */
	if (fseek (fp, (long)(n_skip * n_cols * GMT_IMG_ITEMSIZE), SEEK_SET)) {
		fprintf (stderr, "%s: Unable to seek ahead in file %s\n", GMT_program, imgfile);
		exit (EXIT_FAILURE);
	}

	i2 = (short int *) GMT_memory (VNULL, (size_t)n_cols, sizeof (short int), GMT_program);
	for (j = 0; j < grd->ny; j++) {	/* Read all the rows, offset by 2 boundary rows and cols */
		ij = (j + GMT_pad[3]) * mx + GMT_pad[0];
		fread ((void *)i2, sizeof (short int), n_cols, fp);
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
}

struct GMT_GRID *GMT_create_grid (char *arg)
{	/* Allocates space for a new grid container.  No space allocated for the float grid itself */
	struct GMT_GRID * G;

	G = (struct GMT_GRID *) GMT_memory (VNULL, 1, sizeof (struct GMT_GRID), arg);
	G->header = (struct GRD_HEADER *) GMT_memory (VNULL, 1, sizeof (struct GRD_HEADER), arg);

	return (G);
}

void GMT_destroy_grid (struct GMT_GRID *G, BOOLEAN free_grid)
{
	if (!G) return;	/* Nothing to deallocate */
	if (G->data && free_grid) GMT_free ((void *)G->data);
	if (G->header) GMT_free ((void *)G->header);
	GMT_free ((void *)G);
}
