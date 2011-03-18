/*--------------------------------------------------------------------
 *	$Id: gmt_esri_io.c,v 1.3 2011-03-18 06:12:30 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it wi1552ll be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
 /* Contains the read/write functions needed to handle ERSI Arc/Info ASCII Exchange grids.
  * Based on previous code in grd2xyz and xyz2grd.  A few limitations of the format:
  * 1) Grid spacing must be square (dx = dy) since only one cell_size record.
  * 2) NaNs must be stored via a proxy value [Auto-defaults to -9999 if not set].
  *
  * Paul Wessel, June 2010.
  */

GMT_LONG GMT_is_esri_grid (struct GMT_CTRL *C, struct GRD_HEADER *header)
{	/* Determine if file is an ESRI Interchange ASCII file */
	FILE *fp = NULL;
	char record[BUFSIZ], *not_used = NULL;

	if (!strcmp (header->name, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (C, header->name, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	not_used = GMT_fgets (C, record, BUFSIZ, fp);	/* Just get first line */
	GMT_fclose (C, fp);
	if (strncmp (record, "ncols ", 6)) return (-1);	/* Not this kind of file */

	header->type = GMT_grd_format_decoder (C, "aa");
	return (header->type);
}

GMT_LONG read_esri_info (struct GMT_CTRL *C, FILE *fp, struct GRD_HEADER *header)
{
	int c;
	char record[BUFSIZ], *not_used = NULL;

	header->registration = GMT_GRIDLINE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;
	
	not_used = GMT_fgets (C, record, BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->nx) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding ncols record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	not_used = GMT_fgets (C, record, BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->ny) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding ncols record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	not_used = GMT_fgets (C, record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding xll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "xllcorner", (size_t)9)) header->registration = GMT_PIXEL_REG;	/* Pixel grid */
	not_used = GMT_fgets (C, record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YLO]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding yll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "yllcorner", (size_t)9)) header->registration = GMT_PIXEL_REG;	/* Pixel grid */
	not_used = GMT_fgets (C, record, BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding cellsize record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	/* Handle the optional nodata_value record */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') {	/*	Assume this is a nodata_value record since we found an 'n|N' */
		not_used = GMT_fgets (C, record, BUFSIZ, fp);
		if (sscanf (record, "%*s %lf", &header->nan_value) != 1) {
			GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
			return (GMT_GRDIO_READ_FAILED);
		}
	}
	header->inc[GMT_Y] = header->inc[GMT_X];
	header->wesn[XHI] = header->wesn[XLO] + (header->nx - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YHI] = header->wesn[YLO] + (header->ny - 1 + header->registration) * header->inc[GMT_Y];

	GMT_err_fail (C, GMT_grd_RI_verify (C, header, 1), header->name);

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	GMT_LONG error;
	FILE *fp = NULL;

	if (!strcmp (header->name, "="))	/* Pipe in from stdin */
		fp = C->session.std[GMT_IN];
	else if ((fp = GMT_fopen (C, header->name, "r")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if ((error = read_esri_info (C, fp, header))) return (error);

	GMT_fclose (C, fp);
		
	return (GMT_NOERROR);
}

GMT_LONG write_esri_info (struct GMT_CTRL *C, FILE *fp, struct GRD_HEADER *header)
{
	char record[BUFSIZ], item[GMT_TEXT_LEN];

	sprintf (record, "ncols %d\nnrows %d\n", header->nx, header->ny);
	GMT_fputs (record, fp);		/* Write a text record */
	if (header->registration == GMT_PIXEL_REG) {	/* Pixel format */
		sprintf (record, "xllcorner ");
		sprintf (item, C->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcorner ");
		sprintf (item, C->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
	}
	else {	/* Gridline format */
		sprintf (record, "xllcenter ");
		sprintf (item, C->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcenter ");
		sprintf (item, C->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
	}
	sprintf (record, "cellsize ");
	sprintf (item, C->current.setting.format_float_out, header->inc[GMT_X]);
	strcat  (record, item);	strcat  (record, "\n");
	GMT_fputs (record, fp);		/* Write a text record */
	if (GMT_is_fnan (header->nan_value)) {
		GMT_report (C, GMT_MSG_NORMAL, "WARNING: ESRI Arc/Info ASCII Interchange file must use proxy for NaN; default to -9999\n");
		header->nan_value = -9999.0;
	}
	sprintf (record, "nodata_value %ld\n", (GMT_LONG)irint (header->nan_value));
	GMT_fputs (record, fp);		/* Write a text record */

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	
	if (!strcmp (header->name, "="))	/* Write to stdout */
		fp = C->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (C, header->name, "w")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	
	write_esri_info (C, fp, header);

	GMT_fclose (C, fp);
	
	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG pad[], GMT_LONG complex)
{
	GMT_LONG col, width_out, height_in, ii, kk, in_nx, inc = 1, off = 0;
	GMT_LONG first_col, last_col, first_row, last_row, n_left;
	GMT_LONG row, row2, col2, ij, width_in, check, error, *k = NULL;
	float value, *tmp = NULL;
	FILE *fp = NULL;
	
	if (!strcmp (header->name, "="))	/* Read from pipe */
		fp = C->session.std[GMT_IN];
	else if ((fp = GMT_fopen (C, header->name, C->current.io.r_mode)) != NULL) {
		if ((error = read_esri_info (C, fp, header))) return (error);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);
	
	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	if (complex) {inc = 2; off = complex - 1; }	/* Need twice as much output space since we load every 2nd cell */

	tmp = GMT_memory (C, NULL, header->nx, float);

	n_left = header->nm;

	/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids.
	 * NaNs are not allowed; they are represented by a nodata_value instead. */
	col = row = 0;		/* For the entire file */
	col2 = row2 = 0;	/* For the inside region */
	check = !GMT_is_dnan (header->nan_value);
	in_nx = header->nx;
	header->nx = (int)width_in;
	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	while (fscanf (fp, "%f", &value) == 1 && n_left) {	/* We read all values and skip those not inside our w/e/s/n */
		tmp[col] = value;	/* Build up a single input row */
		col++;
		if (col == in_nx) {	/* End of input row */
			if (row >= first_row && row <= last_row) {	/* We want a piece (or all) of this row */
				ij = GMT_IJP (header, row2, 0);	/* First out index for this row */
				for (ii = 0; ii < width_in; ii++) {
					kk = inc * (ij + ii) + off;
					grid[kk] = (check && tmp[k[ii]] == header->nan_value) ? C->session.f_NaN : tmp[k[ii]];
					if (GMT_is_fnan (grid[kk])) continue;
					/* Update z_min, z_max */
					header->z_min = MIN (header->z_min, (double)grid[kk]);
					header->z_max = MAX (header->z_max, (double)grid[kk]);
				}
				row2++;
			}
			col = 0, row++;
		}
		n_left--;
	}
	if (n_left) {
		GMT_report (C, GMT_MSG_FATAL, "Expected %ld points, found only %ld\n", header->nm, header->nm - n_left);
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fclose (C, fp);
	GMT_free (C, k);
	GMT_free (C, tmp);

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex, GMT_LONG floating)
{
	GMT_LONG i2, j, j2, width_out, height_out, last, inc = 1, off = 0;
	GMT_LONG first_col, last_col, first_row, last_row, kk;
	GMT_LONG i, ij, width_in, *k = NULL;
	char item[GMT_TEXT_LEN], c[2] = {0, 0};
	FILE *fp = NULL;
	
	if (!GMT_IS_ZERO (1.0 - (header->inc[GMT_X] / header->inc[GMT_Y]))) return (GMT_GRDIO_ESRI_NONSQUARE);	/* Only square pixels allowed */
	if (!strcmp (header->name, "="))	/* Write to pipe */
		fp = C->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (C, header->name, C->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	else
		write_esri_info (C, fp, header);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];
	if (complex) { inc = 2; off = complex - 1; }

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Store header information and array */

	i2 = first_col + pad[XLO];
	last = width_out - 1;
	for (j = 0, j2 = first_row + pad[YHI]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		c[0] = '\t';
		for (i = 0; i < width_out; i++) {
			if (i == last) c[0] = '\n';
			kk = inc * (ij+k[i]) + off;
			if (GMT_is_fnan (grid[kk]))
				sprintf (item, "%ld%c", (GMT_LONG)irint (header->nan_value), c[0]);
			else if (floating) {
				sprintf (item, C->current.setting.format_float_out, grid[kk]);
				strcat (item, c);
			}
			else
				sprintf (item, "%ld%c", (GMT_LONG)irint ((double)grid[kk]), c[0]);
			GMT_fputs (item, fp);
		}
	}

	GMT_free (C, k);
	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_writei_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex)
{	/* Standard integer values on output only */
	return (GMT_esri_write_grd (C, header, grid, wesn, pad, complex, FALSE));
}

GMT_LONG GMT_esri_writef_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex)
{	/* Write floating point on output */
	return (GMT_esri_write_grd (C, header, grid, wesn, pad, complex, TRUE));
}
