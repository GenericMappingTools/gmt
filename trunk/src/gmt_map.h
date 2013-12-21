/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * gmt_map.h contains definitions of macros used in GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_MAP_H
#define _GMT_MAP_H

#define METERS_IN_A_FOOT		0.3048			/* 2.54 * 12 / 100 */
#define METERS_IN_A_SURVEY_FOOT		(1200.0/3937.0)		/* ~0.3048006096 m */
#define METERS_IN_A_KM			1000.0
#define METERS_IN_A_MILE		1609.433	/* meters in statute mile */
#define METERS_IN_A_NAUTICAL_MILE	1852.0
#define GMT_MAP_DIST_UNIT		'e'		/* Default distance is the meter */

struct GMT_DIST {	/* Holds info for a particular distance calculation */
	bool init;	/* true if we have initialized settings for this type via GMT_init_distaz */
	bool arc;	/* true if distances are in deg/min/sec or arc; otherwise they are e|f|k|M|n or Cartesian */
	double (*func) (struct GMT_CTRL *, double, double, double, double);	/* pointer to function returning distance between two points points */
	double scale;	/* Scale to convert function output to desired unit */
};

#endif /* _GMT_MAP_H */
