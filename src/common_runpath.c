/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * common_runpath.c contains code shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  GMT_runtime_bindir           Get the directory that contains the main exe
 *                               at run-time, generic *NIX implementation
 *  gmt_runtime_bindir_osx       MacOSX implementation
 *  GMT_runtime_bindir_win32     Windows implementation
 *  gmt_runtime_libdir           Get the directory that contains the shared libs
 *  gmt_guess_sharedir           Determine GMT_SHAREDIR relative to current runtime location
 */

/* CMake definitions: This must be first! */
#include "gmt_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>

#ifdef __APPLE__
#	include <mach-o/dyld.h>
#endif

#ifdef HAVE_DLADDR
#	include <dlfcn.h>
#endif

#ifdef WIN32
#	include <windows.h>
#endif

#include "gmt_notposix.h"
#include "common_string.h"

#include "common_runpath.h"

/* #define DEBUG_RUNPATH */

#ifndef gmt_M_unused
#	define gmt_M_unused(x) (void)(x)
#endif

#define gmt_M_str_free(ptr) (free((void *)(ptr)),(ptr)=NULL)

/* Private functions */
static char *sharedir_from_runtime_libdir (char *sharedir);
static char *sharedir_from_runtime_bindir (char *sharedir, const char *runtime_bindir);

/* Functions for determining the directory that contains the main exe at run-time */
#if defined (__APPLE__)

/* MacOSX implementation of GMT_runtime_bindir */
char *gmt_runtime_bindir_osx (char *result) {
	char *c;
	char path[PATH_MAX+1];
	uint32_t size = PATH_MAX;

	/* Get absolute path of executable */
	if (_NSGetExecutablePath (path, &size) != 0)
		return NULL;
	else {
		/* Resolve symlinks */
		if (realpath (path, result) == NULL)
			return NULL;
		/* Truncate full path to dirname */
		if ( (c = strrchr (result, '/')) && c != result )
			*c = '\0';
#ifdef DEBUG_RUNPATH
		fprintf (stderr, "executable is in '%s' (from _NSGetExecutablePath).\n", result);
#endif
	}
	return result;
}

#elif defined (WIN32) /* defined (__APPLE__) */

/* Windows implementation of GMT_runtime_bindir */
char *GMT_runtime_bindir_win32 (char *result) {
	TCHAR path[PATH_MAX+1];
	char *c;

	/* Get absolute path of executable */
	if ( GetModuleFileName (NULL, path, PATH_MAX) == PATH_MAX )
		/* Path to long */
		return NULL;

	/* Convert to cstring */
#ifdef _UNICODE
	/* TCHAR is wchar_t* */
	wcstombs (result, path, PATH_MAX);
#else
	/* TCHAR is char * */
	strncpy (result, path, PATH_MAX);
#endif

#if 0
	if ( sizeof (TCHAR) == sizeof (char) )
		/* TCHAR is char * */
		strncpy (result, path, PATH_MAX);
	else
		/* TCHAR is wchar_t* */
		wcstombs (result, path, PATH_MAX);
#endif

	/* Replace backslashes */
	gmt_strrepc (result, '\\', '/');

	/* Truncate full path to dirname */
	if (((c = strrchr (result, '/')) != NULL) && c != result)
		*c = '\0';

#ifdef DEBUG_RUNPATH
	fprintf (stderr, "executable is in '%s' (from GetModuleFileName).\n", result);
#endif

	return result;
}

#else /* defined (__APPLE__) */

/* Generic *NIX function */
char *GMT_runtime_bindir (char *result, const char *candidate) {
	char *c, *path, *dir, *save_ptr = NULL;
	char candidate_abs[PATH_MAX+1];
	ssize_t len;
	*result = '\0';

	/* Try proc first */
	if ( (len = readlink ("/proc/self/exe", result, PATH_MAX)) != -1 ) {
		result[len] = '\0';
		/* Truncate absolute path to dirname */
		if ( (c = strrchr (result, '/')) && c != result )
			*c = '\0';
#ifdef DEBUG_RUNPATH
		fprintf (stderr, "executable is in '%s' (from %s).\n", result, link);
#endif
		return result;
	}

	/* If candidate is NULL or empty */
	if ( candidate == NULL || *candidate == '\0' )
		return NULL;

	/* Handle absolute paths */
	if (*candidate == '/') {
		/* Resolve symlinks */
		if (realpath (candidate, result) == NULL)
			return NULL;
		/* Truncate absolute path to dirname */
		if ( (c = strrchr (result, '/')) && c != result )
			*c = '\0';
#ifdef DEBUG_RUNPATH
		fprintf (stderr, "executable is in '%s' (from /).\n", result);
#endif
		return result;
	}

	/* Test if candidate was path from cwd */
	if (strchr (candidate, '/')) {
		/* Get the real path */
		if (realpath (candidate, result) == NULL)
			return NULL;
		/* Truncate absolute path to dirname */
		if ( (c = strrchr (result, '/')) && c != result )
			*c = '\0';
#ifdef DEBUG_RUNPATH
		fprintf (stderr, "executable is in '%s' (from cwd).\n", result);
#endif
		return result;
	}

	/* Search candidate in the PATH */
	path = getenv ("PATH");
	if (path != NULL) {
		/* Must copy string because changing pointers returned by getenv it is not allowed: */
		path = strdup (path);
		for (dir = strtok_r (path, ":", &save_ptr);
				 dir != NULL;
				 dir = strtok_r (NULL, ":", &save_ptr)) {
			strncpy (candidate_abs, dir, PATH_MAX);
			strcat (candidate_abs, "/");
			strcat (candidate_abs, candidate);
			if ( access (candidate_abs, X_OK) == 0 ) {
				/* Get real dirname */
				if (realpath (candidate_abs, result) == NULL)
					return NULL;
				/* Truncate absolute path to dirname */
				if ( (c = strrchr (result, '/')) && c != result )
					*c = '\0';
#ifdef DEBUG_RUNPATH
				fprintf (stderr, "executable is in '%s' (from PATH).\n", result);
#endif
				gmt_M_str_free (path);
				return result;
			}
		}
		gmt_M_str_free (path);
	}
	return NULL;
}

#endif /* defined (__APPLE__) */

/* Get the directory that contains this shared library at run-time */
char *gmt_runtime_libdir (char *result) {
#ifdef HAVE_DLADDR
	char *p;
	Dl_info info;

	if ( dladdr (gmt_runtime_libdir, &info) && info.dli_fname[0] == '/') {
		/* Resolve symlinks */
		if (realpath (info.dli_fname, result) == NULL)
			return NULL;
		/* Truncate absolute path to dirname */
		if ( (p = strrchr (result, '/')) && p != result )
			*p = '\0';
#ifdef DEBUG_RUNPATH
		fprintf (stderr, "library is in '%s' (from dladdr).\n", result);
#endif
		return result;
	}
#endif /* HAVE_DLADDR */

#ifdef WIN32
	/* Get the directory that contains this DLL at run-time on Windows */
	MEMORY_BASIC_INFORMATION mbi;
	HMODULE mod;
	TCHAR path[PATH_MAX+1];
	char *p;
	static int dummy;

	/* Get the directory that contains this DLL at run-time on Windows */
	VirtualQuery (&dummy, &mbi, sizeof(mbi));
	mod = (HMODULE) mbi.AllocationBase;

	/* Get absolute path of this dll */
	if ( GetModuleFileName (mod, path, PATH_MAX) == PATH_MAX )
		/* Path to long */
		return NULL;

	/* Convert to cstring */
#ifdef _UNICODE
	/* TCHAR is wchar_t* */
	wcstombs (result, path, PATH_MAX);
#else
	/* TCHAR is char * */
	strncpy (result, path, PATH_MAX);
#endif

	/* Replace backslashes */
	gmt_strrepc (result, '\\', '/');

	/* Truncate full path to dirname */
	if (((p = strrchr (result, '/')) != NULL) && p != result)
		*p = '\0';

#ifdef DEBUG_RUNPATH
	fprintf (stderr, "DLL is in '%s' (from GetModuleFileName).\n", result);
#endif

	return result;
#endif /* WIN32 */

	/* unsupported, or not shared library */

	return NULL;
}

static char *sharedir_from_runtime_libdir (char *sharedir) {
	size_t len_runtime_libdir, len_libdir_rel, len_base_dir;
	char runtime_libdir[PATH_MAX+1];

	/* Get runtime library dir */
	if ( gmt_runtime_libdir(runtime_libdir) == NULL )
		return NULL;

	/* Get string lengths */
	len_runtime_libdir = strlen (runtime_libdir);
	len_libdir_rel = strlen (GMT_LIBDIR_RELATIVE);
	/* Length of common base directory */
	len_base_dir = len_runtime_libdir - len_libdir_rel;

	/* Check if GMT_LIBDIR_RELATIVE matches end of runtime_libdir */
	if ( strstr (runtime_libdir + len_base_dir, GMT_LIBDIR_RELATIVE) == NULL )
		/* The executable is not located in the expected binary directory */
		return NULL;

	/* Replace GMT_LIBDIR_RELATIVE suffix with GMT_SHARE_DIR_RELATIVE */
	sharedir = strncpy (sharedir, runtime_libdir, len_base_dir);
	sharedir[len_base_dir] = '\0';
	strcat (sharedir, GMT_SHARE_DIR_RELATIVE);

	return sharedir;
}

static char *sharedir_from_runtime_bindir (char *sharedir, const char *runtime_bindir) {
	size_t len_runtime_bindir, len_bindir_rel, len_base_dir;

#if defined __APPLE__ || defined WIN32
	char bindir [PATH_MAX+1];

	if (runtime_bindir == NULL) {
		/* Try to find runtime_bindir */
#	if defined __APPLE__
		runtime_bindir = gmt_runtime_bindir_osx (bindir);
#	elif defined WIN32
		runtime_bindir = GMT_runtime_bindir_win32 (bindir);
#	endif
	}
#endif /* defined __APPLE__ || defined WIN32 */

	/* Cannot continue without runtime_bindir */
	if (runtime_bindir == NULL)
		return NULL;

	/* Get string lengths */
	len_runtime_bindir = strlen (runtime_bindir);
	len_bindir_rel = strlen (GMT_BINDIR_RELATIVE);
	/* Length of common base directory */
	len_base_dir = len_runtime_bindir - len_bindir_rel;

	/* Check if GMT_BINDIR_RELATIVE matches end of runtime_bindir */
	if ( strstr (runtime_bindir + len_base_dir, GMT_BINDIR_RELATIVE) == NULL )
		/* The executable is not located in the expected binary directory */
		return NULL;

	/* Replace GMT_BINDIR_RELATIVE suffix with GMT_SHARE_DIR_RELATIVE */
	sharedir = strncpy (sharedir, runtime_bindir, len_base_dir);
	sharedir[len_base_dir] = '\0';
	strcat (sharedir, GMT_SHARE_DIR_RELATIVE);

	return sharedir;
}

char *gmt_guess_sharedir (char *sharedir, const char *runtime_bindir) {
	/* 1. guess based on runtime_libdir */
	if (sharedir_from_runtime_libdir (sharedir) == NULL) {
		/* 2. guess based on runtime_bindir */
		if (sharedir_from_runtime_bindir (sharedir, runtime_bindir) == NULL)
			return NULL;
	}

	return sharedir;
}
