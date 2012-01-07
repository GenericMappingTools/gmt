/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it wi1552ll be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *	G M T _ C U S T O M I O . C   R O U T I N E S
 *
 * Takes care of all custom format gridfile input/output.  The
 * industrious user may supply his/her own code to read specific data
 * formats.  Five functions must be supplied, and they must conform
 * to the GMT standard and return the same arguments as the generic
 * GMT grdio functions.  See gmt_cdf.c for details.
 *
 * To add another data format:
 *
 *	1. Write the five required routines (see below).
 *	2. Place prototypes in the gmt_customio.h file.
 *	3. Provide another text entry in Grdformats.txt
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
 *	     to GMT_read_grd.
 *	  2. Some formats may permit pipes to be used.  In that case GMT
 *	     expects the filename to be "=" (the equal sign).  It is the
 *	     responsibility of the custom routines to test for "=" and
 *	     give error messages if piping is not supported for this format
 *	     (e.g., netcdf uses fseek and can therefore not use pipes; other
 *	     formats may have similar limitations).
 *	  3. For most formats the write_grd_info and update_grd_info is the
 *	     same function (but netCDF is one exception)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

/* Defined in gmt_cdf.c */
EXTERN_MSC GMT_LONG GMT_cdf_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_cdf_update_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_cdf_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_cdf_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode);
EXTERN_MSC GMT_LONG GMT_cdf_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode);

/* Defined in gmt_nc.c */
EXTERN_MSC GMT_LONG GMT_nc_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_nc_update_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_nc_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_nc_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode);
EXTERN_MSC GMT_LONG GMT_nc_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode);

/* CUSTOM I/O FUNCTIONS FOR GRIDDED DATA FILES */

/*-----------------------------------------------------------
 * Format : dummy
 * Purpose :
 *		Use this function to direct all unsupported formats to.
 * Functions : GMT_dummy_grd_info
 *-----------------------------------------------------------*/

GMT_LONG GMT_dummy_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
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

GMT_LONG GMT_read_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Reads the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	GMT_LONG i, j, value, in[4];

	for (i = 0; i < 8; i++) {

		if (GMT_fread (byte, sizeof (unsigned char), (size_t)4, fp) != 4) return (GMT_GRDIO_READ_FAILED);

		for (j = 0; j < 4; j++) in[j] = (GMT_LONG)byte[j];

		value = (in[0] << 24) + (in[1] << 16) + (in[2] << 8) + in[3];

		switch (i) {
			case 0:
				h->magic = (int)value;
				break;
			case 1:
				h->width = (int)value;
				break;
			case 2:
				h->height = (int)value;
				break;
			case 3:
				h->depth = (int)value;
				break;
			case 4:
				h->length = (int)value;
				break;
			case 5:
				h->type = (int)value;
				break;
			case 6:
				h->maptype = (int)value;
				break;
			case 7:
				h->maplength = (int)value;
				break;
		}
	}

	if (h->type == RT_OLD && h->length == 0) h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;

	return (GMT_NOERROR);
}

GMT_LONG GMT_write_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Writes the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	GMT_LONG i, value;

	if (h->type == RT_OLD && h->length == 0) {
		h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;
		h->type = RT_STANDARD;
	}

	for (i = 0; i < 8; i++) {

		switch (i) {
			case 0:
				value = h->magic;
				break;
			case 1:
				value = h->width;
				break;
			case 2:
				value = h->height;
				break;
			case 3:
				value = h->depth;
				break;
			case 4:
				value = h->length;
				break;
			case 5:
				value = h->type;
				break;
			case 6:
				value = h->maptype;
				break;
			default:
				value = h->maplength;
				break;
		}
		byte[0] = (unsigned char)((value >> 24) & 0xFF);
		byte[1] = (unsigned char)((value >> 16) & 0xFF);
		byte[2] = (unsigned char)((value >> 8) & 0xFF);
		byte[3] = (unsigned char)(value & 0xFF);

		if (GMT_fwrite (byte, sizeof (unsigned char), (size_t)4, fp) != 4) return (GMT_GRDIO_WRITE_FAILED);
	}

	return (GMT_NOERROR);
}

GMT_LONG GMT_is_ras_grid (struct GMT_CTRL *C, struct GRD_HEADER *header)
{	/* Determine if file is a Sun rasterfile */
	FILE *fp = NULL;
	struct rasterfile h;
	if (!strcmp (header->name, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (C, header->name, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
	if (GMT_read_rasheader (fp, &h)) return (GMT_GRDIO_READ_FAILED);
	if (h.magic != RAS_MAGIC) return (GMT_GRDIO_NOT_RAS);
	if (h.type != 1 || h.depth != 8) return (GMT_GRDIO_NOT_8BIT_RAS);
	header->type = GMT_GRD_IS_RB;
	return (header->type);
}

GMT_LONG GMT_ras_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	struct rasterfile h;
	unsigned char u;
	GMT_LONG i;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (GMT_read_rasheader (fp, &h)) return (GMT_GRDIO_READ_FAILED);
	if (h.magic != RAS_MAGIC) return (GMT_GRDIO_NOT_RAS);
	if (h.type != 1 || h.depth != 8) return (GMT_GRDIO_NOT_8BIT_RAS);

	for (i = 0; i < h.maplength; i++) {
		if (GMT_fread (&u, sizeof (unsigned char), (size_t)1, fp) < (size_t)1) return (GMT_GRDIO_READ_FAILED);	/* Skip colormap by reading since fp could be stdin */
	}
	GMT_fclose (C, fp);

	/* Since we have no info on boundary values, just use integer size and steps = 1 */

	header->wesn[XLO] = header->wesn[YLO] = 0.0;
	header->wesn[XHI] = header->nx = h.width;
	header->wesn[YHI] = header->ny = h.height;
	header->inc[GMT_X] = header->inc[GMT_Y] = 1.0;
	header->registration = GMT_PIXEL_REG;	/* Always pixel format */
	header->z_scale_factor = 1.0;	header->z_add_offset = 0.0;

	return (GMT_NOERROR);
}

GMT_LONG GMT_ras_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	struct rasterfile h;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb+")) == NULL && (fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.type = 1;
	h.maptype = h.maplength = 0;

	if (GMT_write_rasheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_ras_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col, first_row, last_row, inc, off;
	GMT_LONG i, j, j2, width_in, width_out, height_in, i_0_out, n2;
	GMT_LONG kk, ij, piping = FALSE, check, *k = NULL;
	FILE *fp = NULL;
	unsigned char *tmp = NULL;
	struct rasterfile h;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) != NULL) {	/* Skip header */
		if (GMT_read_rasheader (fp, &h)) return (GMT_GRDIO_READ_FAILED);
		if (h.maplength && GMT_fseek (fp, (long) h.maplength, SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	n2 = (GMT_LONG) ceil (header->nx / 2.0) * 2;	/* Sun 8-bit rasters are stored using 16-bit words */
	tmp = GMT_memory (C, NULL, n2, unsigned char);

	check = !GMT_is_dnan (header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];
	width_out *= inc;			/* Possibly twice is complex is TRUE */
	i_0_out = inc * pad[XLO] + off;		/* Edge offset in output */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) {
			if (GMT_fread ( tmp, sizeof (unsigned char), (size_t)n2, fp) < (size_t)n2) return (GMT_GRDIO_READ_FAILED);
		}
	}
	else {/* Simply seek by it */
		if (GMT_fseek (fp, (long) (first_row * n2 * sizeof (unsigned char)), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;

	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		if (GMT_fread ( tmp, sizeof (unsigned char), (size_t)n2, fp) < (size_t)n2) return (GMT_GRDIO_READ_FAILED);
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = (float) tmp[k[i]];
			if (check && grid[kk] == header->nan_value) grid[kk] = C->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z min/max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip data by reading it */
		for (j = last_row + 1; j < header->ny; j++) if (GMT_fread ( tmp, sizeof (unsigned char), (size_t)n2, fp) < (size_t)n2) return (GMT_GRDIO_READ_FAILED);
	}
	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);
	return (GMT_NOERROR);
}

GMT_LONG GMT_ras_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to write  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to remove on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */
	/* 		If 64 is added we write no header */

	GMT_LONG i, i2, inc = 1, off = 0, kk, ij, check, *k = NULL;
	GMT_LONG j, j2, width_in, width_out, height_out, n2;
	GMT_LONG first_col, last_col, first_row, last_row, do_header = TRUE;

	unsigned char *tmp = NULL;

	FILE *fp = NULL;

	struct rasterfile h;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.type = 1;
	h.maptype = h.maplength = 0;

	n2 = (GMT_LONG) ceil (header->nx / 2.0) * 2;
	tmp = GMT_memory (C, NULL, n2, unsigned char);

	check = !GMT_is_dnan (header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	do_header = GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	h.width = header->nx;
	h.height = header->ny;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;

	/* store header information and array */

	if (do_header && GMT_write_rasheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	i2 = first_col + pad[XLO];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) {
			kk = inc * (ij + k[i]) + off;
			if (check && GMT_is_fnan (grid[kk])) grid[kk] = (float)header->nan_value;
			tmp[i] = (unsigned char) grid[kk];
		}
		if (GMT_fwrite (tmp, sizeof (unsigned char), (size_t)width_out, fp) < (size_t)width_out) return (GMT_GRDIO_WRITE_FAILED);
	}
	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);

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

GMT_LONG GMT_native_read_grd_header (FILE *fp, struct GRD_HEADER *header)
{
	GMT_LONG err = GMT_NOERROR;
	/* Because GRD_HEADER is not 64-bit aligned we must read it in parts */
	if (GMT_fread (&header->nx, 3*sizeof (int), (size_t)1, fp) != 1 || GMT_fread (header->wesn, sizeof (struct GRD_HEADER) - ((GMT_LONG)header->wesn - (GMT_LONG)&header->nx), (size_t)1, fp) != 1)
	err = GMT_GRDIO_READ_FAILED;
	return (err);
}

GMT_LONG GMT_native_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	/* Read GRD header structure from native binary file.  This is used by
	 * all the native binary formats in GMT */

	GMT_LONG err;
	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	GMT_err_trap (GMT_native_read_grd_header (fp, header));

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_is_native_grid (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	GMT_LONG nm, mx, status, size;
	double item_size;
	struct GMT_STAT buf;
	struct GRD_HEADER t_head;

	if (!strcmp (header->name, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if (GMT_STAT (header->name, &buf)) return (GMT_GRDIO_STAT_FAILED);		/* Inquiry about file failed somehow */
	strcpy (t_head.name, header->name);
	if ((status = GMT_native_read_grd_info (C, &t_head))) return (GMT_GRDIO_READ_FAILED);	/* Failed to read header */
	if (t_head.nx <= 0 || t_head.ny <= 0) return (GMT_GRDIO_BAD_VAL);		/* Garbage for nx or ny */
	nm = GMT_get_nm (C, t_head.nx, t_head.ny);
	if (nm <= 0) return (GMT_GRDIO_BAD_VAL);			/* Overflow for nx * ny? */
	item_size = (double)((buf.st_size - GRD_HEADER_SIZE) / nm);	/* Estimate size of elements */
	size = irint (item_size);
	if (!GMT_IS_ZERO (item_size - (double)size)) return (GMT_GRDIO_BAD_VAL);	/* Size not an integer */

	switch (size) {
		case 0:	/* Possibly bit map; check some more */
			mx = (GMT_LONG) ceil (t_head.nx / 32.0);
			nm = mx * ((GMT_LONG)t_head.ny);
			if ((buf.st_size - GRD_HEADER_SIZE) == nm)	/* Yes, it was a bit mask file */
				header->type = GMT_GRD_IS_BM;
			else	/* No, junk data */
				return (GMT_GRDIO_BAD_VAL);
			break;
		case 1:	/* 1-byte elements */
			header->type = GMT_GRD_IS_BB;
			break;
		case 2:	/* 2-byte short int elements */
			header->type = GMT_GRD_IS_BS;
			break;
		case 4:	/* 4-byte elements - could be int or float */
			/* See if we can decide it is a float grid */
			if ((t_head.z_scale_factor == 1.0 && t_head.z_add_offset == 0.0) || fabs((t_head.z_min/t_head.z_scale_factor) - rint(t_head.z_min/t_head.z_scale_factor)) > GMT_CONV_LIMIT || fabs((t_head.z_max/t_head.z_scale_factor) - rint(t_head.z_max/t_head.z_scale_factor)) > GMT_CONV_LIMIT)
				header->type = GMT_GRD_IS_BF;
			else
				header->type = GMT_GRD_IS_BI;
			break;
		case 8:	/* 8-byte elements */
			header->type = GMT_GRD_IS_BD;
			break;
		default:	/* Garbage */
			return (GMT_GRDIO_BAD_VAL);
			break;
	}
	return (header->type);
}

GMT_LONG GMT_native_write_grd_header (FILE *fp, struct GRD_HEADER *header)
{
	GMT_LONG err = GMT_NOERROR;
	/* Because GRD_HEADER is not 64-bit aligned we must write it in parts */

	if (GMT_fwrite (&header->nx, 3*sizeof (int), (size_t)1, fp) != 1 || GMT_fwrite (header->wesn, sizeof (struct GRD_HEADER) - ((GMT_LONG)header->wesn - (GMT_LONG)&header->nx), (size_t)1, fp) != 1)
		err = GMT_GRDIO_WRITE_FAILED;
	return (err);
}

GMT_LONG GMT_native_skip_grd_header (FILE *fp, struct GRD_HEADER *header)
{
	GMT_LONG err = GMT_NOERROR;
	/* Because GRD_HEADER is not 64-bit aligned we must estimate the # of bytes in parts */

	if (GMT_fseek (fp, (long)(3*sizeof (int) + sizeof (struct GRD_HEADER) - ((GMT_LONG)header->wesn - (GMT_LONG)&header->nx)), SEEK_SET))
		err = GMT_GRDIO_SEEK_FAILED;
	return (err);
}

GMT_LONG GMT_bit_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col, first_row, last_row, word, bit, err;
	GMT_LONG i, j, j2, width_in, width_out, height_in, i_0_out, inc, off, mx;
	GMT_LONG *k = NULL, kk, ij, piping = FALSE, check = FALSE;
	FILE *fp = NULL;
	unsigned int *tmp = NULL, ival;

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) != NULL) {	/* Skip header */
		GMT_err_trap (GMT_native_skip_grd_header (fp, header));
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	check = !GMT_is_dnan (header->nan_value);
	mx = (GMT_LONG) ceil (header->nx / 32.0);	/* Whole multiple of 32-bit integers */

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	width_out *= inc;			/* Possibly twice is complex is TRUE */
	i_0_out = inc * pad[XLO] + off;		/* Edge offset in output */

	tmp = GMT_memory (C, NULL, mx, unsigned int);

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (GMT_fread ( tmp, sizeof (unsigned int), (size_t)mx, fp) < (size_t)mx) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek by it */
		if (GMT_fseek (fp, (long) (first_row * mx * sizeof (unsigned int)), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (GMT_fread ( tmp, sizeof (unsigned int), (size_t)mx, fp) < (size_t)mx) return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		ij = (j2 + pad[YHI]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			word = k[i] / 32;
			bit = k[i] % 32;
			ival = (tmp[word] >> bit) & 1;
			grid[kk] = (float) ival;
			if (check && grid[kk] == header->nan_value) grid[kk] = C->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z min/max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip data by reading it */
		for (j = last_row + 1; j < header->ny; j++) if (GMT_fread ( tmp, sizeof (unsigned int), (size_t)mx, fp) < (size_t)mx) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);
	return (GMT_NOERROR);
}

GMT_LONG GMT_bit_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		If 64 is added we write no header*/

	GMT_LONG j, j2, width_in, width_out, height_out, mx, word, bit, err, inc, off;
	GMT_LONG i, i2, first_col, last_col, first_row, last_row, *k = NULL;
	GMT_LONG kk, ij, check = FALSE, do_header;
	unsigned int *tmp = NULL, ival;

	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	check = !GMT_is_dnan (header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	do_header = GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			ij = inc * (j2 * width_in + i2) + off;
			if (GMT_is_fnan (grid[ij])) {
				if (check) grid[ij] = (float)header->nan_value;
			}
			else {
				ival = (unsigned int) irint ((double)grid[ij]);
				if (ival > 1) ival = 1;	/* Truncate to 1 */
				header->z_min = MIN (header->z_min, (double)ival);
				header->z_max = MAX (header->z_max, (double)ival);
			}
		}
	}

	/* Store header information and array */

	if (do_header) GMT_err_trap (GMT_native_write_grd_header (fp, header));

	mx = (GMT_LONG) ceil (width_out / 32.0);
	tmp = GMT_memory (C, NULL, mx, unsigned int);

	i2 = first_col + pad[XLO];
	for (j = 0, j2 = first_row + pad[YHI]; j < height_out; j++, j2++) {
		GMT_memset (tmp, mx, unsigned int);
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) {
			kk = inc * (ij + k[i]) + off;
			word = i / 32;
			bit = i % 32;
			ival = (unsigned int) irint ((double)grid[kk]);
			if (ival > 1) ival = 1;	/* Truncate to 1 */
			tmp[word] |= (ival << bit);
		}
		if (GMT_fwrite (tmp, sizeof (unsigned int), (size_t)mx, fp) < (size_t)mx) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);

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
 * problem on 64-bit systems, where the GRD_HEADER structure was
 * automatically padded with 4-bytes before the doubles. Taking
 * advantage of the code developed to deal with the 32/64-bit anomaly
 * (below), we have permanently added a 4-byte integer to the
 * GRD_HEADER structure, but skip it when reading or writing the
 * header.
 */

GMT_LONG GMT_native_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	/* Write GRD header structure to native binary file.  This is used by
	 * all the native binary formats in GMT */

	GMT_LONG err;
	FILE *fp = NULL;

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb+")) == NULL && (fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	GMT_err_trap (GMT_native_write_grd_header (fp, header));

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_native_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG height_in;		/* Number of columns in subregion */
	GMT_LONG inc, off;		/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary), and offset */
	GMT_LONG err, i, j, j2, i_0_out;	/* Misc. counters */
	GMT_LONG *k = NULL;		/* Array with indices */
	GMT_LONG type;			/* Data type */
	GMT_LONG width_in;		/* Number of items in one row of the subregion */
	GMT_LONG kk, ij;
	GMT_LONG width_out;		/* Width of row as return (may include padding) */
	GMT_LONG piping = FALSE;	/* TRUE if we read input pipe instead of from file */
	GMT_LONG check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	size_t size;			/* Length of data type */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for reading in rows of data */

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) != NULL)	{	/* Skip header */
		GMT_err_trap (GMT_native_skip_grd_header (fp, header));
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	type = C->session.grdformat[header->type][1];
	size = GMT_grd_data_size (C, header->type, &header->nan_value);
	check = !GMT_is_dnan (header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	width_out *= inc;			/* Possibly twice is complex is TRUE */
	i_0_out = inc * pad[XLO] + off;		/* Edge offset in output */

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = GMT_memory (C, NULL, header->nx * size, char);

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) if (GMT_fread (tmp, size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek over it */
		if (GMT_fseek (fp, (long) (first_row * header->nx * size), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		if (GMT_fread (tmp, (size_t)size, (size_t)header->nx, fp) < (size_t)header->nx) {
			i = 0;
			return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		}
		ij = (j2 + pad[YHI]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = GMT_decode (C, tmp, k[i], type);	/* Convert whatever to float */
			if (check && grid[kk] == header->nan_value) grid[kk] = C->session.f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			/* Update z_min, z_max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < header->ny; j++) if (GMT_fread (tmp, (size_t)size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_native_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */
	/*		If 64 is added we write no header */

	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG width_out;		/* Width of row as return (may include padding) */
	GMT_LONG height_out;		/* Number of columns in subregion */
	GMT_LONG inc;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	GMT_LONG off;			/* Offset in complex array: 0 for real part, 1 for imaginary */
	GMT_LONG i, j, i2, j2, err;	/* Misc. counters */
	GMT_LONG *k = NULL;		/* Array with indices */
	GMT_LONG type;			/* Data type */
	GMT_LONG width_in;		/* Number of items in one row of the subregion */
	GMT_LONG ij;
	GMT_LONG check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	GMT_LONG do_header = TRUE;	/* TRUE if we should write the header first */
	size_t size;			/* Length of data type */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for writing in rows of data */

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	type = C->session.grdformat[header->type][1];
	size = GMT_grd_data_size (C, header->type, &header->nan_value);
	check = !GMT_is_dnan (header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	do_header = GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			ij = (j2 * width_in + i2) * inc + off;
			if (GMT_is_fnan (grid[ij])) {
				if (check) grid[ij] = (float)header->nan_value;
			}
			else {
				header->z_min = MIN (header->z_min, (double)grid[ij]);
				header->z_max = MAX (header->z_max, (double)grid[ij]);
			}
		}
	}

	/* Round off to chosen type */

	if (type != 'f' && type != 'd') {
		header->z_min = irint (header->z_min);
		header->z_max = irint (header->z_max);
	}

	/* Store header information and array */

	if (do_header) GMT_err_trap (GMT_native_write_grd_header (fp, header));

	/* Allocate memory for one row of data (for writing purposes) */

	tmp = GMT_memory (C, NULL, header->nx * size, char);

	i2 = first_col + pad[XLO];
	for (j = 0, j2 = first_row + pad[YHI]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_encode (C, tmp, i, grid[inc*(ij+k[i])+off], type);
		if (GMT_fwrite (tmp, (size_t)size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_free (C, k);
	GMT_free (C, tmp);

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

void GMT_encode (struct GMT_CTRL *C, void *vptr, GMT_LONG k, float z, GMT_LONG type)
{	/* Place the z value in the array location of the (type) pointer */
	switch (type) {
		case 'b':
			((char *)vptr)[k] = (char)irint ((double)z);
			break;
		case 's':
			((short int *)vptr)[k] = (short int)irint ((double)z);
			break;
		case 'i':
		case 'm':
			((int *)vptr)[k] = (int)irint ((double)z);
			break;
		case 'f':
			((float *)vptr)[k] = z;
			break;
		case 'd':
			((double *)vptr)[k] = (double)z;
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "GMT: Bad call to GMT_encode\n");
			break;
	}
}

float GMT_decode (struct GMT_CTRL *C, void *vptr, GMT_LONG k, GMT_LONG type)
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
			GMT_report (C, GMT_MSG_FATAL, "GMT: Bad call to GMT_decode\n");
			fval = C->session.f_NaN;
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

GMT_LONG GMT_is_srf_grid (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	char id[5];
	if (!strcmp (header->name, "=")) return (GMT_GRDIO_PIPE_CODECHECK);	/* Cannot check on pipes */
	if ((fp = GMT_fopen (C, header->name, "rb")) == NULL) return (GMT_GRDIO_OPEN_FAILED);
	if (GMT_fread (id, sizeof (char), (size_t)4, fp) < (size_t)4) return (GMT_GRDIO_READ_FAILED);
	GMT_fclose (C, fp);
	if (!strncmp (id, "DSBB", (size_t)4))
		header->type = GMT_GRD_IS_SF;
	else if (!strncmp (id, "DSRB", (size_t)4))
		header->type = GMT_GRD_IS_SD;
	else
		return (GMT_GRDIO_BAD_VAL);	/* Neither */
	return (header->type);
}

GMT_LONG GMT_read_srfheader6 (FILE *fp, struct srf_header6 *h)
{
	/* Reads the header of a Surfer 6 gridfile */
	/* if (GMT_fread (h, sizeof (struct srf_header6), (size_t)1, fp) < (size_t)1) return (GMT_GRDIO_READ_FAILED); */

	/* UPDATE: Because srf_header6 is not 64-bit aligned we must read it in parts */
	if (GMT_fread (h->id, 4*sizeof (char), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->nx, 2*sizeof (short int), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (h->wesn, sizeof (struct srf_header6) - ((GMT_LONG)h->wesn - (GMT_LONG)h->id), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);

	return (GMT_NOERROR);
}

GMT_LONG GMT_read_srfheader7 (FILE *fp, struct srf_header7 *h)
{
	/* Reads the header of a Surfer 7 gridfile */

	if (GMT_fseek (fp, 3*sizeof(int), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);	/* skip the first 12 bytes */
	/* if (GMT_fread (h, sizeof (struct srf_header7), (size_t)1, fp) < (size_t)1) return (GMT_GRDIO_READ_FAILED); */

	/* UPDATE: Because srf_header6 is not 64-bit aligned we must read it in parts */
	if (GMT_fread (h->id2, 4*sizeof (char), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->len_g, 3*sizeof (int), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->x_min, 8*sizeof (double), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (h->id3, 4*sizeof (char), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fread (&h->len_d, sizeof (int), (size_t)1, fp) != 1) return (GMT_GRDIO_READ_FAILED);

	return (GMT_NOERROR);
}

GMT_LONG GMT_write_srfheader (FILE *fp, struct srf_header6 *h)
{
	/* if (GMT_fwrite (h, sizeof (struct srf_header6), (size_t)1, fp) < (size_t)1) return (GMT_GRDIO_WRITE_FAILED); */
	/* UPDATE: Because srf_header6 is not 64-bit aligned we must write it in parts */
	if (GMT_fwrite (h->id, 4*sizeof (char), (size_t)1, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite (&h->nx, 2*sizeof (short int), (size_t)1, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);
	if (GMT_fwrite (h->wesn, sizeof (struct srf_header6) - ((GMT_LONG)h->wesn - (GMT_LONG)h->id), (size_t)1, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);
	return (GMT_NOERROR);
}

GMT_LONG GMT_srf_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	struct srf_header6 h6;
	struct srf_header7 h7;
	char id[5];

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) == NULL)
		return (GMT_GRDIO_OPEN_FAILED);

	if (GMT_fread (id, sizeof (char), (size_t)4, fp) < (size_t)4) return (GMT_GRDIO_READ_FAILED);
	if (GMT_fseek(fp, 0, SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
	if (strncmp (id, "DSBB", (size_t)4) && strncmp (id, "DSRB", (size_t)4)) return (GMT_GRDIO_NOT_SURFER);

	GMT_memset (&h6, 1, struct srf_header6);
	GMT_memset (&h7, 1, struct srf_header7);
	if (!strncmp (id, "DSBB", (size_t)4)) {		/* Version 6 format */
		if (GMT_read_srfheader6 (fp, &h6)) return (GMT_GRDIO_READ_FAILED);
		header->type = GMT_GRD_IS_SF;
	}
	else {					/* Version 7 format */
		if (GMT_read_srfheader7 (fp, &h7))  return (GMT_GRDIO_READ_FAILED);
		if (h7.len_d != h7.nx * h7.ny * 8 || !strcmp (h7.id2, "GRID")) return (GMT_GRDIO_SURF7_UNSUPPORTED);
		header->type = GMT_GRD_IS_SD;
	}

	GMT_fclose (C, fp);

	header->registration = GMT_GRIDLINE_REG;	/* Grid node registration */
	if (header->type == GMT_GRD_IS_SF) {
		strcpy (header->title, "Grid originally in Surfer 6 format");
		header->nx = (int)h6.nx;		header->ny = (int)h6.ny;
		header->wesn[XLO] = h6.wesn[XLO];	header->wesn[XHI] = h6.wesn[XHI];
		header->wesn[YLO] = h6.wesn[YLO];	header->wesn[YHI] = h6.wesn[YHI];
		header->z_min = h6.z_min;		header->z_max = h6.z_max;
		header->inc[GMT_X] = GMT_get_inc (C, h6.wesn[XLO], h6.wesn[XHI], h6.nx, header->registration);
		header->inc[GMT_Y] = GMT_get_inc (C, h6.wesn[YLO], h6.wesn[YHI], h6.ny, header->registration);
	}
	else {			/* Format GMT_GRD_IS_SD */
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

GMT_LONG GMT_srf_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header)
{
	FILE *fp = NULL;
	struct srf_header6 h;

	if (!strcmp (header->name, "="))	/* Write to pipe */
	{
#ifdef SET_IO_MODE
	GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "rb+")) == NULL && (fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	strncpy (h.id, "DSBB", (size_t)4);
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	if (header->registration == GMT_PIXEL_REG) {
		h.wesn[XLO] = header->wesn[XLO] + header->inc[GMT_X]/2.0;	 h.wesn[XHI] = header->wesn[XHI] - header->inc[GMT_X]/2.0;
		h.wesn[YLO] = header->wesn[YLO] + header->inc[GMT_Y]/2.0;	 h.wesn[YHI] = header->wesn[YHI] - header->inc[GMT_Y]/2.0;
	}
	else
		GMT_memcpy (h.wesn, header->wesn, 4, double);
	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (GMT_write_srfheader (fp, &h)) return (GMT_GRDIO_WRITE_FAILED);

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_srf_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG width_in;		/* Number of items in one row of the subregion */
	GMT_LONG height_in;		/* Number of columns in subregion */
	GMT_LONG inc, off;		/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary), and offset */
	GMT_LONG i, j, j2, i_0_out; 	/* Misc. counters */
	GMT_LONG *k = NULL;		/* Array with indices */
	GMT_LONG type;			/* Data type */
	GMT_LONG kk, ij;
	GMT_LONG width_out;		/* Width of row as return (may include padding) */
	GMT_LONG piping = FALSE;	/* TRUE if we read input pipe instead of from file */
	size_t size;			/* Length of data type */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for reading in rows of data */
	header->nan_value = 0.1701410e39;	/* Test value in Surfer grids */

	if (!strcmp (header->name, "=")) {	/* Read from pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_IN);
#endif
		fp = C->session.std[GMT_IN];
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (C, header->name, "rb")) != NULL) {	/* Skip header */
		if (header->type == GMT_GRD_IS_SF) {	/* Surfer Version 6 */
			if (GMT_fseek (fp, (long) sizeof (struct srf_header6), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
		}
		else {			/* Version 7  (skip also the first 12 bytes) */
			if (GMT_fseek (fp, (long) (3*sizeof(int) + sizeof (struct srf_header7)), SEEK_SET)) return (GMT_GRDIO_SEEK_FAILED);
		}
	}
	else
		return (GMT_GRDIO_OPEN_FAILED);

	type = C->session.grdformat[header->type][1];
	size = GMT_grd_data_size (C, header->type, &header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_out = width_in;		/* Width of output array */
	if (pad[XLO] > 0) width_out += pad[XLO];
	if (pad[XHI] > 0) width_out += pad[XHI];

	width_out *= inc;			/* Possibly twice is complex is TRUE */
	i_0_out = inc * pad[XLO] + off;		/* Edge offset in output */

	if ( (last_row - first_row + 1) != header->ny) {    /* We have a sub-region */
		/* Surfer grids are stored starting from Lower Left, which is contrary to
		   the rest of GMT grids that start at Top Left. So we must do a flip here */
		first_row = header->ny - height_in - first_row;
		last_row = first_row + height_in - 1;
	}

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = GMT_memory (C, NULL, header->nx * size, char);

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++)
			if (GMT_fread (tmp, size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
	}
	else {		/* Simply seek over it */
		if (GMT_fseek (fp, (long) (first_row * header->nx * size), SEEK_CUR)) return (GMT_GRDIO_SEEK_FAILED);
	}

	for (j = first_row, j2 = height_in-1; j <= last_row; j++, j2--) {
		if (GMT_fread (tmp, size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);	/* Get one row */
		ij = (j2 + pad[YHI]) * width_out + i_0_out;
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = GMT_decode (C, tmp, k[i], type);	/* Convert whatever to float */
			if (grid[kk] >= header->nan_value) grid[kk] = C->session.f_NaN;
			/* Update z_min, z_max */
			header->z_min = MIN (header->z_min, (double)grid[kk]);
			header->z_max = MAX (header->z_max, (double)grid[kk]);
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < header->ny; j++)
			if (GMT_fread (tmp, size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_READ_FAILED);
	}

	header->nx = (int)width_in;
	header->ny = (int)height_in;
	GMT_memcpy (header->wesn, wesn, 4, double);

	GMT_fclose (C, fp);

	GMT_free (C, k);
	GMT_free (C, tmp);

	return (GMT_NOERROR);
}

GMT_LONG GMT_srf_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode)
{	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* wesnn:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	GMT_LONG first_col, last_col;	/* First and last column to deal with */
	GMT_LONG first_row, last_row;	/* First and last row to deal with */
	GMT_LONG width_out;		/* Width of row as return (may include padding) */
	GMT_LONG height_out;		/* Number of columns in subregion */
	GMT_LONG inc;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	GMT_LONG off;			/* Offset in complex array: 0 for real part, 1 for imaginary */
	GMT_LONG i, j, i2, j2;		/* Misc. counters */
	GMT_LONG *k = NULL;		/* Array with indices */
	GMT_LONG type;			/* Data type */
	GMT_LONG ij;
	GMT_LONG width_in;		/* Number of items in one row of the subregion */
	size_t size;			/* Length of data type */
	FILE *fp = NULL;		/* File pointer to data or pipe */
	void *tmp = NULL;		/* Array pointer for writing in rows of data */
	struct srf_header6 h;

	header->nan_value = 0.1701410e39;	/* Test value in Surfer grids */

	if (!strcmp (header->name, "=")) {	/* Write to pipe */
#ifdef SET_IO_MODE
		GMT_setmode (C, GMT_OUT);
#endif
		fp = C->session.std[GMT_OUT];
	}
	else if ((fp = GMT_fopen (C, header->name, "wb")) == NULL)
		return (GMT_GRDIO_CREATE_FAILED);

	type = C->session.grdformat[header->type][1];
	size = GMT_grd_data_size (C, header->type, &header->nan_value);

	GMT_err_pass (C, GMT_grd_prep_io (C, header, wesn, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row, &k), header->name);
	(void)GMT_init_complex (complex_mode, &inc, &off);	/* Set stride and offset if complex */

	width_in = width_out;		/* Physical width of input array */
	if (pad[XLO] > 0) width_in += pad[XLO];
	if (pad[XHI] > 0) width_in += pad[XHI];

	GMT_memcpy (header->wesn, wesn, 4, double);

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[YHI]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[XLO]; i <= last_col; i++, i2++) {
			ij = inc * (j2 * width_in + i2) + off;
			if (GMT_is_fnan (grid[ij]))
				grid[ij] = (float)header->nan_value;
			else {
				header->z_min = MIN (header->z_min, (double)grid[ij]);
				header->z_max = MAX (header->z_max, (double)grid[ij]);
			}
		}
	}

	/* store header information and array */

	strncpy (h.id, "DSBB", (size_t)4);
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	if (header->registration == GMT_PIXEL_REG) {
		h.wesn[XLO] = header->wesn[XLO] + header->inc[GMT_X]/2;	 h.wesn[XHI] = header->wesn[XHI] - header->inc[GMT_X]/2;
		h.wesn[YLO] = header->wesn[YLO] + header->inc[GMT_Y]/2;	 h.wesn[YHI] = header->wesn[YHI] - header->inc[GMT_Y]/2;
	}
	else
		GMT_memcpy (h.wesn, header->wesn, 4, double);

	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (GMT_fwrite (&h, sizeof (struct srf_header6), (size_t)1, fp) != 1) return (GMT_GRDIO_WRITE_FAILED);

	/* Allocate memory for one row of data (for writing purposes) */

	tmp = GMT_memory (C, NULL, header->nx * size, char);

	i2 = first_col + pad[XLO];
	for (j = 0, j2 = last_row + pad[YHI]; j < height_out; j++, j2--) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_encode (C, tmp, i, grid[inc*(ij+k[i])+off], type);
		if (GMT_fwrite (tmp, (size_t)size, (size_t)header->nx, fp) < (size_t)header->nx) return (GMT_GRDIO_WRITE_FAILED);
	}

	GMT_free (C, k);
	GMT_free (C, tmp);

	GMT_fclose (C, fp);

	return (GMT_NOERROR);
}

#ifdef USE_GDAL
#include "gmt_gdalread.c"
#include "gmt_gdalwrite.c"
/* Experimental GDAL support */
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

GMT_LONG GMT_gdal_read_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header) {
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;

	if (!strcmp (header->name, "=")) {
		GMT_report (C, GMT_MSG_FATAL, "Pipes cannot be used within the GDAL interface.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	/* Allocate new control structures */
	to_gdalread = GMT_memory (C, NULL, 1, struct GDALREAD_CTRL);
	from_gdalread = GMT_memory (C, NULL, 1, struct GD_CTRL);

	to_gdalread->M.active = TRUE;		/* Metadata only */
	if (GMT_gdalread (C, header->name, to_gdalread, from_gdalread)) {
		GMT_report (C, GMT_MSG_FATAL, "ERROR reading file with gdalread.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	header->type = GMT_GRD_IS_GD;
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

	GMT_free (C, to_gdalread);
	GMT_free (C, from_gdalread->ColorMap);
	GMT_free (C, from_gdalread->band_field_names);
	GMT_free (C, from_gdalread);

	return (GMT_NOERROR);
}

GMT_LONG GMT_gdal_write_grd_info (struct GMT_CTRL *C, struct GRD_HEADER *header) {
	return (GMT_NOERROR);
}

GMT_LONG GMT_gdal_read_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode) {
	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* wesn:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex_mode:	1|2 if complex array is to hold real (1) and imaginary (2) parts (0 = read as real only) */
	/*		Note: The file has only real values, we simply allow space in the complex array */
	/*		for real and imaginary parts when processed by grdfft etc. */

	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;
	GMT_LONG i, j, nBand, subset;
	char strR[128];

	/* Allocate new control structures */
	to_gdalread = GMT_memory (C, NULL, 1, struct GDALREAD_CTRL);
	from_gdalread = GMT_memory (C, NULL, 1, struct GD_CTRL);

	if (complex_mode) {
		to_gdalread->Z.active = TRUE;		/* Force reading into a compex array */
		to_gdalread->Z.complex = (int)complex_mode;
	}

	subset = GMT_is_subset (C, header, wesn);	/* We have a Sub-region demand */
	if (subset) {	/* We have a Sub-region demand */
		to_gdalread->R.active = TRUE;
		sprintf(strR, "%.10f/%.10f/%.10f/%.10f", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
		to_gdalread->R.region = strR;
	}
	if (pad[XLO] > 0) {	/* Here we assume that all pad[0] ... pad[3] are equal. Otherwise ... */
		to_gdalread->p.active = TRUE;
		to_gdalread->p.pad = (int)pad[XLO];
	}
	if (header->pocket) {	/* Have a band request. */
		to_gdalread->B.active = TRUE;
		to_gdalread->B.bands = header->pocket;		/* Band parsing and error testing is done in gmt_gdalread */
	}

	/* Tell gmt_gdalread that we already have the memory allocated and send in the *grid pointer */
	to_gdalread->f_ptr.active = TRUE;
	to_gdalread->f_ptr.grd = grid;

	if (GMT_gdalread (C, header->name, to_gdalread, from_gdalread)) {
		GMT_report (C, GMT_MSG_FATAL, "ERROR reading file with gdalread.\n");
		return (GMT_GRDIO_OPEN_FAILED);
	}

	if (subset) {	/* We had a Sub-region demand */
		header->nx = from_gdalread->RasterXsize;
		header->ny = from_gdalread->RasterYsize;
		GMT_memcpy (header->wesn, from_gdalread->hdr, 4, double);
		header->z_min = from_gdalread->hdr[4];
		header->z_max = from_gdalread->hdr[5];
	}

	header->registration = (int)from_gdalread->hdr[6];	/* Confirm registration. It may not be the same as reported by read_grd_info */

	if (from_gdalread->Float.active) {
		if (!to_gdalread->f_ptr.active)
			grid = GMT_memcpy (grid, from_gdalread->Float.data, header->size, float);
	}
	else {
		/* Convert everything else do float */
		nBand = 0;		/* Need a solution to RGB or multiband files */
		i = nBand * header->nm;
		if (from_gdalread->UInt8.active)
			for (j = 0; j < header->nm; j++)
				grid[j] = (float)from_gdalread->UInt8.data[j+i];
		else if (from_gdalread->UInt16.active)
			for (j = 0; j < header->nm; j++)
				grid[j] = (float)from_gdalread->UInt16.data[j+i];
		else if (from_gdalread->Int16.active)
			for (j = 0; j < header->nm; j++)
				grid[j] = (float)from_gdalread->Int16.data[j+i];
		else if (from_gdalread->Int32.active)
			for (j = 0; j < header->nm; j++)
				grid[j] = (float)from_gdalread->Int32.data[j+i];
		else {
			GMT_report (C, GMT_MSG_FATAL, "ERROR data type not suported with gdalread in gmt_customio.\n");
			return (GMT_GRDIO_OPEN_FAILED);
		}
	}

	if (from_gdalread->nodata != 0) {	/* Data has a nodata value */
		if (!GMT_is_dnan (from_gdalread->nodata)) {
			for (j = 0; j < header->nm; j++)
				if (grid[j] == header->nan_value) grid[j] = C->session.f_NaN;
		}
	}
	header->nan_value = C->session.f_NaN;

	if (from_gdalread->UInt8.active)
		GMT_free (C, from_gdalread->UInt8.data);
	else if ( from_gdalread->Float.active && !to_gdalread->f_ptr.active )	/* Do not release the *grid pointer */
		GMT_free (C, from_gdalread->Float.data);
	else if (from_gdalread->UInt16.active)
		GMT_free (C, from_gdalread->UInt16.data);
	else if (from_gdalread->Int16.active)
		GMT_free (C, from_gdalread->Int16.data);
	else if (from_gdalread->Int32.active)
		GMT_free (C, from_gdalread->Int32.data);

	GMT_free (C, to_gdalread);
	GMT_free (C, from_gdalread->ColorMap);
	for ( i = 0; i < from_gdalread->RasterCount; ++i )
		free(from_gdalread->band_field_names[i].DataType);	/* Those were allocated with strdup */
	GMT_free (C, from_gdalread->band_field_names);
	GMT_free (C, from_gdalread);

	return (GMT_NOERROR);
}

GMT_LONG GMT_gdal_write_grd (struct GMT_CTRL *C, struct GRD_HEADER *header, float *grid, double wesn[], GMT_LONG *pad, GMT_LONG complex_mode) {
	GMT_report (C, GMT_MSG_FATAL, "Error: Unable to write gdal files.\n");
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

void GMT_grdio_init (struct GMT_CTRL *C) {
	GMT_LONG id = -1;

	/* FORMAT: GMT netCDF-based (byte) grdio (COARDS compliant) */

	id++;
	C->session.grdcode[id]    = 15;
	C->session.grdformat[id]  = "nb = GMT netCDF format (byte)   (COARDS-compliant)";
	C->session.readinfo[id]   = (PFL) GMT_nc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_nc_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_nc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_nc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (short) grdio (COARDS compliant) */

	id++;
	C->session.grdcode[id]    = 16;
	C->session.grdformat[id]  = "ns = GMT netCDF format (short)  (COARDS-compliant)";
	C->session.readinfo[id]   = (PFL) GMT_nc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_nc_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_nc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_nc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (int) grdio (COARDS compliant) */

	id++;
	C->session.grdcode[id]    = 17;
	C->session.grdformat[id]  = "ni = GMT netCDF format (int)    (COARDS-compliant)";
	C->session.readinfo[id]   = (PFL) GMT_nc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_nc_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_nc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_nc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (float) grdio (COARDS compliant) */

	id++;
	C->session.grdcode[id]    = 18;
	C->session.grdformat[id]  = "nf = GMT netCDF format (float)  (COARDS-compliant)";
	C->session.readinfo[id]   = (PFL) GMT_nc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_nc_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_nc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_nc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (double) grdio (COARDS compliant) */

	id++;
	C->session.grdcode[id]    = 19;
	C->session.grdformat[id]  = "nd = GMT netCDF format (double) (COARDS-compliant)";
	C->session.readinfo[id]   = (PFL) GMT_nc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_nc_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_nc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_nc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_nc_write_grd;

	/* FORMAT: GMT netCDF-based (byte) grdio */

	id++;
	C->session.grdcode[id]    = 7;
	C->session.grdformat[id]  = "cb = GMT netCDF format (byte) (deprecated)";
	C->session.readinfo[id]   = (PFL) GMT_cdf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_cdf_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_cdf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_cdf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (short) grdio */

	id++;
	C->session.grdcode[id]    = 8;
	C->session.grdformat[id]  = "cs = GMT netCDF format (short) (deprecated)";
	C->session.readinfo[id]   = (PFL) GMT_cdf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_cdf_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_cdf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_cdf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (int) grdio */

	id++;
	C->session.grdcode[id]    = 9;
	C->session.grdformat[id]  = "ci = GMT netCDF format (int) (deprecated)";
	C->session.readinfo[id]   = (PFL) GMT_cdf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_cdf_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_cdf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_cdf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (float) grdio */

	id++;
	C->session.grdcode[id]    = 10;
	C->session.grdformat[id]  = "cf = GMT netCDF format (float) (deprecated)";
	C->session.readinfo[id]   = (PFL) GMT_cdf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_cdf_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_cdf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_cdf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_cdf_write_grd;

	/* FORMAT: GMT netCDF-based (double) grdio */

	id++;
	C->session.grdcode[id]    = 11;
	C->session.grdformat[id]  = "cd = GMT netCDF format (double) (deprecated)";
	C->session.readinfo[id]   = (PFL) GMT_cdf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_cdf_update_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_cdf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_cdf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_cdf_write_grd;

	/* FORMAT: GMT native binary (bit) grdio */

	id++;
	C->session.grdcode[id]    = 5;
	C->session.grdformat[id]  = "bm = GMT native, C-binary format (bit-mask)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_bit_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_bit_write_grd;

	/* FORMAT: GMT native binary (byte) grdio */

	id++;
	C->session.grdcode[id]    = 4;
	C->session.grdformat[id]  = "bb = GMT native, C-binary format (byte)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_native_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_native_write_grd;

	/* FORMAT: GMT native binary (short) grdio */

	id++;
	C->session.grdcode[id]    = 2;
	C->session.grdformat[id]  = "bs = GMT native, C-binary format (short)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_native_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_native_write_grd;

	/* FORMAT: GMT native binary (int) grdio */

	id++;
	C->session.grdcode[id]    = 13;
	C->session.grdformat[id]  = "bi = GMT native, C-binary format (int)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_native_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_native_write_grd;

	/* FORMAT: GMT native binary (float) grdio */

	id++;
	C->session.grdcode[id]    = 1;
	C->session.grdformat[id]  = "bf = GMT native, C-binary format (float)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_native_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_native_write_grd;

	/* FORMAT: GMT native binary (double) grdio */

	id++;
	C->session.grdcode[id]    = 14;
	C->session.grdformat[id]  = "bd = GMT native, C-binary format (double)";
	C->session.readinfo[id]   = (PFL) GMT_native_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_native_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_native_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_native_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_native_write_grd;

	/* FORMAT: SUN 8-bit standard rasterfile grdio */

	id++;
	C->session.grdcode[id]    = 3;
	C->session.grdformat[id]  = "rb = SUN rasterfile format (8-bit standard)";
	C->session.readinfo[id]   = (PFL) GMT_ras_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_ras_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_ras_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_ras_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_ras_write_grd;

	/* FORMAT: NOAA NGDC MGG grid format */

	id++;
	C->session.grdcode[id]    = 12;
	C->session.grdformat[id]  = "rf = GEODAS grid format GRD98 (NGDC)";
	C->session.readinfo[id]   = (PFL) GMT_mgg2_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_mgg2_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_mgg2_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_mgg2_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_mgg2_write_grd;

	/* FORMAT: GMT native binary (float) grdio (Surfer format) */

	id++;
	C->session.grdcode[id]    = 6;
	C->session.grdformat[id]  = "sf = Golden Software Surfer format 6 (float)";
	C->session.readinfo[id]   = (PFL) GMT_srf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_srf_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_srf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_srf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_srf_write_grd;

	/* FORMAT: GMT native binary (double) grdio (Surfer format) */

	id++;
	C->session.grdcode[id]    = 20;
	C->session.grdformat[id]  = "sd = Golden Software Surfer format 7 (double, read-only)";
	C->session.readinfo[id]   = (PFL) GMT_srf_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_srf_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_srf_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_srf_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_srf_write_grd;

	/* FORMAT: GMT native binary (float) grdio (AGC format) */

	id++;
	C->session.grdcode[id]    = 21;
	C->session.grdformat[id]  = "af = Atlantic Geoscience Center format AGC (float)";
	C->session.readinfo[id]   = (PFL) GMT_agc_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_agc_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_agc_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_agc_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_agc_write_grd;

	/* FORMAT: ESRI Arc/Info ASCII Interchange Grid format (integer) */

	id++;
	C->session.grdcode[id]    = 23;
	C->session.grdformat[id]  = "ei = ESRI Arc/Info ASCII Grid Interchange format (integer)";
	C->session.readinfo[id]   = (PFL) GMT_esri_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_esri_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_esri_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_esri_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_esri_writei_grd;

	/* FORMAT: ESRI Arc/Info ASCII Interchange Grid format (float) */

	id++;
	C->session.grdcode[id]    = 24;
	C->session.grdformat[id]  = "ef = ESRI Arc/Info ASCII Grid Interchange format (float)";
	C->session.readinfo[id]   = (PFL) GMT_esri_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_esri_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_esri_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_esri_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_esri_writef_grd;

	/* FORMAT: Import via the GDAL interface */

	id++;
	C->session.grdcode[id]    = 22;
#ifdef USE_GDAL
	C->session.grdformat[id]  = "gd = Import through GDAL (convert to float)";
	C->session.readinfo[id]   = (PFL) GMT_gdal_read_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_gdal_write_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_gdal_write_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_gdal_read_grd;
	C->session.writegrd[id]   = (PFL) GMT_gdal_write_grd;
#else
	C->session.grdformat[id]  = "gd = Import through GDAL (not supported)";
	C->session.readinfo[id]   = (PFL) GMT_dummy_grd_info;
	C->session.updateinfo[id] = (PFL) GMT_dummy_grd_info;
	C->session.writeinfo[id]  = (PFL) GMT_dummy_grd_info;
	C->session.readgrd[id]    = (PFL) GMT_dummy_grd_info;
	C->session.writegrd[id]   = (PFL) GMT_dummy_grd_info;
#endif

	/* ----------------------------------------------
	 * ADD CUSTOM FORMATS BELOW AS THEY ARE NEEDED */
}
