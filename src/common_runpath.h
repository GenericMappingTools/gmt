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
 * common_runpath.h contains prototypes of functions shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 */

#ifndef _GMT_RUNPATH_H
#define _GMT_RUNPATH_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

/* extern char gmt_runpath[PATH_MAX+1]; */

/* Prototypes */
#if defined (__APPLE__)
#	define GMT_runpath(result, argv) GMT_runpath_osx(result)
	EXTERN_MSC char* GMT_runpath_osx (char *result);
#elif defined(_WIN32)
#	define GMT_runpath(result, argv) GMT_runpath_win32(result)
	EXTERN_MSC char* GMT_runpath_win32 (char *result);
#else
	EXTERN_MSC char* GMT_runpath (char *result, const char *candidate);
#endif

EXTERN_MSC char* GMT_guess_sharedir (char* sharedir, const char* runpath);

#ifdef __cplusplus
}
#endif

#endif  /* _GMT_RUNPATH_H */
