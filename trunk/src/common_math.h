/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * common_math.h declares shared math functions
 *
 * Author:  Florian Wobbe
 * Date:    10-MAR-2012
 * Version: 5
 */

#pragma once
#ifndef _COMMON_MATH_H
#define _COMMON_MATH_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

	/* CMake definitions: This must be first! */
#include "gmt_config.h"

	/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#ifdef HAVE_STDINT_H_       /* VS 2010 has stdint.h */
#	include <stdint.h>
#else
#	include "compat/stdint.h" /* msinttypes for VC++ */
#endif /* HAVE_STDINT_H_ */

#ifdef HAVE_STDBOOL_H_
#	include <stdbool.h>
#else
#	include "compat/stdbool.h"
#endif /* HAVE_STDBOOL_H_ */

	/* int32_abs function that works with int32_t */
#if defined(SIZEOF_INT) && SIZEOF_INT == 4 && defined(HAVE_ABS)
#	define int32_abs abs
#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 4
#	define int32_abs labs
#else
#	define int32_abs(x) ((int32_t)(((x) >= 0) ? (x) : -(x)))
#endif

	/* int64_abs function that works with int64_t */
#if defined(_WIN64)
#	define int64_abs _abs64
#elif defined(SIZEOF_INT) && SIZEOF_INT == 8 && defined(HAVE_ABS)
#	define int64_abs abs
#elif defined(SIZEOF_LONG) && SIZEOF_LONG == 8
#	define int64_abs labs
#elif defined(SIZEOF_LONG_LONG) && SIZEOF_LONG_LONG == 8 && defined(HAVE_LLABS)
#	define int64_abs llabs
#else
#	define int64_abs(x) ((int64_t)(((x) >= 0) ? (x) : -(x)))
#endif

	/* Limit casting to one place (here) for dropping lrint output to signed or unsigned ints */
#define irint(x) ((int)lrint(x))
#define urint(x) ((unsigned int)lrint(x))
#define irintf(x) ((int)lrintf(x))
#define urintf(x) ((unsigned int)lrintf(x))

	/* Safe rounding of float and double to signed and unsigned 64 bit ints */
#if defined(SIZEOF_INT) && SIZEOF_LONG == 8
#	define irint64(x) lrint(x)
#	define urint64(x) ((uint64_t)lrint(x))
#	define irint64f(x) lrintf(x)
#	define urint64f(x) ((uint64_t)lrintf(x))
#else /* SIZEOF_LONG_LONG == 8 by ISO C definition */
#	define irint64(x) llrint(x)
#	define urint64(x) ((uint64_t)llrint(x))
#	define irint64f(x) llrintf(x)
#	define urint64f(x) ((uint64_t)llrintf(x))
#endif

	EXTERN_MSC bool floatAlmostEqualUlpsAndAbs(float A, float B, float maxDiff, int maxUlpsDiff);
	EXTERN_MSC bool doubleAlmostEqualUlpsAndAbs(double A, double B, double maxDiff, int maxUlpsDiff);
	EXTERN_MSC bool floatAlmostEqualUlps(float A, float B, int maxUlpsDiff);
	EXTERN_MSC bool doubleAlmostEqualUlps(double A, double B, int maxUlpsDiff);

#	define floatAlmostEqual(A, B) (floatAlmostEqualUlps(A, B, 5))
#	define doubleAlmostEqual(A, B) (doubleAlmostEqualUlps(A, B, 5))
#	define floatAlmostEqualZero(A, B) (floatAlmostEqualUlpsAndAbs(A, B, 5*FLT_EPSILON, 5))
#	define doubleAlmostEqualZero(A, B) (doubleAlmostEqualUlpsAndAbs(A, B, 5*DBL_EPSILON, 5))

#ifdef __cplusplus
}
#endif

#endif /* !_COMMON_MATH_H */
