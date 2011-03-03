/*--------------------------------------------------------------------
 *	$Id: gmt_map.c,v 1.266 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *			G M T _ M A P . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_map.c contains code related to generic coordinate transformation.
 * For the actual projection functions, see gmt_proj.c
 *
 *	Map Transformation Setup Routines
 *	These routines initializes the selected map transformation
 *	The names and main function are listed below
 *	NB! Note that the transformation function does not check that they are
 *	passed valid lon,lat numbers. I.e asking for log10 scaling using values
 *	<= 0 results in problems.
 *
 * The ellipsoid used is selectable by editing the .gmtdefaults4 in your
 * home directory.  If no such file, create one by running gmtdefaults.
 *
 * Usage: Initialize system by calling GMT_map_setup (separate module), and
 * then just use GMT_geo_to_xy() and GMT_xy_to_geo() functions.
 *
 * Author:	Paul Wessel
 * Date:	22-MAR-2006
 * Version:	4.2.2
 *
 *
 * PUBLIC GMT Functions include:
 *
 *	GMT_azim_to_angle :	Converts azimuth to angle on the map
 *	GMT_clip_to_map :	Force polygon points to be inside map
 *	GMT_compact_line :	Remove redundant pen movements
 *	GMT_geo_to_xy :		Generic lon/lat to x/y
 *	GMT_geo_to_xy_line :	Same for polygons
 *	GMT_geoz_to_xy :	Generic 3-D lon/lat/z to x/y
 *	GMT_grd_project :	Generalized grid projection with interpolation
 *	GMT_grdproject_init :	Initialize parameters for grid transformations
 *	GMT_great_circle_dist :	Returns great circle distance in degrees
 *	GMT_map_outside :	Generic function determines if we're outside map boundary
 *	GMT_map_path :		Return latpat or GMT_lonpath
 *	GMT_map_setup :		Initialize map projection
 *	GMT_project3D :		Convert lon/lat/z to xx/yy/zz
 *	GMT_2D_to_3D :		Convert xyz to xy for entire array
 *	GMT_xy_to_geo :		Generic inverse x/y to lon/lat projection
 *	GMT_xyz_to_xy :		Generic xyz to xy projection
 *
 * Internal GMT Functions include:
 *
 *	GMT_get_origin :		Find origin of projection based on pole and 2nd point
 *	GMT_get_rotate_pole :		Find rotation pole based on two points on great circle
 *	GMT_ilinearxy :			Inverse linear projection
 *	GMT_init_three_D :		Initializes parameters needed for 3-D plots
 *	GMT_map_crossing :		Generic function finds crossings between line and map boundary
 *	GMT_latpath :			Return path between 2 points of equal latitide
 *	GMT_lonpath :			Return path between 2 points of equal longitude
 *	GMT_radial_crossing :		Determine map crossing in the Lambert azimuthal equal area projection
 *	GMT_left_boundary :		Return left boundary in x-inches
 *	GMT_linearxy :			Linear xy projection
 *	GMT_lon_inside :		Accounts for wrap-around in longitudes and checks for inside
 *	GMT_ellipse_crossing :		Find map crossings in the Mollweide projection
 *	GMT_move_to_rect :		Move an outside point straight in to nearest edge
 *	GMT_polar_outside :		Determines if a point is outside polar projection region
 *	GMT_pole_rotate_forward :	Compute positions from oblique coordinates
 *	GMT_pole_rotate_inverse :	Compute oblique coordinates
 *	GMT_radial_clip :		Clip path outside radial region
 *	GMT_radial_outside :		Determine if point is outside radial region
 *	GMT_radial_overlap :		Determine overlap, always TRUE for his projection
 *	GMT_rect_clip :			Clip to rectangular region
 *	GMT_rect_crossing :		Find crossing between line and rect region
 *	GMT_rect_outside :		Determine if point is outside rect region
 *	GMT_rect_outside2 :		Determine if point is outside rect region (azimuthal proj only)
 *	GMT_rect_overlap :		Determine overlap between rect regions
 *	GMT_right_boundary :		Return x value of right map boundary
 *	GMT_xy_search :			Find xy map boundary
 *	GMT_wesn_clip:			Clip polygon to wesn boundaries
 *	GMT_wesn_crossing :		Find crossing between line and lon/lat rectangle
 *	GMT_wesn_outside :		Determine if a point is outside a lon/lat rectangle
 *	GMT_wesn_overlap :		Determine overlap between lon/lat rectangles
 *	GMT_wesn_search :		Search for extreme coordinates
 *	GMT_wrap_around_check :		Check if line wraps around due to Greenwich
 *	GMT_x_to_xx :			Generic linear x projection
 *	GMT_xx_to_x :			Generic inverse linear x projection
 *	GMT_y_to_yy :			Generic linear y projection
 *	GMT_yy_to_y :			Generic inverse linear y projection
 *	GMT_z_to_zz :			Generic linear z projection
 *	GMT_zz_to_z :			Generic inverse linear z projection
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_proj.h"


/* ANSI-C Prototypes for functions internal to gmt_map.c */

GMT_LONG GMT_quickconic (void);
GMT_LONG GMT_quicktm (double lon0, double limit);
void GMT_set_polar (double plat);
GMT_LONG GMT_ok_xovers (GMT_LONG nx, double x0, double x1, GMT_LONG *sides);
void GMT_linearxy (double x, double y, double *x_i, double *y_i);	/*	Convert x/y as linear, log10, or power	*/
void GMT_ilinearxy (double *x, double *y, double x_i, double y_i);	/*	Convert inverse x/y as linear, log10, or power	*/

GMT_LONG GMT_wesn_outside (double lon, double lat);		/*	Returns TRUE if a lon/lat point is outside map (rectangular wesn boundaries only)	*/
GMT_LONG GMT_polar_outside (double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the polar boundaries	*/
GMT_LONG GMT_rect_outside (double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the x'/y' boundaries	*/
GMT_LONG GMT_rect_outside2 (double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the x'/y' boundaries (azimuthal maps only)	*/
GMT_LONG GMT_eqdist_outside (double lon, double lat);		/*	Returns TRUE if a x'/y' point is on the map perimeter	*/
GMT_LONG GMT_radial_outside (double lon, double lat);		/*	Returns TRUE if a lon/lat point is outside the Lambert Azimuthal Eq. area boundaries	*/
GMT_LONG GMT_wesn_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
GMT_LONG GMT_rect_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
GMT_LONG GMT_radial_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides);		/*	computes the crossing point between two lon/lat points and the circular map boundary between them */
GMT_LONG GMT_ellipse_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
GMT_LONG GMT_eqdist_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
GMT_LONG GMT_rect_clip (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);		/*	Clips to region based on rectangular xy coordinates	*/
GMT_LONG GMT_rect_clip_old (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);
GMT_LONG GMT_wesn_clip (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);		/*	Clips to region based on rectangular wesn coordinates	*/
GMT_LONG GMT_wesn_clip_old (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx);		/*	Clips to region based on rectangular wesn coordinates	*/
GMT_LONG GMT_radial_clip_new (double *lon, double *lat, GMT_LONG np, double **x, double **y, GMT_LONG *total_nx);		/*	Clips to region based on spherical distance */
GMT_LONG GMT_radial_clip_pscoast (double *lon, double *lat, GMT_LONG np, double **x, double **y, GMT_LONG *total_nx);		/*	Clips to region based on spherical distance */
GMT_LONG GMT_wesn_overlap (double lon0, double lat0, double lon1, double lat1);		/* Checks if two wesn regions overlap */
GMT_LONG GMT_rect_overlap (double lon0, double lat0, double lon1, double lat1);		/* Checks if two xy regions overlap */
GMT_LONG GMT_radial_overlap (double lon0, double lat0, double lon1, double lat1);		/* Currently a dummy routine */
GMT_LONG GMT_genper_overlap(double lon0, double lat0, double lon1, double lat1);	/* Currently a dummy routine */
GMT_LONG GMT_map_crossing  (double lon1, double lat1, double lon2, double lat2, double *xlon, double *xlat, double *xx, double *yy, GMT_LONG *sides);

GMT_LONG GMT_map_init_linear (void);
GMT_LONG GMT_map_init_polar (void);
GMT_LONG GMT_map_init_merc (void);
GMT_LONG GMT_map_init_miller (void);
GMT_LONG GMT_map_init_stereo (void);
GMT_LONG GMT_map_init_lambert (void);
GMT_LONG GMT_map_init_oblique (void);
GMT_LONG GMT_map_init_tm (void);
GMT_LONG GMT_map_init_utm (void);
GMT_LONG GMT_map_init_lambeq (void);
GMT_LONG GMT_map_init_ortho (void);
GMT_LONG GMT_map_init_genper(void);
GMT_LONG GMT_map_init_gnomonic (void);
GMT_LONG GMT_map_init_azeqdist (void);
GMT_LONG GMT_map_init_mollweide (void);
GMT_LONG GMT_map_init_hammer (void);
GMT_LONG GMT_map_init_winkel (void);
GMT_LONG GMT_map_init_eckert4 (void);
GMT_LONG GMT_map_init_eckert6 (void);
GMT_LONG GMT_map_init_cyleq (void);
GMT_LONG GMT_map_init_cyleqdist (void);
GMT_LONG GMT_map_init_cylstereo (void);
GMT_LONG GMT_map_init_robinson (void);
GMT_LONG GMT_map_init_sinusoidal (void);
GMT_LONG GMT_map_init_cassini (void);
GMT_LONG GMT_map_init_albers (void);
GMT_LONG GMT_map_init_grinten (void);
GMT_LONG GMT_map_init_econic (void);
GMT_LONG GMT_map_init_polyconic (void);

void GMT_wesn_search (double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north);
GMT_LONG GMT_horizon_search (double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax);
void GMT_xy_search (double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0);
GMT_LONG GMT_init_three_D (void);
void GMT_map_setxy (double xmin, double xmax, double ymin, double ymax);
void GMT_map_setinfo (double xmin, double xmax, double ymin, double ymax, double scl);
void GMT_set_spherical (void);
void GMT_init_project_info (GMT_LONG spherical);
void GMT_get_origin (double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2);
void GMT_get_rotate_pole (double lon1, double lat1, double lon2, double lat2);
void GMT_pole_rotate_forward (double lon, double lat, double *tlon, double *tlat);
void GMT_pole_rotate_inverse (double *lon, double *lat, double tlon, double tlat);
void GMT_x_wesn_corner (double *x);
void GMT_y_wesn_corner (double *y);
void GMT_x_rect_corner (double *x);
void GMT_y_rect_corner (double *y);
GMT_LONG GMT_lon_inside (double lon, double w, double e);
GMT_LONG GMT_is_wesn_corner (double x, double y);
GMT_LONG GMT_is_rect_corner (double x, double y);
GMT_LONG GMT_move_to_rect (double *x_edge, double *y_edge, GMT_LONG j, GMT_LONG nx);
GMT_LONG GMT_move_to_wesn (double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, GMT_LONG j, GMT_LONG nx);
GMT_LONG GMT_wrap_around_check_x (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides);
GMT_LONG GMT_wrap_around_check_tm (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides);
GMT_LONG GMT_will_it_wrap_x (double *x, double *y, GMT_LONG n, GMT_LONG *start);
GMT_LONG GMT_will_it_wrap_tm (double *x, double *y, GMT_LONG n, GMT_LONG *start);
GMT_LONG GMT_this_point_wraps_x (double x0, double x1, double w_last, double w_this);
GMT_LONG GMT_this_point_wraps_tm (double y0, double y1);
GMT_LONG GMT_truncate_x (double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG l_or_r);
GMT_LONG GMT_truncate_tm (double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG b_or_t);
void GMT_get_crossings_x (double *xc, double *yc, double x0, double y0, double x1, double y1);
void GMT_get_crossings_tm (double *xc, double *yc, double x0, double y0, double x1, double y1);
GMT_LONG GMT_map_jump_x (double x0, double y0, double x1, double y1);
GMT_LONG GMT_map_jump_tm (double x0, double y0, double x1, double y1);
double GMT_lon_to_corner (double lon);
double GMT_lat_to_corner (double lat);
double GMT_x_to_corner (double x);
double GMT_y_to_corner (double y);
GMT_LONG GMT_radial_boundary_arc (GMT_LONG this, double end_x[], double end_y[], double **xarc, double **yarc);

double GMT_left_boundary (double y);		/* Returns x-value of left border for given y	*/
double GMT_right_boundary (double y);		/* Returns x-value of right border for given y	*/
double GMT_left_conic (double y);		/* For Conic wesn maps	*/
double GMT_right_conic (double y);		/* For Conic wesn maps	*/
double GMT_left_rect (double y);		/* For rectangular maps	*/
double GMT_right_rect (double y);		/* For rectangular maps	*/
double GMT_left_circle (double y);		/* For circular maps	*/
double GMT_right_circle (double y);		/* For circular maps	*/
double GMT_left_ellipse (double y);		/* For elliptical maps	*/
double GMT_right_ellipse (double y);		/* For elliptical maps	*/

PFL GMT_radial_clip;

#ifdef DEBUG
GMT_LONG clip_dump = 0, clip_id = 0;
void dumppol (GMT_LONG n, double *x, double *y, GMT_LONG *id);
#endif
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1 :	M A P  - T R A N S F O R M A T I O N S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/*
 * GMT_map_setup sets up the transformations for the chosen map projection.
 * The user must pass:
 *   w,e,s,n,parameters[0] - parameters[np-1] (np = number of parameters), and project:
 *   w,e,s,n defines the area in degrees.
 *   project == GMT_LINEAR, GMT_POLAR, GMT_MERCATOR, GMT_STEREO, GMT_LAMBERT, GMT_OBLIQUE_MERC, GMT_UTM,
 *	GMT_TM, GMT_ORTHO, GMT_AZ_EQDIST, GMT_LAMB_AZ_EQ, GMT_WINKEL, GMT_ROBINSON, GMT_CASSINI, GMT_ALBERS, GMT_ECONIC,
 *	GMT_ECKERT4, GMT_ECKERT6, GMT_CYL_EQ, GMT_CYL_EQDIST, GMT_CYL_STEREO, GMT_MILLER, GMT_VANGRINTEN
 *	For GMT_LINEAR, we may have GMT_LINEAR, GMT_LOG10, or GMT_POW
 *
 * parameters[0] through parameters[np-1] mean different things to the various
 * projections, as explained below. (np also varies, of course)
 *
 * LINEAR projection:
 *	parameters[0] is inch (or cm)/x_user_unit.
 *	parameters[1] is inch (or cm)/y_user_unit. If 0, then yscale = xscale.
 *	parameters[2] is pow for x^pow (if GMT_POW is on).
 *	parameters[3] is pow for y^pow (if GMT_POW is on).
 *
 * POLAR (r,theta) projection:
 *	parameters[0] is inch (or cm)/x_user_unit (radius)
 *
 * MERCATOR projection:
 *	parameters[0] is central meridian
 *	parameters[1] is standard parallel
 *	parameters[2] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * STEREOGRAPHIC projection:
 *	parameters[0] is longitude of pole
 *	parameters[1] is latitude of pole
 *	parameters[2] is radius in inches (or cm) from pole to the latitude specified
 *	   by parameters[3] OR 1:xxxxx OR map-width.
 *
 * LAMBERT projection (Conic):
 *	parameters[0] is first standard parallel
 *	parameters[1] is second standard parallel
 *	parameters[2] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * OBLIQUE MERCATOR projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is latitude of origin
 *	Definition a:
 *		parameters[2] is longitude of second point along oblique equator
 *		parameters[3] is latitude of second point along oblique equator
 *		parameters[4] is scale in inch (or cm)/degree along oblique equator OR 1:xxxxx OR map-width
 *	Definition b:
 *		parameters[2] is azimuth along oblique equator at origin
 *		parameters[3] is scale in inch (or cm)/degree along oblique equator OR 1:xxxxx OR map-width
 *	Definition c:
 *		parameters[2] is longitude of pole of projection
 *		parameters[3] is latitude of pole of projection
 *		parameters[4] is scale in inch (cm)/degree along oblique equator OR 1:xxxxx OR map-width
 *
 * TRANSVERSE MERCATOR (TM) projection
 *	parameters[0] is central meridian
 *	parameters[1] is central parallel
 *	parameters[2] is scale in inch (cm)/degree along this meridian OR 1:xxxxx OR map-width
 *
 * UNIVERSAL TRANSVERSE MERCATOR (UTM) projection
 *	parameters[0] is UTM zone (0-60, use negative for S hemisphere)
 *	parameters[1] is scale in inch (cm)/degree along this meridian OR 1:xxxxx OR map-width
 *
 * LAMBERT AZIMUTHAL EQUAL AREA projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is latitude of origin
 *	parameters[2] is radius in inches (or cm) from pole to the latitude specified
 *	   by parameters[3] OR 1:xxxxx OR map-width.
 *
 * ORTHOGRAPHIC AZIMUTHAL projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is latitude of origin
 *	parameters[2] is radius in inches (or cm) from pole to the latitude specified
 *	   by parameters[3] OR 1:xxxxx OR map-width.
 *
 * GENERAL PERSPECTIVE projection:
 *      parameters[0] is longitude of origin
 *      parameters[1] is latitude of origin
 *      parameters[2] is radius in inches (or cm) from pole to the latitude specified
 *         by parameters[3] OR 1:xxxxx OR map-width.
 *      parameters[4] is the altitude of the view point in kilometers
 *         if altitude is < 10 then it is the distance from the center of the Earth
 *              divided by the radius of the Earth
 *      parameters[5] is the azimuth east for North of the viewpoint
 *      parameters[6] is the tilt upward of the plane of projection
 *      parameters[7] is the width of the viewpoint in degrees
 *         if = 0, no viewpoint clipping
 *      parameters[8] is the height of the viewpoint in degrees
 *         if = 0, no viewpoint clipping
 *
 * AZIMUTHAL EQUIDISTANCE projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is latitude of origin
 *	parameters[2] is radius in inches (or cm) from pole to the latitude specified
 *	   by parameters[3] OR 1:xxxxx OR map-width.
 *
 * MOLLWEIDE EQUAL-AREA projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * HAMMER-AITOFF EQUAL-AREA projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * SINUSOIDAL EQUAL-AREA projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * WINKEL TRIPEL MODIFIED AZIMUTHAL projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * ROBINSON PSEUDOCYLINDRICAL projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * VAN DER VANGRINTEN projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * CASSINI projection
 *	parameters[0] is longitude of origin
 *	parameters[1] is latitude of origin
 *	parameters[2] is scale in inch (cm)/degree along this meridian OR 1:xxxxx OR map-width
 *
 * ALBERS projection (Conic):
 *	parameters[0] is first standard parallel
 *	parameters[1] is second standard parallel
 *	parameters[2] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * CONIC EQUIDISTANT projection:
 *	parameters[0] is first standard parallel
 *	parameters[1] is second standard parallel
 *	parameters[2] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * ECKERT6 IV projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * ECKERT6 IV projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * CYLINDRICAL EQUAL-AREA projections (Behrmann, Gall-Peters):
 *	parameters[0] is longitude of origin
 *	parameters[1] is the standard parallel
 *	parameters[2] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * CYLINDRICAL STEREOGRAPHIC projections (Braun, Gall, B.S.A.M.):
 *	parameters[0] is longitude of origin
 *	parameters[1] is the standard parallel
 *	parameters[2] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * MILLER CYLINDRICAL projection:
 *	parameters[0] is longitude of origin
 *	parameters[1] is scale in inch (or cm)/degree along parallels OR 1:xxxxx OR map-width
 *
 * Pointers to the correct map transformation functions will be set up so that
 * there are no if tests to determine which routine to call. These pointers
 * are forward and inverse, and are called from GMT_geo_to_xy and GMT_xy_to_geo.
 *
 */

GMT_LONG GMT_map_setup (double west, double east, double south, double north)
{
	GMT_LONG search, k;

	if (west == east && south == north) return (GMT_MAP_NO_REGION);

	GMT_init_ellipsoid ();	/* Set parameters depending on the ellipsoid since the latter could have been set explicitly */

	if (project_info.degree[0]) {
		/* Limit east-west range to 360 and make sure east > -180 and west < 360 */
		if (east < west) east += 360.0;
		if ((fabs (east - west) - 360.0) > GMT_SMALL) return (GMT_MAP_EXCEEDS_360);
		while (east < -180.0) {
			west += 360.0;
			east += 360.0;
		}
		while (west > 360.0) {
			west -= 360.0;
			east -= 360.0;
		}
	}
	if (project_info.got_elevations) {
		if (south < 0.0 || south >= 90.0) return (GMT_MAP_BAD_ELEVATION_MIN);
		if (north <= 0.0 || north > 90.0) return (GMT_MAP_BAD_ELEVATION_MAX);
	}
	if (project_info.degree[1]) {
		if (south < -90.0 || south > 90.0) return (GMT_MAP_BAD_LAT_MIN);
		if (north < -90.0 || north > 90.0) return (GMT_MAP_BAD_LAT_MAX);
	}

	project_info.w = west;	project_info.e = east;	project_info.s = south;	project_info.n = north;
	project_info.GMT_convert_latitudes = FALSE;
	if (project_info.gave_map_width) project_info.units_pr_degree = FALSE;

	if (!GMT_z_forward) GMT_z_forward = (PFL) GMT_translin;
	if (!GMT_z_inverse) GMT_z_inverse = (PFL) GMT_itranslin;
	GMT_n_lon_nodes = GMT_n_lat_nodes = 0;
	GMT_wrap_around_check = (PFL) GMT_wrap_around_check_x;
	GMT_map_jump = (PFL) GMT_map_jump_x;
	GMT_will_it_wrap = (PFB) GMT_will_it_wrap_x;
	GMT_this_point_wraps = (PFB) GMT_this_point_wraps_x;
	GMT_get_crossings = (PFV) GMT_get_crossings_x;
	GMT_lon_wrap = TRUE;
	GMT_lat_swap_init ();

	/* Use old radial clip function for pscoast and gmtselect; grdlandmask sets -Jx1d so not included */
	GMT_radial_clip = (GMT_program && (strstr (GMT_program, "pscoast") || strstr (GMT_program, "gmtselect"))) ? GMT_radial_clip_pscoast : GMT_radial_clip_new;
	GMT_radial_clip = GMT_radial_clip_new;

	switch (project_info.projection) {

		case GMT_LINEAR:		/* Linear transformations */
			search = GMT_map_init_linear ();
			break;

		case GMT_POLAR:		/* Both lon/lat are actually theta, radius */
			search = GMT_map_init_polar ();
			break;

		case GMT_MERCATOR:		/* Standard Mercator projection */
			search = GMT_map_init_merc ();
			break;

		case GMT_STEREO:		/* Stereographic projection */
			search = GMT_map_init_stereo ();
			break;

		case GMT_LAMBERT:		/* Lambert Conformal Conic */
			search = GMT_map_init_lambert ();
			break;

		case GMT_OBLIQUE_MERC:	/* Oblique Mercator */
			search = GMT_map_init_oblique ();
			break;

		case GMT_TM:		/* Transverse Mercator */
			search = GMT_map_init_tm ();
			break;

		case GMT_UTM:		/* Universal Transverse Mercator */
			search = GMT_map_init_utm ();
			break;

		case GMT_LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */
			search = GMT_map_init_lambeq ();
			break;

		case GMT_ORTHO:		/* Orthographic Projection */
			search = GMT_map_init_ortho ();
			break;

		case GMT_GENPER:		/* General Perspective Projection */
			search = GMT_map_init_genper ();
			break;

		case GMT_AZ_EQDIST:		/* Azimuthal Equal-Distance Projection */
			search = GMT_map_init_azeqdist ();
			break;

		case GMT_GNOMONIC:		/* Azimuthal Gnomonic Projection */
			search = GMT_map_init_gnomonic ();
			break;

		case GMT_MOLLWEIDE:		/* Mollweide Equal-Area */
			search = GMT_map_init_mollweide ();
			break;

		case GMT_HAMMER:		/* Hammer-Aitoff Equal-Area */
			search = GMT_map_init_hammer ();
			break;

		case GMT_VANGRINTEN:		/* Van der Grinten */
			search = GMT_map_init_grinten ();
			break;

		case GMT_WINKEL:		/* Winkel Tripel */
			search = GMT_map_init_winkel ();
			break;

		case GMT_ECKERT4:		/* Eckert IV */
			search = GMT_map_init_eckert4 ();
			break;

		case GMT_ECKERT6:		/* Eckert VI */
			search = GMT_map_init_eckert6 ();
			break;

		case GMT_CYL_EQ:		/* Cylindrical Equal-Area */
			search = GMT_map_init_cyleq ();
			break;

		case GMT_CYL_STEREO:			/* Cylindrical Stereographic */
			search = GMT_map_init_cylstereo ();
			break;

		case GMT_MILLER:		/* Miller Cylindrical */
			search = GMT_map_init_miller ();
			break;

		case GMT_CYL_EQDIST:	/* Cylindrical Equidistant */
			search = GMT_map_init_cyleqdist ();
			break;

		case GMT_ROBINSON:		/* Robinson */
			search = GMT_map_init_robinson ();
			break;

		case GMT_SINUSOIDAL:	/* Sinusoidal Equal-Area */
			search = GMT_map_init_sinusoidal ();
			break;

		case GMT_CASSINI:		/* Cassini cylindrical */
			search = GMT_map_init_cassini ();
			break;

		case GMT_ALBERS:		/* Albers Equal-Area Conic */
			search = GMT_map_init_albers ();
			break;

		case GMT_ECONIC:		/* Equidistant Conic */
			search = GMT_map_init_econic ();
			break;

		case GMT_POLYCONIC:		/* Polyconic */
			search = GMT_map_init_polyconic ();
			break;

		default:	/* No projection selected, return to a horrible death */
			return (GMT_MAP_NO_PROJECTION);
	}

	project_info.i_x_scale = (project_info.x_scale != 0.0) ? 1.0 / project_info.x_scale : 1.0;
	project_info.i_y_scale = (project_info.y_scale != 0.0) ? 1.0 / project_info.y_scale : 1.0;
	project_info.i_z_scale = (project_info.z_scale != 0.0) ? 1.0 / project_info.z_scale : 1.0;

	GMT_map_width = fabs (project_info.xmax - project_info.xmin);
	GMT_map_height = fabs (project_info.ymax - project_info.ymin);
	GMT_half_map_size = 0.5 * GMT_map_width;
	GMT_half_map_height = 0.5 * GMT_map_height;

	if (!GMT_n_lon_nodes) GMT_n_lon_nodes = irint (GMT_map_width / gmtdefs.line_step);
	if (!GMT_n_lat_nodes) GMT_n_lat_nodes = irint (GMT_map_height / gmtdefs.line_step);

	GMT_dlon = (project_info.e - project_info.w) / GMT_n_lon_nodes;
	GMT_dlat = (project_info.n - project_info.s) / GMT_n_lat_nodes;

	if (GMT_map_width > 400.0 && GMT_ps.paper_width[GMT_Y] < 1000.0) {	/* Plot is way larger than current paper size so we assume this is ***project calling */
		search = FALSE;	/* Safe-guard that prevents region search below for mapproject and others (400 inch = ~> 10 meters) */
	}

	if (search) {	/* Loop around rectangular perimeter and determine min/max lon/lat extent */
		GMT_wesn_search (project_info.xmin, project_info.xmax, project_info.ymin, project_info.ymax, &project_info.w, &project_info.e, &project_info.s, &project_info.n);
		GMT_dlon = (project_info.e - project_info.w) / GMT_n_lon_nodes;
		GMT_dlat = (project_info.n - project_info.s) / GMT_n_lat_nodes;
	}

	if (GMT_IS_AZIMUTHAL && !project_info.region) GMT_horizon_search (west, east, south, north, project_info.xmin, project_info.xmax, project_info.ymin, project_info.ymax);

	if (project_info.central_meridian < project_info.w && (project_info.central_meridian + 360.0) <= project_info.e) project_info.central_meridian += 360.0;
	if (project_info.central_meridian > project_info.e && (project_info.central_meridian - 360.0) >= project_info.w) project_info.central_meridian -= 360.0;

	GMT_init_three_D ();

	/* Default for overlay plots is no shifting */

	if (!project_info.x_off_supplied && GMT_ps.overlay) GMT_ps.x_origin = 0.0;
	if (!project_info.y_off_supplied && GMT_ps.overlay) GMT_ps.y_origin = 0.0;

	k = GMT_ps.portrait;	/* k and !k gives 0,1 or 1,0 depending on -P */
	if (project_info.x_off_supplied == 2) GMT_ps.x_origin = 0.5 * (fabs(GMT_ps.paper_width[!k]/72.0) - GMT_map_width);	/* Want to x center plot on current page size */
	if (project_info.y_off_supplied == 2) GMT_ps.y_origin = 0.5 * (fabs(GMT_ps.paper_width[k]/72.0) - GMT_map_height);	/* Want to y center plot on current page size */

	return (GMT_NOERROR);
}

GMT_LONG GMT_init_three_D (void) {
	GMT_LONG i, easy;
	double tilt_angle, x, y, x0, x1, x2, y0, y1, y2, zmin = 0.0, zmax = 0.0;
	GMT_LONG positive;

	project_info.three_D = (z_project.view_azimuth != 180.0 || z_project.view_elevation != 90.0);
	if (project_info.three_D && gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;	/* Only plain frame for 3-D */

	project_info.z_scale = project_info.z_pars[0];
	if (project_info.z_scale < 0.0) project_info.xyz_pos[2] = FALSE;	/* User wants z to increase down */
	if (project_info.z_level == DBL_MAX) project_info.z_level = (project_info.xyz_pos[2]) ? project_info.z_bottom : project_info.z_top;

	switch (project_info.xyz_projection[2]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			zmin = (project_info.xyz_pos[2]) ? project_info.z_bottom : project_info.z_top;
			zmax = (project_info.xyz_pos[2]) ? project_info.z_top : project_info.z_bottom;
			GMT_z_forward = (PFL) GMT_translin;
			GMT_z_inverse = (PFL) GMT_itranslin;
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (project_info.z_bottom <= 0.0 || project_info.z_top <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR for -Jz -JZ option: limits must be positive for log10 projection\n", GMT_program);
				GMT_exit (EXIT_FAILURE);
			}
			zmin = (project_info.xyz_pos[2]) ? d_log10 (project_info.z_bottom) : d_log10 (project_info.z_top);
			zmax = (project_info.xyz_pos[2]) ? d_log10 (project_info.z_top) : d_log10 (project_info.z_bottom);
			GMT_z_forward = (PFL) GMT_translog10;
			GMT_z_inverse = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			project_info.xyz_pow[2] = project_info.z_pars[1];
			project_info.xyz_ipow[2] = 1.0 / project_info.z_pars[1];
			positive = !(((GMT_LONG)project_info.xyz_pos[2] + (GMT_LONG)(project_info.xyz_pow[2] > 0.0)) % 2);
			zmin = (positive) ? pow (project_info.z_bottom, project_info.xyz_pow[2]) : pow (project_info.z_top, project_info.xyz_pow[2]);
			zmax = (positive) ? pow (project_info.z_top, project_info.xyz_pow[2]) : pow (project_info.z_bottom, project_info.xyz_pow[2]);
			GMT_z_forward = (PFL) GMT_transpowz;
			GMT_z_inverse = (PFL) GMT_itranspowz;
	}
	if (project_info.compute_scale[2]) project_info.z_scale /= fabs (zmin - zmax);
	project_info.zmax = (zmax - zmin) * project_info.z_scale;
	project_info.z0 = -zmin * project_info.z_scale;

	if (z_project.view_azimuth >= 360.0) z_project.view_azimuth -= 360.0;
	z_project.quadrant = (GMT_LONG)ceil (z_project.view_azimuth / 90.0);
	z_project.view_azimuth -= 180.0;	/* Turn into direction instead */
	if (z_project.view_azimuth < 0.0) z_project.view_azimuth += 360.0;
	sincosd (z_project.view_azimuth, &z_project.sin_az, &z_project.cos_az);
	sincosd (z_project.view_elevation, &z_project.sin_el, &z_project.cos_el);
	GMT_geoz_to_xy (project_info.w, project_info.s, project_info.z_bottom, &x0, &y0);
	GMT_geoz_to_xy (project_info.e, project_info.s, project_info.z_bottom, &x1, &y1);
	GMT_geoz_to_xy (project_info.w, project_info.n, project_info.z_bottom, &x2, &y2);
	z_project.phi[0] = d_atan2d (y1 - y0, x1 - x0);
	z_project.phi[1] = d_atan2d (y2 - y0, x2 - x0);
	z_project.phi[2] = 90.0;
	tilt_angle = (z_project.phi[0] + 90.0 - z_project.phi[1]) * D2R;
	z_project.k = (fabs (z_project.cos_az) > fabs (z_project.sin_az)) ? 0 : 1;
	z_project.xshrink[0] = hypot (z_project.cos_az, z_project.sin_az * z_project.sin_el);
	z_project.yshrink[0] = hypot (z_project.sin_az, z_project.cos_az * z_project.sin_el);
	z_project.xshrink[1] = z_project.yshrink[0];
	z_project.yshrink[1] = z_project.xshrink[0];
	z_project.yshrink[0] *= fabs (cos (tilt_angle));	/* New */
	z_project.yshrink[1] *= fabs (cos (tilt_angle));
	z_project.xshrink[2] = fabs (z_project.cos_el);
	z_project.yshrink[2] = (fabs (z_project.cos_az) > fabs (z_project.sin_az)) ? fabs (z_project.cos_az) : fabs (z_project.sin_az);
	z_project.tilt[0] = tan (tilt_angle);
	z_project.tilt[1] = -z_project.tilt[0];
	z_project.tilt[2] = (fabs (z_project.cos_az) > fabs (z_project.sin_az)) ? tand (-z_project.phi[0]) : tand (-z_project.phi[1]);

	/* Determine min/max y of plot */

	/* easy = TRUE means we can use 4 corner points to find min x and y, FALSE
	   means we must search along wesn perimeter the hard way */

	switch (project_info.projection) {
		case GMT_LINEAR:
		case GMT_POLAR:
		case GMT_MERCATOR:
		case GMT_OBLIQUE_MERC:
		case GMT_CYL_EQ:
		case GMT_CYL_EQDIST:
		case GMT_CYL_STEREO:
		case GMT_MILLER:
			easy = TRUE;
			break;
		case GMT_LAMBERT:
		case GMT_TM:
		case GMT_UTM:
		case GMT_CASSINI:
		case GMT_STEREO:
		case GMT_ALBERS:
		case GMT_ECONIC:
		case GMT_POLYCONIC:
		case GMT_LAMB_AZ_EQ:
		case GMT_ORTHO:
		case GMT_GENPER:
		case GMT_GNOMONIC:
		case GMT_AZ_EQDIST:
		case GMT_SINUSOIDAL:
		case GMT_MOLLWEIDE:
		case GMT_HAMMER:
		case GMT_VANGRINTEN:
		case GMT_WINKEL:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_ROBINSON:
			easy = (!project_info.region);
			break;
		default:
			easy = FALSE;
			break;
	}

	if (!project_info.three_D) easy = TRUE;

	z_project.xmin = z_project.ymin = DBL_MAX;	z_project.xmax = z_project.ymax = -DBL_MAX;

	if (easy) {
		double xx[4], yy[4];

		xx[0] = xx[3] = project_info.xmin; xx[1] = xx[2] = project_info.xmax;
		yy[0] = yy[1] = project_info.ymin; yy[2] = yy[3] = project_info.ymax;

		for (i = 0; i < 4; i++) {
			GMT_xy_to_geo (&z_project.corner_x[i], &z_project.corner_y[i], xx[i], yy[i]);
			GMT_xy_do_z_to_xy (xx[i], yy[i], project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			GMT_xy_do_z_to_xy (xx[i], yy[i], project_info.z_top, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
		}
	}
	else {
		z_project.corner_x[0] = z_project.corner_x[3] = (project_info.xyz_pos[0]) ? project_info.w : project_info.e;
		z_project.corner_x[1] = z_project.corner_x[2] = (project_info.xyz_pos[0]) ? project_info.e : project_info.w;
		z_project.corner_y[0] = z_project.corner_y[1] = (project_info.xyz_pos[1]) ? project_info.s : project_info.n;
		z_project.corner_y[2] = z_project.corner_y[3] = (project_info.xyz_pos[1]) ? project_info.n : project_info.s;
		for (i = 0; i < GMT_n_lon_nodes; i++) {	/* S and N */
			GMT_geoz_to_xy (project_info.w + i * GMT_dlon, project_info.s, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			if (project_info.z_top != project_info.z_bottom) {
				GMT_geoz_to_xy (project_info.w + i * GMT_dlon, project_info.s, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}
			GMT_geoz_to_xy (project_info.w + i * GMT_dlon, project_info.n, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			if (project_info.z_top != project_info.z_bottom) {
				GMT_geoz_to_xy (project_info.w + i * GMT_dlon, project_info.n, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}
		}
		for (i = 0; i < GMT_n_lat_nodes; i++) {	/* W and E */
			GMT_geoz_to_xy (project_info.w, project_info.s + i * GMT_dlat, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			if (project_info.z_top != project_info.z_bottom) {
				GMT_geoz_to_xy (project_info.w, project_info.s + i * GMT_dlat, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}
			GMT_geoz_to_xy (project_info.e, project_info.s + i * GMT_dlat, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			if (project_info.z_top != project_info.z_bottom) {
				GMT_geoz_to_xy (project_info.e, project_info.s + i * GMT_dlat, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}
		}
	}

	z_project.face[0] = (z_project.quadrant == 1 || z_project.quadrant == 2) ? 0 : 1;
	z_project.face[1] = (z_project.quadrant == 1 || z_project.quadrant == 4) ? 2 : 3;
	z_project.face[2] = (z_project.view_elevation >= 0.0) ? 4 : 5;
	z_project.draw[0] = (z_project.quadrant == 1 || z_project.quadrant == 4) ? TRUE : FALSE;
	z_project.draw[1] = (z_project.quadrant == 3 || z_project.quadrant == 4) ? TRUE : FALSE;
	z_project.draw[2] = (z_project.quadrant == 2 || z_project.quadrant == 3) ? TRUE : FALSE;
	z_project.draw[3] = (z_project.quadrant == 1 || z_project.quadrant == 2) ? TRUE : FALSE;
	z_project.sign[0] = z_project.sign[3] = -1.0;
	z_project.sign[1] = z_project.sign[2] = 1.0;
	z_project.z_axis = (z_project.quadrant%2) ? z_project.quadrant : z_project.quadrant - 2;

	if (z_project.fixed) {
		if (!z_project.world_given) {	/* Pick center point of region */
			z_project.world_x = (GMT_IS_MAPPING) ? project_info.central_meridian : 0.5 * (project_info.w + project_info.e);
			z_project.world_y = 0.5 * (project_info.s + project_info.n);
			z_project.world_z = project_info.z_level;
		}
		GMT_geoz_to_xy (z_project.world_x, z_project.world_y, z_project.world_z, &x, &y);
		if (!z_project.view_given) {	/* Pick center of current page */
			z_project.view_x = 0.5 * gmtdefs.paper_width[0] * GMT_u2u[GMT_PT][GMT_INCH];
			z_project.view_y = 0.5 * gmtdefs.paper_width[1] * GMT_u2u[GMT_PT][GMT_INCH];
		}
		z_project.x_off = z_project.view_x - x + z_project.xmin;
		z_project.y_off = z_project.view_y - y + z_project.ymin;
	}
	
	return (GMT_NOERROR);
}

/*
 *	GENERIC TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION
 */

void GMT_x_to_xx (double x, double *xx)
{
	/* Converts x to xx using the current linear projection */

	(*GMT_x_forward) (x, xx);
	(*xx) = (*xx) * project_info.x_scale + project_info.x0;
}

void GMT_y_to_yy (double y, double *yy)
{
	/* Converts y to yy using the current linear projection */

	(*GMT_y_forward) (y, yy);
	(*yy) = (*yy) * project_info.y_scale + project_info.y0;
}

void GMT_z_to_zz (double z, double *zz)
{
	/* Converts z to zz using the current linear projection */

	(*GMT_z_forward) (z, zz);
	(*zz) = (*zz) * project_info.z_scale + project_info.z0;
}

void GMT_xx_to_x (double *x, double xx)
{
	/* Converts xx back to x using the current linear projection */

	xx = (xx - project_info.x0) * project_info.i_x_scale;

	(*GMT_x_inverse) (x, xx);
}

void GMT_yy_to_y (double *y, double yy)
{
	/* Converts yy back to y using the current linear projection */

	yy = (yy - project_info.y0) * project_info.i_y_scale;

	(*GMT_y_inverse) (y, yy);
}

void GMT_zz_to_z (double *z, double zz)
{
	/* Converts zz back to z using the current linear projection */

	zz = (zz - project_info.z0) * project_info.i_z_scale;

	(*GMT_z_inverse) (z, zz);
}

GMT_LONG GMT_geo_to_xy (double lon, double lat, double *x, double *y)
{
	/* Converts lon/lat to x/y using the current projection */

	if (GMT_is_dnan (lon) || GMT_is_dnan (lat)) {(*x) = (*y) = GMT_d_NaN; return TRUE;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT_forward) (lon, lat, x, y);
	(*x) = (*x) * project_info.x_scale + project_info.x0;
	(*y) = (*y) * project_info.y_scale + project_info.y0;
	return FALSE;
}

void GMT_xy_to_geo (double *lon, double *lat, double x, double y)
{
	/* Converts x/y to lon/lat using the current projection */

	if (GMT_is_dnan (x) || GMT_is_dnan (y)) {(*lon) = (*lat) = GMT_d_NaN; return;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	x = (x - project_info.x0) * project_info.i_x_scale;
	y = (y - project_info.y0) * project_info.i_y_scale;

	(*GMT_inverse) (lon, lat, x, y);
}

void GMT_geoz_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* Map-projects xy first, the projects xyz onto xy plane */
	double x0, y0, z0;
	GMT_geo_to_xy (x, y, &x0, &y0);
	GMT_z_to_zz (z, &z0);
	GMT_xyz_to_xy (x0, y0, z0, x_out, y_out);
}

void GMT_xyz_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* projects xyz onto xy plane */
	*x_out = x * z_project.cos_az - y * z_project.sin_az + z_project.x_off;
	*y_out = (x * z_project.sin_az + y * z_project.cos_az) * z_project.sin_el + z * z_project.cos_el + z_project.y_off;
}

void GMT_xy_do_z_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* projects xy (inches) z (z-value) onto xy plane */
	double z_out;

	GMT_z_to_zz (z, &z_out);
	GMT_xyz_to_xy (x, y, z_out, x_out, y_out);
}

void GMT_project3D (double x, double y, double z, double *x_out, double *y_out, double *z_out)
{
	GMT_geo_to_xy (x, y, x_out, y_out);
	GMT_z_to_zz (z, z_out);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION (GMT_LINEAR)
 */

GMT_LONG GMT_map_init_linear (void) {
	GMT_LONG positive;
	double xmin, xmax, ymin = 0.0, ymax = 0.0;

	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	GMT_forward = (PFL) GMT_linearxy;
	GMT_inverse = (PFL) GMT_ilinearxy;
	if (project_info.degree[0]) {	/* x is longitude */
		project_info.central_meridian = 0.5 * (project_info.w + project_info.e);
		GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	}
	else
		GMT_lon_wrap = FALSE;
	project_info.x_scale = project_info.pars[0];
	project_info.y_scale = project_info.pars[1];
	if (project_info.x_scale < 0.0) project_info.xyz_pos[0] = FALSE;	/* User wants x to increase left */
	if (project_info.y_scale < 0.0) project_info.xyz_pos[1] = FALSE;	/* User wants y to increase down */
	switch ( (project_info.xyz_projection[0]%3)) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			GMT_x_forward = (PFL) ((project_info.degree[0]) ? GMT_translind : GMT_translin);
			GMT_x_inverse = (PFL) ((project_info.degree[0]) ? GMT_itranslind : GMT_itranslin);
			if (project_info.xyz_pos[0]) {
				(*GMT_x_forward) (project_info.w, &xmin);
				(*GMT_x_forward) (project_info.e, &xmax);
			}
			else {
				(*GMT_x_forward) (project_info.e, &xmin);
				(*GMT_x_forward) (project_info.w, &xmax);
			}
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (project_info.w <= 0.0 || project_info.e <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Jx option:  Limits must be positive for log10 option\n", GMT_program);
				GMT_exit (EXIT_FAILURE);
			}
			xmin = (project_info.xyz_pos[0]) ? d_log10 (project_info.w) : d_log10 (project_info.e);
			xmax = (project_info.xyz_pos[0]) ? d_log10 (project_info.e) : d_log10 (project_info.w);
			GMT_x_forward = (PFL) GMT_translog10;
			GMT_x_inverse = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			project_info.xyz_pow[0] = project_info.pars[2];
			project_info.xyz_ipow[0] = 1.0 / project_info.pars[2];
			positive = !(((GMT_LONG)project_info.xyz_pos[0] + (GMT_LONG)(project_info.xyz_pow[0] > 0.0)) % 2);
			xmin = (positive) ? pow (project_info.w, project_info.xyz_pow[0]) : pow (project_info.e, project_info.xyz_pow[0]);
			xmax = (positive) ? pow (project_info.e, project_info.xyz_pow[0]) : pow (project_info.w, project_info.xyz_pow[0]);
			GMT_x_forward = (PFL) GMT_transpowx;
			GMT_x_inverse = (PFL) GMT_itranspowx;
			break;
	}
	switch (project_info.xyz_projection[1]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			ymin = (project_info.xyz_pos[1]) ? project_info.s : project_info.n;
			ymax = (project_info.xyz_pos[1]) ? project_info.n : project_info.s;
			GMT_y_forward = (PFL) GMT_translin;
			GMT_y_inverse = (PFL) GMT_itranslin;
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (project_info.s <= 0.0 || project_info.n <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Jx option:  Limits must be positive for log10 option\n", GMT_program);
				GMT_exit (EXIT_FAILURE);
			}
			ymin = (project_info.xyz_pos[1]) ? d_log10 (project_info.s) : d_log10 (project_info.n);
			ymax = (project_info.xyz_pos[1]) ? d_log10 (project_info.n) : d_log10 (project_info.s);
			GMT_y_forward = (PFL) GMT_translog10;
			GMT_y_inverse = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			project_info.xyz_pow[1] = project_info.pars[3];
			project_info.xyz_ipow[1] = 1.0 / project_info.pars[3];
			positive = !(((GMT_LONG)project_info.xyz_pos[1] + (GMT_LONG)(project_info.xyz_pow[1] > 0.0)) % 2);
			ymin = (positive) ? pow (project_info.s, project_info.xyz_pow[1]) : pow (project_info.n, project_info.xyz_pow[1]);
			ymax = (positive) ? pow (project_info.n, project_info.xyz_pow[1]) : pow (project_info.s, project_info.xyz_pow[1]);
			GMT_y_forward = (PFL) GMT_transpowy;
			GMT_y_inverse = (PFL) GMT_itranspowy;
	}

	/* Was given axes length instead of scale? */

	if (project_info.compute_scale[0]) project_info.x_scale /= fabs (xmin - xmax);
	if (project_info.compute_scale[1]) project_info.y_scale /= fabs (ymin - ymax);

	/* If either is zero, adjust width or height to the other */

	if (project_info.x_scale == 0) {
		project_info.x_scale = project_info.y_scale;
		project_info.pars[0] = project_info.x_scale * fabs (xmin - xmax);
	}
	if (project_info.y_scale == 0) {
		project_info.y_scale = project_info.x_scale;
		project_info.pars[1] = project_info.y_scale * fabs (ymin - ymax);
	}

	/* This is to make sure that when using -J[x|X]...d degrees work as meters */

	project_info.M_PR_DEG = 1.0;
	project_info.KM_PR_DEG = 0.001;

	GMT_map_setxy (xmin, xmax, ymin, ymax);
	GMT_outside = (PFL) GMT_rect_outside;
	GMT_crossing = (PFL) GMT_rect_crossing;
	GMT_overlap = (PFL) GMT_rect_overlap;
	GMT_map_clip = (PFL) GMT_rect_clip;
	frame_info.check_side = TRUE;
	frame_info.horizontal = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);
}

void GMT_linearxy (double x, double y, double *x_i, double *y_i)
{	/* Transform both x and y linearly */
	(*GMT_x_forward) (x, x_i);
	(*GMT_y_forward) (y, y_i);
}

void GMT_ilinearxy (double *x, double *y, double x_i, double y_i)
{	/* Inversely transform both x and y linearly */
	(*GMT_x_inverse) (x, x_i);
	(*GMT_y_inverse) (y, y_i);
}

/*
 *	TRANSFORMATION ROUTINES FOR POLAR (theta,r) PROJECTION (GMT_POLAR)
 */

GMT_LONG GMT_map_init_polar (void)
{
	double xmin, xmax, ymin, ymax;

	GMT_vpolar (project_info.pars[1]);
	if (project_info.got_elevations) {	/* Requires s >=0 and n <= 90 */
		if (project_info.s < 0.0 || project_info.n > 90.0) {
			fprintf (stderr, "%s: ERROR: -JP...r for elevation plots requires s >= 0 and n <= 90!\n", GMT_program);
			GMT_exit (EXIT_FAILURE);
		}
		if (GMT_IS_ZERO (90.0 - project_info.n)) project_info.edge[2] = FALSE;
	}
	else {
		if (GMT_IS_ZERO (project_info.s)) project_info.edge[0] = FALSE;
	}
	if (GMT_360_RANGE (project_info.w, project_info.e)) project_info.edge[1] = project_info.edge[3] = FALSE;
	GMT_left_edge = (PFD) GMT_left_circle;
	GMT_right_edge = (PFD) GMT_right_circle;
	GMT_forward = (PFL) GMT_polar;
	GMT_inverse = (PFL) GMT_ipolar;
	GMT_world_map = FALSE;	/* There is no wrapping around here */
	GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
	project_info.x_scale = project_info.y_scale = project_info.pars[0];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[0]);
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	/* project_info.r = 0.5 * project_info.xmax; */
	project_info.r = project_info.y_scale * project_info.n;
	GMT_outside = (PFL) GMT_polar_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	frame_info.horizontal = TRUE;
	if (!project_info.got_elevations) gmtdefs.degree_format = -1;	/* Special labeling case */
	GMT_n_lat_nodes = 2;
	GMT_meridian_straight = TRUE;

	return (FALSE);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MERCATOR PROJECTION (GMT_MERCATOR)
 */

GMT_LONG GMT_map_init_merc (void) {
	double xmin, xmax, ymin, ymax, D = 1.0;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) {	/* Set fudge factor */
		GMT_scale_eqrad ();
		D = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / project_info.GMT_lat_swap_vals.rm;
	}
	if (project_info.s <= -90.0 || project_info.n >= 90.0) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -R option:  Cannot include south/north poles with Mercator projection!\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}
	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_vmerc (project_info.pars[0], project_info.pars[1]);
	project_info.j_x *= D;
	project_info.j_ix /= D;
	GMT_forward = (PFL) GMT_merc_sph;
	GMT_inverse = (PFL) GMT_imerc_sph;
	(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
	(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[2] /= (D * project_info.M_PR_DEG);
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	GMT_n_lat_nodes = 2;
	GMT_n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_outside = (PFL) GMT_wesn_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUAL-AREA PROJECTIONS (GMT_CYL_EQ)
 */

GMT_LONG GMT_map_init_cyleq (void) {
	double xmin, xmax, ymin, ymax, D, k0, qp, slat, e, e2;

	project_info.Dx = project_info.Dy = 0.0;
	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) {
		GMT_scale_eqrad ();
		slat = project_info.pars[1];
		project_info.pars[1] = GMT_latg_to_lata (project_info.pars[1]);
		e = project_info.ECC;
		e2 = project_info.ECC2;
		qp = 1.0 - 0.5 * (1.0 - e2) * log ((1.0 - e) / (1.0 + e)) / e;
		k0 = cosd (slat) / d_sqrt (1.0 - e2 * sind (project_info.pars[1]) * sind (project_info.pars[1]));
		D = k0 / cosd (project_info.pars[1]);
		project_info.Dx = D;
		project_info.Dy = 0.5 * qp / D;
	}
	project_info.iDx = 1.0 / project_info.Dx;
	project_info.iDy = 1.0 / project_info.Dy;
	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	GMT_vcyleq (project_info.pars[0], project_info.pars[1]);
	GMT_cyleq (project_info.w, project_info.s, &xmin, &ymin);
	GMT_cyleq (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	GMT_n_lat_nodes = 2;
	GMT_n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFL) GMT_cyleq;
	GMT_inverse = (PFL) GMT_icyleq;
	GMT_outside = (PFL) GMT_wesn_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUIDISTANT PROJECTION (GMT_CYL_EQDIST)
 */

GMT_LONG GMT_map_init_cyleqdist (void) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical ();	/* Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	GMT_vcyleqdist (project_info.pars[0], project_info.pars[1]);
	GMT_cyleqdist (project_info.w, project_info.s, &xmin, &ymin);
	GMT_cyleqdist (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	GMT_n_lat_nodes = 2;
	GMT_n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFL) GMT_cyleqdist;
	GMT_inverse = (PFL) GMT_icyleqdist;
	GMT_outside = (PFL) GMT_wesn_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR MILLER CYLINDRICAL PROJECTION (GMT_MILLER)
 */

GMT_LONG GMT_map_init_miller (void) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical ();	/* Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	GMT_vmiller (project_info.pars[0]);
	GMT_miller (project_info.w, project_info.s, &xmin, &ymin);
	GMT_miller (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_n_lat_nodes = 2;
	GMT_n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFL) GMT_miller;
	GMT_inverse = (PFL) GMT_imiller;
	GMT_outside = (PFL) GMT_wesn_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL STEREOGRAPHIC PROJECTIONS (GMT_CYL_STEREO)
 */

GMT_LONG GMT_map_init_cylstereo (void) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical ();	/* Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	GMT_vcylstereo (project_info.pars[0], project_info.pars[1]);
	GMT_cylstereo (project_info.w, project_info.s, &xmin, &ymin);
	GMT_cylstereo (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	GMT_n_lat_nodes = 2;
	GMT_n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFL) GMT_cylstereo;
	GMT_inverse = (PFL) GMT_icylstereo;
	GMT_outside = (PFL) GMT_wesn_outside;
	GMT_crossing = (PFL) GMT_wesn_crossing;
	GMT_overlap = (PFL) GMT_wesn_overlap;
	GMT_map_clip = (PFL) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}


/*
 *	TRANSFORMATION ROUTINES FOR THE POLAR STEREOGRAPHIC PROJECTION (GMT_STEREO)
 */

GMT_LONG GMT_map_init_stereo (void) {
	double xmin, xmax, ymin, ymax, dummy, radius, latg, D = 1.0;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	latg = project_info.pars[1];

	GMT_set_polar (project_info.pars[1]);

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 0.9996;	/* Select default map scale for Stereographic */
	if (project_info.polar && (irint (project_info.pars[5]) == 1)) gmtdefs.map_scale_factor = 1.0;	/* Gave true scale at given parallel set below */
	/* Equatorial view has a problem with infinite loops.  Until I find a cure
	  we set projection center latitude to 0.001 so equatorial works for now */

	if (fabs (project_info.pars[1]) < GMT_SMALL) project_info.pars[1] = 0.001;

	GMT_vstereo (project_info.pars[0], project_info.pars[1], project_info.pars[2]);

	if (project_info.GMT_convert_latitudes) {	/* Set fudge factors when conformal latitudes are used */
		double e1p, e1m, s, c;

		D = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / project_info.GMT_lat_swap_vals.rm;
		if (project_info.polar) {
			e1p = 1.0 + project_info.ECC;	e1m = 1.0 - project_info.ECC;
			D /= d_sqrt (pow (e1p, e1p) * pow (e1m, e1m));
			if (irint (project_info.pars[5]) == 1) {	/* Gave true scale at given parallel */
				double k_p, m_c, t_c, es;

				sincosd (fabs (project_info.pars[4]), &s, &c);
				es = project_info.ECC * s;
				m_c = c / d_sqrt (1.0 - project_info.ECC2 * s * s);
				t_c = d_sqrt (((1.0 - s) / (1.0 + s)) * pow ((1.0 + es) / (1.0 - es), project_info.ECC));
				k_p = 0.5 * m_c * d_sqrt (pow (e1p, e1p) * pow (e1m, e1m)) / t_c;
				D *= k_p;
			}
		}
		else {
			sincosd (latg, &s, &c);	/* Need original geographic pole coordinates */
			D *= (c / (project_info.cosp * d_sqrt (1.0 - project_info.ECC2 * s * s)));
		}
	}
	project_info.Dx = project_info.Dy = D;

	project_info.iDx = 1.0 / project_info.Dx;
	project_info.iDy = 1.0 / project_info.Dy;

	if (project_info.polar) {	/* Polar aspect */
		GMT_forward = (PFL) GMT_plrs_sph;
		GMT_inverse = (PFL) GMT_iplrs_sph;
		if (project_info.units_pr_degree) {
			(*GMT_forward) (project_info.pars[0], project_info.pars[4], &dummy, &radius);
			project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
		}
		else
			project_info.x_scale = project_info.y_scale = project_info.pars[3];
		GMT_meridian_straight = TRUE;
	}
	else {
		GMT_forward = (GMT_IS_ZERO (project_info.pole)) ? (PFL) GMT_stereo2_sph : (PFL) GMT_stereo1_sph;
		GMT_inverse = (PFL) GMT_istereo_sph;
		if (project_info.units_pr_degree) {
			GMT_vstereo (0.0, 90.0, project_info.pars[2]);
			(*GMT_forward) (0.0, fabs(project_info.pars[4]), &dummy, &radius);
			project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
		}
		else
			project_info.x_scale = project_info.y_scale = project_info.pars[3];

		GMT_vstereo (project_info.pars[0], project_info.pars[1], project_info.pars[2]);
	}


	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s <= -90.0) {
					fprintf (stderr, "%s: Error: South boundary cannot be -90.0 for north polar stereographic projection\n", GMT_program);
					GMT_exit (EXIT_FAILURE);
				}
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n >= 90.0) {
					fprintf (stderr, "%s: Error: North boundary cannot be +90.0 for south polar stereographic projection\n", GMT_program);
					GMT_exit (EXIT_FAILURE);
				}
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFL) GMT_polar_outside;
			GMT_crossing = (PFL) GMT_wesn_crossing;
			GMT_overlap = (PFL) GMT_wesn_overlap;
			GMT_map_clip = (PFL) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			GMT_n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmax = ymax = project_info.rho_max;
			xmin = ymin = -xmax;
			GMT_outside = (PFL) GMT_radial_outside;
			GMT_crossing = (PFL) GMT_radial_crossing;
			GMT_overlap = (PFL) GMT_radial_overlap;
			GMT_map_clip = (PFL) GMT_radial_clip;
			if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT CONFORMAL CONIC PROJECTION (GMT_LAMBERT)
 */

GMT_LONG GMT_map_init_lambert (void) {
	double xmin, xmax, ymin, ymax;

	project_info.GMT_convert_latitudes = GMT_quickconic();
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vlamb (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];
	if (GMT_IS_SPHERICAL || project_info.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFL) GMT_lamb_sph;
		GMT_inverse = (PFL) GMT_ilamb_sph;
	}
	else {
		GMT_forward = (PFL) GMT_lamb;
		GMT_inverse = (PFL) GMT_ilamb;
	}

	if (!project_info.region) {	/* Rectangular box given*/
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	else {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);
	GMT_n_lat_nodes = 2;
	frame_info.horizontal = TRUE;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	GMT_meridian_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE OBLIQUE MERCATOR PROJECTION (GMT_OBLIQUE_MERC)
 */

GMT_LONG GMT_map_init_oblique (void) {
	double xmin, xmax, ymin, ymax;
	double o_x, o_y, p_x, p_y, c_x, c_y, c, az, b_x, b_y, w, e, s, n, P[3];

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;	/* To get plot-units / m */

	o_x = project_info.pars[0];	o_y = project_info.pars[1];

	if (irint (project_info.pars[6]) == 1) {	/* Must get correct origin, then get second point */
		p_x = project_info.pars[2];	p_y = project_info.pars[3];

		project_info.o_pole_lon = p_x;
		project_info.o_pole_lat = p_y;
		project_info.o_sin_pole_lat = sind (p_y);
		project_info.o_cos_pole_lat = cosd (p_y);

		/* Find azimuth to pole, add 90, and compute second point 10 degrees away */

		GMT_get_origin (o_x, o_y, p_x, p_y, &o_x, &o_y);
		az = atand (cosd (p_y) * sind (p_x - o_x) / (cosd (o_y) * sind (p_y) - sind (o_y) * cosd (p_y) * cosd (p_x - o_x))) + 90.0;
		c = 10.0;	/* compute point 10 degrees from origin along azimuth */
		b_x = o_x + atand (sind (c) * sind (az) / (cosd (o_y) * cosd (c) - sind (o_y) * sind (c) * cosd (az)));
		b_y = d_asind (sind (o_y) * cosd (c) + cosd (o_y) * sind (c) * cosd (az));
		project_info.pars[0] = o_x;	project_info.pars[1] = o_y;
		project_info.pars[2] = b_x;	project_info.pars[3] = b_y;
	}
	else {	/* Just find pole */
		b_x = project_info.pars[2];	b_y = project_info.pars[3];
		GMT_get_rotate_pole (o_x, o_y, b_x, b_y);
	}

	/* Here we have pole and origin */

	/* Get forward pole and origin vectors FP, FC */
	GMT_geo_to_cart (project_info.o_pole_lat, project_info.o_pole_lon, project_info.o_FP, TRUE);
	GMT_geo_to_cart (o_y, o_x, P, TRUE);	/* P points to origin  */
	GMT_cross3v (project_info.o_FP, P, project_info.o_FC);
	GMT_normalize3v (project_info.o_FC);

	/* Get inverse pole and origin vectors FP, FC */
	GMT_obl (0.0, M_PI_2, &p_x, &p_y);
	GMT_geo_to_cart (p_y, p_x, project_info.o_IP, FALSE);
	GMT_obl (0.0, 0.0, &c_x, &c_y);
	GMT_geo_to_cart (c_y, c_x, P, FALSE);	/* P points to origin  */
	GMT_cross3v (project_info.o_IP, P, project_info.o_IC);
	GMT_normalize3v (project_info.o_IC);

	GMT_vmerc (0.0, 0.0);

	if (project_info.region) {	/* Gave oblique degrees */
		/* Convert oblique wesn in degrees to meters using regular Mercator */
		if (GMT_360_RANGE (project_info.w, project_info.e)) {
			project_info.w = -180.0;
			project_info.e = +180.0;
		}
		GMT_merc_sph (project_info.w, project_info.s, &xmin, &ymin);
		GMT_merc_sph (project_info.e, project_info.n, &xmax, &ymax);
		project_info.region = FALSE;	/* Since wesn was oblique, not geographical wesn */
	}
	else {
		/* wesn is lower left and upper right corners in normal lon/lats */

		GMT_oblmrc (project_info.w, project_info.s, &xmin, &ymin);
		GMT_oblmrc (project_info.e, project_info.n, &xmax, &ymax);
	}

	GMT_imerc_sph (&w, &s, xmin, ymin);	/* Get oblique wesn boundaries */
	GMT_imerc_sph (&e, &n, xmax, ymax);
	project_info.x_scale = project_info.y_scale = project_info.pars[4];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);
	GMT_forward = (PFL) GMT_oblmrc;
	GMT_inverse = (PFL) GMT_ioblmrc;
	GMT_outside = (PFL) GMT_rect_outside;
	GMT_crossing = (PFL) GMT_rect_crossing;
	GMT_overlap = (PFL) GMT_rect_overlap;
	GMT_map_clip = (PFL) GMT_rect_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;

	GMT_world_map = (fabs (fabs (w - e) - 360.0) < GMT_SMALL);
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
	return (TRUE);
}

void GMT_pole_rotate_forward (double lon, double lat, double *tlon, double *tlat)
{
	/* Given the pole position in project_info, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_lat, cos_lat, cos_lon, sin_lon, cc;

	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon - project_info.o_pole_lon, &sin_lon, &cos_lon);
	cc = cos_lat * cos_lon;
	*tlat = d_asind (project_info.o_sin_pole_lat * sin_lat + project_info.o_cos_pole_lat * cc);
	*tlon = project_info.o_beta + d_atan2d (cos_lat * sin_lon, project_info.o_sin_pole_lat * cc - project_info.o_cos_pole_lat * sin_lat);
}

void GMT_pole_rotate_inverse (double *lon, double *lat, double tlon, double tlat)
{
	/* Given the pole position in project_info, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_tlat, cos_tlat, cos_tlon, sin_tlon, cc;

	sincosd (tlat, &sin_tlat, &cos_tlat);
	sincosd (tlon - project_info.o_beta, &sin_tlon, &cos_tlon);
	cc = cos_tlat * cos_tlon;
	*lat = d_asind (project_info.o_sin_pole_lat * sin_tlat - project_info.o_cos_pole_lat * cc);
	*lon = project_info.o_pole_lon + d_atan2d (cos_tlat * sin_tlon, project_info.o_sin_pole_lat * cc + project_info.o_cos_pole_lat * sin_tlat);
}

void GMT_get_rotate_pole (double lon1, double lat1, double lon2, double lat2)
{
	double plon, plat, beta, dummy, x, y;
	double sin_lon1, cos_lon1, sin_lon2, cos_lon2, sin_lat1, cos_lat1, sin_lat2, cos_lat2;

	sincosd (lon1, &sin_lon1, &cos_lon1);
	sincosd (lon2, &sin_lon2, &cos_lon2);
	sincosd (lat1, &sin_lat1, &cos_lat1);
	sincosd (lat2, &sin_lat2, &cos_lat2);

	y = cos_lat1 * sin_lat2 * cos_lon1 - sin_lat1 * cos_lat2 * cos_lon2;
	x = sin_lat1 * cos_lat2 * sin_lon2 - cos_lat1 * sin_lat2 * sin_lon1;
	plon = d_atan2d (y, x);
	plat = atand (-cosd (plon - lon1) / tand (lat1));
	if (plat < 0.0) {
		plat = -plat;
		plon += 180.0;
		if (plon >= 360.0) plon -= 360.0;
	}
	project_info.o_pole_lon = plon;
	project_info.o_pole_lat = plat;
	sincosd (plat, &project_info.o_sin_pole_lat, &project_info.o_cos_pole_lat);
	GMT_pole_rotate_forward (lon1, lat1, &beta, &dummy);
	project_info.o_beta = -beta;
}

void GMT_get_origin (double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2)
{
	double beta, dummy, d, az, c;

	/* Now find origin that is 90 degrees from pole, let oblique lon=0 go through lon1/lat1 */
	c = cosd (lat_p) * cosd (lat1) * cosd (lon1-lon_p);
	c = d_acosd (sind (lat_p) * sind (lat1) + c);

	if (c != 90.0) {	/* Not true origin */
		d = fabs (90.0 - c);
		az = d_asind (sind (lon_p-lon1) * cosd (lat_p) / sind (c));
		if (c < 90.0) az += 180.0;
		*lat2 = d_asind (sind (lat1) * cosd (d) + cosd (lat1) * sind (d) * cosd (az));
		*lon2 = lon1 + d_atan2d (sind (d) * sind (az), cosd (lat1) * cosd (d) - sind (lat1) * sind (d) * cosd (az));
		if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Correct projection origin = %g/%g\n", GMT_program, *lon2, *lat2);
	}
	else {
		*lon2 = lon1;
		*lat2 = lat1;
	}

	GMT_pole_rotate_forward (*lon2, *lat2, &beta, &dummy);

	project_info.o_beta = -beta;
}

/*
 *	TRANSFORMATION ROUTINES FOR THE TRANSVERSE MERCATOR PROJECTION (GMT_TM)
 */

GMT_LONG GMT_map_init_tm (void) {
	double xmin, xmax, ymin, ymax, w, e, dummy;

	/* Wrap and truncations are in y, not x for TM */

	GMT_wrap_around_check = (PFL) GMT_wrap_around_check_tm;
	GMT_map_jump = (PFL) GMT_map_jump_tm;
	GMT_will_it_wrap = (PFB) GMT_will_it_wrap_tm;
	GMT_this_point_wraps = (PFB) GMT_this_point_wraps_tm;
	GMT_get_crossings = (PFV) GMT_get_crossings_tm;

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 1.0;	/* Select default map scale for TM */
	project_info.GMT_convert_latitudes = GMT_quicktm (project_info.pars[0], 10.0);
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vtm (project_info.pars[0], project_info.pars[1]);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	if (GMT_IS_SPHERICAL || project_info.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFL) GMT_tm_sph;
		GMT_inverse = (PFL) GMT_itm_sph;
	}
	else {
		GMT_forward = (PFL) GMT_tm;
		GMT_inverse = (PFL) GMT_itm;
	}

	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (GMT_world_map) {	/* Gave oblique degrees */
		w = project_info.central_meridian + project_info.s;
		e = project_info.central_meridian + project_info.n;
		project_info.s = -90;
		project_info.n = 90;
		project_info.e = e;
		project_info.w = w;
		GMT_vtm (project_info.pars[0], 0.0);
		(*GMT_forward) (project_info.w, 0.0, &xmin, &dummy);
		(*GMT_forward) (project_info.e, 0.0, &xmax, &dummy);
		(*GMT_forward) (project_info.w, project_info.s, &dummy, &ymin);
		ymax = ymin + (TWO_PI * project_info.EQ_RAD * gmtdefs.map_scale_factor);
		GMT_vtm (project_info.pars[0], project_info.pars[1]);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		GMT_world_map_tm = TRUE;
		project_info.region = FALSE;	/* Since wesn was oblique, not geographical wesn */
		project_info.e = project_info.central_meridian + 180.0;
		project_info.w = project_info.central_meridian - 180.0;
	}
	else if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		GMT_world_map_tm = GMT_IS_ZERO (project_info.n - project_info.s);
		GMT_world_map = FALSE;
	}
	else { /* Find min values */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		GMT_world_map_tm = FALSE;
		GMT_world_map = (fabs (project_info.s - project_info.n) < GMT_SMALL);
	}

	frame_info.horizontal = TRUE;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);

	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE UNIVERSAL TRANSVERSE MERCATOR PROJECTION (GMT_UTM)
 */

GMT_LONG GMT_map_init_utm (void) {
	double xmin, xmax, ymin, ymax, lon0;

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 0.9996;	/* Select default map scale for UTM */
	lon0 = 180.0 + 6.0 * project_info.pars[0] - 3.0;	/* Central meridian for this UTM zone */
	if (lon0 >= 360.0) lon0 -= 360.0;
	project_info.GMT_convert_latitudes = GMT_quicktm (lon0, 10.0);
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vtm (lon0, 0.0);	/* Central meridian for this zone */
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	switch (project_info.utm_hemisphere) {	/* Set hemisphere */
		case -1:
			project_info.north_pole = FALSE;
			break;
		case +1:
			project_info.north_pole = TRUE;
			break;
		default:
			project_info.north_pole = (project_info.s >= 0.0);
			break;
	}
	if (GMT_IS_SPHERICAL || project_info.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFL) GMT_utm_sph;
		GMT_inverse = (PFL) GMT_iutm_sph;
	}
	else {
		GMT_forward = (PFL) GMT_utm;
		GMT_inverse = (PFL) GMT_iutm;
	}

	if (fabs (project_info.w - project_info.e) > 360.0) {	/* -R in UTM meters */
		(*GMT_inverse) (&project_info.w, &project_info.s, project_info.w, project_info.s);
		(*GMT_inverse) (&project_info.e, &project_info.n, project_info.e, project_info.n);
		project_info.region = FALSE;
	}
	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}

	frame_info.horizontal = TRUE;

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);

	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

	return (!project_info.region);
}

/* Setting w/e/s/n for a fully qualified UTM zone */

GMT_LONG GMT_UTMzone_to_wesn (GMT_LONG zone_x, GMT_LONG zone_y, GMT_LONG hemi, double *w, double *e, double *s, double *n)
{	/* Given the full UTM zone specification, return w/e/s/n */

	GMT_LONG error = 0;

	*e = 180.0 + 6.0 * zone_x;	*w = *e - 6.0;

	if (zone_y == 0) {	/* Latitude zone is not specified */
		if (hemi == -1) {
			*s = -80.0;	*n = 0.0;
		}
		else if (hemi == +1) {
			*s = 0.0;	*n = 84.0;
		}
		else
			error = TRUE;
		return (error);
	}
	else if (zone_y < 'A' || zone_y > 'Z')
		error = TRUE;
	else if (zone_y <= 'B') {
		*s = -90.0;	*n = -80.0;
		*e = 180.0 * (zone_y - 'A');
		*w = *e - 180.0;
	}
	else if (zone_y <= 'I') {	/* I will behave as J */
		*s = -80.0 + 8.0 * (zone_y - 'C');	*n = *s + 8.0;
	}
	else if (zone_y <= 'O') {	/* O will behave as P */
		*s = -80.0 + 8.0 * (zone_y - 'D');	*n = *s + 8.0;
	}
	else if (zone_y <= 'W') {
		*s = -80.0 + 8.0 * (zone_y - 'E');	*n = *s + 8.0;
		if (zone_y == 'V' && zone_x == 31) *e = 3.0;
		if (zone_y == 'V' && zone_x == 32) *w = 3.0;
	}
	else if (zone_y == 'X') {
		*s = 72.0;	*n = 84.0;
		if (zone_x == 31) *e = 9.0;
		if (zone_x == 33) {*w = 9.0; *e = 21.0;}
		if (zone_x == 35) {*w = 21.0; *e = 33.0;}
		if (zone_x == 37) *w = 33.0;
		if (zone_x == 32 || zone_x == 34 || zone_x == 36) error = TRUE;
	}
	else {	/* Y or Z */
		*s = 84.0;	*n = 90.0;
		*e = 180.0 * (zone_y - 'Y');
		*w = *e - 180.0;
	}

	return (error);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION (GMT_LAMB_AZ_EQ)
 */

GMT_LONG GMT_map_init_lambeq (void) {
	double xmin, xmax, ymin, ymax, dummy, radius, D, s, c;

	project_info.Dx = project_info.Dy = 1.0;

	GMT_set_polar (project_info.pars[1]);
	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vlambeq (project_info.pars[0], project_info.pars[1], project_info.pars[2]);

	if (project_info.GMT_convert_latitudes) {
		sincosd (project_info.pars[1], &s, &c);
		D = (project_info.polar) ? 1.0 : (gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / project_info.GMT_lat_swap_vals.ra) * c / (project_info.cosp * d_sqrt (1.0 - project_info.ECC2 * s * s));
		project_info.Dx = D;
		project_info.Dy = 1.0 / D;
	}
	project_info.iDx = 1.0 / project_info.Dx;
	project_info.iDy = 1.0 / project_info.Dy;

	GMT_forward = (PFL) GMT_lambeq;
	GMT_inverse = (PFL) GMT_ilambeq;
	if (project_info.units_pr_degree) {
		GMT_vlambeq (0.0, 90.0, project_info.pars[2]);
		GMT_lambeq (0.0, fabs(project_info.pars[4]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
		GMT_vlambeq (project_info.pars[0], project_info.pars[1], project_info.pars[2]);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[3];

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s <= -90.0){
					fprintf (stderr, "%s: Error: South boundary cannot be -90.0 for north polar Lambert azimuthal projection\n", GMT_program);
					GMT_exit (EXIT_FAILURE);
				}
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n >= 90.0) {
					fprintf (stderr, "%s: Error: North boundary cannot be +90.0 for south polar Lambert azimuthal projection\n", GMT_program);
					GMT_exit (EXIT_FAILURE);
				}
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFL) GMT_polar_outside;
			GMT_crossing = (PFL) GMT_wesn_crossing;
			GMT_overlap = (PFL) GMT_wesn_overlap;
			GMT_map_clip = (PFL) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			GMT_n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmax = ymax = project_info.rho_max;
			xmin = ymin = -xmax;
			GMT_outside = (PFL) GMT_radial_outside;
			GMT_crossing = (PFL) GMT_radial_crossing;
			GMT_overlap = (PFL) GMT_radial_overlap;
			GMT_map_clip = (PFL) GMT_radial_clip;
			if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ORTHOGRAPHIC PROJECTION (GMT_ORTHO)
 */

GMT_LONG GMT_map_init_ortho (void) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);

	if (project_info.units_pr_degree) {
		GMT_vortho (0.0, 90.0, project_info.pars[2]);
		GMT_ortho (0.0, fabs(project_info.pars[4]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[3];

	GMT_vortho (project_info.pars[0], project_info.pars[1], project_info.pars[2]);
	GMT_forward = (PFL) GMT_ortho;
	GMT_inverse = (PFL) GMT_iortho;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s < 0.0) {
					fprintf (stderr, "%s: Warning: South boundary cannot be < 0 for north polar orthographic projection (reset to 0)\n", GMT_program);
					project_info.s = 0.0;
				}
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n > 0.0) {
					fprintf (stderr, "%s: Warning: North boundary cannot be > 0 for south polar orthographic projection (reset to 0)\n", GMT_program);
					project_info.n = 0.0;
				}
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFL) GMT_polar_outside;
			GMT_crossing = (PFL) GMT_wesn_crossing;
			GMT_overlap = (PFL) GMT_wesn_overlap;
			GMT_map_clip = (PFL) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			GMT_n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmax = ymax = project_info.rho_max * project_info.EQ_RAD;
			xmin = ymin = -xmax;
			GMT_outside = (PFL) GMT_radial_outside;
			GMT_crossing = (PFL) GMT_radial_crossing;
			GMT_overlap = (PFL) GMT_radial_overlap;
			GMT_map_clip = (PFL) GMT_radial_clip;
			if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE GENERAL PERSPECTIVE PROJECTION (GMT_GENPER)
 */

GMT_LONG GMT_map_init_genper (void) {
	GMT_LONG search;
	double xmin, xmax, ymin, ymax, dummy, radius;

	double alt, azimuth, tilt, width, height;
	double twist, scale, units;

	units = project_info.pars[2];
	scale = project_info.pars[3];
	alt = project_info.pars[4];
	azimuth = project_info.pars[5];
	tilt = project_info.pars[6];
	twist = project_info.pars[7];
	width = project_info.pars[8];
	height = project_info.pars[9];

	if (project_info.g_sphere) GMT_set_spherical (); /* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);

	if (project_info.units_pr_degree) {
		GMT_vgenper (0.0, 90.0, alt, azimuth, tilt, twist, width, height);
		GMT_genper (0.0, fabs(project_info.pars[3]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[2];

	if (project_info.g_debug > 1) {
		fprintf (stderr, "genper: units_pr_degree %ld\n", project_info.units_pr_degree);
		fprintf (stderr, "genper: radius %f\n", radius);
		fprintf (stderr, "genper: scale %f units %f\n", scale, units);
		fprintf (stderr, "genper: x scale %f y scale %f\n", project_info.x_scale, project_info.y_scale);
		fprintf (stderr, "genper: gave_map_width %ld \n",project_info.gave_map_width);
	}

	GMT_vgenper (project_info.pars[0], project_info.pars[1], alt, azimuth, tilt, twist, width, height);
	GMT_forward = (PFL) GMT_genper;
	GMT_inverse = (PFL) GMT_igenper;

	project_info.w = 0.0;
	project_info.e = 360.0;
	project_info.s = -90.0;
	project_info.n = 90.0;

	xmin = ymin = -project_info.g_rmax;
	xmax = ymax = -xmin;

	xmin = project_info.g_xmin;
	xmax = project_info.g_xmax;
	ymin = project_info.g_ymin;
	ymax = project_info.g_ymax;

	if (project_info.g_width != 0.0) {
		project_info.region = 1;
		if (project_info.g_debug > 0) fprintf(stderr,"using windowed region\n");
		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip_old;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
	}
	else {
		if (project_info.g_debug > 0) fprintf(stderr,"using global view\n");

		/* No annotations for global mode */
		frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;

		/* No tickmarks for global mode */
		frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;

		GMT_overlap = (PFL) GMT_genper_overlap;
		GMT_crossing = (PFL) GMT_radial_crossing;
		GMT_map_clip = (PFL) GMT_radial_clip;
		GMT_outside = (PFL) GMT_radial_outside;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;

		if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

		search = FALSE;
  	}

	if (project_info.polar) {
		if (project_info.north_pole) {
			if (project_info.s < (90.0 - project_info.f_horizon)) project_info.s = 90.0 - project_info.f_horizon;
			if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
		} else {
			if (project_info.n > -(90.0 - project_info.f_horizon)) project_info.n = -(90.0 - project_info.f_horizon);
			if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
		}
		if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
	}

	if (project_info.g_debug > 0) fprintf(stderr,"xmin %f xmax %f ymin %f ymax %f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000);

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);

	project_info.r = 0.5 * project_info.xmax;

	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	if (project_info.g_debug > 0) {
		fprintf(stderr,"x scale %e y scale %e\n", project_info.x_scale, project_info.y_scale);
		fprintf(stderr,"x center %f y center %f\n", project_info.c_x0, project_info.c_y0);
		fprintf(stderr,"x max %f y max %f\n", project_info.xmax, project_info.ymax);
		fprintf(stderr,"x0 %f y0 %f\n", project_info.x0, project_info.y0);
		fflush(NULL);
	}

	if (gmtdefs.verbose) fprintf(stderr,"\n\n");

	return (search);

}

/*
 *	TRANSFORMATION ROUTINES FOR THE GNOMONIC PROJECTION (GMT_GNOMONIC)
 */

GMT_LONG GMT_map_init_gnomonic (void) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);

	if (project_info.units_pr_degree) {
		GMT_vgnomonic (0.0, 90.0, 60.0);
		GMT_gnomonic (0.0, fabs(project_info.pars[4]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[3];

	GMT_vgnomonic (project_info.pars[0], project_info.pars[1], project_info.pars[2]);
	GMT_forward = (PFL) GMT_gnomonic;
	GMT_inverse = (PFL) GMT_ignomonic;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s < (90.0 - project_info.f_horizon)) project_info.s = 90.0 - project_info.f_horizon;
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n > -(90.0 - project_info.f_horizon)) project_info.n = -(90.0 - project_info.f_horizon);
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFL) GMT_polar_outside;
			GMT_crossing = (PFL) GMT_wesn_crossing;
			GMT_overlap = (PFL) GMT_wesn_overlap;
			GMT_map_clip = (PFL) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			GMT_n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			/* frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0; */	/* No annotations for global mode */
			/* frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0; */	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmax = ymax = project_info.rho_max * project_info.EQ_RAD;
			xmin = ymin = -xmax;
			GMT_outside = (PFL) GMT_radial_outside;
			GMT_crossing = (PFL) GMT_radial_crossing;
			GMT_overlap = (PFL) GMT_radial_overlap;
			GMT_map_clip = (PFL) GMT_radial_clip;
			if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE AZIMUTHAL EQUIDISTANT PROJECTION (GMT_AZ_EQDIST)
 */

GMT_LONG GMT_map_init_azeqdist (void) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);

	if (project_info.units_pr_degree) {
		GMT_vazeqdist (0.0, 90.0, project_info.pars[2]);
		GMT_azeqdist (0.0, fabs(project_info.pars[4]), &dummy, &radius);
		if (GMT_IS_ZERO (radius)) radius = project_info.rho_max * project_info.EQ_RAD;
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[3] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[3];

	GMT_vazeqdist (project_info.pars[0], project_info.pars[1], project_info.pars[2]);
	GMT_forward = (PFL) GMT_azeqdist;
	GMT_inverse = (PFL) GMT_iazeqdist;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);

		GMT_outside = (PFL) GMT_rect_outside2;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 60.0 && fabs (project_info.n - project_info.s) < 30.0);
	}
	else {
		if (project_info.polar && (project_info.n - project_info.s) < 180.0) {	/* Polar aspect */
			if (!project_info.north_pole && project_info.s <= -90.0) project_info.edge[0] = FALSE;
			if (project_info.north_pole && project_info.n >= 90.0) project_info.edge[2] = FALSE;
			if (GMT_360_RANGE (project_info.w, project_info.e) || GMT_IS_ZERO (project_info.e - project_info.w)) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFL) GMT_polar_outside;
			GMT_crossing = (PFL) GMT_wesn_crossing;
			GMT_overlap = (PFL) GMT_wesn_overlap;
			GMT_map_clip = (PFL) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			GMT_n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only, force wesn = 0/360/-90/90  */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmax = ymax = project_info.rho_max * project_info.EQ_RAD;
			xmin = ymin = -xmax;
			xmax = ymax = -xmin;
			GMT_outside = (PFL) GMT_radial_outside;
			GMT_crossing = (PFL) GMT_radial_crossing;
			GMT_overlap = (PFL) GMT_radial_overlap;
			GMT_map_clip = (PFL) GMT_radial_clip;
			if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MOLLWEIDE EQUAL AREA PROJECTION (GMT_MOLLWEIDE)
 */

GMT_LONG GMT_map_init_mollweide (void) {
	double xmin, xmax, ymin, ymax, y, dummy;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = M_PI * project_info.pars[1] / sqrt (8.0);
	GMT_vmollweide (project_info.pars[0], project_info.pars[1]);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_mollweide (project_info.w, y, &xmin, &dummy);
		GMT_mollweide (project_info.e, y, &xmax, &dummy);
		GMT_mollweide (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_mollweide (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_ellipse;
		GMT_right_edge = (PFD) GMT_right_ellipse;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
	}
	else {
		GMT_mollweide (project_info.w, project_info.s, &xmin, &ymin);
		GMT_mollweide (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_mollweide;
	GMT_inverse = (PFL) GMT_imollweide;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;

	return (!project_info.region);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE HAMMER-AITOFF EQUAL AREA PROJECTION (GMT_HAMMER)
 */

GMT_LONG GMT_map_init_hammer (void) {
	double xmin, xmax, ymin, ymax, x, y, dummy;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = 0.5 * M_PI * project_info.pars[1] / M_SQRT2;
	GMT_vhammer (project_info.pars[0], project_info.pars[1]);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		x = (fabs (project_info.w - project_info.central_meridian) > fabs (project_info.e - project_info.central_meridian)) ? project_info.w : project_info.e;
		GMT_hammer (project_info.w, y, &xmin, &dummy);
		GMT_hammer (project_info.e, y, &xmax, &dummy);
		GMT_hammer (x, project_info.s, &dummy, &ymin);
		GMT_hammer (x, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_ellipse;
		GMT_right_edge = (PFD) GMT_right_ellipse;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
	}
	else {
		GMT_hammer (project_info.w, project_info.s, &xmin, &ymin);
		GMT_hammer (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_hammer;
	GMT_inverse = (PFL) GMT_ihammer;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE VAN DER GRINTEN PROJECTION (GMT_VANGRINTEN)
 */

GMT_LONG GMT_map_init_grinten (void) {
	double xmin, xmax, ymin, ymax, x, y, dummy;

	GMT_set_spherical ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_vgrinten (project_info.pars[0], project_info.pars[1]);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		x = (fabs (project_info.w - project_info.central_meridian) > fabs (project_info.e - project_info.central_meridian)) ? project_info.w : project_info.e;
		GMT_grinten (project_info.w, y, &xmin, &dummy);
		GMT_grinten (project_info.e, y, &xmax, &dummy);
		GMT_grinten (x, project_info.s, &dummy, &ymin);
		GMT_grinten (x, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
	}
	else {
		GMT_grinten (project_info.w, project_info.s, &xmin, &ymin);
		GMT_grinten (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_forward = (PFL) GMT_grinten;
	GMT_inverse = (PFL) GMT_igrinten;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE WINKEL-TRIPEL MODIFIED AZIMUTHAL PROJECTION (GMT_WINKEL)
 */

GMT_LONG GMT_map_init_winkel (void) {
	double xmin, xmax, ymin, ymax, x, y, dummy;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_vwinkel (project_info.pars[0], project_info.pars[1]);
	project_info.x_scale = project_info.y_scale = 2.0 * project_info.pars[1] / (1.0 + project_info.r_cosphi1);

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		x = (fabs (project_info.w - project_info.central_meridian) > fabs (project_info.e - project_info.central_meridian)) ? project_info.w : project_info.e;
		GMT_winkel (project_info.w, y, &xmin, &dummy);
		GMT_winkel (project_info.e, y, &xmax, &dummy);
		GMT_winkel (x, project_info.s, &dummy, &ymin);
		GMT_winkel (x, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_winkel;
		GMT_right_edge = (PFD) GMT_right_winkel;
		frame_info.horizontal = 2;
	}
	else {
		GMT_winkel (project_info.w, project_info.s, &xmin, &ymin);
		GMT_winkel (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_winkel;
	GMT_inverse = (PFL) GMT_iwinkel;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT IV PROJECTION (GMT_ECKERT4)
 */

GMT_LONG GMT_map_init_eckert4 (void) {
	double xmin, xmax, ymin, ymax, y, dummy;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_veckert4 (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = project_info.pars[1];

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_eckert4 (project_info.w, y, &xmin, &dummy);
		GMT_eckert4 (project_info.e, y, &xmax, &dummy);
		GMT_eckert4 (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_eckert4 (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_eckert4;
		GMT_right_edge = (PFD) GMT_right_eckert4;
		frame_info.horizontal = 2;
	}
	else {
		GMT_eckert4 (project_info.w, project_info.s, &xmin, &ymin);
		GMT_eckert4 (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_eckert4;
	GMT_inverse = (PFL) GMT_ieckert4;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT VI PROJECTION (GMT_ECKERT6)
 */

GMT_LONG GMT_map_init_eckert6 (void) {
	double xmin, xmax, ymin, ymax, y, dummy;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_veckert6 (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = 0.5 * project_info.pars[1] * sqrt (2.0 + M_PI);

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_eckert6 (project_info.w, y, &xmin, &dummy);
		GMT_eckert6 (project_info.e, y, &xmax, &dummy);
		GMT_eckert6 (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_eckert6 (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_eckert6;
		GMT_right_edge = (PFD) GMT_right_eckert6;
		frame_info.horizontal = 2;
	}
	else {
		GMT_eckert6 (project_info.w, project_info.s, &xmin, &ymin);
		GMT_eckert6 (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_eckert6;
	GMT_inverse = (PFL) GMT_ieckert6;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ROBINSON PSEUDOCYLINDRICAL PROJECTION (GMT_ROBINSON)
 */

GMT_LONG GMT_map_init_robinson (void) {
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_vrobinson (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = project_info.pars[1] / 0.8487;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_robinson (project_info.w, y, &xmin, &dummy);
		GMT_robinson (project_info.e, y, &xmax, &dummy);
		GMT_robinson (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_robinson (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_robinson;
		GMT_right_edge = (PFD) GMT_right_robinson;
		frame_info.horizontal = 2;
	}
	else {
		GMT_robinson (project_info.w, project_info.s, &xmin, &ymin);
		GMT_robinson (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFL) GMT_robinson;
	GMT_inverse = (PFL) GMT_irobinson;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE SINUSOIDAL EQUAL AREA PROJECTION (GMT_SINUSOIDAL)
 */

GMT_LONG GMT_map_init_sinusoidal (void) {
	double xmin, xmax, ymin, ymax, dummy, y;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
	GMT_vsinusoidal (project_info.pars[0]);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_forward = (PFL) GMT_sinusoidal;
	GMT_inverse = (PFL) GMT_isinusoidal;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_sinusoidal (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_sinusoidal (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_sinusoidal (project_info.w, y, &xmin, &dummy);
		GMT_sinusoidal (project_info.e, y, &xmax, &dummy);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_sinusoidal;
		GMT_right_edge = (PFD) GMT_right_sinusoidal;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
	}
	else {
		GMT_sinusoidal (project_info.w, project_info.s, &xmin, &ymin);
		GMT_sinusoidal (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_parallel_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE CASSINI PROJECTION (GMT_CASSINI)
 */

GMT_LONG GMT_map_init_cassini (void) {
	GMT_LONG too_big;
	double xmin, xmax, ymin, ymax;

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	too_big = GMT_quicktm (project_info.pars[0], 4.0);
	if (too_big) GMT_set_spherical();	/* Cannot use ellipsoidal series for this area */
	GMT_vcassini (project_info.pars[0], project_info.pars[1]);
	if (GMT_IS_SPHERICAL) {
		GMT_forward = (PFL) GMT_cassini_sph;
		GMT_inverse = (PFL) GMT_icassini_sph;
	}
	else {
		GMT_forward = (PFL) GMT_cassini;
		GMT_inverse = (PFL) GMT_icassini;
	}
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}

	frame_info.horizontal = TRUE;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ALBERS PROJECTION (GMT_ALBERS)
 */

GMT_LONG GMT_map_init_albers (void) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	project_info.GMT_convert_latitudes = GMT_quickconic();
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	if (GMT_IS_SPHERICAL || project_info.GMT_convert_latitudes) {	/* Spherical code w/wo authalic latitudes */
		GMT_valbers_sph (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
		GMT_forward = (PFL) GMT_albers_sph;
		GMT_inverse = (PFL) GMT_ialbers_sph;
	}
	else {
		GMT_valbers (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
		GMT_forward = (PFL) GMT_albers;
		GMT_inverse = (PFL) GMT_ialbers;
	}
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];

	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	frame_info.horizontal = TRUE;
	GMT_n_lat_nodes = 2;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);

	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	GMT_geo_to_xy (project_info.central_meridian + 90., project_info.pole, &x1, &y1);
	dy = y1 - project_info.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - project_info.c_x0);
	dy /= (1.0 - cos (az));
	project_info.c_y0 += dy;
	GMT_meridian_straight = TRUE;

	return (!project_info.region);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE EQUIDISTANT CONIC PROJECTION (GMT_ECONIC)
 */


GMT_LONG GMT_map_init_econic (void) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	project_info.GMT_convert_latitudes = !GMT_IS_SPHERICAL;
	if (project_info.GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_veconic (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
	GMT_forward = (PFL) GMT_econic;
	GMT_inverse = (PFL) GMT_ieconic;
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];

	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}
	frame_info.horizontal = TRUE;
	GMT_n_lat_nodes = 2;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);

	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	GMT_geo_to_xy (project_info.central_meridian + 90., project_info.pole, &x1, &y1);
	dy = y1 - project_info.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - project_info.c_x0);
	dy /= (1.0 - cos (az));
	project_info.c_y0 += dy;
	GMT_meridian_straight = TRUE;

	return (!project_info.region);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE POLYCONIC PROJECTION (GMT_POLYCONIC)
 */

GMT_LONG GMT_map_init_polyconic (void) {
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (GMT_is_dnan(project_info.pars[0])) project_info.pars[0] = 0.5 * (project_info.w + project_info.e);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
	GMT_vpolyconic (project_info.pars[0], project_info.pars[1]);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_forward = (PFL) GMT_polyconic;
	GMT_inverse = (PFL) GMT_ipolyconic;
	if (gmtdefs.basemap_type == GMT_IS_FANCY) gmtdefs.basemap_type = GMT_IS_PLAIN;

	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		(*GMT_forward) (project_info.w, y, &xmin, &dummy);
		(*GMT_forward) (project_info.e, y, &xmax, &dummy);
		(*GMT_forward) (project_info.central_meridian, project_info.s, &dummy, &ymin);
		(*GMT_forward) (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFL) GMT_wesn_outside;
		GMT_crossing = (PFL) GMT_wesn_crossing;
		GMT_overlap = (PFL) GMT_wesn_overlap;
		GMT_map_clip = (PFL) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_polyconic;
		GMT_right_edge = (PFD) GMT_right_polyconic;
		project_info.polar = TRUE;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFL) GMT_rect_outside;
		GMT_crossing = (PFL) GMT_rect_crossing;
		GMT_overlap = (PFL) GMT_rect_overlap;
		GMT_map_clip = (PFL) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
	}

	frame_info.horizontal = TRUE;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);

	return (!project_info.region);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1.1 :	S U P P O R T I N G   R O U T I N E S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void GMT_wesn_search (double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north) {
	double dx, dy, w, e, s, n, x, y, lon, lat;
	GMT_LONG i, j;

	/* Search for extreme original coordinates lon/lat */

	dx = (xmax - xmin) / GMT_n_lon_nodes;
	dy = (ymax - ymin) / GMT_n_lat_nodes;
	w = s = DBL_MAX;	e = n = -DBL_MAX;
	for (i = 0; i <= GMT_n_lon_nodes; i++) {
		x = (i == GMT_n_lon_nodes) ? xmax : xmin + i * dx;
		GMT_xy_to_geo (&lon, &lat, x, ymin);
		if (lon < w) w = lon;
		if (lon > e) e = lon;
		if (lat < s) s = lat;
		if (lat > n) n = lat;
		GMT_xy_to_geo (&lon, &lat, x, ymax);
		if (lon < w) w = lon;
		if (lon > e) e = lon;
		if (lat < s) s = lat;
		if (lat > n) n = lat;
	}
	for (j = 0; j <= GMT_n_lat_nodes; j++) {
		y = (j == GMT_n_lat_nodes) ? ymax : ymin + j * dy;
		GMT_xy_to_geo (&lon, &lat, xmin, y);
		if (lon < w) w = lon;
		if (lon > e) e = lon;
		if (lat < s) s = lat;
		if (lat > n) n = lat;
		GMT_xy_to_geo (&lon, &lat, xmax, y);
		if (lon < w) w = lon;
		if (lon > e) e = lon;
		if (lat < s) s = lat;
		if (lat > n) n = lat;
	}

	/* Then check if one or both poles are inside map; then the above wont be correct */

	if (!GMT_map_outside (project_info.central_meridian, 90.0)) {
		n = 90.0;
		w = 0.0;
		e = 360.0;
	}
	if (!GMT_map_outside (project_info.central_meridian, -90.0)) {
		s = -90.0;
		w = 0.0;
		e = 360.0;
	}

	s -= 0.1;	if (s < -90.0) s = -90.0;	/* Make sure point is not inside area, 0.1 is just a small number */
	n += 0.1;	if (n > 90.0) n = 90.0;
	w -= 0.1;	e += 0.1;

	if (fabs (w - e) > 360.0) {
		w = 0.0;
		e = 360.0;
	}
	*west = w;
	*east = e;
	*south = s;
	*north = n;
}

GMT_LONG GMT_horizon_search (double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax) {
	double dx, dy, d, x, y, lon, lat;
	GMT_LONG i, j;
	GMT_LONG beyond = FALSE;

	/* Search for extreme original coordinates lon/lat and see if any fall beyond the horizon */

	dx = (xmax - xmin) / GMT_n_lon_nodes;
	dy = (ymax - ymin) / GMT_n_lat_nodes;
	if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, w, s)) > project_info.f_horizon) beyond = TRUE;
	if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, e, n)) > project_info.f_horizon) beyond = TRUE;
	for (i = 0; !beyond && i <= GMT_n_lon_nodes; i++) {
		x = (i == GMT_n_lon_nodes) ? xmax : xmin + i * dx;
		GMT_xy_to_geo (&lon, &lat, x, ymin);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (&lon, &lat, x, ymax);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
	}
	for (j = 0; !beyond && j <= GMT_n_lat_nodes; j++) {
		y = (j == GMT_n_lat_nodes) ? ymax : ymin + j * dy;
		GMT_xy_to_geo (&lon, &lat, xmin, y);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (&lon, &lat, xmax, y);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
	}
	if (beyond) {
		fprintf (stderr, "%s: ERROR: Rectangular region for azimuthal projection extends beyond the horizon\n", GMT_program);
		fprintf (stderr, "%s: ERROR: Please select a region that is completely within the visible hemisphere\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

void GMT_xy_search (double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0)
{
	GMT_LONG i, j;
	double xmin, xmax, ymin, ymax, w, s, x, y, dlon, dlat;

	/* Find min/max forward values */

	xmax = ymax = -DBL_MAX;
	xmin = ymin = DBL_MAX;
	dlon = fabs (e0 - w0) / 500;
	dlat = fabs (n0 - s0) / 500;

	for (i = 0; i <= 500; i++) {
		w = w0 + i * dlon;
		(*GMT_forward) (w, s0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*GMT_forward) (w, n0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}
	for (j = 0; j <= 500; j++) {
		s = s0 + j * dlat;
		(*GMT_forward) (w0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*GMT_forward) (e0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}

	*x0 = xmin;	*x1 = xmax;	*y0 = ymin;	*y1 = ymax;
}

GMT_LONG GMT_map_crossing (double lon1, double lat1, double lon2, double lat2, double *xlon, double *xlat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Check if line segment between (lon1,lat1) and (lon2,lat2) crosses any map boundary.
	 * If so: create new points xlon[],xlat[] to function as intermediaries as well as their
	 * projected coordinates xx[],yy[].
	 * Return value is zero when no crossing was found, otherwise returns 1 or 2.
	 */

	if (GMT_x_status_old == GMT_x_status_new && GMT_y_status_old == GMT_y_status_new) {
		/* This is naieve. We could have two points outside with a line drawn between crossing the plotting area. */
		return (0);
	}
	else if ((GMT_x_status_old == 0 && GMT_y_status_old == 0) || (GMT_x_status_new == 0 && GMT_y_status_new == 0)) {
		/* This is a crossing */
	}
	else if (!(*GMT_overlap) (lon1, lat1, lon2, lat2))	/* Less clearcut case, check for overlap */
		return (0);

	/* Now compute the crossing */
	GMT_corner = -1;
	return ((*GMT_crossing) (lon1, lat1, lon2, lat2, xlon, xlat, xx, yy, sides));
}

GMT_LONG GMT_map_outside (double lon, double lat)
{
	GMT_x_status_old = GMT_x_status_new;
	GMT_y_status_old = GMT_y_status_new;
	return ((*GMT_outside) (lon, lat));
}

GMT_LONG GMT_rect_overlap (double lon0, double lat0, double lon1, double lat1)
{
	/* Return true if the projection of either (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular map boundary */
	double x0, y0, x1, y1;

	GMT_geo_to_xy (lon0, lat0, &x0, &y0);
	GMT_geo_to_xy (lon1, lat1, &x1, &y1);

	if (x0 > x1) d_swap (x0, x1);
	if (y0 > y1) d_swap (y0, y1);
	/* if (x1 < project_info.xmin || x0 > project_info.xmax) return (FALSE);
	if (y1 < project_info.ymin || y0 > project_info.ymax) return (FALSE); */

	if ((x1 - project_info.xmin) < -GMT_CONV_LIMIT || (x0 - project_info.xmax) > GMT_CONV_LIMIT) return (FALSE);
	if ((y1 - project_info.ymin) < -GMT_CONV_LIMIT || (y0 - project_info.ymax) > GMT_CONV_LIMIT) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_wesn_overlap (double lon0, double lat0, double lon1, double lat1)
{
	/* Return true if either of the points (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular lon/lat boundaries */
	if (lon0 > lon1) d_swap (lon0, lon1);
	if (lat0 > lat1) d_swap (lat0, lat1);
	if (lon1 - project_info.w < -GMT_CONV_LIMIT) {
		lon0 += 360.0;
		lon1 += 360.0;
	}
	else if (lon0 - project_info.e > GMT_CONV_LIMIT) {
		lon0 -= 360.0;
		lon1 -= 360.0;
	}
	/* if (lon1 < project_info.w || lon0 > project_info.e) return (FALSE);
	if (lat1 < project_info.s || lat0 > project_info.n) return (FALSE); */

	if (lon1 - project_info.w < -GMT_CONV_LIMIT || lon0 - project_info.e > GMT_CONV_LIMIT) return (FALSE);
	if (lat1 - project_info.s < -GMT_CONV_LIMIT || lat0 - project_info.n > GMT_CONV_LIMIT) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_radial_overlap (double lon0, double lat0, double lon1, double lat1)
{	/* Dummy routine */
	return (TRUE);
}

GMT_LONG GMT_wrap_around_check_x (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides)
{
	double dx, dy, width, jump, GMT_half_map_width (double y);

	jump = this_x - last_x;
	width = MAX (GMT_half_map_width (this_y), GMT_half_map_width (last_y));

	if (fabs (jump) - width <= GMT_SMALL || fabs(jump) <= GMT_SMALL) return (0);
	dy = this_y - last_y;

	if (jump < -width) {	/* Crossed right boundary */
		dx = GMT_map_width + jump;
		yy[0] = yy[1] = last_y + (GMT_map_width - last_x) * dy / dx;
		if (yy[0] < 0.0 || yy[0] > project_info.ymax) return (0);
		xx[0] = GMT_right_boundary (yy[0]);	xx[1] = GMT_left_boundary (yy[0]);
		sides[0] = 1;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed left boundary */
		dx = GMT_map_width - jump;
		yy[0] = yy[1] = last_y + last_x * dy / dx;
		if (yy[0] < 0.0 || yy[0] > project_info.ymax) return (0);
		xx[0] = GMT_left_boundary (yy[0]);	xx[1] = GMT_right_boundary (yy[0]);
		sides[0] = 3;
		angle[0] = d_atan2d (dy, -dx);
	}
	sides[1] = 4 - sides[0];
	angle[1] = angle[0] + 180.0;

	return (2);
}

/* For global TM maps */

GMT_LONG GMT_wrap_around_check_tm (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides)
{
	double dx, dy, width, jump;

	jump = this_y - last_y;
	width = 0.5 * GMT_map_height;

	if (fabs (jump) - width <= GMT_SMALL || fabs(jump) <= GMT_SMALL) return (0);
	dx = this_x - last_x;

	if (jump < -width) {	/* Crossed top boundary */
		dy = GMT_map_height + jump;
		xx[0] = xx[1] = last_x + (GMT_map_height - last_y) * dx / dy;
		if (xx[0] < 0.0 || xx[0] > project_info.xmax) return (0);
		yy[0] = GMT_map_height;	yy[1] = 0.0;
		sides[0] = 2;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed bottom boundary */
		dy = GMT_map_height - jump;
		xx[0] = xx[1] = last_x + last_y * dx / dy;
		if (xx[0] < 0.0 || xx[0] > project_info.xmax) return (0);
		yy[0] = 0.0;	yy[1] = GMT_map_height;
		sides[0] = 0;
		angle[0] = d_atan2d (dy, -dx);
	}
	angle[1] = angle[0] + 180.0;
	sides[1] = 2 - sides[0];

	return (2);
}

double GMT_left_ellipse (double y)
{
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - project_info.y0) / project_info.w_r;	/* Fraction, relative to Equator */
	return (GMT_half_map_size - 2.0 * project_info.w_r * d_sqrt (1.0 - y * y));
}

double GMT_left_circle (double y)
{
	/* y -= project_info.r; */
	y -= project_info.y0;
	return (GMT_half_map_size - d_sqrt (project_info.r * project_info.r - y * y));
}

double GMT_left_conic (double y)
{
	double x_ws, y_ws, x_wn, y_wn, dy;

	GMT_geo_to_xy (project_info.w, project_info.s, &x_ws, &y_ws);
	GMT_geo_to_xy (project_info.w, project_info.n, &x_wn, &y_wn);
	dy = y_wn - y_ws;
	if (GMT_IS_ZERO (dy)) return (0.0);
	return (x_ws + ((x_wn - x_ws) * (y - y_ws) / dy));
}

double GMT_left_rect (double y)
{
	return (0.0);
}

double GMT_left_boundary (double y)
{
	return ((*GMT_left_edge) (y));
}

double GMT_right_ellipse (double y)
{
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - project_info.y0) / project_info.w_r;	/* Fraction, relative to Equator */
	return (GMT_half_map_size + 2.0 * project_info.w_r * d_sqrt (1.0 - y * y));
}

double GMT_right_circle (double y)
{
	/* y -= project_info.r; */
	y -= project_info.y0;
	return (GMT_half_map_size + d_sqrt (project_info.r * project_info.r - y * y));
}

double GMT_right_conic (double y)
{
	double x_es, y_es, x_en, y_en, dy;

	GMT_geo_to_xy (project_info.e, project_info.s, &x_es, &y_es);
	GMT_geo_to_xy (project_info.e, project_info.n, &x_en, &y_en);
	dy = y_en - y_es;
	if (GMT_IS_ZERO (dy)) return (GMT_map_width);
	return (x_es - ((x_es - x_en) * (y - y_es) / dy));
}

double GMT_right_rect (double y)
{
	return (GMT_map_width);
}

double GMT_right_boundary (double y)
{
	return ((*GMT_right_edge) (y));
}

GMT_LONG GMT_map_jump_x (double x0, double y0, double x1, double y1)
{
	/* TRUE if x-distance between points exceeds 1/2 map width at this y value */
	double dx, map_half_size;

	if (! (GMT_IS_CYLINDRICAL || GMT_IS_MISC)) return (0);	/* Only projections with peroidic boundaries may apply */
	
	if (!GMT_IS_MAPPING || fabs (project_info.w - project_info.e) < 90.0) return (FALSE);

	map_half_size = MAX (GMT_half_map_width (y0), GMT_half_map_width (y1));
	if (fabs (map_half_size) < GMT_SMALL) return (0);

	dx = x1 - x0;
	if (dx > map_half_size)	/* Cross left/west boundary */
		return (-1);
	else if (dx < (-map_half_size)) /* Cross right/east boundary */
		return (1);
	else
		return (0);
}

GMT_LONG GMT_map_jump_tm (double x0, double y0, double x1, double y1)
{
	/* TRUE if y-distance between points exceeds 1/2 map height at this x value */
	/* Only used for TM world maps */

	double dy;

	dy = y1 - y0;
	if (dy > GMT_half_map_height)	/* Cross bottom/south boundary */
		return (-1);
	else if (dy < (-GMT_half_map_height)) /* Cross top/north boundary */
		return (1);
	else
		return (0);
}

void GMT_get_crossings_x (double *xc, double *yc, double x0, double y0, double x1, double y1)
{	/* Finds crossings for wrap-arounds */
	double xa, xb, ya, yb, dxa, dxb, dyb, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (xa > xb) {	/* Make A the minimum x point */
		d_swap (xa, xb);
		d_swap (ya, yb);
	}

	xb -= 2.0 * GMT_half_map_width (yb);

	dxa = xa - GMT_left_boundary (ya);
	dxb = GMT_left_boundary (yb) - xb;
	c = (GMT_IS_ZERO (dxb)) ? 0.0 : 1.0 + dxa/dxb;
	dyb = (GMT_IS_ZERO (c)) ? 0.0 : fabs (yb - ya) / c;
	yc[0] = yc[1] = (ya > yb) ? yb + dyb : yb - dyb;
	xc[0] = GMT_left_boundary (yc[0]);
	xc[1] = GMT_right_boundary (yc[0]);
}

void GMT_get_crossings_tm (double *xc, double *yc, double x0, double y0, double x1, double y1)
{	/* Finds crossings for wrap-arounds for global TM maps */
	double xa, xb, ya, yb, dy, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (ya > yb) {	/* Make A the minimum y point */
		d_swap (xa, xb);
		d_swap (ya, yb);
	}

	yb -= GMT_map_height;

	dy = ya - yb;
	c = (GMT_IS_ZERO (dy)) ? 0.0 : (xa - xb) / dy;
	xc[0] = xc[1] = xb - yb * c;
	if (y0 > y1) {	/* First cut top */
		yc[0] = GMT_map_height;
		yc[1] = 0.0;
	}
	else {
		yc[0] = 0.0;
		yc[1] = GMT_map_height;
	}
}

double GMT_half_map_width (double y)
{	/* Returns 1/2-width of map in inches given y value */
	double half_width;

	switch (project_info.projection) {

		case GMT_STEREO:	/* Must compute width of circular map based on y value (ASSUMES FULL CIRCLE!!!) */
		case GMT_LAMB_AZ_EQ:
		case GMT_ORTHO:
		case GMT_GENPER:
		case GMT_GNOMONIC:
		case GMT_AZ_EQDIST:
		case GMT_VANGRINTEN:
			if (project_info.region && GMT_world_map) {
				y -= project_info.r;
				half_width = d_sqrt (project_info.r * project_info.r - y * y);
			}
			else
				half_width = GMT_half_map_size;
			break;

		case GMT_MOLLWEIDE:		/* Must compute width of Mollweide map based on y value */
		case GMT_HAMMER:
		case GMT_WINKEL:
		case GMT_SINUSOIDAL:
		case GMT_ROBINSON:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
			if (project_info.region && GMT_world_map)
				half_width = GMT_right_boundary (y) - GMT_half_map_size;
			else
				half_width = GMT_half_map_size;
			break;

		default:	/* Rectangular maps are easy */
			half_width = GMT_half_map_size;
			break;
	}
	return (half_width);
}

GMT_LONG GMT_set_greenwich (GMT_LONG mode)
{
	GMT_LONG greenwich = FALSE;

	/* Sets the GMT_LONG choice greenwich which should be FALSE
	 * for Cartesian coordinates and TRUE if we have data crossing
	 * Greenwich and it is not a global map
	 */

	if (mode) return (greenwich);	/* For testing; just return FALSE */

	if (GMT_io.in_col_type[0] & GMT_IS_LON) {	/* Lon/lat data */
		double lon;
		if (project_info.w <= 0.0 || project_info.e <= 0.0)	/* Region straddles Greenwich */
			greenwich = TRUE;
		else if (!GMT_world_map) {	/* A regional map - see if we are closer to Greenwich than Dateline */
			lon = project_info.central_meridian;
			if (lon < 0.0) lon += 360.0;
			greenwich = (lon < 90.0 || lon > 270.0);
		}
	}
	return (greenwich);
}

GMT_LONG GMT_will_it_wrap_x (double *x, double *y, GMT_LONG n, GMT_LONG *start)
{	/* Determines if a polygon will wrap at edges */
	GMT_LONG i;
	GMT_LONG wrap;
	double w_last, w_this;

	if (!GMT_world_map) return (FALSE);

	w_this = GMT_half_map_width (y[0]);
	for (i = 1, wrap = FALSE; !wrap && i < n; i++) {
		w_last = w_this;
		w_this = GMT_half_map_width (y[i]);
		wrap = GMT_this_point_wraps_x (x[i-1], x[i], w_last, w_this);
	}
	*start = i - 1;
	return (wrap);
}

GMT_LONG GMT_will_it_wrap_tm (double *x, double *y, GMT_LONG n, GMT_LONG *start)
{	/* Determines if a polygon will wrap at edges for TM global projection */
	GMT_LONG i;
	GMT_LONG wrap;

	if (!GMT_world_map) return (FALSE);

	for (i = 1, wrap = FALSE; !wrap && i < n; i++) {
		wrap = GMT_this_point_wraps_tm (y[i-1], y[i]);
	}
	*start = i - 1;
	return (wrap);
}

GMT_LONG GMT_this_point_wraps_x (double x0, double x1, double w_last, double w_this)
{
	/* Returns TRUE if the 2 x-points implies a jump at this y-level of the map */

	double w_min, w_max, dx;

	if (w_this > w_last) {
		w_max = w_this;
		w_min = w_last;
	}
	else {
		w_max = w_last;
		w_min = w_this;
	}

	/* Second condition deals with points near bottom/top of maps
	   where map width may shrink to zero */

	return ((dx = fabs (x1 - x0)) > w_max && w_min > GMT_SMALL);
}

GMT_LONG GMT_this_point_wraps_tm (double y0, double y1)
{
	/* Returns TRUE if the 2 y-points implies a jump at this x-level of the TM map */

	double dy;

	return ((dy = fabs (y1 - y0)) > GMT_half_map_height);
}

GMT_LONG GMT_truncate (double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG side)
{	/* Truncates a wrapping polygon against left or right edge.
	   (bottom or top edge when projection is TM)
	   x, y : arrays of plot coordinates
	   n    : length of the arrays
	   start: first point of array to consider
	   side : -1 = left (bottom); +1 = right (top)
	*/

	if (project_info.projection == GMT_TM)
		return (GMT_truncate_tm (x, y, n, start, side));
	else
		return (GMT_truncate_x (x, y, n, start, side));
}
		
GMT_LONG GMT_truncate_x (double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG l_or_r)
{	/* Truncates a wrapping polygon against left or right edge */

	GMT_LONG i, i1, j, k;
	double xc[4], yc[4], w_last, w_this;

	/* First initialize variables that differ for left and right truncation */

	if (l_or_r == -1)	/* Left truncation (-1) */
		/* Find first point that is left of map center */
		i = (x[start] < GMT_half_map_size) ? start : start - 1;
	else				/* Right truncation (+1) */
		/* Find first point that is right of map center */
		i = (x[start] > GMT_half_map_size) ? start : start - 1;

	if (!GMT_n_alloc) GMT_get_plot_array ();

	GMT_x_plot[0] = x[i];	GMT_y_plot[0] = y[i];
	w_this = GMT_half_map_width (y[i]);
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		w_last = w_this;
		w_this = GMT_half_map_width (y[i]);
		if (GMT_this_point_wraps_x (x[i1], x[i], w_last, w_this)) {
			(*GMT_get_crossings) (xc, yc, x[i1], y[i1], x[i], y[i]);
			if (l_or_r == -1)
				GMT_x_plot[j] = GMT_left_boundary (yc[0]);
			else
				GMT_x_plot[j] = GMT_right_boundary (yc[0]);
			GMT_y_plot[j] = yc[0];
			j++;
			if (j >= GMT_n_alloc) GMT_get_plot_array ();
		}
		if (l_or_r == -1) /* Left */
			GMT_x_plot[j] = (x[i] >= GMT_half_map_size) ? GMT_left_boundary (y[i]) : x[i];
		else	/* Right */
			GMT_x_plot[j] = (x[i] < GMT_half_map_size) ? GMT_right_boundary (y[i]) : x[i];
		GMT_y_plot[j] = y[i];
		j++, k++;
		if (j >= GMT_n_alloc) GMT_get_plot_array ();
	}
	return (j);
}

GMT_LONG GMT_truncate_tm (double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG b_or_t)
{	/* Truncates a wrapping polygon against bottom or top edge for global TM maps */

	GMT_LONG i, i1, j, k;
	double xc[4], yc[4], trunc_y;

	/* First initialize variables that differ for bottom and top truncation */

	if (b_or_t == -1) {	/* Bottom truncation (-1) */
		/* Find first point that is below map center */
		i = (y[start] < GMT_half_map_height) ? start : start - 1;
		trunc_y = 0.0;
	}
	else {				/* Top truncation (+1) */
		/* Find first point that is above map center */
		i = (y[start] > GMT_half_map_height) ? start : start - 1;
		trunc_y = GMT_map_height;
	}

	if (!GMT_n_alloc) GMT_get_plot_array ();

	GMT_x_plot[0] = x[i];	GMT_y_plot[0] = y[i];
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		if (GMT_this_point_wraps_tm (y[i1], y[i])) {
			GMT_get_crossings_tm (xc, yc, x[i1], y[i1], x[i], y[i]);
			GMT_x_plot[j] = xc[0];
			GMT_y_plot[j] = trunc_y;
			j++;
			if (j >= GMT_n_alloc) GMT_get_plot_array ();
		}
		if (b_or_t == -1) /* Bottom */
			GMT_y_plot[j] = (y[i] >= GMT_half_map_height) ? 0.0 : y[i];
		else	/* Top */
			GMT_y_plot[j] = (y[i] < GMT_half_map_height) ? GMT_map_height : y[i];
		GMT_x_plot[j] = x[i];
		j++, k++;
		if (j >= GMT_n_alloc) GMT_get_plot_array ();
	}
	return (j);
}

double GMT_great_circle_dist_old (double lon1, double lat1, double lon2, double lat2)
{
	/* great circle distance on a sphere in degrees */

	double cos_c;

	cos_c = GMT_great_circle_dist_cos (lon1, lat1, lon2, lat2);

	return (d_acosd (cos_c));
}

double GMT_great_circle_dist_cos (double lon1, double lat1, double lon2, double lat2)
{
	/* great circle distance on a sphere in cos (angle) */

	double cosa, cosb, sina, sinb;

	if (lat1 == lat2 && lon1 == lon2) return (1.0);

	sincosd (lat1, &sina, &cosa);
	sincosd (lat2, &sinb, &cosb);

	return (sina*sinb + cosa*cosb*cosd(lon1-lon2));
}

double GMT_great_circle_dist (double lon1, double lat1, double lon2, double lat2)
{
	/* Haversine formulate for great circle distance */

	double sx, sy, sc2;

	if (lat1 == lat2 && lon1 == lon2) return (0.0);

	sy = sind (0.5 * (lat2 - lat1));
	sx = sind (0.5 * (lon2 - lon1));
	sc2 = d_sqrt (sy * sy + cosd (lat2) * cosd (lat1) * sx * sx);

	return (2.0 * d_asind (sc2));
}

GMT_LONG GMT_great_circle_intersection (double A[], double B[], double C[], double X[], double *CX_dist)
{
	/* A, B, C are 3-D Cartesian unit vectors, i.e., points on the sphere.
	 * Let points A and B define a great circle, and consider a
	 * third point C.  A second great cirle goes through C and
	 * is orthogonal to the first great circle.  Their intersection
	 * X is the point on (A,B) closest to C.  We must test if X is
	 * between A,B or outside.
	 */
	GMT_LONG i;
	double P[3], E[3], M[3], Xneg[3], cos_AB, cos_MX1, cos_MX2, cos_test;

	GMT_cross3v (A, B, P);			/* Get pole position of plane through A and B (and origin O) */
	GMT_normalize3v (P);			/* Make sure P has unit length */
	GMT_cross3v (C, P, E);			/* Get pole E to plane through C (and origin) but normal to A,B (hence going through P) */
	GMT_normalize3v (E);			/* Make sure E has unit length */
	GMT_cross3v (P, E, X);			/* Intersection between the two planes is oriented line*/
	GMT_normalize3v (X);			/* Make sure X has unit length */
	/* The X we want could be +x or -X; must determine which might be closest to A-B midpoint M */
	for (i = 0; i < 3; i++) {
		M[i] = A[i] + B[i];
		Xneg[i] = -X[i];
	}
	GMT_normalize3v (M);			/* Make sure M has unit length */
	/* Must first check if X is along the (A,B) segment and not on its extension */

	cos_MX1 = GMT_dot3v (M, X);		/* Cos of spherical distance between M and +X */
	cos_MX2 = GMT_dot3v (M, Xneg);		/* Cos of spherical distance between M and -X */
	if (cos_MX2 > cos_MX1) memcpy ((void *)X, (void *)Xneg, 3*sizeof(double));;		/* -X is closest to A-B midpoint */
	cos_AB = fabs (GMT_dot3v (A, B));	/* Cos of spherical distance between A,B */
	cos_test = fabs (GMT_dot3v (A, X));	/* Cos of spherical distance between A and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to A exceeds the A-B length */
	cos_test = fabs (GMT_dot3v (B, X));	/* Cos of spherical distance between B and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to B exceeds the A-B length */

	/* X is between A and B.  Now calculate distance between C and X */

	*CX_dist = GMT_dot3v (C, X);		/* Cos of spherical distance between C and X */
	return (0);				/* Return zero if intersection is between A and B */
}

/* The *_outside routines return the status of the current point.
 * Status is the sum of x_status and y_status.
 *	x_status may be
 *	0	w < lon < e
 *	-1	lon == w
 *	1	lon == e
 *	-2	lon < w
 *	2	lon > e
 *	y_status may be
 *	0	s < lat < n
 *	-1	lat == s
 *	1	lat == n
 *	-2	lat < s
 *	2	lat > n
 */

GMT_LONG GMT_wesn_outside (double lon, double lat)
{
	/* This version ensures that any point will be considered inside if
	 * it is off by a multiple of 360 degrees in longitude. 
	 * This wrapping can be switched off by setting GMT_lon_wrap = FALSE.
	 */

	if (GMT_lon_wrap) {
		while (lon < project_info.w && (lon + 360.0) <= project_info.e) lon += 360.0;
		while (lon > project_info.e && (lon - 360.0) >= project_info.w) lon -= 360.0;
	}

	if (GMT_on_border_is_outside && fabs (lon - project_info.w) < GMT_SMALL)
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lon - project_info.e) < GMT_SMALL)
		GMT_x_status_new = 1;
	else if (lon < project_info.w)
		GMT_x_status_new = -2;
	else if (lon > project_info.e)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;

	if (GMT_on_border_is_outside && fabs (lat - project_info.s) < GMT_SMALL)
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lat - project_info.n) < GMT_SMALL)
		GMT_y_status_new = 1;
	else if (lat < project_info.s)
		GMT_y_status_new = -2;
	else if (lat > project_info.n)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;

	return (GMT_x_status_new != 0 || GMT_y_status_new != 0);
}

GMT_LONG GMT_polar_outside (double lon, double lat)
{
	GMT_wesn_outside (lon, lat);

	if (!project_info.edge[1]) GMT_x_status_new = 0;	/* 360 degrees, no edge */
	if (GMT_y_status_new < 0 && !project_info.edge[0]) GMT_y_status_new = 0;	/* South pole enclosed */
	if (GMT_y_status_new > 0 && !project_info.edge[2]) GMT_y_status_new = 0;	/* North pole enclosed */

	return (GMT_x_status_new != 0 || GMT_y_status_new != 0);
}

GMT_LONG GMT_eqdist_outside (double lon, double lat)
{
	double cc, s, c;

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	sincosd (lat, &s, &c);
	cc = project_info.sinp * s + project_info.cosp * c * cosd (lon);
	if (cc < -1.0) {
		GMT_y_status_new = -1;
		GMT_x_status_new = 0;
	}
	else
		GMT_x_status_new = GMT_y_status_new = 0;
	return (GMT_y_status_new != 0);
}

GMT_LONG GMT_radial_outside (double lon, double lat)
{
	double dist;

	/* Test if point is more than horizon spherical degrees from origin.  For global maps, let all borders be "south" */

	GMT_x_status_new = 0;
	dist = GMT_great_circle_dist (lon, lat, project_info.central_meridian, project_info.pole);
	if (GMT_on_border_is_outside && fabs (dist - project_info.f_horizon) < GMT_SMALL)
		GMT_y_status_new = -1;
	else if (dist > project_info.f_horizon)
		GMT_y_status_new = -2;
	else
		GMT_y_status_new = 0;
	return (GMT_y_status_new != 0);
}

GMT_LONG GMT_rect_outside (double lon, double lat)
{
	double x, y;

	GMT_geo_to_xy (lon, lat, &x, &y);

	if (GMT_on_border_is_outside && fabs (x - project_info.xmin) < GMT_SMALL)
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (x - project_info.xmax) < GMT_SMALL)
		GMT_x_status_new = 1;
	else if (x < project_info.xmin)
		GMT_x_status_new = -2;
	else if (x > project_info.xmax)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;

	if (GMT_on_border_is_outside && fabs (y -project_info.ymin) < GMT_SMALL)
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (y - project_info.ymax) < GMT_SMALL)
		GMT_y_status_new = 1;
	else if (y < project_info.ymin)
		GMT_y_status_new = -2;
	else if (y > project_info.ymax)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;

	return (GMT_x_status_new != 0 || GMT_y_status_new != 0);
}

GMT_LONG GMT_rect_outside2 (double lon, double lat)
{	/* For Azimuthal proj with rect borders since GMT_rect_outside may fail for antipodal points */
	if (GMT_radial_outside (lon, lat)) return (TRUE);	/* Point > 90 degrees away */
	return (GMT_rect_outside (lon, lat));	/* Must check if inside box */
}

GMT_LONG GMT_wesn_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Compute the crossover point(s) on the map boundary for rectangular projections.
	 * Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	GMT_LONG n = 0, i;
	double d, dlon0, dlat0, x0, y0;

	/* If wrapping is allowed: first bring both points between W and E boundaries,
	 * then move the western-most point east if it is further than 180 degrees away.
 	 * This may cause the points to span the eastern boundary */

	if (GMT_lon_wrap) {
		while (lon0 < project_info.w) lon0 += 360.0;
		while (lon0 > project_info.e) lon0 -= 360.0;
		while (lon1 < project_info.w) lon1 += 360.0;
		while (lon1 > project_info.e) lon1 -= 360.0;
		if (fabs (lon0 - lon1) <= 180.0) { /* Nothing */ }
		else if (lon0 < lon1)
			lon0 += 360.0;
		else
			lon1 += 360.0;
	}

	dlon0 = lon0 - lon1;
	dlat0 = lat0 - lat1;

	/* Then set 'almost'-corners to corners */
	GMT_x_wesn_corner (&lon0);
	GMT_x_wesn_corner (&lon1);
	GMT_y_wesn_corner (&lat0);
	GMT_y_wesn_corner (&lat1);

	/* Crossing South */
	if ((lat0 >= project_info.s && lat1 <= project_info.s) || (lat1 >= project_info.s && lat0 <= project_info.s)) {
		sides[n] = 0;
		clat[n] = project_info.s;
		d = lat0 - lat1;
		clon[n] = (GMT_IS_ZERO (d)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		GMT_x_wesn_corner (&clon[n]);
		if (fabs(d) > 0.0 && GMT_lon_inside (clon[n], project_info.w, project_info.e)) n++;
	}
	/* Crossing East */
	if ((lon0 >= project_info.e && lon1 <= project_info.e) || (lon1 >= project_info.e && lon0 <= project_info.e)) {
		sides[n] = 1;
		clon[n] = project_info.e;
		d = lon0 - lon1;
		clat[n] = (GMT_IS_ZERO (d)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		GMT_y_wesn_corner (&clat[n]);
		if (fabs(d) > 0.0 && clat[n] >= project_info.s && clat[n] <= project_info.n) n++;
	}

	/* Now adjust the longitudes so that they might span the western boundary */
	if (GMT_lon_wrap && MAX(lon0, lon1) > project_info.e) {
		lon0 -= 360.0; lon1 -= 360.0;
		GMT_x_wesn_corner (&lon0);
		GMT_x_wesn_corner (&lon1);
	}

	/* Crossing North */
	if ((lat0 >= project_info.n && lat1 <= project_info.n) || (lat1 >= project_info.n && lat0 <= project_info.n)) {
		sides[n] = 2;
		clat[n] = project_info.n;
		d = lat0 - lat1;
		clon[n] = (GMT_IS_ZERO (d)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		GMT_x_wesn_corner (&clon[n]);
		if (fabs(d) > 0.0 && GMT_lon_inside (clon[n], project_info.w, project_info.e)) n++;
	}
	/* Crossing West */
	if ((lon0 >= project_info.w && lon1 <= project_info.w) || (lon1 >= project_info.w && lon0 <= project_info.w)) {
		sides[n] = 3;
		clon[n] = project_info.w;
		d = lon0 - lon1;
		clat[n] = (GMT_IS_ZERO (d)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		GMT_y_wesn_corner (&clat[n]);
		if (fabs(d) > 0.0 && clat[n] >= project_info.s && clat[n] <= project_info.n) n++;
	}

	if (n == 0) return (0);

	for (i = 0; i < n; i++) {
		GMT_geo_to_xy (clon[i], clat[i], &xx[i], &yy[i]);
		if (project_info.projection == GMT_POLAR && sides[i]%2) sides[i] = 4 - sides[i];	/*  toggle 1 <-> 3 */
	}

	if (n == 1) return (1);

	/* Check for corner xover */

	if (GMT_is_wesn_corner (clon[0], clat[0])) return (1);

	if (GMT_is_wesn_corner (clon[1], clat[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	/* Sort the two intermediate points into the right order based on projected distances from the first point */

	GMT_geo_to_xy (lon0, lat0, &x0, &y0);

	if (hypot (x0 - xx[1], y0 - yy[1]) < hypot (x0 - xx[0], y0 - yy[0])) {
		d_swap (clon[0], clon[1]);
		d_swap (clat[0], clat[1]);
		d_swap (xx[0], xx[1]);
		d_swap (yy[0], yy[1]);
		l_swap (sides[0], sides[1]);
	}

	return (2);
}

GMT_LONG GMT_rect_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Compute the crossover point(s) on the map boundary for rectangular projections
	 * Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	GMT_LONG i, j, n = 0;
	double x0, x1, y0, y1, d, dx, dy;

	GMT_geo_to_xy (lon0, lat0, &x0, &y0);
	GMT_geo_to_xy (lon1, lat1, &x1, &y1);

	/* First set 'almost'-corners to corners */

	dx = x0 - x1;	/* Pre-adjust increment in x */
	dy = y0 - y1;	/* Pre-adjust increment in y */
	GMT_x_rect_corner (&x0);
	GMT_x_rect_corner (&x1);
	GMT_y_rect_corner (&y0);
	GMT_y_rect_corner (&y1);

	if ((y0 >= project_info.ymin && y1 <= project_info.ymin) || (y1 >= project_info.ymin && y0 <= project_info.ymin)) {
		sides[n] = 0;
		yy[n] = project_info.ymin;
		d = y0 - y1;
		xx[n] = (GMT_IS_ZERO (d)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (&xx[n]);
		if (fabs(d) > 0.0 && xx[n] >= project_info.xmin && xx[n] <= project_info.xmax) n++;
	}
	if ((x0 <= project_info.xmax && x1 >= project_info.xmax) || (x1 <= project_info.xmax && x0 >= project_info.xmax)) {
		sides[n] = 1;
		xx[n] = project_info.xmax;
		d = x0 - x1;
		yy[n] = (GMT_IS_ZERO (d)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (&yy[n]);
		if (fabs(d) > 0.0 && yy[n] >= project_info.ymin && yy[n] <= project_info.ymax) n++;
	}
	if ((y0 <= project_info.ymax && y1 >= project_info.ymax) || (y1 <= project_info.ymax && y0 >= project_info.ymax)) {
		sides[n] = 2;
		yy[n] = project_info.ymax;
		d = y0 - y1;
		xx[n] = (GMT_IS_ZERO (d)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (&xx[n]);
		if (fabs(d) > 0.0 && xx[n] >= project_info.xmin && xx[n] <= project_info.xmax) n++;
	}
	if ((x0 >= project_info.xmin && x1 <= project_info.xmin) || (x1 >= project_info.xmin && x0 <= project_info.xmin)) {
		sides[n] = 3;
		xx[n] = project_info.xmin;
		d = x0 - x1;
		yy[n] = (GMT_IS_ZERO (dx)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (&yy[n]);
		if (fabs(d) > 0.0 && yy[n] >= project_info.ymin && yy[n] <= project_info.ymax) n++;
	}

	if (n == 0) return (0);

	/* Eliminate duplicates */

	for (i = 0; i < n; i++) {
		for (j = i + 1; j < n; j++) {
			if (GMT_IS_ZERO (xx[i] - xx[j]) && GMT_IS_ZERO (yy[i] - yy[j]))	/* Duplicate */
				sides[j] = -9;	/* Mark as duplicate */
		}
	}
	for (i = 1; i < n; i++) {
		if (sides[i] == -9) {	/* This is a duplicate, overwrite */
			for (j = i + 1; j < n; j++) {
				xx[j-1] = xx[j];
				yy[j-1] = yy[j];
				sides[j-1] = sides[j];
			}
			n--;
			i--;	/* Must start at same point again */
		}
	}

	for (i = 0; i < n; i++)	GMT_xy_to_geo (&clon[i], &clat[i], xx[i], yy[i]);

	if (!GMT_IS_MAPPING) return (n);

	if (n == 1) return (1);

	/* Check for corner xover */

	if (GMT_is_rect_corner (xx[0], yy[0])) return (1);

	if (GMT_is_rect_corner (xx[1], yy[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	/* Sort the two intermediate points into the right order based on projected distances from the first point */

	if (hypot (x0 - xx[1], y0 - yy[1]) < hypot (x0 - xx[0], y0 - yy[0])) {
		d_swap (clon[0], clon[1]);
		d_swap (clat[0], clat[1]);
		d_swap (xx[0], xx[1]);
		d_swap (yy[0], yy[1]);
		l_swap (sides[0], sides[1]);
	}

	return (2);
}

void GMT_x_rect_corner (double *x)
{
	if (fabs (*x) <= GMT_SMALL)
		*x = 0.0;
	else if (fabs (*x - project_info.xmax) <= GMT_SMALL)
		*x = project_info.xmax;
}

void GMT_y_rect_corner (double *y)
{
	if (fabs (*y) <= GMT_SMALL)
		*y = 0.0;
	else if (fabs (*y - project_info.ymax) <= GMT_SMALL)
		*y = project_info.ymax;
}

GMT_LONG GMT_is_rect_corner (double x, double y)
{	/* Checks if point is a corner */
	GMT_corner = -1;
	if (GMT_IS_ZERO (x - project_info.xmin)) {
		if (GMT_IS_ZERO (y - project_info.ymin))
			GMT_corner = 1;
		else if (GMT_IS_ZERO (y - project_info.ymax))
			GMT_corner = 4;
	}
	else if (GMT_IS_ZERO (x - project_info.xmax)) {
		if (GMT_IS_ZERO (y - project_info.ymin))
			GMT_corner = 2;
		else if (GMT_IS_ZERO (y - project_info.ymax))
			GMT_corner = 3;
	}
	return (GMT_corner > 0);
}

void GMT_x_wesn_corner (double *x)
{
/*	if (fabs (fmod (fabs (*x - project_info.w), 360.0)) <= GMT_SMALL)
		*x = project_info.w;
	else if (fabs (fmod (fabs (*x - project_info.e), 360.0)) <= GMT_SMALL)
		*x = project_info.e; */

	if (fabs (*x - project_info.w) <= GMT_SMALL)
		*x = project_info.w;
	else if (fabs (*x - project_info.e) <= GMT_SMALL)
		*x = project_info.e;

}

void GMT_y_wesn_corner (double *y)
{
	if (fabs (*y - project_info.s) <= GMT_SMALL)
		*y = project_info.s;
	else if (fabs (*y - project_info.n) <= GMT_SMALL)
		*y = project_info.n;
}

GMT_LONG GMT_is_wesn_corner (double x, double y)
{	/* Checks if point is a corner */
	GMT_corner = 0;

	if (GMT_IS_ZERO (fmod (fabs (x - project_info.w), 360.0))) {
		if (GMT_IS_ZERO (y - project_info.s))
			GMT_corner = 1;
		else if (GMT_IS_ZERO (y - project_info.n))
			GMT_corner = 4;
	}
	else if (GMT_IS_ZERO (fmod (fabs (x - project_info.e), 360.0))) {
		if (GMT_IS_ZERO (y - project_info.s))
			GMT_corner = 2;
		else if (GMT_IS_ZERO (y - project_info.n))
			GMT_corner = 3;
	}
	return (GMT_corner > 0);
}

GMT_LONG GMT_radial_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Computes the lon/lat of a point that is f_horizon spherical degrees from
	 * the origin and lies on the great circle between points 1 and 2 */

	double dist1, dist2, delta, eps, dlon;

	dist1 = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon1, lat1);
	dist2 = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon2, lat2);
	delta = dist2 - dist1;
	eps = (GMT_IS_ZERO (delta)) ? 0.0 : (project_info.f_horizon - dist1) / delta;
	dlon = lon2 - lon1;
	if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
	clon[0] = lon1 + dlon * eps;
	clat[0] = lat1 + (lat2 - lat1) * eps;

	GMT_geo_to_xy (clon[0], clat[0], &xx[0], &yy[0]);

	sides[0] = 1;

	return (1);
}

GMT_LONG GMT_ellipse_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Compute the crossover point(s) on the map boundary for rectangular projections */
	GMT_LONG n = 0, i, jump;
	double x1, x2, y1, y2;

	/* Crossings here must be at the W or E borders. Lat points may only touch border */

	if (lat1 <= -90.0) {
		sides[n] = 0;
		clon[n] = lon1;
		clat[n] = lat1;
		n = 1;
	}
	else if (lat2 <= -90.0) {
		sides[n] = 0;
		clon[n] = lon2;
		clat[n] = lat2;
		n = 1;
	}
	else if (lat1 >= 90.0) {
		sides[n] = 2;
		clon[n] = lon1;
		clat[n] = lat1;
		n = 1;
	}
	else if (lat2 >= 90.0) {
		sides[n] = 2;
		clon[n] = lon2;
		clat[n] = lat2;
		n = 1;
	}
	else {	/* May cross somewhere else */
		GMT_geo_to_xy (lon1, lat1, &x1, &y1);
		GMT_geo_to_xy (lon2, lat2, &x2, &y2);
		if ((jump = GMT_map_jump_x (x2, y2, x1, y1))) {
			(*GMT_get_crossings) (xx, yy, x2, y2, x1, y1);
			if (jump == 1) {	/* Add right border point first */
				d_swap (xx[0], xx[1]);
				d_swap (yy[0], yy[1]);
			}
			GMT_xy_to_geo (&clon[0], &clat[0], xx[0], yy[0]);
			GMT_xy_to_geo (&clon[1], &clat[1], xx[1], yy[1]);
		}
		n = 2;	/* To signal don't change order */
	}
	if (n == 1) for (i = 0; i < n; i++) GMT_geo_to_xy (clon[i], clat[i], &xx[i], &yy[i]);
	return (n);
}

GMT_LONG GMT_eqdist_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	double angle, x, y, s, c;

	/* Computes the x.y of the antipole point that lies on a radius from
	 * the origin through the inside point */

	if (GMT_eqdist_outside (lon1, lat1)) {	/* Point 1 is on perimeter */
		GMT_geo_to_xy (lon2, lat2, &x, &y);
		angle = d_atan2 (y - project_info.y0, x - project_info.x0);
		sincos (angle, &s, &c);
		xx[0] = project_info.r * c + project_info.x0;
		yy[0] = project_info.r * s + project_info.y0;
		clon[0] = lon1;
		clat[0] = lat1;
	}
	else {	/* Point 2 is on perimeter */
		GMT_geo_to_xy (lon1, lat1, &x, &y);
		angle = d_atan2 (y - project_info.y0, x - project_info.x0);
		sincos (angle, &s, &c);
		xx[0] = project_info.r * c + project_info.x0;
		yy[0] = project_info.r * s + project_info.y0;
		clon[0] = lon2;
		clat[0] = lat2;
	}
	sides[0] = 1;

	return (1);
}

GMT_LONG *GMT_split_line (double **xx, double **yy, GMT_LONG *nn, GMT_LONG add_crossings)
{	/* Accepts x/y array for a line in projected inches and looks for
	 * map jumps.  If found it will insert the boundary crossing points and
	 * build a split integer array with the nodes of the first point
	 * for each segment.  The number of segments is returned.  If
	 * no jumps are found then NULL is returned.  This function is needed
	 * so that the new PS contouring machinery only sees lines that do no
	 * jump across the map.
	 * add_crossings is TRUE if we need to find the crossings; FALSE means
	 * they are already part of the line. */

	double *x, *y, *xin, *yin, xc[2], yc[2];
	GMT_LONG i, j, k, n, n_seg, *split, *pos, *way;
	GMT_LONG l_or_r, n_alloc = 0;

	/* First quick scan to see how many jumps there are */

	xin = *xx;	yin = *yy;
	GMT_set_meminc (GMT_SMALL_CHUNK);
	for (n_seg = 0, i = 1; i < *nn; i++) {
		if ((l_or_r = GMT_map_jump_x (xin[i], yin[i], xin[i-1], yin[i-1]))) {
			if (n_seg == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&pos, (void **)&way, n_seg, n_alloc, sizeof (GMT_LONG), GMT_program);
			pos[n_seg] = i;		/* 2nd of the two points that generate the jump */
			way[n_seg] = (short int)l_or_r;		/* Which way we jump : +1 is right to left, -1 is left to right */
			n_seg++;
		}
	}
	GMT_reset_meminc ();

	if (n_seg == 0) return ((GMT_LONG *)NULL);	/* No jumps, just return NULL */

	/* Here we have one or more jumps so we need to split the line */

	n = *nn;				/* Original line count */
	if (add_crossings) n += 2 * n_seg;	/* Must add 2 crossing points per jump */
	(void)GMT_alloc_memory2 ((void **)&x, (void **)&y, n, 0, sizeof (double), GMT_program);
	split = (GMT_LONG *) GMT_memory (VNULL, (n_seg+2), sizeof (GMT_LONG), GMT_program);
	split[0] = n_seg;

	x[0] = xin[0];
	y[0] = yin[0];
	for (i = j = 1, k = 0; i < *nn; i++, j++) {
		if (k < n_seg && i == pos[k]) {	/* At jump point */
			if (add_crossings) {	/* Find and insert the crossings */
				GMT_get_crossings_x (xc, yc, xin[i], yin[i], xin[i-1], yin[i-1]);
				if (way[k] == 1) {	/* Add right border point first */
					d_swap (xc[0], xc[1]);
					d_swap (yc[0], yc[1]);
				}
				x[j] = xc[0];	y[j++] = yc[0];	/* End of one segment */
				x[j] = xc[1];	y[j++] = yc[1];	/* Start of another */
			}
			split[++k] = j;		/* Node of first point in new segment */
		}
		/* Then copy the regular points */
		x[j] = xin[i];
		y[j] = yin[i];
	}
	split[++k] = j;		/* End of last segment */

	/* Time to return the pointers to new data */

	GMT_free ((void *)pos);
	GMT_free ((void *)way);
	GMT_free ((void *)xin);
	GMT_free ((void *)yin);
	*xx = x;
	*yy = y;
	*nn = j;

	return (split);
}

/*  Routines to add pieces of parallels or meridians */

GMT_LONG GMT_graticule_path (double **x, double **y, GMT_LONG dir, double w, double e, double s, double n)
{	/* Returns the path of a graticule (box of meridians and parallels) */
	GMT_LONG np;
	double *xx, *yy;
	double px0, px1, px2, px3;

	if (dir == 1) {	/* Forward sense */
		px0 = px3 = w;	px1 = px2 = e;
	}
	else {	/* Reverse sense */
		px0 = px3 = e;	px1 = px2 = w;
	}

	/* Close graticule from point 0 through point 4 */

	if (GMT_IS_RECT_GRATICULE) {	/* Simple rectangle in this projection */
		np = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, 5, 0, sizeof (double), GMT_program);
		xx[0] = xx[4] = px0;	xx[1] = px1;	xx[2] = px2;	xx[3] = px3;
		yy[0] = yy[1] = yy[4] = s;	yy[2] = yy[3] = n;
	}
	else {	/* Must assemble path from meridians and parallel pieces */
		double *xtmp, *ytmp;
		GMT_LONG add, n_alloc = 0;

		/* SOUTH BORDER */

		if (GMT_IS_MAPPING && s == -90.0) {	/* No path, just a point */
			np = n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, 1, 0, sizeof (double), GMT_program);
			xx[0] = px1;	yy[0] = -90.0;
		}
		else
			np = n_alloc = GMT_latpath (s, px0, px1, &xx, &yy);	/* South */

		/* EAST (OR WEST) BORDER */

		add = GMT_lonpath (px1, s, n, &xtmp, &ytmp);	/* east (or west if dir == -1) */
		n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n_alloc + add, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);

		/* NORTH BORDER */

		if (GMT_IS_MAPPING && n == 90.0) {	/* No path, just a point */
			add = GMT_alloc_memory2 ((void **)&xtmp, (void **)&ytmp, 1, 0, sizeof (double), GMT_program);
			xtmp[0] = px3;	ytmp[0] = +90.0;
		}
		else
			add = GMT_latpath (n, px2, px3, &xtmp, &ytmp);	/* North */

		n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n_alloc + add, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);

		/* WEST (OR EAST) BORDER */

		add = GMT_lonpath (px3, n, s, &xtmp, &ytmp);	/* west */
		n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n_alloc + add, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);
		(void)GMT_alloc_memory2 ((void **)&xx, (void **)&yy, 0, np, sizeof (double), GMT_program);
	}

	if (GMT_io.in_col_type[0] == GMT_IS_LON) {
		GMT_LONG straddle;
		GMT_LONG i;
		straddle = (project_info.w < 0.0 && project_info.e > 0.0);
		for (i = 0; straddle && i < np; i++) {
			while (xx[i] < 0.0) xx[i] += 360.0;
			if (straddle && xx[i] > 180.0) xx[i] -= 360.0;
		}
	}

	*x = xx;
	*y = yy;
	return (np);
}

GMT_LONG GMT_map_path (double lon1, double lat1, double lon2, double lat2, double **x, double **y)
{
	if (fabs (lat1 - lat2) < 1.0e-10)
		return (GMT_latpath (lat1, lon1, lon2, x, y));
	else
		return (GMT_lonpath (lon1, lat1, lat2, x, y));
}

GMT_LONG GMT_lonpath (double lon, double lat1, double lat2, double **x, double **y)
{
	GMT_LONG ny, n, n_try, keep_trying, pos;
	double dlat, dlat0, *tlon, *tlat, x0, x1, y0, y1, d;
	double min_gap;

	if (GMT_meridian_straight == 2) {	/* For gmtselect/grdlandmask: Straight line with no intermediate points */
		n = 2;
		n = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, n, 0, sizeof (double), GMT_program);

		tlon[0] = tlon[1] = lon;
		tlat[0] = lat1;	tlat[1] = lat2;
		*x = tlon;
		*y = tlat;
		return (n);
	}

	if (GMT_meridian_straight) {	/* Easy, just a straight line connect via quarter-points */
		n = 5;
		n = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, n, 0, sizeof (double), GMT_program);

		tlon[0] = tlon[1] = tlon[2] = tlon[3] = tlon[4] = lon;
		dlat = lat2 - lat1;
		tlat[0] = lat1;	tlat[1] = lat1 + 0.25 * dlat;	tlat[2] = lat1 + 0.5 * dlat;
		tlat[3] = lat1 + 0.75 * dlat;	tlat[4] = lat2;
		*x = tlon;
		*y = tlat;
		return (n);
	}

	n = 0;
	min_gap = 0.1 * gmtdefs.line_step;
	if ((ny = (GMT_LONG)ceil (fabs (lat2 - lat1) / GMT_dlat)) == 0) return (0);

	ny++;
	dlat0 = (lat2 - lat1) / ny;
	pos = (dlat0 > 0.0);

	tlon = (double *) GMT_memory (VNULL, (size_t)ny, sizeof (double), "GMT_lonpath");
	tlat = (double *) GMT_memory (VNULL, (size_t)ny, sizeof (double), "GMT_lonpath");

	tlon[0] = lon;
	tlat[0] = lat1;
	GMT_geo_to_xy (tlon[0], tlat[0], &x0, &y0);
	while ((pos && (tlat[n] < lat2)) || (!pos && (tlat[n] > lat2))) {
		n++;
		if (n == ny-1) {
			ny += GMT_SMALL_CHUNK;
			tlon = (double *) GMT_memory ((void *)tlon, (size_t)ny, sizeof (double), "GMT_lonpath");
			tlat = (double *) GMT_memory ((void *)tlat, (size_t)ny, sizeof (double), "GMT_lonpath");
		}
		n_try = 0;
		keep_trying = TRUE;
		dlat = dlat0;
		tlon[n] = lon;
		do {
			n_try++;
			tlat[n] = tlat[n-1] + dlat;
			if (GMT_IS_MAPPING && fabs (tlat[n]) > 90.0) tlat[n] = copysign (90.0, tlat[n]);
			GMT_geo_to_xy (tlon[n], tlat[n], &x1, &y1);
			if ((*GMT_map_jump) (x0, y0, x1, y1) || (y0 < project_info.ymin || y0 > project_info.ymax))
				keep_trying = FALSE;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > gmtdefs.line_step)
					dlat *= 0.5;
				else if (d < min_gap)
					dlat *= 2.0;
				else
					keep_trying = FALSE;
			}
		} while (keep_trying && n_try < 10);
		x0 = x1;	y0 = y1;
	}
	tlon[n] = lon;
	tlat[n] = lat2;
	n++;

	if (n != ny) {
		tlon = (double *) GMT_memory ((void *)tlon, (size_t)n, sizeof (double), "GMT_lonpath");
		tlat = (double *) GMT_memory ((void *)tlat, (size_t)n, sizeof (double), "GMT_lonpath");
	}

	*x = tlon;	*y = tlat;
	return (n);
}

GMT_LONG GMT_latpath (double lat, double lon1, double lon2, double **x, double **y)
{
	GMT_LONG n_alloc, n, n_try, keep_trying, pos;
	double dlon, dlon0, *tlon, *tlat, x0, x1, y0, y1, d;
	double min_gap;

	if (GMT_parallel_straight == 2) {	/* For gmtselect/grdlandmask: Straight line with no intermediate points */
		n = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, 2, 0, sizeof (double), GMT_program);
		tlat[0] = tlat[1] = lat;
		tlon[0] = lon1;	tlon[1] = lon2;
		*x = tlon;	*y = tlat;
		return (n);
	}
	
	if (GMT_parallel_straight) {	/* Easy, just a straight line connection via quarter points */
		n = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, 5, 0, sizeof (double), GMT_program);
		tlat[0] = tlat[1] = tlat[2] = tlat[3] = tlat[4] = lat;
		dlon = lon2 - lon1;
		tlon[0] = lon1;	tlon[1] = lon1 + 0.25 * dlon;	tlon[2] = lon1 + 0.5 * dlon;
		tlon[3] = lon1 + 0.75 * dlon;	tlon[4] = lon2;
		*x = tlon;	*y = tlat;
		return (n);
	}
	/* Here we try to walk along lat for small increment in longitude to make sure our steps are smaller than the line_step */
	min_gap = 0.1 * gmtdefs.line_step;
	if ((n_alloc = (GMT_LONG)ceil (fabs (lon2 - lon1) / GMT_dlon)) == 0) return (0);	/* Initial guess to path length */

	n_alloc++;
	dlon0 = (lon2 - lon1) / n_alloc;
	pos = (dlon0 > 0.0);

	n_alloc = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, n_alloc, 0, sizeof (double), GMT_program);
	tlon[0] = lon1;	tlat[0] = lat;
	GMT_geo_to_xy (tlon[0], tlat[0], &x0, &y0);
	n = 0;
	while ((pos && (tlon[n] < lon2)) || (!pos && (tlon[n] > lon2))) {
		n++;
		if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, n, n_alloc, sizeof (double), GMT_program);
		n_try = 0;
		keep_trying = TRUE;
		dlon = dlon0;
		tlat[n] = lat;
		do {
			n_try++;
			tlon[n] = tlon[n-1] + dlon;
			GMT_geo_to_xy (tlon[n], tlat[n], &x1, &y1);
			if ((*GMT_map_jump) (x0, y0, x1, y1) || (y0 < project_info.ymin || y0 > project_info.ymax))
				keep_trying = FALSE;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > gmtdefs.line_step)
					dlon *= 0.5;
				else if (d < min_gap)
					dlon *= 2.0;
				else
					keep_trying = FALSE;
			}
		} while (keep_trying && n_try < 10);
		x0 = x1;	y0 = y1;
	}
	tlon[n] = lon2;
	tlat[n] = lat;
	n++;
	(void)GMT_alloc_memory2 ((void **)&tlon, (void **)&tlat, 0, n, sizeof (double), GMT_program);

	*x = tlon;	*y = tlat;
	return (n);
}

GMT_LONG GMT_rhumbline (double lon1, double lat1, double lon2, double lat2, double **x, double **y)
{
	/* Creates a path that follows the rhumbline between the two points [UNTESTED] */

	GMT_LONG i, n;
	double x1, x2, y1, y2, dx, dy, L, dL, f, *xx, *yy;

	GMT_vmerc (0.0, 0.0);
	GMT_merc_sph (lon1, lat1, &x1, &y1);	/* Mercator coordinates of end points in projected meters */
	GMT_merc_sph (lon2, lat2, &x2, &y2);
	dx = x2 - x1;
	dy = y2 - y1;
	L = hypot (dx, dy);
	n = irint (L * GMT_u2u[GMT_M][GMT_INCH]/ gmtdefs.line_step);
	dL = L / n;
	(void)GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n + 1, 0, sizeof (double), GMT_program);
	for (i = 0; i <= n; i++) {
		f = (double)i / (double)n;
		GMT_imerc_sph (&xx[i], &yy[i], x1 + dx * f, y1 + dy * f);
	}
	*x = xx;
	*y = yy;

	return (n+1);
}


/*  Routines to do with clipping */

GMT_LONG GMT_clip_to_map (double *lon, double *lat, GMT_LONG np, double **x, double **y)
{
	/* This routine makes sure that all points are either inside or on the map boundary
	 * and returns the number of points to be used for plotting (in x,y units) */

	GMT_LONG i, n, out, out_x, out_y, np2, total_nx = 0;
	GMT_LONG polygon;
	double *xx, *yy;

	/* First check for trivial cases:  All points outside or all points inside */

	for (i = out = out_x = out_y = 0; i < np; i++)  {
		(void) GMT_map_outside (lon[i], lat[i]);
		out_x += GMT_x_status_new;	/* Completely left of west gives -2 * np, right of east gives + 2 * np */
		out_y += GMT_y_status_new;	/* Completely below south gives -2 * np, above north gives + 2 * np */
		out += (GMT_abs (GMT_x_status_new) == 2 || GMT_abs (GMT_y_status_new) == 2);
	}
	if (out == 0) {		/* All points are inside map boundary; no clipping required */
		(void)GMT_alloc_memory2 ((void **)&xx, (void **)&yy, np, 0, sizeof (double), "GMT_clip_to_map");
		for (i = 0; i < np; i++) GMT_geo_to_xy (lon[i], lat[i], &xx[i], &yy[i]);
		*x = xx;	*y = yy;	n = np;
	}
	else if (out == np) {	/* All points are outside map boundary */
		np2 = 2 * np;
		if (GMT_abs (out_x) == np2 || GMT_abs (out_y) == np2)	/* All points safely outside the region, no part of polygon survives */
			n = 0;
		else {	/* All points are outside, but they are not just to one side so lines _may_ intersect the region */
			n = (*GMT_map_clip) (lon, lat, np, x, y, &total_nx);
			polygon = !GMT_polygon_is_open (lon, lat, np);	/* The following can only be used on closed polygons */
			/* Polygons that completely contains the -R region will not generate crossings, just duplicate -R box */
			if (polygon && n > 0 && total_nx == 0) {	/* No crossings and all points outside means one of two things: */
				/* Either the polygon contains portions of the -R region including corners or it does not.  We pick the corners and check for insidedness: */
				GMT_LONG ok = FALSE;
				if (GMT_non_zero_winding (project_info.w, project_info.s, lon, lat, np)) ok = TRUE;		/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (project_info.e, project_info.s, lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (project_info.e, project_info.n, lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (project_info.w, project_info.n, lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok) {
					/* Polygon does NOT contain the region and we delete it */
					n = 0;
					GMT_free ((void *)*x);
					GMT_free ((void *)*y);
				}
				/* Otherwise the polygon completely contains -R and we pass it along */
			}
		}
	}
	else	/* Mixed case so we must clip the polygon */
		n = (*GMT_map_clip) (lon, lat, np, x, y, &total_nx);

	return (n);
}

GMT_LONG GMT_rect_clip_old (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, j = 0, k, nx, n_alloc = GMT_CHUNK;
	GMT_LONG sides[4];
	double xlon[4], xlat[4], xc[4], yc[4], *xx, *yy;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
	(void) GMT_map_outside (lon[0], lat[0]);
	GMT_geo_to_xy (lon[0], lat[0], &xx[0], &yy[0]);
	j += GMT_move_to_rect (xx, yy, j, (GMT_LONG)0);	/* May add 2 points, << n_alloc */

	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (lon[i], lat[i]);
		nx = GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			xx[j] = xc[k];
			yy[j++] = yc[k];
			if (j >= (n_alloc-2)) {
				n_alloc <<= 1;
				xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
				yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
			}
			(*total_nx) ++;
		}
		GMT_geo_to_xy (lon[i], lat[i], &xx[j], &yy[j]);
		if (j >= (n_alloc-2)) {
			n_alloc <<= 1;
			xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
			yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
		}

		j += GMT_move_to_rect (xx, yy, j, nx);	/* May add 2 points, which explains the n_alloc-2 stuff */
	}

	xx = (double *) GMT_memory ((void *)xx, (size_t)j, sizeof (double), "GMT_rect_clip");
	yy = (double *) GMT_memory ((void *)yy, (size_t)j, sizeof (double), "GMT_rect_clip");
	*x = xx;
	*y = yy;

	return (j);
}

/* Functions and macros used for new rectangular clipping using the Sutherland/Hodgman algorithm
 * in which we clip the polygon against each of the 4 sides.  To avoid lots of if/switch I have
 * two clip functions (for x and y line) and two in/out functions that tells us if a point is
 * inside the polygon relative to the line.  Then, pointers to these functions are passed to
 * make sure the right functions are used for each side in the loop over sides.  Unless I can
 * figure out a more clever recursive way I need to have 2 temporary arrays to shuffle the
 * intermediate results around.
 *
 * P.Wessel, March 2008
 */

/* THis macro calculates the x-coordinates where the line segment crosses the border x = border.
 * By swapping x and y in the call we can use it for finding the y intersection. This macro is
 * never called when (y_prev - y_curr) = 0 so we don't divide by zero.
 */
#define INTERSECTION_COORD(x_curr,y_curr,x_prev,y_prev,border) x_curr + (x_prev - x_curr) * (border - y_curr) / (y_prev - y_curr)

GMT_LONG GMT_clip_sn (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, PFI inside, PFI outside, GMT_LONG *cross)
{	/* Clip against the south or north boundary (i.e., a horizontal line with y = border) */
	*cross = 0;
	if (GMT_IS_ZERO (x_prev-x_curr) && GMT_IS_ZERO (y_prev-y_curr)) return (0);	/* Do nothing for duplicates */
	if (outside (y_prev, border)) {	/* Previous point is outside... */
		if (outside (y_curr, border)) return 0;	/* ...as is the current point. Do nothing. */
		/* Here, the line segment intersects the border - return both intersection and inside point */
		y[0] = border;	x[0] = INTERSECTION_COORD (x_curr, y_curr, x_prev, y_prev, border);
		*cross = +1;	/* Crossing to the inside */
		x[1] = x_curr;	y[1] = y_curr;	return (2);
	}
	/* Here x_prev is inside */
	if (inside (y_curr, border)) {	/* Return current point only */
		x[0] = x_curr;	y[0] = y_curr;	return (1);
	}
	/* Segment intersects border - return intersection only */
	*cross = -1;	/* Crossing to the outside */
	y[0] = border;	x[0] = INTERSECTION_COORD (x_curr, y_curr, x_prev, y_prev, border);	return (1);
}

GMT_LONG GMT_clip_we (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, PFI inside, PFI outside, GMT_LONG *cross)
{	/* Clip against the west or east boundary (i.e., a vertical line with x = border) */
	*cross = 0;
	if (GMT_IS_ZERO (x_prev-x_curr) && GMT_IS_ZERO (y_prev-y_curr)) return (0);	/* Do nothing for duplicates */
	if (outside (x_prev, border)) {	/* Previous point is outside... */
		if (outside (x_curr, border)) return 0;	/* ...as is the current point. Do nothing. */
		/* Here, the line segment intersects the border - return both intersection and inside point */
		x[0] = border;	y[0] = INTERSECTION_COORD (y_curr, x_curr, y_prev, x_prev, border);
		*cross = +1;	/* Crossing to the inside */
		x[1] = x_curr;	y[1] = y_curr;	return (2);
	}
	/* Here x_prev is inside */
	if (inside (x_curr, border)) {	/* Return current point only */
		x[0] = x_curr;	y[0] = y_curr;	return (1);
	}
	/* Segment intersects border - return intersection only */
	*cross = -1;	/* Crossing to the outside */
	x[0] = border;	y[0] = INTERSECTION_COORD (y_curr, x_curr, y_prev, x_prev, border);	return (1);
}

/* Tiny functions to tell if a value is <, <=, >=, > than the limit */
GMT_LONG inside_lower_boundary (double val, double min) {return (val >= min);}
GMT_LONG inside_upper_boundary (double val, double max) {return (val <= max);}
GMT_LONG outside_lower_boundary (double val, double min) {return (val < min);}
GMT_LONG outside_upper_boundary (double val, double max) {return (val > max);}

/* GMT_rect_clip is an implementation of the Sutherland/Hodgman algorithm polygon clipping algorithm.
 * Basically, it compares the polygon to one boundary at the time, and clips the polygon to be inside
 * that boundary; this is then repeated for all boundaries.  Assumptions here are Cartesian coordinates
 * so all boundaries are straight lines in x or y. */

GMT_LONG GMT_rect_clip (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, m, n_alloc = 0;
	GMT_LONG side, j, np, in = 1, out = 0, cross = 0;
	GMT_LONG polygon;
	double *xtmp[2], *ytmp[2], xx[2], yy[2], border[4];
	PFL clipper[4], inside[4], outside[4];
#ifdef DEBUG
	FILE *fp;
	GMT_LONG dump = 0;
#endif

	if (n == 0) return (0);

	polygon = !GMT_polygon_is_open (lon, lat, n);	/* TRUE if input segment is a closed polygon */
	
	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */
	
	/* Set up function pointers.  This could be done once in GMT_begin at some point */
	
	clipper[0] = GMT_clip_sn;	clipper[1] = GMT_clip_we; clipper[2] = GMT_clip_sn;	clipper[3] = GMT_clip_we;
	inside[1] = inside[2] = inside_upper_boundary;	outside[1] = outside[2] = outside_upper_boundary;
	inside[0] = inside[3] = inside_lower_boundary;		outside[0] = outside[3] = outside_lower_boundary;
	border[0] = border[3] = 0.0;	border[1] = GMT_map_width;	border[2] = GMT_map_height;

	n_alloc = (GMT_LONG)irint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], n_alloc, 0, sizeof (double), "GMT_rect_clip");
	
	/* Get Cartesian map coordinates */
	
	for (m = 0; m < n; m++) GMT_geo_to_xy (lon[m], lat[m], &xtmp[0][m], &ytmp[0][m]);

#ifdef DEBUG
	if (dump) {
		fp = fopen ("input.d", "w");
		for (i = 0; i < n; i++) fprintf (fp, "%g\t%g\n", xtmp[0][i], ytmp[0][i]);
		fclose (fp);
	}
#endif
	for (side = 0; side < 4; side++) {	/* Must clip polygon against a single border, one border at a time */
		n = m;	/* Current size of polygon */
		m = 0;	/* Start with nuthin' */
		
		l_swap (in, out);	/* Swap what is input and output for clipping against this border */
		/* Must ensure we copy the very first point if it is inside the clip rectangle */
		if (inside[side] ((side%2) ? xtmp[in][0] : ytmp[in][0], border[side])) {xtmp[out][0] = xtmp[in][0]; ytmp[out][0] = ytmp[in][0]; m = 1;}	/* First point is inside; add it */
		for (i = 1; i < n; i++) {	/* For each line segment */
			np = clipper[side] (xtmp[in][i-1], ytmp[in][i-1], xtmp[in][i], ytmp[in][i], xx, yy, border[side], inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], m, n_alloc, sizeof (double), "GMT_rect_clip");
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && GMT_polygon_is_open (xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], m, n_alloc, sizeof (double), "GMT_rect_clip");
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
	}

	GMT_free ((void *)xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	GMT_free ((void *)ytmp[1]);
	
	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = GMT_alloc_memory2 ((void **)&xtmp[0], (void **)&ytmp[0], 0, m, sizeof (double), "GMT_rect_clip");
		*x = xtmp[0];
		*y = ytmp[0];
#ifdef DEBUG
		if (dump) {
			fp = fopen ("output.d", "w");
			for (i = 0; i < m; i++) fprintf (fp, "%g\t%g\n", xtmp[0][i], ytmp[0][i]);
			fclose (fp);
		}
#endif
	}
	else {	/* Nothing survived the clipping - free the output arrays */
		GMT_free ((void *)xtmp[0]);
		GMT_free ((void *)ytmp[0]);
	}

	return (m);
}

/* GMT_wesn_clip differs from GMT_rect_clip in that the boundaries of constant lon or lat may end up as
 * curved lines depending on the map projection.  Thus, if a line crosses the boundary and reenters at
 * another point on the boundary then the straight line between these crossing points should really
 * project to a curved boundary segment.  The H-S algorithm was originally rectangular so we got straight
 * lines.  Here, we check if (1) the particular boundary being tested is curved, and if TRUE then we
 * keep track of the indices of the exit and entry points in the array, and once a boundary has been
 * processed we must add more points between the exit and entry pairs to properly handle the curved
 * segment.  The arrays x_index and x_type stores the index of the exit/entry points and the type
 * (+1 we enter, -1 we exit).  We then use GMT_map_path to compute the required segments to insert.
 * P. Wessel, 2--9-05-07
 */

GMT_LONG GMT_wesn_clip (double *lon, double *lat, GMT_LONG n_orig, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, n, m, new_n, *x_index = NULL, *x_type = NULL;
	GMT_LONG n_alloc, n_x_alloc;
	GMT_LONG side, j, np, in = 1, n_cross = 0, out = 0, cross = 0;
	GMT_LONG polygon, jump = FALSE, curved, periodic = FALSE;
	double *xtmp[2] = {NULL, NULL}, *ytmp[2] = {NULL, NULL}, xx[2], yy[2], border[4];
	double x1, x2, y1, y2;
	PFL clipper[4], inside[4], outside[4];
#ifdef DEBUG
	FILE *fp;
	GMT_LONG dump = 0;
#endif
	
	if ((n = n_orig) == 0) return (0);

	/* Azimuthal polar projections have to be done the old way for the time being */

/* 	if (GMT_IS_AZIMUTHAL && project_info.polar) return (GMT_wesn_clip_old (lon, lat, n, x, y, total_nx)); */

	/* If there are jumps etc call the old clipper, else we try the new clipper */
	
	GMT_geo_to_xy (lon[0], lat[0], &x1, &y1);
	for (i = 1; !jump && i < n; i++) {
		GMT_geo_to_xy (lon[i], lat[i], &x2, &y2);
		jump = GMT_map_jump_x (x2, y2, x1, y1);
		x1 = x2;	y1 = y2;
	}
	
	if (jump) return (GMT_wesn_clip_old (lon, lat, n, x, y, total_nx));	/* Must do the old way for now */
	periodic = GMT_360_RANGE (project_info.w, project_info.e);	/* No point clipping against W and E if periodic map */
	
	/* Here we can try the Sutherland/Hodgman algorithm */
	
	polygon = !GMT_polygon_is_open (lon, lat, n);	/* TRUE if input segment is a closed polygon */

	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */

	/* Set up function pointers.  This could be done once in GMT_begin at some point */

	clipper[0] = GMT_clip_sn;	clipper[1] = GMT_clip_we; clipper[2] = GMT_clip_sn;	clipper[3] = GMT_clip_we;
	inside[1] = inside[2] = inside_upper_boundary;	outside[1] = outside[2] = outside_upper_boundary;
	inside[0] = inside[3] = inside_lower_boundary;		outside[0] = outside[3] = outside_lower_boundary;
	border[0] = project_info.s; border[3] = project_info.w;	border[1] = project_info.e;	border[2] = project_info.n;

	n_alloc = (GMT_LONG)irint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], n_alloc, 0, sizeof (double), "GMT_wesn_clip");

	/* Make copy of lon/lat coordinates */

	memcpy ((void *)xtmp[0], (void *)lon, n*sizeof (double));	memcpy ((void *)ytmp[0], (void *)lat, n*sizeof (double));
	m = n;

	/* Preallocate space for crossing information */
	
	n_x_alloc = GMT_alloc_memory2 ((void **)&x_index, (void **)&x_type, GMT_TINY_CHUNK, 0, sizeof (GMT_LONG), "GMT_wesn_clip");

#ifdef DEBUG
	if (dump) {
		fp = fopen ("input.d", "w");
		for (i = 0; i < n; i++) fprintf (fp, "%g\t%g\n", xtmp[0][i], ytmp[0][i]);
		fclose (fp);
	}
#endif
	for (side = 0; side < 4; side++) {	/* Must clip polygon against a single border, one border at a time */
		n = m;		/* Current size of polygon */
		m = 0;		/* Start with nuthin' */
		n_cross = 0;	/* No crossings so far */

		curved = !((side%2) ? GMT_meridian_straight : GMT_parallel_straight);	/* Is this border straight or curved when projected */
		l_swap (in, out);	/* Swap what is input and output for clipping against this border */
		if (side%2 && periodic) {	/* No clipping can take place on w or e border; just copy all and go to next side */
			m = n;
			if (m == n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], m, n_alloc, sizeof (double), "GMT_wesn_clip");
			memcpy ((void *)xtmp[out], (void *)xtmp[in], (size_t)(m * sizeof (double)));
			memcpy ((void *)ytmp[out], (void *)ytmp[in], (size_t)(m * sizeof (double)));
			continue;
		}
		
		/* Must ensure we copy the very first point if it is inside the clip rectangle */
		if (inside[side] ((side%2) ? xtmp[in][0] : ytmp[in][0], border[side])) {xtmp[out][0] = xtmp[in][0]; ytmp[out][0] = ytmp[in][0]; m = 1;}	/* First point is inside; add it */
		for (i = 1; i < n; i++) {	/* For each line segment */
			np = clipper[side] (xtmp[in][i-1], ytmp[in][i-1], xtmp[in][i], ytmp[in][i], xx, yy, border[side], inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			if (polygon && cross && curved) {	/* When crossing in/out of a curved boundary we must eventually sample along the curve between crossings */
				x_index[n_cross] = m;		/* Index of intersection point (which will be copied from xx[0], yy[0] below) */
				x_type[n_cross] = cross;	/* -1 going out, +1 going in */
				if (++n_cross == n_x_alloc) n_x_alloc = GMT_alloc_memory2 ((void **)&x_index, (void **)&x_type, n_cross, n_x_alloc, sizeof (GMT_LONG), "GMT_wesn_clip");
			}
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], m, n_alloc, sizeof (double), "GMT_wesn_clip");
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && GMT_polygon_is_open (xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], m, n_alloc, sizeof (double), "GMT_wesn_clip");
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
		if (polygon && curved && n_cross) {	/* Must resample between crossing points */
			double *x_add, *y_add, *x_cpy, *y_cpy;
			GMT_LONG add, np = 0, last_index = 0;
			GMT_LONG p, p_next;
			
			if (n_cross%2 == 1) {	/* Should not happen with a polygon */
				fprintf (stderr, "GMT: Error in GMT_wesn_clip: odd number of crossings?");
			}
			
			/* First copy the current polygon */
			
			(void)GMT_alloc_memory2 ((void **)&x_cpy, (void **)&y_cpy, m, 0, sizeof (double), "GMT_wesn_clip");
			memcpy ((void *)x_cpy, (void *)xtmp[out], (size_t)(m * sizeof (double)));
			memcpy ((void *)y_cpy, (void *)ytmp[out], (size_t)(m * sizeof (double)));
			
			for (p = 0; p < n_cross; p++) {	/* Process each crossing point */
				if (last_index < x_index[p]) {	/* Copy over segment from were we left off to this crossing point */
					add = x_index[p] - last_index;
					if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], new_n, n_alloc, sizeof (double), "GMT_wesn_clip");
					memcpy ((void *)&xtmp[out][np], (void *)&x_cpy[last_index], (size_t)(add * sizeof (double)));
					memcpy ((void *)&ytmp[out][np], (void *)&y_cpy[last_index], (size_t)(add * sizeof (double)));
					np += add;
					last_index = x_index[p];
				}
				if (x_type[p] == -1) {	/* Must add path from this exit to the next entry */
					double start_lon, stop_lon;
					p_next = (p == (n_cross-1)) ? 0 : p + 1;	/* index of the next crossing */
					start_lon = x_cpy[x_index[p]];	stop_lon = x_cpy[x_index[p_next]];
					if (side%2 == 0 && periodic) {	/* Make sure we select the shortest longitude arc */
						if ((x_cpy[x_index[p_next]] - x_cpy[x_index[p]]) < -180.0)
							stop_lon += 360.0;
						else if ((x_cpy[x_index[p_next]] - x_cpy[x_index[p]]) > +180.0)
							stop_lon -= 360.0;
					}
					add = GMT_map_path (start_lon, y_cpy[x_index[p]], stop_lon, y_cpy[x_index[p_next]], &x_add, &y_add);
					if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], new_n, n_alloc, sizeof (double), "GMT_wesn_clip");
					memcpy ((void *)&xtmp[out][np], (void *)x_add, (size_t)(add * sizeof (double)));
					memcpy ((void *)&ytmp[out][np], (void *)y_add, (size_t)(add * sizeof (double)));
					if (add) { GMT_free ((void *)x_add);	GMT_free ((void *)y_add);}
					np += add;
					last_index = x_index[p_next];
				}
			}
			if (x_index[0] > 0) {	/* First point was clean inside, must add last connection */
				add = m - last_index;
				if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_alloc_memory4 ((void **)&xtmp[0], (void **)&ytmp[0], (void **)&xtmp[1], (void **)&ytmp[1], new_n, n_alloc, sizeof (double), "GMT_wesn_clip");
				memcpy ((void *)&xtmp[out][np], (void *)&x_cpy[last_index], (size_t)(add * sizeof (double)));
				memcpy ((void *)&ytmp[out][np], (void *)&y_cpy[last_index], (size_t)(add * sizeof (double)));
				np += add;
			}
			m = np;	/* New total of points */
			GMT_free ((void *)x_cpy);	GMT_free ((void *)y_cpy);
		}
	}

	GMT_free ((void *)xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	GMT_free ((void *)ytmp[1]);
	GMT_free ((void *)x_index);	/* Free the pairs of arrays that holds the crossing info */
	GMT_free ((void *)x_type);

	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = GMT_alloc_memory2 ((void **)&xtmp[0], (void **)&ytmp[0], 0, m, sizeof (double), "GMT_wesn_clip");
		/* Convert to map coordinates */
		for (i = 0; i < m; i++) GMT_geo_to_xy (xtmp[0][i], ytmp[0][i], &xtmp[0][i], &ytmp[0][i]);
		
		*x = xtmp[0];
		*y = ytmp[0];
#ifdef DEBUG
		if (dump) {
			fp = fopen ("output.d", "w");
			for (i = 0; i < m; i++) fprintf (fp, "%g\t%g\n", xtmp[0][i], ytmp[0][i]);
			fclose (fp);
		}
#endif
	}
	else {	/* Nothing survived the clipping - free the output arrays */
		GMT_free ((void *)xtmp[0]);
		GMT_free ((void *)ytmp[0]);
	}

	return (m);
}

GMT_LONG GMT_wesn_clip_old (double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, j = 0, k, nx, n_alloc = GMT_CHUNK;
	GMT_LONG sides[4];
	double xlon[4], xlat[4], xc[4], yc[4], *xx, *yy;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");

	(void) GMT_map_outside (lon[0], lat[0]);
	j = GMT_move_to_wesn (xx, yy, lon[0], lat[0], 0.0, 0.0, (GMT_LONG)0, (GMT_LONG)0);	/* May add 2 points, << n_alloc */

	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (lon[i], lat[i]);
		nx = GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			xx[j] = xc[k];
			yy[j++] = yc[k];
			if (j >= (n_alloc-2)) {
				n_alloc <<= 1;
				xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
				yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
			}
			(*total_nx) ++;
		}
		if (j >= (n_alloc-2)) {
			n_alloc <<= 1;
			xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");

			yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
		}
		j += GMT_move_to_wesn (xx, yy, lon[i], lat[i], lon[i-1], lat[i-1], j, nx);	/* May add 2 points, which explains the n_alloc-2 stuff */
	}

	xx = (double *) GMT_memory ((void *)xx, (size_t)j, sizeof (double), "GMT_wesn_clip");
	yy = (double *) GMT_memory ((void *)yy, (size_t)j, sizeof (double), "GMT_wesn_clip");
	*x = xx;
	*y = yy;

#ifdef CRAP
{
	FILE *fp;
	double out[2];
	fp = fopen ("crap.d", "a");
	fprintf (fp, "> N = %d\n", (int)j);
	for (i = 0; i < j; i++) {
		out[GMT_X] = xx[i];
		out[GMT_Y] = yy[i];
		GMT_output (fp, 2, out);
	}
	fclose (fp);
}
#endif
	return (j);
}

GMT_LONG GMT_move_to_rect (double *x_edge, double *y_edge, GMT_LONG j, GMT_LONG nx)
{
	GMT_LONG n = 0, key;
	double xtmp, ytmp;

	/* May add 0, 1, or 2 points to path */

	if (GMT_x_status_new == 0 && GMT_y_status_new == 0) return (1);	/* Completely Inside */

	if (!nx && j > 0 && GMT_x_status_new != GMT_x_status_old && GMT_y_status_new != GMT_y_status_old) {	/* Must include corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((GMT_x_status_new * GMT_x_status_old) == -4 || (GMT_y_status_new * GMT_y_status_old) == -4) {	/* the two points outside on opposite sides */
			x_edge[j] = (GMT_x_status_old < 0) ? project_info.xmin : ((GMT_x_status_old > 0) ? project_info.xmax : GMT_x_to_corner (x_edge[j-1]));
			y_edge[j] = (GMT_y_status_old < 0) ? project_info.ymin : ((GMT_y_status_old > 0) ? project_info.ymax : GMT_y_to_corner (y_edge[j-1]));
			j++;
			x_edge[j] = (GMT_x_status_new < 0) ? project_info.xmin : ((GMT_x_status_new > 0) ? project_info.xmax : GMT_x_to_corner (xtmp));
			y_edge[j] = (GMT_y_status_new < 0) ? project_info.ymin : ((GMT_y_status_new > 0) ? project_info.ymax : GMT_y_to_corner (ytmp));
			j++;
		}
		else {
			key = MIN (GMT_x_status_new, GMT_x_status_old);
			x_edge[j] = (key < 0) ? project_info.xmin : project_info.xmax;
			key = MIN (GMT_y_status_new, GMT_y_status_old);
			y_edge[j] = (key < 0) ? project_info.ymin : project_info.ymax;
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}

	if (GMT_outside == GMT_rect_outside2) {	/* Need special check because this outside2 test is screwed up... */
		if (x_edge[j] < project_info.xmin) {
			x_edge[j] = project_info.xmin;
			GMT_x_status_new = -2;
		}
		else if (x_edge[j] > project_info.xmax) {
			x_edge[j] = project_info.xmax;
			GMT_x_status_new = 2;
		}
		if (y_edge[j] < project_info.ymin) {
			y_edge[j] = project_info.ymin;
			GMT_y_status_new = -2;
		}
		else if (y_edge[j] > project_info.ymax) {
			y_edge[j] = project_info.ymax;
			GMT_y_status_new = 2;
		}
	}
	else {
		if (GMT_x_status_new != 0) x_edge[j] = (GMT_x_status_new < 0) ? project_info.xmin : project_info.xmax;
		if (GMT_y_status_new != 0) y_edge[j] = (GMT_y_status_new < 0) ? project_info.ymin : project_info.ymax;
	}

	return (n + 1);
}

GMT_LONG GMT_move_to_wesn (double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, GMT_LONG j, GMT_LONG nx)
{
	GMT_LONG n = 0, key;
	double xtmp, ytmp, lon_p, lat_p;

	/* May add 0, 1, or 2 points to path */

	if (!nx && j > 0 && GMT_x_status_new != GMT_x_status_old && GMT_y_status_new != GMT_y_status_old) {	/* Need corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((GMT_x_status_new * GMT_x_status_old) == -4 || (GMT_y_status_new * GMT_y_status_old) == -4) {	/* the two points outside on opposite sides */
			lon_p = (GMT_x_status_old < 0) ? project_info.w : ((GMT_x_status_old > 0) ? project_info.e : GMT_lon_to_corner (lon_old));
			lat_p = (GMT_y_status_old < 0) ? project_info.s : ((GMT_y_status_old > 0) ? project_info.n : GMT_lat_to_corner (lat_old));
			GMT_geo_to_xy (lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
			lon_p = (GMT_x_status_new < 0) ? project_info.w : ((GMT_x_status_new > 0) ? project_info.e : GMT_lon_to_corner (lon));
			lat_p = (GMT_y_status_new < 0) ? project_info.s : ((GMT_y_status_new > 0) ? project_info.n : GMT_lat_to_corner (lat));
			GMT_geo_to_xy (lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		else {
			key = MIN (GMT_x_status_new, GMT_x_status_old);
			lon_p = (key < 0) ? project_info.w : project_info.e;
			key = MIN (GMT_y_status_new, GMT_y_status_old);
			lat_p = (key < 0) ? project_info.s : project_info.n;
			GMT_geo_to_xy (lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}
	if (GMT_x_status_new != 0) lon = (GMT_x_status_new < 0) ? project_info.w : project_info.e;
	if (GMT_y_status_new != 0) lat = (GMT_y_status_new < 0) ? project_info.s : project_info.n;
	GMT_geo_to_xy (lon, lat, &x_edge[j], &y_edge[j]);
	return (n + 1);
}

double GMT_lon_to_corner (double lon) {
	return ( (fabs (lon - project_info.w) < fabs (lon - project_info.e)) ? project_info.w : project_info.e);
}

double GMT_lat_to_corner (double lat) {
	return ( (fabs (lat - project_info.s) < fabs (lat - project_info.n)) ? project_info.s : project_info.n);
}

double GMT_x_to_corner (double x) {
	return ( (fabs (x - project_info.xmin) < fabs (x - project_info.xmax)) ? project_info.xmin : project_info.xmax);
}

double GMT_y_to_corner (double y) {
	return ( (fabs (y - project_info.ymin) < fabs (y - project_info.ymax)) ? project_info.ymin : project_info.ymax);
}

GMT_LONG GMT_radial_clip_new (double *lon, double *lat, GMT_LONG np, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG n = 0, this = FALSE, i, n_alloc = 0, n_arc;
	GMT_LONG sides[4], nx;
	GMT_LONG add_boundary = FALSE;
	double xlon[4], xlat[4], xc[4], yc[4], xr, yr, *xx, *yy, *xarc, *yarc;
	double end_x[3], end_y[3];

	*total_nx = 0;	/* Keep track of total of crossings */

	if (np == 0) return (0);

	if (!GMT_map_outside (lon[0], lat[0])) {
		n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_new");
		GMT_geo_to_xy (lon[0], lat[0], &xx[0], &yy[0]);
		n++;
	}
	nx = 0;
	for (i = 1; i < np; i++) {
		this = GMT_map_outside (lon[i], lat[i]);
		if (GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides)) {	/* Crossed map boundary */
			if (this) {	/* Crossing boundary and leaving circle: Add exit point to the path */
				if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_new");
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
			end_x[nx] = xc[0] - project_info.r;	end_y[nx] = yc[0] - project_info.r;
			nx++;
			(*total_nx) ++;
			if (nx >= 2) {	/* Got a pair of entry+exit points */
				add_boundary = !this;	/* We only add boundary arcs if we first exited and now entered the circle again */
			}
			if (add_boundary) {	/* Crossed twice.  Now add arc between the two crossing points */
				/* PW: Currently, we make the assumption that the shortest arc is the one we want.  However,
				 * extremely large polygons could cut the boundary so that it is the longest arc we want.
				 * The way to improve this algorithm in the future is to find the two opposite points on
				 * the circle boundary that lies on the bisector of az1,az2, and see which point lies
				 * inside the polygon.  This would require that GMT_inonout_sphpol be called.
				 */
				if ((n_arc = GMT_radial_boundary_arc (this, &end_x[nx-2], &end_y[nx-2], &xarc, &yarc)) > 0) {
					if ((n + n_arc) >= n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, (n + n_arc), n_alloc, sizeof (double), "GMT_radial_clip_new");
					memcpy ((void *)&xx[n], (void *)xarc, (size_t)(n_arc * sizeof (double)));	/* Copy longitudes of arc */
					memcpy ((void *)&yy[n], (void *)yarc, (size_t)(n_arc * sizeof (double)));	/* Copy latitudes of arc */
					n += n_arc;	/* Number of arc points added (end points are done separately) */
					GMT_free ((void *) xarc);	GMT_free ((void *) yarc);
				}
				add_boundary = FALSE;
				nx -= 2;	/* Done with those two crossings */
			}
			if (!this) {	/* Crossing boundary and entering circle: Add entry point to the path */
				if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_new");
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
		}
		GMT_geo_to_xy (lon[i], lat[i], &xr, &yr);
		if (!this) {	/* Only add points actually inside the map to the path */
			if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_new");
			xx[n] = xr;	yy[n] = yr;	n++;
		}
	}

	if (nx == 2) {	/* Must close polygon by adding boundary arc */
		if ((n_arc = GMT_radial_boundary_arc (this, end_x, end_y, &xarc, &yarc)) > 0) {
			if ((n + n_arc) >= n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, (n + n_arc), n_alloc, sizeof (double), "GMT_radial_clip_new");
			memcpy ((void *)&xx[n], (void *)xarc, (size_t)(n_arc * sizeof (double)));	/* Copy longitudes of arc */
			memcpy ((void *)&yy[n], (void *)yarc, (size_t)(n_arc * sizeof (double)));	/* Copy latitudes of arc */
			n += n_arc;	/* Number of arc points added (end points are done separately) */
			GMT_free ((void *) xarc);	GMT_free ((void *) yarc);
		}
		if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_new");
		xx[n] = xx[0];	yy[n] = yy[0];	n++;	/* Close the polygon */
	}
	(void) GMT_alloc_memory2 ((void **)&xx, (void **)&yy, 0, n, sizeof (double), "GMT_radial_clip_new");
	*x = xx;
	*y = yy;
#ifdef DEBUG
	if (clip_dump) dumppol (n, xx, yy, &clip_id);
#endif

	return (n);
}

#ifdef DEBUG
/* If we need to dump out clipped polygon then set clip_dump = 1 during execution */
void dumppol (GMT_LONG n, double *x, double *y, GMT_LONG *id)
{
	GMT_LONG i;
	FILE *fp;
	char line[64];
	sprintf (line, "dump_%ld.d", *id);
	fp = fopen (line, "w");
	for (i = 0; i < n; i++) fprintf (fp, "%g\t%g\n", x[i], y[i]);
	fclose (fp);
	(*id)++;
}
#endif

GMT_LONG GMT_radial_boundary_arc (GMT_LONG this, double end_x[], double end_y[], double **xarc, double **yarc) {
	GMT_LONG n_arc, k, pt;
	double az1, az2, d_az, da, xr, yr, da_try, *xx, *yy;
	
	/* When a polygon crosses out then in again into the circle we need to add a boundary arc
	 * to the polygon where it is clipped.  We simply sample the circle as finely as the arc
	 * length and the current line_step demands */
	
	da_try = (gmtdefs.line_step * 360.0) / (TWO_PI * project_info.r);	/* Angular step in degrees */
	az1 = d_atan2d (end_y[0], end_x[0]);	/* azimuth from map center to 1st crossing */
	az2 = d_atan2d (end_y[1], end_x[1]);	/* azimuth from map center to 2nd crossing */
	d_az = az2 - az1;							/* Arc length in degrees */
	if (fabs(d_az) > 180.0) d_az = copysign (360.0 - fabs(d_az), -d_az);	/* Insist we take the short arc for now */
	n_arc = (GMT_LONG)ceil (fabs (d_az)/ da_try);	/* Get number of integer increments of da_try degree */
	da = d_az / (n_arc - 1);			/* Reset da to get exact steps */
	n_arc -= 2;	/* We do not include the end points since these are the crossing points handled in the calling function */
	if (n_arc <= 0) return (0);	/* Arc is too short to have intermediate points */
	(void) GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n_arc, 0, sizeof (double), "GMT_radial_boundary_arc");
	for (k = 1; k <= n_arc; k++) {	/* Create points along arc from first to second crossing point (k-loop excludes the end points) */
		sincosd (az1 + k * da, &yr, &xr);
		pt = (this) ? n_arc - k : k - 1;	/* The order we add the arc depends if we exited or entered the inside area */
		xx[pt] = project_info.r * (1.0 + xr);
		yy[pt] = project_info.r * (1.0 + yr);
	}
	
	*xarc = xx;
	*yarc = yy;
	return (n_arc);
}

GMT_LONG GMT_radial_clip_pscoast (double *lon, double *lat, GMT_LONG np, double **x, double **y, GMT_LONG *total_nx)
{	/* Need this until pscoast uses a different mechanism to clip the polygons */
	GMT_LONG n = 0, this, i, n_alloc = 0;
	GMT_LONG sides[4];
	double xlon[4], xlat[4], xc[4], yc[4], xr, yr, r, scale, x0, y0, *xx, *yy;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (np == 0) return (0);

	if (!GMT_map_outside (lon[0], lat[0])) {
		n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_pscoast");
		GMT_geo_to_xy (lon[0], lat[0], &xx[0], &yy[0]);
		n++;
	}
	for (i = 1; i < np; i++) {
		this = GMT_map_outside (lon[i], lat[i]);
		if (GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides)) {	/* Crossed map boundary */
			if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_pscoast");
			xx[n] = xc[0];	yy[n] = yc[0];	n++;
			(*total_nx) ++;
		}
		GMT_geo_to_xy (lon[i], lat[i], &xr, &yr);
		if (this && (project_info.projection != GMT_GENPER)) {                 /* Project point onto perimeter */
			GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &x0, &y0);
			xr -= x0;	yr -= y0;
			r = hypot (xr, yr);
			scale = project_info.r / r;
			xr *= scale;
			yr *= scale;
			xr += x0;	yr += y0;
		}
		if (n == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), "GMT_radial_clip_pscoast");
		xx[n] = xr;	yy[n] = yr;	n++;
	}

	(void) GMT_alloc_memory2 ((void **)&xx, (void **)&yy, 0, n, sizeof (double), "GMT_radial_clip_pscoast");
	*x = xx;
	*y = yy;

	return (n);
}

GMT_LONG GMT_lon_inside (double lon, double w, double e)
{

	while (lon < project_info.w) lon += 360.0;
	while (lon > project_info.e) lon -= 360.0;

	if (lon < w) return (FALSE);
	if (lon > e) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_geo_to_xy_line (double *lon, double *lat, GMT_LONG n)
{
	/* Traces the lon/lat array and returns x,y plus appropriate pen moves */
	GMT_LONG j, np, inside;
	GMT_LONG sides[4], nx;
	double xlon[4], xlat[4], xx[4], yy[4];
	double this_x, this_y, last_x, last_y, dummy[4];

	while (n > GMT_n_alloc) GMT_get_plot_array ();

	np = 0;
	GMT_geo_to_xy (lon[0], lat[0], &last_x, &last_y);
	if (!GMT_map_outside (lon[0], lat[0])) {
		GMT_x_plot[0] = last_x;	GMT_y_plot[0] = last_y;
		GMT_pen[np++] = 3;
	}
	for (j = 1; j < n; j++) {
		GMT_geo_to_xy (lon[j], lat[j], &this_x, &this_y);
		inside = !GMT_map_outside (lon[j], lat[j]);
		if ((nx = GMT_map_crossing (lon[j-1], lat[j-1], lon[j], lat[j], xlon, xlat, xx, yy, sides))) { /* Nothing */ }
		else if (GMT_world_map)
			nx = (*GMT_wrap_around_check) (dummy, last_x, last_y, this_x, this_y, xx, yy, sides);
		if (nx == 1) {
			/* fprintf (stderr, "1: %ld %g %g : %g %g : %g %g\n", inside, xlon[0], xlat[0], lon[j-1], lat[j-1], lon[j], lat[j]); */
			GMT_x_plot[np] = xx[0];	GMT_y_plot[np] = yy[0];
			GMT_pen[np++] = (inside) ? 3 : 2;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		else if (nx == 2) {
			/* fprintf (stderr, "2: %ld %g %g %g %g\n", inside, xlon[0], xlat[0], xlon[1], xlat[1]); */
			GMT_x_plot[np] = xx[0];	GMT_y_plot[np] = yy[0];
			GMT_pen[np++] = (inside) ? 2 : 3;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
			GMT_x_plot[np] = xx[1];	GMT_y_plot[np] = yy[1];
			GMT_pen[np++] = (inside) ? 3 : 2;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		if (inside) {
			GMT_x_plot[np] = this_x;	GMT_y_plot[np] = this_y;
			GMT_pen[np++] = 2;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		last_x = this_x;	last_y = this_y;
	}
	if (np) GMT_pen[0] = 3;	/* Sanity override: Gotta start off with new start point */
	return (np);
}

GMT_LONG GMT_compact_line (double *x, double *y, GMT_LONG n, GMT_LONG pen_flag, int *pen)
{	/* TRUE if pen movements is present */
	/* GMT_compact_line will remove unnecessary points in paths */
	GMT_LONG i, j;
	double old_slope, new_slope, dx;
	char *flag;

	if (n < 3) return (n);	/* Nothing to do */
	flag = (char *) GMT_memory (VNULL, (size_t)n, sizeof (char), "GMT_compact_line");

	dx = x[1] - x[0];
	old_slope = (GMT_IS_ZERO (dx)) ? copysign (HALF_DBL_MAX, y[1] - y[0]) : (y[1] - y[0]) / dx;

	for (i = 1; i < n-1; i++) {
		dx = x[i+1] - x[i];
		new_slope = (GMT_IS_ZERO (dx)) ? copysign (HALF_DBL_MAX, y[i+1] - y[i]) : (y[i+1] - y[i]) / dx;
		if (GMT_IS_ZERO (new_slope - old_slope) && !(pen_flag && (pen[i]+pen[i+1]) > 4))	/* 4 is 2+2 which is draw line; a 3 will produce > 4 */
			flag[i] = 1;
		else
			old_slope = new_slope;
	}

	for (i = j = 1; i < n; i++) {	/* i = 1 since first point must be included */
		if (flag[i] == 0) {
			x[j] = x[i];
			y[j] = y[i];
			if (pen_flag) pen[j] = pen[i];
			j++;
		}
	}
	GMT_free ((void *)flag);

	return (j);
}

/* Routines to transform grdfiles to/from map projections */

GMT_LONG GMT_grdproject_init (struct GRD_HEADER *head, double x_inc, double y_inc, GMT_LONG nx, GMT_LONG ny, GMT_LONG dpi, GMT_LONG offset)
{
	if (x_inc > 0.0 && y_inc > 0.0) {
		head->nx = (int)GMT_get_n (head->x_min, head->x_max, x_inc, offset);
		head->ny = (int)GMT_get_n (head->y_min, head->y_max, y_inc, offset);
		head->x_inc = GMT_get_inc (head->x_min, head->x_max, head->nx, offset);
		head->y_inc = GMT_get_inc (head->y_min, head->y_max, head->ny, offset);
	}
	else if (nx > 0 && ny > 0) {
		head->nx = (int)nx;	head->ny = (int)ny;
		head->x_inc = GMT_get_inc (head->x_min, head->x_max, head->nx, offset);
		head->y_inc = GMT_get_inc (head->y_min, head->y_max, head->ny, offset);
	}
	else if (dpi > 0) {
		head->nx = (int)(irint ((head->x_max - head->x_min) * dpi) + 1 - offset);
		head->ny = (int)(irint ((head->y_max - head->y_min) * dpi) + 1 - offset);
		head->x_inc = GMT_get_inc (head->x_min, head->x_max, head->nx, offset);
		head->y_inc = GMT_get_inc (head->y_min, head->y_max, head->ny, offset);
	}
	else {
		fprintf (stderr, "GMT_grdproject_init: Necessary arguments not set\n");
		GMT_exit (EXIT_FAILURE);
	}
	head->node_offset = (int)offset;

	GMT_RI_prepare (head);	/* Ensure -R -I consistency and set nx, ny */
	GMT_err_pass (GMT_grd_RI_verify (head, 1), "");

	if (gmtdefs.verbose) fprintf (stderr, "%s: Grid projection from size %ldx%ld to %dx%d\n", GMT_program, nx, ny, head->nx, head->ny);
	return (GMT_NOERROR);
}

GMT_LONG GMT_grd_project (float *z_in, struct GRD_HEADER *I, float *z_out, struct GRD_HEADER *O, struct GMT_EDGEINFO *edgeinfo, GMT_LONG antialias, GMT_LONG interpolant, double threshold, GMT_LONG inverse)
{
	/* Generalized grid projection that deals with both interpolation and averaging effects.
	 * It assumes that the incoming grid was read with 2 boundary rows/cols so that the bcr
	 * functions can be used. Therefore, we need to add these rows/cols when accessing the
	 * grid directly.  The I and z_in represents the input grid which is either in original
	 * (i.e., lon/lat) coordinates or projected x/y (if inverse = TRUE).
	 *
	 * z_in:	Array with input grid on a padded grid with 2 extra rows/columns
	 * I:		Grid header for input grid
	 * z_out:	Array with output grid, no padding needed
	 * O:		Grid header for output grid
	 * edgeinfo:	Structure with information about boundary conditions on input grid
	 * antialias:	TRUE if we need to do the antialiasing STEP 1 (below)
	 * interpolant:	0 = nearest neighbor, 1 = bilinear, 2 = B-spline, 3 = bicubic
	 * threshold:	minumum weight to be used. If weight < threshold interpolation yields NaN.
	 * inverse:	TRUE if input is x/y and we want to invert for a lon/lat grid
	 *
	 * We assume the calling program has initialized the z_out array to any default empty value (NaN etc).
	 *
	 * Changed 10-Sep-07 to include the argument "antialias" and "threshold" and
	 * made "interpolant" an integer (was GMT_LONG bilinear).
	 */

	GMT_LONG i_in, j_in, ij_in, i_out, j_out, ij_out, mx, my;
	short int *nz = VNULL;
	double x_proj, y_proj, I_off, O_off, z_int, inv_nz;
	double *x_in, *y_in, *x_in_proj = VNULL, *y_in_proj = VNULL, *x_out, *y_out, *x_out_proj = VNULL, *y_out_proj = VNULL;
	struct GMT_BCR bcr;

	mx = I->nx + 4;
	my = I->ny + 4;

	GMT_boundcond_param_prep (I, edgeinfo);

	/* Initialize bcr structure:
	   Threshold changed 10 Sep 07 by RS from 1.0 to <threshold> to allow interpolation closer to
	   a NaN value. In case of bilinear interpolation, using 1.0 creates a rectangular hole
	   the size of 4 grid cells for a single NaN grid node. Using 0.25 will create a diamond
	   shaped hole the size of one cell. */
	GMT_bcr_init (I, GMT_pad, interpolant, threshold, &bcr);

	/* Set boundary conditions  */

	GMT_boundcond_set (I, edgeinfo, GMT_pad, z_in);

	if (antialias) nz = (short int *) GMT_memory (VNULL, (size_t)(O->nx * O->ny), sizeof (short int), "GMT_grd_project");
	x_in = (double *) GMT_memory (VNULL, (size_t)I->nx, sizeof (double), "GMT_grd_project");
	y_in = (double *) GMT_memory (VNULL, (size_t)I->ny, sizeof (double), "GMT_grd_project");
	x_out = (double *) GMT_memory (VNULL, (size_t)O->nx, sizeof (double), "GMT_grd_project");
	y_out = (double *) GMT_memory (VNULL, (size_t)O->ny, sizeof (double), "GMT_grd_project");

	I_off = (I->node_offset) ? 0.5 : 0.0;
	O_off = (O->node_offset) ? 0.5 : 0.0;

	/* Precalculate grid coordinates */

	for (i_in = 0; i_in < I->nx; i_in++) x_in[i_in] = GMT_i_to_x (i_in, I->x_min, I->x_max, I->x_inc, I_off, I->nx);
	for (j_in = 0; j_in < I->ny; j_in++) y_in[j_in] = GMT_j_to_y (j_in, I->y_min, I->y_max, I->y_inc, I_off, I->ny);
	for (i_out = 0; i_out < O->nx; i_out++) x_out[i_out] = GMT_i_to_x (i_out, O->x_min, O->x_max, O->x_inc, O_off, O->nx);
	for (j_out = 0; j_out < O->ny; j_out++) y_out[j_out] = GMT_j_to_y (j_out, O->y_min, O->y_max, O->y_inc, O_off, O->ny);

	if (GMT_IS_RECT_GRATICULE) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj = (double *) GMT_memory (VNULL, (size_t)I->nx, sizeof (double), "GMT_grd_project");
		y_in_proj = (double *) GMT_memory (VNULL, (size_t)I->ny, sizeof (double), "GMT_grd_project");
		x_out_proj = (double *) GMT_memory (VNULL, (size_t)O->nx, sizeof (double), "GMT_grd_project");
		y_out_proj = (double *) GMT_memory (VNULL, (size_t)O->ny, sizeof (double), "GMT_grd_project");
		if (inverse) {
			for (i_in = 0; i_in < I->nx; i_in++) GMT_xy_to_geo (&x_in_proj[i_in], &y_proj, x_in[i_in], I->y_min);
			for (j_in = 0; j_in < I->ny; j_in++) GMT_xy_to_geo (&x_proj, &y_in_proj[j_in], I->x_min, y_in[j_in]);
			for (i_out = 0; i_out < O->nx; i_out++) GMT_geo_to_xy (x_out[i_out], I->y_min, &x_out_proj[i_out], &y_proj);
			for (j_out = 0; j_out < O->ny; j_out++) GMT_geo_to_xy (I->y_min, y_out[j_out], &x_proj, &y_out_proj[j_out]);
		}
		else {
			for (i_in = 0; i_in < I->nx; i_in++) GMT_geo_to_xy (x_in[i_in], I->y_min, &x_in_proj[i_in], &y_proj);
			for (j_in = 0; j_in < I->ny; j_in++) GMT_geo_to_xy (I->x_min, y_in[j_in], &x_proj, &y_in_proj[j_in]);
			for (i_out = 0; i_out < O->nx; i_out++) {
				GMT_xy_to_geo (&x_out_proj[i_out], &y_proj, x_out[i_out], I->y_min);
				if (GMT_io.in_col_type[0] == GMT_IS_LON && !GMT_is_dnan (x_out_proj[i_out])) {
					while (x_out_proj[i_out] < I->x_min - GMT_SMALL) x_out_proj[i_out] += 360.0;
					while (x_out_proj[i_out] > I->x_max + GMT_SMALL) x_out_proj[i_out] -= 360.0;
				}
			}
			for (j_out = 0; j_out < O->ny; j_out++) GMT_xy_to_geo (&x_proj, &y_out_proj[j_out], I->y_min, y_out[j_out]);
		}
	}

	for (ij_out = 0; ij_out < O->ny * O->nx; ij_out++) z_out[ij_out] = GMT_f_NaN;	/* So that nodes outside will retain a NaN value */

	/* PART 1: Project input grid points and do a blockmean operation */

	if (antialias) {
	    for (j_in = 0; j_in < I->ny; j_in++) {	/* Loop over the input grid coordinates */
		if (GMT_IS_RECT_GRATICULE) y_proj = y_in_proj[j_in];
		for (i_in = 0; i_in < I->nx; i_in++) {
			if (GMT_IS_RECT_GRATICULE)
				x_proj = x_in_proj[i_in];
			else if (inverse)
				GMT_xy_to_geo (&x_proj, &y_proj, x_in[i_in], y_in[j_in]);
			else {
				if (GMT_outside (x_in[i_in], y_in[j_in])) continue;	/* Quite possible we are beyond the horizon */
				GMT_geo_to_xy (x_in[i_in], y_in[j_in], &x_proj, &y_proj);
			}

			/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */

			j_out = GMT_y_to_j (y_proj, O->y_min, O->y_inc, O_off, O->ny);
			if (j_out < 0 || j_out >= O->ny) continue;	/* Outside our grid region */
			i_out = GMT_x_to_i (x_proj, O->x_min, O->x_inc, O_off, O->nx);
			if (i_out < 0 || i_out >= O->nx) continue;	/* Outside our grid region */

			/* OK, this projected point falls inside the projected grid's rectangular domain */

			ij_in = (j_in + 2) * mx + i_in + 2;		/* ij allowing for boundary padding */
			ij_out = j_out * O->nx + i_out;			/* The output node */
			if (nz[ij_out] == 0) z_out[ij_out] = 0.0;	/* First time, override the initial value */
			if (nz[ij_out] < SHRT_MAX) {			/* Avoid overflow */
				z_out[ij_out] += z_in[ij_in];		/* Add up the z-sum inside this rect... */
				nz[ij_out]++;				/* ..and how many points there were */
			}
		}
	    }
	}

	/* PART 2: Create weighted average of interpolated and observed points */

	for (j_out = ij_out = 0; j_out < O->ny; j_out++) {	/* Loop over the output grid coordinates */
		if (GMT_IS_RECT_GRATICULE) y_proj = y_out_proj[j_out];
		for (i_out = 0; i_out < O->nx; i_out++, ij_out++) {
			if (GMT_IS_RECT_GRATICULE)
				x_proj = x_out_proj[i_out];
			else if (inverse)
				GMT_geo_to_xy (x_out[i_out], y_out[j_out], &x_proj, &y_proj);
			else {
				GMT_xy_to_geo (&x_proj, &y_proj, x_out[i_out], y_out[j_out]);
				if (project_info.projection == GMT_GENPER && project_info.g_outside) continue;	/* We are beyond the horizon */

				/* On 17-Sep-2007 the slack of GMT_SMALL was added to allow for round-off
				   errors in the grid limits. */
				if (GMT_io.in_col_type[0] == GMT_IS_LON && !GMT_is_dnan (x_proj)) {
					while (x_proj < I->x_min - GMT_SMALL) x_proj += 360.0;
					while (x_proj > I->x_max + GMT_SMALL) x_proj -= 360.0;
				}
			}

			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now find nearest node on the input grid */

			z_int = GMT_get_bcr_z (I, x_proj, y_proj, z_in, edgeinfo, &bcr);

			if (!antialias || nz[ij_out] < 2)
				z_out[ij_out] = (float)z_int;
			else if (GMT_is_dnan (z_int))
				z_out[ij_out] /= nz[ij_out];		/* Plain average */
			else {						/* Weighted average */
				inv_nz = 1.0 / nz[ij_out];
				z_out[ij_out] = (float) ((z_out[ij_out] + z_int * inv_nz) / (nz[ij_out] + inv_nz));
			}
		}
	}

	/* Time to clean up our mess */

	GMT_free ((void *)x_in);
	GMT_free ((void *)y_in);
	GMT_free ((void *)x_out);
	GMT_free ((void *)y_out);
	if (GMT_IS_RECT_GRATICULE) {
		GMT_free ((void *)x_in_proj);
		GMT_free ((void *)y_in_proj);
		GMT_free ((void *)x_out_proj);
		GMT_free ((void *)y_out_proj);
	}
	if (antialias) GMT_free ((void *)nz);

	return (GMT_NOERROR);
}

void GMT_2D_to_3D (double *x, double *y, double z, GMT_LONG n)
{
	GMT_LONG i;

	/* Convert from two-D to three-D coordinates */

	for (i = 0; i < n; i++) GMT_xy_do_z_to_xy (x[i], y[i], z, &x[i], &y[i]);
}

void GMT_2Dz_to_3D (double *x, double *y, double z, GMT_LONG n)
{
	GMT_LONG i;

	/* Convert from two-D to three-D coordinates */

	for (i = 0; i < n; i++) GMT_xyz_to_xy (x[i], y[i], z, &x[i], &y[i]);
}

void GMT_azim_to_angle (double lon, double lat, double c, double azim, double *angle)
	/* All variables in degrees */
{
	double lon1, lat1, x0, x1, y0, y1, dx, width, sinaz, cosaz;

	if (GMT_IS_LINEAR) {	/* Trivial case */
		*angle = 90.0 - azim;
		if (project_info.x_scale != project_info.y_scale) {
			sincosd (*angle, &sinaz, &cosaz);
			*angle = d_atan2d (sinaz * project_info.y_scale, cosaz * project_info.x_scale);
		}
		return;
	}

	/* Find second point c spherical degrees away in the azim direction */

	GMT_get_point_from_r_az (lon, lat, c, azim, &lon1, &lat1);

	/* Convert both points to x,y and get angle */

	GMT_geo_to_xy (lon, lat, &x0, &y0);
	GMT_geo_to_xy (lon1, lat1, &x1, &y1);

	/* Check for wrap-around */

	dx = x1 - x0;
	if ((fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_SMALL) && fabs (dx) > (width = GMT_half_map_width (y0))) {
		width *= 2.0;
		dx = copysign (width - fabs (dx), -dx);
		if (x1 < width)
			x0 -= width;
		else
			x0 += width;
	}
	*angle = d_atan2d (y1 - y0, x1 - x0);
}

void GMT_get_point_from_r_az (double lon0, double lat0, double r, double azim, double *lon1, double *lat1)
/* Given point (lon0, lat0), find coordinates of a point r degrees away in the azim direction */
{
	double sinr, cosr, sinaz, cosaz, siny, cosy;
	
	sincosd (azim, &sinaz, &cosaz);
	sincosd (r, &sinr, &cosr);
	sincosd (lat0, &siny, &cosy);

/*	*lon1 = lon0 + atand (sinr * sinaz / (cosy * cosr - siny * sinr * cosaz)); */
	*lon1 = lon0 + atan2d (sinr * sinaz, (cosy * cosr - siny * sinr * cosaz));
	*lat1 = d_asind (siny * cosr + cosy * sinr * cosaz);
}

void GMT_map_setinfo (double xmin, double xmax, double ymin, double ymax, double scl)
{	/* Set [and rescale] parameters */
	double factor = 1.0, w, h;

	if (GMT_world_map && GMT_IS_ZERO (xmax - xmin)) {	/* Safety valve for cases when w & e both project to the same side due to round-off */
		xmax = MAX (fabs (xmin), fabs (xmax));
		xmin =-xmax;
	}
	w = (xmax - xmin) * project_info.x_scale;
	h = (ymax - ymin) * project_info.y_scale;

	if (project_info.gave_map_width == 1)		/* Must rescale to given width */
		factor = scl / w;
	else if (project_info.gave_map_width == 2)	/* Must rescale to given height */
		factor = scl / h;
	else if (project_info.gave_map_width == 3)	/* Must rescale to max dimension */
		factor = scl / MAX (w, h);
	else if (project_info.gave_map_width == 4)	/* Must rescale to min dimension */
		factor = scl / MIN (w, h);
	project_info.x_scale *= factor;
	project_info.y_scale *= factor;
	project_info.w_r *= factor;

	if (project_info.g_debug > 1) {
		fprintf(stderr,"xmin %7.3f xmax %7.3f ymin %7.4f ymax %7.3f scale %6.3f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000, scl);
		fprintf(stderr,"gave_map_width %ld w %9.4e h %9.4e factor %9.4e\n", project_info.gave_map_width, w, h, factor);
	}

	GMT_map_setxy (xmin, xmax, ymin, ymax);
}

void GMT_map_setxy (double xmin, double xmax, double ymin, double ymax)
{	/* Set x/y parameters */

	project_info.xmax = (xmax - xmin) * project_info.x_scale;
	project_info.ymax = (ymax - ymin) * project_info.y_scale;
	project_info.x0 = -xmin * project_info.x_scale;
	project_info.y0 = -ymin * project_info.y_scale;
}

GMT_LONG GMT_map_clip_path (double **x, double **y, GMT_LONG *donut)
{
	/* This function returns a clip path corresponding to the
	 * extent of the map.
	 */

	double *work_x, *work_y, angle, da, r0, s, c, lon, lat;
	GMT_LONG i, j, np;

	*donut = FALSE;

	if (!project_info.region)	/* Rectangular map boundary */
		np = 4;
	else {
		switch (project_info.projection) {
			case GMT_LINEAR:
			case GMT_MERCATOR:
			case GMT_CYL_EQ:
			case GMT_CYL_EQDIST:
			case GMT_CYL_STEREO:
			case GMT_MILLER:
			case GMT_OBLIQUE_MERC:
				np = 4;
				break;
			case GMT_POLAR:
				if (project_info.got_elevations)
					*donut = (project_info.n < 90.0 && GMT_world_map);
				else
					*donut = (project_info.s > 0.0 && GMT_world_map);
				np = GMT_n_lon_nodes + 1;
				if (project_info.s > 0.0)	/* Need inside circle segment */
					np *= 2;
				else if (!GMT_world_map)	/* Need to include origin */
					np++;
				break;
			case GMT_GENPER:
			case GMT_STEREO:
			case GMT_LAMBERT:
			case GMT_LAMB_AZ_EQ:
			case GMT_ORTHO:
			case GMT_GNOMONIC:
			case GMT_AZ_EQDIST:
			case GMT_ALBERS:
			case GMT_ECONIC:
			case GMT_VANGRINTEN:
				np = (project_info.polar && (project_info.s <= -90.0 || project_info.n >= 90.0)) ? GMT_n_lon_nodes + 2: 2 * (GMT_n_lon_nodes + 1);
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				np = 2 * GMT_n_lat_nodes + 2;
				break;
			case GMT_WINKEL:
			case GMT_HAMMER:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				np = 2 * GMT_n_lat_nodes + 2;
				if (project_info.s != -90.0) np += GMT_n_lon_nodes - 1;
				if (project_info.n != 90.0) np += GMT_n_lon_nodes - 1;
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				np = 2 * (GMT_n_lon_nodes + GMT_n_lat_nodes);
				break;
			default:
				fprintf (stderr, "%s: Bad case in GMT_map_clip_path (%ld)\n", GMT_program, project_info.projection);
				np = 0;
				break;
		}
	}

	work_x = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");
	work_y = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");

	if (!project_info.region) {
		work_x[0] = work_x[3] = project_info.xmin;	work_y[0] = work_y[1] = project_info.ymin;
		work_x[1] = work_x[2] = project_info.xmax;	work_y[2] = work_y[3] = project_info.ymax;

	}
	else {
		switch (project_info.projection) {	/* Fill in clip path */
			case GMT_LINEAR:
			case GMT_MERCATOR:
			case GMT_CYL_EQ:
			case GMT_CYL_EQDIST:
			case GMT_CYL_STEREO:
			case GMT_MILLER:
			case GMT_OBLIQUE_MERC:
				work_x[0] = work_x[3] = project_info.xmin;	work_y[0] = work_y[1] = project_info.ymin;
				work_x[1] = work_x[2] = project_info.xmax;	work_y[2] = work_y[3] = project_info.ymax;
				break;
			case GMT_LAMBERT:
			case GMT_ALBERS:
			case GMT_ECONIC:
				for (i = j = 0; i <= GMT_n_lon_nodes; i++, j++) {
					lon = (i == GMT_n_lon_nodes) ? project_info.e : project_info.w + i * GMT_dlon;
					GMT_geo_to_xy (lon, project_info.s, &work_x[j], &work_y[j]);
				}
				for (i = 0; i <= GMT_n_lon_nodes; i++, j++) {
					lon = (i == GMT_n_lon_nodes) ? project_info.w : project_info.e - i * GMT_dlon;
					GMT_geo_to_xy (lon, project_info.n, &work_x[j], &work_y[j]);
				}
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				for (i = j = 0; i < GMT_n_lon_nodes; i++, j++)	/* South */
					GMT_geo_to_xy (project_info.w + i * GMT_dlon, project_info.s, &work_x[j], &work_y[j]);
				for (i = 0; i < GMT_n_lat_nodes; j++, i++)	/* East */
					GMT_geo_to_xy (project_info.e, project_info.s + i * GMT_dlat, &work_x[j], &work_y[j]);
				for (i = 0; i < GMT_n_lon_nodes; i++, j++)	/* North */
					GMT_geo_to_xy (project_info.e - i * GMT_dlon, project_info.n, &work_x[j], &work_y[j]);
				for (i = 0; i < GMT_n_lat_nodes; j++, i++)	/* West */
					GMT_geo_to_xy (project_info.w, project_info.n - i * GMT_dlat, &work_x[j], &work_y[j]);
				break;
			case GMT_POLAR:
				r0 = project_info.r * project_info.s / project_info.n;
				if (*donut) {
					np /= 2;
					da = TWO_PI / np;
					for (i = 0, j = 2*np-1; i < np; i++, j--) {	/* Draw outer clippath */
						angle = i * da;
						sincos (angle, &s, &c);
						work_x[i] = project_info.r * (1.0 + c);
						work_y[i] = project_info.r * (1.0 + s);
						/* Do inner clippath and put it at end of array */
						work_x[j] = project_info.r + r0 * c;
						work_y[j] = project_info.r + r0 * s;
					}
				}
				else {
					da = fabs (project_info.e - project_info.w) / GMT_n_lon_nodes;
					if (project_info.got_elevations) {
						for (i = j = 0; i <= GMT_n_lon_nodes; i++, j++)	/* Draw outer clippath */
							GMT_geo_to_xy (project_info.w + i * da, project_info.s, &work_x[j], &work_y[j]);
						for (i = GMT_n_lon_nodes; project_info.n < 90.0 && i >= 0; i--, j++)	/* Draw inner clippath */
							GMT_geo_to_xy (project_info.w + i * da, project_info.n, &work_x[j], &work_y[j]);
						if (GMT_IS_ZERO (90.0 - project_info.n) && !GMT_world_map)	/* Add origin */
							GMT_geo_to_xy (project_info.w, project_info.n, &work_x[j], &work_y[j]);
					}
					else {
						for (i = j = 0; i <= GMT_n_lon_nodes; i++, j++)	/* Draw outer clippath */
							GMT_geo_to_xy (project_info.w + i * da, project_info.n, &work_x[j], &work_y[j]);
						for (i = GMT_n_lon_nodes; project_info.s > 0.0 && i >= 0; i--, j++)	/* Draw inner clippath */
							GMT_geo_to_xy (project_info.w + i * da, project_info.s, &work_x[j], &work_y[j]);
						if (GMT_IS_ZERO (project_info.s) && !GMT_world_map)	/* Add origin */
							GMT_geo_to_xy (project_info.w, project_info.s, &work_x[j], &work_y[j]);
					}
				}
				break;
			case GMT_GENPER:
				GMT_genper_map_clip_path( np, work_x, work_y);
				break;
			case GMT_LAMB_AZ_EQ:
			case GMT_ORTHO:
			case GMT_GNOMONIC:
			case GMT_VANGRINTEN:
			case GMT_STEREO:
				if (project_info.polar) {
					j = 0;
					if (project_info.s > -90.0) {
						for (i = 0; i <= GMT_n_lon_nodes; i++, j++) {
							lon = (i == GMT_n_lon_nodes) ? project_info.e : project_info.w + i * GMT_dlon;
							GMT_geo_to_xy (lon, project_info.s, &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add S pole */
						GMT_geo_to_xy (project_info.w, -90.0, &work_x[j], &work_y[j]);
						j++;
					}
					if (project_info.n < 90.0) {
						for (i = 0; i <= GMT_n_lon_nodes; i++, j++) {
							lon = (i == GMT_n_lon_nodes) ? project_info.w : project_info.e - i * GMT_dlon;
							GMT_geo_to_xy (lon, project_info.n, &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add N pole */
						GMT_geo_to_xy (project_info.w, 90.0, &work_x[j], &work_y[j]);
						j++;
					}
				}
				else {
					da = TWO_PI / np;
					for (i = 0; i < np; i++) {
						angle = i * da;
						sincos (angle, &s, &c);
						work_x[i] = project_info.r * (1.0 + c);
						work_y[i] = project_info.r * (1.0 + s);
					}
				}
				break;
			case GMT_AZ_EQDIST:
				da = TWO_PI / np;
				for (i = 0; i < np; i++) {
					angle = i * da;
					sincos (angle, &s, &c);
					work_x[i] = project_info.r * (1.0 + c);
					work_y[i] = project_info.r * (1.0 + s);
				}
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				for (i = j = 0; i <= GMT_n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == GMT_n_lat_nodes) ? project_info.n : project_info.s + i * GMT_dlat;
					GMT_geo_to_xy (project_info.e, lat, &work_x[j], &work_y[j]);
				}
				for (i = GMT_n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == GMT_n_lat_nodes) ? project_info.n : project_info.s + i * GMT_dlat;
					GMT_geo_to_xy (project_info.w, lat, &work_x[j], &work_y[j]);
				}
				break;
			case GMT_HAMMER:
			case GMT_WINKEL:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				for (i = j = 0; i <= GMT_n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == GMT_n_lat_nodes) ? project_info.n : project_info.s + i * GMT_dlat;
					GMT_geo_to_xy (project_info.e, lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; project_info.n != 90.0 && i < GMT_n_lon_nodes; i++, j++)
					GMT_geo_to_xy (project_info.e - i * GMT_dlon, project_info.n, &work_x[j], &work_y[j]);
				for (i = GMT_n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == GMT_n_lat_nodes) ? project_info.n : project_info.s + i * GMT_dlat;
					GMT_geo_to_xy (project_info.w, lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; project_info.s != -90.0 && i < GMT_n_lon_nodes; i++, j++)
					GMT_geo_to_xy (project_info.w + i * GMT_dlon, project_info.s, &work_x[j], &work_y[j]);
				break;
		}
	}

	if (!(*donut)) np = GMT_compact_line (work_x, work_y, (GMT_LONG)np, FALSE, (int *)0);
	if (project_info.three_D) GMT_2D_to_3D (work_x, work_y, project_info.z_level, (GMT_LONG)np);

	*x = work_x;
	*y = work_y;

	return (np);
}

GMT_LONG GMT_quickconic (void)
{	/* Returns TRUE if area/scale are large/small enough
	 * so that we can use spherical equations with authalic
	 * or conformal latitudes instead of the full ellipsoidal
	 * equations.
	 */

	double s, dlon, width;

	if (project_info.gave_map_width) {	/* Gave width */
		dlon = project_info.e - project_info.w;
		width = project_info.pars[4] * GMT_u2u[gmtdefs.measure_unit][GMT_M];	/* Convert to meters */
		s = (dlon * project_info.M_PR_DEG) / width;
	}
	else if (project_info.units_pr_degree) {	/* Gave scale */
		/* Convert to meters */
		s = project_info.M_PR_DEG / (project_info.pars[4] * GMT_u2u[gmtdefs.measure_unit][GMT_M]);
	}
	else {	/* Got 1:xxx that was changed */
		s = (1.0 / project_info.pars[4]) / project_info.unit;
	}

	if (s > 1.0e7) {	/* if s in 1:s exceeds 1e7 we do the quick thing */
		if (gmtdefs.verbose) fprintf (stderr, "GMT Warning: Using spherical projection with conformal latitudes\n");
		return (TRUE);
	}
	else /* Use full ellipsoidal terms */
		return (FALSE);
}

GMT_LONG GMT_quicktm (double lon0, double limit)
{	/* Returns TRUE if the region chosen is too large for the
	 * ellipsoidal series to be valid; hence use spherical equations
	 * with authalic latitudes instead.
	 * We let +-limit degrees from central meridian be the cutoff.
	 */

	double d_left, d_right;

	d_left  = lon0 - project_info.w - 360.0;
	d_right = lon0 - project_info.e - 360.0;
	while (d_left  < -180.0) d_left  += 360.0;
	while (d_right < -180.0) d_right += 360.0;
	if (fabs (d_left) > limit || fabs (d_right) > limit) {
		if (gmtdefs.verbose) fprintf (stderr, "GMT Warning: Using spherical projection with authalic latitudes\n");
		return (TRUE);
	}
	else /* Use full ellipsoidal terms */
		return (FALSE);
}

double	GMT_lat_swap_quick (double lat, double c[])
{
	/* Return latitude, in degrees, given latitude, in degrees, based on coefficients c */

	double	delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (GMT_IS_ZERO (lat)) return (0.0);

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (c[0] + cos2phi * (c[1] + cos2phi * (c[2] + cos2phi * c[3])));

	return (lat + R2D * delta);
}

double	GMT_lat_swap (double lat, GMT_LONG itype)
{
	/* Return latitude, in degrees, given latitude, in degrees, based on itype */

	double	delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (GMT_IS_ZERO (lat)) return (0.0);

	if (project_info.GMT_lat_swap_vals.spherical) return (lat);

	if (itype < 0 || itype >= GMT_LATSWAP_N) {
		/* This should never happen -?- or do we want to allow the
			possibility of using itype = -1 to do nothing  */
		fprintf (stderr, "GMT_lat_swap():  Invalid choice.  (Programming bug.)\n");
		return(lat);
	}

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (project_info.GMT_lat_swap_vals.c[itype][0]
		+ cos2phi * (project_info.GMT_lat_swap_vals.c[itype][1]
		+ cos2phi * (project_info.GMT_lat_swap_vals.c[itype][2]
		+ cos2phi * project_info.GMT_lat_swap_vals.c[itype][3])));


	return (lat + R2D * delta);
}

void	GMT_lat_swap_init ()
{
	/* Initialize values in project_info.GMT_lat_swap_vals based on project_info.

	First compute project_info.GMT_lat_swap_vals.ra (and rm), the radii to use in
	spherical formulae for area (respectively, N-S distance) when
	using the authalic (respectively, meridional) latitude.

	Then for each type of swap:
	First load project_info.GMT_lat_swap_vals.c[itype][k], k=0,1,2,3 with the
	coefficient for sin(2 (k +1) phi), based on series from Adams.
	Next reshuffle these coefficients so they form a nested
	polynomial using equations (3-34) and (3-35) on page 19 of
	Snyder.

	References:  J. P. Snyder, "Map projections - a working manual",
	U. S. Geological Survey Professional Paper #1395, 1987.
	O. S. Adams, "Latitude Developments Connected With Geodesy and
	Cartography", U. S. Coast and Geodetic Survey Special Publication
	number 67, 1949.
	P. D. Thomas, "Conformal Projections in Geodesy and Cartography",
	US CGS Special Pub #251, 1952.
	See also other US CGS Special Pubs (#53, 57, 68, 193, and 251).

	Latitudes are named as follows (this only partly conforms to
	names in the literature, which are varied):

	Geodetic   = G, angle between ellipsoid normal and equator
	geocentric = O, angle between radius from Origin and equator
	Parametric = P, angle such that x=a*cos(phi), y=b*sin(phi) is on ellipse
	Authalic   = A, angle to use in equal area   development of ellipsoid
	Conformal  = C, angle to use in conformal    development of ellipsoid
	Meridional = M, angle to use in N-S distance calculation of ellipsoid

	(The parametric latitude is the one used in orthogonal curvilinear
	coordinates and ellipsoidal harmonics.  The term "authalic" was coined
	by A. Tissot in "Memoire sur la Representations des Surfaces et les
	projections des cartes geographiques", Gauthier Villars, Paris, 1881;
	it comes from the Greek meaning equal area.)

	The idea of latitude swaps is this:  Conformal, equal-area, and other
	developments of spherical surfaces usually lead to analytic formulae
	for the forward and inverse projections which are stable over a wide
	range of the values.  It is handy to use the same formulae when
	developing the surface of the ellipsoid.  The authalic (respectively,
	conformal) lat is such that when plugged into a spherical development
	formula, it results in an authalic (meaning equal area) (respectively,
	conformal) development of the ellipsoid.  The meridional lat does the
	same thing for measurement of N-S distances along a meridian.



	Adams gives coefficients for series in sin(2 (k +1) phi), for k
	up to 2 or 3.  I have extended these to k=3 in all cases except
	the authalic.  I have sometimes multiplied his coefficients by
	(-1) so that the sense here is always to give a correction to be
	added to the input lat to get the output lat in GMT_lat_swap().

	I have tested this code by checking that
		fabs(geocentric) < fabs(parametric) < fabs(geodetic)
	and also, for each pair of possible conversions, that the
	forward followed by the inverse returns the original lat to
	within a small tolerance.  This tolerance is as follows:

	geodetic <-> authalic:      max error (degrees) = 1.253344e-08
	geodetic <-> conformal:     max error (degrees) = 2.321796e-07  should be better after 13nov07 fix
	geodetic <-> meridional:    max error (degrees) = 4.490630e-12
	geodetic <-> geocentric:    max error (degrees) = 1.350031e-13
	geodetic <-> parametric:    max error (degrees) = 1.421085e-14
	geocentric <-> parametric:  max error (degrees) = 1.421085e-14


	Currently, (GMT v3.3) the only ones we anticipate using are
	geodetic, authalic, and conformal.  I have put others in here
	for possible future convenience.

	Also, I made this depend on gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid]
	rather than on project_info, so that it will be possible to
	call GMT_lat_swap() without having to pass -R and -J to
	GMT_map_setup(), so that in the future we will be able to use
	lat conversions without plotting maps.

	W H F Smith, 10--13 May 1999.   */

	double	x, xx[4], a, f, e2, e4, e6, e8;
	GMT_LONG	i;

	f = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening;
	a = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius;

	if (GMT_IS_ZERO (f)) {
		memset ((void *)project_info.GMT_lat_swap_vals.c, 0, (size_t)(GMT_LATSWAP_N * 4 * sizeof (double)));
		project_info.GMT_lat_swap_vals.ra = project_info.GMT_lat_swap_vals.rm = a;
		return;
	}

	/* Below are two sums for x to get the two radii.  I have nested the
	parentheses to add the terms in the order that would minimize roundoff
	error.  However, in double precision there may be no need to do this.
	I have carried these to 4 terms (eccentricity to the 8th power) because
	this is as far as Adams goes with anything, but it is not clear what
	the truncation error is, since every term in the sum has the same sign.  */

	e2 = f * (2.0 - f);
	e4 = e2 * e2;
	e6 = e4 * e2;
	e8 = e4 * e4;

	/* This expression for the Authalic radius comes from Adams [1949]  */
	xx[0] = 2.0 / 3.0;
	xx[1] = 3.0 / 5.0;
	xx[2] = 4.0 / 7.0;
	xx[3] = 5.0 / 9.0;
	x = xx[0] * e2 + ( xx[1] * e4 + ( xx[2] * e6 + xx[3] * e8));
	project_info.GMT_lat_swap_vals.ra = a * sqrt( (1.0 + x) * (1.0 - e2));

	/* This expression for the Meridional radius comes from Gradshteyn and Ryzhik, 8.114.1,
	because Adams only gets the first two terms.  This can be worked out by expressing the
	meridian arc length in terms of an integral in parametric latitude, which reduces to
	equatorial radius times Elliptic Integral of the Second Kind.  Expanding this using
	binomial theorem leads to Gradshteyn and Ryzhik's expression:  */
	xx[0] = 1.0 / 4.0;
	xx[1] = xx[0] * 3.0 / 16.0;
	xx[2] = xx[1] * 3.0 * 5.0 / 36.0;
	xx[3] = xx[2] * 5.0 * 7.0 / 64.0;
	x = xx[0] * e2 + ( xx[1] * e4 + ( xx[2] * e6 + xx[3] * e8));
	project_info.GMT_lat_swap_vals.rm = a * (1.0 - x);


	/* Geodetic to authalic:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][0] = -(e2 / 3.0 + (31.0 * e4 / 180.0 + 59.0 * e6 / 560.0));
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][1] = 17.0 * e4 / 360.0 + 61.0 * e6 / 1260;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][2] = -383.0 * e6 / 45360.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][3] = 0.0;

	/* Authalic to geodetic:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][0] = e2 / 3.0 + (31.0 * e4 / 180.0 + 517.0 * e6 / 5040.0);
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][1] = 23.0 * e4 / 360.0 + 251.0 * e6 / 3780;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][2] = 761.0 * e6 / 45360.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][3] = 0.0;

	/* Geodetic to conformal:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][0] = -(e2 / 2.0 + (5.0 * e4 / 24.0 + (3.0 * e6 / 32.0 + 281.0 * e8 / 5760.0)));
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][1] = 5.0 * e4 / 48.0 + (7.0 * e6 / 80.0 + 697.0 * e8 / 11520.0);
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][2] = -(13.0 * e6 / 480.0 + 461.0 * e8 / 13440.0);
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][3] = 1237.0 * e8 / 161280.0;

	/* Conformal to geodetic:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][0] = e2 / 2.0 + (5.0 * e4 / 24.0 + (e6 / 12.0 + 13.0 * e8 / 360.0)) ;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][1] = 7.0 * e4 / 48.0 + (29.0 * e6 / 240.0 + 811.0 * e8 / 11520.0);
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][2] = 7.0 * e6 / 120.0 + 81.0 * e8 / 1120.0;  /* Bug fixed 13nov07 whfs */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][3] = 4279.0 * e8 / 161280.0;


	/* The meridional and parametric developments use this parameter:  */
	x = f/(2.0 - f);		/* Adams calls this n.  It is f/(2-f), or -betaJK in my notes.  */
	xx[0] = x;			/* n  */
	xx[1] = x * x;			/* n-squared  */
	xx[2] = xx[1] * x;		/* n-cubed  */
	xx[3] = xx[2] * x;		/* n to the 4th  */

	/* Geodetic to meridional:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][0] = -(3.0 * xx[0] / 2.0 - 9.0 * xx[2] / 16.0);
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][1] = 15.0 * xx[1] / 16.0 - 15.0 * xx[3] / 32.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][2] = -35.0 * xx[2] / 48.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][3] = 315.0 * xx[3] / 512.0;

	/* Meridional to geodetic:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][0] = 3.0 * xx[0] / 2.0 - 27.0 * xx[2] / 32.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][1] = 21.0 * xx[1] / 16.0 - 55.0 * xx[3] / 32.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][2] = 151.0 * xx[2] / 96.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][3] = 1097.0 * xx[3] / 512.0;

	/* Geodetic to parametric equals parametric to geocentric:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][0] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][0] = -xx[0];
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][1] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][1] = xx[1] / 2.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][2] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][2] = -xx[2] / 3.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][3] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][3] = xx[3] / 4.0;

	/* Parametric to geodetic equals geocentric to parametric:  */
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][0] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][0] = xx[0];
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][1] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][1] = xx[1] / 2.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][2] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][2] = xx[2] / 3.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][3] = project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][3] = xx[3] / 4.0;


	/* The geodetic <->geocentric use this parameter:  */
	x = 1.0 - e2;
	x = (1.0 - x)/(1.0 + x);	/* Adams calls this m.  It is e2/(2-e2), or -betaJK in my notes.  */
	xx[0] = x;			/* m  */
	xx[1] = x * x;			/* m-squared  */
	xx[2] = xx[1] * x;		/* m-cubed  */
	xx[3] = xx[2] * x;		/* m to the 4th  */

	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][0] = -xx[0];
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][1] = xx[1] / 2.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][2] = -xx[2] / 3.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][3] = xx[3] / 4.0;

	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][0] = xx[0];
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][1] = xx[1] / 2.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][2] = xx[2] / 3.0;
	project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][3] = xx[3] / 4.0;


	/* Now do the Snyder Shuffle:  */
	for (i = 0; i < GMT_LATSWAP_N; i++) {
		project_info.GMT_lat_swap_vals.c[i][0] = project_info.GMT_lat_swap_vals.c[i][0] - project_info.GMT_lat_swap_vals.c[i][2];
		project_info.GMT_lat_swap_vals.c[i][1] = 2.0 * project_info.GMT_lat_swap_vals.c[i][1] - 4.0 * project_info.GMT_lat_swap_vals.c[i][3];
		project_info.GMT_lat_swap_vals.c[i][2] *= 4.0;
		project_info.GMT_lat_swap_vals.c[i][3] *= 8.0;
	}

	return;
}

void GMT_scale_eqrad ()
{
	/* Reinitialize project_info.EQ_RAD to the appropriate value */

	switch (project_info.projection) {

		/* Conformal projections */

		case GMT_MERCATOR:
		case GMT_TM:
		case GMT_UTM:
		case GMT_OBLIQUE_MERC:
		case GMT_LAMBERT:
		case GMT_STEREO:

			project_info.EQ_RAD = project_info.GMT_lat_swap_vals.rm;
			break;

		/* Equal Area projections */

		case GMT_LAMB_AZ_EQ:
		case GMT_ALBERS:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:

			project_info.EQ_RAD = project_info.GMT_lat_swap_vals.ra;
			break;

		default:	/* Keep EQ_RAD as is */
			break;
	}

	/* Also reset dependencies of EQ_RAD */

	project_info.i_EQ_RAD = 1.0 / project_info.EQ_RAD;
	project_info.M_PR_DEG = TWO_PI * project_info.EQ_RAD / 360.0;
	project_info.KM_PR_DEG = 0.001 * project_info.M_PR_DEG;
}


/* Compute mean radius r = (2a + b)/3 = a (1 - f/3) */
#define GMT_mean_radius(a, f) (a * (1.0 - f / 3.0))

void GMT_init_ellipsoid (void)
{
	double f;

	/* Set up ellipsoid parameters for the selected ellipsoid since .gmtdefaults could have changed them */

	f = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening;
	project_info.ECC2 = 2.0 * f - f * f;
	project_info.ECC4 = project_info.ECC2 * project_info.ECC2;
	project_info.ECC6 = project_info.ECC2 * project_info.ECC4;
	project_info.one_m_ECC2 = 1.0 - project_info.ECC2;
	project_info.i_one_m_ECC2 = 1.0 / project_info.one_m_ECC2;
	project_info.ECC = d_sqrt (project_info.ECC2);
	project_info.half_ECC = 0.5 * project_info.ECC;
	project_info.i_half_ECC = 0.5 / project_info.ECC;
	project_info.EQ_RAD = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius;
	project_info.i_EQ_RAD = 1.0 / project_info.EQ_RAD;

	/* Spherical degrees to m or km */
	project_info.M_PR_DEG = TWO_PI * GMT_mean_radius (project_info.EQ_RAD, f) / 360.0;
	project_info.KM_PR_DEG = 0.001 * project_info.M_PR_DEG;
}

void GMT_set_spherical (void)
{
	/* Set up ellipsoid parameters using spherical approximation */

	gmtdefs.ref_ellipsoid[GMT_N_ELLIPSOIDS - 1].eq_radius =
		GMT_mean_radius (gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius, gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening);
	gmtdefs.ellipsoid = GMT_N_ELLIPSOIDS - 1;	/* Custom ellipsoid */
	gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening = 0.0;
	if (gmtdefs.verbose) fprintf (stderr, "%s: Warning: spherical approximation used!\n", GMT_program);

	GMT_init_ellipsoid ();
}

void GMT_set_polar (double plat)
{
	/* Determines if the projection pole is N or S pole */

	if (GMT_IS_ZERO (fabs (plat) - 90.0)) {
		project_info.polar = TRUE;
		project_info.north_pole	= (plat > 0.0);
	}
}

/* Datum conversion routines */

void GMT_datum_init (struct GMT_DATUM *from, struct GMT_DATUM *to, GMT_LONG heights)
{
	/* Initialize datum conv structures based on the parsed values*/

	GMT_LONG k;

	GMT_datum.h_given = heights;

	memcpy ((void *)&GMT_datum.from, (void *)from, sizeof (struct GMT_DATUM));
	memcpy ((void *)&GMT_datum.to,   (void *)to,   sizeof (struct GMT_DATUM));

	GMT_datum.da = GMT_datum.to.a - GMT_datum.from.a;
	GMT_datum.df = GMT_datum.to.f - GMT_datum.from.f;
	for (k = 0; k < 3; k++) GMT_datum.dxyz[k] = -(GMT_datum.to.xyz[k] - GMT_datum.from.xyz[k]);	/* Since the X, Y, Z are Deltas relative to WGS-84 */
	GMT_datum.one_minus_f = 1.0 - GMT_datum.from.f;
}

void GMT_ECEF_init (struct GMT_DATUM *D)
{
	/* Duplicate the parsed datum to the GMT from datum */

	memcpy ((void *)&GMT_datum.from, (void *)D, sizeof (struct GMT_DATUM));
}

GMT_LONG GMT_set_datum (char *text, struct GMT_DATUM *D)
{
	GMT_LONG i;
	double t;

	if (text[0] == '\0' || text[0] == '-') {	/* Shortcut for WGS-84 */
		memset ((void *)D->xyz, 0, (size_t)(3 * sizeof (double)));
		D->a = gmtdefs.ref_ellipsoid[0].eq_radius;
		D->f = gmtdefs.ref_ellipsoid[0].flattening;
		D->ellipsoid_id = 0;
	}
	else if (strchr (text, ':')) {	/* Has colons, must get ellipsoid and dr separately */
		char ellipsoid[GMT_LONG_TEXT], dr[GMT_LONG_TEXT];
		if (sscanf (text, "%[^:]:%s", ellipsoid, dr) != 2) {
			fprintf (stderr, "%s: Malformed <ellipsoid>:<dr> argument!\n", GMT_program);
			return (-1);
		}
		if (sscanf (dr, "%lf,%lf,%lf", &D->xyz[0], &D->xyz[1], &D->xyz[2]) != 3) {
			fprintf (stderr, "%s: Malformed <x>,<y>,<z> argument!\n", GMT_program);
			return (-1);
		}
		if ((i = GMT_get_ellipsoid (ellipsoid)) >= 0) {	/* This includes looking for format <a>,<1/f> */
			D->a = gmtdefs.ref_ellipsoid[i].eq_radius;
			D->f = gmtdefs.ref_ellipsoid[i].flattening;
			D->ellipsoid_id = i;
		}
		else {
			fprintf (stderr, "%s: Ellipsoid %s not recognized!\n", GMT_program, ellipsoid);
			return (-1);
		}
	}
	else {		/* Gave a Datum ID tag [ 0-(GMT_N_DATUMS-1)] */
		GMT_LONG k;
		if (sscanf (text, "%" GMT_LL "d", &i) != 1) {
			fprintf (stderr, "%s: Malformed or unrecognized <datum> argument (%s)!\n", GMT_program, text);
			return (-1);
		}
		if (i < 0 || i >= GMT_N_DATUMS) {
			fprintf (stderr, "%s: Datum ID (%ld) outside valid range (0-%d)!\n", GMT_program, i, GMT_N_DATUMS-1);
			return (-1);
		}
		if ((k = GMT_get_ellipsoid (gmtdefs.datum[i].ellipsoid)) < 0) {	/* This should not happen... */
			fprintf (stderr, "%s: Ellipsoid %s not recognized!\n", GMT_program, gmtdefs.datum[i].ellipsoid);
			return (-1);
		}
		D->a = gmtdefs.ref_ellipsoid[k].eq_radius;
		D->f = gmtdefs.ref_ellipsoid[k].flattening;
		D->ellipsoid_id = k;
		for (k = 0; k< 3; k++) D->xyz[k] = gmtdefs.datum[i].xyz[k];
	}
	D->b = D->a * (1 - D->f);
	D->e_squared = 2 * D->f - D->f * D->f;
	t = D->a /D->b;
	D->ep_squared = t * t - 1.0;	/* (a^2 - b^2)/a^2 */
	return 0;
}

void GMT_conv_datum (double in[], double out[])
{
	/* Evaluate J^-1 and B on from ellipsoid */
	/* Based on Standard Molodensky Datum Conversion, implemented from
	 * http://www.colorado.edu/geography/gcraft/notes/datum/gif/molodens.gif */

	double sin_lon, cos_lon, sin_lat, cos_lat, sin_lat2, M, N, h, tmp_1, tmp_2, tmp_3;
	double delta_lat, delta_lon, delta_h, sc_lat;

	h = (GMT_datum.h_given) ? in[2] : 0.0;
	sincosd (in[0], &sin_lon, &cos_lon);
	sincosd (in[1], &sin_lat, &cos_lat);
	sin_lat2 = sin_lat * sin_lat;
	sc_lat = sin_lat * cos_lat;
	M = GMT_datum.from.a * (1.0 - GMT_datum.from.e_squared) / pow (1.0 - GMT_datum.from.e_squared * sin_lat2, 1.5);
	N = GMT_datum.from.a / sqrt (1.0 - GMT_datum.from.e_squared * sin_lat2);

	tmp_1 = -GMT_datum.dxyz[0] * sin_lat * cos_lon - GMT_datum.dxyz[1] * sin_lat * sin_lon + GMT_datum.dxyz[2] * cos_lat;
	tmp_2 = GMT_datum.da * (N * GMT_datum.from.e_squared * sc_lat) / GMT_datum.from.a;
	tmp_3 = GMT_datum.df * (M / GMT_datum.one_minus_f + N * GMT_datum.one_minus_f) * sc_lat;
	delta_lat = (tmp_1 + tmp_2 + tmp_3) / (M + h);

	delta_lon = (-GMT_datum.dxyz[0] * sin_lon + GMT_datum.dxyz[1] * cos_lon) / ((N + h) * cos_lat);

	tmp_1 = GMT_datum.dxyz[0] * cos_lat * cos_lon + GMT_datum.dxyz[1] * cos_lat * sin_lon + GMT_datum.dxyz[2] * sin_lat;
	tmp_2 = -GMT_datum.da * GMT_datum.from.a / N;
	tmp_3 = GMT_datum.df * GMT_datum.one_minus_f * N * sin_lat2;
	delta_h = tmp_1 + tmp_2 + tmp_3;

	out[0] = in[0] + delta_lon * R2D;
	out[1] = in[1] + delta_lat * R2D;
	if (GMT_datum.h_given) out[2] = in[2] + delta_h;
}

void GMT_ECEF_forward (double in[], double out[])
{
	/* Convert geodetic lon, lat, height to ECEF coordinates given the datum parameters.
	 * GMT_datum.from is always the ellipsoid to use */

	double sin_lon, cos_lon, sin_lat, cos_lat, N, tmp;

	sincosd (in[0], &sin_lon, &cos_lon);
	sincosd (in[1], &sin_lat, &cos_lat);

	N = GMT_datum.from.a / d_sqrt (1.0 - GMT_datum.from.e_squared * sin_lat * sin_lat);
	tmp = (N + in[2]) * cos_lat;
	out[0] = tmp * cos_lon + GMT_datum.from.xyz[0];
	out[1] = tmp * sin_lon + GMT_datum.from.xyz[1];
	out[2] = (N * (1 - GMT_datum.from.e_squared) + in[2]) * sin_lat + GMT_datum.from.xyz[2];
}

void GMT_ECEF_inverse (double in[], double out[])
{
	/* Convert ECEF coordinates to geodetic lon, lat, height given the datum parameters.
	 * GMT_datum.from is always the ellipsoid to use */

	double in_p[3], sin_lat, cos_lat, N, p, theta, sin_theta, cos_theta;
	GMT_LONG i;

	/* First remove the xyz shifts, us in_p to avoid changing in */

	for (i = 0; i < 3; i++) in_p[i] = in[i] - GMT_datum.from.xyz[i];

	p = hypot (in_p[0], in_p[1]);
	theta = atan (in_p[2] * GMT_datum.from.a / (p * GMT_datum.from.b));
	sincos (theta, &sin_theta, &cos_theta);
	out[0] = d_atan2d (in_p[1], in_p[0]);
	out[1] = atand ((in_p[2] + GMT_datum.from.ep_squared * GMT_datum.from.b * pow (sin_theta, 3.0)) / (p - GMT_datum.from.e_squared * GMT_datum.from.a * pow (cos_theta, 3.0)));
	sincosd (out[1], &sin_lat, &cos_lat);
	N = GMT_datum.from.a / sqrt (1.0 - GMT_datum.from.e_squared * sin_lat * sin_lat);
	out[2] = (p / cos_lat) - N;
}

double GMT_az_backaz_cartesian (double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	/* Calculate azimuths or backazimuths.  Cartesian case.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy;

	if (baz) {	/* exchange point one and two */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	dx = lonE - lonS;
	dy = latE - latS;
	az = (dx == 0.0 && dy == 0.0) ? GMT_d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_flatearth (double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	/* Calculate azimuths or backazimuths.  Flat earth code.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy, dlon;

	if (baz) {	/* exchange point one and two */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	dlon = lonE - lonS;
	if (fabs (dlon) > 180.0) dlon = copysign ((360.0 - fabs (dlon)), dlon);
	dx = dlon * cosd (0.5 * (latE + latS));
	dy = latE - latS;
	az = (dx == 0.0 && dy == 0.0) ? GMT_d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_sphere (double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	/* Calculate azimuths or backazimuths.  Spherical code.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, sin_yS, cos_yS, sin_yE, cos_yE, sin_dlon, cos_dlon;

	if (baz) {	/* exchange point one and two */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	sincosd (latS, &sin_yS, &cos_yS);
	sincosd (latE, &sin_yE, &cos_yE);
	sincosd (lonS - lonE, &sin_dlon, &cos_dlon);
	az = atan2d (cos_yS * sin_dlon, cos_yE * sin_yS - sin_yE * cos_yS * cos_dlon);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_geodesic (double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	/* Calculate azimuths or backazimuths for geodesics using geocentric latitudes.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, a, b, c, d, e, f, g, h, a1, b1, c1, d1, e1, f1, g1, h1, thg, ss, sc;

	/* Equations are unstable for latitudes of exactly 0 degrees. */
	if (latE == 0.0) latE = 1.0e-08;
	if (latS == 0.0) latS = 1.0e-08;

	/* Must convert from geographic to geocentric coordinates in order
	 * to use the spherical trig equations.  This requires a latitude
	 * correction given by: 1-ECC2=1-2*f + f*f = project_info.one_m_ECC2
	 */

	thg = atan (project_info.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;
	g = -c * e;
	h = c * d;

	/* Calculating some trig constants. */

	thg = atan (project_info.one_m_ECC2 * tand (latS));
	sincos (thg, &c1, &f1);		f1 = -f1;
	sincosd (lonS, &d1, &e1);	e1 = -e1;
	a1 = f1 * e1;
	b1 = -f1 * d1;
	g1 = -c1 * e1;
	h1 = c1 * d1;

	/* Spherical trig relationships used to compute angles. */

	if (baz) {	/* Get Backazimuth */
		ss = pow(a-d1,2.0) + pow(b-e1,2.0) + c * c - 2.0;
		sc = pow(a-g1,2.0) + pow(b-h1,2.0) + pow(c-f1,2.0) - 2.0;
	}
	else {		/* Get Azimuth */
		ss = pow(a1-d, 2.0) + pow(b1-e, 2.0) + c1 * c1 - 2.0;
		sc = pow(a1-g, 2.0) + pow(b1-h, 2.0) + pow(c1-f, 2.0) - 2.0;
	}
	az = atan2d (ss,sc);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_geodesic_dist_degree (double lonS, double latS, double lonE, double latE)
{
	/* Compute the great circle arc length in degrees on an ellipsoidal
	 * Earth.  We do this by converting to geocentric coordinates.
	 */

	double a, b, c, d, e, f, a1, b1, c1, d1, e1, f1, thg, sc, sd, dist;

	/* Equations are unstable for latitudes of exactly 0 degrees. */

	if (latE == 0.0) latE = 1.0e-08;
	if (latS == 0.0) latS = 1.0e-08;

	/* Must convert from geographic to geocentric coordinates in order
	 * to use the spherical trig equations.  This requires a latitude
	 * correction given by: 1-ECC2=1-2*F + F*F = project_info.one_m_ECC2
	 */

	thg = atan (project_info.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;

	/* Calculate some trig constants. */

	thg = atan (project_info.one_m_ECC2 * tand (latS));
	sincos (thg, &c1, &f1);		f1 = -f1;
	sincosd (lonS, &d1, &e1);	e1 = -e1;
	a1 = f1 * e1;
	b1 = -f1 * d1;

	/* Spherical trig relationships used to compute angles. */

	sc = a * a1 + b * b1 + c * c1;
	sd = 0.5 * sqrt ((pow(a-a1,2.0) + pow(b-b1,2.0) + pow(c-c1,2.0)) * (pow(a+a1,2.0) + pow(b+b1, 2.0) + pow(c+c1, 2.0)));
	dist = atan2d (sd, sc);
	if (dist < 0.0) dist += 360.0;

	return (dist);
}

double GMT_geodesic_dist_cos (double lonS, double latS, double lonE, double latE)
{	/* Convenience function to get cosine instead */
	return (cosd (GMT_geodesic_dist_degree (lonS, latS, lonE, latE)));
}

double GMT_geodesic_dist_meter (double lonS, double latS, double lonE, double latE)
{
	/* Compute length of geodesic between locations in meters
	 * We use Rudoe's equation from Bomford.
	 */

	double e1, el, sinthi, costhi, sinthk, costhk, tanthi, tanthk, sina12, cosa12;
	double al, a12top, a12bot, a12, e2, e3, c0, c2, c4, v1, v2, z1, z2, x2, y2, dist;
	double e1p1, sqrte1p1, sin_dl, cos_dl, u1bot, u1, u2top, u2bot, u2, b0, du, pdist;


	/* Equations are unstable for latitudes of exactly 0 degrees. */

	if (latE == 0.0) latE = 1.0e-08;
	if (latS == 0.0) latS = 1.0e-08;

	/* Now compute the distance between the two points using Rudoe's
	 * formula given in Bomford's GEODESY, section 2.15(b).
	 * (Unclear if it is 1971 or 1980 edition)
	 * (There is some numerical problem with the following formulae.
	 * If the station is in the southern hemisphere and the event in
	 * in the northern, these equations give the longer, not the
	 * shorter distance between the two locations.  Since the equations
	 * are fairly messy, the simplist solution is to reverse the
	 * meanings of the two locations for this case.)
	 */

	if (latS < 0.0) {	/* Station in southern hemisphere, swap */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	el = project_info.ECC2 / project_info.one_m_ECC2;
	e1 = 1.0 + el;
	sincosd (latE, &sinthi, &costhi);
	sincosd (latS, &sinthk, &costhk);
	sincosd (lonE - lonS, &sin_dl, &cos_dl);
	tanthi = sinthi / costhi;
	tanthk = sinthk / costhk;
	al = tanthi / (e1 * tanthk) + project_info.ECC2 * sqrt((e1 + tanthi * tanthi) / (e1 + tanthk * tanthk));
	a12top = sin_dl;
	a12bot = (al - cos_dl) * sinthk;
	a12 = atan2 (a12top,a12bot);
	sincos (a12, &sina12, &cosa12);
	e1 = el * (pow(costhk * cosa12,2.0) + sinthk * sinthk);
	e2 = e1 * e1;
	e3 = e1 * e2;
	c0 = 1.0 + 0.25 * e1 - (3.0 / 64.0) * e2 + (5.0 / 256.0) * e3;
	c2 = -0.125 * e1 + (1.0 / 32) * e2 - (15.0 / 1024.0) * e3;
	c4 = -(1.0 / 256.0) * e2 + (3.0 / 1024.0) * e3;
	v1 = project_info.EQ_RAD / sqrt(1.0 - project_info.ECC2 * sinthk * sinthk);
	v2 = project_info.EQ_RAD / sqrt(1.0 - project_info.ECC2 * sinthi * sinthi);
	z1 = v1 * (1.0 - project_info.ECC2) * sinthk;
	z2 = v2 * (1.0 - project_info.ECC2) * sinthi;
	x2 = v2 * costhi * cos_dl;
	y2 = v2 * costhi * sin_dl;
	e1p1 = e1 + 1.0;
	sqrte1p1 = sqrt (e1p1);
	u1bot = sqrte1p1 * cosa12;
	u1 = atan2 (tanthk, u1bot);
	u2top = v1 * sinthk + e1p1 * (z2 - z1);
	u2bot = sqrte1p1 * (x2 * cosa12 - y2 * sinthk * sina12);
	u2 = atan2 (u2top, u2bot);
	b0 = v1 * sqrt (1.0 + el * pow (costhk * cosa12, 2.0)) / e1p1;
	du = u2  - u1;
	if (fabs (du) > M_PI) du = copysign (TWO_PI - fabs (du), du);
	pdist = b0 * (c2 * (sin (2.0 * u2) - sin(2.0 * u1)) + c4 * (sin (4.0 * u2) - sin (4.0 * u1)));
	dist = fabs (b0 * c0 * du + pdist);

	return (dist);
}

double GMT_geodesic_dist_km (double lonS, double latS, double lonE, double latE)
{
	return (0.001 * GMT_geodesic_dist_meter (lonS, latS, lonE, latE));
}

GMT_LONG GMT_distances (double x[], double y[], GMT_LONG n, double scale, GMT_LONG dist_flag, double **dist)
{	/* Returns distances in meter; use scale to get other units */
	GMT_LONG this, prev;
	GMT_LONG cumulative = TRUE, do_scale, xy_not_NaN;
	double *d, cum_dist = 0.0, inc = 0.0;

	if (dist_flag < 0) {	/* Want increments and not cumulative distances */
		dist_flag = GMT_abs (dist_flag);
		cumulative = FALSE;
	}

	if (dist_flag < 0 || dist_flag > 3) return (GMT_MAP_BAD_DIST_FLAG);

	do_scale = (scale != 1.0);
	d = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_distances");

	for (this = 0, prev = -1; this < n; this++) {
		xy_not_NaN = !(GMT_is_dnan (x[this]) || GMT_is_dnan (y[this]));
		if (xy_not_NaN && prev >= 0) {	/* safe to calculate inc */
			switch (dist_flag) {

				case 0:	/* Cartesian distances */

					inc = hypot (x[this] - x[prev], y[this] - y[prev]);
					break;

				case 1:	/* Flat earth distances in meter */

					inc = GMT_flatearth_dist_meter (x[this], y[this], x[prev], y[prev]);
					break;

				case 2:	/* Great circle distances in meter */

					inc = GMT_great_circle_dist_meter (x[this], y[this], x[prev], y[prev]);
					break;

				case 3:	/* Geodesic distances in meter */

					inc = GMT_geodesic_dist_meter (x[this], y[this], x[prev], y[prev]);
					break;
			}

			if (do_scale) inc *= scale;
			if (cumulative) cum_dist += inc;
			d[this] = (cumulative) ? cum_dist : inc;
		}
		else if (this > 0)
			d[this] = GMT_d_NaN;

		if (xy_not_NaN) prev = this;	/* This was a record with OK x,y; make it the previous point for distance calculations */
	}
	*dist = d;
	return (GMT_NOERROR);
}

GMT_LONG GMT_map_latcross (double lat, double west, double east, struct GMT_XINGS **xings)
{
	GMT_LONG i, go = FALSE, nx, nc = 0, n_alloc = GMT_SMALL_CHUNK;
	double lon, lon_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	double GMT_get_angle (double lon1, double lat1, double lon2, double lat2);
	struct GMT_XINGS *X;

	X = (struct GMT_XINGS *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct GMT_XINGS), "GMT_map_latcross");

	lon_old = west - 2.0 * GMT_SMALL;
	GMT_map_outside (lon_old, lat);
	GMT_geo_to_xy (lon_old, lat, &last_x, &last_y);
	for (i = 1; i <= GMT_n_lon_nodes; i++) {
		lon = (i == GMT_n_lon_nodes) ? east + 2.0 * GMT_SMALL : west + i * GMT_dlon;
		GMT_map_outside (lon, lat);
		GMT_geo_to_xy (lon, lat, &this_x, &this_y);
		if ((nx = GMT_map_crossing (lon_old, lat, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (lon_old, lat, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (GMT_corner > 0) {
				X[nc].sides[0] = (GMT_corner%4 > 1) ? 1 : 3;
				if (project_info.got_azimuths) X[nc].sides[0] = (X[nc].sides[0] + 2) % 4;
				GMT_corner = 0;
			}
		}
		else if (GMT_world_map)
			nx = (*GMT_wrap_around_check) (X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT_map_width) < GMT_SMALL && !GMT_world_map)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_SMALL && (gap - GMT_map_height) < GMT_SMALL && !GMT_world_map_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = (struct GMT_XINGS *) GMT_memory ((void *)X, (size_t)n_alloc, sizeof (struct GMT_XINGS), "GMT_map_latcross");
			}
			go = FALSE;
		}
		lon_old = lon;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = (struct GMT_XINGS *) GMT_memory ((void *)X, (size_t)nc, sizeof (struct GMT_XINGS), "GMT_map_latcross");
		*xings = X;
	}
	else
		GMT_free ((void *)X);

	return (nc);
}

GMT_LONG GMT_map_loncross (double lon, double south, double north, struct GMT_XINGS **xings)
{
	GMT_LONG go = FALSE, j, nx, nc = 0, n_alloc = GMT_SMALL_CHUNK;
	double lat, lat_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	double GMT_get_angle (double lon1, double lat1, double lon2, double lat2);
	struct GMT_XINGS *X;

	X = (struct GMT_XINGS *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct GMT_XINGS), "GMT_map_loncross");

	lat_old = ((south - (2.0 * GMT_SMALL)) >= -90.0) ? south - 2.0 * GMT_SMALL : south;	/* Outside */
	if ((north + 2.0 * GMT_SMALL) <= 90.0) north += 2.0 * GMT_SMALL;
	GMT_map_outside (lon, lat_old);
	GMT_geo_to_xy (lon, lat_old, &last_x, &last_y);
	for (j = 1; j <= GMT_n_lat_nodes; j++) {
		lat = (j == GMT_n_lat_nodes) ? north: south + j * GMT_dlat;
		GMT_map_outside (lon, lat);
		GMT_geo_to_xy (lon, lat, &this_x, &this_y);
		if ((nx = GMT_map_crossing (lon, lat_old, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (lon, lat_old, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (GMT_corner > 0) {
				X[nc].sides[0] = (GMT_corner < 3) ? 0 : 2;
				GMT_corner = 0;
			}
		}
		else if (GMT_world_map)
			nx = (*GMT_wrap_around_check) (X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT_map_width) < GMT_SMALL && !GMT_world_map)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_SMALL && (gap - GMT_map_height) < GMT_SMALL && !GMT_world_map_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = (struct GMT_XINGS *) GMT_memory ((void *)X, (size_t)n_alloc, sizeof (struct GMT_XINGS), "GMT_map_loncross");
			}
			go = FALSE;
		}
		lat_old = lat;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = (struct GMT_XINGS *) GMT_memory ((void *)X, (size_t)nc, sizeof (struct GMT_XINGS), "GMT_map_loncross");
		*xings = X;
	}
	else
		GMT_free ((void *)X);

	return (nc);
}

