/*	$Id$
 *
 * Include file defining macros, functions and structures used in gshhg.c
 *
 *	Copyright (c) 1996-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: www.soest.hawaii.edu/pwessel
 *
 *	1-JUL-2014.  For use with GSHHG version 2.3.1
 */

/*!
 * \file gmt_gshhg.h
 * \brief Include file defining macros, functions and structures used in gshhg.c 
 */

#ifndef _GMT_GSHHG
#define _GMT_GSHHG
#include "gmt_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gmt_notposix.h"

#include "common_byteswap.h"

#include "gshhg.h"		/* The definition of the GSHHG header structure */

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#ifndef SEEK_CUR	/* For really ancient systems */
#define SEEK_CUR 1
#endif

#define GSHHG_MAXPOL	200000	/* Should never need to allocate more than this many polygons */

#define GSHHG_STRUCT_N_MEMBERS 11
/*! byteswap all members of GSHHG struct */
static inline void bswap_GSHHG_struct (struct GSHHG_HEADER *h) {
	uint32_t unsigned32[GSHHG_STRUCT_N_MEMBERS];
	uint32_t n;

	/* since all members are 32 bit words: */
	memcpy (&unsigned32, h, sizeof(struct GSHHG_HEADER));

	for (n = 0; n < GSHHG_STRUCT_N_MEMBERS; ++n)
		unsigned32[n] = bswap32 (unsigned32[n]);

	memcpy (h, &unsigned32, sizeof(struct GSHHG_HEADER));
}

/*! byteswap members of GSHHG_POINT struct */
static inline void bswap_POINT_struct (struct GSHHG_POINT *p) {
	uint32_t unsigned32;
	memcpy (&unsigned32, &p->x, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->x, &unsigned32, sizeof(uint32_t));
	memcpy (&unsigned32, &p->y, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->y, &unsigned32, sizeof(uint32_t));
}

#endif	/* _GMT_GSHHG */
