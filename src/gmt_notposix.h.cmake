/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * This include file contains ifdefs that tell us if this system has
 * some of the several functions that are not part of POSIX but are
 * often distributed anyway as part of ANSI C.  The set of defines
 * below is automatically assigned by CMake and determines if the
 * required functions are present or not.  These macros are then used
 * in gmt_math.h to choose between a function prototype (if found), an
 * alternative GMT function, or simply a macro.  The purpose is to
 * take advantage of the built-in functions if they exist and provide
 * alternative definitions otherwise.  For some non-Unix Operating
 * Systems, like Win32, these settings have already been hard-wired
 * into gmt_notunix.h and _GMT_NOTPOSIX_H is defined there so we do not
 * override those system-specific settings.
 *
 * Version:	5
 */

#ifndef _GMT_NOTPOSIX_H
#define _GMT_NOTPOSIX_H

/* >>> THIS SECTION WILL BE MODIFIED BY configure <<<
 * >>> DO NOT EDIT UNLESS YOU KNOW WHAT YOU ARE DOING!! <<<
 */

/* HAVE_<func> is undefined or defined as 1 depending on whether or
 * not <func> is available on this system.  The default setting is undefined:
 * none of the functions are available (the POSIX standard) */

#cmakedefine HAVE_COPYSIGN
#cmakedefine HAVE_LOG2
#cmakedefine HAVE_LOG1P
#cmakedefine HAVE_HYPOT
#cmakedefine HAVE_ACOSH
#cmakedefine HAVE_ASINH
#cmakedefine HAVE_ATANH
#cmakedefine HAVE_RINT
#cmakedefine HAVE_IRINT
#cmakedefine HAVE_ISNANF
#cmakedefine HAVE_ISNAND
#cmakedefine HAVE_ISNAN
#cmakedefine HAVE_J0
#cmakedefine HAVE_J1
#cmakedefine HAVE_JN
#cmakedefine HAVE_Y0
#cmakedefine HAVE_Y1
#cmakedefine HAVE_YN
#cmakedefine HAVE_ERF
#cmakedefine HAVE_ERFC
#cmakedefine HAVE_SINCOS
#cmakedefine HAVE_ALPHASINCOS
#cmakedefine HAVE_GETPWUID
#cmakedefine HAVE_QSORT_R
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_STRTOD
#cmakedefine HAVE_STRTOK_R
#cmakedefine WORDS_BIGENDIAN

#endif /* _GMT_NOTPOSIX_H */
