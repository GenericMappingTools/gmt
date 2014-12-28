/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * common_byteswap.h contains inline functions for byteswapping
 *
 * Author:  Florian Wobbe
 * Date:    12-APR-2012
 * Version: 5
 */

#pragma once
#ifndef _COMMON_BYTESWAP_H
#define _COMMON_BYTESWAP_H

#include "gmt_config.h"

#ifdef HAVE_STDINT_H_          /* VS 2010 has stdint.h */
#	include <stdint.h>
#else
#	include "compat/stdint.h"    /* msinttypes for VC++ */
#endif /* HAVE_STDINT_H_ */

/*
 * Default inline functions that the compiler should optimize properly. Use
 * these functions if you know that you are dealing with constant values.
 */
static inline uint16_t inline_bswap16 (uint16_t x) {
	return
		(((x & 0x00FFU) << 8) |
		 ((x & 0xFF00U) >> 8));
}

static inline uint32_t inline_bswap32 (uint32_t x) {
	return
		(((x & 0xFF000000U) >> 24) |
		 ((x & 0x00FF0000U) >>  8) |
		 ((x & 0x0000FF00U) <<  8) |
		 ((x & 0x000000FFU) << 24));
}

static inline uint64_t inline_bswap64 (uint64_t x) {
	return
		(((x & 0x00000000000000FFULL) << 56) |
		 ((x & 0x000000000000FF00ULL) << 40) |
		 ((x & 0x0000000000FF0000ULL) << 24) |
		 ((x & 0x00000000FF000000ULL) << 8) |
		 ((x & 0x000000FF00000000ULL) >> 8) |
		 ((x & 0x0000FF0000000000ULL) >> 24) |
		 ((x & 0x00FF000000000000ULL) >> 40) |
		 ((x & 0xFF00000000000000ULL) >> 56));
}

/*
 * Use builtin functions for bswap16, bswap32, and bswap64 or - if not
 * available - use the default inline functions defined above.
 */

/* Define bswap16 */
#undef bswap16
#ifdef HAVE___BUILTIN_BSWAP16
#	define bswap16 __builtin_bswap16
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#	define bswap16 gnuc_bswap16
	static inline uint16_t gnuc_bswap16(uint16_t x) {
		if (__builtin_constant_p(x))
			x = inline_bswap16(x);
		else {
#		ifdef __x86_64__
			__asm__("xchgb %h0, %b0" : "+Q" (x));
#		elif defined __i386__
			__asm__("xchgb %h0, %b0" : "+q" (x));
#		endif
		}
		return x;
	}
#elif defined HAVE__BYTESWAP_USHORT /* HAVE___BUILTIN_BSWAP16 */
#	define bswap16 _byteswap_ushort
#else /* HAVE___BUILTIN_BSWAP16 */
#	define bswap16 inline_bswap16
#endif /* HAVE___BUILTIN_BSWAP16 */

/* Define bswap32 */
#undef bswap32
#ifdef HAVE___BUILTIN_BSWAP32
#	define bswap32 __builtin_bswap32
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#	define bswap32 gnuc_bswap32
	static inline uint32_t gnuc_bswap32(uint32_t x) {
		if (__builtin_constant_p(x))
			x = inline_bswap32(x);
		else
			__asm__("bswap %0" : "+r" (x));
		return x;
	}
#elif defined HAVE__BYTESWAP_ULONG /* HAVE___BUILTIN_BSWAP32 */
#	define bswap32 _byteswap_ulong
#else /* HAVE___BUILTIN_BSWAP32 */
#	define bswap32 inline_bswap32
#endif /* HAVE___BUILTIN_BSWAP32 */

/* Define bswap64 */
#undef bswap64
#ifdef HAVE___BUILTIN_BSWAP64
#	define bswap64 __builtin_bswap64
#elif defined __GNUC__ && defined __x86_64__
#	define bswap64 gnuc_bswap64
	static inline uint64_t gnuc_bswap64(uint64_t x) {
		if (__builtin_constant_p(x))
			x = inline_bswap64(x);
		else
			__asm__ ("bswap  %0" : "+r" (x));
		return x;
	}
#elif defined HAVE__BYTESWAP_UINT64 /* HAVE___BUILTIN_BSWAP64 */
#	define bswap64 _byteswap_uint64
#else /* HAVE___BUILTIN_BSWAP64 */
#	define bswap64 inline_bswap64
#endif /* HAVE___BUILTIN_BSWAP64 */

#endif /* !_COMMON_BYTESWAP_H */
