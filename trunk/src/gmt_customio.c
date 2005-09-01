/*--------------------------------------------------------------------
 *	$Id: gmt_customio.c,v 1.38 2005-09-01 01:57:25 remko Exp $
 *
 *	Copyright (c) 1991-2005 by P. Wessel and W. H. F. Smith
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
 *	2. increment parameter N_GRD_FORMATS in file gmt_grdio.h
 *	3. Append another entry in the gmt_customio.h file.
 *	4. Provide another entry in the share/gmtformats.d file
 *
 * Author:	Paul Wessel
 * Date:	9-SEP-1992
 * Modified:	06-DEC-2001
 * Version:	4
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

int GMT_read_rasheader (FILE *fp, struct rasterfile *h);
int GMT_write_rasheader (FILE *fp, struct rasterfile *h);
void GMT_native_read_grd_header (char *file, FILE *fp, struct GRD_HEADER *header);
void GMT_native_write_grd_header (char *file, FILE *fp, struct GRD_HEADER *header);
void GMT_native_skip_grd_header (char *file, FILE *fp, struct GRD_HEADER *header);
int GMT_native_read_grd_info (char *file, struct GRD_HEADER *header);
int GMT_native_write_grd_info (char *file, struct GRD_HEADER *header);
int GMT_native_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_native_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

void GMT_grdio_init (void) {
	int id;

	/* FORMAT # 0: DEFAULT: GMT netCDF-based (float) grdio, same as # 18 */

	id = 0;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/* FORMAT # 1: GMT native binary (float) grdio [No loop over 1 &2 due to MS bug]*/

	id = 1;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_native_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_native_write_grd;

	/* FORMAT # 2: GMT native binary (short) grdio */

	id = 2;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_native_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_native_write_grd;

	/* FORMAT # 3: SUN 8-bit standard rasterfile grdio */

	id = 3;
	GMT_io_readinfo[id]   = (PFI) GMT_ras_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_ras_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_ras_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_ras_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_ras_write_grd;

	/* FORMAT # 4: GMT native binary (byte) grdio */

	id = 4;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_native_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_native_write_grd;

	/* FORMAT # 5: GMT native binary (bit) grdio */

	id = 5;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_bit_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_bit_write_grd;

	/* FORMAT # 6: GMT native binary (float) grdio (Surfer format) */

	id = 6;
	GMT_io_readinfo[id]   = (PFI) GMT_srf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_srf_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_srf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_srf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_srf_write_grd;

	/* FORMAT # 7: GMT netCDF-based (byte) grdio */
 
	id = 7;
	GMT_io_readinfo[id]   = (PFI) GMT_cdf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_cdf_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_cdf_write_grd;

	/* FORMAT # 8: GMT netCDF-based (short) grdio */
 
	id = 8;
	GMT_io_readinfo[id]   = (PFI) GMT_cdf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_cdf_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_cdf_write_grd;

	/* FORMAT # 9: GMT netCDF-based (int) grdio */
 
	id = 9;
	GMT_io_readinfo[id]   = (PFI) GMT_cdf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_cdf_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_cdf_write_grd;

	/* FORMAT # 10: GMT netCDF-based (float) grdio */
 
	id = 10;
	GMT_io_readinfo[id]   = (PFI) GMT_cdf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_cdf_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_cdf_write_grd;

	/* FORMAT # 11: GMT netCDF-based (double) grdio */
 
	id = 11;
	GMT_io_readinfo[id]   = (PFI) GMT_cdf_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_cdf_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_cdf_write_grd;

	/* FORMAT # 12: NOAA NGDC MGG grid format */

	id = 12; 
	GMT_io_readinfo[id]   = (PFI) mgg2_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) mgg2_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) mgg2_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) mgg2_read_grd;
	GMT_io_writegrd[id]   = (PFI) mgg2_write_grd;

	/* FORMAT # 13: GMT native binary (int) grdio */

	id = 13;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_native_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_native_write_grd;

	/* FORMAT # 14: GMT native binary (double) grdio */

	id = 14;
	GMT_io_readinfo[id]   = (PFI) GMT_native_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_native_write_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_native_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_native_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_native_write_grd;

	/* FORMAT # 15: GMT netCDF-based (byte) grdio (COARDS compliant) */

	id = 15;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/* FORMAT # 16: GMT netCDF-based (short) grdio (COARDS compliant) */

	id = 16;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/* FORMAT # 17: GMT netCDF-based (int) grdio (COARDS compliant) */

	id = 17;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/* FORMAT # 18: GMT netCDF-based (float) grdio (COARDS compliant) */

	id = 18;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/* FORMAT # 19: GMT netCDF-based (double) grdio (COARDS compliant) */

	id = 19;
	GMT_io_readinfo[id]   = (PFI) GMT_nc_read_grd_info;
	GMT_io_updateinfo[id] = (PFI) GMT_nc_update_grd_info;
	GMT_io_writeinfo[id]  = (PFI) GMT_nc_write_grd_info;
	GMT_io_readgrd[id]    = (PFI) GMT_nc_read_grd;
	GMT_io_writegrd[id]   = (PFI) GMT_nc_write_grd;

	/*
	 * ----------------------------------------------
	 * ADD CUSTOM FORMATS BELOW AS THEY ARE NEEDED */
}

/* CUSTOM I/O FUNCTIONS FOR GRIDDED DATA FILES */

/*-----------------------------------------------------------
 * Format # :	3
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

int GMT_ras_read_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct rasterfile h;
	unsigned char u;
	int i;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (GMT_read_rasheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	if (h.type != 1 || h.depth != 8) {
		fprintf (stderr, "GMT Fatal Error: file %s not 8-bit standard Sun rasterfile!\n", file);
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < h.maplength; i++) fread ((void *)&u, sizeof (unsigned char *), (size_t)1, fp);	/* Skip colormap */

	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_grd_init (header, 0, (char **)NULL, FALSE);

	/* Since we have no info on boundary values, just use integer size and steps = 1 */

	header->x_min = header->y_min = 0.0;
	header->x_max = header->nx = h.width;
	header->y_max = header->ny = h.height;
	header->x_inc = header->y_inc = 1.0;
	header->node_offset = 1;	/* Pixel format */
	header->z_scale_factor = 1;	header->z_add_offset = 0;

	return (FALSE);
}

int GMT_ras_write_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct rasterfile h;

	if (!strcmp (file, "="))
	{
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.type = 1;
	h.maptype = 0;
	h.maplength = 0;

	if (GMT_write_rasheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

int GMT_ras_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int first_col, last_col, first_row, last_row, kk, inc = 1;
	int i, j, j2, ij, width_in, width_out, height_in, i_0_out, n2;
	FILE *fp;
	BOOLEAN piping = FALSE, check;
	unsigned char *tmp;
	struct rasterfile h;
	int *k;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (file, "rb")) != NULL) {	/* Skip header */
		if (GMT_read_rasheader (fp, &h)) {
			fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
			exit (EXIT_FAILURE);
		}
		if (h.maplength) fseek (fp, (long) h.maplength, SEEK_CUR);
	}
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	n2 = (int) ceil (header->nx / 2.0) * 2;	/* Sun 8-bit rasters are stored using 16-bit words */
	tmp = (unsigned char *) GMT_memory (VNULL, (size_t)n2, sizeof (unsigned char), "GMT_ras_read_grd");

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;

	check = !GMT_is_dnan (GMT_grd_in_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */
	if (complex) {	/* Need twice as much output space since we load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}

	if (piping)	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) fread ((void *) tmp, sizeof (unsigned char), (size_t)n2, fp);
	else /* Simply seek by it */
		fseek (fp, (long) (first_row * n2 * sizeof (unsigned char)), SEEK_CUR);
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		fread ((void *) tmp, sizeof (unsigned char), (size_t)n2, fp);
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = (float) tmp[k[i]];
			if (check && grid[kk] == GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
			if (GMT_is_fnan (grid[kk])) continue;
			if ((double)grid[kk] < header->z_min) header->z_min = (double)grid[kk];
			if ((double)grid[kk] > header->z_max) header->z_max = (double)grid[kk];
		}
	}
	if (piping)	/* Skip data by reading it */
		for (j = last_row + 1; j < header->ny; j++) fread ((void *) tmp, sizeof (unsigned char), (size_t)n2, fp);

	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);
	return (FALSE);
}

int GMT_ras_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to remove on w, e, s, n of grid, respectively */
	/* complex:	Must be FALSE for rasterfiles.    If 64 is added we write no header */

	int i, i2, kk, inc = 1;
	int j, ij, j2, width_in, width_out, height_out, n2;
	int first_col, last_col, first_row, last_row;
	int *k;

	BOOLEAN check, do_header = TRUE;

	unsigned char *tmp;

	FILE *fp;

	struct rasterfile h;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	h.magic = RAS_MAGIC;
	h.width = header->nx;
	h.height = header->ny;
	h.depth = 8;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.type = 1;
	h.maptype = 0;
	h.maplength = 0;

	n2 = (int) ceil (header->nx / 2.0) * 2;
	tmp = (unsigned char *) GMT_memory (VNULL, (size_t)n2, sizeof (unsigned char), "GMT_ras_write_grd");

	check = !GMT_is_dnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	if (complex >= 64) {	/* Want no header, adjust complex */
		complex %= 64;
		do_header = FALSE;
	}
	if (complex) inc = 2;

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	h.width = header->nx;
	h.height = header->ny;
	h.length = header->ny * (int) ceil (header->nx/2.0) * 2;

	/* store header information and array */

	if (do_header && GMT_write_rasheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	i2 = first_col + pad[0];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) {
			kk = inc * (ij + k[i]);
			if (check && GMT_is_fnan (grid[kk])) grid[kk] = (float)GMT_grd_out_nan_value;
			tmp[i] = (unsigned char) grid[kk];
		}
		fwrite ((void *)tmp, sizeof (unsigned char), (size_t)width_out, fp);
	}
	if (fp != GMT_stdout) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);

	return (FALSE);

}

int GMT_read_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Reads the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	int i, j, value, in[4];

	for (i = 0; i < 8; i++) {

		if (fread ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) return (-1);

		for (j = 0; j < 4; j++) in[j] = (int)byte[j];

		value = (in[0] << 24) + (in[1] << 16) + (in[2] << 8) + in[3];

		switch (i) {
			case 0:
				h->magic = value;
				break;
			case 1:
				h->width = value;
				break;
			case 2:
				h->height = value;
				break;
			case 3:
				h->depth = value;
				break;
			case 4:
				h->length = value;
				break;
			case 5:
				h->type = value;
				break;
			case 6:
				h->maptype = value;
				break;
			case 7:
				h->maplength = value;
				break;
		}
	}

	if (h->type == RT_OLD && h->length == 0) h->length = 2 * irint (ceil (h->width * h->depth / 16.0)) * h->height;

	return (0);
}

int GMT_write_rasheader (FILE *fp, struct rasterfile *h)
{
	/* Writes the header of a Sun rasterfile byte by byte
	   since the format is defined as the byte order on the
	   PDP-11.
	 */

	unsigned char byte[4];
	int i, value;

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

		if (fwrite ((void *)byte, sizeof (unsigned char), (size_t)4, fp) != 4) return (-1);
	}

	return (0);
}

/*-----------------------------------------------------------
 * Format # :	5
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

int GMT_bit_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int first_col, last_col, first_row, last_row, kk, word, bit;
	int i, j, j2, ij, width_in, width_out, height_in, i_0_out, inc = 1, mx;
	int *k;
	FILE *fp;
	BOOLEAN piping = FALSE, check = FALSE;
	unsigned int *tmp, ival;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (file, "rb")) != NULL)	/* Skip header */
	{
		GMT_native_skip_grd_header (file, fp, header);
	}
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	check = !GMT_is_dnan (GMT_grd_in_nan_value);
	mx = (int) ceil (header->nx / 32.0);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */
	if (complex) {	/* Need twice as much output space since we load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}
	tmp = (unsigned int *) GMT_memory (VNULL, (size_t)mx, sizeof (unsigned int), "GMT_bit_read_grd");

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) fread ((void *) tmp, sizeof (unsigned int), (size_t)mx, fp);
	}
	else {		/* Simply seek by it */
		fseek (fp, (long) (first_row * mx * sizeof (unsigned int)), SEEK_CUR);
	}
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		fread ((void *) tmp, sizeof (unsigned int), (size_t)mx, fp);	/* Get one row */
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			word = k[i] / 32;
			bit = k[i] % 32;
			ival = (tmp[word] >> bit) & 1;
			grid[kk] = (float) ival;
			if (check && grid[kk] == GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
		}
	}
	if (piping) {	/* Skip data by reading it */
		for (j = last_row + 1; j < header->ny; j++) fread ((void *) tmp, sizeof (unsigned int), (size_t)mx, fp);
	}

	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = 0; j < header->ny; j++) {
		for (i = 0; i < header->nx; i++) {
			ij = inc * ((j + pad[3]) * width_out + i + pad[0]);
			if (GMT_is_fnan ( grid[ij])) continue;
			if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
			if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
		}
	}
	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);
	return (FALSE);
}

int GMT_bit_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc.   If 64 is added we write no header*/

	int i, i2, kk, *k;
	int j, ij, j2, width_in, width_out, height_out, mx, word, bit, inc = 1;
	int first_col, last_col, first_row, last_row;
	BOOLEAN check = FALSE, do_header = TRUE;


	unsigned int *tmp, ival;

	FILE *fp;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	check = !GMT_is_dnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	if (complex >= 64) {	/* Want no header, adjust complex */
		complex %= 64;
		do_header = FALSE;
	}
	if (complex) inc = 2;

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[3]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[0]; i <= last_col; i++, i2++) {
			ij = inc * (j2 * width_in + i2);
			if (GMT_is_fnan (grid[ij])) {
				if (check) grid[ij] = (float)GMT_grd_out_nan_value;
			}
			else {
				ival = (unsigned int) irint ((double)grid[ij]);
				if (ival > 1) ival = 1;	/* Truncate to 1 */
				if ((double)ival < header->z_min) header->z_min = (double)ival;
				if ((double)ival > header->z_max) header->z_max = (double)ival;
			}
		}
	}

	/* Store header information and array */

	if (do_header) GMT_native_write_grd_header (file, fp, header);

	mx = (int) ceil (width_out / 32.0);
	tmp = (unsigned int *) GMT_memory (VNULL, (size_t)mx, sizeof (unsigned int), "GMT_bit_write_grd");

	i2 = first_col + pad[0];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		memset ((void *)tmp, 0, (size_t)(mx * sizeof (unsigned int)));
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) {
			kk = inc * (ij + k[i]);
			word = i / 32;
			bit = i % 32;
			ival = (unsigned int) irint ((double)grid[kk]);
			if (ival > 1) ival = 1;	/* Truncate to 1 */
			tmp[word] |= (ival << bit);
		}
		fwrite ((void *)tmp, sizeof (unsigned int), (size_t)mx, fp);
	}

	if (fp != GMT_stdout) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free ((void *)tmp);

	return (FALSE);
}

/*-----------------------------------------------------------
 * Format # :	1, 2, 4, 13, 14
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

void GMT_native_read_grd_header (char *file, FILE *fp, struct GRD_HEADER *header)
{
	/* Because GRD_HEADER is not 64-bit aligned we must read it in parts */
	
	if (fread ((void *)&header->nx, 3*sizeof (int), (size_t)1, fp) != 1 || fread ((void *)&header->x_min, sizeof (struct GRD_HEADER) - ((long)&header->x_min - (long)&header->nx), (size_t)1, fp) != 1) {
                fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
                exit (EXIT_FAILURE);
        }
}

void GMT_native_write_grd_header (char *file, FILE *fp, struct GRD_HEADER *header)
{
	/* Because GRD_HEADER is not 64-bit aligned we must write it in parts */

	if (fwrite ((void *)&header->nx, 3*sizeof (int), (size_t)1, fp) != 1 || fwrite ((void *)&header->x_min, sizeof (struct GRD_HEADER) - ((long)&header->x_min - (long)&header->nx), (size_t)1, fp) != 1) {
                fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
                exit (EXIT_FAILURE);
        }
}

void GMT_native_skip_grd_header (char *file, FILE *fp, struct GRD_HEADER *header)
{
	/* Because GRD_HEADER is not 64-bit aligned we must estimate the # of bytes in parts */
	
	if (fseek (fp, (long)(3*sizeof (int) + sizeof (struct GRD_HEADER) - ((long)&header->x_min - (long)&header->nx)), SEEK_SET)) {
		fprintf (stderr, "GMT Fatal Error: Error seeking past header file %s!\n", file);
		exit (EXIT_FAILURE);
	}
}

int GMT_native_read_grd_info (char *file, struct GRD_HEADER *header)
{
	/* Read GRD header structure from native binary file.  This is used by
	 * all the native binary formats in GMT */

	FILE *fp;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	GMT_native_read_grd_header (file, fp, header);

	if (fp != GMT_stdin) GMT_fclose (fp);

	return (FALSE);
}

int GMT_native_write_grd_info (char *file, struct GRD_HEADER *header)
{
	/* Write GRD header structure to native binary file.  This is used by
	 * all the native binary formats in GMT */

	FILE *fp;

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	GMT_native_write_grd_header (file, fp, header);

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

int GMT_native_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_in;			/* Number of columns in subregion */
	int inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	int kk, i, j, j2, ij, i_0_out;	/* Misc. counters */
	int *k;				/* Array with indices */
	int type;			/* Data type */
	int size;			/* Length of data type */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN piping = FALSE;		/* TRUE if we read input pipe instead of from file */
	BOOLEAN check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	void *tmp;			/* Array pointer for reading in rows of data */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (file, "rb")) != NULL)	/* Skip header */
		GMT_native_skip_grd_header (file, fp, header);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	type = GMT_grdformats[GMT_grd_i_format][1];
	size = GMT_grd_data_size (GMT_grd_i_format, &GMT_grd_in_nan_value);
	check = !GMT_is_dnan (GMT_grd_in_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */

	if (complex) {	/* Need twice as much output space since we load every 2nd cell */
		width_out *= 2;
		i_0_out *= 2;
		inc = 2;
	}

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = (void *) GMT_memory (VNULL, (size_t)header->nx, size, "GMT_native_read");

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) fread (tmp, size, (size_t)header->nx, fp);
	}
	else {		/* Simply seek over it */
		fseek (fp, (long) (first_row * header->nx * size), SEEK_CUR);
	}
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		fread (tmp, size, (size_t)header->nx, fp);	/* Get one row */
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = GMT_native_decode (tmp, k[i], type);	/* Convert whatever to float */
			if (check && grid[kk] == GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < header->ny; j++) fread (tmp, size, (size_t)header->nx, fp);
	}

	header->nx = width_in;
	header->ny = height_in;
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
			if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
			if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
		}
	}
	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free (tmp);

	return (FALSE);
}

int GMT_native_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc.  If 64 is added we write no header */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_out;			/* Number of columns in subregion */
	int inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	int i, j, i2, j2, ij;		/* Misc. counters */
	int *k;				/* Array with indices */
	int type;			/* Data type */
	int size;			/* Length of data type */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	BOOLEAN do_header = TRUE;	/* TRUE if we should write the header first */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	type = GMT_grdformats[GMT_grd_o_format][1];
	size = GMT_grd_data_size (GMT_grd_o_format, &GMT_grd_out_nan_value);
	check = !GMT_is_dnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	if (complex >= 64) {	/* Want no header, adjust complex */
		complex %= 64;
		do_header = FALSE;
	}
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
				if (check) grid[ij] = (float)GMT_grd_out_nan_value;
			}
			else {
				if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
				if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
			}
		}
	}

	/* Round off to chosen type */

	header->z_min = GMT_native_encode ((float)header->z_min, type);
	header->z_max = GMT_native_encode ((float)header->z_max, type);

	/* Store header information and array */

	if (do_header) GMT_native_write_grd_header (file, fp, header);

	i2 = first_col + pad[0];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_native_write_one (fp, grid[inc*(ij+k[i])], type);
	}
	GMT_free ((void *)k);
	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);

}

float GMT_native_decode (void *vptr, int k, int type)
{
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
			fprintf (stderr, "GMT: Bad call to GMT_native_decode (gmt_customio.c)\n");
			fval = GMT_f_NaN;
			break;
	}

	return (fval);
}

double GMT_native_encode (float z, int type)
{
	char c;
	short int h;
	int i;

	switch (type) {
		case 'b':
			c = (char)irint ((double)z);
			return ((double)c);
			break;
		case 's':
			h = (short int)irint ((double)z);
			return ((double)h);
			break;
		case 'm':
		case 'i':
			i = (int)irint ((double)z);
			return ((double)i);
			break;
		case 'f':
		case 'd':
			return ((double)z);
			break;
		default:
			fprintf (stderr, "GMT: Bad call to GMT_native_encode (gmt_customio.c)\n");
			return (0.0);
			break;
	}
}

size_t GMT_native_write_one (FILE *fp, float z, int type)
{
	char c;
	short int h;
	int i;
	double d;

	switch (type) {
		case 'b':
			c = (char)irint ((double)z);
			return (fwrite ((void *)&c, sizeof(char), (size_t)1, fp));
			break;
		case 's':
			h = (short int)irint ((double)z);
			return (fwrite ((void *)&h, sizeof(short int), (size_t)1, fp));
			break;
		case 'm':
		case 'i':
			i = (int)irint ((double)z);
			return (fwrite ((void *)&i, sizeof(int), (size_t)1, fp));
			break;
		case 'f':
			return (fwrite ((void *)&z, sizeof(float), (size_t)1, fp));
			break;
		case 'd':
			d = (double)z;
			return (fwrite ((void *)&d, sizeof(double), (size_t)1, fp));
			break;
		default:
			break;
	}

	return (-1);	/* Should never get here */
}

/*-----------------------------------------------------------
 * Format # :	6
 * Type :	Native binary (float) C file
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
	short int nx;		/* Number of columns */
	short int ny;		/* Number of rows */
	double x_min;		/* Minimum x coordinate */
	double x_max;		/* Maximum x coordinate */
	double y_min;		/* Minimum y coordinate */
	double y_max;		/* Maximum y coordinate */
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
   the three commented lines). This would make that the hader is composed of 2 char[4] and
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

int srf_fmt;	/* To hold which Surfer format. = 6 for format 6 or = 7 for format 7.*/

int GMT_srf_read_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct srf_header6 h6;
	struct srf_header7 h7;
	int GMT_read_srfheader6 (FILE *fp, struct srf_header6 *h);
	int GMT_read_srfheader7 (FILE *fp, struct srf_header7 *h);
	char id[5];

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = GMT_fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	fread (id, sizeof (char), (size_t)4, fp); 
	rewind(fp);
	if (strncmp (id, "DSBB",4) && strncmp (id, "DSRB",4)) {
		fprintf (stderr, "GMT Fatal Error: %s is not a valid Surfer 6|7 grid\n", file);
		exit (EXIT_FAILURE);
	}

	if (!strncmp (id, "DSBB",4)) {		/* Version 6 format */
		if (GMT_read_srfheader6 (fp, &h6)) {
			fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
			exit (EXIT_FAILURE);
		}
		srf_fmt = 6;
	}
	else {					/* Version 7 format */
		if (GMT_read_srfheader7 (fp, &h7)) {
			fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
			exit (EXIT_FAILURE);
		}
		if ( (h7.len_d != (h7.nx * h7.ny * 8)) || (!strcmp (h7.id2, "GRID")) ) {
			fprintf (stderr, "GMT Fatal Error: The %s Surfer 7 grid appears\n", file);
			fprintf (stderr, "to have break lines or otherwise it uses the full\n");
			fprintf (stderr, "extent of version 7 format. That is not supported.\n");
			exit (EXIT_FAILURE);
		}
		srf_fmt = 7;
	}

	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_grd_init (header, 0, (char **)NULL, FALSE);

	if (srf_fmt == 6) {
		strcpy (header->title, "Grid originally in Surfer 6 format");
		header->nx = (int)h6.nx;	header->ny = (int)h6.ny;
		header->x_min = h6.x_min;	header->x_max = h6.x_max;
		header->y_min = h6.y_min;	header->y_max = h6.y_max;
		header->z_min = h6.z_min;	header->z_max = h6.z_max;
		header->x_inc = (h6.x_max - h6.x_min) / (h6.nx - 1);
		header->y_inc = (h6.y_max - h6.y_min) / (h6.ny - 1);
	}
	else {			/* Format 7 */
		strcpy (header->title, "Grid originally in Surfer 7 format");
		header->nx = h7.nx;		header->ny = h7.ny;
		header->x_min = h7.x_min;	header->y_min = h7.y_min;
		header->x_max = h7.x_min + h7.x_inc * (h7.nx - 1);
		header->y_max = h7.y_min + h7.y_inc * (h7.ny - 1);
		header->z_min = h7.z_min;	header->z_max = h7.z_max;
		header->x_inc = h7.x_inc;	header->y_inc = h7.y_inc;
	}
	header->node_offset = 0;	/* Grid node registration */
	header->z_scale_factor = 1;	header->z_add_offset = 0;

	return (FALSE);
}

int GMT_srf_write_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct srf_header6 h;
	int GMT_write_srfheader (FILE *fp, struct srf_header6 *h);

	if (!strcmp (file, "="))
	{
#ifdef SET_IO_MODE
	GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	strcpy (h.id,"DSBB");
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	h.x_min = header->x_min;	 h.x_max = header->x_max;
	h.y_min = header->y_min;	 h.y_max = header->y_max;
	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (GMT_write_srfheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

int GMT_read_srfheader6 (FILE *fp, struct srf_header6 *h)
{
	/* Reads the header of a Surfer 6 gridfile */

	fread ((void *)h, sizeof (struct srf_header6), (size_t)1, fp); 
	return (0);
}

int GMT_read_srfheader7 (FILE *fp, struct srf_header7 *h)
{
	/* Reads the header of a Surfer 7 gridfile */

	fseek (fp, 3*sizeof(int), SEEK_SET);	/* skip the first 12 bytes */
	fread ((void *)h, sizeof (struct srf_header7), (size_t)1, fp);
	return (0);
}

int GMT_write_srfheader (FILE *fp, struct srf_header6 *h)
{
	fwrite ((void *)h, sizeof (struct srf_header6), (size_t)1, fp); 
	return (0);
}

int GMT_srf_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	-IGNORED- */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_in;			/* Number of columns in subregion */
	int kk, i, j, j2, ij, i_0_out; 	/* Misc. counters */
	int *k;				/* Array with indices */
	int type;			/* Data type */
	int size;			/* Length of data type */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN piping = FALSE;		/* TRUE if we read input pipe instead of from file */
	void *tmp;			/* Array pointer for reading in rows of data */
	GMT_grd_in_nan_value = 0.1701410e39;	/* Test value in Surfer grids */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_IN);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = GMT_fopen (file, "rb")) != NULL)	/* Skip header */
		if (srf_fmt == 6)	/* Version 6 */
			fseek (fp, (long) sizeof (struct srf_header6), SEEK_SET);
		else 			/* Version 7  (skip also the first 12 bytes) */
			fseek (fp, (long) (3*sizeof(int) + sizeof (struct srf_header7)), SEEK_SET);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_in, &height_in, &first_col, &last_col, &first_row, &last_row);

	width_out = width_in;		/* Width of output array */
	if (pad[0] > 0) width_out += pad[0];
	if (pad[1] > 0) width_out += pad[1];

	i_0_out = pad[0];		/* Edge offset in output */

	type = GMT_grdformats[GMT_grd_i_format][1];
	size = GMT_grd_data_size (GMT_grd_i_format, &GMT_grd_in_nan_value);

	if (srf_fmt == 7) {
		size *= 2;	/* Format uses doubles, so we must duplicate "size" */
		type = 'd';
	}

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = (void *) GMT_memory (VNULL, (size_t)header->nx, size, "GMT_srf_read_grd");

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) fread (tmp, size, (size_t)header->nx, fp);
	}
	else {		/* Simply seek over it */
		fseek (fp, (long) (first_row * header->nx * size), SEEK_CUR);
	}

	for (j = first_row, j2 = height_in-1; j <= last_row; j++, j2--) {
		fread (tmp, size, (size_t)header->nx, fp);	/* Get one row */
		ij = (j2 + pad[3]) * width_out + i_0_out;
		for (i = 0; i < width_in; i++) {
			kk = ij + i;
			grid[kk] = GMT_native_decode (tmp, k[i], type);	/* Convert whatever to float */
			if (grid[kk] >= GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < header->ny; j++) fread (tmp, size, (size_t)header->nx, fp);
	}

	header->nx = width_in;
	header->ny = height_in;
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	/* Update zmin, zmaz */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = 0; j < header->ny; j++) {
		for (i = 0; i < header->nx; i++) {
			ij = (j + pad[3]) * width_out + i + pad[0];
			if (GMT_is_fnan (grid[ij])) continue;
			if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
			if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
		}
	}

	if (fp != GMT_stdin) GMT_fclose (fp);

	GMT_free ((void *)k);
	GMT_free (tmp);

	return (FALSE);
}

int GMT_srf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	-IGNORED- */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_out;			/* Number of columns in subregion */
	int i, j, i2, j2, ij;		/* Misc. counters */
	int *k;				/* Array with indices */
	int type;			/* Data type */
	int size;			/* Length of data type */
	FILE *fp;			/* File pointer to data or pipe */
	struct srf_header6 h;

	GMT_grd_out_nan_value = 0.1701410e39;	/* Test value in Surfer grids */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (GMT_OUT);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = GMT_fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	type = GMT_grdformats[GMT_grd_o_format][1];
	size = GMT_grd_data_size (GMT_grd_o_format, &GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;

	/* Find z_min/z_max */

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;
	for (j = first_row, j2 = pad[3]; j <= last_row; j++, j2++) {
		for (i = first_col, i2 = pad[0]; i <= last_col; i++, i2++) {
			ij = (j2 * width_in + i2);
			if (GMT_is_fnan (grid[ij])) 
				grid[i] = (float)GMT_grd_out_nan_value;
			else {
				if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
				if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
			}
		}
	}

	/* store header information and array */

	strcpy (h.id,"DSBB");
	h.nx = (short int)header->nx;	 h.ny = (short int)header->ny;
	h.x_min = header->x_min;	 h.x_max = header->x_max;
	h.y_min = header->y_min;	 h.y_max = header->y_max;
	h.z_min = header->z_min;	 h.z_max = header->z_max;

	if (fwrite ((void *)&h, sizeof (struct srf_header6), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	i2 = first_col + pad[0];
	for (j = 0, j2 = last_row + pad[3]; j < height_out; j++, j2--) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_native_write_one (fp, grid[(ij+k[i])], type);
	}
	GMT_free ((void *)k);
	if (fp != GMT_stdout) GMT_fclose (fp);

	return (FALSE);
}

/* Add custom code here */

/* 12: NOAA NGDC MGG Format */
#include "gmt_mgg_header2.c"
