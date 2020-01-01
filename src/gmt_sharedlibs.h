/*
 *	Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/*!
 * \file gmt_sharedlibs.h
 * \brief Structures needed by the various shared libraries. 
 */

#pragma once
#ifndef GMT_SHAREDLIBS_H
#define GMT_SHAREDLIBS_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
/* Various functions declared in gmt_sharedlibs.c */
EXTERN_MSC void *dlopen (const char *module_name, int mode);
EXTERN_MSC int dlclose (void *handle);
EXTERN_MSC void *dlsym (void *handle, const char *name);
EXTERN_MSC char *dlerror (void);
#else
#include <dlfcn.h>
#endif

EXTERN_MSC void *dlopen_special (const char *name);

/*! Info for each GMT shared library. This array is filled out when parsing GMT_CUSTOM_LIBS at end of GMT_Create_Session */

struct Gmt_libinfo {
	char *name;	/* Library tag name [without leading "lib" and extension], e.g. "gmt", "gmtsuppl" */
	char *path;	/* Full path to library as given in GMT_CUSTOM_LIBS */
	bool skip;	/* true if we tried to open it and it was not available the first time */
	void *handle;	/* Handle to the shared library, returned by dlopen or dlopen_special */
};

#ifdef __cplusplus
}
#endif

#endif /* !GMT_SHAREDLIBS_H */
