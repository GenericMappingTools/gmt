/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include "common_byteswap.h"

EXTERN_MSC void gmt_grd_set_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);

int GMT_is_esri_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is an ESRI Interchange ASCII file */
	FILE *fp = NULL;
	char record[GMT_BUFSIZ];

	if (!strcmp (header->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (GMT, header->name, "r")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (fgets (record, GMT_BUFSIZ, fp) == NULL) {	/* Just get first line. Not using GMT_fgets since we may be reading a binary file */
		return (GMT_GRDIO_OPEN_FAILED);
	}
	GMT_fclose (GMT, fp);
	if (strncmp (record, "ncols ", 6) ) {
		/* Failed to find "ncols"; probably a binary file */
		char *file = NULL;
		size_t name_len;

		/* If it got here, see if a companion .hdr file exists (must test upper & lower cases names) */
		file = strdup (header->name);
		GMT_chop_ext (file);
		name_len = strlen (header->name);
		if (name_len < strlen(file) + 4) {
			/* The file extension had less than 3 chars, which means that 1) it's not an esri file.
			   2) would corrupt the heap with the later strcat (file, ".hdr");
			      On Win this would later cause a crash upon freeing 'file' */
			free (file);
			return (-1);
		}
		if (isupper ((unsigned char) header->name[name_len - 1]))
			strcat (file, ".HDR");
		else
			strcat (file, ".hdr");

		if (!GMT_access (GMT, file, F_OK)) {	/* Now, if first line has BYTEORDER or ncols keywords we are in the game */
			if ((fp = GMT_fopen (GMT, file, "r")) == NULL)
				return (GMT_GRDIO_OPEN_FAILED);
			GMT_fgets (GMT, record, GMT_BUFSIZ, fp);	/* Just get first line */
			GMT_fclose (GMT, fp);

			if (!strncmp (record, "BYTEORDER", 4) ) {
				sscanf (record, "%*s %c", &header->flags[0]);	/* Store the endianness flag here */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80);
			}
			else if (!strncmp (record, "ncols ", 6) ) {	/* Ah. A Arc/Info float binary file with a separate .hdr */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80);
				header->flags[0] = 'L';	/* If is truly 'L' or 'B' we'll find only when parsing the whole header */
				header->flags[1] = '2';	/* Flag to let us know the file type */
			}
			else {	/* Cannot do anything with this data */
				free (file);
				return (-1);
			}

			free (file);
		}
		else {
			/* No header file; see if filename contains w/e/s/n information, as in W|ExxxN|Syy.dem
			 * for GTOPO30 (e.g W020N90.DEM) or N|SyyW|Exxx.hgt for SRTM1|3 (e.g. N00E006.hgt)  */
			size_t len;

			while (GMT_chop_ext (file));	/* Remove all extensions so we know exactly where to look */
			len = strlen (file);
			if ((file[len-3] == 'N' || file[len-3] == 'n' || file[len-3] == 'S' || file[len-3] == 's') &&
				(file[len-7] == 'W' || file[len-7] == 'w' || file[len-7] == 'E' || file[len-7] == 'e')) {
				/* It is a GTOPO30 or SRTM30 source file without a .hdr companion. */
				/* see http://dds.cr.usgs.gov/srtm/version1/SRTM30/GTOPO30_Documentation */
				header->flags[0] = 'B';		/* GTOPO30 & SRTM30 are Big Endians */
				header->flags[1] = '0';		/* Flag to let us know the file type */
				/* Store the file name with all extensions removed.
				 * We'll use this to create header from file name info */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80);
				strcpy  (header->remark, "Assumed to be a GTOPO30 or SRTM30 tile");
			}
			else if (name_len > 3 && !(strncmp (&header->name[name_len-4], ".hgt", 4) && strncmp (&header->name[name_len-4], ".HGT", 4))) {
				/* Probably a SRTM1|3 file. In read_esri_info we'll check further if it is a 1 or 3 sec */
				if ((file[len-4] == 'E' || file[len-4] == 'e' || file[len-4] == 'W' || file[len-4] == 'w') &&
					(file[len-7] == 'N' || file[len-7] == 'n' || file[len-7] == 'S' || file[len-7] == 's')) {
					header->flags[0] = 'B';	/* SRTM1|3 are Big Endians */
					header->flags[1] = '1';	/* Flag to let us know the file type */
					/* Store the file name with all extensions removed.
					 * We'll use this to create header from file name info */
					strncpy (header->title, file, GMT_GRID_TITLE_LEN80);
				}
			}
			else {
				/* Cannot do anything with this data */
				free (file);
				return (-1);	/* Not this kind of file */
			}
		}
	}

	header->type = GMT_GRID_IS_EI;
	return GMT_NOERROR;
}

int read_esri_info_hdr (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Parse the contents of a .HDR file */
	int nB;
	char record[GMT_BUFSIZ];
	FILE *fp = NULL;

	if ((fp = GMT_fopen (GMT, header->title, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	header->registration = GMT_GRID_NODE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BYTEORDER */ 
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);		/* LAYOUT */
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->ny) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding NROWS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->nx) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding NCOLS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &nB) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding NBANDS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	if (nB != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Cannot read file with number of Bands != 1 \n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->bits) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding NBITS record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	if ( header->bits != 16 && header->bits != 32 ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: This data type (%d bits) is not supported\n", header->bits);
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BANDROWBYTES  */ 
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);		/* TOTALROWBYTES */
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BANDGAPBYTES  */
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	while (strncmp (record, "NODATA", 4) )		/* Keep reading till find this keyword */
		GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %f", &header->nan_value) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding ULXMAP record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YHI]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding ULYMAP record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding XDIM record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_Y]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding YDIM record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
			
	GMT_fclose (GMT, fp);

	header->wesn[XHI] = header->wesn[XLO] + (header->nx - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YLO] = header->wesn[YHI] - (header->ny - 1 + header->registration) * header->inc[GMT_Y];

	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, header, 1), header->name);

	return (GMT_NOERROR);
}

int read_esri_info (struct GMT_CTRL *GMT, FILE *fp, struct GMT_GRID_HEADER *header)
{
	int c;
	char record[GMT_BUFSIZ];
	FILE *fp2 = NULL, *fpBAK = NULL;

	header->registration = GMT_GRID_NODE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	if (header->flags[0] == 'M' || header->flags[0] == 'I') {	/* We are dealing with a ESRI .hdr file */
		int error;
		if ((error = read_esri_info_hdr (GMT, header))) 		/* Continue the work someplace else */
			return (error);
		else
			return (GMT_NOERROR);
	}
	else if (header->flags[0] == 'B' && header->flags[1] == '0') {	/* A GTOPO30 or SRTM30 file */
		size_t len = strlen (header->title);

		header->inc[GMT_X] = header->inc[GMT_Y] = 30.0 * GMT_SEC2DEG;	/* 30 arc seconds */
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
		header->registration = GMT_GRID_PIXEL_REG;
		GMT_set_geographic (GMT, GMT_IN);
		gmt_grd_set_units (GMT, header);
		
		/* Different sign of NaN value between GTOPO30 and SRTM30 grids */
		if (strstr (header->name, ".DEM") || strstr (header->name, ".dem"))
			header->nan_value = -9999.0f;
		else
			header->nan_value = 9999.0f;
		header->bits = 16;		/* Temp pocket to store number of bits */
		return (GMT_NOERROR);
	}
	else if (header->flags[0] == 'B' && header->flags[1] == '1') {	/* A SRTM3 or SRTM1 file */
		size_t len = strlen (header->title);
		struct stat F;

		header->wesn[XLO] = atof (&header->title[len-3]);
		if ( header->title[len-4] == 'W' || header->title[len-4] == 'W' ) header->wesn[XLO] *= -1; 
		c = header->title[len-4];
		header->title[len-4] = '\0';
		header->wesn[YLO] = atof (&header->title[len-6]);
		header->title[len-4] = (char)c;		/* Reset because this function is called at least twice */
		if ( header->title[len-7] == 'S' || header->title[len-7] == 's' ) header->wesn[YLO] *= -1; 
		header->wesn[YHI] = header->wesn[YLO] + 1; 
		header->wesn[XHI] = header->wesn[XLO] + 1; 
		header->nan_value = -32768.0f;
		header->bits = 16;		/* Temp pocket to store number of bits */
		stat (header->name, &F);	/* Must finally find out if it is a 1 or 3 arcseconds file */
		if (F.st_size < 3e6) {		/* Actually the true size is 2884802 */
			header->inc[GMT_X] = header->inc[GMT_Y] = 3.0 * GMT_SEC2DEG;	/* 3 arc seconds */
			strcpy (header->remark, "Assumed to be a SRTM3 tile");
		}
		else {
			header->inc[GMT_X] = header->inc[GMT_Y] = 1.0 * GMT_SEC2DEG;	/* 1 arc second  */
			strcpy (header->remark, "Assumed to be a SRTM1 tile");
		}
		GMT_set_geographic (GMT, GMT_IN);
		gmt_grd_set_units (GMT, header);
		return (GMT_NOERROR);
	}
	else if ((header->flags[0] == 'L' || header->flags[0] == 'B') && header->flags[1] == '2') {	/* A Arc/Info BINARY file */
		if ((fp2 = GMT_fopen (GMT, header->title, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
		/* To use the same parsing header code as in the ASCII file case where header and data are in the
		   same file, we will swap the file pointers and undo the swap at the end of this function */
		fpBAK = fp;	/* Copy of the input argument '*fp' */
		fp = fp2;
	}

	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->nx) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding ncols record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->ny) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding nrows record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding xll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "xllcorner", 9U)) header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding yll record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	GMT_str_tolower (record);
	if (!strncmp (record, "yllcorner", 9U)) header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
	GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding cellsize record\n");
		return (GMT_GRDIO_READ_FAILED);
	}
	/* Handle the optional nodata_value record */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') {	/*	Assume this is a nodata_value record since we found an 'n|N' */
		GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
		if (sscanf (record, "%*s %f", &header->nan_value) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
			return (GMT_GRDIO_READ_FAILED);
		}
	}
	header->inc[GMT_Y] = header->inc[GMT_X];
	header->wesn[XHI] = header->wesn[XLO] + (header->nx - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YHI] = header->wesn[YLO] + (header->ny - 1 + header->registration) * header->inc[GMT_Y];

	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, header, 1), header->name);

	if (fpBAK) {		/* Case of Arc/Info binary file with a separate header file. We still have things to do. */
		char tmp[16];
		/* Read an extra record containing the endianness info */
		GMT_fgets (GMT, record, GMT_BUFSIZ, fp);
		if (sscanf (record, "%*s %s", tmp) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Arc/Info BINARY Grid: Error decoding endianness record\n");
			return (GMT_GRDIO_READ_FAILED);
		}
		header->flags[0] = (tmp[0] == 'L') ? 'L' : 'B';

		header->bits = 32;	/* Those float binary files */
		/* Ok, now as mentioned above undo the file pointer swapping (point again to data file) */
		fp = fpBAK;
		GMT_fclose (GMT, fp2);
	}

	return (GMT_NOERROR);
}

int GMT_esri_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	int error;
	FILE *fp = NULL;

	if (!strcmp (header->name, "="))	/* Pipe in from stdin */
		fp = GMT->session.std[GMT_IN];
	else if ((fp = GMT_fopen (GMT, header->name, "r")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if ((error = read_esri_info (GMT, fp, header))) return (error);

	GMT_fclose (GMT, fp);
		
	return (GMT_NOERROR);
}

int write_esri_info (struct GMT_CTRL *GMT, FILE *fp, struct GMT_GRID_HEADER *header)
{
	char record[GMT_BUFSIZ] = {""}, item[GMT_LEN64] = {""};

	sprintf (record, "ncols %d\nnrows %d\n", header->nx, header->ny);
	GMT_fputs (record, fp);		/* Write a text record */
	if (header->registration == GMT_GRID_PIXEL_REG) {	/* Pixel format */
		sprintf (record, "xllcorner ");
		sprintf (item, GMT->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcorner ");
		sprintf (item, GMT->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
	}
	else {	/* Gridline format */
		sprintf (record, "xllcenter ");
		sprintf (item, GMT->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcenter ");
		sprintf (item, GMT->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		GMT_fputs (record, fp);		/* Write a text record */
	}
	sprintf (record, "cellsize ");
	sprintf (item, GMT->current.setting.format_float_out, header->inc[GMT_X]);
	strcat  (record, item);	strcat  (record, "\n");
	GMT_fputs (record, fp);		/* Write a text record */
	if (isnan (header->nan_value)) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: ESRI Arc/Info ASCII Interchange file must use proxy for NaN; default to -9999\n");
		header->nan_value = -9999.0f;
	}
	sprintf (record, "nodata_value %ld\n", lrintf (header->nan_value));
	GMT_fputs (record, fp);		/* Write a text record */

	return (GMT_NOERROR);
}

int GMT_esri_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	
	if (!strcmp (header->name, "="))	/* Write to stdout */
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (GMT, header->name, "w")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	
	write_esri_info (GMT, fp, header);

	GMT_fclose (GMT, fp);
	
	return (GMT_NOERROR);
}

int GMT_esri_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{
	int error;
	bool check, is_binary = false, swap = false;
	unsigned int col, height_in, ii, in_nx;
	int row, first_col, last_col, first_row, last_row;
	unsigned int row2, width_in, *actual_col = NULL;
	unsigned int nBits = 32U;
	uint64_t ij, kk, width_out, imag_offset, n_left = 0;
	size_t n_expected;
	char *r_mode = NULL;
	int16_t *tmp16 = NULL;
	float value, *tmp = NULL;
	FILE *fp = NULL;

	if (header->flags[0]) {	/* We are dealing with a ESRI .hdr file or GTOPO30, SRTM30, SRTM1|3 */
		r_mode = "rb";
		if (((header->flags[0] == 'M' || header->flags[0] == 'B') && !GMT_BIGENDIAN) ||
			(header->flags[0] == 'L' && GMT_BIGENDIAN)) 
			swap = true;
		nBits = header->bits;
		is_binary = true;
	}
	else
		r_mode = GMT->current.io.r_mode;

	if (!strcmp (header->name, "="))	/* Read from pipe */
		fp = GMT->session.std[GMT_IN];
	else if ((fp = GMT_fopen (GMT, header->name, r_mode)) != NULL) {
		if ((error = read_esri_info (GMT, fp, header))) return (error);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);
	
	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];
	n_expected = header->nx;

	if (nBits == 32)		/* Either an ascii file or ESRI .HDR with NBITS = 32, in which case we assume it's a file of floats */
		tmp = GMT_memory (GMT, NULL, n_expected, float);
	else
		tmp16 = GMT_memory (GMT, NULL, n_expected, int16_t);

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	header->has_NaNs = GMT_GRID_NO_NANS;	/* We are about to check for NaNs and if none are found we retain 1, else 2 */
	if (is_binary) {
		int ny = header->ny;
		if (last_row - first_row + 1 != ny)		/* We have a sub-region */
			if (fseek (fp, (off_t) (first_row * n_expected * 4UL * nBits / 32UL), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);

		ij = imag_offset + pad[YHI] * width_out + pad[XLO];

		for (row = first_row; row <= last_row; row++, ij += width_out) {
			if (nBits == 32) {		/* Get one row */
				if (GMT_fread (tmp, 4, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
			}
			else {
				if (GMT_fread (tmp16, 2, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
			}
			for (col = 0, kk = ij; col < width_in; col++, kk++) {
				if (nBits == 32) {
					if (swap) {
						/* need to memcpy because casting from float* to uint32_t*
						 * violates strict-aliasing rules. */
						uint32_t u;
						memcpy (&u, &tmp[actual_col[col]], sizeof (uint32_t));
						u = bswap32 (u);
						memcpy (&tmp[actual_col[col]], &u, sizeof (uint32_t));
					}
					grid[kk] = tmp[actual_col[col]];
				}
				else {
					if (swap) {
						uint16_t *p = (uint16_t *)&tmp16[actual_col[col]]; /* here casting the pointer is allowed */
						*p = bswap16 (*p);
					}
					grid[kk] = tmp16[actual_col[col]];
				}
				if (grid[kk] == header->nan_value) {
					header->has_NaNs = GMT_GRID_HAS_NANS;
					grid[kk] = GMT->session.f_NaN;
				}
				else {		 /* Update z_min, z_max */
					header->z_min = MIN (header->z_min, (double)grid[kk]);
					header->z_max = MAX (header->z_max, (double)grid[kk]);
				}
			}
		}

		if (nBits == 16) GMT_free (GMT, tmp16);
	}
	else {		/* ASCII */
		n_left = header->nm;

		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids.
	 	 * NaNs are not allowed; they are represented by a nodata_value instead. */
		col = row = 0;		/* For the entire file */
		row2 = 0;	/* For the inside region */
		check = !isnan (header->nan_value);
		in_nx = header->nx;
		header->nx = width_in;	/* Needed to be set here due to GMT_IJP below */
		while (fscanf (fp, "%f", &value) == 1 && n_left) {	/* We read all values and skip those not inside our w/e/s/n */
			tmp[col] = value;	/* Build up a single input row */
			col++;
			if (col == in_nx) {	/* End of input row */
				if (row >= first_row && row <= last_row) {	/* We want a piece (or all) of this row */
					ij = imag_offset + GMT_IJP (header, row2, 0);	/* First out index for this row */
					for (ii = 0; ii < width_in; ii++) {
						kk = ij + ii;
						grid[kk] = (check && tmp[actual_col[ii]] == header->nan_value) ? GMT->session.f_NaN : tmp[actual_col[ii]];
						if (GMT_is_fnan (grid[kk])) {
							header->has_NaNs = GMT_GRID_HAS_NANS;
							continue;
						}
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

	GMT_fclose (GMT, fp);
	GMT_free (GMT, actual_col);
	if (tmp) GMT_free (GMT, tmp);

	if (n_left) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Expected % "PRIu64 " points, found only % "PRIu64 "\n", header->nm, header->nm - n_left);
		return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	return (GMT_NOERROR);
}

int GMT_esri_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode, int floating)
{
	unsigned int i2, j, j2, width_out, height_out, last;
	int first_col, last_col, first_row, last_row;
	unsigned int i, *actual_col = NULL;
	uint64_t ij, width_in, kk, imag_offset;
	char item[GMT_LEN64], c[2] = {0, 0};
	FILE *fp = NULL;

	if (!doubleAlmostEqual ((header->inc[GMT_X] / header->inc[GMT_Y]), 1.0))
		return (GMT_GRDIO_ESRI_NONSQUARE);	/* Only square pixels allowed */
	if (!strcmp (header->name, "="))	/* Write to pipe */
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = GMT_fopen (GMT, header->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	else
		write_esri_info (GMT, fp, header);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Store header information and array */

	i2 = first_col + pad[XLO];
	last = width_out - 1;
	for (j = 0, j2 = first_row + pad[YHI]; j < height_out; j++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		c[0] = '\t';
		for (i = 0; i < width_out; i++) {
			if (i == last) c[0] = '\n';
			kk = ij+actual_col[i];
			if (GMT_is_fnan (grid[kk]))
				sprintf (item, "%ld%c", lrintf (header->nan_value), c[0]);
			else if (floating) {
				sprintf (item, GMT->current.setting.format_float_out, grid[kk]);
				strcat (item, c);
			}
			else
				sprintf (item, "%ld%c", lrint ((double)grid[kk]), c[0]);
			GMT_fputs (item, fp);
		}
	}

	GMT_free (GMT, actual_col);
	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_esri_writei_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* Standard integer values on output only */
	return (GMT_esri_write_grd (GMT, header, grid, wesn, pad, complex_mode, false));
}

int GMT_esri_writef_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* Write floating point on output */
	return (GMT_esri_write_grd (GMT, header, grid, wesn, pad, complex_mode, true));
}
