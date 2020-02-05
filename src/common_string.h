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
 * common_string.h contains prototypes of functions shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 */

/*!
 * \file common_string.h
 * \brief Prototypes of functions shared between GMT and PSL
 */

#pragma once
#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#include <stdbool.h>

#include <limits.h> /* defines PATH_MAX */
#include <stdlib.h> /* defines _MAX_PATH on WIN32 */
#ifndef PATH_MAX
#	define PATH_MAX 1024
#endif

EXTERN_MSC unsigned int gmt_strtok (const char *string, const char *sep, unsigned int *start, char *token);
EXTERN_MSC void gmt_strtok_m (char *in, char **token, char **remain, char *sep);
EXTERN_MSC unsigned int gmt_get_modifier (const char *string, char modifier, char *token);
EXTERN_MSC void gmt_chop (char *string);
EXTERN_MSC char *gmt_chop_ext (char *string);
EXTERN_MSC char *gmt_get_ext (const char *string);
EXTERN_MSC void gmt_strstrip(char *string, bool strip_leading);
EXTERN_MSC void gmt_strlshift (char *string, size_t n);
EXTERN_MSC void gmt_strrepc (char *string, int c, int r);
EXTERN_MSC char *gmt_strrep(const char *s1, const char *s2, const char *s3);
EXTERN_MSC size_t gmt_strlcmp (char *str1, char *str2);
EXTERN_MSC char *gmt_strdup_noquote(const char *string);

#ifdef WIN32
EXTERN_MSC void gmt_dos_path_fix (char *dir);
#else
# define gmt_dos_path_fix(e) ((void)0) /* dummy function */
#endif

#if !defined(HAVE_STRTOK_R) && !defined(HAVE_STRTOK_S)
EXTERN_MSC char *strtok_r (char *s, const char *delim, char **save_ptr);
#endif

#ifndef DECLARED_STRSEP
EXTERN_MSC char *strsep (char **stringp, const char *delim);
#elif !defined(HAVE_STRSEP)
EXTERN_MSC char *strsep (char **stringp, const char *delim);
#endif
EXTERN_MSC char *strsepz  (char **stringp, const char *delim, size_t *pos);
EXTERN_MSC char *strsepzp (char **stringp, const char *delim, size_t *pos);
EXTERN_MSC char *stresep (char **stringp, const char *delim, int esc);

#ifndef HAVE_BASENAME
EXTERN_MSC char *basename(char *path);
#endif

#ifdef __cplusplus
}
#endif

#endif /* !COMMON_STRING_H */
