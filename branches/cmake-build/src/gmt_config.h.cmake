/*
 * $Id$
 *
 * Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------
 *
 * This include file contains ifdefs that tell us if this system has
 * some of the several functions that are not part of POSIX but are
 * often distributed anyway as part of ANSI C.  The set of defines
 * below is automatically assigned by CMake and determines if the
 * required functions are present or not.  These macros are then used
 * to choose between a function prototype (if found), an alternative
 * GMT function, or simply a macro.  The purpose is to take advantage
 * of the built-in functions if they exist and provide alternative
 * definitions otherwise.
 *
 * HAVE_<func> is undefined or defined as 1 depending on
 * whether or not <func> is available on this system.
 *
 * Version: @GMT_VERSION_STRING@
 */

#ifndef _GMT_CONFIG_H
#define _GMT_CONFIG_H

#include "../config.h"


/* which regex library <pcre.h>, <regex.h> */
#cmakedefine HAVE_PCRE
#cmakedefine HAVE_POSIX_ERE

/* compile with GDAL support <gdal.h> */
#cmakedefine HAVE_GDAL
#ifdef HAVE_GDAL
#define USE_GDAL    /* Unfortunately, GMT uses this define and not HAVE_GDAL */
#endif

/* file locking */
#cmakedefine FLOCK

/* set triangulation method */
#cmakedefine TRIANGLE_D

/* enable compatibility mode */
#cmakedefine GMT_COMPAT

/* applies only #ifdef _WIN32 */
#cmakedefine USE_MEM_ALIGNED

/* if NetCDF is static; applies only #ifdef _WIN32 */
#cmakedefine NETCDF_STATIC

/* Build Matlab API */
#cmakedefine GMT_MATLAB

/* system specific headers */

#cmakedefine HAVE_ASSERT_H_
#cmakedefine HAVE_ERRNO_H_
#cmakedefine HAVE_FCNTL_H_
#cmakedefine HAVE_STAT_H_
#cmakedefine HAVE_TYPES_H_
#cmakedefine HAVE_UNISTD_H_

/* system specific functions */

#cmakedefine HAVE__ACCESS
#cmakedefine HAVE_CHOWN
#cmakedefine HAVE_CLOSEDIR
#cmakedefine HAVE__FILENO
#cmakedefine HAVE_GETCWD
#cmakedefine HAVE__GETCWD
#cmakedefine HAVE_GETPWUID
#cmakedefine HAVE_MKDIR
#cmakedefine HAVE__MKDIR
#cmakedefine HAVE_QSORT_R
#cmakedefine HAVE_QSORT_S
#cmakedefine HAVE__SETMODE
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_STRTOD
#cmakedefine HAVE_STRTOK_R
#cmakedefine HAVE_STRTOK_S

/* windows specific */

#cmakedefine HAVE_DIRECT_H_
#cmakedefine HAVE_IO_H_

/* C types; C99 exact-width integer types <inttypes.h>, <stdint.h>; etc */

#cmakedefine HAVE_CTYPE_H_
#cmakedefine HAVE_INTTYPES_H_
#cmakedefine HAVE_MACHINE_ENDIAN_H_
#cmakedefine HAVE_STDINT_H_

/* Math headers */

#cmakedefine HAVE_FLOATINGPOINT_H_
#cmakedefine HAVE_IEEEFP_H_

/* Math related */

#cmakedefine HAVE_ACOSH
#cmakedefine HAVE_ALPHASINCOS
#cmakedefine HAVE_ASINH
#cmakedefine HAVE_ATANH
#cmakedefine HAVE_COPYSIGN
#cmakedefine HAVE__COPYSIGN
#cmakedefine HAVE_ERF
#cmakedefine HAVE_ERFC
#cmakedefine HAVE_HYPOT
#cmakedefine HAVE_IRINT
#cmakedefine HAVE_ISNAN
#cmakedefine HAVE_ISNAND
#cmakedefine HAVE_ISNANF
#cmakedefine HAVE__ISNAN
#cmakedefine HAVE__ISNANF
#cmakedefine HAVE_J0
#cmakedefine HAVE_J1
#cmakedefine HAVE_JN
#cmakedefine HAVE_LOG1P
#cmakedefine HAVE_LOG2
#cmakedefine HAVE_RINT
#cmakedefine HAVE_SINCOS
#cmakedefine HAVE_Y0
#cmakedefine HAVE_Y1
#cmakedefine HAVE_YN

#endif /* _GMT_CONFIG_H */

/* vim: set ft=c: */
