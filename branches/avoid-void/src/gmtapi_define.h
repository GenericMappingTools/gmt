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

enum GMT_pars {GMTAPI_TYPE, GMTAPI_NDIM, GMTAPI_NROW, GMTAPI_NCOL, GMTAPI_KIND, GMTAPI_DIML, GMTAPI_FREE, GMTAPI_NODE};
/* The index parameters are as follows:
	GMTAPI_TYPE		0	ipar[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE})
	GMTAPI_NDIM		1	ipar[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D)
	GMTAPI_NROW		2	ipar[2] = number_of_rows (or length of 1-D array)
	GMTAPI_NCOL		3	ipar[3] = number_of_columns (1 for 1-D array)
	GMTAPI_KIND		4	ipar[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran))
	GMTAPI_DIML		5	ipar[5] = length of dimension for row (C) or column (Fortran)
	GMTAPI_FREE		6	ipar[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone
	GMTAPI_NODE		7	ipar[7] = 1 for pixel registration, 0 for node
*/

enum GMT_datatypes {GMTAPI_UCHAR, GMTAPI_CHAR, GMTAPI_USHORT, GMTAPI_SHORT, GMTAPI_UINT, GMTAPI_INT, GMTAPI_LONG, \
	GMTAPI_ULONG, GMTAPI_FLOAT, GMTAPI_DOUBLE, GMTAPI_TEXT, GMTAPI_TIME, GMTAPI_N_TYPES};
/* These data primitive identifiers are as follows:
	GMTAPI_UCHAR		0	The 1-byte unsigned integer type
	GMTAPI_CHAR		1	The 1-byte signed integer type
	GMTAPI_USHORT		2	The 2-byte unsigned integer type
	GMTAPI_SHORT		3	The 2-byte signed integer type
	GMTAPI_UINT		4	The 4-byte unsigned integer type
	GMTAPI_INT		5	The 4-byte signed integer type
	GMTAPI_ULONG		6	The 8-byte unsigned integer type
	GMTAPI_LONG		7	The 8-byte signed integer type
	GMTAPI_FLOAT		8	The 4-byte data float type
	GMTAPI_DOUBLE		9	The 8-byte data float type
  These two are only for OGR/GMT use:
	GMTAPI_TEXT		10	Arbitrarily long text string
	GMTAPI_TIME		11	string with date/time info
	GMTAPI_N_TYPES		12	The number of supported data types above
*/

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
