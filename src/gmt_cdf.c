/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 *
 *	G M T _ C D F . C   R O U T I N E S
 *
 * Takes care of all grd input/output built on NCAR's netCDF routines (which is
 * an XDR implementation)
 * Most functions will return with error message if an internal error is returned.
 * There functions are only called indirectly via the GMT_* grdio functions.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 *
 * Public functions (5):
 *
 *	gmt_cdf_read_grd_info   : Read header from file
 *	gmt_cdf_read_grd        : Read header and data set from file
 *	gmt_cdf_update_grd_info : Update header in existing file
 *	gmt_cdf_write_grd_info  : Write header to new file
 *	gmt_cdf_write_grd       : Write header and data set to new file
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt_dev.h"
#include "gmt_internals.h"

int gmt_cdf_grd_info (struct GMT_CTRL *GMT, int ncid, struct GMT_GRID_HEADER *header, char job) {
	int err;	/* Implicitly by gmt_M_err_trap */
	int nm[2];
	double dummy[2];
	char text[GMT_GRID_COMMAND_LEN320+GMT_GRID_REMARK_LEN160];
	size_t limit = 2147483647U;	/* 2^31 - 1 is the max length of a 1-D array in netCDF */
	nc_type z_type;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	/* Dimension ids, variable ids, etc. */
	int side_dim, xysize_dim, x_range_id, y_range_id, z_range_id, inc_id, nm_id, z_id, dims[1];

	/* Define and get dimensions and variables */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Enter gmt_cdf_grd_info with argument %c\n", (int)job);

	if (job == 'w') {
		if (header->nm > limit) {	/* Print error message and let things crash */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Your grid contains more than 2^31 - 1 nodes (%" PRIu64 ") and cannot be stored with the deprecated GMT netCDF format.\n", header->nm);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Please choose another grid format such as the default netCDF 4 COARDS-compliant grid format.\n");
			return (GMT_DIM_TOO_LARGE);
		}
		gmt_M_err_trap (nc_def_dim (ncid, "side", 2U, &side_dim));
		gmt_M_err_trap (nc_def_dim (ncid, "xysize", header->nm, &xysize_dim));

		dims[0]	= side_dim;
		gmt_M_err_trap (nc_def_var (ncid, "x_range", NC_DOUBLE, 1, dims, &x_range_id));
		gmt_M_err_trap (nc_def_var (ncid, "y_range", NC_DOUBLE, 1, dims, &y_range_id));
		gmt_M_err_trap (nc_def_var (ncid, "z_range", NC_DOUBLE, 1, dims, &z_range_id));
		gmt_M_err_trap (nc_def_var (ncid, "spacing", NC_DOUBLE, 1, dims, &inc_id));
		gmt_M_err_trap (nc_def_var (ncid, "dimension", NC_LONG, 1, dims, &nm_id));

		switch (header->type) {
			case GMT_GRID_IS_CB: z_type = NC_BYTE;   break;
			case GMT_GRID_IS_CS: z_type = NC_SHORT;  break;
			case GMT_GRID_IS_CI: z_type = NC_INT;    break;
			case GMT_GRID_IS_CF: z_type = NC_FLOAT;  break;
			case GMT_GRID_IS_CD: z_type = NC_DOUBLE; break;
			default:			z_type = NC_NAT;
		}

		dims[0]	= xysize_dim;
		gmt_M_err_trap (nc_def_var (ncid, "z", z_type, 1, dims, &z_id));
	}
	else {
		gmt_M_err_trap (nc_inq_varid (ncid, "x_range", &x_range_id));
		gmt_M_err_trap (nc_inq_varid (ncid, "y_range", &y_range_id));
		gmt_M_err_trap (nc_inq_varid (ncid, "z_range", &z_range_id));
		gmt_M_err_trap (nc_inq_varid (ncid, "spacing", &inc_id));
		gmt_M_err_trap (nc_inq_varid (ncid, "dimension", &nm_id));
		gmt_M_err_trap (nc_inq_varid (ncid, "z", &z_id));
		gmt_M_err_trap (nc_inq_vartype (ncid, z_id, &z_type));
		switch (z_type) {
			case NC_BYTE:   header->type = GMT_GRID_IS_CB; HH->orig_datatype = GMT_CHAR;   break;
			case NC_SHORT:  header->type = GMT_GRID_IS_CS; HH->orig_datatype = GMT_SHORT;  break;
			case NC_INT:    header->type = GMT_GRID_IS_CI; HH->orig_datatype = GMT_INT;    break;
			case NC_FLOAT:  header->type = GMT_GRID_IS_CF; HH->orig_datatype = GMT_FLOAT;  break;
			case NC_DOUBLE: header->type = GMT_GRID_IS_CD; HH->orig_datatype = GMT_DOUBLE; break;
			default:        header->type = k_grd_unknown_fmt; break;
		}
	}
	HH->z_id = z_id;

	/* Get or assign attributes */

	gmt_M_memset (text, GMT_GRID_COMMAND_LEN320+GMT_GRID_REMARK_LEN160, char);

	if (job == 'u') gmt_M_err_trap (nc_redef (ncid));

	if (job == 'r') {
		int reg;
		gmt_M_err_trap (nc_get_att_text (ncid, x_range_id, "units", header->x_units));
		gmt_M_err_trap (nc_get_att_text (ncid, y_range_id, "units", header->y_units));
		gmt_M_err_trap (nc_get_att_text (ncid, z_range_id, "units", header->z_units));
		gmt_M_err_trap (nc_get_att_double (ncid, z_id, "scale_factor", &header->z_scale_factor));
		gmt_M_err_trap (nc_get_att_double (ncid, z_id, "add_offset", &header->z_add_offset));
		gmt_M_err_trap (nc_get_att_int (ncid, z_id, "node_offset", &reg));
		header->registration = reg;
#ifdef DOUBLE_PRECISION_GRID
		nc_get_att_double (ncid, z_id, "_FillValue", &header->nan_value);
#else
		nc_get_att_float (ncid, z_id, "_FillValue", &header->nan_value);
#endif
		gmt_M_err_trap (nc_get_att_text (ncid, NC_GLOBAL, "title", header->title));
		gmt_M_err_trap (nc_get_att_text (ncid, NC_GLOBAL, "source", text));
		strncpy (header->command, text, GMT_GRID_COMMAND_LEN320-1);
		strncpy (header->remark, &text[GMT_GRID_COMMAND_LEN320], GMT_GRID_REMARK_LEN160-1);

		gmt_M_err_trap (nc_get_var_double (ncid, x_range_id, dummy));
		header->wesn[XLO] = dummy[0];
		header->wesn[XHI] = dummy[1];
		gmt_M_err_trap (nc_get_var_double (ncid, y_range_id, dummy));
		header->wesn[YLO] = dummy[0];
		header->wesn[YHI] = dummy[1];
		gmt_M_err_trap (nc_get_var_double (ncid, inc_id, dummy));
		header->inc[GMT_X] = dummy[0];
		header->inc[GMT_Y] = dummy[1];
		gmt_M_err_trap (nc_get_var_int (ncid, nm_id, nm));
		header->n_columns = nm[0];
		header->n_rows = nm[1];
		gmt_M_err_trap (nc_get_var_double (ncid, z_range_id, dummy));
		header->z_min = dummy[0];
		header->z_max = dummy[1];
		//HH->row_order = k_nc_start_north; /* N->S but that is just when read.  We don't want to set this for writing later */
	}
	else {
		int reg;
		strncpy (text, header->command, GMT_GRID_COMMAND_LEN320-1);
		strncpy (&text[GMT_GRID_COMMAND_LEN320], header->remark, GMT_GRID_REMARK_LEN160-1);
		gmt_M_err_trap (nc_put_att_text (ncid, x_range_id, "units", GMT_GRID_UNIT_LEN80, header->x_units));
		gmt_M_err_trap (nc_put_att_text (ncid, y_range_id, "units", GMT_GRID_UNIT_LEN80, header->y_units));
		gmt_M_err_trap (nc_put_att_text (ncid, z_range_id, "units", GMT_GRID_UNIT_LEN80, header->z_units));
		gmt_M_err_trap (nc_put_att_double (ncid, z_id, "scale_factor", NC_DOUBLE, 1U, &header->z_scale_factor));
		gmt_M_err_trap (nc_put_att_double (ncid, z_id, "add_offset", NC_DOUBLE, 1U, &header->z_add_offset));
		if (z_type != NC_FLOAT && z_type != NC_DOUBLE)
			header->nan_value = rintf (header->nan_value); /* round to integer */
#ifdef DOUBLE_PRECISION_GRID
		gmt_M_err_trap (nc_put_att_double (ncid, z_id, "_FillValue", z_type, 1U, &header->nan_value));
#else
		gmt_M_err_trap (nc_put_att_float (ncid, z_id, "_FillValue", z_type, 1U, &header->nan_value));
#endif
		reg = header->registration;
		gmt_M_err_trap (nc_put_att_int (ncid, z_id, "node_offset", NC_LONG, 1U, &reg));
		gmt_M_err_trap (nc_put_att_text (ncid, NC_GLOBAL, "title", GMT_GRID_TITLE_LEN80, header->title));
		gmt_M_err_trap (nc_put_att_text (ncid, NC_GLOBAL, "source", GMT_GRID_COMMAND_LEN320+GMT_GRID_REMARK_LEN160, text));

		gmt_M_err_trap (nc_enddef (ncid));

		dummy[0] = header->wesn[XLO];	dummy[1] = header->wesn[XHI];
		gmt_M_err_trap (nc_put_var_double (ncid, x_range_id, dummy));
		dummy[0] = header->wesn[YLO];	dummy[1] = header->wesn[YHI];
		gmt_M_err_trap (nc_put_var_double (ncid, y_range_id, dummy));
		dummy[0] = header->inc[GMT_X];	dummy[1] = header->inc[GMT_Y];
		gmt_M_err_trap (nc_put_var_double (ncid, inc_id, dummy));
		nm[0] = header->n_columns;	nm[1] = header->n_rows;
		gmt_M_err_trap (nc_put_var_int (ncid, nm_id, nm));
		if (header->z_min <= header->z_max) {
			dummy[0] = header->z_min; dummy[1] = header->z_max;
		}
		else {
			dummy[0] = 0.0; dummy[1] = 0.0;
		}
		gmt_M_err_trap (nc_put_var_double (ncid, z_range_id, dummy));
	}
	return (GMT_NOERROR);
}

int gmt_cdf_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	int ncid, err;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	if (!strcmp (HH->name,"=")) return (GMT_GRDIO_NC_NO_PIPE);
	gmt_M_err_trap (nc_open (HH->name, NC_NOWRITE, &ncid));
	gmt_M_err_trap (gmt_cdf_grd_info (GMT, ncid, header, 'r'));
	gmt_M_err_trap (nc_close (ncid));
	return (GMT_NOERROR);
}

int gmt_cdf_update_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	int ncid, old_fill_mode, err;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	if (!strcmp (HH->name,"=")) return (GMT_GRDIO_NC_NO_PIPE);
	gmt_M_err_trap (nc_open (HH->name, NC_WRITE, &ncid));
	gmt_M_err_trap (nc_set_fill (ncid, NC_NOFILL, &old_fill_mode));
	gmt_M_err_trap (gmt_cdf_grd_info (GMT, ncid, header, 'u'));
	gmt_M_err_trap (nc_close (ncid));
	return (GMT_NOERROR);
}

int gmt_cdf_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	int ncid, old_fill_mode, err;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	if (!strcmp (HH->name,"=")) return (GMT_GRDIO_NC_NO_PIPE);
	gmt_M_err_trap (nc_create (HH->name, NC_CLOBBER, &ncid));
	gmt_M_err_trap (nc_set_fill (ncid, NC_NOFILL, &old_fill_mode));
	gmt_M_err_trap (gmt_cdf_grd_info (GMT, ncid, header, 'w'));
	gmt_M_err_trap (nc_close (ncid));
	return (GMT_NOERROR);
}

int gmt_cdf_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to extract  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the complex array
	 *		for real and imaginary parts when processed by grdfft etc.
	 *
	 * Reads a subset of a grid file and optionally pads the array with extra rows and columns
	 * header values for n_columns and n_rows are reset to reflect the dimensions of the logical array,
	 * not the physical size (i.e., the padding is not counted in n_columns and n_rows)
	 */

	bool check;
	int  ncid, j, err, first_col, last_col, first_row, last_row;
	unsigned int i, width_in, height_in;
	unsigned int width_out, *actual_col = NULL;
	uint64_t ij, kk, imag_offset;
	size_t start[1], edge[1];
	gmt_grdfloat *tmp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	/* Open the NetCDF file */

	if (!strcmp (HH->name,"=")) {
		gmt_M_free (GMT, actual_col);
		return (GMT_GRDIO_NC_NO_PIPE);
	}
	if ((err = nc_open (HH->name, NC_NOWRITE, &ncid)) != 0) {
		gmt_M_free (GMT, actual_col);
		return err;
	}
	check = !isnan (header->nan_value);

	/* Load data row by row. The data in the file is stored in the same
	 * "upside down" fashion as within GMT. The first row is the top row */

	tmp = gmt_M_memory (GMT, NULL, header->n_columns, gmt_grdfloat);

	edge[0] = header->n_columns;
	ij = imag_offset + pad[YHI] * width_out + pad[XLO];
	header->z_min =  DBL_MAX;
	header->z_max = -DBL_MAX;
	HH->has_NaNs = GMT_GRID_NO_NANS;	/* We are about to check for NaNs and if none are found we retain 1, else 2 */

	for (j = first_row; j <= last_row; j++, ij += width_out) {
		start[0] = j * header->n_columns;
		if ((err = gmt_nc_get_vara_grdfloat (ncid, HH->z_id, start, edge, tmp))) {	/* Get one row */
			gmt_M_free (GMT, actual_col);
			gmt_M_free (GMT, tmp);
			nc_close (ncid);
			return (err);
		}
		for (i = 0; i < width_in; i++) {	/* Check for and handle NaN proxies */
			kk = ij+i;
			grid[kk] = tmp[actual_col[i]];
			if (check && grid[kk] == header->nan_value)
				grid[kk] = GMT->session.f_NaN;
			if (gmt_M_is_fnan (grid[kk])) {
				HH->has_NaNs = GMT_GRID_HAS_NANS;
				continue;
			}
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}

	header->n_columns = width_in;
	header->n_rows = height_in;
	gmt_M_memcpy (header->wesn, wesn, 4, double);

	gmt_M_free (GMT, actual_col);
	gmt_M_free (GMT, tmp);
	gmt_M_err_trap (nc_close (ncid));

	return (GMT_NOERROR);
}

int gmt_cdf_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to write out  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the complex array
	 *		for real and imaginary parts when processed by grdfft etc.
	 */

	size_t start[1], edge[1];
	int ncid, old_fill_mode, err, first_col, last_col, first_row, last_row;
	long *tmp_i = NULL;
	unsigned int i, *actual_col = NULL;
	unsigned int j, width_out, height_out, width_in;
	uint64_t ij, nr_oor = 0, imag_offset;
	gmt_grdfloat *tmp_f = NULL;
	double limit[2] = {-FLT_MAX, FLT_MAX};
	gmt_grdfloat value;
	nc_type z_type;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name,"=")) return (GMT_GRDIO_NC_NO_PIPE);	/* Cannot do piping on netCDF files */

	/* Determine the value to be assigned to missing data, if not already done so */

	switch (header->type) {
		case GMT_GRID_IS_CB:
			if (isnan (header->nan_value)) header->nan_value = CHAR_MIN;
			limit[0] = CHAR_MIN - 0.5; limit[1] = CHAR_MAX + 0.5;
			z_type = NC_BYTE; break;
		case GMT_GRID_IS_CS:
			if (isnan (header->nan_value)) header->nan_value = SHRT_MIN;
			limit[0] = SHRT_MIN - 0.5; limit[1] = SHRT_MAX + 0.5;
			z_type = NC_SHORT; break;
		case GMT_GRID_IS_CI:
			if (isnan (header->nan_value)) header->nan_value = INT_MIN;
			limit[0] = INT_MIN - 0.5; limit[1] = INT_MAX + 0.5;
			z_type = NC_INT; break;
		case GMT_GRID_IS_CF:
			z_type = NC_FLOAT; break;
		case GMT_GRID_IS_CD:
			z_type = NC_DOUBLE; break;
		default:
			z_type = NC_NAT;
	}

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	gmt_M_memcpy (header->wesn, wesn, 4, double);
	header->n_columns = width_out;
	header->n_rows = height_out;

	/* Write grid header */

	if ((err = nc_create (HH->name, NC_CLOBBER, &ncid))) {
		gmt_M_free (GMT, actual_col);
		return (err);
	}
	if ((err = nc_set_fill (ncid, NC_NOFILL, &old_fill_mode))) {
		gmt_M_free (GMT, actual_col);
		return (err);
	}
	if ((err = gmt_cdf_grd_info (GMT, ncid, header, 'w'))) {
		gmt_M_free (GMT, actual_col);
		return (err);
	}

	/* Set start position for writing grid */

	edge[0] = width_out;
	ij = first_col + pad[XLO] + (uint64_t)(first_row + pad[YHI]) * width_in;
	header->z_min =  DBL_MAX;
	header->z_max = -DBL_MAX;

	/* Store z-variable */

	if (z_type == NC_FLOAT || z_type == NC_DOUBLE) {
		tmp_f = gmt_M_memory (GMT, NULL, width_in, gmt_grdfloat);
		for (j = 0; j < height_out; j++, ij += width_in) {
			start[0] = j * width_out;
			for (i = 0; i < width_out; i++) {
				value = grid[ij+actual_col[i]+imag_offset];
				if (!isfinite (value)) {
					if (isinf(value))
						nr_oor++; /* out of gmt_grdfloat range */
					tmp_f[i] = header->nan_value;
				}
				else {
					tmp_f[i] = value;
					header->z_min = MIN (header->z_min, (double)tmp_f[i]);
					header->z_max = MAX (header->z_max, (double)tmp_f[i]);
				}
			}
			if ((err = gmt_nc_put_vara_grdfloat (ncid, HH->z_id, start, edge, tmp_f))) {
				gmt_M_free (GMT, actual_col);
				gmt_M_free (GMT, tmp_f);
				return (err);
			}
		}
		gmt_M_free (GMT, tmp_f);
	}
	else {
		tmp_i = gmt_M_memory (GMT, NULL, width_in, long);
		for (j = 0; j < height_out; j++, ij += width_in) {
			start[0] = j * width_out;
			for (i = 0; i < width_out; i++) {
				value = grid[ij+actual_col[i]+imag_offset];
				if (!isfinite (value)) {
					if (isinf(value))
						nr_oor++; /* out of gmt_grdfloat range */
					tmp_i[i] = lrintf (header->nan_value);
				}
				else if (value <= limit[0] || value >= limit[1]) {
					tmp_i[i] = lrintf (header->nan_value);
					nr_oor++;
				}
				else {
					tmp_i[i] = lrintf (value);
					header->z_min = MIN (header->z_min, (double)tmp_i[i]);
					header->z_max = MAX (header->z_max, (double)tmp_i[i]);
				}
			}
			if ((err = nc_put_vara_long (ncid, HH->z_id, start, edge, tmp_i))) {
				gmt_M_free (GMT, actual_col);
				gmt_M_free (GMT, tmp_i);
				return (err);
			}
		}
		gmt_M_free (GMT, tmp_i);
	}

	if (nr_oor > 0) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " out-of-range grid values converted to _FillValue [%s]\n", nr_oor, HH->name);

	gmt_M_free (GMT, actual_col);

	if (header->z_min <= header->z_max) {
		limit[0] = header->z_min; limit[1] = header->z_max;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "No valid values in grid [%s]\n", HH->name);
		limit[0] = 0.0; limit[1] = 0.0;
	}
	gmt_M_err_trap (nc_put_var_double (ncid, HH->z_id - 3, limit));

	/* Close grid */

	gmt_M_err_trap (nc_close (ncid));

	return (GMT_NOERROR);
}
