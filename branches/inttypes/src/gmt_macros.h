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
/*
 * gmt_macros.h contains definitions of macros used throught GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_MACROS_H
#define _GMT_MACROS_H

/*--------------------------------------------------------------------
 *			GMT MACROS DEFINITIONS
 *--------------------------------------------------------------------*/

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MOD			/* Knuth-style modulo function (remainder after floored division) */
#define MOD(x, y) (x - y * floor((double)(x)/(double)(y)))
#endif

/* Abstraction to deal with 32 vs 64-bit versions of abs(integer) */
#if defined(__LP64__)
#define GMT_abs(n) labs(n)
#elif defined(_WIN64)
#define GMT_abs(n) _abs64(n)
#else
#define GMT_abs(n) abs(n)
#endif

/* Checking of h,m,s */

#define GMT_hms_is_bad(h,m,s) ((h) < 0 || (h) > 23 || (m) < 0 || (m) > 59 || (s) < 0.0 || (s) >= 61.0)

/* Safe math macros that check arguments */

#define d_log10(C,x) ((x) <= 0.0 ? C->session.d_NaN : log10 (x))
#define d_log101p(C,x) ((x) <= -1.0 ? C->session.d_NaN : log10 (1.0+(x)))
#define d_sqrt(x) ((x) < 0.0 ? 0.0 : sqrt (x))
#define d_acos(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? M_PI : 0.0) : acos(x))
#define d_asin(x) (fabs(x) >= 1.0 ? copysign (M_PI_2, (x)) : asin(x))
#define d_atan2(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2(y, x))
#define d_log(C,x) ((x) <= 0.0 ? C->session.d_NaN : log (x))
#define d_log1p(C,x) ((x) <= -1.0 ? C->session.d_NaN : log1p (x))

/* Macros for degree-based trig */

#define sind(x) sin((x) * D2R)
#define cosd(x) cos((x) * D2R)
#define tand(x) tan((x) * D2R)
#define sincosd(x,s,c) sincos((x) * D2R,s,c)
#define asind(x) (asin(x) * R2D)
#define acosd(x) (acos(x) * R2D)
#define atand(x) (atan(x) * R2D)
#define atan2d(y,x) (atan2(y,x) * R2D)

/* Safe versions of the degree-based trig */

#define d_acosd(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? 180.0 : 0.0) : acosd(x))
#define d_asind(x) (fabs(x) >= 1.0 ? copysign (90.0, (x)) : asind(x))
#define d_atan2d(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2d(y,x))

/* Macros for swapping misc data types */

#define l_swap(x, y) {GMT_LONG tmp; tmp = x, x = y, y = tmp;}
#define i_swap(x, y) {int tmp; tmp = x, x = y, y = tmp;}
#define d_swap(x, y) {double tmp; tmp = x, x = y, y = tmp;}
#define f_swap(x, y) {float tmp; tmp = x, x = y, y = tmp;}

/* Macro to simplify call to memcpy when duplicating values and memset when zeroing out */
#define GMT_memcpy(to,from,n,type) memcpy(to, from, (n)*sizeof(type))
#define GMT_memset(array,n,type) memset(array, 0, (n)*sizeof(type))
/* Macro to set all items in an array to the given value */
#define GMT_setnval(array,n,value) {int k; for (k = 0; k < n; k++) array[k] = value;}
/* Macro to simplify assignment of one 3-vector to another */
#define GMT_cpy3v(to,from) memcpy(to, from, 3*sizeof(double))

/* Macro for exit since this should be returned when called from Matlab */

#ifdef DO_NOT_EXIT
#define GMT_exit(code) return(code)
#else
#define GMT_exit(code) exit(code)
#endif

#endif  /* _GMT_MACROS_H */
