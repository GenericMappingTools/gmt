/*	$Id$
 *
 * Include file defining structures used in gshhg.c
 *
 *	Copyright (c) 1996-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	14-SEP-2004.  PW: Version 1.3.  Header is now n * 8 bytes (n = 5)
 *			  For use with version 1.3 of GSHHG
 *	2-MAY-2006.  PW: Version 1.4.  Header is now 32 bytes (all int 4)
 *			  For use with version 1.4 of GSHHG
 *	31-MAR-2007.  PW: Version 1.5.  no format change
 *			  For use with version 1.5 of GSHHG
 *	28-AUG-2007.  PW: Version 1.6.  no format change
 *			  For use with version 1.6 of GSHHG which now has WDBII
 *			  borders and rivers.
 *	03-JUL-2008.  PW: Version 1.11. New -I<id> option to pull out a single pol
 *	27-MAY-2009.  PW: Version 1.12. Now includes container polygon ID in header,
 *			  an ancestor ID, and area of the reduced polygon. Works on
 *			  GSHHG 2.0 data.
 *			  Header is now 44 bytes (all 4-byte integers)
 *	24-MAY-2010.  PW: Data version is now 2.1.0. [no change to format]
 *	15-JUL-2011.   PW: Data version is now 2.2.0. [Change in header format to store
 *			  area magnitude and let greenwich be 2-bit flag (0-3)].  Also
 *			  flag WDBII riverlakes with the river flag as used for GSHHG.
 *	15-APR-2012.  PW: Data version is now 2.2.1. [no change to format]
 *	1-JAN-2013.   PW: Data version is now 2.2.2. [no change to format]
 *	1-JUL-2013.   PW: Data version is now 2.2.3. [no change to format]
 *	1-NOV-2013.   PW: Data version is now 2.2.4. [no change to format]
 */

#ifndef _GSHHG
#define _GSHHG
#include "gmt_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gmt_notposix.h"

#include "common_byteswap.h"

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#ifndef SEEK_CUR	/* For really ancient systems */
#define SEEK_CUR 1
#endif

#define GSHHG_DATA_RELEASE	12		/* For v2.2.3 data set */
#define GSHHG_PROG_VERSION	"1.16"

#define GSHHG_MAXPOL	200000	/* Should never need to allocate more than this many polygons */
#define GSHHG_SCL	1.0e-6	/* Convert micro-degrees to degrees */

struct GSHHG {	/* Global Self-consistent Hierarchical High-resolution Shorelines */
	uint32_t id;		/* Unique polygon id number, starting at 0 */
	uint32_t n;		/* Number of points in this polygon */
	uint32_t flag;	/* = level + version << 8 + greenwich << 16 + source << 24 + river << 25 + p << 26 */
	/* flag contains 6 items, as follows:
	 * low byte:	level = flag & 255: Values: 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake
	 * 2nd byte:	version = (flag >> 8) & 255: Values: Should be 9 for GSHHG release 9.
 	 * 3rd byte:	greenwich = (flag >> 16) & 3: Values: 0 if Greenwich nor Dateline are crossed,
	 *		1 if Greenwich is crossed, 2 if Dateline is crossed, 3 if both is crossed.
	 * 4th byte:	source = (flag >> 24) & 1: Values: 0 = CIA WDBII, 1 = WVS
	 * 4th byte:	river = (flag >> 25) & 1: Values: 0 = not set, 1 = river-lake and GSHHG level = 2 (or WDBII class 0)
	 * 4th byte:	area magnitude scale p (as in 10^p) = flag >> 26.  We divide area by 10^p.
	 */
	int32_t west, east, south, north;	/* Signed min/max extent in micro-degrees */
	uint32_t area;		/* Area of polygon in km^2 * 10^p for this resolution file */
	uint32_t area_full;	/* Area of corresponding full-resolution polygon in km^2 * 10^p */
	int32_t container;	/* Id of container polygon that encloses this polygon (-1 if none) */
	int32_t ancestor;	/* Id of ancestor polygon in the full resolution set that was the source of this polygon (-1 if none) */
};

/* byteswap all members of GSHHG struct */
#define GSHHG_STRUCT_N_MEMBERS 11
static inline void bswap_GSHHG_struct (struct GSHHG *h) {
	uint32_t unsigned32[GSHHG_STRUCT_N_MEMBERS];
	uint32_t n;

	/* since all members are 32 bit words: */
	memcpy (&unsigned32, h, sizeof(struct GSHHG));

	for (n = 0; n < GSHHG_STRUCT_N_MEMBERS; ++n)
		unsigned32[n] = bswap32 (unsigned32[n]);

	memcpy (h, &unsigned32, sizeof(struct GSHHG));
}

struct	POINT {	/* Each lon, lat pair is stored in micro-degrees in 4-byte integer format */
	int32_t x;
	int32_t y;
};

/* byteswap members of POINT struct */
static inline void bswap_POINT_struct (struct POINT *p) {
	uint32_t unsigned32;
	memcpy (&unsigned32, &p->x, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->x, &unsigned32, sizeof(uint32_t));
	memcpy (&unsigned32, &p->y, sizeof(uint32_t));
	unsigned32 = bswap32 (unsigned32);
	memcpy (&p->y, &unsigned32, sizeof(uint32_t));
}

#endif	/* _GSHHG */
