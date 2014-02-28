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
 * gmt_constants.h contains definitions of constants used throught GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_CONSTANTS_H
#define _GMT_CONSTANTS_H

/*=====================================================================================
 *	GMT API CONSTANTS DEFINITIONS
 *===================================================================================*/

#include "gmt_error_codes.h"			/* All API error codes are defined here */
#include "gmt_define.h"			/* All constant values are defined here */

/*--------------------------------------------------------------------
 *			GMT CONSTANTS MACRO DEFINITIONS
 *--------------------------------------------------------------------*/

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

/* Various allocation-length parameters */
enum GMT_enum_length {
	GMT_TINY_CHUNK  = 8U,
	GMT_SMALL_CHUNK = 64U,
	GMT_CHUNK       = 2048U,
	GMT_BIG_CHUNK   = 65536U,
	GMT_LEN16	= 16U,		/* All strings used to format date/clock output must be this length */
	GMT_LEN32  = 32U,          /* Small length of texts */
	GMT_LEN64  = 64U,          /* Intermediate length of texts */
	GMT_LEN128 = 128U,         /* Double of 64 */
	GMT_LEN256 = 256U,         /* Max size of some text items */
	GMT_MAX_COLUMNS = 4096U,        /* Limit on number of columns in data tables (not grids) */
	GMT_BUFSIZ      = 4096U,        /* Size of char record for i/o */
	GMT_MIN_MEMINC  = 1U,        /* E.g., 16 kb of 8-byte doubles */
	GMT_MAX_MEMINC  = 67108864U};   /* E.g., 512 Mb of 8-byte doubles */

/* The four plot length units [m just used internally] */
enum GMT_enum_unit {
	GMT_CM = 0,
	GMT_INCH,
	GMT_M,
	GMT_PT};

/* Handling of swap/no swap in i/o */
enum GMT_swap_direction {
	k_swap_none = 0,
	k_swap_in,
	k_swap_out};

#define GMT_DIM_UNITS	"cip"		/* Plot dimensions in cm, inch, or point */
#define GMT_LEN_UNITS2	"efkMnu"	/* Distances in meter, foot, survey foot, km, Mile, nautical mile */
#define GMT_LEN_UNITS	"dmsefkMnu"	/* Distances in arc-{degree,minute,second} or meter, foot, km, Mile, nautical mile, survey foot */
#define GMT_DIM_UNITS_DISPLAY	"c|i|p"			/* Same, used to display as options */
#define GMT_LEN_UNITS_DISPLAY	"d|m|s|e|f|k|M|n|u"	/* Same, used to display as options */
#define GMT_LEN_UNITS2_DISPLAY	"e|f|k|M|n|u"		/* Same, used to display as options */
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
enum FFT_implementations {
	k_fft_auto = 0,    /* Automatically select best FFT algorithm */
	k_fft_accelerate,  /* Select Accelerate Framework vDSP FFT [OS X only] */
	k_fft_fftw,        /* Select FFTW */
	k_fft_kiss,        /* Select Kiss FFT (always available) */
	k_fft_brenner,     /* Select Brenner FFT (Legacy*/
	k_n_fft_algorithms /* Number of FFT implementations available in GMT */
};

/* Various algorithms for triangulations */
enum GMT_enum_tri {
	GMT_TRIANGLE_WATSON = 0, /* Select Watson's algorithm */
	GMT_TRIANGLE_SHEWCHUK};  /* Select Shewchuk's algorithm */

/* Various 1-D interpolation modes */
enum GMT_enum_spline {
	GMT_SPLINE_LINEAR = 0, /* Linear spline */
	GMT_SPLINE_AKIMA,      /* Akima spline */
	GMT_SPLINE_CUBIC,      /* Cubic spline */
	GMT_SPLINE_NONE};      /* No spline set */

enum GMT_enum_extrap {
	GMT_EXTRAPOLATE_NONE = 0,   /* No extrapolation; set to NaN outside bounds */
	GMT_EXTRAPOLATE_SPLINE,     /* Let spline extrapolate beyond bounds */
	GMT_EXTRAPOLATE_CONSTANT};  /* Set extrapolation beyond bound to specifiec constant */

/* Various line/grid/image interpolation modes */
enum GMT_enum_track {
	GMT_TRACK_FILL = 0,	/* Normal fix_up_path behavior: Keep all (x,y) points but add intermediate if gap > cutoff */
	GMT_TRACK_FILL_M,	/* Fill in, but navigate via meridians (y), then parallels (x) */
	GMT_TRACK_FILL_P,	/* Fill in, but navigate via parallels (x), then meridians (y) */
	GMT_TRACK_SAMPLE_FIX,	/* Resample the track at equidistant points; old points may be lost. Use given spacing */
	GMT_TRACK_SAMPLE_ADJ};	/* Resample the track at equidistant points; old points may be lost. Adjust spacing to fit length of track exactly */

enum GMT_enum_bcr {
	BCR_NEARNEIGHBOR = 0, /* Nearest neighbor algorithm */
	BCR_BILINEAR,         /* Bilinear spline */
	BCR_BSPLINE,          /* B-spline */
	BCR_BICUBIC};         /* Bicubic spline */

/* Various grid/image boundary conditions */
enum GMT_enum_bc {
	GMT_BC_IS_NOTSET = 0, /* BC not yet set */
	GMT_BC_IS_NATURAL,    /* Use natural BC */
	GMT_BC_IS_PERIODIC,   /* Use periodic BC */
	GMT_BC_IS_GEO,        /* Geographic BC condition */
	GMT_BC_IS_DATA};      /* Fill in BC with actual data */

enum GMT_enum_radius {	/* Various "average" radii for an ellipsoid with axes a,a,b */
	GMT_RADIUS_MEAN = 0,	/* Mean radius IUGG R_1 = (2*a+b)/3 = a (1 - f/3) */
	GMT_RADIUS_AUTHALIC,	/* Authalic radius 4*pi*r^2 = surface area of ellipsoid, R_2 = sqrt (0.5a^2 + 0.5b^2 (tanh^-1 e)/e) */
	GMT_RADIUS_VOLUMETRIC,	/* Volumetric radius 3/4*pi*r^3 = volume of ellipsoid, R_3 = (a*a*b)^(1/3) */
	GMT_RADIUS_MERIDIONAL,	/* Meridional radius, M_r = [(a^3/2 + b^3/2)/2]^2/3 */
	GMT_RADIUS_QUADRATIC};	/* Quadratic radius, Q_r = 1/2 sqrt (3a^2 + b^2) */

enum GMT_enum_latswap {GMT_LATSWAP_NONE = -1,	/* Deactivate latswapping */
	GMT_LATSWAP_G2A = 0,	/* input = geodetic;   output = authalic   */
	GMT_LATSWAP_A2G,	/* input = authalic;   output = geodetic   */
	GMT_LATSWAP_G2C,	/* input = geodetic;   output = conformal  */
	GMT_LATSWAP_C2G,	/* input = conformal;  output = geodetic   */
	GMT_LATSWAP_G2M,	/* input = geodetic;   output = meridional */
	GMT_LATSWAP_M2G,	/* input = meridional; output = geodetic   */
	GMT_LATSWAP_G2O,	/* input = geodetic;   output = geocentric */
	GMT_LATSWAP_O2G,	/* input = geocentric; output = geodetic   */
	GMT_LATSWAP_G2P,	/* input = geodetic;   output = parametric */
	GMT_LATSWAP_P2G,	/* input = parametric; output = geodetic   */
	GMT_LATSWAP_O2P,	/* input = geocentric; output = parametric */
	GMT_LATSWAP_P2O,	/* input = parametric; output = geocentric */
	GMT_LATSWAP_N};		/* number of defined swaps  */

/* Help us with big and little endianness */
#ifdef WORDS_BIGENDIAN
#define GMT_BIGENDIAN	true
#define GMT_ENDIAN		'B'
#else
#define GMT_BIGENDIAN	false
#define GMT_ENDIAN		'L'
#endif

#endif  /* _GMT_CONSTANTS_H */
