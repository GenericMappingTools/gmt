/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * gmt_macros.h contains definitions of macros used through GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	6 API
 */

/*!
 * \file gmt_macros.h
 * \brief Definitions of macros used through GMT.
 */

#ifndef GMT_MACROS_H
#define GMT_MACROS_H

/*--------------------------------------------------------------------
 *			GMT MACROS DEFINITIONS
 *--------------------------------------------------------------------*/

#define gmt_M_compat_check(C,version) (C->current.setting.compatibility <= version)	/* true if this section should be processed with backwards compatibility to given version */

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))	/* min and max value macros */
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MOD	/* Knuth-style modulo function (remainder after floored division) */
#define MOD(x, y) (x - y * floor((double)(x)/(double)(y)))
#endif

#ifdef DOUBLE_PRECISION_GRID
#define GMT_GRDFLOAT GMT_DOUBLE
#define gmt_nc_get_vara_grdfloat nc_get_vara_double
#define gmt_nc_put_vara_grdfloat nc_put_vara_double
#define gmt_nc_get_varm_grdfloat nc_get_varm_double
#define gmt_nc_put_varm_grdfloat nc_put_varm_double
#else
#define gmt_nc_get_vara_grdfloat nc_get_vara_float
#define gmt_nc_put_vara_grdfloat nc_put_vara_float
#define gmt_nc_get_varm_grdfloat nc_get_varm_float
#define gmt_nc_put_varm_grdfloat nc_put_varm_float
#endif

/*! Safe math macros that check arguments */

#define d_log2(C,x) ((x) <= 0.0f ? C->session.f_NaN : log2 (x))
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
#define d_logf(C,x) ((x) <= 0.0f ? C->session.f_NaN : logf (x))
#define d_log1p(C,x) ((x) <= -1.0 ? C->session.d_NaN : log1p (x))
#define d_log1pf(C,x) ((x) <= -1.0f ? C->session.f_NaN : log1pf (x))

/*! Macros for degree-based trig */

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

/*! Safe versions of the degree-based trig */

#define d_acosd(x) (fabs(x) >= 1.0 ? ((x) < 0.0 ? 180.0 : 0.0) : acosd(x))
#define d_asind(x) (fabs(x) >= 1.0 ? copysign (90.0, (x)) : asind(x))
#define d_atan2d(y,x) ((x) == 0.0 && (y) == 0.0 ? 0.0 : atan2d(y,x))

/* Extract column type for given direction and column number */
#define gmt_M_type(C,dir,col) (C->current.io.col_type[dir][col])
/* Compare column type to given type -- true if the same */
#define gmt_M_is_type(C,dir,col,type) (gmt_M_type(C,dir,col) == type)

/*! Macros for swapping misc data types */

#define gmt_M_bool_swap(x, y) {bool bool_tmp; bool_tmp = x, x = y, y = bool_tmp;}
#define gmt_M_char_swap(x, y) {char char_tmp; char_tmp = x, x = y, y = char_tmp;}
#define gmt_M_charp_swap(x, y) {char *char_tmp; char_tmp = x, x = y, y = char_tmp;}
#define gmt_M_uint64_swap(x, y) {uint64_t uint64_t_tmp; uint64_t_tmp = x, x = y, y = uint64_t_tmp;}
#define gmt_M_int_swap(x, y) {int int_tmp; int_tmp = x, x = y, y = int_tmp;}
#define gmt_M_uint_swap(x, y) {unsigned int uint_tmp; uint_tmp = x, x = y, y = uint_tmp;}
#define gmt_M_double_swap(x, y) {double double_tmp; double_tmp = x, x = y, y = double_tmp;}
#define gmt_M_float_swap(x, y) {float float_tmp; float_tmp = x, x = y, y = float_tmp;}

/*! Macro to ensure proper value and sign of a change in longitude from lon1 to lon2 */
#define gmt_M_set_delta_lon(lon1,lon2,delta) {delta = fmod ((lon2) - (lon1), 360.0); if (fabs (delta) > 180.0) delta = copysign (360.0 - fabs (delta), -delta);}

/*! Macro to simplify call to memcpy when duplicating values and memset when zeroing out */
#define gmt_M_memcpy(to,from,n,type) memcpy(to, from, (n)*sizeof(type))
#define gmt_M_memset(array,n,type) memset(array, 0, (n)*sizeof(type))
/*! Macro to set all items in an array to the given value */
#define gmt_M_setnval(array,n,value) {uint64_t k; for (k = 0; k < (uint64_t)n; k++) array[k] = value;}
/*! Macro to simplify assignment of one 3-vector to another */
#define gmt_M_cpy3v(to,from) memcpy(to, from, 3*sizeof(double))

/*! Macros for printing a tic/toc elapsed time message*/
#define gmt_M_tic(C) {if (C->current.setting.verbose == GMT_MSG_TICTOC) GMT_Message(C->parent,GMT_TIME_RESET,"");}
#define gmt_M_toc(C,...) {if (C->current.setting.verbose == GMT_MSG_TICTOC) GMT_Message(C->parent,GMT_TIME_ELAPSED, \
		"(%s) | %s\n", C->init.module_name, __VA_ARGS__);}

/*! Cleaner check to see if a line is associated with the extended syntax or not */
#define gmt_M_showusage(API) (!API->GMT->common.synopsis.extended)

/* COLOR MACROS */

/* Determine if fill is in fact a pattern */
/* Old: always starts with integer dpi.
 * New: Either start with pattern 1-88 or a file which should have an extension */
#define gmt_M_is_pattern(txt) ((txt[0] == 'p' || txt[0] == 'P') && (isdigit((int)txt[1]) || strchr(txt,'.')))

/* Determine if this CPT slice requires a pattern */
#define gmt_M_cptslice_is_pattern(P,index) ((index >= 0 && P->data[index].fill != NULL) || (index < 0 && P->bfn[index+3].fill != NULL))

/* Determine if this CPT slice requires a pattern */
#define gmt_M_get_cptslice_pattern(P,index) ((index >= 0) ? P->data[index].fill : P->bfn[index+3].fill)

/* Determine if we should skip this CPT slice */
#define gmt_M_skip_cptslice(P,index) ((index >= 0 && P->data[index].skip) || (index < 0 && P->bfn[index+3].skip))

/* See if CPT modifiers was given (+u|U modifier) */
#define gmt_M_cpt_mod(arg) ((arg) && ((arg)[0] =='+' && strchr ("uU", (arg)[1])))

/* See if no CPT name was given (+u|U modifier may be present but not filename) */
#define gmt_M_no_cpt_given(arg) (arg == NULL || arg[0] == '\0' || gmt_M_cpt_mod(arg))

/*! Copy two RGB[T] arrays (a = b) */
#define gmt_M_rgb_copy(a,b) memcpy (a, b, 4 * sizeof(double))

/*! To compare is two colors are ~ the same */
#define gmt_M_eq(a,b) (fabs((a)-(b)) < GMT_CONV4_LIMIT)
#define gmt_M_same_rgb(a,b) (gmt_M_eq(a[0],b[0]) && gmt_M_eq(a[1],b[1]) && gmt_M_eq(a[2],b[2]) && gmt_M_eq(a[3],b[3]))

/*! To compare is two pens are ~ the same */
#define gmt_M_same_pen(a,b) (gmt_M_eq(a.width,b.width) && gmt_M_eq(a.offset,b.offset) && gmt_M_same_rgb(a.rgb,b.rgb) && !strcmp (a.style, b.style))

/*! Macros for conversion of RGB in 0-1 range to 0-255 range */
#define gmt_M_s255(s) ((s) * 255.0)
#define gmt_M_t255(t) gmt_M_q(gmt_M_s255(t[0])),gmt_M_q(gmt_M_s255(t[1])),gmt_M_q(gmt_M_s255(t[2]))
#define gmt_M_u255(s) ((unsigned char)rint(gmt_M_s255(s)))

/*! Macros for conversion of RGB in 0-255 range to 0-1 range */
#define gmt_M_is255(s) ((s) / 255.0)

/*! Macro to avoid small numbers in color codes */
#define gmt_M_q(s) ((s) < 1e-5 ? 0.0 : (s))

/*! How B/W TV's convert RGB to Gray */
#define gmt_M_yiq(rgb) (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2])

/*! Determine if a RGB combination is grayshade */
#define gmt_M_is_gray(rgb) (gmt_M_eq(rgb[0],rgb[1]) && gmt_M_eq(rgb[1],rgb[2]))

/*! Determine if a RGB combination is in fact B/W */
#define gmt_M_is_bw(rgb) (gmt_M_is_gray(rgb) && (gmt_M_eq(rgb[0],0.0) || gmt_M_eq(rgb[0],1.0)))

/*! Macros to do conversion to inches with PROJ_LENGTH_UNIT as default */

#define gmt_M_to_inch(GMT,value) gmt_convert_units (GMT, value, GMT->current.setting.proj_length_unit, GMT_INCH)
#define gmt_M_to_points(GMT,value) gmt_convert_units (GMT, value, GMT->current.setting.proj_length_unit, GMT_PT)

/*! Determine default justification for box item */
#define gmt_M_just_default(GMT,refpoint,just) (refpoint->mode == GMT_REFPOINT_JUST_FLIP ? gmt_flip_justify(GMT,refpoint->justify) : refpoint->mode == GMT_REFPOINT_JUST ? refpoint->justify : just)

/*! Determine if we have a special downloadable file */
#define gmt_M_file_is_remotedata(file) (file && file[0] == '@' && !strncmp (&file[1], GMT_DATA_PREFIX, 13U))
#define gmt_M_file_is_cache(file) (file && file[0] == '@' && strncmp (file, "@GMTAPI@-", 9U))
#define gmt_M_file_is_url(file) (file && (!strncmp (file, "http:", 5U) || !strncmp (file, "https:", 6U) || !strncmp (file, "ftp:", 4U)))

/*! Determine if file is an image GDAL can read */
#define gmt_M_file_is_image(file) (file && (strstr (file, "=gd") || strstr (file, ".jpg") || strstr (file, ".png") || strstr (file, ".ppm") || strstr (file, ".tif") || strstr (file, ".bmp") || strstr (file, ".gif")))

/*! Set the correct column mode (trailing vs no trailing text) based on the given string is NULL or not */
#define gmt_M_colmode(text) ((text == NULL) ? GMT_COL_FIX_NO_TEXT : GMT_COL_FIX)

#endif  /* GMT_MACROS_H */
