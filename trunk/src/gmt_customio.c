/*--------------------------------------------------------------------
 *	$Id: gmt_customio.c,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 *	Contact info: www.soest.hawaii.edu/gmt
 *--------------------------------------------------------------------*/
/*
 *
 *	G M T _ C U S T O M I O . C   R O U T I N E S
 *
 * Takes care of all custom format gridfile input/output.  The
 * industrious user may supply his/her own code to read specific data
 * formats.  Four functions must be supplied, and they must conform
 * to the GMT standard and return the same arguments as the generic
 * GMT grdio functions.  See GMT_cdf.c for details.
 *
 * To add another data format:
 *
 *	1. Write the four required routines (see below).
 *	2. increment parameter N_GRD_FORMATS in file GMT_grdio.h
 *	3. Append another entry in the GMT_customio.h file.
 *	4. Provide another entry in the share/gmtformats.d file
 *
 * Author:	Paul Wessel
 * Date:	9-SEP-1992
 * Modified:	22-AUG-2000
 * Version:	3.3.6
 *
 * Functions include:
 *
 *	GMT_*_read_grd_info :	Read header from file
 *	GMT_*_read_grd :	Read header and data set from file
 *	GMT_*_write_grd_info :	Update header in existing file
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
 *	     formats may have similar limitations)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#include "gmt.h"

#define GMT_N_NATIVE_FORMATS	6

#define GMT_NATIVE_CHAR		0
#define GMT_NATIVE_UCHAR	1
#define GMT_NATIVE_SHORT	2
#define GMT_NATIVE_INT		3
#define GMT_NATIVE_FLOAT	4
#define GMT_NATIVE_DOUBLE	5

int GMT_read_rasheader (FILE *fp, struct rasterfile *h);
int GMT_write_rasheader (FILE *fp, struct rasterfile *h);
float GMT_native_decode (void *vptr, int k, int type);
size_t GMT_native_write_one (FILE *fp, float z, int type);
int GMT_native_read_grd_info (char *file, struct GRD_HEADER *header);
int GMT_native_write_grd_info (char *file, struct GRD_HEADER *header);
int GMT_native_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, int type);
int GMT_native_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, int type);
int GMT_cdf_byte_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_cdf_char_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_cdf_short_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_cdf_int_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_cdf_float_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
int GMT_cdf_double_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

int GMT_native_size[GMT_N_NATIVE_FORMATS] = {
	sizeof(char),
	sizeof(unsigned char),
	sizeof(short int),
	sizeof(int),
	sizeof(float),
	sizeof(double)
};

char *GMT_native_name[GMT_N_NATIVE_FORMATS] = {
	"char",
	"unsigned char",
	"short int",
	"int"
	"float",
	"double"
};

void GMT_grdio_init (void) {
	int id;

	/* FORMAT # 0: Default GMT netcdf-based grdio */
	
	GMT_io_readinfo[0]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[0] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[0]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[0]  = (PFI) GMT_cdf_float_write_grd;
	
	/* FORMAT # 1: GMT native binary (float) grdio */
	
	id = 1;
	GMT_io_readinfo[id]  = (PFI) GMT_bin_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_bin_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_bin_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_bin_write_grd;
	
	/* FORMAT # 2: GMT native binary (short) grdio */
	
	id = 2;
	GMT_io_readinfo[id]  = (PFI) GMT_short_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_short_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_short_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_short_write_grd;
	
	/* FORMAT # 3: SUN 8-bit standard rasterfile grdio */
	
	id = 3;
	GMT_io_readinfo[id]  = (PFI) GMT_ras_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_ras_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_ras_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_ras_write_grd;
	
	/* FORMAT # 4: GMT native binary (uchar) grdio */
	
	id = 4;
	GMT_io_readinfo[id]  = (PFI) GMT_uchar_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_uchar_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_uchar_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_uchar_write_grd;

	/* FORMAT # 5: GMT native binary (bit) grdio */
	
	id = 5;
	GMT_io_readinfo[id]  = (PFI) GMT_bit_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_bit_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_bit_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_bit_write_grd;

	/* FORMAT # 6: GMT native binary (float) grdio (Surfer format) */
	
	id = 6;
	GMT_io_readinfo[id]  = (PFI) GMT_srf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_srf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_srf_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_srf_write_grd;

	/* FORMAT # 7: GMT netCDF-based grdio (byte) */
 	
	id = 7;
	GMT_io_readinfo[id]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_cdf_byte_write_grd;
	
	/* FORMAT # 8: GMT netCDF-based grdio (char) */
 	
	id = 8;
	GMT_io_readinfo[id]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_cdf_char_write_grd;
	
	/* FORMAT # 9: GMT netCDF-based grdio (short) */
 	
	id = 9;
	GMT_io_readinfo[id]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id]  = (PFI) GMT_cdf_short_write_grd;
	
	/* FORMAT # 10: GMT netCDF-based grdio (int) */
 	
	id = 10;
	GMT_io_readinfo[id]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id] = (PFI) GMT_cdf_int_write_grd;
	
	/* FORMAT # 11: GMT netCDF-based grdio (double) */
 	
	id = 11;
	GMT_io_readinfo[id]  = (PFI) GMT_cdf_read_grd_info;
	GMT_io_writeinfo[id] = (PFI) GMT_cdf_write_grd_info;
	GMT_io_readgrd[id]   = (PFI) GMT_cdf_read_grd;
	GMT_io_writegrd[id] = (PFI) GMT_cdf_double_write_grd;

	/*
	 * ----------------------------------------------
	 * ADD CUSTOM FORMATS BELOW AS THEY ARE NEEDED */
}

/* CUSTOM I/O FUNCTIONS FOR GRIDDED DATA FILES */

/*-----------------------------------------------------------
 * Format # :	1
 * Type :	Native binary (float) C file
 * Prefix :	GMT_bin_
 * Author :	Paul Wessel, SOEST
 * Date :	17-JUN-1999
 * Purpose:	The native binary output format is used
 *		primarily for piped i/o between programs
 *		that otherwise would use temporary, large
 *		intermediate grdfiles.  Note that not all
 *		programs can support piping (they may have
 *		to re-read the file or accept more than one
 *		grdfile).
 * Functions :	GMT_bin_read_grd_info, GMT_bin_write_grd_info,
 *		GMT_bin_read_grd, GMT_bin_write_grd
 *-----------------------------------------------------------*/
 
int GMT_bin_read_grd_info (char *file, struct GRD_HEADER *header)
{
	return (GMT_native_read_grd_info (file, header));
}

int GMT_bin_write_grd_info (char *file, struct GRD_HEADER *header)
{
	return (GMT_native_write_grd_info (file, header));
}

int GMT_bin_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_read_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_FLOAT));
}

int GMT_bin_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_write_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_FLOAT));
}

/*-----------------------------------------------------------
 * Format # :	2
 * Type :	Native binary (short) C file
 * Prefix :	GMT_short_
 * Author :	Paul Wessel, SOEST
 * Date :	17-JUN-1999
 * Purpose:	The native binary output format is used
 *		primarily for piped i/o between programs
 *		that otherwise would use temporary, large
 *		intermediate grdfiles.  Note that not all
 *		programs can support piping (they may have
 *		to re-read the file or accept more than one
 *		grdfile).  Datasets with limited range may
 *		be stored using 2-byte integers which will
 *		reduce storage space and improve throughput.
 * Functions :	GMT_short_read_grd_info, GMT_short_write_grd_info,
 *		GMT_short_read_grd, GMT_short_write_grd
 *-----------------------------------------------------------*/
 
int GMT_short_read_grd_info (char *file, struct GRD_HEADER *header)
{	
	return (GMT_native_read_grd_info (file, header));
}

int GMT_short_write_grd_info (char *file, struct GRD_HEADER *header)
{
	return (GMT_native_write_grd_info (file, header));
}

int GMT_short_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_read_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_SHORT));
}

int GMT_short_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_write_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_SHORT));
}

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
 *		Such files are always pixel registrered
 *			
 * Functions :	GMT_ras_read_grd_info, GMT_ras_write_grd_info,
 *		GMT_ras_read_grd, GMT_ras_write_grd
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
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	if (GMT_read_rasheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	if (h.ras_type != 1 || h.ras_depth != 8) {
		fprintf (stderr, "GMT Fatal Error: file %s not 8-bit standard Sun rasterfile!\n", file);
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < h.ras_maplength; i++) fread ((void *)&u, sizeof (unsigned char *), (size_t)1, fp);	/* Skip colormap */
	
	if (fp != GMT_stdin) fclose (fp);
	
	GMT_grd_init (header, 0, (char **)NULL, FALSE);

	/* Since we have no info on bounary values, just use integer size and steps = 1 */

	header->x_min = header->y_min = 0.0;
	header->x_max = header->nx = h.ras_width;
	header->y_max = header->ny = h.ras_height;
	header->x_inc = header->y_inc = 1.0;
	header->node_offset = 1;	/* Pixel format */
	
	return (FALSE);
}

int GMT_ras_write_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct rasterfile h;
	
	if (!strcmp (file, "="))
	{
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	h.ras_magic = RAS_MAGIC;
	h.ras_width = header->nx;
	h.ras_height = header->ny;
	h.ras_depth = 8;
	h.ras_length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.ras_type = 1;
	h.ras_maptype = 0;
	h.ras_maplength = 0;
	
	if (GMT_write_rasheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdout) fclose (fp);
	
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
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = fopen (file, "rb")) != NULL) {	/* Skip header */
		if (GMT_read_rasheader (fp, &h)) {
			fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
			exit (EXIT_FAILURE);
		}
		if (h.ras_maplength) fseek (fp, (long) h.ras_maplength, SEEK_CUR);
	}
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	n2 = (int) ceil (header->nx / 2.0) * 2;	/* Sun 8-bit rasters are stored using 16-bit words */
	tmp = (unsigned char *) GMT_memory (VNULL, (size_t)n2, sizeof (unsigned char), "GMT_ras_read_grd");

	header->z_min = DBL_MAX;	header->z_max = -DBL_MAX;

	check = !GMT_is_fnan (GMT_grd_in_nan_value);

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

	if (fp != GMT_stdin) fclose (fp);
	
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
	/* complex:	Must be FALSE for rasterfiles */

	int i, i2, kk, inc = 1;
	int j, ij, j2, width_in, width_out, height_out, n2;
	int first_col, last_col, first_row, last_row;
	int *k;
	
	BOOLEAN check;
	
	unsigned char *tmp;
	
	FILE *fp;
	
	struct rasterfile h;
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	h.ras_magic = RAS_MAGIC;
	h.ras_width = header->nx;
	h.ras_height = header->ny;
	h.ras_depth = 8;
	h.ras_length = header->ny * (int) ceil (header->nx/2.0) * 2;
	h.ras_type = 1;
	h.ras_maptype = 0;
	h.ras_maplength = 0;
	
	n2 = (int) ceil (header->nx / 2.0) * 2;
	tmp = (unsigned char *) GMT_memory (VNULL, (size_t)n2, sizeof (unsigned char), "GMT_ras_write_grd");
	
	check = !GMT_is_fnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	if (complex) inc = 2;

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	h.ras_width = header->nx;
	h.ras_height = header->ny;
	h.ras_length = header->ny * (int) ceil (header->nx/2.0) * 2;
	
	/* store header information and array */
	
	if (GMT_write_rasheader (fp, &h)) {
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
	if (fp != GMT_stdout) fclose (fp);
	
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
				h->ras_magic = value;
				break;
			case 1:
				h->ras_width = value;
				break;
			case 2:
				h->ras_height = value;
				break;
			case 3:
				h->ras_depth = value;
				break;
			case 4:
				h->ras_length = value;
				break;
			case 5:
				h->ras_type = value;
				break;
			case 6:
				h->ras_maptype = value;
				break;
			case 7:
				h->ras_maplength = value;
				break;
		}
	}

	if (h->ras_type == RT_OLD && h->ras_length == 0) h->ras_length = 2 * irint (ceil (h->ras_width * h->ras_depth / 16.0)) * h->ras_height;

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

	if (h->ras_type == RT_OLD && h->ras_length == 0) {
		h->ras_length = 2 * irint (ceil (h->ras_width * h->ras_depth / 16.0)) * h->ras_height;
		h->ras_type = RT_STANDARD;
	}

	for (i = 0; i < 8; i++) {

		switch (i) {
			case 0:
				value = h->ras_magic;
				break;
			case 1:
				value = h->ras_width;
				break;
			case 2:
				value = h->ras_height;
				break;
			case 3:
				value = h->ras_depth;
				break;
			case 4:
				value = h->ras_length;
				break;
			case 5:
				value = h->ras_type;
				break;
			case 6:
				value = h->ras_maptype;
				break;
			case 7:
				value = h->ras_maplength;
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
 * Format # :	4
 * Type :	Native binary (uchar) C file
 * Prefix :	GMT_uchar_
 * Author :	Paul Wessel, SOEST
 * Date :	27-JUN-1994
 * Purpose:	The native binary output format is used
 *		primarily for piped i/o between programs
 *		that otherwise would use temporary, large
 *		intermediate grdfiles.  Note that not all
 *		programs can support piping (they may have
 *		to re-read the file or accept more than one
 *		grdfile).  Datasets with limited range may
 *		be stored using 1-byte unsigned characters which will
 *		reduce storage space and improve throughput.
 * Functions :	GMT_uchar_read_grd_info, GMT_uchar_write_grd_info,
 *		GMT_uchar_read_grd, GMT_uchar_write_grd
 *-----------------------------------------------------------*/
 
int GMT_uchar_read_grd_info (char *file, struct GRD_HEADER *header)
{	
	return (GMT_native_read_grd_info (file, header));
}

int GMT_uchar_write_grd_info (char *file, struct GRD_HEADER *header)
{
	return (GMT_native_write_grd_info (file, header));
}

int GMT_uchar_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_read_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_UCHAR));
}

int GMT_uchar_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_native_write_grd (file, header, grid, w, e, s, n, pad, complex, GMT_NATIVE_UCHAR));
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
 * Functions :	GMT_bit_read_grd_info, GMT_bit_write_grd_info,
 *		GMT_bit_read_grd, GMT_bit_write_grd
 *-----------------------------------------------------------*/
 
int GMT_bit_read_grd_info (char *file, struct GRD_HEADER *header)
{	
	return (GMT_native_read_grd_info (file, header));
}

int GMT_bit_write_grd_info (char *file, struct GRD_HEADER *header)
{
	return (GMT_native_write_grd_info (file, header));
}

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
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = fopen (file, "rb")) != NULL)	/* Skip header */
		fseek (fp, (long) sizeof (struct GRD_HEADER), SEEK_SET);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	check = !GMT_is_fnan (GMT_grd_in_nan_value);
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
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if comlex */
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
	if (fp != GMT_stdin) fclose (fp);
	
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
	/*		for imaginary parts when processed by grdfft etc. */

	int i, i2, kk, *k;
	int j, ij, j2, width_in, width_out, height_out, mx, word, bit, inc = 1;
	int first_col, last_col, first_row, last_row, check = FALSE;
	
	
	unsigned int *tmp, ival;
	
	FILE *fp;
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	check = !GMT_is_fnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	if (complex) inc = 2;

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	/* Find xmin/zmax */
	
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
	
	/* store header information and array */
	
	if (fwrite ((void *)header, sizeof (struct GRD_HEADER), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

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

	if (fp != GMT_stdout) fclose (fp);
	
	GMT_free ((void *)k);
	GMT_free ((void *)tmp);
	
	return (FALSE);
}

/* Here are the generic routines for dealing with native binary files
 * where the grid values are stored as 1-, 2-, 4-, or 8-byte items */

int GMT_native_read_grd_info (char *file, struct GRD_HEADER *header)
{
	/* Read GRD header structure from native binary file.  This is used by
	 * all the native binary formats in GMT */

	FILE *fp;
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	if (fread ((void *)header, sizeof (struct GRD_HEADER), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdin) fclose (fp);
	
	return (FALSE);
}

int GMT_native_write_grd_info (char *file, struct GRD_HEADER *header)
{
	/* Write GRD header structure to native binary file.  This is used by
	 * all the native binary formats in GMT */

	FILE *fp;
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	if (fwrite ((void *)header, sizeof (struct GRD_HEADER), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdout) fclose (fp);
	
	return (FALSE);
}

int GMT_native_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, int type)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to extract  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */
	/* type:	Data type (int, short, float, etc) */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_in;			/* Number of columns in subregion */
	int inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	int kk, i, j, j2, ij, i_0_out;	/* Misc. counters */
	int *k;				/* Array with indeces */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN piping = FALSE;		/* TRUE if we read input pipe instead of from file */
	BOOLEAN check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	void *tmp;			/* Array pointer for reading in rows of data */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = fopen (file, "rb")) != NULL)	/* Skip header */
		fseek (fp, (long) sizeof (struct GRD_HEADER), SEEK_SET);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
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

	tmp = (void *) GMT_memory (VNULL, (size_t)header->nx, GMT_native_size[type], "GMT_native_read");

	/* Now deal with skipping */

	if (piping) {	/* Skip data by reading it */
		for (j = 0; j < first_row; j++) fread (tmp, GMT_native_size[type], (size_t)header->nx, fp);
	}
	else {		/* Simply seek over it */
		fseek (fp, (long) (first_row * header->nx * GMT_native_size[type]), SEEK_CUR);
	}
	for (j = first_row, j2 = 0; j <= last_row; j++, j2++) {
		fread (tmp, GMT_native_size[type], (size_t)header->nx, fp);	/* Get one row */
		ij = (j2 + pad[3]) * width_out + i_0_out;	/* Already has factor of 2 in it if complex */
		for (i = 0; i < width_in; i++) {
			kk = ij + inc * i;
			grid[kk] = GMT_native_decode (tmp, k[i], type);	/* Convert whatever to float */
			if (check && grid[kk] == GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
		}
	}
	if (piping) {	/* Skip remaining data by reading it */
		for (j = last_row + 1; j < header->ny; j++) fread (tmp, GMT_native_size[type], (size_t)header->nx, fp);
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
			ij = inc * ((j + pad[3]) * width_out + i + pad[0]);
			if (GMT_is_fnan (grid[ij])) continue;
			if ((double)grid[ij] < header->z_min) header->z_min = (double)grid[ij];
			if ((double)grid[ij] > header->z_max) header->z_max = (double)grid[ij];
		}
	}
	if (fp != GMT_stdin) fclose (fp);
	
	GMT_free ((void *)k);
	GMT_free (tmp);

	return (FALSE);
}

int GMT_native_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, int type)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* complex:	TRUE if array is to hold real and imaginary parts (read in real only) */
	/*		Note: The file has only real values, we simply allow space in the array */
	/*		for imaginary parts when processed by grdfft etc. */
	/* type:	Data type (int, short, float, etc) */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_out;			/* Number of columns in subregion */
	int inc = 1;			/* Step in array: 1 for ordinary data, 2 for complex (skipping imaginary) */
	int i, j, i2, j2, ij;		/* Misc. counters */
	int *k;				/* Array with indeces */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN check = FALSE;		/* TRUE if nan-proxies are used to signify NaN (for non-floating point types) */
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	check = !GMT_is_dnan (GMT_grd_out_nan_value);

	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	if (complex) inc = 2;

	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	/* Find xmin/zmax */
	
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
	
	/* store header information and array */
	
	if (fwrite ((void *)header, sizeof (struct GRD_HEADER), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	i2 = first_col + pad[0];
	for (j = 0, j2 = first_row + pad[3]; j < height_out; j++, j2++) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_native_write_one (fp, grid[inc*(ij+k[i])], type);
	}
	GMT_free ((void *)k);
	if (fp != GMT_stdout) fclose (fp);
		
	return (FALSE);

}

float GMT_native_decode (void *vptr, int k, int type)
{
	float fval;

	switch (type) {
		case GMT_NATIVE_CHAR:
			fval = (float)(((char *)vptr)[k]);
			break;
		case GMT_NATIVE_UCHAR:
			/* u = (unsigned char *)vptr; */
			fval = (float)(((unsigned char *)vptr)[k]);
			break;
		case GMT_NATIVE_SHORT:
			fval = (float)(((short int *)vptr)[k]);
			break;
		case GMT_NATIVE_INT:
			fval = (float)(((int *)vptr)[k]);
			break;
		case GMT_NATIVE_FLOAT:
			fval = ((float *)vptr)[k];
			break;
		case GMT_NATIVE_DOUBLE:
			fval = (float)(((double *)vptr)[k]);
			break;
		default:
			break;
	}

	return (fval);
}

size_t GMT_native_write_one (FILE *fp, float z, int type)
{
	char c;
	unsigned char u;
	short int h;
	int i;
	double d;

	switch (type) {
		case GMT_NATIVE_CHAR:
			c = (char)z;
			return (fwrite ((void *)&c, GMT_native_size[type], (size_t)1, fp));
			break;
		case GMT_NATIVE_UCHAR:
			u = (unsigned char)z;
			return (fwrite ((void *)&u, GMT_native_size[type], (size_t)1, fp));
			break;
		case GMT_NATIVE_SHORT:
			h = (short int)z;
			return (fwrite ((void *)&h, GMT_native_size[type], (size_t)1, fp));
			break;
		case GMT_NATIVE_INT:
			i = (int)z;
			return (fwrite ((void *)&i, GMT_native_size[type], (size_t)1, fp));
			break;
		case GMT_NATIVE_FLOAT:
			return (fwrite ((void *)&z, GMT_native_size[type], (size_t)1, fp));
			break;
		case GMT_NATIVE_DOUBLE:
			d = (double)z;
			return (fwrite ((void *)&d, GMT_native_size[type], (size_t)1, fp));
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
 * Date :	9-SEP-1999
 * Purpose:	to transform to/from Surfer grid file format
 * Functions :	GMT_srf_read_grd_info, GMT_srf_write_grd_info,
 *		GMT_srf_read_grd, GMT_srf_write_grd	
 *-----------------------------------------------------------*/
 
struct srf_header {	/* Surfer file header structure */
	char id[4];			/* ASCII Binary identifier (DSBB) */
	short int nx;			/* Number of columns */
	short int ny;			/* Number of rows */
	double x_min;			/* Minimum x coordinate */
	double x_max;			/* Maximum x coordinate */
	double y_min;			/* Minimum y coordinate */
	double y_max;			/* Maximum y coordinate */
	double z_min;			/* Minimum z value */
	double z_max;			/* Maximum z value */
};

int GMT_srf_read_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct srf_header h;
	int GMT_read_srfheader (FILE *fp, struct srf_header *h);
	char id[5];
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
	}
	else if ((fp = fopen (file, "rb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	if (GMT_read_srfheader (fp, &h)) {
		fprintf (stderr, "GMT Fatal Error: Error reading file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	sprintf (id, "%.4s", h.id);
	if (strcmp (id, "DSBB")) {
		fprintf (stderr, "GMT Fatal Error: %s is not a valid surfer grid\n", file);
		exit (EXIT_FAILURE);
	}

	if (fp != GMT_stdin) fclose (fp);
	
	GMT_grd_init (header, 0, (char **)NULL, FALSE);

	strcpy (header->title, "Grid originaly from Surfer");
	header->nx = (int)h.nx;		 header->ny = (int)h.ny;
	header->x_min = h.x_min;	 header->x_max = h.x_max;
	header->y_min = h.y_min;	 header->y_max = h.y_max;
	header->z_min = h.z_min;	 header->z_max = h.z_max;
	header->x_inc = (h.x_max - h.x_min) / (h.nx - 1);
	header->y_inc = (h.y_max - h.y_min) / (h.ny - 1);
	header->node_offset = 0;	/* Grid format */

	
	return (FALSE);
}

int GMT_srf_write_grd_info (char *file, struct GRD_HEADER *header)
{
	FILE *fp;
	struct srf_header h;
	int GMT_write_srfheader (FILE *fp, struct srf_header *h);
	
	if (!strcmp (file, "="))
	{
#ifdef SET_IO_MODE
	GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "rb+")) == NULL && (fp = fopen (file, "wb")) == NULL) {
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

	if (fp != GMT_stdout) fclose (fp);
	
	return (FALSE);
}

int GMT_srf_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	int GMT_surfer_read_grd (char *file, struct GRD_HEADER *header, float *grid, int type);
	return (GMT_surfer_read_grd (file, header, grid, GMT_NATIVE_FLOAT));
}

int GMT_srf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	int GMT_surfer_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, int type);
	return (GMT_surfer_write_grd (file, header, grid, w, e, s, n, pad, GMT_NATIVE_FLOAT));
}

int GMT_read_srfheader (FILE *fp, struct srf_header *h)
{
	/* Reads the header of a Surfer gridfile */

	fread ((void *)h, sizeof (struct srf_header), (size_t)1, fp); 
	return (0);
}

int GMT_write_srfheader (FILE *fp, struct srf_header *h)
{
	fwrite ((void *)h, sizeof (struct srf_header), (size_t)1, fp); 
	return (0);
}

int GMT_surfer_read_grd (char *file, struct GRD_HEADER *header, float *grid, int type)
{	/* file:	File name	*/
	/* header:     	grid structure header */
	/* grid:	array with final grid */
	/* type:	Data type (int, short, float, etc) */

	int kk, i, j2, ij;         	/* Misc. counters */
	int *k;				/* Array with indeces */
	FILE *fp;			/* File pointer to data or pipe */
	BOOLEAN piping = FALSE;		/* TRUE if we read input pipe instead of from file */
	void *tmp;			/* Array pointer for reading in rows of data */
	GMT_grd_in_nan_value = 0.1701410e39;	/* Test value in Surfer grids */

	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (0);
#endif
		fp = GMT_stdin;
		piping = TRUE;
	}
	else if ((fp = fopen (file, "rb")) != NULL)	/* Skip header */
		fseek (fp, (long) sizeof (struct srf_header), SEEK_SET);
	else {
		fprintf (stderr, "GMT Fatal Error: Could not open file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	k = (int *) GMT_memory (VNULL, (size_t)header->nx, sizeof (int), "GMT_surfer_read_grd");
	for (i = 0; i < header->nx; i++) k[i] = i;

	/* Allocate memory for one row of data (for reading purposes) */

	tmp = (void *) GMT_memory (VNULL, (size_t)header->nx, GMT_native_size[type], "GMT_native_read");

	for (j2 = (header->ny - 1); j2 >= 0; j2--) {
		fread (tmp, GMT_native_size[type], (size_t)header->nx, fp);	/* Get one row */
		ij = j2 * header->nx;
		for (i = 0; i < header->nx; i++) {
			kk = ij + i;
			grid[kk] = GMT_native_decode (tmp, k[i], type);	/* Convert whatever to float */
			if (grid[kk] >= GMT_grd_in_nan_value) grid[kk] = GMT_f_NaN;
		}
	}
	
	if (fp != GMT_stdin) fclose (fp);
	
	GMT_free ((void *)k);
	GMT_free (tmp);

	return (FALSE);
}

int GMT_surfer_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, int type)
{	/* file:	File name	*/
	/* header:	grid structure header */
	/* grid:	array with final grid */
	/* w,e,s,n:	Sub-region to write out  [Use entire file if 0,0,0,0] */
	/* padding:	# of empty rows/columns to add on w, e, s, n of grid, respectively */
	/* type:	Data type (int, short, float, etc) */

	int first_col, last_col;	/* First and last column to deal with */
	int first_row, last_row;	/* First and last row to deal with */
	int width_in;			/* Number of items in one row of the subregion */
	int width_out;			/* Width of row as return (may include padding) */
	int height_out;			/* Number of columns in subregion */
	int i, j, i2, j2, ij;		/* Misc. counters */
	int *k;				/* Array with indeces */
	FILE *fp;			/* File pointer to data or pipe */
	struct srf_header h;

	GMT_grd_out_nan_value = 0.1701410e39;	/* Test value in Surfer grids */
	
	if (!strcmp (file, "=")) {
#ifdef SET_IO_MODE
		GMT_setmode (1);
#endif
		fp = GMT_stdout;
	}
	else if ((fp = fopen (file, "wb")) == NULL) {
		fprintf (stderr, "GMT Fatal Error: Could not create file %s!\n", file);
		exit (EXIT_FAILURE);
	}
	
	k = GMT_grd_prep_io (header, &w, &e, &s, &n, &width_out, &height_out, &first_col, &last_col, &first_row, &last_row);

	width_in = width_out;		/* Physical width of input array */
	if (pad[0] > 0) width_in += pad[0];
	if (pad[1] > 0) width_in += pad[1];
	
	header->x_min = w;
	header->x_max = e;
	header->y_min = s;
	header->y_max = n;
	
	/* Find xmin/zmax */
	
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
	
	if (fwrite ((void *)&h, sizeof (struct srf_header), (size_t)1, fp) != 1) {
		fprintf (stderr, "GMT Fatal Error: Error writing file %s!\n", file);
		exit (EXIT_FAILURE);
	}

	i2 = first_col + pad[0];
	for (j = 0, j2 = last_row + pad[3]; j < height_out; j++, j2--) {
		ij = j2 * width_in + i2;
		for (i = 0; i < width_out; i++) GMT_native_write_one (fp, grid[(ij+k[i])], type);
	}
	GMT_free ((void *)k);
	if (fp != GMT_stdout) fclose (fp);
		
	return (FALSE);
}

/* Add custom code here */

/*-----------------------------------------------------------
 * Format # :	7 - 11
 * Type :	netCDF-based format
 * Prefix :	GMT_cdf_*_ (byte, char, short, int, float, double)
 * Date :	21-MAY-2000
 * Author :	Masakazu Higaki (m-higaki@naps.kishou.go.jp)
 * Purpose:	Read/Write netcdf with non-float data types
 * Functions :	GMT_cdf_*_write_grd
 *		
 *-----------------------------------------------------------*/
 
int GMT_cdf_byte_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_BYTE));
}
 
int GMT_cdf_char_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_CHAR));
}
 
int GMT_cdf_short_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_SHORT));
}
 
int GMT_cdf_int_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_INT));
}
 
int GMT_cdf_float_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_FLOAT));
}
 
int GMT_cdf_double_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex)
{
	return (GMT_cdf_write_grd (file, header, grid, w, e, s, n, pad, complex, NC_DOUBLE));
}
