/*	$Id: gshhs.h,v 1.3 2001-12-21 03:50:38 ben Exp $
 *
 * Include file defining structures used in gshhs.c
 *
 * Paul Wessel, SOEST
 *
 *	Copyright (c) 1996-1999 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: www.soest.hawaii.edu/wessel

 */

#define _POSIX_SOURCE 1		/* GSHHS code is POSIX compliant */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#ifndef SEEK_CUR	/* For really ancient systems */
#define SEEK_CUR 1
#endif

/* For byte swapping if needed */

#define swabi2(i2) (((i2) >> 8) + (((i2) & 255) << 8))
#define swabi4(i4) (((i4) >> 24) + (((i4) >> 8) & 65280) + (((i4) & 65280) << 8) + (((i4) & 255) << 24))

struct GSHHS {	/* Global Self-consistent Hierarchical High-resolution Shorelines */
	int id;				/* Unique polygon id number, starting at 0 */
	int n;				/* Number of points in this polygon */
	int level;			/* 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake */
	int west, east, south, north;	/* min/max extent in micro-degrees */
	int area;			/* Area of polygon in 1/10 km^2 */
	short int greenwich;		/* Greenwich is 1 if Greenwich is crossed */
	short int source;		/* 0 = CIA WDBII, 1 = WVS */
};

struct	POINT {
	int	x;
	int	y;
};
