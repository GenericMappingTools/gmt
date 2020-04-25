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
 * gshhg_version.h contains prototypes
 *
 * Author:  Florian Wobbe
 * Date:    3-APR-2012
 * Version: 5
 */

/*!
 * \file gshhg_version.h
 * \brief
 */

#ifndef GMT_GSHHG_VERSION_H
#define GMT_GSHHG_VERSION_H

#ifdef __cplusplus      /* Basic C++ support */
extern "C" {
#endif

#ifndef STANDALONE

	/* CMake definitions: This must be first! */
#	include "gmt_config.h"

	/* Declaration modifiers for DLL support (MSC et al) */
#	include "declspec.h"

#else
#	define EXTERN_MSC extern
#endif /* STANDALONE */

/* Structure that holds the GSHHG version */
struct GSHHG_VERSION {
	unsigned major, minor, patch;
};

/* Prototypes */
EXTERN_MSC int gshhg_require_min_version (const char* filename, const struct GSHHG_VERSION min_version);

#ifdef __cplusplus
}
#endif

#endif  /* GMT_GSHHG_VERSION_H */
