/*      $Id: gmt_agc_io.c,v 1.26 2011-03-03 21:02:50 guru Exp $
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 * Rewritten from original code from Robert Helie.  That code was hard-wired
 * in two applications (gmt2agcgrd.c and agc2gmtgrd.c) based on GMT 3.4.
 * The following code is modified to fit within the gmt_customio style
 * and argument passing.  Note that AGC files are assumed to be gridline-
 * oriented.  If a pixel grid is requested to be written in AGC format
 * we will shrink the region by 0.5 dx|dy to obtain gridline registration.
 * Finally, AGC uses 0.0 to represent NaNs.
 */
/*-----------------------------------------------------------
 * Format # :	21
 * Type :	Atlantic Geoscience Genter (AGC) format
 * Prefix :	GMT_agc_
 * Author :	Paul Wessel, by modifying code from Robert Helie
 * Date :	09-APR-2006
 * Purpose:	To transform to/from AGC grid file format
 * Functions :	GMT_agc_read_grd_info, GMT_agc_write_grd_info,
 *		GMT_agc_write_grd_info, GMT_agc_read_grd, GMT_agc_write_grd
 *-----------------------------------------------------------*/

# define ZBLOCKWIDTH 	40
# define ZBLOCKHEIGHT 	40
# define PREHEADSIZE	12
# define POSTHEADSIZE	2
# define HEADINDSIZE	6
# define BUFFHEADSIZE	(HEADINDSIZE + POSTHEADSIZE)
# define RECORDLENGTH 	(ZBLOCKWIDTH*ZBLOCKHEIGHT + PREHEADSIZE + POSTHEADSIZE)

# define AGCHEADINDICATOR	"agchd:"
# define PARAMSIZE		(GMT_LONG)((GRD_REMARK_LEN - HEADINDSIZE) / BUFFHEADSIZE)

GMT_LONG GMT_is_agc_grid (char *file)
{	/* Determine if file is a AGC grid file NOT FINISHED YET!!!! */
	FILE *fp = NULL;
	GMT_LONG nx, ny, predicted_size;
	float recdata[RECORDLENGTH], x_min, x_max, y_min, y_max, x_inc, y_inc;
	struct STAT buf;

	if (!strcmp(file, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if (STAT (file, &buf)) return (GMT_GRDIO_STAT_FAILED);	/* Inquiry about file failed somehow */
	if ((fp = GMT_fopen(file, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
	if (GMT_fread ((void *)recdata, sizeof(float), (size_t)RECORDLENGTH, fp) < RECORDLENGTH) return (GMT_GRDIO_READ_FAILED);
	
	y_min = recdata[0];
	y_max = recdata[1];
	if (y_min >= y_max) return (GMT_GRDIO_BAD_VAL);
	x_min = recdata[2];
	x_max = recdata[3];
	if (x_min >= x_max) return (GMT_GRDIO_BAD_VAL);
	y_inc = recdata[4];
	x_inc = recdata[5];
	if (x_inc <= 0.0 || y_inc <= 0.0) return (GMT_GRDIO_BAD_VAL);
	nx = GMT_get_n (x_min, x_max, x_inc, 0);
	if (nx <= 0) return (GMT_GRDIO_BAD_VAL);
	ny = GMT_get_n (y_min, y_max, y_inc, 0);
	if (ny <= 0) return (GMT_GRDIO_BAD_VAL);
	predicted_size = irint (ceil ((double)ny /ZBLOCKHEIGHT) * ceil ((double)nx / ZBLOCKWIDTH)) * (ZBLOCKHEIGHT * ZBLOCKWIDTH + PREHEADSIZE + POSTHEADSIZE) * sizeof (float);
	if (predicted_size == buf.st_size) return (GMT_grd_format_decoder ("af"));
	return (GMT_GRDIO_BAD_VAL);
}

GMT_LONG GMT_agc_read_grd_info (struct GRD_HEADER *header)
{	/* All AGC files are assumed to be gridline-registered */
	FILE *fp;
	GMT_LONG i;
	float recdata[RECORDLENGTH];
	float agchead[BUFFHEADSIZE];
	void SaveAGCHeader (char *remark, float *agchead);

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (GMT_fread ((void *)recdata, sizeof(float), (size_t)RECORDLENGTH, fp) < RECORDLENGTH) return (GMT_GRDIO_READ_FAILED);
	
	header->node_offset = 0;	/* Hardwired since no info about this in the header */
	header->y_min = recdata[0];
	header->y_max = recdata[1];
	header->x_min = recdata[2];
	header->x_max = recdata[3];
	header->y_inc = recdata[4];
	header->x_inc = recdata[5];
	header->nx = GMT_get_n (header->x_min, header->x_max, header->x_inc, header->node_offset);
	header->ny = GMT_get_n (header->y_min, header->y_max, header->y_inc, header->node_offset);
	header->z_scale_factor = 1.0;
	header->z_add_offset = 0.0;
	for (i = 6; i < PREHEADSIZE; i++) agchead[i-6] = recdata[i];
	agchead[BUFFHEADSIZE-2] = recdata[RECORDLENGTH-2];
	agchead[BUFFHEADSIZE-1] = recdata[RECORDLENGTH-1];
	SaveAGCHeader (header->remark, agchead);
	
	if (fp != GMT_stdin) GMT_fclose (fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_agc_write_grd_info (struct GRD_HEADER *header)
{
	FILE *fp;
	float prez[PREHEADSIZE], postz[POSTHEADSIZE];
	void packAGCheader (float *prez, float *postz, struct GRD_HEADER *header);

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (header->name, "rb+")) == NULL && (fp = fopen (header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	
	packAGCheader (prez, postz, header);	/* Stuff header info into the AGC arrays */

	if (GMT_fwrite ((void *)prez, sizeof(float), (size_t)PREHEADSIZE, fp) < PREHEADSIZE) return (GMT_GRDIO_WRITE_FAILED);

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_agc_read_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex)
{	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG width_in;			/* Number of items in one row of the subregion */
	GMT_LONG width_out;			/* Width of row as return (may include padding) */
	GMT_LONG height_in;			/* Number of columns in subregion */
	GMT_LONG inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	GMT_LONG i, j, j_gmt, i_0_out;	/* Misc. counters */
	GMT_LONG *k;				/* Array with indices */
	GMT_LONG block, n_blocks, n_blocks_x, n_blocks_y;	/* Misc. counters */
	GMT_LONG datablockcol, datablockrow, rowstart, rowend, colstart, colend, row, col;
	GMT_LONG ij;
	FILE *fp;			/* File pointer to data or pipe */
	float z[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	GMT_LONG ReadRecord (FILE *fpi, GMT_LONG recnum, float *z);
	
	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	GMT_err_pass (GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */

	if (complex) {	/* Need twice as much output space since we load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}

	/* Because of the 40x40 blocks we read the entire file and only use what we need */

	/* Rows are read south to north */
	
	header->z_min = +DBL_MAX;
	header->z_max = -DBL_MAX;
	
	n_blocks_y = irint (ceil ((double)header->ny / (double)ZBLOCKHEIGHT));
	n_blocks_x = irint (ceil ((double)header->nx / (double)ZBLOCKWIDTH));
	n_blocks = n_blocks_x * n_blocks_y;
	datablockcol = datablockrow = 0;
	for (block = 0; block < n_blocks; block++) {
		if (ReadRecord(fp, block, (float *)z)) return (GMT_GRDIO_READ_FAILED);
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN(rowstart + ZBLOCKHEIGHT, header->ny);
		for (i = 0, row = rowstart; row < rowend; i++, row++) {
			j_gmt = header->ny - 1 - row;	/* GMT internal row number */
			if (j_gmt < first_row || j_gmt > last_row) continue;
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN(colstart + ZBLOCKWIDTH, header->nx);
			for (j = 0, col = colstart; col < colend; j++, col++) {
				if (col < first_col || col > last_col) continue;
				ij = ((j_gmt - first_row) + pad[3]) * width_out + (col - first_col) + i_0_out;
				grid[ij] = (z[j][i] == 0.0) ? GMT_f_NaN : z[j][i];	/* AGC uses exact zero as NaN flag */
			}
		}

		if (++datablockrow >= n_blocks_y) {
			datablockrow = 0;
			datablockcol++;
		}
	}

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	/* Update z_min, z_maz */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = 0; j < header->ny; j++) {
		for (i = 0; i < header->nx; i++) {
			ij = inc * ((j + pad[3]) * width_out + i + pad[0]);
			if (GMT_is_fnan (grid[ij])) continue;
			header->z_min = MIN (header->z_min, (double)grid[ij]);
			header->z_max = MAX (header->z_max, (double)grid[ij]);
		}
	}
	if (fp != stdin) GMT_fclose (fp);
	
	GMT_free ((void *)k);

	return (GMT_NOERROR);
}

GMT_LONG GMT_agc_write_grd (struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */


	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG width_in;			/* Number of items in one row of the subregion */
	GMT_LONG width_out;			/* Width of row as return (may include padding) */
	GMT_LONG height_out;			/* Number of columns in subregion */
	GMT_LONG inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	GMT_LONG i, j, i2, j2;		/* Misc. counters */
	GMT_LONG *k;				/* Array with indices */
	GMT_LONG block, n_blocks, n_blocks_x, n_blocks_y;	/* Misc. counters */
	GMT_LONG ij;
	FILE *fp;			/* File pointer to data or pipe */
	float outz[ZBLOCKWIDTH][ZBLOCKHEIGHT];
	GMT_LONG rowstart, rowend, colstart, colend = 0, datablockcol, datablockrow;
	GMT_LONG j_gmt, row, col;
	float prez[PREHEADSIZE], postz[POSTHEADSIZE];
	GMT_LONG WriteRecord (FILE *file, float *rec, float *prerec, float *postrec);
	void packAGCheader (float *prez, float *postz, struct GRD_HEADER *header);

	if (!strcmp (header->name, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (header->name, "rb+")) == NULL && (fp = fopen (header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	
	GMT_err_pass (GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	if (complex) inc = 2;

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[3]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[0]; i <= last_col; i++, i2++) {
			ij = (j2 * width_in + i2) * inc;
			if (GMT_is_fnan (grid[ij])) {
				grid[ij] = 0.0;	/* in AGC, NaN <--> 0.0 */
			}
			else {
				header->z_min = MIN (header->z_min, (double)grid[ij]);
				header->z_max = MAX (header->z_max, (double)grid[ij]);
			}
		}
	}
	
	/* Since AGC files are always gridline-registered we must change -R when a pixel grid is to be written */
	if (header->node_offset) {
		header->x_min += 0.5 * header->x_inc;	header->x_max -= 0.5 * header->x_inc;
		header->y_min += 0.5 * header->y_inc;	header->y_max -= 0.5 * header->y_inc;
		header->node_offset = 0;
		if (gmtdefs.verbose) fprintf (stderr, "%s: Warning: AGC grids are always gridline-registered.  Your pixel-registered grid will be converted.\n", GMT_program);
		if (gmtdefs.verbose) fprintf (stderr, "%s: Warning: AGC grid region in file %s reset to %g/%g/%g/%g\n", GMT_program, header->name, header->x_min, header->x_max, header->y_min, header->y_max);
	}
	
	packAGCheader (prez, postz, header);	/* Stuff header info into the AGC arrays */

	n_blocks_y = irint (ceil ((double)header->ny / (double)ZBLOCKHEIGHT));
	n_blocks_x = irint (ceil ((double)header->nx / (double)ZBLOCKWIDTH));
	n_blocks = n_blocks_x * n_blocks_y;
	datablockcol = datablockrow = 0;
	for (block = 0; block < n_blocks; block++) {
		rowstart = datablockrow * ZBLOCKHEIGHT;
		rowend = MIN(rowstart + ZBLOCKHEIGHT, header->ny);
		for (i = 0, row = rowstart; row < rowend; i++, row++) {
			j_gmt = header->ny - 1 - row;	/* GMT internal row number */
			if (j_gmt < first_row || j_gmt > last_row) continue;
			colstart = datablockcol * ZBLOCKWIDTH;
			colend = MIN(colstart + ZBLOCKWIDTH, header->nx);
			for (j = 0, col = colstart; col < colend; j++, col++) {
				if (col < first_col || col > last_col) continue;
				ij = ((j_gmt - first_row) + pad[3]) * width_in + (col - first_col) + pad[0];
				outz[j][i] = grid[ij];
			}
		} 

		if (WriteRecord (fp, (float*)outz, prez, postz)) return (GMT_GRDIO_WRITE_FAILED);

		if (++datablockrow >= n_blocks_y) {
			datablockrow = 0;
			datablockcol++;
		}
	}

	if (fp != GMT_stdout) GMT_fclose (fp);

	GMT_free ((void *)k);

	return (GMT_NOERROR);
}

void packAGCheader (float *prez, float *postz, struct GRD_HEADER *header)
{
	short int i;
	prez[0] = (float)header->y_min;
	prez[1] = (float)header->y_max;
	prez[2] = (float)header->x_min;
	prez[3] = (float)header->x_max;
	prez[4] = (float)header->y_inc;
	prez[5] = (float)header->x_inc;

	for (i = 6; i < PREHEADSIZE; i++) prez[i] = 0.0;
	prez[PREHEADSIZE-1] = (float)RECORDLENGTH;
	postz[0] = postz[1] = 0.0;
}

void SaveAGCHeader (char *remark, float *agchead)
{
	char floatvalue[PARAMSIZE];
	GMT_LONG i, j;

	strcpy(remark, AGCHEADINDICATOR);
	for (i = 0; i < BUFFHEADSIZE; i++) {
		sprintf(floatvalue, "%f", agchead[i]);
		for (j = strlen(floatvalue); j < PARAMSIZE; j++) strcat (floatvalue, " ");
		strcat (remark, floatvalue);
	}
}

GMT_LONG ReadRecord (FILE *fpi, GMT_LONG recnum, float *z)
{ 
	GMT_LONG nitems;
	float garbage[PREHEADSIZE];

	if (GMT_fread ((void *)garbage, sizeof(float), (size_t)PREHEADSIZE, fpi) < (size_t)PREHEADSIZE) return (GMT_GRDIO_READ_FAILED);
	nitems = GMT_fread ((void *)z, sizeof(float), (size_t)(ZBLOCKWIDTH * ZBLOCKHEIGHT), fpi);

	if (nitems != (ZBLOCKWIDTH * ZBLOCKHEIGHT) && !feof(fpi)) return (GMT_GRDIO_READ_FAILED);	/* Bad stuff */
	if (GMT_fread ((void *)garbage, sizeof(float), (size_t)POSTHEADSIZE, fpi) < (size_t)POSTHEADSIZE) return (GMT_GRDIO_READ_FAILED);
	return (GMT_NOERROR);
}

GMT_LONG WriteRecord (FILE *file, float *rec, float *prerec, float *postrec)
{
	if (GMT_fwrite ((void *)prerec, sizeof(float), (size_t)PREHEADSIZE, file) < (size_t)PREHEADSIZE) return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite ((void *)rec, sizeof(float), (size_t)(ZBLOCKWIDTH * ZBLOCKHEIGHT), file) < (size_t)(ZBLOCKWIDTH * ZBLOCKHEIGHT)) return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite( (void *)postrec, sizeof(float), (size_t)POSTHEADSIZE, file) < (size_t)POSTHEADSIZE)  return (GMT_GRDIO_WRITE_FAILED); 
	return (GMT_NOERROR);
}
