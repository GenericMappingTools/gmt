/*--------------------------------------------------------------------
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
 */

#ifndef _GMT_NOTPOSIX_H
#define _GMT_NOTPOSIX_H

/* HAVE_<func> is undefined or defined as 1 depending on
 * whether or not <func> is available on this system.
 * The definitions are stored in gmt_config.h */
#include "gmt_config.h"


/*
 * Include POSIX headers
 */

#ifdef HAVE_ASSERT_H_
#	include <assert.h>
#endif

#ifdef HAVE_CTYPE_H_
#	include <ctype.h>
#endif

#ifdef HAVE_STAT_H_
#	include <sys/stat.h>
#endif

#ifdef HAVE_TYPES_H_
#	include <sys/types.h>
#endif

#ifdef HAVE_FCNTL_H_
#	include <fcntl.h>
#endif

#ifdef HAVE_STAT_H_
#	include <sys/stat.h>
#	define GMT_STAT stat
#endif

#ifdef HAVE_UNISTD_H_
#	include <unistd.h>
#endif
#if defined(HAVE_INTTYPES_H_)
#	include <inttypes.h>        /* Exact-width integer types */
#elif defined(HAVE_STDINT_H_)  /* VS 2010 has stdint.h */
#	include <stdint.h>
#else
#	include "pstdint.h"         /* Free portable implementation */
#endif /* HAVE_INTTYPES_H_ */

/*
 * Windows headers
 */

#ifdef HAVE_DIRECT_H_
#	include <direct.h>
#endif

#ifdef HAVE_IO_H_
#	include <io.h>
#endif

/*
 * Math headers
 */

#if defined (HAVE_IEEEFP_H_) && defined(__ultrix__) && defined(__mips)
/* Needed to get isnan[fd] macros */
#	include <ieeefp.h>
#endif

#ifdef HAVE_FLOATINGPOINT_H_
#	include <floatingpoint.h>
#endif

/* Misc. ANSI-C math functions used by grdmath and gmtmath.
 * These functions are available on many platforms and we
 * seek to use them.  If not available then we compile in
 * replacements from gmt_notposix.c */

#ifndef HAVE_ACOSH
#	define acosh(x) log((x) + (d_sqrt((x) + 1.0)) * (d_sqrt((x) - 1.0)))
#endif

#ifndef HAVE_ASINH
#	define asinh(x) log((x) + (hypot((x), 1.0)))
#endif

#ifndef HAVE_ATANH
	EXTERN_MSC double atanh(double x);
#endif

#if defined HAVE__COPYSIGN && !defined HAVE_COPYSIGN
#	define copysign _copysign
#elif !defined HAVE_COPYSIGN
#	define copysign(x,y) ((y) < 0.0 ? -fabs(x) : fabs(x))
#endif

#ifndef HAVE_ERF
	EXTERN_MSC double erf(double x);
#endif

#ifndef HAVE_ERFC
	EXTERN_MSC double erfc(double x);
#endif

#ifndef HAVE_HYPOT
	EXTERN_MSC double hypot(double x, double y);
#endif

#ifndef HAVE_IRINT
#	define irint (int)rint
#endif

#if defined HAVE__ISNAN && !defined HAVE_ISNAN
#	define isnan _isnan
#elif !defined HAVE_ISNAN
/* define custom function */
#endif

#if defined HAVE__ISNANF && !defined HAVE_ISNANF
#	define isnanf _isnanf
#elif !defined HAVE_ISNANF
/* define custom function */
#endif

#ifndef HAVE_J0
	EXTERN_MSC double j0(double x);
#endif

#ifndef HAVE_J1
	EXTERN_MSC double j1(double x);
#endif

#ifndef HAVE_JN
	EXTERN_MSC double jn(double x);
#endif

#ifndef HAVE_LOG1P
	EXTERN_MSC double log1p(double x);
#endif

#ifndef HAVE_LOG2
#	define log2(x) (log10(x)/0.30102999566398114250631579125183634459972381591796875)
#endif

#ifndef HAVE_RINT
	/* #define rint(x) (floor((x)+0.5f)) does not work reliable.
	 * We use s_rint.c from sun instead. */
	EXTERN_MSC double rint(double x);
#endif

/* On Dec Alpha OSF1 there is a sincos with different syntax.
 * Assembly wrapper provided by Lloyd Parkes <lloyd@must-have-coffee.gen.nz>
 * can be used instead. See alpha-sincos.s */
#if !defined(HAVE_SINCOS) && !defined(HAVE_ALPHASINCOS)
	EXTERN_MSC void sincos (double x, double *s, double *c);
#elif defined(HAVE_ALPHASINCOS)
#	define sincos alpha_sincos
#endif

#ifndef HAVE_Y0
	EXTERN_MSC double y0(double x);
#endif

#ifndef HAVE_Y1
	EXTERN_MSC double y1(double x);
#endif

#ifndef HAVE_YN
	EXTERN_MSC double yn(double x);
#endif

/*
 * System specific
 */

/* access is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */
#if defined HAVE__ACCESS && !defined HAVE_UNISTD
#	define access _access
#endif

/* fileno is usually in stdio.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in stdio.h */
#if defined HAVE__FILENO
#	define fileno _fileno
#endif

/* getcwd is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _;
 * it is defined in direct.h. */
#if defined HAVE__GETCWD && !defined HAVE_GETCWD
#	define getcwd _getcwd
#endif

/* mkdir is usually in sys/stat.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and furthermore does not pass the mode argument;
 * it is defined in direct.h
 * The above was copyed from the old gmt_notunix and is left here
 * because it probably refers to older MSVC compilers, but is not for VS2010 */
#if defined HAVE__MKDIR && !defined HAVE_MKDIR 	/* Visual Studio 2010 has both */
#	define mkdir(path,mode) _mkdir(path)
#endif

#if defined HAVE_QSORT_S && !defined HAVE_QSORT_R
#	define qsort_r qsort_s
#elif !defined HAVE_QSORT_R
/* define custom function */
#endif

#if defined HAVE__SETMODE
#	define setmode _setmode
#endif

#ifndef HAVE_STRDUP
	EXTERN_MSC char *strdup(const char *s);
#endif

#ifndef HAVE_STRTOD
	EXTERN_MSC double strtod(const char *nptr, char **endptr);
#endif

#if defined HAVE_STRTOK_S && !defined HAVE_STRTOK_R
#	define strtok_r strtok_s
#elif !defined HAVE_STRTOK_R
/* define custom function */
#endif

/* GMT normally gets these macros from unistd.h */
#ifndef HAVE_UNISTD_H_
#	define R_OK 04
#	define W_OK 02
#	define X_OK 01
#	define F_OK 00
#endif /* !HAVE_UNISTD_H_ */

/*
 * Make sure Cygwin does not use Windows related tweaks
 */

#ifdef __CYGWIN__
#	undef _WIN32
#	undef WIN32
#endif

/*
 * Windows tweaks
 */

#if defined _WIN32 || defined WIN32

#	ifndef WIN32
#		define WIN32
#	endif

	typedef int mode_t;       /* mode_t not defined under Windows */

#	define PATH_SEPARATOR ';' /* Win uses ; while Unix uses : */

	EXTERN_MSC void GMT_setmode (struct GMT_CTRL *C, int direction);
	EXTERN_MSC void DOS_path_fix (char *dir);

	/* FLOCK is a pain. If cannot be used under Windows.
	 * Also, users have problems with file locking because their
	 * NFS does not support it. Only those who are really sure should
	 * activate -DFLOCK. For these reasons, FLOCK is off by default.
	 */
#	undef FLOCK          /* Do not support file locking */
#	define SET_IO_MODE   /* Need to force binary i/o upon request */

#	if defined(USE_VLD) && defined(DEBUG)
#		include <vld.h>
#	endif

#endif /* defined _WIN32 || defined WIN32 */

#ifndef PATH_SEPARATOR
#	define PATH_SEPARATOR ':' /* Win uses ; while Unix uses : */
#endif

/* Must replace the system qsort with ours which is 64-bit compliant
 * See gmt_qsort.c. */
#ifdef GMT_QSORT
	EXTERN_MSC void GMT_qsort (void *a, size_t n, size_t es, int (*cmp) (const void *, const void *));
#	define qsort GMT_qsort
#endif /* GMT_QSORT */

#endif /* _GMT_NOTPOSIX_H */
