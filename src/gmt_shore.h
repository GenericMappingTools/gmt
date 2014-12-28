/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Include file for gmt_shore.c
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef _GMT_SHORE_H
#define _GMT_SHORE_H

/* Declaration modifier for netcdf DLL support
 * annoying: why can't netcdf.h do this on its own? */
#if defined WIN32 && ! defined NETCDF_STATIC
#define DLL_NETCDF
#endif

#include "netcdf.h"

enum GMT_enum_gshhs {GSHHS_MAX_DELTA = 65535,	/* Largest value to store in a unsigned short, used as largest dx or dy in bin  */
	GSHHS_MAX_LEVEL			= 4,	/* Highest hierarchical level of coastlines */
	GSHHS_N_BLEVELS			= 3,	/* Number of levels for borders */
	GSHHS_N_RLEVELS			= 11,	/* Number of levels for rivers */
	GSHHS_RIVER_INTERMITTENT	= 5,	/* Id for intermittent rivers */
	GSHHS_RIVER_CANALS		= 8,	/* Id for river canals */
	GSHHS_NO_RIVERLAKES		= 1,	/* Flag value */
	GSHHS_NO_LAKES			= 2,	/* Flag value */
	GSHHS_OCEAN_LEVEL		= 0,	/* Level assigned to nodes in the ocean */
	GSHHS_LAND_LEVEL		= 1,	/* Level assigned to nodes on land */
	GSHHS_ANTARCTICA_LIMBO		= 7,	/* Level assigned to nodes between ice and grounding lines */
	GSHHS_ANTARCTICA_ICE_SRC	= 2,	/* Source ID for Antarctica ice line */
	GSHHS_ANTARCTICA_GROUND_SRC	= 3,	/* Source ID for Antarctica grounding line */
	GSHHS_ANTARCTICA_GROUND		= 0,	/* Use Antarctica igrounding line as coastline [Default] */
	GSHHS_ANTARCTICA_ICE		= 1,	/* Use Antarctica ice boundary as coastline */
	GSHHS_ANTARCTICA_SKIP		= 2,	/* Skip Antarctica coastline */
	GSHHS_ANTARCTICA_LIMIT		= -60};	/* Data below 60S is Antarctica */

struct GMT_SHORE_SELECT {	/* Information on levels and min area to use */
	int low;	/* Lowest hierarchical level to use [0] */
	int high;	/* Highest hierarchical level to use [4] */
	int flag;	/* 1 = no riverlakes from level 2; 2 = only riverlakes from level 2 */
	int fraction;	/* If not 0, the microfraction limit on a polygons area vs the full resolution version */
	int antarctica_mode;	/* If 1, we skip all data south of 60S, i.e. the Antarctica continent and islands */
	double area;	/* Area of smallest geographical feature to include [0] */
};

struct GMT_GSHHS_pol {	/* Information pertaining to each GSHHS polygon */
	int *parent;		/* Array with ids of the parent polygon for each GSHHS polygon (-1 for all level 1 polygons) */
	double *area;		/* Array with areas in km^2 of the GSHHS polygons */
	int *area_fraction;	/* Array with micro-fraction fractions of area relative to full res area  */
};

struct GMT_SHORE {	/* Struct used by pscoast and others */

	/* Global variables that remain fixed for all bins */
	
	int nb;		/* Number of bins to use */
	int *bins;		/* Array with the nb bin numbers to use */
	int min_level;	/* Lowest level to include [0] */
	int max_level;	/* Highest level to include [4] */
	int flag;		/* If riverlakes or lakes are to be excluded */
	int has_source;		/* 1 if this GSHHG file contains feature source (0 for older files) */
	int fraction;	/* If not 0, the microfraction limit on a polygons area vs the full resolution version */
	double min_area;	/* Smallest feature to include in km^2 */
	double scale;		/* Multiplier to convert dx, dy back to dlon, dlat in degrees */
	
	/* Variables associated with the current bin */
	
	int ns;			/* Number of segments to use in current bin */
	unsigned char node_level[4];
	struct GMT_SHORE_SEGMENT *seg;	/* Array of these segments */
	struct GSHHS_SIDE *side[4];	/* Has position & id for each side exit/entry */
	int nside[4];		/* Number of entries per side, including corner */
	int n_entries;
	int leftmost_bin;		/* true if current bin is at left edge of map */
	int skip_feature;		/* true if GSHHS version > 2.0 and +r or +l is in use */
	int ant_mode;			/* Antarctica mode [0-2] */
	double bsize;			/* Size of square bins in degrees */
	double lon_sw;			/* Longitude of SW corner */
	double lat_sw;			/* Latitude of SW corner */
	double lon_corner[4];		/* Longitudes of 4 corners (depends on direction) */
	double lat_corner[4];		/* Latitudes of 4 corners (depends on direction) */

	/* Data variables associated with shoreline database */
	
	int bin_size;		/* Size of square bins in minutes */
	int bin_nx;		/* Number of bins in 360 degrees of longitude */
	int bin_ny;		/* Number of bins in 180 degrees of latitude */
	int n_poly;		/* Number of polygons present in the data set */
	int n_bin;		/* Number of bins present in the data set */
	int n_seg;		/* Number of segments present in the data set */
	int n_pt;		/* Number of points present in the data set */
	int n_nodes;		/* Number of grid nodes present in the data set */
	
	int *GSHHS_node;	/* Array with ids of the polygon that enclose each node */
	int *bin_firstseg;	/* Array with ids of first segment per bin */
	short int *bin_info;	/* Array with levels of all 4 nodes per bin */
	short int *bin_nseg;	/* Array with number of segments per bin */
	
	int *GSHHS_parent;		/* Array with ids of the parent polygon for each GSHHS polygon (-1 for all level 1 polygons) */
	double *GSHHS_area;		/* Array with areas in km^2 of the GSHHS polygons */
	int *GSHHS_area_fraction;	/* Array with micro-fraction fractions of area relative to full res area  */

	char units[80];		/* Units of lon/lat */
	char title[80];		/* Title of data set */
	char source[80];	/* Source of data set */
	char version[8];	/* Version of data set */

	/* netCDF ID variables */
	
	int cdfid;		/* netCDF File id for coastbin file */
	
	int bin_size_id;	/* Id for variable bin_size */
	int bin_nx_id;		/* Id for variable bin_nx */
	int bin_ny_id;		/* Id for variable bin_ny */
	int n_poly_id;		/* Id for variable n_bin */
	int n_bin_id;		/* Id for variable n_bin */
	int n_seg_id;		/* Id for variable n_seg */
	int n_pt_id;		/* Id for variable n_pt */
	int n_node_id;		/* Id for variable n_nodes */
	int bin_firstseg_id;	/* Id for variable bin_firstseg */
	int bin_info_id;	/* Id for variable bin_info */
	int bin_nseg_id;	/* Id for variable bin_nseg */
	
	int seg_info_id;	/* Id for variable seg_info */
	int seg_start_id;	/* Id for variable seg_start */
	int seg_GSHHS_ID_id;	/* Id for variable seg_GSHHS_ID */
	
	int GSHHS_parent_id;	/* Id for variable GSHHS_parent */
	int GSHHS_area_id;	/* Id for variable GSHHS_area */
	int GSHHS_areafrac_id;	/* Id for variable GSHHS_area_fraction */
	int GSHHS_node_id;	/* Id for variable GSHHS_node_id */
	
	int pt_dx_id;		/* Id for variable pt_dx */
	int pt_dy_id;		/* Id for variable pt_dy */
};

struct GMT_SHORE_SEGMENT {
	unsigned char level;	/* Level of polygon segment (1 i ocean/land, 2 = land/lake, 3 = lake/island, etc) */
	unsigned char entry;	/* Side (0-3) the segment starts on, or 4 for closed segments */
	unsigned char exit;	/* Side (0-3) the segment ends on, or 4 for closed segments */
	unsigned char fid;	/* Fill id (same as level expect for riverlakes which is 5) */
	unsigned short n;	/* Number of points in segment */
	short int *dx;		/* Array of scaled longitudes relative to SW corner */
	short int *dy;		/* Array of scaled latitudes relative to SW corner */
};

struct GSHHS_SIDE {
	unsigned short pos;	/* Position along side in 0-65535 range */
	short int id;		/* Local segment id */
};

struct GMT_BR {	/* Structure for Borders and Rivers */

	/* Global variables that remain fixed for all bins */
	
	int nb;		/* Number of bins to use */
	int *bins;		/* Array with the nb bin numbers to use */
	double scale;		/* Multiplier to convert dx, dy back to dlon, dlat in degrees */
	
	/* Variables associated with the current bin */
	
	int ns;		/* Number of segments to use in current bin */
	struct GMT_BR_SEGMENT *seg;	/* Array of these segments */
	double lon_sw;		/* Longitude of SW corner */
	double lat_sw;		/* Latitude of SW corner */
	double bsize;		/* Size of square bins in degrees */

	/* Data variables associated with shoreline database */
	
	int bin_size;	/* Size of square bins in minutes */
	int bin_nx;	/* Number of bins in 360 degrees of longitude */
	int bin_ny;	/* Number of bins in 180 degrees of latitude */
	int n_bin;		/* Number of bins present in the data set */
	int n_seg;		/* Number of segments present in the data set */
	int n_pt;		/* Number of points present in the data set */
	
	int *bin_firstseg;	/* Array with ids of first segment per bin */
	short int *bin_nseg;	/* Array with number of segments per bin */
	
	char units[80];		/* Units of lon/lat */
	char title[80];		/* Title of data set */
	char source[80];	/* Source of data set */
	char version[8];	/* Version of data set */

	/* netCDF ID variables */
	
	int cdfid;		/* File id for coastbin file */
	
	int bin_size_id;	/* Id for variable bin_size */
	int bin_nx_id;		/* Id for variable bin_nx */
	int bin_ny_id;		/* Id for variable bin_ny */
	int n_bin_id;		/* Id for variable n_bin */
	int n_seg_id;		/* Id for variable n_seg */
	int n_pt_id;		/* Id for variable n_pt */
	int bin_firstseg_id;	/* Id for variable bin_firstseg */
	int bin_nseg_id;	/* Id for variable bin_nseg */
	
	int seg_n_id;		/* Id for variable seg_n */
	int seg_level_id;	/* Id for variable seg_level */
	int seg_start_id;	/* Id for variable seg_start */
	
	int pt_dx_id;		/* Id for variable pt_dx */
	int pt_dy_id;		/* Id for variable pt_dy */
};

struct GMT_BR_SEGMENT {
	unsigned short n;	/* Number of points in segment */
	unsigned short level;	/* Hierarchical level of segment */
	short int *dx;		/* Array of scaled longitudes relative to SW corner */
	short int *dy;		/* Array of scaled latitudes relative to SW corner */
};

struct GMT_GSHHS_POL {
	int n;
	int interior;	/* true if polygon is inside bin */
	int level;
	int fid;		/* Fill id; same as level but 5 if riverlake */
	double *lon;
	double *lat;
};

#endif /* _GMT_SHORE_H */
