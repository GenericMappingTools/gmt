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
#cmakedefine HAVE__COPYSIGN
#cmakedefine HAVE_LOG2
#cmakedefine HAVE_LOG1P
#cmakedefine HAVE_HYPOT
#cmakedefine HAVE_ACOSH
#cmakedefine HAVE_ASINH
#cmakedefine HAVE_ATANH
#cmakedefine HAVE_RINT
#cmakedefine HAVE_IRINT
#cmakedefine HAVE_ISNANF
#cmakedefine HAVE__ISNANF
#cmakedefine HAVE_ISNAND
#cmakedefine HAVE_ISNAN
#cmakedefine HAVE__ISNAN
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
#cmakedefine HAVE_QSORT_S
#cmakedefine HAVE_STRDUP
#cmakedefine HAVE_STRTOD
#cmakedefine HAVE_STRTOK_R
#cmakedefine HAVE_STRTOK_S
#cmakedefine WORDS_BIGENDIAN
#cmakedefine HAVE_GETCWD
#cmakedefine HAVE__GETCWD
#cmakedefine HAVE__ACCESS
#cmakedefine HAVE_MKDIR
#cmakedefine HAVE__MKDIR
#cmakedefine HAVE__FILENO
#cmakedefine HAVE__SETMODE

#if defined HAVE__COPYSIGN && !defined HAVE_COPYSIGN
#define copysign(x,y) _copysign(x,y)
#elif !defined HAVE_COPYSIGN
/* define custom function */
#endif

#if defined HAVE__ISNAN && !defined HAVE_ISNAN
#define isnan(x) _isnan(x)
#elif !defined HAVE_ISNAN
/* define custom function */
#endif

#if defined HAVE__ISNANF && !defined HAVE_ISNANF
#define isnanf(x) _isnanf(x)
#elif !defined HAVE_ISNANF
/* define custom function */
#endif

#if defined HAVE_QSORT_S && !defined HAVE_QSORT_R
#define qsort_r(x) qsort_s(x)
#elif !defined HAVE_QSORT_R
/* define custom function */
#endif

#if defined HAVE_STRTOK_S && !defined HAVE_STRTOK_R
#define strtok_r(x) strtok_s(x)
#elif !defined HAVE_STRTOK_R
/* define custom function */
#endif

/* getcwd is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _;
 * it is defined in direct.h. */
#if defined HAVE__GETCWD && !defined HAVE_GETCWD
#define getcwd(path, len) _getcwd(path, len)
#endif

/* access is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */
#if defined HAVE__ACCESS && !defined HAVE_UNISTD
#define access(path, mode) _access(path, mode)
#endif

/* mkdir is usually in sys/stat.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and furthermore does not pass the mode argument;
 * it is defined in direct.h
 * The above was copyed from the old gmt_notunix and is left here
 * because it probably refers to older MSVC compilers, but is not for VS2010 */
#if defined HAVE__MKDIR && !defined HAVE_MKDIR 	/* Visual Studio 2010 has both */
#define mkdir(path,mode) _mkdir(path)
#endif

/* fileno is usually in stdio.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in stdio.h */
#if defined HAVE__FILENO
#define fileno(stream) _fileno(stream)
#endif

#if defined HAVE__SETMODE
#define setmode(fd,mode) _setmode(fd,mode)
#endif

#ifdef _WIN32

typedef int mode_t;		/* mode_t not defined under Windows; assumed a signed 4-byte integer */

#define PATH_SEPARATOR ';'	/* Win uses ; while Unix uses : */

#include <io.h>
#include <direct.h>

/* GMT normally gets these macros from unistd.h */

#define R_OK 04
#define W_OK 02
#define X_OK 01
#define F_OK 00

EXTERN_MSC void GMT_setmode (struct GMT_CTRL *C, int direction);
EXTERN_MSC void DOS_path_fix (char *dir);

/* FLOCK is a pain. If cannot be used under Windows.
 * Also, users have problems with file locking because their 
 * NFS does not support it. Only those who are really sure should
 * activate -DFLOCK. For these reasons, FLOCK is off by default.
 */
#undef FLOCK		/* Do not support file locking */
#define SET_IO_MODE	/* Need to force binary i/o upon request */

#endif	/* _WIN32 */


#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR ':'	/* Win uses ; while Unix uses : */
#endif

#ifndef NO_FCNTL
#include <fcntl.h>
#endif

#ifndef GMT_STAT
#define GMT_STAT stat
#endif

#endif /* _GMT_NOTPOSIX_H */
