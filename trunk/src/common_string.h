/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * common_string.h contains prototypes of functions shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 */

#ifndef _COMMON_STRING_H
#define _COMMON_STRING_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#include "gmt_types.h"

EXTERN_MSC GMT_LONG GMT_strtok (const char *string, const char *sep, GMT_LONG *start, char *token);
EXTERN_MSC void GMT_chop (char *string);
EXTERN_MSC char *GMT_chop_ext (char *string);
EXTERN_MSC void GMT_strstrip(char *string, int strip_leading);
EXTERN_MSC void GMT_cr2lf (char *string);
EXTERN_MSC void GMT_strlshift (char *string, size_t n);
EXTERN_MSC void GMT_strrepc (char *string, int c, int r);
EXTERN_MSC size_t GMT_strlcmp (char *str1, char *str2);

#ifdef WIN32
EXTERN_MSC void DOS_path_fix (char *dir);
#else
# define DOS_path_fix(e) ((void)0) /* dummy function */
#endif

#if !defined(HAVE_STRTOK_R) && !defined(HAVE_STRTOK_S)
EXTERN_MSC char *strtok_r (char *s, const char *delim, char **save_ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _COMMON_STRING_H */
