/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * common_runpath.h contains prototypes of functions shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 */

/*!
 * \file common_runpath.h
 * \brief Prototypes of functions shared between GMT and PSL
 */

#pragma once
#ifndef COMMON_RUNPATH_H
#define COMMON_RUNPATH_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/* Prototypes */
#if defined (__APPLE__)
#	define GMT_runtime_bindir(result, argv) gmt_runtime_bindir_osx(result)
	EXTERN_MSC char *gmt_runtime_bindir_osx (char *result);
#elif defined (_WIN32)
#	define GMT_runtime_bindir(result, argv) GMT_runtime_bindir_win32(result)
	EXTERN_MSC char *GMT_runtime_bindir_win32 (char *result);
#else
	EXTERN_MSC char *GMT_runtime_bindir (char *result, const char *candidate);
#endif

EXTERN_MSC char *gmt_runtime_libdir (char *result);
EXTERN_MSC char *gmt_guess_sharedir (char *sharedir, const char *runpath);

#ifdef __cplusplus
}
#endif

#endif  /* !COMMON_RUNPATH_H */
