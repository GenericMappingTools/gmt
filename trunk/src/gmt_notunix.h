/*--------------------------------------------------------------------
 *	$Id: gmt_notunix.h,v 1.48 2011-05-20 14:01:23 remko Exp $
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
 * gmt_notunix.h contains definitions for constants, structures, and
 * other parameters parameters used by GMT that are not supported by
 * non-UNIX operating systems such as Microsoft Windows.  Note that
 * all of these entities are part of POSIX.  The contents of this file
 * is only activated if one of the system preprocessor flags are defined.
 * Currently considered non-UNIX systems include
 *
 *	flag    |	OS
 *	--------------------------------------------
 *	WIN32   | Microsoft Windows
 *	_WIN32  | UNIX emulation under Windows, like
 *		| Cygwin or DJGPP.  WIN32 implies and
 *		| sets _WIN32 but the converse is not
 *		| true.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#ifndef _GMT_NOTUNIX_H
#define _GMT_NOTUNIX_H

/* A few general comments:
 * FLOCK is a pain. If cannot be used under Windows.
 * Also, users have problems with file locking because their 
 * NFS does not support it. Only those who are really sure should
 * activate -DFLOCK. For these reasons, FLOCK is off by default.
 */

/*--------------------------------------------------------------------
 *
 *	   W I N D O W S
 *
 *	 This section applies to all versions of Microsoft Windows
 *
 *--------------------------------------------------------------------*/

#if defined(WIN32) && !defined(__MINGW32__)	/* Start of Windows setup */

#define _GMT_NOTPOSIX_H	/* This forces the following not to be reset in gmt_notposix.h */

/* This section will override those in gmt_notposix.h which cannot
 * automatically be generated under Windows.
 */

/* Turn off some annoying "security" warnings in Visual C/C++ Studio */

#pragma warning( disable : 4996 )
#pragma warning( disable : 4005 )
#pragma warning( disable : 4018)	/* '<' : signed/unsigned mismatch */

/* These functions are available under Windows with MSVC compilers */

#define HAVE_COPYSIGN 1
#undef HAVE_LOG2
#undef HAVE_LOG1P
#define HAVE_HYPOT 1
#undef HAVE_ACOSH
#undef HAVE_ASINH
#undef HAVE_ATANH
#undef HAVE_RINT
#undef HAVE_IRINT
#undef HAVE_ISNANF
#undef HAVE_ISNAND
#define HAVE_ISNAN 1
#define HAVE_J0 1
#define HAVE_J1 1
#define HAVE_JN 1
#define HAVE_Y0 1
#define HAVE_Y1 1
#define HAVE_YN 1
#undef HAVE_ERF
#undef HAVE_ERFC
#undef HAVE_SINCOS
#undef HAVE_ALPHASINCOS
#undef HAVE_GETPWUID
#undef HAVE_QSORT_R
#define HAVE_STRDUP 1
#define HAVE_STRTOD 1
#undef HAVE_STRTOK_R
#undef WORDS_BIGENDIAN

/* Several math functions exist but the names have a leading underscore */

#define copysign(x,y) _copysign(x,y)
#define hypot(x,y) _hypot(x,y)
#define isnan(x) _isnan(x)
#define j0(x) _j0(x)
#define j1(x) _j1(x)
#define jn(n,x) _jn(n,x)
#define y0(x) _y0(x)
#define y1(x) _y1(x)
#define yn(n,x) _yn(n,x)
#define strdup(s) _strdup(s)
#define GMT_STAT _stat
#if defined( _MSC_VER) && (_MSC_VER >= 1400)	/* MSDN says strtok_s is equivalent to strtok_r in unix */
#define strtok_r strtok_s
#endif

typedef int mode_t;		/* mode_t not defined under Windows; assumed a signed 4-byte integer */

/* WIN32 versus _WIN32:
 *
 * In GMT, the WIN32 flag is predefined by the MicroSoft C compiler.
 * If set, we assume we are in a non-posix environment and must make
 * up the missing functions with homespun code.
 * WIN32 will set _WIN32 but the converse is not true.
 *
 * _WIN32 is set whenever we are compiling GMT on a PC not running
 * a Unix flavor.  This is true when GMT is to be installed under
 * Cygwin32.  _WIN32, when set, causes the directory delimiter to
 * be set to \ instead of /, and also attempts to deal with the fact
 * that DOS file systems have both TEXT and BINARY file modes.
 */

#ifndef _WIN32
#define _WIN32
#endif

#define DIR_DELIM '\\'		/* Backslash as directory delimiter */
#define PATH_DELIM ';'		/* (char)   Win uses ;, Unix uses : */
#define PATH_SEPARATOR ";"	/* (char *) Win uses ;, Unix uses : */

#include <io.h>
#include <direct.h>

/* GMT normally gets these macros from unistd.h */

#define R_OK 04
#define W_OK 02
#define X_OK 01
#define F_OK 00

/* getcwd is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _;
 * it is defined in direct.h. */

#ifndef getcwd
#define getcwd(path, len) _getcwd(path, len)
#endif

/* access is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */

#define access(path, mode) _access(path, mode)

/* mkdir is usually in sys/stat.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and furthermore does not pass the mode argument;
 * it is defined in direct.h */

#define mkdir(path,mode) _mkdir(path)

/* fileno is usually in stdio.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in stdio.h */

#define fileno(stream) _fileno(stream)

/* setmode is usually in unistd.h; we use a macro here
 * since the same function under WIN32 is prefixed with _
 * and defined in io.h */

#define setmode(fd,mode) _setmode(fd,mode)

#pragma warning( disable : 4115 )	/* Shut up this warning: 'GMT_CTRL' :named type definition in parentheses */
EXTERN_MSC void GMT_setmode (struct GMT_CTRL *C, int direction);

#endif		/* End of Windows setup */

/*--------------------------------------------------------------------
 *
 *	 		  NON-UNIX
 *
 *	 This section applies to WIN32, Cygwin, and possibly DJGPP
 *
 *--------------------------------------------------------------------*/
 
#if defined(_WIN32) && !defined(__MINGW32__)	/* Start of NON_UNIX */

#undef FLOCK		/* Do not support file locking */
#define SET_IO_MODE	/* Need to force binary i/o upon request */

#include <io.h>

EXTERN_MSC void GMT_setmode (struct GMT_CTRL *C, int direction);

#endif		/* End of NON-UNIX */

/*===================================================================
 *		      U N I X   C L E A N - U P
 *===================================================================*/
 
/* Set a few Default Unix settings if they did not get set above */

#ifndef DIR_DELIM
#define DIR_DELIM '/'
#endif

#ifndef PATH_DELIM
#define PATH_DELIM ':'	/* Win uses ;, Unix uses : */
#endif

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR ":"	/* Win uses ;, Unix uses : */
#endif

#ifndef NO_FCNTL
#include <fcntl.h>
#endif

#ifndef GMT_STAT
#define GMT_STAT stat
#endif

#endif /* _GMT_NOTUNIX_H */
