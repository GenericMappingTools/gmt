/*--------------------------------------------------------------------
 *	$Id: gmt_map.c,v 1.76 2005-01-01 21:48:26 pwessel Exp $
 *
 *	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 *			G M T _ M A P . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_map.c contains code related to coordinate transformation
 *
 * 	Map Transformation Setup Routines
 *	These routines initializes the selected map transformation
 *	The names and main function are listed below
 *	NB! Note that the transformation function does not check that they are
 *	passed valid lon,lat numbers. I.e asking for log10 scaling using values
 *	<= 0 results in problems.
 *
 * Map_projections include functions that will set up the transformation
 * between xy and latlon for several map projections.
 *
 * A few of the core coordinate transformation functions are based on similar
 * FORTRAN routines written by Pat Manley, Doug Shearer, and Bill Haxby, and
 * have been rewritten in C and subsequently streamlined.  The Lambert conformal
 * was originally coded up by Bernie Coakley.  The rest is coded by Wessel based
 * on P. Snyder, "Map Projections - a working manual", USGS Prof paper 1395.
 *
 * Transformations supported (both forward and inverse):
 *
 * Non-geographic projections:
 *
 *	Linear x/y[/z] scaling
 *	Polar (theta, r) scaling
 *
 * Map projections:
 *
 *  Cylindrical:
 *	Mercator
 *	Cassini Cylindrical
 *	Miller Cylindrical
 *	Oblique Mercator
 *	TM Transverse Mercator (Ellipsoidal and Spherical)
 *	UTM Universal Transverse Mercator
 *	Cylindrical Equal-area (e.g., Peters, Gall, Behrmann)
 *	Cylindrical Equidistant (Plate Carree)
 *  Azimuthal
 *	Stereographic Conformal
 *	Lambert Azimuthal Equal-Area
 *	Orthographic
 *	Azimuthal Equidistant
 *	Gnomonic
 *  Conic
 *	Albers Equal-Area Conic
 *	Lambert Conformal Conic
 *	Equidistant Conic
 *  Thematic
 *	Mollweide Equal-Area
 *	Hammer-Aitoff Equal-Area
 *	Sinusoidal Equal-Area
 *	Winkel Tripel
 *	Robinson
 *	Eckert IV
 *	Eckert IV
 *	Van der Grinten
 *
 * The ellipsoid used is selectable by editing the .gmtdefaults4 in your
 * home directory.  If no such file, create one by running gmtdefaults.
 *
 * Usage: Initialize system by calling GMT_map_setup (separate module), and
 * then just use GMT_geo_to_xy() and GMT_xy_to_geo() functions.
 *
 * Author:	Paul Wessel
 * Date:	14-JUL-2000
 * Version:	3.4
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
 *	GMT_grd_forward :	Forward map-transform grid matrix from lon/lat to x/y
 *	GMT_grd_inverse :	Inversely transform grid matrix from x/y to lon/lat
 *	GMT_grdproject_init :	Initialize parameters for grid transformations
 *	GMT_great_circle_dist :	Returns great circle distance in degrees
 *	GMT_map_outside :	Generic function determines if we're outside map boundary
 *	GMT_map_path :		Return latpat or GMT_lonpath
 *	GMT_map_setup :		Initialize map projection
 *	GMT_pen_status :	Determines if pen is up or down
 *	GMT_project3D :		Convert lon/lat/z to xx/yy/zz
 *	GMT_2D_to_3D :		Convert xyz to xy for entire array
 *	GMT_xy_to_geo :		Generic inverse x/y to lon/lat projection
 *	GMT_xyz_to_xy :		Generic xyz to xy projection
 *
 * Internal GMT Functions include:
 *
 *	GMT_break_through :		Checks if we cross a map boundary
 *	GMT_get_origin :		Find origin of projection based on pole and 2nd point
 *	GMT_get_rotate_pole :		Find rotation pole based on two points on great circle
 *	GMT_grinten :			Van der Grinten projection
 *	GMT_igrinten :			Inverse Van der Grinten projection
 *	GMT_hammer :			Hammer-Aitoff equal area projection
 *	GMT_ihammer :			Inverse Hammer-Aitoff equal area projection
 *	GMT_ilamb :			Inverse Lambert conformal conic projection
 *	GMT_ilambeq :			Inverse Lambert azimuthal equal area projection
 *	GMT_ilinearxy :			Inverse linear projection
 *	GMT_imerc :			Inverse Mercator projection
 *	GMT_imollweide :		Inverse Mollweide projection
 *	GMT_init_three_D :		Initializes parameters needed for 3-D plots
 *	GMT_ioblmrc :			Inverse oblique Mercator projection, spherical only
 *	GMT_iortho :			Inverse orthographic projection
 *	GMT_iplrs_sph :			Inverse Polar stereographic projection
 *	GMT_map_crossing :		Generic function finds crossings between line and map boundary
 *	GMT_itranslin :			Inverse linear x projection
 *	GMT_itranslog10 :		Inverse log10 x projection
 *	GMT_itranspowx :		Inverse pow x projection
 *	GMT_itranspowy :		Inverse pow y projection
 *	GMT_itranspowz :		Inverse pow z projection
 *	GMT_itm :			Inverse TM projection
 *	GMT_itm_sph :			Inverse TM projection (Spherical)
 *	GMT_iutm :			Inverse UTM projection
 *	GMT_iutm_sph :			Inverse UTM projection (Spherical)
 *	GMT_lamb :			Lambert conformal conic projection
 *	GMT_lambeq :			Lambert azimuthal equal area projection
 *	GMT_latpath :			Return path between 2 points of equal latitide
 *	GMT_lonpath :			Return path between 2 points of equal longitude
 *	GMT_radial_crossing :		Determine map crossing in the Lambert azimuthal equal area projection
 *	GMT_left_boundary :		Return left boundary in x-inches
 *	GMT_linearxy :			Linear xy projection
 *	GMT_lon_inside :		Accounts for wrap-around in longitudes and checks for inside	
 *	GMT_merc :			Mercator projection
 *	GMT_mollweide :			Mollweide projection
 *	GMT_ellipse_crossing :		Find map crossings in the Mollweide projection
 *	GMT_move_to_rect :		Move an outside point straight in to nearest edge
 *	GMT_oblmrc :			Spherical oblique Mercator projection
 *	GMT_ortho :			Orthographic projection
 *	GMT_plrs_sph :			Polar stereographic projection
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
 *	GMT_translin :			Linear x projection
 *	GMT_translog10 :		Log10 x projection
 *	GMT_transpowx :			Linear pow x projection
 *	GMT_transpowy :			Linear pow y projection
 *	GMT_transpowz :			Linear pow z projection
 *	GMT_tm :			TM projection
 *	GMT_xy_search :			Find xy map boundary
 *	GMT_valbers :			Initialize Albers conic equal area projection
 *	GMT_veconic :			Initialize Equidistant conic projection
 *	GMT_vhammer :			Initialize Hammer-Aitoff equal area projection
 *	GMT_vlamb :			Initialize Lambert conformal conic projection
 *	GMT_vlambeq :			Initialize Lambert azimuthal equal area projection
 *	GMT_vmerc :			Initialize Mercator projection
 *	GMT_vmollweide :		Initialize Mollweide projection
 *	GMT_vortho :			Initialize Orthographic projection
 *	GMT_vgrinten :			Initialize Van der Grinten projection
 *	GMT_vstereo :			Initialize Stereographic projection
 *	GMT_vtm :			Initialize TM projection
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
 
#include "gmt.h"
#include "gmt_map.h"

#define HALF_DBL_MAX (DBL_MAX/2.0)

BOOLEAN GMT_convert_latitudes = FALSE;	/* TRUE if using spherical code with authalic/conformal latitudes */

/* ANSI-C Prototypes for functions internal to gmt_map.c */

BOOLEAN GMT_quickconic (void);
BOOLEAN GMT_quicktm (double lon0, double limit);
void GMT_vpolar(double lon0);
void GMT_vmerc(double cmerid);
void GMT_vcyleq(double lon0, double slat);
void GMT_vmiller(double lon0);
void GMT_vcyleqdist(double lon0);
void GMT_vstereo(double rlong0, double plat);
void GMT_vlamb(double rlong0, double rlat0, double pha, double phb);
void GMT_vtm(double lon0, double lat0);
void GMT_vlambeq(double lon0, double lat0);
void GMT_vortho(double lon0, double lat0);
void GMT_vgnomonic(double lon0, double lat0, double horizon);
void GMT_vazeqdist(double lon0, double lat0);
void GMT_vmollweide(double lon0, double scale);
void GMT_vhammer(double lon0, double scale);
void GMT_vwinkel(double lon0, double scale);
void GMT_veckert4(double lon0);
void GMT_veckert6(double lon0);
void GMT_vrobinson(double lon0);
void GMT_vsinusoidal(double lon0);
void GMT_vcassini(double lon0, double lat0);
void GMT_valbers(double lon0, double lat0, double ph1, double ph2);
void GMT_valbers_sph(double lon0, double lat0, double ph1, double ph2);
void GMT_veconic(double lon0, double lat0, double ph1, double ph2);
void GMT_vgrinten(double lon0, double scale);
void GMT_scale_eqrad ();
void GMT_set_polar (double plat);
int GMT_ok_xovers (int nx, double x0, double x1, int *sides);
void GMT_linearxy(double x, double y, double *x_i, double *y_i);	/*	Convert x/y as linear, log10, or power	*/
void GMT_ilinearxy(double *x, double *y, double x_i, double y_i);	/*	Convert inverse x/y as linear, log10, or power	*/
void GMT_polar(double x, double y, double *x_i, double *y_i);		/*	Convert x/y (being theta,r) to x,y	*/
void GMT_ipolar(double *x, double *y, double x_i, double y_i);		/*	Convert (theta,r) to x,y	*/
void GMT_translin(double forw, double *inv);		/*	Forward linear	*/
void GMT_translind(double forw, double *inv);	/*	Forward linear, but using 0-360 degrees	*/
void GMT_itranslin(double *forw, double inv);	/*	Inverse linear	*/
void GMT_itranslind(double *forw, double inv);	/*	Inverse linear, but using 0-360 degrees	*/
void GMT_translog10(double forw, double *inv);	/*	Forward log10	*/
void GMT_itranslog10(double *forw, double inv);	/*	Inverse log10	*/
void GMT_transpowx(double x, double *x_in);	/*	Forward pow x	*/
void GMT_itranspowx(double *x, double x_in);	/*	Inverse pow x	*/
void GMT_transpowy(double y, double *y_in);	/*	Forward pow y 	*/
void GMT_itranspowy(double *y, double y_in);	/*	Inverse pow y 	*/
void GMT_transpowz(double z, double *z_in);	/*	Forward pow z 	*/
void GMT_itranspowz(double *z, double z_in);	/*	Inverse pow z 	*/
void GMT_albers(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Albers)	*/
void GMT_ialbers(double *lon, double *lat, double x, double y);		/*	Convert x/y (Albers) to lon/lat	*/
void GMT_econic(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Equidistant Conic)	*/
void GMT_ieconic(double *lon, double *lat, double x, double y);		/*	Convert x/y (Equidistant Conic) to lon/lat	*/
void GMT_albers_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Albers Spherical)	*/
void GMT_ialbers_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Albers Spherical) to lon/lat	*/
void GMT_azeqdist(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Azimuthal equal-distance)	*/
void GMT_iazeqdist(double *lon, double *lat, double x, double y);	/*	Convert x/y (Azimuthal equal-distance) to lon/lat	*/
void GMT_cassini(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Cassini)	*/
void GMT_icassini(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cassini) to lon/lat	*/
void GMT_cassini_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Cassini Spherical)	*/
void GMT_icassini_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cassini Spherical) to lon/lat	*/
void GMT_hammer(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Hammer-Aitoff)	*/
void GMT_ihammer(double *lon, double *lat, double x, double y);		/*	Convert x/y (Hammer-Aitoff) to lon/lat	*/
void GMT_grinten(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (van der Grinten)	*/
void GMT_igrinten(double *lon, double *lat, double x, double y);	/*	Convert x/y (van der Grinten) to lon/lat	*/
void GMT_merc_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Mercator Spherical)	*/
void GMT_imerc_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Mercator Spherical) to lon/lat	*/
void GMT_plrs(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Polar)		*/
void GMT_iplrs(double *lon, double *lat, double x, double y);		/*	Convert x/y (Polar) to lon/lat		*/
void GMT_plrs_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Polar Spherical)		*/
void GMT_iplrs_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Polar Spherical) to lon/lat		*/
void GMT_lamb(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Lambert)	*/
void GMT_ilamb(double *lon, double *lat, double x, double y);		/*	Convert x/y (Lambert) to lon/lat 	*/
void GMT_lamb_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Lambert Spherical)	*/
void GMT_ilamb_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Lambert Spherical) to lon/lat 	*/
void GMT_oblmrc(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Oblique Mercator)	*/
void GMT_ioblmrc(double *lon, double *lat, double x, double y);		/*	Convert x/y (Oblique Mercator) to lon/lat 	*/
void GMT_ortho(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (ORTHO)	*/
void GMT_iortho(double *lon, double *lat, double x, double y);		/*	Convert x/y (ORTHO) to lon/lat 	*/
void GMT_gnomonic(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (GNOMONIC)	*/
void GMT_ignomonic(double *lon, double *lat, double x, double y);	/*	Convert x/y (GNOMONIC) to lon/lat 	*/
void GMT_sinusoidal(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (SINUSOIDAL)	*/
void GMT_isinusoidal(double *lon, double *lat, double x, double y);	/*	Convert x/y (SINUSOIDAL) to lon/lat 	*/
void GMT_tm(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (TM)	*/
void GMT_itm(double *lon, double *lat, double x, double y);		/*	Convert x/y (TM) to lon/lat 	*/
void GMT_tm_sph(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (TM Spherical)	*/
void GMT_itm_sph(double *lon, double *lat, double x, double y);		/*	Convert x/y (TM Spherical) to lon/lat 	*/
void GMT_utm(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (UTM)	*/
void GMT_iutm(double *lon, double *lat, double x, double y);		/*	Convert x/y (UTM) to lon/lat 	*/
void GMT_utm_sph(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (UTM Spherical)	*/
void GMT_iutm_sph(double *lon, double *lat, double x, double y);		/*	Convert x/y (UTM Spherical) to lon/lat 	*/
void GMT_winkel(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Winkel)	*/
void GMT_iwinkel(double *lon, double *lat, double x, double y);		/*	Convert x/y (Winkel) to lon/lat	*/
void GMT_eckert4(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Eckert IV)	*/
void GMT_ieckert4(double *lon, double *lat, double x, double y);		/*	Convert x/y (Eckert IV) to lon/lat	*/
void GMT_eckert6(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Eckert VI)	*/
void GMT_ieckert6(double *lon, double *lat, double x, double y);		/*	Convert x/y (Eckert VI) to lon/lat	*/
void GMT_robinson(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Robinson)	*/
void GMT_irobinson(double *lon, double *lat, double x, double y);	/*	Convert x/y (Robinson) to lon/lat	*/
void GMT_stereo1(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Stereographic)	*/
void GMT_stereo2(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Stereographic, equatorial view)	*/
void GMT_istereo(double *lon, double *lat, double x, double y);		/*	Convert x/y (Stereographic) to lon/lat 	*/
void GMT_stereo1_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Stereographic Spherical)	*/
void GMT_stereo2_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Stereographic Spherical, equatorial view)	*/
void GMT_istereo_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Stereographic Spherical) to lon/lat 	*/
void GMT_lambeq(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Lambert Azimuthal Equal-Area)	*/
void GMT_ilambeq(double *lon, double *lat, double x, double y);		/*	Convert x/y (Lambert Azimuthal Equal-Area) to lon/lat 	*/
void GMT_mollweide(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Mollweide Equal-Area)	*/
void GMT_imollweide(double *lon, double *lat, double x, double y);	/*	Convert x/y (Mollweide Equal-Area) to lon/lat 	*/
void GMT_cyleq(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Cylindrical Equal-Area)	*/
void GMT_icyleq(double *lon, double *lat, double x, double y);		/*	Convert x/y (Cylindrical Equal-Area) to lon/lat 	*/
void GMT_cyleqdist(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Cylindrical Equidistant)	*/
void GMT_icyleqdist(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cylindrical Equidistant) to lon/lat 	*/
void GMT_miller(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Miller Cylindrical)	*/
void GMT_imiller(double *lon, double *lat, double x, double y);		/*	Convert x/y (Miller Cylindrical) to lon/lat 	*/
void GMT_obl (double lon, double lat, double *olon, double *olat);	/*	Convert lon/loat to oblique lon/lat		*/
void GMT_iobl (double *lon, double *lat, double olon, double olat);	/*	Convert oblique lon/lat to regular lon/lat	*/

int GMT_wesn_outside(double lon, double lat);		/*	Returns TRUE if a lon/lat point is outside map (rectangular wesn boundaries only)	*/
int GMT_polar_outside(double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the polar boundaries	*/
int GMT_rect_outside(double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the x'/y' boundaries	*/
int GMT_rect_outside2(double lon, double lat);		/*	Returns TRUE if a x'/y' point is outside the x'/y' boundaries (azimuthal maps only)	*/
int GMT_eqdist_outside(double lon, double lat);		/*	Returns TRUE if a x'/y' point is on the map perimeter 	*/
int GMT_radial_outside(double lon, double lat);		/*	Returns TRUE if a lon/lat point is outside the Lambert Azimuthal Eq. area boundaries	*/
int GMT_wesn_crossing(double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, int *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
int GMT_rect_crossing(double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, int *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
int GMT_radial_crossing(double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides);		/*	computes the crossing point between two lon/lat points and the circular map boundary between them */
int GMT_ellipse_crossing(double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
int GMT_eqdist_crossing(double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides);		/*	computes the crossing point between two lon/lat points and the map boundary between them */
int GMT_rect_clip(double *lon, double *lat, int n, double **x, double **y, int *total_nx);		/*	Clips to region based on rectangular xy coordinates	*/
int GMT_wesn_clip(double *lon, double *lat, int n, double **x, double **y, int *total_nx);		/*	Clips to region based on rectangular wesn coordinates	*/
int GMT_radial_clip(double *lon, double *lat, int np, double **x, double **y, int *total_nx);		/*	Clips to region based on spherical distance */
int GMT_wesn_overlap(double lon0, double lat0, double lon1, double lat1);		/*	Checks if two wesn regions overlap	*/
int GMT_rect_overlap(double lon0, double lat0, double lon1, double lat1);		/*	Checks if two xy regions overlap	*/
int GMT_radial_overlap(double lon0, double lat0, double lon1, double lat1);		/*	Currently a dummy routine	*/

int GMT_map_init_linear(void);
int GMT_map_init_polar(void);
int GMT_map_init_merc(void);
int GMT_map_init_miller(void);
int GMT_map_init_stereo(void);
int GMT_map_init_lambert(void);
int GMT_map_init_oblique(void);
int GMT_map_init_tm(void);
int GMT_map_init_utm(void);
int GMT_map_init_lambeq(void);
int GMT_map_init_ortho(void);
int GMT_map_init_gnomonic(void);
int GMT_map_init_azeqdist(void);
int GMT_map_init_mollweide(void);
int GMT_map_init_hammer(void);
int GMT_map_init_winkel(void);
int GMT_map_init_eckert4(void);
int GMT_map_init_eckert6(void);
int GMT_map_init_cyleq(void);
int GMT_map_init_cyleqdist(void);
int GMT_map_init_robinson(void);
int GMT_map_init_sinusoidal(void);
int GMT_map_init_cassini(void);
int GMT_map_init_albers(void);
int GMT_map_init_grinten (void);
int GMT_map_init_econic (void);

void GMT_wesn_search (double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north);
void GMT_horizon_search (double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax);
void GMT_xy_search(double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0);
void GMT_init_three_D(void);
void GMT_map_setxy(double xmin, double xmax, double ymin, double ymax);
void GMT_map_setinfo(double xmin, double xmax, double ymin, double ymax, double scl);
void GMT_check_R_J(double *clon);
void GMT_set_spherical(void);
void GMT_get_origin(double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2);
void GMT_get_rotate_pole(double lon1, double lat1, double lon2, double lat2);
void GMT_pole_rotate_forward(double lon, double lat, double *tlon, double *tlat);
void GMT_pole_rotate_inverse(double *lon, double *lat, double tlon, double tlat);
void GMT_x_wesn_corner(double *x);
void GMT_y_wesn_corner(double *y);
void GMT_x_rect_corner(double *x);
void GMT_y_rect_corner(double *y);
int GMT_lon_inside(double lon, double w, double e);
int GMT_is_wesn_corner(double x, double y);
int GMT_is_rect_corner(double x, double y);
int GMT_move_to_rect (double *x_edge, double *y_edge, int j, int nx);
int GMT_move_to_wesn (double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, int j, int nx);
void GMT_merc_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head);
void GMT_merc_inverse (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head);
void GMT_transx_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head);
void GMT_transy_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head);
int GMT_wrap_around_check_x(double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, int *sides, int *nx);
int GMT_wrap_around_check_tm(double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, int *sides, int *nx);
BOOLEAN GMT_will_it_wrap_x(double *x, double *y, int n, int *start);
BOOLEAN GMT_will_it_wrap_tm(double *x, double *y, int n, int *start);
BOOLEAN GMT_this_point_wraps_x (double x0, double x1, double w_last, double w_this);
BOOLEAN GMT_this_point_wraps_tm (double y0, double y1);
int GMT_truncate_x(double *x, double *y, int n, int start, int l_or_r);
int GMT_truncate_tm(double *x, double *y, int n, int start, int b_or_t);
void GMT_get_crossings_x(double *xc, double *yc, double x0, double y0, double x1, double y1);
void GMT_get_crossings_tm(double *xc, double *yc, double x0, double y0, double x1, double y1);
int GMT_map_jump_x(double x0, double y0, double x1, double y1);
int GMT_map_jump_tm(double x0, double y0, double x1, double y1);
double GMT_lon_to_corner (double lon);
double GMT_lat_to_corner (double lat);
double GMT_x_to_corner (double x);
double GMT_y_to_corner (double y);

double GMT_left_boundary(double y);	/*	Returns x-value of left border for given y	*/
double GMT_right_boundary(double y);	/*	Returns x-value of right border for given y	*/
double GMT_left_conic(double y);	/*	For Conic wesn maps	*/
double GMT_right_conic(double y);	/*	For Conic wesn maps	*/
double GMT_left_rect(double y);		/*	For rectangular maps	*/
double GMT_right_rect(double y);	/*	For rectangular maps	*/
double GMT_left_circle(double y);	/*	For circular maps	*/
double GMT_right_circle(double y);	/*	For circular maps	*/
double GMT_left_ellipse(double y);	/*	For elliptical maps	*/
double GMT_right_ellipse(double y);	/*	For elliptical maps	*/
double GMT_left_winkel(double y);	/*	For Winkel maps	*/
double GMT_right_winkel(double y);	/*	For Winkel maps	*/
double GMT_left_eckert4(double y);	/*	For Eckert IV maps	*/
double GMT_right_eckert4(double y);	/*	For Eckert IV maps	*/
double GMT_left_eckert6(double y);	/*	For Eckert VI maps	*/
double GMT_right_eckert6(double y);	/*	For Eckert VI maps	*/
double GMT_left_robinson(double y);	/*	For Robinson maps	*/
double GMT_right_robinson(double y);	/*	For Robinson maps	*/
double GMT_left_sinusoidal(double y);	/*	For sinusoidal maps	*/
double GMT_right_sinusoidal(double y);	/*	For sinusoidal maps	*/
double GMT_robinson_spline (double xp, double *x, double *y, double *c);
int GMT_set_datum (char *text, struct GMT_DATUM *D);


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
 *   project == LINEAR, POLAR, MERCATOR, STEREO, LAMBERT, OBLIQUE_MERC,
 *	TM, ORTHO, AZ_EQDIST, LAMB_AZ_EQ, WINKEL, ROBINSON, CASSINI, ALBERS, ECONIC,
 *	ECKERT4, ECKERT6, CYL_EQ, CYL_EQDIST, MILLER, GRINTEN
 *	For LINEAR, we may have LINEAR, LOG10, or POW
 *
 * parameters[0] through parameters[np-1] mean different things to the various
 * projections, as explained below. (np also varies, of course)
 *
 * LINEAR projection:
 *	parameters[0] is inch (or cm)/x_user_unit.
 *	parameters[1] is inch (or cm)/y_user_unit. If 0, then yscale = xscale.
 *	parameters[2] is pow for x^pow (if POW is on).
 *	parameters[3] is pow for y^pow (if POW is on).
 *
 * POLAR (r,theta) projection:
 *	parameters[0] is inch (or cm)/x_user_unit (radius)
 *
 * MERCATOR projection:
 *	parameters[0] is in inch (or cm)/degree_longitude @ equator OR 1:xxxxx OR map-width
 *
 * STEREOgraphic projection:
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
 *	parameters[1] is scale in inch (cm)/degree along this meridian OR 1:xxxxx OR map-width
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
 * VAN DER GRINTEN projection
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
 * CYLINDRICAL EQUAL-AREA projections (Behrmann, Gall, Peters):
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

void GMT_map_setup (double west, double east, double south, double north)
{
	int search, k;
	
	/* if (!project_info.region) d_swap (south, east); */  /* Got w/s/e/n, make into w/e/s/n */

	if (west == east && south == north) {
		fprintf (stderr, "%s: GMT Fatal Error: No region selected - Aborts!\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	if (MAPPING) {	/* Special checks only applicable to geographic coordinates */
		if (project_info.degree[0]) {
			if (west < 0.0 && east < 0.0) {
				while (west < 0.0) west += 360.0;
				while (east < 0.0) east += 360.0;
			}
			else if (east < west)
				east += 360.0;

			if ((fabs (east - west) - 360.0) > SMALL) {
				fprintf (stderr, "%s: GMT Fatal Error: Region exceeds 360 degrees!\n", GMT_program);
				exit (EXIT_FAILURE);
			}
		}
		if (project_info.degree[1]) {
			if (south < -90.0 || south > 90.0) {
				fprintf (stderr, "%s: GMT Fatal Error: South (%g) outside +-90 degree range!\n", GMT_program, south);
				exit (EXIT_FAILURE);
			}
			if (north < -90.0 || north > 90.0) {
				fprintf (stderr, "%s: GMT Fatal Error: North (%g) outside +-90 degree range!\n", GMT_program, north);
				exit (EXIT_FAILURE);
			}
		}
	}

	project_info.w = west;	project_info.e = east;	project_info.s = south;	project_info.n = north;
	if (project_info.gave_map_width) project_info.units_pr_degree = FALSE;
	
	if (!GMT_z_forward) GMT_z_forward = (PFI) GMT_translin;
	if (!GMT_z_inverse) GMT_z_inverse = (PFI) GMT_itranslin;
	gmtdefs.n_lon_nodes = gmtdefs.n_lat_nodes = 0;
	GMT_wrap_around_check = (PFI) GMT_wrap_around_check_x;
	GMT_map_jump = (PFI) GMT_map_jump_x;
	GMT_will_it_wrap = (PFB) GMT_will_it_wrap_x;
	GMT_this_point_wraps = (PFB) GMT_this_point_wraps_x;
	GMT_get_crossings = (PFV) GMT_get_crossings_x;
	GMT_truncate = (PFI) GMT_truncate_x;
	GMT_lat_swap_init ();
	
	switch (project_info.projection) {

		case LINEAR:		/* Linear transformations */
		
			search = GMT_map_init_linear ();
			break;

		case POLAR:		/* Both lon/lat are actually theta, radius */

			search = GMT_map_init_polar ();
			break;

		case MERCATOR:		/* Standard Mercator projection */
	
			search = GMT_map_init_merc ();
			break;
		
		case STEREO:		/* Stereographic projection */

			search = GMT_map_init_stereo ();
			break;

		case LAMBERT:		/* Lambert Conformal Conic */

			search = GMT_map_init_lambert ();
			break;

		case OBLIQUE_MERC:	/* Oblique Mercator */

			search = GMT_map_init_oblique ();
			break;

		case TM:		/* Transverse Mercator */

			search = GMT_map_init_tm ();
			break;

		case UTM:		/* Universal Transverse Mercator */

			search = GMT_map_init_utm ();
			break;

		case LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */

			search = GMT_map_init_lambeq ();
			break;

		case ORTHO:		/* Orthographic Projection */

			search = GMT_map_init_ortho ();
			break;

		case AZ_EQDIST:		/* Azimuthal Equal-Distance Projection */

			search = GMT_map_init_azeqdist ();
			break;

		case GNOMONIC:		/* Azimuthal Gnomonic Projection */

			search = GMT_map_init_gnomonic ();
			break;

		case MOLLWEIDE:		/* Mollweide Equal-Area */

			search = GMT_map_init_mollweide ();
			break;

		case HAMMER:		/* Hammer-Aitoff Equal-Area */

			search = GMT_map_init_hammer ();
			break;

		case GRINTEN:		/* Van der Grinten */

			search = GMT_map_init_grinten ();
			break;

		case WINKEL:		/* Winkel Tripel */

			search = GMT_map_init_winkel ();
			break;
			
		case ECKERT4:		/* Eckert IV */

			search = GMT_map_init_eckert4 ();
			break;

		case ECKERT6:		/* Eckert VI */

			search = GMT_map_init_eckert6 ();
			break;

		case CYL_EQ:		/* Cylindrical Equal-Area */

			search = GMT_map_init_cyleq ();
			break;

		case MILLER:		/* Miller Cylindrical */

			search = GMT_map_init_miller ();
			break;

		case CYL_EQDIST:	/* Cylindrical Equidistant */

			search = GMT_map_init_cyleqdist ();
			break;

		case ROBINSON:		/* Robinson */

			search = GMT_map_init_robinson ();
			break;

		case SINUSOIDAL:	/* Sinusoidal Equal-Area */

			search = GMT_map_init_sinusoidal ();
			break;

		case CASSINI:		/* Cassini cylindrical */

			search = GMT_map_init_cassini ();
			break;

		case ALBERS:		/* Albers Equal-Area Conic */

			search = GMT_map_init_albers ();
			break;

		case ECONIC:		/* Equidistant Conic */

			search = GMT_map_init_econic ();
			break;

		default:	/* No projection selected, die a horrible death */

			fprintf (stderr, "%s: GMT Fatal Error: No projection selected - Aborts!\n", GMT_program);
			exit (EXIT_FAILURE);
			break;
	}

	project_info.i_x_scale = (project_info.x_scale != 0.0) ? 1.0 / project_info.x_scale : 1.0;
	project_info.i_y_scale = (project_info.y_scale != 0.0) ? 1.0 / project_info.y_scale : 1.0;
	project_info.i_z_scale = (project_info.z_scale != 0.0) ? 1.0 / project_info.z_scale : 1.0;

	GMT_map_width = fabs (project_info.xmax - project_info.xmin);
	GMT_map_height = fabs (project_info.ymax - project_info.ymin);
	GMT_half_map_size = 0.5 * GMT_map_width;
	GMT_half_map_height = 0.5 * GMT_map_height;
	
	if (!gmtdefs.n_lon_nodes) gmtdefs.n_lon_nodes = irint (GMT_map_width / gmtdefs.line_step);
	if (!gmtdefs.n_lat_nodes) gmtdefs.n_lat_nodes = irint (GMT_map_height / gmtdefs.line_step);
	
	gmtdefs.dlon = (project_info.e - project_info.w) / gmtdefs.n_lon_nodes;
	gmtdefs.dlat = (project_info.n - project_info.s) / gmtdefs.n_lat_nodes;
	
	if (search) {
		GMT_wesn_search (project_info.xmin, project_info.xmax, project_info.ymin, project_info.ymax, &project_info.w, &project_info.e, &project_info.s, &project_info.n);
		gmtdefs.dlon = (project_info.e - project_info.w) / gmtdefs.n_lon_nodes;
		gmtdefs.dlat = (project_info.n - project_info.s) / gmtdefs.n_lat_nodes;
	}

	if (AZIMUTHAL && !project_info.region) GMT_horizon_search (west, east, south, north, project_info.xmin, project_info.xmax, project_info.ymin, project_info.ymax);

	if (project_info.central_meridian < project_info.w && (project_info.central_meridian + 360.0) <= project_info.e) project_info.central_meridian += 360.0;
	if (project_info.central_meridian > project_info.e && (project_info.central_meridian - 360.0) >= project_info.w) project_info.central_meridian -= 360.0;

	GMT_init_three_D ();
	
	/* Default for overlay plots is no shifting */
	
	if (!project_info.x_off_supplied && gmtdefs.overlay) gmtdefs.x_origin = 0.0;
	if (!project_info.y_off_supplied && gmtdefs.overlay) gmtdefs.y_origin = 0.0;
	
	k = gmtdefs.page_orientation & 1;	/* k and !k gives 0,1 or 1,0 depending on -P */
	if (project_info.x_off_supplied == 2) gmtdefs.x_origin = 0.5 * (fabs(gmtdefs.paper_width[!k]/72.0) - GMT_map_width);		/* Want to x center plot on current page size */
	if (project_info.y_off_supplied == 2) gmtdefs.y_origin = 0.5 * (fabs(gmtdefs.paper_width[k]/72.0) - GMT_map_height);	/* Want to y center plot on current page size */
}

void GMT_init_three_D (void) {
	int i, easy;
	double tilt_angle, x, y, x0, x1, x2, y0, y1, y2, zmin, zmax;
	BOOLEAN positive;
	
	project_info.three_D = (z_project.view_azimuth != 180.0 || z_project.view_elevation != 90.0);
	if (project_info.three_D) gmtdefs.basemap_type = GMT_IS_PLAIN;	/* Only plain frame for 3-D */
	
	project_info.z_scale = project_info.z_pars[0];
	if (project_info.z_scale < 0.0) project_info.xyz_pos[2] = FALSE;	/* User wants z to increase down */
	if (project_info.z_level == DBL_MAX) project_info.z_level = (project_info.xyz_pos[2]) ? project_info.z_bottom : project_info.z_top;
	
	switch (project_info.xyz_projection[2]%3 ) {	/* Modulo 3 so that TIME (3) maps to LINEAR (0) */
		case LINEAR:	/* Regular scaling */
			zmin = (project_info.xyz_pos[2]) ? project_info.z_bottom : project_info.z_top;
			zmax = (project_info.xyz_pos[2]) ? project_info.z_top : project_info.z_bottom;
			GMT_z_forward = (PFI) GMT_translin;
			GMT_z_inverse = (PFI) GMT_itranslin;
			break;
		case LOG10:	/* Log10 transformation */
			if (project_info.z_bottom <= 0.0 || project_info.z_top <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR for -Jz -JZ option: limits must be positive for log10 projection\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			zmin = (project_info.xyz_pos[2]) ? d_log10 (project_info.z_bottom) : d_log10 (project_info.z_top);
			zmax = (project_info.xyz_pos[2]) ? d_log10 (project_info.z_top) : d_log10 (project_info.z_bottom);
			GMT_z_forward = (PFI) GMT_translog10;
			GMT_z_inverse = (PFI) GMT_itranslog10;
			break;
		case POW:	/* x^y transformation */
			project_info.xyz_pow[2] = project_info.z_pars[1];
			project_info.xyz_ipow[2] = 1.0 / project_info.z_pars[1];
			positive = !(((int)project_info.xyz_pos[2] + (int)(project_info.xyz_pow[2] > 0.0)) % 2);
			zmin = (positive) ? pow (project_info.z_bottom, project_info.xyz_pow[2]) : pow (project_info.z_top, project_info.xyz_pow[2]);
			zmax = (positive) ? pow (project_info.z_top, project_info.xyz_pow[2]) : pow (project_info.z_bottom, project_info.xyz_pow[2]);
			GMT_z_forward = (PFI) GMT_transpowz;
			GMT_z_inverse = (PFI) GMT_itranspowz;
	}
	if (project_info.compute_scale[2]) project_info.z_scale /= fabs (zmin - zmax);
	project_info.zmax = (zmax - zmin) * project_info.z_scale;
	project_info.z0 = -zmin * project_info.z_scale;
	
	if (z_project.view_azimuth >= 360.0) z_project.view_azimuth -= 360.0;
	z_project.quadrant = (int)ceil (z_project.view_azimuth / 90.0);
	z_project.view_azimuth -= 180.0;	/* Turn into direction instead */
	if (z_project.view_azimuth < 0.0) z_project.view_azimuth += 360.0;
	z_project.view_azimuth *= D2R;
	z_project.view_elevation *= D2R;
	z_project.cos_az = cos (z_project.view_azimuth);
	z_project.sin_az = sin (z_project.view_azimuth);
	z_project.cos_el = cos (z_project.view_elevation);
	z_project.sin_el = sin (z_project.view_elevation);
	GMT_geoz_to_xy (project_info.w, project_info.s, project_info.z_bottom, &x0, &y0);
	GMT_geoz_to_xy (project_info.e, project_info.s, project_info.z_bottom, &x1, &y1);
	GMT_geoz_to_xy (project_info.w, project_info.n, project_info.z_bottom, &x2, &y2);
	z_project.phi[0] = d_atan2 (y1 - y0, x1 - x0) * R2D;
	z_project.phi[1] = d_atan2 (y2 - y0, x2 - x0) * R2D;
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
	z_project.tilt[2] = (fabs (z_project.cos_az) > fabs (z_project.sin_az)) ? tan (-z_project.phi[0] * D2R) : tan (-z_project.phi[1] * D2R);
	
	/* Determine min/max y of plot */
	
	/* easy = TRUE means we can use 4 corner points to find min x and y, FALSE
	   means we must search along wesn perimeter the hard way */
	   
	switch (project_info.projection) {
		case LINEAR:
		case POLAR:
		case MERCATOR:
		case OBLIQUE_MERC:
		case CYL_EQ:
		case CYL_EQDIST:
		case MILLER:
			easy = TRUE;
			break;
		case LAMBERT:
		case TM:
		case UTM:
		case CASSINI:
		case STEREO:
		case ALBERS:
		case ECONIC:
		case LAMB_AZ_EQ:
		case ORTHO:
		case GNOMONIC:
		case AZ_EQDIST:
		case SINUSOIDAL:
		case MOLLWEIDE:
		case HAMMER:
		case GRINTEN:
		case WINKEL:
		case ECKERT4:
		case ECKERT6:
		case ROBINSON:
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
		for (i = 0; i < gmtdefs.n_lon_nodes; i++) {	/* S and N */
			GMT_geoz_to_xy (project_info.w + i * gmtdefs.dlon, project_info.s, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);
			if (project_info.z_top != project_info.z_bottom) {	
				GMT_geoz_to_xy (project_info.w + i * gmtdefs.dlon, project_info.s, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}			
			GMT_geoz_to_xy (project_info.w + i * gmtdefs.dlon, project_info.n, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);			
			if (project_info.z_top != project_info.z_bottom) {	
				GMT_geoz_to_xy (project_info.w + i * gmtdefs.dlon, project_info.n, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}		
		}
		for (i = 0; i < gmtdefs.n_lat_nodes; i++) {	/* W and E */
			GMT_geoz_to_xy (project_info.w, project_info.s + i * gmtdefs.dlat, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);			
			if (project_info.z_top != project_info.z_bottom) {	
				GMT_geoz_to_xy (project_info.w, project_info.s + i * gmtdefs.dlat, project_info.z_top, &x, &y);
				z_project.ymin = MIN (z_project.ymin, y);
				z_project.ymax = MAX (z_project.ymax, y);
				z_project.xmin = MIN (z_project.xmin, x);
				z_project.xmax = MAX (z_project.xmax, x);
			}			
			GMT_geoz_to_xy (project_info.e, project_info.s + i * gmtdefs.dlat, project_info.z_bottom, &x, &y);
			z_project.ymin = MIN (z_project.ymin, y);
			z_project.ymax = MAX (z_project.ymax, y);
			z_project.xmin = MIN (z_project.xmin, x);
			z_project.xmax = MAX (z_project.xmax, x);			
			if (project_info.z_top != project_info.z_bottom) {	
				GMT_geoz_to_xy (project_info.e, project_info.s + i * gmtdefs.dlat, project_info.z_top, &x, &y);
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

void GMT_geo_to_xy (double lon, double lat, double *x, double *y)
{
	/* Converts lon/lat to x/y using the current projection */

	(*GMT_forward) (lon, lat, x, y);
	(*x) = (*x) * project_info.x_scale + project_info.x0;
	(*y) = (*y) * project_info.y_scale + project_info.y0;
}

void GMT_xy_to_geo (double *lon, double *lat, double x, double y)
{
	/* Converts x/y to lon/lat using the current projection */
	
	x = (x - project_info.x0) * project_info.i_x_scale;
	y = (y - project_info.y0) * project_info.i_y_scale;
	
	(*GMT_inverse) (lon, lat, x, y);
}

void GMT_geoz_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* Map-projects xy first, the projects xyz onto xy plane */
	double x0, y0, z0;
	GMT_geo_to_xy (x, y, &x0, &y0);
	GMT_z_to_zz (z, &z0);
	*x_out = x0 * z_project.cos_az - y0 * z_project.sin_az;
	*y_out = (x0 * z_project.sin_az + y0 * z_project.cos_az) * z_project.sin_el + z0 * z_project.cos_el;
}

void GMT_xyz_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* projects xyz onto xy plane */
	*x_out = x * z_project.cos_az - y * z_project.sin_az;
	*y_out = (x * z_project.sin_az + y * z_project.cos_az) * z_project.sin_el + z * z_project.cos_el;
}

void GMT_xy_do_z_to_xy (double x, double y, double z, double *x_out, double *y_out)
{	/* projects xy (inches) z (z-value) onto xy plane */
	double z_out;
	
	GMT_z_to_zz (z, &z_out);
	*x_out = x * z_project.cos_az - y * z_project.sin_az;
	*y_out = (x * z_project.sin_az + y * z_project.cos_az) * z_project.sin_el + z_out * z_project.cos_el;
}

void GMT_project3D (double x, double y, double z, double *x_out, double *y_out, double *z_out)
{
	GMT_geo_to_xy (x, y, x_out, y_out);
	GMT_z_to_zz (z, z_out);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION
 */
 
int GMT_map_init_linear (void) {
	BOOLEAN positive;
	double xmin, xmax, ymin, ymax;

	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	GMT_forward = (PFI) GMT_linearxy;
	GMT_inverse = (PFI) GMT_ilinearxy;
	if (project_info.degree[0]) {	/* x is longitude */
		project_info.central_meridian = 0.5 * (project_info.w + project_info.e);
		GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	}
	project_info.x_scale = project_info.pars[0];
	project_info.y_scale = project_info.pars[1];
	if (project_info.x_scale < 0.0) project_info.xyz_pos[0] = FALSE;	/* User wants x to increase left */
	if (project_info.y_scale < 0.0) project_info.xyz_pos[1] = FALSE;	/* User wants y to increase down */
			
	switch ( (project_info.xyz_projection[0]%3) ) {	/* Modulo 3 so that TIME (3) maps to LINEAR (0) */
		case LINEAR:	/* Regular scaling */
			GMT_x_forward = (PFI) ((project_info.degree[0]) ? GMT_translind : GMT_translin);
			GMT_x_inverse = (PFI) ((project_info.degree[0]) ? GMT_itranslind : GMT_itranslin);
			if (project_info.xyz_pos[0]) {
				(*GMT_x_forward) (project_info.w, &xmin);
				(*GMT_x_forward) (project_info.e, &xmax);
			}
			else {
				(*GMT_x_forward) (project_info.e, &xmin);
				(*GMT_x_forward) (project_info.w, &xmax);
			}
			break;
		case LOG10:	/* Log10 transformation */
			if (project_info.w <= 0.0 || project_info.e <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Jx option:  Limits must be positive for log10 option\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			xmin = (project_info.xyz_pos[0]) ? d_log10 (project_info.w) : d_log10 (project_info.e);
			xmax = (project_info.xyz_pos[0]) ? d_log10 (project_info.e) : d_log10 (project_info.w);
			GMT_x_forward = (PFI) GMT_translog10;
			GMT_x_inverse = (PFI) GMT_itranslog10;
			break;
		case POW:	/* x^y transformation */
			project_info.xyz_pow[0] = project_info.pars[2];
			project_info.xyz_ipow[0] = 1.0 / project_info.pars[2];
			positive = !(((int)project_info.xyz_pos[0] + (int)(project_info.xyz_pow[0] > 0.0)) % 2);
			xmin = (positive) ? pow (project_info.w, project_info.xyz_pow[0]) : pow (project_info.e, project_info.xyz_pow[0]);
			xmax = (positive) ? pow (project_info.e, project_info.xyz_pow[0]) : pow (project_info.w, project_info.xyz_pow[0]);
			GMT_x_forward = (PFI) GMT_transpowx;
			GMT_x_inverse = (PFI) GMT_itranspowx;
			break;
	}
	switch (project_info.xyz_projection[1]%3) {	/* Modulo 3 so that TIME (3) maps to LINEAR (0) */
		case LINEAR:	/* Regular scaling */
			ymin = (project_info.xyz_pos[1]) ? project_info.s : project_info.n;
			ymax = (project_info.xyz_pos[1]) ? project_info.n : project_info.s;
			GMT_y_forward = (PFI) GMT_translin;
			GMT_y_inverse = (PFI) GMT_itranslin;
			break;
		case LOG10:	/* Log10 transformation */
			if (project_info.s <= 0.0 || project_info.n <= 0.0) {
				fprintf (stderr, "%s: GMT SYNTAX ERROR -Jx option:  Limits must be positive for log10 option\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			ymin = (project_info.xyz_pos[1]) ? d_log10 (project_info.s) : d_log10 (project_info.n);
			ymax = (project_info.xyz_pos[1]) ? d_log10 (project_info.n) : d_log10 (project_info.s);
			GMT_y_forward = (PFI) GMT_translog10;
			GMT_y_inverse = (PFI) GMT_itranslog10;
			break;
		case POW:	/* x^y transformation */
			project_info.xyz_pow[1] = project_info.pars[3];
			project_info.xyz_ipow[1] = 1.0 / project_info.pars[3];
			positive = !(((int)project_info.xyz_pos[1] + (int)(project_info.xyz_pow[1] > 0.0)) % 2);
			ymin = (positive) ? pow (project_info.s, project_info.xyz_pow[1]) : pow (project_info.n, project_info.xyz_pow[1]);
			ymax = (positive) ? pow (project_info.n, project_info.xyz_pow[1]) : pow (project_info.s, project_info.xyz_pow[1]);
			GMT_y_forward = (PFI) GMT_transpowy;
			GMT_y_inverse = (PFI) GMT_itranspowy;
	}
			
	/* Was given axes length instead of scale? */
			
	if (project_info.compute_scale[0]) project_info.x_scale /= fabs (xmin - xmax);
	if (project_info.compute_scale[1]) project_info.y_scale /= fabs (ymin - ymax);

	GMT_map_setxy (xmin, xmax, ymin, ymax);
	GMT_outside = (PFI) GMT_rect_outside;
	GMT_crossing = (PFI) GMT_rect_crossing;
	GMT_overlap = (PFI) GMT_rect_overlap;
	GMT_map_clip = (PFI) GMT_rect_clip;
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

void GMT_translin (double forw, double *inv)	/* Linear forward */
                   {
	*inv = forw;
}

void GMT_translind (double forw, double *inv)	/* Linear forward, but with degrees*/
                   {
	while ((forw - project_info.central_meridian) < -180.0) forw += 360.0;
	while ((forw - project_info.central_meridian) > 180.0) forw -= 360.0;
	*inv = forw - project_info.central_meridian;
}

void GMT_itranslind (double *forw, double inv)	/* Linear inverse, but with degrees*/
                   {
	*forw = inv + project_info.central_meridian;
	while ((*forw - project_info.central_meridian) < -180.0) *forw += 360.0;
	while ((*forw - project_info.central_meridian) > 180.0) *forw -= 360.0;
}

void GMT_itranslin (double *forw, double inv)	/* Linear inverse */
                   {
	*forw = inv;
}

void GMT_translog10 (double forw, double *inv)	/* Log10 forward */
                   {
	*inv = d_log10 (forw);
}

void GMT_itranslog10 (double *forw, double inv) /* Log10 inverse */
                   {
	*forw = pow (10.0, inv);
}

void GMT_transpowx (double x, double *x_in)	/* pow x forward */
                 {
	*x_in = pow (x, project_info.xyz_pow[0]);
}

void GMT_itranspowx (double *x, double x_in) /* pow x inverse */
                 {
	*x = pow (x_in, project_info.xyz_ipow[0]);
}

void GMT_transpowy (double y, double *y_in)	/* pow y forward */
                 {
	*y_in = pow (y, project_info.xyz_pow[1]);
}

void GMT_itranspowy (double *y, double y_in) /* pow y inverse */
                 {
	*y = pow (y_in, project_info.xyz_ipow[1]);
}

void GMT_transpowz (double z, double *z_in)	/* pow z forward */
{
	*z_in = pow (z, project_info.xyz_pow[2]);
}

void GMT_itranspowz (double *z, double z_in) /* pow z inverse */
{
	*z = pow (z_in, project_info.xyz_ipow[2]);
}

/*
 *	TRANSFORMATION ROUTINES FOR POLAR (theta,r) PROJECTION
 */
 
int GMT_map_init_polar (void)
{
	double xmin, xmax, ymin, ymax;
	
	GMT_vpolar (project_info.pars[1]);
	if (fabs (project_info.s) < GMT_CONV_LIMIT) project_info.edge[0] = FALSE;
	if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
	GMT_left_edge = (PFD) GMT_left_circle;
	GMT_right_edge = (PFD) GMT_right_circle;
	GMT_forward = (PFI) GMT_polar;
	GMT_inverse = (PFI) GMT_ipolar;
	/* GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL); */
	GMT_world_map = FALSE;	/* There is no wrapping around here */
	GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
	project_info.x_scale = project_info.y_scale = project_info.pars[0];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[0]);
	/* xmin = ymin = -project_info.n;	xmax = ymax = project_info.n;
	project_info.x_scale = project_info.y_scale = project_info.pars[0]; */
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	/* project_info.r = 0.5 * project_info.xmax; */
	project_info.r = project_info.y_scale * project_info.n;
	GMT_outside = (PFI) GMT_polar_outside;
	GMT_crossing = (PFI) GMT_wesn_crossing;
	GMT_overlap = (PFI) GMT_wesn_overlap;
	GMT_map_clip = (PFI) GMT_wesn_clip;
	frame_info.horizontal = TRUE;
	gmtdefs.degree_format = -1;	/* Special labeling case */
	gmtdefs.n_lat_nodes = 2;
	GMT_meridian_straight = TRUE;
	
	return (FALSE);
}

void GMT_vpolar (double lon0)
{
	/* Set up a Polar (theta,r) transformation */
	
	project_info.p_base_angle = lon0;
	project_info.central_meridian = 0.5 * (project_info.e + project_info.w);
	
	/* Plus pretend that it is kind of a south polar projection */
	
	project_info.north_pole = FALSE;
	project_info.pole = 0.0;
}

void GMT_polar (double x, double y, double *x_i, double *y_i)
{	/* Transform x and y to polar(cylindrical) coordinates */
	if (project_info.got_azimuths) x = 90.0 - x;    /* azimuths, not directions */
	x = (x - project_info.p_base_angle) * D2R;	/* Change base line angle and convert to radians */
	sincos (x, y_i, x_i);
	(*x_i) *= y;
	(*y_i) *= y;
}

void GMT_ipolar (double *x, double *y, double x_i, double y_i)
{	/* Inversely transform both x and y from polar(cylindrical) coordinates */
	*x = R2D * d_atan2 (y_i, x_i) + project_info.p_base_angle;
	if (project_info.got_azimuths) *x = 90.0 - (*x);  /* azimuths, not directions */
	*y = hypot (x_i, y_i);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MERCATOR PROJECTION
 */
 
int GMT_map_init_merc (void) {
	double xmin, xmax, ymin, ymax, D = 1.0;
	
	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) {	/* Set fudge factor */
		GMT_scale_eqrad ();
		D = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / GMT_lat_swap_vals.rm;
	}
	if (project_info.s <= -90.0 || project_info.n >= 90.0) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -R option:  Cannot include south/north poles with Mercator projection!\n", GMT_program);
		exit (EXIT_FAILURE);
	}
	GMT_vmerc (0.5 * (project_info.w + project_info.e));
	project_info.m_m *= D;
	project_info.m_im /= D;
	project_info.m_mx = project_info.m_m * D2R;
	project_info.m_imx = project_info.m_im * R2D;
	GMT_forward = (PFI)GMT_merc_sph;
	GMT_inverse = (PFI)GMT_imerc_sph;
	(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
	(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
	/* if (project_info.units_pr_degree) project_info.pars[0] /= project_info.M_PR_DEG; */
	if (project_info.units_pr_degree) project_info.pars[0] /= (D * project_info.M_PR_DEG);
	project_info.x_scale = project_info.y_scale = project_info.pars[0]; 
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[0]);
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	gmtdefs.n_lat_nodes = 2;
	gmtdefs.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_outside = (PFI) GMT_wesn_outside;
	GMT_crossing = (PFI) GMT_wesn_crossing;
	GMT_overlap = (PFI) GMT_wesn_overlap;
	GMT_map_clip = (PFI) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;
	
	return (FALSE);	/* No need to search for wesn */
}

void GMT_vmerc (double cmerid)
{
	/* Set up a Mercator transformation */
	
	if (project_info.projection != MERCATOR || !project_info.m_got_parallel) {

		/* Get here when we use -JMwidth or -Jmscale OR GMT_vmerc is called as part of -JO */

		project_info.central_meridian = cmerid;
		project_info.m_m = project_info.EQ_RAD;
		project_info.pars[1] = project_info.pars[2] = 0.0;
	}
	else {	/* Different standard parallel than equator and also set central meridian */

		/* Got here because we used -JM<clon>/<clat>/<width|scale> */

		project_info.central_meridian = project_info.pars[0];
		project_info.m_m = cosd (project_info.pars[1]) / d_sqrt (1.0 - project_info.ECC2 * sind (project_info.pars[1]) * sind (project_info.pars[1])) * project_info.EQ_RAD;	/* Put project_info.EQ_RAD here instead of in merc */
		project_info.pars[0] = project_info.pars[2];	/* Since GMT_map_init_merc expects scale/width as 0'th arg */
	}
	project_info.m_im = 1.0 / project_info.m_m;
	project_info.central_meridian_rad = project_info.central_meridian * D2R;
}

/* Mercator projection for the sphere */

void GMT_merc_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Mercator x/y (project_info.EQ_RAD in project_info.m_m) */

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	*x = project_info.m_mx * lon;
	*y = (fabs (lat) < 90.0) ? project_info.m_m * d_log (tan (M_PI_4 + 0.5 * D2R * lat)) : copysign (DBL_MAX, lat);
}

void GMT_imerc_sph (double *lon, double *lat, double x, double y)
{
	/* Convert Mercator x/y to lon/lat  (project_info.EQ_RAD in project_info.m_im) */
	
	*lon = x * project_info.m_imx + project_info.central_meridian;
	*lat = atan (sinh (y * project_info.m_im)) * R2D;
	if (GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}
/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUAL-AREA PROJECTIONS (CYL_EQ)
 */
 
int GMT_map_init_cyleq (void) {
	double xmin, xmax, ymin, ymax, D, k0, qp, slat, e, e2;
	
	project_info.Dx = project_info.Dy = 0.0; 
	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) {
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
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	GMT_vcyleq (project_info.pars[0], project_info.pars[1]);
	GMT_cyleq (project_info.w, project_info.s, &xmin, &ymin);
	GMT_cyleq (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	gmtdefs.n_lat_nodes = 2;
	gmtdefs.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFI)GMT_cyleq;		GMT_inverse = (PFI)GMT_icyleq;
	GMT_outside = (PFI) GMT_wesn_outside;
	GMT_crossing = (PFI) GMT_wesn_crossing;
	GMT_overlap = (PFI) GMT_wesn_overlap;
	GMT_map_clip = (PFI) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;
	
	return (FALSE);	/* No need to search for wesn */
}

void GMT_vcyleq (double lon0, double slat)
{
	/* Set up a Cylindrical equal-area transformation */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.y_rx = project_info.EQ_RAD * D2R * cosd (slat);
	project_info.y_ry = project_info.EQ_RAD / cosd (slat);
	project_info.y_i_rx = 1.0 / project_info.y_rx;
	project_info.y_i_ry = 1.0 / project_info.y_ry;
}

void GMT_cyleq (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cylindrical equal-area x/y */

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);

	*x = lon * project_info.y_rx;
	*y = project_info.y_ry * sind (lat);
	if (GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= project_info.Dx;
		(*y) *= project_info.Dy;
	}
}

void GMT_icyleq (double *lon, double *lat, double x, double y)
{

	/* Convert Cylindrical equal-area x/y to lon/lat */
	
	if (GMT_convert_latitudes) {	/* Gotta fudge abit */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}
	*lon = (x * project_info.y_i_rx) + project_info.central_meridian;
	*lat = R2D * d_asin (y * project_info.y_i_ry);
	if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQIDISTANT PROJECTION (CYL_EQDIST)
 */
 
int GMT_map_init_cyleqdist (void) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical ();	/* Force spherical for now */

	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	GMT_vcyleqdist (project_info.pars[0]);
	GMT_cyleqdist (project_info.w, project_info.s, &xmin, &ymin);
	GMT_cyleqdist (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
 	gmtdefs.n_lat_nodes = 2;
	gmtdefs.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFI)GMT_cyleqdist;		GMT_inverse = (PFI)GMT_icyleqdist;
	GMT_outside = (PFI) GMT_wesn_outside;
	GMT_crossing = (PFI) GMT_wesn_crossing;
	GMT_overlap = (PFI) GMT_wesn_overlap;
	GMT_map_clip = (PFI) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;
	
	return (FALSE);	/* No need to search for wesn */
}

void GMT_vcyleqdist (double lon0)
{
	/* Set up a Cylindrical equidistant transformation */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.q_r = D2R * project_info.EQ_RAD;
	project_info.q_ir = 1.0 / project_info.q_r;
}

void GMT_cyleqdist (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cylindrical equidistant x/y */

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	*x = lon * project_info.q_r;
	*y = lat * project_info.q_r;
}

void GMT_icyleqdist (double *lon, double *lat, double x, double y)
{

	/* Convert Cylindrical equal-area x/y to lon/lat */
	
	*lon = x * project_info.q_ir + project_info.central_meridian;
	*lat = y * project_info.q_ir;
}

/*
 *	TRANSFORMATION ROUTINES FOR MILLER CYLINDRICAL PROJECTION (MILLER)
 */
 
int GMT_map_init_miller (void) {
	double xmin, xmax, ymin, ymax;
	
	GMT_set_spherical ();	/* Force spherical for now */

	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	GMT_vmiller (project_info.pars[0]);
	GMT_miller (project_info.w, project_info.s, &xmin, &ymin);
	GMT_miller (project_info.e, project_info.n, &xmax, &ymax);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	gmtdefs.n_lat_nodes = 2;
	gmtdefs.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	GMT_forward = (PFI)GMT_miller;		GMT_inverse = (PFI)GMT_imiller;
	GMT_outside = (PFI) GMT_wesn_outside;
	GMT_crossing = (PFI) GMT_wesn_crossing;
	GMT_overlap = (PFI) GMT_wesn_overlap;
	GMT_map_clip = (PFI) GMT_wesn_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
	frame_info.horizontal = TRUE;
	frame_info.check_side = TRUE;
	GMT_meridian_straight = GMT_parallel_straight = TRUE;
	
	return (FALSE);	/* No need to search for wesn */
}

void GMT_vmiller (double lon0)
{
	/* Set up a Miller Cylindrical transformation */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.j_x = D2R * project_info.EQ_RAD;
	project_info.j_y = 1.25 * project_info.EQ_RAD;
	project_info.j_ix = 1.0 / project_info.j_x; 
	project_info.j_iy = 1.0 / project_info.j_y; 
}

void GMT_miller (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Miller Cylindrical x/y */

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	*x = lon * project_info.j_x;
	*y = project_info.j_y * d_log (tan (M_PI_4 + 0.4 * lat * D2R));
}

void GMT_imiller (double *lon, double *lat, double x, double y)
{

	/* Convert Miller Cylindrical x/y to lon/lat */
	
	*lon = x * project_info.j_ix + project_info.central_meridian;
	*lat = 2.5 * R2D * atan (exp (y * project_info.j_iy)) - 112.5;
}

/*
 *	TRANSFORMATION ROUTINES FOR THE POLAR STEREOGRAPHIC PROJECTION
 */
 
int GMT_map_init_stereo (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dummy, radius, latg, D = 1.0;
	
	GMT_convert_latitudes = !SPHERICAL;
	latg = project_info.pars[1];

	GMT_set_polar (project_info.pars[1]);

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 0.9996;	/* Select default map scale for Stereographic */
	if (project_info.polar && (irint (project_info.pars[4]) == 1)) gmtdefs.map_scale_factor = 1.0;	/* Gave true scale at given parallel set below */
	/* Equatorial view has a problem with infinite loops.  Until I find a cure
	  we set projection center latitude to 0.001 so equatorial works for now */

	if (fabs (project_info.pars[1]) < SMALL) project_info.pars[1] = 0.001;

	GMT_vstereo (project_info.pars[0], project_info.pars[1]);

	if (GMT_convert_latitudes) {	/* Set fudge factors when conformal latitudes are used */
		double e1p, e1m, s, c;

		D = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / GMT_lat_swap_vals.rm;
		if (project_info.polar) {
			e1p = 1.0 + project_info.ECC;	e1m = 1.0 - project_info.ECC;
			D /= d_sqrt (pow (e1p, e1p) * pow (e1m, e1m));
			if (irint (project_info.pars[4]) == 1) {	/* Gave true scale at given parallel */
				double k_p, m_c, t_c, es;
				
				sincos (fabs (project_info.pars[3]) * D2R, &s, &c);
				es = project_info.ECC * s;
				m_c = c / d_sqrt (1.0 - project_info.ECC2 * s * s);
				t_c = d_sqrt (((1.0 - s) / (1.0 + s)) * pow ((1.0 + es) / (1.0 - es), project_info.ECC));
				k_p = 0.5 * m_c * d_sqrt (pow (e1p, e1p) * pow (e1m, e1m)) / t_c;
				D *= k_p;
			}
		}
		else {
			sincos (latg * D2R, &s, &c);	/* Need original geographic pole coordinates */
			D *= (c / (project_info.cosp * d_sqrt (1.0 - project_info.ECC2 * s * s)));
		}
	}
	project_info.Dx = project_info.Dy = D;

	project_info.iDx = 1.0 / project_info.Dx;
	project_info.iDy = 1.0 / project_info.Dy;

	if (project_info.polar) {	/* Polar aspect */
		GMT_forward = (PFI)GMT_plrs_sph;
		GMT_inverse = (PFI)GMT_iplrs_sph;
		if (project_info.units_pr_degree) {
			(*GMT_forward) (project_info.pars[0], project_info.pars[3], &dummy, &radius);
			project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
		}
		else
			project_info.x_scale = project_info.y_scale = project_info.pars[2];
		GMT_meridian_straight = TRUE;
	}
	else {
		GMT_forward = (fabs (project_info.pole) < GMT_CONV_LIMIT) ? (PFI)GMT_stereo2_sph : (PFI)GMT_stereo1_sph;
		GMT_inverse = (PFI)GMT_istereo_sph;
		if (project_info.units_pr_degree) {
			GMT_vstereo (0.0, 90.0);
			(*GMT_forward) (0.0, fabs(project_info.pars[3]), &dummy, &radius);
			project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
		}
		else
			project_info.x_scale = project_info.y_scale = project_info.pars[2];
				
		GMT_vstereo (project_info.pars[0], project_info.pars[1]);
	}

	
	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside2;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s < 0.0) project_info.s = 0.0;
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n > 0.0) project_info.n = 0.0;
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT || fabs (project_info.e - project_info.w) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFI) GMT_polar_outside;
			GMT_crossing = (PFI) GMT_wesn_crossing;
			GMT_overlap = (PFI) GMT_wesn_overlap;
			GMT_map_clip = (PFI) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			gmtdefs.n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmin = ymin = -2.0 * project_info.EQ_RAD;
			xmax = ymax = -xmin;
			GMT_outside = (PFI) GMT_radial_outside;
			GMT_crossing = (PFI) GMT_radial_crossing;
			GMT_overlap = (PFI) GMT_radial_overlap;
			GMT_map_clip = (PFI) GMT_radial_clip;
			gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		search = FALSE;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}
				
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
		
	return (search);
}
	
void GMT_vstereo (double rlong0, double plat)
{
	/* Set up a Stereographic transformation */

	double clat;
	/* GMT_check_R_J (&rlong0); */
	if (GMT_convert_latitudes) {	/* Set Conformal radius and pole latitude */
		GMT_scale_eqrad ();
		clat = GMT_latg_to_latc (plat);
	}
	else
		clat = plat;

	project_info.central_meridian = rlong0;
	project_info.pole = plat;		/* This is always geodetic */
	project_info.sinp = sind (clat);	/* These may be conformal */
	project_info.cosp = cosd (clat);
	project_info.north_pole = (plat > 0.0);
	project_info.s_c = 2.0 * project_info.EQ_RAD * gmtdefs.map_scale_factor;
	project_info.s_ic = 1.0 / project_info.s_c;
}

void GMT_plrs_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to x/y using Spherical polar projection */
	double rho, slon, clon;
	
	if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;

	lon *= D2R;
	sincos (lon, &slon, &clon);
	if (project_info.north_pole) {
		rho = project_info.s_c * tan (M_PI_4 - 0.5 * D2R * lat);
		*y = -rho * clon;
		*x =  rho * slon;
	}
	else {
		rho = project_info.s_c * tan (M_PI_4 + 0.5 * D2R * lat);
		*y = rho * clon;
		*x = rho * slon;
	}
	if (GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= project_info.Dx;
		(*y) *= project_info.Dy;
	}
}

void GMT_iplrs_sph (double *lon, double *lat, double x, double y)
{
	double c;

	/* Convert Spherical polar x/y to lon/lat */
	
	if (x == 0.0 && y == 0.0) {
		*lon = project_info.central_meridian;
		*lat = project_info.pole;
		return;
	}

	if (GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}
	c = 2.0 * atan (hypot (x, y) * project_info.s_ic);

	if (project_info.north_pole) {
		*lon = project_info.central_meridian + d_atan2 (x, -y) * R2D;	
		*lat = d_asin (cos (c)) * R2D;
	}
	else {
		*lon = project_info.central_meridian + d_atan2 (x, y) * R2D;	
		*lat = d_asin (-cos (c)) * R2D;
	}
	if (GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);

}

void GMT_stereo1_sph (double lon, double lat, double *x, double *y)
{
	double dlon, sin_dlon, cos_dlon, s, c, cc, A;
	
	/* Convert lon/lat to x/y using Spherical stereographic projection, oblique view */

	if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
	dlon = D2R * (lon - project_info.central_meridian);
	lat *= D2R;
	sincos (dlon, &sin_dlon, &cos_dlon);
	sincos (lat, &s, &c);
	cc = c * cos_dlon;
	A = project_info.s_c / (1.0 + project_info.sinp * s + project_info.cosp * cc);
	*x = A * c * sin_dlon;
	*y = A * (project_info.cosp * s - project_info.sinp * cc);
	if (GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= project_info.Dx;
		(*y) *= project_info.Dy;
	}
}

void GMT_istereo_sph (double *lon, double *lat, double x, double y)
{
	double rho, c, sin_c, cos_c;
	
	if (x == 0.0 && y == 0.0) {
		*lon = project_info.central_meridian;
		*lat = project_info.pole;
	}
	else {
		if (GMT_convert_latitudes) {	/* Undo effect of fudge factors */
			x *= project_info.iDx;
			y *= project_info.iDy;
		}
		rho = hypot (x, y);
		c = 2.0 * atan (rho * project_info.s_ic);
		sincos (c, &sin_c, &cos_c);
		*lat = d_asin (cos_c * project_info.sinp + (y * sin_c * project_info.cosp / rho)) * R2D;
		*lon = R2D * atan (x * sin_c / (rho * project_info.cosp * cos_c - y * project_info.sinp * sin_c))
		       + project_info.central_meridian;
		if (GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
	}
}

/* Spherical equatorial view */

void GMT_stereo2_sph (double lon, double lat, double *x, double *y)
{
	double dlon, s, c, clon, slon, A;
	
	/* Convert lon/lat to x/y using stereographic projection, equatorial view */

	dlon = lon - project_info.central_meridian;
	if (fabs (dlon - 180.0) < GMT_CONV_LIMIT) {
		*x = *y = 0.0;
	}
	else {
		if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
		dlon *= D2R;
		lat *= D2R;
		sincos (lat, &s, &c);
		sincos (dlon, &slon, &clon);
		A = project_info.s_c / (1.0 + c * clon);
		*x = A * c * slon;
		*y = A * s;
		if (GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= project_info.Dx;
			(*y) *= project_info.Dy;
		}
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT CONFORMAL CONIC PROJECTION
 */

int GMT_map_init_lambert (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax;

	GMT_convert_latitudes = GMT_quickconic();
	if (GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vlamb (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];
	if (SPHERICAL || GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFI)GMT_lamb_sph;
		GMT_inverse = (PFI)GMT_ilamb_sph;
	}
	else {
		GMT_forward = (PFI)GMT_lamb;
		GMT_inverse = (PFI)GMT_ilamb;
	}
			
	if (!project_info.region) {	/* Rectangular box given*/
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	else {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
		search = FALSE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);
	gmtdefs.n_lat_nodes = 2;
	frame_info.horizontal = TRUE;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
 	GMT_meridian_straight = TRUE;

	return (search);
}

void GMT_vlamb (double rlong0, double rlat0, double pha, double phb)
{
	/* Set up a Lambert Conformal Conic projection (Spherical when e = 0) */
	
	double t_pha, m_pha, t_phb, m_phb, t_rlat0;
	
	GMT_check_R_J (&rlong0);
	project_info.north_pole = (rlat0 > 0.0);
	project_info.pole = (project_info.north_pole) ? 90.0 : -90.0;
	pha *= D2R;
	phb *= D2R;
	
	t_pha = tan (M_PI_4 - 0.5 * pha) / pow ((1.0 - project_info.ECC * 
		sin (pha)) / (1.0 + project_info.ECC * sin (pha)), project_info.half_ECC);
	m_pha = cos (pha) / d_sqrt (1.0 - project_info.ECC2 
		* pow (sin (pha), 2.0));
	t_phb = tan (M_PI_4 - 0.5 * phb) / pow ((1.0 - project_info.ECC *
		sin (phb)) / (1.0 + project_info.ECC * sin (phb)), project_info.half_ECC);
	m_phb = cos (phb) / d_sqrt (1.0 - project_info.ECC2 
		* pow (sin (phb), 2.0));
	t_rlat0 = tan (M_PI_4 - 0.5 * rlat0 * D2R) /
		pow ((1.0 - project_info.ECC * sin (rlat0 * D2R)) /
		(1.0 + project_info.ECC * sin (rlat0 * D2R)), project_info.half_ECC);
	
	if(pha != phb)
		project_info.l_N = (d_log(m_pha) - d_log(m_phb))/(d_log(t_pha) - d_log(t_phb));
	else
		project_info.l_N = sin(pha);
	
	project_info.l_i_N = 1.0 / project_info.l_N;
	project_info.l_F = m_pha / (project_info.l_N * pow (t_pha, project_info.l_N));
	project_info.central_meridian = rlong0;
	project_info.l_rF = project_info.EQ_RAD * project_info.l_F;
	project_info.l_i_rF = 1.0 / project_info.l_rF;
	project_info.l_rho0 = project_info.l_rF * pow (t_rlat0, project_info.l_N);
	project_info.l_Nr = project_info.l_N * D2R;
	project_info.l_i_Nr = 1.0 / project_info.l_Nr;
}

void GMT_lamb (double lon, double lat, double *x, double *y)
{
	double rho, theta, hold1, hold2, hold3, es, s, c;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lat *= D2R;

	es = project_info.ECC * sin (lat);
	hold2 = pow (((1.0 - es) / (1.0 + es)), project_info.half_ECC);
	hold3 = tan (M_PI_4 - 0.5 * lat);
	if (fabs (hold3) < GMT_CONV_LIMIT)
		hold1 = 0.0;
	else
		hold1 = pow (hold3 / hold2, project_info.l_N);
	rho = project_info.l_rF * hold1;
	theta = project_info.l_Nr * lon;
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.l_rho0 - rho * c;
}

void GMT_ilamb (double *lon, double *lat, double x, double y)
{
	int i;
	double theta, rho, t, tphi, phi, delta, dy, r;
	
	theta = atan (x / (project_info.l_rho0 - y));
	*lon = theta * project_info.l_i_Nr + project_info.central_meridian;
	
	dy = project_info.l_rho0 - y;
	rho = copysign (hypot (x, dy), project_info.l_N);
	t = pow ((rho * project_info.l_i_rF), project_info.l_i_N);
	tphi = M_PI_2 - 2.0 * atan (t);
	delta = 1.0;
	for (i = 0; i < 100 && delta > GMT_CONV_LIMIT; i++) {
		r = project_info.ECC * sin (tphi);
		phi = M_PI_2 - 2.0 * atan (t * pow ((1.0 - r) / (1.0 + r), project_info.half_ECC));
		delta = fabs (fabs (tphi) - fabs (phi));
		tphi = phi;
	}
	*lat = phi * R2D;
}

/* Spherical cases of Lambert */

void GMT_lamb_sph (double lon, double lat, double *x, double *y)
{
	double rho, theta, A, t, s, c;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	lat *= D2R;
	t = tan (M_PI_4 - 0.5 * lat);
	A = (fabs (t) < GMT_CONV_LIMIT) ? 0.0 : pow (t, project_info.l_N);
	rho = project_info.l_rF * A;
	theta = project_info.l_Nr * lon;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.l_rho0 - rho * c;
}

void GMT_ilamb_sph (double *lon, double *lat, double x, double y)
{
	double theta, rho, t;
	
	theta = atan (x / (project_info.l_rho0 - y));
	rho = hypot (x, project_info.l_rho0 - y);
	if (project_info.l_N < 0.0) rho = -rho;
	t = pow ((rho * project_info.l_i_rF), -project_info.l_i_N);

	*lon = theta * project_info.l_i_Nr + project_info.central_meridian;
	*lat = 2.0 * R2D * atan (t) - 90.0;
	if (GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE OBLIQUE MERCATOR PROJECTION
 */

int GMT_map_init_oblique (void) {
	double xmin, xmax, ymin, ymax;
	double o_x, o_y, p_x, p_y, c_x, c_y, c, az, b_x, b_y, w, e, s, n, P[3];

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;	/* To get plot-units / m */
			
	o_x = project_info.pars[0];	o_y = project_info.pars[1];
			
	if (irint (project_info.pars[6]) == 1) {	/* Must get correct origin, then get second point */
		p_x = project_info.pars[2];	p_y = project_info.pars[3];
				
	 	project_info.o_pole_lon = D2R * p_x;
	 	project_info.o_pole_lat = D2R * p_y;
	 	project_info.o_sin_pole_lat = sind (p_y);
	 	project_info.o_cos_pole_lat = cosd (p_y);
	 	
	 	/* Find azimuth to pole, add 90, and compute second point 10 degrees away */
	 	
		GMT_get_origin (o_x, o_y, p_x, p_y, &o_x, &o_y);
	 	az = R2D * atan (cosd (p_y) * sind (p_x - o_x) / (cosd (o_y) * sind (p_y) - sind (o_y) * cosd (p_y) * cosd (p_x - o_x))) + 90.0;
	 	c = 10.0;	/* compute point 10 degrees from origin along azimuth */
	 	b_x = o_x + R2D * atan (sind (c) * sind (az) / (cosd (o_y) * cosd (c) - sind (o_y) * sind (c) * cosd (az)));
	 	b_y = R2D * d_asin (sind (o_y) * cosd (c) + cosd (o_y) * sind (c) * cosd (az));
	 	project_info.pars[0] = o_x;	project_info.pars[1] = o_y;
	 	project_info.pars[2] = b_x;	project_info.pars[3] = b_y;
	 }
	 else {	/* Just find pole */
		b_x = project_info.pars[2];	b_y = project_info.pars[3];
	 	GMT_get_rotate_pole (o_x, o_y, b_x, b_y);
	}

	/* Here we have pole and origin */
	
	/* Get forward pole and origin vectors FP, FC */
	GMT_geo_to_cart (&project_info.o_pole_lat, &project_info.o_pole_lon, project_info.o_FP, FALSE);
	GMT_geo_to_cart (&o_y, &o_x, P, TRUE);	/* P points to origin  */
	GMT_cross3v (project_info.o_FP, P, project_info.o_FC);
	GMT_normalize3v (project_info.o_FC);
	/* Get inverse pole and origin vectors FP, FC */
	GMT_obl (0.0, M_PI_2, &p_x, &p_y);
	GMT_geo_to_cart (&p_y, &p_x, project_info.o_IP, FALSE);
	GMT_obl (0.0, 0.0, &c_x, &c_y);
	GMT_geo_to_cart (&c_y, &c_x, P, FALSE);	/* P points to origin  */
	GMT_cross3v (project_info.o_IP, P, project_info.o_IC);
	GMT_normalize3v (project_info.o_IC);

	GMT_vmerc (0.0);
	project_info.m_mx = project_info.m_m * D2R;
	project_info.m_imx = project_info.m_im * R2D;
			
	if (project_info.region) {	/* Gave oblique degrees */
		/* Convert oblique wesn in degrees to meters using regular Mercator */
		if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT) {
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
	GMT_forward = (PFI)GMT_oblmrc;		GMT_inverse = (PFI)GMT_ioblmrc;
	GMT_outside = (PFI) GMT_rect_outside;
	GMT_crossing = (PFI) GMT_rect_crossing;
	GMT_overlap = (PFI) GMT_rect_overlap;
	GMT_map_clip = (PFI) GMT_rect_clip;
	GMT_left_edge = (PFD) GMT_left_rect;
	GMT_right_edge = (PFD) GMT_right_rect;
			
	GMT_world_map = (fabs (fabs (w - e) - 360.0) < SMALL);
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
	return (TRUE);
}

void GMT_oblmrc (double lon, double lat, double *x, double *y)
{
	/* Convert a longitude/latitude point to Oblique Mercator coordinates 
	 * by way of rotation coordinates and then using regular Mercator */
	double tlon, tlat;
	
	GMT_obl (lon * D2R, lat * D2R, &tlon, &tlat);

	*x = project_info.m_m * tlon;
	*y = (fabs (tlat) < M_PI_2) ? project_info.m_m * d_log (tan (M_PI_4 + 0.5 * tlat)) : copysign (DBL_MAX, tlat);
}

void GMT_ioblmrc (double *lon, double *lat, double x, double y)
{
	/* Convert a longitude/latitude point from Oblique Mercator coordinates 
	 * by way of regular Mercator and then rotate coordinates */
	double tlon, tlat;
	
	tlon = x * project_info.m_im;
	tlat = atan (sinh (y * project_info.m_im));

	GMT_iobl (lon, lat, tlon, tlat);
	
	(*lon) *= R2D;
	(*lat) *= R2D;
}

void GMT_obl (double lon, double lat, double *olon, double *olat)
{
	/* Convert a longitude/latitude point to Oblique lon/lat (all in rads) */
	double p_cross_x[3], X[3];
	
	GMT_geo_to_cart (&lat, &lon, X, FALSE);

	*olat = d_asin (GMT_dot3v (X, project_info.o_FP));

	GMT_cross3v (project_info.o_FP, X, p_cross_x);
	GMT_normalize3v (p_cross_x);
	
	*olon = copysign (d_acos (GMT_dot3v (p_cross_x, project_info.o_FC)), GMT_dot3v (X, project_info.o_FC));
}

void GMT_iobl (double *lon, double *lat, double olon, double olat)
{
	/* Convert a longitude/latitude point from Oblique lon/lat  (all in rads) */
	double p_cross_x[3], X[3];
	
	GMT_geo_to_cart (&olat, &olon, X, FALSE);
	*lat = d_asin (GMT_dot3v (X, project_info.o_IP));

	GMT_cross3v (project_info.o_IP, X, p_cross_x);
	GMT_normalize3v (p_cross_x);
	
	*lon = copysign (d_acos (GMT_dot3v (p_cross_x, project_info.o_IC)), GMT_dot3v (X, project_info.o_IC));
	
	while ((*lon) < 0.0) (*lon) += TWO_PI;
	while ((*lon) >= TWO_PI) (*lon) -= TWO_PI;
}

void GMT_pole_rotate_forward (double lon, double lat, double *tlon, double *tlat)
{
	/* Given the pole position in project_info, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 */

	double sin_lat, cos_lat, cos_lon, sin_lon, cc;
	 
	lon *= D2R;	lat *= D2R;
	lon -= project_info.o_pole_lon;
	sincos (lat, &sin_lat, &cos_lat);
	sincos (lon, &sin_lon, &cos_lon);
	cc = cos_lat * cos_lon;
	*tlat = R2D * d_asin (project_info.o_sin_pole_lat * sin_lat + project_info.o_cos_pole_lat * cc);
	*tlon = R2D * (project_info.o_beta + d_atan2 (cos_lat * sin_lon, project_info.o_sin_pole_lat * cc - project_info.o_cos_pole_lat * sin_lat));
}
	 
void GMT_pole_rotate_inverse (double *lon, double *lat, double tlon, double tlat)
{
	/* Given the pole position in project_info, geographical coordinates 
	 * are computed from oblique coordinates assuming a spherical earth.
	 */
	 
	double sin_tlat, cos_tlat, cos_tlon, sin_tlon, cc;
	 
	tlon *= D2R;	tlat *= D2R;
	tlon -= project_info.o_beta;
	sincos (tlat, &sin_tlat, &cos_tlat);
	sincos (tlon, &sin_tlon, &cos_tlon);
	cc = cos_tlat * cos_tlon;
	*lat = R2D * d_asin (project_info.o_sin_pole_lat * sin_tlat - project_info.o_cos_pole_lat * cc);
	*lon = R2D * (project_info.o_pole_lon + d_atan2 (cos_tlat * sin_tlon, project_info.o_sin_pole_lat * cc + project_info.o_cos_pole_lat * sin_tlat));
}

void GMT_get_rotate_pole (double lon1, double lat1, double lon2, double lat2)
{
	double plon, plat, beta, dummy, sin_lat1, cos_lat1, sin_lat2, cos_lat2;
	double aix_cpp_sucks1, aix_cpp_sucks2;

	lat1 *= D2R;	lat2 *= D2R;
	lon1 *= D2R;	lon2 *= D2R;
	sin_lat1 = sin (lat1);
	sin_lat2 = sin (lat2);
	cos_lat1 = cos (lat1);
	cos_lat2 = cos (lat2);
	
	aix_cpp_sucks1 = cos_lat1 * sin_lat2 * cos (lon1) - sin_lat1 * cos_lat2 * cos (lon2);
	aix_cpp_sucks2 = sin_lat1 * cos_lat2 * sin (lon2) - cos_lat1 * sin_lat2 * sin (lon1);
	plon = d_atan2 (aix_cpp_sucks1, aix_cpp_sucks2);
	plat = atan (-cos (plon - lon1) / tan (lat1));
	if (plat < 0.0) {
		plat = -plat;
		plon += M_PI;
		if (plon >= TWO_PI) plon -= TWO_PI;
	}
	project_info.o_pole_lon = plon;
	project_info.o_pole_lat = plat;
	project_info.o_sin_pole_lat = sin (plat);
	project_info.o_cos_pole_lat = cos (plat);
	GMT_pole_rotate_forward (lon1, lat1, &beta, &dummy);
	project_info.o_beta = -beta * D2R;
}

void GMT_get_origin (double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2)
{
	double beta, dummy, d, az, c;
	
	
	/* Now find origin that is 90 degrees from pole, let oblique lon=0 go through lon1/lat1 */
#ifdef aix 
        c = cosd (lat_p) * cosd (lat1) * cosd (lon1-lon_p);
        c = R2D * d_acos (sind (lat_p) * sind (lat1) + c);
#else 
        c = R2D * d_acos (sind (lat_p) * sind (lat1) + cosd (lat_p) * cosd (lat1) * cosd (lon1-lon_p));
#endif 	
	
	if (c != 90.0) {	/* Not true origin */
		d = fabs (90.0 - c);
		az = R2D * d_asin (sind (lon_p-lon1) * cosd (lat_p) / sind (c));
		if (c < 90.0) az += 180.0;
		*lat2 = R2D * d_asin (sind (lat1) * cosd (d) + cosd (lat1) * sind (d) * cosd (az));
		*lon2 = lon1 + R2D * d_atan2 (sind (d) * sind (az), cosd (lat1) * cosd (d) - sind (lat1) * sind (d) * cosd (az));
		if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Correct projection origin = %g/%g\n", GMT_program, *lon2, *lat2);
	}
	else {
		*lon2 = lon1;
		*lat2 = lat1;
	}
	
	GMT_pole_rotate_forward (*lon2, *lat2, &beta, &dummy);
	
	
	project_info.o_beta = -beta * D2R;
}

/*
 *	TRANSFORMATION ROUTINES FOR THE TRANSVERSE MERCATOR PROJECTION (TM)
 */

int GMT_map_init_tm (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, w, e, dummy;

	/* Wrap and truncations are in y, not x for TM */

	GMT_wrap_around_check = (PFI) GMT_wrap_around_check_tm;
	GMT_map_jump = (PFI) GMT_map_jump_tm;
	GMT_will_it_wrap = (PFB) GMT_will_it_wrap_tm;
	GMT_this_point_wraps = (PFB) GMT_this_point_wraps_tm;
	GMT_get_crossings = (PFV) GMT_get_crossings_tm;
	GMT_truncate = (PFI) GMT_truncate_tm;

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 1.0;	/* Select default map scale for TM */
	GMT_convert_latitudes = GMT_quicktm (project_info.pars[0], 10.0);
	if (GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_vtm (project_info.pars[0], project_info.pars[1]);
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	if (SPHERICAL || GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFI)GMT_tm_sph;
		GMT_inverse = (PFI)GMT_itm_sph;
	}
	else {
		GMT_forward = (PFI)GMT_tm;
		GMT_inverse = (PFI)GMT_itm;
	}
			
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
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
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
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
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		GMT_world_map_tm = (fabs (project_info.n - project_info.s) < GMT_CONV_LIMIT);
		GMT_world_map = FALSE;
		search = FALSE;
	}
	else { /* Find min values */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		GMT_world_map_tm = FALSE;
		GMT_world_map = (fabs (project_info.s - project_info.n) < SMALL);
		search = TRUE;
	}
			
	frame_info.horizontal = TRUE;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
			
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	
	return (search);
}

void GMT_vtm (double lon0, double lat0)
{
	/* Set up an TM projection */
	double e1, lat2, s2, c2;
	
	/* GMT_check_R_J (&lon0); */
	e1 = (1.0 - d_sqrt (project_info.one_m_ECC2)) / (1.0 + d_sqrt (project_info.one_m_ECC2));
	project_info.t_e2 = project_info.ECC2 * project_info.i_one_m_ECC2;
	project_info.t_c1 = 1.0 - (1.0/4.0) * project_info.ECC2 - (3.0/64.0) * project_info.ECC4 - (5.0/256.0) * project_info.ECC6;
	project_info.t_c2 = -((3.0/8.0) * project_info.ECC2 + (3.0/32.0) * project_info.ECC4 + (25.0/768.0) * project_info.ECC6);
	project_info.t_c3 = (15.0/128.0) * project_info.ECC4 + (45.0/512.0) * project_info.ECC6;
	project_info.t_c4 = -(35.0/768.0) * project_info.ECC6;
	project_info.t_i1 = 1.0 / (project_info.EQ_RAD * project_info.t_c1);
	project_info.t_i2 = (3.0/2.0) * e1 - (29.0/12.0) * pow (e1, 3.0);
	project_info.t_i3 = (21.0/8.0) * e1 * e1 - (1537.0/128.0) * pow (e1, 4.0);
	project_info.t_i4 = (151.0/24.0) * pow (e1, 3.0);
	project_info.t_i5 = (1097.0/64.0) * pow (e1, 4.0);
	project_info.central_meridian = lon0;
	lat0 *= D2R;
	project_info.t_lat0 = lat0;	/* In radians */
	lat2 = 2.0 * lat0;
	sincos (lat2, &s2, &c2);
	project_info.t_M0 = project_info.EQ_RAD * (project_info.t_c1 * lat0 + s2 * (project_info.t_c2 + c2 * (project_info.t_c3 + c2 * project_info.t_c4)));
	project_info.t_r = project_info.EQ_RAD * gmtdefs.map_scale_factor;
	project_info.t_ir = 1.0 / project_info.t_r;
}

/* Ellipsoidal TM functions */

void GMT_tm (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to TM x/y */
	double N, T, T2, C, A, M, dlon, tan_lat, A2, A3, A5, lat2, s, c, s2, c2;
	
	if (fabs (fabs (lat) - 90.0) < GMT_CONV_LIMIT) {
		M = project_info.EQ_RAD * project_info.t_c1 * M_PI_2;
		*x = 0.0;
		*y = gmtdefs.map_scale_factor * M;
	}
	else {
		lat *= D2R;
		lat2 = 2.0 * lat;
		sincos (lat, &s, &c);
		sincos (lat2, &s2, &c2);
		tan_lat = s / c;
		M = project_info.EQ_RAD * (project_info.t_c1 * lat + s2 * (project_info.t_c2 + c2 * (project_info.t_c3 + c2 * project_info.t_c4)));
		dlon = lon - project_info.central_meridian;
		if (fabs (dlon) > 360.0) dlon += copysign (360.0, -dlon);
		if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
		N = project_info.EQ_RAD / d_sqrt (1.0 - project_info.ECC2 * s * s);
		T = tan_lat * tan_lat;
		T2 = T * T;
		C = project_info.t_e2 * c * c;
		A = dlon * D2R * c;
		A2 = A * A;	A3 = A2 * A;	A5 = A3 * A2;
		*x = gmtdefs.map_scale_factor * N * (A + (1.0 - T + C) * (A3 * 0.16666666666666666667)
			+ (5.0 - 18.0 * T + T2 + 72.0 * C - 58.0 * project_info.t_e2) * (A5 * 0.00833333333333333333));
		A3 *= A;	A5 *= A;
		*y = gmtdefs.map_scale_factor * (M - project_info.t_M0 + N * tan_lat * (0.5 * A2 + (5.0 - T + 9.0 * C + 4.0 * C * C) * (A3 * 0.04166666666666666667)
			+ (61.0 - 58.0 * T + T2 + 600.0 * C - 330.0 * project_info.t_e2) * (A5 * 0.00138888888888888889)));
	}
}

void GMT_itm (double *lon, double *lat, double x, double y)
{
	/* Convert TM x/y to lon/lat */
	double M, mu, u2, s, c, phi1, C1, C12, T1, T12, tmp, tmp2, N1, R_1, D, D2, D3, D5, tan_phi1, cp2;
	
	M = y / gmtdefs.map_scale_factor + project_info.t_M0;
	mu = M * project_info.t_i1;

	u2 = 2.0 * mu;
	sincos (u2, &s, &c);
	phi1 = mu + s * (project_info.t_i2 + c * (project_info.t_i3 + c * (project_info.t_i4 + c * project_info.t_i5)));

	sincos (phi1, &s, &c);
	tan_phi1 = s / c;
	cp2 = c * c;
	C1 = project_info.t_e2 * cp2;
	C12 = C1 * C1;
	T1 = tan_phi1 * tan_phi1;
	T12 = T1 * T1;
	tmp = 1.0 - project_info.ECC2 * (1.0 - cp2);
	tmp2 = d_sqrt (tmp);
	N1 = project_info.EQ_RAD / tmp2;
	R_1 = project_info.EQ_RAD * project_info.one_m_ECC2 / (tmp * tmp2);
	D = x / (N1 * gmtdefs.map_scale_factor);
	D2 = D * D;	D3 = D2 * D;	D5 = D3 * D2;
	
	*lon = project_info.central_meridian + R2D * (D - (1.0 + 2.0 * T1 + C1) * (D3 * 0.16666666666666666667) 
		+ (5.0 - 2.0 * C1 + 28.0 * T1 - 3.0 * C12 + 8.0 * project_info.t_e2 + 24.0 * T12)
		* (D5 * 0.00833333333333333333)) / c;
	D3 *= D;	D5 *= D;
	*lat = phi1 - (N1 * tan_phi1 / R_1) * (0.5 * D2 -
		(5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C12 - 9.0 * project_info.t_e2) * (D3 * 0.04166666666666666667)
		+ (61.0 + 90.0 * T1 + 298 * C1 + 45.0 * T12 - 252.0 * project_info.t_e2 - 3.0 * C12) * (D5 * 0.00138888888888888889));
	(*lat) *= R2D;
}

/*Spherical TM functions */

void GMT_tm_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to TM x/y by spherical formula */
	double dlon, b, clat, slat, clon, slon, xx, yy;

	dlon = lon - project_info.central_meridian;
	if (fabs (dlon) > 360.0) dlon += copysign (360.0, -dlon);
	if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
	
	if (fabs (lat) > 90.0) {
		/* Invalid latitude.  Treat as in GMT_merc_sph(), but transversely:  */
		*x = copysign (1.0e100, dlon);
		*y = 0.0;
		return;
	}

	if (GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	lat *= D2R;
	dlon *= D2R;
	sincos (lat, &slat, &clat);
	sincos (dlon, &slon, &clon);
	b = clat * slon;
	if (fabs(b) >= 1.0) {
		/* This corresponds to the transverse "pole"; the point at x = +-infinity, y = -lat0.  
			Treat as in GMT_merc_sph(), but transversely:  */
		*x = copysign (1.0e100, dlon);
		*y = -project_info.t_r * project_info.t_lat0;
		return;
	}

	xx = atanh (b);
	
	/* this should get us "over the pole";
	   see not Snyder's formula but his example Fig. 10 on p. 50:  */

	yy = atan2 (slat, (clat * clon)) - project_info.t_lat0;
	if (yy < -M_PI_2) yy += TWO_PI;
	*x = project_info.t_r * xx;
	*y = project_info.t_r * yy;
}

void GMT_itm_sph (double *lon, double *lat, double x, double y)
{
	/* Convert TM x/y to lon/lat by spherical approximation.  */

	double xx, yy, sinhxx, coshxx, sind, cosd, lambda, phi;

	xx = x * project_info.t_ir;
	yy = y * project_info.t_ir + project_info.t_lat0;

	sinhxx = sinh (xx);
	coshxx = cosh (xx);

	sincos (yy, &sind, &cosd);
	phi = R2D * asin (sind / coshxx);
	*lat = phi;
	
	lambda = R2D * atan2 (sinhxx, cosd);
	*lon = lambda + project_info.central_meridian;
	if (GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE UNIVERSAL TRANSVERSE MERCATOR PROJECTION (UTM)
 */
 
int GMT_map_init_utm (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, lon0;

	if (gmtdefs.map_scale_factor == -1.0) gmtdefs.map_scale_factor = 0.9996;	/* Select default map scale for UTM */
	lon0 = 180.0 + 6.0 * project_info.pars[0] - 3.0;	/* Central meridian for this UTM zone */
	if (lon0 >= 360.0) lon0 -= 360.0;
	GMT_convert_latitudes = GMT_quicktm (lon0, 10.0);
	if (GMT_convert_latitudes) GMT_scale_eqrad ();
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
	if (SPHERICAL || GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		GMT_forward = (PFI)GMT_utm_sph;
		GMT_inverse = (PFI)GMT_iutm_sph;
	}
	else {
		GMT_forward = (PFI)GMT_utm;
		GMT_inverse = (PFI)GMT_iutm;
	}
			
	if (fabs (project_info.w - project_info.e) > 360.0) {	/* -R in UTM meters */
		(*GMT_inverse) (&project_info.w, &project_info.s, project_info.w, project_info.s);
		(*GMT_inverse) (&project_info.e, &project_info.n, project_info.e, project_info.n);
		project_info.region = FALSE;
	}
	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		search = FALSE;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}

	frame_info.horizontal = TRUE;

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
			
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	
	return (search);
}

/* Ellipsoidal UTM */

void GMT_utm (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm (lon, lat, x, y);
	(*x) += FALSE_EASTING;
	if (!project_info.north_pole) (*y) += FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm (double *lon, double *lat, double x, double y)
{
	/* Convert UTM x/y to lon/lat */

	x -= FALSE_EASTING;
	if (!project_info.north_pole) y -= FALSE_NORTHING;
	GMT_itm (lon, lat, x, y);
}

/* Spherical UTM */

void GMT_utm_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm_sph (lon, lat, x, y);
	(*x) += FALSE_EASTING;
	if (!project_info.north_pole) (*y) += FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm_sph (double *lon, double *lat, double x, double y)
{
	/* Convert UTM x/y to lon/lat */

	x -= FALSE_EASTING;
	if (!project_info.north_pole) y -= FALSE_NORTHING;
	GMT_itm_sph (lon, lat, x, y);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT AZIMUTHAL EQUAL AREA PROJECTION
 */

int GMT_map_init_lambeq (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dummy, radius, latg, D, s, c;

	project_info.Dx = project_info.Dy = 1.0;
	latg = project_info.pars[1];

	GMT_set_polar (project_info.pars[1]);

	GMT_convert_latitudes = !SPHERICAL;

	if (GMT_convert_latitudes) {
		GMT_scale_eqrad ();
		project_info.pars[1] = GMT_latg_to_lata (project_info.pars[1]);
	}

	GMT_vlambeq (project_info.pars[0], project_info.pars[1]);

	if (GMT_convert_latitudes) {
		s = sind (latg);	c = cosd (latg);	/* Need original geographic pole coordinates */
		D = (project_info.polar) ? 1.0 : (gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / GMT_lat_swap_vals.ra) * c / (project_info.cosp * d_sqrt (1.0 - project_info.ECC2 * s * s));
		project_info.Dx = D;
		project_info.Dy = 1.0 / D;
	}
	project_info.iDx = 1.0 / project_info.Dx;
	project_info.iDy = 1.0 / project_info.Dy;

	GMT_forward = (PFI)GMT_lambeq;		GMT_inverse = (PFI)GMT_ilambeq;
	if (project_info.units_pr_degree) {
		GMT_vlambeq (0.0, 90.0);
		GMT_lambeq (0.0, fabs(project_info.pars[3]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
		GMT_vlambeq (project_info.pars[0], project_info.pars[1]);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[2];

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside2;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s < 0.0) project_info.s = 0.0;
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n > 0.0) project_info.n = 0.0;
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT || fabs (project_info.e - project_info.w) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFI) GMT_polar_outside;
			GMT_crossing = (PFI) GMT_wesn_crossing;
			GMT_overlap = (PFI) GMT_wesn_overlap;
			GMT_map_clip = (PFI) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			gmtdefs.n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmin = ymin = -d_sqrt (2.0) * project_info.EQ_RAD;
			xmax = ymax = -xmin;
			GMT_outside = (PFI) GMT_radial_outside;
			GMT_crossing = (PFI) GMT_radial_crossing;
			GMT_overlap = (PFI) GMT_radial_overlap;
			GMT_map_clip = (PFI) GMT_radial_clip;
			gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		search = FALSE;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}
				
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;
	
	return (search);
}

void GMT_vlambeq (double lon0, double lat0)
{
	/* Set up Spherical Lambert Azimuthal Equal-Area projection */
	
	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	project_info.sinp = sind (lat0);
	project_info.cosp = cosd (lat0);
}

void GMT_lambeq (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Spherical Lambert Azimuthal Equal-Area x/y */
	double k, tmp, sin_lat, cos_lat, sin_lon, cos_lon, c;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	lon *= D2R;
	lat *= D2R;
	sincos (lat, &sin_lat, &cos_lat);
	sincos (lon, &sin_lon, &cos_lon);
	c = cos_lat * cos_lon;
	tmp = 1.0 + project_info.sinp * sin_lat + project_info.cosp * c;
	if (tmp > 0.0) {
		k = project_info.EQ_RAD * d_sqrt (2.0 / tmp);
		*x = k * cos_lat * sin_lon;
		*y = k * (project_info.cosp * sin_lat - project_info.sinp * c);
		if (GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= project_info.Dx;
			(*y) *= project_info.Dy;
		}
	}
	else
		*x = *y = -DBL_MAX;
}

void GMT_ilambeq (double *lon, double *lat, double x, double y)
{
	/* Convert Lambert Azimuthal Equal-Area x/yto lon/lat */
	double rho, c, sin_c, cos_c;
	
	if (GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}

	rho = hypot (x, y);
	
	if (fabs (rho) < GMT_CONV_LIMIT) {
		*lat = project_info.pole;
		*lon = project_info.central_meridian;
	}
	else {
		c = 2.0 * d_asin (0.5 * rho * project_info.i_EQ_RAD);
		sincos (c, &sin_c, &cos_c);
		*lat = d_asin (cos_c * project_info.sinp + (y * sin_c * project_info.cosp / rho)) * R2D;
		if (project_info.n_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, -y);
		else if (project_info.s_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, y);
		else
			*lon = project_info.central_meridian +
				R2D * d_atan2 (x * sin_c, (rho * project_info.cosp * cos_c - y * project_info.sinp * sin_c));
		if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ORTHOGRAPHIC PROJECTION
 */

int GMT_map_init_ortho (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);
	
	if (project_info.units_pr_degree) {
		GMT_vortho (0.0, 90.0);
		GMT_ortho (0.0, fabs(project_info.pars[3]), &dummy, &radius);
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[2];
				
	GMT_vortho (project_info.pars[0], project_info.pars[1]);
	GMT_forward = (PFI)GMT_ortho;		GMT_inverse = (PFI)GMT_iortho;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside2;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
	}
	else {
		if (project_info.polar) {	/* Polar aspect */
			if (project_info.north_pole) {
				if (project_info.s < 0.0) project_info.s = 0.0;
				if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
			}
			else {
				if (project_info.n > 0.0) project_info.n = 0.0;
				if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
			}
			if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT || fabs (project_info.e - project_info.w) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFI) GMT_polar_outside;
			GMT_crossing = (PFI) GMT_wesn_crossing;
			GMT_overlap = (PFI) GMT_wesn_overlap;
			GMT_map_clip = (PFI) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			gmtdefs.n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmin = ymin = -project_info.EQ_RAD;
			xmax = ymax = -xmin;
			GMT_outside = (PFI) GMT_radial_outside;
			GMT_crossing = (PFI) GMT_radial_crossing;
			GMT_overlap = (PFI) GMT_radial_overlap;
			GMT_map_clip = (PFI) GMT_radial_clip;
			gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		search = FALSE;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}
				
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;
	
	return (search);
}

void GMT_vortho (double lon0, double lat0)
{
	/* Set up Orthographic projection */
	
	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	project_info.sinp = sin (lat0 * D2R);
	project_info.cosp = cos (lat0 * D2R);
}

void GMT_ortho (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Orthographic x/y */
	double sin_lat, cos_lat, sin_lon, cos_lon;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;
	lat *= D2R;

	sincos (lat, &sin_lat, &cos_lat);
	sincos (lon, &sin_lon, &cos_lon);
	*x = project_info.EQ_RAD * cos_lat * sin_lon;
	*y = project_info.EQ_RAD * (project_info.cosp * sin_lat - project_info.sinp * cos_lat * cos_lon);
}

void GMT_iortho (double *lon, double *lat, double x, double y)
{
	/* Convert Orthographic x/yto lon/lat */
	double rho, sin_c, cos_c;
	
	rho = hypot (x, y);
	
	if (fabs (rho) < GMT_CONV_LIMIT) {
		*lat = project_info.pole;
		*lon = project_info.central_meridian;
	}
	else {
		sin_c = rho * project_info.i_EQ_RAD;
		cos_c = d_sqrt (1.0 - sin_c * sin_c);
		*lat = d_asin (cos_c * project_info.sinp + (y * sin_c * project_info.cosp / rho)) * R2D;
		if (project_info.n_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, -y);
		else if (project_info.s_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, y);
		else
			*lon = project_info.central_meridian +
				R2D * d_atan2 (x * sin_c, (rho * project_info.cosp * cos_c - y * project_info.sinp * sin_c));
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE GNOMONIC PROJECTION
 */

int GMT_map_init_gnomonic (void) {
	BOOLEAN search;
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
	GMT_forward = (PFI)GMT_gnomonic;		GMT_inverse = (PFI)GMT_ignomonic;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside2;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 30.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
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
			if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT || fabs (project_info.e - project_info.w) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFI) GMT_polar_outside;
			GMT_crossing = (PFI) GMT_wesn_crossing;
			GMT_overlap = (PFI) GMT_wesn_overlap;
			GMT_map_clip = (PFI) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			gmtdefs.n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			/* frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0; */	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmin = ymin = -project_info.EQ_RAD * tand (project_info.f_horizon);
			xmax = ymax = -xmin;
			GMT_outside = (PFI) GMT_radial_outside;
			GMT_crossing = (PFI) GMT_radial_crossing;
			GMT_overlap = (PFI) GMT_radial_overlap;
			GMT_map_clip = (PFI) GMT_radial_clip;
			gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		search = FALSE;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}
				
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[3]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);

	return (search);
}

void GMT_vgnomonic (double lon0, double lat0, double horizon)
{
	/* Set up Gnomonic projection */
	
	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.f_horizon = horizon;
	project_info.pole = lat0;
	project_info.north_pole = (lat0 > 0.0);
	lat0 *= D2R;
	project_info.sinp = sin (lat0);
	project_info.cosp = cos (lat0);
}

void GMT_gnomonic (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Gnomonic x/y */
	double k, sin_lat, cos_lat, sin_lon, cos_lon, cc;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;
	lat *= D2R;
	sincos (lat, &sin_lat, &cos_lat);
	sincos (lon, &sin_lon, &cos_lon);
	
	cc = cos_lat * cos_lon;
	k =  project_info.EQ_RAD / (project_info.sinp * sin_lat + project_info.cosp * cc);
	*x = k * cos_lat * sin_lon;
	*y = k * (project_info.cosp * sin_lat - project_info.sinp * cc);
}

void GMT_ignomonic (double *lon, double *lat, double x, double y)
{
	/* Convert Gnomonic x/y to lon/lat */
	double rho, c, sin_c, cos_c;
	
	rho = hypot (x, y);
	
	if (fabs (rho) < GMT_CONV_LIMIT) {
		*lat = project_info.pole;
		*lon = project_info.central_meridian;
	}
	else {
		c = atan (rho * project_info.i_EQ_RAD);
		sincos (c, &sin_c, &cos_c);
		*lat = d_asin (cos_c * project_info.sinp + (y * sin_c * project_info.cosp / rho)) * R2D;
		if (project_info.polar && project_info.north_pole)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, -y);
		else if (project_info.polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, y);
		else
			*lon = project_info.central_meridian +
				R2D * d_atan2 (x * sin_c, (rho * project_info.cosp * cos_c - y * project_info.sinp * sin_c));
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE AZIMUTHAL EQUIDISTANT PROJECTION
 */

int GMT_map_init_azeqdist (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	GMT_set_polar (project_info.pars[1]);
	project_info.f_horizon = 180.0;
	
	if (project_info.units_pr_degree) {
		GMT_vazeqdist (0.0, 90.0);
		GMT_azeqdist (0.0, project_info.pars[3], &dummy, &radius);
		if (fabs (radius) < GMT_CONV_LIMIT) radius = M_PI * project_info.EQ_RAD;
		project_info.x_scale = project_info.y_scale = fabs (project_info.pars[2] / radius);
	}
	else
		project_info.x_scale = project_info.y_scale = project_info.pars[2];
				
	GMT_vazeqdist (project_info.pars[0], project_info.pars[1]);
	GMT_forward = (PFI)GMT_azeqdist;		GMT_inverse = (PFI)GMT_iazeqdist;

	if (!project_info.region) {	/* Rectangular box given */
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
				
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = !(gmtdefs.oblique_annotation & 1);
		frame_info.horizontal = (fabs (project_info.pars[1]) < 60.0 && fabs (project_info.n - project_info.s) < 30.0);
		search = TRUE;
	}
	else {
		if (project_info.polar && (project_info.n - project_info.s) < 180.0) {	/* Polar aspect */
			if (!project_info.north_pole && project_info.s <= -90.0) project_info.edge[0] = FALSE;
			if (project_info.north_pole && project_info.n >= 90.0) project_info.edge[2] = FALSE;
			if (fabs (fabs (project_info.e - project_info.w) - 360.0) < GMT_CONV_LIMIT || fabs (project_info.e - project_info.w) < GMT_CONV_LIMIT) project_info.edge[1] = project_info.edge[3] = FALSE;
			GMT_outside = (PFI) GMT_polar_outside;
			GMT_crossing = (PFI) GMT_wesn_crossing;
			GMT_overlap = (PFI) GMT_wesn_overlap;
			GMT_map_clip = (PFI) GMT_wesn_clip;
			frame_info.horizontal = TRUE;
			gmtdefs.n_lat_nodes = 2;
			GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		}
		else {	/* Global view only, force wesn = 0/360/-90/90  */
			frame_info.axis[0].item[0].interval = frame_info.axis[1].item[0].interval = 0.0;	/* No annotations for global mode */
			frame_info.axis[0].item[4].interval = frame_info.axis[1].item[4].interval = 0.0;	/* No tickmarks for global mode */
			project_info.w = 0.0;
			project_info.e = 360.0;
			project_info.s = -90.0;
			project_info.n = 90.0;
			xmin = ymin = -M_PI * project_info.EQ_RAD;
			xmax = ymax = -xmin;
			GMT_outside = (PFI) GMT_eqdist_outside;
			GMT_crossing = (PFI) GMT_eqdist_crossing;
			GMT_overlap = (PFI) GMT_radial_overlap;
			GMT_map_clip = (PFI) GMT_radial_clip;
			gmtdefs.basemap_type = GMT_IS_PLAIN;
		}
		search = FALSE;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
	}
				
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);
	project_info.r = 0.5 * project_info.xmax;
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	if (project_info.polar) GMT_meridian_straight = TRUE;
		
	return (search);
}

void GMT_vazeqdist (double lon0, double lat0)
{
	/* Set up azimuthal equidistant projection */
	
	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	project_info.sinp = sin (lat0 * D2R);
	project_info.cosp = cos (lat0 * D2R);
}

void GMT_azeqdist (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to azimuthal equidistant x/y */
	double k, cc, c, clat, slon, clon, slat, t;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;
	lat *= D2R;
	sincos (lat, &slat, &clat);
	sincos (lon, &slon, &clon);
	
	t = clat * clon;
	cc = project_info.sinp * slat + project_info.cosp * t;
	if (fabs (cc) >= 1.0)
		*x = *y = 0.0;
	else {
		c = d_acos (cc);
		k = project_info.EQ_RAD * c / sin (c);
		*x = k * clat * slon;
		*y = k * (project_info.cosp * slat - project_info.sinp * t);
	}
}

void GMT_iazeqdist (double *lon, double *lat, double x, double y)
{
	/* Convert azimuthal equidistant x/yto lon/lat */
	double rho, c, sin_c, cos_c;
	
	rho = hypot (x, y);
	
	if (fabs (rho) < GMT_CONV_LIMIT) {
		*lat = project_info.pole;
		*lon = project_info.central_meridian;
	}
	else {
		c = rho * project_info.i_EQ_RAD;
		sincos (c, &sin_c, &cos_c);
		*lat = d_asin (cos_c * project_info.sinp + (y * sin_c * project_info.cosp / rho)) * R2D;
		if (project_info.n_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, -y);
		else if (project_info.s_polar)
			*lon = project_info.central_meridian + R2D * d_atan2 (x, y);
		else
			*lon = project_info.central_meridian +
				R2D * d_atan2 (x * sin_c, (rho * project_info.cosp * cos_c - y * project_info.sinp * sin_c));
		if ((*lon) <= -180) (*lon) += 360.0;
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MOLLWEIDE EQUAL AREA PROJECTION
 */

int GMT_map_init_mollweide (void) {
	int search;
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
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
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_ellipse;
		GMT_right_edge = (PFD) GMT_right_ellipse;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
		search = FALSE;
	}
	else {
		GMT_mollweide (project_info.w, project_info.s, &xmin, &ymin);
		GMT_mollweide (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_mollweide;		GMT_inverse = (PFI)GMT_imollweide;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;
	
	return (search);
}

void GMT_vmollweide (double lon0, double scale)
{
	/* Set up Mollweide Equal-Area projection */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.w_x = project_info.EQ_RAD * D2R * d_sqrt (8.0) / M_PI;
	project_info.w_y = project_info.EQ_RAD * M_SQRT2;
	project_info.w_iy = 1.0 / project_info.w_y;
	project_info.w_r = 0.25 * (scale * project_info.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_mollweide (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Mollweide Equal-Area x/y */
	int i;
	double phi, delta, psin_lat, c, s;
	
	if (fabs (fabs (lat) - 90.0) < GMT_CONV_LIMIT) {	/* Special case */
		*x = 0.0;
		*y = copysign (project_info.w_y, lat);
		return;
	}
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	lat *= D2R;
	
	phi = lat;
	psin_lat = M_PI * sin (lat);
	i = 0;
	do {
		i++;
		sincos (phi, &s, &c);
		delta = -(phi + s - psin_lat) / (1.0 + c);
		phi += delta;
	}
	while (fabs (delta) > GMT_CONV_LIMIT && i < 100);
	phi *= 0.5;
	sincos (phi, &s, &c);
	*x = project_info.w_x * lon * c;
	*y = project_info.w_y * s;
}

void GMT_imollweide (double *lon, double *lat, double x, double y)
{
	/* Convert Mollweide Equal-Area x/y to lon/lat */
	double phi, phi2;
	
	phi = d_asin (y * project_info.w_iy);
	phi2 = 2.0 * phi;
	*lat = R2D * d_asin ((phi2 + sin (phi2)) / M_PI);
	*lon = project_info.central_meridian + (x / (project_info.w_x * cos (phi)));
	if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE HAMMER-AITOFF EQUAL AREA PROJECTION
 */

int GMT_map_init_hammer (void) {
	int search;
	double xmin, xmax, ymin, ymax, x, y, dummy;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
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
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_ellipse;
		GMT_right_edge = (PFD) GMT_right_ellipse;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
		search = FALSE;
	}
	else {
		GMT_hammer (project_info.w, project_info.s, &xmin, &ymin);
		GMT_hammer (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_hammer;		GMT_inverse = (PFI)GMT_ihammer;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (search);
}

void GMT_vhammer (double lon0, double scale)
{
	/* Set up Hammer-Aitoff Equal-Area projection */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.w_r = 0.25 * (scale * project_info.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_hammer (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Hammer-Aitoff Equal-Area x/y */
	double slat, clat, slon, clon, D;
	
	if (fabs (fabs (lat) - 90.0) < GMT_CONV_LIMIT) {	/* Save time */
		*x = 0.0;
		*y = M_SQRT2 * copysign (project_info.EQ_RAD, lat);
		return;
	}
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	lat *= D2R;
	lon *= (0.5 * D2R);
	sincos (lat, &slat, &clat);
	sincos (lon, &slon, &clon);
	
	D = project_info.EQ_RAD * sqrt (2.0 / (1.0 + clat * clon));
	*x = 2.0 * D * clat * slon;
	*y = D * slat;
}

void GMT_ihammer (double *lon, double *lat, double x, double y)
{
	/* Convert Hammer_Aitoff Equal-Area x/y to lon/lat */
	double rho, c, angle;
	
	x *= 0.5;
	rho = hypot (x, y);
	
	if (fabs (rho) < GMT_CONV_LIMIT) {
		*lat = 0.0;
		*lon = project_info.central_meridian;
	}
	else {
		c = 2.0 * d_asin (0.5 * rho * project_info.i_EQ_RAD);
		*lat = d_asin (y * sin (c) / rho) * R2D;
		if (fabs (c - M_PI_2) < GMT_CONV_LIMIT)
			angle = (fabs (x) < GMT_CONV_LIMIT) ? 0.0 : copysign (180.0, x);
		else
			angle = 2.0 * R2D * atan (x * tan (c) / rho);
		*lon = project_info.central_meridian + angle;
		if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
	}
}

/*
 *	TRANSFORMATION ROUTINES FOR THE VAN DER GRINTEN PROJECTION
 */

int GMT_map_init_grinten (void) {
	int search;
	double xmin, xmax, ymin, ymax, x, y, dummy;

	GMT_set_spherical ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
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
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_circle;
		GMT_right_edge = (PFD) GMT_right_circle;
		frame_info.horizontal = 2;
		project_info.polar = FALSE;
		search = FALSE;
	}
	else {
		GMT_grinten (project_info.w, project_info.s, &xmin, &ymin);
		GMT_grinten (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
 	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]); 
	project_info.r = 0.5 * project_info.xmax;
	GMT_forward = (PFI)GMT_grinten;		GMT_inverse = (PFI)GMT_igrinten;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (search);
}

void GMT_vgrinten (double lon0, double scale)
{
	/* Set up van der Grinten projection */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.v_r = M_PI * project_info.EQ_RAD;
	project_info.v_ir = 1.0 / project_info.v_r;
}

void GMT_grinten (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to van der Grinten x/y */
	double flat, A, A2, G, P, P2, Q, P2A2, i_P2A2, GP2, c, s, theta;
	
	flat = fabs (lat);
	if (flat > (90.0 - GMT_CONV_LIMIT)) {	/* Save time */
		*x = 0.0;
		*y = M_PI * copysign (project_info.EQ_RAD, lat);
		return;
	}
	if (fabs (lon - project_info.central_meridian) < GMT_CONV_LIMIT) {	/* Save time */
		theta = d_asin (2.0 * fabs (lat) / 180.0);
		*x = 0.0;
		*y = M_PI * copysign (project_info.EQ_RAD, lat) * tan (0.5 * theta);
		return;
	}

	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;

	if (flat < GMT_CONV_LIMIT) {	/* Save time */
		*x = project_info.EQ_RAD * D2R * lon;
		*y = 0.0;
		return;
	}

	theta = d_asin (2.0 * fabs (lat) / 180.0);
	
	A = 0.5 * fabs (180.0 / lon - lon / 180.0);
	A2 = A * A;
	sincos (theta, &s, &c);
	G = c / (s + c - 1.0);
	P = G * (2.0 / s - 1.0);
	Q = A2 + G;
	P2 = P * P;
	P2A2 = P2 + A2;
	GP2 = G - P2;
	i_P2A2 = 1.0 / P2A2;

	*x = copysign (project_info.v_r, lon) * (A * GP2 + sqrt (A2 * GP2 * GP2 - P2A2 * (G*G - P2))) * i_P2A2;
	*y = copysign (project_info.v_r, lat) * (P * Q - A * sqrt ((A2 + 1.0) * P2A2 - Q * Q)) * i_P2A2;
}

void GMT_igrinten (double *lon, double *lat, double x, double y)
{
	/* Convert van der Grinten x/y to lon/lat */
	double x2, c1, x2y2, y2, c2, c3, d, a1, m1, theta1, p;

	x *= project_info.v_ir;
	y *= project_info.v_ir;
	x2 = x * x;	y2 = y * y;
	x2y2 = x2 + y2;
	c1 = -fabs(y) * (1.0 + x2y2);
	c2 = c1 - 2 * y2 + x2;
	p = x2y2 * x2y2;
	c3 = -2.0 * c1 + 1.0 + 2.0 * y2 + p;
	d = y2 / c3 + (2 * pow (c2 / c3, 3.0) - 9.0 * c1 * c2 / (c3 * c3)) / 27.0;
	a1 = (c1 - c2 * c2 / (3.0 * c3)) / c3;
	m1 = 2 * sqrt (-a1 / 3.0);
	theta1 = d_acos (3.0 * d / (a1 * m1)) / 3.0;

	*lat = copysign (180.0, y) * (-m1 * cos (theta1 + M_PI/3.0) - c2 / (3.0 * c3));
	*lon = project_info.central_meridian;
	if (x != 0.0) *lon += 90.0 * (x2y2 - 1.0 + sqrt (1.0 + 2 * (x2 - y2) + p)) / x; 
}

/*
 *	TRANSFORMATION ROUTINES FOR THE WINKEL-TRIPEL MODIFIED AZIMUTHAL PROJECTION
 */

int GMT_map_init_winkel (void) {
	int search;
	double xmin, xmax, ymin, ymax, x, y, dummy;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
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
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_winkel;
		GMT_right_edge = (PFD) GMT_right_winkel;
		frame_info.horizontal = 2;
		search = FALSE;
	}
	else {
		GMT_winkel (project_info.w, project_info.s, &xmin, &ymin);
		GMT_winkel (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_winkel;		GMT_inverse = (PFI)GMT_iwinkel;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	return (search);
}

void GMT_vwinkel (double lon0, double scale)
{
	/* Set up Winkel Tripel projection */
	
	GMT_check_R_J (&lon0);
	project_info.r_cosphi1 = cosd (50.467);
	project_info.central_meridian = lon0;
	project_info.w_r = 0.25 * (scale * project_info.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_winkel (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Winkel Tripel x/y */
	double C, D, x1, x2, y1, c, s;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lat *= D2R;
	lon *= (0.5 * D2R);
	
	/* Fist find Aitoff x/y */
	
	sincos (lat, &s, &c);
	D = d_acos (c * cos (lon));
	if (fabs (D) < GMT_CONV_LIMIT)
		x1 = y1 = 0.0;
	else {
		C = s / sin (D);
		x1 = copysign (D * d_sqrt (1.0 - C * C), lon);
		y1 = D * C;
	}
	
	/* Then get equirectangular projection */
	
	x2 = lon * project_info.r_cosphi1;
	/* y2 = lat; use directly in averaging below */
	
	/* Winkler is the average value */
	
	*x = project_info.EQ_RAD * (x1 + x2);
	*y = 0.5 * project_info.EQ_RAD * (y1 + lat);
}

void GMT_iwinkel (double *lon, double *lat, double x, double y)
{
	/* Convert Winkel Tripel x/y to lon/lat */
	/* Only works if point is on perimeter */
	
	int n_iter = 0;
	double c, phi, phi0, delta, sp, cp;
	
	c = 2.0 * y * project_info.i_EQ_RAD;
	phi = y * project_info.i_EQ_RAD;
	do {
		phi0 = phi;
		sincos (phi0, &sp, &cp);
		phi = phi0 - (phi0 + M_PI_2 * sp - c) / (1.0 + M_PI_2 * cp);
		delta = fabs (phi - phi0);
		n_iter++;
	}
	while (delta > GMT_CONV_LIMIT && n_iter < 100);
	*lat = phi * R2D;
	*lon = project_info.central_meridian + copysign (180.0, x - GMT_half_map_size);
}
/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT IV PROJECTION
 */

int GMT_map_init_eckert4 (void) {
	int search;
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_veckert4 (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	
	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_eckert4 (project_info.w, y, &xmin, &dummy);
		GMT_eckert4 (project_info.e, y, &xmax, &dummy);
		GMT_eckert4 (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_eckert4 (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_eckert4;
		GMT_right_edge = (PFD) GMT_right_eckert4;
		frame_info.horizontal = 2;
		search = FALSE;
	}
	else {
		GMT_eckert4 (project_info.w, project_info.s, &xmin, &ymin);
		GMT_eckert4 (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_eckert4;		GMT_inverse = (PFI)GMT_ieckert4;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;
	
	return (search);
}

void GMT_veckert4 (double lon0)
{
	/* Set up Eckert IV projection */
	
	GMT_check_R_J (&lon0);
	project_info.k4_x = 2.0 * project_info.EQ_RAD / d_sqrt (M_PI * (4.0 + M_PI));
	project_info.k4_y = 2.0 * project_info.EQ_RAD * d_sqrt (M_PI / (4.0 + M_PI));
	project_info.k4_ix = 1.0 / project_info.k4_x;
	project_info.k4_iy = 1.0 / project_info.k4_y;
	project_info.central_meridian = lon0;
}

void GMT_eckert4 (double lon, double lat, double *x, double *y)
{
	int n_iter = 0;
	double phi, delta, s_lat, s, c;
	/* Convert lon/lat to Eckert IV x/y */
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lat *= D2R;
	lon *= D2R;
	phi = 0.5 * lat;
	s_lat = sin (lat);
	do {
		sincos (phi, &s, &c);
		delta = -(phi + s * c + 2.0 * s - (2.0 + M_PI_2) * s_lat) / (2.0 * c * (1.0 + c));
		phi += delta;
	}
	while (fabs(delta) > GMT_CONV_LIMIT && n_iter < 100);
	
	sincos (phi, &s, &c);
	*x = project_info.k4_x * lon * (1.0 + c);
	*y = project_info.k4_y * s;
}

void GMT_ieckert4 (double *lon, double *lat, double x, double y)
{
	/* Convert Eckert IV x/y to lon/lat */
	
	double phi, s, c;
	
	s = y * project_info.k4_iy;
	phi = d_asin (s);
	c = cos (phi);
	*lat = d_asin ((phi + s * c + 2.0 * s) / (2.0 + M_PI_2)) * R2D;
	*lon = project_info.central_meridian + R2D * x * project_info.k4_ix / (1.0 + c);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT VI PROJECTION
 */

int GMT_map_init_eckert6 (void) {
	int search;
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_veckert6 (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = 0.5 * project_info.pars[1] * sqrt (2.0 + M_PI);
	
	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_eckert6 (project_info.w, y, &xmin, &dummy);
		GMT_eckert6 (project_info.e, y, &xmax, &dummy);
		GMT_eckert6 (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_eckert6 (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_eckert6;
		GMT_right_edge = (PFD) GMT_right_eckert6;
		frame_info.horizontal = 2;
		search = FALSE;
	}
	else {
		GMT_eckert6 (project_info.w, project_info.s, &xmin, &ymin);
		GMT_eckert6 (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_eckert6;		GMT_inverse = (PFI)GMT_ieckert6;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;
	
	return (search);
}

void GMT_veckert6 (double lon0)
{
	/* Set up Eckert VI projection */
	
	GMT_check_R_J (&lon0);
	project_info.k6_r = project_info.EQ_RAD / sqrt (2.0 + M_PI);
	project_info.k6_ir = 1.0 / project_info.k6_r;
	project_info.central_meridian = lon0;
}

void GMT_eckert6 (double lon, double lat, double *x, double *y)
{
	int n_iter = 0;
	double phi, delta, s_lat, s, c;
	/* Convert lon/lat to Eckert VI x/y */
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	lat *= D2R;
	lon *= D2R;
	phi = lat;
	s_lat = sin (lat);
	do {
		sincos (phi, &s, &c);
		delta = -(phi + s - (1.0 + M_PI_2) * s_lat) / (1.0 + c);
		phi += delta;
	}
	while (fabs(delta) > GMT_CONV_LIMIT && n_iter < 100);
	
	*x = project_info.k6_r * lon * (1.0 + cos (phi));
	*y = 2.0 * project_info.k6_r * phi;
}

void GMT_ieckert6 (double *lon, double *lat, double x, double y)
{
	/* Convert Eckert VI x/y to lon/lat */
	
	double phi, c, s;
	
	phi = 0.5 * y * project_info.k6_ir;
	sincos (phi, &s, &c);
	*lat = d_asin ((phi + s) / (1.0 + M_PI_2)) * R2D;
	*lon = project_info.central_meridian + R2D * x * project_info.k6_ir / (1.0 + c);
	if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}
/*
 *	TRANSFORMATION ROUTINES FOR THE ROBINSON PSEUDOCYLINDRICAL PROJECTION
 */

int GMT_map_init_robinson (void) {
	int search;
	double xmin, xmax, ymin, ymax, y, dummy;

	GMT_set_spherical ();	/* PW: Force spherical for now */

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	GMT_vrobinson (project_info.pars[0]);
	project_info.x_scale = project_info.y_scale = project_info.pars[1] / 0.8487;
	
	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_robinson (project_info.w, y, &xmin, &dummy);
		GMT_robinson (project_info.e, y, &xmax, &dummy);
		GMT_robinson (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_robinson (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_robinson;
		GMT_right_edge = (PFD) GMT_right_robinson;
		frame_info.horizontal = 2;
		search = FALSE;
	}
	else {
		GMT_robinson (project_info.w, project_info.s, &xmin, &ymin);
		GMT_robinson (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_forward = (PFI)GMT_robinson;		GMT_inverse = (PFI)GMT_irobinson;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	GMT_parallel_straight = TRUE;
	
	return (search);
}

void GMT_vrobinson (double lon0)
{
	int err_flag;
	
	/* Set up Robinson projection */

	if (gmtdefs.interpolant == 0) {	/* Must reset and warn */
		fprintf (stderr, "GMT Warning : -JN requires Akima or Cubic spline interpolant, set to Akima\n");
		gmtdefs.interpolant = 1;
	}
	
	GMT_check_R_J (&lon0);
	project_info.n_cx = 0.8487 * project_info.EQ_RAD * D2R;
	project_info.n_cy = 1.3523 * project_info.EQ_RAD;
	project_info.n_i_cy = 1.0 / project_info.n_cy;
	project_info.central_meridian = lon0;
	
	project_info.n_phi[0] = 0;	project_info.n_X[0] = 1.0000;	project_info.n_Y[0] = 0.0000;
	project_info.n_phi[1] = 5;	project_info.n_X[1] = 0.9986;	project_info.n_Y[1] = 0.0620;
	project_info.n_phi[2] = 10;	project_info.n_X[2] = 0.9954;	project_info.n_Y[2] = 0.1240;
	project_info.n_phi[3] = 15;	project_info.n_X[3] = 0.9900;	project_info.n_Y[3] = 0.1860;
	project_info.n_phi[4] = 20;	project_info.n_X[4] = 0.9822;	project_info.n_Y[4] = 0.2480;
	project_info.n_phi[5] = 25;	project_info.n_X[5] = 0.9730;	project_info.n_Y[5] = 0.3100;
	project_info.n_phi[6] = 30;	project_info.n_X[6] = 0.9600;	project_info.n_Y[6] = 0.3720;
	project_info.n_phi[7] = 35;	project_info.n_X[7] = 0.9427;	project_info.n_Y[7] = 0.4340;
	project_info.n_phi[8] = 40;	project_info.n_X[8] = 0.9216;	project_info.n_Y[8] = 0.4958;
	project_info.n_phi[9] = 45;	project_info.n_X[9] = 0.8962;	project_info.n_Y[9] = 0.5571;
	project_info.n_phi[10] = 50;	project_info.n_X[10] = 0.8679;	project_info.n_Y[10] = 0.6176;
	project_info.n_phi[11] = 55;	project_info.n_X[11] = 0.8350;	project_info.n_Y[11] = 0.6769;
	project_info.n_phi[12] = 60;	project_info.n_X[12] = 0.7986;	project_info.n_Y[12] = 0.7346;
	project_info.n_phi[13] = 65;	project_info.n_X[13] = 0.7597;	project_info.n_Y[13] = 0.7903;
	project_info.n_phi[14] = 70;	project_info.n_X[14] = 0.7186;	project_info.n_Y[14] = 0.8435;
	project_info.n_phi[15] = 75;	project_info.n_X[15] = 0.6732;	project_info.n_Y[15] = 0.8936;
	project_info.n_phi[16] = 80;	project_info.n_X[16] = 0.6213;	project_info.n_Y[16] = 0.9394;
	project_info.n_phi[17] = 85;	project_info.n_X[17] = 0.5722;	project_info.n_Y[17] = 0.9761;
	project_info.n_phi[18] = 90;	project_info.n_X[18] = 0.5322;	project_info.n_Y[18] = 1.0000;
	project_info.n_x_coeff  = (double *) GMT_memory (VNULL, (size_t)(3 * N_ROBINSON), sizeof (double), GMT_program);
	project_info.n_y_coeff  = (double *) GMT_memory (VNULL, (size_t)(3 * N_ROBINSON), sizeof (double), GMT_program);
	project_info.n_iy_coeff = (double *) GMT_memory (VNULL, (size_t)(3 * N_ROBINSON), sizeof (double), GMT_program);
	if (gmtdefs.interpolant == 2) {	/* Natural cubic spline */
		err_flag = GMT_cspline (project_info.n_phi, project_info.n_X, N_ROBINSON, project_info.n_x_coeff);
		err_flag = GMT_cspline (project_info.n_phi, project_info.n_Y, N_ROBINSON, project_info.n_y_coeff);
		err_flag = GMT_cspline (project_info.n_Y, project_info.n_phi, N_ROBINSON, project_info.n_iy_coeff);
	}
	else { 	/* Akimas spline */
		err_flag = GMT_akima (project_info.n_phi, project_info.n_X, N_ROBINSON, project_info.n_x_coeff);
		err_flag = GMT_akima (project_info.n_phi, project_info.n_Y, N_ROBINSON, project_info.n_y_coeff);
		err_flag = GMT_akima (project_info.n_Y, project_info.n_phi, N_ROBINSON, project_info.n_iy_coeff);
	}
	if (err_flag != 0) {
		fprintf (stderr, "GMT ERROR: -JN initialization of spline failed - report to GMT gurus\n");
		exit (EXIT_FAILURE);
	}
	
}

void GMT_robinson (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Robinson x/y */
	double tmp, X, Y;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	tmp = fabs (lat);
	
	X = GMT_robinson_spline (tmp, project_info.n_phi, project_info.n_X, project_info.n_x_coeff);
	Y = GMT_robinson_spline (tmp, project_info.n_phi, project_info.n_Y, project_info.n_y_coeff);
	*x = project_info.n_cx * X * lon;	/* D2R is in n_cx already */
	*y = project_info.n_cy * copysign (Y, lat);
}

void GMT_irobinson (double *lon, double *lat, double x, double y)
{
	/* Convert Robinson x/y to lon/lat */
	double X, Y;
	
	Y = fabs (y * project_info.n_i_cy);
	*lat = GMT_robinson_spline (Y, project_info.n_Y, project_info.n_phi, project_info.n_iy_coeff);
	X = GMT_robinson_spline (*lat, project_info.n_phi, project_info.n_X, project_info.n_x_coeff);
	
	if (y < 0.0) *lat = -(*lat);
	*lon = x / (project_info.n_cx * X) + project_info.central_meridian;
}

double GMT_robinson_spline (double xp, double *x, double *y, double *c) {
	/* Compute the interpolated values from the Robinson coefficients */

	int j = 0, j1;
	double yp, a, b, h, ih, dx;
	
	if (xp < x[0] || xp > x[N_ROBINSON-1])	/* Desired point outside data range */
		return (GMT_d_NaN);

	while (j < N_ROBINSON && x[j] <= xp) j++;
	if (j == N_ROBINSON) j--;
	if (j > 0) j--;

	dx = xp - x[j];
	switch (gmtdefs.interpolant) {
		case 1:
			yp = ((c[3*j+2]*dx + c[3*j+1])*dx + c[3*j])*dx + y[j];
			break;
		case 2:
			j1 = j + 1;
			h = x[j1] - x[j];
			ih = 1.0 / h;
			a = (x[j1] - xp) * ih;
			b = dx * ih;
			yp = a * y[j] + b * y[j1] + ((a*a*a - a) * c[j] + (b*b*b - b) * c[j1]) * (h*h) / 6.0;
			break;
	}
	return (yp);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE SINUSOIDAL EQUAL AREA PROJECTION
 */

int GMT_map_init_sinusoidal (void) {
	int search;
	double xmin, xmax, ymin, ymax, dummy, y;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();

	if (project_info.pars[0] < 0.0) project_info.pars[0] += 360.0;
	GMT_world_map = (fabs (fabs (project_info.e - project_info.w) - 360.0) < SMALL);
	if (project_info.s <= -90.0) project_info.edge[0] = FALSE;
	if (project_info.n >= 90.0) project_info.edge[2] = FALSE;
	GMT_vsinusoidal (project_info.pars[0]);
	if (project_info.units_pr_degree) project_info.pars[1] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[1];
	GMT_forward = (PFI)GMT_sinusoidal;		GMT_inverse = (PFI)GMT_isinusoidal;
	gmtdefs.basemap_type = GMT_IS_PLAIN;
	
	if (project_info.region) {
		y = (project_info.s * project_info.n <= 0.0) ? 0.0 : MIN (fabs(project_info.s), fabs(project_info.n));
		GMT_sinusoidal (project_info.central_meridian, project_info.s, &dummy, &ymin);
		GMT_sinusoidal (project_info.central_meridian, project_info.n, &dummy, &ymax);
		GMT_sinusoidal (project_info.w, y, &xmin, &dummy);
		GMT_sinusoidal (project_info.e, y, &xmax, &dummy);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_sinusoidal;
		GMT_right_edge = (PFD) GMT_right_sinusoidal;
		frame_info.horizontal = 2;
		project_info.polar = TRUE;
		search = FALSE;
	}
	else {
		GMT_sinusoidal (project_info.w, project_info.s, &xmin, &ymin);
		GMT_sinusoidal (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}

	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[1]);
	GMT_parallel_straight = TRUE;

	return (search);
}

void GMT_vsinusoidal (double lon0)
{
	/* Set up Sinusoidal projection */
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
}

void GMT_sinusoidal (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Sinusoidal Equal-Area x/y */
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	
	lon *= D2R;
	lat *= D2R;
	
	*x = project_info.EQ_RAD * lon * cos (lat);
	*y = project_info.EQ_RAD * lat;
}

void GMT_isinusoidal (double *lon, double *lat, double x, double y)
{
	/* Convert Sinusoidal Equal-Area x/y to lon/lat */

	*lat = y * project_info.i_EQ_RAD;
	*lon = ((fabs (fabs (*lat) - M_PI) < GMT_CONV_LIMIT) ? 0.0 : R2D * x / (project_info.EQ_RAD * cos (*lat))) + project_info.central_meridian;
	*lat *= R2D;
	if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE CASSINI PROJECTION
 */

int GMT_map_init_cassini (void) {
	BOOLEAN search, too_big;
	double xmin, xmax, ymin, ymax;

	too_big = GMT_quicktm (project_info.pars[0], 4.0);
	if (too_big) GMT_set_spherical();	/* Cannot use ellipsoidal series for this area */
	GMT_vcassini (project_info.pars[0], project_info.pars[1]);
	if (SPHERICAL) {
		GMT_forward = (PFI)GMT_cassini_sph;
		GMT_inverse = (PFI)GMT_icassini_sph;
	}
	else {
		GMT_forward = (PFI)GMT_cassini;
		GMT_inverse = (PFI)GMT_icassini;
	}
	if (project_info.units_pr_degree) project_info.pars[2] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[2];
	gmtdefs.basemap_type = GMT_IS_PLAIN;

	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
		search = FALSE;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
				
	frame_info.horizontal = TRUE;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[2]);

	return (search);
}

void GMT_vcassini (double lon0, double lat0)
{
	/* Set up Cassini projection */

	double e1, lat2, s2, c2;

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	lat0 *= D2R;
	project_info.c_p = lat0;
	lat2 = 2.0 * lat0;
	sincos (lat2, &s2, &c2);

	e1 = (1.0 - d_sqrt (project_info.one_m_ECC2)) / (1.0 + d_sqrt (project_info.one_m_ECC2));

	project_info.c_c1 = 1.0 - (1.0/4.0) * project_info.ECC2 - (3.0/64.0) * project_info.ECC4 - (5.0/256.0) * project_info.ECC6;
	project_info.c_c2 = -((3.0/8.0) * project_info.ECC2 + (3.0/32.0) * project_info.ECC4 + (25.0/768.0) * project_info.ECC6);
	project_info.c_c3 = (15.0/128.0) * project_info.ECC4 + (45.0/512.0) * project_info.ECC6;
	project_info.c_c4 = -(35.0/768.0) * project_info.ECC6;
	project_info.c_M0 = project_info.EQ_RAD * (project_info.c_c1 * lat0 + s2 * (project_info.c_c2 + c2 * (project_info.c_c3 + c2 * project_info.c_c4)));
	project_info.c_i1 = 1.0 / (project_info.EQ_RAD * project_info.c_c1);
	project_info.c_i2 = (3.0/2.0) * e1 - (29.0/12.0) * pow (e1, 3.0);
	project_info.c_i3 = (21.0/8.0) * e1 * e1 - (1537.0/128.0) * pow (e1, 4.0);
	project_info.c_i4 = (151.0/24.0) * pow (e1, 3.0);
	project_info.c_i5 = (1097.0/64.0) * pow (e1, 4.0);
}

void GMT_cassini (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cassini x/y */
	
	double lat2, tany, N, T, A, C, M, s, c, s2, c2, A2, A3;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;

	if (fabs (lat) < GMT_CONV_LIMIT) {	/* Quick when lat is zero */
		*x = project_info.EQ_RAD * lon;
		*y = -project_info.c_M0;
		return;
	}

	lat *= D2R;
	lat2 = 2.0 * lat;
	sincos (lat, &s, &c);
	sincos (lat2, &s2, &c2);
	tany = s / c;
	N = project_info.EQ_RAD / sqrt (1.0 - project_info.ECC2 * s * s);
	T = tany * tany;
	A = lon * c;
	A2 = A * A;
	A3 = A2 * A;
	C = project_info.ECC2 * c * c * project_info.i_one_m_ECC2;
	M = project_info.EQ_RAD * (project_info.c_c1 * lat + s2 * (project_info.c_c2 + c2 * (project_info.c_c3 + c2 * project_info.c_c4)));
	*x = N * (A - T * A3 / 6.0 - (8.0 - T + 8 * C) * T * A3 * A2 / 120.0);
	*y = M - project_info.c_M0 + N * tany * (0.5 * A2 + (5.0 - T + 6.0 * C) * A2 * A2 / 24.0);
}

void GMT_icassini (double *lon, double *lat, double x, double y)
{
	/* Convert Cassini x/y to lon/lat */
	
	double M1, u1, u2, s, c, phi1, tany, T1, N1, R_1, D, S2, D2, D3;
	
	M1 = project_info.c_M0 + y;
	u1 = M1 * project_info.c_i1;
	u2 = 2.0 * u1;
	sincos (u2, &s, &c);
	phi1 = u1 + s * (project_info.c_i2 + c * (project_info.c_i3 + c * (project_info.c_i4 + c * project_info.c_i5)));
	if (fabs (fabs (phi1) - M_PI_2) < GMT_CONV_LIMIT) {
		*lat = copysign (M_PI_2, phi1);
		*lon = project_info.central_meridian;
	}
	else {
		sincos (phi1, &s, &c);
		tany = s / c;
		T1 = tany * tany;
		S2 = 1.0 - project_info.ECC2 * s * s;
		N1 = project_info.EQ_RAD / sqrt (S2);
		R_1 = project_info.EQ_RAD * project_info.one_m_ECC2 / pow (S2, 1.5);
		D = x / N1;
		D2 = D * D;
		D3 = D2 * D;
		*lat = R2D * (phi1 - (N1 * tany / R_1) * (0.5 * D2 - (1.0 + 3.0 * T1) * D2 * D2 / 24.0));
		*lon = project_info.central_meridian + R2D * (D - T1 * D3 / 3.0 + (1.0 + 3.0 * T1) * T1 * D3 * D2 / 15.0) / c;
	}
}

/* Cassini functions for the Sphere */

void GMT_cassini_sph (double lon, double lat, double *x, double *y)
{
	double slon, clon, clat, tlat, slat;

	/* Convert lon/lat to Cassini x/y */
		
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;
	lat *= D2R;
	sincos (lon, &slon, &clon);
	sincos (lat, &slat, &clat);
	tlat = slat / clat;
	*x = project_info.EQ_RAD * d_asin (clat * slon);
	*y = project_info.EQ_RAD * (atan (tlat / clon) - project_info.c_p);
}

void GMT_icassini_sph (double *lon, double *lat, double x, double y)
{
	/* Convert Cassini x/y to lon/lat */
	
	double D, sD, cD, cx, tx, sx;

	x *= project_info.i_EQ_RAD;
	D = y * project_info.i_EQ_RAD + project_info.c_p;
	sincos (D, &sD, &cD);
	sincos (x, &sx, &cx);
	tx = sx / cx;
	*lat = R2D * d_asin (sD * cx);
	*lon = project_info.central_meridian + R2D * atan (tx / cD);
}
/*
 *	TRANSFORMATION ROUTINES FOR THE ALBERS PROJECTION
 */

int GMT_map_init_albers (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	GMT_convert_latitudes = GMT_quickconic();
	if (GMT_convert_latitudes) GMT_scale_eqrad ();
	if (SPHERICAL || GMT_convert_latitudes) {	/* Spherical code w/wo authalic latitudes */
		GMT_valbers_sph (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
		GMT_forward = (PFI)GMT_albers_sph;
		GMT_inverse = (PFI)GMT_ialbers_sph;
	}
	else {
		GMT_valbers (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
		GMT_forward = (PFI)GMT_albers;
		GMT_inverse = (PFI)GMT_ialbers;
	}
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];
			
	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
		search = FALSE;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	frame_info.horizontal = TRUE;
	gmtdefs.n_lat_nodes = 2;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);

	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	GMT_geo_to_xy (project_info.w, project_info.pole, &x1, &y1);
	dy = y1 - project_info.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - project_info.c_x0);
	dy /= (1.0 - cos (az));
	project_info.c_y0 += dy;
	GMT_meridian_straight = TRUE;

	return (search);
}


void GMT_valbers (double lon0, double lat0, double ph1, double ph2)
{
	/* Set up Albers projection */
	
	double s0, s1, s2, q0, q1, q2, m1, m2;
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.north_pole = (lat0 > 0.0);
	project_info.pole = (project_info.north_pole) ? 90.0 : -90.0;
	lat0 *= D2R;
	ph1 *= D2R;	ph2 *= D2R;
	
	s0 = sin (lat0);	s1 = sin (ph1);		s2 = sin (ph2);
	m1 = cos (ph1) * cos (ph1) / (1.0 - project_info.ECC2 * s1 * s1);	/* Actually m1 and m2 squared */
	m2 = cos (ph2) * cos (ph2) / (1.0 - project_info.ECC2 * s2 * s2);
	q0 = (fabs (project_info.ECC) < GMT_CONV_LIMIT) ? 2.0 * s0 : project_info.one_m_ECC2 * (s0 / (1.0 - project_info.ECC2 * s0 * s0) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s0) / (1.0 + project_info.ECC * s0)));
	q1 = (fabs (project_info.ECC) < GMT_CONV_LIMIT) ? 2.0 * s1 : project_info.one_m_ECC2 * (s1 / (1.0 - project_info.ECC2 * s1 * s1) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s1) / (1.0 + project_info.ECC * s1)));
	q2 = (fabs (project_info.ECC) < GMT_CONV_LIMIT) ? 2.0 * s2 : project_info.one_m_ECC2 * (s2 / (1.0 - project_info.ECC2 * s2 * s2) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s2) / (1.0 + project_info.ECC * s2)));
	
	project_info.a_n = (fabs (ph1 - ph2) < GMT_CONV_LIMIT) ? s1 : (m1 - m2) / (q2 - q1);
	project_info.a_i_n = 1.0 / project_info.a_n;
	project_info.a_C = m1 + project_info.a_n * q1;
	project_info.a_rho0 = project_info.EQ_RAD * sqrt (project_info.a_C - project_info.a_n * q0) * project_info.a_i_n;
	project_info.a_n2ir2 = (project_info.a_n * project_info.a_n) / (project_info.EQ_RAD * project_info.EQ_RAD);
	project_info.a_test = 1.0 - (project_info.i_half_ECC * project_info.one_m_ECC2) * log ((1.0 - project_info.ECC) / (1.0 + project_info.ECC));

}

void GMT_valbers_sph (double lon0, double lat0, double ph1, double ph2)
{
	/* Set up Spherical Albers projection (Snyder, page 100) */
	
	double s1, c1;
	
	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.north_pole = (lat0 > 0.0);
	project_info.pole = (project_info.north_pole) ? 90.0 : -90.0;

	ph1 *= D2R;
	
	s1 = sin (ph1);		c1 = cos (ph1);
	
	project_info.a_n = (fabs (ph1 - ph2) < GMT_CONV_LIMIT) ? s1 : 0.5 * (s1 + sind (ph2));
	project_info.a_i_n = 1.0 / project_info.a_n;
	project_info.a_C = c1 * c1 + 2.0 * project_info.a_n * s1;
	project_info.a_rho0 = project_info.EQ_RAD * sqrt (project_info.a_C - 2.0 * project_info.a_n * sind (lat0)) * project_info.a_i_n;
	project_info.a_n2ir2 = 0.5 * project_info.a_n / (project_info.EQ_RAD * project_info.EQ_RAD);
	project_info.a_Cin = 0.5 * project_info.a_C / project_info.a_n;

}

void GMT_albers (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Albers x/y */
	
	double s, c, q, theta, rho, r;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lon *= D2R;
	lat *= D2R;
	
	s = sin (lat);
	if (fabs (project_info.ECC) < GMT_CONV_LIMIT)
		q = 2.0 * s;
	else {
		r = project_info.ECC * s;
		q = project_info.one_m_ECC2 * (s / (1.0 - project_info.ECC2 * s * s) - project_info.i_half_ECC * log ((1.0 - r) / (1.0 + r)));
	}
	theta = project_info.a_n * lon;
	rho = project_info.EQ_RAD * sqrt (project_info.a_C - project_info.a_n * q) * project_info.a_i_n;
	
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.a_rho0 - rho * c;
}

void GMT_ialbers (double *lon, double *lat, double x, double y)
{
	/* Convert Albers x/y to lon/lat */
	
	int n_iter;
	double theta, rho, q, phi, phi0, s, c, s2, ex_1, delta, r;
	
	theta = (project_info.a_n < 0.0) ? d_atan2 (-x, y - project_info.a_rho0) : d_atan2 (x, project_info.a_rho0 - y);
	rho = hypot (x, project_info.a_rho0 - y);
	q = (project_info.a_C - rho * rho * project_info.a_n2ir2) * project_info.a_i_n;
	
	if (fabs (fabs (q) - project_info.a_test) < GMT_CONV_LIMIT)
		*lat = copysign (90.0, q);
	else {
		phi = d_asin (0.5 * q);
		n_iter = 0;
		do {
			phi0 = phi;
			sincos (phi0, &s, &c);
			r = project_info.ECC * s;
			s2 = s * s;
			ex_1 = 1.0 - project_info.ECC2 * s2;
			phi = phi0 + 0.5 * ex_1 * ex_1 * ((q * project_info.i_one_m_ECC2) - s / ex_1
				+ project_info.i_half_ECC * log ((1 - r) / (1.0 + r))) / c;
			delta = fabs (phi - phi0);
			n_iter++;
		}
		while (delta > GMT_CONV_LIMIT && n_iter < 100);
		*lat = R2D * phi;
	}
	*lon = project_info.central_meridian + R2D * theta * project_info.a_i_n;
}

/* Spherical versions of Albers */

void GMT_albers_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Spherical Albers x/y */
	
	double s, c, theta, rho;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	if (GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	lon *= D2R;
	lat *= D2R;
	
	theta = project_info.a_n * lon;
	rho = project_info.EQ_RAD * sqrt (project_info.a_C - 2.0 * project_info.a_n * sin (lat)) * project_info.a_i_n;
	
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.a_rho0 - rho * c;
}

void GMT_ialbers_sph (double *lon, double *lat, double x, double y)
{
	/* Convert Spherical Albers x/y to lon/lat */
	
	double theta, A, dy;
	
	theta = (project_info.a_n < 0.0) ? d_atan2 (-x, y - project_info.a_rho0) : d_atan2 (x, project_info.a_rho0 - y);
	dy = project_info.a_rho0 - y;
	A = (x * x + dy * dy) * project_info.a_n2ir2;

	*lat = R2D * d_asin (project_info.a_Cin - A);
	*lon = project_info.central_meridian + R2D * theta * project_info.a_i_n;
	if (GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE EQUIDISTANT CONIC PROJECTION
 */


int GMT_map_init_econic (void) {
	BOOLEAN search;
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	GMT_convert_latitudes = !SPHERICAL;
	if (GMT_convert_latitudes) GMT_scale_eqrad ();
	GMT_veconic (project_info.pars[0], project_info.pars[1], project_info.pars[2], project_info.pars[3]);
	GMT_forward = (PFI)GMT_econic;
	GMT_inverse = (PFI)GMT_ieconic;
	if (project_info.units_pr_degree) project_info.pars[4] /= project_info.M_PR_DEG;
	project_info.x_scale = project_info.y_scale = project_info.pars[4];
			
	if (project_info.region) {
		GMT_xy_search (&xmin, &xmax, &ymin, &ymax, project_info.w, project_info.e, project_info.s, project_info.n);
		GMT_outside = (PFI) GMT_wesn_outside;
		GMT_crossing = (PFI) GMT_wesn_crossing;
		GMT_overlap = (PFI) GMT_wesn_overlap;
		GMT_map_clip = (PFI) GMT_wesn_clip;
		GMT_left_edge = (PFD) GMT_left_conic;
		GMT_right_edge = (PFD) GMT_right_conic;
		search = FALSE;
	}
	else {
		(*GMT_forward) (project_info.w, project_info.s, &xmin, &ymin);
		(*GMT_forward) (project_info.e, project_info.n, &xmax, &ymax);
		GMT_outside = (PFI) GMT_rect_outside;
		GMT_crossing = (PFI) GMT_rect_crossing;
		GMT_overlap = (PFI) GMT_rect_overlap;
		GMT_map_clip = (PFI) GMT_rect_clip;
		GMT_left_edge = (PFD) GMT_left_rect;
		GMT_right_edge = (PFD) GMT_right_rect;
		frame_info.check_side = TRUE;
		search = TRUE;
	}
	frame_info.horizontal = TRUE;
	gmtdefs.n_lat_nodes = 2;
	GMT_map_setinfo (xmin, xmax, ymin, ymax, project_info.pars[4]);

	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &project_info.c_x0, &project_info.c_y0);
	GMT_geo_to_xy (project_info.w, project_info.pole, &x1, &y1);
	dy = y1 - project_info.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - project_info.c_x0);
	dy /= (1.0 - cos (az));
	project_info.c_y0 += dy;
 	GMT_meridian_straight = TRUE;

	return (search);
}

void GMT_veconic (double lon0, double lat0, double lat1, double lat2)
{
	double c1;

	/* Set up Equidistant Conic projection */
	
	GMT_check_R_J (&lon0);
	project_info.north_pole = (lat0 > 0.0);
	c1 = cosd (lat1);
	project_info.d_n = (fabs (lat1 - lat2) < GMT_CONV_LIMIT) ? sind (lat1) : (c1 - cosd (lat2)) / (D2R * (lat2 - lat1));
	project_info.d_i_n = R2D / project_info.d_n;	/* R2D put here instead of in lon for ieconic */
	project_info.d_G = (c1 / project_info.d_n) + lat1 * D2R;
	project_info.d_rho0 = project_info.EQ_RAD * (project_info.d_G - lat0 * D2R);
	project_info.central_meridian = lon0;
}

void GMT_econic (double lon, double lat, double *x, double *y)
{
	double rho, theta, s, c;
	/* Convert lon/lat to Equidistant Conic x/y */
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lat *= D2R;
	lon *= D2R;
	rho = project_info.EQ_RAD * (project_info.d_G - lat);
	theta = project_info.d_n * lon;
	
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.d_rho0 - rho * c;
}

void GMT_ieconic (double *lon, double *lat, double x, double y)
{
	/* Convert Equidistant Conic x/y to lon/lat */
	
	double rho, theta;

	rho = hypot (x, project_info.d_rho0 - y);
	if (project_info.d_n < 0.0) rho = -rho;
	theta = (project_info.d_n < 0.0) ? d_atan2 (-x, y - project_info.d_rho0) : d_atan2 (x, project_info.d_rho0 - y);
	*lat = (project_info.d_G - rho * project_info.i_EQ_RAD) * R2D;
	*lon = project_info.central_meridian + theta * project_info.d_i_n;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1.1 :	S U P P O R T I N G   R O U T I N E S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void GMT_wesn_search (double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north) {
	double dx, dy, w, e, s, n, x, y, lon, lat;
	int i, j;
	
	/* Search for extreme original coordinates lon/lat */
			
	dx = (xmax - xmin) / gmtdefs.n_lon_nodes;
	dy = (ymax - ymin) / gmtdefs.n_lat_nodes;
	w = s = DBL_MAX;	e = n = -DBL_MAX;
	for (i = 0; i <= gmtdefs.n_lon_nodes; i++) {
		x = (i == gmtdefs.n_lon_nodes) ? xmax : xmin + i * dx;
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
	for (j = 0; j <= gmtdefs.n_lat_nodes; j++) {
		y = (j == gmtdefs.n_lat_nodes) ? ymax : ymin + j * dy;
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
	/* if (project_info.projection != AZ_EQDIST && !GMT_map_outside (project_info.central_meridian, -90.0)) { why was it like this? */
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

void GMT_horizon_search (double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax) {
	double dx, dy, d, x, y, lon, lat;
	int i, j;
	BOOLEAN beyond = FALSE;
	
	/* Search for extreme original coordinates lon/lat and see if any fall beyond the horizon */
			
	dx = (xmax - xmin) / gmtdefs.n_lon_nodes;
	dy = (ymax - ymin) / gmtdefs.n_lat_nodes;
	if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, w, s)) > project_info.f_horizon) beyond = TRUE;
	if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, e, n)) > project_info.f_horizon) beyond = TRUE;
	for (i = 0; !beyond && i <= gmtdefs.n_lon_nodes; i++) {
		x = (i == gmtdefs.n_lon_nodes) ? xmax : xmin + i * dx;
		GMT_xy_to_geo (&lon, &lat, x, ymin);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (&lon, &lat, x, ymax);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
	}
	for (j = 0; !beyond && j <= gmtdefs.n_lat_nodes; j++) {
		y = (j == gmtdefs.n_lat_nodes) ? ymax : ymin + j * dy;
		GMT_xy_to_geo (&lon, &lat, xmin, y);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (&lon, &lat, xmax, y);
		if ((d = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon, lat)) > project_info.f_horizon) beyond = TRUE;
	}
	if (beyond) {
		fprintf (stderr, "%s: ERROR: Rectangular region for azimuthal projection extends beyond the horizon\n", GMT_program);
		fprintf (stderr, "%s: ERROR: Please select a region that is completely within the visible hemisphere\n", GMT_program);
		exit (EXIT_FAILURE);
	}
}

void GMT_xy_search (double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0)
{
	int i, j;
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

int GMT_map_crossing (double lon1, double lat1, double lon2, double lat2, double *xlon, double *xlat, double *xx, double *yy, int *sides)
{
	int nx;
	GMT_corner = -1;
	nx = (*GMT_crossing) (lon1, lat1, lon2, lat2, xlon, xlat, xx, yy, sides);
	
	/* nx may be -2, in which case we don't want to check the order */
	if (nx == 2) {	/* Must see if crossings are in correct order */
		double da, db;

		if (MAPPING) {
			da = GMT_great_circle_dist (lon1, lat1, xlon[0], xlat[0]);
			db = GMT_great_circle_dist (lon1, lat1, xlon[1], xlat[1]);
		}
		else {
			da = hypot (lon1 - xlon[0], lat1 - xlat[0]);
			db = hypot (lon1 - xlon[1], lat1 - xlat[1]);
		}
		if (da > db) { /* Must swap */
			d_swap (xlon[0], xlon[1]);
			d_swap (xlat[0], xlat[1]);
			d_swap (xx[0], xx[1]);
			d_swap (yy[0], yy[1]);
			i_swap (sides[0], sides[1]);
		}
	}
	return (abs(nx));
}

int GMT_map_outside (double lon, double lat)
{
	GMT_x_status_old = GMT_x_status_new;
	GMT_y_status_old = GMT_y_status_new;
	return ((*GMT_outside) (lon, lat));
}

int GMT_break_through (double x0, double y0, double x1, double y1)
{
	
	if (GMT_x_status_old == GMT_x_status_new && GMT_y_status_old == GMT_y_status_new) return (FALSE);
	if ((GMT_x_status_old == 0 && GMT_y_status_old == 0) || (GMT_x_status_new == 0 && GMT_y_status_new == 0)) return (TRUE);
	
	/* Less clearcut case, check for overlap */
	
	return ( (*GMT_overlap) (x0, y0, x1, y1));
}

int GMT_rect_overlap (double lon0, double lat0, double lon1, double lat1)
{
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

int GMT_wesn_overlap (double lon0, double lat0, double lon1, double lat1)
{
	if (lon0 > lon1) d_swap (lon0, lon1);
	if (lat0 > lat1) d_swap (lat0, lat1);
	/* if (lon1 < project_info.w) { */
	if ((lon1 - project_info.w) < -GMT_CONV_LIMIT) {
		lon0 += 360.0;
		lon1 += 360.0;
	}
	/* else if (lon0 > project_info.e) { */
	else if ((lon0 - project_info.e) > GMT_CONV_LIMIT) {
		lon0 -= 360.0;
		lon1 -= 360.0;
	}
	/* if (lon1 < project_info.w || lon0 > project_info.e) return (FALSE);
	if (lat1 < project_info.s || lat0 > project_info.n) return (FALSE); */

	if ((lon1 - project_info.w) < -GMT_CONV_LIMIT || (lon0 - project_info.e) > GMT_CONV_LIMIT) return (FALSE);
	if ((lat1 - project_info.s) < -GMT_CONV_LIMIT || (lat0 - project_info.n) > GMT_CONV_LIMIT) return (FALSE);
	return (TRUE);
}

int GMT_radial_overlap (double lon0, double lat0, double lon1, double lat1)
{	/* Dummy routine */
	return (TRUE);
}

int GMT_wrap_around_check_x (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, int *sides, int *nx)
{
	int i, wrap = FALSE, skip;
	double dx, dy, width, jump, GMT_half_map_width (double y);
	
	jump = this_x - last_x;
	/* width = GMT_half_map_width (this_y); */
	width = MAX (GMT_half_map_width (this_y), GMT_half_map_width (last_y));
	
	skip = (fabs (jump) < width || (fabs(jump) <= SMALL || fabs(width) <= SMALL));
	dy = this_y - last_y;

	for (i = 0; i < (*nx); i++) {	/* Must check if the crossover found should wrap around */
		if (skip) continue;
		
		if (jump < (-width)) {	/* Crossed right boundary */
			dx = this_x + GMT_map_width - last_x;
			yy[0] = yy[1] = last_y + (GMT_map_width - last_x) * dy / dx;
			xx[0] = GMT_right_boundary (yy[0]);	xx[1] = GMT_left_boundary (yy[0]);
			sides[0] = 1;	sides[1] = 3;
			angle[0] = R2D * d_atan2 (dy, dx);
		}
		else {	/* Crossed left boundary */
			dx = last_x + GMT_map_width - this_x;
			yy[0] = yy[1] = last_y + last_x * dy / dx;
			xx[0] = GMT_left_boundary (yy[0]);	xx[1] = GMT_right_boundary (yy[0]);
			sides[0] = 3;	sides[1] = 1;
			angle[0] = R2D * d_atan2 (dy, -dx);
		}
		angle[1] = angle[0] + 180.0;
		if (yy[0] >= 0.0 && yy[0] <= project_info.ymax) wrap = TRUE;
	}
	
	if (*nx == 0 && !skip) {	/* Must wrap around */
		if (jump < (-width)) {	/* Crossed right boundary */
			dx = this_x + GMT_map_width - last_x;
			yy[0] = yy[1] = last_y + (GMT_map_width - last_x) * dy / dx;
			xx[0] = GMT_right_boundary (yy[0]);	xx[1] = GMT_left_boundary (yy[0]);
			sides[0] = 1;	sides[1] = 3;
			angle[0] = R2D * d_atan2 (dy, dx);
		}
		else {	/* Crossed left boundary */
			dx = last_x + GMT_map_width - this_x;
			yy[0] = yy[1] = last_y + last_x * dy / dx;
			xx[0] = GMT_left_boundary (yy[0]);	xx[1] = GMT_right_boundary (yy[0]);
			sides[0] = 3;	sides[1] = 1;
			angle[0] = R2D * d_atan2 (dy, -dx);
		}
		if (yy[0] >= 0.0 && yy[0] <= project_info.ymax) wrap = TRUE;
		angle[1] = angle[0] + 180.0;
	}
	
	if (wrap) *nx = 2;
	return (wrap);
}

/* For global TM maps */

int GMT_wrap_around_check_tm (double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, int *sides, int *nx)
{
	int i, wrap = FALSE, skip;
	double dx, dy, width, jump;
	
	jump = this_y - last_y;
	width = 0.5 * GMT_map_height;
	
	skip = ((fabs (jump) < width) || (fabs(jump) <= SMALL));
	dx = this_x - last_x;
		     
	for (i = 0; i < (*nx); i++) {	/* Must check if the crossover found should wrap around */
		if (skip) continue;
		
		if (jump < (-width)) {	/* Crossed top boundary */
			yy[0] = GMT_map_height;	yy[1] = 0.0;
			dy = this_y + GMT_map_height - last_y;
			xx[0] = xx[1] = last_x + (GMT_map_height - last_y) * dx / dy;
			sides[0] = 2;	sides[1] = 0;
			angle[0] = R2D * d_atan2 (dy, dx);
		}
		else {	/* Crossed bottom boundary */
			yy[0] = 0.0;	yy[1] = GMT_map_height;
			dy = last_y + GMT_map_height - this_y;
			xx[0] = xx[1] = last_x + last_y * dx / dy;
			sides[0] = 0;	sides[1] = 2;
			angle[0] = R2D * d_atan2 (dy, -dx);
		}
		angle[1] = angle[0] + 180.0;
		if (xx[0] >= 0.0 && xx[0] <= project_info.xmax) wrap = TRUE;
	}
	
	if (*nx == 0 && !skip) {	/* Must wrap around */
		if (jump < (-width)) {	/* Crossed top boundary */
			yy[0] = GMT_map_height;	yy[1] = 0.0;
			dy = this_y + GMT_map_height - last_y;
			xx[0] = xx[1] = last_x + (GMT_map_height - last_y) * dx / dy;
			sides[0] = 2;	sides[1] = 0;
			angle[0] = R2D * d_atan2 (dy, dx);
		}
		else {	/* Crossed bottom boundary */
			yy[0] = 0.0;	yy[1] = GMT_map_height;
			dy = last_y + GMT_map_height - this_y;
			xx[0] = xx[1] = last_x + last_y * dx / dy;
			sides[0] = 0;	sides[1] = 2;
			angle[0] = R2D * d_atan2 (dy, -dx);
		}
		if (xx[0] >= 0.0 && xx[0] <= project_info.xmax) wrap = TRUE;
		angle[1] = angle[0] + 180.0;
	}
	
	if (wrap) *nx = 2;
	return (wrap);
}

double GMT_left_ellipse (double y)
{
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */
	
	y = (y - project_info.y0) / project_info.w_r;	/* Fraction, relative to Equator */
	return (GMT_half_map_size - 2.0 * project_info.w_r * d_sqrt (1.0 - y * y));
}

double GMT_left_winkel (double y)
{
	int n_iter = 0;
	double c, phi, phi0, delta, x, y0, sp, cp;
	
	y0 = 0.5 * project_info.ymax;
	y -= y0;
	y *= project_info.i_y_scale;
	c = 2.0 * y * project_info.i_EQ_RAD;
	phi = y * project_info.i_EQ_RAD;
	do {
		phi0 = phi;
		sincos (phi0, &sp, &cp);
		phi = phi0 - (phi0 + M_PI_2 * sp - c) / (1.0 + M_PI_2 * cp);
		delta = fabs (phi - phi0);
		n_iter++;
	}
	while (delta > GMT_CONV_LIMIT && n_iter < 100);
	GMT_geo_to_xy (project_info.central_meridian-180.0, phi * R2D, &x, &c);
	return (x);
}

double GMT_left_eckert4 (double y)
{
	double x, phi;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = d_asin (y * project_info.k4_iy);
	x = project_info.k4_x * D2R * (project_info.w - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_left_eckert6 (double y)
{
	double x, phi;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = 0.5 * y * project_info.k6_ir;
	x = project_info.k6_r * D2R * (project_info.w - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_left_robinson (double y)
{
	double x, X, Y;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	Y = fabs (y * project_info.n_i_cy);
	GMT_intpol (project_info.n_Y, project_info.n_X, 19, 1, &Y, &X, gmtdefs.interpolant);
	
	x = project_info.n_cx * X * (project_info.w - project_info.central_meridian);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_left_sinusoidal (double y)
{
	double x, lat;
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	lat = y * project_info.i_EQ_RAD;
	x = project_info.EQ_RAD * (project_info.w - project_info.central_meridian) * D2R * cos (lat);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_left_circle (double y)
{
	y -= project_info.r;
	return (GMT_half_map_size - d_sqrt (project_info.r * project_info.r - y * y));
}

double GMT_left_conic (double y)
{
	double x_ws, y_ws, x_wn, y_wn, dy;

	GMT_geo_to_xy (project_info.w, project_info.s, &x_ws, &y_ws);
	GMT_geo_to_xy (project_info.w, project_info.n, &x_wn, &y_wn);
	dy = y_wn - y_ws;
	if (fabs (dy) < GMT_CONV_LIMIT) return (0.0);
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

double GMT_right_winkel (double y)
{
	int n_iter = 0;
	double c, phi, phi0, delta, x, y0, sp, cp;
	
	y0 = 0.5 * project_info.ymax;
	y -= y0;
	y *= project_info.i_y_scale;
	c = 2.0 * y * project_info.i_EQ_RAD;
	phi = y * project_info.i_EQ_RAD;
	do {
		phi0 = phi;
		sincos (phi0, &sp, &cp);
		phi = phi0 - (phi0 + M_PI_2 * sp - c) / (1.0 + M_PI_2 * cp);
		delta = fabs (phi - phi0);
		n_iter++;
	}
	while (delta > GMT_CONV_LIMIT && n_iter < 100);
	GMT_geo_to_xy (project_info.central_meridian+180.0, phi * R2D, &x, &c);
	return (x);
}

double GMT_right_eckert4 (double y)
{
	double x, phi;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = d_asin (y * project_info.k4_iy);
	x = project_info.k4_x * D2R * (project_info.e - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_eckert6 (double y)
{
	double x, phi;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = 0.5 * y * project_info.k6_ir;
	x = project_info.k6_r * D2R * (project_info.e - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_robinson (double y)
{
	double x, X, Y;
	
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	Y = fabs (y * project_info.n_i_cy);
	GMT_intpol (project_info.n_Y, project_info.n_X, 19, 1, &Y, &X, gmtdefs.interpolant);
	
	x = project_info.n_cx * X * (project_info.e - project_info.central_meridian);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_sinusoidal (double y)
{
	double x, lat;
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	lat = y * project_info.i_EQ_RAD;
	x = project_info.EQ_RAD * (project_info.e - project_info.central_meridian) * D2R * cos (lat);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_circle (double y)
{
	y -= project_info.r;
	return (GMT_half_map_size + d_sqrt (project_info.r * project_info.r - y * y));
}

double GMT_right_conic (double y)
{
	double x_es, y_es, x_en, y_en, dy;

	GMT_geo_to_xy (project_info.e, project_info.s, &x_es, &y_es);
	GMT_geo_to_xy (project_info.e, project_info.n, &x_en, &y_en);
	dy = y_en - y_es;
	if (fabs (dy) < GMT_CONV_LIMIT) return (GMT_map_width);
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

int GMT_map_jump_x (double x0, double y0, double x1, double y1)
{
	/* TRUE if x-distance between points exceeds 1/2 map width at this y value */
	double dx, map_half_size;
	
	if (!MAPPING || fabs (project_info.w - project_info.e) < 90.0) return (FALSE);
	
	map_half_size = MAX (GMT_half_map_width (y0), GMT_half_map_width (y1));
	if (fabs (map_half_size) < SMALL) return (0);
	
	dx = x1 - x0;
	if (dx > map_half_size)	/* Cross left/west boundary */
		return (-1);
	else if (dx < (-map_half_size)) /* Cross right/east boundary */
		return (1);
	else
		return (0);
}
	
int GMT_map_jump_tm (double x0, double y0, double x1, double y1)
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
	c = (fabs (dxb) < GMT_CONV_LIMIT) ? 0.0 : 1.0 + dxa/dxb;
	dyb = (fabs (c) < GMT_CONV_LIMIT) ? 0.0 : fabs (yb - ya) / c;
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
	c = (fabs (dy) < GMT_CONV_LIMIT) ? 0.0 : (xa - xb) / dy;
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
	
		case STEREO:	/* Must compute width of circular map based on y value (ASSUMES FULL CIRCLE!!!) */
		case LAMB_AZ_EQ:
		case ORTHO:
		case GNOMONIC:
		case AZ_EQDIST:
		case GRINTEN:
			if (project_info.region && GMT_world_map) {
				y -= project_info.r;
				half_width = d_sqrt (project_info.r * project_info.r - y * y);
			}
			else
				half_width = GMT_half_map_size;
			break;

		case MOLLWEIDE:		/* Must compute width of Mollweide map based on y value */
		case HAMMER:
		case WINKEL:
		case SINUSOIDAL:
		case ROBINSON:
		case ECKERT4:
		case ECKERT6:
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

BOOLEAN GMT_will_it_wrap_x (double *x, double *y, int n, int *start)
{	/* Determines if a polygon will wrap at edges */
	int i;
	BOOLEAN wrap;
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

BOOLEAN GMT_will_it_wrap_tm (double *x, double *y, int n, int *start)
{	/* Determines if a polygon will wrap at edges for TM global projection */
	int i;
	BOOLEAN wrap;
	
	if (!GMT_world_map) return (FALSE);
	
	for (i = 1, wrap = FALSE; !wrap && i < n; i++) {
		wrap = GMT_this_point_wraps_tm (y[i-1], y[i]);
	}
	*start = i - 1;
	return (wrap);
}

BOOLEAN GMT_this_point_wraps_x (double x0, double x1, double w_last, double w_this)
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

	return ((dx = fabs (x1 - x0)) > w_max && w_min > SMALL);
}

BOOLEAN GMT_this_point_wraps_tm (double y0, double y1)
{
	/* Returns TRUE if the 2 y-points implies a jump at this x-level of the TM map */

	double dy;

	return ((dy = fabs (y1 - y0)) > GMT_half_map_height);
}

int GMT_truncate_x (double *x, double *y, int n, int start, int l_or_r)
{	/* Truncates a wrapping polygon agains left or right edge */

	int i, i1, j, k;
	double xc[4], yc[4], w_last, w_this;
	PFD x_on_border;

	/* First initialize variables that differ for left and right truncation */

	if (l_or_r == -1) {	/* Left truncation (-1) */
		/* Find first point that is left of map center */
	
		i = (x[start] < GMT_half_map_size) ? start : start - 1;
		x_on_border = GMT_left_boundary;
	}
	else {				/* Right truncation (+1) */
		/* Find first point that is right of map center */
		i = (x[start] > GMT_half_map_size) ? start : start - 1;
		x_on_border = GMT_right_boundary;
	}

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
			GMT_x_plot[j] = (*x_on_border) (yc[0]);
			GMT_y_plot[j] = yc[0];
			j++;
			if (j >= GMT_n_alloc) GMT_get_plot_array ();
		}
		if (l_or_r == -1) /* Left */
			GMT_x_plot[j] = (x[i] > GMT_half_map_size) ? (*x_on_border) (y[i]) : x[i];
		else	/* Right */
			GMT_x_plot[j] = (x[i] < GMT_half_map_size) ? (*x_on_border) (y[i]) : x[i];
		GMT_y_plot[j] = y[i];
		j++, k++;
		if (j >= GMT_n_alloc) GMT_get_plot_array ();
	}
	return (j);
}

int GMT_truncate_tm (double *x, double *y, int n, int start, int b_or_t)
{	/* Truncates a wrapping polygon agains bottom or top edge for global TM maps */

	int i, i1, j, k;
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
			GMT_y_plot[j] = (y[i] > GMT_half_map_height) ? 0.0 : y[i];
		else	/* Top */
			GMT_y_plot[j] = (y[i] < GMT_half_map_height) ? GMT_map_height : y[i];
		GMT_x_plot[j] = x[i];
		j++, k++;
		if (j >= GMT_n_alloc) GMT_get_plot_array ();
	}
	return (j);
}

double GMT_great_circle_dist(double lon1, double lat1, double lon2, double lat2)
{
	/* great circle distance on a sphere in degrees */

	double C, a, b, c;
	double cosC, cosa, cosb, cosc;
	double sina, sinb;

	if( (lat1==lat2) && (lon1==lon2) ) {
		return( (double)0.0 );
		}

	a=D2R*(90.0-lat2);
	b=D2R*(90.0-lat1);

	C = D2R*( lon2 - lon1 );

	sincos (a, &sina, &cosa);
	sincos (b, &sinb, &cosb);
	cosC = cos(C);

	cosc = cosa*cosb + sina*sinb*cosC;
	if (cosc<-1.0) c=M_PI; else if (cosc>1) c=0.0; else c=d_acos(cosc);

	return( c * R2D);
}

int GMT_great_circle_intersection (double A[], double B[], double C[], double X[], double *CX_dist)
{
	/* A, B, C are 3-D Cartesian unit vectors, i.e., points on the sphere.
	 * Let points A and B define a great circle, and consider a
	 * third point C.  A second great cirle goes through C and
	 * is orthogonal to the first great circle.  Their intersection
	 * X is the point on (A,B) closest to C.  We must test if X is
	 * between A,B or outside.
	 */

	double P[3], E[3], cos_AB, cos_test;
	
	GMT_cross3v (A, B, P);			/* Get pole position of plane through A and B (and origin O) */
	GMT_normalize3v (P);			/* Make sure P has unit length */
	GMT_cross3v (C, P, E);			/* Get pole E to plane through C (and origin) but normal to A,B (hence going through P) */
	GMT_normalize3v (E);			/* Make sure X has unit length */
	GMT_cross3v (P, E, X);			/* Intersection point between the two planes */
	GMT_normalize3v (X);			/* Make sure X has unit length */
			
	/* Must first check if X is along the (A,B) segment and not on its extension */
			
	cos_AB = fabs (GMT_dot3v (A, B));	/* Cos of spherical distance between A,B */
	cos_test = fabs (GMT_dot3v (A, X));	/* Cos of spherical distance between A and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to A exceeds the A-B length */
	cos_test = fabs (GMT_dot3v (B, X));	/* Cos of spherical distance between B and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to B exceeds the A-B length */
			
	/* X is between A and B.  Now calculate distance between C and X */
	
	*CX_dist = fabs (GMT_dot3v (C, X));	/* Cos of spherical distance between C and X */
	return (0);				/* Return zero if intersection is between A and B */
}

/* The *_outside routines returns the status of the current point.  Status is
 * the sum of x_status and y_status. x_status may be
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
 
int GMT_wesn_outside (double lon, double lat)
{
	/* This version ensures that any point will be considered inside if
	 * it is off by a multiple of 360 degrees in longitude.  The following
	 * function does not make that allowance (see comments below) */
	 
	while (lon < project_info.w && (lon + 360.0) <= project_info.e) lon += 360.0;
	while (lon > project_info.e && (lon - 360.0) >= project_info.w) lon -= 360.0;
	
	if (GMT_on_border_is_outside && fabs (lon - project_info.w) < SMALL )
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lon - project_info.e) < SMALL)
		GMT_x_status_new = 1;
	else if (lon < project_info.w)
		GMT_x_status_new = -2;
	else if (lon > project_info.e)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;
		
	if (GMT_on_border_is_outside && fabs (lat - project_info.s) < SMALL )
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lat - project_info.n) < SMALL)
		GMT_y_status_new = 1;
	else if (lat < project_info.s )
		GMT_y_status_new = -2;
	else if (lat > project_info.n)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;
		
	return ( !(GMT_x_status_new == 0 && GMT_y_status_new == 0));
	
}

int GMT_wesn_outside_np (double lon, double lat)
{
	/* This version of GMT_wesn_outside is used when we do not want to
	 * consider the fact that longitude is periodic.  This is necessary
	 * when we are making basemaps and may want to ensure that a point is
	 * slightly outside the border without having it automatically flip by
	 * 360 degrees in a test such as wesn_outside provides.  The GMT_outside
	 * pointer will be temporarily set to point to this routine during the
	 * construction of the basemap and then reset to GMT_wesn_outside */
	
	if (GMT_on_border_is_outside && fabs (lon - project_info.w) < SMALL )
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lon - project_info.e) < SMALL)
		GMT_x_status_new = 1;
	else if (lon < project_info.w)
		GMT_x_status_new = -2;
	else if (lon > project_info.e)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;
		
	if (GMT_on_border_is_outside && fabs (lat - project_info.s) < SMALL )
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lat - project_info.n) < SMALL)
		GMT_y_status_new = 1;
	else if (lat < project_info.s )
		GMT_y_status_new = -2;
	else if (lat > project_info.n)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;
		
	return ( !(GMT_x_status_new == 0 && GMT_y_status_new == 0));
	
}

int GMT_polar_outside (double lon, double lat)
{	

	if (GMT_world_map) {
		/* while ((lon - project_info.central_meridian) < -180.0) lon += 360.0;
		while ((lon - project_info.central_meridian) > 180.0) lon -= 360.0; */
		while (lon < project_info.w && (lon + 360.0) < project_info.e) lon += 360.0;
		while (lon > project_info.e && (lon - 360.0) > project_info.w) lon -= 360.0;
	}
	
	if (GMT_on_border_is_outside && fabs (lon - project_info.w) < SMALL )
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lon - project_info.e) < SMALL)
		GMT_x_status_new = 1;
	else if (lon < project_info.w)
		GMT_x_status_new = -2;
	else if (lon > project_info.e)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;
	if (!project_info.edge[1]) GMT_x_status_new = 0;	/* 360 degrees, no edge */
		
	if (GMT_on_border_is_outside && fabs (lat - project_info.s) < SMALL )
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (lat - project_info.n) < SMALL)
		GMT_y_status_new = 1;
	else if (lat < project_info.s )
		GMT_y_status_new = -2;
	else if (lat > project_info.n)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;
	if (GMT_y_status_new < 0 && !project_info.edge[0]) GMT_y_status_new = 0;	/* South pole enclosed */
	if (GMT_y_status_new > 0 && !project_info.edge[2]) GMT_y_status_new = 0;	/* North pole enclosed */
	
	return ( !(GMT_x_status_new == 0 && GMT_y_status_new == 0));
	
}

int GMT_eqdist_outside (double lon, double lat)
{
	double cc, s, c;
	
	lon -= project_info.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	lat *= D2R;
	sincos (lat, &s, &c);
	cc = project_info.sinp * s + project_info.cosp * c * cosd (lon);
	/* if (cc <= -1.0) { */
	if (cc < -1.0) {
		GMT_y_status_new = -1;
		GMT_x_status_new = 0;
	}
	else
		GMT_x_status_new = GMT_y_status_new = 0;
	return ( !(GMT_y_status_new == 0));
}

int GMT_radial_outside (double lon, double lat)
{
	double dist;
	
	/* Test if point is more than horizon spherical degrees from origin.  For global maps, let all borders by "south" */

	GMT_x_status_new = 0;
	dist = GMT_great_circle_dist (lon, lat, project_info.central_meridian, project_info.pole);
	if (GMT_on_border_is_outside && fabs (dist - project_info.f_horizon) < SMALL)
		GMT_y_status_new = -1;
	else if (dist > project_info.f_horizon)
		GMT_y_status_new = -2;
	else
		GMT_y_status_new = 0;
	return ( !(GMT_y_status_new == 0));
}

int GMT_rect_outside (double lon, double lat)
{
	double x, y;
	
	GMT_geo_to_xy (lon, lat, &x, &y);
	
	if (GMT_on_border_is_outside && fabs (x - project_info.xmin) < SMALL )
		GMT_x_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (x - project_info.xmax) < SMALL)
		GMT_x_status_new = 1;
	else if (x < project_info.xmin)
		GMT_x_status_new = -2;
	else if (x > project_info.xmax)
		GMT_x_status_new = 2;
	else
		GMT_x_status_new = 0;
		
	if (GMT_on_border_is_outside && fabs (y -project_info.ymin) < SMALL )
		GMT_y_status_new = -1;
	else if (GMT_on_border_is_outside && fabs (y - project_info.ymax) < SMALL)
		GMT_y_status_new = 1;
	else if (y < project_info.ymin)
		GMT_y_status_new = -2;
	else if (y > project_info.ymax)
		GMT_y_status_new = 2;
	else
		GMT_y_status_new = 0;
		
	return ( !(GMT_x_status_new == 0 && GMT_y_status_new == 0));
	
}

int GMT_rect_outside2 (double lon, double lat)
{	/* For Azimuthal proj with rect borders since GMT_rect_outside may fail for antipodal points */
	if (GMT_radial_outside (lon, lat)) return (TRUE);	/* Point > 90 degrees away */
	return (GMT_rect_outside (lon, lat));	/* Must check if inside box */
}

int GMT_pen_status (void) {
	int pen = 3;
	
	if (GMT_x_status_old == 0 && GMT_y_status_old == 0)
		pen = 2;
	else if (GMT_x_status_new == 0 && GMT_y_status_new == 0)
		pen = 3;
	return (pen);
}

int GMT_wesn_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, int *sides)
{
	/* Compute the crossover point(s) on the map boundary for rectangular projections */
	int n = 0, i;
	double dlat, dlon, dlon0, dlat0;
	
	/* Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */
	
	/* First align the longitudes to the region */
	
	if (GMT_world_map) {
		while (lon0 < project_info.w) lon0 += 360.0;
		while (lon0 > project_info.e) lon0 -= 360.0;
		while (lon1 < project_info.w) lon1 += 360.0;
		while (lon1 > project_info.e) lon1 -= 360.0;
	}
	
	
	/* Then set 'almost'-corners to corners */
	
	dlon0 = lon0 - lon1;
	dlat0 = lat0 - lat1;
	GMT_x_wesn_corner (&lon0);
	GMT_x_wesn_corner (&lon1);
	GMT_y_wesn_corner (&lat0);
	GMT_y_wesn_corner (&lat1);
	
	
	if ((lat0 >= project_info.s && lat1 <= project_info.s) || (lat1 >= project_info.s && lat0 <= project_info.s)) {
		sides[n] = 0;
		clat[n] = project_info.s;
		dlat = lat0 - lat1;
		clon[n] = (fabs (dlat) < GMT_CONV_LIMIT) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / dlat;
		GMT_x_wesn_corner (&clon[n]);
		if (fabs(dlat0) > 0.0 && GMT_lon_inside (clon[n], project_info.w, project_info.e)) n++;
	}
	if ((lon0 >= project_info.e && lon1 <= project_info.e) || (lon1 >= project_info.e && lon0 <= project_info.e)) {
		sides[n] = 1;
		clon[n] = project_info.e;
		dlon = lon0 - lon1;
		clat[n] = (fabs (dlon) < GMT_CONV_LIMIT) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / dlon;
		GMT_y_wesn_corner (&clat[n]);
		if (fabs(dlon0) > 0.0 && clat[n] >= project_info.s && clat[n] <= project_info.n) n++;
	}
	if ((lat0 >= project_info.n && lat1 <= project_info.n) || (lat1 >= project_info.n && lat0 <= project_info.n)) {
		sides[n] = 2;
		clat[n] = project_info.n;
		dlat = lat0 - lat1;
		clon[n] = (fabs (dlat) < GMT_CONV_LIMIT) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / dlat;
		GMT_x_wesn_corner (&clon[n]);
		if (fabs(dlat0) > 0.0 && GMT_lon_inside (clon[n], project_info.w, project_info.e)) n++;
	}
	if ((lon0 <= project_info.w && lon1 >= project_info.w) || (lon1 <= project_info.w && lon0 >= project_info.w)) {
		sides[n] = 3;
		clon[n] = project_info.w;
		dlon = lon0 - lon1;
		clat[n] = (fabs (dlon) < GMT_CONV_LIMIT) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / dlon;
		GMT_y_wesn_corner (&clat[n]);
		if (fabs(dlon0) > 0.0 && clat[n] >= project_info.s && clat[n] <= project_info.n) n++;
	}
	
	for (i = 0; i < n; i++) {
		GMT_geo_to_xy (clon[i], clat[i], &xx[i], &yy[i]);
		if (project_info.projection == POLAR && sides[i]%2) sides[i] = 4 - sides[i];	/*  toggle 1 <-> 3 */
	}

	/* Check for corner xover */
	
	if (n < 2) return (n);
	
	if (GMT_is_wesn_corner (clon[0], clat[0])) return (1); 
	
	if (GMT_is_wesn_corner (clon[1], clat[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1); 
	}

	return (n);
}

int GMT_rect_crossing (double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, int *sides)
{

	/* Compute the crossover point(s) on the map boundary for rectangular projections */
	int i, j, n = 0;
	double x0, x1, y0, y1, d, dx, dy;
	
	/* Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */
		
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
		xx[n] = (fabs (d) < GMT_CONV_LIMIT) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (&xx[n]);
		if (fabs(dy) > 0.0 && xx[n] >= project_info.xmin && xx[n] <= project_info.xmax) n++;
	}
	if ((x0 <= project_info.xmax && x1 >= project_info.xmax) || (x1 <= project_info.xmax && x0 >= project_info.xmax)) {
		sides[n] = 1;
		xx[n] = project_info.xmax;
		d = x0 - x1;
		yy[n] = (fabs (d) < GMT_CONV_LIMIT) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (&yy[n]);
		if (fabs(dx) > 0.0 && yy[n] >= project_info.ymin && yy[n] <= project_info.ymax) n++;
	}
	if ((y0 <= project_info.ymax && y1 >= project_info.ymax) || (y1 <= project_info.ymax && y0 >= project_info.ymax)) {
		sides[n] = 2;
		yy[n] = project_info.ymax;
		d = y0 - y1;
		xx[n] = (fabs (d) < GMT_CONV_LIMIT) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (&xx[n]);
		if (fabs(dy) > 0.0 && xx[n] >= project_info.xmin && xx[n] <= project_info.xmax) n++;
	}
	if ((x0 >= project_info.xmin && x1 <= project_info.xmin) || (x1 >= project_info.xmin && x0 <= project_info.xmin)) {
		sides[n] = 3;
		xx[n] = project_info.xmin;
		d = x0 - x1;
		yy[n] = (fabs (dx) < GMT_CONV_LIMIT) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (&yy[n]);
		if (fabs(d) > 0.0 && yy[n] >= project_info.ymin && yy[n] <= project_info.ymax) n++;
	}

	/* Eliminate duplicates */

	for (i = 0; i < n; i++) {
		for (j = i + 1; j < n; j++) {
			if (fabs (xx[i] - xx[j]) < GMT_CONV_LIMIT && fabs (yy[i] - yy[j]) < GMT_CONV_LIMIT)	/* Duplicate */
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

	if (!MAPPING) return (n);

	/* Check for corner xover */
	
	if (n < 2) return (n);
	
	if (GMT_is_rect_corner (xx[0], yy[0])) return (1); 
	
	if (GMT_is_rect_corner (xx[1], yy[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	return (n);
}

void GMT_x_rect_corner (double *x)
{
	if (fabs (*x) <= SMALL)
		*x = 0.0;
	else if (fabs (*x - project_info.xmax) <= SMALL)
		*x = project_info.xmax;
}

void GMT_y_rect_corner (double *y)
{
	if (fabs (*y) <= SMALL)
		*y = 0.0;
	else if (fabs (*y - project_info.ymax) <= SMALL)
		*y = project_info.ymax;
}

int GMT_is_rect_corner (double x, double y)
{	/* Checks if point is a corner */
	GMT_corner = -1;
	if (fabs (x - project_info.xmin) < GMT_CONV_LIMIT) {
		if (fabs (y - project_info.ymin) < GMT_CONV_LIMIT)
			GMT_corner = 1;
		else if (fabs (y - project_info.ymax) < GMT_CONV_LIMIT)
			GMT_corner = 4;
	}
	else if (fabs (x - project_info.xmax) < GMT_CONV_LIMIT) {
		if (fabs (y - project_info.ymin) < GMT_CONV_LIMIT)
			GMT_corner = 2;
		else if (fabs (y - project_info.ymax) < GMT_CONV_LIMIT)
			GMT_corner = 3;
	}
	return (GMT_corner > 0);
}

void GMT_x_wesn_corner (double *x)
{
/*	if (fabs (fmod (fabs (*x - project_info.w), 360.0)) <= SMALL)
		*x = project_info.w;
	else if (fabs (fmod (fabs (*x - project_info.e), 360.0)) <= SMALL)
		*x = project_info.e; */
		
	if (fabs (*x - project_info.w) <= SMALL)
		*x = project_info.w;
	else if (fabs (*x - project_info.e) <= SMALL)
		*x = project_info.e;
	
}

void GMT_y_wesn_corner (double *y)
{
	if (fabs (*y - project_info.s) <= SMALL)
		*y = project_info.s;
	else if (fabs (*y - project_info.n) <= SMALL)
		*y = project_info.n;
}

int GMT_is_wesn_corner (double x, double y)
{	/* Checks if point is a corner */
	GMT_corner = 0;
	
	if (fabs (fmod (fabs (x - project_info.w), 360.0)) < GMT_CONV_LIMIT) {
		if (fabs (y - project_info.s) < GMT_CONV_LIMIT)
			GMT_corner = 1;
		else if (fabs (y - project_info.n) < GMT_CONV_LIMIT)
			GMT_corner = 4;
	}
	else if (fabs (fmod (fabs (x - project_info.e), 360.0)) < GMT_CONV_LIMIT) {
		if (fabs (y - project_info.s) < GMT_CONV_LIMIT)
			GMT_corner = 2;
		else if (fabs (y - project_info.n) < GMT_CONV_LIMIT)
			GMT_corner = 3;
	}
	return (GMT_corner > 0);
}

int GMT_radial_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides)
{
	/* Computes the lon/lat of a point that is f_horizon spherical degrees from
	 * the origin and lies on the great circle between points 1 and 2 */
	
	double dist1, dist2, delta, eps, dlon;
	
	dist1 = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon1, lat1);
	dist2 = GMT_great_circle_dist (project_info.central_meridian, project_info.pole, lon2, lat2);
	delta = dist2 - dist1;
	eps = (fabs (delta) < GMT_CONV_LIMIT) ? 0.0 : (project_info.f_horizon - dist1) / delta;
	dlon = lon2 - lon1;
	if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
	clon[0] = lon1 + dlon * eps;
	clat[0] = lat1 + (lat2 - lat1) * eps;
	
	GMT_geo_to_xy (clon[0], clat[0], &xx[0], &yy[0]);
	sides[0] = 1;
	
	return (1);
}

int GMT_ellipse_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides)
{
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
		n = -2;	/* To signal don't change order */
	}
	if (n == 1) for (i = 0; i < n; i++) GMT_geo_to_xy (clon[i], clat[i], &xx[i], &yy[i]);
	return (n);
}

int GMT_eqdist_crossing (double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, int *sides)
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

int *GMT_split_line (double **xx, double **yy, int *nn, BOOLEAN add_crossings)
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
	int i, j, k, n, n_seg, *split, *pos;
	short int *way;
	size_t n_alloc = GMT_SMALL_CHUNK;
	
	/* First quick scan to see how many jumps there are */
	
	xin = *xx;	yin = *yy;
	pos = (int *) GMT_memory (VNULL, n_alloc, sizeof (int), GMT_program);
	way = (short int *) GMT_memory (VNULL, n_alloc, sizeof (short int), GMT_program);
	for (n_seg = 0, i = 1; i < *nn; i++) {
		if ((k = GMT_map_jump_x (xin[i], yin[i], xin[i-1], yin[i-1]))) {
			pos[n_seg] = i;		/* 2nd of the two points that generate the jump */
			way[n_seg] = k;		/* Which way we jump : +1 is right to left, -1 is left to right */
			n_seg++;
			if (n_seg == (int)n_alloc) {
				n_alloc += GMT_SMALL_CHUNK;
				pos = (int *) GMT_memory ((void *)pos, n_alloc, sizeof (int), GMT_program);
				way = (short int *) GMT_memory ((void *)way, n_alloc, sizeof (short int), GMT_program);
			}
		}
	}
	
	if (n_seg == 0) {	/* No jumps, just return NULL */
		GMT_free ((void *)pos);
		GMT_free ((void *)way);
		return ((int *)NULL);
	}
	
	/* Here we have one or more jumps so we need to split the line */
	
	n = *nn;				/* Original line count */
	if (add_crossings) n += 2 * n_seg;	/* Must add 2 crossing points per jump */
	x = (double *) GMT_memory (VNULL, n, sizeof (double), GMT_program);
	y = (double *) GMT_memory (VNULL, n, sizeof (double), GMT_program);
	split = (int *) GMT_memory (VNULL, n_seg+2, sizeof (int), GMT_program);
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

int GMT_graticule_path (double **x, double **y, int dir, double w, double e, double s, double n)
{	/* Returns the path of a graticule (box of meridians and parallels) */
	int np;
	double *xx, *yy;
	double px0, px1, px2, px3;
	
	if (dir == 1) {	/* Forward sense */
		px0 = px3 = w;	px1 = px2 = e;
	}
	else {	/* Reverse sense */
		px0 = px3 = e;	px1 = px2 = w;
	}
	
	/* Close graticule from point 0 through point 4 */
	
	if (RECT_GRATICULE) {	/* Simple rectangle in this projection */
		xx = (double *) GMT_memory (VNULL, 4, sizeof (double), GMT_program);
		yy = (double *) GMT_memory (VNULL, 4, sizeof (double), GMT_program);
		xx[0] = px0;	xx[1] = px1;	xx[2] = px2;	xx[3] = px3;
		yy[0] = yy[1] = s;	yy[2] = yy[3] = n;
		np = 4;
	}
	else {	/* Must assemble path from meridians and parallel pieces */
		double *xtmp, *ytmp;
		int add;
		size_t n_alloc;
		
		/* SOUTH BORDER */
		
		if (MAPPING && s == -90.0) {	/* No path, just a point */
			xx = (double *)GMT_memory (VNULL, 1, sizeof (double), GMT_program);
			yy = (double *)GMT_memory (VNULL, 1, sizeof (double), GMT_program);
			xx[0] = px1;	yy[0] = -90.0;
			np = n_alloc = 1;
		}
		else {			
			np = n_alloc = add = GMT_latpath (s, px0, px1, &xx, &yy);	/* South */
		}
		
		/* EAST (OR WEST) BORDER */
		
		add = GMT_lonpath (px1, s, n, &xtmp, &ytmp);	/* east (or west if dir == -1) */
		n_alloc += add;
		xx = (double *)GMT_memory ((void *)xx, n_alloc, sizeof (double), GMT_program);
		yy = (double *)GMT_memory ((void *)yy, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);
		
		/* NORTH BORDER */
		
		if (MAPPING && n == 90.0) {	/* No path, just a point */
			xtmp = (double *)GMT_memory (VNULL, 1, sizeof (double), GMT_program);
			ytmp = (double *)GMT_memory (VNULL, 1, sizeof (double), GMT_program);
			xtmp[0] = px3;	ytmp[0] = +90.0;
			add = 1;
		}
		else {
			add = GMT_latpath (n, px2, px3, &xtmp, &ytmp);	/* North */
		}
		n_alloc += add;
		xx = (double *)GMT_memory ((void *)xx, n_alloc, sizeof (double), GMT_program);
		yy = (double *)GMT_memory ((void *)yy, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);
		
		/* WEST (OR EAST) BORDER */
		
		add = GMT_lonpath (px3, n, s, &xtmp, &ytmp);	/* west */
		n_alloc += add;
		xx = (double *)GMT_memory ((void *)xx, n_alloc, sizeof (double), GMT_program);
		yy = (double *)GMT_memory ((void *)yy, n_alloc, sizeof (double), GMT_program);
		memcpy ((void *)&xx[np], (void *)xtmp, (size_t)(add * sizeof (double)));
		memcpy ((void *)&yy[np], (void *)ytmp, (size_t)(add * sizeof (double)));
		np += add;
		GMT_free ((void *)xtmp);	GMT_free ((void *)ytmp);
	}
	
	if (GMT_io.in_col_type[0] == GMT_IS_LON) {
		BOOLEAN straddle;
		int i;
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

int GMT_map_path (double lon1, double lat1, double lon2, double lat2, double **x, double **y)
{
	if (fabs (lat1 - lat2) < 1.0e-10)
		return (GMT_latpath (lat1, lon1, lon2, x, y));
	else
		return (GMT_lonpath (lon1, lat1, lat2, x, y));
}

int GMT_lonpath (double lon, double lat1, double lat2, double **x, double **y)
{
	int ny, n, n_try, keep_trying, jump, pos;
	double dlat, dlat0, *tlon, *tlat, x0, x1, y0, y1, d;
	double min_gap;

	if (GMT_meridian_straight) {	/* Easy, just a straight line connect via quarter-points */
		n = 5;
		tlon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_lonpath");
		tlat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_lonpath");

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
	if ((ny = (int)ceil (fabs (lat2 - lat1) / gmtdefs.dlat)) == 0) return (0);

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
			if (MAPPING && fabs (tlat[n]) > 90.0) tlat[n] = copysign (90.0, tlat[n]);
			GMT_geo_to_xy (tlon[n], tlat[n], &x1, &y1);
			jump = (*GMT_map_jump) (x0, y0, x1, y1) || (y0 < project_info.ymin || y0 > project_info.ymax);
			if (!jump && (d = hypot (x1 - x0, y1 - y0)) > gmtdefs.line_step)
				dlat *= 0.5;
			else if (!jump && d < min_gap)
				dlat *= 2.0;
			else
				keep_trying = FALSE;
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

int GMT_latpath (double lat, double lon1, double lon2, double **x, double **y)
{
	int nx, n, n_try, keep_trying, jump, pos;
	double dlon, dlon0, *tlon, *tlat, x0, x1, y0, y1, d;
	double min_gap;

	if (GMT_parallel_straight) {	/* Easy, just a straight line connection via quarter points */
		n = 5;
		tlon = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_latpath");
		tlat = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_latpath");
		tlat[0] = tlat[1] = tlat[2] = tlat[3] = tlat[4] = lat;
		dlon = lon2 - lon1;
		tlon[0] = lon1;	tlon[1] = lon1 + 0.25 * dlon;	tlon[2] = lon1 + 0.5 * dlon;
		tlon[3] = lon1 + 0.75 * dlon;	tlon[4] = lon2;
		*x = tlon;	*y = tlat;
		return (n);
	}

	n = 0;
	min_gap = 0.1 * gmtdefs.line_step;
	if ((nx = (int)ceil (fabs (lon2 - lon1) / gmtdefs.dlon)) == 0) return (0);

	nx++;
	dlon0 = (lon2 - lon1) / nx;
	pos = (dlon0 > 0.0); 
	
	tlon = (double *) GMT_memory (VNULL, (size_t)nx, sizeof (double), "GMT_latpath");
	tlat = (double *) GMT_memory (VNULL, (size_t)nx, sizeof (double), "GMT_latpath");
	
	tlon[0] = lon1;
	tlat[0] = lat;
	GMT_geo_to_xy (tlon[0], tlat[0], &x0, &y0);
	while ((pos && (tlon[n] < lon2)) || (!pos && (tlon[n] > lon2))) {
		n++;
		if (n == nx-1) {
			nx += GMT_CHUNK;
			tlon = (double *) GMT_memory ((void *)tlon, (size_t)nx, sizeof (double), "GMT_latpath");
			tlat = (double *) GMT_memory ((void *)tlat, (size_t)nx, sizeof (double), "GMT_latpath");
		}
		n_try = 0;
		keep_trying = TRUE;
		dlon = dlon0;
		tlat[n] = lat;
		do {
			n_try++;
			tlon[n] = tlon[n-1] + dlon;
			GMT_geo_to_xy (tlon[n], tlat[n], &x1, &y1);
			jump = (*GMT_map_jump) (x0, y0, x1, y1) || (y0 < project_info.ymin || y0 > project_info.ymax);
			if (!jump && (d = hypot (x1 - x0, y1 - y0)) > gmtdefs.line_step)
				dlon *= 0.5;
			else if (!jump && d < min_gap)
				dlon *= 2.0;
			else
				keep_trying = FALSE;
		} while (keep_trying && n_try < 10);
		x0 = x1;	y0 = y1;
	}
	tlon[n] = lon2;
	tlat[n] = lat;
	n++;
	
	if (n != nx) {
		tlon = (double *) GMT_memory ((void *)tlon, (size_t)n, sizeof (double), "GMT_latpath");
		tlat = (double *) GMT_memory ((void *)tlat, (size_t)n, sizeof (double), "GMT_latpath");
	}
	
	*x = tlon;	*y = tlat;
	return (n);
}

/*  Routines to do with clipping */

int GMT_clip_to_map (double *lon, double *lat, int np, double **x, double **y)
{
	/* This routine makes sure that all points are either inside or on the map boundary
	 * and returns the number of points to be used for plotting (in x,y units) */
	 
	int i, n, out, out_x, out_y, np2, total_nx = 0;
	double *xx, *yy;
	
	/* First check for trivial cases:  All points outside or all points inside */
	
	for (i = out = out_x = out_y = 0; i < np; i++)  {
		(void) GMT_map_outside (lon[i], lat[i]);
		out_x += GMT_x_status_new;	/* Completely left of west gives -2 * np, right of east gives + 2 * np */
		out_y += GMT_y_status_new;	/* Completely below south gives -2 * np, above north gives + 2 * np */
		out += (abs (GMT_x_status_new) == 2 || abs (GMT_y_status_new) == 2);
	}
	if (out == 0) {		/* All points are inside map boundary */
		xx = (double *) GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_clip_to_map");
		yy = (double *) GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_clip_to_map");
		for (i = 0; i < np; i++)
			GMT_geo_to_xy (lon[i], lat[i], &xx[i], &yy[i]);
		*x = xx;	*y = yy;
		n = np;
	}
	else if (out == np) {	/* All points are outside map boundary */
		np2 = 2 * np;
		if (abs (out_x) == np2 || abs (out_y) == np2)	/* All points safely outside the region */
			n = 0;
		else {	/* All points are outside, but they are not just to one side so lines _may_ intersect the region */
			n = (*GMT_map_clip) (lon, lat, np, x, y, &total_nx);
			/* Polygons that completely contains the -R region will not generate crossings, just duplicate -R box */
			if (n > 0 && total_nx == 0) {	/* No crossings and all points outside means one of two things: */
				/* Either the polygon contains portions of the -R region including corners or it does not.  We pick the corners and check for insidedness: */
				BOOLEAN ok = FALSE;
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

int GMT_rect_clip (double *lon, double *lat, int n, double **x, double **y, int *total_nx)
{
	int i, j = 0, k, nx, sides[4], n_alloc = GMT_CHUNK;
	double xlon[4], xlat[4], xc[4], yc[4], *xx, *yy;
	
	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);
	
	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
	(void) GMT_map_outside (lon[0], lat[0]);
	GMT_geo_to_xy (lon[0], lat[0], &xx[0], &yy[0]);
	j += GMT_move_to_rect (xx, yy, j, 0);	/* May add 2 points, << n_alloc */
	
	/* for (i = j = 1; i < n; i++) { */
	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (lon[i], lat[i]);
		if (GMT_break_through (lon[i-1], lat[i-1], lon[i], lat[i])) {
			nx = GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
			for (k = 0; k < nx; k++) {
				xx[j] = xc[k];
				yy[j++] = yc[k];
				if (j >= (n_alloc-2)) {
					n_alloc += GMT_CHUNK;
					xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
					yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_rect_clip");
				}
				(*total_nx) ++;
			}
		}
		GMT_geo_to_xy (lon[i], lat[i], &xx[j], &yy[j]);
		if (j >= (n_alloc-2)) {
			n_alloc += GMT_CHUNK;
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
		
int GMT_wesn_clip (double *lon, double *lat, int n, double **x, double **y, int *total_nx)
{
	int i, j = 0, k, nx, sides[4], n_alloc = GMT_CHUNK;
	double xlon[4], xlat[4], xc[4], yc[4], *xx, *yy;

	*total_nx = 0;	/* Keep track of total of crossings */
	
	if (n == 0) return (0);
	
	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");

	(void) GMT_map_outside (lon[0], lat[0]);
	j = GMT_move_to_wesn (xx, yy, lon[0], lat[0], 0.0, 0.0, 0, 0);	/* May add 2 points, << n_alloc */
	
	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (lon[i], lat[i]);
		if (GMT_break_through (lon[i-1], lat[i-1], lon[i], lat[i])) {
			nx = GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
			for (k = 0; k < nx; k++) {
				xx[j] = xc[k];
				yy[j++] = yc[k];
				if (j >= (n_alloc-2)) {
					n_alloc += GMT_CHUNK;
					xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
					yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
				}
				(*total_nx) ++;
			}
		}
		if (j >= (n_alloc-2)) {
			n_alloc += GMT_CHUNK;
			xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");

			yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_wesn_clip");
		}
		j += GMT_move_to_wesn (xx, yy, lon[i], lat[i], lon[i-1], lat[i-1], j, nx);	/* May add 2 points, which explains the n_alloc-2 stuff */
	}

	xx = (double *) GMT_memory ((void *)xx, (size_t)j, sizeof (double), "GMT_wesn_clip");
	yy = (double *) GMT_memory ((void *)yy, (size_t)j, sizeof (double), "GMT_wesn_clip");
	*x = xx;
	*y = yy;

	return (j);
}		
		
int GMT_move_to_rect (double *x_edge, double *y_edge, int j, int nx)
{
	int n = 0, key;
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

int GMT_move_to_wesn (double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, int j, int nx)
{
	int n = 0, key;
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

int GMT_radial_clip (double *lon, double *lat, int np, double **x, double **y, int *total_nx)
{
	int n = 0, this, i, sides[4], n_alloc = GMT_CHUNK;
	double xlon[4], xlat[4], xc[4], yc[4], xr, yr, r, scale, x0, y0, *xx, *yy;
	
	*total_nx = 0;	/* Keep track of total of crossings */

	if (np == 0) return (0);
	
	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
	
	if (!GMT_map_outside (lon[0], lat[0])) {
		GMT_geo_to_xy (lon[0], lat[0], &xx[0], &yy[0]);
		n++;
	}
	for (i = 1; i < np; i++) {
		this = GMT_map_outside (lon[i], lat[i]);
		if (GMT_break_through (lon[i-1], lat[i-1], lon[i], lat[i])) {	/* Crossed map boundary */
			(void) GMT_map_crossing (lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
			xx[n] = xc[0];	yy[n] = yc[0];
			n++;
			(*total_nx) ++;
			if (n == n_alloc) {
				n_alloc += GMT_CHUNK;
				xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
				yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
			}
		}
		GMT_geo_to_xy (lon[i], lat[i], &xr, &yr);
		if (this) {	/* Project point onto perimeter */
			GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &x0, &y0);
			xr -= x0;	yr -= y0;
			r = hypot (xr, yr);
			scale = project_info.r / r;
			xr *= scale;
			yr *= scale;
			xr += x0;	yr += y0;
		}
		xx[n] = xr;	yy[n] = yr;
		n++;
		if (n == n_alloc) {
			n_alloc += GMT_CHUNK;
			xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
			yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_radial_clip");
		}
	}
	
	xx = (double *) GMT_memory ((void *)xx, (size_t)n, sizeof (double), "GMT_radial_clip");
	yy = (double *) GMT_memory ((void *)yy, (size_t)n, sizeof (double), "GMT_radial_clip");
	*x = xx;
	*y = yy;

	return (n);
}

int GMT_lon_inside (double lon, double w, double e)
{
	
	while (lon < project_info.w) lon += 360.0;
	while (lon > project_info.e) lon -= 360.0;
	
	if (lon < w) return (FALSE);
	if (lon > e) return (FALSE);
	return (TRUE);
}

void GMT_ngeo_to_xy (double *lon, double *lat, int n)
{
	/* Converts lon/lat array and returns x,y in same array */
	int j;
	double this_x, this_y;
	
	for (j = 0; j < n; j++) {
		GMT_geo_to_xy (lon[j], lat[j], &this_x, &this_y);
		lon[j] = this_x;
		lat[j] = this_y;
	}
}

int GMT_geo_to_xy_line (double *lon, double *lat, int n)
{
	/* Traces the lon/lat array and returns x,y plus appropriate pen moves */
	int j, np, this, nx, sides[4], wrap = FALSE, ok = FALSE;
	double xlon[4], xlat[4], xx[4], yy[4];
	double this_x, this_y, last_x, last_y, dummy[4];
	
	while (n > GMT_n_alloc) GMT_get_plot_array ();
	
	np = 0;
	GMT_geo_to_xy (lon[0], lat[0], &last_x, &last_y);
	if (!GMT_map_outside (lon[0], lat[0])) {
		GMT_x_plot[0] = last_x;	 GMT_y_plot[0] = last_y;
		GMT_pen[np++] = 3;
	}
	for (j = 1; j < n; j++) {
		GMT_geo_to_xy (lon[j], lat[j], &this_x, &this_y);
		this = GMT_map_outside (lon[j], lat[j]);
		nx = 0;
		if (GMT_break_through (lon[j-1], lat[j-1], lon[j], lat[j]))	{ /* Crossed map boundary */
			nx = GMT_map_crossing (lon[j-1], lat[j-1], lon[j], lat[j], xlon, xlat, xx, yy, sides);
			ok = GMT_ok_xovers (nx, last_x, this_x, sides);
		}
		if (GMT_world_map) {
			wrap = (*GMT_wrap_around_check) (dummy, last_x, last_y, this_x, this_y, xx, yy, sides, &nx);
			ok = wrap;
		}
		if (nx == 1) {
			GMT_x_plot[np] = xx[0];	GMT_y_plot[np] = yy[0];
			GMT_pen[np++] = GMT_pen_status ();
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		else if (nx == 2 && ok) {
			GMT_x_plot[np] = xx[0];	GMT_y_plot[np] = yy[0];
			GMT_pen[np++] = (wrap) ? 2 : 3;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
			GMT_x_plot[np] = xx[1];	GMT_y_plot[np] = yy[1];
			GMT_pen[np++] = (wrap) ? 3 : 2;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		if (!this) {
			GMT_x_plot[np] = this_x; 	GMT_y_plot[np] = this_y;
			GMT_pen[np++] = 2;
			if (np == GMT_n_alloc) GMT_get_plot_array ();
		}
		last_x = this_x;	last_y = this_y;
	}
	if (np) GMT_pen[0] = 3;	/* Sanity override: Gotta start off with new start point */
	return (np);
}	

int GMT_ok_xovers (int nx, double x0, double x1, int *sides)
{
	if (!MAPPING) return (TRUE);	/* Data is not periodic*/
	if (GMT_world_map || nx < 2) return (TRUE);
	if ((sides[0] + sides[1]) == 2) return (TRUE);	/* Crossing in the n-s direction */
	if (fabs (fabs (x0 - x1) - GMT_map_width) < SMALL) return (TRUE);
	if ((sides[0] + sides[1]) != 4) return (TRUE);	/* Not Crossing in the e-w direction */
	return (FALSE);
}	

int GMT_compact_line (double *x, double *y, int n, BOOLEAN pen_flag, int *pen)
{	/* TRUE if pen movements is present */
	/* GMT_compact_line will remove unnecessary points in paths */
	int i, j;
	double old_slope, new_slope, dx;
	char *flag;
	
	if (n < 3) return (n);
	flag = (char *) GMT_memory (VNULL, (size_t)n, sizeof (char), "GMT_compact_line");
	
	dx = x[1] - x[0];
	old_slope = (fabs (dx) < GMT_CONV_LIMIT) ? copysign (HALF_DBL_MAX, y[1] - y[0]) : (y[1] - y[0]) / dx;
	
	for (i = 1; i < n-1; i++) {
		dx = x[i+1] - x[i];
		new_slope = (fabs (dx) < GMT_CONV_LIMIT) ? copysign (HALF_DBL_MAX, y[i+1] - y[i]) : (y[i+1] - y[i]) / dx;
		if (fabs (new_slope - old_slope) < GMT_CONV_LIMIT && !(pen_flag && (pen[i]+pen[i+1]) > 4))
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

void GMT_grdproject_init (struct GRD_HEADER *head, double x_inc, double y_inc, int nx, int ny, int dpi, int offset)
{
	int one;
	
	one = (offset) ? 0 : 1;
	
	if (x_inc > 0.0 && y_inc > 0.0) {
		head->nx = irint ((head->x_max - head->x_min) / x_inc) + one;
		head->ny = irint ((head->y_max - head->y_min) / y_inc) + one;
		head->x_inc = (head->x_max - head->x_min) / (head->nx - one);
		head->y_inc = (head->y_max - head->y_min) / (head->ny - one);
	}
	else if (nx > 0 && ny > 0) {
		head->nx = nx;	head->ny = ny;
		head->x_inc = (head->x_max - head->x_min) / (head->nx - one);
		head->y_inc = (head->y_max - head->y_min) / (head->ny - one);
	}
	else if (dpi > 0) {
		head->nx = irint ((head->x_max - head->x_min) * dpi) + one;
		head->ny = irint ((head->y_max - head->y_min) * dpi) + one;
		head->x_inc = (head->x_max - head->x_min) / (head->nx - one);
		head->y_inc = (head->y_max - head->y_min) / (head->ny- one);
	}
	else {
		fprintf (stderr, "GMT_grdproject_init: Necessary arguments not set\n");
		exit (EXIT_FAILURE);
	}
	head->node_offset = offset;
	
	GMT_grd_RI_verify (head, 1);

	if (gmtdefs.verbose) fprintf (stderr, "%s: New grid size (nx,ny) %d by %d\n", GMT_program, head->nx, head->ny);
}

void GMT_init_search_radius (double *radius, struct GRD_HEADER *r_head, struct GRD_HEADER *g_head, BOOLEAN inverse) {
	double dx, dy, r;
	
	if (fabs (*radius) < GMT_CONV_LIMIT) {	/* Make sensible default for search radius */
		dx = 2.0 * (r_head->x_max - r_head->x_min) / (g_head->nx);
		dy = 2.0 * (r_head->y_max - r_head->y_min) / (g_head->ny);
		if (dx < r_head->x_inc) dx = r_head->x_inc;
		if (dy < r_head->y_inc) dy = r_head->y_inc;
		*radius = MAX (dx, dy);
	}
	if (gmtdefs.verbose && !(project_info.projection == MERCATOR && g_head->nx == r_head->nx)) {
		if (MAPPING) {	/* A geographic projection */
			if (inverse) {	/* Converting back to a geographical grid, so radius is in distance units */
				r = (*radius) * GMT_u2u[GMT_INCH][GMT_M];	/* Get meters */
				if (r > 1000.0) 
					fprintf (stderr, "%s: Search radius for interpolation is %g km\n", GMT_program, 0.001 * r);
				else
					fprintf (stderr, "%s: Search radius for interpolation is %g m\n", GMT_program, r);
			}
			else {
				r = (*radius) * 60.0;	/* Get minutes */
				if (r > 60.0) 
					fprintf (stderr, "%s: Search radius for interpolation is %g degrees\n", GMT_program, (*radius));
				else
					fprintf (stderr, "%s: Search radius for interpolation is %g minutes\n", GMT_program, r);
			}
		}
		else	/* Just linear, use given units */
			fprintf (stderr, "%s: Search radius for interpolation is %g\n", GMT_program, *radius);
	}
}

void GMT_grd_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head, double max_radius)
{	/* Forward projection from geographical to rectangular grid */
	int i, j, k, ij, ii, jj, i_r, j_r, nm, di, dj, not_used = 0;
	float *weight_sum;
	double dr, x_0, y_0, *x, *y, *lon, lat, delta, weight;
	double dx2 = 0.0, dy2 = 0.0, xinc2 = 0.0, yinc2 = 0.0, i_max_3r, idx, idy;
	
	if (project_info.projection == LINEAR && (project_info.xyz_projection[0] != LINEAR && g_head->ny == r_head->ny) && (project_info.xyz_projection[1] != LINEAR && g_head->nx == r_head->nx)) {
		GMT_transx_forward (geo, g_head, rect, r_head);						/* First transform the rows */
		memcpy ((void *)geo, (void *)rect, (size_t)(g_head->ny * g_head->nx * sizeof (float)));	/* Then store intermediate result back in geo */
		GMT_transy_forward (geo, g_head, rect, r_head);						/* And finally transform the columns */
		return;
	}
	if (project_info.projection == LINEAR && project_info.xyz_projection[0] != LINEAR && g_head->ny == r_head->ny) {
		GMT_transx_forward (geo, g_head, rect, r_head);
		return;
	}
	if (project_info.projection == LINEAR && project_info.xyz_projection[1] != LINEAR && g_head->nx == r_head->nx) {
		GMT_transy_forward (geo, g_head, rect, r_head);
		return;
	}
	if (project_info.projection == MERCATOR && g_head->nx == r_head->nx) {
		GMT_merc_forward (geo, g_head, rect, r_head);
		return;
	}
	
	if (fabs (max_radius) < GMT_CONV_LIMIT) {	/* Must pass non-zero radius */
		fprintf (stderr, "%s: Search-radius not initialized\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	/* All internal parameters related to projected values MUST be in GMT inches for this logic */

	nm = r_head->nx * r_head->ny;
	weight_sum = (float *) GMT_memory (VNULL, (size_t)nm, sizeof (float), "GMT_grd_forward");
		
	di = (int)ceil (max_radius / r_head->x_inc);
	dj = (int)ceil (max_radius / r_head->y_inc);
	
	if (g_head->node_offset) {
		dx2 = 0.5 * g_head->x_inc;
		dy2 = 0.5 * g_head->y_inc;
	}
	if (r_head->node_offset) {
		xinc2 = 0.5 * r_head->x_inc;
		yinc2 = 0.5 * r_head->y_inc;
	}
	i_max_3r = 3.0 / max_radius;
	idx = 1.0 / r_head->x_inc;
	idy = 1.0 / r_head->y_inc;
	lon = (double *) GMT_memory (VNULL, (size_t)g_head->nx, sizeof (double), "GMT_grd_forward");
	for (i = 0; i < g_head->nx; i++) {
		lon[i] = g_head->x_min + i * g_head->x_inc + dx2;
		if (lon[i] < project_info.w && (lon[i] + 360.0) <= project_info.e) lon[i] += 360.0;
		if (lon[i] > project_info.e && (lon[i] - 360.0) >= project_info.w) lon[i] -= 360.0;
	}
	x = (double *) GMT_memory (VNULL, (size_t)r_head->nx, sizeof (double), "GMT_grd_forward");
	y = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_grd_forward");
	for (i = 0; i < r_head->nx; i++) x[i] = r_head->x_min + i * r_head->x_inc + xinc2;
	for (j = 0; j < r_head->ny; j++) y[j] = r_head->y_max - j * r_head->y_inc - yinc2;
	
	for (j = ij = 0; j < g_head->ny; j++) {
		lat = g_head->y_max - j * g_head->y_inc - dy2;
		if (project_info.projection == MERCATOR && fabs (lat) >= 90.0) lat = copysign (89.99, lat);
		for (i = 0; i < g_head->nx; i++, ij++) {
			if (GMT_is_fnan (geo[ij])) continue;
				
			if (GMT_map_outside (lon[i], lat)) continue;
			GMT_geo_to_xy (lon[i], lat, &x_0, &y_0);
			if (r_head->node_offset) {
				ii = (fabs (x_0 - r_head->x_max) < GMT_CONV_LIMIT) ? r_head->nx - 1 : (int)floor ( (x_0 - r_head->x_min) * idx);
				jj = (fabs (y_0 - r_head->y_min) < GMT_CONV_LIMIT) ? r_head->ny - 1 : (int)floor ( (r_head->y_max - y_0) * idy);
			}
			else {
				ii = irint ( (x_0 - r_head->x_min) * idx);
				jj = irint ( (r_head->y_max - y_0) * idy);
			}
				
			for (j_r = jj - dj; j_r <= (jj + dj); j_r++) {
				if (j_r < 0 || j_r >= r_head->ny) continue;
				for (i_r = ii - di; i_r <= (ii + di); i_r++) {
					if (i_r < 0 || i_r >= r_head->nx) continue;
					k = j_r * r_head->nx + i_r;
					dr = hypot (x[i_r] - x_0, y[j_r] - y_0);
					if (dr > max_radius) continue;
					delta = dr * i_max_3r;
					weight = 1.0 / (1.0 + delta * delta);
					rect[k] += (float)(weight * geo[ij]);
					weight_sum[k] += (float)weight;
				}
			}
		}
	}
	r_head->z_min = DBL_MAX;	r_head->z_max = -DBL_MAX;
	for (k = 0; k < nm; k++) {
		if (weight_sum[k] > 0.0) {
			rect[k] /= weight_sum[k];
			r_head->z_min = MIN (r_head->z_min, rect[k]);
			r_head->z_max = MAX (r_head->z_max, rect[k]);
		}
		else {
			not_used++;
			rect[k] = GMT_f_NaN;
		}
	}
		
	GMT_free ((void *)weight_sum);
	GMT_free ((void *)lon);
	GMT_free ((void *)x);
	GMT_free ((void *)y);
	
	if (gmtdefs.verbose && not_used) fprintf (stderr, "GMT_grd_forward: some projected nodes not loaded (%d)\n", not_used);
}

void GMT_grd_inverse (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head, double max_radius)
{	/* Transforming from rectangular projection to geographical */
	int i, j, k, ij, ii, jj, i_r, j_r, nm, di, dj, not_used = 0;
	BOOLEAN greenwich;
	float *weight_sum;
	double dr, lat_0, lon_0, *x_0, y_0, *lon, *lat, dlon, x, y, delta, weight;
	double dx2 = 0.0, dy2 = 0.0, xinc2 = 0.0, yinc2 = 0.0, i_max_3r, idx, idy;
	
	if (project_info.projection == MERCATOR && g_head->nx == r_head->nx) {
		GMT_merc_inverse (geo, g_head, rect, r_head);
		return;
	}
	
	if (fabs (max_radius) < GMT_CONV_LIMIT) {	/* Must pass non-zero radius */
		fprintf (stderr, "%s: Search-radius not initialized\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	nm = g_head->nx * g_head->ny;

	/* All internal parameters related to projected values MUST be in GMT inches for this logic */
	
	weight_sum = (float *) GMT_memory (VNULL, (size_t)nm, sizeof (float), "GMT_grd_inverse");
	
	di = (int)ceil (max_radius / r_head->x_inc);
	dj = (int)ceil (max_radius / r_head->y_inc);
		
	if (g_head->node_offset) {
		dx2 = 0.5 * g_head->x_inc;
		dy2 = 0.5 * g_head->y_inc;
	}
	if (r_head->node_offset) {
		xinc2 = 0.5 * r_head->x_inc;
		yinc2 = 0.5 * r_head->y_inc;
	}
	i_max_3r = 3.0 / max_radius;
	idx = 1.0 / g_head->x_inc;
	idy = 1.0 / g_head->y_inc;

	lon = (double *) GMT_memory (VNULL, (size_t)g_head->nx, sizeof (double), "GMT_grd_inverse");
	lat = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_grd_inverse");
	for (i = 0; i < g_head->nx; i++) lon[i] = g_head->x_min + i * g_head->x_inc + dx2;
	for (j = 0; j < g_head->ny; j++) lat[j] = g_head->y_max - j * g_head->y_inc - dy2;
	x_0 = (double *) GMT_memory (VNULL, (size_t)r_head->nx, sizeof (double), "GMT_grd_inverse");
	for (i = 0; i < r_head->nx; i++) x_0[i] = r_head->x_min + i * r_head->x_inc + xinc2;
	greenwich = (g_head->x_min < 0.0);

	for (j = ij = 0; j < r_head->ny; j++) {
		y_0 = r_head->y_max - j * r_head->y_inc - yinc2;
		for (i = 0; i < r_head->nx; i++, ij++) {
			if (GMT_is_fnan (rect[ij])) continue;
				
			GMT_xy_to_geo (&lon_0, &lat_0, x_0[i], y_0);
			if (greenwich && lon_0 > 180.0) lon_0 -= 360.0;
			dlon = lon_0 - g_head->x_min;
			
			if (g_head->node_offset) {
				ii = (fabs (lon_0 - g_head->x_max) < GMT_CONV_LIMIT) ? g_head->nx - 1 : (int)floor (dlon * idx);
				jj = (fabs (lat_0 - g_head->y_min) < GMT_CONV_LIMIT) ? g_head->ny - 1 : (int)floor ( (g_head->y_max - lat_0) * idy);
			}
			else {
				ii = irint (dlon * idx);
				jj = irint ( (g_head->y_max - lat_0) * idy);
			}
				
			for (j_r = jj - dj; j_r <= (jj + dj); j_r++) {
				if (j_r < 0 || j_r >= g_head->ny) continue;
				for (i_r = ii - di; i_r <= (ii + di); i_r++) {
					if (i_r < 0 || i_r >= g_head->nx) continue;
					k = j_r * g_head->nx + i_r;
					GMT_geo_to_xy (lon[i_r], lat[j_r], &x, &y);
					dr = hypot (x - x_0[i], y - y_0);
					if (dr > max_radius) continue;
					delta = dr * i_max_3r;
					weight = 1.0 / (1.0 + delta * delta);
					geo[k] += (float)(weight * rect[ij]);
					weight_sum[k] += (float)weight;
				}
			}
		}
	}
	g_head->z_min = DBL_MAX;	g_head->z_max = -DBL_MAX;
	for (k = 0; k < nm; k++) {	/* Compute weighted average */
		if (weight_sum[k] > 0.0) {
			geo[k] /= weight_sum[k];
			g_head->z_min = MIN (g_head->z_min, geo[k]);
			g_head->z_max = MAX (g_head->z_max, geo[k]);
		}
		else {
			not_used++;
			geo[k] = GMT_f_NaN;
		}
	}
	
	GMT_free ((void *)weight_sum);
	GMT_free ((void *)lon);
	GMT_free ((void *)lat);
	GMT_free ((void *)x_0);
	
	if (gmtdefs.verbose && not_used) fprintf (stderr, "%s: Some geographical nodes not loaded (%d)\n", GMT_program, not_used);
}

int GMT_grd_project (float *z_in, struct GRD_HEADER *I, float *z_out, struct GRD_HEADER *O, struct GMT_EDGEINFO *edgeinfo, BOOLEAN bilinear, BOOLEAN inverse)
{

	/* Generalized grid projection that deals with both interpolation and averaging effects.
	 * It assumes that the incoming grid was read with 2 boundary rows/cols so that the bcr functions can be used.
	 * Therefore, we need to add these rows/cols when accessing the grid directly.  The I and z_in represents
	 * the input grid which is either in original (i.e., lon/lat) coordinates or projected x/y (if inverse = TRUE).
	 *
	 * z_in:	Array with input grid on a padded grid with 2 extra rows/columns
	 * I:		Grid header for input grid
	 * z_out:	Array with output grid, no padding needed
	 * O:		Grid header for output grid
	 * edgeinfo:	Structure with information about boundary conditions on input grid
	 * bilinear:	TRUE if we just want to use bilinear interpolation [Default is bicubic]
	 * inverse:	TRUE if input is x/y and we want to invert for a lon/lat grid
	 * 
	 * We assume the calling program has initialized the z_out array to any default empty value (NaN etc).
	 */
	
	int i_in, j_in, ij_in, i_out, j_out, ij_out, mx, my;
	short int *nz;
	double x_proj, y_proj, I_off, O_off, D_off, z_int, inv_nz, O_idx, O_idy;
	double *x_in, *y_in, *x_in_proj, *y_in_proj, *x_out, *y_out, *x_out_proj, *y_out_proj;
	struct GMT_BCR bcr;
	
	mx = I->nx + 4;
	my = I->ny + 4;

	GMT_boundcond_param_prep (I, edgeinfo);

	/* Initialize bcr structure:  */

	GMT_bcr_init (I, GMT_pad, bilinear, 1.0, &bcr);

	/* Set boundary conditions  */
	
	GMT_boundcond_set (I, edgeinfo, GMT_pad, z_in);

	nz = (short int *) GMT_memory (VNULL, (size_t)(O->nx * O->ny), sizeof (short int), "GMT_grd_forward");
	x_in = (double *) GMT_memory (VNULL, (size_t)I->nx, sizeof (double), "GMT_grd_forward");
	y_in = (double *) GMT_memory (VNULL, (size_t)I->ny, sizeof (double), "GMT_grd_forward");
	x_out = (double *) GMT_memory (VNULL, (size_t)O->nx, sizeof (double), "GMT_grd_forward");
	y_out = (double *) GMT_memory (VNULL, (size_t)O->ny, sizeof (double), "GMT_grd_forward");
	
	I_off = (I->node_offset)  ? 0.5 : 0.0;
	O_off = (O->node_offset) ? 0.5 : 0.0;
	D_off = (!O->node_offset)  ? 0.5 : 0.0;
	O_idx = 1.0 / O->x_inc;
	O_idy = 1.0 / O->y_inc;
	
	/* Precalculate grid coordinates */
	
	for (i_in = 0; i_in < I->nx; i_in++) x_in[i_in] = GMT_i_to_x (i_in, I->x_min, I->x_max, I->x_inc, I_off, I->nx);
	for (j_in = 0; j_in < I->ny; j_in++) y_in[j_in] = GMT_j_to_y (j_in, I->y_min, I->y_max, I->y_inc, I_off, I->ny);
	for (i_out = 0; i_out < O->nx; i_out++) x_out[i_out] = GMT_i_to_x (i_out, O->x_min, O->x_max, O->x_inc, O_off, O->nx);
	for (j_out = 0; j_out < O->ny; j_out++) y_out[j_out] = GMT_j_to_y (j_out, O->y_min, O->y_max, O->y_inc, O_off, O->ny);
	
	if (RECT_GRATICULE) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj = (double *) GMT_memory (VNULL, (size_t)I->nx, sizeof (double), "GMT_grd_forward");
		y_in_proj = (double *) GMT_memory (VNULL, (size_t)I->ny, sizeof (double), "GMT_grd_forward");
		x_out_proj = (double *) GMT_memory (VNULL, (size_t)O->nx, sizeof (double), "GMT_grd_forward");
		y_out_proj = (double *) GMT_memory (VNULL, (size_t)O->ny, sizeof (double), "GMT_grd_forward");
		if (inverse) {
			for (i_in = 0; i_in < I->nx; i_in++) GMT_xy_to_geo (&x_in_proj[i_in], &y_proj, x_in[i_in], I->y_min);
			for (j_in = 0; j_in < I->ny; j_in++) GMT_xy_to_geo (&x_proj, &y_in_proj[j_in], I->x_min, y_in[j_in]);
			for (i_out = 0; i_out < O->nx; i_out++) GMT_geo_to_xy (x_out[i_out], I->y_min, &x_out_proj[i_out], &y_proj);
			for (j_out = 0; j_out < O->ny; j_out++) GMT_geo_to_xy (I->y_min, y_out[j_out], &x_proj, &y_out_proj[j_out]);
		}
		else {
			for (i_in = 0; i_in < I->nx; i_in++) GMT_geo_to_xy (x_in[i_in], I->y_min, &x_in_proj[i_in], &y_proj);
			for (j_in = 0; j_in < I->ny; j_in++) GMT_geo_to_xy (I->x_min, y_in[j_in], &x_proj, &y_in_proj[j_in]);
			for (i_out = 0; i_out < O->nx; i_out++) GMT_xy_to_geo (&x_out_proj[i_out], &y_proj, x_out[i_out], I->y_min);
			for (j_out = 0; j_out < O->ny; j_out++) GMT_xy_to_geo (&x_proj, &y_out_proj[j_out], I->y_min, y_out[j_out]);
		}
	}
	
	/* PART 1: Project input grid points and do a blockmean operation */
	
	for (j_in = 0; j_in < I->ny; j_in++) {	/* Loop over the input grid coordinates */
		if (RECT_GRATICULE) y_proj = y_in_proj[j_in];
		for (i_in = 0; i_in < I->nx; i_in++) {
			if (RECT_GRATICULE)
				x_proj = x_in_proj[i_in];
			else if (inverse)
				GMT_xy_to_geo (&x_proj, &y_proj, x_in[i_in], y_in[j_in]);
			else
				GMT_geo_to_xy (x_in[i_in], y_in[j_in], &x_proj, &y_proj);
				
			/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */
			
			j_out = GMT_y_to_j (y_proj, O->y_min, O_idy, D_off, O->ny);
			if (j_out < 0 || j_out >= O->ny) continue;	/* Outside our grid region */
			i_out = GMT_x_to_i (x_proj, O->x_min, O_idx, D_off, O->nx);
			if (i_out < 0 || i_out >= O->nx) continue;	/* Outside our grid region */

			/* OK, this projected point falls inside the projected grid's rectangular domain */
			
			ij_in = (j_in + 2) * mx + i_in + 2;		/* ij allowing for boundary padding */
			ij_out = j_out * O->nx + i_out;			/* The output node */
			if (nz[ij_out] == 0) z_out[ij_out] = 0.0;	/* First time, override the NaN */
			z_out[ij_out] += z_in[ij_in];			/* Add up the z-sum inside this rect... */
			nz[ij_out]++;					/* ..and how many points there were */
			if (nz[ij_out] == SHRT_MAX) {			/* This bin is getting way to many points... */
				fprintf (stderr, "%s: ERROR: Number of projected points inside a bin exceeds %d\n", GMT_program, SHRT_MAX);
				fprintf (stderr, "%s: ERROR: Incorrect -R -I -J or insanely large grid?\n", GMT_program);
				exit (EXIT_FAILURE);
			}
		}
	}
	
	/* PART 2: Create weighted average of interpolated and observed points */
	
	for (j_out = ij_out = 0; j_out < O->ny; j_out++) {	/* Loop over the output grid coordinates */
		if (RECT_GRATICULE) y_proj = y_out_proj[j_out];
		for (i_out = 0; i_out < O->nx; i_out++, ij_out++) {
			if (RECT_GRATICULE)
				x_proj = x_out_proj[i_out];
			else if (inverse)
				GMT_geo_to_xy (x_out[i_out], y_out[j_out], &x_proj, &y_proj);
			else
				GMT_xy_to_geo (&x_proj, &y_proj, x_out[i_out], y_out[j_out]);
			
			if (GMT_io.in_col_type[0] == GMT_IS_LON) {
				if (x_proj < I->x_min) x_proj += 360.0;
				if (x_proj > I->x_max) x_proj -= 360.0;
			}
			
			if (i_out == (O->nx - 1))
				i_in = 1;
				
			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now find nearest node on the input grid */
			
			z_int = GMT_get_bcr_z (I, x_proj, y_proj, z_in, edgeinfo, &bcr);
			
			if (nz[ij_out] == 0)
				z_out[ij_out] = (float)z_int;
			else {
				if (GMT_is_dnan (z_int))
					z_out[ij_out] /= nz[ij_out];		/* Plain average */
				else {						/* Weighted average */
					inv_nz = 1.0 / nz[ij_out];
					z_out[ij_out] = (float) ((z_out[ij_out] + z_int * inv_nz) / (nz[ij_out] + inv_nz));
				}
			}
		}
	}
	
	/* Time to clean up our mess */
	
	GMT_free ((void *)nz);		
	GMT_free ((void *)x_in);		
	GMT_free ((void *)y_in);		
	GMT_free ((void *)x_out);		
	GMT_free ((void *)y_out);		
	if (RECT_GRATICULE) {
		GMT_free ((void *)x_in_proj);		
		GMT_free ((void *)y_in_proj);		
		GMT_free ((void *)x_out_proj);		
		GMT_free ((void *)y_out_proj);		
	}
	
	return (FALSE);
}

void GMT_merc_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head)
{	/* Forward projection from geographical to mercator grid */
	int i, j, g_last, r_last;
	double dy, y, dummy, *lat_in, *lat_out, *hold, *value;
	

	lat_in = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_merc_forward");
	lat_out = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_merc_forward");
	value = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_merc_forward");
	hold = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_merc_forward");
	
	g_last = g_head->ny - 1;
	r_last = r_head->ny - 1;
	dy = (g_head->node_offset) ? 0.5 * g_head->y_inc : 0.0;
	for (j = 0; j < g_head->ny; j++) lat_in[j] = g_head->y_min + j * g_head->y_inc + dy;
	
	dy = (r_head->node_offset) ? 0.5 * r_head->y_inc : 0.0;

	for (j = 0; j < r_head->ny; j++) { /* Construct merc y-grid */
		y = r_head->y_min + j * r_head->y_inc + dy;
		GMT_xy_to_geo (&dummy, &lat_out[j], 0.0, y);
	}
	
	/* Make sure new nodes outside border are set to be on border (pixel grid only) */
	
	j = 0;
	while (j < r_head->ny && (lat_out[j] - lat_in[0]) < 0.0) lat_out[j++] = lat_in[0];
	j = r_head->ny-1;
	while (j >= 0 && (lat_out[j] - lat_in[g_last]) > 0.0) lat_out[j--] = lat_in[g_last];
	for (i = 0; i < r_head->nx; i++) {	/* r_head->nx must == g_head->nx */
		for (j = 0; j < g_head->ny; j++) hold[g_last-j] = (double)geo[j*g_head->nx+i];	/* Copy and reverse a column */
		GMT_intpol (lat_in, hold, g_head->ny, r_head->ny, lat_out, value, gmtdefs.interpolant);
		for (j = 0; j < r_head->ny; j++) rect[j*r_head->nx+i] = (float)value[r_last-j];	/* Reverse and load new column */
	}
	GMT_free ((void *)lat_in);
	GMT_free ((void *)lat_out);
	GMT_free ((void *)value);
	GMT_free ((void *)hold);
}

void GMT_merc_inverse (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head)
{	/* Inverse projection from mercator to geographical grid */
	int i, j, g_last, r_last;
	double dy, y, dummy, *lat_in, *lat_out, *tmp, *val;

	/* Expects uncentered GMT INCHES for this logic to work */
	
	lat_in = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_merc_inverse");
	lat_out = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_merc_inverse");
	tmp = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_merc_inverse");
	val = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_merc_inverse");
	
	g_last = g_head->ny - 1;
	r_last = r_head->ny - 1;
	dy = (g_head->node_offset) ? 0.5 * g_head->y_inc : 0.0;
	for (j = 0; j < g_head->ny; j++) lat_in[j] = g_head->y_min + j * g_head->y_inc + dy;
	
	dy = (r_head->node_offset) ? 0.5 * r_head->y_inc : 0.0;
	for (j = 0; j < r_head->ny; j++) { /* Construct merc y-grid */
		y = r_head->y_min + j * r_head->y_inc + dy;
		GMT_xy_to_geo (&dummy, &lat_out[j], 0.0, y);
	}
	
	/* Make sure new nodes outside border are set to be on border (pixel grid only) */
	
	j = 0;
	while (j < g_head->ny && (lat_in[j] - lat_out[0]) < 0.0) lat_in[j++] = lat_out[0];
	j = g_head->ny-1;
	
	while (j >= 0 && (lat_in[j] - lat_out[r_last]) > 0.0) lat_in[j--] = lat_out[r_last];
	
	for (i = 0; i < g_head->nx; i++) {	/* r_head->nx must == g_head->nx */
		for (j = 0; j < r_head->ny; j++) val[r_last-j] = (double)rect[j*r_head->nx+i];	/* Copy and reverse a column */
		GMT_intpol (lat_out, val, r_head->ny, g_head->ny, lat_in, tmp, gmtdefs.interpolant);
		for (j = 0; j < g_head->ny; j++) geo[j*g_head->nx+i] = (float)tmp[g_last-j];	/* Copy and reverse a column */
	}
	GMT_free ((void *)lat_in);
	GMT_free ((void *)lat_out);
	GMT_free ((void *)val);
	GMT_free ((void *)tmp);
}

void GMT_transx_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head)
{	/* Forward projection from linear in x to log10 or pow */
	int i, ii, j, g_last, r_last;
	double dx, x, *x_in, *x_out, *hold, *value;
	

	x_in  = (double *) GMT_memory (VNULL, (size_t)g_head->nx, sizeof (double), "GMT_transx_forward");
	x_out = (double *) GMT_memory (VNULL, (size_t)r_head->nx, sizeof (double), "GMT_transx_forward");
	value = (double *) GMT_memory (VNULL, (size_t)r_head->nx, sizeof (double), "GMT_transx_forward");
	hold  = (double *) GMT_memory (VNULL, (size_t)g_head->nx, sizeof (double), "GMT_transx_forward");
	
	g_last = g_head->nx - 1;
	r_last = r_head->nx - 1;
	dx = (g_head->node_offset) ? 0.5 * g_head->x_inc : 0.0;
	for (i = 0; i < g_head->nx; i++) x_in[i] = g_head->x_min + i * g_head->x_inc + dx;
	
	dx = (r_head->node_offset) ? 0.5 * r_head->x_inc : 0.0;

	for (i = 0; i < r_head->nx; i++) { /* Construct log10 or pow x-grid */
		x = r_head->x_min + i * r_head->x_inc + dx;
		GMT_xx_to_x (&x_out[i], x);
	}
	
	/* Make sure new nodes outside border are set to be on border (pixel grid only) */
	
	i = 0;
	while (i < r_head->nx && (x_out[i] - x_in[0]) < 0.0) x_out[i++] = x_in[0];
	i = r_head->nx - 1;
	while (i >= 0 && (x_out[i] - x_in[g_last]) > 0.0) x_out[i--] = x_in[g_last];
	for (j = 0; j < r_head->ny; j++) {	/* r_head->ny must == g_head->ny */
		for (i = 0; i < g_head->nx; i++) hold[i] = (double)geo[j*g_head->nx+i];		/* Copy a row */
		GMT_intpol (x_in, hold, g_head->nx, r_head->nx, x_out, value, gmtdefs.interpolant);
		for (i = 0; i < r_head->nx; i++) {
			ii = (project_info.xyz_pos[0]) ? i : r_last - i;
			rect[j*r_head->nx+i] = (float)value[ii];	/* Load new row */
		}
	}
	GMT_free ((void *)x_in);
	GMT_free ((void *)x_out);
	GMT_free ((void *)value);
	GMT_free ((void *)hold);
}


void GMT_transy_forward (float *geo, struct GRD_HEADER *g_head, float *rect, struct GRD_HEADER *r_head)
{	/* Forward projection from linear in y to log10 or pow */
	int i, j, jj, g_last, r_last;
	double dy, y, *y_in, *y_out, *hold, *value;
	

	y_in  = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_transy_forward");
	y_out = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_transy_forward");
	value = (double *) GMT_memory (VNULL, (size_t)r_head->ny, sizeof (double), "GMT_transy_forward");
	hold  = (double *) GMT_memory (VNULL, (size_t)g_head->ny, sizeof (double), "GMT_transy_forward");
	
	g_last = g_head->ny - 1;
	r_last = r_head->ny - 1;
	dy = (g_head->node_offset) ? 0.5 * g_head->y_inc : 0.0;
	for (j = 0; j < g_head->ny; j++) y_in[j] = g_head->y_min + j * g_head->y_inc + dy;
	
	dy = (r_head->node_offset) ? 0.5 * r_head->y_inc : 0.0;

	for (j = 0; j < r_head->ny; j++) { /* Construct log10 or pow y-grid */
		y = r_head->y_min + j * r_head->y_inc + dy;
		GMT_yy_to_y (&y_out[j], y);
	}
	
	/* Make sure new nodes outside border are set to be on border (pixel grid only) */
	
	j = 0;
	while (j < r_head->ny && (y_out[j] - y_in[0]) < 0.0) y_out[j++] = y_in[0];
	j = r_head->ny-1;
	while (j >= 0 && (y_out[j] - y_in[g_last]) > 0.0) y_out[j--] = y_in[g_last];
	for (i = 0; i < r_head->nx; i++) {	/* r_head->nx must == g_head->nx */
		for (j = 0; j < g_head->ny; j++) hold[g_last-j] = (double)geo[j*g_head->nx+i];	/* Copy a column */
		GMT_intpol (y_in, hold, g_head->ny, r_head->ny, y_out, value, gmtdefs.interpolant);
		for (j = 0; j < r_head->ny; j++) {
			jj = (project_info.xyz_pos[1]) ? r_last - j : j;
			rect[j*r_head->nx+i] = (float)value[jj];	/* Load new column */
		}
	}
	GMT_free ((void *)y_in);
	GMT_free ((void *)y_out);
	GMT_free ((void *)value);
	GMT_free ((void *)hold);
}

void GMT_2D_to_3D (double *x, double *y, double z, int n)
{
	int i;
	
	/* Convert from two-D to three-D coordinates */
	
	for (i = 0; i < n; i++) GMT_xy_do_z_to_xy (x[i], y[i], z, &x[i], &y[i]);
}

void GMT_2Dz_to_3D (double *x, double *y, double z, int n)
{
	int i;
	
	/* Convert from two-D to three-D coordinates */
	
	for (i = 0; i < n; i++) GMT_xyz_to_xy (x[i], y[i], z, &x[i], &y[i]);
}

void GMT_azim_to_angle (double lon, double lat, double c, double azim, double *angle)
                         	/* All variables in degrees */
               {
	double lon1, lat1, x0, x1, y0, y1, dx, width, sinc, cosc, sinaz, cosaz, sinl, cosl;
	
	if (project_info.projection < MERCATOR) {	/* Trivial case */
		*angle = 90.0 - azim;
		return;
	}

	/* Find second point c spherical degrees away in the azim direction */
	
	GMT_geo_to_xy (lon, lat, &x0, &y0);

	azim  *= D2R;
	c   *= D2R;
	lat *= D2R;
	sincos (azim, &sinaz, &cosaz);
	sincos (c, &sinc, &cosc);
	sincos (lat, &sinl, &cosl);
	
	lon1 = lon + R2D * atan (sinc * sinaz / (cosl * cosc - sinl * sinc * cosaz));
	lat1 = R2D * d_asin (sinl * cosc + cosl * sinc * cosaz);

	/* Convert to x,y and get angle */
	
	GMT_geo_to_xy (lon1, lat1, &x1, &y1);
	
	/* Check for wrap-around */
	
	dx = x1 - x0;
	if (fabs (dx) > (width = GMT_half_map_width (y0))) {
		width *= 2.0;
		dx = copysign (width - fabs (dx), -dx);
		if (x1 < width)
			x0 -= width;
		else
			x0 += width;
	}
	*angle = d_atan2 (y1 - y0, x1 - x0) * R2D;
}

void GMT_check_R_J (double *clon)	/* Make sure -R and -J agree for global plots; J given priority */
              {
	double lon0;
	
	lon0 = 0.5 * (project_info.w + project_info.e);
	if (GMT_world_map && lon0 != *clon) {
		project_info.w = *clon - 180.0;
		project_info.e = *clon + 180.0;
		if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Central meridian set with -J (%g) implies -R%g/%g/%g/%g\n",
			GMT_program, *clon, project_info.w, project_info.e, project_info.s, project_info.n);
	}
	else if (!GMT_world_map) {
		lon0 = *clon - 360.0;
		while (lon0 < project_info.w) lon0 += 360.0;
		if (lon0 > project_info.e) {	/* Warn user*/
			if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Central meridian outside region\n", GMT_program);
		}
	}	
}

void GMT_set_spherical (void) {	/* Force spherical solution */
	gmtdefs.ellipsoid = N_ELLIPSOIDS - 1;	/* Use equatorial radius */
	project_info.EQ_RAD = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius;
	project_info.i_EQ_RAD = 1.0 / project_info.EQ_RAD;
	project_info.M_PR_DEG = TWO_PI * project_info.EQ_RAD / 360.0;
	project_info.ECC = project_info.ECC2 = project_info.ECC4 = project_info.ECC6 = 0.0;
	project_info.one_m_ECC2 = project_info.i_one_m_ECC2 = 1.0;
	project_info.half_ECC = project_info.i_half_ECC = 0.0;


	if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Spherical approximation used!\n", GMT_program);
}

void GMT_map_setinfo (double xmin, double xmax, double ymin, double ymax, double scl)
{	/* Set [and rescale] parameters */
	double factor = 1.0, w, h;

	w = (xmax - xmin) * project_info.x_scale;
	h = (ymax - ymin) * project_info.y_scale;
	
	if (project_info.gave_map_width == 1) {		/* Must rescale to given width */
		factor = scl / w;
	}
	else if (project_info.gave_map_width == 2) {	/* Must rescale to given height */
		factor = scl / h;
	}
	else if (project_info.gave_map_width == 3) {	/* Must rescale to max dimension */
		factor = scl / MAX (w, h);
	}
	else if (project_info.gave_map_width == 4) {	/* Must rescale to min dimension */
		factor = scl / MIN (w, h);
	}
	project_info.x_scale *= factor;
	project_info.y_scale *= factor;
	project_info.w_r *= factor;
	
	GMT_map_setxy (xmin, xmax, ymin, ymax);
}

void GMT_map_setxy (double xmin, double xmax, double ymin, double ymax)
{	/* Set x/y parameters */
	 
	project_info.xmax = (xmax - xmin) * project_info.x_scale;
	project_info.ymax = (ymax - ymin) * project_info.y_scale;
	project_info.x0 = -xmin * project_info.x_scale;
	project_info.y0 = -ymin * project_info.y_scale;
}

int GMT_map_clip_path (double **x, double **y, BOOLEAN *donut)
{
	/* This function returns a clip path corresponding to the
	 * extent of the map.
	 */
	 
	double *work_x, *work_y, angle, da, r0, s, c, lon, lat;
	int i, j, np;
	
	*donut = FALSE;
	
	if (!project_info.region)	/* Rectangular map boundary */
		np = 4;
	else {
		switch (project_info.projection) {
			case LINEAR:
			case MERCATOR:
			case CYL_EQ:
			case CYL_EQDIST:
			case MILLER:
			case OBLIQUE_MERC:
				np = 4;
				break;
			case POLAR:
				*donut = (project_info.s > 0.0 && GMT_world_map);
				np = gmtdefs.n_lon_nodes + 1;
				if (project_info.s > 0.0)	/* Need inside circle segment */
					np *= 2;
				else if (!GMT_world_map)	/* Need to include origin */
					np++;
				break;
			case STEREO:
			case LAMBERT:
			case LAMB_AZ_EQ:
			case ORTHO:
			case GNOMONIC:
			case AZ_EQDIST:
			case ALBERS:
			case ECONIC:
			case GRINTEN:
				np = (project_info.polar && (project_info.s <= -90.0 || project_info.n >= 90.0)) ? gmtdefs.n_lon_nodes + 2: 2 * (gmtdefs.n_lon_nodes + 1);
				break;
			case MOLLWEIDE:
			case SINUSOIDAL:
			case ROBINSON:
				np = 2 * gmtdefs.n_lat_nodes + 2;
				break;
			case WINKEL:
			case HAMMER:
			case ECKERT4:
			case ECKERT6:
				np = 2 * gmtdefs.n_lat_nodes + 2;
				if (project_info.s != -90.0) np += gmtdefs.n_lon_nodes - 1;
				if (project_info.n != 90.0) np += gmtdefs.n_lon_nodes - 1;
				break;
			case TM:
			case UTM:
			case CASSINI:
				np = 2 * (gmtdefs.n_lon_nodes + gmtdefs.n_lat_nodes);
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
			case LINEAR:
			case MERCATOR:
			case CYL_EQ:
			case CYL_EQDIST:
			case MILLER:
			case OBLIQUE_MERC:
				work_x[0] = work_x[3] = project_info.xmin;	work_y[0] = work_y[1] = project_info.ymin;
				work_x[1] = work_x[2] = project_info.xmax;	work_y[2] = work_y[3] = project_info.ymax;
				break;
			case LAMBERT:
			case ALBERS:
			case ECONIC:
				for (i = j = 0; i <= gmtdefs.n_lon_nodes; i++, j++) {
					lon = (i == gmtdefs.n_lon_nodes) ? project_info.e : project_info.w + i * gmtdefs.dlon;
					GMT_geo_to_xy (lon, project_info.s, &work_x[j], &work_y[j]);
				}
				for (i = 0; i <= gmtdefs.n_lon_nodes; i++, j++) {
					lon = (i == gmtdefs.n_lon_nodes) ? project_info.w : project_info.e - i * gmtdefs.dlon;
					GMT_geo_to_xy (lon, project_info.n, &work_x[j], &work_y[j]);
				}
				break;
			case TM:
			case UTM:
			case CASSINI:
				for (i = j = 0; i < gmtdefs.n_lon_nodes; i++, j++)	/* South */
					GMT_geo_to_xy (project_info.w + i * gmtdefs.dlon, project_info.s, &work_x[j], &work_y[j]);
				for (i = 0; i < gmtdefs.n_lat_nodes; j++, i++)	/* East */
					GMT_geo_to_xy (project_info.e, project_info.s + i * gmtdefs.dlat, &work_x[j], &work_y[j]);
				for (i = 0; i < gmtdefs.n_lon_nodes; i++, j++)	/* North */
					GMT_geo_to_xy (project_info.e - i * gmtdefs.dlon, project_info.n, &work_x[j], &work_y[j]);
				for (i = 0; i < gmtdefs.n_lat_nodes; j++, i++)	/* West */
					GMT_geo_to_xy (project_info.w, project_info.n - i * gmtdefs.dlat, &work_x[j], &work_y[j]);
				break;
			case POLAR:
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
					da = fabs (project_info.e - project_info.w) / (gmtdefs.n_lon_nodes - 1);
					for (i = j = 0; i <= gmtdefs.n_lon_nodes; i++, j++)	/* Draw outer clippath */
						GMT_geo_to_xy (project_info.w + i * da, project_info.n, &work_x[j], &work_y[j]);
					for (i = gmtdefs.n_lon_nodes; project_info.s > 0.0 && i >= 0; i--, j++)	/* Draw inner clippath */
						GMT_geo_to_xy (project_info.w + i * da, project_info.s, &work_x[j], &work_y[j]);
					if (fabs (project_info.s) < GMT_CONV_LIMIT && !GMT_world_map)	/* Add origin */
						GMT_geo_to_xy (project_info.w, project_info.s, &work_x[j], &work_y[j]);
				}
				break;
			case LAMB_AZ_EQ:
			case ORTHO:
			case GNOMONIC:
			case AZ_EQDIST:
			case GRINTEN:
			case STEREO:
				if (project_info.polar) {
					j = 0;
					if (project_info.s > -90.0) {
						for (i = 0; i <= gmtdefs.n_lon_nodes; i++, j++) {
							lon = (i == gmtdefs.n_lon_nodes) ? project_info.e : project_info.w + i * gmtdefs.dlon;
							GMT_geo_to_xy (lon, project_info.s, &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add S pole */
						GMT_geo_to_xy (project_info.w, -90.0, &work_x[j], &work_y[j]);
						j++;
					}
					if (project_info.n < 90.0) {
						for (i = 0; i <= gmtdefs.n_lon_nodes; i++, j++) {
							lon = (i == gmtdefs.n_lon_nodes) ? project_info.w : project_info.e - i * gmtdefs.dlon;
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
			case MOLLWEIDE:
			case SINUSOIDAL:
			case ROBINSON:
				for (i = j = 0; i <= gmtdefs.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == gmtdefs.n_lat_nodes) ? project_info.n : project_info.s + i * gmtdefs.dlat;
					GMT_geo_to_xy (project_info.e, lat, &work_x[j], &work_y[j]);
				}
				for (i = gmtdefs.n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == gmtdefs.n_lat_nodes) ? project_info.n : project_info.s + i * gmtdefs.dlat;
					GMT_geo_to_xy (project_info.w, lat, &work_x[j], &work_y[j]);
				}
				break;
			case HAMMER:
			case WINKEL:
			case ECKERT4:
			case ECKERT6:
				for (i = j = 0; i <= gmtdefs.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == gmtdefs.n_lat_nodes) ? project_info.n : project_info.s + i * gmtdefs.dlat;
					GMT_geo_to_xy (project_info.e, lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; project_info.n != 90.0 && i < gmtdefs.n_lon_nodes; i++, j++)
					GMT_geo_to_xy (project_info.e - i * gmtdefs.dlon, project_info.n, &work_x[j], &work_y[j]);
				for (i = gmtdefs.n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == gmtdefs.n_lat_nodes) ? project_info.n : project_info.s + i * gmtdefs.dlat;
					GMT_geo_to_xy (project_info.w, lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; project_info.s != -90.0 && i < gmtdefs.n_lon_nodes; i++, j++)
					GMT_geo_to_xy (project_info.w + i * gmtdefs.dlon, project_info.s, &work_x[j], &work_y[j]);
				break;
		}
	}
	
	if (!(*donut)) np = GMT_compact_line (work_x, work_y, np, FALSE, (int *)0);
	if (project_info.three_D) GMT_2D_to_3D (work_x, work_y, project_info.z_level, np);
	
	*x = work_x;
	*y = work_y;
	
	return (np);
}

BOOLEAN GMT_quickconic (void)
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

BOOLEAN GMT_quicktm (double lon0, double limit)
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

	double	rl2, delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (fabs (lat) < GMT_CONV_LIMIT) return (0.0);

	rl2 = 2.0 * lat * D2R;

	sincos (rl2, &sin2phi, &cos2phi);

	delta = sin2phi * (c[0] + cos2phi * (c[1] + cos2phi * (c[2] + cos2phi * c[3] ) ) );

	return (lat + R2D * delta);
}

double	GMT_lat_swap (double lat, int itype)
{
	/* Return latitude, in degrees, given latitude, in degrees, based on itype */
	
	double	rl2, delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (fabs (lat) < GMT_CONV_LIMIT) return (0.0);

	if (GMT_lat_swap_vals.spherical) return (lat);
	
	if (itype < 0 || itype >= GMT_LATSWAP_N) {
		/* This should never happen -?- or do we want to allow the
			possibility of using itype = -1 to do nothing  */
		fprintf (stderr, "GMT_lat_swap():  Invalid choice.  (Programming bug.)\n");
		return(lat);
	}

	rl2 = 2.0 * lat * D2R;

	sincos (rl2, &sin2phi, &cos2phi);

	delta = sin2phi * (GMT_lat_swap_vals.c[itype][0] 
		+ cos2phi * (GMT_lat_swap_vals.c[itype][1] 
		+ cos2phi * (GMT_lat_swap_vals.c[itype][2] 
		+ cos2phi * GMT_lat_swap_vals.c[itype][3] ) ) );
	
	
	return (lat + R2D * delta);
}

void	GMT_lat_swap_init ()
{
	/* Initialize values in GMT_lat_swap_vals based on project_info.
	
	First compute GMT_lat_swap_vals.ra (and rm), the radii to use in
	spherical formulae for area (respectively, N-S distance) when
	using the authalic (respectively, meridional) latitude.  

	Then for each type of swap:
	First load GMT_lat_swap_vals.c[itype][k], k=0,1,2,3 with the
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
	geodetic <-> conformal:     max error (degrees) = 2.321796e-07
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
	int	i;
	
	f = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening;
	a = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius;

	if (fabs (f) < GMT_CONV_LIMIT) {
		memset ((void *)GMT_lat_swap_vals.c, 0, (size_t)(GMT_LATSWAP_N * 4 * sizeof (double)));
		GMT_lat_swap_vals.ra = GMT_lat_swap_vals.rm = a;
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
	x = xx[0] * e2 + ( xx[1] * e4 + ( xx[2] * e6 + xx[3] * e8) );
	GMT_lat_swap_vals.ra = a * sqrt( (1.0 + x) * (1.0 - e2) );

	/* This expression for the Meridional radius comes from Gradshteyn and Ryzhik, 8.114.1,
	because Adams only gets the first two terms.  This can be worked out by expressing the
	meridian arc length in terms of an integral in parametric latitude, which reduces to
	equatorial radius times Elliptic Integral of the Second Kind.  Expanding this using
	binomial theorem leads to Gradshteyn and Ryzhik's expression:  */
	xx[0] = 1.0 / 4.0;
	xx[1] = xx[0] * 3.0 / 16.0;
	xx[2] = xx[1] * 3.0 * 5.0 / 36.0;
	xx[3] = xx[2] * 5.0 * 7.0 / 64.0;
	x = xx[0] * e2 + ( xx[1] * e4 + ( xx[2] * e6 + xx[3] * e8) );
	GMT_lat_swap_vals.rm = a * (1.0 - x);


	/* Geodetic to authalic:  */	
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][0] = -(e2 / 3.0 + (31.0 * e4 / 180.0 + 59.0 * e6 / 560.0) );
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][1] = 17.0 * e4 / 360.0 + 61.0 * e6 / 1260;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][2] = -383.0 * e6 / 45360.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][3] = 0.0;
	
	/* Authalic to geodetic:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][0] = e2 / 3.0 + (31.0 * e4 / 180.0 + 517.0 * e6 / 5040.0);
	GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][1] = 23.0 * e4 / 360.0 + 251.0 * e6 / 3780;
	GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][2] = 761.0 * e6 / 45360.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][3] = 0.0;

	/* Geodetic to conformal:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][0] = -(e2 / 2.0 + (5.0 * e4 / 24.0 + (3.0 * e6 / 32.0 + 281.0 * e8 / 5760.0) ) );
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][1] = 5.0 * e4 / 48.0 + (7.0 * e6 / 80.0 + 697.0 * e8 / 11520.0);
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][2] = -(13.0 * e6 / 480.0 + 461.0 * e8 / 13440.0);
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][3] = 1237.0 * e8 / 161280.0;
	
	/* Conformal to geodetic:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][0] = e2 / 2.0 + (5.0 * e4 / 24.0 + (e6 / 12.0 + 13.0 * e8 / 360.0) ) ;
	GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][1] = 7.0 * e4 / 48.0 + (29.0 * e6 / 240.0 + 811.0 * e8 / 11520.0);
	GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][2] = 81.0 * e6 / 1120.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][3] = 4279.0 * e8 / 161280.0;
	

	/* The meridional and parametric developments use this parameter:  */
	x = f/(2.0 - f);		/* Adams calls this n.  It is f/(2-f), or -betaJK in my notes.  */
	xx[0] = x;			/* n  */
	xx[1] = x * x;			/* n-squared  */
	xx[2] = xx[1] * x;		/* n-cubed  */
	xx[3] = xx[2] * x;		/* n to the 4th  */
	
	/* Geodetic to meridional:  */	
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][0] = -(3.0 * xx[0] / 2.0 - 9.0 * xx[2] / 16.0);
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][1] = 15.0 * xx[1] / 16.0 - 15.0 * xx[3] / 32.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][2] = -35.0 * xx[2] / 48.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][3] = 315.0 * xx[3] / 512.0;

	/* Meridional to geodetic:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][0] = 3.0 * xx[0] / 2.0 - 27.0 * xx[2] / 32.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][1] = 21.0 * xx[1] / 16.0 - 55.0 * xx[3] / 32.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][2] = 151.0 * xx[2] / 96.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][3] = 1097.0 * xx[3] / 512.0;

	/* Geodetic to parametric equals parametric to geocentric:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][0] = GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][0] = -xx[0];
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][1] = GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][1] = xx[1] / 2.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][2] = GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][2] = -xx[2] / 3.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][3] = GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][3] = xx[3] / 4.0;
	
	/* Parametric to geodetic equals geocentric to parametric:  */
	GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][0] = GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][0] = xx[0];
	GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][1] = GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][1] = xx[1] / 2.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][2] = GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][2] = xx[2] / 3.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][3] = GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][3] = xx[3] / 4.0;
	

	/* The geodetic <->geocentric use this parameter:  */
	x = 1.0 - e2;
	x = (1.0 - x)/(1.0 + x);	/* Adams calls this m.  It is e2/(2-e2), or -betaJK in my notes.  */
	xx[0] = x;			/* m  */
	xx[1] = x * x;			/* m-squared  */
	xx[2] = xx[1] * x;		/* m-cubed  */
	xx[3] = xx[2] * x;		/* m to the 4th  */

	GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][0] = -xx[0];
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][1] = xx[1] / 2.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][2] = -xx[2] / 3.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][3] = xx[3] / 4.0;
	
	GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][0] = xx[0];
	GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][1] = xx[1] / 2.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][2] = xx[2] / 3.0;
	GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][3] = xx[3] / 4.0;
	
	
	/* Now do the Snyder Shuffle:  */
	for (i = 0; i < GMT_LATSWAP_N; i++) {
		GMT_lat_swap_vals.c[i][0] = GMT_lat_swap_vals.c[i][0] - GMT_lat_swap_vals.c[i][2];
		GMT_lat_swap_vals.c[i][1] = 2.0 * GMT_lat_swap_vals.c[i][1] - 4.0 * GMT_lat_swap_vals.c[i][3];
		GMT_lat_swap_vals.c[i][2] *= 4.0;	
		GMT_lat_swap_vals.c[i][3] *= 8.0;
	}

	return;
}

void GMT_scale_eqrad ()
{
	/* Reinitialize project_info.EQ_RAD to the appropriate value */

	switch (project_info.projection) {

		/* Conformal projections */

		case MERCATOR:
		case TM:
		case UTM:
		case OBLIQUE_MERC:
		case LAMBERT:
		case STEREO:

			project_info.EQ_RAD = GMT_lat_swap_vals.rm;
			break;

		/* Equal Area projections */

		case LAMB_AZ_EQ:
		/* case CYL_EQ: */
		case ALBERS:
		case ECKERT4:
		case ECKERT6:
		case HAMMER:
		case MOLLWEIDE:
		case SINUSOIDAL:

			project_info.EQ_RAD = GMT_lat_swap_vals.ra;
			break;

		default:	/* Keep EQ_RAD as is */
			break;
	}

	/* Also reset dependencies of EQ_RAD */

	project_info.i_EQ_RAD = 1.0 / project_info.EQ_RAD;
	project_info.M_PR_DEG = TWO_PI * project_info.EQ_RAD / 360.0;

}

void GMT_set_polar (double plat)
{
	/* Determines if the projection pole is N or S pole */

	if (fabs (fabs (plat) - 90.0) < GMT_CONV_LIMIT) {
		project_info.polar = TRUE;
		project_info.north_pole	= (plat > 0.0);
		project_info.n_polar = project_info.north_pole;
		project_info.s_polar = !project_info.n_polar;
	}
}

/* Datum conversion routines */

int GMT_datum_init (char *text)
{
	/* Decode -T option (in mapproject) and initialize datum conv structures */
	
	int k = 0;
	char from[128], to[128];
	
	if (text[0] == 'h') {	/* We will process lon, lat, height data */
		k = 1;
		GMT_datum.h_given = TRUE;	/* If FALSE we set height = 0 */
	}
	
	if (strchr (&text[k], '/')) {	/* Gave from/to */
		sscanf (&text[k], "%[^/]/%s", from, to);
	}
	else {	/* to not given, set to - which means WGS-84 */
		strcpy (to, "-");
		strcpy (from, &text[k]);
	}
	if (GMT_set_datum (to,   &GMT_datum.to)   == -1) return (-1);
	if (GMT_set_datum (from, &GMT_datum.from) == -1) return (-1);
	
	GMT_datum.da = GMT_datum.to.a - GMT_datum.from.a;
	GMT_datum.df = GMT_datum.to.f - GMT_datum.from.f;
	for (k = 0; k < 3; k++) GMT_datum.dxyz[k] = -(GMT_datum.to.xyz[k] - GMT_datum.from.xyz[k]);	/* Since the X, Y, Z are Deltas relative to WGS-84 */
	GMT_datum.one_minus_f = 1.0 - GMT_datum.from.f;
	return 0;
}

int GMT_ECEF_init (char *text)
{
	/* Decode -E option (in mapproject) and initialize ECEF conv structures */
	
	if (GMT_set_datum (text, &GMT_datum.from) == -1) return (-1);
	
	return 0;
}

int GMT_set_datum (char *text, struct GMT_DATUM *D)
{
	int i;
	double t;
	
	if (text[0] == '\0' || text[0] == '-') {	/* Shortcut for WGS-84 */
		memset ((void *)D->xyz, 0, (size_t)(3 * sizeof (double)));
		D->a = 6378137.0;
		D->f = (1.0 / 298.2572235630);
		D->ellipsoid_id = 0;
	}
	else if (strchr (text, ':')) {	/* Has colons, must get ellipsoid and dr separately */
		char ellipsoid[128], dr[64];
		if (sscanf (text, "%[^:]:%s", ellipsoid, dr) != 2) {
			fprintf (stderr, "%s: Malformed <ellipsoid>:<dr> argument!\n", GMT_program);
			return (-1);
		}
		if (sscanf (dr, "%lf,%lf,%lf", &D->xyz[0], &D->xyz[1], &D->xyz[2]) != 3) {
			fprintf (stderr, "%s: Malformed <x>,<y>,<z> argument!\n", GMT_program);
			return (-1);
		}
		if (strchr (ellipsoid, ',')) {	/* Has major, inv_f instead of name */
			if (sscanf (ellipsoid, "%lf,%lf", &D->a, &D->f) != 2) {
				fprintf (stderr, "%s: Malformed <a>,<1/f> argument!\n", GMT_program);
				return (-1);
			}
			if (D->f != 0.0) D->f = 1.0 / D->f;	/* Get f from 1/f */
			D->ellipsoid_id = -1;
		}
		else {	/* Get the ellipsoid # and then the parameters */
			if ((i = GMT_get_ellipsoid (ellipsoid)) < 0) {
				fprintf (stderr, "%s: Ellipsoid %s not recognized!\n", GMT_program, ellipsoid);
				return (-1);
			}
			D->a = gmtdefs.ref_ellipsoid[i].eq_radius;
			D->f = gmtdefs.ref_ellipsoid[i].flattening;
			D->ellipsoid_id = i;
		}
	}
	else {		/* Gave a Datum ID tag [ 0-(N_DATUMS-1)] */
		int k;
		if (sscanf (text, "%d", &i) != 1) {
			fprintf (stderr, "%s: Malformed or unrecognized <datum> argument (%s)!\n", GMT_program, text);
			return (-1);
		}
		if (i < 0 || i >= N_DATUMS) {
			fprintf (stderr, "%s: Datum ID (%d) outside valid range (0-%d)!\n", GMT_program, i, N_DATUMS-1);
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
	
	double sin_lon, cos_lon, sin_lat, cos_lat, sin_lat2, M, N, h, tmp_1, tmp_2, tmp_3;
	double delta_lat, delta_lon, delta_h, sc_lat;
	
	h = (GMT_datum.h_given) ? in[2] : 0.0;
	sincos (in[0] * D2R, &sin_lon, &cos_lon);
	sincos (in[1] * D2R, &sin_lat, &cos_lat);
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
	
	sincos (in[0] * D2R, &sin_lon, &cos_lon);
	sincos (in[1] * D2R, &sin_lat, &cos_lat);
	
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
	int i;
	
	/* First remove the xyz shifts, us in_p to avoid changing in */
	
	for (i = 0; i < 3; i++) in_p[i] = in[i] - GMT_datum.from.xyz[i];
	
	p = hypot (in_p[0], in_p[1]);
	theta = atan (in_p[2] * GMT_datum.from.a / (p * GMT_datum.from.b));
	sincos (theta, &sin_theta, &cos_theta);
	out[0] = d_atan2 (in_p[1], in_p[0]) * R2D;
	out[1] = atan ((in_p[2] + GMT_datum.from.ep_squared * GMT_datum.from.b * pow (sin_theta, 3.0)) / (p - GMT_datum.from.e_squared * GMT_datum.from.a * pow (cos_theta, 3.0)));
	sincos (out[1], &sin_lat, &cos_lat);
	out[1] *= R2D;
	N = GMT_datum.from.a / sqrt (1.0 - GMT_datum.from.e_squared * sin_lat * sin_lat);
	out[2] = (p / cos_lat) - N;
}

double GMT_az_backaz_sphere (double lonE, double latE, double lonS, double latS, BOOLEAN baz)
{
	/* Calculate azimuths or backazimuths.  Spherical code.
	 * First point is considered "Event" and second "Station". */
	
	double az, sin_yS, cos_yS, sin_yE, cos_yE, sin_dlon, cos_dlon;
	
	latE *= D2R;	lonE *= D2R;
        latS *= D2R;	lonS *= D2R;
	
	if (baz) {	/* exchange point one and two */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	sincos (latS, &sin_yS, &cos_yS);
	sincos (latE, &sin_yE, &cos_yE);
	sincos (lonS - lonE, &sin_dlon, &cos_dlon);
	az = (float)(atan2 (cos_yS * sin_dlon, cos_yE * sin_yS - sin_yE * cos_yS * cos_dlon) * R2D);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_geodesic (double lonE, double latE, double lonS, double latS, BOOLEAN baz)
{
	/* Calculate azimuths or backazimuths for geodesics using geocentric latitudes.
	 * First point is considered "Event" and second "Station". */
	
	double az, a, b, c, d, e, f, g, h, a1, b1, c1, d1, e1, f1, g1, h1, thg, ss, sc;
	
	latE *= D2R;	lonE *= D2R;
        latS *= D2R;	lonS *= D2R;
	
	/* (Equations are unstable for latidudes of exactly 0 degrees. */
	if (latE == 0.0) latE = 1.0e-08;
	if (latS == 0.0) latS = 1.0e-08;
	
	/* Must convert from geographic to geocentric coordinates in order
	 * to use the spherical trig equations.  This requires a latitude
	 * correction given by: 1-ECC2=1-2*f + f*f = project_info.one_m_ECC2
	 */

	thg = atan (project_info.one_m_ECC2 * tan (latE));
	sincos (lonE, &d, &e);	e = -e;
	sincos (thg, &c, &f);	f = -f;
	a = f * e;
	b = -f * d;
	g = -c * e;
	h = c * d;

	/* Calculating some trig constants. */

	thg = atan (project_info.one_m_ECC2 * tan(latS));
	sincos (lonS, &d1, &e1);	e1 = -e1;
	sincos (thg, &c1, &f1);		f1 = -f1;
	a1 = f1 * e1;
	b1 = -f1 * d1;
	g1 = -c1 * e1;
	h1 = c1 * d1;

	/* Spherical trig relationships used to compute angles. */

	if (baz) {	/* Get Backazimuth */
		ss = (pow(a-d1,2.0) + pow(b-e1,2.0) + c * c - 2.0);
		sc = (pow(a-g1,2.0) + pow(b-h1,2.0) + pow(c-f1,2.0) - 2.0);
	}
	else {		/* Get Azimuth */
		ss = (pow(a1-d, 2.0) + pow(b1-e, 2.0) + c1 * c1 - 2.0);
		sc = (pow(a1-g, 2.0) + pow(b1-h, 2.0) + pow(c1-f, 2.0) - 2.0);
	}
	az = atan2 (ss,sc) * R2D;
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_geodesic_dist_degree (double lonS, double latS, double lonE, double latE)
{
	/* Compute the great circle arc length in degrees on an ellipsoidal
	 * Earth.  We do this by converting to geocentric coordinates.
	 */

	double a, b, c, d, e, f, a1, b1, c1, d1, e1, f1;
	double thg, sc, sd, dist;
	
	/* Convert event location to radians.
	 * (Equations are unstable for latidudes of exactly 0 degrees.)
	 */

	if (latE == 0.0) latE = 1.0e-08;
	latE *= D2R;
	lonE *= D2R;

	/* Must convert from geographic to geocentric coordinates in order
	 * to use the spherical trig equations.  This requires a latitude
	 * correction given by: 1-ECC2=1-2*F + F*F = project_info.one_m_ECC2
	 */

	thg = atan (project_info.one_m_ECC2 * tan (latE));
	sincos (lonE, &d, &e);	e = -e;
	sincos (thg, &c, &f);	f = -f;
	a = f * e;
	b = -f * d;

	/* Convert to radians */

        if (latS == 0.0) latS = 1.0e-08;
        latS *= D2R;
        lonS *= D2R;

	/* Calculate some trig constants. */
	
        thg = atan (project_info.one_m_ECC2 * tan(latS));
	sincos (lonS, &d1, &e1);	e1 = -e1;
 	sincos (thg, &c1, &f1);	f1 = -f1;
        a1 = f1 * e1;
        b1 = -f1 * d1;
        sc = a * a1 + b * b1 + c * c1;

	/* Spherical trig relationships used to compute angles. */

	sd = 0.5 * sqrt ((pow(a-a1,2.0) + pow(b-b1,2.0) + pow(c-c1,2.0)) * (pow(a+a1,2.0) + pow(b+b1, 2.0) + pow(c+c1, 2.0)));
	dist = atan2 (sd, sc) * R2D;
	if (dist < 0.0) dist += 360.0;
	
	return (dist);
}

double GMT_geodesic_dist_meter (double lonS, double latS, double lonE, double latE)
{
	/* Compute length of geodesic between locations in meters
 	 * We use Rudoe's equation from Bomford.
	 */

	double t1, t2, p1, p2, e1, el, sinthi, costhi, sinthk, costhk, tanthi, tanthk, sina12, cosa12;
	double al, dl, a12top, a12bot, a12, e2, e3, c0, c2, c4, v1, v2, z1, z2, x2, y2, dist;
	double e1p1, sqrte1p1, sin_dl, cos_dl, u1bot, u1, u2top, u2bot, u2, b0, du, pdist;
	

	/* Convert event location to radians.
	 * (Equations are unstable for latidudes of exactly 0 degrees.)
	 */

	if (latE == 0.0) latE = 1.0e-08;
	latE *= D2R;
	lonE *= D2R;
        if (latS == 0.0) latS = 1.0e-08;
        latS *= D2R;
        lonS *= D2R;

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
		t1 = latS;
		p1 = lonS;
		t2 = latE;
		p2 = lonE;
	}
	else {
		t1 = latE;
		p1 = lonE;
		t2 = latS;
		p2 = lonS;
	}
	el = project_info.ECC2 / project_info.one_m_ECC2;
	e1 = 1.0 + el;
	sincos (t1, &sinthi, &costhi);
	sincos (t2, &sinthk, &costhk);
	tanthi = sinthi / costhi;
	tanthk = sinthk / costhk;
	al = tanthi / (e1 * tanthk) + project_info.ECC2 * sqrt((e1 + tanthi * tanthi) / (e1 + tanthk * tanthk));
	dl = p1 - p2;
	sincos (dl, &sin_dl, &cos_dl);
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
	pdist = b0 * (c2 * (sin (2.0 * u2) - sin(2.0 * u1)) + c4 * (sin (4.0 * u2) - sin (4.0 * u1)));
	dist = fabs (b0 * c0 * du + pdist);
	
	return (dist);
}

double GMT_geodesic_dist_km (double lonS, double latS, double lonE, double latE)
{
	return (0.001 * GMT_geodesic_dist_meter (lonS, latS, lonE, latE));
}

