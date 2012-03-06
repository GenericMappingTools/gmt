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
 * common_runpath.c contains code shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  GMT_runpath          Generic *NIX implementation
 *  GMT_runpath_osx      MacOSX implementation
 *  GMT_runpath_win32    Windows implementation
 *  GMT_guess_sharedir   Determine GMT_SHAREDIR relative to current runpath
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>

#ifdef __APPLE__
#	include <mach-o/dyld.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#endif

#include "gmt_config.h"
#include "gmt_notposix.h"
#include "common_string.h"

#include "common_runpath.h"

/*
 * Directory where the executable resides (absolute path, global variable)
 */
/* char gmt_runpath[PATH_MAX+1]; */

#if defined (__APPLE__)

/* MacOSX implementation of GMT_runpath */
char* GMT_runpath_osx (char *result) {
	char *c;
	char path[PATH_MAX+1];
	uint32_t size = PATH_MAX;

	/* Get absolute path of executable */
	if (_NSGetExecutablePath (path, &size) != 0)
		return NULL;
	else {
		/* Truncate full path to dirname */
		if ( (c = strrchr (path, '/')) )
			*c = '\0';
		else
			return NULL;
		/* Resolve symlinks */
		realpath (path, result);
	}
	return result;
}

#elif defined(_WIN32) /* defined (__APPLE__) */

/* Windows implementation of GMT_runpath */
char* GMT_runpath_win32 (char *result) {
	TCHAR path[MAX_PATH+1];
	char *c;

	/* Get absolute path of executable */
	if ( GetModuleFileName (NULL, path, MAX_PATH) == MAX_PATH )
		/* Path to long */
		return NULL;

	/* Convert to cstring */
#ifdef _UNICODE
	/* TCHAR is wchar_t* */
	wcstombs (result, path, MAX_PATH);
#else
	/* TCHAR is char*  */
	strncpy (result, path, MAX_PATH);
#endif

#if 0
	if ( sizeof (TCHAR) == sizeof (char) )
		/* TCHAR is char*  */
		strncpy (result, path, MAX_PATH);
	else
		/* TCHAR is wchar_t* */
		wcstombs (result, path, MAX_PATH);
#endif


	/* Replace backslashes */
	GMT_strrepc (result, '\\', '/');

	/* Truncate full path to dirname */
	if ( (c = strrchr (result, '/')) )
		*c = '\0';

	return result;
}

#else /* defined (__APPLE__) */

/* Generic *NIX function */
char* GMT_runpath (char *result, const char *candidate) {
	char *c, *path, *dir;

	/* If candidate is NULL or empty */
	if ( candidate == NULL || *candidate == '\0' )
		return NULL;

	/* Handle absolute paths */
	if (*candidate == '/') {
		/* Truncate absolute path to dirname */
		if ( (c = strrchr (candidate, '/')) )
			*c = '\0';
		else
			return NULL;
		/* Resolve symlinks */
		realpath (candidate, result);
#ifdef DEBUG_RUNPATH
		printf("executable is in '%s' (from /).\n", result);
#endif
		return result;
	}
	/* Test if candidate was path from cwd */
	if ((c = strrchr (candidate, '/')) != NULL) {
		*c = '\0'; /* Truncate path to dirname */
		/* Get the real path */
		realpath (candidate, result);
#ifdef DEBUG_RUNPATH
		printf("executable is in '%s' (from cwd).\n", result);
#endif
		return result;
	}
	/* Search candidate in the PATH */
	path = getenv ("PATH");
	if (path != NULL) {
		path = strdup (path);
		for (dir = strtok (path, ":"); dir != NULL; dir = strtok (NULL, ":")) {
			strcpy (result, dir);
			strcat (result, "/");
			strcat (result, candidate);
			if ( access (result, X_OK) == 0 ) {
				/* Get real dirname */
				realpath (dir, result);
#ifdef DEBUG_RUNPATH
				printf("executable is in '%s' (from PATH).\n", result);
#endif
				return result;
			}
		}
		free(path);
	}
	return NULL;
}

#endif /* defined (__APPLE__) */

char* GMT_guess_sharedir (char* sharedir, const char* runpath) {
	size_t len_runpath, len_bindir_rel, len_base_dir;

	/* Get string lengths */
	len_runpath = strlen (runpath);
	len_bindir_rel = strlen (GMT_BINDIR_RELATIVE);
	/* Length of common base directory */
	len_base_dir = len_runpath - len_bindir_rel;

	/* Check if GMT_BINDIR_RELATIVE matches end of runpath */
	if ( strstr (runpath + len_base_dir, GMT_BINDIR_RELATIVE) == NULL )
		/* The executable is not located in the expected binary directory */
		return NULL;

	/* Replace GMT_BINDIR_RELATIVE suffix with GMT_SHAREDIR_RELATIVE */
	sharedir = strncpy (sharedir, runpath, len_base_dir);
	sharedir[len_base_dir] = '\0';
	strcat (sharedir, GMT_SHAREDIR_RELATIVE);

	/* Test if the directory exists */
	if ( access (sharedir, R_OK | X_OK) == 0 )
		/* Return sharedir */
		return sharedir;

	return NULL;
}
