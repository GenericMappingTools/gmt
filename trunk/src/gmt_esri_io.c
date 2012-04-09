/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
 /* Contains the read/write functions needed to handle ERSI Arc/Info ASCII Exchange grids.
  * Based on previous code in grd2xyz and xyz2grd.  A few limitations of the format:
  * 1) Grid spacing must be square (dx = dy) since only one cell_size record.
  * 2) NaNs must be stored via a proxy value [Auto-defaults to -9999 if not set].
  *
  * Paul Wessel, June 2010.
  *
  * 3) Read also the so called ESRI .HDR format (a binary raw file plus a companion header)
  * 4) Recognizes GTOPO30 and SRTM30 files from their names and use info coded in the
  *    name to fill the header struct.
  *
  * Joaquim Luis, Mars 2011.
  */

GMT_LONG GMT_is_esri_grid (struct GMT_CTRL *C, struct GRD_HEADER *header)
{	/* Determine if file is an ESRI Interchange ASCII file */
	FILE *fp = NULL;
	char record[GMT_BUFSIZ];

	if (!strcmp (header->name, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (C, header->name, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	fgets (record, GMT_BUFSIZ, fp);	/* Just get first line. Not using GMT_fgets since we may be reading a binary file */ 
	GMT_fclose (C, fp);
	if (strncmp (record, "ncols ", 6) ) {	/* Failed to find "ncols"; probably a binary file */
		char *file = NULL;
		size_t name_len;

		/* If it got here, see if a companion .hdr file exists (must test upper & lower cases names) */
		file = strdup (header->name);
		GMT_chop_ext (file);
		name_len = strlen (header->name);
		if (isupper ((unsigned char) header->name[name_len - 1]))
			strcat (file, ".HDR");
		else
			strcat (file, ".hdr");

		if (!GMT_access (C, file, F_OK)) {	/* Now, if first line has BYTEORDER or ncols keywords we are in the game */
			if ((fp = GMT_fopen (C, file, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
			GMT_fgets (C, record, GMT_BUFSIZ, fp);	/* Just get first line */
			GMT_fclose (C, fp);

			if (!strncmp (record, "BYTEORDER", 4) ) {
				sscanf (record, "%*s %c", &header->flags[0]);	/* Store the endianess flag here */
				strcpy (header->title, file);
			}
			else if (!strncmp (record, "ncols ", 6) ) {	/* Ah. A Arc/Info float binary file with a separate .hdr */
				strcpy (header->title, file);
				header->flags[0] = 'L';	/* If is truly 'L' or 'B' we'll find only when parsing the whole header */
				header->flags[1] = '2';	/* Flag to let us know the file type */
			}
			else {	/* Cannot do anything with this data */
				free (file);
				return (-1);
			}

			free (file);
		}
		else {	/* No header file; see if filename contains w/e/s/n information, as in W|ExxxN|Syy.dem 
			   for GTOPO30 (e.g W020N90.DEM) or N|SyyW|Exxx.hgt for SRTM1|3 (e.g. N00E006.hgt)  */
			size_t len;

			while (GMT_chop_ext (file));	/* Remove all extensions so we know exactly where to look */
			len = strlen (file);
			if ((file[len-3] == 'N' || file[len-3] == 'n' || file[len-3] == 'S' || file[len-3] == 's') &&
				(file[len-7] == 'W' || file[len-7] == 'w' || file[len-7] == 'E' || file[len-7] == 'e')) {
				/* It is a GTOPO30 or SRTM30 source file without a .hdr companion. */
				/* see http://dds.cr.usgs.gov/srtm/version1/SRTM30/GTOPO30_Documentation */
				header->flags[0] = 'B';		/* GTOPO30 & SRTM30 are Big Endians */
				header->flags[1] = '0';		/* Flag to let us know the file type */
				strcpy (header->title, file);		/* Store the file name with all extensions removed.
								   	We'll use this to create header from file name info */
			}
			else if (name_len > 3 && !(strncmp (&header->name[name_len-4], ".hgt", 4) && strncmp (&header->name[name_len-4], ".HGT", 4))) {
				/* Probably a SRTM1|3 file. In read_esri_info we'll check further if it is a 1 or 3 sec */
				if ((file[len-4] == 'E' || file[len-4] == 'e' || file[len-4] == 'W' || file[len-4] == 'w') &&
					(file[len-7] == 'N' || file[len-7] == 'n' || file[len-7] == 'S' || file[len-7] == 's')) {
					header->flags[0] = 'B';	/* SRTM1|3 are Big Endians */
					header->flags[1] = '1';	/* Flag to let us know the file type */
					strcpy (header->title, file);	/* Store the file name with all extensions removed.
								   	We'll use this to create header from file name info */
				}
			}
			else {	/* Cannot do anything with this data */
				free (file);
				return (-1);	/* Not this kind of file */
			}
		}
	}

	header->type = GMT_GRD_IS_EI;
	return (header->type);
}

GMT_LONG read_esri_info_hdr (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	/* Parse the contents of a .HDR file */
	int nB;
	char record[GMT_BUFSIZ];
	FILE *fp = NULL;

	if ((fp = GMT_fopen (C, header->title, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	header->registration = GMT_GRIDLINE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	GMT_fgets (C, record, GMT_BUFSIZ, fp);		/* BYTEORDER */ 
	GMT_fgets (C, record, GMT_BUFSIZ, fp);		/* LAYOUT */
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->ny) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding NROWS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->nx) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding NCOLS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &nB) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding NBANDS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	if (nB != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Cannot read file with number of Bands != 1 \n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %" GMT_LL "d", &header->bits) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding NBITS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	if ( header->bits != 16 && header->bits != 32 ) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: This data type (%ld bits) is not supported\n", header->bits);
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);		/* BANDROWBYTES  */ 
	GMT_fgets (C, record, GMT_BUFSIZ, fp);		/* TOTALROWBYTES */
	GMT_fgets (C, record, GMT_BUFSIZ, fp);		/* BANDGAPBYTES  */
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->nan_value) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding ULXMAP record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YHI]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding ULYMAP record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding XDIM record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_Y]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding YDIM record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
			
	GMT_fclose (C, fp);

	header->wesn[XHI] = header->wesn[XLO] + (header->nx - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YLO] = header->wesn[YHI] - (header->ny - 1 + header->registration) * header->inc[GMT_Y];

	GMT_err_fail (C, GMT_grd_RI_verify (C, header, 1), header->name);

	return (GMT_NOERROR);
}

GMT_LONG read_esri_info (struct GMT_CTRL *C, FILE *fp, struct GRD_HEADER *header)
{
	int c;
	char record[GMT_BUFSIZ];
	FILE *fp2 = NULL, *fpBAK = NULL;

	header->registration = GMT_GRIDLINE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	if (header->flags[0] == 'M' || header->flags[0] == 'I') {	/* We are dealing with a ESRI .hdr file */
		GMT_LONG error;
		if ((error = read_esri_info_hdr (C, header))) 		/* Continue the work someplace else */
			return (error);
		else
			return (GMT_NOERROR);
	}
	else if (header->flags[0] == 'B' && header->flags[1] == '0') {	/* A GTOPO30 or SRTM30 file */
		size_t len = strlen (header->title);
		double inc2;

		header->inc[GMT_X] = header->inc[GMT_Y] = 30.0 * GMT_SEC2DEG;	/* 30 arc seconds */
		inc2 = header->inc[GMT_X] / 2.0;
		header->wesn[YHI] = atof (&header->title[len-2]);
		if ( header->title[len-3] == 'S' || header->title[len-3] == 's' ) header->wesn[YHI] *= -1; 
		c = header->title[len-3];
		header->title[len-3] = '\0';
		header->wesn[XLO] = atof (&header->title[len-6]);
		header->title[len-3] = (char)c;		/* Reset because this function is called at least twice */
		if ( header->title[len-7] == 'W' || header->title[len-7] == 'w' ) header->wesn[XLO] *= -1; 
		if (header->wesn[YHI] > -60) {
			header->wesn[YLO] = header->wesn[YHI] - 50; 
			header->wesn[XHI] = header->wesn[XLO] + 40; 
			header->nx = 4800;
			header->ny = 6000;
		}
		else {	/* Antarctica tiles cover 30 degrees of latitude and 60 degrees of longitude each have 3,600 rows and 7,200 columns */
			header->wesn[YLO] = -90; 
			header->wesn[XHI] = header->wesn[XLO] + 60; 
			header->nx = 7200;
			header->ny = 3600;
		}
		header->registration = GMT_PIXEL_REG;
		
		/* Different sign of NaN value between GTOPO30 and SRTM30 grids */
		if (strstr (header->name, ".DEM") || strstr (header->name, ".dem"))
			header->nan_value = -9999;
		else
			header->nan_value = 9999;
		header->bits = 16;		/* Temp pocket to store number of bits */
		if (!GMT_is_geographic (C, GMT_IN)) GMT_parse_common_options (C, "f", 'f', "g"); /* Implicitly set -fg unless already set */
		return (GMT_NOERROR);
	}
	else if (header->flags[0] == 'B' && header->flags[1] == '1') {	/* A SRTM3 or SRTM1 file */
		size_t len = strlen (header->title);
		struct GMT_STAT F;

		header->wesn[XLO] = atof (&header->title[len-3]);
		if ( header->title[len-4] == 'W' || header->title[len-4] == 'W' ) header->wesn[XLO] *= -1; 
		c = header->title[len-4];
		header->title[len-4] = '\0';
		header->wesn[YLO] = atof (&header->title[len-6]);
		header->title[len-4] = (char)c;		/* Reset because this function is called at least twice */
		if ( header->title[len-7] == 'S' || header->title[len-7] == 's' ) header->wesn[YLO] *= -1; 
		header->wesn[YHI] = header->wesn[YLO] + 1; 
		header->wesn[XHI] = header->wesn[XLO] + 1; 
		header->nan_value = -32768;
		header->bits = 16;		/* Temp pocket to store number of bits */
		GMT_STAT (header->name, &F);	/* Must finally find out if it is a 1 or 3 arcseconds file */
		if (F.st_size < 3e6)		/* Actually the true size is 2884802 */
			header->inc[GMT_X] = header->inc[GMT_Y] = 3.0 * GMT_SEC2DEG;	/* 3 arc seconds */
		else
			header->inc[GMT_X] = header->inc[GMT_Y] = 1.0 * GMT_SEC2DEG;	/* 1 arc second  */
		if (!GMT_is_geographic (C, GMT_IN)) GMT_parse_common_options (C, "f", 'f', "g"); /* Implicitly set -fg unless already set */
		return (GMT_NOERROR);
	}
	else if ((header->flags[0] == 'L' || header->flags[0] == 'B') && header->flags[1] == '2') {	/* A Arc/Info BINARY file */
		if ((fp2 = GMT_fopen (C, header->title, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
		/* To use the same parsing header code as in the ASCII file case where header and data are in the
		   same file, we will swap the file pointers and undo the swap at the end of this function */
		fpBAK = fp;	/* Copy of the input argument '*fp' */
		fp = fp2;
	}

	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->nx) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding ncols record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->ny) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding nrows record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding xll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "xllcorner", (size_t)9)) header->registration = GMT_PIXEL_REG;	/* Pixel grid */
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YLO]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding yll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "yllcorner", (size_t)9)) header->registration = GMT_PIXEL_REG;	/* Pixel grid */
	GMT_fgets (C, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding cellsize record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	/* Handle the optional nodata_value record */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') {	/*	Assume this is a nodata_value record since we found an 'n|N' */
		GMT_fgets (C, record, GMT_BUFSIZ, fp);
		if (sscanf (record, "%*s %lf", &header->nan_value) != 1) {
			GMT_report (C, GMT_MSG_FATAL, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
			return (GMT_GRDIO_READ_FAILED);
		}
	}
	header->inc[GMT_Y] = header->inc[GMT_X];
	header->wesn[XHI] = header->wesn[XLO] + (header->nx - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YHI] = header->wesn[YLO] + (header->ny - 1 + header->registration) * header->inc[GMT_Y];

	GMT_err_fail (C, GMT_grd_RI_verify (C, header, 1), header->name);

	if (fpBAK) {		/* Case of Arc/Info binary file with a separate header file. We still have things to do. */
		char tmp[16];
		/* Read an extra record containing the endianess info */
		GMT_fgets (C, record, GMT_BUFSIZ, fp);
		if (sscanf (record, "%*s %s", tmp) != 1) {
			GMT_report (C, GMT_MSG_FATAL, "Arc/Info BINARY Grid: Error decoding endianess record\n");
			return (GMT_GRDIO_READ_FAILED);
		}
		header->flags[0] = (tmp[0] == 'L') ? 'L' : 'B';

		header->bits = 32;	/* Those float binary files */
		/* Ok, now as mentioned above undo the file pointer swapping (point again to data file) */
		fp = fpBAK;
		GMT_fclose (C, fp2);
	}

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
	char record[GMT_BUFSIZ], item[GMT_TEXT_LEN64];

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
	if (GMT_is_dnan (header->nan_value)) {
		GMT_report (C, GMT_MSG_NORMAL, "Warning: ESRI Arc/Info ASCII Interchange file must use proxy for NaN; default to -9999\n");
		header->nan_value = -9999.0;
	}
	sprintf (record, "nodata_value %ld\n", lrint (header->nan_value));
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

GMT_LONG GMT_esri_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG pad[], GMT_LONG complex_mode)
{
	GMT_LONG col, width_out, height_in, ii, kk, in_nx, inc, off;
	GMT_LONG first_col, last_col, first_row, last_row, n_left = 0;
	GMT_LONG row, row2, col2, ij, width_in, check, error, *k = NULL;
	GMT_LONG nBits = 32, i_0_out, is_binary = FALSE, swap = FALSE;
	char *r_mode = NULL;
	short int *tmp16 = NULL;
	unsigned int *ui = NULL;
	unsigned short *us = NULL;
	float value, *tmp = NULL;
	FILE *fp = NULL;

	if (header->flags[0]) {	/* We are dealing with a ESRI .hdr file or GTOPO30, SRTM30, SRTM1|3 */
		r_mode = "rb";
		if (((header->flags[0] == 'M' || header->flags[0] == 'B') && !GMT_BIGENDIAN) ||
			(header->flags[0] == 'L' && GMT_BIGENDIAN)) 
			swap = TRUE;
		nBits = header->bits;
		is_binary = TRUE;
	}
	else
		r_mode = C->current.io.r_mode;

	if (!strcmp (header->name, "="))	/* Read from pipe */
		fp = C->session.std[GMT_IN];
	else if ((fp = GMT_fopen (C, header->name, r_mode)) != NULL) {
		if ((error = read_esri_info (C, fp, header))) return (error);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);
	
	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];
	width_out *= inc;		/* Possibly twice if complex is TRUE */

	if (nBits == 32)		/* Either an ascii file or ESRI .HDR with NBITS = 32, in which case we assume it's a file of floats */
		tmp = GMT_memory (C, NULL, header->nx, float);
	else
		tmp16 = GMT_memory (C, NULL, header->nx, short int);

	if (is_binary) {

		if (last_row - first_row + 1 != header->ny)		/* We have a sub-region */
			if (GMT_fseek (fp, (long) (first_row * header->nx * 4 * nBits / 32), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);

		i_0_out = inc * pad[XLO] + off;		/* Edge offset in output */
		ij = pad[YHI] * width_out + i_0_out;

		for (row = first_row; row <= last_row; row++, ij += width_out) {
			if (nBits == 32) {		/* Get one row */
				if (GMT_fread (tmp, 4, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
			}
			else {
				if (GMT_fread (tmp16, 2, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
			}
			for (col = 0, kk = ij; col < width_in; col++, kk+=inc) {
				if (nBits == 32) {
					if (swap) {
						ui = (unsigned int *)&tmp[k[col]];	/* These 2 lines do the swap */
						*ui = GMT_swab4 (*ui);
					}
					grid[kk] = tmp[k[col]];
				}
				else {
					if (swap) {
						us = (unsigned short *)&tmp16[k[col]];	/* These 2 lines do the swap */
						*us = GMT_swab2 (*us);
					}
					grid[kk] = tmp16[k[col]];
				}
				if (grid[kk] == header->nan_value) 
					grid[kk] = C->session.f_NaN;
				else {		 /* Update z_min, z_max */
					header->z_min = MIN (header->z_min, (double)grid[kk]);
					header->z_max = MAX (header->z_max, (double)grid[kk]);
				}
			}
		}

		if (nBits == 16) GMT_free (C, tmp16);
	}
	else {		/* ASCII */
		n_left = header->nm;

		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids.
	 	 * NaNs are not allowed; they are represented by a nodata_value instead. */
		col = row = 0;		/* For the entire file */
		col2 = row2 = 0;	/* For the inside region */
		check = !GMT_is_dnan (header->nan_value);
		in_nx = header->nx;
		header->nx = (int)width_in;	/* Needed to be set here due to GMT_IJP below */
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
	}

	GMT_fclose (C, fp);
	GMT_free (C, k);
	GMT_free (C, tmp);

	if (n_left) {
		GMT_report (C, GMT_MSG_FATAL, "Expected %ld points, found only %ld\n", header->nm, header->nm - n_left);
		return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode, GMT_LONG floating)
{
	GMT_LONG i2, j, j2, width_out, height_out, last, inc, off;
	GMT_LONG first_col, last_col, first_row, last_row, kk;
	GMT_LONG i, ij, width_in, *k = NULL;
	char item[GMT_TEXT_LEN64], c[2] = {0, 0};
	FILE *fp = NULL;

	if (!doubleAlmostEqual ((header->inc[GMT_X] / header->inc[GMT_Y]), 1.0))
		return (GMT_GRDIO_ESRI_NONSQUARE);	/* Only square pixels allowed */
	if (!strcmp (header->name, "="))	/* Write to pipe */
		fp = C->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (C, header->name, C->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	else
		write_esri_info (C, fp, header);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

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
				sprintf (item, "%ld%c", lrint (header->nan_value), c[0]);
			else if (floating) {
				sprintf (item, C->current.setting.format_float_out, grid[kk]);
				strcat (item, c);
			}
			else
				sprintf (item, "%ld%c", lrint ((double)grid[kk]), c[0]);
			GMT_fputs (item, fp);
		}
	}

	GMT_free (C, k);
	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_esri_writei_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* Standard integer values on output only */
	return (GMT_esri_write_grd (C, header, grid, wesn, pad, complex_mode, FALSE));
}

GMT_LONG GMT_esri_writef_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* Write floating point on output */
	return (GMT_esri_write_grd (C, header, grid, wesn, pad, complex_mode, TRUE));
}
