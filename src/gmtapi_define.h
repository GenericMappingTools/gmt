/*--------------------------------------------------------------------
 *	$Id: gmtapi_define.h,v 1.4 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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

#ifndef _GMTAPI_DEFINE_H
#define _GMTAPI_DEFINE_H

#define GMTAPI_N_ARRAY_ARGS	8	/* Minimum size of information array used to specify array parameters */
#define GMTAPI_N_GRID_ARGS	4	/* Minimum size of information array used to specify grid parameters */

#define GMTAPI_STRLEN		16	/* Bytes needed to hold the @GMTAPI@-###### resource names */

	/* Index parameters used to access the information arrays */

/* The 8 parameters are all GMT_LONG */
#define GMTAPI_TYPE		0	/* ipar[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
#define GMTAPI_NDIM		1	/* ipar[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
#define GMTAPI_NROW		2	/* ipar[2] = number_of_rows (or length of 1-D array) */
#define GMTAPI_NCOL		3	/* ipar[3] = number_of_columns (1 for 1-D array) */
#define GMTAPI_KIND		4	/* ipar[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
#define GMTAPI_DIML		5	/* ipar[5] = length of dimension for row (C) or column (Fortran) */
#define GMTAPI_FREE		6	/* ipar[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone */
#define GMTAPI_NODE		7	/* ipar[7] = 1 for pixel registration, 0 for node */

	/* Data primitive identifiers */

#define GMTAPI_N_TYPES		8	/* The number of supported data types (below) */
#define GMTAPI_BYTE		0	/* The 1-byte data integer type */
#define GMTAPI_SHORT		1	/* The 2-byte data integer type */
#define GMTAPI_INT		2	/* The 4-byte data integer type */
#define GMTAPI_LONG		3	/* The 8-byte data integer type */
#define GMTAPI_FLOAT		4	/* The 4-byte data float type */
#define GMTAPI_DOUBLE		5	/* The 8-byte data float type */
/* These two are only for OGR/GMT use */
#define GMTAPI_TEXT		6	/* Arbitrarily long text string */
#define GMTAPI_TIME		7	/* string with date/time info */

	/* Array ordering constants */
	
#define GMTAPI_ORDER_ROW	0	/* C-style array order: as index increase we move across rows */
#define GMTAPI_ORDER_COL	1	/* Fortran-style array order: as index increase we move down columns */

#define GMTAPI_USAGE		0	/* Want to report full program usage message */
#define GMTAPI_SYNOPSIS		1	/* Just want the synopsis of usage */

#define GMTAPI_NOTSET		-1	/* Object ID when not set */

#define GMTAPI_GMT		0	/* Want GMT but not PSL initialized */
#define GMTAPI_GMTPSL		1	/* Want GMT and PSL initialized */

#define GMTAPI_OPT_USAGE	'?'
#define GMTAPI_OPT_SYNOPSIS	'^'
#define GMTAPI_OPT_INFILE	'<'
#define GMTAPI_OPT_OUTFILE	'>'
#define GMTAPI_OPT_NUMBER	'#'

#endif /* _GMTAPI_DEFINE_H */
