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

/* Private function visible only in this file */

GMT_LOCAL int esri_write_info (struct GMT_CTRL *GMT, FILE *fp, struct GMT_GRID_HEADER *header) {
	/* Note: fp is an open file pointer passed in; it will be closed upstream and not here */
	char record[GMT_BUFSIZ] = {""}, item[GMT_LEN64] = {""};

	snprintf (record, GMT_BUFSIZ, "ncols %d\nnrows %d\n", header->n_columns, header->n_rows);
	gmt_M_fputs (record, fp);		/* Write a text record */
	if (header->registration == GMT_GRID_PIXEL_REG) {	/* Pixel format */
		sprintf (record, "xllcorner ");
		snprintf (item, GMT_LEN64, GMT->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		gmt_M_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcorner ");
		snprintf (item, GMT_LEN64, GMT->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		gmt_M_fputs (record, fp);		/* Write a text record */
	}
	else {	/* Gridline format */
		sprintf (record, "xllcenter ");
		snprintf (item, GMT_LEN64, GMT->current.setting.format_float_out, header->wesn[XLO]);
		strcat  (record, item);	strcat  (record, "\n");
		gmt_M_fputs (record, fp);		/* Write a text record */
		sprintf (record, "yllcenter ");
		snprintf (item, GMT_LEN64, GMT->current.setting.format_float_out, header->wesn[YLO]);
		strcat  (record, item);	strcat  (record, "\n");
		gmt_M_fputs (record, fp);		/* Write a text record */
	}
	sprintf (record, "cellsize ");
	snprintf (item, GMT_LEN64, GMT->current.setting.format_float_out, header->inc[GMT_X]);
	strcat  (record, item);	strcat  (record, "\n");
	gmt_M_fputs (record, fp);		/* Write a text record */
	if (isnan (header->nan_value)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "ESRI Arc/Info ASCII Interchange file must use proxy for NaN; default to -9999\n");
		header->nan_value = -9999.0f;
	}
	snprintf (record, GMT_BUFSIZ, "nodata_value %ld\n", lrintf (header->nan_value));
	gmt_M_fputs (record, fp);		/* Write a text record */

	return (GMT_NOERROR);
}

GMT_LOCAL int esri_read_info_hdr (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Parse the contents of a .HDR file */
	int nB;
	char record[GMT_BUFSIZ];
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if ((fp = gmt_fopen (GMT, header->title, "r")) == NULL) return (GMT_GRDIO_OPEN_FAILED);

	header->registration = GMT_GRID_NODE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BYTEORDER */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);		/* LAYOUT */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->n_rows) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding NROWS record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->n_columns) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding NCOLS record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &nB) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding NBANDS record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	if (nB != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Cannot read file with number of Bands != 1 \n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->bits) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding NBITS record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	if ( header->bits != 16 && header->bits != 32 ) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: This data type (%d bits) is not supported\n", header->bits);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BANDROWBYTES  */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);		/* TOTALROWBYTES */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);		/* BANDGAPBYTES  */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	while (strncmp (record, "NODATA", 6) )		/* Keep reading till find this keyword */
		gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
#ifdef DOUBLE_PRECISION_GRID
	if (sscanf (record, "%*s %lf", &header->nan_value) != 1) {
#else
	if (sscanf (record, "%*s %f", &header->nan_value) != 1) {
#endif
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding ULXMAP record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YHI]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding ULYMAP record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding XDIM record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_Y]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding YDIM record\n");
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}

	gmt_fclose (GMT, fp);
	HH->orig_datatype = (header->bits == 16) ? GMT_SHORT : GMT_INT;

	header->wesn[XHI] = header->wesn[XLO] + (header->n_columns - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YLO] = header->wesn[YHI] - (header->n_rows - 1 + header->registration) * header->inc[GMT_Y];

	gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, header, 1), HH->name);

	return (GMT_NOERROR);
}

GMT_LOCAL int esri_read_info (struct GMT_CTRL *GMT, FILE *fp, struct GMT_GRID_HEADER *header) {
	/* Note: fp is an open file pointer passed in; it will be closed upstream and not here */
	int c;
	char record[GMT_BUFSIZ];
	FILE *fp2 = NULL, *fpBAK = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	header->registration = GMT_GRID_NODE_REG;
	header->z_scale_factor = 1.0;
	header->z_add_offset   = 0.0;

	if (HH->flags[0] == 'M' || HH->flags[0] == 'I') {	/* We are dealing with a ESRI .hdr file */
		int error;
		if ((error = esri_read_info_hdr (GMT, header)) != 0) 		/* Continue the work someplace else */
			return (error);
		else
			return (GMT_NOERROR);
	}
	else if (HH->flags[0] == 'B' && HH->flags[1] == '0') {	/* A GTOPO30 or SRTM30 file */
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
			header->n_columns = 4800;
			header->n_rows = 6000;
		}
		else {	/* Antarctica tiles cover 30 degrees of latitude and 60 degrees of longitude each have 3,600 rows and 7,200 columns */
			header->wesn[YLO] = -90;
			header->wesn[XHI] = header->wesn[XLO] + 60;
			header->n_columns = 7200;
			header->n_rows = 3600;
		}
		header->registration = GMT_GRID_PIXEL_REG;
		gmt_set_geographic (GMT, GMT_IN);
		gmtlib_grd_set_units (GMT, header);

		/* Different sign of NaN value between GTOPO30 and SRTM30 grids */
		if (strstr (HH->name, ".DEM") || strstr (HH->name, ".dem"))
			header->nan_value = -9999.0f;
		else
			header->nan_value = 9999.0f;
		header->bits = 16;		/* Temp pocket to store number of bits */
		HH->orig_datatype = GMT_SHORT;
		return (GMT_NOERROR);
	}
	else if (HH->flags[0] == 'B' && HH->flags[1] == '1') {	/* A SRTM3 or SRTM1 file */
		size_t len = strlen (header->title);
		struct stat F;

		header->wesn[XLO] = atof (&header->title[len-3]);
		if ( header->title[len-4] == 'W' || header->title[len-4] == 'w' ) header->wesn[XLO] *= -1;
		c = header->title[len-4];
		header->title[len-4] = '\0';
		header->wesn[YLO] = atof (&header->title[len-6]);
		header->title[len-4] = (char)c;		/* Reset because this function is called at least twice */
		if ( header->title[len-7] == 'S' || header->title[len-7] == 's' ) header->wesn[YLO] *= -1;
		header->wesn[YHI] = header->wesn[YLO] + 1;
		header->wesn[XHI] = header->wesn[XLO] + 1;
		header->nan_value = -32768.0f;
		header->bits = 16;		/* Temp pocket to store number of bits */
		HH->orig_datatype = GMT_SHORT;
		if (stat (HH->name, &F))	/* Must finally find out if it is a 1 or 3 arcseconds file */
			return (GMT_GRDIO_STAT_FAILED);			/* Inquiry about file failed somehow */
		if (F.st_size < 3e6) {		/* Actually the true size is 2884802 */
			header->inc[GMT_X] = header->inc[GMT_Y] = 3.0 * GMT_SEC2DEG;	/* 3 arc seconds */
			strcpy (header->remark, "Assumed to be a SRTM3 tile");
		}
		else {
			header->inc[GMT_X] = header->inc[GMT_Y] = 1.0 * GMT_SEC2DEG;	/* 1 arc second  */
			strcpy (header->remark, "Assumed to be a SRTM1 tile");
		}
		gmt_set_geographic (GMT, GMT_IN);
		gmtlib_grd_set_units (GMT, header);
		return (GMT_NOERROR);
	}
	else if ((HH->flags[0] == 'L' || HH->flags[0] == 'B') && HH->flags[1] == '2') {	/* A Arc/Info BINARY file */
		if ((fp2 = gmt_fopen (GMT, header->title, "r")) == NULL) {
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_OPEN_FAILED);
		}
		/* To use the same parsing header code as in the ASCII file case where header and data are in the
		   same file, we will swap the file pointers and undo the swap at the end of this function */
		fpBAK = fp;	/* Copy of the input argument '*fp' */
		fp = fp2;
	}

	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->n_columns) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding ncols record\n");
		if (fpBAK) gmt_fclose (GMT, fp2);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %d", &header->n_rows) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding nrows record\n");
		if (fpBAK) gmt_fclose (GMT, fp2);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[XLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding xll record\n");
		if (fpBAK) gmt_fclose (GMT, fp2);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_str_tolower (record);
	if (!strncmp (record, "xllcorner", 9U)) header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->wesn[YLO]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding yll record\n");
		if (fpBAK) gmt_fclose (GMT, fp2);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	gmt_str_tolower (record);
	if (!strncmp (record, "yllcorner", 9U)) header->registration = GMT_GRID_PIXEL_REG;	/* Pixel grid */
	gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
	if (sscanf (record, "%*s %lf", &header->inc[GMT_X]) != 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding cellsize record\n");
		if (fpBAK) gmt_fclose (GMT, fp2);
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_READ_FAILED);
	}
	/* Handle the optional nodata_value record */
	c = fgetc (fp);	/* Get first char of next line... */
	ungetc (c, fp);	/* ...and put it back where it came from */
	if (c == 'n' || c == 'N') {	/*	Assume this is a nodata_value record since we found an 'n|N' */
		gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
#ifdef DOUBLE_PRECISION_GRID
		if (sscanf (record, "%*s %lf", &header->nan_value) != 1) {
#else
		if (sscanf (record, "%*s %f", &header->nan_value) != 1) {
#endif
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info ASCII Grid: Error decoding nan_value_value record\n");
			if (fpBAK) gmt_fclose (GMT, fp2);
			gmt_fclose (GMT, fp);
			return (GMT_GRDIO_READ_FAILED);
		}
	}
	header->inc[GMT_Y] = header->inc[GMT_X];
	header->wesn[XHI] = header->wesn[XLO] + (header->n_columns - 1 + header->registration) * header->inc[GMT_X];
	header->wesn[YHI] = header->wesn[YLO] + (header->n_rows - 1 + header->registration) * header->inc[GMT_Y];

	gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, header, 1), HH->name);

	if (fpBAK) {		/* Case of Arc/Info binary file with a separate header file. We still have things to do. */
		char tmp[16];
		/* Read an extra record containing the endianness info */
		gmt_fgets (GMT, record, GMT_BUFSIZ, fp);
		if (sscanf (record, "%*s %s", tmp) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Arc/Info BINARY Grid: Error decoding endianness record\n");
			gmt_fclose (GMT, fp2);
			return (GMT_GRDIO_READ_FAILED);
		}
		HH->flags[0] = (tmp[0] == 'L') ? 'L' : 'B';

		header->bits = 32;	/* Those float binary files */
		HH->orig_datatype = GMT_FLOAT;
		/* Ok, now as mentioned above undo the file pointer swapping (point again to data file) */
		gmt_fclose (GMT, fp);
		fp = fpBAK;
		gmt_fclose (GMT, fp2);
	}

	if (fp2) gmt_fclose(GMT, fp2);
	return (GMT_NOERROR);
}

int gmt_is_esri_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is an ESRI Interchange ASCII file */
	FILE *fp = NULL;
	char record[GMT_BUFSIZ];
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = gmt_fopen (GMT, HH->name, "r")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (fgets (record, GMT_BUFSIZ, fp) == NULL) {	/* Just get first line. Not using gmt_fgets since we may be reading a binary file */
		gmt_fclose (GMT, fp);
		return (GMT_GRDIO_OPEN_FAILED);
	}
	gmt_fclose (GMT, fp);
	if (strncmp (record, "ncols ", 6) ) {
		/* Failed to find "ncols"; probably a binary file */
		char *file = NULL;
		size_t name_len;

		HH->orig_datatype = GMT_SHORT;	/* May be overridden below */
		/* If it got here, see if a companion .hdr file exists (must test upper & lower cases names) */
		file = strdup (HH->name);
		gmt_chop_ext (file);
		name_len = strlen (HH->name);
		if (name_len < strlen(file) + 4) {
			/* The file extension had less than 3 chars, which means that 1) it's not an esri file.
			   2) would corrupt the heap with the later strcat (file, ".hdr");
			      On Win this would later cause a crash upon freeing 'file' */
			gmt_M_str_free (file);
			return (-1);
		}
		if (isupper ((unsigned char) HH->name[name_len - 1]))
			strcat (file, ".HDR");
		else
			strcat (file, ".hdr");

		if (!gmt_access (GMT, file, F_OK)) {	/* Now, if first line has BYTEORDER or ncols keywords we are in the game */
			if ((fp = gmt_fopen (GMT, file, "r")) == NULL)
				return (GMT_GRDIO_OPEN_FAILED);
			gmt_fgets (GMT, record, GMT_BUFSIZ, fp);	/* Just get first line */
			gmt_fclose (GMT, fp);

			if (!strncmp (record, "BYTEORDER", 9) ) {
				sscanf (record, "%*s %c", &HH->flags[0]);	/* Store the endianness flag here */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80-1);
			}
			else if (!strncmp (record, "ncols ", 6) ) {	/* Ah. A Arc/Info float binary file with a separate .hdr */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80-1);
				HH->flags[0] = 'L';	/* If is truly 'L' or 'B' we'll find only when parsing the whole header */
				HH->flags[1] = '2';	/* Flag to let us know the file type */
				HH->orig_datatype = GMT_FLOAT;
			}
			else {	/* Cannot do anything with this data */
				gmt_M_str_free (file);
				return (-1);
			}

			gmt_M_str_free (file);
		}
		else {
			/* No header file; see if filename contains w/e/s/n information, as in W|ExxxN|Syy.dem
			 * for GTOPO30 (e.g W020N90.DEM) or N|SyyW|Exxx.hgt for SRTM1|3 (e.g. N00E006.hgt)  */
			size_t len;

			while (gmt_chop_ext (file));	/* Remove all extensions so we know exactly where to look */
			len = strlen (file);
			if ((file[len-3] == 'N' || file[len-3] == 'n' || file[len-3] == 'S' || file[len-3] == 's') &&
				(file[len-7] == 'W' || file[len-7] == 'w' || file[len-7] == 'E' || file[len-7] == 'e')) {
				/* It is a GTOPO30 or SRTM30 source file without a .hdr companion. */
				/* see http://dds.cr.usgs.gov/srtm/version1/SRTM30/GTOPO30_Documentation */
				HH->flags[0] = 'B';		/* GTOPO30 & SRTM30 are Big Endians */
				HH->flags[1] = '0';		/* Flag to let us know the file type */
				/* Store the file name with all extensions removed.
				 * We'll use this to create header from file name info */
				strncpy (header->title, file, GMT_GRID_TITLE_LEN80-1);
				strcpy  (header->remark, "Assumed to be a GTOPO30 or SRTM30 tile");
				HH->orig_datatype = GMT_SHORT;
			}
			else if (name_len > 3 && !(strncmp (&HH->name[name_len-4], ".hgt", 4) && strncmp (&HH->name[name_len-4], ".HGT", 4))) {
				/* Probably a SRTM1|3 file. In esri_read_info we'll check further if it is a 1 or 3 sec */
				if ((file[len-4] == 'E' || file[len-4] == 'e' || file[len-4] == 'W' || file[len-4] == 'w') &&
					(file[len-7] == 'N' || file[len-7] == 'n' || file[len-7] == 'S' || file[len-7] == 's')) {
					HH->flags[0] = 'B';	/* SRTM1|3 are Big Endians */
					HH->flags[1] = '1';	/* Flag to let us know the file type */
					/* Store the file name with all extensions removed.
					 * We'll use this to create header from file name info */
					strncpy (header->title, file, GMT_GRID_TITLE_LEN80-1);
					HH->orig_datatype = GMT_SHORT;
				}
			}
			else {
				/* Cannot do anything with this data */
				gmt_M_str_free (file);
				return (-1);	/* Not this kind of file */
			}
		}
	}

	header->type = GMT_GRID_IS_EI;
	return GMT_NOERROR;
}

int gmt_esri_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	int error;
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))	/* Pipe in from stdin */
		fp = GMT->session.std[GMT_IN];
	else if ((fp = gmt_fopen (GMT, HH->name, "r")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if ((error = esri_read_info (GMT, fp, header)) != 0) {
		gmt_fclose (GMT, fp);
		return (error);
	}

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_esri_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (!strcmp (HH->name, "="))	/* Write to stdout */
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = gmt_fopen (GMT, HH->name, "w")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	esri_write_info (GMT, fp, header);

	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_esri_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
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
	gmt_grdfloat value, *tmp = NULL;
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (HH->flags[0]) {	/* We are dealing with a ESRI .hdr file or GTOPO30, SRTM30, SRTM1|3 */
		r_mode = "rb";
		if (((HH->flags[0] == 'M' || HH->flags[0] == 'B') && !GMT_BIGENDIAN) ||
			(HH->flags[0] == 'L' && GMT_BIGENDIAN))
			swap = true;
		nBits = header->bits;
		is_binary = true;
	}
	else
		r_mode = GMT->current.io.r_mode;

	if (!strcmp (HH->name, "="))	/* Read from pipe */
		fp = GMT->session.std[GMT_IN];
	else if ((fp = gmt_fopen (GMT, HH->name, r_mode)) != NULL) {
		if ((error = esri_read_info (GMT, fp, header)) != 0) {
			gmt_fclose (GMT, fp);
			return (error);
		}
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];
	n_expected = header->n_columns;

	if (nBits == 32)		/* Either an ASCII file or ESRI .HDR with NBITS = 32, in which case we assume it's a file of floats */
		tmp = gmt_M_memory (GMT, NULL, n_expected, float);
	else
		tmp16 = gmt_M_memory (GMT, NULL, n_expected, int16_t);

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	HH->has_NaNs = GMT_GRID_NO_NANS;	/* We are about to check for NaNs and if none are found we retain 1, else 2 */
	if (is_binary) {
		int n_rows = header->n_rows;
		if (last_row - first_row + 1 != n_rows)		/* We have a sub-region */
			if (fseek (fp, (off_t) (first_row * n_expected * 4UL * nBits / 32UL), SEEK_CUR)) {
				gmt_fclose (GMT, fp);
				gmt_M_free (GMT, actual_col);
				if (nBits == 32) gmt_M_free (GMT, tmp); else gmt_M_free (GMT, tmp16);
				return (GMT_GRDIO_SEEK_FAILED);
			}

		ij = imag_offset + pad[YHI] * width_out + pad[XLO];

		for (row = first_row; row <= last_row; row++, ij += width_out) {
			if (nBits == 32) {		/* Get one row */
				if (gmt_M_fread (tmp, 4, n_expected, fp) < n_expected) {
					gmt_fclose (GMT, fp);
					gmt_M_free (GMT, actual_col);
					gmt_M_free (GMT, tmp);
					return (GMT_GRDIO_READ_FAILED);
				}
			}
			else {
				if (gmt_M_fread (tmp16, 2, n_expected, fp) < n_expected) {
					gmt_fclose (GMT, fp);
					gmt_M_free (GMT, actual_col);
					gmt_M_free (GMT, tmp16);
					return (GMT_GRDIO_READ_FAILED);
				}
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
					HH->has_NaNs = GMT_GRID_HAS_NANS;
					grid[kk] = GMT->session.f_NaN;
				}
				else {		 /* Update z_min, z_max */
					header->z_min = MIN (header->z_min, (double)grid[kk]);
					header->z_max = MAX (header->z_max, (double)grid[kk]);
				}
			}
		}

	}
	else {		/* ASCII */
		n_left = header->nm;

		/* ESRI grids are scanline oriented (top to bottom), as are the GMT grids.
	 	 * NaNs are not allowed; they are represented by a nodata_value instead. */
		col = row = 0;		/* For the entire file */
		row2 = 0;	/* For the inside region */
		check = !isnan (header->nan_value);
		in_nx = header->n_columns;
		header->n_columns = width_in;	/* Needed to be set here due to gmt_M_ijp below */
#ifdef DOUBLE_PRECISION_GRID
		while (fscanf (fp, "%lf", &value) == 1 && n_left) {	/* We read all values and skip those not inside our w/e/s/n */
#else
		while (fscanf (fp, "%f", &value) == 1 && n_left) {	/* We read all values and skip those not inside our w/e/s/n */
#endif
			tmp[col] = value;	/* Build up a single input row */
			col++;
			if (col == in_nx) {	/* End of input row */
				if (row >= first_row && row <= last_row) {	/* We want a piece (or all) of this row */
					ij = imag_offset + gmt_M_ijp (header, row2, 0);	/* First out index for this row */
					for (ii = 0; ii < width_in; ii++) {
						kk = ij + ii;
						grid[kk] = (check && tmp[actual_col[ii]] == header->nan_value) ? GMT->session.f_NaN : tmp[actual_col[ii]];
						if (gmt_M_is_fnan (grid[kk])) {
							HH->has_NaNs = GMT_GRID_HAS_NANS;
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
	if (header->z_min == DBL_MAX && header->z_max == -DBL_MAX) /* No valid data values in the grid */
		header->z_min = header->z_max = NAN;

	gmt_fclose (GMT, fp);
	gmt_M_free (GMT, actual_col);
	gmt_M_free (GMT, tmp);
	if (nBits != 32) gmt_M_free (GMT, tmp16);

	if (n_left) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Expected % "PRIu64 " points, found only % "PRIu64 "\n", header->nm, header->nm - n_left);
		return (GMT_GRDIO_READ_FAILED);
	}

	header->n_columns = width_in;
	header->n_rows = height_in;
	gmt_M_memcpy (header->wesn, wesn, 4, double);

	return (GMT_NOERROR);
}

int gmt_esri_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode, bool floating) {
	unsigned int i2, j, j2, width_out, height_out, last;
	int first_col, last_col, first_row, last_row;
	unsigned int i, *actual_col = NULL;
	uint64_t ij, width_in, kk, imag_offset;
	char item[GMT_LEN64], c[2] = {0, 0};
	FILE *fp = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (header);

	if (fabs (header->inc[GMT_X] / header->inc[GMT_Y] - 1.0) > GMT_CONV8_LIMIT)
		return (GMT_GRDIO_ESRI_NONSQUARE);	/* Only square pixels allowed */
	if (!strcmp (HH->name, "="))	/* Write to pipe */
		fp = GMT->session.std[GMT_OUT];
	else if ((fp = gmt_fopen (GMT, HH->name, GMT->current.io.w_mode)) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);
	else
		esri_write_info (GMT, fp, header);

	gmt_M_err_pass (GMT, gmt_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), HH->name);
	(void)gmtlib_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	gmt_M_memcpy (header->wesn, wesn, 4, double);

	/* Store header information and array */

	i2 = first_col + pad[XLO];
	last = width_out - 1;
	for (j = 0, j2 = first_row + pad[YHI]; j < height_out; j++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		c[0] = '\t';
		for (i = 0; i < width_out; i++) {
			if (i == last) c[0] = '\n';
			kk = ij+actual_col[i];
			if (gmt_M_is_fnan (grid[kk]))
				snprintf (item, GMT_LEN64, "%ld%c", lrintf (header->nan_value), c[0]);
			else if (floating) {
				snprintf (item, GMT_LEN64-1, GMT->current.setting.format_float_out, grid[kk]);
				strcat (item, c);
			}
			else
				snprintf (item, GMT_LEN64, "%ld%c", lrint ((double)grid[kk]), c[0]);
			gmt_M_fputs (item, fp);
		}
	}

	gmt_M_free (GMT, actual_col);
	gmt_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int gmt_esri_writei_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* Standard integer values on output only */
	return (gmt_esri_write_grd (GMT, header, grid, wesn, pad, complex_mode, false));
}

int gmt_esri_writef_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* Write floating point on output */
	return (gmt_esri_write_grd (GMT, header, grid, wesn, pad, complex_mode, true));
}
