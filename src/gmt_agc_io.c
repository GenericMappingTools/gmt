/*
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
 * Rewritten from original code by Robert Helie.  That code was hard-wired
 * in two applications (gmt2agcgrd.c and agc2gmtgrd.c) based on GMT 3.4.
 * The following code is modified to fit within the gmt_customio style
 * and argument passing.  Note that AGC files are ASSUMED to be gridline-
 * oriented.  If a pixel grid is requested to be written in AGC format
 * we will shrink the region by 0.5 dx|dy to obtain gridline registration.
 * Finally, AGC uses 0.0 to represent NaNs; I know, not the smartest but
 * it is what it is.  P.Wessel.
 */
/*-----------------------------------------------------------
 * Format # :	21
 * Type :	Atlantic Geoscience Genter (AGC) format
 * Prefix :	GMT_agc_
 * Author :	Paul Wessel, by modifying code from Robert Helie
 * Date :	09-APR-2006
 * Purpose:	To transform to/from AGC grid file format
 * Functions :	gmt_agc_read_grd_info, gmt_agc_write_grd_info,
 *		gmt_agc_write_grd_info, gmt_agc_read_grd, gmt_agc_write_grd
 *-----------------------------------------------------------*/

/* Public Functions:

int gmt_is_agc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
int gmt_agc_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
int gmt_agc_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
int gmt_agc_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
int gmt_agc_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)

*/

# define ZBLOCKWIDTH 	40U
# define ZBLOCKHEIGHT 	40U
# define PREHEADSIZE	12U
# define POSTHEADSIZE	2U
# define HEADINDSIZE	6U
# define BUFFHEADSIZE	(HEADINDSIZE + POSTHEADSIZE)
# define RECORDLENGTH 	(ZBLOCKWIDTH*ZBLOCKHEIGHT + PREHEADSIZE + POSTHEADSIZE)

# define AGCHEADINDICATOR	"agchd:"
# define PARAMSIZE		((GMT_GRID_REMARK_LEN160 - HEADINDSIZE) / BUFFHEADSIZE)

/* Local runctions used by the public functions: */

GMT_LOCAL int agc_read_record (FILE *fpi, gmt_grdfloat z[ZBLOCKWIDTH][ZBLOCKHEIGHT]) {
	/* Reads one block of data, including pre- and post-headers */
	size_t nitems;
	gmt_grdfloat garbage[PREHEADSIZE];

	if (gmt_M_fread (garbage, sizeof(gmt_grdfloat), PREHEADSIZE, fpi) < PREHEADSIZE)
		return (GMT_GRDIO_READ_FAILED);
	nitems = gmt_M_fread (z, sizeof(gmt_grdfloat), ZBLOCKWIDTH * ZBLOCKHEIGHT, fpi);

	if (nitems != ZBLOCKWIDTH * ZBLOCKHEIGHT && !feof(fpi))
		return (GMT_GRDIO_READ_FAILED);	/* Bad stuff */
	if (gmt_M_fread (garbage, sizeof(gmt_grdfloat), POSTHEADSIZE, fpi) < POSTHEADSIZE)
		return (GMT_GRDIO_READ_FAILED);
	return (GMT_NOERROR);
}

GMT_LOCAL int agc_write_record (FILE *file, gmt_grdfloat rec[ZBLOCKWIDTH][ZBLOCKHEIGHT], gmt_grdfloat *prerec, gmt_grdfloat *postrec) {
	/* Writes one block of data, including pre- and post-headers */
	if (gmt_M_fwrite (prerec, sizeof(gmt_grdfloat), PREHEADSIZE, file) < PREHEADSIZE)
		return (GMT_GRDIO_WRITE_FAILED);
	if (gmt_M_fwrite (rec, sizeof(gmt_grdfloat), ZBLOCKWIDTH * ZBLOCKHEIGHT, file) < ZBLOCKWIDTH * ZBLOCKHEIGHT)
		return (GMT_GRDIO_WRITE_FAILED);
	if (gmt_M_fwrite (postrec, sizeof(gmt_grdfloat), POSTHEADSIZE, file) < POSTHEADSIZE)
		return (GMT_GRDIO_WRITE_FAILED);
	return (GMT_NOERROR);
}

GMT_LOCAL void agc_pack_header (gmt_grdfloat *prez, gmt_grdfloat *postz, struct GMT_GRID_HEADER *header) {
	/* Places grd header info in the AGC header array */
	gmt_M_memset (prez,  PREHEADSIZE,  gmt_grdfloat);
	gmt_M_memset (postz, POSTHEADSIZE, gmt_grdfloat);
	prez[0] = (gmt_grdfloat)header->wesn[YLO];
	prez[1] = (gmt_grdfloat)header->wesn[YHI];
	prez[2] = (gmt_grdfloat)header->wesn[XLO];
	prez[3] = (gmt_grdfloat)header->wesn[XHI];
	prez[4] = (gmt_grdfloat)header->inc[GMT_Y];
	prez[5] = (gmt_grdfloat)header->inc[GMT_X];
	prez[PREHEADSIZE-1] = (gmt_grdfloat)RECORDLENGTH;
}

GMT_LOCAL void agc_save_header (char *remark, gmt_grdfloat *agchead) {
	/* Place AGC header data in remark string */
	char floatvalue[PARAMSIZE+1];	/* Allow space for final \0 */
	unsigned int i;
	size_t j;

	strcpy (remark, AGCHEADINDICATOR);
	for (i = 0; i < BUFFHEADSIZE; i++) {
		snprintf (floatvalue, PARAMSIZE, "%f", agchead[i]);
		for (j = strlen (floatvalue); j < PARAMSIZE; j++) strcat (floatvalue, " ");
		strcat (remark, floatvalue);
	}
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

int gmt_is_agc_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is an AGC grid file. */
	FILE *fp = NULL;
	int n_columns, n_rows;
	off_t predicted_size;
	gmt_grdfloat recdata[RECORDLENGTH], x_min, x_max, y_min, y_max, x_inc, y_inc;
	struct stat buf;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);
	if (!strcmp (HH->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);		/* Cannot check on pipes */
	if (stat (HH->name, &buf))
		return (GMT_GRDIO_STAT_FAILED);			/* Inquiry about file failed somehow */
	if ((fp = gmt_fopen (GMT, HH->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);			/* Opening the file failed somehow */
	if (gmt_M_fread (recdata, sizeof(gmt_grdfloat), RECORDLENGTH, fp) < RECORDLENGTH) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}

	y_min = recdata[0];	y_max = recdata[1];
	if (y_min >= y_max) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_BAD_VAL);
	}
	x_min = recdata[2];	x_max = recdata[3];
	if (x_min >= x_max) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_BAD_VAL);
	}
	y_inc = recdata[4];	x_inc = recdata[5];
	if (x_inc <= 0.0 || y_inc <= 0.0) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_BAD_VAL);
	}
	n_columns = (int)gmt_M_get_n (GMT, x_min, x_max, x_inc, 0);
	if (n_columns <= 0) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_BAD_VAL);
	}
	n_rows = (int)gmt_M_get_n (GMT, y_min, y_max, y_inc, 0);
	if (n_rows <= 0) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_BAD_VAL);
	}
	gmt_fclose (GMT, fp);
	/* OK so far; see if file size matches the predicted size given the header info */
	predicted_size = lrint (ceil ((double)n_rows /ZBLOCKHEIGHT) * ceil ((double)n_columns / ZBLOCKWIDTH)) * (ZBLOCKHEIGHT * ZBLOCKWIDTH + PREHEADSIZE + POSTHEADSIZE) * sizeof (gmt_grdfloat);
	if (predicted_size == buf.st_size) {
		/* Yes, appears to be an AGC grid */
		header->type = GMT_GRID_IS_AF;
		header->nan_value = 0.0f; /* NaN value for AGC format */
		HH->orig_datatype = (sizeof (gmt_grdfloat) == sizeof (float)) ? GMT_FLOAT : GMT_DOUBLE;
		return GMT_NOERROR;
	}
	return GMT_GRDIO_BAD_VAL;
}

int gmt_agc_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Read header info. NOTE: All AGC files are assumed to be gridline-registered */
	unsigned int i;
	FILE *fp = NULL;
	gmt_grdfloat recdata[RECORDLENGTH], agchead[BUFFHEADSIZE];
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "=")) {
#ifdef SET_IO_MODE
		gmt_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
	}
	else if ((fp = gmt_fopen (GMT, HH->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (gmt_M_fread (recdata, sizeof(gmt_grdfloat), RECORDLENGTH, fp) < RECORDLENGTH) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}

	header->registration = GMT_GRID_NODE_REG;	/* Hardwired since no info about this in the header */
	HH->orig_datatype = (sizeof (gmt_grdfloat) == sizeof (float)) ? GMT_FLOAT : GMT_DOUBLE;
	header->wesn[XLO]  = recdata[2];
	header->wesn[XHI]  = recdata[3];
	header->wesn[YLO]  = recdata[0];
	header->wesn[YHI]  = recdata[1];
	header->inc[GMT_Y] = recdata[4];
	header->inc[GMT_X] = recdata[5];
	header->n_columns = gmt_M_grd_get_nx (GMT, header);
	header->n_rows = gmt_M_grd_get_ny (GMT, header);
	header->z_scale_factor = 1.0;
	header->z_add_offset = 0.0;
	for (i = 6; i < PREHEADSIZE; i++) agchead[i-6] = recdata[i];
	agchead[BUFFHEADSIZE-2] = recdata[RECORDLENGTH-2];
	agchead[BUFFHEADSIZE-1] = recdata[RECORDLENGTH-1];
	agc_save_header (header->remark, agchead);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_agc_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Write grd header info to file */
	FILE *fp = NULL;
	gmt_grdfloat prez[PREHEADSIZE], postz[POSTHEADSIZE];
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "=")) {
#ifdef SET_IO_MODE
		gmt_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = gmt_fopen (GMT, HH->name, "rb+")) == NULL && (fp = gmt_fopen (GMT, HH->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	agc_pack_header (prez, postz, header);	/* Stuff header info into the AGC arrays */

	if (gmt_M_fwrite (prez, sizeof(gmt_grdfloat), PREHEADSIZE, fp) < PREHEADSIZE) {
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_WRITE_FAILED);
	}

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_agc_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* header:     	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to extract  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the complex array
	 *		for real and imaginary parts when processed by grdfft etc.
	 */

	int first_col, last_col, j, col;		/* First and last column to deal with */
	int first_row, last_row, j_gmt, colend;		/* First and last row to deal with */
	unsigned int width_in;			/* Number of items in one row of the subregion */
	/* width_out is width of row as return (may include padding) */
	unsigned int height_in;			/* Number of columns in subregion */
	unsigned int i;				/* Misc. counters */
	unsigned int *k = NULL;			/* Array with indices */
	unsigned int block, n_blocks, n_blocks_x, n_blocks_y;	/* Misc. counters */
	unsigned int datablockcol, datablockrow, rowstart, rowend, colstart, row;
	uint64_t ij, imag_offset, width_out;
	gmt_grdfloat z[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	FILE *fp = NULL;			/* File pointer to data or pipe */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		gmt_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
	}
	else if ((fp = gmt_fopen (GMT, HH->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	/* Because of the 40x40 blocks we read the entire file and only use what we need */

	/* Rows are read south to north */

	gmt_M_memset (z, ZBLOCKWIDTH * ZBLOCKHEIGHT, gmt_grdfloat); /* Initialize buffer to zero */

	header->z_min = +DBL_MAX;	header->z_max = -DBL_MAX;
	HH->has_NaNs = GMT_GRID_NO_NANS;	/* We are about to check for NaNs and if none are found we retain 1, else 2 */

	n_blocks_y = urint (ceil ((double)header->n_rows / (double)ZBLOCKHEIGHT));
	n_blocks_x = urint (ceil ((double)header->n_columns / (double)ZBLOCKWIDTH));
	n_blocks = n_blocks_x * n_blocks_y;
	datablockcol = datablockrow = 0;
	for (block = 0; block < n_blocks; block++) {
		if (agc_read_record (fp, z)) {
			gmt_M_free (GMT, k);
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_READ_FAILED);
		}
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN (rowstart + ZBLOCKHEIGHT, header->n_rows);
		for (i = 0, row = rowstart; row < rowend; i++, row++) {
			j_gmt = header->n_rows - 1 - row;	/* GMT internal row number */
			if (j_gmt < first_row || j_gmt > last_row) continue;
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN (colstart + ZBLOCKWIDTH, header->n_columns);
			for (j = 0, col = colstart; col < colend; j++, col++) {
				if (col < first_col || col > last_col) continue;
				ij = imag_offset + ((uint64_t)(((j_gmt - first_row) + pad[YHI])) * width_out + col - first_col) + pad[XLO];
				grid[ij] = (z[j][i] == 0.0) ? GMT->session.f_NaN : z[j][i];	/* AGC uses exact zero as NaN flag */
				if (gmt_M_is_fnan (grid[ij])) {
					HH->has_NaNs = GMT_GRID_HAS_NANS;
					continue;
				}
				header->z_min = MIN (header->z_min, (double)grid[ij]);
				header->z_max = MAX (header->z_max, (double)grid[ij]);
			}
		}

		if (++datablockrow >= n_blocks_y) {
			datablockrow = 0;
			datablockcol++;
		}
	}
	gmt_M_free (GMT, k);
	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;

	header->n_columns = width_in;	header->n_rows = height_in;
	gmt_M_memcpy (header->wesn, wesn, 4, double);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_agc_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* header:	grid structure header
	 * grid:	array with final grid
	 * wesn:	Sub-region to write out  [Use entire file if 0,0,0,0]
	 * padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively
	 * complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only)
	 *		Note: The file has only real values, we simply allow space in the complex array
	 *		for real and imaginary parts when processed by grdfft etc.
	 */

	int first_col, last_col, col, colend = 0;		/* First and last column to deal with */
	int j_gmt, i, j, first_row, last_row;		/* First and last row to deal with */
	/* width_in is number of items in one row of the subregion */
	unsigned int width_out;			/* Width of row as return (may include padding) */
	unsigned int height_out;			/* Number of columns in subregion */
	unsigned int i2, j2;			/* Misc. counters */
	unsigned int *k = NULL;			/* Array with indices */
	unsigned int block, n_blocks, n_blocks_x, n_blocks_y;	/* Misc. counters */
	unsigned int row, rowstart, rowend, colstart, datablockcol, datablockrow;
	uint64_t kk, ij, imag_offset, width_in;
	gmt_grdfloat prez[PREHEADSIZE], postz[POSTHEADSIZE];
	gmt_grdfloat outz[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	FILE *fp = NULL;			/* File pointer to data or pipe */
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		gmt_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = gmt_fopen (GMT, HH->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	gmt_M_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		ij = imag_offset + j2 * width_in;
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			kk = ij + i2;
			if (gmt_M_is_fnan (grid[kk]))	/* in AGC, NaN <--> 0.0 */
				grid[ij] = 0.0f;
			else {
				header->z_min = MIN (header->z_min, (double)grid[kk]);
				header->z_max = MAX (header->z_max, (double)grid[kk]);
			}
		}
	}
	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;

	/* Since AGC files are always gridline-registered we must change -R when a pixel grid is to be written */
	if (header->registration == GMT_GRID_PIXEL_REG) {
		gmt_change_grdreg (GMT, header, GMT_GRID_NODE_REG);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "AGC grids are always gridline-registered.  Your pixel-registered grid will be converted.\n");
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "AGC grid region in file %s reset to %g/%g/%g/%g\n", HH->name, header->wesn[XLO], header->wesn[XHI], header->wesn[YLO], header->wesn[YHI]);
	}

	agc_pack_header (prez, postz, header);	/* Stuff header info into the AGC arrays */

	gmt_M_memset (outz, ZBLOCKWIDTH * ZBLOCKHEIGHT, gmt_grdfloat); /* or agc_write_record (fp, outz, prez, postz); points to uninitialised buffer */

	n_blocks_y = urint (ceil ((double)header->n_rows / (double)ZBLOCKHEIGHT));
	n_blocks_x = urint (ceil ((double)header->n_columns / (double)ZBLOCKWIDTH));
	n_blocks = n_blocks_x * n_blocks_y;
	datablockcol = datablockrow = 0;
	for (block = 0; block < n_blocks; block++) {
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN (rowstart + ZBLOCKHEIGHT, header->n_rows);
		for (i = 0, row = rowstart; row < rowend; i++, row++) {
			j_gmt = header->n_rows - 1 - row;	/* GMT internal row number */
			if (j_gmt < first_row || j_gmt > last_row) continue;
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN (colstart + ZBLOCKWIDTH, header->n_columns);
			for (j = 0, col = colstart; col < colend; j++, col++) {
				if (col < first_col || col > last_col) continue;
				ij = imag_offset + ((uint64_t)((j_gmt - first_row) + pad[YHI])) * width_in + (col - first_col) + pad[XLO];
				outz[j][i] = grid[ij];
			}
		}

		if (agc_write_record (fp, outz, prez, postz)) {
			gmt_M_free (GMT, k);
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_WRITE_FAILED);
		}

		if (++datablockrow >= n_blocks_y) {
			datablockrow = 0;
			datablockcol++;
		}
	}
	gmt_M_free (GMT, k);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}
