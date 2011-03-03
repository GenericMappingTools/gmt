/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.h,v 1.53 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
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

/* Constants for *.img grids */

#define GMT_IMG_MINLON		0.0
#define GMT_IMG_MAXLON		360.0
#define GMT_IMG_MINLAT_72	-72.0059773539
#define GMT_IMG_MAXLAT_72	+72.0059773539
#define GMT_IMG_MINLAT_80	-80.7380086280
#define GMT_IMG_MAXLAT_80	+80.7380086280
#define GMT_IMG_NLON_1M		21600	/* At 1 min resolution */
#define GMT_IMG_NLON_2M		10800	/* At 2 min resolution */
#define GMT_IMG_NLAT_1M_72	12672	/* At 1 min resolution */
#define GMT_IMG_NLAT_1M_80	17280	/* At 1 min resolution */
#define GMT_IMG_NLAT_2M_72	6336	/* At 1 min resolution */
#define GMT_IMG_NLAT_2M_80	8640	/* At 1 min resolution */
#define GMT_IMG_ITEMSIZE	2	/* Size of 2 byte short ints */

EXTERN_MSC GMT_LONG GMT_grdformats [GMT_N_GRD_FORMATS][2];

EXTERN_MSC GMT_LONG GMT_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex);
EXTERN_MSC GMT_LONG GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, GMT_LONG *pad, GMT_LONG complex);

EXTERN_MSC GMT_LONG GMT_grd_data_size (GMT_LONG format, double *nan_value);
EXTERN_MSC GMT_LONG GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, GMT_LONG *width, GMT_LONG *height, GMT_LONG *first_col, GMT_LONG *last_col, GMT_LONG *first_row, GMT_LONG *last_row, GMT_LONG **index);
EXTERN_MSC GMT_LONG GMT_adjust_loose_wesn (double *w, double *e, double *s, double *n, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax, GMT_LONG interpolant);
EXTERN_MSC GMT_LONG GMT_grd_format_decoder (const char *code);
EXTERN_MSC void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, GMT_LONG update);
EXTERN_MSC void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift);
EXTERN_MSC void GMT_decode_grd_h_info (char *input, struct GRD_HEADER *h);
EXTERN_MSC GMT_LONG GMT_grd_RI_verify (struct GRD_HEADER *h, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_grd_get_format (char *file, struct GRD_HEADER *header, GMT_LONG magic);
EXTERN_MSC GMT_LONG GMT_grd_is_global (struct GRD_HEADER *h);

/* These are pointers to the various functions and are set in GMT_grdio_init() */

EXTERN_MSC PFL GMT_io_readinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFL GMT_io_updateinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFL GMT_io_writeinfo[GMT_N_GRD_FORMATS];
EXTERN_MSC PFL GMT_io_readgrd[GMT_N_GRD_FORMATS];
EXTERN_MSC PFL GMT_io_writegrd[GMT_N_GRD_FORMATS];

#include "gmt_customio.h"

struct GMT_GRD_INFO {	/* Holds any -R -I -F settings passed indirectly via -R<grdfile> */
	struct GRD_HEADER grd;	/* Header of grid file passed via -R */
	GMT_LONG active;		/* TRUE if initialized via -R */
};

struct GMT_GRID {	/* To hold a GMT float grid and its header in one container */
	struct GRD_HEADER *header;	/* Pointer to full GMT header for the grid */
	float *data;			/* Pointer to the float grid */
};

struct GMT_GRDFILE {
	GMT_LONG size;		/* Bytes per item */
	GMT_LONG n_byte;	/* Number of bytes for row */
	GMT_LONG row;		/* Current row */
	int fid;		/* NetCDF file number */
	size_t edge[2];		/* Dimension arrays for netCDF files */
	size_t start[2];	/* same */

	GMT_LONG check;		/* TRUE if we must replace NaNs with another representation on i/o */
	GMT_LONG auto_advance;	/* TRUE if we want to read file sequentially */
	
	double scale;		/* scale to use for i/o */
	double offset;		/* offset to use for i/o */
	
	FILE *fp;		/* File pointer for native files */
	
	void *v_row;		/* Void Row pointer for any format */
	
	struct GRD_HEADER header;	/* Full GMT header for the file */
};
	
/* Row i/o functions */

EXTERN_MSC GMT_LONG GMT_open_grd (char *file, struct GMT_GRDFILE *G, char mode);
EXTERN_MSC void GMT_close_grd (struct GMT_GRDFILE *G);
EXTERN_MSC GMT_LONG GMT_read_grd_row (struct GMT_GRDFILE *G, GMT_LONG row_no, float *row);
EXTERN_MSC GMT_LONG GMT_write_grd_row (struct GMT_GRDFILE *G, GMT_LONG row_no, float *row);

/* IMG read function */

EXTERN_MSC GMT_LONG GMT_read_img (char *imgfile, struct GRD_HEADER *h, float **grid, double w, double e, double s, double n, double scale, GMT_LONG mode, double lat, GMT_LONG init);

/* Grid container allocation/deallocation routines */

EXTERN_MSC struct GMT_GRID *GMT_create_grid (char *arg);
EXTERN_MSC void GMT_destroy_grid (struct GMT_GRID *G, GMT_LONG free_grid);

#endif /* GMT_GRDIO_H */
