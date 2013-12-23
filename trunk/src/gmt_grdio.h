/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#endif /* GMT_GRDIO_H */
