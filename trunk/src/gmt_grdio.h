/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.h,v 1.8 2004-01-02 22:45:13 pwessel Exp $
 *
 *	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
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
 * Include file for grd i/o
 *
 * Author:	Paul Wessel
 * Date:	21-AUG-1995
 * Revised:	06-DEC-2001
 * Version:	4
 */

#ifndef GMT_GRDIO_H
#define GMT_GRDIO_H

#define N_GRD_FORMATS	12	/* Number of supported grd file formats */

EXTERN_MSC int GMT_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

EXTERN_MSC int GMT_grd_i_format;
EXTERN_MSC int GMT_grd_o_format;
EXTERN_MSC int *GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, int *width, int *height, int *first_col, int *last_col, int *first_row, int *last_row);

/* These are pointers to the various functions and are set in GMT_grdio_init() */

EXTERN_MSC PFI GMT_io_readinfo[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_updateinfo[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writeinfo[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_readgrd[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writegrd[N_GRD_FORMATS];

/* Default format # 0 */

EXTERN_MSC int GMT_cdf_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_cdf_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_cdf_write_grd_info  (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_cdf_read_grd  (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, nc_type nc_type);
EXTERN_MSC int GMT_cdf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, nc_type nc_type);


#include "gmt_customio.h"

struct GMT_GRDFILE {
	char name[256];		/* Actual name of the file after any =<stuff> has been removed */
	int id;			/* Gridfile format id (0-N_GRD_FORMATS) */
	int fid;		/* NetCDF file number */
	int z_id;		/* NetCDF z array ID */
	size_t edge[2];		/* Dimension arrays for netCDF files */
	size_t start[2];	/* same */
	int size;		/* Bytes per item */
	int n_byte;		/* Number of bytes for row */
	int type;		/* Which GMT NATIVE type */
	int row;		/* Current row */
	
	BOOLEAN is_cdf;		/* TRUE for netCDF files */
	BOOLEAN check;		/* TRUE if we must replace NaNs with another representation on i/o */
	BOOLEAN auto_advance;	/* TRUE if we want to read file sequentially */
	
	double scale;		/* scale to use for i/o */
	double offset;		/* offset to use for i/i */
	
	FILE *fp;		/* File pointer for native files */
	
	signed char *c_row;	/* Row pointer for character files */
	unsigned char *b_row;	/* Row pointer for unsiged character (byte) files */
	short int *s_row;	/* Row pointer for short int files */
	int *i_row;		/* Row pointer for integer files */
	unsigned int *u_row;	/* Row pointer for unsigned int files */
	float *f_row;		/* Row pointer for float files */
	double *d_row;		/* Row pointer for double format */
	void *v_row;		/* Void Row pointer for any format */
	
	struct GRD_HEADER header;	/* Full GMT header for the file */
};
	
/* Row i/o functions */

EXTERN_MSC void GMT_open_grd (char *file, struct GMT_GRDFILE *G, char mode);
EXTERN_MSC void GMT_close_grd (struct GMT_GRDFILE *G);
EXTERN_MSC int GMT_read_grd_row (struct GMT_GRDFILE *G, int row_no, float *row);
EXTERN_MSC int GMT_write_grd_row (struct GMT_GRDFILE *G, int row_no, float *row);

#endif /* GMT_GRDIO_H */
