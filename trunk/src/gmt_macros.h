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

#define GMT_compat_check(C,version) (C->current.setting.compatibility <= version)	/* true if this section should be processed with backwards compatibility to given version */

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MOD			/* Knuth-style modulo function (remainder after floored division) */
#define MOD(x, y) (x - y * floor((double)(x)/(double)(y)))
#endif

/* Checking of h,m,s */

#define GMT_hms_is_bad(h,m,s) ((h) < 0 || (h) > 23 || (m) < 0 || (m) > 59 || (s) < 0.0 || (s) >= 61.0)

/* Safe math macros that check arguments */

#define d_log10(C,x) ((x) <= 0.0 ? C->session.d_NaN : log10 (x))
#define d_log10f(C,x) ((x) <= 0.0f ? C->session.f_NaN : log10f (x))
#define d_log101p(C,x) ((x) <= -1.0 ? C->session.d_NaN : log10 (1.0+(x)))
#define d_log101pf(C,x) ((x) <= -1.0f ? C->session.f_NaN : log10f (1.0f+(x)))
#define d_sqrt(x) ((x) < 0.0 ? 0.0 : sqrt (x))
#define d_acos(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? M_PI : 0.0) : acos(x))
#define d_acosf(x) (fabsf(x) >= 1.0 ? ((x) < 0.0f ? (float)M_PI : 0.0f) : acosf(x))
#define d_asin(x) (fabs(x) >= 1.0 ? copysign (M_PI_2, x) : asin(x))
#define d_asinf(x) (fabsf(x) >= 1.0 ? copysignf ((float)M_PI_2, x) : asinf(x))
#define d_atan2(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2(y, x))
#define d_atan2f(y,x) ((x) == 0.0f && (y) == 0.0f ? 0.0f : atan2f(y, x))
#define d_log(C,x) ((x) <= 0.0 ? C->session.d_NaN : log (x))
#define d_log2(C,x) ((x) <= 0.0f ? C->session.f_NaN : logf (x))
#define d_logf(C,x) ((x) <= 0.0f ? C->session.f_NaN : logf (x))
#define d_log1p(C,x) ((x) <= -1.0 ? C->session.d_NaN : log1p (x))
#define d_log1pf(C,x) ((x) <= -1.0f ? C->session.f_NaN : log1pf (x))

/* Macros for degree-based trig */

#define sind(x) sin((x) * D2R)
#define sindf(x) sinf((x) * D2R)
#define cosd(x) cos((x) * D2R)
#define cosdf(x) cosf((x) * D2R)
#define tand(x) tan((x) * D2R)
#define tandf(x) tanf((x) * D2R)
#define sincosd(x,s,c) sincos((x) * D2R,s,c)
#define asind(x) (asin(x) * R2D)
#define acosd(x) (acos(x) * R2D)
#define atand(x) (atan(x) * R2D)
#define atan2d(y,x) (atan2(y,x) * R2D)
#define atan2df(y,x) (atan2f(y,x) * R2D)

/* Safe versions of the degree-based trig */

#define d_acosd(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? 180.0 : 0.0) : acosd(x))
#define d_asind(x) (fabs(x) >= 1.0 ? copysign (90.0, (x)) : asind(x))
#define d_atan2d(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2d(y,x))

/* Macros for swapping misc data types */

#define bool_swap(x, y) {bool bool_tmp; bool_tmp = x, x = y, y = bool_tmp;}
#define uint64_swap(x, y) {uint64_t uint64_t_tmp; uint64_t_tmp = x, x = y, y = uint64_t_tmp;}
#define int_swap(x, y) {int int_tmp; int_tmp = x, x = y, y = int_tmp;}
#define uint_swap(x, y) {unsigned int uint_tmp; uint_tmp = x, x = y, y = uint_tmp;}
#define double_swap(x, y) {double double_tmp; double_tmp = x, x = y, y = double_tmp;}
#define float_swap(x, y) {float float_tmp; float_tmp = x, x = y, y = float_tmp;}

/* Macro to ensure proper value and sign of a change in longitude from lon1 to lon2 */
#define GMT_set_delta_lon(lon1,lon2,delta) {delta = lon2 - lon1; if (fabs (delta) > 180.0) delta = copysign (360.0 - fabs (delta), -delta);}

/* Macro to simplify call to memcpy when duplicating values and memset when zeroing out */
#define GMT_memcpy(to,from,n,type) memcpy(to, from, (n)*sizeof(type))
#define GMT_memset(array,n,type) memset(array, 0, (n)*sizeof(type))
/* Macro to set all items in an array to the given value */
#define GMT_setnval(array,n,value) {uint64_t k; for (k = 0; k < (uint64_t)n; k++) array[k] = value;}
/* Macro to simplify assignment of one 3-vector to another */
#define GMT_cpy3v(to,from) memcpy(to, from, 3*sizeof(double))

#endif  /* _GMT_MACROS_H */
