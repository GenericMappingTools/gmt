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
 * qsort.h contains prototypes for GLIBC compatible functions
 *   _quicksort, qsort, and qsort_r
 *
 * Author:  Florian Wobbe
 * Date:    7-MAY-2012
 * Version: 5
 */

#pragma once
#ifndef _QSORT_H
#define _QSORT_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

#ifndef HAVE_QSORT_R_GLIBC

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/* Shorthand for type of comparison functions.  */
typedef int (*__compar_fn_t) (const void *, const void *);
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);

/* Optimized quicksort function from GLIBC */
EXTERN_MSC void _quicksort (void *const pbase, size_t total_elems,
		size_t size, __compar_d_fn_t cmp, void *arg);

/* Sort NMEMB elements of BASE, of SIZE bytes each,
	 using COMPAR to perform the comparisons. */
static inline void qsort_glibc (void *base, size_t nmemb, size_t size,
																__compar_fn_t compar) {
	_quicksort (base, nmemb, size, (__compar_d_fn_t) compar, NULL);
}

static inline void qsort_r_glibc (void *base, size_t nmemb, size_t size,
																	__compar_d_fn_t compar, void *arg) {
	_quicksort (base, nmemb, size, compar, arg);
}

#define qsort_r qsort_r_glibc /* override builtin qsort_r */

#endif /* !HAVE_QSORT_R_GLIBC */
#endif /* !_QSORT_H */
