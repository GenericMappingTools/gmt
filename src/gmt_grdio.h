/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

/*
 * Include file for grd i/o
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
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
#define GMT_IMG_MINLAT_85	-85.0511287798
#define GMT_IMG_MAXLAT_85	+85.0511287798

enum GMT_enum_img {
	GMT_IMG_NLON_1M    = 21600U, /* At 1 min resolution */
	GMT_IMG_NLON_2M    = 10800U, /* At 2 min resolution */
	GMT_IMG_NLAT_1M_72 = 12672U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_72 = 6336U,  /* At 2 min resolution */
	GMT_IMG_NLAT_1M_80 = 17280U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_80 = 8640U,  /* At 2 min resolution */
	GMT_IMG_NLAT_1M_85 = 21600U, /* At 1 min resolution */
	GMT_IMG_NLAT_2M_85 = 10800U, /* At 2 min resolution */
	GMT_IMG_ITEMSIZE   = 2U      /* Size of 2 byte short ints */
};

/* Special grid format IDs */

enum Gmt_grid_id {
	/* DO NOT change the order because id values have grown historically.
	 * Append newly introduced id's at the end. */
	k_grd_unknown_fmt = 0, /* if grid format cannot be auto-detected */
	GMT_GRID_IS_BF,         /* GMT native, C-binary format (32-bit float) */
	GMT_GRID_IS_BS,         /* GMT native, C-binary format (16-bit integer) */
	GMT_GRID_IS_RB,         /* SUN rasterfile format (8-bit standard) */
	GMT_GRID_IS_BB,         /* GMT native, C-binary format (8-bit integer) */
	GMT_GRID_IS_BM,         /* GMT native, C-binary format (bit-mask) */
	GMT_GRID_IS_SF,         /* Golden Software Surfer format 6 (32-bit float) */
	GMT_GRID_IS_CB,         /* GMT netCDF format (8-bit integer) */
	GMT_GRID_IS_CS,         /* GMT netCDF format (16-bit integer) */
	GMT_GRID_IS_CI,         /* GMT netCDF format (32-bit integer) */
	GMT_GRID_IS_CF,         /* GMT netCDF format (32-bit float) */
	GMT_GRID_IS_CD,         /* GMT netCDF format (64-bit float) */
	GMT_GRID_IS_RF,         /* GEODAS grid format GRD98 (NGDC) */
	GMT_GRID_IS_BI,         /* GMT native, C-binary format (32-bit integer) */
	GMT_GRID_IS_BD,         /* GMT native, C-binary format (64-bit float) */
	GMT_GRID_IS_NB,         /* GMT netCDF format (8-bit integer) */
	GMT_GRID_IS_NS,         /* GMT netCDF format (16-bit integer) */
	GMT_GRID_IS_NI,         /* GMT netCDF format (32-bit integer) */
	GMT_GRID_IS_NF,         /* GMT netCDF format (32-bit float) */
	GMT_GRID_IS_ND,         /* GMT netCDF format (64-bit float) */
	GMT_GRID_IS_SD,         /* Golden Software Surfer format 7 (64-bit float, read-only) */
	GMT_GRID_IS_AF,         /* Atlantic Geoscience Center format AGC (32-bit float) */
	GMT_GRID_IS_GD,         /* Import through GDAL */
	GMT_GRID_IS_EI,         /* ESRI Arc/Info ASCII Grid Interchange format (ASCII integer) */
	GMT_GRID_IS_EF          /* ESRI Arc/Info ASCII Grid Interchange format (ASCII float, write-only) */
};
#define GMT_N_GRD_FORMATS 25 /* Number of formats above plus 1 */

#define GMT_GRID_IS_GOLDEN7	GMT_GRID_IS_SD
#define GMT_GRID_IS_GDAL	GMT_GRID_IS_GD

#include "gmt_customio.h"

struct GMT_GRID_INFO {	/* Holds any -R -I -F settings passed indirectly via -R<grdfile> */
	struct GMT_GRID_HEADER grd;	/* Header of grid file passed via -R */
	bool active;		/* true if initialized via -R */
};

struct GMT_GRID_ROWBYROW {	/* Holds book-keeping information needed for row-by-row actions */
	size_t size;		/* Bytes per item [4 for float, 1 for byte, etc] */
	size_t n_byte;		/* Number of bytes for row */
	unsigned int row;	/* Current row */
	bool open;		/* true if we have already opened the file */
	bool check;		/* true if we must replace NaNs with another representation on i/o */
	bool auto_advance;	/* true if we want to read file sequentially */

	int fid;		/* NetCDF file number [netcdf files only] */
	size_t edge[2];		/* Dimension arrays [netcdf files only] */
	size_t start[2];	/* Position arrays [netcdf files only] */

	FILE *fp;		/* File pointer [for native files] */

	void *v_row;		/* Void Row pointer for any data format */
};

#ifdef __APPLE__ /* Accelerate framework */
#include <Accelerate/Accelerate.h>
#undef I /* because otherwise we are in trouble with, e.g., struct GMT_IMAGE *I */
#endif

static inline void scale_and_offset_f (float *data, size_t length, float scale, float offset) {
	/* Routine that scales and offsets the data in a vector
	 *  data:   Single-precision real input vector
	 *  length: The number of elements to process
	 * This function uses the vDSP portion of the Accelerate framework if possible */
#ifndef __APPLE__
	size_t n;
#endif
	if (scale == 1.0) /* offset only */
#ifdef __APPLE__ /* Accelerate framework */
		vDSP_vsadd (data, 1, &offset, data, 1, length);
#else
		for (n = 0; n < length; ++n)
			data[n] += offset;
#endif
	else if (offset == 0.0) /* scale only */
#ifdef __APPLE__ /* Accelerate framework */
		vDSP_vsmul (data, 1, &scale, data, 1, length);
#else
		for (n = 0; n < length; ++n)
			data[n] *= scale;
#endif
	else /* scale + offset */
#ifdef __APPLE__ /* Accelerate framework */
		vDSP_vsmsa (data, 1, &scale, &offset, data, 1, length);
#else
		for (n = 0; n < length; ++n)
			data[n] = data[n] * scale + offset;
#endif
}

EXTERN_MSC int GMT_grd_format_decoder (struct GMT_CTRL *GMT, const char *code, unsigned int *type_id);
EXTERN_MSC int GMT_grd_get_format (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, bool magic);
EXTERN_MSC int GMT_grd_prep_io (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double wesn[], unsigned int *width, unsigned int *height, int *first_col, int *last_col, int *first_row, int *last_row, unsigned int **index);
EXTERN_MSC int GMT_update_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC void GMT_scale_and_offset_f (struct GMT_CTRL *GMT, float *data, size_t length, double scale, double offset);
EXTERN_MSC int gmt_grd_layout (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, float *grid, unsigned int complex_mode, unsigned int direction);
EXTERN_MSC void GMT_grd_mux_demux (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, float *data, unsigned int mode);

#endif /* GMT_GRDIO_H */
