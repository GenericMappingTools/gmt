/*--------------------------------------------------------------------
 *	$Id: gmt_cdf.c,v 1.3 2001-04-11 19:58:09 pwessel Exp $
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
 *
 *	G M T _ C D F . C   R O U T I N E S
 *
 * Takes care of all grd input/output built on NCAR's netCDF routines (which is
 * an XDR implementation)
 * Most functions will exit with error message if an internal error is returned.
 * There functions are only called indirectly via the GMT_* grdio functions.
 *
 * Author:	Paul Wessel
 * Date:	9-SEP-1992
 * Modified:	13-JAN-2001
 * Version:	3.4
 *
 * Functions include:
 *
 *	GMT_cdf_read_grd_info :		Read header from file
 *	GMT_cdf_read_grd :		Read header and data set from file
 *	GMT_cdf_write_grd_info :	Update header in existing file
 *	GMT_cdf_write_grd :		Write header and data set to new file
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt.h"

char cdf_file[BUFSIZ];

int check_nc_status(int status);
int GMT_cdf_read_grd_info (char *file, struct GRD_HEADER *header)
{

	int cdfid, nm[2];
	size_t start[2], edge[2];
	double dummy[2];
	char text[GRD_COMMAND_LEN+GRD_REMARK_LEN];
	
	/* variable ids */
	int  x_range_id, y_range_id, z_range_id, inc_id, nm_id, z_id;

	if (!strcmp (file,"=")) {	/* Check if piping is attempted */
		fprintf (stderr, "%s: GMT Fatal Error: netcdf-based i/o does not support piping - exiting\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	
	/* Open file and get info */
	
	strcpy (cdf_file, file);
	check_nc_status (nc_open (file, NC_NOWRITE, &cdfid));

	memset ((void *)text, 0, (size_t)(GRD_COMMAND_LEN+GRD_REMARK_LEN));

	/* Get variable ids */
	check_nc_status (nc_inq_varid (cdfid, "x_range", &x_range_id));
	check_nc_status (nc_inq_varid (cdfid, "y_range", &y_range_id));
	check_nc_status (nc_inq_varid (cdfid, "z_range", &z_range_id));
        check_nc_status (nc_inq_varid (cdfid, "spacing", &inc_id));
        check_nc_status (nc_inq_varid (cdfid, "dimension", &nm_id));
        check_nc_status (nc_inq_varid (cdfid, "z", &z_id));

	/* Get attributes */
	check_nc_status (nc_get_att_text  (cdfid, x_range_id, "units", header->x_units));
        check_nc_status (nc_get_att_text  (cdfid, y_range_id, "units", header->y_units));
	check_nc_status (nc_get_att_text  (cdfid, z_range_id, "units", header->z_units));
        check_nc_status (nc_get_att_double  (cdfid, z_id, "scale_factor", &header->z_scale_factor));
        check_nc_status (nc_get_att_double  (cdfid, z_id, "add_offset", &header->z_add_offset));
        check_nc_status (nc_get_att_int  (cdfid, z_id, "node_offset", &header->node_offset));
        check_nc_status (nc_get_att_text  (cdfid, NC_GLOBAL, "title", header->title));
        check_nc_status (nc_get_att_text  (cdfid, NC_GLOBAL, "source", text));

	strncpy (header->command, text, GRD_COMMAND_LEN);
	strncpy (header->remark, &text[GRD_COMMAND_LEN], GRD_REMARK_LEN);
	
	/* Get variables */
	
	start[0] = 0;
	edge[0] = 2;
	
	check_nc_status (nc_get_vara_double(cdfid, x_range_id, start, edge, dummy));
	header->x_min = dummy[0];
	header->x_max = dummy[1];
	check_nc_status (nc_get_vara_double(cdfid, y_range_id, start, edge, dummy));
	header->y_min = dummy[0];
	header->y_max = dummy[1];
	check_nc_status (nc_get_vara_double(cdfid, inc_id, start, edge, dummy));
	header->x_inc = dummy[0];
	header->y_inc = dummy[1];
	check_nc_status (nc_get_vara_int(cdfid, nm_id, start, edge, nm));
	header->nx = nm[0];
	header->ny = nm[1];
	check_nc_status (nc_get_vara_double(cdfid, z_range_id, start, edge, dummy));
	header->z_min = dummy[0];
	header->z_max = dummy[1];

	check_nc_status (nc_close (cdfid));
	
	return (0);
}

int GMT_cdf_write_grd_info (char *file, struct GRD_HEADER *header)
{

	int  cdfid, nm[2];
	size_t start[2], edge[2];
	double dummy[2];
	char text[GRD_COMMAND_LEN+GRD_REMARK_LEN];
	
	/* variable ids */
	int  x_range_id, y_range_id, z_range_id, inc_id, nm_id, z_id;

	if (!strcmp (file,"=")) {	/* Check if piping is attempted */
		fprintf (stderr, "%s: GMT Fatal Error: netcdf-based i/o does not support piping - exiting\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	/* Open file and get info */
	
	strcpy (cdf_file, file);
	check_nc_status (nc_open (file, NC_WRITE, &cdfid));

	/* Get variable ids */
	check_nc_status (nc_inq_varid (cdfid, "x_range", &x_range_id));
        check_nc_status (nc_inq_varid (cdfid, "y_range", &y_range_id));
        check_nc_status (nc_inq_varid (cdfid, "z_range", &z_range_id));
        check_nc_status (nc_inq_varid (cdfid, "spacing", &inc_id));
        check_nc_status (nc_inq_varid (cdfid, "dimension", &nm_id));
        check_nc_status (nc_inq_varid (cdfid, "z", &z_id));

	/* rewrite attributes */
	
	check_nc_status (nc_redef (cdfid));

	memset ((void *)text, 0, (size_t)(GRD_COMMAND_LEN+GRD_REMARK_LEN));
	
	strcpy (text, header->command);
	strcpy (&text[GRD_COMMAND_LEN], header->remark);
	check_nc_status (nc_put_att_text (cdfid, x_range_id, "units", GRD_UNIT_LEN, header->x_units));
        check_nc_status (nc_put_att_text (cdfid, y_range_id, "units", GRD_UNIT_LEN, header->y_units));
        check_nc_status (nc_put_att_text (cdfid, z_range_id, "units", GRD_UNIT_LEN, header->z_units));
        check_nc_status (nc_put_att_double (cdfid, z_id, "scale_factor", NC_DOUBLE, 1, &header->z_scale_factor));
        check_nc_status (nc_put_att_double (cdfid, z_id, "add_offset", NC_DOUBLE, 1, &header->z_add_offset));
        check_nc_status (nc_put_att_int (cdfid, z_id, "node_offset", NC_LONG, 1, &header->node_offset));
        check_nc_status (nc_put_att_text (cdfid, NC_GLOBAL, "title", GRD_TITLE_LEN, header->title));
        check_nc_status (nc_put_att_text (cdfid, NC_GLOBAL, "source", (GRD_COMMAND_LEN+GRD_REMARK_LEN), text));
	
	/* leave define mode */
	check_nc_status (nc_enddef (cdfid));

	/* rewrite header information */
	
	start[0] = 0;
	edge[0] = 2;
	dummy[0] = header->x_min;
	dummy[1] = header->x_max;
	check_nc_status (nc_put_vara_double(cdfid, x_range_id, start, edge, dummy));
	dummy[0] = header->y_min;
	dummy[1] = header->y_max;
	check_nc_status (nc_put_vara_double(cdfid, y_range_id, start, edge, dummy));
	dummy[0] = header->x_inc;
	dummy[1] = header->y_inc;
	check_nc_status (nc_put_vara_double(cdfid, inc_id, start, edge, dummy));
	nm[0] = header->nx;
	nm[1] = header->ny;
	check_nc_status (nc_put_vara_int(cdfid, nm_id, start, edge, nm));
	dummy[0] = header->z_min;
	dummy[1] = header->z_max;
	check_nc_status (nc_put_vara_double(cdfid, z_range_id, start, edge, dummy));

	check_nc_status (nc_close (cdfid));
	
	return (0);
}

int GMT_cdf_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	/*
	 * Reads a subset of a grdfile and optionally pads the array with extra rows and columns
	 * header values for nx and ny are reset to reflect the dimensions of the logical array,
	 * not the physical size (i.e., the padding is not counted in nx and ny)
	 */
	 
	int  cdfid, z_id;
	size_t start[2], edge[2];
	int first_col, last_col, first_row, last_row;
	int i, j, ij, j2, width_in, width_out, height_in, i_0_out, kk, inc = 1;
	int *k;
	BOOLEAN check = !GMT_is_fnan (GMT_grd_in_nan_value);
		
	float *tmp;
		
	/* Open file and get info */
	
	if (!strcmp (file,"=")) {	/* Check if piping is attempted */
		fprintf (stderr, "%s: GMT Fatal Error: netcdf-based i/o does not support piping - exiting\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	
	strcpy (cdf_file, file);
 	check_nc_status (nc_open (file, NC_NOWRITE, &cdfid));

	/* Get variable id */
	check_nc_status (nc_inq_varid (cdfid, "z", &z_id));

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);
	
	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];
	
	i_0_out = pad[0];		/* Edge offset in output */
	if (complex) {	/* Need twice as much space and load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}
	
	tmp = (float *) GMT_memory (VNULL, (size_t)header->nx, sizeof (float), "GMT_cdf_read_grd");
	edge[0] = header->nx;
		
	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		start[0] = j * header->nx;
		/* Get one row */
		check_nc_status (nc_get_vara_float (cdfid, z_id, start, edge, tmp));
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij+i*inc;
			grid[kk] = tmp[k[i]];
			if (check && grid[kk] == GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			if ((double)grid[kk] < header->z_min) header->z_min = (double)grid[kk];
			if ((double)grid[kk] > header->z_max) header->z_max = (double)grid[kk];
		}
	}
	
	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	check_nc_status (nc_close (cdfid));
	
	GMT_free ((void *)tmp);
	GMT_free ((void *)k);

	return (0);
}

int GMT_cdf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, nc_type nc_type)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	size_t start[2], edge[2];
	int cdfid, nm[2];
	int i, i2, inc = 1, *k;
	int j, ij, j2, width_in, width_out, height_out;
	int first_col, last_col, first_row, last_row;
	
	double dummy[2];
	
	char text[GRD_COMMAND_LEN+GRD_REMARK_LEN];
	
	float *tmp;
	BOOLEAN check = !GMT_is_fnan (GMT_grd_out_nan_value);
	
	/* dimension ids */
	int  side_dim, xysize_dim;

	/* variable ids */
	int  x_range_id, y_range_id, z_range_id, inc_id, nm_id, z_id;

	/* variable shapes */
	int dims[1];

	if (!strcmp (file,"=")) {	/* Check if piping is attempted */
		fprintf (stderr, "%s: GMT Fatal Error: netcdf-based i/o does not support piping - exiting\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	
	/* Create file and enter define mode */
	
	check_nc_status (nc_create (file, NC_CLOBBER,&cdfid));

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	edge[0] = width_out;
	if (complex) inc = 2;

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	/* define dimensions */
	check_nc_status (nc_def_dim(cdfid, "side", 2, &side_dim));
	check_nc_status (nc_def_dim(cdfid, "xysize", (int) (width_out * height_out), &xysize_dim));

	/* define variables */

	dims[0]		= side_dim;
	check_nc_status (nc_def_var (cdfid, "x_range", NC_DOUBLE, 1, dims, &x_range_id));
        check_nc_status (nc_def_var (cdfid, "y_range", NC_DOUBLE, 1, dims, &y_range_id));
        check_nc_status (nc_def_var (cdfid, "z_range", NC_DOUBLE, 1, dims, &z_range_id));
        check_nc_status (nc_def_var (cdfid, "spacing", NC_DOUBLE, 1, dims, &inc_id));
        check_nc_status (nc_def_var (cdfid, "dimension", NC_LONG, 1, dims, &nm_id));


	dims[0]		= xysize_dim;
	check_nc_status (nc_def_var (cdfid, "z", nc_type, 1, dims, &z_id));

	/* assign attributes */
	
	memset ((void *)text, 0, (size_t)(GRD_COMMAND_LEN+GRD_REMARK_LEN));
	
	strcpy (text, header->command);
	strcpy (&text[GRD_COMMAND_LEN], header->remark);
	check_nc_status (nc_put_att_text (cdfid, x_range_id, "units", GRD_UNIT_LEN, header->x_units));
        check_nc_status (nc_put_att_text (cdfid, y_range_id, "units", GRD_UNIT_LEN, header->y_units));
        check_nc_status (nc_put_att_text (cdfid, z_range_id, "units", GRD_UNIT_LEN, header->z_units));
        check_nc_status (nc_put_att_double (cdfid, z_id, "scale_factor", NC_DOUBLE, 1, &header->z_scale_factor));
        check_nc_status (nc_put_att_double (cdfid, z_id, "add_offset", NC_DOUBLE, 1, &header->z_add_offset));
        check_nc_status (nc_put_att_int (cdfid, z_id, "node_offset", NC_LONG, 1, &header->node_offset));
        check_nc_status (nc_put_att_text (cdfid, NC_GLOBAL, "title", GRD_TITLE_LEN, header->title));
        check_nc_status (nc_put_att_text (cdfid, NC_GLOBAL, "source", (GRD_COMMAND_LEN+GRD_REMARK_LEN), text));
	
	/* leave define mode */
	check_nc_status (nc_enddef (cdfid));

	/* Find zmin/zmax */
	
	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[3]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[0]; i <= last_col; i++, i2++) {
			ij = (j2 * width_in + i2) * inc;
			if (GMT_is_fnan (grid[ij])) {
				if (check) grid[ij] = (float)GMT_grd_out_nan_value;
				continue;
			}
			header->z_min = MIN (header->z_min, grid[ij]);
			header->z_max = MAX (header->z_max, grid[ij]);
		}
	}
	
	/* store header information and array */
	
	start[0] = 0;
	edge[0] = 2;
	dummy[0] = header->x_min;
	dummy[1] = header->x_max;
	check_nc_status (nc_put_vara_double(cdfid, x_range_id, start, edge, dummy));
	dummy[0] = header->y_min;
	dummy[1] = header->y_max;
	check_nc_status (nc_put_vara_double(cdfid, y_range_id, start, edge, dummy));
	dummy[0] = header->x_inc;
	dummy[1] = header->y_inc;
	check_nc_status (nc_put_vara_double(cdfid, inc_id, start, edge, dummy));
	nm[0] = width_out;
	nm[1] = height_out;
	check_nc_status (nc_put_vara_int(cdfid, nm_id, start, edge, nm));
	dummy[0] = header->z_min;
	dummy[1] = header->z_max;
	check_nc_status (nc_put_vara_double(cdfid, z_range_id, start, edge, dummy));

	tmp = (float *) GMT_memory (VNULL, (size_t)width_in, sizeof (float), "GMT_cdf_write_grd");
		
	edge[0] = width_out;
	i2 = first_col + pad[0];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		start[0] = j * width_out;
		for (i = 0; i < width_out; i++) tmp[i] = grid[inc * (ij+k[i])];
		check_nc_status (nc_put_vara_float (cdfid, z_id, start, edge, tmp));
	}
	check_nc_status (nc_close (cdfid));

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);
	
	return (0);
}

/* This function checks the return status of a netcdf function and takes
 * appropriate action if the status != NC_NOERR
 */

int check_nc_status (int status)/*
int status;*/
{
	if (status != NC_NOERR) {
		fprintf (stderr, "%s: %s [%s]\n", GMT_program, nc_strerror(status), cdf_file);
		exit (EXIT_FAILURE);
	}
	
	return 0;
}
