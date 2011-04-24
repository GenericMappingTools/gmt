/*--------------------------------------------------------------------
 *	$Id: gmt_map.c,v 1.285 2011-04-24 20:47:41 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Usage: Initialize system by calling GMT_map_setup (separate module), and
 * then just use GMT_geo_to_xy() and GMT_xy_to_geo() functions.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
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
 *	GMT_map_path :		Return GMT_latpath or GMT_lonpath
 *	GMT_map_setup :		Initialize map projection
 *	GMT_xy_to_geo :		Generic inverse x/y to lon/lat projection
 *	GMT_xyz_to_xy :		Generic xyz to xy projection
 *	GMT_xyz_to_xy_n :	Same for an array
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
 *	GMT_wrap_around_check_{x,tm} :	Check if line wraps around due to Greenwich
 *	GMT_x_to_xx :			Generic linear x projection
 *	GMT_xx_to_x :			Generic inverse linear x projection
 *	GMT_y_to_yy :			Generic linear y projection
 *	GMT_yy_to_y :			Generic inverse linear y projection
 *	GMT_z_to_zz :			Generic linear z projection
 *	GMT_zz_to_z :			Generic inverse linear z projection
 */

#include "pslib.h"
#include "gmt.h"
#include "gmt_internals.h"

/* Private functions internal to gmt_map.c */

GMT_LONG GMT_quickconic (struct GMT_CTRL *C)
{	/* Returns TRUE if area/scale are large/small enough
	 * so that we can use spherical equations with authalic
	 * or conformal latitudes instead of the full ellipsoidal
	 * equations.
	 */

	double s, dlon, width;

	if (C->current.proj.gave_map_width) {	/* Gave width */
		dlon = C->common.R.wesn[XHI] - C->common.R.wesn[XLO];
		width = C->current.proj.pars[4] * C->session.u2u[C->current.setting.proj_length_unit][GMT_M];	/* Convert to meters */
		s = (dlon * C->current.proj.M_PR_DEG) / width;
	}
	else if (C->current.proj.units_pr_degree) {	/* Gave scale */
		/* Convert to meters */
		s = C->current.proj.M_PR_DEG / (C->current.proj.pars[4] * C->session.u2u[C->current.setting.proj_length_unit][GMT_M]);
	}
	else {	/* Got 1:xxx that was changed */
		s = (1.0 / C->current.proj.pars[4]) / C->current.proj.unit;
	}

	if (s > 1.0e7) {	/* if s in 1:s exceeds 1e7 we do the quick thing */
		GMT_report (C, GMT_MSG_NORMAL, "Warning: Using spherical projection with conformal latitudes\n");
		return (TRUE);
	}
	else /* Use full ellipsoidal terms */
		return (FALSE);
}

GMT_LONG GMT_quicktm (struct GMT_CTRL *C, double lon0, double limit)
{	/* Returns TRUE if the region chosen is too large for the
	 * ellipsoidal series to be valid; hence use spherical equations
	 * with authalic latitudes instead.
	 * We let +-limit degrees from central meridian be the cutoff.
	 */

	double d_left, d_right;

	d_left  = lon0 - C->common.R.wesn[XLO] - 360.0;
	d_right = lon0 - C->common.R.wesn[XHI] - 360.0;
	while (d_left  < -180.0) d_left  += 360.0;
	while (d_right < -180.0) d_right += 360.0;
	if (fabs (d_left) > limit || fabs (d_right) > limit) {
		GMT_report (C, GMT_MSG_NORMAL, "Warning: Using spherical projection with authalic latitudes\n");
		return (TRUE);
	}
	else /* Use full ellipsoidal terms */
		return (FALSE);
}

void GMT_set_polar (struct GMT_CTRL *C)
{
	/* Determines if the projection pole is N or S pole */

	if (GMT_IS_ZERO (fabs (C->current.proj.pars[1]) - 90.0)) {
		C->current.proj.polar = TRUE;
		C->current.proj.north_pole = (C->current.proj.pars[1] > 0.0);
	}
}

void GMT_lat_swap_init (struct GMT_CTRL *C)
{
	/* Initialize values in C->current.proj.GMT_lat_swap_vals based on C->current.proj.

	First compute C->current.proj.GMT_lat_swap_vals.ra (and rm), the radii to use in
	spherical formulae for area (respectively, N-S distance) when
	using the authalic (respectively, meridional) latitude.

	Then for each type of swap:
	First load C->current.proj.GMT_lat_swap_vals.c[itype][k], k=0,1,2,3 with the
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

	Currently, (GMT v5) the only ones we anticipate using are
	geodetic, authalic, and conformal.  I have put others in here
	for possible future convenience.

	Also, I made this depend on C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid]
	rather than on C->current.proj, so that it will be possible to
	call GMT_lat_swap() without having to pass -R and -J to
	GMT_map_setup(), so that in the future we will be able to use
	lat conversions without plotting maps.

	W H F Smith, 10--13 May 1999.   */

	GMT_LONG i;
	double x, xx[4], a, f, e2, e4, e6, e8;

	f = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening;
	a = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius;

	if (GMT_IS_ZERO (f)) {
		GMT_memset (C->current.proj.GMT_lat_swap_vals.c, GMT_LATSWAP_N * 4, double);
		C->current.proj.GMT_lat_swap_vals.ra = C->current.proj.GMT_lat_swap_vals.rm = a;
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
	C->current.proj.GMT_lat_swap_vals.ra = a * sqrt( (1.0 + x) * (1.0 - e2));

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
	C->current.proj.GMT_lat_swap_vals.rm = a * (1.0 - x);


	/* Geodetic to authalic:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][0] = -(e2 / 3.0 + (31.0 * e4 / 180.0 + 59.0 * e6 / 560.0));
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][1] = 17.0 * e4 / 360.0 + 61.0 * e6 / 1260;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][2] = -383.0 * e6 / 45360.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A][3] = 0.0;

	/* Authalic to geodetic:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][0] = e2 / 3.0 + (31.0 * e4 / 180.0 + 517.0 * e6 / 5040.0);
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][1] = 23.0 * e4 / 360.0 + 251.0 * e6 / 3780;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][2] = 761.0 * e6 / 45360.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G][3] = 0.0;

	/* Geodetic to conformal:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][0] = -(e2 / 2.0 + (5.0 * e4 / 24.0 + (3.0 * e6 / 32.0 + 281.0 * e8 / 5760.0)));
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][1] = 5.0 * e4 / 48.0 + (7.0 * e6 / 80.0 + 697.0 * e8 / 11520.0);
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][2] = -(13.0 * e6 / 480.0 + 461.0 * e8 / 13440.0);
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C][3] = 1237.0 * e8 / 161280.0;

	/* Conformal to geodetic:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][0] = e2 / 2.0 + (5.0 * e4 / 24.0 + (e6 / 12.0 + 13.0 * e8 / 360.0)) ;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][1] = 7.0 * e4 / 48.0 + (29.0 * e6 / 240.0 + 811.0 * e8 / 11520.0);
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][2] = 7.0 * e6 / 120.0 + 81.0 * e8 / 1120.0;  /* Bug fixed 13nov07 whfs */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G][3] = 4279.0 * e8 / 161280.0;


	/* The meridional and parametric developments use this parameter:  */
	x = f/(2.0 - f);		/* Adams calls this n.  It is f/(2-f), or -betaJK in my notes.  */
	xx[0] = x;			/* n  */
	xx[1] = x * x;			/* n-squared  */
	xx[2] = xx[1] * x;		/* n-cubed  */
	xx[3] = xx[2] * x;		/* n to the 4th  */

	/* Geodetic to meridional:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][0] = -(3.0 * xx[0] / 2.0 - 9.0 * xx[2] / 16.0);
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][1] = 15.0 * xx[1] / 16.0 - 15.0 * xx[3] / 32.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][2] = -35.0 * xx[2] / 48.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2M][3] = 315.0 * xx[3] / 512.0;

	/* Meridional to geodetic:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][0] = 3.0 * xx[0] / 2.0 - 27.0 * xx[2] / 32.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][1] = 21.0 * xx[1] / 16.0 - 55.0 * xx[3] / 32.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][2] = 151.0 * xx[2] / 96.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_M2G][3] = 1097.0 * xx[3] / 512.0;

	/* Geodetic to parametric equals parametric to geocentric:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][0] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][0] = -xx[0];
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][1] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][1] = xx[1] / 2.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][2] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][2] = -xx[2] / 3.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2P][3] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2O][3] = xx[3] / 4.0;

	/* Parametric to geodetic equals geocentric to parametric:  */
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][0] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][0] = xx[0];
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][1] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][1] = xx[1] / 2.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][2] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][2] = xx[2] / 3.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_P2G][3] = C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2P][3] = xx[3] / 4.0;


	/* The geodetic <->geocentric use this parameter:  */
	x = 1.0 - e2;
	x = (1.0 - x)/(1.0 + x);	/* Adams calls this m.  It is e2/(2-e2), or -betaJK in my notes.  */
	xx[0] = x;			/* m  */
	xx[1] = x * x;			/* m-squared  */
	xx[2] = xx[1] * x;		/* m-cubed  */
	xx[3] = xx[2] * x;		/* m to the 4th  */

	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][0] = -xx[0];
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][1] = xx[1] / 2.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][2] = -xx[2] / 3.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2O][3] = xx[3] / 4.0;

	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][0] = xx[0];
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][1] = xx[1] / 2.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][2] = xx[2] / 3.0;
	C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_O2G][3] = xx[3] / 4.0;


	/* Now do the Snyder Shuffle:  */
	for (i = 0; i < GMT_LATSWAP_N; i++) {
		C->current.proj.GMT_lat_swap_vals.c[i][0] = C->current.proj.GMT_lat_swap_vals.c[i][0] - C->current.proj.GMT_lat_swap_vals.c[i][2];
		C->current.proj.GMT_lat_swap_vals.c[i][1] = 2.0 * C->current.proj.GMT_lat_swap_vals.c[i][1] - 4.0 * C->current.proj.GMT_lat_swap_vals.c[i][3];
		C->current.proj.GMT_lat_swap_vals.c[i][2] *= 4.0;
		C->current.proj.GMT_lat_swap_vals.c[i][3] *= 8.0;
	}

	return;
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

GMT_LONG GMT_wesn_outside (struct GMT_CTRL *C, double lon, double lat)
{
	/* Determine if a point (lon,lat) is outside or on the rectangular lon/lat boundaries
	 * The check C->current.map.lon_wrap is include since we need to consider the 360
	 * degree periodicity of the longitude coordinate.
	 * When we are making basemaps and may want to ensure that a point is
	 * slightly outside the border without having it automatically flip by
	 * 360 degrees. In that case C->current.map.lon_wrap will be temporarily set to FALSE.
	 */

	if (C->current.map.lon_wrap) {
		while (lon < C->common.R.wesn[XLO] && lon + 360.0 <= C->common.R.wesn[XHI]) lon += 360.0;
		while (lon > C->common.R.wesn[XHI] && lon - 360.0 >= C->common.R.wesn[XLO]) lon -= 360.0;
	}

	if (C->current.map.on_border_is_outside && fabs (lon - C->common.R.wesn[XLO]) < GMT_SMALL)
		C->current.map.this_x_status = -1;
	else if (C->current.map.on_border_is_outside && fabs (lon - C->common.R.wesn[XHI]) < GMT_SMALL)
		C->current.map.this_x_status = 1;
	else if (lon < C->common.R.wesn[XLO])
		C->current.map.this_x_status = -2;
	else if (lon > C->common.R.wesn[XHI])
		C->current.map.this_x_status = 2;
	else
		C->current.map.this_x_status = 0;

	if (C->current.map.on_border_is_outside && fabs (lat - C->common.R.wesn[YLO]) < GMT_SMALL)
		C->current.map.this_y_status = -1;
	else if (C->current.map.on_border_is_outside && fabs (lat - C->common.R.wesn[YHI]) < GMT_SMALL)
		C->current.map.this_y_status = 1;
	else if (lat < C->common.R.wesn[YLO])
		C->current.map.this_y_status = -2;
	else if (lat > C->common.R.wesn[YHI])
		C->current.map.this_y_status = 2;
	else
		C->current.map.this_y_status = 0;

	return (C->current.map.this_x_status != 0 || C->current.map.this_y_status != 0);

}

GMT_LONG GMT_polar_outside (struct GMT_CTRL *C, double lon, double lat)
{
	GMT_wesn_outside (C, lon, lat);

	if (!C->current.proj.edge[1]) C->current.map.this_x_status = 0;	/* 360 degrees, no edge */
	if (C->current.map.this_y_status < 0 && !C->current.proj.edge[0]) C->current.map.this_y_status = 0;	/* South pole enclosed */
	if (C->current.map.this_y_status > 0 && !C->current.proj.edge[2]) C->current.map.this_y_status = 0;	/* North pole enclosed */

	return (C->current.map.this_x_status != 0 || C->current.map.this_y_status != 0);
}

GMT_LONG GMT_eqdist_outside (struct GMT_CTRL *C, double lon, double lat)
{
	double cc, s, c;

	lon -= C->current.proj.central_meridian;
	while (lon < -180.0) lon += 360.0;
	while (lon > 180.0) lon -= 360.0;
	sincosd (lat, &s, &c);
	cc = C->current.proj.sinp * s + C->current.proj.cosp * c * cosd (lon);
	if (cc < -1.0) {
		C->current.map.this_y_status = -1;
		C->current.map.this_x_status = 0;
	}
	else
		C->current.map.this_x_status = C->current.map.this_y_status = 0;
	return (C->current.map.this_y_status != 0);
}

GMT_LONG GMT_radial_outside (struct GMT_CTRL *C, double lon, double lat)
{
	double dist;

	/* Test if point is more than horizon spherical degrees from origin.  For global maps, let all borders be "south" */

	C->current.map.this_x_status = 0;
	dist = GMT_great_circle_dist_degree (C, lon, lat, C->current.proj.central_meridian, C->current.proj.pole);
	if (C->current.map.on_border_is_outside && fabs (dist - C->current.proj.f_horizon) < GMT_SMALL)
		C->current.map.this_y_status = -1;
	else if (dist > C->current.proj.f_horizon)
		C->current.map.this_y_status = -2;
	else
		C->current.map.this_y_status = 0;
	return (C->current.map.this_y_status != 0);
}

GMT_LONG GMT_rect_outside (struct GMT_CTRL *C, double lon, double lat)
{
	double x, y;

	GMT_geo_to_xy (C, lon, lat, &x, &y);

	if (C->current.map.on_border_is_outside && fabs (x - C->current.proj.rect[XLO]) < GMT_SMALL)
		C->current.map.this_x_status = -1;
	else if (C->current.map.on_border_is_outside && fabs (x - C->current.proj.rect[XHI]) < GMT_SMALL)
		C->current.map.this_x_status = 1;
	else if (x < C->current.proj.rect[XLO])
		C->current.map.this_x_status = -2;
	else if (x > C->current.proj.rect[XHI])
		C->current.map.this_x_status = 2;
	else
		C->current.map.this_x_status = 0;

	if (C->current.map.on_border_is_outside && fabs (y -C->current.proj.rect[YLO]) < GMT_SMALL)
		C->current.map.this_y_status = -1;
	else if (C->current.map.on_border_is_outside && fabs (y - C->current.proj.rect[YHI]) < GMT_SMALL)
		C->current.map.this_y_status = 1;
	else if (y < C->current.proj.rect[YLO])
		C->current.map.this_y_status = -2;
	else if (y > C->current.proj.rect[YHI])
		C->current.map.this_y_status = 2;
	else
		C->current.map.this_y_status = 0;

	return (C->current.map.this_x_status != 0 || C->current.map.this_y_status != 0);
}

GMT_LONG GMT_rect_outside2 (struct GMT_CTRL *C, double lon, double lat)
{	/* For Azimuthal proj with rect borders since GMT_rect_outside may fail for antipodal points */
	if (GMT_radial_outside (C, lon, lat)) return (TRUE);	/* Point > 90 degrees away */
	return (GMT_rect_outside (C, lon, lat));	/* Must check if inside box */
}

void GMT_x_wesn_corner (struct GMT_CTRL *C, double *x)
{
/*	if (fabs (fmod (fabs (*x - C->common.R.wesn[XLO]), 360.0)) <= GMT_SMALL)
		*x = C->common.R.wesn[XLO];
	else if (fabs (fmod (fabs (*x - C->common.R.wesn[XHI]), 360.0)) <= GMT_SMALL)
		*x = C->common.R.wesn[XHI]; */

	if (fabs (*x - C->common.R.wesn[XLO]) <= GMT_SMALL)
		*x = C->common.R.wesn[XLO];
	else if (fabs (*x - C->common.R.wesn[XHI]) <= GMT_SMALL)
		*x = C->common.R.wesn[XHI];
}

void GMT_y_wesn_corner (struct GMT_CTRL *C, double *y)
{
	if (fabs (*y - C->common.R.wesn[YLO]) <= GMT_SMALL)
		*y = C->common.R.wesn[YLO];
	else if (fabs (*y - C->common.R.wesn[YHI]) <= GMT_SMALL)
		*y = C->common.R.wesn[YHI];
}

GMT_LONG GMT_is_wesn_corner (struct GMT_CTRL *C, double x, double y)
{	/* Checks if point is a corner */
	C->current.map.corner = 0;

	if (GMT_IS_ZERO (fmod (fabs (x - C->common.R.wesn[XLO]), 360.0))) {
		if (GMT_IS_ZERO (y - C->common.R.wesn[YLO]))
			C->current.map.corner = 1;
		else if (GMT_IS_ZERO (y - C->common.R.wesn[YHI]))
			C->current.map.corner = 4;
	}
	else if (GMT_IS_ZERO (fmod (fabs (x - C->common.R.wesn[XHI]), 360.0))) {
		if (GMT_IS_ZERO (y - C->common.R.wesn[YLO]))
			C->current.map.corner = 2;
		else if (GMT_IS_ZERO (y - C->common.R.wesn[YHI]))
			C->current.map.corner = 3;
	}
	return (C->current.map.corner > 0);
}

GMT_LONG GMT_lon_inside (struct GMT_CTRL *C, double lon, double w, double e)
{
	while (lon < C->common.R.wesn[XLO]) lon += 360.0;
	while (lon > C->common.R.wesn[XHI]) lon -= 360.0;

	if (lon < w) return (FALSE);
	if (lon > e) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_wesn_crossing (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Compute all crossover points of a line segment with the rectangular lat/lon boundaries
	 * Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	GMT_LONG n = 0, i;
	double d, dlon0, dlat0, x0, y0;

	/* If wrapping is allowed: first bring both points between W and E boundaries,
	 * then move the western-most point east if it is further than 180 degrees away.
	 * This may cause the points to span the eastern boundary */

	if (C->current.map.lon_wrap) {
		while (lon0 < C->common.R.wesn[XLO]) lon0 += 360.0;
		while (lon0 > C->common.R.wesn[XHI]) lon0 -= 360.0;
		while (lon1 < C->common.R.wesn[XLO]) lon1 += 360.0;
		while (lon1 > C->common.R.wesn[XHI]) lon1 -= 360.0;
		if (fabs (lon0 - lon1) <= 180.0) { /* Nothing */ }
		else if (lon0 < lon1)
			lon0 += 360.0;
		else
			lon1 += 360.0;
	}

	dlat0 = lat0 - lat1;
	dlon0 = lon0 - lon1;

	/* Then set 'almost'-corners to corners */
	GMT_x_wesn_corner (C, &lon0);
	GMT_x_wesn_corner (C, &lon1);
	GMT_y_wesn_corner (C, &lat0);
	GMT_y_wesn_corner (C, &lat1);

	/* Crossing South */
	if ((lat0 >= C->common.R.wesn[YLO] && lat1 <= C->common.R.wesn[YLO]) || (lat1 >= C->common.R.wesn[YLO] && lat0 <= C->common.R.wesn[YLO])) {
		sides[n] = 0;
		clat[n] = C->common.R.wesn[YLO];
		d = lat0 - lat1;
		clon[n] = (GMT_IS_ZERO (d)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		GMT_x_wesn_corner (C, &clon[n]);
		if (fabs(dlat0) > 0.0 && GMT_lon_inside (C, clon[n], C->common.R.wesn[XLO], C->common.R.wesn[XHI])) n++;
	}
	/* Crossing East */
	if ((lon0 >= C->common.R.wesn[XHI] && lon1 <= C->common.R.wesn[XHI]) || (lon1 >= C->common.R.wesn[XHI] && lon0 <= C->common.R.wesn[XHI])) {
		sides[n] = 1;
		clon[n] = C->common.R.wesn[XHI];
		d = lon0 - lon1;
		clat[n] = (GMT_IS_ZERO (d)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		GMT_y_wesn_corner (C, &clat[n]);
		if (fabs(dlon0) > 0.0 && clat[n] >= C->common.R.wesn[YLO] && clat[n] <= C->common.R.wesn[YHI]) n++;
	}

	/* Now adjust the longitudes so that they might span the western boundary */
	if (C->current.map.lon_wrap && MAX(lon0, lon1) > C->common.R.wesn[XHI]) {
		lon0 -= 360.0; lon1 -= 360.0;
	}

	/* Crossing North */
	if ((lat0 >= C->common.R.wesn[YHI] && lat1 <= C->common.R.wesn[YHI]) || (lat1 >= C->common.R.wesn[YHI] && lat0 <= C->common.R.wesn[YHI])) {
		sides[n] = 2;
		clat[n] = C->common.R.wesn[YHI];
		d = lat0 - lat1;
		clon[n] = (GMT_IS_ZERO (d)) ? lon1 : lon1 + (lon0 - lon1) * (clat[n] - lat1) / d;
		GMT_x_wesn_corner (C, &clon[n]);
		if (fabs(dlat0) > 0.0 && GMT_lon_inside (C, clon[n], C->common.R.wesn[XLO], C->common.R.wesn[XHI])) n++;
	}
	/* Crossing West */
	if ((lon0 <= C->common.R.wesn[XLO] && lon1 >= C->common.R.wesn[XLO]) || (lon1 <= C->common.R.wesn[XLO] && lon0 >= C->common.R.wesn[XLO])) {
		sides[n] = 3;
		clon[n] = C->common.R.wesn[XLO];
		d = lon0 - lon1;
		clat[n] = (GMT_IS_ZERO (d)) ? lat1 : lat1 + (lat0 - lat1) * (clon[n] - lon1) / d;
		GMT_y_wesn_corner (C, &clat[n]);
		if (fabs(dlon0) > 0.0 && clat[n] >= C->common.R.wesn[YLO] && clat[n] <= C->common.R.wesn[YHI]) n++;
	}

	if (n == 0) return (0);

	for (i = 0; i < n; i++) {
		GMT_geo_to_xy (C, clon[i], clat[i], &xx[i], &yy[i]);
		if (C->current.proj.projection == GMT_POLAR && sides[i]%2) sides[i] = 4 - sides[i];	/*  toggle 1 <-> 3 */
	}

	if (n == 1) return (1);

	/* Check for corner xover if n == 2 */

	if (GMT_is_wesn_corner (C, clon[0], clat[0])) return (1);

	if (GMT_is_wesn_corner (C, clon[1], clat[1])) {
		clon[0] = clon[1];
		clat[0] = clat[1];
		xx[0] = xx[1];
		yy[0] = yy[1];
		sides[0] = sides[1];
		return (1);
	}

	/* Sort the two intermediate points into the right order based on projected distances from the first point */

	GMT_geo_to_xy (C, lon0, lat0, &x0, &y0);

	if (hypot (x0 - xx[1], y0 - yy[1]) < hypot (x0 - xx[0], y0 - yy[0])) {
		d_swap (clon[0], clon[1]);
		d_swap (clat[0], clat[1]);
		d_swap (xx[0], xx[1]);
		d_swap (yy[0], yy[1]);
		l_swap (sides[0], sides[1]);
	}

	return (2);
}

void GMT_x_rect_corner (struct GMT_CTRL *C, double *x)
{
	if (fabs (*x) <= GMT_SMALL)
		*x = 0.0;
	else if (fabs (*x - C->current.proj.rect[XHI]) <= GMT_SMALL)
		*x = C->current.proj.rect[XHI];
}

void GMT_y_rect_corner (struct GMT_CTRL *C, double *y)
{
	if (fabs (*y) <= GMT_SMALL)
		*y = 0.0;
	else if (fabs (*y - C->current.proj.rect[YHI]) <= GMT_SMALL)
		*y = C->current.proj.rect[YHI];
}

GMT_LONG GMT_is_rect_corner (struct GMT_CTRL *C, double x, double y)
{	/* Checks if point is a corner */
	C->current.map.corner = -1;
	if (GMT_IS_ZERO (x - C->current.proj.rect[XLO])) {
		if (GMT_IS_ZERO (y - C->current.proj.rect[YLO]))
			C->current.map.corner = 1;
		else if (GMT_IS_ZERO (y - C->current.proj.rect[YHI]))
			C->current.map.corner = 4;
	}
	else if (GMT_IS_ZERO (x - C->current.proj.rect[XHI])) {
		if (GMT_IS_ZERO (y - C->current.proj.rect[YLO]))
			C->current.map.corner = 2;
		else if (GMT_IS_ZERO (y - C->current.proj.rect[YHI]))
			C->current.map.corner = 3;
	}
	return (C->current.map.corner > 0);
}

GMT_LONG GMT_rect_crossing (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Compute all crossover points of a line segment with the boundaries in a rectangular projection */

	GMT_LONG i, j, n = 0;
	double x0, x1, y0, y1, d, dx, dy;

	/* Since it may not be obvious which side the line may cross, and since in some cases the two points may be
	 * entirely outside the region but still cut through it, we first find all possible candidates and then decide
	 * which ones are valid crossings.  We may find 0, 1, or 2 intersections */

	GMT_geo_to_xy (C, lon0, lat0, &x0, &y0);
	GMT_geo_to_xy (C, lon1, lat1, &x1, &y1);

	/* First set 'almost'-corners to corners */

	dx = x0 - x1;	/* Pre-adjust increment in x */
	dy = y0 - y1;	/* Pre-adjust increment in y */
	GMT_x_rect_corner (C, &x0);
	GMT_x_rect_corner (C, &x1);
	GMT_y_rect_corner (C, &y0);
	GMT_y_rect_corner (C, &y1);

	if ((y0 >= C->current.proj.rect[YLO] && y1 <= C->current.proj.rect[YLO]) || (y1 >= C->current.proj.rect[YLO] && y0 <= C->current.proj.rect[YLO])) {
		sides[n] = 0;
		yy[n] = C->current.proj.rect[YLO];
		d = y0 - y1;
		xx[n] = (GMT_IS_ZERO (d)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (C, &xx[n]);
		if (fabs(d) > 0.0 && xx[n] >= C->current.proj.rect[XLO] && xx[n] <= C->current.proj.rect[XHI]) n++;
	}
	if ((x0 <= C->current.proj.rect[XHI] && x1 >= C->current.proj.rect[XHI]) || (x1 <= C->current.proj.rect[XHI] && x0 >= C->current.proj.rect[XHI])) {
		sides[n] = 1;
		xx[n] = C->current.proj.rect[XHI];
		d = x0 - x1;
		yy[n] = (GMT_IS_ZERO (d)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (C, &yy[n]);
		if (fabs(d) > 0.0 && yy[n] >= C->current.proj.rect[YLO] && yy[n] <= C->current.proj.rect[YHI]) n++;
	}
	if ((y0 <= C->current.proj.rect[YHI] && y1 >= C->current.proj.rect[YHI]) || (y1 <= C->current.proj.rect[YHI] && y0 >= C->current.proj.rect[YHI])) {
		sides[n] = 2;
		yy[n] = C->current.proj.rect[YHI];
		d = y0 - y1;
		xx[n] = (GMT_IS_ZERO (d)) ? x0 : x1 + (x0 - x1) * (yy[n] - y1) / d;
		GMT_x_rect_corner (C, &xx[n]);
		if (fabs(d) > 0.0 && xx[n] >= C->current.proj.rect[XLO] && xx[n] <= C->current.proj.rect[XHI]) n++;
	}
	if ((x0 >= C->current.proj.rect[XLO] && x1 <= C->current.proj.rect[XLO]) || (x1 >= C->current.proj.rect[XLO] && x0 <= C->current.proj.rect[XLO])) {
		sides[n] = 3;
		xx[n] = C->current.proj.rect[XLO];
		d = x0 - x1;
		yy[n] = (GMT_IS_ZERO (dx)) ? y0 : y1 + (y0 - y1) * (xx[n] - x1) / d;
		GMT_y_rect_corner (C, &yy[n]);
		if (fabs(d) > 0.0 && yy[n] >= C->current.proj.rect[YLO] && yy[n] <= C->current.proj.rect[YHI]) n++;
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

	for (i = 0; i < n; i++)	GMT_xy_to_geo (C, &clon[i], &clat[i], xx[i], yy[i]);

	if (!GMT_is_geographic (C, GMT_IN)) return (n);

	if (n < 2) return (n);

	/* Check for corner xover if n == 2 */

	if (GMT_is_rect_corner (C, xx[0], yy[0])) return (1);

	if (GMT_is_rect_corner (C, xx[1], yy[1])) {
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

GMT_LONG GMT_radial_crossing (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	/* Computes the lon/lat of a point that is f_horizon spherical degrees from
	 * the origin and lies on the great circle between points 1 and 2 */

	double dist1, dist2, delta, eps, dlon;

	dist1 = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon1, lat1);
	dist2 = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon2, lat2);
	delta = dist2 - dist1;
	eps = (GMT_IS_ZERO (delta)) ? 0.0 : (C->current.proj.f_horizon - dist1) / delta;
	dlon = lon2 - lon1;
	if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
	clon[0] = lon1 + dlon * eps;
	clat[0] = lat1 + (lat2 - lat1) * eps;

	GMT_geo_to_xy (C, clon[0], clat[0], &xx[0], &yy[0]);

	sides[0] = 1;

	return (1);
}

GMT_LONG GMT_map_jump_x (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{
	/* TRUE if x-distance between points exceeds 1/2 map width at this y value */
	double dx, map_half_size;

	if (!(GMT_IS_CYLINDRICAL (C) || GMT_IS_MISC (C))) return (0);	/* Only projections with peroidic boundaries may apply */

	if (!GMT_is_geographic (C, GMT_IN) || fabs (C->common.R.wesn[XLO] - C->common.R.wesn[XHI]) < 90.0) return (FALSE);

	map_half_size = MAX (GMT_half_map_width (C, y0), GMT_half_map_width (C, y1));
	if (fabs (map_half_size) < GMT_SMALL) return (0);

	dx = x1 - x0;
	if (dx > map_half_size)	return (-1);	/* Cross left/west boundary */
	if (dx < (-map_half_size)) return (1);	/* Cross right/east boundary */
	return (0);
}

GMT_LONG GMT_ellipse_crossing (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
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
		GMT_geo_to_xy (C, lon1, lat1, &x1, &y1);
		GMT_geo_to_xy (C, lon2, lat2, &x2, &y2);
		if ((jump = GMT_map_jump_x (C, x2, y2, x1, y1))) {
			(*C->current.map.get_crossings) (C, xx, yy, x2, y2, x1, y1);
			if (jump == 1) {	/* Add right border point first */
				d_swap (xx[0], xx[1]);
				d_swap (yy[0], yy[1]);
			}
			GMT_xy_to_geo (C, &clon[0], &clat[0], xx[0], yy[0]);
			GMT_xy_to_geo (C, &clon[1], &clat[1], xx[1], yy[1]);
			n = 2;
		}
	}
	if (n == 1) for (i = 0; i < n; i++) GMT_geo_to_xy (C, clon[i], clat[i], &xx[i], &yy[i]);
	return (n);
}

GMT_LONG GMT_eqdist_crossing (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double *clon, double *clat, double *xx, double *yy, GMT_LONG *sides)
{
	double angle, x, y, s, c;

	/* Computes the x.y of the antipole point that lies on a radius from
	 * the origin through the inside point */

	if (GMT_eqdist_outside (C, lon1, lat1)) {	/* Point 1 is on perimeter */
		GMT_geo_to_xy (C, lon2, lat2, &x, &y);
		angle = d_atan2 (y - C->current.proj.origin[GMT_Y], x - C->current.proj.origin[GMT_X]);
		sincos (angle, &s, &c);
		xx[0] = C->current.proj.r * c + C->current.proj.origin[GMT_X];
		yy[0] = C->current.proj.r * s + C->current.proj.origin[GMT_Y];
		clon[0] = lon1;
		clat[0] = lat1;
	}
	else {	/* Point 2 is on perimeter */
		GMT_geo_to_xy (C, lon1, lat1, &x, &y);
		angle = d_atan2 (y - C->current.proj.origin[GMT_Y], x - C->current.proj.origin[GMT_X]);
		sincos (angle, &s, &c);
		xx[0] = C->current.proj.r * c + C->current.proj.origin[GMT_X];
		yy[0] = C->current.proj.r * s + C->current.proj.origin[GMT_Y];
		clon[0] = lon2;
		clat[0] = lat2;
	}
	sides[0] = 1;

	return (1);
}

/*  Routines to do with clipping */

GMT_LONG GMT_map_crossing (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double *xlon, double *xlat, double *xx, double *yy, GMT_LONG *sides)
{
	if (C->current.map.prev_x_status == C->current.map.this_x_status && C->current.map.prev_y_status == C->current.map.this_y_status) {
		/* This is naive. We could have two points outside with a line drawn between crossing the plotting area. */
		return (0);
	}
	else if ((C->current.map.prev_x_status == 0 && C->current.map.prev_y_status == 0) || (C->current.map.this_x_status == 0 && C->current.map.this_y_status == 0)) {
		/* This is a crossing */
	}
	else if (!(*C->current.map.overlap) (C, lon1, lat1, lon2, lat2))	/* Less clearcut case, check for overlap */
		return (0);

	/* Now compute the crossing */

	C->current.map.corner = -1;
	return ((*C->current.map.crossing) (C, lon1, lat1, lon2, lat2, xlon, xlat, xx, yy, sides));
}

GMT_LONG GMT_clip_to_map (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG np, double **x, double **y)
{
	/* This routine makes sure that all points are either inside or on the map boundary
	 * and returns the number of points to be used for plotting (in x,y units) */

	GMT_LONG i, n, out, out_x, out_y, np2, total_nx = 0, polygon;
	double *xx = NULL, *yy = NULL;

	/* First check for trivial cases:  All points outside or all points inside */

	for (i = out = out_x = out_y = 0; i < np; i++)  {
		(void) GMT_map_outside (C, lon[i], lat[i]);
		out_x += C->current.map.this_x_status;	/* Completely left of west gives -2 * np, right of east gives + 2 * np */
		out_y += C->current.map.this_y_status;	/* Completely below south gives -2 * np, above north gives + 2 * np */
		out += (GMT_abs (C->current.map.this_x_status) == 2 || GMT_abs (C->current.map.this_y_status) == 2);
	}
	if (out == 0) {		/* All points are inside map boundary; no clipping required */
		(void)GMT_malloc2 (C, xx, yy, np, 0, double);
		for (i = 0; i < np; i++) GMT_geo_to_xy (C, lon[i], lat[i], &xx[i], &yy[i]);
		*x = xx;	*y = yy;	n = np;
	}
	else if (out == np) {	/* All points are outside map boundary */
		np2 = 2 * np;
		if (GMT_abs (out_x) == np2 || GMT_abs (out_y) == np2)	/* All points safely outside the region, no part of polygon survives */
			n = 0;
		else {	/* All points are outside, but they are not just to one side so lines _may_ intersect the region */
			n = (*C->current.map.clip) (C, lon, lat, np, x, y, &total_nx);
			polygon = !GMT_polygon_is_open (C, lon, lat, np);	/* The following can only be used on closed polygons */
			/* Polygons that completely contains the -R region will not generate crossings, just duplicate -R box */
			if (polygon && n > 0 && total_nx == 0) {	/* No crossings and all points outside means one of two things: */
				/* Either the polygon contains portions of the -R region including corners or it does not.  We pick the corners and check for insidedness: */
				GMT_LONG ok = FALSE;
				if (GMT_non_zero_winding (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], lon, lat, np)) ok = TRUE;		/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO], lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok && GMT_non_zero_winding (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI], lon, lat, np)) ok = TRUE;	/* TRUE if inside */
				if (!ok) {
					/* Polygon does NOT contain the region and we delete it */
					n = 0;
					GMT_free (C, *x);
					GMT_free (C, *y);
				}
				/* Otherwise the polygon completely contains -R and we pass it along */
			}
		}
	}
	else	/* Mixed case so we must clip the polygon */
		n = (*C->current.map.clip) (C, lon, lat, np, x, y, &total_nx);

	return (n);
}

double GMT_x_to_corner (struct GMT_CTRL *C, double x) {
	return ( (fabs (x - C->current.proj.rect[XLO]) < fabs (x - C->current.proj.rect[XHI])) ? C->current.proj.rect[XLO] : C->current.proj.rect[XHI]);
}

double GMT_y_to_corner (struct GMT_CTRL *C, double y) {
	return ( (fabs (y - C->current.proj.rect[YLO]) < fabs (y - C->current.proj.rect[YHI])) ? C->current.proj.rect[YLO] : C->current.proj.rect[YHI]);
}

GMT_LONG GMT_move_to_rect (struct GMT_CTRL *C, double *x_edge, double *y_edge, GMT_LONG j, GMT_LONG nx)
{
	GMT_LONG n = 0, key;
	double xtmp, ytmp;

	/* May add 0, 1, or 2 points to path */

	if (C->current.map.this_x_status == 0 && C->current.map.this_y_status == 0) return (1);	/* Completely Inside */

	if (!nx && j > 0 && C->current.map.this_x_status != C->current.map.prev_x_status && C->current.map.this_y_status != C->current.map.prev_y_status) {	/* Must include corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((C->current.map.this_x_status * C->current.map.prev_x_status) == -4 || (C->current.map.this_y_status * C->current.map.prev_y_status) == -4) {	/* the two points outside on opposite sides */
			x_edge[j] = (C->current.map.prev_x_status < 0) ? C->current.proj.rect[XLO] : ((C->current.map.prev_x_status > 0) ? C->current.proj.rect[XHI] : GMT_x_to_corner (C, x_edge[j-1]));
			y_edge[j] = (C->current.map.prev_y_status < 0) ? C->current.proj.rect[YLO] : ((C->current.map.prev_y_status > 0) ? C->current.proj.rect[YHI] : GMT_y_to_corner (C, y_edge[j-1]));
			j++;
			x_edge[j] = (C->current.map.this_x_status < 0) ? C->current.proj.rect[XLO] : ((C->current.map.this_x_status > 0) ? C->current.proj.rect[XHI] : GMT_x_to_corner (C, xtmp));
			y_edge[j] = (C->current.map.this_y_status < 0) ? C->current.proj.rect[YLO] : ((C->current.map.this_y_status > 0) ? C->current.proj.rect[YHI] : GMT_y_to_corner (C, ytmp));
			j++;
		}
		else {
			key = MIN (C->current.map.this_x_status, C->current.map.prev_x_status);
			x_edge[j] = (key < 0) ? C->current.proj.rect[XLO] : C->current.proj.rect[XHI];
			key = MIN (C->current.map.this_y_status, C->current.map.prev_y_status);
			y_edge[j] = (key < 0) ? C->current.proj.rect[YLO] : C->current.proj.rect[YHI];
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}

	if (C->current.map.outside == GMT_rect_outside2) {	/* Need special check because this outside2 test is screwed up... */
		if (x_edge[j] < C->current.proj.rect[XLO]) {
			x_edge[j] = C->current.proj.rect[XLO];
			C->current.map.this_x_status = -2;
		}
		else if (x_edge[j] > C->current.proj.rect[XHI]) {
			x_edge[j] = C->current.proj.rect[XHI];
			C->current.map.this_x_status = 2;
		}
		if (y_edge[j] < C->current.proj.rect[YLO]) {
			y_edge[j] = C->current.proj.rect[YLO];
			C->current.map.this_y_status = -2;
		}
		else if (y_edge[j] > C->current.proj.rect[YHI]) {
			y_edge[j] = C->current.proj.rect[YHI];
			C->current.map.this_y_status = 2;
		}
	}
	else {
		if (C->current.map.this_x_status != 0) x_edge[j] = (C->current.map.this_x_status < 0) ? C->current.proj.rect[XLO] : C->current.proj.rect[XHI];
		if (C->current.map.this_y_status != 0) y_edge[j] = (C->current.map.this_y_status < 0) ? C->current.proj.rect[YLO] : C->current.proj.rect[YHI];
	}

	return (n + 1);
}

GMT_LONG GMT_rect_clip_old (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, j = 0, k, nx, n_alloc = GMT_CHUNK, sides[4];
	double xlon[4], xlat[4], xc[4], yc[4], *xx = NULL, *yy = NULL;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	xx = GMT_memory (C, NULL, n_alloc, double);
	yy = GMT_memory (C, NULL, n_alloc, double);
	(void) GMT_map_outside (C, lon[0], lat[0]);
	GMT_geo_to_xy (C, lon[0], lat[0], &xx[0], &yy[0]);
	j += GMT_move_to_rect (C, xx, yy, j, 0);	/* May add 2 points, << n_alloc */

	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (C, lon[i], lat[i]);
		nx = GMT_map_crossing (C, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			xx[j] = xc[k];
			yy[j++] = yc[k];
			if (j >= (n_alloc-2)) {
				n_alloc <<= 1;
				xx = GMT_memory (C, xx, n_alloc, double);
				yy = GMT_memory (C, yy, n_alloc, double);
			}
			(*total_nx) ++;
		}
		GMT_geo_to_xy (C, lon[i], lat[i], &xx[j], &yy[j]);
		if (j >= (n_alloc-2)) {
			n_alloc <<= 1;
			xx = GMT_memory (C, xx, n_alloc, double);
			yy = GMT_memory (C, yy, n_alloc, double);
		}

		j += GMT_move_to_rect (C, xx, yy, j, nx);	/* May add 2 points, which explains the n_alloc-2 stuff */
	}

	xx = GMT_memory (C, xx, j, double);
	yy = GMT_memory (C, yy, j, double);
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

GMT_LONG GMT_clip_sn (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, PFL inside, PFL outside, GMT_LONG *cross)
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

GMT_LONG GMT_clip_we (double x_prev, double y_prev, double x_curr, double y_curr, double x[], double y[], double border, PFL inside, PFL outside, GMT_LONG *cross)
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

GMT_LONG GMT_rect_clip (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, m, n_alloc = 0, side, j, np, in = 1, out = 0, cross = 0, polygon;
	double *xtmp[2] = {NULL, NULL}, *ytmp[2] = {NULL, NULL}, xx[2], yy[2], border[4];
	PFL clipper[4], inside[4], outside[4];
#ifdef DEBUG
	FILE *fp = NULL;
	GMT_LONG dump = 0;
#endif

	if (n == 0) return (0);

	polygon = !GMT_polygon_is_open (C, lon, lat, n);	/* TRUE if input segment is a closed polygon */

	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */

	/* Set up function pointers.  This could be done once in GMT_begin at some point */

	clipper[0] = GMT_clip_sn;	clipper[1] = GMT_clip_we; clipper[2] = GMT_clip_sn;	clipper[3] = GMT_clip_we;
	inside[1] = inside[2] = inside_upper_boundary;	outside[1] = outside[2] = outside_upper_boundary;
	inside[0] = inside[3] = inside_lower_boundary;		outside[0] = outside[3] = outside_lower_boundary;
	border[0] = border[3] = 0.0;	border[1] = C->current.map.width;	border[2] = C->current.map.height;

	n_alloc = (GMT_LONG)irint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], n_alloc, 0, double);

	/* Get Cartesian map coordinates */

	for (m = 0; m < n; m++) GMT_geo_to_xy (C, lon[m], lat[m], &xtmp[0][m], &ytmp[0][m]);

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
				if (m == n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, n_alloc, double);
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && GMT_polygon_is_open (C, xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, n_alloc, double);
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
	}

	GMT_free (C, xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	GMT_free (C, ytmp[1]);

	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = GMT_malloc2 (C, xtmp[0], ytmp[0], 0, m, double);
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
		GMT_free (C, xtmp[0]);
		GMT_free (C, ytmp[0]);
	}

	return (m);
}

/* GMT_dateline_clip simply clips a polygon agains the dateline and results in two polygons in L */

GMT_LONG GMT_split_poly_at_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, struct GMT_LINE_SEGMENT ***Lout)
{
	GMT_LONG k, m, n_alloc = 0, side, j, np, cross = 0;
	char label[BUFSIZ], *part = "EW";
	double xx[2], yy[2];
	struct GMT_LINE_SEGMENT **L = NULL;
	PFL inside[2], outside[2];

	inside[0] = inside_upper_boundary;	outside[0] = outside_upper_boundary;
	inside[1] = inside_lower_boundary;	outside[1] = outside_lower_boundary;
	L = GMT_memory (C, NULL, 2, struct GMT_LINE_SEGMENT *);	/* The two polygons */

	for (k = 0; k < S->n_rows; k++) GMT_lon_range_adjust (C, GMT_IS_0_TO_P360, &S->coord[GMT_X][k]);	/* First enforce 0 <= lon < 360 so we dont have to check again */

	for (side = 0; side < 2; side++) {	/* Do it twice to get two truncated polygons */
		L[side] = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);
		n_alloc = (GMT_LONG)irint (1.05*S->n_rows+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
		GMT_alloc_segment (C, L[side], n_alloc, S->n_columns, TRUE);	/* Temp segment with twice the number of points as we will add crossings*/
		m = 0;		/* Start with nuthin' */

		/* Must ensure we copy the very first point if it is left of the Dateline */
		if (S->coord[GMT_X][0] < 180.0) { L[side]->coord[GMT_X][0] = S->coord[GMT_X][0]; L[side]->coord[GMT_Y][0] = S->coord[GMT_Y][0]; }	/* First point is inside; add it */
		for (k = 1; k < S->n_rows; k++) {	/* For each line segment */
			np = GMT_clip_we (S->coord[GMT_X][k-1], S->coord[GMT_Y][k-1], S->coord[GMT_X][k], S->coord[GMT_Y][k], xx, yy, 180.0, inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) GMT_alloc_segment (C, L[side], n_alloc << 2, S->n_columns, FALSE);
				L[side]->coord[GMT_X][m] = xx[j]; L[side]->coord[GMT_Y][m] = yy[j]; m++;
			}
		}
		if (GMT_polygon_is_open (C, L[side]->coord[GMT_X], L[side]->coord[GMT_Y], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) GMT_alloc_segment (C, L[side], n_alloc << 2, S->n_columns, FALSE);
			L[side]->coord[GMT_X][m] = L[side]->coord[GMT_X][0];	L[side]->coord[GMT_Y][m] = L[side]->coord[GMT_Y][0];	m++;	/* Yes. */
		}
		if (m != n_alloc) GMT_alloc_segment (C, L[side], m, S->n_columns, FALSE);
		L[side]->n_rows = m;
		if (S->label) {
			sprintf (label, "%s part %c", S->label, part[side]);
			L[side]->label = strdup (label);
		}
		if (S->header) L[side]->header = strdup (S->header);
		if (S->ogr) GMT_duplicate_ogr_seg (C, L[side], S);
	}
	L[0]->range = 2;	L[1]->range = 3;	
	*Lout = L;
	return (2);
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

double GMT_lon_to_corner (struct GMT_CTRL *C, double lon) {
	return ( (fabs (lon - C->common.R.wesn[XLO]) < fabs (lon - C->common.R.wesn[XHI])) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI]);
}

double GMT_lat_to_corner (struct GMT_CTRL *C, double lat) {
	return ( (fabs (lat - C->common.R.wesn[YLO]) < fabs (lat - C->common.R.wesn[YHI])) ? C->common.R.wesn[YLO] : C->common.R.wesn[YHI]);
}

GMT_LONG GMT_move_to_wesn (struct GMT_CTRL *C, double *x_edge, double *y_edge, double lon, double lat, double lon_old, double lat_old, GMT_LONG j, GMT_LONG nx)
{
	GMT_LONG n = 0, key;
	double xtmp, ytmp, lon_p, lat_p;

	/* May add 0, 1, or 2 points to path */

	if (!nx && j > 0 && C->current.map.this_x_status != C->current.map.prev_x_status && C->current.map.this_y_status != C->current.map.prev_y_status) {	/* Need corner */
		xtmp = x_edge[j];	ytmp = y_edge[j];
		if ((C->current.map.this_x_status * C->current.map.prev_x_status) == -4 || (C->current.map.this_y_status * C->current.map.prev_y_status) == -4) {	/* the two points outside on opposite sides */
			lon_p = (C->current.map.prev_x_status < 0) ? C->common.R.wesn[XLO] : ((C->current.map.prev_x_status > 0) ? C->common.R.wesn[XHI] : GMT_lon_to_corner (C, lon_old));
			lat_p = (C->current.map.prev_y_status < 0) ? C->common.R.wesn[YLO] : ((C->current.map.prev_y_status > 0) ? C->common.R.wesn[YHI] : GMT_lat_to_corner (C, lat_old));
			GMT_geo_to_xy (C, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
			lon_p = (C->current.map.this_x_status < 0) ? C->common.R.wesn[XLO] : ((C->current.map.this_x_status > 0) ? C->common.R.wesn[XHI] : GMT_lon_to_corner (C, lon));
			lat_p = (C->current.map.this_y_status < 0) ? C->common.R.wesn[YLO] : ((C->current.map.this_y_status > 0) ? C->common.R.wesn[YHI] : GMT_lat_to_corner (C, lat));
			GMT_geo_to_xy (C, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		else {
			key = MIN (C->current.map.this_x_status, C->current.map.prev_x_status);
			lon_p = (key < 0) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
			key = MIN (C->current.map.this_y_status, C->current.map.prev_y_status);
			lat_p = (key < 0) ? C->common.R.wesn[YLO] : C->common.R.wesn[YHI];
			GMT_geo_to_xy (C, lon_p, lat_p, &x_edge[j], &y_edge[j]);
			j++;
		}
		x_edge[j] = xtmp;	y_edge[j] = ytmp;
		n = 1;
	}
	if (C->current.map.this_x_status != 0) lon = (C->current.map.this_x_status < 0) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
	if (C->current.map.this_y_status != 0) lat = (C->current.map.this_y_status < 0) ? C->common.R.wesn[YLO] : C->common.R.wesn[YHI];
	GMT_geo_to_xy (C, lon, lat, &x_edge[j], &y_edge[j]);
	return (n + 1);
}

GMT_LONG GMT_wesn_clip_old (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, j = 0, k, nx, n_alloc = GMT_CHUNK, sides[4];
	double xlon[4], xlat[4], xc[4], yc[4], *xx = NULL, *yy = NULL;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (n == 0) return (0);

	xx = GMT_memory (C, NULL, n_alloc, double);
	yy = GMT_memory (C, NULL, n_alloc, double);

	(void) GMT_map_outside (C, lon[0], lat[0]);
	j = GMT_move_to_wesn (C, xx, yy, lon[0], lat[0], 0.0, 0.0, 0, 0);	/* May add 2 points, << n_alloc */

	for (i = 1; i < n; i++) {
		(void) GMT_map_outside (C, lon[i], lat[i]);
		nx = GMT_map_crossing (C, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides);
		for (k = 0; k < nx; k++) {
			xx[j] = xc[k];
			yy[j++] = yc[k];
			if (j >= (n_alloc-2)) {
				n_alloc <<= 1;
				xx = GMT_memory (C, xx, n_alloc, double);
				yy = GMT_memory (C, yy, n_alloc, double);
			}
			(*total_nx) ++;
		}
		if (j >= (n_alloc-2)) {
			n_alloc <<= 1;
			xx = GMT_memory (C, xx, n_alloc, double);

			yy = GMT_memory (C, yy, n_alloc, double);
		}
		j += GMT_move_to_wesn (C, xx, yy, lon[i], lat[i], lon[i-1], lat[i-1], j, nx);	/* May add 2 points, which explains the n_alloc-2 stuff */
	}

	xx = GMT_memory (C, xx, j, double);
	yy = GMT_memory (C, yy, j, double);
	*x = xx;
	*y = yy;

#ifdef CRAP
{
	FILE *fp = NULL;
	double out[2];
	fp = fopen ("crap.d", "a");
	fprintf (fp, "> N = %d\n", (int)j);
	for (i = 0; i < j; i++) {
		out[GMT_X] = xx[i];
		out[GMT_Y] = yy[i];
		C->current.io.output (C, fp, 2, out);
	}
	fclose (fp);
}
#endif
	return (j);
}

GMT_LONG GMT_wesn_clip (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n_orig, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG i, n, m, new_n, *x_index = NULL, *x_type = NULL;
	GMT_LONG n_alloc, n_x_alloc, side, j, np, in = 1, n_cross = 0, out = 0, cross = 0;
	GMT_LONG polygon, jump = FALSE, curved, periodic = FALSE;
	double *xtmp[2] = {NULL, NULL}, *ytmp[2] = {NULL, NULL}, xx[2], yy[2], border[4];
	double x1, x2, y1, y2;
	PFL clipper[4], inside[4], outside[4];
#ifdef DEBUG
	FILE *fp = NULL;
	GMT_LONG dump = 0;
#endif

	if ((n = n_orig) == 0) return (0);

	/* If there are jumps etc call the old clipper, else we try the new clipper */

	GMT_geo_to_xy (C, lon[0], lat[0], &x1, &y1);
	for (i = 1; !jump && i < n; i++) {
		GMT_geo_to_xy (C, lon[i], lat[i], &x2, &y2);
		jump = GMT_map_jump_x (C, x2, y2, x1, y1);
		x1 = x2;	y1 = y2;
	}

	if (jump) return (GMT_wesn_clip_old (C, lon, lat, n, x, y, total_nx));	/* Must do the old way for now */
	periodic = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);	/* No point clipping against W and E if periodic map */

	/* Here we can try the Sutherland/Hodgman algorithm */

	polygon = !GMT_polygon_is_open (C, lon, lat, n);	/* TRUE if input segment is a closed polygon */

	*total_nx = 1;	/* So that calling program will not discard the clipped polygon */

	/* Set up function pointers.  This could be done once in GMT_begin at some point */

	clipper[0] = GMT_clip_sn;	clipper[1] = GMT_clip_we; clipper[2] = GMT_clip_sn;	clipper[3] = GMT_clip_we;
	inside[1] = inside[2] = inside_upper_boundary;	outside[1] = outside[2] = outside_upper_boundary;
	inside[0] = inside[3] = inside_lower_boundary;		outside[0] = outside[3] = outside_lower_boundary;
	border[0] = C->common.R.wesn[YLO]; border[3] = C->common.R.wesn[XLO];	border[1] = C->common.R.wesn[XHI];	border[2] = C->common.R.wesn[YHI];

	n_alloc = (GMT_LONG)irint (1.05*n+5);	/* Anticipate just a few crossings (5%)+5, allocate more later if needed */
	/* Create a pair of arrays for holding input and output */
	n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], n_alloc, 0, double);

	/* Make copy of lon/lat coordinates */

	GMT_memcpy (xtmp[0], lon, n, double);	GMT_memcpy (ytmp[0], lat, n, double);
	m = n;

	/* Preallocate space for crossing information */

	n_x_alloc = GMT_malloc2 (C, x_index, x_type, GMT_TINY_CHUNK, 0, GMT_LONG);

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

		curved = !((side%2) ? C->current.map.meridian_straight : C->current.map.parallel_straight);	/* Is this border straight or curved when projected */
		l_swap (in, out);	/* Swap what is input and output for clipping against this border */
		if (side%2 && periodic) {	/* No clipping can take place on w or e border; just copy all and go to next side */
			m = n;
			if (m == n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, n_alloc, double);
			GMT_memcpy (xtmp[out], xtmp[in], m, double);
			GMT_memcpy (ytmp[out], ytmp[in], m, double);
			continue;
		}
		/* Must ensure we copy the very first point if it is inside the clip rectangle */
		if (inside[side] ((side%2) ? xtmp[in][0] : ytmp[in][0], border[side])) {xtmp[out][0] = xtmp[in][0]; ytmp[out][0] = ytmp[in][0]; m = 1;}	/* First point is inside; add it */
		for (i = 1; i < n; i++) {	/* For each line segment */
			np = clipper[side] (xtmp[in][i-1], ytmp[in][i-1], xtmp[in][i], ytmp[in][i], xx, yy, border[side], inside[side], outside[side], &cross);	/* Returns 0, 1, or 2 points */
			if (polygon && cross && curved) {	/* When crossing in/out of a curved boundary we must eventually sample along the curve between crossings */
				x_index[n_cross] = m;		/* Index of intersection point (which will be copied from xx[0], yy[0] below) */
				x_type[n_cross] = cross;	/* -1 going out, +1 going in */
				if (++n_cross == n_x_alloc) n_x_alloc = GMT_malloc2 (C, x_index, x_type, n_cross, n_x_alloc, GMT_LONG);
			}
			for (j = 0; j < np; j++) {	/* Add the np returned points to the new clipped polygon path */
				if (m == n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, n_alloc, double);
				xtmp[out][m] = xx[j]; ytmp[out][m] = yy[j]; m++;
			}
		}
		if (polygon && GMT_polygon_is_open (C, xtmp[out], ytmp[out], m)) {	/* Do we need to explicitly close this clipped polygon? */
			if (m == n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], m, n_alloc, double);
			xtmp[out][m] = xtmp[out][0];	ytmp[out][m] = ytmp[out][0];	m++;	/* Yes. */
		}
		if (polygon && curved && n_cross) {	/* Must resample between crossing points */
			double *x_add = NULL, *y_add = NULL, *x_cpy = NULL, *y_cpy = NULL;
			GMT_LONG add, np = 0, last_index = 0;
			GMT_LONG p, p_next;

			if (n_cross%2 == 1) {	/* Should not happen with a polygon */
				GMT_message (C, "Error in GMT_wesn_clip: odd number of crossings?");
			}

			/* First copy the current polygon */

			(void)GMT_malloc2 (C, x_cpy, y_cpy, m, 0, double);
			GMT_memcpy (x_cpy, xtmp[out], m, double);
			GMT_memcpy (y_cpy, ytmp[out], m, double);

			for (p = 0; p < n_cross; p++) {	/* Process each crossing point */
				if (last_index < x_index[p]) {	/* Copy over segment from were we left off to this crossing point */
					add = x_index[p] - last_index;
					if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, n_alloc, double);
					GMT_memcpy (&xtmp[out][np], &x_cpy[last_index], add, double);
					GMT_memcpy (&ytmp[out][np], &y_cpy[last_index], add, double);
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
					add = GMT_map_path (C, start_lon, y_cpy[x_index[p]], stop_lon, y_cpy[x_index[p_next]], &x_add, &y_add);
					if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, n_alloc, double);
					GMT_memcpy (&xtmp[out][np], x_add, add, double);
					GMT_memcpy (&ytmp[out][np], y_add, add, double);
					if (add) { GMT_free (C, x_add);	GMT_free (C, y_add); }
					np += add;
					last_index = x_index[p_next];
				}
			}
			if (x_index[0] > 0) {	/* First point was clean inside, must add last connection */
				add = m - last_index;
				if ((new_n = (np+add)) >= n_alloc) n_alloc = GMT_malloc4 (C, xtmp[0], ytmp[0], xtmp[1], ytmp[1], new_n, n_alloc, double);
				GMT_memcpy (&xtmp[out][np], &x_cpy[last_index], add, double);
				GMT_memcpy (&ytmp[out][np], &y_cpy[last_index], add, double);
				np += add;
			}
			m = np;	/* New total of points */
			GMT_free (C, x_cpy);	GMT_free (C, y_cpy);
		}
	}

	GMT_free (C, xtmp[1]);	/* Free the pairs of arrays that holds the last input array */
	GMT_free (C, ytmp[1]);
	GMT_free (C, x_index);	/* Free the pairs of arrays that holds the crossing info */
	GMT_free (C, x_type);

	if (m) {	/* Reallocate and return the array with the final clipped polygon */
		n_alloc = GMT_malloc2 (C, xtmp[0], ytmp[0], 0, m, double);
		/* Convert to map coordinates */
		for (i = 0; i < m; i++) GMT_geo_to_xy (C, xtmp[0][i], ytmp[0][i], &xtmp[0][i], &ytmp[0][i]);

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
		GMT_free (C, xtmp[0]);
		GMT_free (C, ytmp[0]);
	}

	return (m);
}

GMT_LONG GMT_radial_boundary_arc (struct GMT_CTRL *C, GMT_LONG this, double end_x[], double end_y[], double **xarc, double **yarc) {
	GMT_LONG n_arc, k, pt;
	double az1, az2, d_az, da, xr, yr, da_try, *xx = NULL, *yy = NULL;

	/* When a polygon crosses out then in again into the circle we need to add a boundary arc
	 * to the polygon where it is clipped.  We simply sample the circle as finely as the arc
	 * length and the current line_step demands */

	da_try = (C->current.setting.map_line_step * 360.0) / (TWO_PI * C->current.proj.r);	/* Angular step in degrees */
	az1 = d_atan2d (end_y[0], end_x[0]);	/* azimuth from map center to 1st crossing */
	az2 = d_atan2d (end_y[1], end_x[1]);	/* azimuth from map center to 2nd crossing */
	d_az = az2 - az1;							/* Arc length in degrees */
	if (fabs(d_az) > 180.0) d_az = copysign (360.0 - fabs(d_az), -d_az);	/* Insist we take the short arc for now */
	n_arc = (GMT_LONG)ceil (fabs (d_az)/ da_try);	/* Get number of integer increments of da_try degree */
	da = d_az / (n_arc - 1);			/* Reset da to get exact steps */
	n_arc -= 2;	/* We do not include the end points since these are the crossing points handled in the calling function */
	if (n_arc <= 0) return (0);	/* Arc is too short to have intermediate points */
	(void) GMT_malloc2 (C, xx, yy, n_arc, 0, double);
	for (k = 1; k <= n_arc; k++) {	/* Create points along arc from first to second crossing point (k-loop excludes the end points) */
		sincosd (az1 + k * da, &yr, &xr);
		pt = (this) ? n_arc - k : k - 1;	/* The order we add the arc depends if we exited or entered the inside area */
		xx[pt] = C->current.proj.r * (1.0 + xr);
		yy[pt] = C->current.proj.r * (1.0 + yr);
	}

	*xarc = xx;
	*yarc = yy;
	return (n_arc);
}

#ifdef DEBUG
GMT_LONG clip_dump = 0, clip_id = 0;
void dumppol (GMT_LONG n, double *x, double *y, GMT_LONG *id);
#endif

GMT_LONG GMT_radial_clip (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG np, double **x, double **y, GMT_LONG *total_nx)
{
	GMT_LONG n = 0, this = FALSE, i, n_alloc = 0, n_arc, sides[4], nx, add_boundary = FALSE;
	double xlon[4], xlat[4], xc[4], yc[4], end_x[3], end_y[3], xr, yr;
	double *xx = NULL, *yy = NULL, *xarc = NULL, *yarc = NULL;

	*total_nx = 0;	/* Keep track of total of crossings */

	if (np == 0) return (0);

	if (!GMT_map_outside (C, lon[0], lat[0])) {
		n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
		GMT_geo_to_xy (C, lon[0], lat[0], &xx[0], &yy[0]);
		n++;
	}
	nx = 0;
	for (i = 1; i < np; i++) {
		this = GMT_map_outside (C, lon[i], lat[i]);
		if (GMT_map_crossing (C, lon[i-1], lat[i-1], lon[i], lat[i], xlon, xlat, xc, yc, sides)) {
			if (this) {	/* Crossing boundary and leaving circle: Add exit point to the path */
				if (n == n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
			end_x[nx] = xc[0] - C->current.proj.r;	end_y[nx] = yc[0] - C->current.proj.r;
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
				if ((n_arc = GMT_radial_boundary_arc (C, this, &end_x[nx-2], &end_y[nx-2], &xarc, &yarc)) > 0) {
					if ((n + n_arc) >= n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n + n_arc, n_alloc, double);
					GMT_memcpy (&xx[n], xarc, n_arc, double);	/* Copy longitudes of arc */
					GMT_memcpy (&yy[n], yarc, n_arc, double);	/* Copy latitudes of arc */
					n += n_arc;	/* Number of arc points added (end points are done separately) */
					GMT_free (C, xarc);	GMT_free (C,  yarc);
				}
				add_boundary = FALSE;
				nx -= 2;	/* Done with those two crossings */
			}
			if (!this) {	/* Crossing boundary and entering circle: Add entry point to the path */
				if (n == n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
				xx[n] = xc[0];	yy[n] = yc[0];	n++;
			}
		}
		GMT_geo_to_xy (C, lon[i], lat[i], &xr, &yr);
		if (!this) {	/* Only add points actually inside the map to the path */
			if (n == n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
			xx[n] = xr;	yy[n] = yr;	n++;
		}
	}

	if (nx == 2) {	/* Must close polygon by adding boundary arc */
		if ((n_arc = GMT_radial_boundary_arc (C, this, end_x, end_y, &xarc, &yarc)) > 0) {
			if ((n + n_arc) >= n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n + n_arc, n_alloc, double);
			GMT_memcpy (&xx[n], xarc, n_arc, double);	/* Copy longitudes of arc */
			GMT_memcpy (&yy[n], yarc, n_arc, double);	/* Copy latitudes of arc */
			n += n_arc;	/* Number of arc points added (end points are done separately) */
			GMT_free (C, xarc);	GMT_free (C,  yarc);
		}
		if (n == n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
		xx[n] = xx[0];	yy[n] = yy[0];	n++;	/* Close the polygon */
	}
	(void) GMT_malloc2 (C, xx, yy, 0, n, double);
	*x = xx;
	*y = yy;
#ifdef DEBUG
	if (clip_dump) dumppol (n, xx, yy, &clip_id);
#endif

	return (n);
}

GMT_LONG GMT_rect_overlap (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1)
{
	/* Return true if the projection of either (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular map boundary */
	double x0, y0, x1, y1;

	GMT_geo_to_xy (C, lon0, lat0, &x0, &y0);
	GMT_geo_to_xy (C, lon1, lat1, &x1, &y1);

	if (x0 > x1) d_swap (x0, x1);
	if (y0 > y1) d_swap (y0, y1);

	if (x1 - C->current.proj.rect[XLO] < -GMT_CONV_LIMIT || x0 - C->current.proj.rect[XHI] > GMT_CONV_LIMIT) return (FALSE);
	if (y1 - C->current.proj.rect[YLO] < -GMT_CONV_LIMIT || y0 - C->current.proj.rect[YHI] > GMT_CONV_LIMIT) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_wesn_overlap (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1)
{
	/* Return true if either of the points (lon0,lat0) and (lon1,lat1) is inside (not on) the rectangular lon/lat boundaries */
	if (lon0 > lon1) d_swap (lon0, lon1);
	if (lat0 > lat1) d_swap (lat0, lat1);
	if (lon1 - C->common.R.wesn[XLO] < -GMT_CONV_LIMIT) {
		lon0 += 360.0;
		lon1 += 360.0;
	}
	else if (lon0 - C->common.R.wesn[XHI] > GMT_CONV_LIMIT) {
		lon0 -= 360.0;
		lon1 -= 360.0;
	}

	if (lon1 - C->common.R.wesn[XLO] < -GMT_CONV_LIMIT || lon0 - C->common.R.wesn[XHI] > GMT_CONV_LIMIT) return (FALSE);
	if (lat1 - C->common.R.wesn[YLO] < -GMT_CONV_LIMIT || lat0 - C->common.R.wesn[YHI] > GMT_CONV_LIMIT) return (FALSE);
	return (TRUE);
}

GMT_LONG GMT_radial_overlap (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1)
{	/* Dummy routine */
	return (TRUE);
}

GMT_LONG GMT_genper_overlap (struct GMT_CTRL *C, double lon0, double lat0, double lon1, double lat1)
{
/* Dummy routine */
	if (C->current.proj.g_debug > 0) GMT_message (C, "genper_overlap: overlap called\n");
	return (TRUE);
}

void GMT_xy_search (struct GMT_CTRL *C, double *x0, double *x1, double *y0, double *y1, double w0, double e0, double s0, double n0)
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
		(*C->current.proj.fwd) (C, w, s0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*C->current.proj.fwd) (C, w, n0, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}
	for (j = 0; j <= 500; j++) {
		s = s0 + j * dlat;
		(*C->current.proj.fwd) (C, w0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
		(*C->current.proj.fwd) (C, e0, s, &x, &y);
		if (x < xmin) xmin = x;
		if (y < ymin) ymin = y;
		if (x > xmax) xmax = x;
		if (y > ymax) ymax = y;
	}

	*x0 = xmin;	*x1 = xmax;	*y0 = ymin;	*y1 = ymax;
}

void GMT_map_setxy (struct GMT_CTRL *C, double xmin, double xmax, double ymin, double ymax)
{	/* Set x/y parameters */

	C->current.proj.rect[XHI] = (xmax - xmin) * C->current.proj.scale[GMT_X];
	C->current.proj.rect[YHI] = (ymax - ymin) * C->current.proj.scale[GMT_Y];
	C->current.proj.origin[GMT_X] = -xmin * C->current.proj.scale[GMT_X];
	C->current.proj.origin[GMT_Y] = -ymin * C->current.proj.scale[GMT_Y];
}

void GMT_map_setinfo (struct GMT_CTRL *C, double xmin, double xmax, double ymin, double ymax, double scl)
{	/* Set [and rescale] parameters */
	double factor = 1.0, w, h;

	if (C->current.map.is_world && GMT_IS_ZERO (xmax - xmin)) {	/* Safety valve for cases when w & e both project to the same side due to round-off */
		xmax = MAX (fabs (xmin), fabs (xmax));
		xmin =-xmax;
	}
	w = (xmax - xmin) * C->current.proj.scale[GMT_X];
	h = (ymax - ymin) * C->current.proj.scale[GMT_Y];

	if (C->current.proj.gave_map_width == 1)		/* Must rescale to given width */
		factor = scl / w;
	else if (C->current.proj.gave_map_width == 2)	/* Must rescale to given height */
		factor = scl / h;
	else if (C->current.proj.gave_map_width == 3)	/* Must rescale to max dimension */
		factor = scl / MAX (w, h);
	else if (C->current.proj.gave_map_width == 4)	/* Must rescale to min dimension */
		factor = scl / MIN (w, h);
	C->current.proj.scale[GMT_X] *= factor;
	C->current.proj.scale[GMT_Y] *= factor;
	C->current.proj.w_r *= factor;

	if (C->current.proj.g_debug > 1) {
		GMT_message (C, "xmin %7.3f xmax %7.3f ymin %7.4f ymax %7.3f scale %6.3f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000, scl);
		GMT_message (C, "gave_map_width %ld w %9.4e h %9.4e factor %9.4e\n", C->current.proj.gave_map_width, w, h, factor);
	}

	GMT_map_setxy (C, xmin, xmax, ymin, ymax);
}

/* Compute mean radius r = (2a + b)/3 = a (1 - f/3) */
#define GMT_mean_radius(a, f) (a * (1.0 - f / 3.0))

void GMT_set_spherical (struct GMT_CTRL *C)
{
	/* Set up ellipsoid parameters using spherical approximation */

	C->current.setting.ref_ellipsoid[GMT_N_ELLIPSOIDS - 1].eq_radius =
		GMT_mean_radius (C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius, C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening);
	C->current.setting.proj_ellipsoid = GMT_N_ELLIPSOIDS - 1;	/* Custom ellipsoid */
	C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening = 0.0;
	GMT_report (C, GMT_MSG_NORMAL, "Warning: spherical approximation used!\n");

	GMT_init_ellipsoid (C);
}

double GMT_left_boundary (struct GMT_CTRL *C, double y)
{
	return ((*C->current.map.left_edge) (C, y));
}

double GMT_right_boundary (struct GMT_CTRL *C, double y)
{
	return ((*C->current.map.right_edge) (C, y));
}

double GMT_left_conic (struct GMT_CTRL *C, double y)
{
	double x_ws, y_ws, x_wn, y_wn, dy;

	GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &x_ws, &y_ws);
	GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI], &x_wn, &y_wn);
	dy = y_wn - y_ws;
	if (GMT_IS_ZERO (dy)) return (0.0);
	return (x_ws + ((x_wn - x_ws) * (y - y_ws) / dy));
}

double GMT_right_conic (struct GMT_CTRL *C, double y)
{
	double x_es, y_es, x_en, y_en, dy;

	GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO], &x_es, &y_es);
	GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &x_en, &y_en);
	dy = y_en - y_es;
	if (GMT_IS_ZERO (dy)) return (C->current.map.width);
	return (x_es - ((x_es - x_en) * (y - y_es) / dy));
}

double GMT_left_rect (struct GMT_CTRL *C, double y)
{
	return (0.0);
}

double GMT_right_rect (struct GMT_CTRL *C, double y)
{
	return (C->current.map.width);
}

double GMT_left_circle (struct GMT_CTRL *C, double y)
{
	/* y -= C->current.proj.r; */
	y -= C->current.proj.origin[GMT_Y];
	return (C->current.map.half_width - d_sqrt (C->current.proj.r * C->current.proj.r - y * y));
}

double GMT_right_circle (struct GMT_CTRL *C, double y)
{
	/* y -= C->current.proj.r; */
	y -= C->current.proj.origin[GMT_Y];
	return (C->current.map.half_width + d_sqrt (C->current.proj.r * C->current.proj.r - y * y));
}

double GMT_left_ellipse (struct GMT_CTRL *C, double y)
{
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - C->current.proj.origin[GMT_Y]) / C->current.proj.w_r;	/* Fraction, relative to Equator */
	return (C->current.map.half_width - 2.0 * C->current.proj.w_r * d_sqrt (1.0 - y * y));
}

double GMT_right_ellipse (struct GMT_CTRL *C, double y)
{
	/* Applies to Hammer and Mollweide only, where major axis = 2 * minor axis */

	y = (y - C->current.proj.origin[GMT_Y]) / C->current.proj.w_r;	/* Fraction, relative to Equator */
	return (C->current.map.half_width + 2.0 * C->current.proj.w_r * d_sqrt (1.0 - y * y));
}

void GMT_get_point_from_r_az (struct GMT_CTRL *C, double lon0, double lat0, double r, double azim, double *lon1, double *lat1)
/* Given point (lon0, lat0), find coordinates of a point r degrees away in the azim direction */
{
	double sinr, cosr, sinaz, cosaz, siny, cosy;

	sincosd (azim, &sinaz, &cosaz);
	sincosd (r, &sinr, &cosr);
	sincosd (lat0, &siny, &cosy);

	*lon1 = lon0 + atan2d (sinr * sinaz, (cosy * cosr - siny * sinr * cosaz));
	*lat1 = d_asind (siny * cosr + cosy * sinr * cosaz);
}

double GMT_az_backaz_cartesian (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
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
	az = (dx == 0.0 && dy == 0.0) ? C->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_cartesian_proj (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	/* Calculate azimuths or backazimuths.  Cartesian case.
	 * First point is considered "Event" and second "Station".
	 * Azimuth is direction from Station to Event.
	 * BackAzimuth is direction from Event to Station */

	double az, dx, dy, xE, yE, xS, yS;

	if (baz) {	/* exchange point one and two */
		d_swap (lonS, lonE);
		d_swap (latS, latE);
	}
	GMT_geo_to_xy (C, lonE, latE, &xE, &yE);
	GMT_geo_to_xy (C, lonS, latS, &xS, &yS);
	dx = xE - xS;
	dy = yE - yS;
	az = (dx == 0.0 && dy == 0.0) ? C->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_flatearth (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
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
	az = (dx == 0.0 && dy == 0.0) ? C->session.d_NaN : 90.0 - atan2d (dy, dx);
	if (az < 0.0) az += 360.0;
	return (az);
}

double GMT_az_backaz_sphere (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
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

double GMT_az_backaz_geodesic (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
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
	 * correction given by: 1-ECC2=1-2*f + f*f = C->current.proj.one_m_ECC2
	 */

	thg = atan (C->current.proj.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;
	g = -c * e;
	h = c * d;

	/* Calculating some trig constants. */

	thg = atan (C->current.proj.one_m_ECC2 * tand (latS));
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

double GMT_az_backaz (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz)
{
	return (C->current.map.azimuth_func (C, lonE, latE, lonS, latS, baz));
}

double GMT_cartesian_dist (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);
double GMT_cartesian_dist_proj (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
double GMT_flatearth_dist_degree (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);
double GMT_flatearth_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);
double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
double GMT_great_circle_dist_cos(struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
GMT_LONG GMT_near_lines_cartesian (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
GMT_LONG GMT_near_a_line_cartesian (struct GMT_CTRL *C, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
GMT_LONG GMT_near_a_point_cartesian (struct GMT_CTRL *C, double x, double y, struct GMT_TABLE *T, double dist);
GMT_LONG GMT_near_lines_spherical (struct GMT_CTRL *P, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
GMT_LONG GMT_near_a_line_spherical (struct GMT_CTRL *P, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
GMT_LONG GMT_near_a_point_spherical (struct GMT_CTRL *C, double x, double y, struct GMT_TABLE *T, double dist);
void GMT_set_distaz (struct GMT_CTRL *C, GMT_LONG mode, GMT_LONG type);
GMT_LONG GMT_great_circle_intersection (struct GMT_CTRL *T, double A[], double B[], double C[], double X[], double *CX_dist);

EXTERN_MSC double GMT_get_angle (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);

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

/*
 *	GENERIC TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION
 */

double GMT_x_to_xx (struct GMT_CTRL *C, double x)
{	/* Converts x to xx using the current linear projection */
	double xx;
	(*C->current.proj.fwd_x) (C, x, &xx);
	return (xx * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_y_to_yy (struct GMT_CTRL *C, double y)
{	/* Converts y to yy using the current linear projection */
	double yy;
	(*C->current.proj.fwd_y) (C, y, &yy);
	return (yy * C->current.proj.scale[GMT_Y] + C->current.proj.origin[GMT_Y]);
}

double GMT_z_to_zz (struct GMT_CTRL *C, double z)
{	/* Converts z to zz using the current linear projection */
	double zz;
	(*C->current.proj.fwd_z) (C, z, &zz);
	return (zz * C->current.proj.scale[GMT_Z] + C->current.proj.origin[GMT_Z]);
}

double GMT_xx_to_x (struct GMT_CTRL *C, double xx)
{	/* Converts xx back to x using the current linear projection */
	double x;
	xx = (xx - C->current.proj.origin[GMT_X]) * C->current.proj.i_scale[GMT_X];
	(*C->current.proj.inv_x) (C, &x, xx);
	return (x);
}

double GMT_yy_to_y (struct GMT_CTRL *C, double yy)
{	/* Converts yy back to y using the current linear projection */
	double y;
	yy = (yy - C->current.proj.origin[GMT_Y]) * C->current.proj.i_scale[GMT_Y];
	(*C->current.proj.inv_y) (C, &y, yy);
	return (y);
}

double GMT_zz_to_z (struct GMT_CTRL *C, double zz)
{	/* Converts zz back to z using the current linear projection */
	double z;
	zz = (zz - C->current.proj.origin[GMT_Z]) * C->current.proj.i_scale[GMT_Z];
	(*C->current.proj.inv_z) (C, &z, zz);
	return (z);
}

GMT_LONG GMT_geo_to_xy (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Converts lon/lat to x/y using the current projection */
	if (GMT_is_dnan (lon) || GMT_is_dnan (lat)) {(*x) = (*y) = C->session.d_NaN; return TRUE;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	(*C->current.proj.fwd) (C, lon, lat, x, y);
	(*x) = (*x) * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X];
	(*y) = (*y) * C->current.proj.scale[GMT_Y] + C->current.proj.origin[GMT_Y];
	return FALSE;
}

void GMT_xy_to_geo (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{
	/* Converts x/y to lon/lat using the current projection */

	if (GMT_is_dnan (x) || GMT_is_dnan (y)) {(*lon) = (*lat) = C->session.d_NaN; return;}	/* Quick and safe way to ensure NaN-input results in NaNs */
	x = (x - C->current.proj.origin[GMT_X]) * C->current.proj.i_scale[GMT_X];
	y = (y - C->current.proj.origin[GMT_Y]) * C->current.proj.i_scale[GMT_Y];

	(*C->current.proj.inv) (C, lon, lat, x, y);
}

void GMT_geoz_to_xy (struct GMT_CTRL *C, double x, double y, double z, double *x_out, double *y_out)
{	/* Map-projects xy first, the projects xyz onto xy plane */
	double x0, y0;
	GMT_geo_to_xy (C, x, y, &x0, &y0);
	GMT_xyz_to_xy (C, x0, y0, GMT_z_to_zz (C, z), x_out, y_out);
}

void GMT_xyz_to_xy (struct GMT_CTRL *C, double x, double y, double z, double *x_out, double *y_out)
{	/* projects xyz (inches) onto perspective xy plane (inches) */
	*x_out = - x * C->current.proj.z_project.cos_az + y * C->current.proj.z_project.sin_az + C->current.proj.z_project.x_off;
	*y_out = - (x * C->current.proj.z_project.sin_az + y * C->current.proj.z_project.cos_az) * C->current.proj.z_project.sin_el + z * C->current.proj.z_project.cos_el + C->current.proj.z_project.y_off;
}

void GMT_xyz_to_xy_n (struct GMT_CTRL *C, double *x, double *y, double z, GMT_LONG n)
{	/* projects xyz (inches) onto perspective xy plane (inches) for multiple points */
	GMT_LONG i;
	for (i = 0; i < n; i++) GMT_xyz_to_xy (C, x[i], y[i], z, &x[i], &y[i]);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LINEAR PROJECTION (GMT_LINEAR)
 */

void GMT_linearxy (struct GMT_CTRL *C, double x, double y, double *x_i, double *y_i)
{	/* Transform both x and y linearly */
	(*C->current.proj.fwd_x) (C, x, x_i);
	(*C->current.proj.fwd_y) (C, y, y_i);
}

void GMT_ilinearxy (struct GMT_CTRL *C, double *x, double *y, double x_i, double y_i)
{	/* Inversely transform both x and y linearly */
	(*C->current.proj.inv_x) (C, x, x_i);
	(*C->current.proj.inv_y) (C, y, y_i);
}

GMT_LONG GMT_map_init_linear (struct GMT_CTRL *C) {
	GMT_LONG positive;
	double xmin, xmax, ymin = 0.0, ymax = 0.0;

	C->current.map.left_edge  = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.proj.fwd = (PFL) GMT_linearxy;
	C->current.proj.inv = (PFL) GMT_ilinearxy;
	if (GMT_x_is_lon (C, GMT_IN)) {	/* x is longitude */
		C->current.proj.central_meridian = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
		C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	}
	else
		C->current.map.lon_wrap = FALSE;
	C->current.proj.scale[GMT_X] = C->current.proj.pars[0];
	C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];
	if (C->current.proj.scale[GMT_X] < 0.0) C->current.proj.xyz_pos[GMT_X] = FALSE;	/* User wants x to increase left */
	if (C->current.proj.scale[GMT_Y] < 0.0) C->current.proj.xyz_pos[GMT_Y] = FALSE;	/* User wants y to increase down */
	switch ( (C->current.proj.xyz_projection[GMT_X]%3)) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			C->current.proj.fwd_x = (PFL) ((GMT_x_is_lon (C, GMT_IN)) ? GMT_translind : GMT_translin);
			C->current.proj.inv_x = (PFL) ((GMT_x_is_lon (C, GMT_IN)) ? GMT_itranslind : GMT_itranslin);
			if (C->current.proj.xyz_pos[GMT_X]) {
				(*C->current.proj.fwd_x) (C, C->common.R.wesn[XLO], &xmin);
				(*C->current.proj.fwd_x) (C, C->common.R.wesn[XHI], &xmax);
			}
			else {
				(*C->current.proj.fwd_x) (C, C->common.R.wesn[XHI], &xmin);
				(*C->current.proj.fwd_x) (C, C->common.R.wesn[XLO], &xmax);
			}
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (C->common.R.wesn[XLO] <= 0.0 || C->common.R.wesn[XHI] <= 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Jx option:  Limits must be positive for log10 option\n");
				GMT_exit (EXIT_FAILURE);
			}
			xmin = (C->current.proj.xyz_pos[GMT_X]) ? d_log10 (C, C->common.R.wesn[XLO]) : d_log10 (C, C->common.R.wesn[XHI]);
			xmax = (C->current.proj.xyz_pos[GMT_X]) ? d_log10 (C, C->common.R.wesn[XHI]) : d_log10 (C, C->common.R.wesn[XLO]);
			C->current.proj.fwd_x = (PFL) GMT_translog10;
			C->current.proj.inv_x = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			C->current.proj.xyz_pow[GMT_X] = C->current.proj.pars[2];
			C->current.proj.xyz_ipow[GMT_X] = 1.0 / C->current.proj.pars[2];
			positive = !(((GMT_LONG)C->current.proj.xyz_pos[GMT_X] + (GMT_LONG)(C->current.proj.xyz_pow[GMT_X] > 0.0)) % 2);
			xmin = (positive) ? pow (C->common.R.wesn[XLO], C->current.proj.xyz_pow[GMT_X]) : pow (C->common.R.wesn[XHI], C->current.proj.xyz_pow[GMT_X]);
			xmax = (positive) ? pow (C->common.R.wesn[XHI], C->current.proj.xyz_pow[GMT_X]) : pow (C->common.R.wesn[XLO], C->current.proj.xyz_pow[GMT_X]);
			C->current.proj.fwd_x = (PFL) GMT_transpowx;
			C->current.proj.inv_x = (PFL) GMT_itranspowx;
			break;
	}
	switch (C->current.proj.xyz_projection[GMT_Y]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			ymin = (C->current.proj.xyz_pos[GMT_Y]) ? C->common.R.wesn[YLO] : C->common.R.wesn[YHI];
			ymax = (C->current.proj.xyz_pos[GMT_Y]) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO];
			C->current.proj.fwd_y = (PFL) GMT_translin;
			C->current.proj.inv_y = (PFL) GMT_itranslin;
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (C->common.R.wesn[YLO] <= 0.0 || C->common.R.wesn[YHI] <= 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error -Jx option:  Limits must be positive for log10 option\n");
				GMT_exit (EXIT_FAILURE);
			}
			ymin = (C->current.proj.xyz_pos[GMT_Y]) ? d_log10 (C, C->common.R.wesn[YLO]) : d_log10 (C, C->common.R.wesn[YHI]);
			ymax = (C->current.proj.xyz_pos[GMT_Y]) ? d_log10 (C, C->common.R.wesn[YHI]) : d_log10 (C, C->common.R.wesn[YLO]);
			C->current.proj.fwd_y = (PFL) GMT_translog10;
			C->current.proj.inv_y = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			C->current.proj.xyz_pow[GMT_Y] = C->current.proj.pars[3];
			C->current.proj.xyz_ipow[GMT_Y] = 1.0 / C->current.proj.pars[3];
			positive = !(((GMT_LONG)C->current.proj.xyz_pos[GMT_Y] + (GMT_LONG)(C->current.proj.xyz_pow[GMT_Y] > 0.0)) % 2);
			ymin = (positive) ? pow (C->common.R.wesn[YLO], C->current.proj.xyz_pow[GMT_Y]) : pow (C->common.R.wesn[YHI], C->current.proj.xyz_pow[GMT_Y]);
			ymax = (positive) ? pow (C->common.R.wesn[YHI], C->current.proj.xyz_pow[GMT_Y]) : pow (C->common.R.wesn[YLO], C->current.proj.xyz_pow[GMT_Y]);
			C->current.proj.fwd_y = (PFL) GMT_transpowy;
			C->current.proj.inv_y = (PFL) GMT_itranspowy;
	}

	/* Was given axes length instead of scale? */

	if (C->current.proj.compute_scale[GMT_X]) C->current.proj.scale[GMT_X] /= fabs (xmin - xmax);
	if (C->current.proj.compute_scale[GMT_Y]) C->current.proj.scale[GMT_Y] /= fabs (ymin - ymax);

	/* If either is zero, adjust width or height to the other */

	if (C->current.proj.scale[GMT_X] == 0) {
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y];
		C->current.proj.pars[0] = C->current.proj.scale[GMT_X] * fabs (xmin - xmax);
	}
	if (C->current.proj.scale[GMT_Y] == 0) {
		C->current.proj.scale[GMT_Y] = C->current.proj.scale[GMT_X];
		C->current.proj.pars[1] = C->current.proj.scale[GMT_Y] * fabs (ymin - ymax);
	}

	/* This override ensures that when using -J[x|X]...d degrees work as meters */

	C->current.proj.M_PR_DEG = 1.0;
	C->current.proj.KM_PR_DEG = C->current.proj.M_PR_DEG / METERS_IN_A_KM;

	GMT_map_setxy (C, xmin, xmax, ymin, ymax);
	C->current.map.outside = (PFL) GMT_rect_outside;
	C->current.map.crossing = (PFL) GMT_rect_crossing;
	C->current.map.overlap = (PFL) GMT_rect_overlap;
	C->current.map.clip = (PFL) GMT_rect_clip;
	C->current.map.frame.check_side = TRUE;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);
}

/*
 *	TRANSFORMATION ROUTINES FOR POLAR (theta,r) PROJECTION (GMT_POLAR)
 */

GMT_LONG GMT_map_init_polar (struct GMT_CTRL *C)
{
	double xmin, xmax, ymin, ymax;

	GMT_vpolar (C, C->current.proj.pars[1]);
	if (C->current.proj.got_elevations) {	/* Requires s >= 0 and n <= 90 */
		if (C->common.R.wesn[YLO] < 0.0 || C->common.R.wesn[YHI] > 90.0) {
			GMT_report (C, GMT_MSG_FATAL, "Error: -JP...r for elevation plots requires s >= 0 and n <= 90!\n");
			GMT_exit (EXIT_FAILURE);
		}
		if (GMT_IS_ZERO (90.0 - C->common.R.wesn[YHI])) C->current.proj.edge[2] = FALSE;
	}
	else {
		if (GMT_IS_ZERO (C->common.R.wesn[YLO])) C->current.proj.edge[0] = FALSE;
	}
	if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
	C->current.map.left_edge = (PFD) GMT_left_circle;
	C->current.map.right_edge = (PFD) GMT_right_circle;
	C->current.proj.fwd = (PFL) GMT_polar;
	C->current.proj.inv = (PFL) GMT_ipolar;
	C->current.map.is_world = FALSE;	/* There is no wrapping around here */
	GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[0];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[0]);
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);

	/* C->current.proj.r = 0.5 * C->current.proj.rect[XHI]; */
	C->current.proj.r = C->current.proj.scale[GMT_Y] * C->common.R.wesn[YHI];
	C->current.map.outside = (PFL) GMT_polar_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.frame.horizontal = TRUE;
	if (!C->current.proj.got_elevations) C->current.plot.r_theta_annot = TRUE;	/* Special labeling case (see GMT_get_annot_label) */
	C->current.map.n_lat_nodes = 2;
	C->current.map.meridian_straight = TRUE;

	return (FALSE);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MERCATOR PROJECTION (GMT_MERCATOR)
 */

GMT_LONG GMT_map_init_merc (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, D = 1.0;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) {	/* Set fudge factor */
		GMT_scale_eqrad (C);
		D = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius / C->current.proj.GMT_lat_swap_vals.rm;
	}
	if (C->common.R.wesn[YLO] <= -90.0 || C->common.R.wesn[YHI] >= 90.0) {
		GMT_report (C, GMT_MSG_FATAL, "Syntax error -R option:  Cannot include south/north poles with Mercator projection!\n");
		GMT_exit (EXIT_FAILURE);
	}
	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	GMT_vmerc (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	C->current.proj.j_x *= D;
	C->current.proj.j_ix /= D;
	C->current.proj.fwd = (PFL) GMT_merc_sph;
	C->current.proj.inv = (PFL) GMT_imerc_sph;
	(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
	(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= (D * C->current.proj.M_PR_DEG);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	C->current.map.outside = (PFL) GMT_wesn_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.frame.check_side = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUAL-AREA PROJECTIONS (GMT_CYL_EQ)
 */

GMT_LONG GMT_map_init_cyleq (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.Dx = C->current.proj.Dy = 0.0;
	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) {
		double D, k0, qp, slat, e, e2;
		GMT_scale_eqrad (C);
		slat = C->current.proj.pars[1];
		C->current.proj.pars[1] = GMT_latg_to_lata (C, C->current.proj.pars[1]);
		e = C->current.proj.ECC;
		e2 = C->current.proj.ECC2;
		qp = 1.0 - 0.5 * (1.0 - e2) * log ((1.0 - e) / (1.0 + e)) / e;
		k0 = cosd (slat) / d_sqrt (1.0 - e2 * sind (C->current.proj.pars[1]) * sind (C->current.proj.pars[1]));
		D = k0 / cosd (C->current.proj.pars[1]);
		C->current.proj.Dx = D;
		C->current.proj.Dy = 0.5 * qp / D;
	}
	C->current.proj.iDx = 1.0 / C->current.proj.Dx;
	C->current.proj.iDy = 1.0 / C->current.proj.Dy;
	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	GMT_vcyleq (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	GMT_cyleq (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
	GMT_cyleq (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	C->current.proj.fwd = (PFL) GMT_cyleq;
	C->current.proj.inv = (PFL) GMT_icyleq;
	C->current.map.outside = (PFL) GMT_wesn_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.frame.check_side = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL EQUIDISTANT PROJECTION (GMT_CYL_EQDIST)
 */

GMT_LONG GMT_map_init_cyleqdist (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	GMT_vcyleqdist (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	GMT_cyleqdist (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
	GMT_cyleqdist (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	C->current.proj.fwd = (PFL) GMT_cyleqdist;
	C->current.proj.inv = (PFL) GMT_icyleqdist;
	C->current.map.outside = (PFL) GMT_wesn_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.frame.check_side = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR MILLER CYLINDRICAL PROJECTION (GMT_MILLER)
 */

GMT_LONG GMT_map_init_miller (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	GMT_vmiller (C, C->current.proj.pars[0]);
	GMT_miller (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
	GMT_miller (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	C->current.proj.fwd = (PFL) GMT_miller;
	C->current.proj.inv = (PFL) GMT_imiller;
	C->current.map.outside = (PFL) GMT_wesn_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.frame.check_side = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}

/*
 *	TRANSFORMATION ROUTINES FOR CYLINDRICAL STEREOGRAPHIC PROJECTIONS (GMT_CYL_STEREO)
 */

GMT_LONG GMT_map_init_cylstereo (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	GMT_vcylstereo (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	GMT_cylstereo (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
	GMT_cylstereo (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.n_lon_nodes = 3;	/* > 2 to avoid map-jumps */
	C->current.proj.fwd = (PFL) GMT_cylstereo;
	C->current.proj.inv = (PFL) GMT_icylstereo;
	C->current.map.outside = (PFL) GMT_wesn_outside;
	C->current.map.crossing = (PFL) GMT_wesn_crossing;
	C->current.map.overlap = (PFL) GMT_wesn_overlap;
	C->current.map.clip = (PFL) GMT_wesn_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;
	C->current.map.frame.horizontal = TRUE;
	C->current.map.frame.check_side = TRUE;
	C->current.map.meridian_straight = C->current.map.parallel_straight = TRUE;

	return (FALSE);	/* No need to search for wesn */
}


/*
 *	TRANSFORMATION ROUTINES FOR THE POLAR STEREOGRAPHIC PROJECTION (GMT_STEREO)
 */

GMT_LONG GMT_map_init_stereo (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dummy, radius, latg, D = 1.0;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	latg = C->current.proj.pars[1];

	GMT_set_polar (C);

	if (C->current.setting.proj_scale_factor == -1.0) C->current.setting.proj_scale_factor = 0.9996;	/* Select default map scale for Stereographic */
	if (C->current.proj.polar && (irint (C->current.proj.pars[5]) == 1)) C->current.setting.proj_scale_factor = 1.0;	/* Gave true scale at given parallel set below */
	/* Equatorial view has a problem with infinite loops.  Until I find a cure
	  we set projection center latitude to 0.001 so equatorial works for now */

	if (fabs (C->current.proj.pars[1]) < GMT_SMALL) C->current.proj.pars[1] = 0.001;

	GMT_vstereo (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);

	if (C->current.proj.GMT_convert_latitudes) {	/* Set fudge factors when conformal latitudes are used */
		double e1p, e1m, s, c;

		D = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius / C->current.proj.GMT_lat_swap_vals.rm;
		if (C->current.proj.polar) {
			e1p = 1.0 + C->current.proj.ECC;	e1m = 1.0 - C->current.proj.ECC;
			D /= d_sqrt (pow (e1p, e1p) * pow (e1m, e1m));
			if (irint (C->current.proj.pars[5]) == 1) {	/* Gave true scale at given parallel */
				double k_p, m_c, t_c, es;

				sincosd (fabs (C->current.proj.pars[4]), &s, &c);
				es = C->current.proj.ECC * s;
				m_c = c / d_sqrt (1.0 - C->current.proj.ECC2 * s * s);
				t_c = d_sqrt (((1.0 - s) / (1.0 + s)) * pow ((1.0 + es) / (1.0 - es), C->current.proj.ECC));
				k_p = 0.5 * m_c * d_sqrt (pow (e1p, e1p) * pow (e1m, e1m)) / t_c;
				D *= k_p;
			}
		}
		else {
			sincosd (latg, &s, &c);	/* Need original geographic pole coordinates */
			D *= (c / (C->current.proj.cosp * d_sqrt (1.0 - C->current.proj.ECC2 * s * s)));
		}
	}
	C->current.proj.Dx = C->current.proj.Dy = D;

	C->current.proj.iDx = 1.0 / C->current.proj.Dx;
	C->current.proj.iDy = 1.0 / C->current.proj.Dy;

	if (C->current.proj.polar) {	/* Polar aspect */
		C->current.proj.fwd = (PFL) GMT_plrs_sph;
		C->current.proj.inv = (PFL) GMT_iplrs_sph;
		if (C->current.proj.units_pr_degree) {
			(*C->current.proj.fwd) (C, C->current.proj.pars[0], C->current.proj.pars[4], &dummy, &radius);
			C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
		}
		else
			C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];
		C->current.map.meridian_straight = TRUE;
	}
	else {
		C->current.proj.fwd = (GMT_IS_ZERO (C->current.proj.pole)) ? (PFL) GMT_stereo2_sph : (PFL) GMT_stereo1_sph;
		C->current.proj.inv = (PFL) GMT_istereo_sph;
		if (C->current.proj.units_pr_degree) {
			GMT_vstereo (C, 0.0, 90.0, C->current.proj.pars[2]);
			(*C->current.proj.fwd) (C, 0.0, fabs(C->current.proj.pars[4]), &dummy, &radius);
			C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
		}
		else
			C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];

		GMT_vstereo (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);
	}


	if (C->common.R.oblique) {	/* Rectangular box given */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 30.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
	}
	else {
		if (C->current.proj.polar) {	/* Polar aspect */
			if (C->current.proj.north_pole) {
				if (C->common.R.wesn[YLO] <= -90.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error: South boundary cannot be -90.0 for north polar stereographic projection\n");
					GMT_exit (EXIT_FAILURE);
				}
				if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
			}
			else {
				if (C->common.R.wesn[YHI] >= 90.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error: North boundary cannot be +90.0 for south polar stereographic projection\n");
					GMT_exit (EXIT_FAILURE);
				}
				if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
			C->current.map.outside = (PFL) GMT_polar_outside;
			C->current.map.crossing = (PFL) GMT_wesn_crossing;
			C->current.map.overlap = (PFL) GMT_wesn_overlap;
			C->current.map.clip = (PFL) GMT_wesn_clip;
			C->current.map.frame.horizontal = TRUE;
			C->current.map.n_lat_nodes = 2;
			GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			C->current.map.frame.axis[GMT_X].item[0].interval = C->current.map.frame.axis[GMT_Y].item[0].interval = 0.0;	/* No annotations for global mode */
			C->current.map.frame.axis[GMT_X].item[4].interval = C->current.map.frame.axis[GMT_Y].item[4].interval = 0.0;	/* No tickmarks for global mode */
			C->common.R.wesn[XLO] = 0.0;
			C->common.R.wesn[XHI] = 360.0;
			C->common.R.wesn[YLO] = -90.0;
			C->common.R.wesn[YHI] = 90.0;
			xmax = ymax = C->current.proj.rho_max;
			xmin = ymin = -xmax;
			C->current.map.outside = (PFL) GMT_radial_outside;
			C->current.map.crossing = (PFL) GMT_radial_crossing;
			C->current.map.overlap = (PFL) GMT_radial_overlap;
			C->current.map.clip = (PFL) GMT_radial_clip;
			if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[3]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT CONFORMAL CONIC PROJECTION (GMT_LAMBERT)
 */

GMT_LONG GMT_map_init_lambert (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.GMT_convert_latitudes = GMT_quickconic (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	GMT_vlamb (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2], C->current.proj.pars[3]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[4] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[4];
	if (GMT_IS_SPHERICAL (C) || C->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		C->current.proj.fwd = (PFL) GMT_lamb_sph;
		C->current.proj.inv = (PFL) GMT_ilamb_sph;
	}
	else {
		C->current.proj.fwd = (PFL) GMT_lamb;
		C->current.proj.inv = (PFL) GMT_ilamb;
	}

	if (C->common.R.oblique) {	/* Rectangular box given*/
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_conic;
		C->current.map.right_edge = (PFD) GMT_right_conic;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[4]);
	C->current.map.n_lat_nodes = 2;
	C->current.map.frame.horizontal = TRUE;
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE OBLIQUE MERCATOR PROJECTION (GMT_OBLIQUE_MERC)
 */

void GMT_pole_rotate_forward (struct GMT_CTRL *C, double lon, double lat, double *tlon, double *tlat)
{
	/* Given the pole position in C->current.proj, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_lat, cos_lat, cos_lon, sin_lon, cc;

	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon - C->current.proj.o_pole_lon, &sin_lon, &cos_lon);
	cc = cos_lat * cos_lon;
	*tlat = d_asind (C->current.proj.o_sin_pole_lat * sin_lat + C->current.proj.o_cos_pole_lat * cc);
	*tlon = C->current.proj.o_beta + d_atan2d (cos_lat * sin_lon, C->current.proj.o_sin_pole_lat * cc - C->current.proj.o_cos_pole_lat * sin_lat);
}

#if 0
/* Not curently used in GMT */
void GMT_pole_rotate_inverse (struct GMT_CTRL *C, double *lon, double *lat, double tlon, double tlat)
{
	/* Given the pole position in C->current.proj, geographical coordinates
	 * are computed from oblique coordinates assuming a spherical earth.
	 * Latitutes and longitudes are in degrees.
	 */

	double sin_tlat, cos_tlat, cos_tlon, sin_tlon, cc;

	sincosd (tlat, &sin_tlat, &cos_tlat);
	sincosd (tlon - C->current.proj.o_beta, &sin_tlon, &cos_tlon);
	cc = cos_tlat * cos_tlon;
	*lat = d_asind (C->current.proj.o_sin_pole_lat * sin_tlat - C->current.proj.o_cos_pole_lat * cc);
	*lon = C->current.proj.o_pole_lon + d_atan2d (cos_tlat * sin_tlon, C->current.proj.o_sin_pole_lat * cc + C->current.proj.o_cos_pole_lat * sin_tlat);
}
#endif

void GMT_get_origin (struct GMT_CTRL *C, double lon1, double lat1, double lon_p, double lat_p, double *lon2, double *lat2)
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
		GMT_report (C, GMT_MSG_NORMAL, "Warning: Correct projection origin = %g/%g\n", *lon2, *lat2);
	}
	else {
		*lon2 = lon1;
		*lat2 = lat1;
	}

	GMT_pole_rotate_forward (C, *lon2, *lat2, &beta, &dummy);

	C->current.proj.o_beta = -beta;
}

void GMT_get_rotate_pole (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2)
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
	C->current.proj.o_pole_lon = plon;
	C->current.proj.o_pole_lat = plat;
	sincosd (plat, &C->current.proj.o_sin_pole_lat, &C->current.proj.o_cos_pole_lat);
	GMT_pole_rotate_forward (C, lon1, lat1, &beta, &dummy);
	C->current.proj.o_beta = -beta;
}

GMT_LONG GMT_map_init_oblique (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;
	double o_x, o_y, p_x, p_y, c_x, c_y, c, az, b_x, b_y, w, e, s, n, P[3];

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	if (C->current.proj.units_pr_degree) C->current.proj.pars[4] /= C->current.proj.M_PR_DEG;	/* To get plot-units / m */

	o_x = C->current.proj.pars[0];	o_y = C->current.proj.pars[1];

	if (irint (C->current.proj.pars[6]) == 1) {	/* Must get correct origin, then get second point */
		p_x = C->current.proj.pars[2];	p_y = C->current.proj.pars[3];

		C->current.proj.o_pole_lon = p_x;
		C->current.proj.o_pole_lat = p_y;
		C->current.proj.o_sin_pole_lat = sind (p_y);
		C->current.proj.o_cos_pole_lat = cosd (p_y);

		/* Find azimuth to pole, add 90, and compute second point 10 degrees away */

		GMT_get_origin (C, o_x, o_y, p_x, p_y, &o_x, &o_y);
		az = atand (cosd (p_y) * sind (p_x - o_x) / (cosd (o_y) * sind (p_y) - sind (o_y) * cosd (p_y) * cosd (p_x - o_x))) + 90.0;
		c = 10.0;	/* compute point 10 degrees from origin along azimuth */
		b_x = o_x + atand (sind (c) * sind (az) / (cosd (o_y) * cosd (c) - sind (o_y) * sind (c) * cosd (az)));
		b_y = d_asind (sind (o_y) * cosd (c) + cosd (o_y) * sind (c) * cosd (az));
		C->current.proj.pars[0] = o_x;	C->current.proj.pars[1] = o_y;
		C->current.proj.pars[2] = b_x;	C->current.proj.pars[3] = b_y;
	}
	else {	/* Just find pole */
		b_x = C->current.proj.pars[2];	b_y = C->current.proj.pars[3];
		GMT_get_rotate_pole (C, o_x, o_y, b_x, b_y);
	}

	/* Here we have pole and origin */

	/* Get forward pole and origin vectors FP, FC */
	GMT_geo_to_cart (C, C->current.proj.o_pole_lat, C->current.proj.o_pole_lon, C->current.proj.o_FP, TRUE);
	GMT_geo_to_cart (C, o_y, o_x, P, TRUE);	/* P points to origin  */
	GMT_cross3v (C, C->current.proj.o_FP, P, C->current.proj.o_FC);
	GMT_normalize3v (C, C->current.proj.o_FC);

	/* Get inverse pole and origin vectors FP, FC */
	GMT_obl (C, 0.0, M_PI_2, &p_x, &p_y);
	GMT_geo_to_cart (C, p_y, p_x, C->current.proj.o_IP, FALSE);
	GMT_obl (C, 0.0, 0.0, &c_x, &c_y);
	GMT_geo_to_cart (C, c_y, c_x, P, FALSE);	/* P points to origin  */
	GMT_cross3v (C, C->current.proj.o_IP, P, C->current.proj.o_IC);
	GMT_normalize3v (C, C->current.proj.o_IC);

	GMT_vmerc (C, 0.0, 0.0);

	if (C->common.R.oblique) {	/* wesn is lower left and upper right corners in normal lon/lats */
		GMT_oblmrc (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_oblmrc (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
	}
	else {	/* Gave oblique degrees */
		/* Convert oblique wesn in degrees to meters using regular Mercator */
		if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI])) {
			C->common.R.wesn[XLO] = -180.0;
			C->common.R.wesn[XHI] = +180.0;
		}
		GMT_merc_sph (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_merc_sph (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->common.R.oblique = TRUE;	/* Since wesn was oblique, not geographical wesn */
	}	

	GMT_imerc_sph (C, &w, &s, xmin, ymin);	/* Get oblique wesn boundaries */
	GMT_imerc_sph (C, &e, &n, xmax, ymax);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[4];
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[4]);
	C->current.proj.fwd = (PFL) GMT_oblmrc;
	C->current.proj.inv = (PFL) GMT_ioblmrc;
	C->current.map.outside = (PFL) GMT_rect_outside;
	C->current.map.crossing = (PFL) GMT_rect_crossing;
	C->current.map.overlap = (PFL) GMT_rect_overlap;
	C->current.map.clip = (PFL) GMT_rect_clip;
	C->current.map.left_edge = (PFD) GMT_left_rect;
	C->current.map.right_edge = (PFD) GMT_right_rect;

	C->current.map.is_world = GMT_360_RANGE (w, e);
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
	return (TRUE);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE TRANSVERSE MERCATOR PROJECTION (GMT_TM)
 */

/* For global TM maps */

GMT_LONG GMT_wrap_around_check_tm (struct GMT_CTRL *C, double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides)
{
	double dx, dy, width, jump;

	jump = this_y - last_y;
	width = 0.5 * C->current.map.height;

	if (fabs (jump) - width <= GMT_SMALL || fabs(jump) <= GMT_SMALL) return (0);
	dx = this_x - last_x;

	if (jump < -width) {	/* Crossed top boundary */
		dy = C->current.map.height + jump;
		xx[0] = xx[1] = last_x + (C->current.map.height - last_y) * dx / dy;
		if (xx[0] < 0.0 || xx[0] > C->current.proj.rect[XHI]) return (0);
		yy[0] = C->current.map.height;	yy[1] = 0.0;
		sides[0] = 2;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed bottom boundary */
		dy = C->current.map.height - jump;
		xx[0] = xx[1] = last_x + last_y * dx / dy;
		if (xx[0] < 0.0 || xx[0] > C->current.proj.rect[XHI]) return (0);
		yy[0] = 0.0;	yy[1] = C->current.map.height;
		sides[0] = 0;
		angle[0] = d_atan2d (dy, -dx);
	}
	angle[1] = angle[0] + 180.0;
	sides[1] = 2 - sides[0];

	return (2);
}

GMT_LONG GMT_this_point_wraps_tm (struct GMT_CTRL *C, double y0, double y1)
{
	/* Returns TRUE if the 2 y-points implies a jump at this x-level of the TM map */

	double dy;

	return ((dy = fabs (y1 - y0)) > C->current.map.half_height);
}

GMT_LONG GMT_will_it_wrap_tm (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG *start)
{	/* Determines if a polygon will wrap at edges for TM global projection */
	GMT_LONG i, wrap;

	if (!C->current.map.is_world) return (FALSE);

	for (i = 1, wrap = FALSE; !wrap && i < n; i++) {
		wrap = GMT_this_point_wraps_tm (C, y[i-1], y[i]);
	}
	*start = i - 1;
	return (wrap);
}

void GMT_get_crossings_tm (struct GMT_CTRL *C, double *xc, double *yc, double x0, double y0, double x1, double y1)
{	/* Finds crossings for wrap-arounds for global TM maps */
	double xa, xb, ya, yb, dy, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (ya > yb) {	/* Make A the minimum y point */
		d_swap (xa, xb);
		d_swap (ya, yb);
	}

	yb -= C->current.map.height;

	dy = ya - yb;
	c = (GMT_IS_ZERO (dy)) ? 0.0 : (xa - xb) / dy;
	xc[0] = xc[1] = xb - yb * c;
	if (y0 > y1) {	/* First cut top */
		yc[0] = C->current.map.height;
		yc[1] = 0.0;
	}
	else {
		yc[0] = 0.0;
		yc[1] = C->current.map.height;
	}
}

GMT_LONG GMT_map_jump_tm (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{
	/* TRUE if y-distance between points exceeds 1/2 map height at this x value */
	/* Only used for TM world maps */

	double dy;

	dy = y1 - y0;
	if (dy > C->current.map.half_height) return (-1);	/* Cross bottom/south boundary */
	if (dy < (-C->current.map.half_height)) return (1);	/* Cross top/north boundary */
	return (0);
}

GMT_LONG GMT_map_init_tm (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	/* Wrap and truncations are in y, not x for TM */

	C->current.map.wrap_around_check = (PFL) GMT_wrap_around_check_tm;
	C->current.map.jump = (PFL) GMT_map_jump_tm;
	C->current.map.will_it_wrap = (PFB) GMT_will_it_wrap_tm;
	C->current.map.this_point_wraps = (PFB) GMT_this_point_wraps_tm;
	C->current.map.get_crossings = (PFV) GMT_get_crossings_tm;

	if (C->current.setting.proj_scale_factor == -1.0) C->current.setting.proj_scale_factor = 1.0;	/* Select default map scale for TM */
	C->current.proj.GMT_convert_latitudes = GMT_quicktm (C, C->current.proj.pars[0], 10.0);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	GMT_vtm (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	if (GMT_IS_SPHERICAL (C) || C->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		C->current.proj.fwd = (PFL) GMT_tm_sph;
		C->current.proj.inv = (PFL) GMT_itm_sph;
	}
	else {
		C->current.proj.fwd = (PFL) GMT_tm;
		C->current.proj.inv = (PFL) GMT_itm;
	}

	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.map.is_world) {	/* Gave oblique degrees */
		double w, e, dummy;
		w = C->current.proj.central_meridian + C->common.R.wesn[YLO];
		e = C->current.proj.central_meridian + C->common.R.wesn[YHI];
		C->common.R.wesn[YLO] = -90;
		C->common.R.wesn[YHI] = 90;
		C->common.R.wesn[XHI] = e;
		C->common.R.wesn[XLO] = w;
		GMT_vtm (C, C->current.proj.pars[0], 0.0);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], 0.0, &xmin, &dummy);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], 0.0, &xmax, &dummy);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &dummy, &ymin);
		ymax = ymin + (TWO_PI * C->current.proj.EQ_RAD * C->current.setting.proj_scale_factor);
		GMT_vtm (C, C->current.proj.pars[0], C->current.proj.pars[1]);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
		C->current.map.is_world_tm = TRUE;
		C->common.R.oblique = TRUE;	/* Since wesn was oblique, not geographical wesn */
		C->common.R.wesn[XHI] = C->current.proj.central_meridian + 180.0;
		C->common.R.wesn[XLO] = C->current.proj.central_meridian - 180.0;
	}
	else if (!C->common.R.oblique) {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.is_world_tm = GMT_IS_ZERO (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]);
		C->current.map.is_world = FALSE;
	}
	else { /* Find min values */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
		C->current.map.is_world_tm = FALSE;
		C->current.map.is_world = (fabs (C->common.R.wesn[YLO] - C->common.R.wesn[YHI]) < GMT_SMALL);
	}

	C->current.map.frame.horizontal = TRUE;
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);

	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE UNIVERSAL TRANSVERSE MERCATOR PROJECTION (GMT_UTM)
 */

GMT_LONG GMT_map_init_utm (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, lon0;

	if (C->current.setting.proj_scale_factor == -1.0) C->current.setting.proj_scale_factor = 0.9996;	/* Select default map scale for UTM */
	lon0 = 180.0 + 6.0 * C->current.proj.pars[0] - 3.0;	/* Central meridian for this UTM zone */
	if (lon0 >= 360.0) lon0 -= 360.0;
	C->current.proj.GMT_convert_latitudes = GMT_quicktm (C, lon0, 10.0);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	GMT_vtm (C, lon0, 0.0);	/* Central meridian for this zone */
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];
	switch (C->current.proj.utm_hemisphere) {	/* Set hemisphere */
		case -1:
			C->current.proj.north_pole = FALSE;
			break;
		case +1:
			C->current.proj.north_pole = TRUE;
			break;
		default:
			C->current.proj.north_pole = (C->common.R.wesn[YLO] >= 0.0);
			break;
	}
	if (GMT_IS_SPHERICAL (C) || C->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo conformal latitudes */
		C->current.proj.fwd = (PFL) GMT_utm_sph;
		C->current.proj.inv = (PFL) GMT_iutm_sph;
	}
	else {
		C->current.proj.fwd = (PFL) GMT_utm;
		C->current.proj.inv = (PFL) GMT_iutm;
	}

	if (fabs (C->common.R.wesn[XLO] - C->common.R.wesn[XHI]) > 360.0) {	/* -R in UTM meters */
		(*C->current.proj.inv) (C, &C->common.R.wesn[XLO], &C->common.R.wesn[YLO], C->common.R.wesn[XLO], C->common.R.wesn[YLO]);
		(*C->current.proj.inv) (C, &C->common.R.wesn[XHI], &C->common.R.wesn[YHI], C->common.R.wesn[XHI], C->common.R.wesn[YHI]);
		C->common.R.oblique = TRUE;
	}
	if (C->common.R.oblique) {
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
	}

	C->current.map.frame.horizontal = TRUE;

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);

	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	return (C->common.R.oblique);
}

/* Setting w/e/s/n for a fully qualified UTM zone */

GMT_LONG GMT_UTMzone_to_wesn (struct GMT_CTRL *C, GMT_LONG zone_x, GMT_LONG zone_y, GMT_LONG hemi, double wesn[])
{	/* Given the full UTM zone specification, return w/e/s/n */

	GMT_LONG error = 0;

	wesn[XHI] = 180.0 + 6.0 * zone_x;	wesn[XLO] = wesn[XHI] - 6.0;

	if (zone_y == 0) {	/* Latitude zone is not specified */
		if (hemi == -1) {
			wesn[YLO] = -80.0;	wesn[YHI] = 0.0;
		}
		else if (hemi == +1) {
			wesn[YLO] = 0.0;	wesn[YHI] = 84.0;
		}
		else
			error = TRUE;
		return (error);
	}
	else if (zone_y < 'A' || zone_y > 'Z')
		error = TRUE;
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
		if (zone_x == 32 || zone_x == 34 || zone_x == 36) error = TRUE;
	}
	else {	/* Y or Z */
		wesn[YLO] = 84.0;	wesn[YHI] = 90.0;
		wesn[XHI] = 180.0 * (zone_y - 'Y');
		wesn[XLO] = wesn[XHI] - 180.0;
	}

	return (error);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION (GMT_LAMB_AZ_EQ)
 */

GMT_LONG GMT_map_init_lambeq (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	C->current.proj.Dx = C->current.proj.Dy = 1.0;

	GMT_set_polar (C);
	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	GMT_vlambeq (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);

	if (C->current.proj.GMT_convert_latitudes) {
		double D, s, c;
		sincosd (C->current.proj.pars[1], &s, &c);
		D = (C->current.proj.polar) ? 1.0 : (C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius / C->current.proj.GMT_lat_swap_vals.ra) * c / (C->current.proj.cosp * d_sqrt (1.0 - C->current.proj.ECC2 * s * s));
		C->current.proj.Dx = D;
		C->current.proj.Dy = 1.0 / D;
	}
	C->current.proj.iDx = 1.0 / C->current.proj.Dx;
	C->current.proj.iDy = 1.0 / C->current.proj.Dy;

	C->current.proj.fwd = (PFL) GMT_lambeq;
	C->current.proj.inv = (PFL) GMT_ilambeq;
	if (C->current.proj.units_pr_degree) {
		GMT_vlambeq (C, 0.0, 90.0, C->current.proj.pars[2]);
		GMT_lambeq (C, 0.0, fabs(C->current.proj.pars[4]), &dummy, &radius);
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
		GMT_vlambeq (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);
	}
	else
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];

	if (C->common.R.oblique) {	/* Rectangular box given */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 30.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
	}
	else {
		if (C->current.proj.polar) {	/* Polar aspect */
			if (C->current.proj.north_pole) {
				if (C->common.R.wesn[YLO] <= -90.0){
					GMT_report (C, GMT_MSG_FATAL, "Error: South boundary cannot be -90.0 for north polar Lambert azimuthal projection\n");
					GMT_exit (EXIT_FAILURE);
				}
				if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
			}
			else {
				if (C->common.R.wesn[YHI] >= 90.0) {
					GMT_report (C, GMT_MSG_FATAL, "Error: North boundary cannot be +90.0 for south polar Lambert azimuthal projection\n");
					GMT_exit (EXIT_FAILURE);
				}
				if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
			C->current.map.outside = (PFL) GMT_polar_outside;
			C->current.map.crossing = (PFL) GMT_wesn_crossing;
			C->current.map.overlap = (PFL) GMT_wesn_overlap;
			C->current.map.clip = (PFL) GMT_wesn_clip;
			C->current.map.frame.horizontal = TRUE;
			C->current.map.n_lat_nodes = 2;
			GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			C->current.map.frame.axis[GMT_X].item[0].interval = C->current.map.frame.axis[GMT_Y].item[0].interval = 0.0;	/* No annotations for global mode */
			C->current.map.frame.axis[GMT_X].item[4].interval = C->current.map.frame.axis[GMT_Y].item[4].interval = 0.0;	/* No tickmarks for global mode */
			C->common.R.wesn[XLO] = 0.0;
			C->common.R.wesn[XHI] = 360.0;
			C->common.R.wesn[YLO] = -90.0;
			C->common.R.wesn[YHI] = 90.0;
			xmax = ymax = C->current.proj.rho_max;
			xmin = ymin = -xmax;
			C->current.map.outside = (PFL) GMT_radial_outside;
			C->current.map.crossing = (PFL) GMT_radial_crossing;
			C->current.map.overlap = (PFL) GMT_radial_overlap;
			C->current.map.clip = (PFL) GMT_radial_clip;
			if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[3]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	if (C->current.proj.polar) C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ORTHOGRAPHIC PROJECTION (GMT_ORTHO)
 */

GMT_LONG GMT_map_init_ortho (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	GMT_set_polar (C);

	if (C->current.proj.units_pr_degree) {
		GMT_vortho (C, 0.0, 90.0, C->current.proj.pars[2]);
		GMT_ortho (C, 0.0, fabs(C->current.proj.pars[4]), &dummy, &radius);
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
	}
	else
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];

	GMT_vortho (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);
	C->current.proj.fwd = (PFL) GMT_ortho;
	C->current.proj.inv = (PFL) GMT_iortho;

	if (C->common.R.oblique) {	/* Rectangular box given */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 30.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
	}
	else {
		if (C->current.proj.polar) {	/* Polar aspect */
			if (C->current.proj.north_pole) {
				if (C->common.R.wesn[YLO] < 0.0) {
					GMT_report (C, GMT_MSG_FATAL, "Warning: South boundary cannot be < 0 for north polar orthographic projection (reset to 0)\n");
					C->common.R.wesn[YLO] = 0.0;
				}
				if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
			}
			else {
				if (C->common.R.wesn[YHI] > 0.0) {
					GMT_report (C, GMT_MSG_FATAL, "Warning: North boundary cannot be > 0 for south polar orthographic projection (reset to 0)\n");
					C->common.R.wesn[YHI] = 0.0;
				}
				if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
			C->current.map.outside = (PFL) GMT_polar_outside;
			C->current.map.crossing = (PFL) GMT_wesn_crossing;
			C->current.map.overlap = (PFL) GMT_wesn_overlap;
			C->current.map.clip = (PFL) GMT_wesn_clip;
			C->current.map.frame.horizontal = TRUE;
			C->current.map.n_lat_nodes = 2;
			GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			C->current.map.frame.axis[GMT_X].item[0].interval = C->current.map.frame.axis[GMT_Y].item[0].interval = 0.0;	/* No annotations for global mode */
			C->current.map.frame.axis[GMT_X].item[4].interval = C->current.map.frame.axis[GMT_Y].item[4].interval = 0.0;	/* No tickmarks for global mode */
			C->common.R.wesn[XLO] = 0.0;
			C->common.R.wesn[XHI] = 360.0;
			C->common.R.wesn[YLO] = -90.0;
			C->common.R.wesn[YHI] = 90.0;
			xmax = ymax = C->current.proj.rho_max * C->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			C->current.map.outside = (PFL) GMT_radial_outside;
			C->current.map.crossing = (PFL) GMT_radial_crossing;
			C->current.map.overlap = (PFL) GMT_radial_overlap;
			C->current.map.clip = (PFL) GMT_radial_clip;
			if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[3]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	if (C->current.proj.polar) C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE GENERAL PERSPECTIVE PROJECTION (GMT_GENPER)
 */

GMT_LONG GMT_map_init_genper (struct GMT_CTRL *C) {
	GMT_LONG search;
	double xmin, xmax, ymin, ymax, dummy, radius;

	double alt, azimuth, tilt, width, height;
	double twist, scale, units;

	units = C->current.proj.pars[2];
	scale = C->current.proj.pars[3];
	alt = C->current.proj.pars[4];
	azimuth = C->current.proj.pars[5];
	tilt = C->current.proj.pars[6];
	twist = C->current.proj.pars[7];
	width = C->current.proj.pars[8];
	height = C->current.proj.pars[9];

	if (C->current.proj.g_sphere) GMT_set_spherical (C); /* PW: Force spherical for now */

	GMT_set_polar (C);

	if (C->current.proj.units_pr_degree) {
		GMT_vgenper (C, 0.0, 90.0, alt, azimuth, tilt, twist, width, height);
		GMT_genper (C, 0.0, fabs(C->current.proj.pars[3]), &dummy, &radius);
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[2] / radius);
	}
	else
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];

	if (C->current.proj.g_debug > 1) {
		GMT_message (C, "genper: units_pr_degree %ld\n", C->current.proj.units_pr_degree);
		GMT_message (C, "genper: radius %f\n", radius);
		GMT_message (C, "genper: scale %f units %f\n", scale, units);
		GMT_message (C, "genper: x scale %f y scale %f\n", C->current.proj.scale[GMT_X], C->current.proj.scale[GMT_Y]);
		GMT_message (C, "genper: gave_map_width %ld \n",C->current.proj.gave_map_width);
	}

	GMT_vgenper (C, C->current.proj.pars[0], C->current.proj.pars[1], alt, azimuth, tilt, twist, width, height);
	C->current.proj.fwd = (PFL) GMT_genper;
	C->current.proj.inv = (PFL) GMT_igenper;

	C->common.R.wesn[XLO] = 0.0;
	C->common.R.wesn[XHI] = 360.0;
	C->common.R.wesn[YLO] = -90.0;
	C->common.R.wesn[YHI] = 90.0;

	xmin = ymin = -C->current.proj.g_rmax;
	xmax = ymax = -xmin;

	xmin = C->current.proj.g_xmin;
	xmax = C->current.proj.g_xmax;
	ymin = C->current.proj.g_ymin;
	ymax = C->current.proj.g_ymax;

	if (C->current.proj.g_width != 0.0) {
		C->common.R.oblique = FALSE;
		if (C->current.proj.g_debug > 0) GMT_message (C, "using windowed region\n");
		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip_old;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 30.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
		search = TRUE;
	}
	else {
		if (C->current.proj.g_debug > 0) GMT_message (C, "using global view\n");

		/* No annotations for global mode */
		C->current.map.frame.axis[GMT_X].item[0].interval = C->current.map.frame.axis[GMT_Y].item[0].interval = 0.0;

		/* No tickmarks for global mode */
		C->current.map.frame.axis[GMT_X].item[4].interval = C->current.map.frame.axis[GMT_Y].item[4].interval = 0.0;

		C->current.map.overlap = (PFL) GMT_genper_overlap;
		C->current.map.crossing = (PFL) GMT_radial_crossing;
		C->current.map.clip = (PFL) GMT_radial_clip;
		C->current.map.outside = (PFL) GMT_radial_outside;
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;

		if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

		search = FALSE;
  	}

	if (C->current.proj.polar) {
		if (C->current.proj.north_pole) {
			if (C->common.R.wesn[YLO] < (90.0 - C->current.proj.f_horizon)) C->common.R.wesn[YLO] = 90.0 - C->current.proj.f_horizon;
			if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
		} else {
			if (C->common.R.wesn[YHI] > -(90.0 - C->current.proj.f_horizon)) C->common.R.wesn[YHI] = -(90.0 - C->current.proj.f_horizon);
			if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
		}
		if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
	}

	if (C->current.proj.g_debug > 0) GMT_message (C, "xmin %f xmax %f ymin %f ymax %f\n", xmin/1000, xmax/1000, ymin/1000, ymax/1000);

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);

	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];

	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);

	if (C->current.proj.g_debug > 0) {
		GMT_message (C, "x scale %e y scale %e\n", C->current.proj.scale[GMT_X], C->current.proj.scale[GMT_Y]);
		GMT_message (C, "x center %f y center %f\n", C->current.proj.c_x0, C->current.proj.c_y0);
		GMT_message (C, "x max %f y max %f\n", C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		GMT_message (C, "x0 %f y0 %f\n\n", C->current.proj.origin[GMT_X], C->current.proj.origin[GMT_Y]);
		fflush(NULL);
	}

	return (search);

}

/*
 *	TRANSFORMATION ROUTINES FOR THE GNOMONIC PROJECTION (GMT_GNOMONIC)
 */

GMT_LONG GMT_map_init_gnomonic (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	GMT_set_polar (C);

	if (C->current.proj.units_pr_degree) {
		GMT_vgnomonic (C, 0.0, 90.0, 60.0);
		GMT_gnomonic (C, 0.0, fabs(C->current.proj.pars[4]), &dummy, &radius);
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
	}
	else
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];

	GMT_vgnomonic (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);
	C->current.proj.fwd = (PFL) GMT_gnomonic;
	C->current.proj.inv = (PFL) GMT_ignomonic;

	if (C->common.R.oblique) {	/* Rectangular box given */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 30.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
	}
	else {
		if (C->current.proj.polar) {	/* Polar aspect */
			if (C->current.proj.north_pole) {
				if (C->common.R.wesn[YLO] < (90.0 - C->current.proj.f_horizon)) C->common.R.wesn[YLO] = 90.0 - C->current.proj.f_horizon;
				if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
			}
			else {
				if (C->common.R.wesn[YHI] > -(90.0 - C->current.proj.f_horizon)) C->common.R.wesn[YHI] = -(90.0 - C->current.proj.f_horizon);
				if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
			}
			if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
			C->current.map.outside = (PFL) GMT_polar_outside;
			C->current.map.crossing = (PFL) GMT_wesn_crossing;
			C->current.map.overlap = (PFL) GMT_wesn_overlap;
			C->current.map.clip = (PFL) GMT_wesn_clip;
			C->current.map.frame.horizontal = TRUE;
			C->current.map.n_lat_nodes = 2;
			GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		}
		else {	/* Global view only */
			C->common.R.wesn[XLO] = 0.0;
			C->common.R.wesn[XHI] = 360.0;
			C->common.R.wesn[YLO] = -90.0;
			C->common.R.wesn[YHI] = 90.0;
			xmax = ymax = C->current.proj.rho_max * C->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			C->current.map.outside = (PFL) GMT_radial_outside;
			C->current.map.crossing = (PFL) GMT_radial_crossing;
			C->current.map.overlap = (PFL) GMT_radial_overlap;
			C->current.map.clip = (PFL) GMT_radial_clip;
			if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[3]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE AZIMUTHAL EQUIDISTANT PROJECTION (GMT_AZ_EQDIST)
 */

GMT_LONG GMT_map_init_azeqdist (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dummy, radius;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	GMT_set_polar (C);

	if (C->current.proj.units_pr_degree) {
		GMT_vazeqdist (C, 0.0, 90.0, C->current.proj.pars[2]);
		GMT_azeqdist (C, 0.0, fabs(C->current.proj.pars[4]), &dummy, &radius);
		if (GMT_IS_ZERO (radius)) radius = C->current.proj.rho_max * C->current.proj.EQ_RAD;
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = fabs (C->current.proj.pars[3] / radius);
	}
	else
		C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[3];

	GMT_vazeqdist (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2]);
	C->current.proj.fwd = (PFL) GMT_azeqdist;
	C->current.proj.inv = (PFL) GMT_iazeqdist;

	if (C->common.R.oblique) {	/* Rectangular box given */
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);

		C->current.map.outside = (PFL) GMT_rect_outside2;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = !(C->current.setting.map_annot_oblique & 1);
		C->current.map.frame.horizontal = (fabs (C->current.proj.pars[1]) < 60.0 && fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 30.0);
	}
	else {
		if (C->current.proj.polar && (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) < 180.0) {	/* Polar aspect */
			if (!C->current.proj.north_pole && C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
			if (C->current.proj.north_pole && C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
			if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) || GMT_IS_ZERO (C->common.R.wesn[XHI] - C->common.R.wesn[XLO])) C->current.proj.edge[1] = C->current.proj.edge[3] = FALSE;
			C->current.map.outside = (PFL) GMT_polar_outside;
			C->current.map.crossing = (PFL) GMT_wesn_crossing;
			C->current.map.overlap = (PFL) GMT_wesn_overlap;
			C->current.map.clip = (PFL) GMT_wesn_clip;
			C->current.map.frame.horizontal = TRUE;
			C->current.map.n_lat_nodes = 2;
			GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		}
		else {	/* Global view only, force wesn = 0/360/-90/90  */
			C->current.map.frame.axis[GMT_X].item[0].interval = C->current.map.frame.axis[GMT_Y].item[0].interval = 0.0;	/* No annotations for global mode */
			C->current.map.frame.axis[GMT_X].item[4].interval = C->current.map.frame.axis[GMT_Y].item[4].interval = 0.0;	/* No tickmarks for global mode */
			C->common.R.wesn[XLO] = 0.0;
			C->common.R.wesn[XHI] = 360.0;
			C->common.R.wesn[YLO] = -90.0;
			C->common.R.wesn[YHI] = 90.0;
			xmax = ymax = C->current.proj.rho_max * C->current.proj.EQ_RAD;
			xmin = ymin = -xmax;
			xmax = ymax = -xmin;
			C->current.map.outside = (PFL) GMT_radial_outside;
			C->current.map.crossing = (PFL) GMT_radial_crossing;
			C->current.map.overlap = (PFL) GMT_radial_overlap;
			C->current.map.clip = (PFL) GMT_radial_clip;
			if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
		}
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[3]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	if (C->current.proj.polar) C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE MOLLWEIDE EQUAL AREA PROJECTION (GMT_MOLLWEIDE)
 */

GMT_LONG GMT_map_init_mollweide (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, y, dummy;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = M_PI * C->current.proj.pars[1] / sqrt (8.0);
	GMT_vmollweide (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
	if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;

	if (C->common.R.oblique) {
		GMT_mollweide (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_mollweide (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		GMT_mollweide (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_mollweide (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_mollweide (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_mollweide (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_ellipse;
		C->current.map.right_edge = (PFD) GMT_right_ellipse;
		C->current.map.frame.horizontal = 2;
		C->current.proj.polar = TRUE;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_mollweide;
	C->current.proj.inv = (PFL) GMT_imollweide;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	C->current.map.parallel_straight = TRUE;

	return (C->common.R.oblique);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE HAMMER-AITOFF EQUAL AREA PROJECTION (GMT_HAMMER)
 */

GMT_LONG GMT_map_init_hammer (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = 0.5 * M_PI * C->current.proj.pars[1] / M_SQRT2;
	GMT_vhammer (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
	if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;

	if (C->common.R.oblique) {
		GMT_hammer (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_hammer (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double x, y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		x = (fabs (C->common.R.wesn[XLO] - C->current.proj.central_meridian) > fabs (C->common.R.wesn[XHI] - C->current.proj.central_meridian)) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
		GMT_hammer (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_hammer (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_hammer (C, x, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_hammer (C, x, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_ellipse;
		C->current.map.right_edge = (PFD) GMT_right_ellipse;
		C->current.map.frame.horizontal = 2;
		C->current.proj.polar = TRUE;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_hammer;
	C->current.proj.inv = (PFL) GMT_ihammer;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE VAN DER GRINTEN PROJECTION (GMT_VANGRINTEN)
 */

GMT_LONG GMT_map_init_grinten (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];
	GMT_vgrinten (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
	if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;

	if (C->common.R.oblique) {
		GMT_grinten (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_grinten (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double x, y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		x = (fabs (C->common.R.wesn[XLO] - C->current.proj.central_meridian) > fabs (C->common.R.wesn[XHI] - C->current.proj.central_meridian)) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
		GMT_grinten (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_grinten (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_grinten (C, x, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_grinten (C, x, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_circle;
		C->current.map.right_edge = (PFD) GMT_right_circle;
		C->current.map.frame.horizontal = 2;
		C->current.proj.polar = TRUE;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.r = 0.5 * C->current.proj.rect[XHI];
	C->current.proj.fwd = (PFL) GMT_grinten;
	C->current.proj.inv = (PFL) GMT_igrinten;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE WINKEL-TRIPEL MODIFIED AZIMUTHAL PROJECTION (GMT_WINKEL)
 */

GMT_LONG GMT_map_init_winkel (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	GMT_vwinkel (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = 2.0 * C->current.proj.pars[1] / (1.0 + C->current.proj.r_cosphi1);

	if (C->common.R.oblique) {
		GMT_winkel (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_winkel (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double x, y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		x = (fabs (C->common.R.wesn[XLO] - C->current.proj.central_meridian) > fabs (C->common.R.wesn[XHI] - C->current.proj.central_meridian)) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
		GMT_winkel (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_winkel (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_winkel (C, x, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_winkel (C, x, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_winkel;
		C->current.map.right_edge = (PFD) GMT_right_winkel;
		C->current.map.frame.horizontal = 2;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_winkel;
	C->current.proj.inv = (PFL) GMT_iwinkel;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT IV PROJECTION (GMT_ECKERT4)
 */

GMT_LONG GMT_map_init_eckert4 (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	GMT_veckert4 (C, C->current.proj.pars[0]);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];

	if (C->common.R.oblique) {
		GMT_eckert4 (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_eckert4 (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		GMT_eckert4 (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_eckert4 (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_eckert4 (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_eckert4 (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_eckert4;
		C->current.map.right_edge = (PFD) GMT_right_eckert4;
		C->current.map.frame.horizontal = 2;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_eckert4;
	C->current.proj.inv = (PFL) GMT_ieckert4;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	C->current.map.parallel_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ECKERT VI PROJECTION (GMT_ECKERT6)
 */

GMT_LONG GMT_map_init_eckert6 (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	GMT_veckert6 (C, C->current.proj.pars[0]);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = 0.5 * C->current.proj.pars[1] * sqrt (2.0 + M_PI);

	if (C->common.R.oblique) {
		GMT_eckert6 (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_eckert6 (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		GMT_eckert6 (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_eckert6 (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_eckert6 (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_eckert6 (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_eckert6;
		C->current.map.right_edge = (PFD) GMT_right_eckert6;
		C->current.map.frame.horizontal = 2;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_eckert6;
	C->current.proj.inv = (PFL) GMT_ieckert6;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	C->current.map.parallel_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ROBINSON PSEUDOCYLINDRICAL PROJECTION (GMT_ROBINSON)
 */

GMT_LONG GMT_map_init_robinson (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	GMT_vrobinson (C, C->current.proj.pars[0]);
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1] / 0.8487;

	if (C->common.R.oblique) {
		GMT_robinson (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_robinson (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		GMT_robinson (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_robinson (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		GMT_robinson (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_robinson (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_robinson;
		C->current.map.right_edge = (PFD) GMT_right_robinson;
		C->current.map.frame.horizontal = 2;
	}
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.proj.fwd = (PFL) GMT_robinson;
	C->current.proj.inv = (PFL) GMT_irobinson;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	C->current.map.parallel_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE SINUSOIDAL EQUAL AREA PROJECTION (GMT_SINUSOIDAL)
 */

GMT_LONG GMT_map_init_sinusoidal (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	if (C->current.proj.pars[0] < 0.0) C->current.proj.pars[0] += 360.0;
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
	if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
	GMT_vsinusoidal (C, C->current.proj.pars[0]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[1] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[1];
	C->current.proj.fwd = (PFL) GMT_sinusoidal;
	C->current.proj.inv = (PFL) GMT_isinusoidal;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (C->common.R.oblique) {
		GMT_sinusoidal (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		GMT_sinusoidal (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double dummy, y;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		GMT_sinusoidal (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		GMT_sinusoidal (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		GMT_sinusoidal (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		GMT_sinusoidal (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_sinusoidal;
		C->current.map.right_edge = (PFD) GMT_right_sinusoidal;
		C->current.map.frame.horizontal = 2;
		C->current.proj.polar = TRUE;
	}

	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[1]);
	C->current.map.parallel_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE CASSINI PROJECTION (GMT_CASSINI)
 */

GMT_LONG GMT_map_init_cassini (struct GMT_CTRL *C) {
	GMT_LONG too_big;
	double xmin, xmax, ymin, ymax;

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	too_big = GMT_quicktm (C, C->current.proj.pars[0], 4.0);
	if (too_big) GMT_set_spherical (C);	/* Cannot use ellipsoidal series for this area */
	GMT_vcassini (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (GMT_IS_SPHERICAL (C)) {
		C->current.proj.fwd = (PFL) GMT_cassini_sph;
		C->current.proj.inv = (PFL) GMT_icassini_sph;
	}
	else {
		C->current.proj.fwd = (PFL) GMT_cassini;
		C->current.proj.inv = (PFL) GMT_icassini;
	}
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (C->common.R.oblique) {
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_conic;
		C->current.map.right_edge = (PFD) GMT_right_conic;
	}

	C->current.map.frame.horizontal = TRUE;
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE ALBERS PROJECTION (GMT_ALBERS)
 */

GMT_LONG GMT_map_init_albers (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	C->current.proj.GMT_convert_latitudes = GMT_quickconic (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	if (GMT_IS_SPHERICAL (C) || C->current.proj.GMT_convert_latitudes) {	/* Spherical code w/wo authalic latitudes */
		GMT_valbers_sph (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2], C->current.proj.pars[3]);
		C->current.proj.fwd = (PFL) GMT_albers_sph;
		C->current.proj.inv = (PFL) GMT_ialbers_sph;
	}
	else {
		GMT_valbers (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2], C->current.proj.pars[3]);
		C->current.proj.fwd = (PFL) GMT_albers;
		C->current.proj.inv = (PFL) GMT_ialbers;
	}
	if (C->current.proj.units_pr_degree) C->current.proj.pars[4] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[4];

	if (C->common.R.oblique) {
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_conic;
		C->current.map.right_edge = (PFD) GMT_right_conic;
	}
	C->current.map.frame.horizontal = TRUE;
	C->current.map.n_lat_nodes = 2;
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[4]);

	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	GMT_geo_to_xy (C, C->current.proj.central_meridian + 90., C->current.proj.pole, &x1, &y1);
	dy = y1 - C->current.proj.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - C->current.proj.c_x0);
	dy /= (1.0 - cos (az));
	C->current.proj.c_y0 += dy;
	C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}


/*
 *	TRANSFORMATION ROUTINES FOR THE EQUIDISTANT CONIC PROJECTION (GMT_ECONIC)
 */


GMT_LONG GMT_map_init_econic (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax, dy, az, x1, y1;

	C->current.proj.GMT_convert_latitudes = !GMT_IS_SPHERICAL (C);
	if (C->current.proj.GMT_convert_latitudes) GMT_scale_eqrad (C);
	GMT_veconic (C, C->current.proj.pars[0], C->current.proj.pars[1], C->current.proj.pars[2], C->current.proj.pars[3]);
	C->current.proj.fwd = (PFL) GMT_econic;
	C->current.proj.inv = (PFL) GMT_ieconic;
	if (C->current.proj.units_pr_degree) C->current.proj.pars[4] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[4];

	if (C->common.R.oblique) {
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		GMT_xy_search (C, &xmin, &xmax, &ymin, &ymax, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_conic;
		C->current.map.right_edge = (PFD) GMT_right_conic;
	}
	C->current.map.frame.horizontal = TRUE;
	C->current.map.n_lat_nodes = 2;
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[4]);

	GMT_geo_to_xy (C, C->current.proj.central_meridian, C->current.proj.pole, &C->current.proj.c_x0, &C->current.proj.c_y0);
	GMT_geo_to_xy (C, C->current.proj.central_meridian + 90., C->current.proj.pole, &x1, &y1);
	dy = y1 - C->current.proj.c_y0;
	az = 2.0 * d_atan2 (dy, x1 - C->current.proj.c_x0);
	dy /= (1.0 - cos (az));
	C->current.proj.c_y0 += dy;
	C->current.map.meridian_straight = TRUE;

	return (C->common.R.oblique);
}

/*
 *	TRANSFORMATION ROUTINES FOR THE POLYCONIC PROJECTION (GMT_POLYCONIC)
 */

GMT_LONG GMT_map_init_polyconic (struct GMT_CTRL *C) {
	double xmin, xmax, ymin, ymax;

	GMT_set_spherical (C);	/* PW: Force spherical for now */

	if (GMT_is_dnan (C->current.proj.pars[0])) C->current.proj.pars[0] = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	C->current.map.is_world = GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]);
	if (C->common.R.wesn[YLO] <= -90.0) C->current.proj.edge[0] = FALSE;
	if (C->common.R.wesn[YHI] >= 90.0) C->current.proj.edge[2] = FALSE;
	GMT_vpolyconic (C, C->current.proj.pars[0], C->current.proj.pars[1]);
	if (C->current.proj.units_pr_degree) C->current.proj.pars[2] /= C->current.proj.M_PR_DEG;
	C->current.proj.scale[GMT_X] = C->current.proj.scale[GMT_Y] = C->current.proj.pars[2];
	C->current.proj.fwd = (PFL) GMT_polyconic;
	C->current.proj.inv = (PFL) GMT_ipolyconic;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	if (C->common.R.oblique) {
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xmin, &ymin);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xmax, &ymax);
		C->current.map.outside = (PFL) GMT_rect_outside;
		C->current.map.crossing = (PFL) GMT_rect_crossing;
		C->current.map.overlap = (PFL) GMT_rect_overlap;
		C->current.map.clip = (PFL) GMT_rect_clip;
		C->current.map.left_edge = (PFD) GMT_left_rect;
		C->current.map.right_edge = (PFD) GMT_right_rect;
		C->current.map.frame.check_side = TRUE;
	}
	else {
		double y, dummy;
		y = (C->common.R.wesn[YLO] * C->common.R.wesn[YHI] <= 0.0) ? 0.0 : MIN (fabs(C->common.R.wesn[YLO]), fabs(C->common.R.wesn[YHI]));
		(*C->current.proj.fwd) (C, C->common.R.wesn[XLO], y, &xmin, &dummy);
		(*C->current.proj.fwd) (C, C->common.R.wesn[XHI], y, &xmax, &dummy);
		(*C->current.proj.fwd) (C, C->current.proj.central_meridian, C->common.R.wesn[YLO], &dummy, &ymin);
		(*C->current.proj.fwd) (C, C->current.proj.central_meridian, C->common.R.wesn[YHI], &dummy, &ymax);
		C->current.map.outside = (PFL) GMT_wesn_outside;
		C->current.map.crossing = (PFL) GMT_wesn_crossing;
		C->current.map.overlap = (PFL) GMT_wesn_overlap;
		C->current.map.clip = (PFL) GMT_wesn_clip;
		C->current.map.left_edge = (PFD) GMT_left_polyconic;
		C->current.map.right_edge = (PFD) GMT_right_polyconic;
		C->current.proj.polar = TRUE;
	}

	C->current.map.frame.horizontal = TRUE;
	GMT_map_setinfo (C, xmin, xmax, ymin, ymax, C->current.proj.pars[2]);

	return (C->common.R.oblique);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 *	S E C T I O N  1.1 :	S U P P O R T I N G   R O U T I N E S
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void GMT_wesn_search (struct GMT_CTRL *C, double xmin, double xmax, double ymin, double ymax, double *west, double *east, double *south, double *north) {
	double dx, dy, w, e, s, n, x, y, lat, *lon = NULL;
	GMT_LONG i, j, k;

	/* Search for extreme lon/lat coordinates by matching along the rectangular boundary */

	dx = (xmax - xmin) / C->current.map.n_lon_nodes;
	dy = (ymax - ymin) / C->current.map.n_lat_nodes;
	/* Need temp array to hold all the longitudes we compute */
	lon = GMT_memory (C, NULL, 2 * (C->current.map.n_lon_nodes + C->current.map.n_lat_nodes + 2), double);
	w = s = DBL_MAX;	e = n = -DBL_MAX;
	for (i = k = 0; i <= C->current.map.n_lon_nodes; i++) {
		x = (i == C->current.map.n_lon_nodes) ? xmax : xmin + i * dx;
		GMT_xy_to_geo (C, &lon[k++], &lat, x, ymin);
		if (lat < s) s = lat;	if (lat > n) n = lat;
		GMT_xy_to_geo (C, &lon[k++], &lat, x, ymax);
		if (lat < s) s = lat;	if (lat > n) n = lat;
	}
	for (j = 0; j <= C->current.map.n_lat_nodes; j++) {
		y = (j == C->current.map.n_lat_nodes) ? ymax : ymin + j * dy;
		GMT_xy_to_geo (C, &lon[k++], &lat, xmin, y);
		if (lat < s) s = lat;	if (lat > n) n = lat;
		GMT_xy_to_geo (C, &lon[k++], &lat, xmax, y);
		if (lat < s) s = lat;	if (lat > n) n = lat;
	}
	GMT_get_lon_minmax (C, lon, k, &w, &e);	/* Determine lon-range by robust quandrant check */
	GMT_free (C, lon);

	/* Then check if one or both poles are inside map; then the above wont be correct */

	if (!GMT_map_outside (C, C->current.proj.central_meridian, +90.0)) { n = +90.0; w = 0.0; e = 360.0; }
	if (!GMT_map_outside (C, C->current.proj.central_meridian, -90.0)) { s = -90.0; w = 0.0; e = 360.0; }

	s -= 0.1;	if (s < -90.0) s = -90.0;	/* Make sure point is not inside area, 0.1 is just a small arbitrary number */
	n += 0.1;	if (n > 90.0) n = 90.0;		/* But dont go crazy beyond the pole */
	w -= 0.1;	e += 0.1;	if (fabs (w - e) > 360.0) { w = 0.0; e = 360.0; }	/* Ensure max 360 range */
	*west = w;	*east = e;	*south = s;	*north = n;	/* Pass back our findings */
}

GMT_LONG GMT_horizon_search (struct GMT_CTRL *C, double w, double e, double s, double n, double xmin, double xmax, double ymin, double ymax) {
	double dx, dy, d, x, y, lon, lat;
	GMT_LONG i, j, beyond = FALSE;

	/* Search for extreme original coordinates lon/lat and see if any fall beyond the horizon */

	dx = (xmax - xmin) / C->current.map.n_lon_nodes;
	dy = (ymax - ymin) / C->current.map.n_lat_nodes;
	if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, w, s)) > C->current.proj.f_horizon) beyond = TRUE;
	if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, e, n)) > C->current.proj.f_horizon) beyond = TRUE;
	for (i = 0; !beyond && i <= C->current.map.n_lon_nodes; i++) {
		x = (i == C->current.map.n_lon_nodes) ? xmax : xmin + i * dx;
		GMT_xy_to_geo (C, &lon, &lat, x, ymin);
		if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon, lat)) > C->current.proj.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (C, &lon, &lat, x, ymax);
		if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon, lat)) > C->current.proj.f_horizon) beyond = TRUE;
	}
	for (j = 0; !beyond && j <= C->current.map.n_lat_nodes; j++) {
		y = (j == C->current.map.n_lat_nodes) ? ymax : ymin + j * dy;
		GMT_xy_to_geo (C, &lon, &lat, xmin, y);
		if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon, lat)) > C->current.proj.f_horizon) beyond = TRUE;
		GMT_xy_to_geo (C, &lon, &lat, xmax, y);
		if ((d = GMT_great_circle_dist_degree (C, C->current.proj.central_meridian, C->current.proj.pole, lon, lat)) > C->current.proj.f_horizon) beyond = TRUE;
	}
	if (beyond) {
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Rectangular region for azimuthal projection extends beyond the horizon\n");
		GMT_report (C, GMT_MSG_FATAL, "ERROR: Please select a region that is completely within the visible hemisphere\n");
		GMT_exit (EXIT_FAILURE);
	}
	return (GMT_NOERROR);
}

GMT_LONG GMT_map_outside (struct GMT_CTRL *C, double lon, double lat)
{	/* Save current status in previous status and update current in/out status */
	C->current.map.prev_x_status = C->current.map.this_x_status;
	C->current.map.prev_y_status = C->current.map.this_y_status;
	return ((*C->current.map.outside) (C, lon, lat));
}

GMT_LONG GMT_wrap_around_check_x (struct GMT_CTRL *C, double *angle, double last_x, double last_y, double this_x, double this_y, double *xx, double *yy, GMT_LONG *sides)
{
	double dx, dy, width, jump, GMT_half_map_width (struct GMT_CTRL *C, double y);

	jump = this_x - last_x;
	width = MAX (GMT_half_map_width (C, this_y), GMT_half_map_width (C, last_y));

	if (fabs (jump) - width <= GMT_SMALL || fabs(jump) <= GMT_SMALL) return (0);
	dy = this_y - last_y;

	if (jump < -width) {	/* Crossed right boundary */
		dx = C->current.map.width + jump;
		yy[0] = yy[1] = last_y + (C->current.map.width - last_x) * dy / dx;
		if (yy[0] < 0.0 || yy[0] > C->current.proj.rect[YHI]) return (0);
		xx[0] = GMT_right_boundary (C, yy[0]);	xx[1] = GMT_left_boundary (C, yy[0]);
		sides[0] = 1;
		angle[0] = d_atan2d (dy, dx);
	}
	else {	/* Crossed left boundary */
		dx = C->current.map.width - jump;
		yy[0] = yy[1] = last_y + last_x * dy / dx;
		if (yy[0] < 0.0 || yy[0] > C->current.proj.rect[YHI]) return (0);
		xx[0] = GMT_left_boundary (C, yy[0]);	xx[1] = GMT_right_boundary (C, yy[0]);
		sides[0] = 3;
		angle[0] = d_atan2d (dy, -dx);
	}
	sides[1] = 4 - sides[0];
	angle[1] = angle[0] + 180.0;

	return (2);
}

void GMT_get_crossings_x (struct GMT_CTRL *C, double *xc, double *yc, double x0, double y0, double x1, double y1)
{	/* Finds crossings for wrap-arounds */
	double xa, xb, ya, yb, dxa, dxb, dyb, c;

	xa = x0;	xb = x1;
	ya = y0;	yb = y1;
	if (xa > xb) {	/* Make A the minimum x point */
		d_swap (xa, xb);
		d_swap (ya, yb);
	}

	xb -= 2.0 * GMT_half_map_width (C, yb);

	dxa = xa - GMT_left_boundary (C, ya);
	dxb = GMT_left_boundary (C, yb) - xb;
	c = (GMT_IS_ZERO (dxb)) ? 0.0 : 1.0 + dxa/dxb;
	dyb = (GMT_IS_ZERO (c)) ? 0.0 : fabs (yb - ya) / c;
	yc[0] = yc[1] = (ya > yb) ? yb + dyb : yb - dyb;
	xc[0] = GMT_left_boundary (C, yc[0]);
	xc[1] = GMT_right_boundary (C, yc[0]);
}

double GMT_half_map_width (struct GMT_CTRL *C, double y)
{	/* Returns 1/2-width of map in inches given y value */
	double half_width;

	switch (C->current.proj.projection) {

		case GMT_STEREO:	/* Must compute width of circular map based on y value (ASSUMES FULL CIRCLE!!!) */
		case GMT_LAMB_AZ_EQ:
		case GMT_ORTHO:
		case GMT_GENPER:
		case GMT_GNOMONIC:
		case GMT_AZ_EQDIST:
		case GMT_VANGRINTEN:
			if (!C->common.R.oblique && C->current.map.is_world) {
				y -= C->current.proj.r;
				half_width = d_sqrt (C->current.proj.r * C->current.proj.r - y * y);
			}
			else
				half_width = C->current.map.half_width;
			break;

		case GMT_MOLLWEIDE:		/* Must compute width of Mollweide map based on y value */
		case GMT_HAMMER:
		case GMT_WINKEL:
		case GMT_SINUSOIDAL:
		case GMT_ROBINSON:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
			if (!C->common.R.oblique && C->current.map.is_world)
				half_width = GMT_right_boundary (C, y) - C->current.map.half_width;
			else
				half_width = C->current.map.half_width;
			break;

		default:	/* Rectangular maps are easy */
			half_width = C->current.map.half_width;
			break;
	}
	return (half_width);
}

GMT_LONG GMT_this_point_wraps_x (struct GMT_CTRL *C, double x0, double x1, double w_last, double w_this)
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

GMT_LONG GMT_will_it_wrap_x (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG *start)
{	/* Determines if a polygon will wrap at edges */
	GMT_LONG i, wrap;
	double w_last, w_this;

	if (!C->current.map.is_world) return (FALSE);

	w_this = GMT_half_map_width (C, y[0]);
	for (i = 1, wrap = FALSE; !wrap && i < n; i++) {
		w_last = w_this;
		w_this = GMT_half_map_width (C, y[i]);
		wrap = GMT_this_point_wraps_x (C, x[i-1], x[i], w_last, w_this);
	}
	*start = i - 1;
	return (wrap);
}

GMT_LONG GMT_map_truncate_x (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG l_or_r)
{	/* Truncates a wrapping polygon against left or right edge */

	GMT_LONG i, i1, j, k;
	double xc[4], yc[4], w_last, w_this;

	/* First initialize variables that differ for left and right truncation */

	if (l_or_r == -1)	/* Left truncation (-1) */
		/* Find first point that is left of map center */
		i = (x[start] < C->current.map.half_width) ? start : start - 1;
	else				/* Right truncation (+1) */
		/* Find first point that is right of map center */
		i = (x[start] > C->current.map.half_width) ? start : start - 1;

	if (!C->current.plot.n_alloc) GMT_get_plot_array (C);

	C->current.plot.x[0] = x[i];	C->current.plot.y[0] = y[i];
	w_this = GMT_half_map_width (C, y[i]);
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		w_last = w_this;
		w_this = GMT_half_map_width (C, y[i]);
		if (GMT_this_point_wraps_x (C, x[i1], x[i], w_last, w_this)) {
			(*C->current.map.get_crossings) (C, xc, yc, x[i1], y[i1], x[i], y[i]);
			if (l_or_r == -1)
				C->current.plot.x[j] = GMT_left_boundary (C, yc[0]);
			else
				C->current.plot.x[j] = GMT_right_boundary (C, yc[0]);
			C->current.plot.y[j] = yc[0];
			j++;
			if (j >= C->current.plot.n_alloc) GMT_get_plot_array (C);
		}
		if (l_or_r == -1) /* Left */
			C->current.plot.x[j] = (x[i] >= C->current.map.half_width) ? GMT_left_boundary (C, y[i]) : x[i];
		else	/* Right */
			C->current.plot.x[j] = (x[i] < C->current.map.half_width) ? GMT_right_boundary (C, y[i]) : x[i];
		C->current.plot.y[j] = y[i];
		j++, k++;
		if (j >= C->current.plot.n_alloc) GMT_get_plot_array (C);
	}
	return (j);
}

GMT_LONG GMT_map_truncate_tm (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG b_or_t)
{	/* Truncates a wrapping polygon against bottom or top edge for global TM maps */

	GMT_LONG i, i1, j, k;
	double xc[4], yc[4], trunc_y;

	/* First initialize variables that differ for bottom and top truncation */

	if (b_or_t == -1) {	/* Bottom truncation (-1) */
		/* Find first point that is below map center */
		i = (y[start] < C->current.map.half_height) ? start : start - 1;
		trunc_y = 0.0;
	}
	else {				/* Top truncation (+1) */
		/* Find first point that is above map center */
		i = (y[start] > C->current.map.half_height) ? start : start - 1;
		trunc_y = C->current.map.height;
	}

	if (!C->current.plot.n_alloc) GMT_get_plot_array (C);

	C->current.plot.x[0] = x[i];	C->current.plot.y[0] = y[i];
	k = j = 1;
	while (k <= n) {
		i1 = i;
		i = (i + 1)%n;	/* Next point */
		if (GMT_this_point_wraps_tm (C, y[i1], y[i])) {
			GMT_get_crossings_tm (C, xc, yc, x[i1], y[i1], x[i], y[i]);
			C->current.plot.x[j] = xc[0];
			C->current.plot.y[j] = trunc_y;
			j++;
			if (j >= C->current.plot.n_alloc) GMT_get_plot_array (C);
		}
		if (b_or_t == -1) /* Bottom */
			C->current.plot.y[j] = (y[i] >= C->current.map.half_height) ? 0.0 : y[i];
		else	/* Top */
			C->current.plot.y[j] = (y[i] < C->current.map.half_height) ? C->current.map.height : y[i];
		C->current.plot.x[j] = x[i];
		j++, k++;
		if (j >= C->current.plot.n_alloc) GMT_get_plot_array (C);
	}
	return (j);
}

GMT_LONG GMT_map_truncate (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG side)
{	/* Truncates a wrapping polygon against left or right edge.
	   (bottom or top edge when projection is TM)
	   x, y : arrays of plot coordinates
	   n    : length of the arrays
	   start: first point of array to consider
	   side : -1 = left (bottom); +1 = right (top)
	*/

	if (C->current.proj.projection == GMT_TM)
		return (GMT_map_truncate_tm (C, x, y, n, start, side));
	else
		return (GMT_map_truncate_x (C, x, y, n, start, side));
}

/* GMT generic distance calculations between a pair of points in 2-D */

double GMT_distance (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE)
{	/* Generic function available to programs */
	return (C->current.map.dist[0].scale * C->current.map.dist[0].func (C, lonS, latS, lonE, latE));
}

double GMT_distance2 (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE, GMT_LONG id)
{	/* Generic function available to programs for contour/label distance calculations */
	return (C->current.map.dist[id].scale * C->current.map.dist[id].func (C, lonS, latS, lonE, latE));
}

GMT_LONG GMT_near_a_point (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, double dist)
{	/* Compute distance to nearest point in T from (lon, lat) */
	return (C->current.map.near_point_func (C, lon, lat, T, dist));
}

GMT_LONG GMT_near_lines (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{	/* Compute distance to nearest line in T from (lon,lat) */
	return (C->current.map.near_lines_func (C, lon, lat, T, return_mindist, dist_min, x_near, y_near));
}

GMT_LONG GMT_near_a_line (struct GMT_CTRL *C, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{	/* Compute distance to the line S from (lon,lat) */
	return (C->current.map.near_a_line_func (C, lon, lat, seg, S, return_mindist, dist_min, x_near, y_near));
}

/* Specific functions that are accessed via pointer only */

double GMT_cartesian_dist (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{	/* Calculates the good-old straight line distance in users units */
	return (hypot ( (x1 - x0), (y1 - y0)));
}

double GMT_cartesian_dist_proj (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2)
{	/* Calculates the good-old straight line distance after first projecting the data */
	double x0, y0, x1, y1;
	GMT_geo_to_xy (C, lon1, lat1, &x0, &y0);
	GMT_geo_to_xy (C, lon2, lat2, &x1, &y1);
	return (hypot ( (x1 - x0), (y1 - y0)));
}

double GMT_flatearth_dist_degree (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{
	/* Calculates the approximate flat earth distance in degrees.
	   If difference in longitudes exceeds 180 we pick the other
	   offset (360 - offset)
	 */
	double dlon = x1 - x0;
	
	if (fabs (dlon) > 180.0) dlon = copysign ((360.0 - fabs (dlon)), dlon);

	return (hypot ( dlon * cosd (0.5 * (y1 + y0)), (y1 - y0)));
}

double GMT_flatearth_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{
	/* Calculates the approximate flat earth distance in km.
	   If difference in longitudes exceeds 180 we pick the other
	   offset (360 - offset)
	 */
	return (GMT_flatearth_dist_degree (C, x0, y0, x1, y1) * C->current.proj.DIST_M_PR_DEG);
}

double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2)
{	/* great circle distance on a sphere in degrees */

	double cos_c = GMT_great_circle_dist_cos (C, lon1, lat1, lon2, lat2);
	return (d_acosd (cos_c));
}

double GMT_great_circle_dist_cos (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2)
{	/* great circle distance on a sphere in cos (angle) */

	double cosa, cosb, sina, sinb;

	if (lat1==lat2 && lon1==lon2) return (1.0);

	sincosd (lat1, &sina, &cosa);
	sincosd (lat2, &sinb, &cosb);

	return (sina*sinb + cosa*cosb*cosd(lon1-lon2));
}

double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1)
{	/* Calculates the great circle distance in meter */
	return (GMT_great_circle_dist_degree (C, x0, y0, x1, y1) * C->current.proj.DIST_M_PR_DEG);
}

double GMT_geodesic_dist_degree (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE)
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
	 * correction given by: 1-ECC2=1-2*F + F*F = C->current.proj.one_m_ECC2
	 */

	thg = atan (C->current.proj.one_m_ECC2 * tand (latE));
	sincos (thg, &c, &f);		f = -f;
	sincosd (lonE, &d, &e);		e = -e;
	a = f * e;
	b = -f * d;

	/* Calculate some trig constants. */

	thg = atan (C->current.proj.one_m_ECC2 * tand (latS));
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

double GMT_geodesic_dist_cos (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE)
{	/* Convenience function to get cosine instead */
	return (cosd (GMT_geodesic_dist_degree (C, lonS, latS, lonE, latE)));
}

double GMT_geodesic_dist_meter (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE)
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
	el = C->current.proj.ECC2 / C->current.proj.one_m_ECC2;
	e1 = 1.0 + el;
	sincosd (latE, &sinthi, &costhi);
	sincosd (latS, &sinthk, &costhk);
	sincosd (lonE - lonS, &sin_dl, &cos_dl);
	tanthi = sinthi / costhi;
	tanthk = sinthk / costhk;
	al = tanthi / (e1 * tanthk) + C->current.proj.ECC2 * sqrt ((e1 + tanthi * tanthi) / (e1 + tanthk * tanthk));
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
	v1 = C->current.proj.EQ_RAD / sqrt (1.0 - C->current.proj.ECC2 * sinthk * sinthk);
	v2 = C->current.proj.EQ_RAD / sqrt (1.0 - C->current.proj.ECC2 * sinthi * sinthi);
	z1 = v1 * (1.0 - C->current.proj.ECC2) * sinthk;
	z2 = v2 * (1.0 - C->current.proj.ECC2) * sinthi;
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

/* Functions dealing with distance between points */

double GMT_mindist_to_point (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG *id)
{
	GMT_LONG i, j;
	double d, d_min;

	d_min = DBL_MAX;
	for (i = 0; i < T->n_segments; i++) {
		for (j = 0; j < T->segment[i]->n_rows; j++) {
			d = GMT_distance (C, lon, lat, T->segment[i]->coord[GMT_X][j], T->segment[i]->coord[GMT_Y][j]);
			if (d < d_min) {	/* Update the shortest distance and the point responsible */
				d_min = d;	id[0] = i;	id[1] = j;
			}
		}
	}
	return (d_min);
}

GMT_LONG GMT_near_a_point_spherical (struct GMT_CTRL *C, double x, double y, struct GMT_TABLE *T, double dist)
{
	GMT_LONG i, j, inside = FALSE, each_point_has_distance;
	double d;

	each_point_has_distance = (dist <= 0.0 && T->segment[0]->n_columns > 2);
	for (i = 0; !inside && i < T->n_segments; i++) {
		for (j = 0; !inside && j < T->segment[i]->n_rows; j++) {
			d = GMT_distance (C, x, y, T->segment[i]->coord[GMT_X][j], T->segment[i]->coord[GMT_Y][j]);
			if (each_point_has_distance) dist = T->segment[i]->coord[GMT_Z][j];
			inside = (d <= dist);
		}
	}
	return (inside);
}

GMT_LONG GMT_near_a_point_cartesian (struct GMT_CTRL *C, double x, double y, struct GMT_TABLE *T, double dist)
{
	GMT_LONG i, j, inside = FALSE, each_point_has_distance;
	double d, x0, y0, xn, d0, dn;

	each_point_has_distance = (dist <= 0.0 && T->segment[0]->n_columns > 2);

	/* Assumes the points have been sorted so xp[0] is xmin and xp[n-1] is xmax] !!! */

	/* See if we are safely outside the range */
	x0 = T->segment[0]->coord[GMT_X][0];
	d0 = (each_point_has_distance) ? T->segment[0]->coord[GMT_Z][0] : dist;
	xn = T->segment[T->n_segments-1]->coord[GMT_X][T->segment[T->n_segments-1]->n_rows-1];
	dn = (each_point_has_distance) ? T->segment[T->n_segments-1]->coord[GMT_Z][T->segment[T->n_segments-1]->n_rows-1] : dist;
	if ((x < (x0 - d0)) || (x > (xn) + dn)) return (FALSE);

	/* No, must search the points */

	for (i = 0; !inside && i < T->n_segments; i++) {
		for (j = 0; !inside && j < T->segment[i]->n_rows; j++) {
			x0 = T->segment[i]->coord[GMT_X][j];
			d0 = (each_point_has_distance) ? T->segment[i]->coord[GMT_Z][j] : dist;
			if (fabs (x - x0) <= d0) {	/* Simple x-range test first */
				y0 = T->segment[i]->coord[GMT_Y][j];
				if (fabs (y - y0) <= d0) {	/* Simple y-range test next */
					/* Here we must compute distance */
					d = GMT_distance (C, x, y, x0, y0);
					inside = (d <= d0);
				}
			}
		}
	}
	return (inside);
}

/* Functions involving distance from arbitrary points to a line */

GMT_LONG GMT_near_lines_cartesian (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{
	GMT_LONG seg, mode = return_mindist, status, OK = FALSE;
	if (mode >= 10) mode -= 10;	/* Exclude (or flag) circular region surrounding line endpoints */
	if (mode) *dist_min = DBL_MAX;	/* Want to find the minimum distance so init to huge */

	for (seg = 0; seg < T->n_segments; seg++) {	/* Loop over each line segment */
		status = GMT_near_a_line_cartesian (C, lon, lat, seg, T->segment[seg], return_mindist, dist_min, x_near, y_near);
		if (status) {	/* Got a min distance or satisfied the min dist requirement */
			if (!return_mindist) return (TRUE);	/* Done, we are within distance of one of the lines */
			OK = TRUE;
		}
	}
	return (OK);	
}

GMT_LONG GMT_near_a_line_cartesian (struct GMT_CTRL *C, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{
	GMT_LONG j0, j1, perpendicular_only = FALSE, interior, within;
	double edge, dx, dy, xc, yc, s, s_inv, d, dist_AB, fraction;
	/* GMT_near_a_line_cartesian works in one of two modes, depending on return_mindist.
	   Since return_mindist is composed of two settings we must first set
	   perpendicular_only = (return_mindist >= 10);
	   return_mindist -= 10 * perpendicular_only;
	   That is, if 10 was added it means perpendicular_only is set and then the 10 is
	   removed.  We now consider what is left of return_mindist:
	   (1) return_mindist == 0:
	      We expect each segment to have its dist variable set to a minimum distance,
	      and if the point is within this distance from the line then we return TRUE;
	      otherwise we return FALSE.  If the segments have not set their distances then
	      it will have been initialized at 0 and only a point on the line will return TRUE.
	      If perpendicular_only we ignore a point that is within the distance of the
	      linesegment endpoints but project onto the extension of the line (i.e., it is
	      "outside" the extent of the line).  We return FALSE in that case.
	   (2) return_mindist != 0:
	      Return the minimum distance via dist_min. In addition, if > 1:
	      If == 2 we also return the coordinate of nearest point via x_near, y_near.
	      If == 3 we instead return segment number and point number (fractional) of that point via x_near, y_near.
	      The function will always return TRUE, except if perpendicular_only is set: then we
	      return FALSE if the point projects onto the extension of the line (i.e., it is "outside"
	      the extent of the line).  */

	if (return_mindist >= 10) {	/* Exclude circular region surrounding line endpoints */
		perpendicular_only = TRUE;
		return_mindist -= 10;
	}

	if (S->n_rows <= 0) return (FALSE);	/* empty; skip */

	if (return_mindist) S->dist = 0.0;	/* Explicitly set dist to zero so the shortest distance can be found */

	/* Find nearest point on this line */

	for (j0 = 0; j0 < S->n_rows; j0++) {	/* loop over nodes on current line */
		d = GMT_distance (C, lon, lat, S->coord[GMT_X][j0], S->coord[GMT_Y][j0]);	/* Distance between our point and j'th node on seg'th line */
		if (return_mindist && d < (*dist_min)) {	/* Update min distance */
			*dist_min = d;
			if (return_mindist == 2) { *x_near = S->coord[GMT_X][j0]; *y_near = S->coord[GMT_Y][j0]; }	/* Also update (x,y) of nearest point on the line */
			else if (return_mindist == 3) { *x_near = (double)seg; *y_near = (double)j0;}		/* Instead update (seg, pt) of nearest point on the line */
		}
		interior = (j0 > 0 && j0 < (S->n_rows - 1));	/* Only FALSE if we are processing one of the end points */
		if (d <= S->dist && (interior || !perpendicular_only)) return (TRUE);		/* Node inside the critical distance; we are done */
	}

	if (S->n_rows < 2) return (FALSE);	/* 1-point "line" is a point; skip segment check */

	/* If we get here we must check for intermediate points along the straight lines between segment nodes.
	 * However, since we know all nodes are outside the circle, we first check if the pair of nodes making
	 * up the next line segment are outside of the circumscribing square before we need to solve for the
	 * intersection between the line segment and the normal from our point. */

	for (j0 = 0, j1 = 1, within = FALSE; j1 < S->n_rows; j0++, j1++) {	/* loop over straight segments on current line */
		if (!return_mindist) {
			edge = lon - S->dist;
			if (S->coord[GMT_X][j0] < edge && S->coord[GMT_X][j1] < edge) continue;	/* Left of square */
			edge = lon + S->dist;
			if (S->coord[GMT_X][j0] > edge && S->coord[GMT_X][j1] > edge) continue;	/* Right of square */
			edge = lat - S->dist;
			if (S->coord[GMT_Y][j0] < edge && S->coord[GMT_Y][j1] < edge) continue;	/* Below square */
			edge = lat + S->dist;
			if (S->coord[GMT_Y][j0] > edge && S->coord[GMT_Y][j1] > edge) continue;	/* Above square */
		}

		/* Here there is potential for the line segment crossing inside the circle */

		dx = S->coord[GMT_X][j1] - S->coord[GMT_X][j0];
		dy = S->coord[GMT_Y][j1] - S->coord[GMT_Y][j0];
		if (dx == 0.0) {		/* Line segment is vertical, our normal is thus horizontal */
			if (dy == 0.0) continue;	/* Dummy segment with no length */
			xc = S->coord[GMT_X][j0];
			yc = lat;
			if (S->coord[GMT_Y][j0] < yc && S->coord[GMT_Y][j1] < yc ) continue;	/* Cross point is on extension */
			if (S->coord[GMT_Y][j0] > yc && S->coord[GMT_Y][j1] > yc ) continue;	/* Cross point is on extension */
		}
		else {	/* Line segment is not vertical */
			if (dy == 0.0) {	/* Line segment is horizontal, our normal is thus vertical */
				xc = lon;
				yc = S->coord[GMT_Y][j0];
			}
			else {	/* General case of oblique line */
				s = dy / dx;
				s_inv = -1.0 / s;
				xc = (lat - S->coord[GMT_Y][j0] + s * S->coord[GMT_X][j0] - s_inv * lon ) / (s - s_inv);
				yc = S->coord[GMT_Y][j0] + s * (xc - S->coord[GMT_X][j0]);

			}
			/* To be inside, (xc, yc) must (1) be on the line segment and not its extension and (2) be within dist of our point */

			if (S->coord[GMT_X][j0] < xc && S->coord[GMT_X][j1] < xc ) continue;	/* Cross point is on extension */
			if (S->coord[GMT_X][j0] > xc && S->coord[GMT_X][j1] > xc ) continue;	/* Cross point is on extension */
		}

		/* OK, here we must check how close the crossing point is */

		d = GMT_distance (C, lon, lat, xc, yc);			/* Distance between our point and intersection */
		if (return_mindist && d < (*dist_min)) {			/* Update min distance */
			*dist_min = d;
			if (return_mindist == 2) { *x_near = xc; *y_near = yc;}	/* Also update nearest point on the line */
			else if (return_mindist == 3) {	/* Instead update (seg, pt) of nearest point on the line */
				*x_near = (double)seg;
				dist_AB = GMT_distance (C, S->coord[GMT_X][j0], S->coord[GMT_Y][j0], S->coord[GMT_X][j1], S->coord[GMT_Y][j1]);
				fraction = (dist_AB > 0.0) ? GMT_distance (C, S->coord[GMT_X][j0], S->coord[GMT_Y][j0], xc, yc) / dist_AB : 0.0;
				*y_near = (double)j0 + fraction;
			}
			within = TRUE;
		}
		if (d <= S->dist) return (TRUE);		/* Node inside the critical distance; we are done */
	}

	return (within);	/* All tests failed, we are not close to the line(s), or we just return distance and interior (see comments above) */
}

GMT_LONG GMT_near_lines_spherical (struct GMT_CTRL *P, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{
	GMT_LONG seg, mode = return_mindist, status, OK = FALSE;
	if (mode >= 10) mode -= 10;	/* Exclude (or flag) circular region surrounding line endpoints */
	if (mode) *dist_min = DBL_MAX;	/* Want to find the minimum distance so init to huge */

	for (seg = 0; seg < T->n_segments; seg++) {	/* Loop over each line segment */
		status = GMT_near_a_line_spherical (P, lon, lat, seg, T->segment[seg], return_mindist, dist_min, x_near, y_near);
		if (status) {	/* Got a min distance or satisfied the min dist requirement */
			if (!return_mindist) return (TRUE);	/* Done, we are within distance of one of the lines */
			OK = TRUE;
		}
	}
	return (OK);
}

GMT_LONG GMT_near_a_line_spherical (struct GMT_CTRL *P, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near)
{
	GMT_LONG row, j0, perpendicular_only = FALSE, interior, within;
	double d, A[3], B[3], C[3], X[3], xlon, xlat, cx_dist, cos_dist, dist_AB, fraction;

	/* GMT_near_a_line_spherical works in one of two modes, depending on return_mindist.
	   Since return_mindist is composed of two settings we must first set
	   perpendicular_only = (return_mindist >= 10);
	   return_mindist -= 10 * perpendicular_only;
	   That is, if 10 was added it means perpendicular_only is set and then the 10 is
	   removed.  We now consider what is left of return_mindist:
	   (1) return_mindist == 0:
	      We expect each segment to have its dist variable set to a minimum distance,
	      and if the point is within this distance from the line then we return TRUE;
	      otherwise we return FALSE.  If the segments have not set their distances then
	      it will have been initialized at 0 and only a point on the line will return TRUE.
	      If perpendicular_only we ignore a point that is within the distance of the
	      linesegment endpoints but project onto the extension of the line (i.e., it is
	      "outside" the extent of the line).  We return FALSE in that case.
	   (2) return_mindist != 0:
	      Return the minimum distance via dist_min. In addition, if > 1:
	      If == 2 we also return the coordinate of nearest point via x_near, y_near.
	      If == 3 we instead return segment number and point number (fractional) of that point via x_near, y_near.
	      The function will always return TRUE, except if perpendicular_only is set: then we
	      return FALSE if the point projects onto the extension of the line (i.e., it is "outside"
	      the extent of the line).  */

	if (return_mindist >= 10) {	/* Exclude (or flag) circular region surrounding line endpoints */
		perpendicular_only = TRUE;
		return_mindist -= 10;
	}
	GMT_geo_to_cart (P, lat, lon, C, TRUE);	/* Our point to test is now C */

	if (S->n_rows <= 0) return (FALSE);	/* Empty ; skip */

	/* Find nearest point on this line */

	if (return_mindist) S->dist = 0.0;	/* Explicitly set dist to zero so the shortest distance can be found */

	for (row = 0; row < S->n_rows; row++) {	/* loop over nodes on current line */
		d = GMT_distance (P, lon, lat, S->coord[GMT_X][row], S->coord[GMT_Y][row]);	/* Distance between our point and row'th node on seg'th line */
		if (return_mindist && d < (*dist_min)) {	/* Update minimum distance */
			*dist_min = d;
			if (return_mindist == 2) *x_near = S->coord[GMT_X][row], *y_near = S->coord[GMT_Y][row];	/* Also update (x,y) of nearest point on the line */
			if (return_mindist == 3) *x_near = (double)seg, *y_near = (double)row;	/* Also update (seg, pt) of nearest point on the line */
		}
		interior = (row > 0 && row < (S->n_rows - 1));	/* Only FALSE if we are processing one of the end points */
		if (d <= S->dist && (interior || !perpendicular_only)) return (TRUE);			/* Node inside the critical distance; we are done */
	}

	if (S->n_rows < 2) return (FALSE);	/* 1-point "line" is a point; skip segment check */

	/* If we get here we must check for intermediate points along the great circle lines between segment nodes.*/

	if (return_mindist)		/* Cosine of the great circle distance we are checking for. 2 ensures failure to be closer */
		cos_dist = 2.0;
	else if (P->current.map.dist[GMT_MAP_DIST].arc)	/* Used angular distance measure */
		cos_dist = cosd (S->dist / P->current.map.dist[GMT_MAP_DIST].scale);
	else	/* Used distance units (e.g., meter, km). Conv to meters, then to degrees */
		cos_dist = cosd ((S->dist / P->current.map.dist[GMT_MAP_DIST].scale) / P->current.proj.DIST_M_PR_DEG);
	GMT_geo_to_cart (P, S->coord[GMT_Y][0], S->coord[GMT_X][0], B, TRUE);		/* 3-D vector of end of last segment */

	for (row = 1, within = FALSE; row < S->n_rows; row++) {				/* loop over great circle segments on current line */
		GMT_memcpy (A, B, 3, double);	/* End of last segment is start of new segment */
		GMT_geo_to_cart (P, S->coord[GMT_Y][row], S->coord[GMT_X][row], B, TRUE);	/* 3-D vector of end of this segment */
		if (GMT_great_circle_intersection (P, A, B, C, X, &cx_dist)) continue;	/* X not between A and B */
		if (return_mindist) {		/* Get lon, lat of X, calculate distance, and update min_dist if needed */
			GMT_cart_to_geo (P, &xlat, &xlon, X, TRUE);
			d = GMT_distance (P, xlon, xlat, lon, lat);	/* Distance between our point and closest perpendicular point on seg'th line */
			if (d < (*dist_min)) {	/* Update minimum distance */
				*dist_min = d;
				if (return_mindist == 2) { *x_near = xlon; *y_near = xlat;}	/* Also update (x,y) of nearest point on the line */
				else if (return_mindist == 3) {	/* Also update (seg, pt) of nearest point on the line */
					*x_near = (double)seg;
					j0 = row - 1;
					dist_AB = GMT_distance (P, S->coord[GMT_X][j0], S->coord[GMT_Y][j0], S->coord[GMT_X][row], S->coord[GMT_Y][row]);
					fraction = (dist_AB > 0.0) ? GMT_distance (P, S->coord[GMT_X][j0], S->coord[GMT_Y][j0], xlon, xlat) / dist_AB : 0.0;
					*y_near = (double)j0 + fraction;
				}
				within = TRUE;	/* Found at least one segment with a valid inside distance */
			}
		}
		if (cx_dist >= cos_dist) return (TRUE);	/* X is on the A-B extension AND within specified distance */
	}

	return (within);	/* All tests failed, we are not close to the line(s), or we return a mindist (see comments above) */
}

GMT_LONG GMT_great_circle_intersection (struct GMT_CTRL *T, double A[], double B[], double C[], double X[], double *CX_dist)
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

	GMT_cross3v (T, A, B, P);			/* Get pole position of plane through A and B (and origin O) */
	GMT_normalize3v (T, P);			/* Make sure P has unit length */
	GMT_cross3v (T, C, P, E);			/* Get pole E to plane through C (and origin) but normal to A,B (hence going through P) */
	GMT_normalize3v (T, E);			/* Make sure E has unit length */
	GMT_cross3v (T, P, E, X);			/* Intersection between the two planes is oriented line*/
	GMT_normalize3v (T, X);			/* Make sure X has unit length */
	/* The X we want could be +x or -X; must determine which might be closest to A-B midpoint M */
	for (i = 0; i < 3; i++) {
		M[i] = A[i] + B[i];
		Xneg[i] = -X[i];
	}
	GMT_normalize3v (T, M);			/* Make sure M has unit length */
	/* Must first check if X is along the (A,B) segment and not on its extension */

	cos_MX1 = GMT_dot3v (T, M, X);		/* Cos of spherical distance between M and +X */
	cos_MX2 = GMT_dot3v (T, M, Xneg);	/* Cos of spherical distance between M and -X */
	if (cos_MX2 > cos_MX1) GMT_memcpy (X, Xneg, 3, double);		/* -X is closest to A-B midpoint */
	cos_AB = fabs (GMT_dot3v (T, A, B));	/* Cos of spherical distance between A,B */
	cos_test = fabs (GMT_dot3v (T, A, X));	/* Cos of spherical distance between A and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to A exceeds the A-B length */
	cos_test = fabs (GMT_dot3v (T, B, X));	/* Cos of spherical distance between B and X */
	if (cos_test < cos_AB) return 1;	/* X must be on the A-B extension if its distance to B exceeds the A-B length */

	/* X is between A and B.  Now calculate distance between C and X */

	*CX_dist = GMT_dot3v (T, C, X);		/* Cos of spherical distance between C and X */
	return (0);				/* Return zero if intersection is between A and B */
}

GMT_LONG *GMT_split_line (struct GMT_CTRL *C, double **xx, double **yy, GMT_LONG *nn, GMT_LONG add_crossings)
{	/* Accepts x/y array for a line in projected inches and looks for
	 * map jumps.  If found it will insert the boundary crossing points and
	 * build a split integer array with the nodes of the first point
	 * for each segment.  The number of segments is returned.  If
	 * no jumps are found then NULL is returned.  This function is needed
	 * so that the new PS contouring machinery only sees lines that do no
	 * jump across the map.
	 * add_crossings is TRUE if we need to find the crossings; FALSE means
	 * they are already part of the line. */

	GMT_LONG i, j, k, n, n_seg, *split = NULL, *pos = NULL, *way = NULL, l_or_r, n_alloc = 0;
	double *x = NULL, *y = NULL, *xin = NULL, *yin = NULL, xc[2], yc[2];

	/* First quick scan to see how many jumps there are */

	xin = *xx;	yin = *yy;
	GMT_set_meminc (C, GMT_SMALL_CHUNK);
	for (n_seg = 0, i = 1; i < *nn; i++) {
		if ((l_or_r = GMT_map_jump_x (C, xin[i], yin[i], xin[i-1], yin[i-1]))) {
			if (n_seg == n_alloc) n_alloc = GMT_malloc2 (C, pos, way, n_seg, n_alloc, GMT_LONG);
			pos[n_seg] = i;		/* 2nd of the two points that generate the jump */
			way[n_seg] = (short int)l_or_r;		/* Which way we jump : +1 is right to left, -1 is left to right */
			n_seg++;
		}
	}
	GMT_reset_meminc (C);

	if (n_seg == 0) return ((GMT_LONG *)NULL);	/* No jumps, just return NULL */

	/* Here we have one or more jumps so we need to split the line */

	n = *nn;				/* Original line count */
	if (add_crossings) n += 2 * n_seg;	/* Must add 2 crossing points per jump */
	(void)GMT_malloc2 (C, x, y, n, 0, double);
	split = GMT_memory (C, NULL, n_seg+2, GMT_LONG);
	split[0] = n_seg;

	x[0] = xin[0];	y[0] = yin[0];
	for (i = j = 1, k = 0; i < *nn; i++, j++) {
		if (k < n_seg && i == pos[k]) {	/* At jump point */
			if (add_crossings) {	/* Find and insert the crossings */
				GMT_get_crossings_x (C, xc, yc, xin[i], yin[i], xin[i-1], yin[i-1]);
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
		x[j] = xin[i];	y[j] = yin[i];
	}
	split[++k] = j;		/* End of last segment */

	/* Time to return the pointers to new data */

	GMT_free (C, pos);
	GMT_free (C, way);
	GMT_free (C, xin);
	GMT_free (C, yin);
	*xx = x;
	*yy = y;
	*nn = j;

	return (split);
}

/*  Routines to add pieces of parallels or meridians */

GMT_LONG GMT_graticule_path (struct GMT_CTRL *C, double **x, double **y, GMT_LONG dir, double w, double e, double s, double n)
{	/* Returns the path of a graticule (box of meridians and parallels) */
	GMT_LONG np;
	double *xx = NULL, *yy = NULL;
	double px0, px1, px2, px3;

	if (dir == 1) {	/* Forward sense */
		px0 = px3 = w;	px1 = px2 = e;
	}
	else {	/* Reverse sense */
		px0 = px3 = e;	px1 = px2 = w;
	}

	/* Close graticule from point 0 through point 4 */

	if (GMT_IS_RECT_GRATICULE(C)) {	/* Simple rectangle in this projection */
		np = GMT_malloc2 (C, xx, yy, 5, 0, double);
		xx[0] = xx[4] = px0;	xx[1] = px1;	xx[2] = px2;	xx[3] = px3;
		yy[0] = yy[1] = yy[4] = s;	yy[2] = yy[3] = n;
	}
	else {	/* Must assemble path from meridians and parallel pieces */
		double *xtmp = NULL, *ytmp = NULL;
		GMT_LONG add, n_alloc = 0;

		/* SOUTH BORDER */

		if (GMT_is_geographic (C, GMT_IN) && s == -90.0) {	/* No path, just a point */
			np = n_alloc = GMT_malloc2 (C, xx, yy, 1, 0, double);
			xx[0] = px1;	yy[0] = -90.0;
		}
		else
			np = n_alloc = GMT_latpath (C, s, px0, px1, &xx, &yy);	/* South */

		/* EAST (OR WEST) BORDER */

		add = GMT_lonpath (C, px1, s, n, &xtmp, &ytmp);	/* east (or west if dir == -1) */
		n_alloc = GMT_malloc2 (C, xx, yy, n_alloc + add, n_alloc, double);
		GMT_memcpy (&xx[np], xtmp, add, double);
		GMT_memcpy (&yy[np], ytmp, add, double);
		np += add;
		GMT_free (C, xtmp);	GMT_free (C, ytmp);

		/* NORTH BORDER */

		if (GMT_is_geographic (C, GMT_IN) && n == 90.0) {	/* No path, just a point */
			add = GMT_malloc2 (C, xtmp, ytmp, 1, 0, double);
			xtmp[0] = px3;	ytmp[0] = +90.0;
		}
		else
			add = GMT_latpath (C, n, px2, px3, &xtmp, &ytmp);	/* North */

		n_alloc = GMT_malloc2 (C, xx, yy, n_alloc + add, n_alloc, double);
		GMT_memcpy (&xx[np], xtmp, add, double);
		GMT_memcpy (&yy[np], ytmp, add, double);
		np += add;
		GMT_free (C, xtmp);	GMT_free (C, ytmp);

		/* WEST (OR EAST) BORDER */

		add = GMT_lonpath (C, px3, n, s, &xtmp, &ytmp);	/* west */
		n_alloc = GMT_malloc2 (C, xx, yy, n_alloc + add, n_alloc, double);
		GMT_memcpy (&xx[np], xtmp, add, double);
		GMT_memcpy (&yy[np], ytmp, add, double);
		np += add;
		GMT_free (C, xtmp);	GMT_free (C, ytmp);
		(void)GMT_malloc2 (C, xx, yy, 0, np, double);
	}

	if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON) {
		GMT_LONG straddle;
		GMT_LONG i;
		straddle = (C->common.R.wesn[XLO] < 0.0 && C->common.R.wesn[XHI] > 0.0);
		for (i = 0; straddle && i < np; i++) {
			while (xx[i] < 0.0) xx[i] += 360.0;
			if (straddle && xx[i] > 180.0) xx[i] -= 360.0;
		}
	}

	*x = xx;
	*y = yy;
	return (np);
}

GMT_LONG GMT_map_path (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double **x, double **y)
{
	if (GMT_IS_ZERO (lat1 - lat2))
		return (GMT_latpath (C, lat1, lon1, lon2, x, y));
	else
		return (GMT_lonpath (C, lon1, lat1, lat2, x, y));
}

GMT_LONG GMT_lonpath (struct GMT_CTRL *C, double lon, double lat1, double lat2, double **x, double **y)
{
	GMT_LONG ny, n, n_try, keep_trying, pos;
	double dlat, dlat0, *tlon = NULL, *tlat = NULL, x0, x1, y0, y1, d, min_gap;

	if (C->current.map.meridian_straight == 2) {	/* Special non-sampling for gmtselect/grdlandmask */
		n = GMT_malloc2 (C, tlon, tlat, 2, 0, double);
		tlon[0] = tlon[1] = lon;
		tlat[0] = lat1;	tlat[1] = lat2;
		*x = tlon;
		*y = tlat;
		return (n);
	}

	if (C->current.map.meridian_straight) {	/* Easy, just a straight line connect via quarter-points */
		n = GMT_malloc2 (C, tlon, tlat, 5, 0, double);
		tlon[0] = tlon[1] = tlon[2] = tlon[3] = tlon[4] = lon;
		dlat = lat2 - lat1;
		tlat[0] = lat1;	tlat[1] = lat1 + 0.25 * dlat;	tlat[2] = lat1 + 0.5 * dlat;
		tlat[3] = lat1 + 0.75 * dlat;	tlat[4] = lat2;
		*x = tlon;
		*y = tlat;
		return (n);
	}

	/* Must do general case */
	n = 0;
	min_gap = 0.1 * C->current.setting.map_line_step;
	if ((ny = (GMT_LONG)ceil (fabs (lat2 - lat1) / C->current.map.dlat)) == 0) return (0);

	ny++;
	dlat0 = (lat2 - lat1) / ny;
	pos = (dlat0 > 0.0);

	ny = GMT_malloc2 (C, tlon, tlat, ny, 0, double);

	tlon[0] = lon;
	tlat[0] = lat1;
	GMT_geo_to_xy (C, tlon[0], tlat[0], &x0, &y0);
	while ((pos && (tlat[n] < lat2)) || (!pos && (tlat[n] > lat2))) {
		n++;
		if (n == ny-1) {
			ny += GMT_SMALL_CHUNK;
			tlon = GMT_memory (C, tlon, ny, double);
			tlat = GMT_memory (C, tlat, ny, double);
		}
		n_try = 0;
		keep_trying = TRUE;
		dlat = dlat0;
		tlon[n] = lon;
		do {
			n_try++;
			tlat[n] = tlat[n-1] + dlat;
			if (GMT_is_geographic (C, GMT_IN) && fabs (tlat[n]) > 90.0) tlat[n] = copysign (90.0, tlat[n]);
			GMT_geo_to_xy (C, tlon[n], tlat[n], &x1, &y1);
			if ((*C->current.map.jump) (C, x0, y0, x1, y1) || (y0 < C->current.proj.rect[YLO] || y0 > C->current.proj.rect[YHI]))
				keep_trying = FALSE;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > C->current.setting.map_line_step)
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
		tlon = GMT_memory (C, tlon, n, double);
		tlat = GMT_memory (C, tlat, n, double);
	}

	*x = tlon;	*y = tlat;
	return (n);
}

GMT_LONG GMT_latpath (struct GMT_CTRL *C, double lat, double lon1, double lon2, double **x, double **y)
{
	GMT_LONG n_alloc, n, n_try, keep_trying, pos;
	double dlon, dlon0, *tlon = NULL, *tlat = NULL, x0, x1, y0, y1, d, min_gap;

	if (C->current.map.parallel_straight == 2) {	/* Special non-sampling for gmtselect/grdlandmask */
		n = GMT_malloc2 (C, tlon, tlat, 2, 0, double);
		tlat[0] = tlat[1] = lat;
		tlon[0] = lon1;	tlon[1] = lon2;
		*x = tlon;	*y = tlat;
		return (n);
	}
	if (C->current.map.parallel_straight) {	/* Easy, just a straight line connection via quarter points */
		n = GMT_malloc2 (C, tlon, tlat, 5, 0, double);
		tlat[0] = tlat[1] = tlat[2] = tlat[3] = tlat[4] = lat;
		dlon = lon2 - lon1;
		tlon[0] = lon1;	tlon[1] = lon1 + 0.25 * dlon;	tlon[2] = lon1 + 0.5 * dlon;
		tlon[3] = lon1 + 0.75 * dlon;	tlon[4] = lon2;
		*x = tlon;	*y = tlat;
		return (n);
	}
	/* Here we try to walk along lat for small increment in longitude to make sure our steps are smaller than the line_step */
	min_gap = 0.1 * C->current.setting.map_line_step;
	if ((n_alloc = (GMT_LONG)ceil (fabs (lon2 - lon1) / C->current.map.dlon)) == 0) return (0);	/* Initial guess to path length */

	n_alloc++;
	dlon0 = (lon2 - lon1) / n_alloc;
	pos = (dlon0 > 0.0);

	n_alloc = GMT_malloc2 (C, tlon, tlat, n_alloc, 0, double);
	tlon[0] = lon1;	tlat[0] = lat;
	GMT_geo_to_xy (C, tlon[0], tlat[0], &x0, &y0);
	n = 0;
	while ((pos && (tlon[n] < lon2)) || (!pos && (tlon[n] > lon2))) {
		n++;
		if (n == n_alloc) n_alloc = GMT_malloc2 (C, tlon, tlat, n, n_alloc, double);
		n_try = 0;
		keep_trying = TRUE;
		dlon = dlon0;
		tlat[n] = lat;
		do {
			n_try++;
			tlon[n] = tlon[n-1] + dlon;
			GMT_geo_to_xy (C, tlon[n], tlat[n], &x1, &y1);
			if ((*C->current.map.jump) (C, x0, y0, x1, y1) || (y0 < C->current.proj.rect[YLO] || y0 > C->current.proj.rect[YHI]))
				keep_trying = FALSE;
			else {
				d = hypot (x1 - x0, y1 - y0);
				if (d > C->current.setting.map_line_step)
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
	(void)GMT_malloc2 (C, tlon, tlat, 0, n, double);

	*x = tlon;	*y = tlat;
	return (n);
}

#ifdef DEBUG
/* If we need to dump out clipped polygon then set clip_dump = 1 during execution */
void dumppol (GMT_LONG n, double *x, double *y, GMT_LONG *id)
{
	GMT_LONG i;
	FILE *fp = NULL;
	char line[64];
	sprintf (line, "dump_%ld.d", *id);
	fp = fopen (line, "w");
	for (i = 0; i < n; i++) fprintf (fp, "%g\t%g\n", x[i], y[i]);
	fclose (fp);
	(*id)++;
}
#endif

GMT_LONG GMT_geo_to_xy_line (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n)
{
	/* Traces the lon/lat array and returns x,y plus appropriate pen moves
	 * Pen moves are caused by breakthroughs of the map boundary or when
	 * a point has lon = NaN or lat = NaN (this means "pick up pen") */
	GMT_LONG j, np, inside, sides[4], nx;
	double xlon[4], xlat[4], xx[4], yy[4];
	double this_x, this_y, last_x, last_y, dummy[4];

	while (n > C->current.plot.n_alloc) GMT_get_plot_array (C);

	np = 0;
	GMT_geo_to_xy (C, lon[0], lat[0], &last_x, &last_y);
	if (!GMT_map_outside (C, lon[0], lat[0])) {
		C->current.plot.x[0] = last_x;	C->current.plot.y[0] = last_y;
		C->current.plot.pen[np++] = PSL_MOVE;
	}
	for (j = 1; j < n; j++) {
		GMT_geo_to_xy (C, lon[j], lat[j], &this_x, &this_y);
		inside = !GMT_map_outside (C, lon[j], lat[j]);
		if (GMT_is_dnan (lon[j]) || GMT_is_dnan (lat[j])) continue;	/* Skip NaN point now */
		if (GMT_is_dnan (lon[j-1]) || GMT_is_dnan (lat[j-1])) {		/* Point after NaN needs a move */
			C->current.plot.x[np] = this_x;	C->current.plot.y[np] = this_y;
			C->current.plot.pen[np++] = PSL_MOVE;
			if (np == C->current.plot.n_alloc) GMT_get_plot_array (C);
			last_x = this_x;	last_y = this_y;
			continue;
		}
		if ((nx = GMT_map_crossing (C, lon[j-1], lat[j-1], lon[j], lat[j], xlon, xlat, xx, yy, sides))) { /* Nothing */ }
		else if (C->current.map.is_world)
			nx = (*C->current.map.wrap_around_check) (C, dummy, last_x, last_y, this_x, this_y, xx, yy, sides);
		if (nx == 1) {	/* inside-outside or outside-inside */
			if (GMT_is_dnan (yy[0])) fprintf (stderr, "y[0] NaN\n");
			C->current.plot.x[np] = xx[0];	C->current.plot.y[np] = yy[0];
			C->current.plot.pen[np++] = (inside) ? PSL_MOVE : PSL_DRAW;
			if (np == C->current.plot.n_alloc) GMT_get_plot_array (C);
		}
		else if (nx == 2) {	/* outside-inside-outside or (with wrapping) inside-outside-inside */
			C->current.plot.x[np] = xx[0];	C->current.plot.y[np] = yy[0];
			C->current.plot.pen[np++] = (inside) ? PSL_DRAW : PSL_MOVE;
			if (np == C->current.plot.n_alloc) GMT_get_plot_array (C);
			C->current.plot.x[np] = xx[1];	C->current.plot.y[np] = yy[1];
			C->current.plot.pen[np++] = (inside) ? PSL_MOVE : PSL_DRAW;
			if (np == C->current.plot.n_alloc) GMT_get_plot_array (C);
		}
		if (inside) {
			if ( np >= C->current.plot.n_alloc ) {
				fprintf(stderr, "bad access: cannot access current.plot.x[%ld], np=%ld, C->current.plot.n=%ld\n", np, np, C->current.plot.n);
			}
			else {
				C->current.plot.x[np] = this_x;	C->current.plot.y[np] = this_y;
				C->current.plot.pen[np++] = PSL_DRAW;
			}
			if (np == C->current.plot.n_alloc) GMT_get_plot_array (C);
		}
		last_x = this_x;	last_y = this_y;
	}
	if (np) C->current.plot.pen[0] = PSL_MOVE;	/* Sanity override: Gotta start off with new start point */
	return (np);
}

GMT_LONG GMT_compact_line (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG pen_flag, int *pen)
{	/* TRUE if pen movements is present */
	/* GMT_compact_line will remove unnecessary points in paths */
	GMT_LONG i, j;
	double old_slope, new_slope, dx;
	char *flag = NULL;

	if (n < 3) return (n);	/* Nothing to do */
	flag = GMT_memory (C, NULL, n, char);

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
	GMT_free (C, flag);

	return (j);
}

/* Routines to transform grdfiles to/from map projections */

GMT_LONG GMT_grdproject_init (struct GMT_CTRL *C, struct GMT_GRID *G, double *inc, GMT_LONG nx, GMT_LONG ny, GMT_LONG dpi, GMT_LONG offset)
{
	if (inc[GMT_X] > 0.0 && inc[GMT_Y] > 0.0) {
		G->header->nx = (int)GMT_get_n (G->header->wesn[XLO], G->header->wesn[XHI], inc[GMT_X], offset);
		G->header->ny = (int)GMT_get_n (G->header->wesn[YLO], G->header->wesn[YHI], inc[GMT_Y], offset);
		G->header->inc[GMT_X] = GMT_get_inc (G->header->wesn[XLO], G->header->wesn[XHI], G->header->nx, offset);
		G->header->inc[GMT_Y] = GMT_get_inc (G->header->wesn[YLO], G->header->wesn[YHI], G->header->ny, offset);
	}
	else if (nx > 0 && ny > 0) {
		G->header->nx = (int)nx;	G->header->ny = (int)ny;
		G->header->inc[GMT_X] = GMT_get_inc (G->header->wesn[XLO], G->header->wesn[XHI], G->header->nx, offset);
		G->header->inc[GMT_Y] = GMT_get_inc (G->header->wesn[YLO], G->header->wesn[YHI], G->header->ny, offset);
	}
	else if (dpi > 0) {
		G->header->nx = (int)irint ((G->header->wesn[XHI] - G->header->wesn[XLO]) * dpi) + 1 - (int)offset;
		G->header->ny = (int)irint ((G->header->wesn[YHI] - G->header->wesn[YLO]) * dpi) + 1 - (int)offset;
		G->header->inc[GMT_X] = GMT_get_inc (G->header->wesn[XLO], G->header->wesn[XHI], G->header->nx, offset);
		G->header->inc[GMT_Y] = GMT_get_inc (G->header->wesn[YLO], G->header->wesn[YHI], G->header->ny, offset);
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "GMT_grdproject_init: Necessary arguments not set\n");
		GMT_exit (EXIT_FAILURE);
	}
	G->header->registration = (int)offset;

	GMT_RI_prepare (C, G->header);	/* Ensure -R -I consistency and set nx, ny */
	GMT_err_pass (C, GMT_grd_RI_verify (C, G->header, 1), "");
	GMT_grd_setpad (G->header, C->current.io.pad);			/* Assign default pad */
	GMT_set_grddim (C, G->header);	/* Set all dimensions before returning */

	GMT_report (C, GMT_MSG_NORMAL, "Grid projection from size %ldx%ld to %dx%d\n", nx, ny, G->header->nx, G->header->ny);
	return (GMT_NOERROR);
}

GMT_LONG GMT_project_init (struct GMT_CTRL *C, struct GRD_HEADER *header, double *inc, GMT_LONG nx, GMT_LONG ny, GMT_LONG dpi, GMT_LONG offset)
{
	if (inc[GMT_X] > 0.0 && inc[GMT_Y] > 0.0) {
		header->nx = (int)GMT_get_n (header->wesn[XLO], header->wesn[XHI], inc[GMT_X], offset);
		header->ny = (int)GMT_get_n (header->wesn[YLO], header->wesn[YHI], inc[GMT_Y], offset);
		header->inc[GMT_X] = GMT_get_inc (header->wesn[XLO], header->wesn[XHI], header->nx, offset);
		header->inc[GMT_Y] = GMT_get_inc (header->wesn[YLO], header->wesn[YHI], header->ny, offset);
	}
	else if (nx > 0 && ny > 0) {
		header->nx = (int)nx;	header->ny = (int)ny;
		header->inc[GMT_X] = GMT_get_inc (header->wesn[XLO], header->wesn[XHI], header->nx, offset);
		header->inc[GMT_Y] = GMT_get_inc (header->wesn[YLO], header->wesn[YHI], header->ny, offset);
	}
	else if (dpi > 0) {
		header->nx = (int)irint ((header->wesn[XHI] - header->wesn[XLO]) * dpi) + 1 - (int)offset;
		header->ny = (int)irint ((header->wesn[YHI] - header->wesn[YLO]) * dpi) + 1 - (int)offset;
		header->inc[GMT_X] = GMT_get_inc (header->wesn[XLO], header->wesn[XHI], header->nx, offset);
		header->inc[GMT_Y] = GMT_get_inc (header->wesn[YLO], header->wesn[YHI], header->ny, offset);
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "GMT_grdproject_init: Necessary arguments not set\n");
		GMT_exit (EXIT_FAILURE);
	}
	header->registration = (int)offset;

	GMT_RI_prepare (C, header);	/* Ensure -R -I consistency and set nx, ny */
	GMT_err_pass (C, GMT_grd_RI_verify (C, header, 1), "");
	GMT_grd_setpad (header, C->current.io.pad);			/* Assign default pad */
	GMT_set_grddim (C, header);	/* Set all dimensions before returning */

	GMT_report (C, GMT_MSG_NORMAL, "Grid projection from size %ldx%ld to %dx%d\n", nx, ny, header->nx, header->ny);
	return (GMT_NOERROR);
}


GMT_LONG GMT_grd_project (struct GMT_CTRL *C, struct GMT_GRID *I, struct GMT_GRID *O, struct GMT_EDGEINFO *edgeinfo, GMT_LONG inverse)
{
	/* Generalized grid projection that deals with both interpolation and averaging effects.
	 * It requires the input grid to have 2 boundary rows/cols so that the bcr
	 * functions can be used.  The I struct represents the input grid which is either in original
	 * (i.e., lon/lat) coordinates or projected x/y (if inverse = TRUE).
	 *
	 * I:	Grid and header with input grid on a padded grid with 2 extra rows/columns
	 * O:	Grid and header for output grid, no padding needed (but allowed)
	 * edgeinfo:	Structure with information about boundary conditions on input grid
	 * inverse:	TRUE if input is x/y and we want to invert for a lon/lat grid
	 *
	 * In addition, these settings (via -n) control interpolation:
	 * antialias:	TRUE if we need to do the antialiasing STEP 1 (below)
	 * interpolant:	0 = nearest neighbor, 1 = bilinear, 2 = B-spline, 3 = bicubic
	 * threshold:	minumum weight to be used. If weight < threshold interpolation yields NaN.
	 * We initialize the O->data array to NaN.
	 *
	 * Changed 10-Sep-07 to include the argument "antialias" and "threshold" and
	 * made "interpolant" an integer (was GMT_LONG bilinear).
	 */

	GMT_LONG col_in, row_in, ij_in, col_out, row_out, ij_out;
	short int *nz = NULL;
	double x_proj, y_proj, z_int, inv_nz;
	double *x_in = NULL, *x_out = NULL, *x_in_proj = NULL, *x_out_proj = NULL;
	double *y_in = NULL, *y_out = NULL, *y_in_proj = NULL, *y_out_proj = NULL;
	struct GMT_BCR bcr;

	/* Only input grid MUST have at least 2 rows/cols padding */
	if (I->header->pad[XLO] < 2 || I->header->pad[XHI] < 2 || I->header->pad[YLO] < 2 || I->header->pad[YHI] < 2) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_grd_project: Input grid does not have sufficient (2) padding\n");
		GMT_exit (EXIT_FAILURE);
	}

	GMT_boundcond_param_prep (C, I->header, edgeinfo);	/* Init the BC parameters */

	/* Initialize bcr structure:
	   Threshold changed 10 Sep 07 by RS from 1.0 to <threshold> to allow interpolation closer to
	   a NaN value. In case of bilinear interpolation, using 1.0 creates a rectangular hole
	   the size of 4 grid cells for a single NaN grid node. Using 0.25 will create a diamond
	   shaped hole the size of one cell. */
	GMT_bcr_init (C, I->header, C->common.n.interpolant, C->common.n.threshold, &bcr);

	/* Set boundary conditions  */

	GMT_boundcond_grid_set (C, I, edgeinfo);

	x_in  = GMT_memory (C, NULL, I->header->nx, double);
	y_in  = GMT_memory (C, NULL, I->header->ny, double);
	x_out = GMT_memory (C, NULL, O->header->nx, double);
	y_out = GMT_memory (C, NULL, O->header->ny, double);

	/* Precalculate grid coordinates */

	GMT_row_loop  (I, row_in) y_in[row_in] = GMT_grd_row_to_y (row_in, I->header);
	GMT_col_loop2 (I, col_in) x_in[col_in] = GMT_grd_col_to_x (col_in, I->header);
	GMT_row_loop  (O, row_out) y_out[row_out] = GMT_grd_row_to_y (row_out, O->header);
	GMT_col_loop2 (O, col_out) x_out[col_out] = GMT_grd_col_to_x (col_out, O->header);

	if (GMT_IS_RECT_GRATICULE (C)) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj  = GMT_memory (C, NULL, I->header->nx, double);
		y_in_proj  = GMT_memory (C, NULL, I->header->ny, double);
		x_out_proj = GMT_memory (C, NULL, O->header->nx, double);
		y_out_proj = GMT_memory (C, NULL, O->header->ny, double);
		if (inverse) {
			GMT_row_loop  (I, row_in)  GMT_xy_to_geo (C, &x_proj, &y_in_proj[row_in], I->header->wesn[XLO], y_in[row_in]);
			GMT_col_loop2 (I, col_in)  GMT_xy_to_geo (C, &x_in_proj[col_in], &y_proj, x_in[col_in], I->header->wesn[YLO]);
			GMT_row_loop  (O, row_out) GMT_geo_to_xy (C, I->header->wesn[YLO], y_out[row_out], &x_proj, &y_out_proj[row_out]);
			GMT_col_loop2 (O, col_out) GMT_geo_to_xy (C, x_out[col_out], I->header->wesn[YLO], &x_out_proj[col_out], &y_proj);
		}
		else {
			GMT_row_loop  (I, row_in) GMT_geo_to_xy (C, I->header->wesn[XLO], y_in[row_in], &x_proj, &y_in_proj[row_in]);
			GMT_col_loop2 (I, col_in) GMT_geo_to_xy (C, x_in[col_in], I->header->wesn[YLO], &x_in_proj[col_in], &y_proj);
			GMT_row_loop (O, row_out) GMT_xy_to_geo (C, &x_proj, &y_out_proj[row_out], I->header->wesn[YLO], y_out[row_out]);
			GMT_col_loop2 (O, col_out) {	/* Here we must also align longitudes properly */
				GMT_xy_to_geo (C, &x_out_proj[col_out], &y_proj, x_out[col_out], I->header->wesn[YLO]);
				if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && !GMT_is_dnan (x_out_proj[col_out])) {
					while (x_out_proj[col_out] < I->header->wesn[XLO] - GMT_SMALL) x_out_proj[col_out] += 360.0;
					while (x_out_proj[col_out] > I->header->wesn[XHI] + GMT_SMALL) x_out_proj[col_out] -= 360.0;
				}
			}
		}
	}

	GMT_grd_loop (O, row_out, col_out, ij_out) O->data[ij_out] = C->session.f_NaN;	/* So that nodes outside will retain a NaN value */

	/* PART 1: Project input grid points and do a blockmean operation */

	if (C->common.n.antialias) {	/* Blockaverage repeat pixels, at least the first ~32767 of them... */
		nz = GMT_memory (C, NULL, O->header->size, short int);
		GMT_row_loop (I, row_in) {	/* Loop over the input grid row coordinates */
			if (GMT_IS_RECT_GRATICULE (C)) y_proj = y_in_proj[row_in];
			GMT_col_loop (I, row_in, col_in, ij_in) {	/* Loop over the input grid col coordinates */
				if (GMT_IS_RECT_GRATICULE (C))
					x_proj = x_in_proj[col_in];
				else if (inverse)
					GMT_xy_to_geo (C, &x_proj, &y_proj, x_in[col_in], y_in[row_in]);
				else {
					if (C->current.map.outside (C, x_in[col_in], y_in[row_in])) continue;	/* Quite possible we are beyond the horizon */
					GMT_geo_to_xy (C, x_in[col_in], y_in[row_in], &x_proj, &y_proj);
				}

				/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */

				row_out = GMT_grd_y_to_row (y_proj, O->header);
				if (row_out < 0 || row_out >= O->header->ny) continue;	/* Outside our grid region */
				col_out = GMT_grd_x_to_col (x_proj, O->header);
				if (col_out < 0 || col_out >= O->header->nx) continue;	/* Outside our grid region */

				/* OK, this projected point falls inside the projected grid's rectangular domain */

				ij_out = GMT_IJP (O->header, row_out, col_out);	/* The output node */
				if (nz[ij_out] == 0) O->data[ij_out] = 0.0;	/* First time, override the initial value */
				if (nz[ij_out] < SHRT_MAX) {			/* Avoid overflow */
					O->data[ij_out] += I->data[ij_in];	/* Add up the z-sum inside this rect... */
					nz[ij_out]++;				/* ..and how many points there were */
				}
			}
		}
	}

	/* PART 2: Create weighted average of interpolated and observed points */

	GMT_row_loop (O, row_out) {	/* Loop over the output grid row coordinates */
		if (GMT_IS_RECT_GRATICULE (C)) y_proj = y_out_proj[row_out];
		GMT_col_loop (O, row_out, col_out, ij_out) {	/* Loop over the output grid col coordinates */
			if (GMT_IS_RECT_GRATICULE (C))
				x_proj = x_out_proj[col_out];
			else if (inverse)
				GMT_geo_to_xy (C, x_out[col_out], y_out[row_out], &x_proj, &y_proj);
			else {
				GMT_xy_to_geo (C, &x_proj, &y_proj, x_out[col_out], y_out[row_out]);
				if (C->current.proj.projection == GMT_GENPER && C->current.proj.g_outside) continue;	/* We are beyond the horizon */

				/* On 17-Sep-2007 the slack of GMT_SMALL was added to allow for round-off
				   errors in the grid limits. */
				if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && !GMT_is_dnan (x_proj)) {
					while (x_proj < I->header->wesn[XLO] - GMT_SMALL) x_proj += 360.0;
					while (x_proj > I->header->wesn[XHI] + GMT_SMALL) x_proj -= 360.0;
				}
			}

			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now find nearest node on the input grid */

			z_int = GMT_get_bcr_z (C, I, x_proj, y_proj, &bcr);

			if (!C->common.n.antialias || nz[ij_out] < 2)	/* Just use the interpolated value */
				O->data[ij_out] = (float)z_int;
			else if (GMT_is_dnan (z_int))		/* Take the average of what we accumulated */
				O->data[ij_out] /= nz[ij_out];		/* Plain average */
			else {						/* Weighted average between blockmean'ed and interpolated values */
				inv_nz = 1.0 / nz[ij_out];
				O->data[ij_out] = (float) ((O->data[ij_out] + z_int * inv_nz) / (nz[ij_out] + inv_nz));
			}
		}
	}

	/* Time to clean up our mess */

	GMT_free (C, x_in);
	GMT_free (C, y_in);
	GMT_free (C, x_out);
	GMT_free (C, y_out);
	if (GMT_IS_RECT_GRATICULE(C)) {
		GMT_free (C, x_in_proj);
		GMT_free (C, y_in_proj);
		GMT_free (C, x_out_proj);
		GMT_free (C, y_out_proj);
	}
	if (C->common.n.antialias) GMT_free (C, nz);

	return (GMT_NOERROR);
}

GMT_LONG GMT_img_project (struct GMT_CTRL *C, struct GMT_IMAGE *I, struct GMT_IMAGE *O, struct GMT_EDGEINFO *edgeinfo, GMT_LONG inverse)
{
	/* Generalized image projection that deals with both interpolation and averaging effects.
	 * It requires the input image to have 2 boundary rows/cols so that the bcr
	 * functions can be used.  The I struct represents the input image which is either in original
	 * (i.e., lon/lat) coordinates or projected x/y (if inverse = TRUE).
	 *
	 * I:	Image and header with input image on a padded image with 2 extra rows/columns
	 * O:	Image and header for output image, no padding needed (but allowed)
	 * edgeinfo:	Structure with information about boundary conditions on input image
	 * inverse:	TRUE if input is x/y and we want to invert for a lon/lat image
	 *
	 * In addition, these settings (via -n) control interpolation:
	 * antialias:	TRUE if we need to do the antialiasing STEP 1 (below)
	 * interpolant:	0 = nearest neighbor, 1 = bilinear, 2 = B-spline, 3 = bicubic
	 * threshold:	minumum weight to be used. If weight < threshold interpolation yields NaN.
	 *
	 * We initialize the O->data array to the NaN color.
	 *
	 * Changed 10-Sep-07 to include the argument "antialias" and "threshold" and
	 * made "interpolant" an integer (was GMT_LONG bilinear).
	 */

	GMT_LONG col_in, row_in, ij_in, col_out, row_out, ij_out, b, nb = I->n_bands;
	short int *nz = NULL;
	double x_proj, y_proj, inv_nz, rgb[4];
	double *x_in = NULL, *x_out = NULL, *x_in_proj = NULL, *x_out_proj = NULL;
	double *y_in = NULL, *y_out = NULL, *y_in_proj = NULL, *y_out_proj = NULL;
	unsigned char z_int[4];
	struct GMT_BCR bcr;

	/* Only input image MUST have at least 2 rows/cols padding */
	if (I->header->pad[XLO] < 2 || I->header->pad[XHI] < 2 || I->header->pad[YLO] < 2 || I->header->pad[YHI] < 2) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_img_project: Input image does not have sufficient (2) padding\n");
		GMT_exit (EXIT_FAILURE);
	}

	GMT_boundcond_param_prep (C, I->header, edgeinfo);	/* Init the BC parameters */

	/* Initialize bcr structure:
	   Threshold changed 10 Sep 07 by RS from 1.0 to <threshold> to allow interpolation closer to
	   a NaN value. In case of bilinear interpolation, using 1.0 creates a rectangular hole
	   the size of 4 grid cells for a single NaN grid node. Using 0.25 will create a diamond
	   shaped hole the size of one cell. */
	GMT_bcr_init (C, I->header, C->common.n.interpolant, C->common.n.threshold, &bcr);

	/* Set boundary conditions  */

	GMT_boundcond_image_set (C, I, edgeinfo);

	x_in  = GMT_memory (C, NULL, I->header->nx, double);
	y_in  = GMT_memory (C, NULL, I->header->ny, double);
	x_out = GMT_memory (C, NULL, O->header->nx, double);
	y_out = GMT_memory (C, NULL, O->header->ny, double);

	/* Precalculate grid coordinates */

	GMT_row_loop  (I, row_in) y_in[row_in] = GMT_grd_row_to_y (row_in, I->header);
	GMT_col_loop2 (I, col_in) x_in[col_in] = GMT_grd_col_to_x (col_in, I->header);
	GMT_row_loop  (O, row_out) y_out[row_out] = GMT_grd_row_to_y (row_out, O->header);
	GMT_col_loop2 (O, col_out) x_out[col_out] = GMT_grd_col_to_x (col_out, O->header);

	if (GMT_IS_RECT_GRATICULE (C)) {	/* Since lon/lat parallels x/y it pays to precalculate projected grid coordinates up front */
		x_in_proj  = GMT_memory (C, NULL, I->header->nx, double);
		y_in_proj  = GMT_memory (C, NULL, I->header->ny, double);
		x_out_proj = GMT_memory (C, NULL, O->header->nx, double);
		y_out_proj = GMT_memory (C, NULL, O->header->ny, double);
		if (inverse) {
			GMT_row_loop  (I, row_in)  GMT_xy_to_geo (C, &x_proj, &y_in_proj[row_in], I->header->wesn[XLO], y_in[row_in]);
			GMT_col_loop2 (I, col_in)  GMT_xy_to_geo (C, &x_in_proj[col_in], &y_proj, x_in[col_in], I->header->wesn[YLO]);
			GMT_row_loop  (O, row_out) GMT_geo_to_xy (C, I->header->wesn[YLO], y_out[row_out], &x_proj, &y_out_proj[row_out]);
			GMT_col_loop2 (O, col_out) GMT_geo_to_xy (C, x_out[col_out], I->header->wesn[YLO], &x_out_proj[col_out], &y_proj);
		}
		else {
			GMT_row_loop  (I, row_in) GMT_geo_to_xy (C, I->header->wesn[XLO], y_in[row_in], &x_proj, &y_in_proj[row_in]);
			GMT_col_loop2 (I, col_in) GMT_geo_to_xy (C, x_in[col_in], I->header->wesn[YLO], &x_in_proj[col_in], &y_proj);
			GMT_row_loop (O, row_out) GMT_xy_to_geo (C, &x_proj, &y_out_proj[row_out], I->header->wesn[YLO], y_out[row_out]);
			GMT_col_loop2 (O, col_out) {	/* Here we must also align longitudes properly */
				GMT_xy_to_geo (C, &x_out_proj[col_out], &y_proj, x_out[col_out], I->header->wesn[YLO]);
				if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && !GMT_is_dnan (x_out_proj[col_out])) {
					while (x_out_proj[col_out] < I->header->wesn[XLO] - GMT_SMALL) x_out_proj[col_out] += 360.0;
					while (x_out_proj[col_out] > I->header->wesn[XHI] + GMT_SMALL) x_out_proj[col_out] -= 360.0;
				}
			}
		}
	}

	GMT_grd_loop (O, row_out, col_out, ij_out) 		/* So that nodes outside will have the NaN color */
		for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = GMT_u255 (C->current.setting.color_patch[GMT_NAN][b]);

	/* PART 1: Project input image points and do a blockmean operation */

	if (C->common.n.antialias) {	/* Blockaverage repeat pixels, at least the first ~32767 of them... */
		nz = GMT_memory (C, NULL, O->header->size, short int);
		GMT_row_loop (I, row_in) {	/* Loop over the input grid row coordinates */
			if (GMT_IS_RECT_GRATICULE (C)) y_proj = y_in_proj[row_in];
			GMT_col_loop (I, row_in, col_in, ij_in) {	/* Loop over the input grid col coordinates */
				if (GMT_IS_RECT_GRATICULE (C))
					x_proj = x_in_proj[col_in];
				else if (inverse)
					GMT_xy_to_geo (C, &x_proj, &y_proj, x_in[col_in], y_in[row_in]);
				else {
					if (C->current.map.outside (C, x_in[col_in], y_in[row_in])) continue;	/* Quite possible we are beyond the horizon */
					GMT_geo_to_xy (C, x_in[col_in], y_in[row_in], &x_proj, &y_proj);
				}

				/* Here, (x_proj, y_proj) is the projected grid point.  Now find nearest node on the output grid */

				row_out = GMT_grd_y_to_row (y_proj, O->header);
				if (row_out < 0 || row_out >= O->header->ny) continue;	/* Outside our grid region */
				col_out = GMT_grd_x_to_col (x_proj, O->header);
				if (col_out < 0 || col_out >= O->header->nx) continue;	/* Outside our grid region */

				/* OK, this projected point falls inside the projected grid's rectangular domain */

				ij_out = GMT_IJP (O->header, row_out, col_out);	/* The output node */
				if (nz[ij_out] == 0) for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = 0;	/* First time, override the initial value */
				if (nz[ij_out] < SHRT_MAX) {	/* Avoid overflow */
					for (b = 0; b < nb; b++) {
						rgb[b] = ((double)nz[ij_out] * O->data[nb*ij_out+b] + I->data[nb*ij_in+b])/(nz[ij_out] + 1.0);	/* Update the mean pix values inside this rect... */
						O->data[nb*ij_out+b] = (unsigned char) irint (GMT_0_255_truncate (rgb[b]));
					}
					nz[ij_out]++;		/* ..and how many points there were */
				}
			}
		}
	}

	/* PART 2: Create weighted average of interpolated and observed points */

	GMT_row_loop (O, row_out) {	/* Loop over the output grid row coordinates */
		if (GMT_IS_RECT_GRATICULE (C)) y_proj = y_out_proj[row_out];
		GMT_col_loop (O, row_out, col_out, ij_out) {	/* Loop over the output grid col coordinates */
			if (GMT_IS_RECT_GRATICULE (C))
				x_proj = x_out_proj[col_out];
			else if (inverse)
				GMT_geo_to_xy (C, x_out[col_out], y_out[row_out], &x_proj, &y_proj);
			else {
				GMT_xy_to_geo (C, &x_proj, &y_proj, x_out[col_out], y_out[row_out]);
				if (C->current.proj.projection == GMT_GENPER && C->current.proj.g_outside) continue;	/* We are beyond the horizon */

				/* On 17-Sep-2007 the slack of GMT_SMALL was added to allow for round-off
				   errors in the grid limits. */
				if (C->current.io.col_type[GMT_IN][GMT_X] == GMT_IS_LON && !GMT_is_dnan (x_proj)) {
					while (x_proj < I->header->wesn[XLO] - GMT_SMALL) x_proj += 360.0;
					while (x_proj > I->header->wesn[XHI] + GMT_SMALL) x_proj -= 360.0;
				}
			}

			/* Here, (x_proj, y_proj) is the inversely projected grid point.  Now find nearest node on the input grid */

			GMT_get_bcr_img (C, I, x_proj, y_proj, &bcr, z_int);

			if (!C->common.n.antialias || nz[ij_out] < 2)	/* Just use the interpolated value */
				for (b = 0; b < nb; b++) O->data[nb*ij_out+b] = z_int[b];
			else {						/* Weighted average between blockmean'ed and interpolated values */
				inv_nz = 1.0 / nz[ij_out];
				for (b = 0; b < nb; b++) {
					rgb[b] = ((double)nz[ij_out] * O->data[nb*ij_out+b] + z_int[b] * inv_nz) / (nz[ij_out] + inv_nz);
					O->data[nb*ij_out+b] = (unsigned char) irint (GMT_0_255_truncate (rgb[b]));
				}
			}
		}
	}

	/* Time to clean up our mess */

	GMT_free (C, x_in);
	GMT_free (C, y_in);
	GMT_free (C, x_out);
	GMT_free (C, y_out);
	if (GMT_IS_RECT_GRATICULE(C)) {
		GMT_free (C, x_in_proj);
		GMT_free (C, y_in_proj);
		GMT_free (C, x_out_proj);
		GMT_free (C, y_out_proj);
	}
	if (C->common.n.antialias) GMT_free (C, nz);

	return (GMT_NOERROR);
}

void GMT_azim_to_angle (struct GMT_CTRL *C, double lon, double lat, double c, double azim, double *angle)
	/* All variables in degrees */
{
	double lon1, lat1, x0, x1, y0, y1, dx, width, sinaz, cosaz;

	if (GMT_IS_LINEAR (C)) {	/* Trivial case */
		*angle = 90.0 - azim;
		if (C->current.proj.scale[GMT_X] != C->current.proj.scale[GMT_Y]) {	/* But allow for different x,y scaling */
			sincosd (*angle, &sinaz, &cosaz);
			*angle = d_atan2d (sinaz * C->current.proj.scale[GMT_Y], cosaz * C->current.proj.scale[GMT_X]);
		}
		return;
	}

	/* Find second point c spherical degrees away in the azim direction */

	GMT_get_point_from_r_az (C, lon, lat, c, azim, &lon1, &lat1);

	/* Convert both points to x,y and get angle */

	GMT_geo_to_xy (C, lon, lat, &x0, &y0);
	GMT_geo_to_xy (C, lon1, lat1, &x1, &y1);

	/* Check for wrap-around */

	dx = x1 - x0;
	if (GMT_360_RANGE (C->common.R.wesn[XLO], C->common.R.wesn[XHI]) && fabs (dx) > (width = GMT_half_map_width (C, y0))) {
		width *= 2.0;
		dx = copysign (width - fabs (dx), -dx);
		if (x1 < width)
			x0 -= width;
		else
			x0 += width;
	}
	*angle = d_atan2d (y1 - y0, x1 - x0);
}

GMT_LONG GMT_map_clip_path (struct GMT_CTRL *C, double **x, double **y, GMT_LONG *donut)
{
	/* This function returns a clip path corresponding to the
	 * extent of the map.
	 */

	GMT_LONG i, j, np;
	double *work_x = NULL, *work_y = NULL, da, r0, s, c, lon, lat;

	*donut = FALSE;

	if (C->common.R.oblique)	/* Rectangular map boundary */
		np = 4;
	else {
		switch (C->current.proj.projection) {
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
				if (C->current.proj.got_elevations)
					*donut = (C->common.R.wesn[YHI] < 90.0 && C->current.map.is_world);
				else
					*donut = (C->common.R.wesn[YLO] > 0.0 && C->current.map.is_world);
				np = C->current.map.n_lon_nodes + 1;
				if (C->common.R.wesn[YLO] > 0.0)	/* Need inside circle segment */
					np *= 2;
				else if (!C->current.map.is_world)	/* Need to include origin */
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
				np = (C->current.proj.polar && (C->common.R.wesn[YLO] <= -90.0 || C->common.R.wesn[YHI] >= 90.0)) ? C->current.map.n_lon_nodes + 2: 2 * (C->current.map.n_lon_nodes + 1);
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				np = 2 * C->current.map.n_lat_nodes + 2;
				break;
			case GMT_WINKEL:
			case GMT_HAMMER:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				np = 2 * C->current.map.n_lat_nodes + 2;
				if (C->common.R.wesn[YLO] != -90.0) np += C->current.map.n_lon_nodes - 1;
				if (C->common.R.wesn[YHI] != 90.0) np += C->current.map.n_lon_nodes - 1;
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				np = 2 * (C->current.map.n_lon_nodes + C->current.map.n_lat_nodes);
				break;
			default:
				GMT_report (C, GMT_MSG_FATAL, "Bad case in GMT_map_clip_path (%ld)\n", C->current.proj.projection);
				np = 0;
				break;
		}
	}

	work_x = GMT_memory (C, NULL, np, double);
	work_y = GMT_memory (C, NULL, np, double);

	if (C->common.R.oblique) {
		work_x[0] = work_x[3] = C->current.proj.rect[XLO];	work_y[0] = work_y[1] = C->current.proj.rect[YLO];
		work_x[1] = work_x[2] = C->current.proj.rect[XHI];	work_y[2] = work_y[3] = C->current.proj.rect[YHI];
	}
	else {
		switch (C->current.proj.projection) {	/* Fill in clip path */
			case GMT_LINEAR:
			case GMT_MERCATOR:
			case GMT_CYL_EQ:
			case GMT_CYL_EQDIST:
			case GMT_CYL_STEREO:
			case GMT_MILLER:
			case GMT_OBLIQUE_MERC:
				work_x[0] = work_x[3] = C->current.proj.rect[XLO];	work_y[0] = work_y[1] = C->current.proj.rect[YLO];
				work_x[1] = work_x[2] = C->current.proj.rect[XHI];	work_y[2] = work_y[3] = C->current.proj.rect[YHI];
				break;
			case GMT_LAMBERT:
			case GMT_ALBERS:
			case GMT_ECONIC:
				for (i = j = 0; i <= C->current.map.n_lon_nodes; i++, j++) {
					lon = (i == C->current.map.n_lon_nodes) ? C->common.R.wesn[XHI] : C->common.R.wesn[XLO] + i * C->current.map.dlon;
					GMT_geo_to_xy (C, lon, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				}
				for (i = 0; i <= C->current.map.n_lon_nodes; i++, j++) {
					lon = (i == C->current.map.n_lon_nodes) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI] - i * C->current.map.dlon;
					GMT_geo_to_xy (C, lon, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				}
				break;
			case GMT_TM:
			case GMT_UTM:
			case GMT_CASSINI:
			case GMT_POLYCONIC:
				for (i = j = 0; i < C->current.map.n_lon_nodes; i++, j++)	/* South */
					GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				for (i = 0; i < C->current.map.n_lat_nodes; i++, j++)	/* East */
					GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO] + i * C->current.map.dlat, &work_x[j], &work_y[j]);
				for (i = 0; i < C->current.map.n_lon_nodes; i++, j++)	/* North */
					GMT_geo_to_xy (C, C->common.R.wesn[XHI] - i * C->current.map.dlon, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				for (i = 0; i < C->current.map.n_lat_nodes; i++, j++)	/* West */
					GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI] - i * C->current.map.dlat, &work_x[j], &work_y[j]);
				break;
			case GMT_POLAR:
				r0 = C->current.proj.r * C->common.R.wesn[YLO] / C->common.R.wesn[YHI];
				if (*donut) {
					np /= 2;
					da = TWO_PI / np;
					for (i = 0, j = 2 * np - 1; i < np; i++, j--) {	/* Draw outer clippath */
						sincos (i * da, &s, &c);
						work_x[i] = C->current.proj.r * (1.0 + c);
						work_y[i] = C->current.proj.r * (1.0 + s);
						/* Do inner clippath and put it at end of array */
						work_x[j] = C->current.proj.r + r0 * c;
						work_y[j] = C->current.proj.r + r0 * s;
					}
				}
				else {
					da = fabs (C->common.R.wesn[XHI] - C->common.R.wesn[XLO]) / C->current.map.n_lon_nodes;
					if (C->current.proj.got_elevations) {
						for (i = j = 0; i <= C->current.map.n_lon_nodes; i++, j++)	/* Draw outer clippath */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * da, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						for (i = C->current.map.n_lon_nodes; C->common.R.wesn[YHI] < 90.0 && i >= 0; i--, j++)	/* Draw inner clippath */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * da, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						if (GMT_IS_ZERO (90.0 - C->common.R.wesn[YHI]) && !C->current.map.is_world)	/* Add origin */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
					}
					else {
						for (i = j = 0; i <= C->current.map.n_lon_nodes; i++, j++)	/* Draw outer clippath */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * da, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						for (i = C->current.map.n_lon_nodes; C->common.R.wesn[YLO] > 0.0 && i >= 0; i--, j++)	/* Draw inner clippath */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * da, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						if (GMT_IS_ZERO (C->common.R.wesn[YLO]) && !C->current.map.is_world)	/* Add origin */
							GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
					}
				}
				break;
			case GMT_GENPER:
				GMT_genper_map_clip_path (C, np, work_x, work_y);
				break;
			case GMT_LAMB_AZ_EQ:
			case GMT_ORTHO:
			case GMT_GNOMONIC:
			case GMT_VANGRINTEN:
			case GMT_STEREO:
				if (C->current.proj.polar) {
					j = 0;
					if (C->common.R.wesn[YLO] > -90.0) {
						for (i = 0; i <= C->current.map.n_lon_nodes; i++, j++) {
							lon = (i == C->current.map.n_lon_nodes) ? C->common.R.wesn[XHI] : C->common.R.wesn[XLO] + i * C->current.map.dlon;
							GMT_geo_to_xy (C, lon, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add S pole */
						GMT_geo_to_xy (C, C->common.R.wesn[XLO], -90.0, &work_x[j], &work_y[j]);
						j++;
					}
					if (C->common.R.wesn[YHI] < 90.0) {
						for (i = 0; i <= C->current.map.n_lon_nodes; i++, j++) {
							lon = (i == C->current.map.n_lon_nodes) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI] - i * C->current.map.dlon;
							GMT_geo_to_xy (C, lon, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
						}
					}
					else { /* Just add N pole */
						GMT_geo_to_xy (C, C->common.R.wesn[XLO], 90.0, &work_x[j], &work_y[j]);
						j++;
					}
				}
				else {
					da = TWO_PI / np;
					for (i = 0; i < np; i++) {
						sincos (i * da, &s, &c);
						work_x[i] = C->current.proj.r * (1.0 + c);
						work_y[i] = C->current.proj.r * (1.0 + s);
					}
				}
				break;
			case GMT_AZ_EQDIST:
				da = TWO_PI / np;
				for (i = 0; i < np; i++) {
					sincos (i * da, &s, &c);
					work_x[i] = C->current.proj.r * (1.0 + c);
					work_y[i] = C->current.proj.r * (1.0 + s);
				}
				break;
			case GMT_MOLLWEIDE:
			case GMT_SINUSOIDAL:
			case GMT_ROBINSON:
				for (i = j = 0; i <= C->current.map.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == C->current.map.n_lat_nodes) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO] + i * C->current.map.dlat;
					GMT_geo_to_xy (C, C->common.R.wesn[XHI], lat, &work_x[j], &work_y[j]);
				}
				for (i = C->current.map.n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == C->current.map.n_lat_nodes) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO] + i * C->current.map.dlat;
					GMT_geo_to_xy (C, C->common.R.wesn[XLO], lat, &work_x[j], &work_y[j]);
				}
				break;
			case GMT_HAMMER:
			case GMT_WINKEL:
			case GMT_ECKERT4:
			case GMT_ECKERT6:
				for (i = j = 0; i <= C->current.map.n_lat_nodes; i++, j++) {	/* Right */
					lat = (i == C->current.map.n_lat_nodes) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO] + i * C->current.map.dlat;
					GMT_geo_to_xy (C, C->common.R.wesn[XHI], lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; C->common.R.wesn[YHI] != 90.0 && i < C->current.map.n_lon_nodes; i++, j++)
					GMT_geo_to_xy (C, C->common.R.wesn[XHI] - i * C->current.map.dlon, C->common.R.wesn[YHI], &work_x[j], &work_y[j]);
				for (i = C->current.map.n_lat_nodes; i >= 0; j++, i--)	{	/* Left */
					lat = (i == C->current.map.n_lat_nodes) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO] + i * C->current.map.dlat;
					GMT_geo_to_xy (C, C->common.R.wesn[XLO], lat, &work_x[j], &work_y[j]);
				}
				for (i = 1; C->common.R.wesn[YLO] != -90.0 && i < C->current.map.n_lon_nodes; i++, j++)
					GMT_geo_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YLO], &work_x[j], &work_y[j]);
				break;
		}
	}

	if (!(*donut)) np = GMT_compact_line (C, work_x, work_y, np, FALSE, (int *)0);

	*x = work_x;
	*y = work_y;

	return (np);
}

double GMT_lat_swap_quick (struct GMT_CTRL *C, double lat, double c[])
{
	/* Return latitude, in degrees, given latitude, in degrees, based on coefficients c */

	double delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (GMT_IS_ZERO (lat)) return (0.0);

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (c[0] + cos2phi * (c[1] + cos2phi * (c[2] + cos2phi * c[3])));

	return (lat + R2D * delta);
}

double GMT_lat_swap (struct GMT_CTRL *C, double lat, GMT_LONG itype)
{
	/* Return latitude, in degrees, given latitude, in degrees, based on itype */

	double delta, cos2phi, sin2phi;

	/* First deal with trivial cases */

	if (lat >=  90.0) return ( 90.0);
	if (lat <= -90.0) return (-90.0);
	if (GMT_IS_ZERO (lat)) return (0.0);

	if (C->current.proj.GMT_lat_swap_vals.spherical) return (lat);

	if (itype < 0 || itype >= GMT_LATSWAP_N) {
		/* This should never happen -?- or do we want to allow the
			possibility of using itype = -1 to do nothing  */
		GMT_report (C, GMT_MSG_FATAL, "GMT_lat_swap(): Invalid choice, programming bug.\n");
		return(lat);
	}

	sincosd (2.0 * lat, &sin2phi, &cos2phi);

	delta = sin2phi * (C->current.proj.GMT_lat_swap_vals.c[itype][0]
		+ cos2phi * (C->current.proj.GMT_lat_swap_vals.c[itype][1]
		+ cos2phi * (C->current.proj.GMT_lat_swap_vals.c[itype][2]
		+ cos2phi * C->current.proj.GMT_lat_swap_vals.c[itype][3])));

	return (lat + R2D * delta);
}

void GMT_scale_eqrad (struct GMT_CTRL *C)
{
	/* Reinitialize C->current.proj.EQ_RAD to the appropriate value */

	switch (C->current.proj.projection) {

		/* Conformal projections */

		case GMT_MERCATOR:
		case GMT_TM:
		case GMT_UTM:
		case GMT_OBLIQUE_MERC:
		case GMT_LAMBERT:
		case GMT_STEREO:

			C->current.proj.EQ_RAD = C->current.proj.GMT_lat_swap_vals.rm;
			break;

		/* Equal Area projections */

		case GMT_LAMB_AZ_EQ:
		case GMT_ALBERS:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:

			C->current.proj.EQ_RAD = C->current.proj.GMT_lat_swap_vals.ra;
			break;

		default:	/* Keep EQ_RAD as is */
			break;
	}

	/* Also reset dependencies of EQ_RAD */

	C->current.proj.i_EQ_RAD = 1.0 / C->current.proj.EQ_RAD;
	C->current.proj.M_PR_DEG = TWO_PI * C->current.proj.EQ_RAD / 360.0;
	C->current.proj.KM_PR_DEG = C->current.proj.M_PR_DEG / METERS_IN_A_KM;
}


void GMT_init_ellipsoid (struct GMT_CTRL *C)
{
	double f;

	/* Set up ellipsoid parameters for the selected ellipsoid since gmt.conf could have changed them */

	f = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening;
	C->current.proj.ECC2 = 2.0 * f - f * f;
	C->current.proj.ECC4 = C->current.proj.ECC2 * C->current.proj.ECC2;
	C->current.proj.ECC6 = C->current.proj.ECC2 * C->current.proj.ECC4;
	C->current.proj.one_m_ECC2 = 1.0 - C->current.proj.ECC2;
	C->current.proj.i_one_m_ECC2 = 1.0 / C->current.proj.one_m_ECC2;
	C->current.proj.ECC = d_sqrt (C->current.proj.ECC2);
	C->current.proj.half_ECC = 0.5 * C->current.proj.ECC;
	C->current.proj.i_half_ECC = 0.5 / C->current.proj.ECC;
	C->current.proj.EQ_RAD = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius;
	C->current.proj.i_EQ_RAD = 1.0 / C->current.proj.EQ_RAD;

	/* Spherical degrees to m or km */
	C->current.proj.M_PR_DEG = TWO_PI * GMT_mean_radius (C->current.proj.EQ_RAD, f) / 360.0;
	C->current.proj.KM_PR_DEG = C->current.proj.M_PR_DEG / METERS_IN_A_KM;
	C->current.proj.DIST_M_PR_DEG = C->current.proj.M_PR_DEG;
	C->current.proj.DIST_KM_PR_DEG = C->current.proj.KM_PR_DEG;
}

/* Datum conversion routines */

void GMT_datum_init (struct GMT_CTRL *C, struct GMT_DATUM *from, struct GMT_DATUM *to, GMT_LONG heights)
{
	/* Initialize datum conv structures based on the parsed values*/

	GMT_LONG k;

	C->current.proj.datum.h_given = heights;

	GMT_memcpy (&C->current.proj.datum.from, from, 1, struct GMT_DATUM);
	GMT_memcpy (&C->current.proj.datum.to,   to,   1, struct GMT_DATUM);

	C->current.proj.datum.da = C->current.proj.datum.to.a - C->current.proj.datum.from.a;
	C->current.proj.datum.df = C->current.proj.datum.to.f - C->current.proj.datum.from.f;
	for (k = 0; k < 3; k++) C->current.proj.datum.dxyz[k] = -(C->current.proj.datum.to.xyz[k] - C->current.proj.datum.from.xyz[k]);	/* Since the X, Y, Z are Deltas relative to WGS-84 */
	C->current.proj.datum.one_minus_f = 1.0 - C->current.proj.datum.from.f;
}

void GMT_ECEF_init (struct GMT_CTRL *C, struct GMT_DATUM *D)
{
	/* Duplicate the parsed datum to the GMT from datum */

	GMT_memcpy (&C->current.proj.datum.from, D, 1, struct GMT_DATUM);
}

GMT_LONG GMT_set_datum (struct GMT_CTRL *C, char *text, struct GMT_DATUM *D)
{
	GMT_LONG i;
	double t;

	if (text[0] == '\0' || text[0] == '-') {	/* Shortcut for WGS-84 */
		GMT_memset (D->xyz, 3, double);
		D->a = C->current.setting.ref_ellipsoid[0].eq_radius;
		D->f = C->current.setting.ref_ellipsoid[0].flattening;
		D->ellipsoid_id = 0;
	}
	else if (strchr (text, ':')) {	/* Has colons, must get ellipsoid and dr separately */
		char ellipsoid[GMT_TEXT_LEN256], dr[GMT_TEXT_LEN256];
		if (sscanf (text, "%[^:]:%s", ellipsoid, dr) != 2) {
			GMT_report (C, GMT_MSG_FATAL, "Malformed <ellipsoid>:<dr> argument!\n");
			return (-1);
		}
		if (sscanf (dr, "%lf,%lf,%lf", &D->xyz[GMT_X], &D->xyz[GMT_Y], &D->xyz[GMT_Z]) != 3) {
			GMT_report (C, GMT_MSG_FATAL, "Malformed <x>,<y>,<z> argument!\n");
			return (-1);
		}
		if ((i = GMT_get_ellipsoid (C, ellipsoid)) >= 0) {	/* This includes looking for format <a>,<1/f> */
			D->a = C->current.setting.ref_ellipsoid[i].eq_radius;
			D->f = C->current.setting.ref_ellipsoid[i].flattening;
			D->ellipsoid_id = i;
		}
		else {
			GMT_report (C, GMT_MSG_FATAL, "Ellipsoid %s not recognized!\n", ellipsoid);
			return (-1);
		}
	}
	else {		/* Gave a Datum ID tag [ 0-(GMT_N_DATUMS-1)] */
		GMT_LONG k;
		if (sscanf (text, "%" GMT_LL "d", &i) != 1) {
			GMT_report (C, GMT_MSG_FATAL, "Malformed or unrecognized <datum> argument (%s)!\n", text);
			return (-1);
		}
		if (i < 0 || i >= GMT_N_DATUMS) {
			GMT_report (C, GMT_MSG_FATAL, "Datum ID (%ld) outside valid range (0-%d)!\n", i, GMT_N_DATUMS-1);
			return (-1);
		}
		if ((k = GMT_get_ellipsoid (C, C->current.setting.proj_datum[i].ellipsoid)) < 0) {	/* This should not happen... */
			GMT_report (C, GMT_MSG_FATAL, "Ellipsoid %s not recognized!\n", C->current.setting.proj_datum[i].ellipsoid);
			return (-1);
		}
		D->a = C->current.setting.ref_ellipsoid[k].eq_radius;
		D->f = C->current.setting.ref_ellipsoid[k].flattening;
		D->ellipsoid_id = k;
		for (k = 0; k< 3; k++) D->xyz[k] = C->current.setting.proj_datum[i].xyz[k];
	}
	D->b = D->a * (1 - D->f);
	D->e_squared = 2 * D->f - D->f * D->f;
	t = D->a /D->b;
	D->ep_squared = t * t - 1.0;	/* (a^2 - b^2)/a^2 */
	return 0;
}

void GMT_conv_datum (struct GMT_CTRL *C, double in[], double out[])
{
	/* Evaluate J^-1 and B on from ellipsoid */
	/* Based on Standard Molodensky Datum Conversion, implemented from
	 * http://www.colorado.edu/geography/gcraft/notes/datum/gif/molodens.gif */

	double sin_lon, cos_lon, sin_lat, cos_lat, sin_lat2, M, N, h, tmp_1, tmp_2, tmp_3;
	double delta_lat, delta_lon, delta_h, sc_lat;

	h = (C->current.proj.datum.h_given) ? in[GMT_Z] : 0.0;
	sincosd (in[GMT_X], &sin_lon, &cos_lon);
	sincosd (in[GMT_Y], &sin_lat, &cos_lat);
	sin_lat2 = sin_lat * sin_lat;
	sc_lat = sin_lat * cos_lat;
	M = C->current.proj.datum.from.a * (1.0 - C->current.proj.datum.from.e_squared) / pow (1.0 - C->current.proj.datum.from.e_squared * sin_lat2, 1.5);
	N = C->current.proj.datum.from.a / sqrt (1.0 - C->current.proj.datum.from.e_squared * sin_lat2);

	tmp_1 = -C->current.proj.datum.dxyz[GMT_X] * sin_lat * cos_lon - C->current.proj.datum.dxyz[GMT_Y] * sin_lat * sin_lon + C->current.proj.datum.dxyz[GMT_Z] * cos_lat;
	tmp_2 = C->current.proj.datum.da * (N * C->current.proj.datum.from.e_squared * sc_lat) / C->current.proj.datum.from.a;
	tmp_3 = C->current.proj.datum.df * (M / C->current.proj.datum.one_minus_f + N * C->current.proj.datum.one_minus_f) * sc_lat;
	delta_lat = (tmp_1 + tmp_2 + tmp_3) / (M + h);

	delta_lon = (-C->current.proj.datum.dxyz[GMT_X] * sin_lon + C->current.proj.datum.dxyz[GMT_Y] * cos_lon) / ((N + h) * cos_lat);

	tmp_1 = C->current.proj.datum.dxyz[GMT_X] * cos_lat * cos_lon + C->current.proj.datum.dxyz[GMT_Y] * cos_lat * sin_lon + C->current.proj.datum.dxyz[GMT_Z] * sin_lat;
	tmp_2 = -C->current.proj.datum.da * C->current.proj.datum.from.a / N;
	tmp_3 = C->current.proj.datum.df * C->current.proj.datum.one_minus_f * N * sin_lat2;
	delta_h = tmp_1 + tmp_2 + tmp_3;

	out[GMT_X] = in[GMT_X] + delta_lon * R2D;
	out[GMT_Y] = in[GMT_Y] + delta_lat * R2D;
	if (C->current.proj.datum.h_given) out[GMT_Z] = in[GMT_Z] + delta_h;
}

void GMT_ECEF_forward (struct GMT_CTRL *C, double in[], double out[])
{
	/* Convert geodetic lon, lat, height to ECEF coordinates given the datum parameters.
	 * C->current.proj.datum.from is always the ellipsoid to use */

	double sin_lon, cos_lon, sin_lat, cos_lat, N, tmp;

	sincosd (in[GMT_X], &sin_lon, &cos_lon);
	sincosd (in[GMT_Y], &sin_lat, &cos_lat);

	N = C->current.proj.datum.from.a / d_sqrt (1.0 - C->current.proj.datum.from.e_squared * sin_lat * sin_lat);
	tmp = (N + in[GMT_Z]) * cos_lat;
	out[GMT_X] = tmp * cos_lon + C->current.proj.datum.from.xyz[GMT_X];
	out[GMT_Y] = tmp * sin_lon + C->current.proj.datum.from.xyz[GMT_Y];
	out[GMT_Z] = (N * (1 - C->current.proj.datum.from.e_squared) + in[GMT_Z]) * sin_lat + C->current.proj.datum.from.xyz[GMT_Z];
}

void GMT_ECEF_inverse (struct GMT_CTRL *C, double in[], double out[])
{
	/* Convert ECEF coordinates to geodetic lon, lat, height given the datum parameters.
	 * C->current.proj.datum.from is always the ellipsoid to use */

	GMT_LONG i;
	double in_p[3], sin_lat, cos_lat, N, p, theta, sin_theta, cos_theta;

	/* First remove the xyz shifts, us in_p to avoid changing in */

	for (i = 0; i < 3; i++) in_p[i] = in[i] - C->current.proj.datum.from.xyz[i];

	p = hypot (in_p[GMT_X], in_p[GMT_Y]);
	theta = atan (in_p[GMT_Z] * C->current.proj.datum.from.a / (p * C->current.proj.datum.from.b));
	sincos (theta, &sin_theta, &cos_theta);
	out[GMT_X] = d_atan2d (in_p[GMT_Y], in_p[GMT_X]);
	out[GMT_Y] = atand ((in_p[GMT_Z] + C->current.proj.datum.from.ep_squared * C->current.proj.datum.from.b * pow (sin_theta, 3.0)) / (p - C->current.proj.datum.from.e_squared * C->current.proj.datum.from.a * pow (cos_theta, 3.0)));
	sincosd (out[GMT_Y], &sin_lat, &cos_lat);
	N = C->current.proj.datum.from.a / sqrt (1.0 - C->current.proj.datum.from.e_squared * sin_lat * sin_lat);
	out[GMT_Z] = (p / cos_lat) - N;
}

GMT_LONG GMT_dist_array (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, double scale, GMT_LONG dist_flag, double **dist)
{	/* Returns distances in meter; use scale to get other units */
	GMT_LONG this, prev, cumulative = TRUE, do_scale, xy_not_NaN;
	double *d = NULL, cum_dist = 0.0, inc = 0.0;

	if (dist_flag < 0) {	/* Want increments and not cumulative distances */
		dist_flag = GMT_abs (dist_flag);
		cumulative = FALSE;
	}

	if (dist_flag < 0 || dist_flag > 3) return (GMT_MAP_BAD_DIST_FLAG);

	do_scale = (scale != 1.0);
	d = GMT_memory (C, NULL, n, double);

	for (this = 0, prev = -1; this < n; this++) {
		xy_not_NaN = !(GMT_is_dnan (x[this]) || GMT_is_dnan (y[this]));
		if (xy_not_NaN && prev >= 0) {	/* safe to calculate inc */
			switch (dist_flag) {

				case 0:	/* Cartesian distances */

					inc = hypot (x[this] - x[prev], y[this] - y[prev]);
					break;

				case 1:	/* Flat earth distances in meter */

					inc = GMT_flatearth_dist_meter (C, x[this], y[this], x[prev], y[prev]);
					break;

				case 2:	/* Great circle distances in meter */

					inc = GMT_great_circle_dist_meter (C, x[this], y[this], x[prev], y[prev]);
					break;

				case 3:	/* Geodesic distances in meter */

					inc = GMT_geodesic_dist_meter (C, x[this], y[this], x[prev], y[prev]);
					break;
			}

			if (do_scale) inc *= scale;
			if (cumulative) cum_dist += inc;
			d[this] = (cumulative) ? cum_dist : inc;
		}
		else if (this > 0)
			d[this] = C->session.d_NaN;

		if (xy_not_NaN) prev = this;	/* This was a record with OK x,y; make it the previous point for distance calculations */
	}
	*dist = d;
	return (GMT_NOERROR);
}

GMT_LONG GMT_map_latcross (struct GMT_CTRL *C, double lat, double west, double east, struct GMT_XINGS **xings)
{
	GMT_LONG i, go = FALSE, nx, nc = 0, n_alloc = GMT_SMALL_CHUNK;
	double lon, lon_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	struct GMT_XINGS *X = NULL;

	X = GMT_memory (C, NULL, n_alloc, struct GMT_XINGS);

	lon_old = west - 2.0 * GMT_SMALL;
	GMT_map_outside (C, lon_old, lat);
	GMT_geo_to_xy (C, lon_old, lat, &last_x, &last_y);
	for (i = 1; i <= C->current.map.n_lon_nodes; i++) {
		lon = (i == C->current.map.n_lon_nodes) ? east + 2.0 * GMT_SMALL : west + i * C->current.map.dlon;
		GMT_map_outside (C, lon, lat);
		GMT_geo_to_xy (C, lon, lat, &this_x, &this_y);
		if ((nx = GMT_map_crossing (C, lon_old, lat, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (C, lon_old, lat, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (C->current.map.corner > 0) {
				X[nc].sides[0] = (C->current.map.corner%4 > 1) ? 1 : 3;
				if (C->current.proj.got_azimuths) X[nc].sides[0] = (X[nc].sides[0] + 2) % 4;
				C->current.map.corner = 0;
			}
		}
		else if (C->current.map.is_world)
			nx = (*C->current.map.wrap_around_check) (C, X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - C->current.map.width) < GMT_SMALL && !C->current.map.is_world)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_SMALL && (gap - C->current.map.height) < GMT_SMALL && !C->current.map.is_world_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = GMT_memory (C, X, n_alloc, struct GMT_XINGS);
			}
			go = FALSE;
		}
		lon_old = lon;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = GMT_memory (C, X, nc, struct GMT_XINGS);
		*xings = X;
	}
	else
		GMT_free (C, X);

	return (nc);
}

GMT_LONG GMT_map_loncross (struct GMT_CTRL *C, double lon, double south, double north, struct GMT_XINGS **xings)
{
	GMT_LONG go = FALSE, j, nx, nc = 0, n_alloc = GMT_SMALL_CHUNK;
	double lat, lat_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	struct GMT_XINGS *X = NULL;

	X = GMT_memory (C, NULL, n_alloc, struct GMT_XINGS);

	lat_old = ((south - (2.0 * GMT_SMALL)) >= -90.0) ? south - 2.0 * GMT_SMALL : south;	/* Outside */
	if ((north + 2.0 * GMT_SMALL) <= 90.0) north += 2.0 * GMT_SMALL;
	GMT_map_outside (C, lon, lat_old);
	GMT_geo_to_xy (C, lon, lat_old, &last_x, &last_y);
	for (j = 1; j <= C->current.map.n_lat_nodes; j++) {
		lat = (j == C->current.map.n_lat_nodes) ? north: south + j * C->current.map.dlat;
		GMT_map_outside (C, lon, lat);
		GMT_geo_to_xy (C, lon, lat, &this_x, &this_y);
		if ((nx = GMT_map_crossing (C, lon, lat_old, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides))) {
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (C, lon, lat_old, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (C->current.map.corner > 0) {
				X[nc].sides[0] = (C->current.map.corner < 3) ? 0 : 2;
				C->current.map.corner = 0;
			}
		}
		else if (C->current.map.is_world)
			nx = (*C->current.map.wrap_around_check) (C, X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - C->current.map.width) < GMT_SMALL && !C->current.map.is_world)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > GMT_SMALL && (gap - C->current.map.height) < GMT_SMALL && !C->current.map.is_world_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc <<= 1;
				X = GMT_memory (C, X, n_alloc, struct GMT_XINGS);
			}
			go = FALSE;
		}
		lat_old = lat;
		last_x = this_x;	last_y = this_y;
	}

	if (nc > 0) {
		X = GMT_memory (C, X, nc, struct GMT_XINGS);
		*xings = X;
	}
	else
		GMT_free (C, X);

	return (nc);
}

GMT_LONG GMT_init_three_D (struct GMT_CTRL *C) {
	GMT_LONG i, easy, positive;
	double x, y, zmin = 0.0, zmax = 0.0;

	C->current.proj.three_D = (C->current.proj.z_project.view_azimuth != 180.0 || C->current.proj.z_project.view_elevation != 90.0);
	C->current.proj.scale[GMT_Z] = C->current.proj.z_pars[0];
	C->current.proj.xyz_pos[GMT_Z] = (C->current.proj.scale[GMT_Z] >= 0.0);	/* Increase z up or not */
	/* z_level == DBL_MAX is signaling that it was not set by the user. In that case we change it to the lower z level */
	if (C->current.proj.z_level == DBL_MAX) C->current.proj.z_level = (C->current.proj.xyz_pos[GMT_Z]) ?  C->common.R.wesn[ZLO] : C->common.R.wesn[ZHI];

	switch (C->current.proj.xyz_projection[GMT_Z]%3) {	/* Modulo 3 so that GMT_TIME (3) maps to GMT_LINEAR (0) */
		case GMT_LINEAR:	/* Regular scaling */
			zmin = (C->current.proj.xyz_pos[GMT_Z]) ? C->common.R.wesn[ZLO] : C->common.R.wesn[ZHI];
			zmax = (C->current.proj.xyz_pos[GMT_Z]) ? C->common.R.wesn[ZHI] : C->common.R.wesn[ZLO];
			C->current.proj.fwd_z = (PFL) GMT_translin;
			C->current.proj.inv_z = (PFL) GMT_itranslin;
			break;
		case GMT_LOG10:	/* Log10 transformation */
			if (C->common.R.wesn[ZLO] <= 0.0 || C->common.R.wesn[ZHI] <= 0.0) {
				GMT_report (C, GMT_MSG_FATAL, "Syntax error for -Jz -JZ option: limits must be positive for log10 projection\n");
				GMT_exit (EXIT_FAILURE);
			}
			zmin = (C->current.proj.xyz_pos[GMT_Z]) ? d_log10 (C, C->common.R.wesn[ZLO]) : d_log10 (C, C->common.R.wesn[ZHI]);
			zmax = (C->current.proj.xyz_pos[GMT_Z]) ? d_log10 (C, C->common.R.wesn[ZHI]) : d_log10 (C, C->common.R.wesn[ZLO]);
			C->current.proj.fwd_z = (PFL) GMT_translog10;
			C->current.proj.inv_z = (PFL) GMT_itranslog10;
			break;
		case GMT_POW:	/* x^y transformation */
			C->current.proj.xyz_pow[GMT_Z] = C->current.proj.z_pars[1];
			C->current.proj.xyz_ipow[GMT_Z] = 1.0 / C->current.proj.z_pars[1];
			positive = !(((GMT_LONG)C->current.proj.xyz_pos[GMT_Z] + (GMT_LONG)(C->current.proj.xyz_pow[GMT_Z] > 0.0)) % 2);
			zmin = (positive) ? pow (C->common.R.wesn[ZLO], C->current.proj.xyz_pow[GMT_Z]) : pow (C->common.R.wesn[ZHI], C->current.proj.xyz_pow[GMT_Z]);
			zmax = (positive) ? pow (C->common.R.wesn[ZHI], C->current.proj.xyz_pow[GMT_Z]) : pow (C->common.R.wesn[ZLO], C->current.proj.xyz_pow[GMT_Z]);
			C->current.proj.fwd_z = (PFL) GMT_transpowz;
			C->current.proj.inv_z = (PFL) GMT_itranspowz;
	}
	if (C->current.proj.compute_scale[GMT_Z]) C->current.proj.scale[GMT_Z] /= fabs (zmin - zmax);
	C->current.proj.zmax = (zmax - zmin) * C->current.proj.scale[GMT_Z];
	C->current.proj.origin[GMT_Z] = -zmin * C->current.proj.scale[GMT_Z];

	if (C->current.proj.z_project.view_azimuth >= 360.0) C->current.proj.z_project.view_azimuth -= 360.0;
	if (C->current.proj.z_project.view_azimuth < 0.0)    C->current.proj.z_project.view_azimuth += 360.0;
	C->current.proj.z_project.quadrant = (GMT_LONG)floor (C->current.proj.z_project.view_azimuth / 90.0) + 1;
	sincosd (C->current.proj.z_project.view_azimuth, &C->current.proj.z_project.sin_az, &C->current.proj.z_project.cos_az);
	sincosd (C->current.proj.z_project.view_elevation, &C->current.proj.z_project.sin_el, &C->current.proj.z_project.cos_el);

	/* Determine min/max y of plot */

	/* easy = TRUE means we can use 4 corner points to find min x and y, FALSE
	   means we must search along wesn perimeter the hard way */

	switch (C->current.proj.projection) {
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
			easy = C->common.R.oblique;
			break;
		default:
			easy = FALSE;
			break;
	}

	if (!C->current.proj.three_D) easy = TRUE;

	C->current.proj.z_project.xmin = C->current.proj.z_project.ymin = DBL_MAX;
	C->current.proj.z_project.xmax = C->current.proj.z_project.ymax = -DBL_MAX;

	if (easy) {
		double xx[4], yy[4];

		xx[0] = xx[3] = C->current.proj.rect[XLO]; xx[1] = xx[2] = C->current.proj.rect[XHI];
		yy[0] = yy[1] = C->current.proj.rect[YLO]; yy[2] = yy[3] = C->current.proj.rect[YHI];

		for (i = 0; i < 4; i++) {
			GMT_xy_to_geo (C, &C->current.proj.z_project.corner_x[i], &C->current.proj.z_project.corner_y[i], xx[i], yy[i]);
			GMT_xyz_to_xy (C, xx[i], yy[i], GMT_z_to_zz(C, C->common.R.wesn[ZLO]), &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			GMT_xyz_to_xy (C, xx[i], yy[i], GMT_z_to_zz(C, C->common.R.wesn[ZHI]), &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
		}
	}
	else {
		C->current.proj.z_project.corner_x[0] = C->current.proj.z_project.corner_x[3] = (C->current.proj.xyz_pos[GMT_X]) ? C->common.R.wesn[XLO] : C->common.R.wesn[XHI];
		C->current.proj.z_project.corner_x[1] = C->current.proj.z_project.corner_x[2] = (C->current.proj.xyz_pos[GMT_X]) ? C->common.R.wesn[XHI] : C->common.R.wesn[XLO];
		C->current.proj.z_project.corner_y[0] = C->current.proj.z_project.corner_y[1] = (C->current.proj.xyz_pos[GMT_Y]) ? C->common.R.wesn[YLO] : C->common.R.wesn[YHI];
		C->current.proj.z_project.corner_y[2] = C->current.proj.z_project.corner_y[3] = (C->current.proj.xyz_pos[GMT_Y]) ? C->common.R.wesn[YHI] : C->common.R.wesn[YLO];
		for (i = 0; i < C->current.map.n_lon_nodes; i++) {	/* S and N */
			GMT_geoz_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YLO], C->common.R.wesn[ZLO], &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			if (C->common.R.wesn[ZHI] != C->common.R.wesn[ZLO]) {
				GMT_geoz_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YLO], C->common.R.wesn[ZHI], &x, &y);
				C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
				C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
				C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
				C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			}
			GMT_geoz_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YHI], C->common.R.wesn[ZLO], &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			if (C->common.R.wesn[ZHI] != C->common.R.wesn[ZLO]) {
				GMT_geoz_to_xy (C, C->common.R.wesn[XLO] + i * C->current.map.dlon, C->common.R.wesn[YHI], C->common.R.wesn[ZHI], &x, &y);
				C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
				C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
				C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
				C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			}
		}
		for (i = 0; i < C->current.map.n_lat_nodes; i++) {	/* W and E */
			GMT_geoz_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO] + i * C->current.map.dlat, C->common.R.wesn[ZLO], &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			if (C->common.R.wesn[ZHI] != C->common.R.wesn[ZLO]) {
				GMT_geoz_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO] + i * C->current.map.dlat, C->common.R.wesn[ZHI], &x, &y);
				C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
				C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
				C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
				C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			}
			GMT_geoz_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO] + i * C->current.map.dlat, C->common.R.wesn[ZLO], &x, &y);
			C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
			C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
			C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
			C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			if (C->common.R.wesn[ZHI] != C->common.R.wesn[ZLO]) {
				GMT_geoz_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO] + i * C->current.map.dlat, C->common.R.wesn[ZHI], &x, &y);
				C->current.proj.z_project.ymin = MIN (C->current.proj.z_project.ymin, y);
				C->current.proj.z_project.ymax = MAX (C->current.proj.z_project.ymax, y);
				C->current.proj.z_project.xmin = MIN (C->current.proj.z_project.xmin, x);
				C->current.proj.z_project.xmax = MAX (C->current.proj.z_project.xmax, x);
			}
		}
	}

	C->current.proj.z_project.face[0] = (C->current.proj.z_project.quadrant == 1 || C->current.proj.z_project.quadrant == 2) ? 0 : 1;
	C->current.proj.z_project.face[1] = (C->current.proj.z_project.quadrant == 1 || C->current.proj.z_project.quadrant == 4) ? 2 : 3;
	C->current.proj.z_project.face[2] = (C->current.proj.z_project.view_elevation >= 0.0) ? 4 : 5;
	C->current.proj.z_project.draw[0] = (C->current.proj.z_project.quadrant == 1 || C->current.proj.z_project.quadrant == 4) ? TRUE : FALSE;
	C->current.proj.z_project.draw[1] = (C->current.proj.z_project.quadrant == 3 || C->current.proj.z_project.quadrant == 4) ? TRUE : FALSE;
	C->current.proj.z_project.draw[2] = (C->current.proj.z_project.quadrant == 2 || C->current.proj.z_project.quadrant == 3) ? TRUE : FALSE;
	C->current.proj.z_project.draw[3] = (C->current.proj.z_project.quadrant == 1 || C->current.proj.z_project.quadrant == 2) ? TRUE : FALSE;
	C->current.proj.z_project.sign[0] = C->current.proj.z_project.sign[3] = -1.0;
	C->current.proj.z_project.sign[1] = C->current.proj.z_project.sign[2] = 1.0;
	C->current.proj.z_project.z_axis = (C->current.proj.z_project.quadrant%2) ? C->current.proj.z_project.quadrant : C->current.proj.z_project.quadrant - 2;

	if (C->current.proj.z_project.fixed) {
		if (!C->current.proj.z_project.world_given) {	/* Pick center point of region */
			C->current.proj.z_project.world_x = (GMT_is_geographic (C, GMT_IN)) ? C->current.proj.central_meridian : 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
			C->current.proj.z_project.world_y = 0.5 * (C->common.R.wesn[YLO] + C->common.R.wesn[YHI]);
			C->current.proj.z_project.world_z = C->current.proj.z_level;
		}
		GMT_geoz_to_xy (C, C->current.proj.z_project.world_x, C->current.proj.z_project.world_y, C->current.proj.z_project.world_z, &x, &y);
		if (!C->current.proj.z_project.view_given) {	/* Pick center of current page */
			C->current.proj.z_project.view_x = 0.5 * C->current.setting.ps_page_size[0] * C->session.u2u[GMT_PT][GMT_INCH];
			C->current.proj.z_project.view_y = 0.5 * C->current.setting.ps_page_size[1] * C->session.u2u[GMT_PT][GMT_INCH];
		}
		C->current.proj.z_project.x_off = C->current.proj.z_project.view_x - x;
		C->current.proj.z_project.y_off = C->current.proj.z_project.view_y - y;
	}
	else {
		C->current.proj.z_project.x_off = -C->current.proj.z_project.xmin;
		C->current.proj.z_project.y_off = -C->current.proj.z_project.ymin;
	}

	/* Adjust the xmin/xmax and ymin/ymax because of xoff and yoff */
	C->current.proj.z_project.xmin += C->current.proj.z_project.x_off;
	C->current.proj.z_project.xmax += C->current.proj.z_project.x_off;
	C->current.proj.z_project.ymin += C->current.proj.z_project.y_off;
	C->current.proj.z_project.ymax += C->current.proj.z_project.y_off;

	return (GMT_NOERROR);
}

GMT_LONG GMT_map_setup (struct GMT_CTRL *C, double wesn[])
{
	GMT_LONG search;

	if (!C->common.J.active) return (GMT_MAP_NO_PROJECTION);
	if (wesn[XHI] == wesn[XLO] && wesn[YHI] == wesn[YLO]) return (GMT_MAP_NO_REGION);	/* Since -R may not be involved if there are grids */

	GMT_init_ellipsoid (C);	/* Set parameters depending on the ellipsoid since the latter could have been set explicitly */

	if (GMT_x_is_lon (C, GMT_IN)) {
		/* Limit east-west range to 360 and make sure east > -180 and west < 360 */
		if (wesn[XHI] < wesn[XLO]) wesn[XHI] += 360.0;
		if ((fabs (wesn[XHI] - wesn[XLO]) - 360.0) > GMT_SMALL) return (GMT_MAP_EXCEEDS_360);
		while (wesn[XHI] < -180.0) {
			wesn[XLO] += 360.0;
			wesn[XHI] += 360.0;
		}
		while (wesn[XLO] > 360.0) {
			wesn[XLO] -= 360.0;
			wesn[XHI] -= 360.0;
		}
	}
	if (C->current.proj.got_elevations) {
		if (wesn[YLO] < 0.0 || wesn[YLO] >= 90.0) return (GMT_MAP_BAD_ELEVATION_MIN);
		if (wesn[YHI] <= 0.0 || wesn[YHI] > 90.0) return (GMT_MAP_BAD_ELEVATION_MAX);
	}
	if (GMT_y_is_lat (C, GMT_IN)) {
		if (wesn[YLO] < -90.0 || wesn[YLO] > 90.0) return (GMT_MAP_BAD_LAT_MIN);
		if (wesn[YHI] < -90.0 || wesn[YHI] > 90.0) return (GMT_MAP_BAD_LAT_MAX);
	}

	GMT_memcpy (C->common.R.wesn, wesn, 4, double);
	C->current.proj.GMT_convert_latitudes = FALSE;
	if (C->current.proj.gave_map_width) C->current.proj.units_pr_degree = FALSE;

	C->current.map.n_lon_nodes = C->current.map.n_lat_nodes = 0;
	C->current.map.wrap_around_check = (PFL) GMT_wrap_around_check_x;
	C->current.map.jump = (PFL) GMT_map_jump_x;
	C->current.map.will_it_wrap = (PFB) GMT_will_it_wrap_x;
	C->current.map.this_point_wraps = (PFB) GMT_this_point_wraps_x;
	C->current.map.get_crossings = (PFV) GMT_get_crossings_x;

	C->current.map.lon_wrap = TRUE;
	GMT_lat_swap_init (C);

	switch (C->current.proj.projection) {

		case GMT_LINEAR:		/* Linear transformations */
			search = GMT_map_init_linear (C);
			break;

		case GMT_POLAR:		/* Both lon/lat are actually theta, radius */
			search = GMT_map_init_polar (C);
			break;

		case GMT_MERCATOR:		/* Standard Mercator projection */
			search = GMT_map_init_merc (C);
			break;

		case GMT_STEREO:		/* Stereographic projection */
			search = GMT_map_init_stereo (C);
			break;

		case GMT_LAMBERT:		/* Lambert Conformal Conic */
			search = GMT_map_init_lambert (C);
			break;

		case GMT_OBLIQUE_MERC:	/* Oblique Mercator */
			search = GMT_map_init_oblique (C);
			break;

		case GMT_TM:		/* Transverse Mercator */
			search = GMT_map_init_tm (C);
			break;

		case GMT_UTM:		/* Universal Transverse Mercator */
			search = GMT_map_init_utm (C);
			break;

		case GMT_LAMB_AZ_EQ:	/* Lambert Azimuthal Equal-Area */
			search = GMT_map_init_lambeq (C);
			break;

		case GMT_ORTHO:		/* Orthographic Projection */
			search = GMT_map_init_ortho (C);
			break;

		case GMT_GENPER:		/* General Perspective Projection */
			search = GMT_map_init_genper (C);
			break;

		case GMT_AZ_EQDIST:		/* Azimuthal Equal-Distance Projection */
			search = GMT_map_init_azeqdist (C);
			break;

		case GMT_GNOMONIC:		/* Azimuthal Gnomonic Projection */
			search = GMT_map_init_gnomonic (C);
			break;

		case GMT_MOLLWEIDE:		/* Mollweide Equal-Area */
			search = GMT_map_init_mollweide (C);
			break;

		case GMT_HAMMER:		/* Hammer-Aitoff Equal-Area */
			search = GMT_map_init_hammer (C);
			break;

		case GMT_VANGRINTEN:		/* Van der Grinten */
			search = GMT_map_init_grinten (C);
			break;

		case GMT_WINKEL:		/* Winkel Tripel */
			search = GMT_map_init_winkel (C);
			break;

		case GMT_ECKERT4:		/* Eckert IV */
			search = GMT_map_init_eckert4 (C);
			break;

		case GMT_ECKERT6:		/* Eckert VI */
			search = GMT_map_init_eckert6 (C);
			break;

		case GMT_CYL_EQ:		/* Cylindrical Equal-Area */
			search = GMT_map_init_cyleq (C);
			break;

		case GMT_CYL_STEREO:			/* Cylindrical Stereographic */
			search = GMT_map_init_cylstereo (C);
			break;

		case GMT_MILLER:		/* Miller Cylindrical */
			search = GMT_map_init_miller (C);
			break;

		case GMT_CYL_EQDIST:	/* Cylindrical Equidistant */
			search = GMT_map_init_cyleqdist (C);
			break;

		case GMT_ROBINSON:		/* Robinson */
			search = GMT_map_init_robinson (C);
			break;

		case GMT_SINUSOIDAL:	/* Sinusoidal Equal-Area */
			search = GMT_map_init_sinusoidal (C);
			break;

		case GMT_CASSINI:		/* Cassini cylindrical */
			search = GMT_map_init_cassini (C);
			break;

		case GMT_ALBERS:		/* Albers Equal-Area Conic */
			search = GMT_map_init_albers (C);
			break;

		case GMT_ECONIC:		/* Equidistant Conic */
			search = GMT_map_init_econic (C);
			break;

		case GMT_POLYCONIC:		/* Polyconic */
			search = GMT_map_init_polyconic (C);
			break;

		default:	/* No projection selected, return to a horrible death */
			return (GMT_MAP_NO_PROJECTION);
	}

	C->current.proj.i_scale[GMT_X] = (C->current.proj.scale[GMT_X] != 0.0) ? 1.0 / C->current.proj.scale[GMT_X] : 1.0;
	C->current.proj.i_scale[GMT_Y] = (C->current.proj.scale[GMT_Y] != 0.0) ? 1.0 / C->current.proj.scale[GMT_Y] : 1.0;
	C->current.proj.i_scale[GMT_Z] = (C->current.proj.scale[GMT_Z] != 0.0) ? 1.0 / C->current.proj.scale[GMT_Z] : 1.0;

	C->current.map.width  = fabs (C->current.proj.rect[XHI] - C->current.proj.rect[XLO]);
	C->current.map.height = fabs (C->current.proj.rect[YHI] - C->current.proj.rect[YLO]);
	C->current.map.half_width  = 0.5 * C->current.map.width;
	C->current.map.half_height = 0.5 * C->current.map.height;

	if (!C->current.map.n_lon_nodes) C->current.map.n_lon_nodes = irint (C->current.map.width / C->current.setting.map_line_step);
	if (!C->current.map.n_lat_nodes) C->current.map.n_lat_nodes = irint (C->current.map.height / C->current.setting.map_line_step);

	C->current.map.dlon = (C->common.R.wesn[XHI] - C->common.R.wesn[XLO]) / C->current.map.n_lon_nodes;
	C->current.map.dlat = (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) / C->current.map.n_lat_nodes;

	if (C->current.map.width > 400.0 && !C->PSL) {	/* PSL not active so we assume this is ***project calling */
		search = FALSE;	/* Safe-guard that prevents region search below for mapproject and others (400 inch = ~> 10 meters) */
		GMT_report (C, GMT_MSG_DEBUG, "Warning: GMT_map_setup perimeter search skipped due to excessive (> 10m) plot size; probably a mapproject process.\n");
	}

	if (search) {	/* Loop around rectangular perimeter and determine min/max lon/lat extent */
		GMT_wesn_search (C, C->current.proj.rect[XLO], C->current.proj.rect[XHI], C->current.proj.rect[YLO], C->current.proj.rect[YHI], &C->common.R.wesn[XLO], &C->common.R.wesn[XHI], &C->common.R.wesn[YLO], &C->common.R.wesn[YHI]);
		C->current.map.dlon = (C->common.R.wesn[XHI] - C->common.R.wesn[XLO]) / C->current.map.n_lon_nodes;
		C->current.map.dlat = (C->common.R.wesn[YHI] - C->common.R.wesn[YLO]) / C->current.map.n_lat_nodes;
	}

	if (GMT_IS_AZIMUTHAL(C) && C->common.R.oblique) GMT_horizon_search (C, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI], C->current.proj.rect[XLO], C->current.proj.rect[XHI], C->current.proj.rect[YLO], C->current.proj.rect[YHI]);

	if (C->current.proj.central_meridian < C->common.R.wesn[XLO] && (C->current.proj.central_meridian + 360.0) <= C->common.R.wesn[XHI]) C->current.proj.central_meridian += 360.0;
	if (C->current.proj.central_meridian > C->common.R.wesn[XHI] && (C->current.proj.central_meridian - 360.0) >= C->common.R.wesn[XLO]) C->current.proj.central_meridian -= 360.0;

	/* Maximum step size (in degrees) used for interpolation of line segments along great circles (or meridians/parallels)  before they are plotted */
	C->current.map.path_step = C->current.setting.map_line_step / C->current.proj.scale[GMT_X] / C->current.proj.M_PR_DEG;

	GMT_init_three_D (C);

	return (GMT_NOERROR);
}

GMT_LONG GMT_init_distaz (struct GMT_CTRL *C, char c, GMT_LONG mode, GMT_LONG type)
{
	/* Initializes distance calcuation given the selected values for:
	 * Distance unit c: must be on of the following:
	 *  1) d|e|f|k|m|M|n|s
	 *  2) C (Cartesian distance after projecting with -J) | X (Cartesian)
	 *  3) S (cosine distance) | P (cosine after first inverse projecting with -J)
	 * distance-calculation modifer mode: 0 (Cartesian), 1 (flat Earth), 2 (great-circle, 3 (geodesic)
	 * type: 0 = map distances, 1 = contour distances, 2 = contour annotation distances
	 * We set distance and azimuth functions and scales for this type.
	 * At the moment there is only one azimuth function pointer for all.
	 */

	GMT_LONG proj_type = GMT_GEOGRAPHIC;	/* Default is to just use the geographic coordinates as they are */

	switch (c) {
			/* First the three arc angular distance units */
			
		case 'd':	/* Arc degrees on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_DEG + mode, type);
			C->current.map.dist[type].arc = TRUE;	/* Angular measure */
			break;
		case 'm':	/* Arc minutes on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_DEG + mode, type);
			C->current.map.dist[type].scale = GMT_DEG2MIN_F;
			C->current.map.dist[type].arc = TRUE;	/* Angular measure */
			break;
		case 's':	/* Arc seconds on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_DEG + mode, type);
			C->current.map.dist[type].scale = GMT_DEG2SEC_F;
			C->current.map.dist[type].arc = TRUE;	/* Angular measure */
			break;
			
			/* Various distance units on the planetary body */
			
		case 'e':	/* Meters on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_M + mode, type);
			break;
		case 'f':	/* Feet on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_M + mode, type);
			C->current.map.dist[type].scale = 1.0 / METERS_IN_A_FOOT;
			break;
		case 'k':	/* Km on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_M + mode, type);
			C->current.map.dist[type].scale = 1.0 / METERS_IN_A_KM;
			break;
		case 'M':	/* Statute Miles on spherical body using desired metric mode  */
			GMT_set_distaz (C, GMT_DIST_M + mode, type);
			C->current.map.dist[type].scale = 1.0 / METERS_IN_A_MILE;
			break;
		case 'n':	/* Nautical miles on spherical body using desired metric mode */
			GMT_set_distaz (C, GMT_DIST_M + mode, type);
			C->current.map.dist[type].scale = 1.0 / METERS_IN_A_NAUTICAL_MILE;
			break;
			
			/* Cartesian distances */
			
		case 'X':	/* Cartesian distances in user units */
			proj_type = GMT_CARTESIAN;
			GMT_set_distaz (C, GMT_CARTESIAN_DIST, type);
			break;
		case 'C':	/* Cartesian distances (in PROJ_LENGTH_UNIT) after first projecting input coordinates with -J */
			GMT_set_distaz (C, GMT_CARTESIAN_DIST_PROJ, type);
			proj_type = GMT_GEO2CART;
			break;
			
			/* Specialized cosine distances used internally only (e.g., greenspline) */
			
		case 'S':	/* Spherical cosine distances (for various gridding functions) */
			GMT_set_distaz (C, GMT_DIST_COS + mode, type);
			break;
		case 'P':	/* Spherical distances after first inversily projecting Cartesian coordinates with -J */
			GMT_set_distaz (C, GMT_CARTESIAN_DIST_PROJ_INV, type);
			proj_type = GMT_CART2GEO;
			break;
			
		default:
			GMT_report (C, GMT_MSG_FATAL, "Syntax error: Distance units must be one of %s|X|C|S|P\n", GMT_LEN_UNITS_DISPLAY);
			GMT_exit (EXIT_FAILURE);
			break;
	}
	
	C->current.map.dist[type].init = TRUE;	/* OK, we have now initialized the info for this type */
	return (proj_type);
}

void GMT_set_distaz (struct GMT_CTRL *C, GMT_LONG mode, GMT_LONG type)
{	/* Assigns pointers to the chosen distance and azimuth functions */
	PFD az_func = NULL;
	C->current.map.dist[type].scale = 1.0;	/* Default scale */

	switch (mode) {	/* Set pointers to distance functions */
		case GMT_CARTESIAN_DIST:	/* Cartesian 2-D x,y data */
			C->current.map.dist[type].func = GMT_cartesian_dist;
			az_func = GMT_az_backaz_cartesian;
			break;
		case GMT_CARTESIAN_DIST_PROJ:	/* Cartesian distance after projecting 2-D lon,lat data */
			C->current.map.dist[type].func = GMT_cartesian_dist_proj;
			az_func = GMT_az_backaz_cartesian_proj;
			break;
		case GMT_DIST_M+GMT_FLATEARTH:	/* 2-D lon, lat data, but scale to Cartesian flat earth in meter */
			C->current.map.dist[type].func = GMT_flatearth_dist_meter;
			az_func  = GMT_az_backaz_flatearth;
			break;
		case GMT_DIST_M+GMT_GREATCIRCLE:	/* 2-D lon, lat data, use spherical distances in meter */
			C->current.map.dist[type].func = GMT_great_circle_dist_meter;
			az_func = GMT_az_backaz_sphere;
			break;
		case GMT_DIST_M+GMT_GEODESIC:	/* 2-D lon, lat data, use geodesic distances in meter */
			C->current.map.dist[type].func = GMT_geodesic_dist_meter;
			az_func = GMT_az_backaz_geodesic;
			break;
		case GMT_DIST_DEG+GMT_FLATEARTH:	/* 2-D lon, lat data, use Flat Earth distances in degrees */
			C->current.map.dist[type].func = GMT_flatearth_dist_degree;
			az_func = GMT_az_backaz_flatearth;
			break;
		case GMT_DIST_DEG+GMT_GREATCIRCLE:	/* 2-D lon, lat data, use spherical distances in degrees */
			C->current.map.dist[type].func = GMT_great_circle_dist_degree;
			az_func = GMT_az_backaz_sphere;
			break;
		case GMT_DIST_DEG+GMT_GEODESIC:	/* 2-D lon, lat data, use geodesic distances in degrees */
			C->current.map.dist[type].func = GMT_geodesic_dist_degree;
			az_func = GMT_az_backaz_geodesic;
			break;
		case GMT_DIST_COS+GMT_GREATCIRCLE:	/* 2-D lon, lat data, and Green's function needs cosine of spherical distance */
			C->current.map.dist[type].func = GMT_great_circle_dist_cos;
			az_func = GMT_az_backaz_sphere;
			break;
		case GMT_DIST_COS+GMT_GEODESIC:	/* 2-D lon, lat data, and Green's function needs cosine of geodesic distance */
			C->current.map.dist[type].func = GMT_geodesic_dist_cos;
			az_func = GMT_az_backaz_geodesic;
			break;
		default:	/* Cannot happen unless we make a bug */
			GMT_report (C, GMT_MSG_FATAL, "Mode (=%ld) for distance function is unknown. Must be bug.\n", mode);
			exit (EXIT_FAILURE);
			break;
	}
	if (type > 0) return;	/* Contour-related assignemnts end here */

	/* Mapping only */
	C->current.map.azimuth_func = az_func;
	if (mode == GMT_CARTESIAN_DIST)	{	/* Cartesian data */
		C->current.map.near_lines_func  = (PFL) GMT_near_lines_cartesian;
		C->current.map.near_a_line_func  = (PFL) GMT_near_a_line_cartesian;
		C->current.map.near_point_func = (PFL) GMT_near_a_point_cartesian;
	}
	else {	/* Geographic data */
		C->current.map.near_lines_func  = (PFL) GMT_near_lines_spherical;
		C->current.map.near_a_line_func  = (PFL) GMT_near_a_line_spherical;
		C->current.map.near_point_func = (PFL) GMT_near_a_point_spherical;
	}
}
