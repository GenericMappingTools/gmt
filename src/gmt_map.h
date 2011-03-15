/*--------------------------------------------------------------------
 *	$Id: gmt_map.h,v 1.38 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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

#define GMT_GEOGRAPHIC			0	/* Means coordinates are lon,lat : compute spherical distances */
#define GMT_CARTESIAN			1	/* Means coordinates are Cartesian x,y : compute Cartesian distances */
#define GMT_GEO2CART			2	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */
#define GMT_CART2GEO			3	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */

#define GMT_MAP_DIST			0	/* Distance in the map */
#define GMT_CONT_DIST			1	/* Distance along a contour or line in dist units */
#define GMT_LABEL_DIST			2	/* Distance along a contour or line in dist label units */

#define METERS_IN_A_FOOT		0.3048	/* 2.54 * 12 / 100 */
#define METERS_IN_A_KM			1000.0
#define METERS_IN_A_MILE		1609.433
#define METERS_IN_A_NAUTICAL_MILE	1852.0

#define GMT_CARTESIAN_DIST		0	/* Cartesian 2-D x,y data */
#define GMT_CARTESIAN_DIST_PROJ		1	/* Project lon,lat to Cartesian 2-D x,y data, then get distance */
#define GMT_CARTESIAN_DIST_PROJ_INV	2	/* Project Cartesian 2-D x,y data to lon,lat, then get distance */
#define GMT_FLATEARTH			1	/* Compute Flat Earth distances */
#define GMT_GREATCIRCLE			2	/* Compute great circle distances */
#define GMT_GEODESIC			3	/* Compute geodesic distances */
#define GMT_DIST_M			10	/* 2-D lon, lat data, convert distance to meter */
#define GMT_DIST_DEG			20	/* 2-D lon, lat data, convert distance to spherical degree */
#define GMT_DIST_COS			30	/* 2-D lon, lat data, convert distance to cos of spherical degree */

#define GMT_MAP_DIST_UNIT		'e'	/* Default distance is the meter */

struct GMT_DIST {	/* Holds info for a particular distance calculation */
	GMT_LONG init;	/* TRUE if we have initialized settings for this type via GMT_init_distaz */
	GMT_LONG arc;	/* TRUE if distances are in deg/min/sec or arc; otherwise they are e|f|k|M|n or Cartesian */
	PFD func;	/* pointer to function returning distance between two points points */
	double scale;	/* Scale to convert function output to desired unit */
};

/* The enxt is used when we are not doing grid coordinates but have loops for (i = 0; i <= n; i++) x = x0 * i * dx;, i.e. loop includes n, and off = 0 */
#define GMT_i_to_coord(i,x0,x1,dx,nx) (((i) == ((nx))) ? (x1) : (x0) + (i) * (dx))

#endif /* _GMT_MAP_H */
