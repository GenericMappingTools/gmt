/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * gshhs_version.h contains prototypes
 *
 * Author:  Florian Wobbe
 * Date:    3-APR-2012
 * Version: 5
 */

#ifndef _GMT_GSHHS_VERSION_H
#define _GMT_GSHHS_VERSION_H

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

/* Structure that holds the GSHHS version */
struct GSHHS_VERSION {
	unsigned major, minor, patch;
};

/* Prototypes */
EXTERN_MSC int gshhs_get_version (const char* filename, struct GSHHS_VERSION *gshhs_version);
EXTERN_MSC int gshhs_require_min_version (const char* filename, const struct GSHHS_VERSION min_version);

#ifdef __cplusplus
}
#endif

#endif  /* _GMT_GSHHS_VERSION_H */
