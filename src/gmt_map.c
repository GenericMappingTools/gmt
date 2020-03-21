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
 * The ellipsoid used is selectable by editing the gmt.conf in your
 * home directory.  If no such file, create one by running gmtdefaults.
 *
 * Usage: Initialize system by calling gmt_g (separate module), and
 * then just use gmt_geo_to_xy() and gmt_xy_to_geo() functions.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 */

/*!
 * \file gmt_map.c
 * \brief gmt_map.c contains code related to generic coordinate transformation.
 *
 * PUBLIC GMT Functions include:
 *
 *	gmt_azim_to_angle :	Converts azimuth to angle on the map\n
 *	gmt_clip_to_map :	Force polygon points to be inside map\n
 *	gmt_compact_line :	Remove redundant pen movements\n
 *	gmt_geo_to_xy :		Generic lon/lat to x/y\n
 *	gmt_geo_to_xy_line :	Same for polygons\n
 *	gmt_geoz_to_xy :	Generic 3-D lon/lat/z to x/y\n
 *	gmt_grd_project :	Generalized grid projection with interpolation\n
 *	GMT_great_circle_dist :	Returns great circle distance in degrees\n
 *	gmt_img_project :	Generalized image projection with interpolation\n
 *	gmt_map_outside :	Generic function determines if we're outside map boundary\n
 *	gmtlib_map_path :		Return gmtlib_latpath or gmtlib_lonpath\n
 *	gmt_map_setup :		Initialize map projection\n
 *	gmt_project_init :	Initialize parameters for grid/image transformations\n
 *	gmt_xy_to_geo :		Generic inverse x/y to lon/lat projection\n
 *	gmt_xyz_to_xy :		Generic xyz to xy projection\n
 *
 * Internal GMT Functions include:
 *
 *	map_get_origin :		Find origin of projection based on pole and 2nd point\n
 *	map_get_rotate_pole :		Find rotation pole based on two points on great circle\n
 *	map_ilinearxy :			Inverse linear projection\n
 *	map_init_three_D :		Initializes parameters needed for 3-D plots\n
 *	map_crossing :		Generic function finds crossings between line and map boundary\n
 *	gmtlib_latpath :			Return path between 2 points of equal latitude\n
 *	gmtlib_lonpath :			Return path between 2 points of equal longitude\n
 *	map_radial_crossing :		Determine map crossing in the Lambert azimuthal equal area projection\n
 *	gmtmap_left_boundary :		Return left boundary in x-inches\n
 *	map_linearxy :			Linear xy projection\n
 *	map_lon_inside :		Accounts for wrap-around in longitudes and checks for inside\n
 *	map_ellipse_crossing :		Find map crossings in the Mollweide projection\n
 *	map_move_to_rect :		Move an outside point straight in to nearest edge\n
 *	map_polar_outside :		Determines if a point is outside polar projection region\n
 *	map_pole_rotate_forward :	Compute positions from oblique coordinates\n
 *	map_radial_clip :		Clip path outside radial region\n
 *	map_radial_outside :		Determine if point is outside radial region\n
 *	map_radial_overlap :		Determine overlap, always true for his projection\n
 *	map_rect_clip :			Clip to rectangular region\n
 *	map_rect_crossing :		Find crossing between line and rect region\n
 *	map_rect_outside :		Determine if point is outside rect region\n
 *	map_rect_outside2 :		Determine if point is outside rect region (azimuthal proj only)\n
 *	map_rect_overlap :		Determine overlap between rect regions\n
 *	gmtmap_right_boundary :		Return x value of right map boundary\n
 *	map_xy_search :			Find xy map boundary\n
 *	map_wesn_clip:			Clip polygon to wesn boundaries\n
 *	map_wesn_crossing :		Find crossing between line and lon/lat rectangle\n
 *	map_wesn_outside :		Determine if a point is outside a lon/lat rectangle\n
 *	map_wesn_overlap :		Determine overlap between lon/lat rectangles\n
 *	gmt_wesn_search :		Search for extreme coordinates\n
 *	GMT_wrap_around_check_{x,tm} :	Check if line wraps around due to Greenwich\n
 *	gmt_x_to_xx :			Generic linear x projection\n
 *	map_xx_to_x :			Generic inverse linear x projection\n
 *	gmt_y_to_yy :			Generic linear y projection\n
 *	map_yy_to_y :			Generic inverse linear y projection\n
 *	gmt_z_to_zz :			Generic linear z projection\n
 *	map_zz_to_z :			Generic inverse linear z projection\n
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

/* We put all the declaration of external functions from gmt_proj.h here since
 * (a) they are only called in gmt_map.c so no need to go in gmt_internals.h
 * (b) this will change completely when proj4 is relacing gmt_proj in GMT 6.
 */

EXTERN_MSC void gmt_vpolar (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_vmerc (struct GMT_CTRL *GMT, double lon0, double slat);
EXTERN_MSC void gmt_vcyleq (struct GMT_CTRL *GMT, double lon0, double slat);
EXTERN_MSC void gmt_vcyleqdist (struct GMT_CTRL *GMT, double lon0, double slat);
EXTERN_MSC void gmt_vcylstereo (struct GMT_CTRL *GMT, double lon0, double slat);
EXTERN_MSC void gmt_vmiller (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_vstereo (struct GMT_CTRL *GMT, double lon0, double lat0, double horizon);
EXTERN_MSC void gmt_vlamb (struct GMT_CTRL *GMT, double lon0, double lat0, double pha, double phb);
EXTERN_MSC void gmt_vtm (struct GMT_CTRL *GMT, double lon0, double lat0);
EXTERN_MSC void gmt_vlambeq (struct GMT_CTRL *GMT, double lon0, double lat0, double horizon);
EXTERN_MSC void gmt_vortho (struct GMT_CTRL *GMT, double lon0, double lat0, double horizon);
EXTERN_MSC void gmt_vgenper (struct GMT_CTRL *GMT, double lon0, double lat0, double altitude, double azimuth, double tilt, double rotation, double width, double height);
EXTERN_MSC void gmt_vgnomonic (struct GMT_CTRL *GMT, double lon0, double lat0, double horizon);
EXTERN_MSC void gmt_vazeqdist (struct GMT_CTRL *GMT, double lon0, double lat0, double horizon);
EXTERN_MSC void gmt_vmollweide (struct GMT_CTRL *GMT, double lon0, double scale);
EXTERN_MSC void gmt_vhammer (struct GMT_CTRL *GMT, double lon0, double scale);
EXTERN_MSC void gmt_vwinkel (struct GMT_CTRL *GMT, double lon0, double scale);
EXTERN_MSC void gmt_veckert4 (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_veckert6 (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_vrobinson (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_vsinusoidal (struct GMT_CTRL *GMT, double lon0);
EXTERN_MSC void gmt_vcassini (struct GMT_CTRL *GMT, double lon0, double lat0);
EXTERN_MSC void gmt_valbers (struct GMT_CTRL *GMT, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void gmt_valbers_sph (struct GMT_CTRL *GMT, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void gmt_veconic (struct GMT_CTRL *GMT, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void gmt_vpolyconic (struct GMT_CTRL *GMT, double lon0, double lat0);
EXTERN_MSC void gmt_vgrinten (struct GMT_CTRL *GMT, double lon0, double scale);
EXTERN_MSC void gmt_polar (struct GMT_CTRL *GMT, double x, double y, double *x_i, double *y_i);		/* Convert x/y (being theta,r) to x,y	*/
EXTERN_MSC void gmt_ipolar (struct GMT_CTRL *GMT, double *x, double *y, double x_i, double y_i);		/* Convert (theta,r) to x,y	*/
EXTERN_MSC void gmt_translin (struct GMT_CTRL *GMT, double forw, double *inv);				/* Forward linear	*/
EXTERN_MSC void gmt_translind (struct GMT_CTRL *GMT, double forw, double *inv);				/* Forward linear, but using 0-360 degrees	*/
EXTERN_MSC void gmt_itranslin (struct GMT_CTRL *GMT, double *forw, double inv);				/* Inverse linear	*/
EXTERN_MSC void gmt_itranslind (struct GMT_CTRL *GMT, double *forw, double inv);				/* Inverse linear, but using 0-360 degrees	*/
EXTERN_MSC void gmt_translog10 (struct GMT_CTRL *GMT, double forw, double *inv);				/* Forward log10	*/
EXTERN_MSC void gmt_itranslog10 (struct GMT_CTRL *GMT, double *forw, double inv);				/* Inverse log10	*/
EXTERN_MSC void gmt_transpowx (struct GMT_CTRL *GMT, double x, double *x_in);				/* Forward pow x	*/
EXTERN_MSC void gmt_itranspowx (struct GMT_CTRL *GMT, double *x, double x_in);				/* Inverse pow x	*/
EXTERN_MSC void gmt_transpowy (struct GMT_CTRL *GMT, double y, double *y_in);				/* Forward pow y 	*/
EXTERN_MSC void gmt_itranspowy (struct GMT_CTRL *GMT, double *y, double y_in);				/* Inverse pow y 	*/
EXTERN_MSC void gmt_transpowz (struct GMT_CTRL *GMT, double z, double *z_in);				/* Forward pow z 	*/
EXTERN_MSC void gmt_itranspowz (struct GMT_CTRL *GMT, double *z, double z_in);				/* Inverse pow z 	*/
EXTERN_MSC void gmt_albers (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Albers)	*/
EXTERN_MSC void gmt_ialbers (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Albers) to lon/lat	*/
EXTERN_MSC void gmt_econic (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Equidistant Conic)	*/
EXTERN_MSC void gmt_ieconic (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Equidistant Conic) to lon/lat	*/
EXTERN_MSC void gmt_polyconic (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Polyconic)	*/
EXTERN_MSC void gmt_ipolyconic (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Polyconic) to lon/lat	*/
EXTERN_MSC void gmt_albers_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Albers Spherical)	*/
EXTERN_MSC void gmt_ialbers_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Albers Spherical) to lon/lat	*/
EXTERN_MSC void gmt_azeqdist (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Azimuthal equal-distance)*/
EXTERN_MSC void gmt_iazeqdist (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Azimuthal equal-distance) to lon/lat*/
EXTERN_MSC void gmt_cassini (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Cassini)	*/
EXTERN_MSC void gmt_icassini (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Cassini) to lon/lat	*/
EXTERN_MSC void gmt_cassini_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cassini Spherical)	*/
EXTERN_MSC void gmt_icassini_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Cassini Spherical) to lon/lat	*/
EXTERN_MSC void gmt_hammer (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Hammer-Aitoff)	*/
EXTERN_MSC void gmt_ihammer (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Hammer-Aitoff) to lon/lat	*/
EXTERN_MSC void gmt_grinten (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (van der Grinten)	*/
EXTERN_MSC void gmt_igrinten (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (van der Grinten) to lon/lat	*/
EXTERN_MSC void gmt_merc_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Mercator Spherical)	*/
EXTERN_MSC void gmt_imerc_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Mercator Spherical) to lon/lat	*/
EXTERN_MSC void gmt_plrs_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Polar Spherical)	*/
EXTERN_MSC void gmt_iplrs_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Polar Spherical) to lon/lat	*/
EXTERN_MSC void gmt_lamb (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Lambert)	*/
EXTERN_MSC void gmt_ilamb (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Lambert) to lon/lat 	*/
EXTERN_MSC void gmt_lamb_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Lambert Spherical)	*/
EXTERN_MSC void gmt_ilamb_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Lambert Spherical) to lon/lat 	*/
EXTERN_MSC void gmt_oblmrc (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Oblique Mercator)	*/
EXTERN_MSC void gmt_ioblmrc (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Oblique Mercator) to lon/lat 	*/
EXTERN_MSC void gmt_genper (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (ORTHO)  */
EXTERN_MSC void gmt_igenper (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (ORTHO) to lon/lat  */
EXTERN_MSC void gmt_ortho (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (GMT_ORTHO)	*/
EXTERN_MSC void gmt_iortho (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (GMT_ORTHO) to lon/lat 	*/
EXTERN_MSC void gmt_gnomonic (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (GMT_GNOMONIC)	*/
EXTERN_MSC void gmt_ignomonic (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (GMT_GNOMONIC) to lon/lat 	*/
EXTERN_MSC void gmt_sinusoidal (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (GMT_SINUSOIDAL)	*/
EXTERN_MSC void gmt_isinusoidal (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (GMT_SINUSOIDAL) to lon/lat 	*/
EXTERN_MSC void gmt_tm (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (TM)	*/
EXTERN_MSC void gmt_itm (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (TM) to lon/lat 	*/
EXTERN_MSC void gmt_tm_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (GMT_TM Spherical)	*/
EXTERN_MSC void gmt_itm_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (GMT_TM Spherical) to lon/lat 	*/
EXTERN_MSC void gmt_utm (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (UTM)	*/
EXTERN_MSC void gmt_iutm (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (UTM) to lon/lat 	*/
EXTERN_MSC void gmt_utm_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (UTM Spherical)	*/
EXTERN_MSC void gmt_iutm_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (UTM Spherical) to lon/lat 	*/
EXTERN_MSC void gmt_winkel (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Winkel)	*/
EXTERN_MSC void gmt_iwinkel (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Winkel) to lon/lat	*/
EXTERN_MSC void gmt_eckert4 (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Eckert IV)	*/
EXTERN_MSC void gmt_ieckert4 (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Eckert IV) to lon/lat	*/
EXTERN_MSC void gmt_eckert6 (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Eckert VI)	*/
EXTERN_MSC void gmt_ieckert6 (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Eckert VI) to lon/lat	*/
EXTERN_MSC void gmt_robinson (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Robinson)	*/
EXTERN_MSC void gmt_irobinson (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Robinson) to lon/lat	*/
EXTERN_MSC void gmt_stereo1_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic Spherical)*/
EXTERN_MSC void gmt_stereo2_sph (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic Spherical, equatorial view)	*/
EXTERN_MSC void gmt_istereo_sph (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Stereographic Spherical) to lon/lat 	*/
EXTERN_MSC void gmt_lambeq (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Lambert Azimuthal Equal-Area)*/
EXTERN_MSC void gmt_ilambeq (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Lambert Azimuthal Equal-Area) to lon/lat*/
EXTERN_MSC void gmt_mollweide (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Mollweide Equal-Area) */
EXTERN_MSC void gmt_imollweide (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Mollweide Equal-Area) to lon/lat */
EXTERN_MSC void gmt_cyleq (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Cylindrical Equal-Area)	*/
EXTERN_MSC void gmt_icyleq (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Cylindrical Equal-Area) to lon/lat	*/
EXTERN_MSC void gmt_cyleqdist (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cylindrical Equidistant)	*/
EXTERN_MSC void gmt_icyleqdist (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Cylindrical Equidistant) to lon/lat 	*/
EXTERN_MSC void gmt_miller (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Miller Cylindrical)	*/
EXTERN_MSC void gmt_imiller (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);		/* Convert x/y (Miller Cylindrical) to lon/lat 	*/
EXTERN_MSC void gmt_cylstereo (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cylindrical Stereographic)	*/
EXTERN_MSC void gmt_icylstereo (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);	/* Convert x/y (Cylindrical Stereographic) to lon/lat */
EXTERN_MSC void gmt_obl (struct GMT_CTRL *GMT, double lon, double lat, double *olon, double *olat);	/* Convert lon/loat to oblique lon/lat */
EXTERN_MSC void gmt_iobl (struct GMT_CTRL *GMT, double *lon, double *lat, double olon, double olat);	/* Convert oblique lon/lat to regular lon/lat */
EXTERN_MSC double gmt_left_winkel (struct GMT_CTRL *GMT, double y);	/* For Winkel maps	*/
EXTERN_MSC double gmt_right_winkel (struct GMT_CTRL *GMT, double y);	/* For Winkel maps	*/
EXTERN_MSC double gmt_left_eckert4 (struct GMT_CTRL *GMT, double y);	/* For Eckert IV maps	*/
EXTERN_MSC double gmt_right_eckert4 (struct GMT_CTRL *GMT, double y);	/* For Eckert IV maps	*/
EXTERN_MSC double gmt_left_eckert6 (struct GMT_CTRL *GMT, double y);	/* For Eckert VI maps	*/
EXTERN_MSC double gmt_right_eckert6 (struct GMT_CTRL *GMT, double y);	/* For Eckert VI maps	*/
EXTERN_MSC double gmt_left_robinson (struct GMT_CTRL *GMT, double y);	/* For Robinson maps	*/
EXTERN_MSC double gmt_right_robinson (struct GMT_CTRL *GMT, double y);	/* For Robinson maps	*/
EXTERN_MSC double gmt_left_sinusoidal (struct GMT_CTRL *GMT, double y);	/* For sinusoidal maps	*/
EXTERN_MSC double gmt_right_sinusoidal (struct GMT_CTRL *GMT, double y);	/* For sinusoidal maps	*/
EXTERN_MSC double gmt_left_polyconic (struct GMT_CTRL *GMT, double y);	/* For polyconic maps	*/
EXTERN_MSC double gmt_right_polyconic (struct GMT_CTRL *GMT, double y);	/* For polyconic maps	*/

/*! CCW order of side in some tests */
enum GMT_side {
	GMT_BOTTOM = 0,
	GMT_RIGHT = 1,
	GMT_TOP = 2,
	GMT_LEFT = 3};

/* Basic error reporting when things go badly wrong. This Return macro can be
 * used in stead of regular return(code) to print out what the code is before
 * it returns.  We assume the GMT pointer is available in the function!
 */
#define VAR_TO_STR(arg)      #arg
#define Return(err) { GMT_Report(GMT->parent,GMT_MSG_ERROR,"Internal Failure = %s\n",VAR_TO_STR(err)); return (err);}

/* Note by P. Wessel, 18-Oct-2012, updated 08-JAN-2014:
 * In the olden days, GMT only did great circle distances.  In GMT 4 we implemented geodesic
 * distances by Rudoe's formula as given in Bomford [1971].  However, that geodesic is not
 * exactly what we wanted as it is a normal section and does not strictly follow the geodesic.
 * Other candidates are Vincenty [1975], which is widely used and Karney [2012], which is super-
 * accurate.  At this point their differences are in the micro-meter level.  For GMT 5 we have
 * now switched to the Vincenty algorithm as provided by Gerald Evenden, USGS [author of proj4],
 * which is a modified translation of the NGS algorithm and not exactly what is in proj4's geod
 * program (which Evenden thinks is inferior.)  I ran a comparison between many algorithms that
 * either were available via codes or had online calculators.  I sought the geodesic distance
 * from (0,0) to (10,10) on WGS-84; the results were (in meters):
 *
 *	GMT4 (Rudoe):		1565109.099232116
 *	proj4:			1565109.095557918
 *	vdist(0,0,10,10) [0]	1565109.09921775
 *	Karney [1]: 		1565109.09921789
 *	Vincenty [2]:		1565109.099218036
 *	NGS [3]			1565109.0992
 *	Andoyer [4]		1565092.276857755
 *
 * [0] via Joaquim Luis, supposedly Vincenty [2012]
 * [1] via online calculator at max precision http://geographiclib.sourceforge.net/cgi-bin/Geod
 * [2] downloading, compiling and running http://article.gmane.org/gmane.comp.gis.proj-4.devel/3478.
 *     This is not identical to Vincenty in proj4 but written by Evenden (proj.4 author)
 * [3] via online calculator http://www.ngs.noaa.gov/cgi-bin/Inv_Fwd/inverse2.prl. Their notes says
 *     this is Vincenty; unfortunately I cannot control the output precision.
 * [4] Andoyer approximate from Astronomical Algorithms, Jean Meeus, 2009, second edition, Willmann-Bell, Inc.
 *
 * Based on these comparisons we decided to implement the Vincenty [2] code as given.  The older Rudoe
 * code is also accessible, as is the approximation by Andoyer which is good to a few tens of m.
 * The choice was based on the readily available C code versus having to reimplement Karney in C.
 * When GMT 6 is started we expect to use proj4
 */

static char *GEOD_TEXT[3] = {"Vincenty", "Andoyer", "Rudoe"};

EXTERN_MSC double gmt_get_angle (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);

/*! . */
double gmtmap_left_boundary (struct GMT_CTRL *GMT, double y) {
	return ((*GMT->current.map.left_edge) (GMT, y));
}

/*! . */
double gmtmap_right_boundary (struct GMT_CTRL *GMT, double y) {
	return ((*GMT->current.map.right_edge) (GMT, y));
}

/* Private functions internal to gmt_map.c */

/*! . */
GMT_LOCAL bool map_quickconic (struct GMT_CTRL *GMT) {
	/* Returns true if area/scale are large/small enough
	 * so that we can use spherical equations with authalic
	 * or conformal latitudes instead of the full ellipsoidal
	 * equations.
	 */

	double s, dlon, width;

	if (GMT->current.proj.gave_map_width) {	/* Gave width */
		dlon = GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO];
		width = GMT->current.proj.pars[4] * GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_M];	/* Convert to meters */
		s = (dlon * GMT->current.proj.M_PR_DEG) / width;
	}
	else if (GMT->current.proj.units_pr_degree) {	/* Gave scale */
		/* Convert to meters */
		s = GMT->current.proj.M_PR_DEG / (GMT->current.proj.pars[4] * GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_M]);
	}
	else {	/* Got 1:xxx that was changed */
		s = (1.0 / GMT->current.proj.pars[4]) / GMT->current.proj.unit;
	}

	if (s > 1.0e7) {	/* if s in 1:s exceeds 1e7 we do the quick thing */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Using spherical projection with conformal latitudes\n");
		return (true);
	}
	else /* Use full ellipsoidal terms */
		return (false);
}

/*! . */
GMT_LOCAL bool map_quicktm (struct GMT_CTRL *GMT, double lon0, double limit) {
	/* Returns true if the region chosen is too large for the
	 * ellipsoidal series to be valid; hence use spherical equations
	 * with authalic latitudes instead.
	 * We let +-limit degrees from central meridian be the cutoff.
	 */

	double d_left, d_right;

	d_left  = lon0 - GMT->common.R.wesn[XLO] - 360.0;
	d_right = lon0 - GMT->common.R.wesn[XHI] - 360.0;
	while (d_left  < -180.0) d_left  += 360.0;
	while (d_right < -180.0) d_right += 360.0;
	if (fabs (d_left) > limit || fabs (d_right) > limit) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Using spherical projection with authalic latitudes\n");
		return (true);
	}
	else /* Use full ellipsoidal terms */
		return (false);
}

/*! . */
GMT_LOCAL void map_set_polar (struct GMT_CTRL *GMT) {
	/* Determines if the projection pole is N or S pole */

	if (doubleAlmostEqual (fabs (GMT->current.proj.pars[1]), 90.0)) {
		GMT->current.proj.polar = true;
		GMT->current.proj.north_pole = (GMT->current.proj.pars[1] > 0.0);
	}
}

GMT_LOCAL bool central_meridian_not_set (struct GMT_CTRL *GMT) {
	/* Just to make it clearer to understand the code.  If NaN then we were never given a central meridian */
	return (gmt_M_is_dnan (GMT->current.proj.pars[0]));
}

GMT_LOCAL void set_default_central_meridian (struct GMT_CTRL *GMT) {
	/* Pick half-way between w and e, but watch for -R+r */
	if (GMT->common.R.oblique && GMT->common.R.wesn[XHI] < GMT->common.R.wesn[XLO])
		GMT->current.proj.pars[0] = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI] + 360.0);	/* Set to middle lon  but watch for e < w */
	else
		GMT->current.proj.pars[0] = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);	/* Set to middle lon */
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Central meridian not given, default to %g\n", GMT->current.proj.pars[0]);
}

/*! . */
GMT_LOCAL void map_cyl_validate_clon (struct GMT_CTRL *GMT, unsigned int mode) {
	/* Make sure that for global (360-range) cylindrical projections, the central meridian is neither west nor east.
	 * If so then we reset it to the middle value or we change -R:
	 * mode == 0: <clon> should be reset based on w/e mid-point
	 * mode == 1: -J<clon> is firm so w/e is centered on <c.lon>
	 */
	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	else if (GMT->current.map.is_world && (GMT->current.proj.pars[0] == GMT->common.R.wesn[XLO] || GMT->current.proj.pars[0] == GMT->common.R.wesn[XHI])) {
		/* Reset central meridian since cannot be 360 away from one of the boundaries since that gives xmin == xmax below */
		if (mode == 1) {	/* Change -R to fit central meridian */
			double w = GMT->current.proj.pars[0] - 180.0, e = GMT->current.proj.pars[0] + 180.0;
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Region for global cylindrical projection had to be reset from %g/%g to %g/%g\n",
				GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], w, e);
			GMT->common.R.wesn[XLO] = w;	GMT->common.R.wesn[XHI] = e;
		}
		else {	/* Change central meridian to fit -R */
			double new_lon = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Central meridian for global cylindrical projection had to be reset from %g to %g\n", GMT->current.proj.pars[0], new_lon);
			GMT->current.proj.pars[0] = new_lon;
		}
	}
	else if (!GMT->current.map.is_world) {	/* For reginal areas we cannot have clon > 180 away from either boundary */
		if (fabs (GMT->current.proj.pars[0] - GMT->common.R.wesn[XLO]) > 180.0 || fabs (GMT->current.proj.pars[0] - GMT->common.R.wesn[XHI]) > 180.0) {
			double new_lon = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Central meridian for cylindrical projection had to be reset from %g to %g\n", GMT->current.proj.pars[0], new_lon);
			GMT->current.proj.pars[0] = new_lon;
		}
	}
}

/*! . */
GMT_LOCAL void map_lat_swap_init (struct GMT_CTRL *GMT) {
	/* Initialize values in GMT->current.proj.lat_swap_vals based on GMT->current.proj.

	First compute GMT->current.proj.lat_swap_vals.ra (and rm), the radii to use in
	spherical formulae for area (respectively, N-S distance) when
	using the authalic (respectively, meridional) latitude.

	Then for each type of swap:
	First load GMT->current.proj.lat_swap_vals.c[itype][k], k=0,1,2,3 with the
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
	Conformal  = GMT, angle to use in conformal    development of ellipsoid
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
	added to the input lat to get the output lat in gmt_lat_swap().

	I have tested this code by checking that
		fabs (geocentric) < fabs (parametric) < fabs (geodetic)
	and also, for each pair of possible conversions, that the
	forward followed by the inverse returns the original lat to
	within a small tolerance.  This tolerance is as follows:

	geodetic <-> authalic:      max error (degrees) = 1.253344e-08
	geodetic <-> conformal:     max error (degrees) = 2.321796e-07  should be better after 13nov07 fix
	geodetic <-> meridional:    max error (degrees) = 4.490630e-12
	geodetic <-> geocentric:    max error (degrees) = 1.350031e-13
	geodetic <-> parametric:    max error (degrees) = 1.421085e-14
	geocentric <-> parametric:  max error (degrees) = 1.421085e-14

	Currently, (GMT v5) the only ones we anticipate using are
	geodetic, authalic, and conformal.  I have put others in here
	for possible future convenience.

	Also, I made this depend on GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid]
	rather than on GMT->current.proj, so that it will be possible to
	call gmt_lat_swap() without having to pass -R and -J to
	gmt_map_setup(), so that in the future we will be able to use
	lat conversions without plotting maps.

	W H F Smith, 10--13 May 1999.   */

	/* PW notes: Projections only convert latitudes if GMT->current.proj.GMT_convert_latitudes is true.
	 *       This is set by gmt_map_setup if the ellipsoid is not a sphere.  Calling map_lat_swap_init by itself
	 *	 does not affect the mapping machinery.  Since various situations call for the use
	 *	 of auxiliary latitudes we initialize map_lat_swap_init in gmt_begin.  This means
	 *	 programs can use functions like gmt_lat_swap whenever needed.
	 */

	unsigned int i;
	double x, xx[4], a, f, e2, e4, e6, e8;

	f = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening;
	a = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius;

	if (gmt_M_is_zero (f)) {
		gmt_M_memset (GMT->current.proj.lat_swap_vals.c, GMT_LATSWAP_N * 4, double);
		GMT->current.proj.lat_swap_vals.ra = GMT->current.proj.lat_swap_vals.rm = a;
		GMT->current.proj.lat_swap_vals.spherical = true;
		return;
	}
	GMT->current.proj.lat_swap_vals.spherical = false;

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
	GMT->current.proj.lat_swap_vals.ra = a * sqrt( (1.0 + x) * (1.0 - e2));

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
	GMT->current.proj.lat_swap_vals.rm = a * (1.0 - x);


	/* Geodetic to authalic:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2A][0] = -(e2 / 3.0 + (31.0 * e4 / 180.0 + 59.0 * e6 / 560.0));
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2A][1] = 17.0 * e4 / 360.0 + 61.0 * e6 / 1260;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2A][2] = -383.0 * e6 / 45360.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2A][3] = 0.0;

	/* Authalic to geodetic:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_A2G][0] = e2 / 3.0 + (31.0 * e4 / 180.0 + 517.0 * e6 / 5040.0);
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_A2G][1] = 23.0 * e4 / 360.0 + 251.0 * e6 / 3780;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_A2G][2] = 761.0 * e6 / 45360.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_A2G][3] = 0.0;

	/* Geodetic to conformal:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2C][0] = -(e2 / 2.0 + (5.0 * e4 / 24.0 + (3.0 * e6 / 32.0 + 281.0 * e8 / 5760.0)));
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2C][1] = 5.0 * e4 / 48.0 + (7.0 * e6 / 80.0 + 697.0 * e8 / 11520.0);
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2C][2] = -(13.0 * e6 / 480.0 + 461.0 * e8 / 13440.0);
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2C][3] = 1237.0 * e8 / 161280.0;

	/* Conformal to geodetic:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_C2G][0] = e2 / 2.0 + (5.0 * e4 / 24.0 + (e6 / 12.0 + 13.0 * e8 / 360.0)) ;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_C2G][1] = 7.0 * e4 / 48.0 + (29.0 * e6 / 240.0 + 811.0 * e8 / 11520.0);
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_C2G][2] = 7.0 * e6 / 120.0 + 81.0 * e8 / 1120.0;  /* Bug fixed 13nov07 whfs */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_C2G][3] = 4279.0 * e8 / 161280.0;


	/* The meridional and parametric developments use this parameter:  */
	x = f/(2.0 - f);		/* Adams calls this n.  It is f/(2-f), or -betaJK in my notes.  */
	xx[0] = x;			/* n  */
	xx[1] = x * x;			/* n-squared  */
	xx[2] = xx[1] * x;		/* n-cubed  */
	xx[3] = xx[2] * x;		/* n to the 4th  */

	/* Geodetic to meridional:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2M][0] = -(3.0 * xx[0] / 2.0 - 9.0 * xx[2] / 16.0);
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2M][1] = 15.0 * xx[1] / 16.0 - 15.0 * xx[3] / 32.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2M][2] = -35.0 * xx[2] / 48.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2M][3] = 315.0 * xx[3] / 512.0;

	/* Meridional to geodetic:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_M2G][0] = 3.0 * xx[0] / 2.0 - 27.0 * xx[2] / 32.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_M2G][1] = 21.0 * xx[1] / 16.0 - 55.0 * xx[3] / 32.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_M2G][2] = 151.0 * xx[2] / 96.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_M2G][3] = 1097.0 * xx[3] / 512.0;

	/* Geodetic to parametric equals parametric to geocentric:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2P][0] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2O][0] = -xx[0];
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2P][1] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2O][1] = xx[1] / 2.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2P][2] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2O][2] = -xx[2] / 3.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2P][3] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2O][3] = xx[3] / 4.0;

	/* Parametric to geodetic equals geocentric to parametric:  */
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2G][0] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2P][0] = xx[0];
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2G][1] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2P][1] = xx[1] / 2.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2G][2] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2P][2] = xx[2] / 3.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_P2G][3] = GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2P][3] = xx[3] / 4.0;


	/* The geodetic <->geocentric use this parameter:  */
	x = 1.0 - e2;
	x = (1.0 - x)/(1.0 + x);	/* Adams calls this m.  It is e2/(2-e2), or -betaJK in my notes.  */
	xx[0] = x;			/* m  */
	xx[1] = x * x;			/* m-squared  */
	xx[2] = xx[1] * x;		/* m-cubed  */
	xx[3] = xx[2] * x;		/* m to the 4th  */

	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2O][0] = -xx[0];
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2O][1] = xx[1] / 2.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2O][2] = -xx[2] / 3.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_G2O][3] = xx[3] / 4.0;

	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2G][0] = xx[0];
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2G][1] = xx[1] / 2.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2G][2] = xx[2] / 3.0;
	GMT->current.proj.lat_swap_vals.c[GMT_LATSWAP_O2G][3] = xx[3] / 4.0;


	/* Now do the Snyder Shuffle:  */
	for (i = 0; i < GMT_LATSWAP_N; i++) {
		GMT->current.proj.lat_swap_vals.c[i][0] = GMT->current.proj.lat_swap_vals.c[i][0] - GMT->current.proj.lat_swap_vals.c[i][2];
		GMT->current.proj.lat_swap_vals.c[i][1] = 2.0 * GMT->current.proj.lat_swap_vals.c[i][1] - 4.0 * GMT->current.proj.lat_swap_vals.c[i][3];
		GMT->current.proj.lat_swap_vals.c[i][2] *= 4.0;
		GMT->current.proj.lat_swap_vals.c[i][3] *= 8.0;
	}
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

/*! . */
GMT_LOCAL bool map_wesn_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	/* Determine if a point (lon,lat) is outside or on the rectangular lon/lat boundaries
	 * The check GMT->current.map.lon_wrap is include since we need to consider the 360
	 * degree periodicity of the longitude coordinate.
	 * When we are making basemaps and may want to ensure that a point is
	 * slightly outside the border without having it automatically flip by
	 * 360 degrees. In that case GMT->current.map.lon_wrap will be temporarily set to false.
	 */

	if (GMT->current.map.lon_wrap) {
		while (lon < GMT->common.R.wesn[XLO] && lon + 360.0 <= GMT->common.R.wesn[XHI]) lon += 360.0;
		while (lon > GMT->common.R.wesn[XHI] && lon - 360.0 >= GMT->common.R.wesn[XLO]) lon -= 360.0;
	}

	/* Note PW: 8-20-2014: Was GMT_CONV4_LIMIT instead of GMT_CONV8_LIMIT.  Trying the latter */
	if (GMT->current.map.on_border_is_outside && fabs (lon - GMT->common.R.wesn[XLO]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = -1;
	else if (GMT->current.map.on_border_is_outside && fabs (lon - GMT->common.R.wesn[XHI]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = 1;
	else if (lon < GMT->common.R.wesn[XLO])
		GMT->current.map.this_x_status = -2;
	else if (lon > GMT->common.R.wesn[XHI])
		GMT->current.map.this_x_status = 2;
	else
		GMT->current.map.this_x_status = 0;

	if (GMT->current.map.on_border_is_outside && fabs (lat - GMT->common.R.wesn[YLO]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = -1;
	else if (GMT->current.map.on_border_is_outside && fabs (lat - GMT->common.R.wesn[YHI]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = 1;
	else if (lat < GMT->common.R.wesn[YLO])
		GMT->current.map.this_y_status = -2;
	else if (lat > GMT->common.R.wesn[YHI])
		GMT->current.map.this_y_status = 2;
	else
		GMT->current.map.this_y_status = 0;

	return (GMT->current.map.this_x_status != 0 || GMT->current.map.this_y_status != 0);

}

/*! . */
GMT_LOCAL bool map_polar_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	map_wesn_outside (GMT, lon, lat);

	if (!GMT->current.proj.edge[1]) GMT->current.map.this_x_status = 0;	/* 360 degrees, no edge */
	if (GMT->current.map.this_y_status < 0 && !GMT->current.proj.edge[0]) GMT->current.map.this_y_status = 0;	/* South pole enclosed */
	if (GMT->current.map.this_y_status > 0 && !GMT->current.proj.edge[2]) GMT->current.map.this_y_status = 0;	/* North pole enclosed */

	return (GMT->current.map.this_x_status != 0 || GMT->current.map.this_y_status != 0);
}

/*! . */
GMT_LOCAL bool map_radial_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	double dist;

	/* Test if point is more than horizon spherical degrees from origin.  For global maps, let all borders be "south" */

	/* Note PW: 8-20-2014: Was GMT_CONV4_LIMIT instead of GMT_CONV8_LIMIT.  Trying the latter */
	GMT->current.map.this_x_status = 0;
	dist = gmtlib_great_circle_dist_degree (GMT, lon, lat, GMT->current.proj.central_meridian, GMT->current.proj.pole);
	if (GMT->current.map.on_border_is_outside && fabs (dist - GMT->current.proj.f_horizon) < GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = -1;
	else if (dist > GMT->current.proj.f_horizon)
		GMT->current.map.this_y_status = -2;
	else
		GMT->current.map.this_y_status = 0;
	return (GMT->current.map.this_y_status != 0);
}

/*! . */
GMT_LOCAL bool map_rect_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	double x, y;

	gmt_geo_to_xy (GMT, lon, lat, &x, &y);

	return (gmt_cart_outside (GMT, x, y));
}

/*! . */
GMT_LOCAL bool map_rect_outside2 (struct GMT_CTRL *GMT, double lon, double lat) {
	/* For Azimuthal proj with rect borders since map_rect_outside may fail for antipodal points */
	if (map_radial_outside (GMT, lon, lat)) return (true);	/* Point > 90 degrees away */
	return (map_rect_outside (GMT, lon, lat));	/* Must check if inside box */
}

/*! . */
GMT_LOCAL void map_x_wesn_corner (struct GMT_CTRL *GMT, double *x) {
/*	if (fabs (fmod (fabs (*x - GMT->common.R.wesn[XLO]), 360.0)) <= GMT_CONV4_LIMIT)
		*x = GMT->common.R.wesn[XLO];
	else if (fabs (fmod (fabs (*x - GMT->common.R.wesn[XHI]), 360.0)) <= GMT_CONV4_LIMIT)
		*x = GMT->common.R.wesn[XHI]; */

	/* Note PW: 8-20-2014: Was GMT_CONV4_LIMIT instead of GMT_CONV8_LIMIT.  Trying the latter */
	if (fabs (*x - GMT->common.R.wesn[XLO]) <= GMT_CONV8_LIMIT)
		*x = GMT->common.R.wesn[XLO];
	else if (fabs (*x - GMT->common.R.wesn[XHI]) <= GMT_CONV8_LIMIT)
		*x = GMT->common.R.wesn[XHI];
}

/*! . */
GMT_LOCAL void map_y_wesn_corner (struct GMT_CTRL *GMT, double *y) {
	/* Note PW: 8-20-2014: Was GMT_CONV4_LIMIT instead of GMT_CONV8_LIMIT.  Trying the latter */
	if (fabs (*y - GMT->common.R.wesn[YLO]) <= GMT_CONV8_LIMIT)
		*y = GMT->common.R.wesn[YLO];
	else if (fabs (*y - GMT->common.R.wesn[YHI]) <= GMT_CONV8_LIMIT)
		*y = GMT->common.R.wesn[YHI];
}

/*! . */
GMT_LOCAL bool map_is_wesn_corner (struct GMT_CTRL *GMT, double x, double y) {
	/* Checks if point is a corner */
	GMT->current.map.corner = 0;

	if (doubleAlmostEqualZero (fmod(fabs(x), 360.0), fmod(fabs(GMT->common.R.wesn[XLO]), 360.0))) {
		if (doubleAlmostEqualZero (y, GMT->common.R.wesn[YLO]))
			GMT->current.map.corner = 1;
		else if (doubleAlmostEqualZero (y, GMT->common.R.wesn[YHI]))
			GMT->current.map.corner = 4;
	}
	else if (doubleAlmostEqualZero (fmod(fabs(x), 360.0), fmod(fabs(GMT->common.R.wesn[XHI]), 360.0))) {
		if (doubleAlmostEqualZero (y, GMT->common.R.wesn[YLO]))
			GMT->current.map.corner = 2;
		else if (doubleAlmostEqualZero (y, GMT->common.R.wesn[YHI]))
			GMT->current.map.corner = 3;
	}
	return (GMT->current.map.corner > 0);
}

/*! . */
GMT_LOCAL bool map_lon_inside (struct GMT_CTRL *GMT, double lon, double w, double e) {
	while (lon < GMT->common.R.wesn[XLO]) lon += 360.0;
	while (lon > GMT->common.R.wesn[XHI]) lon -= 360.0;

	if (lon < w) return (false);
	if (lon > e) return (false);
	return (true);
}

/*! . */
GMT_LOCAL unsigned int map_wesn_crossing (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, unsigned int *sides) {
	/* Compute all crossover points of a line segment with the rectangular lat/lon boundaries
	 * Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	unsigned int n = 0, i;
	double d, x0, y0;

	/* If wrapping is allowed: first bring both points between W and E boundaries,
	 * then move the western-most point east if it is further than 180 degrees away.
	 * This may cause the points to span the eastern boundary */

	if (GMT->current.map.lon_wrap) {
		while (lon0 < GMT->common.R.wesn[XLO]) lon0 += 360.0;
		while (lon0 > GMT->common.R.wesn[XHI]) lon0 -= 360.0;
		while (lon1 < GMT->common.R.wesn[XLO]) lon1 += 360.0;
		while (lon1 > GMT->common.R.wesn[XHI]) lon1 -= 360.0;
		if (fabs (lon0 - lon1) <= 180.0) { /* Nothing */ }
		else if (lon0 < lon1)
			lon0 += 360.0;
		else
			lon1 += 360.0;
	}

	/* Then set 'almost'-corners to corners */
	map_x_wesn_corner (GMT, &lon0);
	map_x_wesn_corner (GMT, &lon1);
	map_y_wesn_corner (GMT, &lat0);
	map_y_wesn_corner (GMT, &lat1);

	/* Crossing South */
	if ((lat0 >= GMT->common.R.wesn[YLO] && lat1 <= GMT->common.R.wesn[YLO]) || (lat1 >= GMT->common.R.wesn[YLO] && lat0 <= GMT->common.R.wesn[YLO])) {
		sides[n] = 0;
		clat[n] = GMT->common.R.wesn[YLO];
		d = lat0 - lat1;
		clon[n] = (doubleAlmostEqualZero (lat0, lat1)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		map_x_wesn_corner (GMT, &clon[n]);
		if (fabs (d) > 0.0 && map_lon_inside (GMT, clon[n], GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) n++;
	}
	/* Crossing East */
	if ((lon0 >= GMT->common.R.wesn[XHI] && lon1 <= GMT->common.R.wesn[XHI]) || (lon1 >= GMT->common.R.wesn[XHI] && lon0 <= GMT->common.R.wesn[XHI])) {
		sides[n] = 1;
		clon[n] = GMT->common.R.wesn[XHI];
		d = lon0 - lon1;
		clat[n] = (doubleAlmostEqualZero (lon0, lon1)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		map_y_wesn_corner (GMT, &clat[n]);
		if (fabs (d) > 0.0 && clat[n] >= GMT->common.R.wesn[YLO] && clat[n] <= GMT->common.R.wesn[YHI]) n++;
	}

	/* Now adjust the longitudes so that they might span the western boundary */
	if (GMT->current.map.lon_wrap && MAX(lon0, lon1) > GMT->common.R.wesn[XHI]) {
		lon0 -= 360.0; lon1 -= 360.0;
	}

	/* Crossing North */
	if ((lat0 >= GMT->common.R.wesn[YHI] && lat1 <= GMT->common.R.wesn[YHI]) || (lat1 >= GMT->common.R.wesn[YHI] && lat0 <= GMT->common.R.wesn[YHI])) {
		sides[n] = 2;
		clat[n] = GMT->common.R.wesn[YHI];
		d = lat0 - lat1;
		clon[n] = (doubleAlmostEqualZero (lat0, lat1)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		map_x_wesn_corner (GMT, &clon[n]);
		if (fabs (d) > 0.0 && map_lon_inside (GMT, clon[n], GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) n++;
	}
	/* Crossing West */
	if ((lon0 <= GMT->common.R.wesn[XLO] && lon1 >= GMT->common.R.wesn[XLO]) || (lon1 <= GMT->common.R.wesn[XLO] && lon0 >= GMT->common.R.wesn[XLO])) {
		sides[n] = 3;
		clon[n] = GMT->common.R.wesn[XLO];
		d = lon0 - lon1;
		clat[n] = (doubleAlmostEqualZero (lon0, lon1)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		map_y_wesn_corner (GMT, &clat[n]);
		if (fabs (d) > 0.0 && clat[n] >= GMT->common.R.wesn[YLO] && clat[n] <= GMT->common.R.wesn[YHI]) n++;
	}

	if (n == 0) return (0);

	for (i = 0; i < n; i++) {
		gmt_geo_to_xy (GMT, clon[i], clat[i], &xx[i], &yy[i]);
		if (GMT->current.proj.projection_GMT == GMT_POLAR && sides[i]%2) sides[i] = 4 - sides[i];	/*  toggle 1 <-> 3 */
	}

	if (n == 1) return (1);

	/* Check for corner xover if n == 2 */

	if (map_is_wesn_corner (GMT, clon[0], clat[0])) return (1);

	if (map_is_wesn_corner (GMT, clon[1], clat[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	/* Sort the two intermediate points into the right order based on projected distances from the first point */

	gmt_geo_to_xy (GMT, lon0, lat0, &x0, &y0);

	if (hypot (x0 - xx[1], y0 - yy[1]) < hypot (x0 - xx[0], y0 - yy[0])) {
		gmt_M_double_swap (clon[0], clon[1]);
		gmt_M_double_swap (clat[0], clat[1]);
		gmt_M_double_swap (xx[0], xx[1]);
		gmt_M_double_swap (yy[0], yy[1]);
		gmt_M_uint_swap (sides[0], sides[1]);
	}

	return (2);
}

#if 0
GMT_LOCAL void map_x_rect_corner (struct GMT_CTRL *GMT, double *x) {
	if (fabs (*x) <= GMT_CONV4_LIMIT)
		*x = 0.0;
	else if (fabs (*x - GMT->current.proj.rect[XHI]) <= GMT_CONV4_LIMIT)
		*x = GMT->current.proj.rect[XHI];
}

GMT_LOCAL void map_y_rect_corner (struct GMT_CTRL *GMT, double *y) {
	if (fabs (*y) <= GMT_CONV4_LIMIT)
		*y = 0.0;
	else if (fabs (*y - GMT->current.proj.rect[YHI]) <= GMT_CONV4_LIMIT)
		*y = GMT->current.proj.rect[YHI];
}
#endif

/*! . */
GMT_LOCAL void map_x_rect_corner (struct GMT_CTRL *GMT, double *x) {
	if (fabs (*x) <= GMT_CONV8_LIMIT)
		*x = 0.0;
	else if (fabs (*x - GMT->current.proj.rect[XHI]) <= GMT_CONV8_LIMIT)
		*x = GMT->current.proj.rect[XHI];
}

/*! . */
GMT_LOCAL void map_y_rect_corner (struct GMT_CTRL *GMT, double *y) {
	if (fabs (*y) <= GMT_CONV8_LIMIT)
		*y = 0.0;
	else if (fabs (*y - GMT->current.proj.rect[YHI]) <= GMT_CONV8_LIMIT)
		*y = GMT->current.proj.rect[YHI];
}

/*! . */
GMT_LOCAL bool map_is_rect_corner (struct GMT_CTRL *GMT, double x, double y) {
	/* Checks if point is a corner */
	GMT->current.map.corner = -1;
	if (doubleAlmostEqualZero (x, GMT->current.proj.rect[XLO])) {
		if (doubleAlmostEqualZero (y, GMT->current.proj.rect[YLO]))
			GMT->current.map.corner = 1;
		else if (doubleAlmostEqualZero (y, GMT->current.proj.rect[YHI]))
			GMT->current.map.corner = 4;
	}
	else if (doubleAlmostEqualZero (x, GMT->current.proj.rect[XHI])) {
		if (doubleAlmostEqualZero (y, GMT->current.proj.rect[YLO]))
			GMT->current.map.corner = 2;
		else if (doubleAlmostEqualZero (y, GMT->current.proj.rect[YHI]))
			GMT->current.map.corner = 3;
	}
	return (GMT->current.map.corner > 0);
}

/*! . */
GMT_LOCAL unsigned int map_rect_crossing (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, unsigned int *sides) {
	/* Compute all crossover points of a line segment with the boundaries in a rectangular projection */

	unsigned int i, j, n = 0;
	double x0, x1, y0, y1, d;

	/* Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	gmt_geo_to_xy (GMT, lon0, lat0, &x0, &y0);
	gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);

	/* First set 'almost'-corners to corners */

	map_x_rect_corner (GMT, &x0);
	map_x_rect_corner (GMT, &x1);
	map_y_rect_corner (GMT, &y0);
	map_y_rect_corner (GMT, &y1);

	if ((y0 >= GMT->current.proj.rect[YLO] && y1 <= GMT->current.proj.rect[YLO]) || (y1 >= GMT->current.proj.rect[YLO] && y0 <= GMT->current.proj.rect[YLO])) {
		sides[n] = 0;
		yy[n] = GMT->current.proj.rect[YLO];
		d = y0 - y1;
		xx[n] = (doubleAlmostEqualZero (y0, y1)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		map_x_rect_corner (GMT, &xx[n]);
		if (fabs (d) > 0.0 && xx[n] >= GMT->current.proj.rect[XLO] && xx[n] <= GMT->current.proj.rect[XHI]) n++;
	}
	if ((x0 <= GMT->current.proj.rect[XHI] && x1 >= GMT->current.proj.rect[XHI]) || (x1 <= GMT->current.proj.rect[XHI] && x0 >= GMT->current.proj.rect[XHI])) {
		sides[n] = 1;
		xx[n] = GMT->current.proj.rect[XHI];
		d = x0 - x1;
		yy[n] = (doubleAlmostEqualZero (x0, x1)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		map_y_rect_corner (GMT, &yy[n]);
		if (fabs (d) > 0.0 && yy[n] >= GMT->current.proj.rect[YLO] && yy[n] <= GMT->current.proj.rect[YHI]) n++;
	}
	if ((y0 <= GMT->current.proj.rect[YHI] && y1 >= GMT->current.proj.rect[YHI]) || (y1 <= GMT->current.proj.rect[YHI] && y0 >= GMT->current.proj.rect[YHI])) {
		sides[n] = 2;
		yy[n] = GMT->current.proj.rect[YHI];
		d = y0 - y1;
		xx[n] = (doubleAlmostEqualZero (y0, y1)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		map_x_rect_corner (GMT, &xx[n]);
		if (fabs (d) > 0.0 && xx[n] >= GMT->current.proj.rect[XLO] && xx[n] <= GMT->current.proj.rect[XHI]) n++;
	}
	if ((x0 >= GMT->current.proj.rect[XLO] && x1 <= GMT->current.proj.rect[XLO]) || (x1 >= GMT->current.proj.rect[XLO] && x0 <= GMT->current.proj.rect[XLO])) {
		sides[n] = 3;
		xx[n] = GMT->current.proj.rect[XLO];
		d = x0 - x1;
		yy[n] = (doubleAlmostEqualZero (x0, x1)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		map_y_rect_corner (GMT, &yy[n]);
		if (fabs (d) > 0.0 && yy[n] >= GMT->current.proj.rect[YLO] && yy[n] <= GMT->current.proj.rect[YHI]) n++;
	}

	if (n == 0) return (0);

	/* Eliminate duplicates */

	for (i = 0; i < n; ++i) {
		for (j = i + 1; j < n; ++j) {
			if (doubleAlmostEqualZero (xx[i], xx[j]) && doubleAlmostEqualZero (yy[i], yy[j]))	/* Duplicate */
				sides[j] = 99;	/* Mark as duplicate */
		}
	}
	for (i = 1; i < n; i++) {
		if (sides[i] == 99) {	/* This is a duplicate, overwrite */
			for (j = i + 1; j < n; j++) {
				xx[j-1] = xx[j];
				yy[j-1] = yy[j];
				sides[j-1] = sides[j];
			}
			n--;
			i--;	/* Must start at same point again */
		}
	}

	for (i = 0; i < n; i++)	gmt_xy_to_geo (GMT, &clon[i], &clat[i], xx[i], yy[i]);

	if (gmt_M_is_cartesian (GMT, GMT_IN)) return (n);

	if (n < 2) return (n);

	/* Check for corner xover if n == 2 */

	if (map_is_rect_corner (GMT, xx[0], yy[0])) return (1);

	if (map_is_rect_corner (GMT, xx[1], yy[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	/* Sort the two intermediate points into the right order based on projected distances from the first point */

	if (hypot (x0 - xx[1], y0 - yy[1]) < hypot (x0 - xx[0], y0 - yy[0])) {
		gmt_M_double_swap (clon[0], clon[1]);
		gmt_M_double_swap (clat[0], clat[1]);
		gmt_M_double_swap (xx[0], xx[1]);
		gmt_M_double_swap (yy[0], yy[1]);
		gmt_M_uint_swap (sides[0], sides[1]);
	}

	return (2);
}

/*! . */
GMT_LOCAL unsigned int map_radial_crossing (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, unsigned int *sides) {
	/* Computes the lon/lat of a point that is f_horizon spherical degrees from
	 * the origin and lies on the great circle between points 1 and 2 */

	double dist1, dist2, delta, eps, dlon;

	dist1 = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon1, lat1);
	dist2 = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon2, lat2);
	delta = dist2 - dist1;
	eps = (doubleAlmostEqualZero (dist1, dist2)) ? 0.0 : (GMT->current.proj.f_horizon - dist1) / delta;
	gmt_M_set_delta_lon (lon1, lon2, dlon);
	clon[0] = lon1 + dlon * eps;
	clat[0] = lat1 + (lat2 - lat1) * eps;

	gmt_geo_to_xy (GMT, clon[0], clat[0], &xx[0], &yy[0]);

	sides[0] = 1;

	return (1);
}

/*! . */
GMT_LOCAL unsigned int map_genper_crossing (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, unsigned int *sides) {
	/* Awkward case of windowing which gives a boundary that is a clipped circle.
	 * Here, the rectangular and circular boundaries take turns in being the definitive boundary.
	 * Approach: Determine how the points do relative to either boundary. */

	bool h_out[2], r_out[2];

	/* Check if point 1 is beyond horizon: */
	h_out[0] = map_radial_outside (GMT, lon1, lat1);	/* true if point 1 is beyond the horizon */
	h_out[1] = map_radial_outside (GMT, lon2, lat2);	/* true if point 2 is beyond the horizon */
	r_out[0] = map_rect_outside (GMT, lon1, lat1);		/* true if point 1 is beyond the map box */
	r_out[1] = map_rect_outside (GMT, lon2, lat2);		/* true if point 2 is beyond the map box */
	if (h_out[0] == false && h_out[1] == false)	/* We are not beyond horizon so technically inside unless clipped by the window: */
		return (map_rect_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	if (r_out[0] == false && r_out[1] == false)	/* We are not outside the map box but might be beyond horizon: */
		return (map_radial_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	/* Here we have a mixed bag.  Try our best */
	if (h_out[0] == false && r_out[0] == true)	/* Point 1 is outside box but inside horizon.  That means point 2 is inside and we use rect_crossing */
		return (map_rect_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	if (h_out[0] == true && r_out[0] == false)	/* Point 1 is beyond horizon but inside box.  That means point 2 is inside and we use map_radial_crossing */
		return (map_radial_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	if (h_out[1] == false && r_out[1] == true)	/* Point 2 is outside box but inside horizon.  That means point 1 is inside and we use rect_crossing */
		return (map_rect_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	if (h_out[1] == true && r_out[1] == false)	/* Point 2 is beyond horizon but inside box.  That means point 1 is inside and we use map_radial_crossing */
		return (map_radial_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));
	/* Don't think we should get here... */
	GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in map_genper_crossing: None of the cases matched crossing scenario");
	return (map_radial_crossing (GMT, lon1, lat1, lon2, lat2, clon, clat, xx, yy, sides));

}

GMT_LOCAL int map_jump_x (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* true if x-distance between points exceeds 1/2 map width at this y value */
	double dx, map_half_size, half_lon_range;

	if (!(gmt_M_is_cylindrical (GMT) || gmt_M_is_perspective (GMT) || gmt_M_is_misc (GMT))) return (0);	/* Only projections with periodic boundaries may apply */

	if (gmt_M_is_cartesian (GMT, GMT_IN) || fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) < 90.0) return (0);

	map_half_size = MAX (gmt_half_map_width (GMT, y0), gmt_half_map_width (GMT, y1));
	if (fabs (map_half_size) < GMT_CONV4_LIMIT) return (0);

	half_lon_range = (GMT->common.R.oblique) ? 180.0 : 0.5 * (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]);
	dx = x1 - x0;
	if (fabs (dx) > map_half_size) {	/* Possible jump; let's see how far apart those longitudes really are */
		/* This test on longitudes was added to deal with issue #672, also see test/psxy/nojump.sh */
		double last_lon, this_lon, dummy, dlon;
		gmt_xy_to_geo (GMT, &last_lon, &dummy, x0, y0);
		gmt_xy_to_geo (GMT, &this_lon, &dummy, x1, y1);
		gmt_M_set_delta_lon (last_lon, this_lon, dlon);	/* Beware of jumps due to sign differences */
		if (fabs (dlon) < half_lon_range) /* Not going the long way so we judge this to be no jump */
			return (0);
		/* Jump it is */
		if (dx > map_half_size)	return (-1);	/* Cross left/west boundary */
		if (dx < (-map_half_size)) return (1);	/* Cross right/east boundary */
	}
	return (0);
}

GMT_LOCAL int map_jump_xy (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* true if x- or y-distance between points exceeds 1/2 map width at this y value or map height */
	double dx, dy, map_half_width, map_half_height;

	if (!(gmt_M_is_cylindrical (GMT) || gmt_M_is_perspective (GMT) || gmt_M_is_misc (GMT))) return (0);	/* Only projections with periodic boundaries may apply */

	if (gmt_M_is_cartesian (GMT, GMT_IN) || fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) < 90.0) return (0);

	map_half_width = MAX (gmt_half_map_width (GMT, y0), gmt_half_map_width (GMT, y1));
	if (fabs (map_half_width) < GMT_CONV4_LIMIT) return (0);	/* Very close to a pole for some projections */
	map_half_height = 0.5 * GMT->current.map.height;	/* We assume this is constant */

	dx = x1 - x0;
	if (fabs (dx) > map_half_width) {	/* Possible jump; let's see how far apart those longitudes really are */
		/* This test on longitudes was added to deal with issue #672, also see test/psxy/nojump.sh */
		double last_lon, this_lon, dummy, dlon;
		double half_lon_range = (GMT->common.R.oblique) ? 180.0 : 0.5 * (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]);
		gmt_xy_to_geo (GMT, &last_lon, &dummy, x0, y0);
		gmt_xy_to_geo (GMT, &this_lon, &dummy, x1, y1);
		gmt_M_set_delta_lon (last_lon, this_lon, dlon);	/* Beware of jumps due to sign differences */
		if (fabs (dlon) < half_lon_range) /* Not going the long way so we judge this to be no jump */
			return (0);
		/* Jump it is */
		if (dx > map_half_width)	return (-1);	/* Cross left/west boundary */
		if (dx < (-map_half_width)) return (1);	/* Cross right/east boundary */
	}
	else if (fabs ((dy = y1 - y0)) > map_half_height) {	/* Possible jump for TM or UTM */
		/* Jump it is */
		if (dy > map_half_height)	return (-2);	/* Cross bottom/south boundary */
		if (dy < (-map_half_height)) return (2);	/* Cross top/north boundary */
	}
	return (0);	/* No jump */
}

GMT_LOCAL int map_jump_not (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
#if 0
	double dx, map_half_size;
	/* For Genper we believe there should not be jumps [Introduced to test issue #667] */
	/* However, it does not work for all cases.  Consider this one
	 * psbasemap -Rg -JG280.969616634/58.7193421224/2513/0/0/0/200/180/6i -Ba30f30g30/a5f5g5NWSE -P
	 * which draws jump lines across.  I believe this is related to the fact that in this ase
	 * the map region is a clipped circle and we have no effective way of determining which points
	 * are outside the map of not.
	 */
	map_half_size = MAX (gmt_half_map_width (GMT, y0), gmt_half_map_width (GMT, y1));
	if (fabs (map_half_size) < GMT_CONV4_LIMIT) return (0);
	dx = x1 - x0;
	if (dx > map_half_size)	return (-1);	/* Cross left/west boundary */
	if (dx < (-map_half_size)) return (1);	/* Cross right/east boundary */
#else
	gmt_M_unused(GMT);
	gmt_M_unused(x0);
	gmt_M_unused(y0);
	gmt_M_unused(x1);
	gmt_M_unused(y1);
#endif
	return (0);
}

#if 0
/* NOT USED ?? */
GMT_LOCAL int map_ellipse_crossing (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides) {
	/* Compute the crossover point(s) on the map boundary for rectangular projections */
	int n = 0, i, jump;
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
		gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);
		gmt_geo_to_xy (GMT, lon2, lat2, &x2, &y2);
		if ((jump = map_jump_x (GMT, x2, y2, x1, y1))) {
			(*GMT->current.map.get_crossings) (GMT, xx, yy, x2, y2, x1, y1);
			if (jump == 1) {	/* Add right border point first */
				gmt_M_double_swap (xx[0], xx[1]);
				gmt_M_double_swap (yy[0], yy[1]);
			}
			gmt_xy_to_geo (GMT, &clon[0], &clat[0], xx[0], yy[0]);
			gmt_xy_to_geo (GMT, &clon[1], &clat[1], xx[1], yy[1]);
			n = 2;
		}
	}
	if (n == 1) for (i = 0; i < n; i++) gmt_geo_to_xy (GMT, clon[i], clat[i], &xx[i], &yy[i]);
	return (n);
}

/* NOT USED ?? */
GMT_LOCAL bool map_eqdist_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	double cc, s, c;

	lon -= GMT->current.proj.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	sincosd (lat, &s, &c);
	cc = GMT->current.proj.sinp * s + GMT->current.proj.cosp * c * cosd (lon);
	if (cc < -1.0) {
		GMT->current.map.this_y_status = -1;
		GMT->current.map.this_x_status = 0;
	}
	else
		GMT->current.map.this_x_status = GMT->current.map.this_y_status = 0;
	return (GMT->current.map.this_y_status != 0);
}

GMT_LOCAL int map_eqdist_crossing (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides) {
	double angle, x, y, s, c;

	/* Computes the x.y of the antipole point that lies on a radius from
	 * the origin through the inside point */

	if (map_eqdist_outside (GMT, lon1, lat1)) {	/* Point 1 is on perimeter */
		gmt_geo_to_xy (GMT, lon2, lat2, &x, &y);
		angle = d_atan2 (y - GMT->current.proj.origin[GMT_Y], x - GMT->current.proj.origin[GMT_X]);
		sincos (angle, &s, &c);
		xx[0] = GMT->current.proj.r * c + GMT->current.proj.origin[GMT_X];
		yy[0] = GMT->current.proj.r * s + GMT->current.proj.origin[GMT_Y];
		clon[0] = lon1;
		clat[0] = lat1;
	}
	else {	/* Point 2 is on perimeter */
		gmt_geo_to_xy (GMT, lon1, lat1, &x, &y);
		angle = d_atan2 (y - GMT->current.proj.origin[GMT_Y], x - GMT->current.proj.origin[GMT_X]);
		sincos (angle, &s, &c);
		xx[0] = GMT->current.proj.r * c + GMT->current.proj.origin[GMT_X];
		yy[0] = GMT->current.proj.r * s + GMT->current.proj.origin[GMT_Y];
		clon[0] = lon2;
		clat[0] = lat2;
	}
	sides[0] = 1;

	return (1);
}
#endif

/*  Routines to do with clipping */

/*! . */
GMT_LOCAL unsigned int map_crossing (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double *xlon, double *xlat, double *xx, double *yy, unsigned int *sides) {
	if (GMT->current.map.prev_x_status == GMT->current.map.this_x_status && GMT->current.map.prev_y_status == GMT->current.map.this_y_status) {
		/* This is naive. We could have two points outside with a line drawn between crossing the plotting area. */
		return (0);
	}
	else if ((GMT->current.map.prev_x_status == 0 && GMT->current.map.prev_y_status == 0) || (GMT->current.map.this_x_status == 0 && GMT->current.map.this_y_status == 0)) {
		/* This is a crossing */
	}
	else if (!(*GMT->current.map.overlap) (GMT, lon1, lat1, lon2, lat2))	/* Less clearcut case, check for overlap */
		return (0);

	/* Now compute the crossing */

	GMT->current.map.corner = -1;
	return ((*GMT->current.map.crossing) (GMT, lon1, lat1, lon2, lat2, xlon, xlat, xx, yy, sides));
}

/*! . */
GMT_LOCAL double map_x_to_corner (struct GMT_CTRL *GMT, double x) {
	return ( (fabs (x - GMT->current.proj.rect[XLO]) < fabs (x - GMT->current.proj.rect[XHI])) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI]);
}

/*! . */
GMT_LOCAL double map_y_to_corner (struct GMT_CTRL *GMT, double y) {
	return ( (fabs (y - GMT->current.proj.rect[YLO]) < fabs (y - GMT->current.proj.rect[YHI])) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI]);
}

/*! . */
GMT_LOCAL uint64_t map_move_to_rect (struct GMT_CTRL *GMT, double *x_edge, double *y_edge, uint64_t j, uint64_t nx) {
	uint64_t n = 0;
	int key;
	double xtmp, ytmp;

	/* May add 0, 1, or 2 points to path */

	if (GMT->current.map.this_x_status == 0 && GMT->current.map.this_y_status == 0) return (1);	/* Completely Inside */

	if (!nx && j > 0 && GMT->current.map.this_x_status != GMT->current.map.prev_x_status && GMT->current.map.this_y_status != GMT->current.map.prev_y_status) {	/* Must include corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((GMT->current.map.this_x_status * GMT->current.map.prev_x_status) == -4 || (GMT->current.map.this_y_status * GMT->current.map.prev_y_status) == -4) {	/* the two points outside on opposite sides */
			x_edge[j] = (GMT->current.map.prev_x_status < 0) ? GMT->current.proj.rect[XLO] : ((GMT->current.map.prev_x_status > 0) ? GMT->current.proj.rect[XHI] : map_x_to_corner (GMT, x_edge[j-1]));
			y_edge[j] = (GMT->current.map.prev_y_status < 0) ? GMT->current.proj.rect[YLO] : ((GMT->current.map.prev_y_status > 0) ? GMT->current.proj.rect[YHI] : map_y_to_corner (GMT, y_edge[j-1]));
			j++;
			x_edge[j] = (GMT->current.map.this_x_status < 0) ? GMT->current.proj.rect[XLO] : ((GMT->current.map.this_x_status > 0) ? GMT->current.proj.rect[XHI] : map_x_to_corner (GMT, xtmp));
			y_edge[j] = (GMT->current.map.this_y_status < 0) ? GMT->current.proj.rect[YLO] : ((GMT->current.map.this_y_status > 0) ? GMT->current.proj.rect[YHI] : map_y_to_corner (GMT, ytmp));
			j++;
		}
		else {
			key = MIN (GMT->current.map.this_x_status, GMT->current.map.prev_x_status);
			x_edge[j] = (key < 0) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI];
			key = MIN (GMT->current.map.this_y_status, GMT->current.map.prev_y_status);
			y_edge[j] = (key < 0) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI];
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}

	if (GMT->current.map.outside == map_rect_outside2) {	/* Need special check because this outside2 test is screwed up... */
		if (x_edge[j] < GMT->current.proj.rect[XLO]) {
			x_edge[j] = GMT->current.proj.rect[XLO];
			GMT->current.map.this_x_status = -2;
		}
		else if (x_edge[j] > GMT->current.proj.rect[XHI]) {
			x_edge[j] = GMT->current.proj.rect[XHI];
			GMT->current.map.this_x_status = 2;
		}
		if (y_edge[j] < GMT->current.proj.rect[YLO]) {
			y_edge[j] = GMT->current.proj.rect[YLO];
			GMT->current.map.this_y_status = -2;
		}
		else if (y_edge[j] > GMT->current.proj.rect[YHI]) {
			y_edge[j] = GMT->current.proj.rect[YHI];
			GMT->current.map.this_y_status = 2;
		}
	}
	else {
		if (GMT->current.map.this_x_status != 0) x_edge[j] = (GMT->current.map.this_x_status < 0) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI];
		if (GMT->current.map.this_y_status != 0) y_edge[j] = (GMT->current.map.this_y_status < 0) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI];
	}

	return (n + 1);
}

/*! . */
GMT_LOCAL uint64_t map_rect_clip_old (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n, double **x, double **y, uint64_t *total_nx) {
	uint64_t i, j = 0;
	unsigned int nx, k, sides[4];
	double xlon[4], xlat[4], xc[4], yc[4];

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	gmt_prep_tmp_arrays (GMT, GMT_NOTSET, 1, 2);	/* Init or reallocate tmp vectors */
	(void) gmt_map_outside (GMT, lon[0], lat[0]);
	gmt_geo_to_xy (GMT, lon[0], lat[0], &GMT->hidden.mem_coord[GMT_X][0], &GMT->hidden.mem_coord[GMT_Y][0]);
	j += map_move_to_rect (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], 0, 0);
	for (i = 1; i < n; i++) {
		(void) gmt_map_outside (GMT, lon[i], lat[i]);
		nx = map_crossing (GMT, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			gmt_prep_tmp_arrays (GMT, GMT_NOTSET, j, 2);	/* Init or reallocate tmp vectors */
			GMT->hidden.mem_coord[GMT_X][j] = xc[k];
			GMT->hidden.mem_coord[GMT_Y][j++] = yc[k];
			(*total_nx) ++;
		}
		gmt_geo_to_xy (GMT, lon[i], lat[i], &GMT->hidden.mem_coord[GMT_X][j], &GMT->hidden.mem_coord[GMT_Y][j]);
		gmt_prep_tmp_arrays (GMT, GMT_NOTSET, j+2, 2);	/* Init or reallocate tmp vectors */
		j += map_move_to_rect (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], j, nx);	/* May add 2 points, which explains the j+2 stuff */
	}

	*x = gmtlib_assign_vector (GMT, j, GMT_X);
	*y = gmtlib_assign_vector (GMT, j, GMT_Y);

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

/*! This macro calculates the x-coordinates where the line segment crosses the border x = border.
 * By swapping x and y in the call we can use it for finding the y intersection. This macro is
 * never called when (y_prev - y_curr) = 0 so we don't divide by zero.
 */
#define INTERSECTION_COORD(x_curr,y_curr,x_prev,y_prev,border) x_curr + (x_prev - x_curr) * (border - y_curr) / (y_prev - y_curr)

/*! . */
GMT_LOCAL unsigned int map_clip_sn (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, bool (*inside) (double, double), bool (*outside) (double, double), int *cross) {
	/* Clip against the south or north boundary (i.e., a horizontal line with y = border) */
	*cross = 0;
	if (doubleAlmostEqualZero (x_prev, x_curr) && doubleAlmostEqualZero (y_prev, y_curr))
		return (0);	/* Do nothing for duplicates */
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

/*! . */
GMT_LOCAL unsigned int map_clip_we (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, bool (*inside) (double, double), bool (*outside) (double, double), int *cross) {
	/* Clip against the west or east boundary (i.e., a vertical line with x = border) */
	*cross = 0;
	if (doubleAlmostEqualZero (x_prev, x_curr) && doubleAlmostEqualZero (y_prev, y_curr))
		return (0);	/* Do nothing for duplicates */
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
static inline bool gmt_inside_lower_boundary (double val, double min) {return (val >= min);}
static inline bool gmt_inside_upper_boundary (double val, double max) {return (val <= max);}
static inline bool gmt_outside_lower_boundary (double val, double min) {return (val < min);}
static inline bool gmt_outside_upper_boundary (double val, double max) {return (val > max);}

/*! map_rect_clip is an implementation of the Sutherland/Hodgman algorithm polygon clipping algorithm.
 * Basically, it compares the polygon to one boundary at the time, and clips the polygon to be inside
 * that boundary; this is then repeated for all boundaries.  Assumptions here are Cartesian coordinates
 * so all boundaries are straight lines in x or y. */
GMT_LOCAL uint64_t map_rect_clip (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n, double **x, double **y, uint64_t *total_nx) {
	uint64_t i, n_get, m;
	size_t n_alloc = 0;
	unsigned int side, in = 1, out = 0, j, np;
	int cross = 0;
	bool polygon;
	double *xtmp[2] = {NULL, NULL}, *ytmp[2] = {NULL, NULL}, xx[2], yy[2], border[4];
	unsigned int (*clipper[4]) (double, double, double, double, double *, double *, double, bool (*inside) (double, double), bool (*outside) (double, double), int *);
	bool (*inside[4]) (double, double);
	bool (*outside[4]) (double, double);

#ifdef DEBUG
	FILE *fp = NULL;
	bool dump = false;
#endif

	if (n == 0) return (0);

	polygon = !gmt_polygon_is_open (GMT, lon, lat, n);	/* true if input segment is a closed polygon */

	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */

	/* Set up function pointers.  This could be done once in gmt_begin at some point */

	clipper[GMT_BOTTOM] = map_clip_sn;	clipper[GMT_RIGHT] = map_clip_we; clipper[GMT_TOP] = map_clip_sn;	clipper[GMT_LEFT] = map_clip_we;
	inside[GMT_RIGHT] = inside[GMT_TOP] = gmt_inside_upper_boundary;	outside[GMT_RIGHT] = outside[GMT_TOP] = gmt_outside_upper_boundary;
	inside[GMT_BOTTOM] = inside[GMT_LEFT] = gmt_inside_lower_boundary;		outside[GMT_BOTTOM] = outside[GMT_LEFT] = gmt_outside_lower_boundary;
	border[GMT_BOTTOM] = border[GMT_LEFT] = 0.0;	border[GMT_RIGHT] = GMT->current.map.width;	border[GMT_TOP] = GMT->current.map.height;

	n_get = lrint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], n_get, &n_alloc, double);

	/* Get Cartesian map coordinates */

	for (m = 0; m < n; m++) gmt_geo_to_xy (GMT, lon[m], lat[m], &xtmp[0][m], &ytmp[0][m]);

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

		gmt_M_uint_swap (in, out);	/* Swap what is input and output for clipping against this border */
		if (n) {
			/* Must ensure we copy the very first point if it is inside the clip rectangle */
			if (inside[side] ((side%2) ? xtmp[in][0] : ytmp[in][0], border[side])) {xtmp[out][0] = xtmp[in][0]; ytmp[out][0] = ytmp[in][0]; m = 1;}	/* First point is inside; add it */
		}
		for (i = 1; i < n; i++) {	/* For each line segment */
			np = clipper[side] (xtmp[in][i-1], ytmp[in][i-1], xtmp[in][i], ytmp[in][i], xx, yy, border[side], inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, &n_alloc, double);
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && gmt_polygon_is_open (GMT, xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, &n_alloc, double);
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
	}

	gmt_M_free (GMT, xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	gmt_M_free (GMT, ytmp[1]);

	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = m;
		gmt_M_malloc2 (GMT, xtmp[0], ytmp[0], 0U, &n_alloc, double);
		*x = xtmp[0];
		*y = ytmp[0];
#ifdef DEBUG
		if (dump) {
			fp = fopen ("output.d", "w");
			for (i = 0; i < m; i++) fprintf (fp, "%g\t%g\n", xtmp[0][i], ytmp[0][i]);
			fclose (fp);
		}
#endif
		if (GMT->common.R.oblique) {	/* Ensure region corners are added if any poles are enclosed */
			double ycoord_pole = 0;
			bool do_it = true;
			if (GMT->current.proj.corner[0] && GMT->current.proj.corner[1])	/* Ensure S pole is included */
				ycoord_pole = GMT->current.proj.rect[YLO];
			else if (GMT->current.proj.corner[2] && GMT->current.proj.corner[3])	/* Ensure N pole is included */
				ycoord_pole = GMT->current.proj.rect[YHI];
			else
				do_it = false;

			if (do_it) {
				for (i = 1; i < m; i++) {
					if (doubleAlmostEqual (fabs (xtmp[0][i] - xtmp[0][i-1]), GMT->current.map.width))
						ytmp[0][i] = ytmp[0][i-1] = ycoord_pole;
				}
			}
		}
	}
	else {	/* Nothing survived the clipping - free the output arrays */
		gmt_M_free (GMT, xtmp[0]);
		gmt_M_free (GMT, ytmp[0]);
	}

	return (m);
}

/* map_wesn_clip differs from map_rect_clip in that the boundaries of constant lon or lat may end up as
 * curved lines depending on the map projection.  Thus, if a line crosses the boundary and reenters at
 * another point on the boundary then the straight line between these crossing points should really
 * project to a curved boundary segment.  The H-S algorithm was originally rectangular so we got straight
 * lines.  Here, we check if (1) the particular boundary being tested is curved, and if true then we
 * keep track of the indices of the exit and entry points in the array, and once a boundary has been
 * processed we must add more points between the exit and entry pairs to properly handle the curved
 * segment.  The arrays x_index and x_type stores the index of the exit/entry points and the type
 * (+1 we enter, -1 we exit).  We then use gmtlib_map_path to compute the required segments to insert.
 * P. Wessel, 2--9-05-07
 */

/*! . */
GMT_LOCAL double map_lon_to_corner (struct GMT_CTRL *GMT, double lon) {
	return ( (fabs (lon - GMT->common.R.wesn[XLO]) < fabs (lon - GMT->common.R.wesn[XHI])) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI]);
}

/*! . */
GMT_LOCAL double map_lat_to_corner (struct GMT_CTRL *GMT, double lat) {
	return ( (fabs (lat - GMT->common.R.wesn[YLO]) < fabs (lat - GMT->common.R.wesn[YHI])) ? GMT->common.R.wesn[YLO] : GMT->common.R.wesn[YHI]);
}

/*! . */
GMT_LOCAL int map_move_to_wesn (struct GMT_CTRL *GMT, double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, uint64_t j, uint64_t nx) {
	int n = 0, key;
	double xtmp, ytmp, lon_p, lat_p;

	/* May add 0, 1, or 2 points to path */

	if (!nx && j > 0 && GMT->current.map.this_x_status != GMT->current.map.prev_x_status && GMT->current.map.this_y_status != GMT->current.map.prev_y_status) {	/* Need corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((GMT->current.map.this_x_status * GMT->current.map.prev_x_status) == -4 || (GMT->current.map.this_y_status * GMT->current.map.prev_y_status) == -4) {	/* the two points outside on opposite sides */
			lon_p = (GMT->current.map.prev_x_status < 0) ? GMT->common.R.wesn[XLO] : ((GMT->current.map.prev_x_status > 0) ? GMT->common.R.wesn[XHI] : map_lon_to_corner (GMT, lon_old));
			lat_p = (GMT->current.map.prev_y_status < 0) ? GMT->common.R.wesn[YLO] : ((GMT->current.map.prev_y_status > 0) ? GMT->common.R.wesn[YHI] : map_lat_to_corner (GMT, lat_old));
			gmt_geo_to_xy (GMT, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
			lon_p = (GMT->current.map.this_x_status < 0) ? GMT->common.R.wesn[XLO] : ((GMT->current.map.this_x_status > 0) ? GMT->common.R.wesn[XHI] : map_lon_to_corner (GMT, lon));
			lat_p = (GMT->current.map.this_y_status < 0) ? GMT->common.R.wesn[YLO] : ((GMT->current.map.this_y_status > 0) ? GMT->common.R.wesn[YHI] : map_lat_to_corner (GMT, lat));
			gmt_geo_to_xy (GMT, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		else {
			key = MIN (GMT->current.map.this_x_status, GMT->current.map.prev_x_status);
			lon_p = (key < 0) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
			key = MIN (GMT->current.map.this_y_status, GMT->current.map.prev_y_status);
			lat_p = (key < 0) ? GMT->common.R.wesn[YLO] : GMT->common.R.wesn[YHI];
			gmt_geo_to_xy (GMT, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}
	if (GMT->current.map.this_x_status != 0) lon = (GMT->current.map.this_x_status < 0) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
	if (GMT->current.map.this_y_status != 0) lat = (GMT->current.map.this_y_status < 0) ? GMT->common.R.wesn[YLO] : GMT->common.R.wesn[YHI];
	gmt_geo_to_xy (GMT, lon, lat, &x_edge[j], &y_edge[j]);
	return (n + 1);
}

/*! . */
GMT_LOCAL uint64_t map_wesn_clip_old (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n, double **x, double **y, uint64_t *total_nx) {
	uint64_t i, j = 0, nx, k;
	unsigned int sides[4];
	double xlon[4], xlat[4], xc[4], yc[4];

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	gmt_prep_tmp_arrays (GMT, GMT_NOTSET, 1, 2);	/* Init or reallocate tmp vectors */

	(void) gmt_map_outside (GMT, lon[0], lat[0]);
	j = map_move_to_wesn (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], lon[0], lat[0], 0.0, 0.0, 0, 0);	/* Add one point */

	for (i = 1; i < n; i++) {
		(void) gmt_map_outside (GMT, lon[i], lat[i]);
		nx = map_crossing (GMT, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			gmt_prep_tmp_arrays (GMT, GMT_NOTSET, j, 2);	/* Init or reallocate tmp vectors */
			GMT->hidden.mem_coord[GMT_X][j]   = xc[k];
			GMT->hidden.mem_coord[GMT_Y][j++] = yc[k];
			(*total_nx) ++;
		}
		gmt_prep_tmp_arrays (GMT, GMT_NOTSET, j+2, 2);	/* Init or reallocate tmp vectors */
		j += map_move_to_wesn (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], lon[i], lat[i], lon[i-1], lat[i-1], j, nx);	/* May add 2 points, which explains the j+2 stuff */
	}

	*x = gmtlib_assign_vector (GMT, j, GMT_X);
	*y = gmtlib_assign_vector (GMT, j, GMT_Y);

#ifdef CRAP
{
	FILE *fp = NULL;
	double out[2];
	fp = fopen ("crap.d", "a");
	fprintf (fp, "> N = %d\n", (int)j);
	for (i = 0; i < j; i++) {
		out[GMT_X] = *x[i];
		out[GMT_Y] = *y[i];
		GMT->current.io.output (GMT, fp, 2, out);
	}
	fclose (fp);
}
#endif
	return (j);
}

/*! . */
uint64_t map_wesn_clip (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n_orig, double **x, double **y, uint64_t *total_nx) {
	char *x_type = NULL;
	size_t n_alloc = 0, n_x_alloc = 0, n_t_alloc = 0;
	uint64_t new_n, i, n_get, n, m, n_cross = 0, *x_index = NULL;
	unsigned int j, np, side, in = 1, out = 0;
	int cross = 0;
	bool curved, jump = false, polygon, periodic = false;
	double *xtmp[2] = {NULL, NULL}, *ytmp[2] = {NULL, NULL}, xx[2], yy[2], border[4];
	double x1, x2, y1, y2;
	unsigned int (*clipper[4]) (double, double, double, double, double *, double *, double, bool (*inside) (double, double), bool (*outside) (double, double), int *);
	bool (*inside[4]) (double, double);
	bool (*outside[4]) (double, double);

#ifdef DEBUG
	FILE *fp = NULL;
	bool dump = false;
#endif

	if ((n = n_orig) == 0) return (0);

	/* If there are jumps etc call the old clipper, else we try the new clipper */

	gmt_geo_to_xy (GMT, lon[0], lat[0], &x1, &y1);
	for (i = 1; !jump && i < n; i++) {
		gmt_geo_to_xy (GMT, lon[i], lat[i], &x2, &y2);
		jump = map_jump_x (GMT, x2, y2, x1, y1);
		x1 = x2;	y1 = y2;
	}

	if (jump) return (map_wesn_clip_old (GMT, lon, lat, n, x, y, total_nx));	/* Must do the old way for now */
	periodic = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);	/* No point clipping against W and E if periodic map */

	/* Here we can try the Sutherland/Hodgman algorithm */

	polygon = !gmt_polygon_is_open (GMT, lon, lat, n);	/* true if input segment is a closed polygon */

	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */

	/* Set up function pointers.  This could be done once in gmt_begin at some point */

	clipper[GMT_BOTTOM] = map_clip_sn;	clipper[GMT_RIGHT] = map_clip_we; clipper[GMT_TOP] = map_clip_sn;	clipper[GMT_LEFT] = map_clip_we;
	inside[GMT_RIGHT] = inside[GMT_TOP] = gmt_inside_upper_boundary;	outside[GMT_RIGHT] = outside[GMT_TOP] = gmt_outside_upper_boundary;
	inside[GMT_BOTTOM] = inside[GMT_LEFT] = gmt_inside_lower_boundary;	outside[GMT_BOTTOM] = outside[GMT_LEFT] = gmt_outside_lower_boundary;
	border[GMT_BOTTOM] = GMT->common.R.wesn[YLO]; border[GMT_LEFT] = GMT->common.R.wesn[XLO];	border[GMT_RIGHT] = GMT->common.R.wesn[XHI];	border[GMT_TOP] = GMT->common.R.wesn[YHI];

/* This is new approach to get rid of those crossing lines for filled polygons,
 * i.e., issue # 949.  Also see comments further down.
 * P. Wessel, Dec 1 2016 */
	if (GMT->current.map.coastline && periodic) {	/* Make data longitudes have no jumps [This is for pscoast] */
		for (i = 0; i < n; i++) {
			if (lon[i] < border[GMT_LEFT] && (lon[i] + 360.0) <= border[GMT_RIGHT])
				lon[i] += 360.0;
			else if (lon[i] > border[GMT_RIGHT] && (lon[i] - 360.0) >= border[GMT_LEFT])
				lon[i] -= 360.0;
		}
	}
	else {	/* Don't let points be > 180 from center longitude */
		double mid_lon = 0.5 * (border[GMT_LEFT] + border[GMT_RIGHT]), diff;
		for (i = 0; i < n; i++) {
			diff = lon[i] - mid_lon;
			if (diff > 180.0) lon[i] -= 360.0;
			else if (diff < -180.0) lon[i] += 360.0;
		}
	}
	if (!GMT->current.map.coastline) {	/* Not do if pscoast since it has its own oddness */
		double xmin, xmax;
		gmtlib_get_lon_minmax (GMT, lon, n, &xmin, &xmax);
		xmin -= 360.0;	xmax -= 360.0;
		while (xmax < border[GMT_LEFT]) {xmin += 360.0;	xmax += 360.0;}
		if (xmin > border[GMT_RIGHT]) return 0;	/* Outside */
	}

	n_get = lrint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], n_get, &n_alloc, double);

	/* Make copy of lon/lat coordinates */

	gmt_M_memcpy (xtmp[0], lon, n, double);	gmt_M_memcpy (ytmp[0], lat, n, double);
	m = n;

	/* Preallocate space for crossing information */

	x_index = gmt_M_malloc (GMT, NULL, GMT_TINY_CHUNK, &n_x_alloc, uint64_t);
	x_type = gmt_M_malloc (GMT, NULL,  GMT_TINY_CHUNK, &n_t_alloc, char);

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

		curved = !((side%2) ? GMT->current.map.meridian_straight : GMT->current.map.parallel_straight);	/* Is this border straight or curved when projected */
		gmt_M_uint_swap (in, out);	/* Swap what is input and output for clipping against this border */
		if (side%2 && periodic) {	/* No clipping can take place on w or e border; just copy all and go to next side */
			m = n;
			if (m == n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, &n_alloc, double);
			gmt_M_memcpy (xtmp[out], xtmp[in], m, double);
			gmt_M_memcpy (ytmp[out], ytmp[in], m, double);
			continue;
		}
#if 0
/* This caused lots of issues with various map types so replaced by the check about
 * that uses the mid-longitude as center and does a +/- 180 test from that.
 * P. Wessel, Dec 1 2016 */
		if (!GMT->current.map.coastline && side % 2) {	/* Either left or right border */
			/* For non-periodic maps we have to be careful to position the polygon so it does
			 * not have longitude jumps at the current border.  This does not apply to pscoast
			 * which has special handling and hence bypasses this test.
			 * Note that we may be using the map longitude range if this exceeds 180
			 * as it is tricky to make those plots with large but < 360 longitudes. */
			//double map_lon_range = MAX (180.0, border[GMT_RIGHT] - border[GMT_LEFT]);
			double map_lon_range = 1800.0, diff;
			for (i = 0; i < n; i++) {	/* If points is > map_lon_range degrees from border, flip side */
				diff = xtmp[in][i] - border[side];
				if (diff > map_lon_range) xtmp[in][i] -= 360.0;
				else if (diff < -map_lon_range) xtmp[in][i] += 360.0;
			}
		}
#endif
		if (n) {
			/* Must ensure we copy the very first point if it is inside the clip rectangle */
			if (inside[side] ((side%2) ? xtmp[in][0] : ytmp[in][0], border[side])) {xtmp[out][0] = xtmp[in][0]; ytmp[out][0] = ytmp[in][0]; m = 1;}	/* First point is inside; add it */
		}
		for (i = 1; i < n; i++) {	/* For each line segment */
			np = clipper[side] (xtmp[in][i-1], ytmp[in][i-1], xtmp[in][i], ytmp[in][i], xx, yy, border[side], inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			if (polygon && cross && curved) {	/* When crossing in/out of a curved boundary we must eventually sample along the curve between crossings */
				x_index[n_cross] = m;		/* Index of intersection point (which will be copied from xx[0], yy[0] below) */
				x_type[n_cross] = (char)cross;	/* -1 going out, +1 going in */
				if (++n_cross == n_x_alloc) {
					x_index = gmt_M_malloc (GMT, x_index, n_cross, &n_x_alloc, uint64_t);
					x_type = gmt_M_malloc (GMT, x_type,  n_cross, &n_t_alloc, char);
				}
			}
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, &n_alloc, double);
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && gmt_polygon_is_open (GMT, xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, &n_alloc, double);
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
		if (polygon && curved && n_cross) {	/* Must resample between crossing points */
			double *x_add = NULL, *y_add = NULL, *x_cpy = NULL, *y_cpy = NULL;
			size_t np = 0;
			uint64_t add, last_index = 0, p, p_next;

			if (n_cross%2 == 1) {	/* Should not happen with a polygon */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in map_wesn_clip: odd number of crossings?");
			}

			/* First copy the current polygon */

			gmt_M_malloc2 (GMT, x_cpy, y_cpy, m, &np, double);
			gmt_M_memcpy (x_cpy, xtmp[out], m, double);
			gmt_M_memcpy (y_cpy, ytmp[out], m, double);

			for (p = np = 0; p < n_cross; p++) {	/* Process each crossing point */
				if (last_index < x_index[p]) {	/* Copy over segment from were we left off to this crossing point */
					add = x_index[p] - last_index;
					if ((new_n = (np+add)) >= n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, &n_alloc, double);
					gmt_M_memcpy (&xtmp[out][np], &x_cpy[last_index], add, double);
					gmt_M_memcpy (&ytmp[out][np], &y_cpy[last_index], add, double);
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
					add = gmtlib_map_path (GMT, start_lon, y_cpy[x_index[p]], stop_lon, y_cpy[x_index[p_next]], &x_add, &y_add);
					if ((new_n = (np+add)) >= n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, &n_alloc, double);
					gmt_M_memcpy (&xtmp[out][np], x_add, add, double);
					gmt_M_memcpy (&ytmp[out][np], y_add, add, double);
					if (add) { gmt_M_free (GMT, x_add);	gmt_M_free (GMT, y_add); }
					np += add;
					last_index = x_index[p_next];
				}
			}
			if (x_index[0] > 0) {	/* First point was clean inside, must add last connection */
				add = m - last_index;
				if ((new_n = (np+add)) >= n_alloc) gmt_M_malloc4 (GMT, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, &n_alloc, double);
				gmt_M_memcpy (&xtmp[out][np], &x_cpy[last_index], add, double);
				gmt_M_memcpy (&ytmp[out][np], &y_cpy[last_index], add, double);
				np += add;
			}
			m = np;	/* New total of points */
			gmt_M_free (GMT, x_cpy);	gmt_M_free (GMT, y_cpy);
		}
	}

	gmt_M_free (GMT, xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	gmt_M_free (GMT, ytmp[1]);
	gmt_M_free (GMT, x_index);	/* Free the pairs of arrays that holds the crossing info */
	gmt_M_free (GMT, x_type);

	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = m;
		gmt_M_malloc2 (GMT, xtmp[0], ytmp[0], 0U, &n_alloc, double);
		/* Convert to map coordinates */
		for (i = 0; i < m; i++) gmt_geo_to_xy (GMT, xtmp[0][i], ytmp[0][i], &xtmp[0][i], &ytmp[0][i]);

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
		gmt_M_free (GMT, xtmp[0]);
		gmt_M_free (GMT, ytmp[0]);
	}

	return (m);
}

/*! . */
GMT_LOCAL uint64_t map_radial_boundary_arc (struct GMT_CTRL *GMT, int this_way, double end_x[], double end_y[], double **xarc, double **yarc) {
	uint64_t n_arc, k, pt;
	double az1, az2, d_az, da, xr, yr, da_try, *xx = NULL, *yy = NULL;

	/* When a polygon crosses out then in again into the circle we need to add a boundary arc
	 * to the polygon where it is clipped.  We simply sample the circle as finely as the arc
	 * length and the current line_step demands */

	da_try = (GMT->current.setting.map_line_step * 360.0) / (TWO_PI * GMT->current.proj.r);	/* Angular step in degrees */
	az1 = d_atan2d (end_y[0], end_x[0]);	/* azimuth from map center to 1st crossing */
	az2 = d_atan2d (end_y[1], end_x[1]);	/* azimuth from map center to 2nd crossing */
	gmt_M_set_delta_lon (az1, az2, d_az);	/* Insist we take the short arc for now */
	n_arc = lrint (ceil (fabs (d_az) / da_try));	/* Get number of integer increments of da_try degree... */
	if (n_arc < 2) n_arc = 2;	/* ...but minimum 2 */
	da = d_az / (n_arc - 1);	/* Reset da to get exact steps */
	if (n_arc <= 2) return (0);	/* Arc is too short to have intermediate points */
	n_arc -= 2;	/* We do not include the end points since these are the crossing points handled in the calling function */
	gmt_M_malloc2 (GMT, xx, yy, n_arc, NULL, double);
	for (k = 1; k <= n_arc; k++) {	/* Create points along arc from first to second crossing point (k-loop excludes the end points) */
		sincosd (az1 + k * da, &yr, &xr);
		pt = (this_way) ? n_arc - k : k - 1;	/* The order we add the arc depends if we exited or entered the inside area */
		xx[pt] = GMT->current.proj.r * (1.0 + xr);
		yy[pt] = GMT->current.proj.r * (1.0 + yr);
	}

	*xarc = xx;
	*yarc = yy;
	return (n_arc);
}

#ifdef DEBUG
/* If we need to dump out clipped polygon then set clip_dump = 1 during execution */
GMT_LOCAL int clip_dump = 0, clip_id = 0;
GMT_LOCAL void map_dumppol (uint64_t n, double *x, double *y, int *id) {
	uint64_t i;
	FILE *fp = NULL;
	char line[GMT_LEN64];
	snprintf (line, GMT_LEN64, "dump_%d.d", *id);
	fp = fopen (line, "w");
	for (i = 0; i < n; i++) fprintf (fp, "%g\t%g\n", x[i], y[i]);
	fclose (fp);
	(*id)++;
}
#endif

/*! . */
GMT_LOCAL uint64_t map_radial_clip (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t np, double **x, double **y, uint64_t *total_nx) {
	size_t n_alloc = 0;
	uint64_t n = 0, n_arc;
	unsigned int i, nx;
	unsigned int sides[4];
	bool this_side = false, add_boundary = false;
	double xlon[4], xlat[4], xc[4], yc[4], end_x[3], end_y[3], xr, yr;
	double *xx = NULL, *yy = NULL, *xarc = NULL, *yarc = NULL;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (np == 0) return (0);

	if (!gmt_map_outside (GMT, lon[0], lat[0])) {
		gmt_M_malloc2 (GMT, xx, yy, n, &n_alloc, double);
		gmt_geo_to_xy (GMT, lon[0], lat[0], &xx[0], &yy[0]);
		n++;
	}
	nx = 0;
	for (i = 1; i < np; i++) {
		this_side = gmt_map_outside (GMT, lon[i], lat[i]);
		if (map_crossing (GMT, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides)) {
			if (this_side) {	/* Crossing boundary and leaving circle: Add exit point to the path */
				if (n == n_alloc) gmt_M_malloc2 (GMT, xx, yy, n, &n_alloc, double);
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
			end_x[nx] = xc[0] - GMT->current.proj.r;	end_y[nx] = yc[0] - GMT->current.proj.r;
			nx++;
			(*total_nx) ++;
			if (nx >= 2) {	/* Got a pair of entry+exit points */
				add_boundary = !this_side;	/* We only add boundary arcs if we first exited and now entered the circle again */
			}
			if (add_boundary) {	/* Crossed twice.  Now add arc between the two crossing points */
				/* PW: Currently, we make the assumption that the shortest arc is the one we want.  However,
				 * extremely large polygons could cut the boundary so that it is the longest arc we want.
				 * The way to improve this algorithm in the future is to find the two opposite points on
				 * the circle boundary that lies on the bisector of az1,az2, and see which point lies
				 * inside the polygon.  This would require that gmt_inonout_sphpol be called.
				 */
				if ((n_arc = map_radial_boundary_arc (GMT, this_side, &end_x[nx-2], &end_y[nx-2], &xarc, &yarc)) > 0) {
					if ((n + n_arc) >= n_alloc) gmt_M_malloc2 (GMT, xx, yy, n + n_arc, &n_alloc, double);
					gmt_M_memcpy (&xx[n], xarc, n_arc, double);	/* Copy longitudes of arc */
					gmt_M_memcpy (&yy[n], yarc, n_arc, double);	/* Copy latitudes of arc */
					n += n_arc;	/* Number of arc points added (end points are done separately) */
					gmt_M_free (GMT, xarc);	gmt_M_free (GMT,  yarc);
				}
				add_boundary = false;
				nx -= 2;	/* Done with those two crossings */
			}
			if (!this_side) {	/* Crossing boundary and entering circle: Add entry point to the path */
				if (n == n_alloc) gmt_M_malloc2 (GMT, xx, yy, n, &n_alloc, double);
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
		}
		gmt_geo_to_xy (GMT, lon[i], lat[i], &xr, &yr);
		if (!this_side) {	/* Only add points actually inside the map to the path */
			if (n == n_alloc) gmt_M_malloc2 (GMT, xx, yy, n, &n_alloc, double);
			xx[n] = xr;	yy[n] = yr;	n++;
		}
	}

	if (nx == 2) {	/* Must close polygon by adding boundary arc */
		if ((n_arc = map_radial_boundary_arc (GMT, this_side, end_x, end_y, &xarc, &yarc)) > 0) {
			if ((n + n_arc) >= n_alloc) gmt_M_malloc2 (GMT, xx, yy, n + n_arc, &n_alloc, double);
			gmt_M_memcpy (&xx[n], xarc, n_arc, double);	/* Copy longitudes of arc */
			gmt_M_memcpy (&yy[n], yarc, n_arc, double);	/* Copy latitudes of arc */
			n += n_arc;	/* Number of arc points added (end points are done separately) */
			gmt_M_free (GMT, xarc);	gmt_M_free (GMT,  yarc);
		}
		if (n == n_alloc) gmt_M_malloc2 (GMT, xx, yy, n, &n_alloc, double);
		xx[n] = xx[0];	yy[n] = yy[0];	n++;	/* Close the polygon */
	}
	n_alloc = n;
	gmt_M_malloc2 (GMT, xx, yy, 0U, &n_alloc, double);
	*x = xx;
	*y = yy;
#ifdef DEBUG
	if (clip_dump) map_dumppol (n, xx, yy, &clip_id);
#endif

	return (n);
}

/*! . */
GMT_LOCAL bool map_cartesian_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	/* Return true if the projection of either (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular map boundary */
	/* Here, lon,lat etc are Cartesian and not geographic coordinates, otherwise map_rect_overlap is used */
	double x0, y0, x1, y1;

	gmt_geo_to_xy (GMT, lon0, lat0, &x0, &y0);
	gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);

	if (x0 > x1) gmt_M_double_swap (x0, x1);
	if (y0 > y1) gmt_M_double_swap (y0, y1);

	if (x1 - GMT->current.proj.rect[XLO] < -GMT_CONV8_LIMIT || x0 - GMT->current.proj.rect[XHI] > GMT_CONV8_LIMIT) return (false);
	if (y1 - GMT->current.proj.rect[YLO] < -GMT_CONV8_LIMIT || y0 - GMT->current.proj.rect[YHI] > GMT_CONV8_LIMIT) return (false);
	return (true);
}

/*! . */
GMT_LOCAL bool map_rect_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	/* Return true if the projection of either (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular map boundary */
	double x0, y0, x1, y1;

	gmt_geo_to_xy (GMT, lon0, lat0, &x0, &y0);
	gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);

	if (x0 > x1) gmt_M_double_swap (x0, x1);
	if (y0 > y1) gmt_M_double_swap (y0, y1);

	if (x1 - GMT->current.proj.rect[XLO] < -GMT_CONV8_LIMIT || x0 - GMT->current.proj.rect[XHI] > GMT_CONV8_LIMIT) return (false);
	if (y1 - GMT->current.proj.rect[YLO] < -GMT_CONV8_LIMIT || y0 - GMT->current.proj.rect[YHI] > GMT_CONV8_LIMIT) return (false);
	if (x0 < GMT->current.proj.rect[XLO] && x1 > GMT->current.proj.rect[XHI]) {	/* Possibly a map jump but is it reasonable? */
		/* What can happen for a small non-360 map is that points that approach being +180 degrees in longitude away and the
		 * next point that is +181 will be seen as -179 degrees away and suddenly the x-coordinate jumps from very positive
		 * to very negative (or the other way).  Before this attempt, the function would return true and give crossing lines
		 * across the map.  Here we check if the change in x is a large (> 10x) multiple of the map width. This is likely
		 * not to be fool-proof.  Works with current test scripts so we will give it a go.  Paul Wessel, 7/31/2014 */
		if ((x1 - x0)/(GMT->current.proj.rect[XHI] - GMT->current.proj.rect[XLO]) > 10.0) return (false);
	}
	return (true);
}

/*! . */
GMT_LOCAL bool map_wesn_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	/* Return true if either of the points (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular lon/lat boundaries */
	if (lon0 > lon1) gmt_M_double_swap (lon0, lon1);
	if (lat0 > lat1) gmt_M_double_swap (lat0, lat1);
	if (lon1 - GMT->common.R.wesn[XLO] < -GMT_CONV8_LIMIT) {
		lon0 += 360.0;
		lon1 += 360.0;
	}
	else if (lon0 - GMT->common.R.wesn[XHI] > GMT_CONV8_LIMIT) {
		lon0 -= 360.0;
		lon1 -= 360.0;
	}

	if (lon1 - GMT->common.R.wesn[XLO] < -GMT_CONV8_LIMIT || lon0 - GMT->common.R.wesn[XHI] > GMT_CONV8_LIMIT) return (false);
	if (lat1 - GMT->common.R.wesn[YLO] < -GMT_CONV8_LIMIT || lat0 - GMT->common.R.wesn[YHI] > GMT_CONV8_LIMIT) return (false);
	return (true);
}

/*! . */
GMT_LOCAL bool map_radial_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	/* Dummy routine */
	gmt_M_unused(GMT); gmt_M_unused(lon0); gmt_M_unused(lat0); gmt_M_unused(lon1); gmt_M_unused(lat1);
	return (true);
}

/*! . */
GMT_LOCAL bool map_genperg_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	/* Dummy routine */
	gmt_M_unused(lon0); gmt_M_unused(lat0); gmt_M_unused(lon1); gmt_M_unused(lat1);
	if (GMT->current.proj.g_debug > 0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper_overlap: overlap called\n");
	return (true);
}

/*! . */
GMT_LOCAL bool map_genperw_overlap (struct GMT_CTRL *GMT, double lon0, double lat0, double lon1, double lat1) {
	bool out0, out1;
	gmt_M_unused(lon0); gmt_M_unused(lat0); gmt_M_unused(lon1); gmt_M_unused(lat1);
	/* Return true if either of the points (lon0,lat0) and (lon1,lat1) is inside (not on) the windowed genper boundary */
	/* Check if point 1 is beyond horizon: */
	out0 = map_radial_outside (GMT, lon0, lat0);		/* true if point 0 is beyond the horizon */
	if (!out0) out0 = map_rect_outside (GMT, lon0, lat0);	/* true if point 0 is beyond the map box */
	out1 = map_radial_outside (GMT, lon1, lat1);		/* true if point 1 is beyond the horizon */
	if (!out1) out1 = map_rect_outside (GMT, lon1, lat1);	/* true if point 1 is beyond the map box */
	return (out0 != out1);
}

/*! . */
GMT_LOCAL void map_xy_search (struct GMT_CTRL *GMT, double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0) {
	unsigned int i, j;
	double xmin, xmax, ymin, ymax, w, s, x, y, dlon, dlat;

	/* Find min/max forward values */

	xmax = ymax = -DBL_MAX;
	xmin = ymin = DBL_MAX;
	dlon = fabs (e0 - w0) / 500;
	dlat = fabs (n0 - s0) / 500;

	for (i = 0; i <= 500; i++) {
		w = w0 + i * dlon;
		(*GMT->current.proj.fwd) (GMT, w, s0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*GMT->current.proj.fwd) (GMT, w, n0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}
	for (j = 0; j <= 500; j++) {
		s = s0 + j * dlat;
		(*GMT->current.proj.fwd) (GMT, w0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*GMT->current.proj.fwd) (GMT, e0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}

	*x0 = xmin;	*x1 = xmax;	*y0 = ymin;	*y1 = ymax;
}

/*! . */
GMT_LOCAL void adjust_panel_for_gaps (struct GMT_CTRL *GMT, struct GMT_SUBPLOT *P) {
	/* Checks the caps array and makes adjustment to w/h and adjusts the x/y origin */
	gmt_M_unused (GMT);

	/* Shrink the available panel dimensions based on the gaps */
	P->w -= (P->gap[XLO] + P->gap[XHI]);
	P->h -= (P->gap[YLO] + P->gap[YHI]);
}

/*! . */
GMT_LOCAL void map_setxy (struct GMT_CTRL *GMT, double xmin, double xmax, double ymin, double ymax) {
	/* Set x/y parameters */
	struct GMT_SUBPLOT *P = &(GMT->current.plot.panel);	/* P->active == 1 if a subplot */
	struct GMT_INSET *I = &(GMT->current.plot.inset);	/* I->active == 1 if an inset */
	unsigned int no_scaling = P->no_scaling;
	bool update_parameters = false;
	double fw, fh, fx, fy, w, h;

	/* Set up the original min/max values, the rectangular map dimensionsm and the projection offset */
	GMT->current.proj.rect_m[XLO] = xmin;	GMT->current.proj.rect_m[XHI] = xmax;	/* This is in original meters */
	GMT->current.proj.rect_m[YLO] = ymin;	GMT->current.proj.rect_m[YHI] = ymax;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Projected values in meters: %g %g %g %g\n", xmin, xmax, ymin, ymax);
	GMT->current.proj.rect[XHI] = (xmax - xmin) * GMT->current.proj.scale[GMT_X];
	GMT->current.proj.rect[YHI] = (ymax - ymin) * GMT->current.proj.scale[GMT_Y];
	GMT->current.proj.origin[GMT_X] = -xmin * GMT->current.proj.scale[GMT_X];
	GMT->current.proj.origin[GMT_Y] = -ymin * GMT->current.proj.scale[GMT_Y];
	if (GMT->current.proj.obl_flip) {
		GMT->current.proj.origin[GMT_Y] = ymax * GMT->current.proj.scale[GMT_Y];
		GMT->current.proj.scale[GMT_Y] = -GMT->current.proj.scale[GMT_Y];
	}

	if (!strncmp (GMT->init.module_name, "inset", 5U))
		no_scaling = 1;	/* Don't scale yet if we are calling inset begin (inset end would come here too but not affected since no mapping done by that module) */

	w = GMT->current.proj.rect[XHI];	h = GMT->current.proj.rect[YHI];

	/* Check inset first since an inset may be inside a subplot but there are no subplots inside an inset */
	if (I->active && no_scaling == 0) {	/* Must rescale to fit inside the inset dimensions and set dx,dy for centering */
		fw = w / I->w;	fh = h / I->h;
		if (gmt_M_is_geographic (GMT, GMT_IN) || GMT->current.proj.projection == GMT_POLAR || GMT->current.proj.gave_map_width == 0) {	/* Giving -Jx will end up here with map projections */
			if (fw > fh) {	/* Wider than taller given inset dims; adjust width to fit exactly and set dy for centering */
				fx = fy = 1.0 / fw;	I->dx = 0.0;	I->dy = 0.5 * (I->h - h * fy);
			}
			else {	/* Taller than wider given inset dims; adjust height to fit exactly and set dx for centering */
				fx = fy = 1.0 / fh;	I->dy = 0.0;	I->dx = 0.5 * (I->w - w * fx);
			}
		}
		else {	/* Cartesian is scaled independently to fit the inset */
			fx = 1.0 / fw;	fy = 1.0 / fh;	I->dx = I->dy = 0.0;
		}
		update_parameters = true;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rescaling map for inset by factors fx = %g fy = %g dx = %g dy = %g\n", fx, fy, I->dx, I->dy);
	}
	else if (P->active && no_scaling == 0)	{	/* Must rescale to fit inside subplot dimensions and set dx,dy for centering */
		adjust_panel_for_gaps (GMT, P);	/* Deal with any gaps requested via subplot -C: shrink w/h and adjust origin */
		fw = w / P->w;	fh = h / P->h;
		if (gmt_M_is_geographic (GMT, GMT_IN) || GMT->current.proj.projection == GMT_POLAR || GMT->current.proj.gave_map_width == 0) {	/* Giving -Jx will end up here with map projections */
			if (fw > fh) {	/* Wider than taller given panel dims; adjust width to fit exactly */
				fx = fy = 1.0 / fw;	P->dx = 0.0;	P->dy = 0.5 * (P->h - h * fy);
			}
			else {	/* Taller than wider given panel dims; adjust height to fit exactly and set dx for centering */
				fx = fy = 1.0 / fh;	P->dy = 0.0;	P->dx = 0.5 * (P->w - w * fx);
			}
		}
		else {	/* Cartesian is scaled independently to fit the subplot fully */
			fx = 1.0 / fw;	fy = 1.0 / fh;	P->dx = P->dy = 0.0;
		}
		update_parameters = true;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rescaling map for subplot by factors fx = %g fy = %g dx = %g dy = %g\n", fx, fy, P->dx, P->dy);
		if (gmt_M_is_rect_graticule (GMT) && P->parallel) {
			strcpy (GMT->current.setting.map_annot_ortho, "");	/* All annotations will be parallel to axes */
			GMT->current.setting.map_annot_oblique |= GMT_OBL_ANNOT_LAT_PARALLEL;	/* Plot latitude parallel to frame for geo maps */
		}
	}
	if (update_parameters) {	/* Scale the parameters due to inset or subplot adjustments */
		/* Update all projection parameters given the reduction factors fx, fy */
		GMT->current.proj.scale[GMT_X] *= fx;
		GMT->current.proj.scale[GMT_Y] *= fy;
		GMT->current.proj.w_r *= fx;	/* Only matter for geographic where fx = fy anyway */
		GMT->current.proj.rect[XHI] = (xmax - xmin) * GMT->current.proj.scale[GMT_X];
		GMT->current.proj.rect[YHI] = (ymax - ymin) * GMT->current.proj.scale[GMT_Y];
		GMT->current.proj.origin[GMT_X] = -xmin * GMT->current.proj.scale[GMT_X];
		GMT->current.proj.origin[GMT_Y] = -ymin * GMT->current.proj.scale[GMT_Y];
	}
}

/*! . */
GMT_LOCAL void map_setinfo (struct GMT_CTRL *GMT, double xmin, double xmax, double ymin, double ymax, double scl) {
	/* Set [and rescale] parameters */
	double factor = 1.0, w, h;

	if (GMT->current.map.is_world && doubleAlmostEqualZero (xmax, xmin)) {	/* Safety valve for cases when w & e both project to the same side due to round-off */
		xmax = MAX (fabs (xmin), fabs (xmax));
		xmin =-xmax;
	}
	w = (xmax - xmin) * GMT->current.proj.scale[GMT_X];
	h = (ymax - ymin) * GMT->current.proj.scale[GMT_Y];

	if (GMT->current.proj.gave_map_width == 1)	/* Must rescale to given width */
		factor = scl / w;
	else if (GMT->current.proj.gave_map_width == 2)	/* Must rescale to given height */
		factor = scl / h;
	else if (GMT->current.proj.gave_map_width == 3)	/* Must rescale to max dimension */
		factor = scl / MAX (w, h);
	else if (GMT->current.proj.gave_map_width == 4)	/* Must rescale to min dimension */
		factor = scl / MIN (w, h);
	GMT->current.proj.scale[GMT_X] *= factor;
	GMT->current.proj.scale[GMT_Y] *= factor;
	GMT->current.proj.w_r *= factor;

	if (GMT->current.proj.g_debug > 1) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "xmin %7.3f xmax %7.3f ymin %7.4f ymax %7.3f scale %6.3f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000, scl);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gave_map_width %d w %9.4e h %9.4e factor %9.4e\n", GMT->current.proj.gave_map_width, w, h, factor);
	}

	map_setxy (GMT, xmin, xmax, ymin, ymax);
}

/*! . */
GMT_LOCAL double map_mean_radius (struct GMT_CTRL *GMT, double a, double f) {
	double r = 0, b = a * (1 - f);

	if (f == 0.0) return a;	/* Not that hard */

	switch (GMT->current.setting.proj_mean_radius) {
		case GMT_RADIUS_MEAN:
			r = a * (1.0 - f / 3.0);
			break;
		case GMT_RADIUS_AUTHALIC:
			r = sqrt (0.5 * a * a + 0.5 * b * b * atanh (GMT->current.proj.ECC) / GMT->current.proj.ECC);
			break;
		case GMT_RADIUS_VOLUMETRIC:
			r = pow (a*a*b, 1.0/3.0);
			break;
		case GMT_RADIUS_MERIDIONAL:
			r = pow (0.5 * (pow (a, 1.5) + pow (b, 1.5)), 2.0/3.0);
			break;
		case GMT_RADIUS_QUADRATIC:
			r = 0.5 * sqrt (3.0 * a * a + b * b);
			break;
		default:	/* Cannot get here! Safety valve */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "GMT mean radius not specified\n");
			GMT_exit (GMT, GMT_RUNTIME_ERROR);
			break;
	}

	return (r);
}

/*! . */
GMT_LOCAL void map_set_spherical (struct GMT_CTRL *GMT, bool notify) {
	/* Set up ellipsoid parameters using spherical approximation */

	GMT->current.setting.ref_ellipsoid[GMT_N_ELLIPSOIDS - 1].eq_radius =
		map_mean_radius (GMT, GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius, GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening);
	GMT->current.setting.proj_ellipsoid = GMT_N_ELLIPSOIDS - 1;	/* Custom ellipsoid */
	GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening = 0.0;
	if (notify) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Spherical approximation used\n");
	GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_NONE;	/* No lat swapping for spherical */

	gmtlib_init_ellipsoid (GMT);
}

/*! . */
GMT_LOCAL double map_left_conic (struct GMT_CTRL *GMT, double y) {
	double x_ws, y_ws, x_wn, y_wn, dy;

	gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &x_ws, &y_ws);
	gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI], &x_wn, &y_wn);
	dy = y_wn - y_ws;
	if (doubleAlmostEqualZero (y_wn, y_ws))
		return (0.0);
	return (x_ws + ((x_wn - x_ws) * (y - y_ws) / dy));
}

/*! . */
GMT_LOCAL double map_right_conic (struct GMT_CTRL *GMT, double y) {
	double x_es, y_es, x_en, y_en, dy;

	gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], &x_es, &y_es);
	gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &x_en, &y_en);
	dy = y_en - y_es;
	if (doubleAlmostEqualZero (y_en, y_es))
		return (GMT->current.map.width);
	return (x_es - ((x_es - x_en) * (y - y_es) / dy));
}

/*! . */
GMT_LOCAL double map_left_rect (struct GMT_CTRL *GMT, double y) {
	gmt_M_unused(GMT); gmt_M_unused(y);
	return (0.0);
}

/*! . */
GMT_LOCAL double map_right_rect (struct GMT_CTRL *GMT, double y) {
	gmt_M_unused(y);
	return (GMT->current.map.width);
}

/*! . */
GMT_LOCAL double map_left_circle (struct GMT_CTRL *GMT, double y) {
	y -= GMT->current.proj.origin[GMT_Y];
	return (GMT->current.map.half_width - d_sqrt (GMT->current.proj.r * GMT->current.proj.r - y * y));
}

/*! . */
GMT_LOCAL double map_right_circle (struct GMT_CTRL *GMT, double y) {
	/* y -= GMT->current.proj.r; */
	y -= GMT->current.proj.origin[GMT_Y];
	return (GMT->current.map.half_width + d_sqrt (GMT->current.proj.r * GMT->current.proj.r - y * y));
}

/*! . */
GMT_LOCAL double map_left_ellipse (struct GMT_CTRL *GMT, double y) {
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - GMT->current.proj.origin[GMT_Y]) / GMT->current.proj.w_r;	/* Fraction, relative to Equator */
	return (GMT->current.map.half_width - 2.0 * GMT->current.proj.w_r * d_sqrt (1.0 - y * y));
}

/*! . */
GMT_LOCAL double map_right_ellipse (struct GMT_CTRL *GMT, double y) {
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - GMT->current.proj.origin[GMT_Y]) / GMT->current.proj.w_r;	/* Fraction, relative to Equator */
	return (GMT->current.map.half_width + 2.0 * GMT->current.proj.w_r * d_sqrt (1.0 - y * y));
}

/*! . */
GMT_LOCAL double map_az_backaz_cartesian (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	/* Calculate azimuths or backazimuths.  Cartesian case.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy;

	if (baz) {	/* exchange point one and two */
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	dx = lonS - lonE;
	dy = latS - latE;
	az = (dx == 0.0 && dy == 0.0) ? GMT->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

/*! . */
GMT_LOCAL double map_az_backaz_cartesian_proj (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	/* Calculate azimuths or backazimuths.  Cartesian case.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy, xE, yE, xS, yS;

	if (baz) {	/* exchange point one and two */
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	gmt_geo_to_xy (GMT, lonE, latE, &xE, &yE);
	gmt_geo_to_xy (GMT, lonS, latS, &xS, &yS);
	dx = xS - xE;
	dy = yS - yE;
	az = (dx == 0.0 && dy == 0.0) ? GMT->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

/*! . */
GMT_LOCAL double map_az_backaz_flatearth (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	/* Calculate azimuths or backazimuths.  Flat earth code.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy, dlon;

	if (baz) {	/* exchange point one and two */
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	gmt_M_set_delta_lon (lonE, lonS, dlon);
	dx = dlon * cosd (0.5 * (latS + latE));
	dy = latS - latE;
	az = (dx == 0.0 && dy == 0.0) ? GMT->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

/*! . */
GMT_LOCAL double map_az_backaz_sphere (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	/* Calculate azimuths or backazimuths.  Spherical code.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, sin_yS, cos_yS, sin_yE, cos_yE, sin_dlon, cos_dlon;
	gmt_M_unused(GMT);

	if (baz) {	/* exchange point one and two */
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	sincosd (latS, &sin_yS, &cos_yS);
	sincosd (latE, &sin_yE, &cos_yE);
	sincosd (lonS - lonE, &sin_dlon, &cos_dlon);
	az = atan2d (cos_yS * sin_dlon, cos_yE * sin_yS - sin_yE * cos_yS * cos_dlon);
	if (az < 0.0) az += 360.0;
	return (az);
}

#define VINCENTY_EPS		5e-14
#define VINCENTY_MAX_ITER	50
/*! . */
GMT_LOCAL double map_az_backaz_vincenty (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool back_az) {
	/* Translation of NGS FORTRAN code for determination of true distance
	** and respective forward and back azimuths between two points on the
	** ellipsoid.  Good for any pair of points that are not antipodal.
	**
	**      INPUT
	**	latS, lonS -- latitude and longitude of first point in radians.
	**	latE, lonE -- latitude and longitude of second point in radians.
	**
	**	OUTPUT
	**  	faz -- azimuth from first point to second in radians clockwise from North.
	**	baz -- azimuth from second point back to first point.
	**	bool back_az controls which is returned
	** Modified by P.W. from: http://article.gmane.org/gmane.comp.gis.proj-4.devel/3478
	*/
	int n_iter = 0;
	double az, c, d, e, r, f, d_lon, dx, x, y, sa, cx, cy, cz, sx, sy, c2a, cu1, cu2, su1, tu1, tu2, ts, baz, faz;

	f = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening;
	r = 1.0 - f;
	tu1 = r * tand (latS);
	tu2 = r * tand (latE);
	cu1 = 1.0 / sqrt (tu1 * tu1 + 1.0);
	su1 = cu1 * tu1;
	cu2 = 1.0 / sqrt (tu2 * tu2 + 1.0);
	ts  = cu1 * cu2;
	baz = ts * tu2;
	faz = baz * tu1;
	gmt_M_set_delta_lon (lonS, lonE, d_lon);
	if (gmt_M_is_zero (d_lon) && doubleAlmostEqualZero (latS, latE)) return GMT->session.d_NaN;
	x = dx = D2R * d_lon;
	do {
		n_iter++;
		sincos (x, &sx, &cx);
		tu1 = cu2 * sx;
		tu2 = baz - su1 * cu2 * cx;
		sy = sqrt (tu1 * tu1 + tu2 * tu2);
		cy = ts * cx + faz;
		y = atan2 (sy, cy);
		sa = ts * sx / sy;
		c2a = -sa * sa + 1.0;
		cz = faz + faz;
		if (c2a > 0.0) cz = -cz / c2a + cy;
		e = cz * cz * 2.0 - 1.0;
		c = ((c2a * -3.0 + 4.0) * f + 4.0) * c2a * f / 16.0;
		d = x;
		x = ((e * cy * c + cz) * sy * c + y) * sa;
		x = (1.0 - c) * x * f + dx;
	} while (fabs (d - x) > VINCENTY_EPS && n_iter <= VINCENTY_MAX_ITER);
	if (n_iter > VINCENTY_MAX_ITER) {
		GMT->current.proj.n_geodesic_approx++;	/* Count inaccurate results */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Near- or actual antipodal points encountered. Precision may be reduced slightly.\n");
	}
	GMT->current.proj.n_geodesic_calls++;
	/* To give the same sense of results as all other codes, we must basically swap baz and faz; here done in the ? test */
	az = (back_az) ? atan2 (tu1, tu2) : atan2 (cu1 * sx, baz * cx - su1 * cu2) + M_PI;
	return (R2D * az);
}

/*! . */
GMT_LOCAL double map_az_backaz_rudoe (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
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
	 * correction given by: 1-ECC2=1-2*f + f*f = GMT->current.proj.one_m_ECC2
	 */

	thg = atan (GMT->current.proj.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;
	g = -c * e;
	h = c * d;

	/* Calculating some trig constants. */

	thg = atan (GMT->current.proj.one_m_ECC2 * tand (latS));
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

/*
 *	TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION (GMT_LINEAR)
 */

/*! . */
GMT_LOCAL void map_linearxy (struct GMT_CTRL *GMT, double x, double y, double *x_i, double *y_i) {
	/* Transform both x and y linearly */
	(*GMT->current.proj.fwd_x) (GMT, x, x_i);
	(*GMT->current.proj.fwd_y) (GMT, y, y_i);
}

/*! . */
GMT_LOCAL void map_ilinearxy (struct GMT_CTRL *GMT, double *x, double *y, double x_i, double y_i) {
	/* Inversely transform both x and y linearly */
	(*GMT->current.proj.inv_x) (GMT, x, x_i);
	(*GMT->current.proj.inv_y) (GMT, y, y_i);
}

/*! . */
GMT_LOCAL bool map_init_linear (struct GMT_CTRL *GMT) {
	bool positive;
	double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0;

	GMT->current.map.left_edge  = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.proj.fwd = &map_linearxy;
	GMT->current.proj.inv = &map_ilinearxy;
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* x is longitude */
		GMT->current.proj.central_meridian = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
		GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	}
	else
		GMT->current.map.lon_wrap = false;
	if (gmt_M_y_is_lon (GMT, GMT_IN)) {	/* y is longitude */
		GMT->current.proj.central_meridian = 0.5 * (GMT->common.R.wesn[YLO] + GMT->common.R.wesn[YHI]);
	}
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.pars[0];
	GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];
	GMT->current.proj.xyz_pos[GMT_X] = (GMT->current.proj.scale[GMT_X] >= 0.0);	/* False if user wants x to increase left */
	GMT->current.proj.xyz_pos[GMT_Y] = (GMT->current.proj.scale[GMT_Y] >= 0.0);	/* False if user wants y to increase down */
	switch ( (GMT->current.proj.xyz_projection[GMT_X]%3)) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			if (gmt_M_type (GMT, GMT_IN, GMT_X) == GMT_IS_ABSTIME && GMT->current.proj.xyz_projection[GMT_X] != GMT_TIME)
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Option -JX|x: Your x-column contains absolute time but -JX|x...T was not specified!\n");
			GMT->current.proj.fwd_x = ((gmt_M_x_is_lon (GMT, GMT_IN)) ? &gmt_translind  : &gmt_translin);
			GMT->current.proj.inv_x = ((gmt_M_x_is_lon (GMT, GMT_IN)) ? &gmt_itranslind : &gmt_itranslin);
			if (GMT->current.proj.xyz_pos[GMT_X]) {
				(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XLO], &xmin);
				(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XHI], &xmax);
			}
			else {
				(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XHI], &xmin);
				(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XLO], &xmax);
			}
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (GMT->common.R.wesn[XLO] <= 0.0 || GMT->common.R.wesn[XHI] <= 0.0) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -JX|x:  Limits must be positive for log10 option\n");
				GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
			}
			xmin = (GMT->current.proj.xyz_pos[GMT_X]) ? d_log10 (GMT, GMT->common.R.wesn[XLO]) : d_log10 (GMT, GMT->common.R.wesn[XHI]);
			xmax = (GMT->current.proj.xyz_pos[GMT_X]) ? d_log10 (GMT, GMT->common.R.wesn[XHI]) : d_log10 (GMT, GMT->common.R.wesn[XLO]);
			GMT->current.proj.fwd_x = &gmt_translog10;
			GMT->current.proj.inv_x = &gmt_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			GMT->current.proj.xyz_pow[GMT_X] = GMT->current.proj.pars[2];
			GMT->current.proj.xyz_ipow[GMT_X] = 1.0 / GMT->current.proj.pars[2];
			positive = !((GMT->current.proj.xyz_pos[GMT_X] + (GMT->current.proj.xyz_pow[GMT_X] > 0.0)) % 2);
			xmin = (positive) ? pow (GMT->common.R.wesn[XLO], GMT->current.proj.xyz_pow[GMT_X]) : pow (GMT->common.R.wesn[XHI], GMT->current.proj.xyz_pow[GMT_X]);
			xmax = (positive) ? pow (GMT->common.R.wesn[XHI], GMT->current.proj.xyz_pow[GMT_X]) : pow (GMT->common.R.wesn[XLO], GMT->current.proj.xyz_pow[GMT_X]);
			GMT->current.proj.fwd_x = &gmt_transpowx;
			GMT->current.proj.inv_x = &gmt_itranspowx;
			break;
	}
	switch (GMT->current.proj.xyz_projection[GMT_Y]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			if (gmt_M_type (GMT, GMT_IN, GMT_Y) == GMT_IS_ABSTIME && GMT->current.proj.xyz_projection[GMT_Y] != GMT_TIME)
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Option -JX|x:  Your y-column contains absolute time but -JX|x...T was not specified!\n");
			GMT->current.proj.fwd_y = ((gmt_M_y_is_lon (GMT, GMT_IN)) ? &gmt_translind  : &gmt_translin);
			GMT->current.proj.inv_y = ((gmt_M_y_is_lon (GMT, GMT_IN)) ? &gmt_itranslind : &gmt_itranslin);
			if (GMT->current.proj.xyz_pos[GMT_Y]) {
				(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YLO], &ymin);
				(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YHI], &ymax);
			}
			else {
				(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YHI], &ymin);
				(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YLO], &ymax);
			}
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (GMT->common.R.wesn[YLO] <= 0.0 || GMT->common.R.wesn[YHI] <= 0.0) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -JX|x:  Limits must be positive for log10 option\n");
				GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
			}
			ymin = (GMT->current.proj.xyz_pos[GMT_Y]) ? d_log10 (GMT, GMT->common.R.wesn[YLO]) : d_log10 (GMT, GMT->common.R.wesn[YHI]);
			ymax = (GMT->current.proj.xyz_pos[GMT_Y]) ? d_log10 (GMT, GMT->common.R.wesn[YHI]) : d_log10 (GMT, GMT->common.R.wesn[YLO]);
			GMT->current.proj.fwd_y = &gmt_translog10;
			GMT->current.proj.inv_y = &gmt_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			GMT->current.proj.xyz_pow[GMT_Y] = GMT->current.proj.pars[3];
			GMT->current.proj.xyz_ipow[GMT_Y] = 1.0 / GMT->current.proj.pars[3];
			positive = !((GMT->current.proj.xyz_pos[GMT_Y] + (GMT->current.proj.xyz_pow[GMT_Y] > 0.0)) % 2);
			ymin = (positive) ? pow (GMT->common.R.wesn[YLO], GMT->current.proj.xyz_pow[GMT_Y]) : pow (GMT->common.R.wesn[YHI], GMT->current.proj.xyz_pow[GMT_Y]);
			ymax = (positive) ? pow (GMT->common.R.wesn[YHI], GMT->current.proj.xyz_pow[GMT_Y]) : pow (GMT->common.R.wesn[YLO], GMT->current.proj.xyz_pow[GMT_Y]);
			GMT->current.proj.fwd_y = &gmt_transpowy;
			GMT->current.proj.inv_y = &gmt_itranspowy;
	}

	/* Was given axes length instead of scale? */

	if (GMT->current.proj.compute_scale[GMT_X]) GMT->current.proj.scale[GMT_X] /= fabs (xmin - xmax);
	if (GMT->current.proj.compute_scale[GMT_Y]) GMT->current.proj.scale[GMT_Y] /= fabs (ymin - ymax);

	/* If either is zero, adjust width or height to the other */

	if (GMT->current.proj.scale[GMT_X] == 0) {	/* Must redo x-scaling by using y-scale */
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.autoscl[GMT_X] * GMT->current.proj.scale[GMT_Y];
		if (GMT->current.proj.autoscl[GMT_X] == -1) GMT->current.proj.xyz_pos[GMT_X] = !GMT->current.proj.xyz_pos[GMT_Y];
		switch ( (GMT->current.proj.xyz_projection[GMT_X]%3)) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
			case GMT_LINEAR:	/* Regular scaling */
				if (GMT->current.proj.xyz_pos[GMT_X]) {
					(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XLO], &xmin);
					(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XHI], &xmax);
				}
				else {
					(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XHI], &xmin);
					(*GMT->current.proj.fwd_x) (GMT, GMT->common.R.wesn[XLO], &xmax);
				}
				break;
			case GMT_LOG10:	/* Log10 transformation */
				xmin = (GMT->current.proj.xyz_pos[GMT_X]) ? d_log10 (GMT, GMT->common.R.wesn[XLO]) : d_log10 (GMT, GMT->common.R.wesn[XHI]);
				xmax = (GMT->current.proj.xyz_pos[GMT_X]) ? d_log10 (GMT, GMT->common.R.wesn[XHI]) : d_log10 (GMT, GMT->common.R.wesn[XLO]);
				break;
			case GMT_POW:	/* x^y transformation */
				positive = !((GMT->current.proj.xyz_pos[GMT_X] + (GMT->current.proj.xyz_pow[GMT_X] > 0.0)) % 2);
				xmin = (positive) ? pow (GMT->common.R.wesn[XLO], GMT->current.proj.xyz_pow[GMT_X]) : pow (GMT->common.R.wesn[XHI], GMT->current.proj.xyz_pow[GMT_X]);
				xmax = (positive) ? pow (GMT->common.R.wesn[XHI], GMT->current.proj.xyz_pow[GMT_X]) : pow (GMT->common.R.wesn[XLO], GMT->current.proj.xyz_pow[GMT_X]);
				break;
		}
		GMT->current.proj.pars[0] = GMT->current.proj.scale[GMT_X] * fabs (xmin - xmax);
	}
	else if (GMT->current.proj.scale[GMT_Y] == 0) {	/* Must redo y-scaling by using x-scale */
		GMT->current.proj.scale[GMT_Y] = GMT->current.proj.autoscl[GMT_Y] * GMT->current.proj.scale[GMT_X];
		if (GMT->current.proj.autoscl[GMT_Y] == -1) GMT->current.proj.xyz_pos[GMT_Y] = !GMT->current.proj.xyz_pos[GMT_X];
		switch (GMT->current.proj.xyz_projection[GMT_Y]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
			case GMT_LINEAR:	/* Regular scaling */
				if (GMT->current.proj.xyz_pos[GMT_Y]) {
					(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YLO], &ymin);
					(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YHI], &ymax);
				}
				else {
					(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YHI], &ymin);
					(*GMT->current.proj.fwd_y) (GMT, GMT->common.R.wesn[YLO], &ymax);
				}
				break;
			case GMT_LOG10:	/* Log10 transformation */
				ymin = (GMT->current.proj.xyz_pos[GMT_Y]) ? d_log10 (GMT, GMT->common.R.wesn[YLO]) : d_log10 (GMT, GMT->common.R.wesn[YHI]);
				ymax = (GMT->current.proj.xyz_pos[GMT_Y]) ? d_log10 (GMT, GMT->common.R.wesn[YHI]) : d_log10 (GMT, GMT->common.R.wesn[YLO]);
				break;
			case GMT_POW:	/* x^y transformation */
				positive = !((GMT->current.proj.xyz_pos[GMT_Y] + (GMT->current.proj.xyz_pow[GMT_Y] > 0.0)) % 2);
				ymin = (positive) ? pow (GMT->common.R.wesn[YLO], GMT->current.proj.xyz_pow[GMT_Y]) : pow (GMT->common.R.wesn[YHI], GMT->current.proj.xyz_pow[GMT_Y]);
				ymax = (positive) ? pow (GMT->common.R.wesn[YHI], GMT->current.proj.xyz_pow[GMT_Y]) : pow (GMT->common.R.wesn[YLO], GMT->current.proj.xyz_pow[GMT_Y]);
		}
		GMT->current.proj.pars[1] = GMT->current.proj.scale[GMT_Y] * fabs (ymin - ymax);
	}

	/* This override ensures that when using -J[x|X]...d degrees work as meters */

	GMT->current.proj.M_PR_DEG = 1.0;
	GMT->current.proj.KM_PR_DEG = GMT->current.proj.M_PR_DEG / METERS_IN_A_KM;

	map_setxy (GMT, xmin, xmax, ymin, ymax);
	if (gmt_M_type (GMT, GMT_IN, GMT_X) == GMT_IS_LON) {	/* Using linear projection with longitudes */
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
	}
	else {
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_cartesian_overlap;
		GMT->current.map.clip = &map_rect_clip;
	}
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.map.frame.check_side = true;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);
}

/*!
 *	TRANSFORMATION ROUTINES FOR POLAR (theta,r) PROJECTION (GMT_POLAR)
 */
GMT_LOCAL bool map_init_polar (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	gmt_vpolar (GMT, GMT->current.proj.pars[1]);
	if (GMT->current.proj.flip) {	/* Check restrictions */
		if (GMT->common.R.wesn[YLO] < 0.0 || GMT->common.R.wesn[YHI] > GMT->current.proj.flip_radius) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "-JP...+f requires s >= 0 and n <= %g!\n", GMT->current.proj.flip_radius);
			GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
		}
		if (doubleAlmostEqual (GMT->common.R.wesn[YHI], GMT->current.proj.flip_radius) && gmt_M_is_zero (GMT->current.proj.radial_offset))
			GMT->current.proj.edge[2] = false;
	}
	else {
		if (gmt_M_is_zero (GMT->common.R.wesn[YLO]) && gmt_M_is_zero (GMT->current.proj.radial_offset))
			GMT->current.proj.edge[0] = false;
	}
	if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
	GMT->current.map.left_edge = &map_left_circle;
	GMT->current.map.right_edge = &map_right_circle;
	GMT->current.proj.fwd = &gmt_polar;
	GMT->current.proj.inv = &gmt_ipolar;
	GMT->current.map.is_world = false;	/* There is no wrapping around here */
	map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[0];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[0]);
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	/* GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI]; */
	GMT->current.proj.r = GMT->current.proj.scale[GMT_Y] * GMT->common.R.wesn[YHI];
	GMT->current.map.outside = &map_polar_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.frame.horizontal = 1;
	if (!GMT->current.proj.got_elevations) GMT->current.plot.r_theta_annot = true;	/* Special labeling case (see gmtlib_get_annot_label) */
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.meridian_straight = 1;

	return (false);
}

/*!
 *	TRANSFORMATION ROUTINES FOR THE MERCATOR PROJECTION (GMT_MERCATOR)
 */
GMT_LOCAL bool map_init_merc (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, D = 1.0;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) {	/* Set fudge factor */
		gmtlib_scale_eqrad (GMT);
		D = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius / GMT->current.proj.lat_swap_vals.rm;
	}
	if (GMT->common.R.wesn[YLO] <= -90.0 || GMT->common.R.wesn[YHI] >= 90.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -R:  Cannot include south/north poles with Mercator projection!\n");
		GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
	}
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	map_cyl_validate_clon (GMT, 0);	/* Make sure the central longitude is valid */
	gmt_vmerc (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	GMT->current.proj.j_x *= D;
	GMT->current.proj.j_yc *= D;
	GMT->current.proj.j_ix /= D;
	GMT->current.proj.fwd = &gmt_merc_sph;
	GMT->current.proj.inv = &gmt_imerc_sph;
	(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
	(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= (D * GMT->current.proj.M_PR_DEG);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.map.outside = &map_wesn_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.frame.check_side = true;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);	/* No need to search for wesn */
}

/*!
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUAL-AREA PROJECTIONS (GMT_CYL_EQ)
 */
GMT_LOCAL bool map_init_cyleq (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.Dx = GMT->current.proj.Dy = 0.0;
	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) {
		double D, k0, qp, slat, e, e2;
		gmtlib_scale_eqrad (GMT);
		slat = GMT->current.proj.pars[1];
		GMT->current.proj.pars[1] = gmt_M_latg_to_lata (GMT, GMT->current.proj.pars[1]);
		e = GMT->current.proj.ECC;
		e2 = GMT->current.proj.ECC2;
		qp = 1.0 - 0.5 * (1.0 - e2) * log ((1.0 - e) / (1.0 + e)) / e;
		k0 = cosd (slat) / d_sqrt (1.0 - e2 * sind (GMT->current.proj.pars[1]) * sind (GMT->current.proj.pars[1]));
		D = k0 / cosd (GMT->current.proj.pars[1]);
		GMT->current.proj.Dx = D;
		GMT->current.proj.Dy = 0.5 * qp / D;
	}
	GMT->current.proj.iDx = 1.0 / GMT->current.proj.Dx;
	GMT->current.proj.iDy = 1.0 / GMT->current.proj.Dy;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	map_cyl_validate_clon (GMT, 1);	/* Make sure the central longitude is valid */
	gmt_vcyleq (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	gmt_cyleq (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
	gmt_cyleq (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.proj.fwd = &gmt_cyleq;
	GMT->current.proj.inv = &gmt_icyleq;
	GMT->current.map.outside = &map_wesn_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.frame.check_side = true;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);	/* No need to search for wesn */
}

/*!
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUIDISTANT PROJECTION (GMT_CYL_EQDIST)
 */

GMT_LOCAL bool map_init_cyleqdist (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* Force spherical for now */

	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	map_cyl_validate_clon (GMT, 1);	/* Make sure the central longitude is valid */
	gmt_vcyleqdist (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	gmt_cyleqdist (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
	gmt_cyleqdist (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.proj.fwd = &gmt_cyleqdist;
	GMT->current.proj.inv = &gmt_icyleqdist;
	GMT->current.map.outside = &map_wesn_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.frame.check_side = true;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);	/* No need to search for wesn */
}
//#define CHRISTMAS

/*!
 *	TRANSFORMATION ROUTINES FOR MILLER CYLINDRICAL PROJECTION (GMT_MILLER)
 */
GMT_LOCAL bool map_init_miller (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* Force spherical for now */

	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	map_cyl_validate_clon (GMT, 1);	/* Make sure the central longitude is valid */
	gmt_vmiller (GMT, GMT->current.proj.pars[0]);
	gmt_miller (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
#ifdef CHRISTMAS
	if (GMT->common.R.wesn[YLO] > 0.0) {
		gmt_miller (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_miller (GMT, 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]), GMT->common.R.wesn[YHI], &xmax, &ymax);
	}
	else {
		gmt_miller (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmin, &ymin);
		gmt_miller (GMT, 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]), GMT->common.R.wesn[YLO], &xmax, &ymax);
	}
#else
	gmt_miller (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
	gmt_miller (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
#endif
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.proj.fwd = &gmt_miller;
	GMT->current.proj.inv = &gmt_imiller;
	GMT->current.map.outside = &map_wesn_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.frame.check_side = true;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);	/* No need to search for wesn */
}

/*!
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL STEREOGRAPHIC PROJECTIONS (GMT_CYL_STEREO)
 */
GMT_LOCAL bool map_init_cylstereo (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* Force spherical for now */

	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	map_cyl_validate_clon (GMT, 1);	/* Make sure the central longitude is valid */
	gmt_vcylstereo (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	gmt_cylstereo (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
	gmt_cylstereo (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT->current.proj.fwd = &gmt_cylstereo;
	GMT->current.proj.inv = &gmt_icylstereo;
	GMT->current.map.outside = &map_wesn_outside;
	GMT->current.map.crossing = &map_wesn_crossing;
	GMT->current.map.overlap = &map_wesn_overlap;
	GMT->current.map.clip = &map_wesn_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.frame.check_side = true;
	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 1;

	return (false);	/* No need to search for wesn */
}


/*!
 *	TRANSFORMATION ROUTINES FOR THE POLAR STEREOGRAPHIC PROJECTION (GMT_STEREO)
 */

GMT_LOCAL bool map_init_stereo (struct GMT_CTRL *GMT) {
	unsigned int i;
	double xmin, xmax, ymin, ymax, dummy, radius, latg, D = 1.0;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	latg = GMT->current.proj.pars[1];

	map_set_polar (GMT);

	if (GMT->current.setting.proj_scale_factor == -1.0) GMT->current.setting.proj_scale_factor = 0.9996;	/* Select default map scale for Stereographic */
	if (GMT->current.proj.polar && (lrint (GMT->current.proj.pars[5]) == 1)) GMT->current.setting.proj_scale_factor = 1.0;	/* Gave true scale at given parallel set below */
	/* Equatorial view has a problem with infinite loops.  Until I find a cure
	  we set projection center latitude to 0.001 so equatorial works for now */

	if (fabs (GMT->current.proj.pars[1]) < GMT_CONV4_LIMIT) GMT->current.proj.pars[1] = 0.001;

	gmt_vstereo (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);

	if (GMT->current.proj.GMT_convert_latitudes) {	/* Set fudge factors when conformal latitudes are used */
		double e1p, e1m, s, c;

		D = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius / GMT->current.proj.lat_swap_vals.rm;
		if (GMT->current.proj.polar) {
			e1p = 1.0 + GMT->current.proj.ECC;	e1m = 1.0 - GMT->current.proj.ECC;
			D /= d_sqrt (pow (e1p, e1p) * pow (e1m, e1m));
			if (lrint (GMT->current.proj.pars[5]) == 1) {	/* Gave true scale at given parallel */
				double k_p, m_c, t_c, es;

				sincosd (fabs (GMT->current.proj.pars[4]), &s, &c);
				es = GMT->current.proj.ECC * s;
				m_c = c / d_sqrt (1.0 - GMT->current.proj.ECC2 * s * s);
				t_c = d_sqrt (((1.0 - s) / (1.0 + s)) * pow ((1.0 + es) / (1.0 - es), GMT->current.proj.ECC));
				k_p = 0.5 * m_c * d_sqrt (pow (e1p, e1p) * pow (e1m, e1m)) / t_c;
				D *= k_p;
			}
		}
		else {
			sincosd (latg, &s, &c);	/* Need original geographic pole coordinates */
			D *= (c / (GMT->current.proj.cosp * d_sqrt (1.0 - GMT->current.proj.ECC2 * s * s)));
		}
	}
	GMT->current.proj.Dx = GMT->current.proj.Dy = D;

	GMT->current.proj.iDx = 1.0 / GMT->current.proj.Dx;
	GMT->current.proj.iDy = 1.0 / GMT->current.proj.Dy;

	if (GMT->current.proj.polar) {	/* Polar aspect */
		GMT->current.proj.fwd = &gmt_plrs_sph;
		GMT->current.proj.inv = &gmt_iplrs_sph;
		if (GMT->current.proj.units_pr_degree) {
			(*GMT->current.proj.fwd) (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[4], &dummy, &radius);
			GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
		}
		else
			GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];
		GMT->current.map.meridian_straight = 1;
	}
	else {
		GMT->current.proj.fwd = (gmt_M_is_zero (GMT->current.proj.pole)) ? &gmt_stereo2_sph : &gmt_stereo1_sph;
		GMT->current.proj.inv = &gmt_istereo_sph;
		if (GMT->current.proj.units_pr_degree) {
			gmt_vstereo (GMT, 0.0, 90.0, GMT->current.proj.pars[2]);
			(*GMT->current.proj.fwd) (GMT, 0.0, fabs (GMT->current.proj.pars[4]), &dummy, &radius);
			GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
		}
		else
			GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];

		gmt_vstereo (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);
	}


	if (GMT->common.R.oblique) {	/* Rectangular box given */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 30.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
	}
	else {
		if (GMT->current.proj.polar) {	/* Polar aspect */
			if (GMT->current.proj.north_pole) {
				if (GMT->common.R.wesn[YLO] <= -90.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "South boundary cannot be -90.0 for north polar stereographic projection\n");
					GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
				}
				if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
			}
			else {
				if (GMT->common.R.wesn[YHI] >= 90.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "North boundary cannot be +90.0 for south polar stereographic projection\n");
					GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
				}
				if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
			}
			if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
					|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
				GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
			GMT->current.map.outside = &map_polar_outside;
			GMT->current.map.crossing = &map_wesn_crossing;
			GMT->current.map.overlap = &map_wesn_overlap;
			GMT->current.map.clip = &map_wesn_clip;
			GMT->current.map.frame.horizontal = 1;
			GMT->current.map.n_lat_nodes = 2;
			map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			/* No annotations or tickmarks in global mode */
			for (i = 0; i < GMT_GRID_UPPER; i++)
				GMT->current.map.frame.axis[GMT_X].item[i].active = GMT->current.map.frame.axis[GMT_Y].item[i].active = false,
				GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval = 0.0;
			GMT->common.R.wesn[XLO] = 0.0;
			GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;
			GMT->common.R.wesn[YHI] = 90.0;
			xmax = ymax = GMT->current.proj.rho_max;
			xmin = ymin = -xmax;
			GMT->current.map.outside = &map_radial_outside;
			GMT->current.map.crossing = &map_radial_crossing;
			GMT->current.map.overlap = &map_radial_overlap;
			GMT->current.map.clip = &map_radial_clip;
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[3]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);

	return (GMT->common.R.oblique);
}

/*!
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT CONFORMAL CONIC PROJECTION (GMT_LAMBERT)
 */

GMT_LOCAL bool map_init_lambert (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.GMT_convert_latitudes = map_quickconic (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	gmt_vlamb (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2], GMT->current.proj.pars[3]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[4] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[4];
	if (gmt_M_is_spherical (GMT) || GMT->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT->current.proj.fwd = &gmt_lamb_sph;
		GMT->current.proj.inv = &gmt_ilamb_sph;
	}
	else {
		GMT->current.proj.fwd = &gmt_lamb;
		GMT->current.proj.inv = &gmt_ilamb;
	}

	if (GMT->common.R.oblique) {	/* Rectangular box given*/
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_conic;
		GMT->current.map.right_edge = &map_right_conic;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[4]);
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	GMT->current.map.n_lat_nodes = 2;
	GMT->current.map.frame.horizontal = 1;
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}

/*!
 *	TRANSFORMATION ROUTINES FOR THE OBLIQUE MERCATOR PROJECTION (GMT_OBLIQUE_MERC)
 */

GMT_LOCAL void map_pole_rotate_forward (struct GMT_CTRL *GMT, double lon, double lat, double *tlon, double *tlat) {
	/* Given the pole position in GMT->current.proj, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_lat, cos_lat, cos_lon, sin_lon, cc;

	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon - GMT->current.proj.o_pole_lon, &sin_lon, &cos_lon);
	cc = cos_lat * cos_lon;
	*tlat = d_asind (GMT->current.proj.o_sin_pole_lat * sin_lat + GMT->current.proj.o_cos_pole_lat * cc);
	*tlon = GMT->current.proj.o_beta + d_atan2d (cos_lat * sin_lon, GMT->current.proj.o_sin_pole_lat * cc - GMT->current.proj.o_cos_pole_lat * sin_lat);
}

#if 0
/* Not currently used in GMT */
GMT_LOCAL void map_pole_rotate_inverse (struct GMT_CTRL *GMT, double *lon, double *lat, double tlon, double tlat) {
	/* Given the pole position in GMT->current.proj, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_tlat, cos_tlat, cos_tlon, sin_tlon, cc;

	sincosd (tlat, &sin_tlat, &cos_tlat);
	sincosd (tlon - GMT->current.proj.o_beta, &sin_tlon, &cos_tlon);
	cc = cos_tlat * cos_tlon;
	*lat = d_asind (GMT->current.proj.o_sin_pole_lat * sin_tlat - GMT->current.proj.o_cos_pole_lat * cc);
	*lon = GMT->current.proj.o_pole_lon + d_atan2d (cos_tlat * sin_tlon, GMT->current.proj.o_sin_pole_lat * cc + GMT->current.proj.o_cos_pole_lat * sin_tlat);
}
#endif


/*! . */
GMT_LOCAL void map_get_origin (struct GMT_CTRL *GMT, double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2) {
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
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Correct projection origin = %g/%g\n", *lon2, *lat2);
	}
	else {
		*lon2 = lon1;
		*lat2 = lat1;
	}

	map_pole_rotate_forward (GMT, *lon2, *lat2, &beta, &dummy);

	GMT->current.proj.o_beta = -beta;
}

/*! . */
GMT_LOCAL void map_get_rotate_pole (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	double plon, plat, beta, dummy;
	double A[3], B[3], P[3];
	/* Given A = (lon1, lat1) and B = (lon2, lat2), get P = A x B */
	gmt_geo_to_cart (GMT, lat1, lon1, A, true);	/* Cartesian vector for origin */
	gmt_geo_to_cart (GMT, lat2, lon2, B, true);	/* Cartesian vector for 2nd point along oblique Equator */
	gmt_cross3v (GMT, A, B, P);			/* Get pole position of plane through A and B (and center of Earth O) */
	gmt_normalize3v (GMT, P);			/* Make sure P has unit length */
	gmt_cart_to_geo (GMT, &plat, &plon, P, true);	/* Recover lon,lat of pole */

	if (plat < 0.0 && GMT->hidden.func_level == 0 && !strncmp (GMT->init.module_name, "mapproject", 10U))	/* Only allowed in mapproject at top level */
		GMT->current.proj.o_spole = true;
	if (GMT->current.proj.N_hemi && plat < 0.0) {	/* Insist on a Northern hemisphere pole */
		plat = -plat;
		plon += 180.0;
		if (plon >= 360.0) plon -= 360.0;
	}
	GMT->current.proj.o_pole_lon = plon;
	GMT->current.proj.o_pole_lat = plat;
	sincosd (plat, &GMT->current.proj.o_sin_pole_lat, &GMT->current.proj.o_cos_pole_lat);
	map_pole_rotate_forward (GMT, lon1, lat1, &beta, &dummy);
	GMT->current.proj.o_beta = -beta;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Oblique Mercator pole is %.12g %.12g, with beta = %.12g\n", plon, plat, -beta);
}

/*! . */
GMT_LOCAL bool map_init_oblique (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, dummy;
	double o_x, o_y, p_x, p_y, c, az, b_x, b_y, w, e, s, n;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	if (strncmp (GMT->init.module_name, "mapproject", 10U))
		GMT->current.proj.o_spole = false;	/* Only used in mapproject */

	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[4] /= GMT->current.proj.M_PR_DEG;	/* To get plot-units / m */

	o_x = GMT->current.proj.pars[0];	o_y = GMT->current.proj.pars[1];

	if (lrint (GMT->current.proj.pars[6]) == 1) {	/* Must get correct origin, then get second point */
		p_x = GMT->current.proj.pars[2];	p_y = GMT->current.proj.pars[3];

		GMT->current.proj.o_pole_lon = p_x;
		GMT->current.proj.o_pole_lat = p_y;
		GMT->current.proj.o_sin_pole_lat = sind (p_y);
		GMT->current.proj.o_cos_pole_lat = cosd (p_y);

		/* Find azimuth to pole, add 90, and compute second point 10 degrees away */

		map_get_origin (GMT, o_x, o_y, p_x, p_y, &o_x, &o_y);
		az = atand (cosd (p_y) * sind (p_x - o_x) / (cosd (o_y) * sind (p_y) - sind (o_y) * cosd (p_y) * cosd (p_x - o_x))) + 90.0;
		c = 10.0;	/* compute point 10 degrees from origin along azimuth */
		b_x = o_x + atand (sind (c) * sind (az) / (cosd (o_y) * cosd (c) - sind (o_y) * sind (c) * cosd (az)));
		b_y = d_asind (sind (o_y) * cosd (c) + cosd (o_y) * sind (c) * cosd (az));
		GMT->current.proj.pars[0] = o_x;	GMT->current.proj.pars[1] = o_y;
		GMT->current.proj.pars[2] = b_x;	GMT->current.proj.pars[3] = b_y;
	}
	else {	/* Just find pole */
		b_x = GMT->current.proj.pars[2];	b_y = GMT->current.proj.pars[3];
		map_get_rotate_pole (GMT, o_x, o_y, b_x, b_y);
	}

	/* Here we have pole and origin */

	gmtlib_set_oblique_pole_and_origin (GMT, GMT->current.proj.o_pole_lon, GMT->current.proj.o_pole_lat, o_x, o_y);
	gmt_vmerc (GMT, 0.0, 0.0);
	/* Internally, the origin is at the intersection between the meridian through the pole and the selected
	 * origin and the oblique equator, but we need to shift that up to the selected oblique latitude through
	 * the user's origin. */
	gmt_oblmrc (GMT, GMT->current.proj.lon0, GMT->current.proj.lat0, &dummy, &GMT->current.proj.o_shift);
	if (GMT->current.proj.o_spole) GMT->current.proj.o_shift = -GMT->current.proj.o_shift;

	if (GMT->common.R.oblique) {	/* wesn is lower left and upper right corners in normal lon/lats */
		gmt_oblmrc (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_oblmrc (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
	}
	else {	/* Gave oblique degrees */
		/* Convert oblique wesn in degrees to meters using regular Mercator */
		if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) {
			GMT->common.R.wesn[XLO] = -180.0;
			GMT->common.R.wesn[XHI] = +180.0;
		}
		gmt_merc_sph (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_merc_sph (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->common.R.oblique = true;	/* Since wesn was oblique, not geographical wesn */
	}

	gmt_imerc_sph (GMT, &w, &s, xmin, ymin);	/* Get oblique wesn boundaries */
	gmt_imerc_sph (GMT, &e, &n, xmax, ymax);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[4];
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[4]);
	GMT->current.proj.fwd = &gmt_oblmrc;
	GMT->current.proj.inv = &gmt_ioblmrc;
	GMT->current.map.outside = &map_rect_outside;
	GMT->current.map.crossing = &map_rect_crossing;
	GMT->current.map.overlap = &map_rect_overlap;
	GMT->current.map.clip = &map_rect_clip;
	GMT->current.map.left_edge = &map_left_rect;
	GMT->current.map.right_edge = &map_right_rect;

	GMT->current.map.is_world = gmt_M_360_range (w, e);
#if 0
	if (GMT->current.map.is_world == false) {	/* Check if one of the poles are inside the area */
		if (!gmt_map_outside (GMT, 0.0, -90.0))	/* South pole is inside map */
			GMT->current.map.is_world = true;
		else if (!gmt_map_outside (GMT, 0.0, +90.0))	/* North pole is inside map */
			GMT->current.map.is_world = true;
	}
#endif
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
	return (true);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE TRANSVERSE MERCATOR PROJECTION (GMT_TM)
 */

/* For global TM maps */

/*! . */
GMT_LOCAL unsigned int map_wrap_around_check_tm (struct GMT_CTRL *GMT, double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, unsigned int *sides) {
	double dx, dy, width, jump;

	jump = this_y - last_y;
	width = 0.5 * GMT->current.map.height;

	if (fabs (jump) - width <= GMT_CONV4_LIMIT || fabs (jump) <= GMT_CONV4_LIMIT) return (0);
	dx = this_x - last_x;

	if (jump < -width) {	/* Crossed top boundary */
		dy = GMT->current.map.height + jump;
		xx[0] = xx[1] = last_x + (GMT->current.map.height - last_y) * dx / dy;
		if (xx[0] < 0.0 || xx[0] > GMT->current.proj.rect[XHI]) return (0);
		yy[0] = GMT->current.map.height;	yy[1] = 0.0;
		sides[0] = 2;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed bottom boundary */
		dy = GMT->current.map.height - jump;
		xx[0] = xx[1] = last_x + last_y * dx / dy;
		if (xx[0] < 0.0 || xx[0] > GMT->current.proj.rect[XHI]) return (0);
		yy[0] = 0.0;	yy[1] = GMT->current.map.height;
		sides[0] = 0;
		angle[0] = d_atan2d (dy, -dx);
	}
	angle[1] = angle[0] + 180.0;
	sides[1] = 2 - sides[0];

	return (2);
}

/*! . */
GMT_LOCAL bool map_this_point_wraps_tm (struct GMT_CTRL *GMT, double y0, double y1) {
	/* Returns true if the 2 y-points implies a jump at this x-level of the TM map */

	double dy;

	return ((dy = fabs (y1 - y0)) > GMT->current.map.half_height);
}

/*! . */
GMT_LOCAL bool map_will_it_wrap_tm (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t *start) {
	/* Determines if a polygon will wrap at edges for TM global projection */
	uint64_t i;
	bool wrap;
	gmt_M_unused(x);

	if (!GMT->current.map.is_world) return (false);

	for (i = 1, wrap = false; !wrap && i < n; i++) {
		wrap = map_this_point_wraps_tm (GMT, y[i-1], y[i]);
	}
	*start = i - 1;
	return (wrap);
}

/*! . */
GMT_LOCAL void map_get_crossings_tm (struct GMT_CTRL *GMT, double *xc, double *yc, double x0, double y0, double x1, double y1) {
	/* Finds crossings for wrap-arounds for global TM maps */
	double xa, xb, ya, yb, dy, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (ya > yb) {	/* Make A the minimum y point */
		gmt_M_double_swap (xa, xb);
		gmt_M_double_swap (ya, yb);
	}

	yb -= GMT->current.map.height;

	dy = ya - yb;
	c = (doubleAlmostEqualZero (ya, yb)) ? 0.0 : (xa - xb) / dy;
	xc[0] = xc[1] = xb - yb * c;
	if (y0 > y1) {	/* First cut top */
		yc[0] = GMT->current.map.height;
		yc[1] = 0.0;
	}
	else {
		yc[0] = 0.0;
		yc[1] = GMT->current.map.height;
	}
}

/*! . */
GMT_LOCAL int map_jump_tm (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* true if y-distance between points exceeds 1/2 map height at this x value */
	/* Only used for TM world maps */

	double dy;
	gmt_M_unused(x0); gmt_M_unused(x1);

	dy = y1 - y0;
	if (dy > GMT->current.map.half_height) return (-1);	/* Cross bottom/south boundary */
	if (dy < (-GMT->current.map.half_height)) return (1);	/* Cross top/north boundary */
	return (0);
}

/*! . */
GMT_LOCAL bool map_init_tm (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	/* Wrap and truncations are in y, not x for TM */

	GMT->current.map.wrap_around_check = &map_wrap_around_check_tm;
	GMT->current.map.jump = &map_jump_tm;
	GMT->current.map.will_it_wrap = &map_will_it_wrap_tm;
#if 0
	GMT->current.map.this_point_wraps = &map_this_point_wraps_tm;
#endif
	GMT->current.map.get_crossings = &map_get_crossings_tm;

	if (GMT->current.setting.proj_scale_factor == -1.0) GMT->current.setting.proj_scale_factor = 1.0;	/* Select default map scale for TM */
	GMT->current.proj.GMT_convert_latitudes = map_quicktm (GMT, GMT->current.proj.pars[0], 10.0);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	gmt_vtm (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	if (gmt_M_is_spherical (GMT) || GMT->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT->current.proj.fwd = &gmt_tm_sph;
		GMT->current.proj.inv = &gmt_itm_sph;
	}
	else {
		GMT->current.proj.fwd = &gmt_tm;
		GMT->current.proj.inv = &gmt_itm;
	}

	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.map.is_world) {	/* Gave oblique degrees */
		double w, e, dummy;
		w = GMT->current.proj.central_meridian + GMT->common.R.wesn[YLO];
		e = GMT->current.proj.central_meridian + GMT->common.R.wesn[YHI];
		GMT->common.R.wesn[YLO] = -90;
		GMT->common.R.wesn[YHI] = 90;
		GMT->common.R.wesn[XHI] = e;
		GMT->common.R.wesn[XLO] = w;
		gmt_vtm (GMT, GMT->current.proj.pars[0], 0.0);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], 0.0, &xmin, &dummy);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], 0.0, &xmax, &dummy);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &dummy, &ymin);
		ymax = ymin + (TWO_PI * GMT->current.proj.EQ_RAD * GMT->current.setting.proj_scale_factor);
		gmt_vtm (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
		GMT->current.map.is_world_tm = true;
		GMT->common.R.oblique = true;	/* Since wesn was oblique, not geographical wesn */
		GMT->common.R.wesn[XHI] = GMT->current.proj.central_meridian + 180.0;
		GMT->common.R.wesn[XLO] = GMT->current.proj.central_meridian - 180.0;
	}
	else if (!GMT->common.R.oblique) {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.is_world_tm = doubleAlmostEqualZero (GMT->common.R.wesn[YHI], GMT->common.R.wesn[YLO]);
		GMT->current.map.is_world = false;
	}
	else { /* Find min values */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
		GMT->current.map.is_world_tm = false;
		GMT->current.map.is_world = (fabs (GMT->common.R.wesn[YLO] - GMT->common.R.wesn[YHI]) < GMT_CONV4_LIMIT);
	}

	GMT->current.map.frame.horizontal = 1;
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);

	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE UNIVERSAL TRANSVERSE MERCATOR PROJECTION (GMT_UTM)
 */

/*! . */
GMT_LOCAL void map_set_utmzone (struct GMT_CTRL *GMT) {
	/* When no UTM zone is given we determine the best one from midpoint of -R.
	 * We pass back via GMT->current.proj.pars[0] the zone number.
	 * Note that despite the funky non-standard zones around Norway
	 * the central meridian still follows the simple 6-degree stepping.
	 */
	int kx, ky;
	double clon, clat;
	char zone[4] = {""};

	clon = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
	if (clon > 180.0) clon -= 360.0;
	else if (clon < -180.0) clon += 360.0;
	kx = irint (floor ((clon + 180.0) / 6.0)) + 1;	/* Best UTM zone */
	GMT->current.proj.lon0 = -180.0 + kx * 6.0 - 3.0;	/* Best centered longitude */
	if (GMT->common.R.wesn[YLO] >= 0.0)		/* By doing >= we are implicitly saying that when no info we default to northern hemisphere */
		GMT->current.proj.utm_hemisphere = +1;
	else
		GMT->current.proj.utm_hemisphere = -1;
	GMT->current.proj.utm_zoney = 0;
	clat = 0.5 * (GMT->common.R.wesn[YLO] + GMT->common.R.wesn[YHI]);
	if (clat < -80.0) {	/* A or B */
		zone[0] = (GMT->current.proj.lon0 < 0.0) ? 'A' : 'B';
	}
	else if (clat > 84.0) {	/* Y or Z */
		zone[0] = (GMT->current.proj.lon0 < 0.0) ? 'Y' : 'Z';
	}
	else {	/* In the general latitude range of 1-60 zones */
		ky = irint (floor ((clat + 80.0) / 8.0));
		if (ky == 20) ky = 19;	/* Since band X is 12 degrees tall there is no 20 */
		if (ky >= 11) ky += 4;		/* zone = (P-X) */
		else if (ky >= 6) ky += 3;	/* zone = (J-N) */
		else ky += 2;			/* zone = (C-H) */
		GMT->current.proj.utm_zoney = 'A' + (char)ky;
		if (GMT->current.proj.utm_zoney == 'X') {	/* Deal with funky zone X */
			if (clon >= 0.0 && clon < 9.0)
				kx = 31;
			else if (clon >= 9.0  && clon < 21.0)
				kx = 33;
			else if (clon >= 21.0 && clon < 33.0)
				kx = 35;
			else if (clon >= 33.0 && clon < 42.0)
				kx = 37;
		}
		else if (GMT->current.proj.utm_zoney == 'V') {	/* Deal with funky zone V */
			if (clon >= 3.0 && clon < 12.0)
				kx = 32;
		}
		snprintf (zone, 4, "%d%c", kx, GMT->current.proj.utm_zoney);
	}
	GMT->current.proj.pars[0] = (double)kx;
	GMT->current.proj.lat0 = 0.0;
	GMT_Report (GMT->parent, GMT_MSG_WARNING, "No UTM zone given; zone %s selected\n", zone);
}

/*! . */
GMT_LOCAL bool map_init_utm (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, lon0;

	if (GMT->current.setting.proj_scale_factor == -1.0) GMT->current.setting.proj_scale_factor = 0.9996;	/* Select default map scale for UTM */
	if (GMT->current.proj.pars[0] < 0.0) map_set_utmzone (GMT);	/* Determine UTM zone from -R */
	lon0 = 180.0 + 6.0 * GMT->current.proj.pars[0] - 3.0;	/* Central meridian for this UTM zone */
	if (lon0 >= 360.0) lon0 -= 360.0;
	GMT->current.proj.GMT_convert_latitudes = map_quicktm (GMT, lon0, 10.0);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	gmt_vtm (GMT, lon0, 0.0);	/* Central meridian for this zone */
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];
	switch (GMT->current.proj.utm_hemisphere) {	/* Set hemisphere */
		case -1:
			GMT->current.proj.north_pole = false;
			break;
		case +1:
			GMT->current.proj.north_pole = true;
			break;
		default:
			GMT->current.proj.north_pole = (GMT->common.R.wesn[YLO] >= 0.0);
			break;
	}
	if (gmt_M_is_spherical (GMT) || GMT->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT->current.proj.fwd = &gmt_utm_sph;
		GMT->current.proj.inv = &gmt_iutm_sph;
	}
	else {
		GMT->current.proj.fwd = &gmt_utm;
		GMT->current.proj.inv = &gmt_iutm;
	}

	if (fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) > 360.0) {	/* -R in UTM meters */
		(*GMT->current.proj.inv) (GMT, &GMT->common.R.wesn[XLO], &GMT->common.R.wesn[YLO], GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO]);
		(*GMT->current.proj.inv) (GMT, &GMT->common.R.wesn[XHI], &GMT->common.R.wesn[YHI], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI]);
		GMT->common.R.oblique = true;
	}
	if (GMT->common.R.oblique) {
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
	}

	GMT->current.map.frame.horizontal = 1;

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);

	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

	return (GMT->common.R.oblique);
}

#if 0
GMT_LOCAL double map_UTMzone_to_clon (struct GMT_CTRL *GMT, unsigned int zone_x, char zone_y) {
	/* Return the central longitude of this UTM zone */
	double clon = 180.0 + 6.0 * zone_x - 3.0;	/* This is valid for most zones */

	if (zone_y == 0) return (clon);	/* Latitude zone is not specified so we are done */
	if ((zone_y < 'A' || zone_y > 'Z') || zone_y < 'I' || zone_y < 'O') return (GMT->session.d_NaN);	/* Bad latitude zone so return NaN*/
	if (zone_y <= 'B') return (-90.0 + 180.0 * (zone_y - 'A'));	/* Either -90 or +90 */
	if (zone_y == 'V' && zone_x == 31) return (1.5);	/* Center of 31V */
	if (zone_y == 'V' && zone_x == 32) return (7.5);	/* Center of 32V */
	if (zone_y == 'X') {
		if (zone_x == 31) return (4.5);		/* Center of 31X */
		if (zone_x == 33) return (15.0);	/* Center of 33X */
		if (zone_x == 35) return (27.0);	/* Center of 35X */
		if (zone_x == 37) return (37.5);	/* Center of 37X */
		if (zone_x == 32 || zone_x == 34 || zone_x == 36) return (GMT->session.d_NaN);	/* Bad latitude zone so return NaN*/
		return (clon);
	}
	/* Only Y or Z left */
	return (-90.0 + 180.0 * (zone_y - 'Y'));	/* Either -90 or +90 */
}
#endif

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION (GMT_LAMB_AZ_EQ)
 */

/*! . */
GMT_LOCAL bool map_init_lambeq (struct GMT_CTRL *GMT) {
	unsigned int i;
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT->current.proj.Dx = GMT->current.proj.Dy = 1.0;

	map_set_polar (GMT);
	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	gmt_vlambeq (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);

	if (GMT->current.proj.GMT_convert_latitudes) {
		double D, s, c;
		sincosd (GMT->current.proj.pars[1], &s, &c);
		D = (GMT->current.proj.polar) ? 1.0 : (GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius / GMT->current.proj.lat_swap_vals.ra) * c / (GMT->current.proj.cosp * d_sqrt (1.0 - GMT->current.proj.ECC2 * s * s));
		GMT->current.proj.Dx = D;
		GMT->current.proj.Dy = 1.0 / D;
	}
	GMT->current.proj.iDx = 1.0 / GMT->current.proj.Dx;
	GMT->current.proj.iDy = 1.0 / GMT->current.proj.Dy;

	GMT->current.proj.fwd = &gmt_lambeq;
	GMT->current.proj.inv = &gmt_ilambeq;
	if (GMT->current.proj.units_pr_degree) {
		gmt_vlambeq (GMT, 0.0, 90.0, GMT->current.proj.pars[2]);
		gmt_lambeq (GMT, 0.0, fabs (GMT->current.proj.pars[4]), &dummy, &radius);
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
		gmt_vlambeq (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);
	}
	else
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];

	if (GMT->common.R.oblique) {	/* Rectangular box given */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 30.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
	}
	else {
		if (GMT->current.proj.polar) {	/* Polar aspect */
			if (GMT->current.proj.north_pole) {
				if (GMT->common.R.wesn[YLO] <= -90.0){
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "South boundary cannot be -90.0 for north polar Lambert azimuthal projection\n");
					GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
				}
				if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
			}
			else {
				if (GMT->common.R.wesn[YHI] >= 90.0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "North boundary cannot be +90.0 for south polar Lambert azimuthal projection\n");
					GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
				}
				if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
			}
			if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
					|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
				GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
			GMT->current.map.outside = &map_polar_outside;
			GMT->current.map.crossing = &map_wesn_crossing;
			GMT->current.map.overlap = &map_wesn_overlap;
			GMT->current.map.clip = &map_wesn_clip;
			GMT->current.map.frame.horizontal = 1;
			GMT->current.map.n_lat_nodes = 2;
			map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			/* No annotations or tickmarks in global mode */
			for (i = 0; i < GMT_GRID_UPPER; i++)
				GMT->current.map.frame.axis[GMT_X].item[i].active = GMT->current.map.frame.axis[GMT_Y].item[i].active = false,
				GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval = 0.0;
			GMT->common.R.wesn[XLO] = 0.0;
			GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;
			GMT->common.R.wesn[YHI] = 90.0;
			xmax = ymax = GMT->current.proj.rho_max;
			xmin = ymin = -xmax;
			GMT->current.map.outside = &map_radial_outside;
			GMT->current.map.crossing = &map_radial_crossing;
			GMT->current.map.overlap = &map_radial_overlap;
			GMT->current.map.clip = &map_radial_clip;
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[3]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	if (GMT->current.proj.polar) GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ORTHOGRAPHIC PROJECTION (GMT_ORTHO)
 */

/*! . */
GMT_LOCAL bool map_init_ortho (struct GMT_CTRL *GMT) {
	unsigned int i;
	double xmin, xmax, ymin, ymax, dummy, radius;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	map_set_polar (GMT);

	if (GMT->current.proj.units_pr_degree) {
		gmt_vortho (GMT, 0.0, 90.0, GMT->current.proj.pars[2]);
		gmt_ortho (GMT, 0.0, fabs (GMT->current.proj.pars[4]), &dummy, &radius);
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
	}
	else
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];

	gmt_vortho (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);
	GMT->current.proj.fwd = &gmt_ortho;
	GMT->current.proj.inv = &gmt_iortho;

	if (GMT->common.R.oblique) {	/* Rectangular box given */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 30.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
	}
	else {
		if (GMT->current.proj.polar) {	/* Polar aspect */
			if (GMT->current.proj.north_pole) {
				if (GMT->common.R.wesn[YLO] < 0.0) {
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "South boundary cannot be < 0 for north polar orthographic projection (reset to 0)\n");
					GMT->common.R.wesn[YLO] = 0.0;
				}
				if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
			}
			else {
				if (GMT->common.R.wesn[YHI] > 0.0) {
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "North boundary cannot be > 0 for south polar orthographic projection (reset to 0)\n");
					GMT->common.R.wesn[YHI] = 0.0;
				}
				if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
			}
			if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
					|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
				GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
			GMT->current.map.outside = &map_polar_outside;
			GMT->current.map.crossing = &map_wesn_crossing;
			GMT->current.map.overlap = &map_wesn_overlap;
			GMT->current.map.clip = &map_wesn_clip;
			GMT->current.map.frame.horizontal = 1;
			GMT->current.map.n_lat_nodes = 2;
			map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			/* No annotations or tickmarks in global mode */
			for (i = 0; i < GMT_GRID_UPPER; i++)
				GMT->current.map.frame.axis[GMT_X].item[i].active = GMT->current.map.frame.axis[GMT_Y].item[i].active = false,
				GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval = 0.0;
			GMT->common.R.wesn[XLO] = 0.0;
			GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;
			GMT->common.R.wesn[YHI] = 90.0;
			xmax = ymax = GMT->current.proj.rho_max * GMT->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			GMT->current.map.outside = &map_radial_outside;
			GMT->current.map.crossing = &map_radial_crossing;
			GMT->current.map.overlap = &map_radial_overlap;
			GMT->current.map.clip = &map_radial_clip;
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[3]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	if (GMT->current.proj.polar) GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}

/*! . */
GMT_LOCAL double gmt_left_genper (struct GMT_CTRL *GMT, double y)
{	/* Windowed genper may need to consider the inner of circle and rectangle */
	return (MAX (0.0, map_left_circle (GMT, y)));
}

/*! . */
GMT_LOCAL double gmt_right_genper (struct GMT_CTRL *GMT, double y)
{
	return (MIN (GMT->current.map.width, map_right_circle (GMT, y)));
}

/*! . */
GMT_LOCAL bool map_init_genper (struct GMT_CTRL *GMT) {
	bool search;
	unsigned int i;
	double xmin, xmax, ymin, ymax, dummy, radius = 0.0;
	double alt, azimuth, tilt, width, height;
	double twist, scale, units;

	units = GMT->current.proj.pars[2];
	scale = GMT->current.proj.pars[3];
	alt = GMT->current.proj.pars[4];
	azimuth = GMT->current.proj.pars[5];
	tilt = GMT->current.proj.pars[6];
	twist = GMT->current.proj.pars[7];
	width = GMT->current.proj.pars[8];
	height = GMT->current.proj.pars[9];

	if (GMT->current.proj.g_sphere) map_set_spherical (GMT, true); /* PW: Force spherical for now */

	map_set_polar (GMT);

	if (GMT->current.proj.units_pr_degree) {
		gmt_vgenper (GMT, 0.0, 90.0, alt, azimuth, tilt, twist, width, height);
		gmt_genper (GMT, 0.0, fabs (GMT->current.proj.pars[3]), &dummy, &radius);
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[2] / radius);
	}
	else
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];

	if (GMT->current.proj.g_debug > 1) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper: units_pr_degree %d\n", GMT->current.proj.units_pr_degree);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper: radius %f\n", radius);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper: scale %f units %f\n", scale, units);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper: x scale %f y scale %f\n", GMT->current.proj.scale[GMT_X], GMT->current.proj.scale[GMT_Y]);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "genper: gave_map_width %d \n",GMT->current.proj.gave_map_width);
	}

	gmt_vgenper (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], alt, azimuth, tilt, twist, width, height);
	GMT->current.proj.fwd = &gmt_genper;
	GMT->current.proj.inv = &gmt_igenper;

	GMT->common.R.wesn[XLO] = 0.0;
	GMT->common.R.wesn[XHI] = 360.0;
	GMT->common.R.wesn[YLO] = -90.0;
	GMT->common.R.wesn[YHI] = 90.0;

	xmin = GMT->current.proj.g_xmin;
	xmax = GMT->current.proj.g_xmax;
	ymin = GMT->current.proj.g_ymin;
	ymax = GMT->current.proj.g_ymax;

	if (GMT->current.proj.g_width != 0.0) {
		GMT->common.R.oblique = false;
		GMT->current.proj.windowed = true;
		if (GMT->current.proj.g_debug > 0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "using windowed region\n");
		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
#if 0
		GMT->current.map.crossing = &map_genper_crossing;
		GMT->current.map.overlap = &map_genperw_overlap;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
#endif
		GMT->current.map.clip = &map_rect_clip_old;
		GMT->current.map.left_edge = &gmt_left_genper;
		GMT->current.map.right_edge = &gmt_right_genper;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 30.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
		GMT->current.map.jump = &map_jump_not;
		search = true;
	}
	else {
		if (GMT->current.proj.g_debug > 0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "using global view\n");
		/* No annotations or tickmarks in global mode */
		for (i = 0; i < GMT_GRID_UPPER; i++)
			GMT->current.map.frame.axis[GMT_X].item[i].active = GMT->current.map.frame.axis[GMT_Y].item[i].active = false,
			GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval = 0.0;
		GMT->current.map.overlap = &map_genperg_overlap;
		GMT->current.map.crossing = &map_radial_crossing;
		GMT->current.map.clip = &map_radial_clip;
		GMT->current.map.outside = &map_radial_outside;
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;

		if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

		search = false;
  	}

	if (GMT->current.proj.polar) {
		if (GMT->current.proj.north_pole) {
			if (GMT->common.R.wesn[YLO] < (90.0 - GMT->current.proj.f_horizon)) GMT->common.R.wesn[YLO] = 90.0 - GMT->current.proj.f_horizon;
			if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
		} else {
			if (GMT->common.R.wesn[YHI] > -(90.0 - GMT->current.proj.f_horizon)) GMT->common.R.wesn[YHI] = -(90.0 - GMT->current.proj.f_horizon);
			if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
		}
		if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
				|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
			GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
	}

	if (GMT->current.proj.g_debug > 0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "xmin %f xmax %f ymin %f ymax %f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000);

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);

	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];

	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);

	if (GMT->current.proj.g_debug > 0) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "x scale %e y scale %e\n", GMT->current.proj.scale[GMT_X], GMT->current.proj.scale[GMT_Y]);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "x center %f y center %f\n", GMT->current.proj.c_x0, GMT->current.proj.c_y0);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "x max %f y max %f\n", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI]);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "x0 %f y0 %f\n\n", GMT->current.proj.origin[GMT_X], GMT->current.proj.origin[GMT_Y]);
		fflush(NULL);
	}

	return (search);

}

/*
 *	TRANSFORMATION ROUTINES FOR THE GNOMONIC PROJECTION (GMT_GNOMONIC)
 */

/*! . */
GMT_LOCAL bool map_init_gnomonic (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */	map_set_polar (GMT);

	if (GMT->current.proj.units_pr_degree) {
		gmt_vgnomonic (GMT, 0.0, 90.0, 60.0);
		gmt_gnomonic (GMT, 0.0, fabs (GMT->current.proj.pars[4]), &dummy, &radius);
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
	}
	else
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];

	gmt_vgnomonic (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);
	GMT->current.proj.fwd = &gmt_gnomonic;
	GMT->current.proj.inv = &gmt_ignomonic;

	if (GMT->common.R.oblique) {	/* Rectangular box given */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 30.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
	}
	else {
		if (GMT->current.proj.polar) {	/* Polar aspect */
			if (GMT->current.proj.north_pole) {
				if (GMT->common.R.wesn[YLO] < (90.0 - GMT->current.proj.f_horizon)) GMT->common.R.wesn[YLO] = 90.0 - GMT->current.proj.f_horizon;
				if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
			}
			else {
				if (GMT->common.R.wesn[YHI] > -(90.0 - GMT->current.proj.f_horizon)) GMT->common.R.wesn[YHI] = -(90.0 - GMT->current.proj.f_horizon);
				if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
			}
			if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
					|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
				GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
			GMT->current.map.outside = &map_polar_outside;
			GMT->current.map.crossing = &map_wesn_crossing;
			GMT->current.map.overlap = &map_wesn_overlap;
			GMT->current.map.clip = &map_wesn_clip;
			GMT->current.map.frame.horizontal = 1;
			GMT->current.map.n_lat_nodes = 2;
			map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			GMT->common.R.wesn[XLO] = 0.0;
			GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;
			GMT->common.R.wesn[YHI] = 90.0;
			xmax = ymax = GMT->current.proj.rho_max * GMT->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			GMT->current.map.outside = &map_radial_outside;
			GMT->current.map.crossing = &map_radial_crossing;
			GMT->current.map.overlap = &map_radial_overlap;
			GMT->current.map.clip = &map_radial_clip;
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[3]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE AZIMUTHAL EQUIDISTANT PROJECTION (GMT_AZ_EQDIST)
 */

/*! . */
GMT_LOCAL bool map_init_azeqdist (struct GMT_CTRL *GMT) {
	unsigned int i;
	double xmin, xmax, ymin, ymax, dummy, radius;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	map_set_polar (GMT);

	if (GMT->current.proj.units_pr_degree) {
		gmt_vazeqdist (GMT, 0.0, 90.0, GMT->current.proj.pars[2]);
		gmt_azeqdist (GMT, 0.0, fabs (GMT->current.proj.pars[4]), &dummy, &radius);
		if (gmt_M_is_zero (radius)) radius = GMT->current.proj.rho_max * GMT->current.proj.EQ_RAD;
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = fabs (GMT->current.proj.pars[3] / radius);
	}
	else
		GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[3];

	gmt_vazeqdist (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2]);
	GMT->current.proj.fwd = &gmt_azeqdist;
	GMT->current.proj.inv = &gmt_iazeqdist;

	if (GMT->common.R.oblique) {	/* Rectangular box given */
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);

		GMT->current.map.outside = &map_rect_outside2;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = !(GMT->current.setting.map_annot_oblique & GMT_OBL_ANNOT_ANYWHERE);
		GMT->current.map.frame.horizontal = (fabs (GMT->current.proj.pars[1]) < 60.0 && fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 30.0) ? 1 : 0;
	}
	else {
		if (GMT->current.proj.polar && (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) < 180.0) {	/* Polar aspect */
			if (!GMT->current.proj.north_pole && GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
			if (GMT->current.proj.north_pole && GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
			if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])
					|| doubleAlmostEqualZero (GMT->common.R.wesn[XHI], GMT->common.R.wesn[XLO]))
				GMT->current.proj.edge[1] = GMT->current.proj.edge[3] = false;
			GMT->current.map.outside = &map_polar_outside;
			GMT->current.map.crossing = &map_wesn_crossing;
			GMT->current.map.overlap = &map_wesn_overlap;
			GMT->current.map.clip = &map_wesn_clip;
			GMT->current.map.frame.horizontal = 1;
			GMT->current.map.n_lat_nodes = 2;
			map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		else {	/* Global view only, force wesn = 0/360/-90/90  */
			/* No annotations or tickmarks in global mode */
			for (i = 0; i < GMT_GRID_UPPER; i++)
				GMT->current.map.frame.axis[GMT_X].item[i].active = GMT->current.map.frame.axis[GMT_Y].item[i].active = false,
				GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval = 0.0;
			GMT->common.R.wesn[XLO] = 0.0;
			GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;
			GMT->common.R.wesn[YHI] = 90.0;
			xmax = ymax = GMT->current.proj.rho_max * GMT->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			GMT->current.map.outside = &map_radial_outside;
			GMT->current.map.crossing = &map_radial_crossing;
			GMT->current.map.overlap = &map_radial_overlap;
			GMT->current.map.clip = &map_radial_clip;
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[3]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	if (GMT->current.proj.polar) GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MOLLWEIDE EQUAL AREA PROJECTION (GMT_MOLLWEIDE)
 */

/*! . */
GMT_LOCAL bool map_init_mollweide (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = M_PI * GMT->current.proj.pars[1] / sqrt (8.0);
	gmt_vmollweide (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
	if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;

	if (GMT->common.R.oblique) {
		gmt_mollweide (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_mollweide (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		gmt_mollweide (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_mollweide (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_mollweide (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_mollweide (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_ellipse;
		GMT->current.map.right_edge = &map_right_ellipse;
		GMT->current.map.frame.horizontal = 2;
		GMT->current.proj.polar = true;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_mollweide;
	GMT->current.proj.inv = &gmt_imollweide;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	GMT->current.map.parallel_straight = 1;

	return (GMT->common.R.oblique);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE HAMMER-AITOFF EQUAL AREA PROJECTION (GMT_HAMMER)
 */

/*! . */
GMT_LOCAL bool map_init_hammer (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = 0.5 * M_PI * GMT->current.proj.pars[1] / M_SQRT2;
	gmt_vhammer (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
	if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;

	if (GMT->common.R.oblique) {
		gmt_hammer (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_hammer (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double x, y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		x = (fabs (GMT->common.R.wesn[XLO] - GMT->current.proj.central_meridian) > fabs (GMT->common.R.wesn[XHI] - GMT->current.proj.central_meridian)) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
		gmt_hammer (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_hammer (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_hammer (GMT, x, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_hammer (GMT, x, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_ellipse;
		GMT->current.map.right_edge = &map_right_ellipse;
		GMT->current.map.frame.horizontal = 2;
		GMT->current.proj.polar = true;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_hammer;
	GMT->current.proj.inv = &gmt_ihammer;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE VAN DER GRINTEN PROJECTION (GMT_VANGRINTEN)
 */

/*! . */
GMT_LOCAL bool map_init_grinten (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];
	gmt_vgrinten (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
	if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;

	if (GMT->common.R.oblique) {
		gmt_grinten (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_grinten (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double x, y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		x = (fabs (GMT->common.R.wesn[XLO] - GMT->current.proj.central_meridian) > fabs (GMT->common.R.wesn[XHI] - GMT->current.proj.central_meridian)) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
		gmt_grinten (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_grinten (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_grinten (GMT, x, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_grinten (GMT, x, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_circle;
		GMT->current.map.right_edge = &map_right_circle;
		GMT->current.map.frame.horizontal = 2;
		GMT->current.proj.polar = true;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.r = 0.5 * GMT->current.proj.rect[XHI];
	GMT->current.proj.fwd = &gmt_grinten;
	GMT->current.proj.inv = &gmt_igrinten;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE WINKEL-TRIPEL MODIFIED AZIMUTHAL PROJECTION (GMT_WINKEL)
 */

/*! . */
GMT_LOCAL bool map_init_winkel (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	gmt_vwinkel (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = 2.0 * GMT->current.proj.pars[1] / (1.0 + GMT->current.proj.r_cosphi1);

	if (GMT->common.R.oblique) {
		gmt_winkel (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_winkel (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double x, y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		x = (fabs (GMT->common.R.wesn[XLO] - GMT->current.proj.central_meridian) > fabs (GMT->common.R.wesn[XHI] - GMT->current.proj.central_meridian)) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
		gmt_winkel (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_winkel (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_winkel (GMT, x, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_winkel (GMT, x, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_winkel;
		GMT->current.map.right_edge = &gmt_right_winkel;
		GMT->current.map.frame.horizontal = 2;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_winkel;
	GMT->current.proj.inv = &gmt_iwinkel;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT IV PROJECTION (GMT_ECKERT4)
 */

/*! . */
GMT_LOCAL bool map_init_eckert4 (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	gmt_veckert4 (GMT, GMT->current.proj.pars[0]);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];

	if (GMT->common.R.oblique) {
		gmt_eckert4 (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_eckert4 (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		gmt_eckert4 (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_eckert4 (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_eckert4 (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_eckert4 (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_eckert4;
		GMT->current.map.right_edge = &gmt_right_eckert4;
		GMT->current.map.frame.horizontal = 2;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_eckert4;
	GMT->current.proj.inv = &gmt_ieckert4;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	GMT->current.map.parallel_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT VI PROJECTION (GMT_ECKERT6)
 */

/*! . */
GMT_LOCAL bool map_init_eckert6 (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	gmt_veckert6 (GMT, GMT->current.proj.pars[0]);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = 0.5 * GMT->current.proj.pars[1] * sqrt (2.0 + M_PI);

	if (GMT->common.R.oblique) {
		gmt_eckert6 (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_eckert6 (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		gmt_eckert6 (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_eckert6 (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_eckert6 (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_eckert6 (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_eckert6;
		GMT->current.map.right_edge = &gmt_right_eckert6;
		GMT->current.map.frame.horizontal = 2;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_eckert6;
	GMT->current.proj.inv = &gmt_ieckert6;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	GMT->current.map.parallel_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ROBINSON PSEUDOCYLINDRICAL PROJECTION (GMT_ROBINSON)
 */

/*! . */
GMT_LOCAL bool map_init_robinson (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	gmt_vrobinson (GMT, GMT->current.proj.pars[0]);
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1] / 0.8487;

	if (GMT->common.R.oblique) {
		gmt_robinson (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_robinson (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		gmt_robinson (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_robinson (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		gmt_robinson (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_robinson (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_robinson;
		GMT->current.map.right_edge = &gmt_right_robinson;
		GMT->current.map.frame.horizontal = 2;
	}
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.proj.fwd = &gmt_robinson;
	GMT->current.proj.inv = &gmt_irobinson;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
	GMT->current.map.parallel_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE SINUSOIDAL EQUAL AREA PROJECTION (GMT_SINUSOIDAL)
 */

/*! . */
GMT_LOCAL bool map_init_sinusoidal (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if (GMT->current.proj.pars[0] < 0.0) GMT->current.proj.pars[0] += 360.0;
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
	if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
	gmt_vsinusoidal (GMT, GMT->current.proj.pars[0]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[1] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[1];
	GMT->current.proj.fwd = &gmt_sinusoidal;
	GMT->current.proj.inv = &gmt_isinusoidal;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (GMT->common.R.oblique) {
		gmt_sinusoidal (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		gmt_sinusoidal (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double dummy, y;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		gmt_sinusoidal (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		gmt_sinusoidal (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		gmt_sinusoidal (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		gmt_sinusoidal (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_sinusoidal;
		GMT->current.map.right_edge = &gmt_right_sinusoidal;
		GMT->current.map.frame.horizontal = 2;
		GMT->current.proj.polar = true;
	}

	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[1]);
	GMT->current.map.parallel_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE CASSINI PROJECTION (GMT_CASSINI)
 */

/*! . */
GMT_LOCAL bool map_init_cassini (struct GMT_CTRL *GMT) {
	bool too_big;
	double xmin, xmax, ymin, ymax;

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	if ((GMT->current.proj.pars[0] - GMT->common.R.wesn[XLO]) > 90.0 || (GMT->common.R.wesn[XHI] - GMT->current.proj.pars[0]) > 90.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Max longitude extension away from central meridian is limited to +/- 90 degrees\n");
		GMT_exit (GMT, GMT_PROJECTION_ERROR); return false;
	}
	too_big = map_quicktm (GMT, GMT->current.proj.pars[0], 4.0);
	if (too_big) map_set_spherical (GMT, true);	/* Cannot use ellipsoidal series for this area */
	gmt_vcassini (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (gmt_M_is_spherical (GMT)) {
		GMT->current.proj.fwd = &gmt_cassini_sph;
		GMT->current.proj.inv = &gmt_icassini_sph;
	}
	else {
		GMT->current.proj.fwd = &gmt_cassini;
		GMT->current.proj.inv = &gmt_icassini;
	}
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (GMT->common.R.oblique) {
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_conic;
		GMT->current.map.right_edge = &map_right_conic;
	}

	GMT->current.map.frame.horizontal = 1;
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ALBERS PROJECTION (GMT_ALBERS)
 */

/*! . */
GMT_LOCAL bool map_init_albers (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	GMT->current.proj.GMT_convert_latitudes = map_quickconic (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	if (gmt_M_is_spherical (GMT) || GMT->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo authalic latitudes */
		gmt_valbers_sph (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2], GMT->current.proj.pars[3]);
		GMT->current.proj.fwd = &gmt_albers_sph;
		GMT->current.proj.inv = &gmt_ialbers_sph;
	}
	else {
		gmt_valbers (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2], GMT->current.proj.pars[3]);
		GMT->current.proj.fwd = &gmt_albers;
		GMT->current.proj.inv = &gmt_ialbers;
	}
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[4] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[4];

	if (GMT->common.R.oblique) {
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_conic;
		GMT->current.map.right_edge = &map_right_conic;
	}
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.n_lat_nodes = 2;
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[4]);

	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian + 90., GMT->current.proj.pole, &x1, &y1);
	dy = y1 - GMT->current.proj.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - GMT->current.proj.c_x0);
	dy /= (1.0 - cos (az));
	GMT->current.proj.c_y0 += dy;
	GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE EQUIDISTANT CONIC PROJECTION (GMT_ECONIC)
 */


/*! . */
GMT_LOCAL bool map_init_econic (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	GMT->current.proj.GMT_convert_latitudes = !gmt_M_is_spherical (GMT);
	if (GMT->current.proj.GMT_convert_latitudes) gmtlib_scale_eqrad (GMT);
	gmt_veconic (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1], GMT->current.proj.pars[2], GMT->current.proj.pars[3]);
	GMT->current.proj.fwd = &gmt_econic;
	GMT->current.proj.inv = &gmt_ieconic;
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[4] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[4];

	if (GMT->common.R.oblique) {
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
	}
	else {
		map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &map_left_conic;
		GMT->current.map.right_edge = &map_right_conic;
	}
	GMT->current.map.frame.horizontal = 1;
	GMT->current.map.n_lat_nodes = 2;
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[4]);

	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &GMT->current.proj.c_x0, &GMT->current.proj.c_y0);
	gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian + 90., GMT->current.proj.pole, &x1, &y1);
	dy = y1 - GMT->current.proj.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - GMT->current.proj.c_x0);
	dy /= (1.0 - cos (az));
	GMT->current.proj.c_y0 += dy;
	GMT->current.map.meridian_straight = 1;

	return (GMT->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE POLYCONIC PROJECTION (GMT_POLYCONIC)
 */

/*! . */
GMT_LOCAL bool map_init_polyconic (struct GMT_CTRL *GMT) {
	double xmin, xmax, ymin, ymax;

	map_set_spherical (GMT, true);	/* PW: Force spherical for now */

	if (central_meridian_not_set (GMT))
		set_default_central_meridian (GMT);
	GMT->current.map.is_world = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	if (GMT->common.R.wesn[YLO] <= -90.0) GMT->current.proj.edge[0] = false;
	if (GMT->common.R.wesn[YHI] >= 90.0) GMT->current.proj.edge[2] = false;
	gmt_vpolyconic (GMT, GMT->current.proj.pars[0], GMT->current.proj.pars[1]);
	if (GMT->current.proj.units_pr_degree) GMT->current.proj.pars[2] /= GMT->current.proj.M_PR_DEG;
	GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.pars[2];
	GMT->current.proj.fwd = &gmt_polyconic;
	GMT->current.proj.inv = &gmt_ipolyconic;
	if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (GMT->common.R.oblique) {
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
		GMT->current.map.outside = &map_rect_outside;
		GMT->current.map.crossing = &map_rect_crossing;
		GMT->current.map.overlap = &map_rect_overlap;
		GMT->current.map.clip = &map_rect_clip;
		GMT->current.map.left_edge = &map_left_rect;
		GMT->current.map.right_edge = &map_right_rect;
		GMT->current.map.frame.check_side = true;
	}
	else {
		double y, dummy;
		y = (GMT->common.R.wesn[YLO] * GMT->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs (GMT->common.R.wesn[YLO]), fabs (GMT->common.R.wesn[YHI]));
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XLO], y, &xmin, &dummy);
		(*GMT->current.proj.fwd) (GMT, GMT->common.R.wesn[XHI], y, &xmax, &dummy);
		(*GMT->current.proj.fwd) (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YLO], &dummy, &ymin);
		(*GMT->current.proj.fwd) (GMT, GMT->current.proj.central_meridian, GMT->common.R.wesn[YHI], &dummy, &ymax);
		GMT->current.map.outside = &map_wesn_outside;
		GMT->current.map.crossing = &map_wesn_crossing;
		GMT->current.map.overlap = &map_wesn_overlap;
		GMT->current.map.clip = &map_wesn_clip;
		GMT->current.map.left_edge = &gmt_left_polyconic;
		GMT->current.map.right_edge = &gmt_right_polyconic;
		GMT->current.proj.polar = true;
	}

	GMT->current.map.frame.horizontal = 1;
	map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.pars[2]);

	return (GMT->common.R.oblique);
}

#ifdef HAVE_GDAL
/*!
 *	TRANSFORMATION ROUTINES FOR PROJ4 TRANSFORMATIONS (GMT_PROJ4_PROJS)
 */
 GMT_LOCAL bool map_init_proj4 (struct GMT_CTRL *GMT) {
	/*
	*  Here we use the trick of letting the previous GMT functions do the necessary initializations
	*  and at the end just replace the pointers to the FWD & INV transform functions to those of GDAL.
	*/
	bool search = false;
	double xmin, xmax, ymin, ymax;

	switch (GMT->current.proj.projection_GMT) {
		case GMT_LINEAR:        search = map_init_linear (GMT); break;      /* Linear transformations */
		case GMT_POLAR:         search = map_init_polar (GMT); break;       /* Both lon/lat are actually theta, radius */
		case GMT_MERCATOR:      search = map_init_merc (GMT); break;        /* Standard Mercator projection */
		case GMT_STEREO:        search = map_init_stereo (GMT); break;      /* Stereographic projection */
		case GMT_LAMBERT:       search = map_init_lambert (GMT); break;     /* Lambert Conformal Conic */
		case GMT_OBLIQUE_MERC:  search = map_init_oblique (GMT); break;     /* Oblique Mercator */
		case GMT_TM:            search = map_init_tm (GMT); break;          /* Transverse Mercator */
		case GMT_UTM:           search = map_init_utm (GMT); break;         /* Universal Transverse Mercator */
		case GMT_LAMB_AZ_EQ:    search = map_init_lambeq (GMT); break;      /* Lambert Azimuthal Equal-Area */
		case GMT_ORTHO:         search = map_init_ortho (GMT); break;       /* Orthographic Projection */
		case GMT_GENPER:        search = map_init_genper (GMT); break;      /* General Perspective Projection */
		case GMT_AZ_EQDIST:     search = map_init_azeqdist (GMT); break;    /* Azimuthal Equal-Distance Projection */
		case GMT_GNOMONIC:      search = map_init_gnomonic (GMT); break;    /* Azimuthal Gnomonic Projection */
		case GMT_MOLLWEIDE:     search = map_init_mollweide (GMT); break;   /* Mollweide Equal-Area */
		case GMT_HAMMER:        search = map_init_hammer (GMT); break;      /* Hammer-Aitoff Equal-Area */
		case GMT_VANGRINTEN:    search = map_init_grinten (GMT); break;     /* Van der Grinten */
		case GMT_WINKEL:        search = map_init_winkel (GMT); break;      /* Winkel Tripel */
		case GMT_ECKERT4:       search = map_init_eckert4 (GMT); break;     /* Eckert IV */
		case GMT_ECKERT6:       search = map_init_eckert6 (GMT); break;     /* Eckert VI */
		case GMT_CYL_EQ:        search = map_init_cyleq (GMT); break;       /* Cylindrical Equal-Area */
		case GMT_CYL_STEREO:    search = map_init_cylstereo (GMT); break;   /* Cylindrical Stereographic */
		case GMT_MILLER:        search = map_init_miller (GMT); break;      /* Miller Cylindrical */
		case GMT_CYL_EQDIST:    search = map_init_cyleqdist (GMT); break;   /* Cylindrical Equidistant */
		case GMT_ROBINSON:      search = map_init_robinson (GMT); break;    /* Robinson */
		case GMT_SINUSOIDAL:    search = map_init_sinusoidal (GMT); break;  /* Sinusoidal Equal-Area */
		case GMT_CASSINI:       search = map_init_cassini (GMT); break;     /* Cassini cylindrical */
		case GMT_ALBERS:        search = map_init_albers (GMT); break;      /* Albers Equal-Area Conic */
		case GMT_ECONIC:        search = map_init_econic (GMT); break;      /* Equidistant Conic */
		case GMT_POLYCONIC:     search = map_init_polyconic (GMT); break;   /* Polyconic */
		default:	/* Non-GMT proj4 projection.  Try to assign functions */
			GMT->current.proj.fwd = &gmt_proj4_fwd;
			GMT->current.proj.inv = &gmt_proj4_inv;
			GMT->current.proj.scale[GMT_X] = GMT->current.proj.scale[GMT_Y] = GMT->current.proj.proj4_scl;
			if (GMT->common.R.oblique) {
				gmt_proj4_fwd (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &xmin, &ymin);
				gmt_proj4_fwd (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], &xmax, &ymax);
				GMT->current.map.outside = &map_rect_outside;
				GMT->current.map.crossing = &map_rect_crossing;
				GMT->current.map.overlap = &map_rect_overlap;
				GMT->current.map.clip = &map_rect_clip;
				GMT->current.map.left_edge = &map_left_rect;
				GMT->current.map.right_edge = &map_right_rect;
				GMT->current.map.frame.check_side = true;
			}
			else {
				map_xy_search (GMT, &xmin, &xmax, &ymin, &ymax, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
				GMT->current.map.outside = &map_wesn_outside;
				GMT->current.map.crossing = &map_wesn_crossing;
				GMT->current.map.overlap = &map_wesn_overlap;
				GMT->current.map.clip = &map_wesn_clip;
				GMT->current.map.left_edge = &map_left_rect;
				GMT->current.map.right_edge = &map_right_rect;
				GMT->current.map.frame.horizontal = 2;
			}
			map_setinfo (GMT, xmin, xmax, ymin, ymax, GMT->current.proj.proj4_scl);
			if (GMT->current.setting.map_frame_type & GMT_IS_FANCY) GMT->current.setting.map_frame_type = GMT_IS_PLAIN;
			break;
	}
	/* Now we only have to replace the pointers to the FWD and INV transform functions */
	GMT->current.proj.fwd = &gmt_proj4_fwd;
	GMT->current.proj.inv = &gmt_proj4_inv;
	return search;
}
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1.1 :	S U P P O R T I N G   R O U T I N E S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/*! . */
void gmt_wesn_search (struct GMT_CTRL *GMT, double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north) {
	double dx, dy, w, e, s, n, x, y, lat, *lon = NULL;
	unsigned int i, j, k;
	bool test_pole[2] = {true, true};

	/* Search for extreme lon/lat coordinates by matching along the rectangular boundary */

	if (!GMT->current.map.n_lon_nodes) GMT->current.map.n_lon_nodes = urint (GMT->current.map.width / GMT->current.setting.map_line_step);
	if (!GMT->current.map.n_lat_nodes) GMT->current.map.n_lat_nodes = urint (GMT->current.map.height / GMT->current.setting.map_line_step);

	dx = (xmax - xmin) / GMT->current.map.n_lon_nodes;
	dy = (ymax - ymin) / GMT->current.map.n_lat_nodes;
	/* Need temp array to hold all the longitudes we compute */
	lon = gmt_M_memory (GMT, NULL, 2 * (GMT->current.map.n_lon_nodes + GMT->current.map.n_lat_nodes + 2), double);
	w = s = DBL_MAX;	e = n = -DBL_MAX;
	for (i = k = 0; i <= GMT->current.map.n_lon_nodes; i++) {
		x = (i == GMT->current.map.n_lon_nodes) ? xmax : xmin + i * dx;
		gmt_xy_to_geo (GMT, &lon[k++], &lat, x, ymin);
		if (lat < s) s = lat;
		if (lat > n) n = lat;
		gmt_xy_to_geo (GMT, &lon[k++], &lat, x, ymax);
		if (lat < s) s = lat;
		if (lat > n) n = lat;
	}
	for (j = 0; j <= GMT->current.map.n_lat_nodes; j++) {
		y = (j == GMT->current.map.n_lat_nodes) ? ymax : ymin + j * dy;
		gmt_xy_to_geo (GMT, &lon[k++], &lat, xmin, y);
		if (lat < s) s = lat;
		if (lat > n) n = lat;
		gmt_xy_to_geo (GMT, &lon[k++], &lat, xmax, y);
		if (lat < s) s = lat;
		if (lat > n) n = lat;
	}
	gmtlib_get_lon_minmax (GMT, lon, k, &w, &e);	/* Determine lon-range by robust quandrant check */
	gmt_M_free (GMT, lon);

	/* Then check if one or both poles are inside map; then the above won't be correct */

	if (GMT->current.proj.projection_GMT == GMT_AZ_EQDIST) {	/* Must be careful since if a pole equals an antipode we get NaNs as coordinates */
		gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, -90.0, &x, &y);
		if (gmt_M_is_dnan (x) && gmt_M_is_dnan (y)) test_pole[0] = false;
		gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, +90.0, &x, &y);
		if (gmt_M_is_dnan (x) && gmt_M_is_dnan (y)) test_pole[1] = false;
	}
	if (test_pole[0] && !gmt_map_outside (GMT, GMT->current.proj.central_meridian, -90.0)) { s = -90.0; w = 0.0; e = 360.0; }
	if (test_pole[1] && !gmt_map_outside (GMT, GMT->current.proj.central_meridian, +90.0)) { n = +90.0; w = 0.0; e = 360.0; }

	s -= 0.1;	if (s < -90.0) s = -90.0;	/* Make sure point is not inside area, 0.1 is just a small arbitrary number */
	n += 0.1;	if (n > 90.0) n = 90.0;		/* But don't go crazy beyond the pole */
	w -= 0.1;	e += 0.1;	if (fabs (w - e) > 360.0) { w = 0.0; e = 360.0; }	/* Ensure max 360 range */
	*west = w;	*east = e;	*south = s;	*north = n;	/* Pass back our findings */
}

/*! . */
GMT_LOCAL void gmtmap_genper_search (struct GMT_CTRL *GMT, double *west, double *east, double *south, double *north) {
	double w, e, s = 90.0, n = -90.0, lat, *lon = NULL, *work_x = NULL, *work_y = NULL;
	uint64_t np, k;
	/* Because the genper clip path may be a mix of straight borders and a curved horizon, we must determine this
	 * clip path and search along it, getting lon,lat along the way, and find the extreme values to use. Before this
	 * function was added, we ended up in the gmt_wesn_search function which would fail along the horizon in many
	 * cases.  P. Wessel, July 20, 2019. */

	np = (GMT->current.proj.polar && (GMT->common.R.wesn[YLO] <= -90.0 || GMT->common.R.wesn[YHI] >= 90.0)) ? GMT->current.map.n_lon_nodes + 2: 2 * (GMT->current.map.n_lon_nodes + 1);
	work_x = gmt_M_memory (GMT, NULL, np, double);
	work_y = gmt_M_memory (GMT, NULL, np, double);
	gmtlib_genper_map_clip_path (GMT, np, work_x, work_y);

	/* Search for extreme lon/lat coordinates by matching along the genper boundary */

	/* Need temp array to hold all the longitudes we compute */
	lon = gmt_M_memory (GMT, NULL, np, double);
	for (k = 0; k < np; k++) {
		gmt_xy_to_geo (GMT, &lon[k], &lat, work_x[k], work_y[k]);
		if (lat < s) s = lat;
		if (lat > n) n = lat;
	}
	gmt_M_free (GMT, work_x);
	gmt_M_free (GMT, work_y);

	gmtlib_get_lon_minmax (GMT, lon, np, &w, &e);	/* Determine lon-range by robust quandrant check */
	gmt_M_free (GMT, lon);

	/* Then check if one or both poles are inside map; then the above won't be correct */

	if (!gmt_map_outside (GMT, GMT->current.proj.central_meridian, -90.0)) { s = -90.0; w = 0.0; e = 360.0; }
	if (!gmt_map_outside (GMT, GMT->current.proj.central_meridian, +90.0)) { n = +90.0; w = 0.0; e = 360.0; }

	s -= 0.1;	if (s < -90.0) s = -90.0;	/* Make sure point is not inside area, 0.1 is just a small arbitrary number */
	n += 0.1;	if (n > 90.0) n = 90.0;		/* But don't go crazy beyond the pole */
	w -= 0.1;	e += 0.1;	if (fabs (w - e) > 360.0) { w = 0.0; e = 360.0; }	/* Ensure max 360 range */
	*west = w;	*east = e;	*south = s;	*north = n;	/* Pass back our findings */
}

/*! . */
GMT_LOCAL int map_horizon_search (struct GMT_CTRL *GMT, double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax) {
	double dx, dy, d, x, y, lon, lat;
	unsigned int i, j;
	bool beyond = false;

	/* Search for extreme original coordinates lon/lat and see if any fall beyond the horizon */

	dx = (xmax - xmin) / GMT->current.map.n_lon_nodes;
	dy = (ymax - ymin) / GMT->current.map.n_lat_nodes;
	if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, w, s)) > GMT->current.proj.f_horizon) beyond = true;
	if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, e, n)) > GMT->current.proj.f_horizon) beyond = true;
	for (i = 0; !beyond && i <= GMT->current.map.n_lon_nodes; i++) {
		x = (i == GMT->current.map.n_lon_nodes) ? xmax : xmin + i * dx;
		gmt_xy_to_geo (GMT, &lon, &lat, x, ymin);
		if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon, lat)) > GMT->current.proj.f_horizon) beyond = true;
		gmt_xy_to_geo (GMT, &lon, &lat, x, ymax);
		if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon, lat)) > GMT->current.proj.f_horizon) beyond = true;
	}
	for (j = 0; !beyond && j <= GMT->current.map.n_lat_nodes; j++) {
		y = (j == GMT->current.map.n_lat_nodes) ? ymax : ymin + j * dy;
		gmt_xy_to_geo (GMT, &lon, &lat, xmin, y);
		if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon, lat)) > GMT->current.proj.f_horizon) beyond = true;
		gmt_xy_to_geo (GMT, &lon, &lat, xmax, y);
		if ((d = gmtlib_great_circle_dist_degree (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, lon, lat)) > GMT->current.proj.f_horizon) beyond = true;
	}
	if (beyond) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Rectangular region for azimuthal projection extends beyond the horizon\n");
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Please select a region that is completely within the visible hemisphere\n");
		GMT_exit (GMT, GMT_PROJECTION_ERROR); return GMT_PROJECTION_ERROR;
	}
	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL unsigned int map_wrap_around_check_x (struct GMT_CTRL *GMT, double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, unsigned int *sides) {
	double dx, dy, width, jump, gmt_half_map_width (struct GMT_CTRL *GMT, double y);

	jump = this_x - last_x;
	width = MAX (gmt_half_map_width (GMT, this_y), gmt_half_map_width (GMT, last_y));

	if (fabs (jump) - width <= GMT_CONV4_LIMIT || fabs (jump) <= GMT_CONV4_LIMIT) return (0);
	dy = this_y - last_y;

	if (jump < -width) {	/* Crossed right boundary */
		dx = GMT->current.map.width + jump;
		yy[0] = yy[1] = last_y + (GMT->current.map.width - last_x) * dy / dx;
		if (yy[0] < 0.0 || yy[0] > GMT->current.proj.rect[YHI]) return (0);
		xx[0] = gmtmap_right_boundary (GMT, yy[0]);	xx[1] = gmtmap_left_boundary (GMT, yy[0]);
		sides[0] = 1;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed left boundary */
		dx = GMT->current.map.width - jump;
		yy[0] = yy[1] = last_y + last_x * dy / dx;
		if (yy[0] < 0.0 || yy[0] > GMT->current.proj.rect[YHI]) return (0);
		xx[0] = gmtmap_left_boundary (GMT, yy[0]);	xx[1] = gmtmap_right_boundary (GMT, yy[0]);
		sides[0] = 3;
		angle[0] = d_atan2d (dy, -dx);
	}
	sides[1] = 4 - sides[0];
	angle[1] = angle[0] + 180.0;

	return (2);
}

/*! . */
GMT_LOCAL void map_get_crossings_x (struct GMT_CTRL *GMT, double *xc, double *yc, double x0, double y0, double x1, double y1) {
	/* Finds crossings for wrap-arounds */
	double xa, xb, ya, yb, dxa, dxb, dyb, left_yb, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (xa > xb) {	/* Make A the minimum x point */
		gmt_M_double_swap (xa, xb);
		gmt_M_double_swap (ya, yb);
	}

	xb -= 2.0 * gmt_half_map_width (GMT, yb);

	dxa = xa - gmtmap_left_boundary (GMT, ya);
	left_yb = gmtmap_left_boundary (GMT, yb);
	dxb = left_yb - xb;
	c = (doubleAlmostEqualZero (left_yb, xb)) ? 0.0 : 1.0 + dxa/dxb;
	dyb = (gmt_M_is_zero (c)) ? 0.0 : fabs (yb - ya) / c;
	yc[0] = yc[1] = (ya > yb) ? yb + dyb : yb - dyb;
	xc[0] = gmtmap_left_boundary (GMT, yc[0]);
	xc[1] = gmtmap_right_boundary (GMT, yc[0]);
}

/*! . */
GMT_LOCAL void map_get_crossings_y (struct GMT_CTRL *GMT, double *xc, double *yc, double x0, double y0, double x1, double y1) {
	/* Finds crossings for wrap-arounds in y, assuming rectangular domain */
	double xa, xb, ya, yb, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (ya > yb) {	/* Make A the minimum y point */
		gmt_M_double_swap (xa, xb);
		gmt_M_double_swap (ya, yb);
	}

	yb -= GMT->current.map.height;	/* Now below the y = 0 line */
	c = (doubleAlmostEqualZero (ya, yb)) ? 0.0 : (xa - xb) / (ya - yb);
	xc[0] = xc[1] = xa - c * ya;
	yc[0] = 0.0;
	yc[1] = GMT->current.map.height;
}

/*! . */
GMT_LOCAL bool map_this_point_wraps_x (struct GMT_CTRL *GMT, double x0, double x1, double w_last, double w_this) {
	/* Returns true if the 2 x-points implies a jump at this y-level of the map */

	double w_min, w_max, dx;
	gmt_M_unused(GMT);

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

	return ((dx = fabs (x1 - x0)) > w_max && w_min > GMT_CONV4_LIMIT);
}

/*! . */
GMT_LOCAL bool map_will_it_wrap_x (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t *start) {
	/* Determines if a polygon will wrap at edges */
	uint64_t i;
	bool wrap;
	double w_last, w_this;

	if (!GMT->current.map.is_world) return (false);
	//if (!GMT->current.map.is_world)
	//	f = (2.0 - GMT_CONV8_LIMIT);
	w_this = gmt_half_map_width (GMT, y[0]);
	for (i = 1, wrap = false; !wrap && i < n; i++) {
		w_last = w_this;
		w_this = gmt_half_map_width (GMT, y[i]);
		wrap = map_this_point_wraps_x (GMT, x[i-1], x[i], w_last, w_this);
	}
	*start = i - 1;
	return (wrap);
}

/*! . */
GMT_LOCAL uint64_t map_truncate_x (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t start, int l_or_r) {
	/* Truncates a wrapping polygon against left or right edge */

	uint64_t i, i1, j, k;
	double xc[4], yc[4], w_last, w_this;

	/* First initialize variables that differ for left and right truncation */

	if (l_or_r == -1)	/* Left truncation (-1) */
		/* Find first point that is left of map center */
		i = (x[start] < GMT->current.map.half_width) ? start : start - 1;
	else				/* Right truncation (+1) */
		/* Find first point that is right of map center */
		i = (x[start] > GMT->current.map.half_width) ? start : start - 1;

	if (!GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);

	GMT->current.plot.x[0] = x[i];	GMT->current.plot.y[0] = y[i];
	w_this = gmt_half_map_width (GMT, y[i]);
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		w_last = w_this;
		w_this = gmt_half_map_width (GMT, y[i]);
		if (map_this_point_wraps_x (GMT, x[i1], x[i], w_last, w_this)) {
			(*GMT->current.map.get_crossings) (GMT, xc, yc, x[i1], y[i1], x[i], y[i]);
			if (l_or_r == -1)
				GMT->current.plot.x[j] = gmtmap_left_boundary (GMT, yc[0]);
			else
				GMT->current.plot.x[j] = gmtmap_right_boundary (GMT, yc[0]);
			GMT->current.plot.y[j] = yc[0];
			j++;
			if (j >= GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		}
		if (l_or_r == -1) /* Left */
			GMT->current.plot.x[j] = (x[i] >= GMT->current.map.half_width) ? gmtmap_left_boundary (GMT, y[i]) : x[i];
		else	/* Right */
			GMT->current.plot.x[j] = (x[i] < GMT->current.map.half_width) ? gmtmap_right_boundary (GMT, y[i]) : x[i];
		GMT->current.plot.y[j] = y[i];
		j++, k++;
		if (j >= GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
	}
	return (j);
}

/*! . */
GMT_LOCAL uint64_t map_truncate_tm (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t start, int b_or_t) {
	/* Truncates a wrapping polygon against bottom or top edge for global TM maps */

	uint64_t i, i1, j, k;
	double xc[4], yc[4], trunc_y;

	/* First initialize variables that differ for bottom and top truncation */

	if (b_or_t == -1) {	/* Bottom truncation (-1) */
		/* Find first point that is below map center */
		i = (y[start] < GMT->current.map.half_height) ? start : start - 1;
		trunc_y = 0.0;
	}
	else {				/* Top truncation (+1) */
		/* Find first point that is above map center */
		i = (y[start] > GMT->current.map.half_height) ? start : start - 1;
		trunc_y = GMT->current.map.height;
	}

	if (!GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);

	GMT->current.plot.x[0] = x[i];	GMT->current.plot.y[0] = y[i];
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		if (map_this_point_wraps_tm (GMT, y[i1], y[i])) {
			map_get_crossings_tm (GMT, xc, yc, x[i1], y[i1], x[i], y[i]);
			GMT->current.plot.x[j] = xc[0];
			GMT->current.plot.y[j] = trunc_y;
			j++;
			if (j >= GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		}
		if (b_or_t == -1) /* Bottom */
			GMT->current.plot.y[j] = (y[i] >= GMT->current.map.half_height) ? 0.0 : y[i];
		else	/* Top */
			GMT->current.plot.y[j] = (y[i] < GMT->current.map.half_height) ? GMT->current.map.height : y[i];
		GMT->current.plot.x[j] = x[i];
		j++, k++;
		if (j >= GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
	}
	return (j);
}

/*! . */
GMT_LOCAL double map_cartesian_dist2 (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* Calculates the good-old straight line distance squared in users units */
	gmt_M_unused(GMT);
	x1 -= x0;	y1 -= y0;
	return (x1*x1 + y1*y1);
}

/*! . */
GMT_LOCAL double map_cartesian_dist_proj2 (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Calculates the good-old straight line distance squared after first projecting the data */
	double x0, y0, x1, y1;
	gmt_geo_to_xy (GMT, lon1, lat1, &x0, &y0);
	gmt_geo_to_xy (GMT, lon2, lat2, &x1, &y1);
	x1 -= x0;	y1 -= y0;
	return (x1*x1 + y1*y1);
}

/*! . */
GMT_LOCAL double map_flatearth_dist_degree (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* Calculates the approximate flat earth distance in degrees.
	   If difference in longitudes exceeds 180 we pick the other
	   offset (360 - offset)
	 */
	double dlon;
	gmt_M_unused(GMT);

	gmt_M_set_delta_lon (x0, x1, dlon);
	return (hypot ( dlon * cosd (0.5 * (y1 + y0)), (y1 - y0)));
}

/*! . */
GMT_LOCAL double map_flatearth_dist_meter (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* Calculates the approximate flat earth distance in km.
	   If difference in longitudes exceeds 180 we pick the other
	   offset (360 - offset)
	 */
	return (map_flatearth_dist_degree (GMT, x0, y0, x1, y1) * GMT->current.proj.DIST_M_PR_DEG);
}

/*! . */
GMT_LOCAL double map_haversine (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Haversine formula for great circle distance.  Intermediate function that returns sin^2 (half_angle).
	 * This avoids problems with short distances where cos(c) is close to 1 and acos is inaccurate.
	 */

	double sx, sy, sin_half_squared;

	if (lat1 == lat2 && lon1 == lon2) return (0.0);

	if (GMT->current.setting.proj_aux_latitude != GMT_LATSWAP_NONE) {	/* Use selected auxiliary latitude */
		lat1 = gmt_lat_swap (GMT, lat1, GMT->current.setting.proj_aux_latitude);
		lat2 = gmt_lat_swap (GMT, lat2, GMT->current.setting.proj_aux_latitude);
	}

	sy = sind (0.5 * (lat2 - lat1));
	sx = sind (0.5 * (lon2 - lon1));	/* If there is a 360 wrap here then the sign of sx is wrong but we only use sx^2 */
	sin_half_squared = sy * sy + cosd (lat2) * cosd (lat1) * sx * sx;

	return (sin_half_squared);
}

/*! . */
GMT_LOCAL double map_geodesic_dist_degree (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE) {
	/* Compute the great circle arc length in degrees on an ellipsoidal
	 * Earth.  We do this by converting to geocentric coordinates.
	 */

	double a, b, c, d, e, f, a1, b1, c1, d1, e1, f1, thg, sc, sd, dist;

	/* Equations are unstable for latitudes of exactly 0 degrees. */

	if (latE == 0.0) latE = 1.0e-08;
	if (latS == 0.0) latS = 1.0e-08;

	/* Must convert from geographic to geocentric coordinates in order
	 * to use the spherical trig equations.  This requires a latitude
	 * correction given by: 1-ECC2=1-2*F + F*F = GMT->current.proj.one_m_ECC2
	 */

	thg = atan (GMT->current.proj.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;

	/* Calculate some trig constants. */

	thg = atan (GMT->current.proj.one_m_ECC2 * tand (latS));
	sincos (thg, &c1, &f1);		f1 = -f1;
	sincosd (lonS, &d1, &e1);	e1 = -e1;
	a1 = f1 * e1;
	b1 = -f1 * d1;

	/* Spherical trig relationships used to compute angles. */

	sc = a * a1 + b * b1 + c * c1;
	sd = 0.5 * sqrt ((pow (a-a1,2.0) + pow (b-b1,2.0) + pow (c-c1,2.0)) * (pow (a+a1,2.0) + pow (b+b1, 2.0) + pow (c+c1, 2.0)));
	dist = atan2d (sd, sc);
	if (dist < 0.0) dist += 360.0;

	return (dist);
}

/*! . */
GMT_LOCAL double map_andoyer_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Approximate geodesic distance on an ellipsoid in degrees
	 *  H. Andoyer from Astronomical Algorithms, Jean Meeus, second edition.
	 */
	double sg = sind (0.5 * (lat2 - lat1));
	double sf = sind (0.5 * (lat2 + lat1));
	double sl = sind (0.5 * (lon2 - lon1));	/* Might have wrong sign if 360 wrap but we only use sl^2 */
	double s, c, w, r, h1, h2;
	sg *= sg;
	sl *= sl;
	sf *= sf;
	s = sg * (1.0 - sl) + (1.0 - sf) * sl;
	c = (1.0 - sg) * (1.0 - sl) + sf * sl;

	w = atan (sqrt (s/c));
	r = sqrt (s*c) / w;
	h1 = 0.5 * (3.0 * r - 1.0) / c;
	h2 = 0.5 * (3.0 * r + 1.0) / s;
	return (2.0 * w * (1.0 + GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening * (h1 * sf * (1.0 - sg) - h2 * (1.0 - sf) * sg)));
}

/*! . */
GMT_LOCAL double map_andoyer_dist_meter (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	return (GMT->current.proj.EQ_RAD * map_andoyer_dist_degree (GMT, lon1, lat1, lon2, lat2));
}

/*! . */
GMT_LOCAL double map_vincenty_dist_meter (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE) {
	/* Translation of NGS FORTRAN code for determination of true distance
	** and respective forward and back azimuths between two points on the
	** ellipsoid.  Good for any pair of points that are not antipodal.
	**
	**      INPUT
	**	latS, lonS -- latitude and longitude of first point in radians.
	**	latE, lonE -- latitude and longitude of second point in radians.
	**
	**	OUTPUT
	**	s -- distance between points in meters.
	** Modified by P.W. from: http://article.gmane.org/gmane.comp.gis.proj-4.devel/3478
	*/
	double s, c, d, e, r, f, d_lon, dx, x, y, sa, cx, cy, cz, sx, sy, c2a, cu1, cu2, su1, tu1, tu2, ts, baz, faz;
	int n_iter = 0;

	f = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening;
	r = 1.0 - f;
	tu1 = r * tand (latS);
	tu2 = r * tand (latE);
	cu1 = 1.0 / sqrt (tu1 * tu1 + 1.0);
	su1 = cu1 * tu1;
	cu2 = 1.0 / sqrt (tu2 * tu2 + 1.0);
	ts  = cu1 * cu2;
	baz = ts * tu2;
	faz = baz * tu1;
	gmt_M_set_delta_lon (lonS, lonE, d_lon);
	if (gmt_M_is_zero (d_lon) && doubleAlmostEqualZero (latS, latE)) return 0.0;
	x = dx = D2R * d_lon;
	do {
		n_iter++;
		sincos (x, &sx, &cx);
		tu1 = cu2 * sx;
		tu2 = baz - su1 * cu2 * cx;
		sy = sqrt (tu1 * tu1 + tu2 * tu2);
		cy = ts * cx + faz;
		y = atan2 (sy, cy);
		sa = ts * sx / sy;
		c2a = -sa * sa + 1.0;
		cz = faz + faz;
		if (c2a > 0.0) cz = -cz / c2a + cy;
		e = cz * cz * 2.0 - 1.0;
		c = ((c2a * -3.0 + 4.0) * f + 4.0) * c2a * f / 16.0;
		d = x;
		x = ((e * cy * c + cz) * sy * c + y) * sa;
		x = (1.0 - c) * x * f + dx;
	} while (fabs (d - x) > VINCENTY_EPS && n_iter <= 50);
	if (n_iter > VINCENTY_MAX_ITER) {
		GMT->current.proj.n_geodesic_approx++;	/* Count inaccurate results */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Near- or actual antipodal points encountered. Precision may be reduced slightly.\n");
		s = M_PI;
	}
	else {
		x = sqrt ((1.0 / r / r - 1.0) * c2a + 1.0) + 1.0;
		x = (x - 2.0) / x;
		c = (x * x / 4.0 + 1.0) / (1.0 - x);
		d = (x * 0.375 * x - 1.0) * x;
		s = ((((sy * sy * 4.0 - 3.0) * (1.0 - e - e) * cz * d / 6.0 - e * cy) * d / 4.0 + cz) * sy * d + y) * c * r;
		if (s > M_PI) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Near- or actual antipodal points encountered. Precision may be reduced slightly.\n");
			s = M_PI;
		}
	}
	GMT->current.proj.n_geodesic_calls++;
	return (s * GMT->current.proj.EQ_RAD);
}

/*! . */
GMT_LOCAL double map_rudoe_dist_meter (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE) {
	/* Compute length of geodesic between locations in meters
	 * We use Rudoe's equation from Bomford.
	 */

	double e1, el, sinthi, costhi, sinthk, costhk, tanthi, tanthk, sina12, cosa12, d_lon;
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
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	el = GMT->current.proj.ECC2 / GMT->current.proj.one_m_ECC2;
	e1 = 1.0 + el;
	sincosd (latE, &sinthi, &costhi);
	sincosd (latS, &sinthk, &costhk);
	gmt_M_set_delta_lon (lonS, lonE, d_lon);
	sincosd (d_lon, &sin_dl, &cos_dl);
	tanthi = sinthi / costhi;
	tanthk = sinthk / costhk;
	al = tanthi / (e1 * tanthk) + GMT->current.proj.ECC2 * sqrt ((e1 + tanthi * tanthi) / (e1 + tanthk * tanthk));
	a12top = sin_dl;
	a12bot = (al - cos_dl) * sinthk;
	a12 = atan2 (a12top,a12bot);
	sincos (a12, &sina12, &cosa12);
	e1 = el * (pow (costhk * cosa12, 2.0) + sinthk * sinthk);
	e2 = e1 * e1;
	e3 = e1 * e2;
	c0 = 1.0 + 0.25 * e1 - (3.0 / 64.0) * e2 + (5.0 / 256.0) * e3;
	c2 = -0.125 * e1 + (1.0 / 32) * e2 - (15.0 / 1024.0) * e3;
	c4 = -(1.0 / 256.0) * e2 + (3.0 / 1024.0) * e3;
	v1 = GMT->current.proj.EQ_RAD / sqrt (1.0 - GMT->current.proj.ECC2 * sinthk * sinthk);
	v2 = GMT->current.proj.EQ_RAD / sqrt (1.0 - GMT->current.proj.ECC2 * sinthi * sinthi);
	z1 = v1 * (1.0 - GMT->current.proj.ECC2) * sinthk;
	z2 = v2 * (1.0 - GMT->current.proj.ECC2) * sinthi;
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

/*! . */
GMT_LOCAL double map_loxodrome_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Calculates the distance along the loxodrome, in meter */
	double dist, d_lon;
	gmt_M_set_delta_lon (lon1, lon2, d_lon);
	if (doubleAlmostEqualZero (lat1, lat2)) {	/* Along parallel */
		if (GMT->current.proj.GMT_convert_latitudes) lat1 = gmt_M_latg_to_latc (GMT, lat1);
		dist = fabs (d_lon) * cosd (lat1);
	}
	else { /* General case */
		double dx, dy, Az;
		if (GMT->current.proj.GMT_convert_latitudes) {
			lat1 = gmt_M_latg_to_latc (GMT, lat1);
			lat2 = gmt_M_latg_to_latc (GMT, lat2);
		}
		dx = D2R * d_lon;
		dy = d_log (GMT, tand (45.0 + 0.5 * lat2)) - d_log (GMT, tand (45.0 + 0.5 * lat1));
		Az = atan2 (dx, dy);
		dist = fabs (dx / cos (Az));
	}
	return (dist);
}

/*! . */
GMT_LOCAL double map_loxodrome_dist_meter (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Calculates the loxodrome distance in meter */
	return (map_loxodrome_dist_degree (GMT, lon1, lat1, lon2, lat2) * GMT->current.proj.DIST_M_PR_DEG);
}

/*! . */
GMT_LOCAL double map_az_backaz_loxodrome (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	/* Calculate azimuths or backazimuths.  Loxodrome mode.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, d_lon;

	if (baz) {	/* exchange point one and two */
		gmt_M_double_swap (lonS, lonE);
		gmt_M_double_swap (latS, latE);
	}
	gmt_M_set_delta_lon (lonE, lonS, d_lon);
	if (doubleAlmostEqualZero (latS, latE))	/* Along parallel */
		az = (d_lon > 0.0) ? 90 : -90.0;
	else { /* General case */
		double dx, dy;
		if (GMT->current.proj.GMT_convert_latitudes) {
			latS = gmt_M_latg_to_latc (GMT, latS);
			latE = gmt_M_latg_to_latc (GMT, latE);
		}
		dx = D2R * d_lon;
		dy = d_log (GMT, tand (45.0 + 0.5 * latS)) - d_log (GMT, tand (45.0 + 0.5 * latE));
		az = atan2d (dx, dy);
		if (az < 0.0) az += 360.0;
	}
	return (az);
}

/*! . */
GMT_LOCAL bool map_near_a_point_spherical (struct GMT_CTRL *GMT, double x, double y, struct GMT_DATATABLE *T, double dist) {
	uint64_t row, seg;
	bool each_point_has_distance;
	double d;

	each_point_has_distance = (dist <= 0.0 && T->segment[0]->n_columns > 2);
	for (seg = 0; seg < T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			d = gmt_distance (GMT, x, y, T->segment[seg]->data[GMT_X][row], T->segment[seg]->data[GMT_Y][row]);
			if (each_point_has_distance) dist = T->segment[seg]->data[GMT_Z][row];
			if (d <= dist) return (true);
		}
	}
	return (false);
}

/*! . */
GMT_LOCAL bool map_near_a_point_cartesian (struct GMT_CTRL *GMT, double x, double y, struct GMT_DATATABLE *T, double dist) {
	/* Since Cartesian we use a gmt_distance set to return distances^2 (avoiding hypot) */
	bool each_point_has_distance;
	uint64_t row, seg;
	double d, x0, y0, xn, d0, d02 = 0.0, dn;

	each_point_has_distance = (dist <= 0.0 && T->segment[0]->n_columns > 2);

	/* Assumes the points have been sorted so xp[0] is xmin and xp[n-1] is xmax] !!! */

	/* See if we are safely outside the range */
	x0 = T->segment[0]->data[GMT_X][0];
	d0 = (each_point_has_distance) ? T->segment[0]->data[GMT_Z][0] : dist;
	xn = T->segment[T->n_segments-1]->data[GMT_X][T->segment[T->n_segments-1]->n_rows-1];
	dn = (each_point_has_distance) ? T->segment[T->n_segments-1]->data[GMT_Z][T->segment[T->n_segments-1]->n_rows-1] : dist;
	if ((x < (x0 - d0)) || (x > (xn) + dn)) return (false);

	/* No, must search the points */
	if (!each_point_has_distance) d02 = dist * dist;

	for (seg = 0; seg < T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			x0 = T->segment[seg]->data[GMT_X][row];
			d0 = (each_point_has_distance) ? T->segment[seg]->data[GMT_Z][row] : dist;
			if (fabs (x - x0) <= d0) {	/* Simple x-range test first */
				y0 = T->segment[seg]->data[GMT_Y][row];
				if (fabs (y - y0) <= d0) {	/* Simple y-range test next */
					/* Here we must compute distance */
					if (each_point_has_distance) d02 = d0 * d0;
					d = gmt_distance (GMT, x, y, x0, y0);
					if (d <= d02) return (true);
				}
			}
		}
	}
	return (false);
}

/* Functions involving distance from arbitrary points to a line */

/*! . */
GMT_LOCAL bool map_near_a_line_cartesian (struct GMT_CTRL *GMT, double lon, double lat, uint64_t seg, struct GMT_DATASEGMENT *S, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	bool perpendicular_only = false, interior, within;
	uint64_t row0, row1;
	double edge, dx, dy, xc, yc, s, s_inv, d, dist_AB, fraction;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	/* map_near_a_line_cartesian works in one of two modes, depending on return_mindist.
	   Since return_mindist is composed of two settings we must first set
	   perpendicular_only = (return_mindist >= 10);
	   return_mindist -= 10 * perpendicular_only;
	   That is, if 10 was added it means perpendicular_only is set and then the 10 is
	   removed.  We now consider what is left of return_mindist:
	   (1) return_mindist == 0:
	      We expect each segment to have its dist variable set to a minimum distance,
	      and if the point is within this distance from the line then we return true;
	      otherwise we return false.  If the segments have not set their distances then
	      it will have been initialized at 0 and only a point on the line will return true.
	      If perpendicular_only we ignore a point that is within the distance of the
	      linesegment endpoints but project onto the extension of the line (i.e., it is
	      "outside" the extent of the line).  We return false in that case.
	   (2) return_mindist != 0:
	      Return the minimum distance via dist_min. In addition, if > 1:
	      If == 2 we also return the coordinate of nearest point via x_near, y_near.
	      If == 3 we instead return segment number and point number (fractional) of that point via x_near, y_near.
	      The function will always return true, except if perpendicular_only is set: then we
	      return false if the point projects onto the extension of the line (i.e., it is "outside"
	      the extent of the line).  */

	if (return_mindist >= 10) {	/* Exclude circular region surrounding line endpoints */
		perpendicular_only = true;
		return_mindist -= 10;
	}

	if (S->n_rows <= 0) return (false);	/* empty; skip */

	if (return_mindist) SH->dist = 0.0;	/* Explicitly set dist to zero so the shortest distance can be found */

	/* Find nearest point on this line */

	for (row0 = 0; row0 < S->n_rows; row0++) {	/* loop over nodes on current line */
		d = gmt_distance (GMT, lon, lat, S->data[GMT_X][row0], S->data[GMT_Y][row0]);	/* Distance between our point and j'th node on seg'th line */
		if (return_mindist && d < (*dist_min)) {	/* Update min distance */
			*dist_min = d;
			if (return_mindist == 2) { *x_near = S->data[GMT_X][row0]; *y_near = S->data[GMT_Y][row0]; }	/* Also update (x,y) of nearest point on the line */
			else if (return_mindist == 3) { *x_near = (double)seg; *y_near = (double)row0;}		/* Instead update (seg, pt) of nearest point on the line */
		}
		interior = (row0 > 0 && row0 < (S->n_rows - 1));	/* Only false if we are processing one of the end points */
		if (d <= SH->dist && (interior || !perpendicular_only)) return (true);		/* Node inside the critical distance; we are done */
	}

	if (S->n_rows < 2) return (false);	/* 1-point "line" is a point; skip segment check */

	/* If we get here we must check for intermediate points along the straight lines between segment nodes.
	 * However, since we know all nodes are outside the circle, we first check if the pair of nodes making
	 * up the next line segment are outside of the circumscribing square before we need to solve for the
	 * intersection between the line segment and the normal from our point. */

	for (row0 = 0, row1 = 1, within = false; row1 < S->n_rows; row0++, row1++) {	/* loop over straight segments on current line */
		if (!return_mindist) {
			edge = lon - SH->dist;
			if (S->data[GMT_X][row0] < edge && S->data[GMT_X][row1] < edge) continue;	/* Left of square */
			edge = lon + SH->dist;
			if (S->data[GMT_X][row0] > edge && S->data[GMT_X][row1] > edge) continue;	/* Right of square */
			edge = lat - SH->dist;
			if (S->data[GMT_Y][row0] < edge && S->data[GMT_Y][row1] < edge) continue;	/* Below square */
			edge = lat + SH->dist;
			if (S->data[GMT_Y][row0] > edge && S->data[GMT_Y][row1] > edge) continue;	/* Above square */
		}

		/* Here there is potential for the line segment crossing inside the circle */

		dx = S->data[GMT_X][row1] - S->data[GMT_X][row0];
		dy = S->data[GMT_Y][row1] - S->data[GMT_Y][row0];
		if (dx == 0.0) {		/* Line segment is vertical, our normal is thus horizontal */
			if (dy == 0.0) continue;	/* Dummy segment with no length */
			xc = S->data[GMT_X][row0];
			yc = lat;
			if (S->data[GMT_Y][row0] < yc && S->data[GMT_Y][row1] < yc ) continue;	/* Cross point is on extension */
			if (S->data[GMT_Y][row0] > yc && S->data[GMT_Y][row1] > yc ) continue;	/* Cross point is on extension */
		}
		else {	/* Line segment is not vertical */
			if (dy == 0.0) {	/* Line segment is horizontal, our normal is thus vertical */
				xc = lon;
				yc = S->data[GMT_Y][row0];
			}
			else {	/* General case of oblique line */
				s = dy / dx;
				s_inv = -1.0 / s;
				xc = (lat - S->data[GMT_Y][row0] + s * S->data[GMT_X][row0] - s_inv * lon ) / (s - s_inv);
				yc = S->data[GMT_Y][row0] + s * (xc - S->data[GMT_X][row0]);

			}
			/* To be inside, (xc, yc) must (1) be on the line segment and not its extension and (2) be within dist of our point */

			if (S->data[GMT_X][row0] < xc && S->data[GMT_X][row1] < xc ) continue;	/* Cross point is on extension */
			if (S->data[GMT_X][row0] > xc && S->data[GMT_X][row1] > xc ) continue;	/* Cross point is on extension */
		}

		/* OK, here we must check how close the crossing point is */

		d = gmt_distance (GMT, lon, lat, xc, yc);			/* Distance between our point and intersection */
		if (return_mindist && d < (*dist_min)) {			/* Update min distance */
			*dist_min = d;
			if (return_mindist == 2) { *x_near = xc; *y_near = yc;}	/* Also update nearest point on the line */
			else if (return_mindist == 3) {	/* Instead update (seg, pt) of nearest point on the line */
				*x_near = (double)seg;
				dist_AB = gmt_distance (GMT, S->data[GMT_X][row0], S->data[GMT_Y][row0], S->data[GMT_X][row1], S->data[GMT_Y][row1]);
				fraction = (dist_AB > 0.0) ? gmt_distance (GMT, S->data[GMT_X][row0], S->data[GMT_Y][row0], xc, yc) / dist_AB : 0.0;
				*y_near = (double)row0 + fraction;
			}
			within = true;
		}
		if (d <= SH->dist) return (true);		/* Node inside the critical distance; we are done */
	}

	return (within);	/* All tests failed, we are not close to the line(s), or we just return distance and interior (see comments above) */
}

/*! . */
GMT_LOCAL bool map_near_lines_cartesian (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	uint64_t seg;
	int mode = return_mindist, status;
	bool OK = false;
	if (mode >= 10) mode -= 10;	/* Exclude (or flag) circular region surrounding line endpoints */
	if (mode) *dist_min = DBL_MAX;	/* Want to find the minimum distance so init to huge */

	for (seg = 0; seg < T->n_segments; seg++) {	/* Loop over each line segment */
		status = map_near_a_line_cartesian (GMT, lon, lat, seg, T->segment[seg], return_mindist, dist_min, x_near, y_near);
		if (status) {	/* Got a min distance or satisfied the min dist requirement */
			if (!return_mindist) return (true);	/* Done, we are within distance of one of the lines */
			OK = true;
		}
	}
	return (OK);
}

/*! . */
GMT_LOCAL bool map_near_a_line_spherical (struct GMT_CTRL *P, double lon, double lat, uint64_t seg, struct GMT_DATASEGMENT *S, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	bool perpendicular_only = false, interior, within;
	uint64_t row, prev_row;
	double d, A[3], B[3], GMT[3], X[3], xlon, xlat, cx_dist, cos_dist, dist_AB, fraction;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);

	/* map_near_a_line_spherical works in one of two modes, depending on return_mindist.
	   Since return_mindist is composed of two settings we must first set
	   perpendicular_only = (return_mindist >= 10);
	   return_mindist -= 10 * perpendicular_only;
	   That is, if 10 was added it means perpendicular_only is set and then the 10 is
	   removed.  We now consider what is left of return_mindist:
	   (1) return_mindist == 0:
	      We expect each segment to have its dist variable set to a minimum distance,
	      and if the point is within this distance from the line then we return true;
	      otherwise we return false.  If the segments have not set their distances then
	      it will have been initialized at 0 and only a point on the line will return true.
	      If perpendicular_only we ignore a point that is within the distance of the
	      linesegment endpoints but project onto the extension of the line (i.e., it is
	      "outside" the extent of the line).  We return false in that case.
	   (2) return_mindist != 0:
	      Return the minimum distance via dist_min. In addition, if > 1:
	      If == 2 we also return the coordinate of nearest point via x_near, y_near.
	      If == 3 we instead return segment number and point number (fractional) of that point via x_near, y_near.
	      The function will always return true, except if perpendicular_only is set: then we
	      return false if the point projects onto the extension of the line (i.e., it is "outside"
	      the extent of the line).  */

	if (return_mindist >= 10) {	/* Exclude (or flag) circular region surrounding line endpoints */
		perpendicular_only = true;
		return_mindist -= 10;
	}
	gmt_geo_to_cart (P, lat, lon, GMT, true);	/* Our point to test is now GMT */

	if (S->n_rows <= 0) return (false);	/* Empty ; skip */

	/* Find nearest point on this line */

	if (return_mindist) SH->dist = 0.0;	/* Explicitly set dist to zero so the shortest distance can be found */

	for (row = 0; row < S->n_rows; row++) {	/* loop over nodes on current line */
		d = gmt_distance (P, lon, lat, S->data[GMT_X][row], S->data[GMT_Y][row]);	/* Distance between our point and row'th node on seg'th line */
		if (return_mindist && d < (*dist_min)) {	/* Update minimum distance */
			*dist_min = d;
			if (return_mindist == 2) *x_near = S->data[GMT_X][row], *y_near = S->data[GMT_Y][row];	/* Also update (x,y) of nearest point on the line */
			else if (return_mindist == 3) *x_near = (double)seg, *y_near = (double)row;	/* Also update (seg, pt) of nearest point on the line */
		}
		interior = (row > 0 && row < (S->n_rows - 1));	/* Only false if we are processing one of the end points */
		if (d <= SH->dist && (interior || !perpendicular_only)) return (true);			/* Node inside the critical distance; we are done */
	}

	if (S->n_rows < 2) return (false);	/* 1-point "line" is a point; skip segment check */

	/* If we get here we must check for intermediate points along the great circle lines between segment nodes.*/

	if (return_mindist)		/* Cosine of the great circle distance we are checking for. 2 ensures failure to be closer */
		cos_dist = 2.0;
	else if (P->current.map.dist[GMT_MAP_DIST].arc)	/* Used angular distance measure */
		cos_dist = cosd (SH->dist / P->current.map.dist[GMT_MAP_DIST].scale);
	else	/* Used distance units (e.g., meter, km). Conv to meters, then to degrees */
		cos_dist = cosd ((SH->dist / P->current.map.dist[GMT_MAP_DIST].scale) / P->current.proj.DIST_M_PR_DEG);
	gmt_geo_to_cart (P, S->data[GMT_Y][0], S->data[GMT_X][0], B, true);		/* 3-D vector of end of last segment */

	for (row = 1, within = false; row < S->n_rows; row++) {				/* loop over great circle segments on current line */
		gmt_M_memcpy (A, B, 3, double);	/* End of last segment is start of new segment */
		gmt_geo_to_cart (P, S->data[GMT_Y][row], S->data[GMT_X][row], B, true);	/* 3-D vector of end of this segment */
		if (gmtlib_great_circle_intersection (P, A, B, GMT, X, &cx_dist)) continue;	/* X not between A and B */
		if (return_mindist) {		/* Get lon, lat of X, calculate distance, and update min_dist if needed */
			gmt_cart_to_geo (P, &xlat, &xlon, X, true);
			d = gmt_distance (P, xlon, xlat, lon, lat);	/* Distance between our point and closest perpendicular point on seg'th line */
			if (d < (*dist_min)) {	/* Update minimum distance */
				*dist_min = d;
				if (return_mindist == 2) { *x_near = xlon; *y_near = xlat;}	/* Also update (x,y) of nearest point on the line */
				else if (return_mindist == 3) {	/* Also update (seg, pt) of nearest point on the line */
					*x_near = (double)seg;
					prev_row = row - 1;
					dist_AB = gmt_distance (P, S->data[GMT_X][prev_row], S->data[GMT_Y][prev_row], S->data[GMT_X][row], S->data[GMT_Y][row]);
					fraction = (dist_AB > 0.0) ? gmt_distance (P, S->data[GMT_X][prev_row], S->data[GMT_Y][prev_row], xlon, xlat) / dist_AB : 0.0;
					*y_near = (double)prev_row + fraction;
				}
				within = true;	/* Found at least one segment with a valid inside distance */
			}
		}
		if (cx_dist >= cos_dist) return (true);	/* X is on the A-B extension AND within specified distance */
	}

	if (return_mindist && *dist_min < GMT_CONV8_LIMIT) *dist_min = 0.0;

	return (within);	/* All tests failed, we are not close to the line(s), or we return a mindist (see comments above) */
}

/*! . */
GMT_LOCAL bool map_near_lines_spherical (struct GMT_CTRL *P, double lon, double lat, struct GMT_DATATABLE *T, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	uint64_t seg;
	int mode = return_mindist, status;
	bool OK = false;
	if (mode >= 10) mode -= 10;	/* Exclude (or flag) circular region surrounding line endpoints */
	if (mode) *dist_min = DBL_MAX;	/* Want to find the minimum distance so init to huge */

	for (seg = 0; seg < T->n_segments; seg++) {	/* Loop over each line segment */
		status = map_near_a_line_spherical (P, lon, lat, seg, T->segment[seg], return_mindist, dist_min, x_near, y_near);
		if (status) {	/* Got a min distance or satisfied the min dist requirement */
			if (!return_mindist) return (true);	/* Done, we are within distance of one of the lines */
			OK = true;
		}
	}
	return (OK);
}

/*! . */
GMT_LOCAL int map_init_three_D (struct GMT_CTRL *GMT) {
	unsigned int i;
	bool easy, positive;
	double x, y, zmin = 0.0, zmax = 0.0, z_range;

	GMT->current.proj.three_D = (GMT->current.proj.z_project.view_azimuth != 180.0 || GMT->current.proj.z_project.view_elevation != 90.0);
	GMT->current.proj.scale[GMT_Z] = GMT->current.proj.z_pars[0];
	GMT->current.proj.xyz_pos[GMT_Z] = (GMT->current.proj.scale[GMT_Z] >= 0.0);	/* Increase z up or not */
	/* z_level == DBL_MAX is signaling that it was not set by the user. In that case we change it to the lower z level */
	if (GMT->current.proj.z_level == DBL_MAX) GMT->current.proj.z_level = (GMT->current.proj.xyz_pos[GMT_Z]) ?  GMT->common.R.wesn[ZLO] : GMT->common.R.wesn[ZHI];

	switch (GMT->current.proj.xyz_projection[GMT_Z]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			zmin = (GMT->current.proj.xyz_pos[GMT_Z]) ? GMT->common.R.wesn[ZLO] : GMT->common.R.wesn[ZHI];
			zmax = (GMT->current.proj.xyz_pos[GMT_Z]) ? GMT->common.R.wesn[ZHI] : GMT->common.R.wesn[ZLO];
			GMT->current.proj.fwd_z = &gmt_translin;
			GMT->current.proj.inv_z = &gmt_itranslin;
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (GMT->common.R.wesn[ZLO] <= 0.0 || GMT->common.R.wesn[ZHI] <= 0.0) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Jz -JZ: limits must be positive for log10 projection\n");
				GMT_exit (GMT, GMT_PROJECTION_ERROR); return GMT_PROJECTION_ERROR;
			}
			zmin = (GMT->current.proj.xyz_pos[GMT_Z]) ? d_log10 (GMT, GMT->common.R.wesn[ZLO]) : d_log10 (GMT, GMT->common.R.wesn[ZHI]);
			zmax = (GMT->current.proj.xyz_pos[GMT_Z]) ? d_log10 (GMT, GMT->common.R.wesn[ZHI]) : d_log10 (GMT, GMT->common.R.wesn[ZLO]);
			GMT->current.proj.fwd_z = &gmt_translog10;
			GMT->current.proj.inv_z = &gmt_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			GMT->current.proj.xyz_pow[GMT_Z] = GMT->current.proj.z_pars[1];
			GMT->current.proj.xyz_ipow[GMT_Z] = 1.0 / GMT->current.proj.z_pars[1];
			positive = !((GMT->current.proj.xyz_pos[GMT_Z] + (GMT->current.proj.xyz_pow[GMT_Z] > 0.0)) % 2);
			zmin = (positive) ? pow (GMT->common.R.wesn[ZLO], GMT->current.proj.xyz_pow[GMT_Z]) : pow (GMT->common.R.wesn[ZHI], GMT->current.proj.xyz_pow[GMT_Z]);
			zmax = (positive) ? pow (GMT->common.R.wesn[ZHI], GMT->current.proj.xyz_pow[GMT_Z]) : pow (GMT->common.R.wesn[ZLO], GMT->current.proj.xyz_pow[GMT_Z]);
			GMT->current.proj.fwd_z = &gmt_transpowz;
			GMT->current.proj.inv_z = &gmt_itranspowz;
	}
	z_range = zmax - zmin;
	if (z_range == 0.0 && GMT->current.proj.compute_scale[GMT_Z])
		GMT->current.proj.scale[GMT_Z] = 0.0;	/* No range given, just flat projected map */
	else if (GMT->current.proj.compute_scale[GMT_Z])
		GMT->current.proj.scale[GMT_Z] /= fabs (z_range);
	GMT->current.proj.zmax = z_range * GMT->current.proj.scale[GMT_Z];
	GMT->current.proj.origin[GMT_Z] = -zmin * GMT->current.proj.scale[GMT_Z];

	if (GMT->current.proj.z_project.view_azimuth >= 360.0) GMT->current.proj.z_project.view_azimuth -= 360.0;
	if (GMT->current.proj.z_project.view_azimuth < 0.0)    GMT->current.proj.z_project.view_azimuth += 360.0;
	GMT->current.proj.z_project.quadrant = urint (floor (GMT->current.proj.z_project.view_azimuth / 90.0)) + 1;
	sincosd (GMT->current.proj.z_project.view_azimuth, &GMT->current.proj.z_project.sin_az, &GMT->current.proj.z_project.cos_az);
	sincosd (GMT->current.proj.z_project.view_elevation, &GMT->current.proj.z_project.sin_el, &GMT->current.proj.z_project.cos_el);

	/* Determine min/max y of plot */

	/* easy = true means we can use 4 corner points to find min x and y, false
	   means we must search along wesn perimeter the hard way */

	switch (GMT->current.proj.projection_GMT) {
		case GMT_LINEAR:
		case GMT_MERCATOR:
		case GMT_OBLIQUE_MERC:
		case GMT_CYL_EQ:
		case GMT_CYL_EQDIST:
		case GMT_CYL_STEREO:
		case GMT_MILLER:
			easy = true;
			break;
		case GMT_POLAR:
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
			easy = GMT->common.R.oblique;
			break;
		default:
			easy = false;
			break;
	}

	if (!GMT->current.proj.three_D) easy = true;

	GMT->current.proj.z_project.xmin = GMT->current.proj.z_project.ymin = DBL_MAX;
	GMT->current.proj.z_project.xmax = GMT->current.proj.z_project.ymax = -DBL_MAX;

	if (easy) {
		double xx[4], yy[4];

		xx[0] = xx[3] = GMT->current.proj.rect[XLO]; xx[1] = xx[2] = GMT->current.proj.rect[XHI];
		yy[0] = yy[1] = GMT->current.proj.rect[YLO]; yy[2] = yy[3] = GMT->current.proj.rect[YHI];

		for (i = 0; i < 4; i++) {
			gmt_xy_to_geo (GMT, &GMT->current.proj.z_project.corner_x[i], &GMT->current.proj.z_project.corner_y[i], xx[i], yy[i]);
			gmt_xyz_to_xy (GMT, xx[i], yy[i], gmt_z_to_zz(GMT, GMT->common.R.wesn[ZLO]), &x, &y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			gmt_xyz_to_xy (GMT, xx[i], yy[i], gmt_z_to_zz(GMT, GMT->common.R.wesn[ZHI]), &x, &y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
		}
	}
	else if (GMT->current.proj.r > 0.0) {	/* Do not think the next four lines mean anything in this case, just copied from the general case */
		GMT->current.proj.z_project.corner_x[0] = GMT->current.proj.z_project.corner_x[3] = (GMT->current.proj.xyz_pos[GMT_X]) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
		GMT->current.proj.z_project.corner_x[1] = GMT->current.proj.z_project.corner_x[2] = (GMT->current.proj.xyz_pos[GMT_X]) ? GMT->common.R.wesn[XHI] : GMT->common.R.wesn[XLO];
		GMT->current.proj.z_project.corner_y[0] = GMT->current.proj.z_project.corner_y[1] = (GMT->current.proj.xyz_pos[GMT_Y]) ? GMT->common.R.wesn[YLO] : GMT->common.R.wesn[YHI];
		GMT->current.proj.z_project.corner_y[2] = GMT->current.proj.z_project.corner_y[3] = (GMT->current.proj.xyz_pos[GMT_Y]) ? GMT->common.R.wesn[YHI] : GMT->common.R.wesn[YLO];
		for (i = 0; i < 360; i++) {	/* Go around the circle */
			sincosd (i * 1.0, &y, &x);
			gmt_xyz_to_xy (GMT, GMT->current.proj.r * (1.0 + x), GMT->current.proj.r * (1.0 + y), gmt_z_to_zz(GMT, GMT->common.R.wesn[ZLO]), &x, &y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			gmt_xyz_to_xy (GMT, GMT->current.proj.r * (1.0 + x), GMT->current.proj.r * (1.0 + y), gmt_z_to_zz(GMT, GMT->common.R.wesn[ZHI]), &x, &y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
		}
	}
	else {
		GMT->current.proj.z_project.corner_x[0] = GMT->current.proj.z_project.corner_x[3] = (GMT->current.proj.xyz_pos[GMT_X]) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI];
		GMT->current.proj.z_project.corner_x[1] = GMT->current.proj.z_project.corner_x[2] = (GMT->current.proj.xyz_pos[GMT_X]) ? GMT->common.R.wesn[XHI] : GMT->common.R.wesn[XLO];
		GMT->current.proj.z_project.corner_y[0] = GMT->current.proj.z_project.corner_y[1] = (GMT->current.proj.xyz_pos[GMT_Y]) ? GMT->common.R.wesn[YLO] : GMT->common.R.wesn[YHI];
		GMT->current.proj.z_project.corner_y[2] = GMT->current.proj.z_project.corner_y[3] = (GMT->current.proj.xyz_pos[GMT_Y]) ? GMT->common.R.wesn[YHI] : GMT->common.R.wesn[YLO];
		for (i = 0; i < GMT->current.map.n_lon_nodes; i++) {	/* S and N */
			gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YLO], GMT->common.R.wesn[ZLO], &x, &y);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			if (GMT->common.R.wesn[ZHI] != GMT->common.R.wesn[ZLO]) {
				gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YLO], GMT->common.R.wesn[ZHI], &x, &y);
				GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
				GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
				GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
				GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			}
			gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YHI], GMT->common.R.wesn[ZLO], &x, &y);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			if (GMT->common.R.wesn[ZHI] != GMT->common.R.wesn[ZLO]) {
				gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YHI], GMT->common.R.wesn[ZHI], &x, &y);
				GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
				GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
				GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
				GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			}
		}
		for (i = 0; i < GMT->current.map.n_lat_nodes; i++) {	/* W and E */
			gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat, GMT->common.R.wesn[ZLO], &x, &y);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			if (GMT->common.R.wesn[ZHI] != GMT->common.R.wesn[ZLO]) {
				gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat, GMT->common.R.wesn[ZHI], &x, &y);
				GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
				GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
				GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
				GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			}
			gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat, GMT->common.R.wesn[ZLO], &x, &y);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
			GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
			GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			if (GMT->common.R.wesn[ZHI] != GMT->common.R.wesn[ZLO]) {
				gmt_geoz_to_xy (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat, GMT->common.R.wesn[ZHI], &x, &y);
				GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, y);
				GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, y);
				GMT->current.proj.z_project.xmin = MIN (GMT->current.proj.z_project.xmin, x);
				GMT->current.proj.z_project.xmax = MAX (GMT->current.proj.z_project.xmax, x);
			}
		}
	}

	GMT->current.proj.z_project.face[0] = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? 0 : 1;
	GMT->current.proj.z_project.face[1] = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? 2 : 3;
	GMT->current.proj.z_project.face[2] = (GMT->current.proj.z_project.view_elevation >= 0.0) ? 4 : 5;
	GMT->current.proj.z_project.draw[0] = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? true : false;
	GMT->current.proj.z_project.draw[1] = (GMT->current.proj.z_project.quadrant == 3 || GMT->current.proj.z_project.quadrant == 4) ? true : false;
	GMT->current.proj.z_project.draw[2] = (GMT->current.proj.z_project.quadrant == 2 || GMT->current.proj.z_project.quadrant == 3) ? true : false;
	GMT->current.proj.z_project.draw[3] = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? true : false;
	GMT->current.proj.z_project.sign[0] = GMT->current.proj.z_project.sign[3] = -1.0;
	GMT->current.proj.z_project.sign[1] = GMT->current.proj.z_project.sign[2] = 1.0;
	GMT->current.proj.z_project.z_axis = (GMT->current.proj.z_project.quadrant%2) ? GMT->current.proj.z_project.quadrant : GMT->current.proj.z_project.quadrant - 2;

	if (GMT->current.proj.z_project.fixed) {
		if (!GMT->current.proj.z_project.world_given) {	/* Pick center point of region */
			GMT->current.proj.z_project.world_x = (gmt_M_is_geographic (GMT, GMT_IN)) ? GMT->current.proj.central_meridian : 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
			GMT->current.proj.z_project.world_y = 0.5 * (GMT->common.R.wesn[YLO] + GMT->common.R.wesn[YHI]);
			GMT->current.proj.z_project.world_z = GMT->current.proj.z_level;
		}
		gmt_geoz_to_xy (GMT, GMT->current.proj.z_project.world_x, GMT->current.proj.z_project.world_y, GMT->current.proj.z_project.world_z, &x, &y);
		if (!GMT->current.proj.z_project.view_given) {	/* Pick center of current page */
			GMT->current.proj.z_project.view_x = 0.5 * GMT->current.setting.ps_page_size[0] * GMT->session.u2u[GMT_PT][GMT_INCH];
			GMT->current.proj.z_project.view_y = 0.5 * GMT->current.setting.ps_page_size[1] * GMT->session.u2u[GMT_PT][GMT_INCH];
		}
		GMT->current.proj.z_project.x_off = GMT->current.proj.z_project.view_x - x;
		GMT->current.proj.z_project.y_off = GMT->current.proj.z_project.view_y - y;
	}
	else {
		GMT->current.proj.z_project.x_off = -GMT->current.proj.z_project.xmin;
		GMT->current.proj.z_project.y_off = -GMT->current.proj.z_project.ymin;
	}

	/* Adjust the xmin/xmax and ymin/ymax because of xoff and yoff */
	GMT->current.proj.z_project.xmin += GMT->current.proj.z_project.x_off;
	GMT->current.proj.z_project.xmax += GMT->current.proj.z_project.x_off;
	GMT->current.proj.z_project.ymin += GMT->current.proj.z_project.y_off;
	GMT->current.proj.z_project.ymax += GMT->current.proj.z_project.y_off;

	return (GMT_NOERROR);
}

/*! . */
GMT_LOCAL double map_geodesic_dist_cos (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE) {
	/* Convenience function to get cosine instead */
	return (cosd (map_geodesic_dist_degree (GMT, lonS, latS, lonE, latE)));
}

/*! . */
GMT_LOCAL double map_great_circle_dist_cos (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Return cosine of great circle distance */

	double sin_half_squared = map_haversine (GMT, lon1, lat1, lon2, lat2);
	return (1.0 - 2.0 * sin_half_squared);	/* Convert sin^2 (half-angle) to cos (angle) */
}

/*! . */
GMT_LOCAL double gmt_cartesian_dist_periodic (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* Calculates the good-old straight line distance in users units */
	double dx = x1 - x0, dy = y1 - y0;
	if (GMT->common.n.periodic[GMT_X] && (dx = fabs (dx)) > GMT->common.n.half_range[GMT_X]) dx = GMT->common.n.range[GMT_X] - dx;
	if (GMT->common.n.periodic[GMT_Y] && (dy = fabs (dy)) > GMT->common.n.half_range[GMT_Y]) dy = GMT->common.n.range[GMT_Y] - dy;
	return (hypot (dx, dy));
}

/*! . */
GMT_LOCAL void map_set_distaz (struct GMT_CTRL *GMT, unsigned int mode, unsigned int type, char *unit_name) {
	/* Assigns pointers to the chosen distance and azimuth functions */
	char *type_name[3] = {"Map", "Contour", "Contour annotation"};
	char *aux[6] = {"no", "authalic", "conformal", "meridional", "geocentric", "parametric"};
	char *rad[5] = {"mean (R_1)", "authalic (R_2)", "volumetric (R_3)", "meridional", "quadratic"};
	int choice = (GMT->current.setting.proj_aux_latitude == GMT_LATSWAP_NONE) ? 0 : 1 + GMT->current.setting.proj_aux_latitude/2;
	GMT->current.map.dist[type].scale = 1.0;	/* Default scale */

	switch (mode) {	/* Set pointers to distance functions */
		case GMT_CARTESIAN_DIST_PERIODIC:	/* Cartesian 2-D x,y data but with one or two periodic dimensions */
			GMT->current.map.dist[type].func = &gmt_cartesian_dist_periodic;
			GMT->current.map.azimuth_func = &map_az_backaz_cartesian;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Cartesian [periodic]\n", type_name[type]);
			break;
		case GMT_CARTESIAN_DIST:	/* Cartesian 2-D x,y data */
			GMT->current.map.dist[type].func = &gmtlib_cartesian_dist;
			GMT->current.map.azimuth_func = &map_az_backaz_cartesian;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Cartesian\n", type_name[type]);
			break;
		case GMT_CARTESIAN_DIST2:	/* Cartesian 2-D x,y data, use r^2 instead of hypot */
			GMT->current.map.dist[type].func = &map_cartesian_dist2;
			GMT->current.map.azimuth_func = &map_az_backaz_cartesian;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Cartesian\n", type_name[type]);
			break;
		case GMT_CARTESIAN_DIST_PROJ:	/* Cartesian distance after projecting 2-D lon,lat data */
			GMT->current.map.dist[type].func = &gmtlib_cartesian_dist_proj;
			GMT->current.map.azimuth_func = &map_az_backaz_cartesian_proj;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Cartesian after first projecting via -J\n", type_name[type]);
			break;
		case GMT_CARTESIAN_DIST_PROJ2:	/* Cartesian distance after projecting 2-D lon,lat data, use r^2 instead of hypot  */
			GMT->current.map.dist[type].func = &map_cartesian_dist_proj2;
			GMT->current.map.azimuth_func = &map_az_backaz_cartesian_proj;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Cartesian after first projecting via -J\n", type_name[type]);
			break;
		case GMT_DIST_M+GMT_FLATEARTH:	/* 2-D lon, lat data, but scale to Cartesian flat earth in meter */
			GMT->current.map.dist[type].func = &map_flatearth_dist_meter;
			GMT->current.map.azimuth_func  = &map_az_backaz_flatearth;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Flat Earth in %s\n", type_name[type], unit_name);
			break;
		case GMT_DIST_M+GMT_GREATCIRCLE:	/* 2-D lon, lat data, use spherical distances in meter */
			GMT->current.map.dist[type].func = &gmt_great_circle_dist_meter;
			GMT->current.map.azimuth_func = &map_az_backaz_sphere;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using great circle approximation with %s auxiliary latitudes and %s radius = %.4f m, in %s.\n",
				type_name[type], aux[choice], rad[GMT->current.setting.proj_mean_radius], GMT->current.proj.mean_radius, unit_name);
			break;
		case GMT_DIST_M+GMT_GEODESIC:	/* 2-D lon, lat data, use geodesic distances in meter */
			GMT->current.map.dist[type].func = GMT->current.map.geodesic_meter;
			GMT->current.map.azimuth_func = GMT->current.map.geodesic_az_backaz;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using %s geodesics in %s\n", type_name[type], GEOD_TEXT[GMT->current.setting.proj_geodesic], unit_name);
			break;
		case GMT_DIST_DEG+GMT_FLATEARTH:	/* 2-D lon, lat data, use Flat Earth distances in degrees */
			GMT->current.map.dist[type].func = map_flatearth_dist_degree;
			GMT->current.map.azimuth_func = &map_az_backaz_flatearth;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be Flat Earth in %s\n", type_name[type], unit_name);
			break;
		case GMT_DIST_DEG+GMT_GREATCIRCLE:	/* 2-D lon, lat data, use spherical distances in degrees */
			GMT->current.map.dist[type].func = &gmtlib_great_circle_dist_degree;
			GMT->current.map.azimuth_func = &map_az_backaz_sphere;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using great circle approximation with %s auxiliary latitudes and return lengths in %s.\n", unit_name,
				type_name[type], aux[choice]);
			break;
		case GMT_DIST_DEG+GMT_GEODESIC:	/* 2-D lon, lat data, use geodesic distances in degrees */
			GMT->current.map.dist[type].func = &map_geodesic_dist_degree;
			GMT->current.map.azimuth_func = GMT->current.map.geodesic_az_backaz;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using geodesics in %s\n", type_name[type], unit_name);
			break;
		case GMT_DIST_COS+GMT_GREATCIRCLE:	/* 2-D lon, lat data, and Green's function needs cosine of spherical distance */
			GMT->current.map.dist[type].func = &map_great_circle_dist_cos;
			GMT->current.map.azimuth_func = &map_az_backaz_sphere;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using great circle approximation with %s auxiliary latitudes and return cosine of spherical angles.\n",
				type_name[type], aux[choice]);
			break;
		case GMT_DIST_COS+GMT_GEODESIC:	/* 2-D lon, lat data, and Green's function needs cosine of geodesic distance */
			GMT->current.map.dist[type].func = &map_geodesic_dist_cos;
			GMT->current.map.azimuth_func = GMT->current.map.geodesic_az_backaz;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be using cosine of geodesic angle\n", type_name[type]);
			break;
		case GMT_DIST_M+GMT_LOXODROME:	/* 2-D lon, lat data, but measure distance along rhumblines in meter */
			GMT->current.map.dist[type].func = &map_loxodrome_dist_meter;
			GMT->current.map.azimuth_func  = &map_az_backaz_loxodrome;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be along loxodromes in meters\n", type_name[type]);
			break;
		case GMT_DIST_DEG+GMT_LOXODROME:	/* 2-D lon, lat data, but measure distance along rhumblines in degrees */
			GMT->current.map.dist[type].func = &map_loxodrome_dist_degree;
			GMT->current.map.azimuth_func = &map_az_backaz_loxodrome;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "%s distance calculation will be along loxodromes with %s auxiliary latitudes and return lengths in degrees.\n",
				type_name[type], aux[choice]);
			break;
		default:	/* Cannot happen unless we make a bug */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Mode (=%d) for distance function is unknown. Must be bug.\n", mode);
			GMT_exit (GMT, GMT_PROJECTION_ERROR);
			break;
	}
	if (type > 0) return;	/* Contour-related assignments end here */

	/* Mapping only */
	if (mode == GMT_CARTESIAN_DIST || mode == GMT_CARTESIAN_DIST2)	{	/* Cartesian data */
		GMT->current.map.near_lines_func   = &map_near_lines_cartesian;
		GMT->current.map.near_a_line_func  = &map_near_a_line_cartesian;
		GMT->current.map.near_point_func   = &map_near_a_point_cartesian;
	}
	else {	/* Geographic data */
		GMT->current.map.near_lines_func   = &map_near_lines_spherical;
		GMT->current.map.near_a_line_func  = &map_near_a_line_spherical;
		GMT->current.map.near_point_func   = &map_near_a_point_spherical;
	}
}


#if 0
void GMT_set_geocentric (struct GMT_CTRL *GMT, bool notify)
{	/* The idea is to call this from sample1d/grdtrack */
	/* Set up ellipsoid parameters for spherical approximation with geocentric parameters */

	GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2O;	/* Geocentric/Geodetic conversion */
	GMT->current.setting.proj_mean_radius = GMT_RADIUS_MERIDIONAL;
	gmtlib_init_ellipsoid (GMT);
}
#endif

/* GMT_dateline_clip simply clips a polygon against the dateline and results in two polygons in L */

/*! . */
void gmtlib_set_oblique_pole_and_origin (struct GMT_CTRL *GMT, double plon, double plat, double olon, double olat) {
	/* The set quantities are used in gmt_obl and gmt_iobl */
	/* Get forward pole and origin vectors FP, FC */
	double P[3];
	gmt_geo_to_cart (GMT, plat, plon, GMT->current.proj.o_FP, true);	/* Set forward Cartesian pole o_FP */
	gmt_geo_to_cart (GMT, olat, olon, P, true);			/* P points to the origin  */
	gmt_cross3v (GMT, GMT->current.proj.o_FP, P, GMT->current.proj.o_FC);	/* Set forward Cartesian center o_FC */
	gmt_normalize3v (GMT, GMT->current.proj.o_FC);

	/* Get inverse pole and origin vectors FP, FC */
	gmt_obl (GMT, 0.0, M_PI_2, &plon, &plat);
	gmt_geo_to_cart (GMT, plat, plon, GMT->current.proj.o_IP, false);	/* Set inverse Cartesian pole o_IP */
	gmt_obl (GMT, 0.0, 0.0, &olon, &olat);
	gmt_geo_to_cart (GMT, olat, olon, P, false);			/* P points to origin  */
	gmt_cross3v (GMT, GMT->current.proj.o_IP, P, GMT->current.proj.o_IC);	/* Set inverse Cartesian center o_FC */
	gmt_normalize3v (GMT, GMT->current.proj.o_IC);
}

/*! . */
bool gmt_cart_outside (struct GMT_CTRL *GMT, double x, double y) {
	/* Expects x,y to be projected and comparing with rectangular projected domain */
	/* Note PW: 8-20-2014: The on_border_is_outside tests had GMT_CONV4_LIMIT instead of GMT_CONV8_LIMIT.  Trying the latter */
	if (GMT->current.map.on_border_is_outside && fabs (x - GMT->current.proj.rect[XLO]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = -1;
	else if (GMT->current.map.on_border_is_outside && fabs (x - GMT->current.proj.rect[XHI]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = 1;
	else if (x < GMT->current.proj.rect[XLO] - GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = -2;
	else if (x > GMT->current.proj.rect[XHI] + GMT_CONV8_LIMIT)
		GMT->current.map.this_x_status = 2;
	else
		GMT->current.map.this_x_status = 0;

	if (GMT->current.map.on_border_is_outside && fabs (y -GMT->current.proj.rect[YLO]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = -1;
	else if (GMT->current.map.on_border_is_outside && fabs (y - GMT->current.proj.rect[YHI]) < GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = 1;
	else if (y < GMT->current.proj.rect[YLO] - GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = -2;
	else if (y > GMT->current.proj.rect[YHI] + GMT_CONV8_LIMIT)
		GMT->current.map.this_y_status = 2;
	else
		GMT->current.map.this_y_status = 0;

	return ((GMT->current.map.this_x_status != 0 || GMT->current.map.this_y_status != 0) ? true : false);
}

/*! . */
uint64_t gmt_clip_to_map (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t np, double **x, double **y) {
	/* This routine makes sure that all points are either inside or on the map boundary
	 * and returns the number of points to be used for plotting (in x,y units) */

	uint64_t i, out, n;
	uint64_t total_nx = 0;
	int64_t out_x, out_y, np2;
	bool polygon;
	double *xx = NULL, *yy = NULL;

	/* First check for trivial cases:  All points outside or all points inside */

	for (i = out = out_x = out_y = 0; i < np; i++)  {
		(void) gmt_map_outside (GMT, lon[i], lat[i]);
		out_x += GMT->current.map.this_x_status;	/* Completely left of west gives -2 * np, right of east gives + 2 * np */
		out_y += GMT->current.map.this_y_status;	/* Completely below south gives -2 * np, above north gives + 2 * np */
		out += (abs (GMT->current.map.this_x_status) == 2 || abs (GMT->current.map.this_y_status) == 2);
	}
	if (out == 0) {		/* All points are inside map boundary; no clipping required */
		gmt_M_malloc2 (GMT, xx, yy, np, NULL, double);
		for (i = 0; i < np; i++) gmt_geo_to_xy (GMT, lon[i], lat[i], &xx[i], &yy[i]);
		*x = xx;	*y = yy;	n = np;
	}
	else if (out == np) {	/* All points are outside map boundary */
		np2 = 2 * np;
		if (int64_abs (out_x) == np2 || int64_abs (out_y) == np2)	/* All points safely outside the region, no part of polygon survives */
			n = 0;
		else {	/* All points are outside, but they are not just to one side so lines _may_ intersect the region */
			n = (*GMT->current.map.clip) (GMT, lon, lat, np, x, y, &total_nx);
			polygon = !gmt_polygon_is_open (GMT, lon, lat, np);	/* The following can only be used on closed polygons */
			/* Polygons that completely contains the -R region will not generate crossings, just duplicate -R box */
			if (polygon && n > 0 && total_nx == 0) {	/* No crossings and all points outside means one of two things: */
				/* Either the polygon contains portions of the -R region including corners or it does not.  We pick the corners and check for insidedness: */
				bool ok = false;
				if (gmt_non_zero_winding (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], lon, lat, np)) ok = true;		/* true if inside */
				if (!ok && gmt_non_zero_winding (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], lon, lat, np)) ok = true;	/* true if inside */
				if (!ok && gmt_non_zero_winding (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YHI], lon, lat, np)) ok = true;	/* true if inside */
				if (!ok && gmt_non_zero_winding (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI], lon, lat, np)) ok = true;	/* true if inside */
				if (!ok) {
					/* Polygon does NOT contain the region and we delete it */
					n = 0;
					gmt_M_free (GMT, *x);
					gmt_M_free (GMT, *y);
				}
				/* Otherwise the polygon completely contains -R and we pass it along */
			}
			else if (GMT->common.R.oblique && GMT->current.proj.projection_GMT == GMT_AZ_EQDIST && n <= 5 && !strncmp (GMT->init.module_name, "pscoast", 7U)) {
				/* Special check for -JE where a coastline block is completely outside yet fully surrounds the rectangular -R -JE...r region.
				   This results in a rectangular closed polygon after the clipping. */
				n = 0;
				gmt_M_free (GMT, *x);
				gmt_M_free (GMT, *y);
			}
		}
	}
	else	/* Mixed case so we must clip the polygon */
		n = (*GMT->current.map.clip) (GMT, lon, lat, np, x, y, &total_nx);

	return (n);
}

/*! . */
unsigned int gmt_split_poly_at_dateline (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_DATASEGMENT ***Lout) {
	int side, j, np, cross = 0;
	unsigned int smode = (S->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
	uint64_t row, m;
	size_t n_alloc = 0;
	char label[GMT_LEN256] = {""}, *part = "EW";
	double xx[2], yy[2];
	struct GMT_DATASEGMENT **L = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	bool (*inside[2]) (double, double);
	bool (*outside[2]) (double, double);


	inside[0] = gmt_inside_upper_boundary;	outside[0] = gmt_outside_upper_boundary;
	inside[1] = gmt_inside_lower_boundary;	outside[1] = gmt_outside_lower_boundary;
	L = gmt_M_memory (GMT, NULL, 2, struct GMT_DATASEGMENT *);	/* The two polygons */

	for (row = 0; row < S->n_rows; row++) gmt_lon_range_adjust (GMT_IS_0_TO_P360_RANGE, &S->data[GMT_X][row]);	/* First enforce 0 <= lon < 360 so we don't have to check again */

	for (side = 0; side < 2; side++) {	/* Do it twice to get two truncated polygons */
		if (S->n_rows == 0) continue;	/* Nothing! */
		n_alloc = lrint (1.05*S->n_rows + 5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
		L[side] = GMT_Alloc_Segment (GMT->parent, smode, n_alloc, S->n_columns, NULL, NULL);
		m = 0;		/* Start with nuthin' */

		/* Must ensure we copy the very first point if it is left of the Dateline */
		if (S->data[GMT_X][0] < 180.0) { L[side]->data[GMT_X][0] = S->data[GMT_X][0]; L[side]->data[GMT_Y][0] = S->data[GMT_Y][0]; }	/* First point is inside; add it */
		for (row = 1; row < S->n_rows; row++) {	/* For each line segment */
			np = map_clip_we (S->data[GMT_X][row-1], S->data[GMT_Y][row-1], S->data[GMT_X][row], S->data[GMT_Y][row], xx, yy, 180.0, inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) L[side] = GMT_Alloc_Segment (GMT->parent, smode, n_alloc << 2, S->n_columns, NULL, L[side]);
				L[side]->data[GMT_X][m] = xx[j]; L[side]->data[GMT_Y][m] = yy[j]; m++;
			}
		}
		if (gmt_polygon_is_open (GMT, L[side]->data[GMT_X], L[side]->data[GMT_Y], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) L[side] = GMT_Alloc_Segment (GMT->parent, smode, n_alloc << 2, S->n_columns, NULL, L[side]);
			L[side]->data[GMT_X][m] = L[side]->data[GMT_X][0];	L[side]->data[GMT_Y][m] = L[side]->data[GMT_Y][0];	m++;	/* Yes. */
		}
		if (m != n_alloc) L[side] = GMT_Alloc_Segment (GMT->parent, smode, m, S->n_columns, NULL, L[side]);
		L[side]->n_rows = m;
		if (S->label) {
			snprintf (label, GMT_LEN256, "%s part %c", S->label, part[side]);
			L[side]->label = strdup (label);
		}
		if (S->header) L[side]->header = strdup (S->header);
		if (SH->ogr) gmt_duplicate_ogr_seg (GMT, L[side], S);
	}
	SH = gmt_get_DS_hidden (L[0]);
	SH->range = GMT_IS_0_TO_P360;
	SH = gmt_get_DS_hidden (L[1]);
	SH->range = GMT_IS_M360_TO_0_RANGE;
	*Lout = L;
	return (2);
}

/*! . */
void gmtlib_get_point_from_r_az (struct GMT_CTRL *GMT, double lon0, double lat0, double r, double azim, double *lon1, double *lat1) {
	/* Given point (lon0, lat0), find coordinates of a point r degrees away in the azim direction */

	double sinr, cosr, sinaz, cosaz, siny, cosy;
	gmt_M_unused(GMT);

	sincosd (azim, &sinaz, &cosaz);
	sincosd (r, &sinr, &cosr);
	sincosd (lat0, &siny, &cosy);

	*lon1 = lon0 + atan2d (sinr * sinaz, (cosy * cosr - siny * sinr * cosaz));
	*lat1 = d_asind (siny * cosr + cosy * sinr * cosaz);
}

/*! . */
double gmt_az_backaz (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz) {
	return (GMT->current.map.azimuth_func (GMT, lonE, latE, lonS, latS, baz));
}

GMT_LOCAL double auto_time_increment (double inc, char *unit) {
	/* Given the first guess interval inc, determine closest time unit and return
	 * the number of seconds in that unit and its code */
	int k, kk = -1;
	double f;
	static char units[6]  = {'S', 'M', 'H', 'D', 'O', 'Y'};
	static double incs[6] = {1.0, GMT_MIN2SEC_F, GMT_HR2SEC_F, GMT_DAY2SEC_F, GMT_MON2SEC_F, GMT_YR2SEC_F};
	for (k = 5; kk == -1 && k >= 0; k--) {	/* Find the largest time unit that is smaller than guess interval */
		f = inc / incs[k];
		if (irint (f) >= 1)
			kk = k;
	}
	if (kk == -1) kk = 0;	/* Safety valve */
	*unit = units[kk];
	return (incs[kk]);
}

/*! . */
void gmt_auto_frame_interval (struct GMT_CTRL *GMT, unsigned int axis, unsigned int item) {
	/* Determine the annotation and frame tick interval when they are not set (interval = 0) */
	int i = 0, n = 6;
	char unit = 's', sunit[2], tmp[GMT_LEN16] = {""}, string[GMT_LEN64] = {""}, par[GMT_LEN128] = {""}, ax_code[4] = "xyz";
	bool set_a = false, interval = false, is_time = gmt_M_axis_is_time (GMT, axis);
	double defmaj[7] = {2.0, 5.0, 10.0, 15.0, 30.0, 60.0, 90.0}, defsub[7] = {1.0, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0};
	double Hmaj[4] = {2.0, 3.0, 6.0, 12.0}, Hsub[4] = {1.0, 1.0, 3.0, 3.0};
	double Omaj[4] = {3.0, 6.0}, Osub[4] = {1.0, 3.0};
	double Dmaj[4] = {2.0, 3.0, 7.0, 14.0}, Dsub[4] = {1.0, 1.0, 1.0, 7.0};
	double d, f, p, *maj = defmaj, *sub = defsub;
	struct GMT_PLOT_AXIS *A = &GMT->current.map.frame.axis[axis];
	struct GMT_PLOT_AXIS_ITEM *T;

	if (A->special == GMT_CUSTOM) return;	/* Got custom annotation/tick information via user file */
	if (A->type == GMT_LOG10 || A->type == GMT_POW) return;	/* These axes still needs to have automatic selections implemented */

	if (!(A->item[item].active && A->item[item].interval == 0.0) &&
		!(A->item[item+2].active && A->item[item+2].interval == 0.0) &&
		!(A->item[item+4].active && A->item[item+4].interval == 0.0)) return;

	/* f = frame width/height (inch); d = domain width/height (world coordinates) */
	if (axis == GMT_X) {
		f = fabs (GMT->current.proj.rect[XHI] - GMT->current.proj.rect[XLO]);
		d = fabs (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]);
	}
	else if (axis == GMT_Y) {
		f = fabs (GMT->current.proj.rect[YHI] - GMT->current.proj.rect[YLO]);
		d = fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]);
	}
	else {
		f = fabs (GMT->current.proj.zmax - GMT->current.proj.zmin);
		d = fabs (GMT->common.R.wesn[ZHI] - GMT->common.R.wesn[ZLO]);
	}
	f *= GMT->session.u2u[GMT_INCH][GMT_PT];	/* Change to points */

	/* First guess of interval */
	d *= MAX (0.05, MIN (5.0 * GMT->current.setting.font_annot[item].size / f, 0.20));

	/* Now determine 'round' major and minor tick intervals */
	if (gmt_M_axis_is_geo (GMT, axis))	/* Geographical coordinate */
		p = (d < GMT_MIN2DEG) ? GMT_SEC2DEG : (d < 1.0) ? GMT_MIN2DEG : 1.0;
	else if (is_time)	/* Time axis coordinate, get p in seconds and the unit it represents */
		p = auto_time_increment (d, &unit);
	else	/* General (linear) axis */
		p = pow (10.0, floor (log10 (d)));
	d /= p;	/* d is now in degrees, minutes or seconds (or time seconds), or in the range [1;10) */
	if (is_time) {	/* p was in seconds but we will use unit, so reset p = 1 */
		p = 1.0;
		switch (unit) {	/* Select other steps more suitable for month, day, hour.  */
			case 'O': maj = Omaj; sub = Osub; n = 1; break;
			case 'D': maj = Dmaj; sub = Dsub; n = 3; break;
			case 'H': maj = Hmaj; sub = Hsub; n = 3;
				break;
		}
		if ((unit == 'H' || unit == 'M') && !strcmp (GMT->current.setting.format_clock_map, "hh:mm:ss")) {
			/* Strip off the seconds from the formatting if default setting is used */
			strcpy (GMT->current.setting.format_clock_map, "hh:mm");
			gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_map, &GMT->current.plot.calclock.clock, 2);
			sprintf (par, " --FORMAT_CLOCK_MAP=hh:mm");
		}
		else if (unit == 'D' && !strcmp (GMT->current.setting.format_date_map, "yyyy-mm-dd")) {
			/* Use month name and day */
			strcpy (GMT->current.setting.format_date_map, "o dd");
			gmtlib_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
			sprintf (par, " --FORMAT_DATE_MAP=\"o dd\"");
		}
		else if (unit == 'O' && !strcmp (GMT->current.setting.format_date_map, "yyyy-mm-dd")) {
			/* Use year and month name for month intervals */
			strcpy (GMT->current.setting.format_date_map, "o yyyy");
			gmtlib_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
			sprintf (par, " --FORMAT_DATE_MAP=\"o yyyy\"");
		}

		interval = (unit == 'Y' || unit == 'O' || unit == 'D');
	}
	while (i < n && maj[i] < d) i++;	/* Wind up to largest reasonable interval */
	d = maj[i] * p, f = sub[i] * p;		/* Scale up intervals in multiple of unit */
	if (is_time) {	/* Last check to change a 12 month unit to 1 year and 24 hours to 1 day */
		if (unit == 'O' && d == 12.0) d = 1.0, f /= 12.0, unit = 'Y';
		if (unit == 'H' && d == 24.0) d = 1.0, f /= 24.0, unit = 'D';
		sunit[0] = unit;	/* Since we need a string in strcat */
	}

	/* Set annotation/major tick interval */
	T = &A->item[item];
	if (T->active && T->interval == 0.0) {
		T->interval = d, T->generated = set_a = true;
		snprintf (tmp, GMT_LEN16, "a%g", T->interval); strcat (string, tmp);
		if (is_time) T->unit = unit, strcat (string, sunit);
		if (interval) T->type = 'i', T->flavor = 1;
	}

	/* Set minor ticks as well (if copied from annotation, set to major interval) */
	T = &A->item[item+2];
	if (T->active && T->interval == 0.0) {
		T->interval = (T->type == 'f' || T->type == 'F') ? f : d, T->generated = true;
		snprintf (tmp, GMT_LEN16, "f%g", T->interval); strcat (string, tmp);
		if (is_time) T->unit = unit, strcat (string, sunit);
	}

	/* Finally set grid interval (if annotation set as well, use major, otherwise minor interval) */
	T = &A->item[item+4];
	if (T->active && T->interval == 0.0) {
		T->interval = set_a ? d : f, T->generated = true;
		snprintf (tmp, GMT_LEN16, "g%g", T->interval); strcat (string, tmp);
		if (is_time) T->unit = unit, strcat (string, sunit);
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Auto-frame interval for axis %d item %d: d = %g  f = %g\n", axis, item, d, f);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Auto-frame interval for %c-axis (item %d): %s%s\n", ax_code[axis], item, string, par);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1 :	M A P  - T R A N S F O R M A T I O N S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/*
 * gmt_map_setup sets up the transformations for the chosen map projection.
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
 * CYLINDRICAL EQUAL-AREA projections (Behrmann, Gall):
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
 * are forward and inverse, and are called from gmt_geo_to_xy and gmt_xy_to_geo.
 *
 */

/*
 *	GENERIC TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION
 */

/*! . */
double gmt_x_to_xx (struct GMT_CTRL *GMT, double x) {
	/* Converts x to xx using the current linear projection */
	double xx;
	(*GMT->current.proj.fwd_x) (GMT, x, &xx);
	return (xx * GMT->current.proj.scale[GMT_X] + GMT->current.proj.origin[GMT_X]);
}

/*! . */
double gmt_y_to_yy (struct GMT_CTRL *GMT, double y) {
	/* Converts y to yy using the current linear projection */
	double yy;
	(*GMT->current.proj.fwd_y) (GMT, y, &yy);
	return (yy * GMT->current.proj.scale[GMT_Y] + GMT->current.proj.origin[GMT_Y]);
}

/*! . */
double gmt_z_to_zz (struct GMT_CTRL *GMT, double z) {
	/* Converts z to zz using the current linear projection */
	double zz;
	(*GMT->current.proj.fwd_z) (GMT, z, &zz);
	return (zz * GMT->current.proj.scale[GMT_Z] + GMT->current.proj.origin[GMT_Z]);
}

/*! . */
bool gmt_geo_to_xy (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y) {
	/* Converts lon/lat to x/y using the current projection */
	if (gmt_M_is_dnan (lon) || gmt_M_is_dnan (lat)) {(*x) = (*y) = GMT->session.d_NaN; return true;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT->current.proj.fwd) (GMT, lon, lat, x, y);
	(*x) = (*x) * GMT->current.proj.scale[GMT_X] + GMT->current.proj.origin[GMT_X];
	(*y) = (*y) * GMT->current.proj.scale[GMT_Y] + GMT->current.proj.origin[GMT_Y];
	return false;
}

/*! . */
bool gmt_geo_to_xy_noshift (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y) {
	/* Converts lon/lat to x/y using the current projection but applies no shift */
	if (gmt_M_is_dnan (lon) || gmt_M_is_dnan (lat)) {(*x) = (*y) = GMT->session.d_NaN; return true;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT->current.proj.fwd) (GMT, lon, lat, x, y);
	(*x) = (*x) * GMT->current.proj.scale[GMT_X];
	(*y) = (*y) * GMT->current.proj.scale[GMT_Y];
	return false;
}

/*! . */
bool gmt_geo_to_xy_noshiftscale (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y) {
	/* Converts lon/lat to x/y using the current projection but applies no shift */
	if (gmt_M_is_dnan (lon) || gmt_M_is_dnan (lat)) {(*x) = (*y) = GMT->session.d_NaN; return true;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT->current.proj.fwd) (GMT, lon, lat, x, y);
	return false;
}

/*! . */
void gmt_xy_to_geo (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y) {
	/* Converts x/y to lon/lat using the current projection */

	if (gmt_M_is_dnan (x) || gmt_M_is_dnan (y)) {(*lon) = (*lat) = GMT->session.d_NaN; return;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	x = (x - GMT->current.proj.origin[GMT_X]) * GMT->current.proj.i_scale[GMT_X];
	y = (y - GMT->current.proj.origin[GMT_Y]) * GMT->current.proj.i_scale[GMT_Y];

	(*GMT->current.proj.inv) (GMT, lon, lat, x, y);
}

/*! . */
void gmt_xy_to_geo_noshiftscale (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y) {
	/* Converts x/y to lon/lat using the current projection but applies no shift */

	if (gmt_M_is_dnan (x) || gmt_M_is_dnan (y)) {(*lon) = (*lat) = GMT->session.d_NaN; return;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT->current.proj.inv) (GMT, lon, lat, x, y);
}

/*! . */
void gmt_xy_to_geo_noshift (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y) {
	/* Converts x/y to lon/lat using the current projection but applies no shift */

	if (gmt_M_is_dnan (x) || gmt_M_is_dnan (y)) {(*lon) = (*lat) = GMT->session.d_NaN; return;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*GMT->current.proj.inv) (GMT, lon, lat, x, y);
}

/*! . */
void gmt_geoz_to_xy (struct GMT_CTRL *GMT, double x, double y, double z, double *x_out, double *y_out) {
	/* Map-projects xy first, the projects xyz onto xy plane */
	double x0, y0;
	gmt_geo_to_xy (GMT, x, y, &x0, &y0);
	gmt_xyz_to_xy (GMT, x0, y0, gmt_z_to_zz (GMT, z), x_out, y_out);
}

/*! . */
void gmt_xyz_to_xy (struct GMT_CTRL *GMT, double x, double y, double z, double *x_out, double *y_out) {
	/* projects xyz (inches) onto perspective xy plane (inches) */
	*x_out = - x * GMT->current.proj.z_project.cos_az + y * GMT->current.proj.z_project.sin_az + GMT->current.proj.z_project.x_off;
	*y_out = - (x * GMT->current.proj.z_project.sin_az + y * GMT->current.proj.z_project.cos_az) * GMT->current.proj.z_project.sin_el + z * GMT->current.proj.z_project.cos_el + GMT->current.proj.z_project.y_off;
}


/*! Setting w/e/s/n for a fully qualified UTM zone */

bool gmt_UTMzone_to_wesn (struct GMT_CTRL *GMT, unsigned int zone_x, char zone_y, int hemi, double wesn[]) {
	/* Given the full UTM zone specification, return w/e/s/n */

	bool error = false;
	gmt_M_unused(GMT);

	wesn[XHI] = -180.0 + 6.0 * zone_x;	wesn[XLO] = wesn[XHI] - 6.0;

	if (zone_y == 0) {	/* Latitude zone is not specified */
		if (hemi == -1) {
			wesn[YLO] = -80.0;	wesn[YHI] = 0.0;
		}
		else if (hemi == +1) {
			wesn[YLO] = 0.0;	wesn[YHI] = 84.0;
		}
		else
			error = true;
		return (error);
	}
	else if (zone_y < 'A' || zone_y > 'Z')
		error = true;
	else if (zone_y <= 'B') {
		wesn[YLO] = -90.0;	wesn[YHI] = -80.0;
		wesn[XHI] = 180.0 * (zone_y - 'A');
		wesn[XLO] = wesn[XHI] - 180.0;
	}
	else if (zone_y <= 'I') {	/* I will behave as J */
		wesn[YLO] = -80.0 + 8.0 * (zone_y - 'C');	wesn[YHI] = wesn[YLO] + 8.0;
	}
	else if (zone_y <= 'O') {	/* O will behave as P */
		wesn[YLO] = -80.0 + 8.0 * (zone_y - 'D');	wesn[YHI] = wesn[YLO] + 8.0;
	}
	else if (zone_y <= 'W') {
		wesn[YLO] = -80.0 + 8.0 * (zone_y - 'E');	wesn[YHI] = wesn[YLO] + 8.0;
		if (zone_y == 'V' && zone_x == 31) wesn[XHI] = 3.0;
		if (zone_y == 'V' && zone_x == 32) wesn[XLO] = 3.0;
	}
	else if (zone_y == 'X') {
		wesn[YLO] = 72.0;	wesn[YHI] = 84.0;
		if (zone_x == 31) wesn[XHI] = 9.0;
		if (zone_x == 33) {wesn[XLO] = 9.0; wesn[XHI] = 21.0;}
		if (zone_x == 35) {wesn[XLO] = 21.0; wesn[XHI] = 33.0;}
		if (zone_x == 37) wesn[XLO] = 33.0;
		if (zone_x == 32 || zone_x == 34 || zone_x == 36) error = true;
	}
	else {	/* Y or Z */
		wesn[YLO] = 84.0;	wesn[YHI] = 90.0;
		wesn[XHI] = 180.0 * (zone_y - 'Y');
		wesn[XLO] = wesn[XHI] - 180.0;
	}

	return (error);
}


/*! . */
bool gmt_genper_reset (struct GMT_CTRL *GMT, bool reset) {
	/* Switch between the windowed functions for lines etc and the rect ones for annotations.
	 * Because of issue # 667 it became clear that we had problems with gridlines when the
	 * projection became windowed, i.e., view is a clipped circle with both straight and
	 * curved boundaries.  I added several new functions to deal with overlaps, crossings,
	 * left and right boundary x-value, etc (all called gmt_genper_*) but they yield way too
	 * many annotations (probably due to some confusion) so until that might be looked into I
	 * have kept the older, simpler functions and switch from those to the new ones when we
	 * do gridlines.  P. Wessel, Feb 10, 2015.
	 */
	if (GMT->current.proj.projection_GMT == GMT_GENPER && GMT->current.proj.windowed) {
		if (reset) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Revert to old genper crossing/overlap functions\n");
			GMT->current.map.crossing = &map_rect_crossing;
			GMT->current.map.overlap = &map_rect_overlap;
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Switch to new genper crossing/overlap functions\n");
			GMT->current.map.crossing = &map_genper_crossing;
			GMT->current.map.overlap = &map_genperw_overlap;
		}
		return true;
	}
	return false;
}

/*! . */
bool gmt_map_outside (struct GMT_CTRL *GMT, double lon, double lat) {
	/* Save current status in previous status and update current in/out status */
	GMT->current.map.prev_x_status = GMT->current.map.this_x_status;
	GMT->current.map.prev_y_status = GMT->current.map.this_y_status;
	if (GMT->current.map.outside == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_map_outside: FATAL ERROR - the pointer to the projection function is NULL.\n");
		return -1;
	}
	return ((*GMT->current.map.outside) (GMT, lon, lat));
}

/*! . */
double gmt_half_map_width (struct GMT_CTRL *GMT, double y) {
	/* Returns 1/2-width of map in inches given y value */
	double half_width;

	switch (GMT->current.proj.projection_GMT) {

		case GMT_STEREO:	/* Must compute width of circular map based on y value (ASSUMES FULL CIRCLE!!!) */
		case GMT_LAMB_AZ_EQ:
		case GMT_GNOMONIC:
		case GMT_AZ_EQDIST:
		case GMT_VANGRINTEN:
			if (!GMT->common.R.oblique && GMT->current.map.is_world) {
				y -= GMT->current.proj.r;
				half_width = d_sqrt (GMT->current.proj.r * GMT->current.proj.r - y * y);
			}
			else
				half_width = GMT->current.map.half_width;
			break;

		case GMT_ORTHO:
		case GMT_GENPER:
			if (!GMT->common.R.oblique && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) {
				y -= GMT->current.proj.r;
				half_width = d_sqrt (GMT->current.proj.r * GMT->current.proj.r - y * y);
			}
			else
				half_width = GMT->current.map.half_width;
			break;
		case GMT_MOLLWEIDE:		/* Must compute width of Mollweide map based on y value */
		case GMT_HAMMER:
		case GMT_WINKEL:
		case GMT_SINUSOIDAL:
		case GMT_ROBINSON:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
			if (!GMT->common.R.oblique && GMT->current.map.is_world)
				half_width = gmtmap_right_boundary (GMT, y) - GMT->current.map.half_width;
			else
				half_width = GMT->current.map.half_width;
			break;

		default:	/* Rectangular maps are easy */
			half_width = GMT->current.map.half_width;
			break;
	}
	return (half_width);
}

/*! . */
uint64_t gmt_map_truncate (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t start, int side) {
	/* Truncates a wrapping polygon against left or right edge.
	   (bottom or top edge when projection is TM)
	   x, y : arrays of plot coordinates
	   n    : length of the arrays
	   start: first point of array to consider
	   side : -1 = left (bottom); +1 = right (top)
	*/

	if (GMT->current.proj.projection_GMT == GMT_TM)
		return (map_truncate_tm (GMT, x, y, n, start, side));
	else
		return (map_truncate_x (GMT, x, y, n, start, side));
}

/* GMT generic distance calculations between a pair of points in 2-D */

/*! . */
double gmt_distance_type (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE, unsigned int id) {
	/* Generic function available to programs for contour/label distance calculations */
	return (GMT->current.map.dist[id].scale * GMT->current.map.dist[id].func (GMT, lonS, latS, lonE, latE));
}

/*! . */
double gmt_distance (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE) {
	/* Generic function available to programs */
	return (gmt_distance_type (GMT, lonS, latS, lonE, latE, 0));
}

/*! . */
bool gmt_near_a_point (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, double dist) {
	/* Compute distance to nearest point in T from (lon, lat) */
	return (GMT->current.map.near_point_func (GMT, lon, lat, T, dist));
}

/*! . */
bool gmt_near_lines (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	/* Compute distance to nearest line in T from (lon,lat) */
	return (GMT->current.map.near_lines_func (GMT, lon, lat, T, return_mindist, dist_min, x_near, y_near));
}

/*! . */
bool gmt_near_a_line (struct GMT_CTRL *GMT, double lon, double lat, uint64_t seg, struct GMT_DATASEGMENT *S, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near) {
	/* Compute distance to the line S from (lon,lat) */
	return (GMT->current.map.near_a_line_func (GMT, lon, lat, seg, S, return_mindist, dist_min, x_near, y_near));
}

/* Specific functions that are accessed via pointer only */

/*! . */
double gmtlib_cartesian_dist (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1) {
	/* Calculates the good-old straight line distance in users units */
	gmt_M_unused(GMT);
	return (hypot ( (x1 - x0), (y1 - y0)));
}

/*! . */
double gmtlib_cartesian_dist_proj (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Calculates the good-old straight line distance after first projecting the data */
	double x0, y0, x1, y1;
	gmt_geo_to_xy (GMT, lon1, lat1, &x0, &y0);
	gmt_geo_to_xy (GMT, lon2, lat2, &x1, &y1);
	return (hypot ( (x1 - x0), (y1 - y0)));
}

/*! . */
double gmtlib_great_circle_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Great circle distance on a sphere in degrees */

	double sin_half_squared = map_haversine (GMT, lon1, lat1, lon2, lat2);
	return (2.0 * d_asind (d_sqrt (sin_half_squared)));
}

/*! . */
double gmt_great_circle_dist_meter (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2) {
	/* Calculates the great circle distance in meter */
	return (gmtlib_great_circle_dist_degree (GMT, lon1, lat1, lon2, lat2) * GMT->current.proj.DIST_M_PR_DEG);
}

/* Functions dealing with distance between points */

/*! . */
double gmt_mindist_to_point (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, uint64_t *id) {
	uint64_t row, seg;
	double d, d_min;

	d_min = DBL_MAX;
	for (seg = 0; seg < T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			d = gmt_distance (GMT, lon, lat, T->segment[seg]->data[GMT_X][row], T->segment[seg]->data[GMT_Y][row]);
			if (d < d_min) {	/* Update the shortest distance and the point responsible */
				d_min = d;	id[0] = seg;	id[1] = row;
			}
		}
	}
	return (d_min);
}

int gmtlib_small_circle_intersection (struct GMT_CTRL *GMT, double Ax, double Ay, double Ar, double Bx, double By, double Br, double Xx[2], double Yy[2]) {
	/* Let (Ax,Ay) and (Bx,By) be the poles of two small circles, each with radius Ar and Br, respectively, in degrees.
	 * Pass out the 0-2 intersections via Xx, Yx and return the number of intersections 0-2.
	 * Modified from http://gis.stackexchange.com/questions/48937/how-to-calculate-the-intersection-of-2-circles
	 */
	unsigned int k;
	double A[3], B[3], X[3], N[3], P[3], t, a, b, L_X2, L_N2, cos_AB, AB, cos_Ar, cos_Br, cos_AB2;

	gmt_geo_to_cart (GMT, Ay, Ax, A, true);	/* Get pole vector A */
	gmt_geo_to_cart (GMT, By, Bx, B, true);	/* Get pole vector B */
	cos_AB = gmt_dot3v (GMT, A, B);		/* Cos of spherical distance between A and B */
	AB = d_acosd (cos_AB);			/* Distance between the two poles in degrees */
	if ((Ar + Br) < AB) return 0;		/* No intersection possible */
	if (doubleAlmostEqual (Ar + Br, AB)) {	/* Tangent point lines along great circle form A to B */
		t = Ar / AB;	/* Fractional distance of X from A */
		for (k = 0; k < 3; k++) X[k] = A[k] * (1.0 - t) + B[k] * t;
		gmt_normalize3v (GMT, X);	/* Make sure X has unit length */
		gmt_cart_to_geo (GMT, &Yy[0], &Xx[0], X, true);	/* Recover lon,lat of tangent point */
		return 1;
	}
	/* Here we have two intersections */
	cos_AB2 = cos_AB * cos_AB;
	cos_Ar = cosd (Ar);	cos_Br = cosd (Br);
	a = (cos_Ar - cos_Br * cos_AB) / (1.0 - cos_AB2);
	b = (cos_Br - cos_Ar * cos_AB) / (1.0 - cos_AB2);
	for (k = 0; k < 3; k++) X[k] = a * A[k] + b * B[k];
	L_X2 = gmt_dot3v (GMT, X, X);	/* Length^2 of X */
	gmt_cross3v (GMT, A, B, N);	/* Get pole position of plane through A and B (and origin O) */
	L_N2 = gmt_dot3v (GMT, N, N);	/* Length^2 of N */
	t = sqrt ((1.0 - L_X2) / L_N2);
	for (k = 0; k < 3; k++) P[k] = X[k] + t * N[k];
	gmt_normalize3v (GMT, P);	/* Make sure P has unit length */
	gmt_cart_to_geo (GMT, &Yy[0], &Xx[0], P, true);		/* Recover lon,lat of 1st intersection point */
	for (k = 0; k < 3; k++) P[k] = X[k] - t * N[k];
	gmt_normalize3v (GMT, P);	/* Make sure P has unit length */
	gmt_cart_to_geo (GMT, &Yy[1], &Xx[1], P, true);		/* Recover lon,lat of 2nd intersection point */
	return 2;
}

/*! . */
int gmtlib_great_circle_intersection (struct GMT_CTRL *GMT, double A[], double B[], double C[], double X[], double *CX_dist) {
	/* A, B, C are 3-D Cartesian unit vectors, i.e., points on the sphere.
	 * Let points A and B define a great circle, and consider a
	 * third point C.  A second great circle goes through C and
	 * is orthogonal to the first great circle.  Their intersection
	 * X is the point on (A,B) closest to C.  We must test if X is
	 * between A,B or outside.
	 */
	unsigned int i;
	double P[3], E[3], M[3], Xneg[3], cos_AB, cos_MX1, cos_MX2, cos_test;

	gmt_cross3v (GMT, A, B, P);			/* Get pole position of plane through A and B (and origin O) */
	gmt_normalize3v (GMT, P);			/* Make sure P has unit length */
	gmt_cross3v (GMT, C, P, E);			/* Get pole E to plane through C (and origin) but normal to A,B (hence going through P) */
	gmt_normalize3v (GMT, E);			/* Make sure E has unit length */
	gmt_cross3v (GMT, P, E, X);			/* Intersection between the two planes is oriented line*/
	gmt_normalize3v (GMT, X);			/* Make sure X has unit length */
	/* The X we want could be +x or -X; must determine which might be closest to A-B midpoint M */
	for (i = 0; i < 3; i++) {
		M[i] = A[i] + B[i];
		Xneg[i] = -X[i];
	}
	gmt_normalize3v (GMT, M);			/* Make sure M has unit length */
	/* Must first check if X is along the (A,B) segment and not on its extension */

	cos_MX1 = gmt_dot3v (GMT, M, X);		/* Cos of spherical distance between M and +X */
	cos_MX2 = gmt_dot3v (GMT, M, Xneg);	/* Cos of spherical distance between M and -X */
	if (cos_MX2 > cos_MX1) gmt_M_memcpy (X, Xneg, 3, double);		/* -X is closest to A-B midpoint */
	cos_AB = fabs (gmt_dot3v (GMT, A, B));	/* Cos of spherical distance between A,B */
	cos_test = fabs (gmt_dot3v (GMT, A, X));	/* Cos of spherical distance between A and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to A exceeds the A-B length */
	cos_test = fabs (gmt_dot3v (GMT, B, X));	/* Cos of spherical distance between B and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to B exceeds the A-B length */

	/* X is between A and B.  Now calculate distance between C and X */

	*CX_dist = gmt_dot3v (GMT, C, X);		/* Cos of spherical distance between C and X */
	return (0);				/* Return zero if intersection is between A and B */
}

/*! . */
uint64_t *gmtlib_split_line (struct GMT_CTRL *GMT, double **xx, double **yy, uint64_t *nn, bool add_crossings) {
	/* Accepts x/y array for a line in projected inches and looks for
	 * map jumps.  If found it will insert the boundary crossing points and
	 * build a split integer array with the nodes of the first point
	 * for each segment.  The number of segments is returned.  If
	 * no jumps are found then NULL is returned.  This function is needed
	 * so that the new PS contouring machinery only sees lines that do no
	 * jump across the map.
	 * add_crossings is true if we need to find the crossings; false means
	 * they are already part of the line. */

	uint64_t i, j, jc, k, n, n_seg, *split = NULL, *pos = NULL;
	size_t n_alloc = 0;
 	int l_or_r;
 	char *way = NULL;
	double *x = NULL, *y = NULL, *xin = NULL, *yin = NULL, xc[2], yc[2];

	/* First quick scan to see how many jumps there are */

	xin = *xx;	yin = *yy;
	gmt_set_meminc (GMT, GMT_SMALL_CHUNK);
	for (n_seg = 0, i = 1; i < *nn; i++) {
		if ((l_or_r = map_jump_xy (GMT, xin[i], yin[i], xin[i-1], yin[i-1]))) {
			if (n_seg == n_alloc) {
				pos = gmt_M_malloc (GMT, pos, n_seg, &n_alloc, uint64_t);
				n_alloc = n_seg;
				way = gmt_M_malloc (GMT, way, n_seg, &n_alloc, char);
			}
			pos[n_seg] = i;		/* 2nd of the two points that generate the jump */
			way[n_seg] = (char)l_or_r;	/* Which way we jump : +1 is right to left, -1 is left to right, +2 is top to bottom, -2 is bottom to top */
			n_seg++;
		}
	}
	gmt_reset_meminc (GMT);

	if (n_seg == 0) return (NULL);	/* No jumps, just return NULL */

	/* Here we have one or more jumps so we need to split the line */

	n = *nn;				/* Original line count */
	if (add_crossings) n += 2 * n_seg;	/* Must add 2 crossing points per jump */
	n_alloc = 0;
	gmt_M_malloc2 (GMT, x, y, n, &n_alloc, double);
	split = gmt_M_memory (GMT, NULL, n_seg+2, uint64_t);
	split[0] = n_seg;	/* Number of segments we will need */

	x[0] = xin[0];	y[0] = yin[0];
	for (i = j = 1, k = 0; i < *nn; i++, j++) {
		if (k < n_seg && i == pos[k]) {	/* At jump point */
			if (add_crossings) {	/* Find and insert the crossings */
				if (way[k] == -2 || way[k] == +2) {	/* Jump in y for TM */
					map_get_crossings_y (GMT, xc, yc, xin[i], yin[i], xin[i-1], yin[i-1]);
					if (way[k] == -2) {	/* Add top border point first */
						gmt_M_double_swap (xc[0], xc[1]);
						gmt_M_double_swap (yc[0], yc[1]);
					}
				}
				else {	/* Jump in x */
					map_get_crossings_x (GMT, xc, yc, xin[i], yin[i], xin[i-1], yin[i-1]);
					if (way[k] == 1) {	/* Add right border point first */
						gmt_M_double_swap (xc[0], xc[1]);
						gmt_M_double_swap (yc[0], yc[1]);
					}
				}
				x[j] = xc[0];	y[j++] = yc[0];	/* End of one segment */
				jc = j;		/* Node of first point in next segment */
				x[j] = xc[1];	y[j++] = yc[1];	/* Start of another */
			}
			else
				jc = j;
			split[++k] = jc;		/* Node of first point in new segment */
		}
		/* Then copy the regular points */
		x[j] = xin[i];	y[j] = yin[i];
	}
	split[++k] = j;		/* End of last segment */

	/* Time to return the pointers to new data */

	gmt_M_free (GMT, pos);
	gmt_M_free (GMT, way);
	gmt_M_free (GMT, xin);
	gmt_M_free (GMT, yin);
	*xx = x;
	*yy = y;
	*nn = j;

	return (split);
}

/*  Routines to add pieces of parallels or meridians */

/*! . */
uint64_t gmt_graticule_path (struct GMT_CTRL *GMT, double **x, double **y, int dir, bool check, double w, double e, double s, double n) {
	/* Returns the path of a graticule (box of meridians and parallels).
	 * dir determines if we go counter-clockwise or clockwise.
	 * check, if true, forces a straddle check at the end for geographic data.
	*/
	size_t n_alloc = 0;
	uint64_t np = 0;
	double *xx = NULL, *yy = NULL;
	double px0, px1, px2, px3;

	if (dir == 1) {	/* Forward sense */
		px0 = px3 = w;	px1 = px2 = e;
	}
	else {	/* Reverse sense */
		px0 = px3 = e;	px1 = px2 = w;
	}

	/* Close graticule from point 0 through point 4 */

	if (gmt_M_is_rect_graticule(GMT)) {	/* Simple rectangle in this projection */
		gmt_M_malloc2 (GMT, xx, yy, 5U, NULL, double);
		xx[0] = xx[4] = px0;	xx[1] = px1;	xx[2] = px2;	xx[3] = px3;
		yy[0] = yy[1] = yy[4] = s;	yy[2] = yy[3] = n;
		np = 5U;
	}
	else {	/* Must assemble path from meridians and parallel pieces */
		double *xtmp = NULL, *ytmp = NULL;
		size_t add;
		uint64_t k;

		/* SOUTH BORDER */

		if (gmt_M_is_geographic (GMT, GMT_IN) && s == -90.0 && gmt_M_pole_is_point(GMT)) {	/* No path, just a point */
			gmt_M_malloc2 (GMT, xx, yy, 1U, &n_alloc, double);
			xx[0] = px1;	yy[0] = -90.0;
		}
		else
			n_alloc = gmtlib_latpath (GMT, s, px0, px1, &xx, &yy);	/* South */
		np = n_alloc;

		/* EAST (OR WEST) BORDER */

		add = gmtlib_lonpath (GMT, px1, s, n, &xtmp, &ytmp);	/* east (or west if dir == -1) */
		k = n_alloc + add;
		gmt_M_malloc2 (GMT, xx, yy, k, &n_alloc, double);
		gmt_M_memcpy (&xx[np], xtmp, add, double);
		gmt_M_memcpy (&yy[np], ytmp, add, double);
		np += add;
		gmt_M_free (GMT, xtmp);	gmt_M_free (GMT, ytmp);

		/* NORTH BORDER */

		if (gmt_M_is_geographic (GMT, GMT_IN) && n == 90.0 && gmt_M_pole_is_point(GMT)) {	/* No path, just a point */
			add = 0;
			gmt_M_malloc2 (GMT, xtmp, ytmp, 1U, &add, double);
			xtmp[0] = px3;	ytmp[0] = +90.0;
		}
		else
			add = gmtlib_latpath (GMT, n, px2, px3, &xtmp, &ytmp);	/* North */

		k = n_alloc + add;
		gmt_M_malloc2 (GMT, xx, yy, k, &n_alloc, double);
		gmt_M_memcpy (&xx[np], xtmp, add, double);
		gmt_M_memcpy (&yy[np], ytmp, add, double);
		np += add;
		gmt_M_free (GMT, xtmp);	gmt_M_free (GMT, ytmp);

		/* WEST (OR EAST) BORDER */

		add = gmtlib_lonpath (GMT, px3, n, s, &xtmp, &ytmp);	/* west */
		k = n_alloc + add;
		gmt_M_malloc2 (GMT, xx, yy, k, &n_alloc, double);
		gmt_M_memcpy (&xx[np], xtmp, add, double);
		gmt_M_memcpy (&yy[np], ytmp, add, double);
		np += add;
		n_alloc = np;
		gmt_M_free (GMT, xtmp);	gmt_M_free (GMT, ytmp);
		gmt_M_malloc2 (GMT, xx, yy, 0, &n_alloc, double);
	}

	if (check && gmt_M_x_is_lon (GMT, GMT_IN)) {
		bool straddle;
		uint64_t i;
		straddle = (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] > 0.0);
		for (i = 0; straddle && i < np; i++) {
			while (xx[i] < 0.0) xx[i] += 360.0;
			if (straddle && xx[i] > 180.0) xx[i] -= 360.0;
		}
	}

	*x = xx;
	*y = yy;
	return (np);
}

/*! . */
uint64_t gmtlib_map_path (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double **x, double **y) {
	if (doubleAlmostEqualZero (lat1, lat2))
		return (gmtlib_latpath (GMT, lat1, lon1, lon2, x, y));
	else
		return (gmtlib_lonpath (GMT, lon1, lat1, lat2, x, y));
}

/*! . */
uint64_t gmtlib_lonpath (struct GMT_CTRL *GMT, double lon, double lat1, double lat2, double **x, double **y) {
	size_t n_alloc = 0;
	uint64_t n, k;
	int n_try, pos;
	bool keep_trying;
	double dlat, dlat0, *tlon = NULL, *tlat = NULL, x0, x1, y0, y1, d, min_gap;

	if (GMT->current.map.meridian_straight == 2) {	/* Special non-sampling for gmtselect/grdlandmask */
		gmt_M_malloc2 (GMT, tlon, tlat, 2U, NULL, double);
		tlon[0] = tlon[1] = lon;
		tlat[0] = lat1;	tlat[1] = lat2;
		*x = tlon;
		*y = tlat;
		return (2ULL);
	}

	if (GMT->current.map.meridian_straight) {	/* Easy, just a straight line connect via quarter-points */
		gmt_M_malloc2 (GMT, tlon, tlat, 5, &n_alloc, double);
		tlon[0] = tlon[1] = tlon[2] = tlon[3] = tlon[4] = lon;
		dlat = lat2 - lat1;
		tlat[0] = lat1;	tlat[1] = lat1 + 0.25 * dlat;	tlat[2] = lat1 + 0.5 * dlat;
		tlat[3] = lat1 + 0.75 * dlat;	tlat[4] = lat2;
		*x = tlon;
		*y = tlat;
		return (n = n_alloc);
	}

	/* Must do general case */
	n = 0;
	min_gap = 0.1 * GMT->current.setting.map_line_step;
	if ((n_alloc = lrint (ceil (fabs (lat2 - lat1) / GMT->current.map.dlat))) == 0) return (0);

	n_alloc++;	/* So n_alloc is at least 2 */
	dlat0 = (lat2 - lat1) / n_alloc;
	pos = (dlat0 > 0.0);

	k = n_alloc;	n_alloc = 0;
	gmt_M_malloc2 (GMT, tlon, tlat, k, &n_alloc, double);

	tlon[0] = lon;
	tlat[0] = lat1;
	gmt_geo_to_xy (GMT, tlon[0], tlat[0], &x0, &y0);
	while ((pos && (tlat[n] < lat2)) || (!pos && (tlat[n] > lat2))) {
		n++;
		if (n == n_alloc-1) {
			n_alloc += GMT_SMALL_CHUNK;
			tlon = gmt_M_memory (GMT, tlon, n_alloc, double);
			tlat = gmt_M_memory (GMT, tlat, n_alloc, double);
		}
		n_try = 0;
		keep_trying = true;
		dlat = dlat0;
		tlon[n] = lon;
		do {
			n_try++;
			tlat[n] = tlat[n-1] + dlat;
			if (gmt_M_is_geographic (GMT, GMT_IN) && fabs (tlat[n]) > 90.0) tlat[n] = copysign (90.0, tlat[n]);
			gmt_geo_to_xy (GMT, tlon[n], tlat[n], &x1, &y1);
			if ((*GMT->current.map.jump) (GMT, x0, y0, x1, y1) || (y0 < GMT->current.proj.rect[YLO] || y0 > GMT->current.proj.rect[YHI]))
				keep_trying = false;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > GMT->current.setting.map_line_step)
					dlat *= 0.5;
				else if (d < min_gap)
					dlat *= 2.0;
				else
					keep_trying = false;
			}
		} while (keep_trying && n_try < 10);
		x0 = x1;	y0 = y1;
	}
	tlon[n] = lon;
	tlat[n] = lat2;
	n++;

	if (n != n_alloc) {
		tlon = gmt_M_memory (GMT, tlon, n, double);
		tlat = gmt_M_memory (GMT, tlat, n, double);
	}

	*x = tlon;	*y = tlat;
	return (n);
}

/*! . */
uint64_t gmtlib_latpath (struct GMT_CTRL *GMT, double lat, double lon1, double lon2, double **x, double **y) {
	size_t n_alloc = 0;
	uint64_t k, n;
	int n_try, pos;
	bool keep_trying;
	double dlon, dlon0, *tlon = NULL, *tlat = NULL, x0, x1, y0, y1, d, min_gap;

	if (GMT->current.map.parallel_straight == 2) {	/* Special non-sampling for gmtselect/grdlandmask */
		gmt_M_malloc2 (GMT, tlon, tlat, 2U, NULL, double);
		tlat[0] = tlat[1] = lat;
		tlon[0] = lon1;	tlon[1] = lon2;
		*x = tlon;	*y = tlat;
		return (2ULL);
	}
	if (GMT->current.map.parallel_straight) {	/* Easy, just a straight line connection via quarter points */
		gmt_M_malloc2 (GMT, tlon, tlat, 5U, NULL, double);
		tlat[0] = tlat[1] = tlat[2] = tlat[3] = tlat[4] = lat;
		dlon = lon2 - lon1;
		tlon[0] = lon1;	tlon[1] = lon1 + 0.25 * dlon;	tlon[2] = lon1 + 0.5 * dlon;
		tlon[3] = lon1 + 0.75 * dlon;	tlon[4] = lon2;
		*x = tlon;	*y = tlat;
		return (5ULL);
	}
	/* Here we try to walk along lat for small increment in longitude to make sure our steps are smaller than the line_step */
	min_gap = 0.1 * GMT->current.setting.map_line_step;
	if ((n_alloc = lrint (ceil (fabs (lon2 - lon1) / GMT->current.map.dlon))) == 0) return (0);	/* Initial guess to path length */

	n_alloc++;	/* n_alloc is at least 2 */
	dlon0 = (lon2 - lon1) / n_alloc;
	pos = (dlon0 > 0.0);

	k = n_alloc;	n_alloc = 0;
	gmt_M_malloc2 (GMT, tlon, tlat, k, &n_alloc, double);
	tlon[0] = lon1;	tlat[0] = lat;
	gmt_geo_to_xy (GMT, tlon[0], tlat[0], &x0, &y0);
	n = 0;
	while ((pos && (tlon[n] < lon2)) || (!pos && (tlon[n] > lon2))) {
		n++;
		if (n == n_alloc) gmt_M_malloc2 (GMT, tlon, tlat, n, &n_alloc, double);
		n_try = 0;
		keep_trying = true;
		dlon = dlon0;
		tlat[n] = lat;
		do {
			n_try++;
			tlon[n] = tlon[n-1] + dlon;
			gmt_geo_to_xy (GMT, tlon[n], tlat[n], &x1, &y1);
			if ((*GMT->current.map.jump) (GMT, x0, y0, x1, y1) || (y0 < GMT->current.proj.rect[YLO] || y0 > GMT->current.proj.rect[YHI]))
				keep_trying = false;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > GMT->current.setting.map_line_step)
					dlon *= 0.5;
				else if (d < min_gap)
					dlon *= 2.0;
				else
					keep_trying = false;
			}
		} while (keep_trying && n_try < 10);
		x0 = x1;	y0 = y1;
	}
	tlon[n] = lon2;
	tlat[n] = lat;
	n++;
	n_alloc = n;
	gmt_M_malloc2 (GMT, tlon, tlat, 0, &n_alloc, double);

	*x = tlon;	*y = tlat;
	return (n);
}

GMT_LOCAL bool accept_the_jump (struct GMT_CTRL *GMT, double lon1, double lon0, double xx[], bool cartesian) {
	/* Carefully examine if we really want to draw line from left to right boundary.
	 * We want to avoid E-W wrapping lines for near-global areas where points simply move
	 * from being > 180 degres from the map area to < -180 even though the points do not
	 * really reflect motion across the area */
	double dlon0, dlon1, dlon;
	int xm, ym;
	gmt_M_unused(xx);
	if (cartesian) return true;	/* No wrap issues if Cartesian x,y */
	xm = GMT->current.map.this_x_status - GMT->current.map.prev_x_status;
	ym = GMT->current.map.this_y_status - GMT->current.map.prev_y_status;
	gmt_M_set_delta_lon (lon0, GMT->current.proj.central_meridian, dlon0);
	gmt_M_set_delta_lon (lon1, GMT->current.proj.central_meridian, dlon1);
	dlon = dlon1 - dlon0;
	/* When the line is so far away from the regino that it crosses 180 from the central meridian
	 * we get false jumps.  However, we find those by looking at the two dlons and that the change
	 * in x status is 4 (going from left of region to right of region of the opposite).  We then
	 * reject the jump, otherwise we accept it. */
	//fprintf (stderr, "dlon0 = %g dlon1 = %g dlon = %g xx0 = %g xx1 = %g xm = %d ym = %d\n", dlon0, dlon1, dlon, xx[0], xx[1], xm, ym);
	if (!(fabs (dlon) > 180.0 && abs (xm) == 4 && ym == 0)) return true;	/* Very unlikely to be a real wrap/jump */
	return false;
}

/*! . */
uint64_t gmt_geo_to_xy_line (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n) {
	/* Traces the lon/lat array and returns x,y plus appropriate pen moves
	 * Pen moves are caused by breakthroughs of the map boundary or when
	 * a point has lon = NaN or lat = NaN (this means "pick up pen") */
	uint64_t j, k, np, n_sections;
	bool this_inside, jump, cartesian = gmt_M_is_cartesian (GMT, GMT_IN);
	/* bool last_inside = false; */
	unsigned int sides[4];
	unsigned int nx;
	double xlon[4], xlat[4], xx[4], yy[4];
	double this_x, this_y, last_x, last_y, dummy[4];

	while (n > GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);

	np = 0;
	gmt_geo_to_xy (GMT, lon[0], lat[0], &last_x, &last_y);
	if (!gmt_map_outside (GMT, lon[0], lat[0])) {	/* First point is inside the region */
		GMT->current.plot.x[0] = last_x;	GMT->current.plot.y[0] = last_y;
		GMT->current.plot.pen[np++] = PSL_MOVE;
		/* last_inside = true; */
	}
	for (j = 1; j < n; j++) {
		gmt_geo_to_xy (GMT, lon[j], lat[j], &this_x, &this_y);
		this_inside = !gmt_map_outside (GMT, lon[j], lat[j]);
		if (gmt_M_is_dnan (lon[j]) || gmt_M_is_dnan (lat[j])) continue;	/* Skip NaN point now */
		if (gmt_M_is_dnan (lon[j-1]) || gmt_M_is_dnan (lat[j-1])) {		/* Point after NaN needs a move */
			GMT->current.plot.x[np] = this_x;	GMT->current.plot.y[np] = this_y;
			GMT->current.plot.pen[np++] = PSL_MOVE;
			if (np == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
			last_x = this_x;	last_y = this_y;	/* last_inside = this_inside; */
			continue;
		}
		if ((nx = map_crossing (GMT, lon[j-1], lat[j-1], lon[j], lat[j], xlon, xlat, xx, yy, sides))) { /* Do nothing if we get crossings*/ }
		else if (GMT->current.map.is_world)	/* Check global wrapping if 360 range */
			nx = (*GMT->current.map.wrap_around_check) (GMT, dummy, last_x, last_y, this_x, this_y, xx, yy, sides);
		if (nx == 1) {	/* inside-outside or outside-inside; set move&clip vs draw flags */
			GMT->current.plot.x[np] = xx[0];	GMT->current.plot.y[np] = yy[0];
			/* If next point is inside then we want to move to the crossing, otherwise we want to draw to the crossing */
			GMT->current.plot.pen[np++] = (this_inside) ? PSL_MOVE|PSL_CLIP : PSL_DRAW|PSL_CLIP;
			if (np == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		}
		else if (nx == 2) {	/* outside-inside-outside or (with wrapping) inside-outside-inside */
			/* PW: I will be working on things here to solve the polygon wrap problem reported by Nicky */
			jump = accept_the_jump (GMT, lon[j], lon[j-1], xx, cartesian);
			if (jump) {
			//if ((this_inside && last_inside) || cartesian || dy > 0.1) {	/* outside-inside-outside or (with wrapping) inside-outside-inside */
				GMT->current.plot.x[np] = xx[0];	GMT->current.plot.y[np] = yy[0];
				GMT->current.plot.pen[np++] = (this_inside) ? PSL_DRAW|PSL_CLIP : PSL_MOVE|PSL_CLIP;
				if (np == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
				GMT->current.plot.x[np] = xx[1];	GMT->current.plot.y[np] = yy[1];
				GMT->current.plot.pen[np++] = (this_inside) ? PSL_MOVE|PSL_CLIP : PSL_DRAW|PSL_CLIP;
				if (np == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
			}
		}
		if (this_inside) {
			if ( np >= GMT->current.plot.n_alloc ) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "bad access: cannot access current.plot.x[%" PRIu64 "], np=%" PRIu64 ", GMT->current.plot.n=%" PRIu64 "\n", np, np, GMT->current.plot.n);
			}
			else {
				GMT->current.plot.x[np] = this_x;	GMT->current.plot.y[np] = this_y;
				GMT->current.plot.pen[np++] = PSL_DRAW;
			}
			if (np == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		}
		last_x = this_x;	last_y = this_y;	/* last_inside = this_inside; */
	}
	if (np) GMT->current.plot.pen[0] |= PSL_MOVE;	/* Sanity override: Gotta start off with new start point */

	/* When a line that starts and ends inside the domain exits and reenters, we end up with two pieces.
	 * When these are plotted separately the sections are not joined properly and ugly gaps can appear, especially if
	 * the pen width is large.  Here we try to fix this case since it happens fairly frequently */

	for (j = n_sections = k = 0; j < np; j++) if (GMT->current.plot.pen[j] & PSL_MOVE) {
		n_sections++;
		if (n_sections == 2) k = j;	/* Start of 2nd section */
	}
	if (n_sections == 2 && doubleAlmostEqualZero (GMT->current.plot.x[0], GMT->current.plot.x[np-1]) && doubleAlmostEqualZero (GMT->current.plot.y[0], GMT->current.plot.y[np-1])) {
		double *tmp = gmt_M_memory (GMT, NULL, np, double);
		/* Shuffle x-array safely */
		gmt_M_memcpy (tmp, &GMT->current.plot.x[k], np-k, double);
		gmt_M_memcpy (&tmp[np-k], GMT->current.plot.x, k, double);
		gmt_M_memcpy (GMT->current.plot.x, tmp, np, double);
		/* Shuffle y-array safely */
		gmt_M_memcpy (tmp, &GMT->current.plot.y[k], np-k, double);
		gmt_M_memcpy (&tmp[np-k], GMT->current.plot.y, k, double);
		gmt_M_memcpy (GMT->current.plot.y, tmp, np, double);
		/* Change PSL_MOVE to PSL_DRAW at start of 2nd section */
		GMT->current.plot.pen[k] = PSL_DRAW;
		if (k && GMT->current.plot.pen[k-1] & PSL_CLIP) GMT->current.plot.pen[k-1] = PSL_DRAW;
		if (k < (np-1) && GMT->current.plot.pen[k+1] & PSL_CLIP) GMT->current.plot.pen[k+1] = PSL_DRAW;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_geo_to_xy_line: Clipping in two separate abutting lines that were joined into a single line\n");
		gmt_M_free (GMT, tmp);
	}

	return (np);
}

/*! . */
uint64_t gmt_compact_line (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, int pen_flag, int *pen) {
	/* true if pen movements is present */
	/* gmt_compact_line will remove unnecessary points in paths, but does not reallocate to the shorter size */
	uint64_t i, j;
	double old_slope, new_slope, dx;
	char *flag = NULL;

	if (n < 3) return (n);	/* Nothing to do */
	flag = gmt_M_memory (GMT, NULL, n, char);

	dx = x[1] - x[0];
	old_slope = (doubleAlmostEqualZero (x[1], x[0])) ? copysign (HALF_DBL_MAX, y[1] - y[0]) : (y[1] - y[0]) / dx;

	for (i = 1; i < n-1; ++i) {
		dx = x[i+1] - x[i];
		new_slope = (doubleAlmostEqualZero (x[i+1], x[i])) ? copysign (HALF_DBL_MAX, y[i+1] - y[i]) : (y[i+1] - y[i]) / dx;
		if (doubleAlmostEqualZero (new_slope, old_slope) && !(pen_flag && (pen[i]+pen[i+1]) > 4))	/* 4 is 2+2 which is draw line; a 3 will produce > 4 */
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
	gmt_M_free (GMT, flag);

	return (j);
}

/* Routines to transform grdfiles to/from map projections */

/*! . */
int gmt_project_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double *inc, unsigned int n_columns, unsigned int n_rows, unsigned int dpi, unsigned int offset) {
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_project_init: IN: Inc [%.12g/%.12g] n_columns/n_rows [%u/%u] dpi = %u offset = %u\n",
		inc[0], inc[1], n_columns, n_rows, dpi, offset);

	if (inc[GMT_X] > 0.0 && inc[GMT_Y] > 0.0) {
		if (GMT->current.io.inc_code[GMT_X] || GMT->current.io.inc_code[GMT_Y]) {	/* Must convert from distance units to degrees */
			gmt_M_memcpy (header->inc, inc, 2, double);	/* Set these temporarily as the grids incs */
			gmt_RI_prepare (GMT, header);			/* Converts the proper xinc/yinc units */
			gmt_M_memcpy (inc, header->inc, 2, double);	/* Restore the inc array for use below */
			GMT->current.io.inc_code[GMT_X] = GMT->current.io.inc_code[GMT_Y] = 0;
		}
#if 0
		if (<to ensure x-ymax is adjusted so the increments are exact>) {
			header->wesn[XHI] = ceil  (header->wesn[XHI] / inc[GMT_X]) * inc[GMT_X];
			header->wesn[YHI] = ceil  (header->wesn[YHI] / inc[GMT_Y]) * inc[GMT_Y];
		}
#endif
		header->n_columns = gmt_M_get_n (GMT, header->wesn[XLO], header->wesn[XHI], inc[GMT_X], offset);
		header->n_rows = gmt_M_get_n (GMT, header->wesn[YLO], header->wesn[YHI], inc[GMT_Y], offset);
		header->inc[GMT_X] = gmt_M_get_inc (GMT, header->wesn[XLO], header->wesn[XHI], header->n_columns, offset);
		header->inc[GMT_Y] = gmt_M_get_inc (GMT, header->wesn[YLO], header->wesn[YHI], header->n_rows, offset);
	}
	else if (n_columns > 0 && n_rows > 0) {
		header->n_columns = n_columns;	header->n_rows = n_rows;
		header->inc[GMT_X] = gmt_M_get_inc (GMT, header->wesn[XLO], header->wesn[XHI], header->n_columns, offset);
		header->inc[GMT_Y] = gmt_M_get_inc (GMT, header->wesn[YLO], header->wesn[YHI], header->n_rows, offset);
	}
	else if (dpi > 0) {
		header->n_columns = urint ((header->wesn[XHI] - header->wesn[XLO]) * dpi) + 1 - offset;
		header->n_rows = urint ((header->wesn[YHI] - header->wesn[YLO]) * dpi) + 1 - offset;
		header->inc[GMT_X] = gmt_M_get_inc (GMT, header->wesn[XLO], header->wesn[XHI], header->n_columns, offset);
		header->inc[GMT_Y] = gmt_M_get_inc (GMT, header->wesn[YLO], header->wesn[YHI], header->n_rows, offset);
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_project_init: Necessary arguments not set\n");
		GMT_exit (GMT, GMT_PROJECTION_ERROR); return GMT_PROJECTION_ERROR;
	}
	header->registration = offset;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_project_init: OUT: Inc [%.12g/%.12g] n_columns/n_rows [%u/%u] dpi = %u offset = %u\n",
		inc[0], inc[1], n_columns, n_rows, dpi, offset);

	gmt_RI_prepare (GMT, header);	/* Ensure -R -I consistency and set n_columns, n_rows */
	gmt_M_err_pass (GMT, gmt_grd_RI_verify (GMT, header, 1), "");
	gmt_M_grd_setpad (GMT, header, GMT->current.io.pad);			/* Assign default pad */
	gmt_set_grddim (GMT, header);	/* Set all dimensions before returning */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Grid projection from size %dx%d to %dx%d\n", n_columns, n_rows, header->n_columns, header->n_rows);
	return (GMT_NOERROR);
}


/*! . */
int gmt_grd_project (struct GMT_CTRL *GMT, struct GMT_GRID *I, struct GMT_GRID *O, bool inverse) {
	/* Generalized grid projection that deals with both interpolation and averaging effects.
	 * It requires the input grid to have 2 boundary rows/cols so that the bcr
	 * functions can be used.  The I struct represents the input grid which is either in original
	 * (i.e., lon/lat) coordinates or projected x/y (if inverse = true).
	 *
	 * I:	Grid and header with input grid on a padded grid with 2 extra rows/columns
	 * O:	Grid and header for output grid, no padding needed (but allowed)
	 * inverse:	true if input is x/y and we want to invert for a lon/lat grid
	 *
	 * In addition, these settings (via -n) control interpolation:
	 * antialias:	true if we need to do the antialiasing STEP 1 (below)
	 * interpolant:	0 = nearest neighbor, 1 = bilinear, 2 = B-spline, 3 = bicubic
	 * threshold:	minimum weight to be used. If weight < threshold interpolation yields NaN.
	 * We initialize the O->data array to NaN.
	 *
	 * Changed 10-Sep-07 to include the argument "antialias" and "threshold" and
	 * made "interpolant" an integer (was int bilinear).
	 */

	bool in = false, out = false;
	int col_in, row_in, col_out, row_out;
 	uint64_t ij_in, ij_out;
	short int *nz = NULL;
	double x_proj = 0.0, y_proj = 0.0, z_int, inv_nz;
	double *x_in = NULL, *x_out = NULL, *x_in_proj = NULL, *x_out_proj = NULL;
	double *y_in = NULL, *y_out = NULL, *y_in_proj = NULL, *y_out_proj = NULL;

	/* Only input grid MUST have at least 2 rows/cols padding */
	if (I->header->pad[XLO] < 2 || I->header->pad[XHI] < 2 || I->header->pad[YLO] < 2 || I->header->pad[YHI] < 2) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_grd_project: Input grid does not have sufficient (2) padding\n");
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_grd_project: In [%.12g/%.12g/%.12g/%.12g] and out [%.12g/%.12g/%.12g/%.12g]\n",
		I->header->wesn[XLO], I->header->wesn[XHI], I->header->wesn[YLO], I->header->wesn[YHI],
		O->header->wesn[XLO], O->header->wesn[XHI], O->header->wesn[YLO], O->header->wesn[YHI]);

	/* Precalculate grid coordinates */

	if (I->x) {	/* Reuse existing arrays */
		x_in = I->x;	y_in = I->y;	in = true;
	}
	else {	/* Must allocate here */
		x_in  = gmt_grd_coord (GMT, I->header, GMT_X);
		y_in  = gmt_grd_coord (GMT, I->header, GMT_Y);
	}
	if (O->x) {	/* Reuse existing arrays */
		x_out = O->x;	y_out = O->y;	out = true;
	}
	else {	/* Must allocate here */
		x_out = gmt_grd_coord (GMT, O->header, GMT_X);
		y_out = gmt_grd_coord (GMT, O->header, GMT_Y);
	}

	if (gmt_M_is_rect_graticule (GMT)) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj  = gmt_M_memory (GMT, NULL, I->header->n_columns, double);
		y_in_proj  = gmt_M_memory (GMT, NULL, I->header->n_rows, double);
		x_out_proj = gmt_M_memory (GMT, NULL, O->header->n_columns, double);
		y_out_proj = gmt_M_memory (GMT, NULL, O->header->n_rows, double);
		if (inverse) {
			gmt_M_row_loop  (GMT, I, row_in)  gmt_xy_to_geo (GMT, &x_proj, &y_in_proj[row_in], I->header->wesn[XLO], y_in[row_in]);
			gmt_M_col_loop2 (GMT, I, col_in)  gmt_xy_to_geo (GMT, &x_in_proj[col_in], &y_proj, x_in[col_in], I->header->wesn[YLO]);
			gmt_M_row_loop  (GMT, O, row_out) gmt_geo_to_xy (GMT, I->header->wesn[YLO], y_out[row_out], &x_proj, &y_out_proj[row_out]);
			gmt_M_col_loop2 (GMT, O, col_out) gmt_geo_to_xy (GMT, x_out[col_out], I->header->wesn[YLO], &x_out_proj[col_out], &y_proj);
		}
		else {
			gmt_M_row_loop  (GMT, I, row_in) gmt_geo_to_xy (GMT, I->header->wesn[XLO], y_in[row_in], &x_proj, &y_in_proj[row_in]);
			gmt_M_col_loop2 (GMT, I, col_in) gmt_geo_to_xy (GMT, x_in[col_in], I->header->wesn[YLO], &x_in_proj[col_in], &y_proj);
			gmt_M_row_loop  (GMT, O, row_out) gmt_xy_to_geo (GMT, &x_proj, &y_out_proj[row_out], I->header->wesn[YLO], y_out[row_out]);
			gmt_M_col_loop2 (GMT, O, col_out) {	/* Here we must also align longitudes properly */
				gmt_xy_to_geo (GMT, &x_out_proj[col_out], &y_proj, x_out[col_out], I->header->wesn[YLO]);
				if (gmt_M_x_is_lon (GMT, GMT_IN) && !gmt_M_is_dnan (x_out_proj[col_out])) {
					while (x_out_proj[col_out] < I->header->wesn[XLO] - GMT_CONV4_LIMIT) x_out_proj[col_out] += 360.0;
					while (x_out_proj[col_out] > I->header->wesn[XHI] + GMT_CONV4_LIMIT) x_out_proj[col_out] -= 360.0;
				}
			}
		}
	}

	gmt_M_grd_loop (GMT, O, row_out, col_out, ij_out) O->data[ij_out] = GMT->session.f_NaN;	/* So that nodes outside will retain a NaN value */

	/* PART 1: Project input grid points and do a blockmean operation */

	O->header->z_min = FLT_MAX; O->header->z_max = -FLT_MAX;	/* Min/max for out */
	if (GMT->common.n.antialias) {	/* Blockaverage repeat pixels, at least the first ~32767 of them... */
		int n_columns = O->header->n_columns, n_rows = O->header->n_rows;
		nz = gmt_M_memory (GMT, NULL, O->header->size, short int);
		/* Cannot do OPENMP yet here since it would require a reduction into an output array (nz) */

		gmt_M_row_loop (GMT, I, row_in) {	/* Loop over the input grid row coordinates */
			if (gmt_M_is_rect_graticule (GMT)) y_proj = y_in_proj[row_in];
			gmt_M_col_loop (GMT, I, row_in, col_in, ij_in) {	/* Loop over the input grid col coordinates */
				if (gmt_M_is_rect_graticule (GMT))
					x_proj = x_in_proj[col_in];
				else if (inverse)
					gmt_xy_to_geo (GMT, &x_proj, &y_proj, x_in[col_in], y_in[row_in]);
				else {
					if (GMT->current.map.outside (GMT, x_in[col_in], y_in[row_in])) continue;	/* Quite possible we are beyond the horizon */
					gmt_geo_to_xy (GMT, x_in[col_in], y_in[row_in], &x_proj, &y_proj);
				}

				/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */

				row_out = gmt_M_grd_y_to_row (GMT, y_proj, O->header);
				if (row_out < 0 || row_out >= n_rows) continue;	/* Outside our grid region */
				col_out = gmt_M_grd_x_to_col (GMT, x_proj, O->header);
				if (col_out < 0 || col_out >= n_columns) continue;	/* Outside our grid region */

				/* OK, this projected point falls inside the projected grid's rectangular domain */

				ij_out = gmt_M_ijp (O->header, row_out, col_out);	/* The output node */
				if (nz[ij_out] == 0) O->data[ij_out] = 0.0f;	/* First time, override the initial value */
				if (nz[ij_out] < SHRT_MAX) {			/* Avoid overflow */
					O->data[ij_out] += I->data[ij_in];	/* Add up the z-sum inside this rect... */
					nz[ij_out]++;				/* ..and how many points there were */
				}
				if (gmt_M_is_fnan (O->data[ij_out])) continue;
				if (O->data[ij_out] < O->header->z_min) O->header->z_min = O->data[ij_out];
				else if (O->data[ij_out] > O->header->z_max) O->header->z_max = O->data[ij_out];
			}
		}
	}

	/* PART 2: Create weighted average of interpolated and observed points */

/* Open MP does not work yet */

/* The OpenMP loop below fails and yields nodes still set to NaN.  I cannot see any errors but obviously
 * there is something that is not quite correct. */

//#ifdef _OPENMP
//#pragma omp parallel for private(row_out,y_proj,col_out,ij_out,x_proj,z_int,inv_nz) shared(O,GMT,y_out_proj,x_out_proj,inverse,x_out,y_out,I,nz)
//#endif
	for (row_out = 0; row_out < (int)O->header->n_rows; row_out++) {	/* Loop over the output grid row coordinates */
		if (gmt_M_is_rect_graticule (GMT)) y_proj = y_out_proj[row_out];
		gmt_M_col_loop (GMT, O, row_out, col_out, ij_out) {	/* Loop over the output grid col coordinates */
			if (gmt_M_is_rect_graticule (GMT))
				x_proj = x_out_proj[col_out];
			else if (inverse)
				gmt_geo_to_xy (GMT, x_out[col_out], y_out[row_out], &x_proj, &y_proj);
			else {
				gmt_xy_to_geo (GMT, &x_proj, &y_proj, x_out[col_out], y_out[row_out]);
				if (GMT->current.proj.projection_GMT == GMT_GENPER && GMT->current.proj.g_outside) continue;	/* We are beyond the horizon */

				/* On 17-Sep-2007 the slack of GMT_CONV4_LIMIT was added to allow for round-off
				   errors in the grid limits. */
				if (gmt_M_x_is_lon (GMT, GMT_IN) && !gmt_M_is_dnan (x_proj)) {
					while (x_proj < I->header->wesn[XLO] - GMT_CONV4_LIMIT) x_proj += 360.0;
					while (x_proj > I->header->wesn[XHI] + GMT_CONV4_LIMIT) x_proj -= 360.0;
				}
			}

			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now the interpret the input grid at that projected output point */

			z_int = gmt_bcr_get_z (GMT, I, x_proj, y_proj);

			if (!GMT->common.n.antialias || nz[ij_out] < 2)	/* Just use the interpolated value */
				O->data[ij_out] = (gmt_grdfloat)z_int;
			else if (gmt_M_is_dnan (z_int))		/* Take the average of what we accumulated */
				O->data[ij_out] /= nz[ij_out];	/* Plain average */
			else {					/* Weighted average between blockmean'ed and interpolated values */
				inv_nz = 1.0 / nz[ij_out];
				O->data[ij_out] = (gmt_grdfloat) ((O->data[ij_out] + z_int * inv_nz) / (nz[ij_out] + inv_nz));
			}
			if (O->data[ij_out] < O->header->z_min) O->header->z_min = O->data[ij_out];
			if (O->data[ij_out] > O->header->z_max) O->header->z_max = O->data[ij_out];
		}
	}

	if (O->header->z_min < I->header->z_min || O->header->z_max > I->header->z_max) {	/* Truncate output to input extrema */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gmt_grd_project: Output grid extrema [%g/%g] exceed extrema of input grid [%g/%g] due to resampling\n",
			O->header->z_min, O->header->z_max, I->header->z_min, I->header->z_max);
		if (GMT->common.n.truncate) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gmt_grd_project: Output grid clipped to input grid extrema\n");
			gmt_M_grd_loop (GMT, O, row_out, col_out, ij_out) {
				if (O->data[ij_out] < I->header->z_min) O->data[ij_out] = (gmt_grdfloat)I->header->z_min;
				else if (O->data[ij_out] > I->header->z_max) O->data[ij_out] = (gmt_grdfloat)I->header->z_max;
			}
			O->header->z_min = I->header->z_min;	O->header->z_max = I->header->z_max;
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "gmt_grd_project: See option -n+c to clip resampled output range to given input range\n");
	}

	/* Time to clean up our mess */

	if (!in) {
		gmt_M_free (GMT, x_in);
		gmt_M_free (GMT, y_in);
	}
	if (!out) {
		gmt_M_free (GMT, x_out);
		gmt_M_free (GMT, y_out);
	}
	if (gmt_M_is_rect_graticule(GMT)) {
		gmt_M_free (GMT, x_in_proj);
		gmt_M_free (GMT, y_in_proj);
		gmt_M_free (GMT, x_out_proj);
		gmt_M_free (GMT, y_out_proj);
	}
	if (GMT->common.n.antialias) gmt_M_free (GMT, nz);

	return (GMT_NOERROR);
}

/*! . */
int gmt_img_project (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, struct GMT_IMAGE *O, bool inverse) {
	/* Generalized image projection that deals with both interpolation and averaging effects.
	 * It requires the input image to have 2 boundary rows/cols so that the bcr
	 * functions can be used.  The I struct represents the input image which is either in original
	 * (i.e., lon/lat) coordinates or projected x/y (if inverse = true).
	 *
	 * I:	Image and header with input image on a padded image with 2 extra rows/columns
	 * O:	Image and header for output image, no padding needed (but allowed)
	 * edgeinfo:	Structure with information about boundary conditions on input image
	 * inverse:	true if input is x/y and we want to invert for a lon/lat image
	 *
	 * In addition, these settings (via -n) control interpolation:
	 * antialias:	true if we need to do the antialiasing STEP 1 (below)
	 * interpolant:	0 = nearest neighbor, 1 = bilinear, 2 = B-spline, 3 = bicubic
	 * threshold:	minimum weight to be used. If weight < threshold interpolation yields NaN.
	 *
	 * We initialize the O->data array to the NaN color.
	 *
	 * Changed 10-Sep-07 to include the argument "antialias" and "threshold" and
	 * made "interpolant" an integer (was int bilinear).
	 */

	bool in = false, out = false;
	int col_in, row_in, col_out, row_out, b, nb = I->header->n_bands;
 	uint64_t ij_in, ij_out;
	short int *nz = NULL;
	double x_proj = 0.0, y_proj = 0.0, inv_nz, rgb[4];
	double *x_in = NULL, *x_out = NULL, *x_in_proj = NULL, *x_out_proj = NULL;
	double *y_in = NULL, *y_out = NULL, *y_in_proj = NULL, *y_out_proj = NULL;
	unsigned char z_int[4], z_int_bg[4];

	/* Only input image MUST have at least 2 rows/cols padding */
	if (I->header->pad[XLO] < 2 || I->header->pad[XHI] < 2 || I->header->pad[YLO] < 2 || I->header->pad[YHI] < 2) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_img_project: Input image does not have sufficient (2) padding\n");
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	/* Precalculate grid coordinates */

	if (I->x) {	/* Reuse existing arrays */
		x_in = I->x;	y_in = I->y;	in = true;
	}
	else {	/* Must allocate here */
		x_in  = gmt_grd_coord (GMT, I->header, GMT_X);
		y_in  = gmt_grd_coord (GMT, I->header, GMT_Y);
	}
	if (O->x) {	/* Reuse existing arrays */
		x_out = O->x;	y_out = O->y;	out = true;
	}
	else {	/* Must allocate here */
		x_out = gmt_grd_coord (GMT, O->header, GMT_X);
		y_out = gmt_grd_coord (GMT, O->header, GMT_Y);
	}

	if (gmt_M_is_rect_graticule (GMT)) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj  = gmt_M_memory (GMT, NULL, I->header->n_columns, double);
		y_in_proj  = gmt_M_memory (GMT, NULL, I->header->n_rows, double);
		x_out_proj = gmt_M_memory (GMT, NULL, O->header->n_columns, double);
		y_out_proj = gmt_M_memory (GMT, NULL, O->header->n_rows, double);
		if (inverse) {
			gmt_M_row_loop  (GMT, I, row_in)  gmt_xy_to_geo (GMT, &x_proj, &y_in_proj[row_in], I->header->wesn[XLO], y_in[row_in]);
			gmt_M_col_loop2 (GMT, I, col_in)  gmt_xy_to_geo (GMT, &x_in_proj[col_in], &y_proj, x_in[col_in], I->header->wesn[YLO]);
			gmt_M_row_loop  (GMT, O, row_out) gmt_geo_to_xy (GMT, I->header->wesn[YLO], y_out[row_out], &x_proj, &y_out_proj[row_out]);
			gmt_M_col_loop2 (GMT, O, col_out) gmt_geo_to_xy (GMT, x_out[col_out], I->header->wesn[YLO], &x_out_proj[col_out], &y_proj);
		}
		else {
			gmt_M_row_loop  (GMT, I, row_in) gmt_geo_to_xy (GMT, I->header->wesn[XLO], y_in[row_in], &x_proj, &y_in_proj[row_in]);
			gmt_M_col_loop2 (GMT, I, col_in) gmt_geo_to_xy (GMT, x_in[col_in], I->header->wesn[YLO], &x_in_proj[col_in], &y_proj);
			gmt_M_row_loop  (GMT, O, row_out) gmt_xy_to_geo (GMT, &x_proj, &y_out_proj[row_out], I->header->wesn[YLO], y_out[row_out]);
			gmt_M_col_loop2 (GMT, O, col_out) {	/* Here we must also align longitudes properly */
				gmt_xy_to_geo (GMT, &x_out_proj[col_out], &y_proj, x_out[col_out], I->header->wesn[YLO]);
				if (gmt_M_x_is_lon (GMT, GMT_IN) && !gmt_M_is_dnan (x_out_proj[col_out])) {
					while (x_out_proj[col_out] < I->header->wesn[XLO] - GMT_CONV4_LIMIT) x_out_proj[col_out] += 360.0;
					while (x_out_proj[col_out] > I->header->wesn[XHI] + GMT_CONV4_LIMIT) x_out_proj[col_out] -= 360.0;
				}
			}
		}
	}

#if 0
	gmt_M_grd_loop (GMT, O, row_out, col_out, ij_out) 		/* So that nodes outside will have the NaN color */
		for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = gmt_M_u255 (GMT->current.setting.color_patch[GMT_NAN][b]);
#endif
	for (b = 0; b < 4; b++) z_int_bg[b] = gmt_M_u255 (GMT->current.setting.color_patch[GMT_NAN][b]);

	/* PART 1: Project input image points and do a blockmean operation */

	if (GMT->common.n.antialias) {	/* Blockaverage repeat pixels, at least the first ~32767 of them... */
		int n_columns = O->header->n_columns, n_rows = O->header->n_rows;
		nz = gmt_M_memory (GMT, NULL, O->header->size, short int);
		/* Cannot do OPENMP yet here since it would require a reduction into an output array (nz) */

		gmt_M_row_loop (GMT, I, row_in) {	/* Loop over the input grid row coordinates */
			if (gmt_M_is_rect_graticule (GMT)) y_proj = y_in_proj[row_in];
			gmt_M_col_loop (GMT, I, row_in, col_in, ij_in) {	/* Loop over the input grid col coordinates */
				if (gmt_M_is_rect_graticule (GMT))
					x_proj = x_in_proj[col_in];
				else if (inverse)
					gmt_xy_to_geo (GMT, &x_proj, &y_proj, x_in[col_in], y_in[row_in]);
				else {
					if (GMT->current.map.outside (GMT, x_in[col_in], y_in[row_in])) continue;	/* Quite possible we are beyond the horizon */
					gmt_geo_to_xy (GMT, x_in[col_in], y_in[row_in], &x_proj, &y_proj);
				}

				/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */

				row_out = (int)gmt_M_grd_y_to_row (GMT, y_proj, O->header);
				if (row_out < 0 || row_out >= n_rows) continue;	/* Outside our grid region */
				col_out = (int)gmt_M_grd_x_to_col (GMT, x_proj, O->header);
				if (col_out < 0 || col_out >= n_columns) continue;	/* Outside our grid region */

				/* OK, this projected point falls inside the projected grid's rectangular domain */

				ij_out = gmt_M_ijp (O->header, row_out, col_out);	/* The output node */
				if (nz[ij_out] == 0) for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = 0;	/* First time, override the initial value */
				if (nz[ij_out] < SHRT_MAX) {	/* Avoid overflow */
					for (b = 0; b < nb; b++) {
						rgb[b] = ((double)nz[ij_out] * O->data[nb*ij_out+b] + I->data[nb*ij_in+b])/(nz[ij_out] + 1.0);	/* Update the mean pix values inside this rect... */
						O->data[nb*ij_out+b] = (unsigned char) lrint (gmt_M_0_255_truncate (rgb[b]));
					}
					nz[ij_out]++;		/* ..and how many points there were */
				}
			}
		}
	}

	/* PART 2: Create weighted average of interpolated and observed points */

//#ifdef _OPENMP
//#pragma omp parallel for private(row_out,y_proj,col_out,ij_out,x_proj,z_int,inv_nz,b) shared(O,GMT,y_out_proj,x_out_proj,inverse,x_out,y_out,I,nz,z_int_bg,nb)
//#endif
	for (row_out = 0; row_out < (int)O->header->n_rows; row_out++) {	/* Loop over the output grid row coordinates */
		if (gmt_M_is_rect_graticule (GMT)) y_proj = y_out_proj[row_out];
		gmt_M_col_loop (GMT, O, row_out, col_out, ij_out) {	/* Loop over the output grid col coordinates */
			if (gmt_M_is_rect_graticule (GMT))
				x_proj = x_out_proj[col_out];
			else if (inverse)
				gmt_geo_to_xy (GMT, x_out[col_out], y_out[row_out], &x_proj, &y_proj);
			else {
				gmt_xy_to_geo (GMT, &x_proj, &y_proj, x_out[col_out], y_out[row_out]);
				if (GMT->current.proj.projection_GMT == GMT_GENPER && GMT->current.proj.g_outside) continue;	/* We are beyond the horizon */

				/* On 17-Sep-2007 the slack of GMT_CONV4_LIMIT was added to allow for round-off
				   errors in the grid limits. */
				if (gmt_M_x_is_lon (GMT, GMT_IN) && !gmt_M_is_dnan (x_proj)) {
					while (x_proj < I->header->wesn[XLO] - GMT_CONV4_LIMIT) x_proj += 360.0;
					while (x_proj > I->header->wesn[XHI] + GMT_CONV4_LIMIT) x_proj -= 360.0;
				}
			}

			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now find nearest node on the input grid */

			if (gmtlib_bcr_get_img (GMT, I, x_proj, y_proj, z_int))		/* So that nodes outside will have the NaN color */
				for (b = 0; b < 4; b++) z_int[b] = z_int_bg[b];

			if (!GMT->common.n.antialias || nz[ij_out] < 2)	/* Just use the interpolated value */
				for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = z_int[b];
			else {						/* Weighted average between blockmean'ed and interpolated values */
				inv_nz = 1.0 / nz[ij_out];
				for (b = 0; b < nb; b++) {
					rgb[b] = ((double)nz[ij_out] * O->data[nb*ij_out+b] + z_int[b] * inv_nz) / (nz[ij_out] + inv_nz);
					O->data[nb*ij_out+b] = (unsigned char) lrint (gmt_M_0_255_truncate (rgb[b]));
				}
			}
		}
	}

	/* Time to clean up our mess */

	if (!in) {
		gmt_M_free (GMT, x_in);
		gmt_M_free (GMT, y_in);
	}
	if (!out) {
		gmt_M_free (GMT, x_out);
		gmt_M_free (GMT, y_out);
	}
	if (gmt_M_is_rect_graticule(GMT)) {
		gmt_M_free (GMT, x_in_proj);
		gmt_M_free (GMT, y_in_proj);
		gmt_M_free (GMT, x_out_proj);
		gmt_M_free (GMT, y_out_proj);
	}
	if (GMT->common.n.antialias) gmt_M_free (GMT, nz);

	return (GMT_NOERROR);
}

/*! . */
double gmt_azim_to_angle (struct GMT_CTRL *GMT, double lon, double lat, double c, double azim) {
	/* All variables in degrees */

	double lon1, lat1, x0, x1, y0, y1, dx, width, sinaz, cosaz, angle;

	if (gmt_M_is_linear (GMT)) {	/* Trivial case */
		angle = 90.0 - azim;
		if (GMT->current.proj.scale[GMT_X] != GMT->current.proj.scale[GMT_Y]) {	/* But allow for different x,y scaling */
			sincosd (angle, &sinaz, &cosaz);
			angle = d_atan2d (sinaz * GMT->current.proj.scale[GMT_Y], cosaz * GMT->current.proj.scale[GMT_X]);
		}
		return (angle);
	}
	else if (GMT->current.proj.projection_GMT == GMT_POLAR) {	/* r/theta */
		return (azim);	/* Place holder - not correct yet but don't want to go into the below if r/theta */
	}

	/* Find second point c spherical degrees away in the azim direction */

	gmtlib_get_point_from_r_az (GMT, lon, lat, c, azim, &lon1, &lat1);

	/* Convert both points to x,y and get angle */

	gmt_geo_to_xy (GMT, lon, lat, &x0, &y0);
	gmt_geo_to_xy (GMT, lon1, lat1, &x1, &y1);

	/* Check for wrap-around */

	dx = x1 - x0;
	if (gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && fabs (dx) > (width = gmt_half_map_width (GMT, y0))) {
		width *= 2.0;
		if (x1 < width)
			x0 -= width;
		else
			x0 += width;
	}
	angle = d_atan2d (y1 - y0, x1 - x0);
	return (angle);
}

/*! . */
uint64_t gmt_map_clip_path (struct GMT_CTRL *GMT, double **x, double **y, bool *donut) {
	/* This function returns a clip path corresponding to the
	 * extent of the map.
	 */

	uint64_t i, j, np;
	bool do_circle = false;
	double *work_x = NULL, *work_y = NULL, da, r0, s, c, lon, lat;

	*donut = false;

	if (GMT->common.R.oblique)	/* Rectangular map boundary */
		np = 4;
	else {
		switch (GMT->current.proj.projection_GMT) {
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
				if (GMT->current.proj.flip)
					*donut = (GMT->common.R.wesn[YHI] < GMT->current.proj.flip_radius && GMT->current.map.is_world);
				else
					*donut = (GMT->common.R.wesn[YLO] > 0.0 && GMT->current.map.is_world);
				np = GMT->current.map.n_lon_nodes + 1;
				if ((GMT->current.proj.flip && GMT->common.R.wesn[YHI] < GMT->current.proj.flip_radius) || (!GMT->current.proj.flip && GMT->common.R.wesn[YLO] > 0.0))	/* Need inside circle segment */
					np *= 2;
				else if (!GMT->current.map.is_world)	/* Need to include origin */
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
				np = (GMT->current.proj.polar && (GMT->common.R.wesn[YLO] <= -90.0 || GMT->common.R.wesn[YHI] >= 90.0)) ? GMT->current.map.n_lon_nodes + 2: 2 * (GMT->current.map.n_lon_nodes + 1);
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				np = 2 * GMT->current.map.n_lat_nodes + 2;
				break;
			case GMT_WINKEL:
			case GMT_HAMMER:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				np = 2 * GMT->current.map.n_lat_nodes + 2;
				if (GMT->common.R.wesn[YLO] != -90.0) np += GMT->current.map.n_lon_nodes - 1;
				if (GMT->common.R.wesn[YHI] != 90.0) np += GMT->current.map.n_lon_nodes - 1;
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				np = 2 * (GMT->current.map.n_lon_nodes + GMT->current.map.n_lat_nodes);
				break;
			default:
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bad case in gmt_map_clip_path (%d)\n", GMT->current.proj.projection_GMT);
				np = 0;
				break;
		}
	}

	work_x = gmt_M_memory (GMT, NULL, np+1, double);	/* Add one for manual closure */
	work_y = gmt_M_memory (GMT, NULL, np+1, double);

	if (GMT->common.R.oblique) {
		work_x[0] = work_x[3] = GMT->current.proj.rect[XLO];	work_y[0] = work_y[1] = GMT->current.proj.rect[YLO];
		work_x[1] = work_x[2] = GMT->current.proj.rect[XHI];	work_y[2] = work_y[3] = GMT->current.proj.rect[YHI];
	}
	else {
		switch (GMT->current.proj.projection_GMT) {	/* Fill in clip path */
			case GMT_LINEAR:
			case GMT_MERCATOR:
			case GMT_CYL_EQ:
			case GMT_CYL_EQDIST:
			case GMT_CYL_STEREO:
			case GMT_MILLER:
			case GMT_OBLIQUE_MERC:
				work_x[0] = work_x[3] = GMT->current.proj.rect[XLO];	work_y[0] = work_y[1] = GMT->current.proj.rect[YLO];
				work_x[1] = work_x[2] = GMT->current.proj.rect[XHI];	work_y[2] = work_y[3] = GMT->current.proj.rect[YHI];
				break;
			case GMT_LAMBERT:
			case GMT_ALBERS:
			case GMT_ECONIC:
				for (i = j = 0; i <= GMT->current.map.n_lon_nodes; i++, j++) {
					lon = (i == GMT->current.map.n_lon_nodes) ? GMT->common.R.wesn[XHI] : GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon;
					gmt_geo_to_xy (GMT, lon, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				}
				for (i = 0; i <= GMT->current.map.n_lon_nodes; i++, j++) {
					lon = (i == GMT->current.map.n_lon_nodes) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI] - i * GMT->current.map.dlon;
					gmt_geo_to_xy (GMT, lon, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				}
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				for (i = j = 0; i < GMT->current.map.n_lon_nodes; i++, j++)	/* South */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				for (i = 0; i < GMT->current.map.n_lat_nodes; i++, j++)	/* East */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat, &work_x[j], &work_y[j]);
				for (i = 0; i < GMT->current.map.n_lon_nodes; i++, j++)	/* North */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI] - i * GMT->current.map.dlon, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				for (i = 0; i < GMT->current.map.n_lat_nodes; i++, j++)	/* West */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI] - i * GMT->current.map.dlat, &work_x[j], &work_y[j]);
				break;
			case GMT_POLAR:
				r0 = GMT->current.proj.r * GMT->common.R.wesn[YLO] / GMT->common.R.wesn[YHI];
				if (*donut) {
					np /= 2;
					da = TWO_PI / np;
					for (i = 0, j = 2 * np - 1; i < np; i++, j--) {	/* Draw outer clippath */
						sincos (i * da, &s, &c);
						work_x[i] = GMT->current.proj.r * (1.0 + c);
						work_y[i] = GMT->current.proj.r * (1.0 + s);
						/* Do inner clippath and put it at end of array */
						work_x[j] = GMT->current.proj.r + r0 * c;
						work_y[j] = GMT->current.proj.r + r0 * s;
					}
				}
				else {
					da = fabs (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) / GMT->current.map.n_lon_nodes;
					if (GMT->current.proj.flip) {
						for (i = j = 0; i <= GMT->current.map.n_lon_nodes; i++, j++)	/* Draw outer clippath */
							gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + i * da, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						if (GMT->common.R.wesn[YHI] < GMT->current.proj.flip_radius) {	/* Must do the inner path as well */
							for (i = GMT->current.map.n_lon_nodes + 1; i > 0; i--, j++)	/* Draw inner clippath */
								gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + (i-1) * da, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						}
						if (doubleAlmostEqual (GMT->common.R.wesn[YHI], 90.0) && !GMT->current.map.is_world)	/* Add origin */
							gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
					}
					else {
						for (i = j = 0; i <= GMT->current.map.n_lon_nodes; i++, j++)	/* Draw outer clippath */
							gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + i * da, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						if (GMT->common.R.wesn[YLO] > 0.0) {	/* Must do the inner path as well */
							for (i = GMT->current.map.n_lon_nodes + 1; i > 0; i--, j++)	/* Draw inner clippath */
								gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + (i-1) * da, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						}
						if (gmt_M_is_zero (GMT->common.R.wesn[YLO]) && !GMT->current.map.is_world)	/* Add origin */
							gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
					}
				}
				break;
			case GMT_GENPER:
				gmtlib_genper_map_clip_path (GMT, np, work_x, work_y);
				break;
			case GMT_VANGRINTEN:
				do_circle = GMT->current.map.is_world;
				/* Intentionally fall through */
			case GMT_LAMB_AZ_EQ:
			case GMT_AZ_EQDIST:
			case GMT_ORTHO:
			case GMT_GNOMONIC:
			case GMT_STEREO:
				if (GMT->current.proj.polar && !do_circle) {
					j = 0;
					if (GMT->common.R.wesn[YLO] > -90.0) {
						for (i = 0; i <= GMT->current.map.n_lon_nodes; i++, j++) {
							lon = (i == GMT->current.map.n_lon_nodes) ? GMT->common.R.wesn[XHI] : GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon;
							gmt_geo_to_xy (GMT, lon, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add S pole */
						gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], -90.0, &work_x[j], &work_y[j]);
						j++;
					}
					if (GMT->common.R.wesn[YHI] < 90.0) {
						for (i = 0; i <= GMT->current.map.n_lon_nodes; i++, j++) {
							lon = (i == GMT->current.map.n_lon_nodes) ? GMT->common.R.wesn[XLO] : GMT->common.R.wesn[XHI] - i * GMT->current.map.dlon;
							gmt_geo_to_xy (GMT, lon, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add N pole */
						gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], 90.0, &work_x[j], &work_y[j]);
						j++;
					}
				}
				else {
					da = TWO_PI / np;
					for (i = 0; i < np; i++) {
						sincos (i * da, &s, &c);
						work_x[i] = GMT->current.proj.r * (1.0 + c);
						work_y[i] = GMT->current.proj.r * (1.0 + s);
					}
				}
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				for (i = j = 0; i <= GMT->current.map.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == GMT->current.map.n_lat_nodes) ? GMT->common.R.wesn[YHI] : GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat;
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI], lat, &work_x[j], &work_y[j]);
				}
				gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);	j++;
				for (i = GMT->current.map.n_lat_nodes; i > 0; j++, i--)	{	/* Left */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO] + (i-1) * GMT->current.map.dlat, &work_x[j], &work_y[j]);
				}
				break;
			case GMT_HAMMER:
			case GMT_WINKEL:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				for (i = j = 0; i <= GMT->current.map.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == GMT->current.map.n_lat_nodes) ? GMT->common.R.wesn[YHI] : GMT->common.R.wesn[YLO] + i * GMT->current.map.dlat;
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI], lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; GMT->common.R.wesn[YHI] != 90.0 && i < GMT->current.map.n_lon_nodes; i++, j++)
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XHI] - i * GMT->current.map.dlon, GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YHI], &work_x[j], &work_y[j]);	j++;
				for (i = GMT->current.map.n_lat_nodes; i > 0; j++, i--)	{	/* Left */
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[YLO] + (i-1)* GMT->current.map.dlat, &work_x[j], &work_y[j]);
				}
				for (i = 1; GMT->common.R.wesn[YLO] != -90.0 && i < GMT->current.map.n_lon_nodes; i++, j++)
					gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO] + i * GMT->current.map.dlon, GMT->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				break;
		}
	}

	/* CLose the clipping polygon */
	work_x[np] = work_x[0];
	work_y[np] = work_y[0];
	np++;
	if (!(*donut)) np = gmt_compact_line (GMT, work_x, work_y, np, false, NULL);

	*x = work_x;
	*y = work_y;

	return (np);
}

/*! . */
double gmtmap_lat_swap_quick (struct GMT_CTRL *GMT, double lat, double c[]) {
	/* Return latitude, in degrees, given latitude, in degrees, based on coefficients c */

	double delta, cos2phi, sin2phi;
	gmt_M_unused(GMT);

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (gmt_M_is_zero (lat)) return (0.0);

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (c[0] + cos2phi * (c[1] + cos2phi * (c[2] + cos2phi * c[3])));

	return (lat + R2D * delta);
}

/*! . */
double gmt_lat_swap (struct GMT_CTRL *GMT, double lat, unsigned int itype) {
	/* Return latitude, in degrees, given latitude, in degrees, based on itype */

	double delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (gmt_M_is_zero (lat)) return (0.0);

	if (GMT->current.proj.lat_swap_vals.spherical) return (lat);

	if (itype >= GMT_LATSWAP_N) {
		/* This should never happen -?- or do we want to allow the
			possibility of using itype = -1 to do nothing  */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmt_lat_swap(): Invalid choice, programming bug.\n");
		return(lat);
	}

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (GMT->current.proj.lat_swap_vals.c[itype][0]
		+ cos2phi * (GMT->current.proj.lat_swap_vals.c[itype][1]
		+ cos2phi * (GMT->current.proj.lat_swap_vals.c[itype][2]
		+ cos2phi * GMT->current.proj.lat_swap_vals.c[itype][3])));

	return (lat + R2D * delta);
}

/*! . */
void gmtlib_scale_eqrad (struct GMT_CTRL *GMT) {
	/* Reinitialize GMT->current.proj.EQ_RAD to the appropriate value */

	switch (GMT->current.proj.projection_GMT) {

		/* Conformal projections */

		case GMT_MERCATOR:
		case GMT_TM:
		case GMT_UTM:
		case GMT_OBLIQUE_MERC:
		case GMT_LAMBERT:
		case GMT_STEREO:

			GMT->current.proj.EQ_RAD = GMT->current.proj.lat_swap_vals.rm;
			break;

		/* Equal Area projections */

		case GMT_LAMB_AZ_EQ:
		case GMT_ALBERS:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:

			GMT->current.proj.EQ_RAD = GMT->current.proj.lat_swap_vals.ra;
			break;

		default:	/* Keep EQ_RAD as is */
			break;
	}

	/* Also reset dependencies of EQ_RAD */

	GMT->current.proj.i_EQ_RAD = 1.0 / GMT->current.proj.EQ_RAD;
	GMT->current.proj.M_PR_DEG = TWO_PI * GMT->current.proj.EQ_RAD / 360.0;
	GMT->current.proj.KM_PR_DEG = GMT->current.proj.M_PR_DEG / METERS_IN_A_KM;
}


/*! . */
void gmtlib_init_ellipsoid (struct GMT_CTRL *GMT) {
	double f;

	/* Set up ellipsoid parameters for the selected ellipsoid since gmt.conf could have changed them */

	f = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].flattening;
	GMT->current.proj.ECC2 = 2.0 * f - f * f;
	GMT->current.proj.ECC4 = GMT->current.proj.ECC2 * GMT->current.proj.ECC2;
	GMT->current.proj.ECC6 = GMT->current.proj.ECC2 * GMT->current.proj.ECC4;
	GMT->current.proj.one_m_ECC2 = 1.0 - GMT->current.proj.ECC2;
	GMT->current.proj.i_one_m_ECC2 = 1.0 / GMT->current.proj.one_m_ECC2;
	GMT->current.proj.ECC = d_sqrt (GMT->current.proj.ECC2);
	GMT->current.proj.half_ECC = 0.5 * GMT->current.proj.ECC;
	if (GMT->current.proj.ECC != 0) { /* avoid division by 0 */
		GMT->current.proj.i_half_ECC = 0.5 / GMT->current.proj.ECC;	/* Only used in inverse Alberts when e > 0 anyway */
	}
	GMT->current.proj.EQ_RAD = GMT->current.setting.ref_ellipsoid[GMT->current.setting.proj_ellipsoid].eq_radius;
	GMT->current.proj.i_EQ_RAD = 1.0 / GMT->current.proj.EQ_RAD;

	/* Spherical degrees to m or km */
	GMT->current.proj.mean_radius = map_mean_radius (GMT, GMT->current.proj.EQ_RAD, f);
	GMT->current.proj.M_PR_DEG = TWO_PI * GMT->current.proj.mean_radius / 360.0;
	GMT->current.proj.KM_PR_DEG = GMT->current.proj.M_PR_DEG / METERS_IN_A_KM;
	GMT->current.proj.DIST_M_PR_DEG = GMT->current.proj.M_PR_DEG;
	GMT->current.proj.DIST_KM_PR_DEG = GMT->current.proj.KM_PR_DEG;

	/* Compute coefficients needed for auxiliary latitude conversions */
	map_lat_swap_init (GMT);
}

/*! . */
void gmtlib_init_geodesic (struct GMT_CTRL *GMT) {
	switch (GMT->current.setting.proj_geodesic) {
		case GMT_GEODESIC_VINCENTY:
			GMT->current.map.geodesic_meter = map_vincenty_dist_meter;
			GMT->current.map.geodesic_az_backaz = map_az_backaz_vincenty;
			break;
		case GMT_GEODESIC_ANDOYER:
			GMT->current.map.geodesic_meter = map_andoyer_dist_meter;
			GMT->current.map.geodesic_az_backaz = map_az_backaz_vincenty;	/* This may change later */
			break;
		case GMT_GEODESIC_RUDOE:
			GMT->current.map.geodesic_meter = map_rudoe_dist_meter;
			GMT->current.map.geodesic_az_backaz = map_az_backaz_rudoe;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "The PROJ_GEODESIC is not set! - use Vincenty\n");
			GMT->current.setting.proj_geodesic = GMT_GEODESIC_VINCENTY;
			GMT->current.map.geodesic_meter = map_vincenty_dist_meter;
			GMT->current.map.geodesic_az_backaz = map_az_backaz_vincenty;
			break;
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "The PROJ_GEODESIC set to %s\n", GEOD_TEXT[GMT->current.setting.proj_geodesic]);
}

/* Datum conversion routines */

/*! . */
void gmt_datum_init (struct GMT_CTRL *GMT, struct GMT_DATUM *from, struct GMT_DATUM *to, bool heights) {
	/* Initialize datum conv structures based on the parsed values*/
	unsigned int k;

	GMT->current.proj.datum.h_given = heights;
	gmt_M_memcpy (&GMT->current.proj.datum.from, from, 1, struct GMT_DATUM);
	gmt_M_memcpy (&GMT->current.proj.datum.to,   to,   1, struct GMT_DATUM);

	GMT->current.proj.datum.da = GMT->current.proj.datum.to.a - GMT->current.proj.datum.from.a;
	GMT->current.proj.datum.df = GMT->current.proj.datum.to.f - GMT->current.proj.datum.from.f;
	for (k = 0; k < 3; k++)
		GMT->current.proj.datum.dxyz[k] = -(GMT->current.proj.datum.to.xyz[k] - GMT->current.proj.datum.from.xyz[k]);	/* Since the X, Y, Z are Deltas relative to WGS-84 */
	GMT->current.proj.datum.one_minus_f = 1.0 - GMT->current.proj.datum.from.f;
}

/*! . */
void gmt_ECEF_init (struct GMT_CTRL *GMT, struct GMT_DATUM *D) {
	/* Duplicate the parsed datum to the GMT from datum */
	gmt_M_memcpy (&GMT->current.proj.datum.from, D, 1, struct GMT_DATUM);
}

/*! . */
int gmt_set_datum (struct GMT_CTRL *GMT, char *text, struct GMT_DATUM *D) {
	int i;
	double t;

	if (text[0] == '\0' || text[0] == '-') {	/* Shortcut for WGS-84 */
		if ((i = gmt_get_ellipsoid (GMT, "WGS-84")) >= 0) {	/* For Coverity's benefit */
			gmt_M_memset (D->xyz, 3, double);
			D->a = GMT->current.setting.ref_ellipsoid[i].eq_radius;
			D->f = GMT->current.setting.ref_ellipsoid[i].flattening;
			D->ellipsoid_id = i;
		}
	}
	else if (strchr (text, ':')) {	/* Has colons, must get ellipsoid and dr separately */
		char ellipsoid[GMT_LEN256] = {""}, dr[GMT_LEN256] = {""};
		if (sscanf (text, "%[^:]:%s", ellipsoid, dr) != 2) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Malformed <ellipsoid>:<dr> argument!\n");
			return (-1);
		}
		if (sscanf (dr, "%lf,%lf,%lf", &D->xyz[GMT_X], &D->xyz[GMT_Y], &D->xyz[GMT_Z]) != 3) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Malformed <x>,<y>,<z> argument!\n");
			return (-1);
		}
		if ((i = gmt_get_ellipsoid (GMT, ellipsoid)) >= 0) {	/* This includes looking for format <a>,<1/f> */
			D->a = GMT->current.setting.ref_ellipsoid[i].eq_radius;
			D->f = GMT->current.setting.ref_ellipsoid[i].flattening;
			D->ellipsoid_id = i;
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Ellipsoid %s not recognized!\n", ellipsoid);
			return (-1);
		}
	}
	else {		/* Gave a Datum ID tag [ 0-(GMT_N_DATUMS-1)] */
		int k;
		if (sscanf (text, "%d", &i) != 1) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Malformed or unrecognized <datum> argument (%s)!\n", text);
			return (-1);
		}
		if (i < 0 || i >= GMT_N_DATUMS) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Datum ID (%d) outside valid range (0-%d)!\n", i, GMT_N_DATUMS-1);
			return (-1);
		}
		if ((k = gmt_get_ellipsoid (GMT, GMT->current.setting.proj_datum[i].ellipsoid)) < 0) {	/* This should not happen... */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Ellipsoid %s not recognized!\n", GMT->current.setting.proj_datum[i].ellipsoid);
			return (-1);
		}
		D->a = GMT->current.setting.ref_ellipsoid[k].eq_radius;
		D->f = GMT->current.setting.ref_ellipsoid[k].flattening;
		D->ellipsoid_id = k;
		for (k = 0; k < 3; k++) D->xyz[k] = GMT->current.setting.proj_datum[i].xyz[k];
	}
	D->b = D->a * (1 - D->f);
	D->e_squared = 2 * D->f - D->f * D->f;
	t = D->a /D->b;
	D->ep_squared = t * t - 1.0;	/* (a^2 - b^2)/b^2 */
	return 0;
}

/*! Compute the Abridged Molodensky transformation (3 parametrs). */
void gmt_conv_datum (struct GMT_CTRL *GMT, double in[], double out[]) {
	/* Evaluate J^-1 and B on from ellipsoid */
	/* Based on Standard Molodensky Datum Conversion, implemented from
	 * http://www.colorado.edu/geography/gcraft/notes/datum/gif/molodens.gif */

	double sin_lon, cos_lon, sin_lat, cos_lat, sin_lat2, M, N, h, tmp_1, tmp_2, tmp_3;
	double delta_lat, delta_lon, delta_h, sc_lat;

	h = (GMT->current.proj.datum.h_given) ? in[GMT_Z] : 0.0;
	sincosd (in[GMT_X], &sin_lon, &cos_lon);
	sincosd (in[GMT_Y], &sin_lat, &cos_lat);
	sin_lat2 = sin_lat * sin_lat;
	sc_lat = sin_lat * cos_lat;
	M = GMT->current.proj.datum.from.a * (1.0 - GMT->current.proj.datum.from.e_squared) / pow (1.0 - GMT->current.proj.datum.from.e_squared * sin_lat2, 1.5);
	N = GMT->current.proj.datum.from.a / sqrt (1.0 - GMT->current.proj.datum.from.e_squared * sin_lat2);

	tmp_1 = -GMT->current.proj.datum.dxyz[GMT_X] * sin_lat * cos_lon - GMT->current.proj.datum.dxyz[GMT_Y] * sin_lat * sin_lon + GMT->current.proj.datum.dxyz[GMT_Z] * cos_lat;
	tmp_2 = GMT->current.proj.datum.da * (N * GMT->current.proj.datum.from.e_squared * sc_lat) / GMT->current.proj.datum.from.a;
	tmp_3 = GMT->current.proj.datum.df * (M / GMT->current.proj.datum.one_minus_f + N * GMT->current.proj.datum.one_minus_f) * sc_lat;
	delta_lat = (tmp_1 + tmp_2 + tmp_3) / (M + h);

	delta_lon = (-GMT->current.proj.datum.dxyz[GMT_X] * sin_lon + GMT->current.proj.datum.dxyz[GMT_Y] * cos_lon) / ((N + h) * cos_lat);

	tmp_1 = GMT->current.proj.datum.dxyz[GMT_X] * cos_lat * cos_lon + GMT->current.proj.datum.dxyz[GMT_Y] * cos_lat * sin_lon + GMT->current.proj.datum.dxyz[GMT_Z] * sin_lat;
	tmp_2 = -GMT->current.proj.datum.da * GMT->current.proj.datum.from.a / N;
	tmp_3 = GMT->current.proj.datum.df * GMT->current.proj.datum.one_minus_f * N * sin_lat2;
	delta_h = tmp_1 + tmp_2 + tmp_3;

	out[GMT_X] = in[GMT_X] + delta_lon * R2D;
	out[GMT_Y] = in[GMT_Y] + delta_lat * R2D;
	if (GMT->current.proj.datum.h_given) out[GMT_Z] = in[GMT_Z] + delta_h;
}

/*! . */
void gmt_ECEF_forward (struct GMT_CTRL *GMT, double in[], double out[]) {
	/* Convert geodetic lon, lat, height to ECEF coordinates given the datum parameters.
	 * GMT->current.proj.datum.from is always the ellipsoid to use */

	double sin_lon, cos_lon, sin_lat, cos_lat, N, tmp;

	sincosd (in[GMT_X], &sin_lon, &cos_lon);
	sincosd (in[GMT_Y], &sin_lat, &cos_lat);

	N = GMT->current.proj.datum.from.a / d_sqrt (1.0 - GMT->current.proj.datum.from.e_squared * sin_lat * sin_lat);
	tmp = (N + in[GMT_Z]) * cos_lat;
	out[GMT_X] = tmp * cos_lon + GMT->current.proj.datum.from.xyz[GMT_X];
	out[GMT_Y] = tmp * sin_lon + GMT->current.proj.datum.from.xyz[GMT_Y];
	out[GMT_Z] = (N * (1 - GMT->current.proj.datum.from.e_squared) + in[GMT_Z]) * sin_lat + GMT->current.proj.datum.from.xyz[GMT_Z];
}

/*! . */
void gmt_ECEF_inverse (struct GMT_CTRL *GMT, double in[], double out[]) {
	/* Convert ECEF coordinates to geodetic lon, lat, height given the datum parameters.
	 * GMT->current.proj.datum.from is always the ellipsoid to use */

	unsigned int i;
	double in_p[3], sin_lat, cos_lat, N, p, theta, sin_theta, cos_theta;

	/* First remove the xyz shifts, us in_p to avoid changing in */
	for (i = 0; i < 3; i++) in_p[i] = in[i] - GMT->current.proj.datum.from.xyz[i];

	p = hypot (in_p[GMT_X], in_p[GMT_Y]);
	theta = atan (in_p[GMT_Z] * GMT->current.proj.datum.from.a / (p * GMT->current.proj.datum.from.b));
	sincos (theta, &sin_theta, &cos_theta);
	out[GMT_X] = d_atan2d (in_p[GMT_Y], in_p[GMT_X]);
	out[GMT_Y] = atand ((in_p[GMT_Z] + GMT->current.proj.datum.from.ep_squared * GMT->current.proj.datum.from.b * pow (sin_theta, 3.0)) / (p - GMT->current.proj.datum.from.e_squared * GMT->current.proj.datum.from.a * pow (cos_theta, 3.0)));
	sincosd (out[GMT_Y], &sin_lat, &cos_lat);
	N = GMT->current.proj.datum.from.a / sqrt (1.0 - GMT->current.proj.datum.from.e_squared * sin_lat * sin_lat);
	out[GMT_Z] = (p / cos_lat) - N;
}

/*! Convert ECEF coordinates to geodetic lon, lat, height given the 'to' parameters. Used in 7 params Bursa-Wolf transform */
void gmt_ECEF_inverse_dest_datum (struct GMT_CTRL *GMT, double in[], double out[]) {
	double sin_lat, cos_lat, N, p, theta, sin_theta, cos_theta;

	p = hypot (in[GMT_X], in[GMT_Y]);
	theta = atan (in[GMT_Z] * GMT->current.proj.datum.to.a / (p * GMT->current.proj.datum.to.b));
	sincos (theta, &sin_theta, &cos_theta);
	out[GMT_X] = d_atan2d (in[GMT_Y], in[GMT_X]);
	out[GMT_Y] = atand ((in[GMT_Z] + GMT->current.proj.datum.to.ep_squared * GMT->current.proj.datum.to.b * pow (sin_theta, 3.0)) / (p - GMT->current.proj.datum.to.e_squared * GMT->current.proj.datum.to.a * pow (cos_theta, 3.0)));
	sincosd (out[GMT_Y], &sin_lat, &cos_lat);
	N = GMT->current.proj.datum.to.a / sqrt (1.0 - GMT->current.proj.datum.to.e_squared * sin_lat * sin_lat);
	out[GMT_Z] = (p / cos_lat) - N;
}

/*! . */
double gmt_line_length (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, bool project) {
	/* Returns distance of line in units set by GMT_distaz. It bypassed points where x and/or y are NaN. */
	uint64_t this_p, prev;
	bool xy_not_NaN;
	double cum_dist = 0.0, xp0 = 0, xp1, yp0 = 0, yp1;

	if (n == 0) return 0.0;
	if (project) gmt_geo_to_xy (GMT, x[0], y[0], &xp0, &yp0);
	for (this_p = 1, prev = 0; this_p < n; this_p++) {
		xy_not_NaN = !(gmt_M_is_dnan (x[this_p]) || gmt_M_is_dnan (y[this_p]));
		if (xy_not_NaN)	{	/* safe to calculate inc */
			if (project) {
				gmt_geo_to_xy (GMT, x[this_p], y[this_p], &xp1, &yp1);
				cum_dist += hypot (xp0 - xp1, yp0 - yp1);
				xp0 = xp1;	yp0 = yp1;
			}
			else
				cum_dist += gmt_distance (GMT, x[this_p], y[this_p], x[prev], y[prev]);
			prev = this_p;	/* This was a record with OK x,y; make it the previous point for distance calculations */
		}
	}
	if (project) cum_dist *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];

	return (cum_dist);
}

/*! . */
double *gmt_dist_array (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, bool cumulative) {
	/* Returns distances in units set by GMT_distaz. It bypassed points where x and/or y are NaN.
	 * If cumulative is false we just return the increments; otherwise we add up distances */
	uint64_t this_p, prev;
	bool xy_not_NaN;
	double *d = NULL, cum_dist = 0.0, inc = 0.0;

	if (n == 0) return (NULL);
	d = gmt_M_memory (GMT, NULL, n, double);
	if (gmt_M_is_dnan (x[0]) || gmt_M_is_dnan (y[0])) d[0] = GMT->session.d_NaN;
	for (this_p = 1, prev = 0; this_p < n; this_p++) {
		xy_not_NaN = !(gmt_M_is_dnan (x[this_p]) || gmt_M_is_dnan (y[this_p]));
		if (xy_not_NaN) {	/* safe to calculate inc */
			inc = gmt_distance (GMT, x[this_p], y[this_p], x[prev], y[prev]);
			if (cumulative) {
				cum_dist += inc;
				d[this_p] = cum_dist;
			}
			else
				d[this_p] = inc;
		}
		else
			d[this_p] = GMT->session.d_NaN;

		if (xy_not_NaN) prev = this_p;	/* This was a record with OK x,y; make it the previous point for distance calculations */
	}
	return (d);
}

/*! . */
double * gmt_dist_array_2 (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, double scale, int dist_flag) {
	/* Returns distances in meter; use scale to get other units */
	uint64_t this_p, prev;
	bool cumulative = true, do_scale, xy_not_NaN;
	double *d = NULL, cum_dist = 0.0, inc = 0.0;

	if (dist_flag < 0) {	/* Want increments and not cumulative distances */
		dist_flag = abs (dist_flag);
		cumulative = false;
	}

	if (dist_flag < 0 || dist_flag > 3) return (NULL);

	do_scale = (scale != 1.0);
	d = gmt_M_memory (GMT, NULL, n, double);
	if (gmt_M_is_dnan (x[0]) || gmt_M_is_dnan (y[0])) d[0] = GMT->session.d_NaN;
	for (this_p = 1, prev = 0; this_p < n; this_p++) {
		xy_not_NaN = !(gmt_M_is_dnan (x[this_p]) || gmt_M_is_dnan (y[this_p]));
		if (xy_not_NaN) {	/* safe to calculate inc */
			switch (dist_flag) {

				case 0:	/* Cartesian distances */

					inc = hypot (x[this_p] - x[prev], y[this_p] - y[prev]);
					break;

				case 1:	/* Flat earth distances in meter */

					inc = map_flatearth_dist_meter (GMT, x[this_p], y[this_p], x[prev], y[prev]);
					break;

				case 2:	/* Great circle distances in meter */

					inc = gmt_great_circle_dist_meter (GMT, x[this_p], y[this_p], x[prev], y[prev]);
					break;

				case 3:	/* Geodesic distances in meter */

					inc = (*GMT->current.map.geodesic_meter) (GMT, x[this_p], y[this_p], x[prev], y[prev]);
					break;
			}

			if (do_scale) inc *= scale;
			if (cumulative) cum_dist += inc;
			d[this_p] = (cumulative) ? cum_dist : inc;
		}
		else
			d[this_p] = GMT->session.d_NaN;

		if (xy_not_NaN) prev = this_p;	/* This was a record with OK x,y; make it the previous point for distance calculations */
	}
	return (d);
}

/*! . */
unsigned int gmtlib_map_latcross (struct GMT_CTRL *GMT, double lat, double west, double east, struct GMT_XINGS **xings) {
	bool go = false;
	unsigned int i, nx, nc = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	double lon, lon_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	struct GMT_XINGS *X = NULL;


	X = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_XINGS);

	lon_old = west - 2.0 * GMT_CONV4_LIMIT;
	gmt_map_outside (GMT, lon_old, lat);
	gmt_geo_to_xy (GMT, lon_old, lat, &last_x, &last_y);
	for (i = 1; i <= GMT->current.map.n_lon_nodes; i++) {
		lon = (i == GMT->current.map.n_lon_nodes) ? east + 2.0 * GMT_CONV4_LIMIT : west + i * GMT->current.map.dlon;
		gmt_map_outside (GMT, lon, lat);
		gmt_geo_to_xy (GMT, lon, lat, &this_x, &this_y);
		if ((nx = map_crossing (GMT, lon_old, lat, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			X[nc].angle[0] = gmt_get_angle (GMT, lon_old, lat, lon, lat);	/* Get angle at first crossing */
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;	/* If a 2nd crossing it must be really close so just add 180 */
			if (GMT->current.map.corner > 0) {
				X[nc].sides[0] = (GMT->current.map.corner%4 > 1) ? 1 : 3;
				if (GMT->current.proj.got_azimuths) X[nc].sides[0] = (X[nc].sides[0] + 2) % 4;
				GMT->current.map.corner = 0;
			}
		}
		else if (GMT->current.map.is_world)	/* Deal with possibility of wrapping around 360 */
			nx = (*GMT->current.map.wrap_around_check) (GMT, X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && fabs (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT->current.map.width) < GMT_CONV4_LIMIT && !GMT->current.map.is_world)
			/* I assume this means if we have a crossing and both left and right and not worldmap then skip as something went wrong? */
			go = false;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_CONV4_LIMIT && fabs (gap - GMT->current.map.height) < GMT_CONV4_LIMIT && !GMT->current.map.is_world_tm)
			/* I assume this means if we have a crossing and both top and bottom and it is not a global UTM map then skip as something went wrong? */
			go = false;
		else if (nx > 0)
			go = true;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = gmt_M_memory (GMT, X, n_alloc, struct GMT_XINGS);
			}
			go = false;
		}
		lon_old = lon;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = gmt_M_memory (GMT, X, nc, struct GMT_XINGS);
		*xings = X;
	}
	else
		gmt_M_free (GMT, X);

	return (nc);
}

/*! . */
unsigned int gmtlib_map_loncross (struct GMT_CTRL *GMT, double lon, double south, double north, struct GMT_XINGS **xings) {
	bool go = false;
	unsigned int j, nx, nc = 0;
	size_t n_alloc = GMT_SMALL_CHUNK;
	double lat, lat_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	struct GMT_XINGS *X = NULL;

	X = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_XINGS);

	lat_old = ((south - (2.0 * GMT_CONV4_LIMIT)) >= -90.0) ? south - 2.0 * GMT_CONV4_LIMIT : south;	/* Outside */
	if ((north + 2.0 * GMT_CONV4_LIMIT) <= 90.0) north += 2.0 * GMT_CONV4_LIMIT;
	gmt_map_outside (GMT, lon, lat_old);
	gmt_geo_to_xy (GMT, lon, lat_old, &last_x, &last_y);
	for (j = 1; j <= GMT->current.map.n_lat_nodes; j++) {
		lat = (j == GMT->current.map.n_lat_nodes) ? north: south + j * GMT->current.map.dlat;
		gmt_map_outside (GMT, lon, lat);
		gmt_geo_to_xy (GMT, lon, lat, &this_x, &this_y);
		if ((nx = map_crossing (GMT, lon, lat_old, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			X[nc].angle[0] = gmt_get_angle (GMT, lon, lat_old, lon, lat);	/* Get angle at first crossing */
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;	/* If a 2nd crossing it must be really close so just add 180 */
			if (GMT->current.map.corner > 0) {
				X[nc].sides[0] = (GMT->current.map.corner < 3) ? 0 : 2;
				GMT->current.map.corner = 0;
			}
		}
		else if (GMT->current.map.is_world)	/* Deal with possibility of wrapping around 360 */
			nx = (*GMT->current.map.wrap_around_check) (GMT, X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && fabs (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT->current.map.width) < GMT_CONV4_LIMIT && !GMT->current.map.is_world)
			/* I assume this means if we have a crossing and both left and right and not worldmap then skip as something went wrong? */
			go = false;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_CONV4_LIMIT && fabs (gap - GMT->current.map.height) < GMT_CONV4_LIMIT && !GMT->current.map.is_world_tm)
			/* I assume this means if we have a crossing and both top and bottom and it is not a global UTM map then skip as something went wrong? */
			go = false;
		else if (nx > 0)
			go = true;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = gmt_M_memory (GMT, X, n_alloc, struct GMT_XINGS);
			}
			go = false;
		}
		lat_old = lat;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = gmt_M_memory (GMT, X, nc, struct GMT_XINGS);
		*xings = X;
	}
	else
		gmt_M_free (GMT, X);

	return (nc);
}

int gmt_proj_setup (struct GMT_CTRL *GMT, double wesn[]) {
	/* Core map setup function for the projection.  All program needing projection support
	 * will call this either directly or indirectly.  Programs that _just_ need to project
	 * to/from may call this function only, while all the plotting program need to have more
	 * things initialized and they will call gmt_map_setup instead, which starts by calling
	 * gmt_proj_setup first. */

	bool search = false;

	if (wesn[XHI] == wesn[XLO] && wesn[YHI] == wesn[YLO]) Return (GMT_MAP_NO_REGION);	/* Since -R may not be involved if there are grids */

	if (!GMT->common.J.active) {
		char *def_args[2] = {"X15c", "Q15c"};
		unsigned int geo;
		if (GMT->current.setting.run_mode == GMT_CLASSIC)	/* This is a fatal error in classic mode */
			Return (GMT_MAP_NO_PROJECTION);
		if (!GMT->current.ps.active)
			Return (GMT_MAP_NO_PROJECTION);	/* Only auto-setup a projection for mapping and plots */
		/* Here we are in modern mode starting a new plot without a map projection.  The rules says use -JQ15c (geo) or -JX15c (Cartesian) */
		geo = gmt_M_is_geographic (GMT, GMT_IN);
		gmt_parse_common_options (GMT, "J", 'J', def_args[geo]);
		GMT->common.J.active = true;
	}

	gmtlib_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid since the latter could have been set explicitly */

	if (gmt_M_x_is_lon (GMT, GMT_IN)) {
		/* Limit east-west range to 360 and make sure east > -180 and west < 360 */
		if (!GMT->common.R.oblique) {	/* Only makes sense if not corner coordinates */
			if (wesn[XHI] < wesn[XLO]) wesn[XHI] += 360.0;
			if ((fabs (wesn[XHI] - wesn[XLO]) - 360.0) > GMT_CONV4_LIMIT) Return (GMT_MAP_EXCEEDS_360);
		}
		while (wesn[XHI] < -180.0) {
			wesn[XLO] += 360.0;
			wesn[XHI] += 360.0;
		}
		while (wesn[XLO] > 360.0) {
			wesn[XLO] -= 360.0;
			wesn[XHI] -= 360.0;
		}
	}
	else if (gmt_M_y_is_lon (GMT, GMT_IN)) {
		/* Limit east-west range to 360 and make sure east > -180 and west < 360 */
		if (wesn[YHI] < wesn[YLO]) wesn[YHI] += 360.0;
		if ((fabs (wesn[YHI] - wesn[YLO]) - 360.0) > GMT_CONV4_LIMIT) Return (GMT_MAP_EXCEEDS_360);
		while (wesn[YHI] < -180.0) {
			wesn[YLO] += 360.0;
			wesn[YHI] += 360.0;
		}
		while (wesn[YLO] > 360.0) {
			wesn[YLO] -= 360.0;
			wesn[YHI] -= 360.0;
		}
	}
	if (GMT->current.proj.got_elevations) {
		if (wesn[YLO] < 0.0 || wesn[YLO] >= 90.0) Return (GMT_MAP_BAD_ELEVATION_MIN);
		if (wesn[YHI] <= 0.0 || wesn[YHI] > 90.0) Return (GMT_MAP_BAD_ELEVATION_MAX);
	}
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {
		if (wesn[YLO] < -90.0 || wesn[YLO] > 90.0) Return (GMT_MAP_BAD_LAT_MIN);
		if (wesn[YHI] < -90.0 || wesn[YHI] > 90.0) Return (GMT_MAP_BAD_LAT_MAX);
	}
	else if (gmt_M_x_is_lat (GMT, GMT_IN)) {
		if (wesn[XLO] < -90.0 || wesn[XLO] > 90.0) Return (GMT_MAP_BAD_LAT_MIN);
		if (wesn[XHI] < -90.0 || wesn[XHI] > 90.0) Return (GMT_MAP_BAD_LAT_MAX);
	}

	if (GMT->common.R.wesn != wesn)		/* In many cases they are both copies of same pointer */
		gmt_M_memcpy (GMT->common.R.wesn, wesn, 4, double);
	GMT->current.proj.GMT_convert_latitudes = false;
	if (GMT->current.proj.gave_map_width) GMT->current.proj.units_pr_degree = false;

	GMT->current.map.meridian_straight = GMT->current.map.parallel_straight = 0;
	GMT->current.map.n_lon_nodes = GMT->current.map.n_lat_nodes = 0;
	GMT->current.map.wrap_around_check = &map_wrap_around_check_x;
	GMT->current.map.jump = &map_jump_x;
	GMT->current.map.will_it_wrap = &map_will_it_wrap_x;
#if 0
	GMT->current.map.this_point_wraps = &map_this_point_wraps_x;
#endif
	GMT->current.map.get_crossings = &map_get_crossings_x;

	GMT->current.map.lon_wrap = true;

	switch (GMT->current.proj.projection) {

		case GMT_LINEAR:		/* Linear transformations */
			search = map_init_linear (GMT);
			break;

		case GMT_POLAR:		/* Both lon/lat are actually theta, radius */
			search = map_init_polar (GMT);
			break;

		case GMT_MERCATOR:		/* Standard Mercator projection */
			search = map_init_merc (GMT);
			break;

		case GMT_STEREO:		/* Stereographic projection */
			search = map_init_stereo (GMT);
			break;

		case GMT_LAMBERT:		/* Lambert Conformal Conic */
			search = map_init_lambert (GMT);
			break;

		case GMT_OBLIQUE_MERC:	/* Oblique Mercator */
			search = map_init_oblique (GMT);
			break;

		case GMT_TM:		/* Transverse Mercator */
			search = map_init_tm (GMT);
			break;

		case GMT_UTM:		/* Universal Transverse Mercator */
			search = map_init_utm (GMT);
			break;

		case GMT_LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */
			search = map_init_lambeq (GMT);
			break;

		case GMT_ORTHO:		/* Orthographic Projection */
			search = map_init_ortho (GMT);
			break;

		case GMT_GENPER:		/* General Perspective Projection */
			search = map_init_genper (GMT);
			break;

		case GMT_AZ_EQDIST:		/* Azimuthal Equal-Distance Projection */
			search = map_init_azeqdist (GMT);
			break;

		case GMT_GNOMONIC:		/* Azimuthal Gnomonic Projection */
			search = map_init_gnomonic (GMT);
			break;

		case GMT_MOLLWEIDE:		/* Mollweide Equal-Area */
			search = map_init_mollweide (GMT);
			break;

		case GMT_HAMMER:		/* Hammer-Aitoff Equal-Area */
			search = map_init_hammer (GMT);
			break;

		case GMT_VANGRINTEN:		/* Van der Grinten */
			search = map_init_grinten (GMT);
			break;

		case GMT_WINKEL:		/* Winkel Tripel */
			search = map_init_winkel (GMT);
			break;

		case GMT_ECKERT4:		/* Eckert IV */
			search = map_init_eckert4 (GMT);
			break;

		case GMT_ECKERT6:		/* Eckert VI */
			search = map_init_eckert6 (GMT);
			break;

		case GMT_CYL_EQ:		/* Cylindrical Equal-Area */
			search = map_init_cyleq (GMT);
			break;

		case GMT_CYL_STEREO:			/* Cylindrical Stereographic */
			search = map_init_cylstereo (GMT);
			break;

		case GMT_MILLER:		/* Miller Cylindrical */
			search = map_init_miller (GMT);
			break;

		case GMT_CYL_EQDIST:	/* Cylindrical Equidistant */
			search = map_init_cyleqdist (GMT);
			break;

		case GMT_ROBINSON:		/* Robinson */
			search = map_init_robinson (GMT);
			break;

		case GMT_SINUSOIDAL:	/* Sinusoidal Equal-Area */
			search = map_init_sinusoidal (GMT);
			break;

		case GMT_CASSINI:		/* Cassini cylindrical */
			search = map_init_cassini (GMT);
			break;

		case GMT_ALBERS:		/* Albers Equal-Area Conic */
			search = map_init_albers (GMT);
			break;

		case GMT_ECONIC:		/* Equidistant Conic */
			search = map_init_econic (GMT);
			break;

		case GMT_POLYCONIC:		/* Polyconic */
			search = map_init_polyconic (GMT);
			break;

#ifdef HAVE_GDAL
		case GMT_PROJ4_PROJS:	/* All proj.4 projections */
			search = map_init_proj4 (GMT);
			break;
#endif

		default:	/* No projection selected, return to a horrible death */
			Return (GMT_MAP_NO_PROJECTION);
	}

	if (GMT->current.proj.fwd == NULL)	/* Some wrror in projection projection parameters, return to a horrible death */
		Return(GMT_MAP_NO_PROJECTION);

	GMT->current.proj.search = search;

	GMT->current.proj.i_scale[GMT_X] = (GMT->current.proj.scale[GMT_X] != 0.0) ? 1.0 / GMT->current.proj.scale[GMT_X] : 1.0;
	GMT->current.proj.i_scale[GMT_Y] = (GMT->current.proj.scale[GMT_Y] != 0.0) ? 1.0 / GMT->current.proj.scale[GMT_Y] : 1.0;
	GMT->current.proj.i_scale[GMT_Z] = (GMT->current.proj.scale[GMT_Z] != 0.0) ? 1.0 / GMT->current.proj.scale[GMT_Z] : 1.0;

	GMT->current.map.width  = fabs (GMT->current.proj.rect[XHI] - GMT->current.proj.rect[XLO]);
	GMT->current.map.height = fabs (GMT->current.proj.rect[YHI] - GMT->current.proj.rect[YLO]);
	GMT->current.map.half_width  = 0.5 * GMT->current.map.width;
	GMT->current.map.half_height = 0.5 * GMT->current.map.height;

	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* x is longitude */
		if (GMT->current.proj.central_meridian < GMT->common.R.wesn[XLO] && (GMT->current.proj.central_meridian + 360.0) <= GMT->common.R.wesn[XHI]) GMT->current.proj.central_meridian += 360.0;
		if (GMT->current.proj.central_meridian > GMT->common.R.wesn[XHI] && (GMT->current.proj.central_meridian - 360.0) >= GMT->common.R.wesn[XLO]) GMT->current.proj.central_meridian -= 360.0;
	}

	if (!GMT->current.map.n_lon_nodes) GMT->current.map.n_lon_nodes = urint (GMT->current.map.width / GMT->current.setting.map_line_step);
	if (!GMT->current.map.n_lat_nodes) GMT->current.map.n_lat_nodes = urint (GMT->current.map.height / GMT->current.setting.map_line_step);

	map_init_three_D (GMT);

	return (GMT_NOERROR);
}

/*! . */
int gmt_map_setup (struct GMT_CTRL *GMT, double wesn[]) {
	unsigned int i;
	bool search, double_auto[6];
	double scale, i_scale;

	if ((i = gmt_proj_setup (GMT, wesn)) != GMT_NOERROR) return (i);

	search = GMT->current.proj.search;

	/* If intervals are not set specifically, round them to some "nice" values
	 * Remember whether frame items in both directions were automatically set */
	for (i = 0; i < 6; i++)
		double_auto[i] = gmt_M_is_geographic (GMT, GMT_IN) && GMT->current.map.frame.set_both &&
		GMT->current.map.frame.axis[GMT_X].item[i].active && GMT->current.map.frame.axis[GMT_X].item[i].interval == 0.0 &&
		GMT->current.map.frame.axis[GMT_Y].item[i].active && GMT->current.map.frame.axis[GMT_Y].item[i].interval == 0.0;

	gmt_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_UPPER);
	gmt_auto_frame_interval (GMT, GMT_Y, GMT_ANNOT_UPPER);
	gmt_auto_frame_interval (GMT, GMT_Z, GMT_ANNOT_UPPER);
	gmt_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_LOWER);
	gmt_auto_frame_interval (GMT, GMT_Y, GMT_ANNOT_LOWER);
	gmt_auto_frame_interval (GMT, GMT_Z, GMT_ANNOT_LOWER);

	/* Now set the pairs of automatically set intervals to be the same in both x- and y-direction */
	for (i = 0; i < 6; i++) {
		if (double_auto[i]) GMT->current.map.frame.axis[GMT_X].item[i].interval = GMT->current.map.frame.axis[GMT_Y].item[i].interval =
		MAX (GMT->current.map.frame.axis[GMT_X].item[i].interval, GMT->current.map.frame.axis[GMT_Y].item[i].interval);
	}

	if (!GMT->current.map.n_lon_nodes) GMT->current.map.n_lon_nodes = urint (GMT->current.map.width / GMT->current.setting.map_line_step);
	if (!GMT->current.map.n_lat_nodes) GMT->current.map.n_lat_nodes = urint (GMT->current.map.height / GMT->current.setting.map_line_step);

	GMT->current.map.dlon = (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) / GMT->current.map.n_lon_nodes;
	GMT->current.map.dlat = (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) / GMT->current.map.n_lat_nodes;

	if (GMT->current.map.width > 400.0 && gmt_M_is_grdmapproject (GMT)) {	/* ***project calling with true scale, probably  */
		search = false;	/* Safe-guard that prevents region search below for (map|grd)project and others (400 inch = ~> 10 meters) */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_map_setup perimeter search skipped when using true scale with grdproject or mapproject.\n");
	}

	if (search) {	/* Loop around rectangular perimeter and determine min/max lon/lat extent */
		if (GMT->current.proj.projection_GMT == GMT_GENPER)	/* Need special considerations for this projection */
			gmtmap_genper_search (GMT, &GMT->common.R.wesn[XLO], &GMT->common.R.wesn[XHI], &GMT->common.R.wesn[YLO], &GMT->common.R.wesn[YHI]);
		else	/* Search along the projected boarder */
			gmt_wesn_search (GMT, GMT->current.proj.rect[XLO], GMT->current.proj.rect[XHI], GMT->current.proj.rect[YLO], GMT->current.proj.rect[YHI], &GMT->common.R.wesn[XLO], &GMT->common.R.wesn[XHI], &GMT->common.R.wesn[YLO], &GMT->common.R.wesn[YHI]);
		GMT->current.map.dlon = (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) / GMT->current.map.n_lon_nodes;
		GMT->current.map.dlat = (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) / GMT->current.map.n_lat_nodes;
		if (gmt_M_is_azimuthal(GMT) && GMT->common.R.oblique) map_horizon_search (GMT, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], GMT->current.proj.rect[XLO], GMT->current.proj.rect[XHI], GMT->current.proj.rect[YLO], GMT->current.proj.rect[YHI]);
	}

	/* Maximum step size (in degrees) used for interpolation of line segments along great circles (or meridians/parallels)  before they are plotted */
	GMT->current.map.path_step = GMT->current.setting.map_line_step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;

	i_scale = 1.0 / (0.0254 * GMT->current.proj.scale[GMT_X]);
	scale = 0.001 / (GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit] * GMT->current.proj.scale[GMT_X]);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Map scale is %g km per %s or 1:%g.\n",
		scale, GMT->session.unit_name[GMT->current.setting.proj_length_unit], i_scale);

	return (GMT_NOERROR);
}

/*! . */
unsigned int gmt_init_distaz (struct GMT_CTRL *GMT, char unit, unsigned int mode, unsigned int type) {
	/* Initializes distance calcuation given the selected values for:
	 * Distance unit: must be on of the following:
	 *  1) d|e|f|k|m|M|n|s
	 *  2) GMT (Cartesian distance after projecting with -J) | X (Cartesian)
	 *  3) S (cosine distance) | P (cosine after first inverse projecting with -J)
	 * distance-calculation modifier mode flags are:
	 *   0 (Cartesian), 1 (flat Earth), 2 (great-circle [Default]), 3 (geodesic), 4 (loxodrome)
	 *   However, if -j<mode> is set it takes precedence.
	 * type: 0 = map distances, 1 = contour distances, 2 = contour annotation distances
	 * We set distance and azimuth functions and scales for this type.
	 * At the moment there is only one azimuth function pointer for all.
	 *
	 * The input args for gmt_init_distaz normally comes from calling gmt_get_distance.
	 */

	unsigned int proj_type = GMT_GEOGRAPHIC;	/* Default is to just use the geographic coordinates as they are */

	if (strchr (GMT_LEN_UNITS, unit) && gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Want geographic distance units but -fg (or -J) not set */
		gmt_parse_common_options (GMT, "f", 'f', "g");
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Your distance unit (%c) implies geographic data; -fg has been set.\n", unit);
	}

	/* Determine if -j selected a particular mode */
	if (gmt_M_is_geographic (GMT, GMT_IN) && GMT->common.j.active) {	/* User specified a -j setting */
		static char *kind[5] = {"Cartesian", "Flat Earth", "Great Circle", "Geodesic", "Loxodrome"};
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Spherical distance calculation mode: %s.\n", kind[GMT->common.j.active]);
		if (mode != GMT_GREATCIRCLE)	/* We override a selection due to deprecated leading -|+ signs before increment or radius */
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your distance mode (%s) differs from your -j option (%s) which takes precedence.\n", kind[mode], kind[GMT->common.j.active]);
		mode = GMT->common.j.mode;	/* Override with what -j said */
	}

	switch (unit) {
			/* First the three arc angular distance units */

		case 'd':	/* Arc degrees on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_DEG + mode, type, "arc-degree");
			GMT->current.map.dist[type].arc = true;	/* Angular measure */
			break;
		case 'm':	/* Arc minutes on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_DEG + mode, type, "arc-minute");
			GMT->current.map.dist[type].scale = GMT_DEG2MIN_F;
			GMT->current.map.dist[type].arc = true;	/* Angular measure */
			break;
		case 's':	/* Arc seconds on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_DEG + mode, type, "arc-second");
			GMT->current.map.dist[type].scale = GMT_DEG2SEC_F;
			GMT->current.map.dist[type].arc = true;	/* Angular measure */
			break;

			/* Various distance units on the planetary body */

		case 'e':	/* Meters on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "meter");
			break;
		case 'f':	/* Feet on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "foot");
			GMT->current.map.dist[type].scale = 1.0 / METERS_IN_A_FOOT;
			break;
		case 'k':	/* Kilometers on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "km");
			GMT->current.map.dist[type].scale = 1.0 / METERS_IN_A_KM;
			break;
		case 'M':	/* Statute Miles on spherical body using desired metric mode  */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "mile");
			GMT->current.map.dist[type].scale = 1.0 / METERS_IN_A_MILE;
			break;
		case 'n':	/* Nautical miles on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "nautical mile");
			GMT->current.map.dist[type].scale = 1.0 / METERS_IN_A_NAUTICAL_MILE;
			break;
		case 'u':	/* Survey feet on spherical body using desired metric mode */
			map_set_distaz (GMT, GMT_DIST_M + mode, type, "survey feet");
			GMT->current.map.dist[type].scale = 1.0 / METERS_IN_A_SURVEY_FOOT;
			break;

			/* Cartesian distances.  Note: The X|C|R|Z|S|P 'units' are only passed internally and are not available as user selections directly */

		case 'X':	/* Cartesian distances in user units */
			proj_type = GMT_CARTESIAN;
			if (GMT->common.n.periodic[GMT_X] || GMT->common.n.periodic[GMT_Y])
				map_set_distaz (GMT, GMT_CARTESIAN_DIST_PERIODIC, type, "");
			else
				map_set_distaz (GMT, GMT_CARTESIAN_DIST, type, "");
			break;
		case 'C':	/* Cartesian distances (in PROJ_LENGTH_UNIT) after first projecting input coordinates with -J */
			map_set_distaz (GMT, GMT_CARTESIAN_DIST_PROJ, type, "");
			proj_type = GMT_GEO2CART;
			break;

		case 'R':	/* Cartesian distances squared in user units */
			proj_type = GMT_CARTESIAN;
			map_set_distaz (GMT, GMT_CARTESIAN_DIST2, type, "");
			break;
		case 'Z':	/* Cartesian distances squared (in PROJ_LENGTH_UNIT^2) after first projecting input coordinates with -J */
			map_set_distaz (GMT, GMT_CARTESIAN_DIST_PROJ2, type, "");
			proj_type = GMT_GEO2CART;
			break;

			/* Specialized cosine distances used internally only (e.g., greenspline) */

		case 'S':	/* Spherical cosine distances (for various gridding functions) */
			map_set_distaz (GMT, GMT_DIST_COS + mode, type, "");
			break;
		case 'P':	/* Spherical distances after first inversily projecting Cartesian coordinates with -J */
			map_set_distaz (GMT, GMT_CARTESIAN_DIST_PROJ_INV, type, "");
			proj_type = GMT_CART2GEO;
			break;

		default:
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Distance units must be one of %s\n", GMT_LEN_UNITS_DISPLAY);
			GMT_exit (GMT, GMT_NOT_A_VALID_TYPE); return GMT_NOT_A_VALID_TYPE;
			break;
	}

	GMT->current.map.dist[type].init = true;	/* OK, we have now initialized the info for this type */
	return (proj_type);
}

/*! . */
struct GMT_DATASEGMENT * gmt_get_smallcircle (struct GMT_CTRL *GMT, double plon, double plat, double colat, uint64_t m) {
	/* Function to generate m equidistant coordinates for an oblique small circle about a given pole.
	 * To avoid some downstream problems we make sure that if circle crosses Greenwhich or Dateline that
	 * we insert a point at lon = 0 and/or lon = 180. I imagine that in the worst-case scenario we could
	 * cross those lines twice each, hence I allocate 4 more spaces than needed. */
	double P[3], X[3], N[3], R[3][3], xlat, dlon, xx, yy, x, y, last_x = 0.0, last_y = 0.0, dx;
	uint64_t k, n;
	struct GMT_DATASEGMENT *S = NULL;
	if (m < 2) return NULL;

	S = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, m+4, 2, NULL, NULL);	/* The output segment - allocate array space (+ 4 extra) */
	plat = gmt_lat_swap (GMT, plat, GMT_LATSWAP_G2O);	/* Convert to geocentric coordinates */
	gmt_geo_to_cart (GMT, plat, plon, P, true);		/* Get pole vector P */
	xlat = (plat > 0.0) ? plat - colat : plat + colat;	/* Starting point along meridian through P but colat degrees away */
	xlat = gmt_lat_swap (GMT, xlat, GMT_LATSWAP_G2O);	/* Convert to geocentric */
	gmt_geo_to_cart (GMT, xlat, plon, X, true);		/* Generating vector X we will rotate about P */
	dlon = 360.0 / (m - 1);					/* Point spacing along the small circle in oblique longitude degrees */
	for (k = n = 0; k < m; k++, n++) {	/* Given how dlon was set we end up closing the polygon exactly */
		gmt_make_rot_matrix2 (GMT, P, k * dlon, R);	/* Rotation matrix about P for this increment in oblique longitude */
		gmt_matrix_vect_mult (GMT, 3U, R, X, N);	/* Rotate the X-vector to N */
		gmt_cart_to_geo (GMT, &y, &x, N, true);		/* Recover lon,lat of rotated point */
		y = gmt_lat_swap (GMT, y, GMT_LATSWAP_O2G);	/* Convert back to geodetic coordinates */
		if (k) {
			dx = x - last_x;
			if (fabs (dx) > 180.0) {	/* Jump in longitudes */
				dx = copysign (360.0 - fabs (dx), -dx);
				xx = (fabs (last_x) > 270 || fabs(last_x) < 90.0) ? 0.0 : copysign (180.0, last_x);
				yy = last_y + (y - last_y) * (xx - last_x) / dx;
				S->data[GMT_X][n] = xx;	S->data[GMT_Y][n++] = yy;	/* Assign the extra element */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_get_smallcircle: Added extra point at %g/%g\n", xx, yy);
			}
			else if ((x > 0.0 && last_x < 0.0) || (x < 0.0 && last_x > 0.0)) {	/* Crossing Greenwhich */
				xx = 0.0;
				yy = last_y + (y - last_y) * (xx - last_x) / dx;
				S->data[GMT_X][n] = xx;	S->data[GMT_Y][n++] = yy;	/* Assign the extra element */
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "gmt_get_smallcircle: Added extra point at %g/%g\n", xx, yy);
			}
		}
		S->data[GMT_X][n] = x;	S->data[GMT_Y][n] = y;	/* Assign the array elements */
		last_x = x;	last_y = y;
	}
	S->data[GMT_X][n - 1] = S->data[GMT_X][0];		/* Make really sure the polygon is closed */
	S->data[GMT_Y][n - 1] = S->data[GMT_Y][0];
	S->n_rows = n;
	gmt_set_seg_polar (GMT, S);				/* Prepare if a polar cap */
	return (S);	/* Pass out the results */
}

GMT_LOCAL void ellipse_point (struct GMT_CTRL *GMT, double lon, double lat, double center, double sinp, double cosp, double major, double minor, double cos_azimuth, double sin_azimuth, double angle, double *plon, double *plat)
{
	/* Lon, lat is center of ellipse, our point is making an angle with the major axis. */
	double x, y, x_prime, y_prime, rho, c, sin_c, cos_c;
	sincos (angle, &y, &x);
	x *= major;	y *= minor;
	/* Get rotated coordinates in m */
	x_prime = x * cos_azimuth - y * sin_azimuth;
	y_prime = x * sin_azimuth + y * cos_azimuth;
	/* Convert m back to lon lat */
	rho = hypot (x_prime, y_prime);
	c = rho / GMT->current.proj.EQ_RAD;
	sincos (c, &sin_c, &cos_c);
	*plat = d_asind (cos_c * sinp + (y_prime * sin_c * cosp / rho));
	if ((lat - 90.0) > -GMT_CONV8_LIMIT)	/* origin in Northern hemisphere */
		*plon = lon + d_atan2d (x_prime, -y_prime);
	else if ((lat + 90.0) < GMT_CONV8_LIMIT)	/* origin in Southern hemisphere */
		*plon = lon + d_atan2d (x_prime, y_prime);
	else
		*plon = lon + d_atan2d (x_prime * sin_c, (rho * cosp * cos_c - y_prime * sinp * sin_c));
	while ((*plon - center) < -180.0) *plon += 360.0;
	while ((*plon - center) > +180.0) *plon -= 360.0;
}

#define GMT_ELLIPSE_APPROX 72

struct GMT_DATASEGMENT * gmt_get_geo_ellipse (struct GMT_CTRL *GMT, double lon, double lat, double major_km, double minor_km, double azimuth, uint64_t m) {
	/* gmt_get_geo_ellipse takes the location, axes (in km), and azimuth of the ellipse's major axis
	   and computes coordinates for an approximate ellipse using N-sided polygon.
	   If m > 0 then we l\set N = m and use that many points.  Otherwise (m == 0), we will
	   determine N by looking at the spacing between successive points for a trial N and
	   then scale N up or down to satisfy the minimum point spacing criteria. */

	uint64_t i, N;
	double delta_azimuth, sin_azimuth, cos_azimuth, sinp, cosp, ax, ay, axx, ayy, bx, by, bxx, byy, L;
	double major, minor, center, *px = NULL, *py = NULL;
	char header[GMT_LEN256] = {""};
	struct GMT_DATASEGMENT *S = NULL;

	major = major_km * 500.0, minor = minor_km * 500.0;	/* Convert to meters (x1000) of semi-major (/2) and semi-minor axes */
	/* Set up local azimuthal equidistant projection */
	sincosd (90.0 - azimuth, &sin_azimuth, &cos_azimuth);
	sincosd (lat, &sinp, &cosp);

	center = (GMT->current.proj.central_meridian < GMT->common.R.wesn[XLO] || GMT->current.proj.central_meridian > GMT->common.R.wesn[XHI]) ? 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]) : GMT->current.proj.central_meridian;

	if (m == 0) {	/* Determine N */
		delta_azimuth = 2.0 * M_PI / GMT_ELLIPSE_APPROX;	/* Initial guess of angular spacing */
		/* Compute distance between first two points and compare to map_line_step to determine angular spacing */
		ellipse_point (GMT, lon, lat, center, sinp, cosp, major, minor, cos_azimuth, sin_azimuth, 0.0, &ax, &ay);
		ellipse_point (GMT, lon, lat, center, sinp, cosp, major, minor, cos_azimuth, sin_azimuth, delta_azimuth, &bx, &by);
		gmt_geo_to_xy (GMT, ax, ay, &axx, &ayy);
		gmt_geo_to_xy (GMT, bx, by, &bxx, &byy);
		L = hypot (axx - bxx, ayy - byy);
		N = (uint64_t) irint (GMT_ELLIPSE_APPROX * L / GMT->current.setting.map_line_step);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Ellipse will be approximated by %d-sided polygon\n", N);
		/* Approximate ellipse by a N-sided polygon */
	}
	else	/* Approximate ellipse by a m-sided polygon */
		N = m;
	delta_azimuth = 2.0 * M_PI / N;
	/* Allocate datasetgment of the right size */
	S = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, N+1, 2, NULL, NULL);
	px = S->data[GMT_X];	py = S->data[GMT_Y];

	for (i = 0; i < N; i++)
		ellipse_point (GMT, lon, lat, center, sinp, cosp, major, minor, cos_azimuth, sin_azimuth, i * delta_azimuth, &px[i], &py[i]);

	/* Explicitly close the polygon */
	px[N] = px[0], py[N] = py[0];
	snprintf (header, GMT_LEN256, "Ellipse around %g/%g with major/minor axes %g/%g km and major axis azimuth %g approximated by %" PRIu64 " points", lon, lat, major_km, minor_km, azimuth, N);
	S->header = strdup (header);
	return (S);
}
