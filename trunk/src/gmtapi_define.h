/*--------------------------------------------------------------------
 *	$Id: gmtapi_define.h,v 1.3 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
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

#define GMTAPI_N_ARRAY_ARGS	8	/* Minimum size of information array used to specify array parameters */
#define GMTAPI_N_GRID_ARGS	12	/* Minimum size of information array used to specify grid parameters */

	/* Index parameters used to access the information arrays */

#define GMTAPI_TYPE		0	/* arg[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
#define GMTAPI_NDIM		1	/* arg[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
#define GMTAPI_NROW		2	/* arg[2] = number_of_rows (or length of 1-D array) */
#define GMTAPI_NCOL		3	/* arg[3] = number_of_columns (1 for 1-D array) */
#define GMTAPI_KIND		4	/* arg[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
#define GMTAPI_DIML		5	/* arg[5] = length of dimension for row (C) or column (Fortran) */
#define GMTAPI_FREE		6	/* arg[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone */
#define GMTAPI_NODE		7	/* arg[7] = 1 for pixel registration, 0 for node */
#define GMTAPI_XMIN		8	/* arg[8] = x_min (west) of grid */
#define GMTAPI_XMAX		9	/* arg[9] = x_max (east) of grid */
#define GMTAPI_YMIN		10	/* arg[10] = y_min (south) of grid */
#define GMTAPI_YMAX		11	/* arg[11] = y_max (north) of grid */

	/* Data primitive identifiers */

#define GMTAPI_N_TYPES		5	/* The number of supported data types (below) */
#define GMTAPI_BYTE		0	/* The 1-byte data integer type */
#define GMTAPI_SHORT		1	/* The 2-byte data integer type */
#define GMTAPI_INT		2	/* The 4-byte data integer type */
#define GMTAPI_FLOAT		3	/* The 4-byte data float type */
#define GMTAPI_DOUBLE		4	/* The 8-byte data float type */

	/* Array ordering constants */
	
#define GMTAPI_ORDER_ROW	0	/* C-style array order: as index increase we move across rows */
#define GMTAPI_ORDER_COL	1	/* Fortran-style array order: as index increase we move down columns */
