/*--------------------------------------------------------------------
 *	$Id: gmt_constants.h,v 1.8 2011-04-29 03:08:11 guru Exp $
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
/*
 * gmt_constants.h contains definitions of constants used throught GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_CONSTANTS_H
#define _GMT_CONSTANTS_H

/*--------------------------------------------------------------------
 *			GMT CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TWO_PI
#define TWO_PI        6.28318530717958647692
#endif
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2          1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4          0.78539816339744830962
#endif
#ifndef M_E
#define	M_E		2.7182818284590452354
#endif
#ifndef M_SQRT2
#define	M_SQRT2		1.41421356237309504880
#endif
#ifndef M_LN2_INV
#define	M_LN2_INV	(1.0 / 0.69314718055994530942)
#endif
#ifndef M_EULER
#define M_EULER		0.577215664901532860606512	/* Euler's constant (gamma) */
#endif

#define GMT_CONV_LIMIT	1.0e-8		/* Fairly tight convergence limit or "close to zero" limit */
#define GMT_SMALL	1.0e-4		/* Needed when results aren't exactly zero but close */
#define GMT_MIN_MEMINC	2048		/* E.g., 16 kb of 8-byte doubles */
#define GMT_MAX_MEMINC	67108864	/* E.g., 512 Mb of 8-byte doubles */

#define GMT_CHUNK	2048
#define GMT_SMALL_CHUNK	64
#define GMT_TINY_CHUNK	8
#define GMT_TEXT_LEN64	64
#define GMT_TEXT_LEN256	256
#define GMT_MAX_COLUMNS	4096		/* Limit on number of columns in data tables (not grids) */
#define GMT_BUFSIZ	4096		/* Size of char record for i/o */
#define CNULL		((char *)NULL)
#define GMT_CM		0
#define GMT_INCH	1
#define GMT_M		2	/* No longer used */
#define GMT_PT		3

#define GMT_DIM_UNITS	"cip"		/* Plot dimensions in cm, inch, or point */
#define GMT_LEN_UNITS2	"efkMn"		/* Distances in meter, feet, km, Miles, nautical miles */
#define GMT_LEN_UNITS	"dmsefkMn"	/* Distances in arc-{degree,minute,second} or meter, feet, km, Miles, nautical miles */
#define GMT_DIM_UNITS_DISPLAY	"c|i|p"			/* Same, used to display as options */
#define GMT_LEN_UNITS_DISPLAY	"d|m|s|e|f|k|M|n"	/* Same, used to display as options */
#define GMT_LEN_UNITS2_DISPLAY	"e|f|k|M|n"		/* Same, used to display as options */
#define GMT_DEG2SEC_F	3600.0
#define GMT_DEG2SEC_I	3600
#define GMT_SEC2DEG	(1.0 / GMT_DEG2SEC_F)
#define GMT_DEG2MIN_F	60.0
#define GMT_DEG2MIN_I	60
#define GMT_MIN2DEG	(1.0 / GMT_DEG2MIN_F)
#define GMT_MIN2SEC_F	60.0
#define GMT_MIN2SEC_I	60
#define GMT_SEC2MIN	(1.0 / GMT_MIN2SEC_F)
#define GMT_DAY2HR_F	24.0
#define GMT_DAY2HR_I	24
#define GMT_HR2DAY	(1.0 / GMT_DAY2HR_F)
#define GMT_DAY2MIN_F	1440.0
#define GMT_DAY2MIN_I	1440
#define GMT_MIN2DAY	(1.0 / GMT_DAY2MIN_F)
#define GMT_DAY2SEC_F	86400.0
#define GMT_DAY2SEC_I	86400
#define GMT_SEC2DAY	(1.0 / GMT_DAY2SEC_F)
#define GMT_HR2SEC_F	3600.0
#define GMT_HR2SEC_I	3600
#define GMT_SEC2HR	(1.0 / GMT_HR2SEC_F)
#define GMT_HR2MIN_F	60.0
#define GMT_HR2MIN_I	60
#define GMT_MIN2HR	(1.0 / GMT_HR2MIN_F)

#define GMT_YR2SEC_F	(365.2425 * 86400.0)
#define GMT_MON2SEC_F	(365.2425 * 86400.0 / 12.0)

#define GMT_DEC_SIZE	0.54	/* Size of a decimal number compared to point size */
#define GMT_PER_SIZE	0.30	/* Size of a decimal point compared to point size */

#define GMT_PEN_LEN	128
#define GMT_PENWIDTH	0.25	/* Default pen width in points */

/* Various options for FFT calculations [Default is 0] */
#define GMT_FFTPACK		1
#define GMT_FFTW		2
#define GMT_SUN4		3
#define GMT_VECLIB		4

/* Various directions and modes to call the FFT */
#define GMT_FFT_FWD		0
#define GMT_FFT_INV		1
#define GMT_FFT_REAL		0
#define GMT_FFT_COMPLEX		1

/* Various grid/image interpolation modes */
#define BCR_NEARNEIGHBOR	0
#define BCR_BILINEAR		1
#define BCR_BSPLINE		2
#define BCR_BICUBIC		3

/* Various grid/image boundary conditions */
#define GMT_BC_IS_NOTSET	0
#define GMT_BC_IS_NATURAL	1
#define GMT_BC_IS_PERIODIC	2
#define GMT_BC_IS_POLE		3
#define GMT_BC_IS_DATA		4

#endif  /* _GMT_CONSTANTS_H */
