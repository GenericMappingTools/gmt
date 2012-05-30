/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

#ifndef _GMTAPI_DEFINE_H
#define _GMTAPI_DEFINE_H

enum GMT_enum_api {
	GMTAPI_NOTSET		= -1,	/* Object ID when not set */
	GMTAPI_ORDER_ROW 	= 0,	/* C-style array order: as index increase we move across rows */
	GMTAPI_ORDER_COL	= 1,	/* Fortran-style array order: as index increase we move down columns */
	GMTAPI_USAGE		= 0,	/* Want to report full program usage message */
	GMTAPI_SYNOPSIS		= 1,	/* Just want the synopsis of usage */
	GMTAPI_GMT		= 0,	/* Want GMT but not PSL initialized */
	GMTAPI_GMTPSL		= 1,	/* Want GMT and PSL initialized */
	GMTAPI_N_GRID_ARGS	= 4,	/* Minimum size of information array used to specify grid parameters */
	GMTAPI_N_ARRAY_ARGS	= 8,	/* Minimum size of information array used to specify array parameters */
	GMTAPI_STRLEN		= 16	/* Bytes needed to hold the @GMTAPI@-###### resource names */
};

/* Index parameters used to access the information arrays */

enum GMT_enum_pars {GMTAPI_TYPE = 0,	/* ipar[0] = data type (GMTAPI_{BYTE|SHORT|FLOAT|INT|DOUBLE}) */
	GMTAPI_NDIM,		/* ipar[1] = dimensionality of data (1, 2, or 3) (GMT grids = 2 yet stored internally as 1D) */
	GMTAPI_NROW,		/* ipar[2] = number_of_rows (or length of 1-D array) */
	GMTAPI_NCOL,		/* ipar[3] = number_of_columns (1 for 1-D array) */
	GMTAPI_KIND,		/* ipar[4] = arrangment of rows/col (0 = rows (C), 1 = columns (Fortran)) */
	GMTAPI_DIML,		/* ipar[5] = length of dimension for row (C) or column (Fortran) */
	GMTAPI_FREE,		/* ipar[6] = 1 to free array after use (IN) or before filling with output (OUT), 0 to leave alone */
	GMTAPI_NODE};		/* ipar[7] = 1 for pixel registration, 0 for node */

/* These data primitive identifiers are as follows: */
enum GMT_enum_type {
	GMTAPI_CHAR = 0,  /* int8_t, 1-byte signed integer type */
	GMTAPI_UCHAR,     /* uint8_t, 1-byte unsigned integer type */
	GMTAPI_SHORT,     /* int16_t, 2-byte signed integer type */
	GMTAPI_USHORT,    /* uint16_t, 2-byte unsigned integer type */
	GMTAPI_INT,       /* int32_t, 4-byte signed integer type */
	GMTAPI_UINT,      /* uint32_t, 4-byte unsigned integer type */
	GMTAPI_LONG,      /* int64_t, 8-byte signed integer type */
	GMTAPI_ULONG,     /* uint64_t, 8-byte unsigned integer type */
	GMTAPI_FLOAT,     /* 4-byte data float type */
	GMTAPI_DOUBLE,    /* 8-byte data float type */
	GMTAPI_TEXT,      /* Arbitrarily long text string [OGR/GMT use only] */
	GMTAPI_TIME,      /* string with date/time info [OGR/GMT use only] */
	GMTAPI_N_TYPES};  /* The number of supported data types above */

#define GMTAPI_OPT_USAGE	'?'
#define GMTAPI_OPT_SYNOPSIS	'^'
#define GMTAPI_OPT_INFILE	'<'
#define GMTAPI_OPT_OUTFILE	'>'

#endif /* _GMTAPI_DEFINE_H */
