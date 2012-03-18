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
 * gmt_map.h contains definitions of macros used in GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_MAP_H
#define _GMT_MAP_H

#define METERS_IN_A_FOOT		0.3048		/* 2.54 * 12 / 100 */
#define METERS_IN_A_KM			1000.0
#define METERS_IN_A_MILE		1609.433	/* meters in statute mile */
#define METERS_IN_A_NAUTICAL_MILE	1852.0
#define GMT_MAP_DIST_UNIT		'e'		/* Default distance is the meter */

enum GMT_enum_coord {GMT_GEOGRAPHIC = 0,	/* Means coordinates are lon,lat : compute spherical distances */
	GMT_CARTESIAN,	/* Means coordinates are Cartesian x,y : compute Cartesian distances */
	GMT_GEO2CART,	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */
	GMT_CART2GEO};	/* Means coordinates are lon,lat but must be mapped to (x,y) : compute Cartesian distances */

enum GMT_enum_dist {GMT_MAP_DIST = 0,	/* Distance in the map */
	GMT_CONT_DIST,		/* Distance along a contour or line in dist units */
	GMT_LABEL_DIST};	/* Distance along a contour or line in dist label units */

enum GMT_enum_path {GMT_RESAMPLE_PATH = 0,	/* Default: Resample geographic paths based in a max gap allowed (path_step) */
	GMT_LEAVE_PATH};	/* Options like -A can turn of this resampling, where available */

enum GMT_enum_cdist {GMT_CARTESIAN_DIST	 = 0,	/* Cartesian 2-D x,y data */
	GMT_CARTESIAN_DIST_PROJ,	/* Project lon,lat to Cartesian 2-D x,y data, then get distance */
	GMT_CARTESIAN_DIST_PROJ_INV};	/* Project Cartesian 2-D x,y data to lon,lat, then get distance */
enum GMT_enum_mdist {GMT_FLATEARTH = 1,	/* Compute Flat Earth distances */
	GMT_GREATCIRCLE,	/* Compute great circle distances */
	GMT_GEODESIC};		/* Compute geodesic distances */
enum GMT_enum_sph {GMT_DIST_M = 10,	/* 2-D lon, lat data, convert distance to meter */
	GMT_DIST_DEG = 20,	/* 2-D lon, lat data, convert distance to spherical degree */
	GMT_DIST_COS = 30};	/* 2-D lon, lat data, convert distance to cos of spherical degree */

struct GMT_DIST {	/* Holds info for a particular distance calculation */
	GMT_LONG init;	/* TRUE if we have initialized settings for this type via GMT_init_distaz */
	GMT_LONG arc;	/* TRUE if distances are in deg/min/sec or arc; otherwise they are e|f|k|M|n or Cartesian */
	PFD func;	/* pointer to function returning distance between two points points */
	double scale;	/* Scale to convert function output to desired unit */
};

/* The enxt is used when we are not doing grid coordinates but have loops for (i = 0; i <= n; i++) x = x0 * i * dx;, i.e. loop includes n, and off = 0 */
#define GMT_i_to_coord(i,x0,x1,dx,nx) (((i) == ((nx))) ? (x1) : (x0) + (i) * (dx))

#endif /* _GMT_MAP_H */
