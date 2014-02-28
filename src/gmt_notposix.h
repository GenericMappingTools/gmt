/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
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

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/*
 * Include POSIX headers
 */

#include <stdlib.h>

#ifdef HAVE_ASSERT_H_
#	include <assert.h>
#else
#	define assert(e) ((void)0)
#endif

#ifdef HAVE_BASENAME
#	include <libgen.h>
#else
#	define basename GMT_basename
#endif

#ifdef HAVE_CTYPE_H_
#	include <ctype.h>
#endif

#ifdef HAVE_STDBOOL_H_
#	include <stdbool.h>
#else
#	include "compat/stdbool.h"
#endif

#ifdef HAVE_SYS_TYPES_H_
#	include <sys/types.h>
#endif

#ifndef SIZEOF_MODE_T
  /* MSC does not define mode_t */
	typedef unsigned int mode_t;
#endif

#ifdef HAVE_FCNTL_H_
#	include <fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H_
#	include <sys/stat.h>
#endif

#ifdef HAVE_STDDEF_H_
#	include <stddef.h>
#endif

#ifdef HAVE_UNISTD_H_
#	include <unistd.h>
#endif

#ifdef HAVE_STDINT_H_          /* VS 2010 has stdint.h */
#	include <stdint.h>
#else
#	include "compat/stdint.h"    /* msinttypes for VC++ */
#endif /* HAVE_STDINT_H_ */

#ifdef HAVE_INTTYPES_H_
#	include <inttypes.h>         /* Exact-width integer types */
#else
#	include "compat/inttypes.h"  /* msinttypes for VC++ */
#endif /* HAVE_INTTYPES_H_ */

/* Size prefixes for printf/scanf for size_t and ptrdiff_t */
#ifdef _MSC_VER
#	define PRIxS "Ix"  /* printf size_t */
#	define PRIuS "Iu"  /* printf size_t */
#	define PRIdS "Id"  /* printf ptrdiff_t */
#	define SCNuS "u"   /* scanf  size_t (__int32 even on 64-bit platforms) */
#	define SCNdS "d"   /* scanf  ptrdiff_t (__int32 even on 64-bit platforms) */
#else
#	define PRIxS "zx"  /* printf size_t */
#	define PRIuS "zu"  /* printf size_t */
#	define PRIdS "zd"  /* printf ptrdiff_t */
#	define SCNuS PRIuS /* scanf  size_t */
#	define SCNdS PRIdS /* scanf  ptrdiff_t */
#endif

/*
 * Windows headers
 */

#ifdef HAVE_DIRECT_H_
#	include <direct.h>
#endif

#ifdef HAVE_IO_H_
#	include <io.h>
#endif

#ifdef HAVE_PROCESS_H_
#	include <process.h>
#endif

/*
 * Math headers
 */

#include <math.h>
#include <float.h>

#if defined (HAVE_IEEEFP_H_) && defined(__ultrix__) && defined(__mips)
/* Needed to get isnan[fd] macros */
#	include <ieeefp.h>
#endif

#ifdef HAVE_FLOATINGPOINT_H_
#	include <floatingpoint.h>
#endif

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

#if defined _WIN32

#	ifndef WIN32
#		define WIN32
#	endif

	/* Reduce the size of the Win32 header files and speed up compilation. */
#	define WIN32_LEAN_AND_MEAN

#	define PATH_SEPARATOR ';' /* Win uses ; while Unix uses : */

#	define SET_IO_MODE   /* Need to force binary i/o upon request */

#	if defined(USE_VLD) && defined(DEBUG)
#		include <vld.h>
#	endif

#	ifdef _MSC_VER
		/* Suppress Visual Studio deprecation warnings */
#		pragma warning( disable : 4996 )
		/* Issue warning 4244 (conversion of int64_t to int32_t) only once */
/*#		pragma warning( once : 4244 4267 ) */
#	 	if (_MSC_VER <= 1600)
			/* Older Visual Studio do not understand C99 restrict keyword */
#			define restrict
# 		endif

		/* isspace, isalpha, ...: avoid assert (only happens with debug CRT) 
		   if passed a parameter that isn't EOF or in the range of 0 through 0xFF. */
#		ifdef _DEBUG
#			define isspace(c) (c > 0 && c < 0xFF && isspace(c))
#			define isalpha(c) (c > 0 && c < 0xFF && isalpha(c))
#		endif /* _DEBUG */

#	endif /* defined _MSC_VER */

#endif /* defined _WIN32 */

#ifndef PATH_SEPARATOR
#	define PATH_SEPARATOR ':' /* Win uses ; while Unix uses : */
#endif

/* Misc. ANSI-C math functions used by grdmath and gmtmath.
 * These functions are available on many platforms and we
 * seek to use them.  If not available then we compile in
 * replacements from gmt_notposix.c */

#ifndef HAVE_ABS
#	define abs(x) (int)labs(x)
#endif
#ifndef HAVE_ACOSF
#	define acosf(x) (float)acos((double)(x))
#endif
#ifndef HAVE_ACOSH
#	define acosh(x) log((x) + (d_sqrt((x) + 1.0)) * (d_sqrt((x) - 1.0)))
#endif
#ifndef HAVE_ACOSHF
#	define acoshf(x) (float)acosh((double)(x))
#endif
#ifndef HAVE_ASINF
#	define asinf(x) (float)asin((double)(x))
#endif
#ifndef HAVE_ASINH
#	define asinh(x) log((x) + (hypot((x), 1.0)))
#endif
#ifndef HAVE_ASINHF
#	define asinhf(x) (float)asinh((double)(x))
#endif
#ifndef HAVE_ATANF
#	define atanf(x) (float)atan((double)(x))
#endif
#ifndef HAVE_ATAN2F
#	define atan2f(y,x) (float)atan2((double)(y),(double)(x))
#endif
#ifndef HAVE_ATANH
	EXTERN_MSC double atanh(double x);
#endif
#ifndef HAVE_ATANHF
#	define atanhf(x) (float)atanh((double)(x))
#endif

#if defined HAVE__COPYSIGN && !defined HAVE_COPYSIGN
#	define copysign _copysign
#elif !defined HAVE_COPYSIGN
#	define copysign(x,y) ((y) < 0.0 ? -fabs(x) : fabs(x))
#endif
#ifndef HAVE_COPYSIGNF
#	define copysignf(x,y) (float)copysign((double)(x),(double)(y))
#endif

#ifndef HAVE_CEILF
#	define ceilf(x) (float)ceil((double)(x))
#endif
#ifndef HAVE_COSF
#	define cosf(x) (float)cos((double)(x))
#endif
#ifndef HAVE_COSHF
#	define coshf(x) (float)cosh((double)(x))
#endif
#ifndef HAVE_ERF
	EXTERN_MSC double erf(double x);
#endif
#ifndef HAVE_ERFF
#	define erff(x) (float)erf((double)(x))
#endif
#ifndef HAVE_ERFC
	EXTERN_MSC double erfc(double x);
#endif
#ifndef HAVE_ERFCF
#	define erfcf(x) (float)erfc((double)(x))
#endif
#ifndef HAVE_EXPF
#	define expf(x) (float)exp((double)(x))
#endif
#ifndef HAVE_FLOORF
#	define floorf(x) (float)floor((double)(x))
#endif
#ifndef HAVE_FMODF
#	define fmodf(x,y) (float)fmod((double)(x),(double)(y))
#endif
#ifndef HAVE_HYPOT
	EXTERN_MSC double hypot(double x, double y);
#endif
#ifndef HAVE_HYPOTF
#	define hypotf(x,y) (float)hypot((double)(x),(double)(y))
#endif
#ifndef HAVE_LOGF
#	define logf(x) (float)log((double)(x))
#endif
#ifndef HAVE_LOG2F
#	define log2f(x) (float)log2((double)(x))
#endif
#ifndef HAVE_LOG10F
#	define log10f(x) (float)log10((double)(x))
#endif
#ifndef HAVE_LOG1PF
#	define log1pf(x) (float)log1p((double)(x))
#endif
#ifndef HAVE_LRINT
#	define lrint(x) (long)rint(x)
#endif
#ifndef HAVE_LRINTF
#	define lrintf(x) lrint((double)(x))
#endif
#ifndef HAVE_LLRINT
#	define llrint(x) (long long)rint(x)
#endif
#ifndef HAVE_LLRINTF
#	define llrintf(x) llrint((double)(x))
#endif
#ifndef HAVE_RINTF
#	define rintf(x) (float)rint((double)(x))
#endif
#ifndef HAVE_POWF
#	define powf(x) (float)pow((double)(x))
#endif
#ifndef HAVE_SINF
#	define sinf(x) (float)sin((double)(x))
#endif

/* Handle IEEE NaNs */

#ifndef NAN
#	ifdef _MSC_VER
#		include <ymath.h>
#		define NAN _Nan._Double
#	else /* _MSC_VER */
		static const double _NAN = (HUGE_VAL-HUGE_VAL);
#		define NAN _NAN
#	endif /* _MSC_VER */
#endif /* !NAN */

#ifndef HAVE_ISNAN
#	if defined HAVE__ISNAN /* only WIN32 */
#		define isnan _isnan
#	elif defined HAVE_ISNAND && defined HAVE_ISNANF
#		define isnan \
			( sizeof (x) == sizeof(double) ? isnand((double)(x)) \
			: sizeof (x) == sizeof(float) ? isnanf((float)(x)) \
			: (x != x) )
#	else /* defined HAVE__ISNAN */
#		define isnan (x != x)
#	endif
#endif /* !HAVE_ISNAN */

/* End IEEE NaNs */

/* floating-point classes */

#ifndef isinf
#	ifdef HAVE__FPCLASS
		/* only WIN32 */
		static __inline int isinf (double x) {
			int fpc = _fpclass (x);
			return fpc == _FPCLASS_PINF || fpc == _FPCLASS_NINF;
		}
#	else
#		define isinf(x) \
			( sizeof (x) == sizeof (float)  ? __inline_isinf_f (x) \
			: sizeof (x) == sizeof (double) ? __inline_isinf_d (x) \
			:                                 __inline_isinf (x))
		static inline int __inline_isinf_f (float x) {
			return !isnan (x) && isnan (x - x);
		}
		static inline int __inline_isinf_d (double x) {
			return !isnan (x) && isnan (x - x);
		}
		static inline int __inline_isinf   (long double x) {
			return !isnan (x) && isnan (x - x);
		}
#	endif /* HAVE__FPCLASS */
#endif /* !isinf */

#ifndef isfinite
#	ifdef HAVE__FINITE /* only WIN32 */
#		define isfinite _finite
#	else
#		define isfinite(x) (!isinf(x) && !isnan(x))
#	endif
#endif

#ifndef isnormal
#	ifdef HAVE__FPCLASS
		/* only WIN32 */
		static __inline int isnormal (double x) {
			int fpc = _fpclass (x);
			return fpc == _FPCLASS_PN || fpc == _FPCLASS_NN;
		}
#	else
#		define isnormal(x) \
			( sizeof (x) == sizeof (float)  ? __inline_isnormal_f (x) \
			: sizeof (x) == sizeof (double) ? __inline_isnormal_d (x) \
			:                                 __inline_isnormal (x))
		static inline int __inline_isnormal_f ( float x ) {
			float abs_x = fabsf(x);
			if ( x != x )
				return 0;
			return abs_x < HUGE_VALF && abs_x >= FLT_MIN;
		}
		static inline int __inline_isnormal_d ( double x ) {
			double abs_x = fabs(x);
			if ( x != x )
				return 0;
			return abs_x < HUGE_VAL && abs_x >= DBL_MIN;
		}
		static inline int __inline_isnormal ( long double x ) {
			long double abs_x = fabsl(x);
			if ( x != x )
				return 0;
			return abs_x < HUGE_VALL && abs_x >= LDBL_MIN;
		}
#	endif /* HAVE__FPCLASS */
#endif /* !isnormal */

/* End floating-point classes */

#ifndef HAVE_J0
	EXTERN_MSC double j0(double x);
#endif

#ifndef HAVE_J1
	EXTERN_MSC double j1(double x);
#endif

#ifndef HAVE_JN
	EXTERN_MSC double jn(int n, double x);
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

#ifndef HAVE_SINCOS
	EXTERN_MSC void sincos (double x, double *s, double *c);
#endif

#ifndef HAVE_Y0
	EXTERN_MSC double y0(double x);
#endif

#ifndef HAVE_Y1
	EXTERN_MSC double y1(double x);
#endif

#ifndef HAVE_YN
	EXTERN_MSC double yn(int n, double x);
#endif

/*
 * System specific
 */

/* GMT normally gets these macros from unistd.h */
#ifndef HAVE_UNISTD_H_
#	define R_OK 4
#	define W_OK 2
#	ifdef WIN32
#		define X_OK R_OK /* X_OK == 1 crashes on Windows */
#	else
#		define X_OK 1
#	endif
#	define F_OK 0
#endif /* !HAVE_UNISTD_H_ */

/* access is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */
#if defined HAVE__ACCESS && !defined HAVE_ACCESS
#	define access _access
#endif

/* fileno is usually in stdio.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in stdio.h */
#if defined HAVE__FILENO && !defined HAVE_FILENO
#	define fileno _fileno
#endif

/* getcwd is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in direct.h */
#ifdef HAVE__GETCWD
#	define getcwd _getcwd
#endif

/* getpid is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in process.h */
#if defined HAVE__GETPID && !defined HAVE_GETPID
#	define getpid _getpid
#endif

/* MSVC implementation of popen and pclose */
#if defined HAVE__PCLOSE && !defined HAVE_PCLOSE
#	define pclose _pclose
#endif
#if defined HAVE__POPEN && !defined HAVE_POPEN
#	define popen _popen
#endif

#if defined HAVE__SETMODE && !defined HAVE_SETMODE
#	define setmode _setmode
#endif

#if defined HAVE__SNPRINTF_ && !defined HAVE_SNPRINTF_
#	define snprintf _snprintf
#elif !defined HAVE_SNPRINTF_
#	define snprintf(s, n, format , ...) sprintf(s, format , ##__VA_ARGS__)
#endif

#if defined HAVE__VSNPRINTF_ && !defined HAVE_VSNPRINTF_
#	define vsnprintf _vsnprintf
#elif !defined HAVE_VSNPRINTF_
#	define vsnprintf(s, n, format, arg) vsprintf(s, format, arg)
#endif

#ifdef HAVE__STATI64
#	define stat _stati64
#elif defined HAVE__STAT
#	define stat _stat
#endif

#if defined(HAVE_STRICMP) && !defined(HAVE_STRCASECMP)
#	define strcasecmp stricmp
#endif
#if defined(HAVE_STRNICMP) && !defined(HAVE_STRNCASECMP)
#	define strncasecmp strnicmp
#endif

#ifndef DECLARED_STRDUP
	EXTERN_MSC char *strdup(const char *s);
#endif

#ifndef HAVE_STRTOD
	EXTERN_MSC double strtod(const char *nptr, char **endptr);
#endif

#ifndef HAVE_STRTOF_
	static inline float strtof(const char *nptr, char **endptr) {
		return (float)strtod(nptr, endptr);
	}
#endif

#if defined HAVE_STRTOK_S && !defined HAVE_STRTOK_R
#	define strtok_r strtok_s
#elif !defined HAVE_STRTOK_R
/* define custom function */
#endif

/* If GLIBC compatible qsort_r is not available */
#ifndef HAVE_QSORT_R_GLIBC
#	include "compat/qsort.h"
#endif

#endif /* _GMT_NOTPOSIX_H */
