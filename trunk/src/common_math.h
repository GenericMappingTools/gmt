/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * common_math.h declares shared math functions determined by configure.
 * It is included by gmt_lib.h and gmtspatial.c
 * It depends on configure and is not distributed as part of the API or DEV.
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

#ifdef __cplusplus
}
#endif

#endif /* !_COMMON_MATH_H */
