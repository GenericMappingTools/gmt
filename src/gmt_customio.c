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
 *	This program is distributed in the hope that it wi1552ll be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *	G M T _ C U S T O M I O . C   R O U T I N E S
 *
 * GMT has built-in support for several grid formats, which is extended
 * to a broad selection of formats via GDAL.  If there are very unusual
 * and custom formats you wish to see supported by GMT then please make
 * your case on the GMT trac website and we can start a dialog of whether
 * or not we should support your suggested format.
 *
 * For gurus: To add another data format:
 *
 *  1. Write the five required routines (see below).
 *  2. Edit gmt_grdio.h and add one entry to the Gmt_grid_id enum
 *  3. Increment GMT_N_GRD_FORMATS (in gmt_grdio.h too)
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 *
 * Functions include:
 *
 *	GMT_*_read_grd_info :	Read header from file
 *	GMT_*_read_grd :	Read header and data set from file
 *	GMT_*_update_grd_info :	Update header in existing file
 *	GMT_*_write_grd_info :	Write header to new file
 *	GMT_*_write_grd :	Write header and data set to new file
 *
 * where * is a tag specific to a particular data format
 *
 * NOTE:  1. GMT assumes that GMT_read_grd_info has been called before calls
 *	     to GMT_read_grd.  This normally is done via GMT_Read_Data.
 *	  2. Some formats may permit pipes to be used.  In that case GMT
 *	     expects the filename to be "=" (the equal sign).  It is the
 *	     responsibility of the custom routines to test for "=" and
 *	     give error messages if piping is not supported for this format
 *	     (e.g., netcdf uses fseek and can therefore not use pipes; other
 *	     formats may have similar limitations).
 *	  3. For most formats the write_grd_info and update_grd_info is the
 *	     same function (but netCDF is one exception)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt_dev.h"
#include "gmt_internals.h"

/* Defined in gmt_cdf.c */
int GMT_cdf_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_cdf_update_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_cdf_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_cdf_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode);
int GMT_cdf_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode);

/* Defined in gmt_nc.c */
int GMT_nc_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_nc_update_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_nc_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
int GMT_nc_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode);
int GMT_nc_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode);

/* CUSTOM I/O FUNCTIONS FOR GRIDDED DATA FILES */

/*-----------------------------------------------------------
 * Format : dummy
 * Purpose :
 *		Use this function to direct all unsupported formats to.
 * Functions : GMT_dummy_grd_info, GMT_dummy_grd_read
 *-----------------------------------------------------------*/

int GMT_dummy_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	if (header) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unknown grid format.\n");
	return (GMT_GRDIO_UNKNOWN_FORMAT);
}

int GMT_dummy_grd_read (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	if (header && grid && wesn && pad && complex_mode < 1024) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unknown grid format.\n");
	return (GMT_GRDIO_UNKNOWN_FORMAT);
}

/*-----------------------------------------------------------
 * Format :	rb
 * Type :	Standard 8-bit Sun rasterfiles
 * Prefix :	GMT_ras_
 * Author :	Paul Wessel, SOEST
 * Date :	17-JUN-1999
 * Purpose:	Rasterfiles may often be used to store
 *		datasets of limited dynamic range where
 *		8-bits is all that is needed.  Since the
 *		usual information like w,e,s,n is not part
 *		of a rasterfile's header, we assign default
 *		values as follows:
 *			w = s = 0.
 *			e = ras_width;
 *			n = ras_height;
 *			dx = dy = 1
 *		Such files are always pixel registered
 *
 * Functions :	GMT_ras_read_grd_info,
 *		GMT_ras_write_grd_info, GMT_ras_read_grd, GMT_ras_write_grd
 *-----------------------------------------------------------*/

#define	RAS_MAGIC	0x59a66a95

int GMT_read_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Reads the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	uint8_t byte[4];
	int32_t i, value;

	for (i = 0; i < 8; i++) {

		if (GMT_fread (byte, sizeof (uint8_t), 4, fp) != 4)
			return (GMT_GRDIO_READ_FAILED);

		value = (byte[0] << 24) + (byte[1] << 16) + (byte[2] << 8) + byte[3];

		switch (i) {
			case 0: h->magic = value;	break;
			case 1: h->width = value;	break;
			case 2: h->height = value;	break;
			case 3: h->depth = value;	break;
			case 4: h->length = value;	break;
			case 5: h->type = value;	break;
			case 6: h->maptype = value;	break;
			case 7: h->maplength = value;	break;
		}
	}

	if (h->type == RT_OLD && h->length == 0) h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;

	return (GMT_NOERROR);
}

int GMT_write_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Writes the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	int i;
	uint8_t byte[4];
	int32_t value;

	if (h->type == RT_OLD && h->length == 0) {
		h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;
		h->type = RT_STANDARD;
	}

	for (i = 0; i < 8; i++) {

		switch (i) {
			case 0: value = h->magic;	break;
			case 1:	value = h->width;	break;
			case 2: value = h->height;	break;
			case 3: value = h->depth;	break;
			case 4: value = h->length;	break;
			case 5: value = h->type;	break;
			case 6: value = h->maptype;	break;
			default: value = h->maplength;	break;
		}
		byte[0] = (uint8_t)((value >> 24) & 0xFF);
		byte[1] = (uint8_t)((value >> 16) & 0xFF);
		byte[2] = (uint8_t)((value >> 8) & 0xFF);
		byte[3] = (uint8_t)(value & 0xFF);

		if (GMT_fwrite (byte, sizeof (uint8_t), 4, fp) != 4)
			return (GMT_GRDIO_WRITE_FAILED);
	}

	return (GMT_NOERROR);
}

int GMT_is_ras_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	/* Determine if file is a Sun rasterfile */
	FILE *fp = NULL;
	struct rasterfile h;
	if (!strcmp (header->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (GMT, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);
	GMT_memset (&h, 1, struct rasterfile);
	if (GMT_read_rasheader (fp, &h))
		return (GMT_GRDIO_READ_FAILED);
	if (h.magic != RAS_MAGIC)
		return (GMT_GRDIO_NOT_RAS);
	if (h.type != 1 || h.depth != 8)
		return (GMT_GRDIO_NOT_8BIT_RAS);
	header->type = GMT_GRID_IS_RB;
	return GMT_NOERROR;
}

int GMT_ras_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	struct rasterfile h;
	unsigned char u;
	int i;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	GMT_memset (&h, 1, struct rasterfile);
	if (GMT_read_rasheader (fp, &h)) return (GMT_GRDIO_READ_FAILED);
	if (h.magic != RAS_MAGIC) return (GMT_GRDIO_NOT_RAS);
	if (h.type != 1 || h.depth != 8) return (GMT_GRDIO_NOT_8BIT_RAS);

	for (i = 0; i < h.maplength; i++) {
		if (GMT_fread (&u, sizeof (unsigned char), 1U, fp) < 1U) return (GMT_GRDIO_READ_FAILED);	/* Skip colormap by reading since fp could be stdin */
	}
	GMT_fclose (GMT, fp);

	/* Since we have no info on boundary values, just use integer size and steps = 1 */

	header->wesn[XLO] = header->wesn[YLO] = 0.0;
	header->wesn[XHI] = header->nx = h.width;
	header->wesn[YHI] = header->ny = h.height;
	header->inc[GMT_X] = header->inc[GMT_Y] = 1.0;
	header->registration = GMT_GRID_PIXEL_REG;	/* Always pixel format */
	header->z_scale_factor = 1.0;	header->z_add_offset = 0.0;

	return (GMT_NOERROR);
}

int GMT_ras_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	struct rasterfile h;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb+")) == NULL && (fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * irint (ceil (header->nx/2.0)) * 2;
	h.type = 1;
	h.maptype = h.maplength = 0;

	if (GMT_write_rasheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_ras_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	bool piping = false, check;
	int j, first_col, last_col, first_row, last_row;
	unsigned int i, width_in, height_in, *actual_row = NULL;
	size_t n2;
	uint64_t kk, ij, j2, width_out, imag_offset;
	FILE *fp = NULL;
	unsigned char *tmp = NULL;
	struct rasterfile h;

	GMT_memset (&h, 1, struct rasterfile);
	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) != NULL) {	/* Skip header */
		if (GMT_read_rasheader (fp, &h)) return (GMT_GRDIO_READ_FAILED);
		if (h.maplength && fseek (fp, (off_t) h.maplength, SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	n2 = lrint (ceil (header->nx / 2.0)) * 2;	/* Sun 8-bit rasters are stored using 16-bit words */
	tmp = GMT_memory (GMT, NULL, n2, unsigned char);

	check = !isnan (header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_row), header->name);

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) {
			if (GMT_fread (tmp, sizeof (unsigned char), n2, fp) < n2) return (GMT_GRDIO_READ_FAILED);
		}
	}
	else {/* Simply seek by it */
		if (first_row && fseek (fp, (off_t) (first_row * n2 * sizeof (unsigned char)), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;

	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		ij = imag_offset + (j2 + pad[3]) * width_out + pad[XLO];
		if (GMT_fread ( tmp, sizeof (unsigned char), n2, fp) < n2) return (GMT_GRDIO_READ_FAILED);
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			grid[kk] = (float) tmp[actual_row[i]];
			if (check && grid[kk] == header->nan_value)
				grid[kk] = GMT->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z min/max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip data by reading it */
		int ny = header->ny;
		for (j = last_row + 1; j < ny; j++) if (GMT_fread ( tmp, sizeof (unsigned char), n2, fp) < n2) return (GMT_GRDIO_READ_FAILED);
	}
	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (GMT, fp);

	GMT_free (GMT, actual_row);
	GMT_free (GMT, tmp);
	return (GMT_NOERROR);
}

int GMT_ras_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to write  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to remove on w, e, s, n of grid, respectively */
	/* complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */
	/* 		If 64 is added we write no header */

	bool check;
	unsigned int i, i2, j, width_out, height_out, n2, *actual_col = NULL;
	int first_col, last_col, first_row, last_row;
	uint64_t kk, ij, j2, width_in, imag_offset;

	unsigned char *tmp = NULL;

	FILE *fp = NULL;

	struct rasterfile h;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * irint (ceil (header->nx/2.0)) * 2;
	h.type = 1;
	h.maptype = h.maplength = 0;

	n2 = irint (ceil (header->nx / 2.0)) * 2;
	tmp = GMT_memory (GMT, NULL, n2, unsigned char);

	check = !isnan (header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);

	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	h.width = header->nx;
	h.height = header->ny;
	h.length = header->ny * irint (ceil (header->nx/2.0)) * 2;

	/* store header information and array */

	if (GMT_write_rasheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	i2 = first_col + pad[XLO];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		for (i = 0; i < width_out; i++) {
			kk = ij + actual_col[i];
			if (check && GMT_is_fnan (grid[kk])) grid[kk] = header->nan_value;
			tmp[i] = (unsigned char) grid[kk];
		}
		if (GMT_fwrite (tmp, sizeof (unsigned char), n2, fp) < n2) return (GMT_GRDIO_WRITE_FAILED);
	}
	GMT_fclose (GMT, fp);

	GMT_free (GMT, actual_col);
	GMT_free (GMT, tmp);

	return (GMT_NOERROR);

}

/*-----------------------------------------------------------
 * Format :	bm
 * Type :	Native binary (bit) C file
 * Prefix :	GMT_bit_
 * Author :	Paul Wessel, SOEST
 * Date :	27-JUN-1994
 * Purpose:	The native binary bit format is used
 *		primarily for piped i/o between programs
 *		that otherwise would use temporary, large
 *		intermediate grdfiles.  Note that not all
 *		programs can support piping (they may have
 *		to re-read the file or accept more than one
 *		grdfile).  Datasets containing ON/OFF information
 *		like bitmasks can be stored using bits
 *		We use 4-byte integers to store 32 bits at the time
 * Functions :	GMT_bit_read_grd, GMT_bit_write_grd
 *-----------------------------------------------------------*/

/* sizeof the first part of the native header */
#define SIZEOF_NATIVE_GRD_HDR1  12 /* 3 * sizeof (unsigned) */
/* sizeof the last part of the native header */
#define SIZEOF_NATIVE_GRD_HDR2 880 /* sizeof (struct GMT_GRID_HEADER) - (size_t)header->wesn */
/* sizeof the complete native header */
#define SIZEOF_NATIVE_GRD_HDR  892 /* SIZEOF_NATIVE_GRD_HDR1 + SIZEOF_NATIVE_GRD_HD */

int GMT_native_read_grd_header (FILE *fp, struct GMT_GRID_HEADER *header)
{
	int err = GMT_NOERROR;
	/* Because GMT_GRID_HEADER is not 64-bit aligned we must read it in parts */
	if (GMT_fread (&header->nx, SIZEOF_NATIVE_GRD_HDR1, 1U, fp) != 1 ||
			GMT_fread (header->wesn, SIZEOF_NATIVE_GRD_HDR2, 1U, fp) != 1)
	err = GMT_GRDIO_READ_FAILED;
	return (err);
}

int GMT_native_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	/* Read GRD header structure from native binary file.  This is used by
	 * all the native binary formats in GMT */

	int status;
	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	status = GMT_native_read_grd_header (fp, header);
	GMT_fclose (GMT, fp);
	return status;
}

int GMT_is_native_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	uint64_t mx, status, size;
	off_t nm;
	double item_size;
	struct stat buf;
	struct GMT_GRID_HEADER t_head;

	if (!strcmp (header->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if (stat (header->name, &buf))
		return (GMT_GRDIO_STAT_FAILED);		/* Inquiry about file failed somehow */
	strncpy (t_head.name, header->name, GMT_LEN256);
	if ((status = GMT_native_read_grd_info (GMT, &t_head)))
		return (GMT_GRDIO_READ_FAILED);	/* Failed to read header */
	if (t_head.nx <= 0 || t_head.ny <= 0 || !(t_head.registration == GMT_GRID_NODE_REG || t_head.registration == GMT_GRID_PIXEL_REG))
		return (GMT_GRDIO_BAD_VAL);		/* Garbage for nx or ny */
	if (t_head.wesn[XLO] >= t_head.wesn[XHI] || t_head.wesn[YLO] >= t_head.wesn[YHI])
		return (GMT_GRDIO_BAD_VAL);		/* Garbage for wesn */
	nm = GMT_get_nm (GMT, t_head.nx, t_head.ny);
	if (nm <= 0)
		return (GMT_GRDIO_BAD_VAL);			/* Overflow for nx * ny? */
	item_size = (double)((buf.st_size - GMT_GRID_HEADER_SIZE) / nm);	/* Estimate size of elements */
	size = lrint (item_size);
	if (!doubleAlmostEqualZero (item_size, (double)size))
		return (GMT_GRDIO_BAD_VAL);	/* Size not an integer */

	switch (size) {
		case 0:	/* Possibly bit map; check some more */
			mx = lrint (ceil (t_head.nx / 32.0));	/* Number of 4-byte integers it would take per row to store the bits */
			nm = 4 * mx * ((uint64_t)t_head.ny);	/* Number of bytes to store all the rows */
			if ((buf.st_size - GMT_GRID_HEADER_SIZE) == nm)	/* Yes, it was a bit mask file */
				header->type = GMT_GRID_IS_BM;
			else	/* No, junk data */
				return (GMT_GRDIO_BAD_VAL);
			break;
		case 1:	/* 1-byte elements */
			header->type = GMT_GRID_IS_BB;
			break;
		case 2:	/* 2-byte short int elements */
			header->type = GMT_GRID_IS_BS;
			break;
		case 4:	/* 4-byte elements - could be int or float */
			/* See if we can decide it is a float grid */
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Will try to determine if a native 4-byte grid is float or int but may be wrong.\n");
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Please append =bf (float) or =bi (integer) to avoid this situation.\n");
				/* Naive test to see if we can decide it is a float grid */
				if ((t_head.z_scale_factor == 1.0 && t_head.z_add_offset == 0.0) || fabs((t_head.z_min/t_head.z_scale_factor) - rint(t_head.z_min/t_head.z_scale_factor)) > GMT_CONV8_LIMIT || fabs((t_head.z_max/t_head.z_scale_factor) - rint(t_head.z_max/t_head.z_scale_factor)) > GMT_CONV8_LIMIT) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Based on header values we guessed the grid is 4-byte float.  If wrong you must add =bi.\n");
					header->type = GMT_GRID_IS_BF;
				}
				else {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Based on header values we guessed the grid is 4-byte int.  If wrong you must add =bf.\n");
					header->type = GMT_GRID_IS_BI;
				}
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: Cannot determine if a native 4-byte grid is float or int without more information.\n");
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR: You must append =bf (float) or =bi (integer) to avoid this situation.\n");
				return (GMT_GRDIO_NONUNIQUE_FORMAT);
			}
			break;
		case 8:	/* 8-byte elements */
			header->type = GMT_GRID_IS_BD;
			break;
		default:	/* Garbage */
			return (GMT_GRDIO_BAD_VAL);
			break;
	}
	return GMT_NOERROR;
}

int GMT_native_write_grd_header (FILE *fp, struct GMT_GRID_HEADER *header)
{
	int err = GMT_NOERROR;
	/* Because GMT_GRID_HEADER is not 64-bit aligned we must write it in parts */

	if (GMT_fwrite (&header->nx, SIZEOF_NATIVE_GRD_HDR1, 1U, fp) != 1 ||
			GMT_fwrite (header->wesn, SIZEOF_NATIVE_GRD_HDR2, 1U, fp) != 1)
		err = GMT_GRDIO_WRITE_FAILED;
	return (err);
}

int GMT_native_skip_grd_header (FILE *fp, struct GMT_GRID_HEADER *header)
{
	GMT_UNUSED(header);
	int err = GMT_NOERROR;
	/* Because GMT_GRID_HEADER is not 64-bit aligned we must estimate the # of bytes in parts */

	if (fseek (fp, SIZEOF_NATIVE_GRD_HDR, SEEK_SET))
		err = GMT_GRDIO_SEEK_FAILED;
	return (err);
}

int GMT_bit_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	int j, err, bit;
	bool piping = false, check = false;
	int first_col, last_col, first_row, last_row;
	unsigned int i, width_in, height_in, mx, word;
	unsigned int *actual_col = NULL;
	uint64_t kk, ij, j2, width_out, imag_offset;
	FILE *fp = NULL;
	unsigned int *tmp = NULL, ival;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) != NULL) {	/* Skip header */
		GMT_err_trap (GMT_native_skip_grd_header (fp, header));
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	check = !isnan (header->nan_value);
	mx = urint (ceil (header->nx / 32.0));	/* Whole multiple of 32-bit integers */

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	tmp = GMT_memory (GMT, NULL, mx, unsigned int);

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (GMT_fread (tmp, sizeof (unsigned int), mx, fp) < mx) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek by it */
		if (fseek (fp, (off_t) (first_row * mx * sizeof (unsigned int)), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (GMT_fread ( tmp, sizeof (unsigned int), mx, fp) < mx) return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		ij = imag_offset + (j2 + pad[YHI]) * width_out + pad[XLO];
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			word = actual_col[i] / 32;
			bit = actual_col[i] % 32;
			ival = (tmp[word] >> bit) & 1;
			grid[kk] = (float) ival;
			if (check && grid[kk] == header->nan_value)
				grid[kk] = GMT->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z min/max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip data by reading it */
		int ny = header->ny;
		for (j = last_row + 1; j < ny; j++) if (GMT_fread ( tmp, sizeof (unsigned int), mx, fp) < mx) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (GMT, fp);

	GMT_free (GMT, actual_col);
	GMT_free (GMT, tmp);
	return (GMT_NOERROR);
}

int GMT_bit_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&1 | &2 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		If 64 is added we write no header*/

	unsigned int i2, iu, ju, width_out, height_out, mx, word, *actual_col = NULL;
	int first_col, last_col, first_row, last_row;
	int i, j, bit, err;
	bool check = false, do_header = true;
	uint64_t kk, ij, j2, width_in, imag_offset;
	unsigned int *tmp = NULL, ival;

	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	check = !isnan (header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &actual_col), header->name);
	do_header = GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			ij = j2 * width_in + i2 + imag_offset;
			if (GMT_is_fnan (grid[ij])) {
				if (check)
					grid[ij] = header->nan_value;
			}
			else {
				ival = (unsigned)lrintf (grid[ij]);
				if (ival > 1) ival = 1;	/* Truncate to 1 */
				header->z_min = MIN (header->z_min, (double)ival);
				header->z_max = MAX (header->z_max, (double)ival);
			}
		}
	}

	/* Store header information and array */

	if (do_header) GMT_err_trap (GMT_native_write_grd_header (fp, header));

	mx = urint (ceil (width_out / 32.0));
	tmp = GMT_memory (GMT, NULL, mx, unsigned int);

	i2 = first_col + pad[XLO];
	for (ju = 0, j2 = first_row + pad[YHI]; ju < height_out; ju++, j2++) {
		GMT_memset (tmp, mx, unsigned int);
		ij = j2 * width_in + i2 + imag_offset;
		for (iu = 0; iu < width_out; iu++) {
			kk = ij + actual_col[iu];
			word = iu / 32;
			bit = iu % 32;
			ival = (unsigned)lrintf (grid[kk]);
			if (ival > 1) ival = 1;	/* Truncate to 1 */
			tmp[word] |= (ival << bit);
		}
		if (GMT_fwrite (tmp, sizeof (unsigned int), mx, fp) < mx) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_fclose (GMT, fp);

	GMT_free (GMT, actual_col);
	GMT_free (GMT, tmp);

	return (GMT_NOERROR);
}

/*-----------------------------------------------------------
 * Format :	bb, bs, bi, bf, bd
 * Type :	Native binary C file
 * Prefix :	GMT_native_
 * Author :	Paul Wessel, SOEST
 * Date :	17-JUN-1999
 * Purpose:	The native binary output format is used
 *		primarily for piped i/o between programs
 *		that otherwise would use temporary, large
 *		intermediate grdfiles.  Note that not all
 *		programs can support piping (they may have
 *		to re-read the file or accept more than one
 *		grdfile).  Datasets with limited range may
 *		be stored using 1-, 2-, or 4-byte integers
 *		which will reduce storage space and improve
 *		throughput.
 *-----------------------------------------------------------*/

/* GMT 64-bit Modification:
 *
 * Read/write GRD header structure from native binary file.  This is
 * used by all the native binary formats in GMT.  We isolate the I/O of
 * the header structure here because of 32/64 bit issues of alignment.
 * The GRD header is 892 bytes long, three 4-byte integers followed
 * by ten 8-byte doubles and six character strings. This created a
 * problem on 64-bit systems, where the GMT_GRID_HEADER structure was
 * automatically padded with 4-bytes before the doubles. Taking
 * advantage of the code developed to deal with the 32/64-bit anomaly
 * (below), we have permanently added a 4-byte integer to the
 * GMT_GRID_HEADER structure, but skip it when reading or writing the
 * header.
 */

int GMT_native_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	/* Write GRD header structure to native binary file.  This is used by
	 * all the native binary formats in GMT */

	int err;
	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb+")) == NULL && (fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	GMT_err_trap (GMT_native_write_grd_header (fp, header));

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_native_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array is to hold real (1) and imaginary (2) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	int j, type;			/* Data type */
	bool piping = false;		/* true if we read input pipe instead of from file */
	bool check = false;		/* true if nan-proxies are used to signify NaN (for non-floating point types) */
	unsigned int err;		/* Error code returned */
	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	unsigned int height_in;		/* Number of columns in subregion */
	unsigned int i;			/* Misc. counters */
	unsigned int *k = NULL;		/* Array with indices */
	unsigned int width_in;		/* Number of items in one row of the subregion */
	uint64_t kk, ij, j2, width_out, imag_offset;	/* Width of row as return (may include padding) */
	size_t size;			/* Length of data type */ 
	size_t n_expected;		/* Length of row to read */ 
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for reading in rows of data */

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) != NULL)	{	/* Skip header */
		GMT_err_trap (GMT_native_skip_grd_header (fp, header));
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	type = GMT->session.grdformat[header->type][1];
	size = GMT_grd_data_size (GMT, header->type, &header->nan_value);
	check = !isnan (header->nan_value);

	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	/* Allocate memory for one row of data (for reading purposes) */

	n_expected = header->nx;
	tmp = GMT_memory (GMT, NULL, n_expected * size, char);

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (GMT_fread (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek over it */
		if (fseek (fp, (off_t) (first_row * n_expected * size), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (GMT_fread (tmp, size, n_expected, fp) < n_expected)
			return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		ij = imag_offset + (j2 + pad[YHI]) * width_out + pad[XLO];
		for (i = 0, kk = ij; i < width_in; i++, kk++) {
			grid[kk] = GMT_decode (GMT, tmp, k[i], type);	/* Convert whatever to float */
			if (check && grid[kk] == header->nan_value)
				grid[kk] = GMT->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z_min, z_max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip remaining data by reading them */
		int ny = header->ny;
		for (j = last_row + 1; j < ny; j++) if (GMT_fread (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (GMT, fp);

	GMT_free (GMT, k);
	GMT_free (GMT, tmp);

	return (GMT_NOERROR);
}

int GMT_native_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array holds real (4) and imaginary (8) parts (otherwise write real only) */
	/*		Note: The file has only real values, we simply have allowed for space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */
	/*		If 64 is added we write no header */

	int err;			/* Error code*/
	int i, j, type;			/* Data type */
	bool check = false;		/* true if nan-proxies are used to signify NaN (for non-floating point types) */
	bool do_header = true;		/* true if we should write the header first */
	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	unsigned int width_out;		/* Width of row as return (may include padding) */
	unsigned int height_out;	/* Number of columns in subregion */
	unsigned int i2, ju, iu;	/* Misc. counters */
	unsigned int *k = NULL;		/* Array with indices */
	uint64_t ij, width_in, imag_offset, j2;
	size_t size;			/* Length of data type */
	size_t n_expected;		/* Length of row to read */ 
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for writing in rows of data */

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	type = GMT->session.grdformat[header->type][1];
	size = GMT_grd_data_size (GMT, header->type, &header->nan_value);
	check = !isnan (header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	do_header = GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			ij = imag_offset + (j2 * width_in + i2);
			if (GMT_is_fnan (grid[ij])) {
				if (check) grid[ij] = header->nan_value;
			}
			else {
				header->z_min = MIN (header->z_min, (double)grid[ij]);
				header->z_max = MAX (header->z_max, (double)grid[ij]);
			}
		}
	}

	/* Round off to chosen type */

	if (type != 'f' && type != 'd') {
		header->z_min = rint (header->z_min);
		header->z_max = rint (header->z_max);
	}

	/* Store header information and array */

	if (do_header) GMT_err_trap (GMT_native_write_grd_header (fp, header));

	/* Allocate memory for one row of data (for writing purposes) */

	n_expected = header->nx;
	tmp = GMT_memory (GMT, NULL, n_expected * size, char);

	i2 = first_col + pad[XLO];
	for (ju = 0, j2 = first_row + pad[YHI]; ju < height_out; ju++, j2++) {
		ij = imag_offset + j2 * width_in + i2;
		for (iu = 0; iu < width_out; iu++) GMT_encode (GMT, tmp, iu, grid[ij+k[iu]], type);
		if (GMT_fwrite (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_free (GMT, k);
	GMT_free (GMT, tmp);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

void GMT_encode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, float z, unsigned int type)
{	/* Place the z value in the array location of the (type) pointer */
	switch (type) {
		case 'b':
			((char *)vptr)[k] = (char)lrintf (z);
			break;
		case 's':
			((short int *)vptr)[k] = (short int)lrintf (z);
			break;
		case 'i':
		case 'm':
			((int *)vptr)[k] = (int)lrintf (z);
			break;
		case 'f':
			((float *)vptr)[k] = z;
			break;
		case 'd':
			((double *)vptr)[k] = (double)z;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT: Bad call to GMT_encode\n");
			break;
	}
}

float GMT_decode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, unsigned int type)
{	/* Retrieve the z value from the array location of the (type) pointer */
	float fval;

	switch (type) {
		case 'b':
			fval = (float)(((char *)vptr)[k]);
			break;
		case 's':
			fval = (float)(((short int *)vptr)[k]);
			break;
		case 'i':
		case 'm':
			fval = (float)(((int *)vptr)[k]);
			break;
		case 'f':
			fval = ((float *)vptr)[k];
			break;
		case 'd':
			fval = (float)(((double *)vptr)[k]);
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT: Bad call to GMT_decode\n");
			fval = GMT->session.f_NaN;
			break;
	}

	return (fval);
}

/*-----------------------------------------------------------
 * Format :	sf, sd
 * Type :	Surfer 6 and 7 (float) file
 * Prefix :	GMT_srf_
 * Author :	Joaquim Luis
 * Date :	09-SEP-1999
 * 		27-AUG-2005	Added minimalist support to GS format 7
 * 				Type :	Native binary (double) C file
 * Purpose:	to transform to/from Surfer grid file format
 * Functions :	GMT_srf_read_grd_info, GMT_srf_write_grd_info,
 *		GMT_srf_write_grd_info, GMT_srf_read_grd, GMT_srf_write_grd
 *-----------------------------------------------------------*/

struct srf_header6 {	/* Surfer 6 file header structure */
	char id[4];		/* ASCII Binary identifier (DSBB) */
	unsigned short int nx;	/* Number of columns -- NOTE: original definition by GoldenSoft is "short int" */
	unsigned short int ny;	/* Number of rows */
	double wesn[4];		/* Min/maximum x/y coordinates */
	double z_min;		/* Minimum z value */
	double z_max;		/* Maximum z value */
};

/* The format 7 is rather more complicated. It has now headers that point to "sections"
   that may either be also headers or have real data. Besides that, is follows also the
   stupidity of storing the grid using doubles (I would bat that there are no more than 0-1
   Surfer users that really need to save their grids in doubles). The following header
   does not strictly follow the GS format description, but its enough for reading grids
   that do not contain break-lines (again my estimate is that it covers (100 - 1e4)% users).

   Note: I had significant troubles to be able to read correctly the Surfer 7 format.
   In its basic and mostly used form (that is, without break-lines info) what we normally
   call a header, can be described by the srf_header7 structure bellow (but including
   the three commented lines). This would make that the header is composed of 2 char[4] and
   and 5 ints followed by doubles. The problem was that after the ints the doubles were not
   read correctly. It looked like everything was displaced by 4 bytes.
   I than found the note about the GMT 64-bit Modification and tried the same trick.
   While that worked with gcc compiled code, it crashed whith the VC6 compiler.
   Since of the first 3 variables, only the first is important to find out which Surfer
   grid format we are dealing with, I removed them from the header definition (and jump
   12 bytes before reading the header). As a result the header has now one 4 byte char +
   trhee 4-bytes ints followed by 8-bytes doubles. With this organization the header is
   read correctly by GMT_read_srfheader7. Needless to say that I don't understand why the
   even number of 4-bytes variables before the 8-bytes caused that the doubles we incorrectly read.

   Joaquim Luis 08-2005. */

struct srf_header7 {	/* Surfer 7 file header structure */
	/*char id[4];		 * ASCII Binary identifier (DSRB) */
	/*int idumb1;		 * Size of Header in bytes (is == 1) */
	/*int idumb2;		 * Version number of the file format. Currently must be set to 1*/
	char id2[4];		/* Tag ID indicating a grid section (GRID) */
	int len_g;		/* Length in bytes of the grid section (72) */
	int ny;			/* Number of rows */
	int nx;			/* Number of columns */
	double x_min;		/* Minimum x coordinate */
	double y_min;		/* Minimum y coordinate */
	double x_inc;		/* Spacing between columns */
	double y_inc;		/* Spacing between rows */
	double z_min;		/* Minimum z value */
	double z_max;		/* Maximum z value */
	double rotation;	/* not currently used */
	double no_value;	/* If GS were cleverer this would be NaN */
	char id3[4];		/* Tag ID idicating a data section (DATA) */
	int len_d;		/* Length in bytes of the DATA section */
};

int GMT_is_srf_grid (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	FILE *fp = NULL;
	char id[5];
	if (!strcmp (header->name, "="))
		return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (GMT, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);
	if (GMT_fread (id, sizeof (char), 4U, fp) < 4U)
		return (GMT_GRDIO_READ_FAILED);
	GMT_fclose (GMT, fp);
	if (!strncmp (id, "DSBB", 4U))
		header->type = GMT_GRID_IS_SF;
	else if (!strncmp (id, "DSRB", 4U))
		header->type = GMT_GRID_IS_SD;
	else
		return (GMT_GRDIO_BAD_VAL);	/* Neither */
	return GMT_NOERROR;
}

int GMT_read_srfheader6 (FILE *fp, struct srf_header6 *h) {
	/* Reads the header of a Surfer 6 gridfile */
	/* if (GMT_fread (h, sizeof (struct srf_header6), 1U, fp) < 1U) return (GMT_GRDIO_READ_FAILED); */

	/* UPDATE: Because srf_header6 is not 64-bit aligned we must read it in parts */
	if (GMT_fread (h->id, 4*sizeof (char), 1U, fp) != 1U)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->nx, 2*sizeof (short int), 1U, fp) != 1U)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (h->wesn, sizeof (struct srf_header6) - ((size_t)h->wesn - (size_t)h->id), 1U, fp) != 1U)
		return (GMT_GRDIO_READ_FAILED);

	return GMT_NOERROR;
}

int GMT_read_srfheader7 (FILE *fp, struct srf_header7 *h) {
	/* Reads the header of a Surfer 7 gridfile */

	if (fseek (fp, (off_t)(3*sizeof(int)), SEEK_SET))
		return (GMT_GRDIO_SEEK_FAILED);	/* skip the first 12 bytes */
	/* if (GMT_fread (h, sizeof (struct srf_header7), 1U, fp) < 1U) return (GMT_GRDIO_READ_FAILED); */

	/* UPDATE: Because srf_header6 is not 64-bit aligned we must read it in parts */
	if (GMT_fread (h->id2, 4*sizeof (char), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->len_g, 3*sizeof (int), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->x_min, 8*sizeof (double), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (h->id3, 4*sizeof (char), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->len_d, sizeof (int), 1U, fp) != 1)
		return (GMT_GRDIO_READ_FAILED);

	return GMT_NOERROR;
}

int GMT_write_srfheader (FILE *fp, struct srf_header6 *h) {
	/* if (GMT_fwrite (h, sizeof (struct srf_header6), 1U, fp) < 1U) return (GMT_GRDIO_WRITE_FAILED); */
	/* UPDATE: Because srf_header6 is not 64-bit aligned we must write it in parts */
	if (GMT_fwrite (h->id, 4*sizeof (char), 1U, fp) != 1)
		return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite (&h->nx, 2*sizeof (short int), 1U, fp) != 1)
		return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite (h->wesn, sizeof (struct srf_header6) - ((size_t)h->wesn - (size_t)h->id), 1U, fp) != 1)
		return (GMT_GRDIO_WRITE_FAILED);
	return GMT_NOERROR;
}

int GMT_srf_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	struct srf_header6 h6;
	struct srf_header7 h7;
	char id[5];

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (GMT_fread (id, sizeof (char), 4U, fp) < 4U) return (GMT_GRDIO_READ_FAILED);
	if (fseek(fp, (off_t)0, SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
	if (strncmp (id, "DSBB", 4U) && strncmp (id, "DSRB", 4U)) return (GMT_GRDIO_NOT_SURFER);

	GMT_memset (&h6, 1, struct srf_header6);
	GMT_memset (&h7, 1, struct srf_header7);
	if (!strncmp (id, "DSBB", 4U)) {		/* Version 6 format */
		if (GMT_read_srfheader6 (fp, &h6)) return (GMT_GRDIO_READ_FAILED);
		header->type = GMT_GRID_IS_SF;
	}
	else {					/* Version 7 format */
		if (GMT_read_srfheader7 (fp, &h7))  return (GMT_GRDIO_READ_FAILED);
		if (h7.len_d != h7.nx * h7.ny * 8 || !strcmp (h7.id2, "GRID")) return (GMT_GRDIO_SURF7_UNSUPPORTED);
		header->type = GMT_GRID_IS_SD;
	}

	GMT_fclose (GMT, fp);

	header->registration = GMT_GRID_NODE_REG;	/* Grid node registration */
	if (header->type == GMT_GRID_IS_SF) {
		strcpy (header->title, "Grid originally in Surfer 6 format");
		header->nx = h6.nx;		header->ny = h6.ny;
		header->wesn[XLO] = h6.wesn[XLO];	header->wesn[XHI] = h6.wesn[XHI];
		header->wesn[YLO] = h6.wesn[YLO];	header->wesn[YHI] = h6.wesn[YHI];
		header->z_min = h6.z_min;		header->z_max = h6.z_max;
		header->inc[GMT_X] = GMT_get_inc (GMT, h6.wesn[XLO], h6.wesn[XHI], h6.nx, header->registration);
		header->inc[GMT_Y] = GMT_get_inc (GMT, h6.wesn[YLO], h6.wesn[YHI], h6.ny, header->registration);
	}
	else {			/* Format GMT_GRID_IS_SD */
		strcpy (header->title, "Grid originally in Surfer 7 format");
		header->nx = h7.nx;		header->ny = h7.ny;
		header->wesn[XLO] = h7.x_min;	header->wesn[YLO] = h7.y_min;
		header->wesn[XHI] = h7.x_min + h7.x_inc * (h7.nx - 1);
		header->wesn[YHI] = h7.y_min + h7.y_inc * (h7.ny - 1);
		header->z_min = h7.z_min;	header->z_max = h7.z_max;
		header->inc[GMT_X] = h7.x_inc;	header->inc[GMT_Y] = h7.y_inc;
	}
	header->z_scale_factor = 1;	header->z_add_offset = 0;

	return (GMT_NOERROR);
}

int GMT_srf_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header)
{
	FILE *fp = NULL;
	struct srf_header6 h;

	if (!strcmp (header->name, "="))	/* Write to pipe */
	{
#ifdef SET_IO_MODE
	GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb+")) == NULL && (fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	strncpy (h.id, "DSBB", 4U);
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	if (header->registration == GMT_GRID_PIXEL_REG) {
		h.wesn[XLO] = header->wesn[XLO] + header->inc[GMT_X]/2.0;	 h.wesn[XHI] = header->wesn[XHI] - header->inc[GMT_X]/2.0;
		h.wesn[YLO] = header->wesn[YLO] + header->inc[GMT_Y]/2.0;	 h.wesn[YHI] = header->wesn[YHI] - header->inc[GMT_Y]/2.0;
	}
	else
		GMT_memcpy (h.wesn, header->wesn, 4, double);
	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (GMT_write_srfheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

int GMT_srf_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	int j, type, ny;			/* Data type */
	bool piping = false;		/* true if we read input pipe instead of from file */
	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	unsigned int width_in;		/* Number of items in one row of the subregion */
	unsigned int height_in;		/* Number of columns in subregion */
	unsigned int i; 		/* Misc. counters */
	unsigned int *k = NULL;		/* Array with indices */
	uint64_t kk, ij, j2, width_out, imag_offset;
	size_t size;			/* Length of data type */
	size_t n_expected;		/* Length of a row */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for reading in rows of data */

	/* Fixed nan_value in Surfer grids */
	header->nan_value = 1.70141e38f;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
		fp = GMT->session.std[GMT_IN];
		piping = true;
	}
	else if ((fp = GMT_fopen (GMT, header->name, "rb")) != NULL) {	/* Skip header */
		if (header->type == GMT_GRID_IS_SF) {	/* Surfer Version 6 */
			if (fseek (fp, (off_t) sizeof (struct srf_header6), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
		}
		else {			/* Version 7  (skip also the first 12 bytes) */
			if (fseek (fp, (off_t) (3*sizeof(int) + sizeof (struct srf_header7)), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
		}
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	type = GMT->session.grdformat[header->type][1];
	size = GMT_grd_data_size (GMT, header->type, &header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	ny = header->ny;
	if ( (last_row - first_row + 1) != ny) {    /* We have a sub-region */
		/* Surfer grids are stored starting from Lower Left, which is contrary to
		   the rest of GMT grids that start at Top Left. So we must do a flip here */
		first_row = ny - height_in - first_row;
		last_row = first_row + height_in - 1;
	}

	/* Allocate memory for one row of data (for reading purposes) */

	n_expected = header->nx;
	tmp = GMT_memory (GMT, NULL, n_expected * size, char);

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++)
			if (GMT_fread (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek over it */
		if (first_row && fseek (fp, (off_t) (first_row * n_expected * size), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	for (j = first_row, j2 = height_in-1; j <= last_row; j++, j2--) {
		if (GMT_fread (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		ij = imag_offset + (j2 + pad[YHI]) * width_out + pad[XLO];
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			grid[kk] = GMT_decode (GMT, tmp, k[i], type);	/* Convert whatever to float */
			if (grid[kk] >= header->nan_value)
				grid[kk] = GMT->session.f_NaN;
			else {	/* Update z_min, z_max */
				header->z_min = MIN (header->z_min, (double)grid[kk]);
				header->z_max = MAX (header->z_max, (double)grid[kk]);
			}
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < ny; j++)
			if (GMT_fread (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = width_in;
	header->ny = height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (GMT, fp);

	GMT_free (GMT, k);
	GMT_free (GMT, tmp);

	return (GMT_NOERROR);
}

int GMT_srf_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesnn:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	int i, j, type;			/* Data type */
	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	unsigned int width_out;		/* Width of row as return (may include padding) */
	unsigned int height_out;		/* Number of columns in subregion */
	unsigned int i2, ju, iu;		/* Misc. counters */
	unsigned int *k = NULL;		/* Array with indices */
	uint64_t ij, kk, j2, width_in, imag_offset;	/* Number of items in one row of the subregion */
	size_t size;			/* Length of data type */
	size_t n_expected;		/* Length of a row */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for writing in rows of data */
	struct srf_header6 h;

	if (GMT->session.grdformat[header->type][1] == 'd') {
#ifdef HAVE_GDAL
		GMT_Message(GMT->parent, GMT_TIME_NONE,
			"Surfer 7 format in GMT is read-only but you can do it via GDAL by appending '=gd:GS7BG' to the file name\n");
#else
		GMT_Message(GMT->parent, GMT_TIME_NONE,
			"As mentioned in the manual, Surfer 7 format in GMT is read-only\n");
#endif
		return (GMT_NOERROR);
	}

	header->nan_value = 1.70141e38f;	/* Fixed nan_value in Surfer grids */

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
		fp = GMT->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (GMT, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	type = GMT->session.grdformat[header->type][1];
	size = GMT_grd_data_size (GMT, header->type, &header->nan_value);

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		ij = imag_offset + j2 * width_in;
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			kk = ij + i2;
			if (isnan (grid[kk]))
				grid[kk] = header->nan_value;
			else {
				header->z_min = MIN (header->z_min, (double)grid[kk]);
				header->z_max = MAX (header->z_max, (double)grid[kk]);
			}
		}
	}

	/* store header information and array */

	strncpy (h.id, "DSBB", 4U);
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	if (header->registration == GMT_GRID_PIXEL_REG) {
		h.wesn[XLO] = header->wesn[XLO] + header->inc[GMT_X]/2;	 h.wesn[XHI] = header->wesn[XHI] - header->inc[GMT_X]/2;
		h.wesn[YLO] = header->wesn[YLO] + header->inc[GMT_Y]/2;	 h.wesn[YHI] = header->wesn[YHI] - header->inc[GMT_Y]/2;
	}
	else
		GMT_memcpy (h.wesn, header->wesn, 4, double);

	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (GMT_fwrite (&h, sizeof (struct srf_header6), 1U, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);

	/* Allocate memory for one row of data (for writing purposes) */

	n_expected = header->nx;
	tmp = GMT_memory (GMT, NULL, n_expected * size, char);

	i2 = first_col + pad[XLO];
	for (ju = 0, j2 = last_row + pad[YHI]; ju < height_out; ju++, j2--) {
		ij = imag_offset + j2 * width_in + i2;
		for (iu = 0; iu < width_out; iu++) GMT_encode (GMT, tmp, iu, grid[ij+k[iu]], type);
		if (GMT_fwrite (tmp, size, n_expected, fp) < n_expected) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_free (GMT, k);
	GMT_free (GMT, tmp);

	GMT_fclose (GMT, fp);

	return (GMT_NOERROR);
}

#ifdef HAVE_GDAL
#include "gmt_gdalread.c"
#include "gmt_gdalwrite.c"
/* GDAL support */
/*-----------------------------------------------------------
 * Format :	gd
 * Type :	GDAL compatible format
 * Prefix :	GMT_gdal_
 * Author :	Joaquim Luis
 * Date :	06-SEP-2009
 *
 * Purpose:	to access data read trough the GDAL interface
 * Functions :	GMT_gdal_read_grd_info, GMT_gdal_write_grd_info,
 *		GMT_gdal_write_grd_info, GMT_gdal_read_grd, GMT_gdal_write_grd
 *-----------------------------------------------------------*/

static inline void free_from_gdalread(struct GMT_CTRL *GMT, struct GMT_GDALREAD_OUT_CTRL *from_gdalread) {
	int i;
	if (from_gdalread->ColorMap)
		GMT_free (GMT, from_gdalread->ColorMap);
	for (i = 0; i < from_gdalread->RasterCount; i++)
		free (from_gdalread->band_field_names[i].DataType); /* Those were allocated with strdup */
	if (from_gdalread->ProjectionRefPROJ4 != NULL)
		free (from_gdalread->ProjectionRefPROJ4);
	if (from_gdalread->ProjectionRefWKT != NULL)
		free (from_gdalread->ProjectionRefWKT);
	GMT_free (GMT, from_gdalread->band_field_names);
	GMT_free (GMT, from_gdalread);
}

int GMT_gdal_read_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	struct GMT_GDALREAD_IN_CTRL *to_gdalread = NULL;
	struct GMT_GDALREAD_OUT_CTRL *from_gdalread = NULL;

	if (!strcmp (header->name, "=")) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Pipes cannot be used within the GDAL interface.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	/* Allocate new control structures */
	to_gdalread = GMT_memory (GMT, NULL, 1, struct GMT_GDALREAD_IN_CTRL);
	from_gdalread = GMT_memory (GMT, NULL, 1, struct GMT_GDALREAD_OUT_CTRL);

	to_gdalread->M.active = true;		/* Metadata only */

	if (GMT_gdalread (GMT, header->name, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading file with gdalread.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	header->type = GMT_GRID_IS_GD;
	header->registration = (int)from_gdalread->hdr[6];	/* Which registration? */
	strcpy (header->title, "Grid imported via GDAL");
	header->nx = from_gdalread->RasterXsize, header->ny = from_gdalread->RasterYsize;
	GMT_memcpy (header->wesn, from_gdalread->hdr, 4, double);
	header->inc[GMT_X] = from_gdalread->hdr[7];
	header->inc[GMT_Y] = from_gdalread->hdr[8];
	header->z_min = from_gdalread->hdr[4];
	header->z_max = from_gdalread->hdr[5];
	if (from_gdalread->band_field_names) {
		header->z_scale_factor = from_gdalread->band_field_names[0].ScaleOffset[0];
		header->z_add_offset   = from_gdalread->band_field_names[0].ScaleOffset[1];
	}
	else {
		header->z_scale_factor = 1.0;
		header->z_add_offset   = 0.0;
	}

	GMT_free (GMT, to_gdalread);
	free_from_gdalread (GMT, from_gdalread);

	return (GMT_NOERROR);
}

int GMT_gdal_write_grd_info (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header) {
	GMT_UNUSED(GMT); GMT_UNUSED(header);
	return (GMT_NOERROR);
}

int GMT_gdal_read_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	&4 | &8 if complex array is to hold real (4) and imaginary (8) parts (otherwise read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	struct GMT_GDALREAD_IN_CTRL *to_gdalread = NULL;
	struct GMT_GDALREAD_OUT_CTRL *from_gdalread = NULL;
	int nBand, subset;
	uint64_t i, j, row, col;
	char strR[128];

	/* Allocate new control structures */
	to_gdalread = GMT_memory (GMT, NULL, 1, struct GMT_GDALREAD_IN_CTRL);
	from_gdalread = GMT_memory (GMT, NULL, 1, struct GMT_GDALREAD_OUT_CTRL);

	if (complex_mode & GMT_GRID_IS_COMPLEX_MASK) {
		to_gdalread->Z.active = true;		/* Force reading into a compex array */
		to_gdalread->Z.complex_mode = (int)((complex_mode & GMT_GRID_IS_COMPLEX_MASK) >> 2);	/* Gives 0, 1, or 2 */
	}

	subset = GMT_is_subset (GMT, header, wesn);	/* We have a Sub-region demand */
	if (subset) {	/* We have a Sub-region demand */
		to_gdalread->R.active = true;
		sprintf(strR, "%.10f/%.10f/%.10f/%.10f", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		to_gdalread->R.region = strR;
		to_gdalread->registration.val = header->registration;	/* Due to pix-reg only by GDAL we need to inform it about our reg type */
		to_gdalread->registration.x_inc = header->inc[GMT_X];
		to_gdalread->registration.y_inc = header->inc[GMT_Y];
	}

	/* This code chunk fixed #173 (r10638) grdpaste of via-GDAL grids but later caused #403 */
	if (pad[XLO] > 0 || pad[XHI] > 0 || pad[YLO] > 0 || pad[YHI] > 0) {
		to_gdalread->mini_hdr.active = true;
		if (pad[XLO] >= header->nx - 1) {	/* With -1 we account for both grid & pixel registration cases */
			to_gdalread->mini_hdr.offset = pad[XLO];		to_gdalread->mini_hdr.side[0] = 'r';
			to_gdalread->mini_hdr.mx = header->mx;
			if (GMT_check_condition (GMT, !header->mx, "Programming error, header.mx not set\n")) return (EXIT_FAILURE);
		}
		else if (pad[XHI] >= header->nx - 1) {
			to_gdalread->mini_hdr.offset = pad[XHI];		to_gdalread->mini_hdr.side[0] = 'l';
			to_gdalread->mini_hdr.mx = header->mx;
			if (GMT_check_condition (GMT, !header->mx, "Programming error, header.mx not set\n")) return (EXIT_FAILURE);
		}
		else if (pad[YLO] >= header->ny - 1) {
			to_gdalread->mini_hdr.offset = pad[YLO];		to_gdalread->mini_hdr.side[0] = 't';
			to_gdalread->mini_hdr.my = header->my;
			if (GMT_check_condition (GMT, !header->my, "Programming error, header.my not set\n")) return (EXIT_FAILURE);
		}
		else if (pad[YHI] >= header->ny - 1) {
			to_gdalread->mini_hdr.offset = pad[YHI];		to_gdalread->mini_hdr.side[0] = 'b';
			to_gdalread->mini_hdr.my = header->my;
			if (GMT_check_condition (GMT, !header->my, "Programming error, header.my not set\n")) return (EXIT_FAILURE);
		}
		else {
			/* Here we assume that all pad[0] ... pad[3] are equal. Otherwise ... */
			to_gdalread->mini_hdr.active = false;	/* Undo above setting */
			to_gdalread->p.active = true;
			to_gdalread->p.pad = (int)pad[XLO];
		}

		/* OK, now test if we are under the condition of #403 (very small grids).
		   If yes, undo the mini_hdr solution ... and hope no more troubles arise. */
		if (to_gdalread->mini_hdr.active && (header->nx <= 4 || header->ny <= 4)) {
			to_gdalread->mini_hdr.active = false;
			to_gdalread->p.active = true;
			to_gdalread->p.pad = (int)pad[XLO];
		}
	}
	if (header->pocket) {	/* Have a band request. */
		to_gdalread->B.active = true;
		to_gdalread->B.bands = header->pocket;		/* Band parsing and error testing is done in gmt_gdalread */
	}

	/* Tell gmt_gdalread that we already have the memory allocated and send in the *grid pointer */
	to_gdalread->f_ptr.active = true;
	to_gdalread->f_ptr.grd = grid;
	
	/* If header->nan_value != NaN tell gdalread to replace nan_value by NaN (in floats only) */
	to_gdalread->N.nan_value = header->nan_value;

	if (GMT_gdalread (GMT, header->name, to_gdalread, from_gdalread)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading file with gdalread.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	if (to_gdalread->B.active) {
		free(header->pocket);		/* It was allocated by strdup. Free it for an eventual reuse. */
		header->pocket = NULL;
	}

	if (subset) {	/* We had a Sub-region demand */
		header->nx = from_gdalread->RasterXsize;
		header->ny = from_gdalread->RasterYsize;
		header->nm = gmt_grd_get_nm (header);		/* Sets the number of actual data items */
		GMT_memcpy (header->wesn, from_gdalread->hdr, 4, double);
		header->z_min = from_gdalread->hdr[4];
		header->z_max = from_gdalread->hdr[5];
	}

	header->registration = (int)from_gdalread->hdr[6];	/* Confirm registration. It may not be the same as reported by read_grd_info */

	if (from_gdalread->Float.active) {
		if (!to_gdalread->f_ptr.active)		/* If the float array was allocated inside _gdalread */
			GMT_memcpy (grid, from_gdalread->Float.data, header->size, float);
		else if (!to_gdalread->c_ptr.active && from_gdalread->UInt8.active) {		/* If the char array was allocated inside _gdalread */
			for (j = 0; j < header->size; j++)
				grid[j] = (float)from_gdalread->UInt8.data[j];
		}
	}
	else {
		/* Convert everything else do float */
		nBand = 0;		/* Need a solution to RGB or multiband files */
		i = nBand * header->size;
		if (from_gdalread->UInt8.active)
			for (j = 0; j < header->size; j++)
				grid[j] = (float)from_gdalread->UInt8.data[j+i];
		else if (from_gdalread->UInt16.active)
			for (j = 0; j < header->size; j++)
				grid[j] = (float)from_gdalread->UInt16.data[j+i];
		else if (from_gdalread->Int16.active)
			for (j = 0; j < header->size; j++)
				grid[j] = (float)from_gdalread->Int16.data[j+i];
		else if (from_gdalread->Int32.active)
			for (j = 0; j < header->size; j++)
				grid[j] = (float)from_gdalread->Int32.data[j+i];
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR data type not suported with gdalread in gmt_customio.\n");
			return (GMT_GRDIO_OPEN_FAILED);
		}
	}

	if (from_gdalread->nodata && !GMT_is_dnan (from_gdalread->nodata)) {	/* Data has a nodata value */
		/* Since all originally integer types were actually converted to float above, we can use
		   the same test to search for nodata values other than NaN. Note that gmt_galread sets
		   unknown nodata return by GDAL as NaN, so in those cases this block of code is not executed */

		/* Pointer arithmetic solution that should be parallelizable */
		grid += (header->pad[YHI] * header->mx + header->pad[XLO]);	/* Position pointer at start of first row taking pad into acount */
		for (row = 0; row < header->ny; row++) {
			for (col = 0; col < header->nx; col++, grid++) {
				if (*grid == (float)from_gdalread->nodata)
					*grid = GMT->session.f_NaN;
			}
			grid += (header->pad[XLO] + header->pad[XHI]);	/* Advance the pad number of columns */
		}
		grid = &grid[0];	/* Put the pointer pointing back to first element in array */
	}
	header->nan_value = GMT->session.f_NaN;

	if (from_gdalread->UInt8.active)
		GMT_free (GMT, from_gdalread->UInt8.data);
	else if ( from_gdalread->Float.active && !to_gdalread->f_ptr.active )	/* Do not release the *grid pointer */
		GMT_free (GMT, from_gdalread->Float.data);
	else if (from_gdalread->UInt16.active)
		GMT_free (GMT, from_gdalread->UInt16.data);
	else if (from_gdalread->Int16.active)
		GMT_free (GMT, from_gdalread->Int16.data);
	else if (from_gdalread->Int32.active)
		GMT_free (GMT, from_gdalread->Int32.data);

	GMT_free (GMT, to_gdalread);
	free_from_gdalread (GMT, from_gdalread);

	return (GMT_NOERROR);
}

int GMT_gdal_write_grd (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *grid, double wesn[], unsigned int *pad, unsigned int complex_mode) {
	GMT_UNUSED(pad);
	uint64_t node = 0, ij, imag_offset;
	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	unsigned int width_out;	/* Width of row as return (may include padding) */
	unsigned int height_out;	/* Number of columns in subregion */
	unsigned int *k = NULL;	/* Array with indices */
	unsigned int row, col;
	char driver[16], type[16];
	unsigned char *zu8 = NULL;
	short int *zi16 = NULL;
	unsigned short int *zu16 = NULL;
	int *zi32 = NULL;
	unsigned int *zu32 = NULL;
	struct GMT_GDALWRITE_CTRL *to_GDALW = NULL;
	type[0] = '\0';

	if (header->pocket == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Cannot write with GDAL without knowing which driver to use.\n");
		return (GMT_NOERROR);
	}

	GMT_err_pass (GMT, GMT_grd_prep_io (GMT, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (header, complex_mode, &imag_offset);	/* Set offset for imaginary complex component */

	sscanf (header->pocket, "%[^/]/%s", driver, type);
	to_GDALW = GMT_memory (GMT, NULL, 1, struct GMT_GDALWRITE_CTRL);
	to_GDALW->driver = strdup(driver);
	to_GDALW->P.ProjectionRefPROJ4 = NULL;
	to_GDALW->flipud = 0;
	if (GMT_is_geographic (GMT, GMT_IN))
		to_GDALW->geog = 1;
	else
		to_GDALW->geog = 0;
	to_GDALW->nx = width_out;
	to_GDALW->ny = height_out;
	to_GDALW->nXSizeFull = header->mx;
	to_GDALW->n_bands = header->n_bands;
	to_GDALW->registration = header->registration;
	to_GDALW->pad[0] = header->pad[XLO];		to_GDALW->pad[1] = header->pad[XHI];
	to_GDALW->pad[2] = header->pad[YLO];		to_GDALW->pad[3] = header->pad[YHI];
	to_GDALW->ULx = wesn[XLO];
	to_GDALW->ULy = wesn[YHI];
	to_GDALW->x_inc = GMT_get_inc (GMT, header->wesn[XLO], header->wesn[XHI], header->nx, header->registration);
	to_GDALW->y_inc = GMT_get_inc (GMT, header->wesn[YLO], header->wesn[YHI], header->ny, header->registration);
	to_GDALW->nan_value = header->nan_value;
	to_GDALW->command = header->command;

	/* Lazy implementation of nodata value update as it doesn't check and apply on a eventual sub-region on output only */
	if (!isnan (header->nan_value)) {
		for (ij = 0; ij < header->mx * header->my; ij++)
			if (isnan (grid[ij]))
				grid[ij] = header->nan_value;
	}

	if (!type[0] || GMT_strlcmp(type, "float32")) {
		/* We have to shift the grid pointer in order to use the GDALRasterIO ability to extract a subregion. */
		/* See: osgeo-org.1560.n6.nabble.com/gdal-dev-writing-a-subregion-with-GDALRasterIO-td4960500.html */
		to_GDALW->data = &grid[2 * header->mx + (header->pad[XLO] + first_col)+imag_offset];
		to_GDALW->type = strdup("float32");
		GMT_gdalwrite(GMT, header->name, to_GDALW);
		free (to_GDALW->driver);
		free (to_GDALW->type);
		GMT_free (GMT, to_GDALW);
		GMT_free (GMT, k);
		return (GMT_NOERROR);
	}
	else if (GMT_strlcmp(type,"u8") || GMT_strlcmp(type,"u08")) {
		zu8 = GMT_memory(GMT, NULL, width_out * height_out, unsigned char);
		for (row = first_row; row < height_out; row++)
			for (col = first_col, ij = GMT_IJP (header, row, 0)+imag_offset; col < width_out; col++, ij++)
				zu8[node++] = (unsigned char)grid[ij];

		to_GDALW->data = zu8;
		to_GDALW->type = strdup("uint8");
	}
	else if (GMT_strlcmp(type,"i16")) {
		zi16 = GMT_memory(GMT, NULL, width_out * height_out, short int);
		for (row = first_row; row < height_out; row++)
			for (col = first_col, ij = GMT_IJP (header, row, 0)+imag_offset; col < width_out; col++, ij++)
				zi16[node++] = (short int)grid[ij];

		to_GDALW->data = zi16;
		to_GDALW->type = strdup("int16");
	}
	else if (GMT_strlcmp(type,"u16")) {
		zu16 = GMT_memory(GMT, NULL, width_out * height_out, unsigned short int);
		for (row = first_row; row < height_out; row++)
			for (col = first_col, ij = GMT_IJP (header, row, 0)+imag_offset; col < width_out; col++, ij++)
				zu16[node++] = (unsigned short int)grid[ij];

		to_GDALW->data = zu16;
		to_GDALW->type = strdup("uint16");
	}
	else if (GMT_strlcmp(type,"i32")) {
		zi32 = GMT_memory(GMT, NULL, width_out * height_out, int);
		for (row = first_row; row < height_out; row++)
			for (col = first_col, ij = GMT_IJP (header, row, 0)+imag_offset; col < width_out; col++, ij++)
				zi32[node++] = (int)grid[ij];

		to_GDALW->data = zi32;
		to_GDALW->type = strdup("int32");
	}
	else if (GMT_strlcmp(type,"u32")) {
		zu32 = GMT_memory(GMT, NULL, width_out * height_out, unsigned int);
		for (row = first_row; row < height_out; row++)
			for (col = first_col, ij = GMT_IJP (header, row, 0)+imag_offset; col < width_out; col++, ij++)
				zu32[node++] = (unsigned int)grid[ij];

		to_GDALW->data = zu32;
		to_GDALW->type = strdup("uint32");
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unknown or unsupported data type code in gmt_customio for writing file with GDAL.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	GMT_gdalwrite(GMT, header->name, to_GDALW);

	GMT_free (GMT, k);
	GMT_free (GMT, to_GDALW->data);
	free (to_GDALW->driver);
	free (to_GDALW->type);
	GMT_free (GMT, to_GDALW);
	return (GMT_NOERROR);
}

#endif

/* Add custom code here */

/* 12: NOAA NGDC MGG Format */
#include "gmt_mgg_header2.c"

/* 21: Atlantic Geoscience Center format */
#include "gmt_agc_io.c"

/* 23: ESRI Arc/Info ASCII interchange format */
#include "gmt_esri_io.c"

void GMT_grdio_init (struct GMT_CTRL *GMT) {
	unsigned int id;

	/* First element is empty */

	id                        = k_grd_unknown_fmt;
	GMT->session.grdformat[id]  = "Unknown grid format";
	GMT->session.readinfo[id]   = &GMT_dummy_grd_info;
	GMT->session.updateinfo[id] = &GMT_dummy_grd_info;
	GMT->session.writeinfo[id]  = &GMT_dummy_grd_info;
	GMT->session.readgrd[id]    = &GMT_dummy_grd_read;
	GMT->session.writegrd[id]   = &GMT_dummy_grd_read;

	/* FORMAT: GMT netCDF-based (byte) grdio (COARDS compliant) */

	id                        = GMT_GRID_IS_NB;
	GMT->session.grdformat[id]  = "nb = GMT netCDF format (8-bit integer), " GMT_NC_CONVENTION;
	GMT->session.readinfo[id]   = &GMT_nc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_nc_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_nc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_nc_read_grd;
	GMT->session.writegrd[id]   = &GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (short) grdio (COARDS compliant) */

	id                        = GMT_GRID_IS_NS;
	GMT->session.grdformat[id]  = "ns = GMT netCDF format (16-bit integer), " GMT_NC_CONVENTION;
	GMT->session.readinfo[id]   = &GMT_nc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_nc_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_nc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_nc_read_grd;
	GMT->session.writegrd[id]   = &GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (int) grdio (COARDS compliant) */

	id                        = GMT_GRID_IS_NI;
	GMT->session.grdformat[id]  = "ni = GMT netCDF format (32-bit integer), " GMT_NC_CONVENTION;
	GMT->session.readinfo[id]   = &GMT_nc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_nc_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_nc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_nc_read_grd;
	GMT->session.writegrd[id]   = &GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (float) grdio (COARDS compliant) */

	id                        = GMT_GRID_IS_NF;
	GMT->session.grdformat[id]  = "nf = GMT netCDF format (32-bit float), " GMT_NC_CONVENTION;
	GMT->session.readinfo[id]   = &GMT_nc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_nc_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_nc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_nc_read_grd;
	GMT->session.writegrd[id]   = &GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (double) grdio (COARDS compliant) */

	id                        = GMT_GRID_IS_ND;
	GMT->session.grdformat[id]  = "nd = GMT netCDF format (64-bit float), " GMT_NC_CONVENTION;
	GMT->session.readinfo[id]   = &GMT_nc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_nc_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_nc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_nc_read_grd;
	GMT->session.writegrd[id]   = &GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (byte) grdio */

	id                        = GMT_GRID_IS_CB;
	GMT->session.grdformat[id]  = "cb = GMT netCDF format (8-bit integer, deprecated)";
	GMT->session.readinfo[id]   = &GMT_cdf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_cdf_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_cdf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_cdf_read_grd;
	GMT->session.writegrd[id]   = &GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (short) grdio */

	id                        = GMT_GRID_IS_CS;
	GMT->session.grdformat[id]  = "cs = GMT netCDF format (16-bit integer, deprecated)";
	GMT->session.readinfo[id]   = &GMT_cdf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_cdf_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_cdf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_cdf_read_grd;
	GMT->session.writegrd[id]   = &GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (int) grdio */

	id                        = GMT_GRID_IS_CI;
	GMT->session.grdformat[id]  = "ci = GMT netCDF format (32-bit integer, deprecated)";
	GMT->session.readinfo[id]   = &GMT_cdf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_cdf_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_cdf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_cdf_read_grd;
	GMT->session.writegrd[id]   = &GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (float) grdio */

	id                        = GMT_GRID_IS_CF;
	GMT->session.grdformat[id]  = "cf = GMT netCDF format (32-bit float, deprecated)";
	GMT->session.readinfo[id]   = &GMT_cdf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_cdf_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_cdf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_cdf_read_grd;
	GMT->session.writegrd[id]   = &GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (double) grdio */

	id                        = GMT_GRID_IS_CD;
	GMT->session.grdformat[id]  = "cd = GMT netCDF format (64-bit float, deprecated)";
	GMT->session.readinfo[id]   = &GMT_cdf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_cdf_update_grd_info;
	GMT->session.writeinfo[id]  = &GMT_cdf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_cdf_read_grd;
	GMT->session.writegrd[id]   = &GMT_cdf_write_grd;

	/* FORMAT: GMT native binary (bit) grdio */

	id                        = GMT_GRID_IS_BM;
	GMT->session.grdformat[id]  = "bm = GMT native, C-binary format (bit-mask)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_bit_read_grd;
	GMT->session.writegrd[id]   = &GMT_bit_write_grd;

	/* FORMAT: GMT native binary (byte) grdio */

	id                        = GMT_GRID_IS_BB;
	GMT->session.grdformat[id]  = "bb = GMT native, C-binary format (8-bit integer)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_native_read_grd;
	GMT->session.writegrd[id]   = &GMT_native_write_grd;

	/* FORMAT: GMT native binary (short) grdio */

	id                        = GMT_GRID_IS_BS;
	GMT->session.grdformat[id]  = "bs = GMT native, C-binary format (16-bit integer)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_native_read_grd;
	GMT->session.writegrd[id]   = &GMT_native_write_grd;

	/* FORMAT: GMT native binary (int) grdio */

	id                        = GMT_GRID_IS_BI;
	GMT->session.grdformat[id]  = "bi = GMT native, C-binary format (32-bit integer)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_native_read_grd;
	GMT->session.writegrd[id]   = &GMT_native_write_grd;

	/* FORMAT: GMT native binary (float) grdio */

	id                        = GMT_GRID_IS_BF;
	GMT->session.grdformat[id]  = "bf = GMT native, C-binary format (32-bit float)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_native_read_grd;
	GMT->session.writegrd[id]   = &GMT_native_write_grd;

	/* FORMAT: GMT native binary (double) grdio */

	id                        = GMT_GRID_IS_BD;
	GMT->session.grdformat[id]  = "bd = GMT native, C-binary format (64-bit float)";
	GMT->session.readinfo[id]   = &GMT_native_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_native_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_native_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_native_read_grd;
	GMT->session.writegrd[id]   = &GMT_native_write_grd;

	/* FORMAT: SUN 8-bit standard rasterfile grdio */

	id                        = GMT_GRID_IS_RB;
	GMT->session.grdformat[id]  = "rb = SUN rasterfile format (8-bit standard)";
	GMT->session.readinfo[id]   = &GMT_ras_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_ras_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_ras_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_ras_read_grd;
	GMT->session.writegrd[id]   = &GMT_ras_write_grd;

	/* FORMAT: NOAA NGDC MGG grid format */

	id                        = GMT_GRID_IS_RF;
	GMT->session.grdformat[id]  = "rf = GEODAS grid format GRD98 (NGDC)";
	GMT->session.readinfo[id]   = &GMT_mgg2_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_mgg2_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_mgg2_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_mgg2_read_grd;
	GMT->session.writegrd[id]   = &GMT_mgg2_write_grd;

	/* FORMAT: GMT native binary (float) grdio (Surfer format) */

	id                        = GMT_GRID_IS_SF;
	GMT->session.grdformat[id]  = "sf = Golden Software Surfer format 6 (32-bit float)";
	GMT->session.readinfo[id]   = &GMT_srf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_srf_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_srf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_srf_read_grd;
	GMT->session.writegrd[id]   = &GMT_srf_write_grd;

	/* FORMAT: GMT native binary (double) grdio (Surfer format) */

	id                        = GMT_GRID_IS_SD;
	GMT->session.grdformat[id]  = "sd = Golden Software Surfer format 7 (64-bit float, read-only)";
	GMT->session.readinfo[id]   = &GMT_srf_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_srf_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_srf_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_srf_read_grd;
	GMT->session.writegrd[id]   = &GMT_srf_write_grd;

	/* FORMAT: GMT native binary (float) grdio (AGC format) */

	id                        = GMT_GRID_IS_AF;
	GMT->session.grdformat[id]  = "af = Atlantic Geoscience Center format AGC (32-bit float)";
	GMT->session.readinfo[id]   = &GMT_agc_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_agc_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_agc_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_agc_read_grd;
	GMT->session.writegrd[id]   = &GMT_agc_write_grd;

	/* FORMAT: ESRI Arc/Info ASCII Interchange Grid format (integer) */

	id                        = GMT_GRID_IS_EI;
	GMT->session.grdformat[id]  = "ei = ESRI Arc/Info ASCII Grid Interchange format (ASCII integer)";
	GMT->session.readinfo[id]   = &GMT_esri_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_esri_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_esri_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_esri_read_grd;
	GMT->session.writegrd[id]   = &GMT_esri_writei_grd;

	/* FORMAT: ESRI Arc/Info ASCII Interchange Grid format (float) */

	id                        = GMT_GRID_IS_EF;
	GMT->session.grdformat[id]  = "ef = ESRI Arc/Info ASCII Grid Interchange format (ASCII float)";
	GMT->session.readinfo[id]   = &GMT_esri_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_esri_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_esri_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_esri_read_grd;
	GMT->session.writegrd[id]   = &GMT_esri_writef_grd;

	/* FORMAT: Import via the GDAL interface */

	id                        = GMT_GRID_IS_GD;
#ifdef HAVE_GDAL
	GMT->session.grdformat[id]  = "gd = Import/export through GDAL";
	GMT->session.readinfo[id]   = &GMT_gdal_read_grd_info;
	GMT->session.updateinfo[id] = &GMT_gdal_write_grd_info;
	GMT->session.writeinfo[id]  = &GMT_gdal_write_grd_info;
	GMT->session.readgrd[id]    = &GMT_gdal_read_grd;
	GMT->session.writegrd[id]   = &GMT_gdal_write_grd;
#else
	GMT->session.grdformat[id]  = "gd = Import/export through GDAL (not supported)";
	GMT->session.readinfo[id]   = &GMT_dummy_grd_info;
	GMT->session.updateinfo[id] = &GMT_dummy_grd_info;
	GMT->session.writeinfo[id]  = &GMT_dummy_grd_info;
	GMT->session.readgrd[id]    = &GMT_dummy_grd_read;
	GMT->session.writegrd[id]   = &GMT_dummy_grd_read;
#endif

	/* ----------------------------------------------
	 * ADD CUSTOM FORMATS BELOW AS THEY ARE NEEDED */
}

/* Comparator for qsort in GMT_grdformats_sorted() */
int compare_grd_fmt_strings(const void* a, const void* b) {
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	return strncmp(*ia, *ib, 2);
}

/* This function sorts the grid formats alphabetically by their type id */
char **GMT_grdformats_sorted (struct GMT_CTRL *Ctrl) {
	static char *formats_sorted[GMT_N_GRD_FORMATS];
	static bool sorted = false;

	if (sorted)
		return formats_sorted;

	/* copy array with char pointers to type id strings: */
	memcpy (formats_sorted, Ctrl->session.grdformat, GMT_N_GRD_FORMATS * sizeof(char*));
	/* sort pointers beginning from the 2nd element: */
	qsort (formats_sorted + 1, GMT_N_GRD_FORMATS - 1, sizeof(char*), &compare_grd_fmt_strings);
	sorted = true;

	return formats_sorted;
}
