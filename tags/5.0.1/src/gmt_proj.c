/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * gmt_proj.c contains the specific projection functions that convert
 * longitude and latitude to x, y.  These are used via function pointers
 * set in gmt_map.c
 *
 * Map_projections include functions that will set up the transformation
 * between xy and latlon for several map projections.
 *
 * A few of the core coordinate transformation functions are based on similar
 * FORTRAN routines written by Pat Manley, Doug Shearer, and Bill Haxby, and
 * have been rewritten in C and subsequently streamlined.  The Lambert conformal
 * was originally coded up by Bernie Coakley.  The rest is coded by Wessel/Smith
 * based on P. Snyder, "Map Projections - a working manual", USGS Prof paper 1395.
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
 *	Cylindrical Stereographic (e.g., Gall, B.S.A.M)
 *	Miller Cylindrical
 *	Oblique Mercator
 *	TM Transverse Mercator (Ellipsoidal and Spherical)
 *	UTM Universal Transverse Mercator
 *	Cylindrical Equal-area (e.g., Gall-Peters, Behrmann)
 *	Cylindrical Equidistant (e.g., Plate Carree)
 *  Conic:
 *	Albers Equal-Area Conic
 *	Lambert Conformal Conic
 *	Equidistant Conic
 *  Azimuthal:
 *	Stereographic Conformal
 *	Lambert Azimuthal Equal-Area
 *	Orthographic
 *	Azimuthal Equidistant
 *	Gnomonic
 *  Thematic:
 *	Mollweide Equal-Area
 *	Hammer-Aitoff Equal-Area
 *	Sinusoidal Equal-Area
 *	Winkel Tripel
 *	Robinson
 *	Eckert IV
 *	Eckert IV
 *	Van der Grinten
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5.x
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

void gmt_check_R_J (struct GMT_CTRL *C, double *clon)	/* Make sure -R and -J agree for global plots; J given priority */
{
	double lon0 = 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]);
	
	if (C->current.map.is_world && lon0 != *clon) {
		C->common.R.wesn[XLO] = *clon - 180.0;
		C->common.R.wesn[XHI] = *clon + 180.0;
		GMT_report (C, GMT_MSG_NORMAL, "Warning: Central meridian set with -J (%g) implies -R%g/%g/%g/%g\n",
			*clon, C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI]);
	}
	else if (!C->current.map.is_world) {
		lon0 = *clon - 360.0;
		while (lon0 < C->common.R.wesn[XLO]) lon0 += 360.0;
		if (lon0 > C->common.R.wesn[XHI]) GMT_report (C, GMT_MSG_NORMAL, "Warning: Central meridian outside region\n");
	}
}

/* LINEAR TRANSFORMATIONS */

void GMT_translin (struct GMT_CTRL *C, double forw, double *inv)	/* Linear forward */
{
	*inv = forw;
}

void GMT_translind (struct GMT_CTRL *C, double forw, double *inv)	/* Linear forward, but with degrees*/
{
	GMT_WIND_LON (C, forw)	/* Make sure forw is in -180/+180 range after removing central meridian */
	*inv = forw ;
}

void GMT_itranslind (struct GMT_CTRL *C, double *forw, double inv)	/* Linear inverse, but with degrees*/
{
	while (inv < -GMT_180) inv += 360.0;
	while (inv > +GMT_180) inv -= 360.0;
	*forw = inv + C->current.proj.central_meridian;
}

void GMT_itranslin (struct GMT_CTRL *C, double *forw, double inv)	/* Linear inverse */
{
	*forw = inv;
}

void GMT_translog10 (struct GMT_CTRL *C, double forw, double *inv)	/* Log10 forward */
{
	*inv = d_log10 (C, forw);
}

void GMT_itranslog10 (struct GMT_CTRL *C, double *forw, double inv) /* Log10 inverse */
{
	*forw = pow (10.0, inv);
}

void GMT_transpowx (struct GMT_CTRL *C, double x, double *x_in)	/* pow x forward */
{
	*x_in = pow (x, C->current.proj.xyz_pow[GMT_X]);
}

void GMT_itranspowx (struct GMT_CTRL *C, double *x, double x_in) /* pow x inverse */
{
	*x = pow (x_in, C->current.proj.xyz_ipow[GMT_X]);
}

void GMT_transpowy (struct GMT_CTRL *C, double y, double *y_in)	/* pow y forward */
{
	*y_in = pow (y, C->current.proj.xyz_pow[GMT_Y]);
}

void GMT_itranspowy (struct GMT_CTRL *C, double *y, double y_in) /* pow y inverse */
{
	*y = pow (y_in, C->current.proj.xyz_ipow[GMT_Y]);
}

void GMT_transpowz (struct GMT_CTRL *C, double z, double *z_in)	/* pow z forward */
{
	*z_in = pow (z, C->current.proj.xyz_pow[GMT_Z]);
}

void GMT_itranspowz (struct GMT_CTRL *C, double *z, double z_in) /* pow z inverse */
{
	*z = pow (z_in, C->current.proj.xyz_ipow[GMT_Z]);
}

/* -JP POLAR (r-theta) PROJECTION */

void GMT_vpolar (struct GMT_CTRL *C, double lon0)
{
	/* Set up a Polar (theta,r) transformation */

	C->current.proj.p_base_angle = lon0;
	C->current.proj.central_meridian = 0.5 * (C->common.R.wesn[XHI] + C->common.R.wesn[XLO]);

	/* Plus pretend that it is kind of a geographic polar projection */

	C->current.proj.north_pole = C->current.proj.got_elevations;
	C->current.proj.pole = (C->current.proj.got_elevations) ? 90.0 : 0.0;
}

void GMT_polar (struct GMT_CTRL *C, double x, double y, double *x_i, double *y_i)
{	/* Transform x and y to polar(cylindrical) coordinates */
	if (C->current.proj.got_azimuths) x = 90.0 - x;		/* azimuths, not directions */
	if (C->current.proj.got_elevations) y = 90.0 - y;		/* elevations */
	sincosd (x - C->current.proj.p_base_angle, y_i, x_i);	/* Change base line angle */
	(*x_i) *= y;
	(*y_i) *= y;
}

void GMT_ipolar (struct GMT_CTRL *C, double *x, double *y, double x_i, double y_i)
{	/* Inversely transform both x and y from polar(cylindrical) coordinates */
	*x = d_atan2d (y_i, x_i) + C->current.proj.p_base_angle;
	if (C->current.proj.got_azimuths) *x = 90.0 - (*x);		/* azimuths, not directions */
	*y = hypot (x_i, y_i);
	if (C->current.proj.got_elevations) *y = 90.0 - (*y);    /* elevations, presumably */
}

/* -JM MERCATOR PROJECTION */

void GMT_vmerc (struct GMT_CTRL *C, double lon0, double slat)
{	/* Set up a Mercator transformation */

	C->current.proj.central_meridian = lon0;
	C->current.proj.j_x = cosd (slat) / d_sqrt (1.0 - C->current.proj.ECC2 * sind (slat) * sind (slat)) * C->current.proj.EQ_RAD;
	C->current.proj.j_ix = 1.0 / C->current.proj.j_x;
}

/* Mercator projection for the sphere */

void GMT_merc_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Mercator x/y (C->current.proj.EQ_RAD in C->current.proj.j_x) */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);

	*x = C->current.proj.j_x * D2R * lon;
	*y = (fabs (lat) < 90.0) ? C->current.proj.j_x * d_log (C, tand (45.0 + 0.5 * lat)) : copysign (DBL_MAX, lat);
}

void GMT_imerc_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Mercator x/y to lon/lat  (C->current.proj.EQ_RAD in C->current.proj.j_ix) */

	*lon = x * C->current.proj.j_ix * R2D + C->current.proj.central_meridian;
	*lat = atand (sinh (y * C->current.proj.j_ix));
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_latc_to_latg (C, *lat);
}

/* -JY CYLINDRICAL EQUAL-AREA PROJECTION */

void GMT_vcyleq (struct GMT_CTRL *C, double lon0, double slat)
{	/* Set up a Cylindrical equal-area transformation */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.j_x = C->current.proj.EQ_RAD * D2R * cosd (slat);
	C->current.proj.j_y = C->current.proj.EQ_RAD / cosd (slat);
	C->current.proj.j_ix = 1.0 / C->current.proj.j_x;
	C->current.proj.j_iy = 1.0 / C->current.proj.j_y;
}

void GMT_cyleq (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Cylindrical equal-area x/y */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);

	*x = lon * C->current.proj.j_x;
	*y = C->current.proj.j_y * sind (lat);
	if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= C->current.proj.Dx;
		(*y) *= C->current.proj.Dy;
	}
}

void GMT_icyleq (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Cylindrical equal-area x/y to lon/lat */

	if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
		x *= C->current.proj.iDx;
		y *= C->current.proj.iDy;
	}
	*lon = x * C->current.proj.j_ix + C->current.proj.central_meridian;
	*lat = d_asind (y * C->current.proj.j_iy);
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

/* -JQ CYLINDRICAL EQUIDISTANT PROJECTION */

void GMT_vcyleqdist (struct GMT_CTRL *C, double lon0, double slat)
{	/* Set up a Cylindrical equidistant transformation */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.j_x = D2R * C->current.proj.EQ_RAD * cosd (slat);
	C->current.proj.j_y = D2R * C->current.proj.EQ_RAD;
	C->current.proj.j_ix = 1.0 / C->current.proj.j_x;
	C->current.proj.j_iy = 1.0 / C->current.proj.j_y;
}

void GMT_cyleqdist (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Cylindrical equidistant x/y */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * C->current.proj.j_x;
	*y = lat * C->current.proj.j_y;
}

void GMT_icyleqdist (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Cylindrical equal-area x/y to lon/lat */

	*lon = x * C->current.proj.j_ix + C->current.proj.central_meridian;
	*lat = y * C->current.proj.j_iy;
}

/* -JJ MILLER CYLINDRICAL PROJECTION */

void GMT_vmiller (struct GMT_CTRL *C, double lon0)
{	/* Set up a Miller Cylindrical transformation */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.j_x = D2R * C->current.proj.EQ_RAD;
	C->current.proj.j_y = 1.25 * C->current.proj.EQ_RAD;
	C->current.proj.j_ix = 1.0 / C->current.proj.j_x;
	C->current.proj.j_iy = 1.0 / C->current.proj.j_y;
}

void GMT_miller (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Miller Cylindrical x/y */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * C->current.proj.j_x;
	*y = C->current.proj.j_y * d_log (C, tand (45.0 + 0.4 * lat));
}

void GMT_imiller (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Miller Cylindrical x/y to lon/lat */

	*lon = x * C->current.proj.j_ix + C->current.proj.central_meridian;
	*lat = 2.5 * atand (exp (y * C->current.proj.j_iy)) - 112.5;
}

/* -JCyl_stere CYLINDRICAL STEREOGRAPHIC PROJECTION */

void GMT_vcylstereo (struct GMT_CTRL *C, double lon0, double slat)
{	/* Set up a Cylindrical Stereographic transformation */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.j_x = C->current.proj.EQ_RAD * D2R * cosd (slat);
	C->current.proj.j_y = C->current.proj.EQ_RAD * (1.0 + cosd (slat));
	C->current.proj.j_ix = 1.0 / C->current.proj.j_x;
	C->current.proj.j_iy = 1.0 / C->current.proj.j_y;
}

void GMT_cylstereo (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Cylindrical Stereographic x/y */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * C->current.proj.j_x;
	*y = C->current.proj.j_y * tand (0.5 * lat);
}

void GMT_icylstereo (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Cylindrical Stereographic x/y to lon/lat */

	*lon = x * C->current.proj.j_ix + C->current.proj.central_meridian;
	*lat = 2.0 * atand (y * C->current.proj.j_iy);
}

/* -JS POLAR STEREOGRAPHIC PROJECTION */

void GMT_vstereo (struct GMT_CTRL *C, double lon0, double lat0, double horizon)
{	/* Set up a Stereographic transformation */

	double clat;
	if (C->current.proj.GMT_convert_latitudes) {	/* Set Conformal radius and pole latitude */
		GMT_scale_eqrad (C);
		clat = GMT_latg_to_latc (C, lat0);
	}
	else
		clat = lat0;

	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;		/* This is always geodetic */
	sincosd (clat, &(C->current.proj.sinp), &(C->current.proj.cosp));	/* These may be conformal */
	C->current.proj.north_pole = (lat0 > 0.0);
	C->current.proj.s_c = 2.0 * C->current.proj.EQ_RAD * C->current.setting.proj_scale_factor;
	C->current.proj.s_ic = 1.0 / C->current.proj.s_c;
	C->current.proj.f_horizon = horizon;
	C->current.proj.rho_max = tand (0.5 * horizon) * C->current.proj.s_c;
}

void GMT_plrs_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to x/y using Spherical polar projection */
	double rho, slon, clon;

	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);
	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lon, &slon, &clon);
	if (C->current.proj.north_pole) {
		rho = C->current.proj.s_c * tand (45.0 - 0.5 * lat);
		*y = -rho * clon;
		*x =  rho * slon;
	}
	else {
		rho = C->current.proj.s_c * tand (45.0 + 0.5 * lat);
		*y = rho * clon;
		*x = rho * slon;
	}
	if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= C->current.proj.Dx;
		(*y) *= C->current.proj.Dy;
	}
}

void GMT_iplrs_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Spherical polar x/y to lon/lat */
	double c;

	if (x == 0.0 && y == 0.0) {
		*lon = C->current.proj.central_meridian;
		*lat = C->current.proj.pole;
		return;
	}

	if (C->current.proj.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= C->current.proj.iDx;
		y *= C->current.proj.iDy;
	}
	c = 2.0 * atan (hypot (x, y) * C->current.proj.s_ic);

	if (C->current.proj.north_pole) {
		*lon = C->current.proj.central_meridian + d_atan2d (x, -y);
		*lat = d_asind (cos (c));
	}
	else {
		*lon = C->current.proj.central_meridian + d_atan2d (x, y);
		*lat = d_asind (-cos (c));
	}
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_latc_to_latg (C, *lat);

}

void GMT_stereo1_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to x/y using Spherical stereographic projection, oblique view */

	double sin_dlon, cos_dlon, s, c, cc, A;

	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);
	sincosd (lon - C->current.proj.central_meridian, &sin_dlon, &cos_dlon);
	sincosd (lat, &s, &c);
	cc = c * cos_dlon;
	A = C->current.proj.s_c / (1.0 + C->current.proj.sinp * s + C->current.proj.cosp * cc);
	*x = A * c * sin_dlon;
	*y = A * (C->current.proj.cosp * s - C->current.proj.sinp * cc);
	if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= C->current.proj.Dx;
		(*y) *= C->current.proj.Dy;
	}
}

void GMT_istereo_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{
	double rho, c, sin_c, cos_c;

	if (C->current.proj.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= C->current.proj.iDx;
		y *= C->current.proj.iDy;
	}
	rho = hypot (x, y);
	if (rho > C->current.proj.rho_max) {
		*lat = *lon = C->session.d_NaN;
		return;
	}
	c = 2.0 * atan (rho * C->current.proj.s_ic);
	sincos (c, &sin_c, &cos_c);
	if (rho != 0.0) sin_c /= rho;
	*lat = asind (cos_c * C->current.proj.sinp + y * sin_c * C->current.proj.cosp);
	*lon = d_atan2d (x * sin_c, cos_c * C->current.proj.cosp - y * sin_c * C->current.proj.sinp) + C->current.proj.central_meridian;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_latc_to_latg (C, *lat);
}

/* Spherical equatorial view */

void GMT_stereo2_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to x/y using stereographic projection, equatorial view */

	double dlon, s, c, clon, slon, A;

	dlon = lon - C->current.proj.central_meridian;
	if (doubleAlmostEqual (dlon, 180.0)) {
		*x = *y = 0.0;
	}
	else {
		if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);
		sincosd (lat, &s, &c);
		sincosd (dlon, &slon, &clon);
		A = C->current.proj.s_c / (1.0 + c * clon);
		*x = A * c * slon;
		*y = A * s;
		if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= C->current.proj.Dx;
			(*y) *= C->current.proj.Dy;
		}
	}
}

/* -JL LAMBERT CONFORMAL CONIC PROJECTION */

void GMT_vlamb (struct GMT_CTRL *C, double rlong0, double rlat0, double pha, double phb)
{	/* Set up a Lambert Conformal Conic projection (Spherical when e = 0) */

	double sin_pha, cos_pha, sin_phb, cos_phb, t_pha, m_pha, t_phb, m_phb, t_rlat0;

	gmt_check_R_J (C, &rlong0);
	C->current.proj.north_pole = (C->common.R.wesn[YHI] > 0.0 && (C->common.R.wesn[YLO] >= 0.0 || (-C->common.R.wesn[YLO]) < C->common.R.wesn[YHI]));
	C->current.proj.pole = (C->current.proj.north_pole) ? 90.0 : -90.0;
	sincosd (pha, &sin_pha, &cos_pha);
	sincosd (phb, &sin_phb, &cos_phb);
	
	t_pha = tand (45.0 - 0.5 * pha) / pow ((1.0 - C->current.proj.ECC *
		sin_pha) / (1.0 + C->current.proj.ECC * sin_pha), C->current.proj.half_ECC);
	m_pha = cos_pha / d_sqrt (1.0 - C->current.proj.ECC2 * sin_pha * sin_pha);
	t_phb = tand (45.0 - 0.5 * phb) / pow ((1.0 - C->current.proj.ECC *
		sin_phb) / (1.0 + C->current.proj.ECC * sin_phb), C->current.proj.half_ECC);
	m_phb = cos_phb / d_sqrt (1.0 - C->current.proj.ECC2 * sin_phb * sin_phb);
	t_rlat0 = tand (45.0 - 0.5 * rlat0) /
		pow ((1.0 - C->current.proj.ECC * sind (rlat0)) /
		(1.0 + C->current.proj.ECC * sind (rlat0)), C->current.proj.half_ECC);

	if (doubleAlmostEqualZero (pha, phb))
		C->current.proj.l_N = sin (pha);
	else
		C->current.proj.l_N = (d_log (C, m_pha) - d_log (C, m_phb))/(d_log (C, t_pha) - d_log (C, t_phb));

	C->current.proj.l_i_N = 1.0 / C->current.proj.l_N;
	C->current.proj.l_F = m_pha / (C->current.proj.l_N * pow (t_pha, C->current.proj.l_N));
	C->current.proj.central_meridian = rlong0;
	C->current.proj.l_rF = C->current.proj.EQ_RAD * C->current.proj.l_F;
	C->current.proj.l_i_rF = 1.0 / C->current.proj.l_rF;
	C->current.proj.l_rho0 = C->current.proj.l_rF * pow (t_rlat0, C->current.proj.l_N);
	C->current.proj.l_Nr = C->current.proj.l_N * D2R;
	C->current.proj.l_i_Nr = 1.0 / C->current.proj.l_Nr;
}

void GMT_lamb (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{
	double rho, theta, hold1, hold2, hold3, es, s, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	es = C->current.proj.ECC * sind (lat);
	hold2 = pow ((1.0 - es) / (1.0 + es), C->current.proj.half_ECC);
	hold3 = tand (45.0 - 0.5 * lat);
	hold1 = pow (hold3 / hold2, C->current.proj.l_N);
	rho = C->current.proj.l_rF * hold1;
	theta = C->current.proj.l_Nr * lon;
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = C->current.proj.l_rho0 - rho * c;
}

void GMT_ilamb (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{
	GMT_LONG i;
	double theta, rho, t, tphi, phi, dy, r;

	dy = C->current.proj.l_rho0 - y;
	theta = (C->current.proj.l_N < 0.0) ? d_atan2 (-x, -dy) : d_atan2 (x, dy);
	*lon = theta * C->current.proj.l_i_Nr + C->current.proj.central_meridian;

	rho = copysign (hypot (x, dy), C->current.proj.l_N);
	t = pow (rho * C->current.proj.l_i_rF, C->current.proj.l_i_N);
	phi = 0.0; tphi = 999.0;	/* Initialize phi = 0 */
	for (i = 0; i < 100 && fabs (tphi - phi) > GMT_CONV_LIMIT; i++) {
		tphi = phi;
		r = C->current.proj.ECC * sin (phi);
		phi = M_PI_2 - 2.0 * atan (t * pow ((1.0 - r) / (1.0 + r), C->current.proj.half_ECC));
	}
	*lat = phi * R2D;
}

/* Spherical cases of Lambert */

void GMT_lamb_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{
	double rho, theta, t, s, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);

	t = MAX (0.0, tand (45.0 - 0.5 * lat));	/* Guard against negative t */
	rho = C->current.proj.l_rF * pow (t, C->current.proj.l_N);
	theta = C->current.proj.l_Nr * lon;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = C->current.proj.l_rho0 - rho * c;
}

void GMT_ilamb_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{
	double theta, rho, t, dy;

	dy = C->current.proj.l_rho0 - y;
	theta = (C->current.proj.l_N < 0.0) ? d_atan2 (-x, -dy) : d_atan2 (x, dy);
	*lon = theta * C->current.proj.l_i_Nr + C->current.proj.central_meridian;

	rho = copysign (hypot (x, dy), C->current.proj.l_N);
	t = pow (rho * C->current.proj.l_i_rF, C->current.proj.l_i_N);
	*lat = 90.0 - 2.0 * atand (t);
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_latc_to_latg (C, *lat);
}

/* -JO OBLIQUE MERCATOR PROJECTION */

void GMT_oblmrc (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert a longitude/latitude point to Oblique Mercator coordinates
	 * by way of rotation coordinates and then using regular Mercator */
	double tlon, tlat;

	GMT_obl (C, lon * D2R, lat * D2R, &tlon, &tlat);

	*x = C->current.proj.j_x * tlon;
	*y = (fabs (tlat) < M_PI_2) ? C->current.proj.j_x * d_log (C, tan (M_PI_4 + 0.5 * tlat)) : copysign (DBL_MAX, tlat);
}

void GMT_ioblmrc (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert a longitude/latitude point from Oblique Mercator coordinates
	 * by way of regular Mercator and then rotate coordinates */
	double tlon, tlat;

	tlon = x * C->current.proj.j_ix;
	tlat = atan (sinh (y * C->current.proj.j_ix));

	GMT_iobl (C, lon, lat, tlon, tlat);

	(*lon) *= R2D;	(*lat) *= R2D;
}

void GMT_obl (struct GMT_CTRL *C, double lon, double lat, double *olon, double *olat)
{	/* Convert a longitude/latitude point to Oblique lon/lat (all in rads) */
	double p_cross_x[3], X[3];

	GMT_geo_to_cart (C, lat, lon, X, FALSE);

	*olat = d_asin (GMT_dot3v (C, X, C->current.proj.o_FP));

	GMT_cross3v (C, C->current.proj.o_FP, X, p_cross_x);
	GMT_normalize3v (C, p_cross_x);

	*olon = copysign (d_acos (GMT_dot3v (C, p_cross_x, C->current.proj.o_FC)), GMT_dot3v (C, X, C->current.proj.o_FC));
}

void GMT_iobl (struct GMT_CTRL *C, double *lon, double *lat, double olon, double olat)
{	/* Convert a longitude/latitude point from Oblique lon/lat  (all in rads) */
	double p_cross_x[3], X[3];

	GMT_geo_to_cart (C, olat, olon, X, FALSE);
	*lat = d_asin (GMT_dot3v (C, X, C->current.proj.o_IP));

	GMT_cross3v (C, C->current.proj.o_IP, X, p_cross_x);
	GMT_normalize3v (C, p_cross_x);

	*lon = copysign (d_acos (GMT_dot3v (C, p_cross_x, C->current.proj.o_IC)), GMT_dot3v (C, X, C->current.proj.o_IC));

	while ((*lon) < 0.0) (*lon) += TWO_PI;
	while ((*lon) >= TWO_PI) (*lon) -= TWO_PI;
}

/* -JT TRANSVERSE MERCATOR PROJECTION */

void GMT_vtm (struct GMT_CTRL *C, double lon0, double lat0)
{	/* Set up an TM projection */
	double e1, s2, c2;

	/* gmt_check_R_J (&lon0); */
	e1 = (1.0 - d_sqrt (C->current.proj.one_m_ECC2)) / (1.0 + d_sqrt (C->current.proj.one_m_ECC2));
	C->current.proj.t_e2 = C->current.proj.ECC2 * C->current.proj.i_one_m_ECC2;
	C->current.proj.t_c1 = 1.0 - (1.0/4.0) * C->current.proj.ECC2 - (3.0/64.0) * C->current.proj.ECC4 - (5.0/256.0) * C->current.proj.ECC6;
	C->current.proj.t_c2 = -((3.0/8.0) * C->current.proj.ECC2 + (3.0/32.0) * C->current.proj.ECC4 + (25.0/768.0) * C->current.proj.ECC6);
	C->current.proj.t_c3 = (15.0/128.0) * C->current.proj.ECC4 + (45.0/512.0) * C->current.proj.ECC6;
	C->current.proj.t_c4 = -(35.0/768.0) * C->current.proj.ECC6;
	C->current.proj.t_i1 = 1.0 / (C->current.proj.EQ_RAD * C->current.proj.t_c1);
	C->current.proj.t_i2 = (3.0/2.0) * e1 - (29.0/12.0) * pow (e1, 3.0);
	C->current.proj.t_i3 = (21.0/8.0) * e1 * e1 - (1537.0/128.0) * pow (e1, 4.0);
	C->current.proj.t_i4 = (151.0/24.0) * pow (e1, 3.0);
	C->current.proj.t_i5 = (1097.0/64.0) * pow (e1, 4.0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.t_lat0 = lat0 * D2R;	/* In radians */
	sincos (2.0 * C->current.proj.t_lat0, &s2, &c2);
	C->current.proj.t_M0 = C->current.proj.EQ_RAD * (C->current.proj.t_c1 * C->current.proj.t_lat0 + s2 * (C->current.proj.t_c2 + c2 * (C->current.proj.t_c3 + c2 * C->current.proj.t_c4)));
	C->current.proj.t_r = C->current.proj.EQ_RAD * C->current.setting.proj_scale_factor;
	C->current.proj.t_ir = 1.0 / C->current.proj.t_r;
}

/* Ellipsoidal TM functions */

void GMT_tm (struct GMT_CTRL *P, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to TM x/y */
	double N, T, T2, C, A, M, dlon, tan_lat, A2, A3, A5, s, c, s2, c2;

	if (doubleAlmostEqual (fabs (lat), 90.0)) {
		M = P->current.proj.EQ_RAD * P->current.proj.t_c1 * M_PI_2;
		*x = 0.0;
		*y = P->current.setting.proj_scale_factor * M;
	}
	else {
		lat *= D2R;
		sincos (lat, &s, &c);
		sincos (2.0 * lat, &s2, &c2);
		tan_lat = s / c;
		M = P->current.proj.EQ_RAD * (P->current.proj.t_c1 * lat + s2 * (P->current.proj.t_c2 + c2 * (P->current.proj.t_c3 + c2 * P->current.proj.t_c4)));
		dlon = lon - P->current.proj.central_meridian;
		if (fabs (dlon) > 360.0) dlon += copysign (360.0, -dlon);
		if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
		N = P->current.proj.EQ_RAD / d_sqrt (1.0 - P->current.proj.ECC2 * s * s);
		T = tan_lat * tan_lat;
		T2 = T * T;
		C = P->current.proj.t_e2 * c * c;
		A = dlon * D2R * c;
		A2 = A * A;	A3 = A2 * A;	A5 = A3 * A2;
		*x = P->current.setting.proj_scale_factor * N * (A + (1.0 - T + C) * (A3 * 0.16666666666666666667)
			+ (5.0 - 18.0 * T + T2 + 72.0 * C - 58.0 * P->current.proj.t_e2) * (A5 * 0.00833333333333333333));
		A3 *= A;	A5 *= A;
		*y = P->current.setting.proj_scale_factor * (M - P->current.proj.t_M0 + N * tan_lat * (0.5 * A2 + (5.0 - T + 9.0 * C + 4.0 * C * C) * (A3 * 0.04166666666666666667)
			+ (61.0 - 58.0 * T + T2 + 600.0 * C - 330.0 * P->current.proj.t_e2) * (A5 * 0.00138888888888888889)));
	}
}

void GMT_itm (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert TM x/y to lon/lat */
	double M, mu, u2, s, c, phi1, C1, C12, T1, T12, tmp, tmp2, N1, R_1, D, D2, D3, D5, tan_phi1, cp2;

	M = y / C->current.setting.proj_scale_factor + C->current.proj.t_M0;
	mu = M * C->current.proj.t_i1;

	u2 = 2.0 * mu;
	sincos (u2, &s, &c);
	phi1 = mu + s * (C->current.proj.t_i2 + c * (C->current.proj.t_i3 + c * (C->current.proj.t_i4 + c * C->current.proj.t_i5)));

	sincos (phi1, &s, &c);
	tan_phi1 = s / c;
	cp2 = c * c;
	C1 = C->current.proj.t_e2 * cp2;
	C12 = C1 * C1;
	T1 = tan_phi1 * tan_phi1;
	T12 = T1 * T1;
	tmp = 1.0 - C->current.proj.ECC2 * (1.0 - cp2);
	tmp2 = d_sqrt (tmp);
	N1 = C->current.proj.EQ_RAD / tmp2;
	R_1 = C->current.proj.EQ_RAD * C->current.proj.one_m_ECC2 / (tmp * tmp2);
	D = x / (N1 * C->current.setting.proj_scale_factor);
	D2 = D * D;	D3 = D2 * D;	D5 = D3 * D2;

	*lon = C->current.proj.central_meridian + R2D * (D - (1.0 + 2.0 * T1 + C1) * (D3 * 0.16666666666666666667)
		+ (5.0 - 2.0 * C1 + 28.0 * T1 - 3.0 * C12 + 8.0 * C->current.proj.t_e2 + 24.0 * T12)
		* (D5 * 0.00833333333333333333)) / c;
	D3 *= D;	D5 *= D;
	*lat = phi1 - (N1 * tan_phi1 / R_1) * (0.5 * D2 -
		(5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C12 - 9.0 * C->current.proj.t_e2) * (D3 * 0.04166666666666666667)
		+ (61.0 + 90.0 * T1 + 298 * C1 + 45.0 * T12 - 252.0 * C->current.proj.t_e2 - 3.0 * C12) * (D5 * 0.00138888888888888889));
	(*lat) *= R2D;
}

/*Spherical TM functions */

void GMT_tm_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to TM x/y by spherical formula */
	double dlon, b, clat, slat, clon, slon, xx, yy;

	dlon = lon - C->current.proj.central_meridian;
	if (fabs (dlon) > 360.0) dlon += copysign (360.0, -dlon);
	if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);

	if (fabs (lat) > 90.0) {
		/* Invalid latitude.  Treat as in GMT_merc_sph(), but transversely:  */
		*x = copysign (1.0e100, dlon);
		*y = 0.0;
		return;
	}

	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_latc (C, lat);

	sincosd (lat, &slat, &clat);
	sincosd (dlon, &slon, &clon);
	b = clat * slon;
	if (fabs(b) >= 1.0) {
		/* This corresponds to the transverse "pole"; the point at x = +-infinity, y = -lat0.
			Treat as in GMT_merc_sph(), but transversely:  */
		*x = copysign (1.0e100, dlon);
		*y = -C->current.proj.t_r * C->current.proj.t_lat0;
		return;
	}

	xx = atanh (b);

	/* this should get us "over the pole";
	   see not Snyder's formula but his example Fig. 10 on p. 50:  */

	yy = atan2 (slat, (clat * clon)) - C->current.proj.t_lat0;
	if (yy < -M_PI_2) yy += TWO_PI;
	*x = C->current.proj.t_r * xx;
	*y = C->current.proj.t_r * yy;
}

void GMT_itm_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert TM x/y to lon/lat by spherical approximation.  */

	double xx, yy, sinhxx, coshxx, sind, cosd, lambda, phi;

	xx = x * C->current.proj.t_ir;
	yy = y * C->current.proj.t_ir + C->current.proj.t_lat0;

	sinhxx = sinh (xx);
	coshxx = cosh (xx);

	sincos (yy, &sind, &cosd);
	phi = asind (sind / coshxx);
	*lat = phi;

	lambda = atan2d (sinhxx, cosd);
	*lon = lambda + C->current.proj.central_meridian;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_latc_to_latg (C, *lat);
}

/* -JU UNIVERSAL TRANSVERSE MERCATOR PROJECTION */

/* Ellipsoidal UTM */

void GMT_utm (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm (C, lon, lat, x, y);
	(*x) += GMT_FALSE_EASTING;
	if (!C->current.proj.north_pole) (*y) += GMT_FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert UTM x/y to lon/lat */

	x -= GMT_FALSE_EASTING;
	if (!C->current.proj.north_pole) y -= GMT_FALSE_NORTHING;
	GMT_itm (C, lon, lat, x, y);
}

/* Spherical UTM */

void GMT_utm_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm_sph (C, lon, lat, x, y);
	(*x) += GMT_FALSE_EASTING;
	if (!C->current.proj.north_pole) (*y) += GMT_FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert UTM x/y to lon/lat */

	x -= GMT_FALSE_EASTING;
	if (!C->current.proj.north_pole) y -= GMT_FALSE_NORTHING;
	GMT_itm_sph (C, lon, lat, x, y);
}

/* -JA LAMBERT AZIMUTHAL EQUAL AREA PROJECTION */

void GMT_vlambeq (struct GMT_CTRL *C, double lon0, double lat0, double horizon)
{	/* Set up Spherical Lambert Azimuthal Equal-Area projection */

	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;
	if (C->current.proj.GMT_convert_latitudes) lat0 = GMT_latg_to_lata (C, lat0);
	sincosd (lat0, &(C->current.proj.sinp), &(C->current.proj.cosp));
	C->current.proj.f_horizon = horizon;
	C->current.proj.rho_max = 2.0 * sind (0.5 * horizon) * C->current.proj.EQ_RAD;
}

void GMT_lambeq (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Spherical Lambert Azimuthal Equal-Area x/y */
	double k, tmp, sin_lat, cos_lat, sin_lon, cos_lon, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);
	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);
	c = cos_lat * cos_lon;
	tmp = 1.0 + C->current.proj.sinp * sin_lat + C->current.proj.cosp * c;
	if (tmp > 0.0) {
		k = C->current.proj.EQ_RAD * d_sqrt (2.0 / tmp);
		*x = k * cos_lat * sin_lon;
		*y = k * (C->current.proj.cosp * sin_lat - C->current.proj.sinp * c);
		if (C->current.proj.GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= C->current.proj.Dx;
			(*y) *= C->current.proj.Dy;
		}
	}
	else
		*x = *y = -DBL_MAX;
}

void GMT_ilambeq (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Lambert Azimuthal Equal-Area x/y to lon/lat */
	double rho, a, sin_c, cos_c;

	if (C->current.proj.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= C->current.proj.iDx;
		y *= C->current.proj.iDy;
	}

	rho = hypot (x, y);
	if (rho > C->current.proj.rho_max) {			/* Horizon		*/
		*lat = *lon = C->session.d_NaN;
		return;
	}
	a = 0.5 * rho * C->current.proj.i_EQ_RAD;			/* a = sin(c/2)		*/
	a *= a;							/* a = sin(c/2)**2	*/
	cos_c = 1.0 - 2.0 * a;					/* cos_c = cos(c)	*/
	sin_c = sqrt (1.0 - a) * C->current.proj.i_EQ_RAD;		/* sin_c = sin(c)/rho	*/
	*lat = d_asind (cos_c * C->current.proj.sinp + y * sin_c * C->current.proj.cosp);
	*lon = d_atan2d (x * sin_c, C->current.proj.cosp * cos_c - y * C->current.proj.sinp * sin_c) + C->current.proj.central_meridian;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

/* -JG GENERAL PERSPECTIVE PROJECTION */

/* Set up General Perspective projection */

void gmt_genper_to_xtyt (struct GMT_CTRL *C, double angle, double x, double y, double offset, double *xt, double *yt)
{
	double A, theta, xp, yp;

	theta = C->current.proj.g_azimuth - angle;

	A = (y * C->current.proj.g_cos_azimuth + x * C->current.proj.g_sin_azimuth) * C->current.proj.g_sin_tilt / C->current.proj.g_H + C->current.proj.g_cos_tilt;

	if (A > 0.0) {
		xp = (x * C->current.proj.g_cos_azimuth - y * C->current.proj.g_sin_azimuth) * C->current.proj.g_cos_tilt / A;
		yp = (y * C->current.proj.g_cos_azimuth + x * C->current.proj.g_sin_azimuth) / A;
		if (fabs(yp) > fabs(C->current.proj.g_max_yt)) {
			yp = -C->current.proj.g_max_yt;
			xp = -yp * tand(theta);
		}
	}
	else {
		yp = -C->current.proj.g_max_yt;
		xp = -yp * tand(theta);
	}

	yp -= offset;

	*xt = xp * C->current.proj.g_cos_twist - yp * C->current.proj.g_sin_twist;
	*yt = yp * C->current.proj.g_cos_twist + xp * C->current.proj.g_sin_twist;

	return;
}

/* conversion from geodetic latitude to geocentric latitude */

double gmt_genper_getgeocentric (struct GMT_CTRL *C, double phi, double h)
{
	double phig, sphi, cphi, N1;

	sincosd (phi, &sphi, &cphi);

	N1 = C->current.proj.EQ_RAD/sqrt(1.0 - (C->current.proj.ECC2*sphi*sphi));

	phig = phi - asind(N1*C->current.proj.ECC2*sphi*cphi/((h/C->current.proj.EQ_RAD+1.0)*C->current.proj.EQ_RAD));

	return (phig);
}

void gmt_genper_toxy (struct GMT_CTRL *P, double lat, double lon, double h, double *x, double *y)
{
	double angle;
	double xp, yp, rp;
	double N, C, S, K;
	double sphi, cphi;
	double sdphi, cdphi;
	double sphi1, cphi1;
	double sdlon, cdlon;

	cdphi = P->current.proj.g_cdphi;
	sdphi = P->current.proj.g_sdphi;

	cphi1 = P->current.proj.g_cphi1;
	sphi1 = P->current.proj.g_sphi1;
	h *= 1e3;

	sincosd (lat, &sphi, &cphi);

	N = P->current.proj.g_R/sqrt(1.0 - (P->current.proj.ECC2*sphi*sphi));
	C = ((N+h)/P->current.proj.g_R)*cphi;
	S = ((N*P->current.proj.one_m_ECC2 + h)/P->current.proj.g_R)*sphi;

	sincosd (lon - P->current.proj.g_lon0, &sdlon, &cdlon);

	K = P->current.proj.g_H / (P->current.proj.g_P*cdphi - S*sphi1 - C*cphi1*cdlon);

	xp = K*C*sdlon;
	yp = K*(P->current.proj.g_P*sdphi + S*cphi1 - C*sphi1*cdlon);

	rp = sqrt(xp*xp + yp*yp);

	if (rp > P->current.proj.g_rmax) {
		angle = atan2(xp, yp);
		sincos (angle, &xp, &yp);
		xp *= P->current.proj.g_rmax;
		yp *= P->current.proj.g_rmax;
		angle *= R2D;
	}
	*x = xp;
	*y = yp;

	if (P->current.proj.g_debug > 1) {
		GMT_message (P, "\n");
		GMT_message (P, "lat  %12.3f\n", lat);
		GMT_message (P, "lon  %12.3f\n", lon);
		GMT_message (P, "h    %12.3f\n", h);
		GMT_message (P, "N    %12.1f\n", N);
		GMT_message (P, "C    %12.7f\n", C);
		GMT_message (P, "S    %12.7f\n", S);
		GMT_message (P, "K    %12.1f\n", K);
		GMT_message (P, "x    %12.1f\n", *x);
		GMT_message (P, "y    %12.1f\n", *y);
	}
}

void gmt_genper_tolatlong (struct GMT_CTRL *C, double x, double y, double h, double *lat, double *lon)
{
	double P, H, B, D;
	double u, v, t, Kp, X, Y;
	double E, S;
	double phi_last;
	double Kp2;
	double phi, sphi;
	double cphig;
	double e2, R, one_m_e2;
	double cphi1, sphi1;

	GMT_LONG niter;
	GMT_LONG set_exit = 0;

	h *= 1e3;

	H = C->current.proj.g_H;
	P = C->current.proj.g_P;
	R = C->current.proj.g_R;

	one_m_e2 = C->current.proj.one_m_ECC2;
	e2 = C->current.proj.ECC2;

	cphig = C->current.proj.g_cphig;
	cphi1 = C->current.proj.g_cphi1;
	sphi1 = C->current.proj.g_sphi1;

	B = C->current.proj.g_B;
	D = C->current.proj.g_D;

	u = C->current.proj.g_BLH - C->current.proj.g_DG*y + C->current.proj.g_BJ*y + C->current.proj.g_DHJ;
	v = C->current.proj.g_LH2 + C->current.proj.g_G*y*y - C->current.proj.g_HJ*y + one_m_e2*x*x;

	if (C->current.proj.g_debug > 1) {
		GMT_message (C, "\n");
		GMT_message (C, "gmt_genper_tolatlong - 1 \n");
		GMT_message (C, "x    %12.1f\n", x);
		GMT_message (C, "y    %12.1f\n", y);
		GMT_message (C, "\n");
		GMT_message (C, "P    %12.7f\n", P);
		GMT_message (C, "phig %12.7f\n", C->current.proj.g_phig);
		GMT_message (C, "\n");
		GMT_message (C, "B    %12.7f\n", B);
		GMT_message (C, "D    %12.7f\n", D);
		GMT_message (C, "u    %12.1f\n", u);
		GMT_message (C, "v    %12.6e\n", v);
	}
	E = 1;

	t = P*P*(1.0 - e2*cphig*cphig) - E*one_m_e2;
	Kp2 = (1.0 - 4.0*(t/u)*(v/u));
	if (Kp2 < 0.0)
		Kp = -u/(2.0*t);
	else
		Kp = (-u + sqrt(u*u-4.0*t*v))/(2.0*t);
	X = R*((B-H/Kp)*cphi1 - (y/Kp - D)*sphi1);
	Y = R*x/Kp;
	S = (y/Kp-D)*cphi1 + (B-H/Kp)*sphi1;

	if (GMT_is_dnan(Kp) || GMT_is_dnan(X) || GMT_is_dnan(Y) || GMT_is_dnan(S)) set_exit++;

	if (set_exit == 1) {
		GMT_message (C, "\n");
		GMT_message (C, "gmt_genper_tolatlong - 2\n");
		GMT_message (C, "x    %12.1f\n", x);
		GMT_message (C, "y    %12.1f\n", y);
		GMT_message (C, "\n");
		GMT_message (C, "P    %12.7f\n", P);
		GMT_message (C, "phig %12.7f\n", C->current.proj.g_phig);
		GMT_message (C, "\n");
		GMT_message (C, "B    %12.7f\n", B);
		GMT_message (C, "D    %12.7f\n", D);
		GMT_message (C, "u    %12.1f\n", u);
		GMT_message (C, "v    %12.6e\n", v);
	}
	if (set_exit || C->current.proj.g_debug > 1) {
		GMT_message (C, "t    %12.7f\n", t);
		GMT_message (C, "Kp   %12.1f\n", Kp);
		GMT_message (C, "Kp2  %12.1f\n", Kp2);
		GMT_message (C, "X    %12.1f\n", X);
		GMT_message (C, "Y    %12.1f\n", Y);
		GMT_message (C, "S    %12.7f\n", S);
	}

	if (h == 0) {
		phi = atan(S/sqrt(one_m_e2*(1.0 - e2 - S*S)));
		/* if (GMT_is_dnan(phi)) set_exit++; */
	}
	else {
		double t1, t2;
		niter = 0;
		t2 = h*h/(R*R*one_m_e2);

		sphi = S/(one_m_e2/sqrt(1.0 - e2*S*S) + h/R);
		phi = asin(sphi);

		t1 = (1.0/sqrt(1.0 - e2*sphi*sphi) + h/R);
		E = t1 * t1 - e2*sphi*sphi*(1.0/(1.0 - e2*sphi*sphi) - t2);

		if (GMT_is_dnan(E)) set_exit++;

		if (set_exit == 1) GMT_message (C, "gmt_genper_tolatlong - 3\n");
		if (C->current.proj.g_debug > 1 || set_exit) {
			GMT_message (C, "asinS %12.7f\n", asind(S));
			GMT_message (C, "phi   %12.7f\n", R2D*phi);
			GMT_message (C, "E     %12.7f\n", E);
		}

		do {
			niter++;
			phi_last = phi;
			t = P*P*(1.0 - e2*cphig*cphig) - E*one_m_e2;
			Kp2 = (1.0 - 4.0*(t/u)*(v/u));
			if (Kp2 < 0.0)
				Kp = -u/(2.0*t);
			else
				Kp = (-u + sqrt(u*u-4.0*t*v))/(2.0*t);
			X = R*((B-H/Kp)*cphi1 - (y/Kp - D)*sphi1);
			Y = R*x/Kp;
			S = (y/Kp-D)*cphi1 + (B-H/Kp)*sphi1;
			sphi = S/(one_m_e2/sqrt(1.0 - e2*sphi*sphi) + h/R);
			phi = asin(sphi);
			t1 = (1.0/sqrt(1.0 - e2*sphi*sphi) + h/R);
			E = t1 * t1 - e2*sphi*sphi*(1.0/(1.0 - e2*sphi*sphi) - t2);

			if (GMT_is_dnan(Kp) || GMT_is_dnan(X) || GMT_is_dnan(Y) || GMT_is_dnan(S) || GMT_is_dnan(phi) || GMT_is_dnan(E)) set_exit++;
			if (set_exit == 1) GMT_message (C, "gmt_genper_tolatlong - 4 \n");
			if (set_exit || C->current.proj.g_debug > 1) {
				GMT_message (C, "\niter %ld\n", niter);
				GMT_message (C, "t    %12.7f\n", t);
				GMT_message (C, "Kp   %12.1f\n", Kp);
				GMT_message (C, "X    %12.1f\n", X);
				GMT_message (C, "Y    %12.1f\n", Y);
				GMT_message (C, "S    %12.7f\n", S);
				GMT_message (C, "phi  %12.7f\n", phi*R2D);
				GMT_message (C, "E    %12.7f\n", E);
			}
		}
		while (fabs(phi - phi_last) > 1e-7);
	}
	if (set_exit == 1) GMT_message (C, "gmt_genper_tolatlong - 5\n");
	if (set_exit || C->current.proj.g_debug > 1) {
		GMT_message (C, "phi    %12.7f\n", phi*R2D);
		GMT_exit(1);
	}
	*lat = phi * R2D;
	*lon = atan2d (Y, X) + C->current.proj.g_lon0;
	return;
}

void gmt_genper_setup (struct GMT_CTRL *C, double h0, double altitude, double lat, double lon0)
{
/* if ellipsoid lat0 is geodetic latitude and must convert to geocentric latitude */

	double N1, phig_last;
	double R, H, P;
	double sphi1, cphi1, sphig, cphig;
	double a, e2, phig;

	GMT_LONG niter;

	a = C->current.proj.EQ_RAD;
	e2 = C->current.proj.ECC2;

	h0 *= 1e3;

	sincosd (lat, &sphi1, &cphi1);
	sphig = sphi1; cphig = cphi1;

	N1 = a / sqrt (1.0 - (e2*sphi1*sphi1));
	niter = 0;

	if (C->current.proj.g_radius || altitude < -10.0) {
		/* use altitude as the radial distance from the center of the earth */
		H = fabs (altitude*1e3) - a;
		P = H/a + 1.0;
		phig = lat;
	}
	else if (altitude <= 0.0) {
		/* setup altitude of geosynchronous viewpoint n */
		double temp = 86164.1/TWO_PI;
		H = pow (3.98603e14*temp*temp, 0.3333) - a;
		P = H/a + 1.0;
		phig = lat - asind(N1*e2*sphi1*cphi1/(P*a));
		sincosd (phig, &sphig, &cphig);
		if (cphi1 != 0.0)
			H = P*a*cphig/cphi1 - N1 - h0;
		else
			H = P*a - N1 - h0;
	}
	else if (altitude < 10.0) {
		P = altitude;
		/* need to setup H from P equation */
		phig = lat - asind(N1*e2*sphi1*cphi1/(P*a));
		sincosd (phig, &sphig, &cphig);
		if (cphi1 != 0.0)
			H = P*a*cphig/cphi1 - N1 - h0;
		else
			H = P*a - N1 - h0;
	}
	else {
		/* GMT_message (C, "altitude %f\n", altitude); */
		H = altitude*1e3;
		/* need to setup P from iterating phig */
		phig = lat;
		do {
			niter++;
			sincosd (phig, &sphig, &cphig);
			P = (cphi1/cphig) * (H + N1 + h0)/a;
			phig_last = phig;
			phig = lat - asind(N1*e2*sphi1*cphi1/(P*a));
			/* GMT_message (C, "%2d P %12.7f phig %12.7f\n", niter, P, phig); */
		}
		while (fabs (phig - phig_last) > 1e-9);
		sincosd (phig, &sphig, &cphig);
		P = (cphi1/cphig)*(H + N1 + h0)/a;
	}

	/* R = H/(P-1.0); */
	R = a;
	/* XXX Which one is it ? */

	C->current.proj.g_H = H;
	C->current.proj.g_P = P;
	C->current.proj.g_R = R;
	C->current.proj.g_lon0 = lon0;
	C->current.proj.g_sphi1 = sphi1;
	C->current.proj.g_cphi1 = cphi1;

	C->current.proj.g_phig = phig;
	C->current.proj.g_sphig = sphig;
	C->current.proj.g_cphig = cphig;

	sincosd (lat-phig, &(C->current.proj.g_sdphi), &(C->current.proj.g_cdphi));

	C->current.proj.g_L = 1.0 - e2*cphi1*cphi1;
	C->current.proj.g_G = 1.0 - e2*sphi1*sphi1;
	C->current.proj.g_J = 2.0*e2*sphi1*cphi1;
	C->current.proj.g_B = P*C->current.proj.g_cdphi;
	C->current.proj.g_D = P*C->current.proj.g_sdphi;
	C->current.proj.g_BLH = -2.0*C->current.proj.g_B*C->current.proj.g_L*H;
	C->current.proj.g_DG = 2.0*C->current.proj.g_D*C->current.proj.g_G;
	C->current.proj.g_BJ = C->current.proj.g_B*C->current.proj.g_J;
	C->current.proj.g_HJ = H*C->current.proj.g_J;
	C->current.proj.g_DHJ = C->current.proj.g_D*C->current.proj.g_HJ;
	C->current.proj.g_LH2 = C->current.proj.g_L*H*H;

	if (C->current.proj.g_debug > 0) {
		GMT_message (C, "a    %12.4f\n", a);
		GMT_message (C, "R    %12.4f\n", R);
		GMT_message (C, "e^2  %12.7f\n", e2);
		GMT_message (C, "H    %12.4f\n", H);
		GMT_message (C, "phi1 %12.4f\n", lat);
		GMT_message (C, "lon0 %12.4f\n", lon0);
		GMT_message (C, "h0   %12.4f\n", h0);
		GMT_message (C, "N1   %12.1f\n", N1);
		GMT_message (C, "P    %12.7f\n", P);
		GMT_message (C, "phig %12.7f\n", phig);
	}

	return;
}

void GMT_vgenper (struct GMT_CTRL *C, double lon0, double lat0, double altitude, double azimuth, double tilt, double twist, double width, double height)
{
	double R, Req, Rpolar, Rlat0;
	double H, P, PP;
	double rho, rho2;
	double eca, cos_eca, sin_eca ;
	double yt_min, yt_max;
	double xt_min, xt_max;
	double rmax;
	double rmax_at_lat0, rmax_min, rmax_max;
	double t1, t2, t12, t22;
	double kp;
	double xt, yt;
	double x, y;
	double gamma, Omega, sinOmega, cosOmega, omega_max;
	double sinlatvp, coslatvp, latvp, lonvp;
	double xt_vp, yt_vp;
	double eccen;
	double max_yt;
	double sin_lat0, cos_lat0, sin_lat1, cos_lat1, dlong;
	double lat0_save;
	double vp_lat, vp_long;
	GMT_LONG az;

	C->current.proj.central_meridian = lon0;

	lat0_save = lat0;

	Req = R = C->current.proj.EQ_RAD;
	Rpolar = Req * sqrt(C->current.proj.one_m_ECC2);

	sincosd (lat0, &t2, &t1);

	t1 *= R;
	t12 = R*t1;
	t2 *= Rpolar;
	t22 = Rpolar*t2;

	Rlat0 = sqrt ((t12*t12 + t22*t22)/(t1*t1 + t2*t2));

	lat0 = gmt_genper_getgeocentric (C, lat0, 0.0);
	sincosd (lat0, &(C->current.proj.sinp), &(C->current.proj.cosp));

	if (!GMT_IS_ZERO (C->current.proj.ECC2)) {
		gmt_genper_setup (C, 0.0, altitude, lat0_save, lon0);
		C->current.proj.central_meridian = lon0;
		C->current.proj.pole = C->current.proj.g_phig;
	}
	else {
		C->current.proj.pole = lat0;

		if (C->current.proj.g_radius || (altitude < -10.0)) {
			/* use altitude as the radial distance from the center of the earth*/
			H = fabs(altitude*1e3) - R;
			P = H/R + 1.0;
		}
		else if (altitude <= 0.0) {
			/* compute altitude of geosynchronous viewpoint n*/
			double temp = 86164.1/TWO_PI;
			H = pow(3.98603e14*temp*temp, 0.3333) - R;
			P = H/R + 1.0;
		}
		else if (altitude < 10.0) {
			P = altitude;
			H = R * (P - 1.0);
		}
		else {
			H = altitude*1e3;
			P = H/R + 1.0;
		}
		C->current.proj.g_R = R;
		C->current.proj.g_H = H;
		C->current.proj.g_P = P;
	}

	H = C->current.proj.g_H;
	P = C->current.proj.g_P;
	R = C->current.proj.g_R;

	C->current.proj.g_P_inverse = P > 0.0 ? 1.0/P : 1.0;

	if (C->current.proj.g_longlat_set) {
		double norm_long = lon0;

		vp_lat = tilt;
		vp_long = azimuth;

		if (vp_lat == lat0_save && vp_long == lon0)
			tilt = azimuth = 0.0;
		else {
			if (C->current.proj.g_debug > 0) {
				GMT_message (C, " sensor point long %7.4f lat  %7.4f\n", lon0, lat0);
				GMT_message (C, " input view point long %7.4f lat %7.4f\n", vp_long, vp_lat);
				GMT_message (C, " input twist %7.4f\n", twist);
				GMT_message (C, " altitude %f H %f R %f P %7.4f\n", altitude, H/1000.0, R/1000.0,P);
			}

			sincosd (90.0 - lat0, &sin_lat0, &cos_lat0);
			sincosd (90.0 - vp_lat, &sin_lat1, &cos_lat1);

			while (vp_long < 0.0) vp_long += 360.0;
			while (norm_long < 0.0) norm_long += 360.0;

			dlong  = vp_long - norm_long;
			if (dlong < -GMT_180) dlong += 360.0;

			cos_eca = cos_lat0*cos_lat1 + sin_lat0*sin_lat1*cosd(dlong);
			eca = d_acos (cos_eca);
			sin_eca = sin (eca);

			rho2 = P*P + 1.0 - 2.0*P*cos_eca;
			rho = sqrt (rho2);

			tilt = d_acosd ((rho2 + P*P - 1.0)/(2.0*rho*P));

			azimuth = d_acosd ((cos_lat1 - cos_lat0*cos_eca)/(sin_lat0*sin_eca));

			if (dlong < 0) azimuth = 360.0 - azimuth;

		}
		if (C->current.proj.g_debug > 0) GMT_message (C, "vgenper: pointing at longitude %10.4f latitude %10.4f\n           with computed tilt %5.2f azimuth %6.2f\n", vp_long, vp_lat, tilt, azimuth);

	}
	else if (C->current.proj.g_debug > 1) {
		GMT_message (C, " sensor point long %6.3f lat  %6.3f\n", lon0, lat0);
		GMT_message (C, " input azimuth   %6.3f tilt %6.3f\n", azimuth, tilt);
		GMT_message (C, " input twist %6.3f\n", twist);
	}

	if (tilt < 0.0) tilt = d_asind (C->current.proj.g_P_inverse);

	sincosd (tilt, &(C->current.proj.g_sin_tilt), &(C->current.proj.g_cos_tilt));
	sincosd (twist, &(C->current.proj.g_sin_twist), &(C->current.proj.g_cos_twist));

	C->current.proj.g_box = !(fabs (width) < GMT_SMALL);

	if (width != 0.0 && height == 0) height = width;
	if (height != 0.0 && width == 0) width = height;
	C->current.proj.g_width = width/2.0;

	C->current.proj.g_azimuth = azimuth;
	sincosd (azimuth, &(C->current.proj.g_sin_azimuth), &(C->current.proj.g_cos_azimuth));

	PP = sqrt ((P - 1.0)/(P + 1.0));
	rmax = R*PP;
	rmax_min = Rpolar*PP;
	rmax_max = Req*PP;
	rmax_at_lat0 = Rlat0*PP;

	if (C->current.proj.ECC2 != 0.0) rmax = rmax_at_lat0;

	kp = R*(P - 1.0) / (P - C->current.proj.g_P_inverse);

	omega_max = d_acosd(C->current.proj.g_P_inverse);
	C->current.proj.g_rmax = rmax;

	/* C->current.proj.f_horizon = C->current.proj.g_P_inverse; */
	C->current.proj.f_horizon = omega_max;
	/* XXX Which one is it ? */

	max_yt = 2.0*R*sind (0.5*omega_max);

	eccen = sind (tilt)/sqrt (1.0 - 1.0/(P*P));

	gamma = 180.0 - d_asind (C->current.proj.g_sin_tilt * P);
	Omega = 180.0 - tilt - gamma;

	if (C->current.proj.g_debug > 0) GMT_message (C, "vgenper: tilt %6.3f sin_tilt %10.6f P %6.4f gamma %6.4f\n   Omega %6.4f eccen %10.4f\n", tilt, C->current.proj.g_sin_tilt, P, gamma, Omega, eccen);

	if (eccen == 1.0) {
		max_yt = MIN (max_yt, rmax * 2.0);
		if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: Projected map is a parabola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n Plot truncated for projected distances > rmax %8.2f\n", tilt, omega_max, rmax/1000.0);
	}
	else if (eccen > 1.0) {
		if (width != 0.0) {
			if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: Projected map is a hyperbola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n", tilt, omega_max);
		}
		else {
			max_yt = MIN (max_yt, rmax * 2.0);
			if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: Projected map is a hyperbola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n Plot truncated for projected distances > rmax %8.2f\n", tilt, omega_max, rmax/1000.0);
		}
	}
	else if (eccen > 0.5) {
		if (width != 0.0) {
			double t = sind (tilt), Pecc, maxecc = 0.5;
			Pecc = sqrt (1.0/(1.0 - (t*t/maxecc)));
			max_yt = R*sqrt ((Pecc-1.0)/(Pecc+1.0));
			if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: Projected map is an enlongated ellipse (eccentricity of %6.4f) with requested tilt %6.3f\nwill truncate plot at rmax %8.2f\n", eccen, tilt, max_yt);
		}
		else {
			if (max_yt > rmax *2.0) max_yt = rmax * 2.0;
			if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: Projected map is an enlongated ellipse with requested tilt %6.3f\n eccentricity %6.3f\n Plot truncated for projected distances > rmax %8.2f\n", tilt, eccen, rmax/1000.0);
		}
	}

	C->current.proj.g_max_yt = max_yt;

	sincosd (Omega, &sinOmega, &cosOmega);

	rho2 = P*P + 1.0 - 2.0*P*cosOmega;
	rho = sqrt (rho2);

	sinlatvp = C->current.proj.sinp*cosOmega + C->current.proj.cosp*sinOmega*C->current.proj.g_cos_azimuth;
	latvp = d_asind (sinlatvp);
	coslatvp = sqrt (1.0 - sinlatvp*sinlatvp);

	lonvp = acosd ((cosOmega - sinlatvp*C->current.proj.sinp)/(C->current.proj.cosp*coslatvp));
	if (azimuth > 180.0) lonvp = -lonvp;
	lonvp += lon0;

	if (C->current.proj.g_debug > 1) GMT_message (C, "vgenper: pointing at longitude %10.4f latitude %10.4f\n          with tilt %5.2f azimuth %6.2f at distance %6.4f\n         with width %6.3f height %6.3f twist %6.2f\n", lonvp, latvp, tilt, azimuth, rho, width, height, twist);

	C->current.proj.g_yoffset = 0.0;

	if (height != 0.0) {
		C->current.proj.g_yoffset = C->current.proj.g_sin_tilt * H ;
		xt_max = R * rho * sind (0.5*width);
		xt_min = -xt_max;
		yt_max = R * rho * sind (0.5*height);
		yt_min = -yt_max;
	}
	else {
		FILE *fp = NULL;
		xt_min = 1e20;
		xt_max = -xt_min;

		yt_min = 1e20;
		yt_max = -yt_min;

		if (C->current.proj.g_debug > 2) {
			fp = fopen("g_border.txt", "w");

			GMT_message (C, "tilt %10.4f sin_tilt %10.4f cos_tilt %10.4f\n", tilt, C->current.proj.g_sin_tilt, C->current.proj.g_cos_tilt);
			GMT_message (C, "azimuth %10.4f sin_azimuth %10.4f cos_azimuth %10.4f\n", azimuth, C->current.proj.g_sin_azimuth, C->current.proj.g_cos_azimuth);
		}

		for (az = 0 ; az < 360 ; az++) {
			sincosd ((double)az, &x, &y);
			x *= rmax;
			y *= rmax;
			gmt_genper_to_xtyt (C, (double)az, x, y, C->current.proj.g_yoffset, &xt, &yt);

			if (C->current.proj.g_debug > 2) fprintf (fp,"%3ld x %10.2f y %10.2f xt %10.3f yt %10.3f\n", az, x/1000, y/1000, xt/1000, yt/1000);

			xt_min = MIN (xt, xt_min);
			xt_max = MAX (xt, xt_max);
			yt_min = MIN (yt, yt_min);
			yt_max = MAX (yt, yt_max);
		}
		if (C->current.proj.g_debug > 2) fclose(fp);
		if (eccen > 0.5) {
			C->current.proj.g_width = atand (2.0*rmax/H);
			height = width = 2.0 * C->current.proj.g_width;
			xt_max = yt_max = R * rho * sind (C->current.proj.g_width);
			xt_min = yt_min = -xt_max;
		}
	}
	if (C->current.proj.g_debug > 1) {
		GMT_message (C, "vgenper: xt max %7.1f km\n", xt_max/1000.0);
		GMT_message (C, "vgenper: xt min %7.1f km\n", xt_min/1000.0);
		GMT_message (C, "vgenper: yt max %7.1f km\n", yt_max/1000.0);
		GMT_message (C, "vgenper: yt min %7.1f km\n", yt_min/1000.0);
	}

	C->current.proj.g_xmin = xt_min;
	C->current.proj.g_xmax = xt_max;
	C->current.proj.g_ymin = yt_min;
	C->current.proj.g_ymax = yt_max;

	if (width != 0.0) C->current.proj.scale[GMT_Y] = C->current.proj.scale[GMT_X]/width*height;

	if (C->current.proj.g_debug > 0) {
		GMT_genper (C, lonvp, latvp, &xt_vp, &yt_vp);
		GMT_message (C, "\nvgenper: polar %ld north %ld\n", C->current.proj.polar, C->current.proj.north_pole);
		GMT_message (C, "vgenper: altitude H %7.1f km P %7.4f\n", H/1000.0, P);
		GMT_message (C, "vgenper: azimuth %5.1f tilt %5.1f\n", azimuth, tilt);
		GMT_message (C, "vgenper: viewpoint width %5.1f height %5.1f degrees\n", width, height);
		GMT_message (C, "vgenper: radius max %7.1f km\n", C->current.proj.g_rmax/1000.0);
		GMT_message (C, "vgenper: eccentricity %7.4f km\n", eccen);
		GMT_message (C, "vgenper: eq radius max %7.1f km\n", rmax_max/1000.0);
		GMT_message (C, "vgenper: polar radius max %7.1f km\n", rmax_min/1000.0);
		GMT_message (C, "vgenper: lat0 radius max %7.1f km\n", rmax_at_lat0/1000.0);
		GMT_message (C, "vgenper: kp %7.1f \n", kp/1000.0);
		GMT_message (C, "vgenper: y offset %7.1f km\n", C->current.proj.g_yoffset/1000.0);
		GMT_message (C, "vgenper: yt max %7.1f km\n", yt_max/1000.0);
		GMT_message (C, "vgenper: yt min %7.1f km\n", yt_min/1000.0);
		GMT_message (C, "vgenper: y max %7.1f km\n", C->current.proj.g_ymax/1000.0);
		GMT_message (C, "vgenper: y min %7.1f km\n", C->current.proj.g_ymin/1000.0);
		GMT_message (C, "vgenper: x max %7.1f km\n", C->current.proj.g_xmax/1000.0);
		GMT_message (C, "vgenper: x min %7.1f km\n", C->current.proj.g_xmin/1000.0);
		GMT_message (C, "vgenper: omega max %6.2f degrees\n", omega_max);
		GMT_message (C, "vgenper: gamma %6.3f Omega %6.3f \n", gamma, Omega);
		GMT_message (C, "vgenper: viewpoint lon %6.3f lat %6.3f \n", lonvp, latvp);
		GMT_message (C, "vgenper: viewpoint xt %6.3f yt %6.3f \n", xt_vp/1000.0, yt_vp/1000.0);
		GMT_message (C, "vgenper: user viewpoint %ld\n", C->current.proj.g_box);
	}

}

/* Convert lon/lat to General Perspective x/y */

void GMT_genper (struct GMT_CTRL *C, double lon, double lat, double *xt, double *yt)
{
	double dlon, sin_lat, cos_lat, sin_dlon, cos_dlon;
	double cosc, sinc;
	double x, y, kp;
	double angle;

	dlon = lon - C->current.proj.central_meridian;
	while (dlon < -GMT_180) dlon += 360.0;
	while (dlon > 180.0) dlon -= 360.0;
	dlon *= D2R;

	lat = gmt_genper_getgeocentric (C, lat, 0.0);

	sincosd (lat, &sin_lat, &cos_lat);
	sincos (dlon, &sin_dlon, &cos_dlon);

	cosc = C->current.proj.sinp * sin_lat + C->current.proj.cosp * cos_lat * cos_dlon;
	sinc = d_sqrt(1.0 - cosc * cosc);

	C->current.proj.g_outside = FALSE;

	angle = M_PI - dlon;
	if (cosc < C->current.proj.g_P_inverse) { /* over the horizon */
		C->current.proj.g_outside = TRUE;

		if (C->current.proj.polar)
			angle = M_PI - dlon;
		else if (C->current.proj.cosp*sinc != 0.0) {
			angle = d_acos((sin_lat - C->current.proj.sinp*cosc)/(C->current.proj.cosp*sinc));
			if (dlon < 0.0) angle = -angle;
		}
		else
			angle = 0.0;

		sincos (angle, &x, &y);
		x *= C->current.proj.g_rmax;
		y *= C->current.proj.g_rmax;
		angle *= R2D;
	}
	else if (C->current.proj.ECC2 != 0.0) { /* within field of view, ellipsoidal earth */
		gmt_genper_toxy (C, lat, lon, 0.0, &x, &y);
		/* angle = C->current.proj.g_azimuth; */
		angle = atan2d(x, y);
		/* XXX Which one is it? Forgotten R2D. Switched x and y. */
	}
	else { /* within field of view, spherical earth */
		kp = C->current.proj.g_R * (C->current.proj.g_P - 1.0) / (C->current.proj.g_P - cosc);
		x = kp * cos_lat * sin_dlon;
		y = kp * (C->current.proj.cosp * sin_lat - C->current.proj.sinp * cos_lat * cos_dlon);
		/* angle = C->current.proj.g_azimuth; */
		angle = atan2d(x, y);
		/* XXX Which one is it? Forgotten R2D. Switched x and y. */
	}

	gmt_genper_to_xtyt (C, angle, x, y, C->current.proj.g_yoffset, xt, yt);

	if (GMT_is_dnan(*yt) || GMT_is_dnan(*xt)) {
		GMT_message (C, "genper: yt or xt nan\n");
		GMT_message (C, "genper: lon %6.3f lat %6.3f\n", lon, lat);
		GMT_message (C, "genper: xt %10.3e yt %10.3e\n", *xt, *yt);
		GMT_exit(1);
	}
}

/* Convert General Perspective x/y to lon/lat */

void GMT_igenper (struct GMT_CTRL *C, double *lon, double *lat, double xt, double yt)
{
	double H, P, R;
	double sin_c, cos_c;
	double x, y;
	double M, Q;
	double con, com, rho;

	H = C->current.proj.g_H;
	R = C->current.proj.g_R;
	P = C->current.proj.g_P;

	x = xt;
	y = yt;

	xt = (x * C->current.proj.g_cos_twist + y * C->current.proj.g_sin_twist);
	yt = (y * C->current.proj.g_cos_twist - x * C->current.proj.g_sin_twist);
	yt += C->current.proj.g_yoffset;

	M = H * xt / (H - yt * C->current.proj.g_sin_tilt);
	Q = H * yt * C->current.proj.g_cos_tilt /(H - yt * C->current.proj.g_sin_tilt);
	x = M * C->current.proj.g_cos_azimuth + Q * C->current.proj.g_sin_azimuth;
	y = Q * C->current.proj.g_cos_azimuth - M * C->current.proj.g_sin_azimuth;

	rho = hypot(x, y);

	C->current.proj.g_outside = FALSE;

	if (rho < GMT_SMALL) {
		*lat = C->current.proj.pole;
		*lon = C->current.proj.central_meridian;
		return;
	}
	if (rho > C->current.proj.g_rmax) {
		x *= C->current.proj.g_rmax/rho;
		y *= C->current.proj.g_rmax/rho;
		rho = C->current.proj.g_rmax;
		C->current.proj.g_outside = TRUE;
	}

	con = P - 1.0;
	com = P + 1.0;

	if (C->current.proj.ECC2 != 0.0)
		gmt_genper_tolatlong (C, x, y, 0.0, lat, lon);
	else {
		sin_c = (P - d_sqrt(1.0 - (rho * rho * com)/(R*R*con))) / (R * con / rho + rho/(R*con));
		cos_c = d_sqrt(1.0 - sin_c * sin_c);
		sin_c /= rho;
		*lat = d_asind (cos_c * C->current.proj.sinp + y * sin_c * C->current.proj.cosp);
		*lon = d_atan2d (x * sin_c, cos_c * C->current.proj.cosp - y * sin_c * C->current.proj.sinp) + C->current.proj.central_meridian;
	}
	return;
}

GMT_LONG GMT_genper_map_clip_path (struct GMT_CTRL *C, GMT_LONG np, double *work_x, double *work_y)
{
	GMT_LONG i;
	double da, angle;
	double x, y, xt, yt;

	if (C->current.proj.g_debug > 0) {
		GMT_message (C, "\n\ngenper_map_clip_path: np %ld\n", np);
		GMT_message (C, " x_scale %e y_scale %e, x0 %e y0 %e\n", C->current.proj.scale[GMT_X], C->current.proj.scale[GMT_Y], C->current.proj.origin[GMT_X], C->current.proj.origin[GMT_Y]);
	}

	da = TWO_PI/(np-1);

	for (i = 0; i < np; i++) {
		angle = i * da;
		sincos (angle, &x, &y);
		x *= C->current.proj.g_rmax;
		y *= C->current.proj.g_rmax;

		/* XXX forgotten R2D */
		gmt_genper_to_xtyt (C, angle*R2D, x, y, C->current.proj.g_yoffset, &xt, &yt);

		if (C->current.proj.g_width != 0.0) {
			xt = MAX (C->current.proj.g_xmin, MIN (xt, C->current.proj.g_xmax));
			yt = MAX (C->current.proj.g_ymin, MIN (yt, C->current.proj.g_ymax));
		}
		work_x[i] = xt * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X];
		work_y[i] = yt * C->current.proj.scale[GMT_Y] + C->current.proj.origin[GMT_Y];
	}
	return 0;
}

/* -JG ORTHOGRAPHIC PROJECTION */

void GMT_vortho (struct GMT_CTRL *C, double lon0, double lat0, double horizon)
{	/* Set up Orthographic projection */

	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;
	sincosd (lat0, &(C->current.proj.sinp), &(C->current.proj.cosp));
	C->current.proj.f_horizon = horizon;
	C->current.proj.rho_max = sind (horizon);
}

void GMT_ortho (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Orthographic x/y */
	double sin_lat, cos_lat, sin_lon, cos_lon;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);
	*x = C->current.proj.EQ_RAD * cos_lat * sin_lon;
	*y = C->current.proj.EQ_RAD * (C->current.proj.cosp * sin_lat - C->current.proj.sinp * cos_lat * cos_lon);
}

void GMT_iortho (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Orthographic x/y to lon/lat */
	double rho, cos_c;

	x *= C->current.proj.i_EQ_RAD;
	y *= C->current.proj.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > C->current.proj.rho_max) {
		*lat = *lon = C->session.d_NaN;
		return;
	}
	cos_c = sqrt (1.0 - rho * rho);	/* Produces NaN for rho > 1: beyond horizon */
	*lat = d_asind (cos_c * C->current.proj.sinp + y * C->current.proj.cosp);
	*lon = d_atan2d (x, cos_c * C->current.proj.cosp - y * C->current.proj.sinp) + C->current.proj.central_meridian;
}

/* -JF GNOMONIC PROJECTION */

void GMT_vgnomonic (struct GMT_CTRL *C, double lon0, double lat0, double horizon)
{	/* Set up Gnomonic projection */

	C->current.proj.central_meridian = lon0;
	C->current.proj.f_horizon = horizon;
	C->current.proj.rho_max = tand (C->current.proj.f_horizon);
	C->current.proj.pole = lat0;
	C->current.proj.north_pole = (lat0 > 0.0);
	sincosd (lat0, &(C->current.proj.sinp), &(C->current.proj.cosp));
}

void GMT_gnomonic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Gnomonic x/y */
	double k, sin_lat, cos_lat, sin_lon, cos_lon, cc;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);

	cc = cos_lat * cos_lon;
	k =  C->current.proj.EQ_RAD / (C->current.proj.sinp * sin_lat + C->current.proj.cosp * cc);
	*x = k * cos_lat * sin_lon;
	*y = k * (C->current.proj.cosp * sin_lat - C->current.proj.sinp * cc);
}

void GMT_ignomonic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Gnomonic x/y to lon/lat */
	double rho, c;

	x *= C->current.proj.i_EQ_RAD;
	y *= C->current.proj.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > C->current.proj.rho_max) {
		*lat = *lon = C->session.d_NaN;
		return;
	}
	c = atan (rho);
	*lat = d_asind (cos(c) * (C->current.proj.sinp + y * C->current.proj.cosp));
	*lon = d_atan2d (x, C->current.proj.cosp - y * C->current.proj.sinp) + C->current.proj.central_meridian;
}

/* -JE AZIMUTHAL EQUIDISTANT PROJECTION */

void GMT_vazeqdist (struct GMT_CTRL *C, double lon0, double lat0, double horizon)
{	/* Set up azimuthal equidistant projection */

	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;
	sincosd (lat0, &(C->current.proj.sinp), &(C->current.proj.cosp));
	C->current.proj.f_horizon = horizon;
	C->current.proj.rho_max = horizon * D2R;
}

void GMT_azeqdist (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to azimuthal equidistant x/y */
	double k, cc, c, clat, slon, clon, slat, t;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (GMT_IS_ZERO (lat-C->current.proj.pole) && GMT_IS_ZERO (lat-C->current.proj.central_meridian)) {	/* Center of projection */
		*x = *y = 0.0;
		return;
	}
	sincosd (lat, &slat, &clat);
	sincosd (lon, &slon, &clon);

	t = clat * clon;
	cc = C->current.proj.sinp * slat + C->current.proj.cosp * t;
	if (cc <= -1.0) {	/* Antipode is a circle, so flag x,y as NaN and increase counter */
		*x = *y = C->session.d_NaN;
		C->current.proj.n_antipoles++;
	}
	else {
		c = d_acos (cc);
		k = C->current.proj.EQ_RAD * c / sin (c);
		*x = k * clat * slon;
		*y = k * (C->current.proj.cosp * slat - C->current.proj.sinp * t);
	}
}

void GMT_iazeqdist (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert azimuthal equidistant x/y to lon/lat */
	double rho, sin_c, cos_c;

	x *= C->current.proj.i_EQ_RAD;
	y *= C->current.proj.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > C->current.proj.rho_max) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	sincos (rho, &sin_c, &cos_c);
	if (rho != 0.0) sin_c /= rho;
	/* Since in the rest we only have sin_c in combination with x or y, it is not necessary to set
	   sin_c = 1 when rho == 0. In that case x*sin_c and y*sin_c or 0 anyhow. */
	*lat = d_asind (cos_c * C->current.proj.sinp + y * sin_c * C->current.proj.cosp);
	*lon = d_atan2d (x * sin_c, cos_c * C->current.proj.cosp - y * sin_c * C->current.proj.sinp) + C->current.proj.central_meridian;
}

/* -JW MOLLWEIDE EQUAL AREA PROJECTION */

void GMT_vmollweide (struct GMT_CTRL *C, double lon0, double scale)
{	/* Set up Mollweide Equal-Area projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.w_x = C->current.proj.EQ_RAD * D2R * d_sqrt (8.0) / M_PI;
	C->current.proj.w_y = C->current.proj.EQ_RAD * M_SQRT2;
	C->current.proj.w_iy = 1.0 / C->current.proj.w_y;
	C->current.proj.w_r = 0.25 * (scale * C->current.proj.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_mollweide (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Mollweide Equal-Area x/y */
	GMT_LONG i;
	double phi, delta, psin_lat, c, s;

	if (doubleAlmostEqual (fabs (lat), 90.0)) {	/* Special case */
		*x = 0.0;
		*y = copysign (C->current.proj.w_y, lat);
		return;
	}

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);
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
	*x = C->current.proj.w_x * lon * c;
	*y = C->current.proj.w_y * s;
}

void GMT_imollweide (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Mollweide Equal-Area x/y to lon/lat */
	double phi, phi2;

	phi = asin (y * C->current.proj.w_iy);
	*lon = x / (C->current.proj.w_x * cos(phi));
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	*lon += C->current.proj.central_meridian;
	phi2 = 2.0 * phi;
	*lat = asind ((phi2 + sin (phi2)) / M_PI);
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

/* -JH HAMMER-AITOFF EQUAL AREA PROJECTION */

void GMT_vhammer (struct GMT_CTRL *C, double lon0, double scale)
{	/* Set up Hammer-Aitoff Equal-Area projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.w_r = 0.25 * (scale * C->current.proj.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_hammer (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Hammer-Aitoff Equal-Area x/y */
	double slat, clat, slon, clon, D;

	if (doubleAlmostEqual (fabs (lat), 90.0)) {	/* Save time */
		*x = 0.0;
		*y = M_SQRT2 * copysign (C->current.proj.EQ_RAD, lat);
		return;
	}

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);
	sincosd (lat, &slat, &clat);
	sincosd (0.5 * lon, &slon, &clon);

	D = C->current.proj.EQ_RAD * sqrt (2.0 / (1.0 + clat * clon));
	*x = 2.0 * D * clat * slon;
	*y = D * slat;
}

void GMT_ihammer (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Hammer-Aitoff Equal-Area x/y to lon/lat */
	double rho, a, sin_c, cos_c;

	x *= 0.5;
	rho = hypot (x, y);
	a = 0.5 * rho * C->current.proj.i_EQ_RAD;			/* a = sin(c/2)		*/
	a *= a;							/* a = sin(c/2)**2	*/
	cos_c = 1.0 - 2.0 * a;					/* cos_c = cos(c)	*/
	if (cos_c < -GMT_CONV_LIMIT) {					/* Horizon		*/
		*lat = *lon = C->session.d_NaN;
		return;
	}
	sin_c = sqrt (1.0 - a) * C->current.proj.i_EQ_RAD;		/* sin_c = sin(c)/rho	*/
	*lat = asind (y * sin_c);
	*lon = 2.0 * d_atan2d (x * sin_c, cos_c) + C->current.proj.central_meridian;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

/* -JV VAN DER GRINTEN PROJECTION */

void GMT_vgrinten (struct GMT_CTRL *C, double lon0, double scale)
{	/* Set up van der Grinten projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.v_r = M_PI * C->current.proj.EQ_RAD;
	C->current.proj.v_ir = 1.0 / C->current.proj.v_r;
}

void GMT_grinten (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to van der Grinten x/y */
	double flat, A, A2, G, P, P2, Q, P2A2, i_P2A2, GP2, c, s, theta;

	flat = fabs (lat);
	if (flat > (90.0 - GMT_CONV_LIMIT)) {	/* Save time */
		*x = 0.0;
		*y = M_PI * copysign (C->current.proj.EQ_RAD, lat);
		return;
	}
	if (doubleAlmostEqualZero (lon, C->current.proj.central_meridian)) {	/* Save time */
		theta = d_asin (2.0 * fabs (lat) / 180.0);
		*x = 0.0;
		*y = M_PI * copysign (C->current.proj.EQ_RAD, lat) * tan (0.5 * theta);
		return;
	}

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (GMT_IS_ZERO (flat)) {	/* Save time */
		*x = C->current.proj.EQ_RAD * D2R * lon;
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

	*x = copysign (C->current.proj.v_r, lon) * (A * GP2 + sqrt (A2 * GP2 * GP2 - P2A2 * (G*G - P2))) * i_P2A2;
	*y = copysign (C->current.proj.v_r, lat) * (P * Q - A * sqrt ((A2 + 1.0) * P2A2 - Q * Q)) * i_P2A2;
}

void GMT_igrinten (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert van der Grinten x/y to lon/lat */
	double x2, c1, x2y2, y2, c2, c3, d, a1, m1, theta1, p;

	x *= C->current.proj.v_ir;
	y *= C->current.proj.v_ir;
	x2 = x * x;	y2 = y * y;
	x2y2 = x2 + y2;
	if (x2y2 > 1.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	c1 = -fabs(y) * (1.0 + x2y2);
	c2 = c1 - 2 * y2 + x2;
	p = x2y2 * x2y2;
	c3 = -2.0 * c1 + 1.0 + 2.0 * y2 + p;
	d = y2 / c3 + (2 * pow (c2 / c3, 3.0) - 9.0 * c1 * c2 / (c3 * c3)) / 27.0;
	a1 = (c1 - c2 * c2 / (3.0 * c3)) / c3;
	m1 = 2.0 * sqrt (-a1 / 3.0);
	theta1 = d_acos (3.0 * d / (a1 * m1)) / 3.0;

	*lat = copysign (180.0, y) * (-m1 * cos (theta1 + M_PI/3.0) - c2 / (3.0 * c3));
	*lon = C->current.proj.central_meridian;
	if (x != 0.0) *lon += 90.0 * (x2y2 - 1.0 + sqrt (1.0 + 2 * (x2 - y2) + p)) / x;
}

/* -JR WINKEL-TRIPEL MODIFIED GMT_IS_AZIMUTHAL PROJECTION */

void GMT_vwinkel (struct GMT_CTRL *C, double lon0, double scale)
{	/* Set up Winkel Tripel projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.r_cosphi1 = cosd (50.0+(28.0/60.0));
	C->current.proj.central_meridian = lon0;
	C->current.proj.w_r = 0.25 * (scale * C->current.proj.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_winkel (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Winkel Tripel x/y */
	double X, D, x1, x2, y1, c, s;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	lat *= D2R;
	lon *= (0.5 * D2R);

	/* Fist find Aitoff x/y */

	sincos (lat, &s, &c);
	D = d_acos (c * cos (lon));
	if (GMT_IS_ZERO (D))
		x1 = y1 = 0.0;
	else {
		X = s / sin (D);
		x1 = copysign (D * d_sqrt (1.0 - X * X), lon);
		y1 = D * X;
	}

	/* Then get equirectangular projection */

	x2 = lon * C->current.proj.r_cosphi1;
	/* y2 = lat; use directly in averaging below */

	/* Winkler is the average value */

	*x = C->current.proj.EQ_RAD * (x1 + x2);
	*y = 0.5 * C->current.proj.EQ_RAD * (y1 + lat);
}

void GMT_iwinkel (struct GMT_CTRL *P, double *lon, double *lat, double x, double y)
{	/* Convert Winkel Tripel x/y to lon/lat */
	/* Based on iterative solution published by:
	 * Ipbuker, 2002, Cartography & Geographical Information Science, 20, 1, 37-42.
	 */

	GMT_LONG n_iter = 0;
	double phi0, lambda0, sp, cp, s2p, sl, cl, sl2, cl2, C, D, sq_C, C_32;
	double f1, f2, df1dp, df1dl, df2dp, df2dl, denom, delta;

	x *= P->current.proj.i_EQ_RAD;
	y *= P->current.proj.i_EQ_RAD;
	*lat = y / M_PI;	/* Initial guesses for lon and lat */
	*lon = x / M_PI;
	do {
		phi0 = *lat;
		lambda0 = *lon;
		sincos (phi0, &sp, &cp);
		sincos (lambda0, &sl, &cl);
		sincos (0.5 * lambda0, &sl2, &cl2);
		s2p = sin (2.0 * phi0);
		D = acos (cp * cl2);
		C = 1.0 - cp * cp * cl2 * cl2;
		sq_C = sqrt (C);
		C_32 = C * sq_C;
		f1 = 0.5 * ((2.0 * D * cp * sl2) / sq_C + lambda0 * P->current.proj.r_cosphi1) - x;
		f2 = 0.5 * (D * sp / sq_C + phi0) - y;
		df1dp = (0.25 * (sl * s2p) / C) - ((D * sp * sl2) / C_32);
		df1dl = 0.5 * ((cp * cp * sl2 * sl2) / C + (D * cp * cl2 * sp * sp) / C_32 + P->current.proj.r_cosphi1);
		df2dp = 0.5 * ((sp * sp * cl2) / C + (D * (1.0 - cl2 * cl2) * cp) / C_32 + 1.0);
		df2dl = 0.125 * ((s2p * sl2) / C - (D * sp * cp * cp * sl) / C_32);
		denom = df1dp * df2dl - df2dp * df1dl;
		*lat = phi0 - (f1 * df2dl - f2 * df1dl) / denom;
		*lon = lambda0 - (f2 * df1dp - f1 * df2dp) / denom;
		delta = fabs (*lat - phi0) + fabs (*lon - lambda0);
		n_iter++;
	}
	while (delta > 1e-12 && n_iter < 100);
	*lon *= R2D;
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = P->session.d_NaN;
		return;
	}
	*lon += P->current.proj.central_meridian;
	*lat *= R2D;
}

void gmt_iwinkel_sub (struct GMT_CTRL *C, double y, double *phi)
{	/* Valid only along meridian 180 degree from central meridian.  Used in left/right_winkel only */
	GMT_LONG n_iter = 0;
	double c, phi0, delta, sp, cp;

	c = 2.0 * y * C->current.proj.i_EQ_RAD;
	*phi = y * C->current.proj.i_EQ_RAD;
	do {
		phi0 = *phi;
		sincos (phi0, &sp, &cp);
		*phi = phi0 - (phi0 + M_PI_2 * sp - c) / (1.0 + M_PI_2 * cp);
		delta = fabs (*phi - phi0);
		n_iter++;
	}
	while (delta > GMT_CONV_LIMIT && n_iter < 100);
	*phi *= R2D;
}

double GMT_left_winkel (struct GMT_CTRL *C, double y)
{
	double c, phi, x;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	gmt_iwinkel_sub (C, y, &phi);
	GMT_geo_to_xy (C, C->current.proj.central_meridian - 180.0, phi, &x, &c);
	return (x);
}

double GMT_right_winkel (struct GMT_CTRL *C, double y)
{
	double c, phi, x;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	gmt_iwinkel_sub (C, y, &phi);
	GMT_geo_to_xy (C, C->current.proj.central_meridian + 180.0, phi, &x, &c);
	return (x);
}

/* -JKf ECKERT IV PROJECTION */

void GMT_veckert4 (struct GMT_CTRL *C, double lon0)
{	/* Set up Eckert IV projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.k4_x = 2.0 * C->current.proj.EQ_RAD / d_sqrt (M_PI * (4.0 + M_PI));
	C->current.proj.k4_y = 2.0 * C->current.proj.EQ_RAD * d_sqrt (M_PI / (4.0 + M_PI));
	C->current.proj.k4_ix = 1.0 / C->current.proj.k4_x;
	C->current.proj.k4_iy = 1.0 / C->current.proj.k4_y;
	C->current.proj.central_meridian = lon0;
}

void GMT_eckert4 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Eckert IV x/y */
	GMT_LONG n_iter = 0;
	double phi, delta, s_lat, s, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	phi = lat * D2R;
	s_lat = sin (phi);
	phi *= 0.5;
	do {
		sincos (phi, &s, &c);
		delta = -(phi + s * c + 2.0 * s - (2.0 + M_PI_2) * s_lat) / (2.0 * c * (1.0 + c));
		phi += delta;
	}
	while (fabs(delta) > GMT_CONV_LIMIT && n_iter < 100);

	sincos (phi, &s, &c);
	*x = C->current.proj.k4_x * lon * D2R * (1.0 + c);
	*y = C->current.proj.k4_y * s;
}

void GMT_ieckert4 (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Eckert IV x/y to lon/lat */

	double phi, s, c;

	s = y * C->current.proj.k4_iy;
	phi = d_asin (s);
	c = cos (phi);
	*lon = R2D * x * C->current.proj.k4_ix / (1.0 + c);
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	*lon += C->current.proj.central_meridian;
	*lat = d_asind ((phi + s * c + 2.0 * s) / (2.0 + M_PI_2));
}

double GMT_left_eckert4 (struct GMT_CTRL *C, double y)
{
	double x, phi;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	phi = d_asin (y * C->current.proj.k4_iy);
	x = C->current.proj.k4_x * D2R * (C->common.R.wesn[XLO] - C->current.proj.central_meridian) * (1.0 + cos (phi));
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_right_eckert4 (struct GMT_CTRL *C, double y)
{
	double x, phi;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	phi = d_asin (y * C->current.proj.k4_iy);
	x = C->current.proj.k4_x * D2R * (C->common.R.wesn[XHI] - C->current.proj.central_meridian) * (1.0 + cos (phi));
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

/* -JKs ECKERT VI PROJECTION */

void GMT_veckert6 (struct GMT_CTRL *C, double lon0)
{	/* Set up Eckert VI projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.k6_r = C->current.proj.EQ_RAD / sqrt (2.0 + M_PI);
	C->current.proj.k6_ir = 1.0 / C->current.proj.k6_r;
	C->current.proj.central_meridian = lon0;
}

void GMT_eckert6 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Eckert VI x/y */
	GMT_LONG n_iter = 0;
	double phi, delta, s_lat, s, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);
	phi = lat * D2R;
	s_lat = sin (phi);
	do {
		sincos (phi, &s, &c);
		delta = -(phi + s - (1.0 + M_PI_2) * s_lat) / (1.0 + c);
		phi += delta;
	}
	while (fabs(delta) > GMT_CONV_LIMIT && n_iter < 100);

	*x = C->current.proj.k6_r * lon * D2R * (1.0 + cos (phi));
	*y = 2.0 * C->current.proj.k6_r * phi;
}

void GMT_ieckert6 (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Eckert VI x/y to lon/lat */

	double phi, c, s;

	phi = 0.5 * y * C->current.proj.k6_ir;
	sincos (phi, &s, &c);
	*lon = R2D * x * C->current.proj.k6_ir / (1.0 + c);
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	*lon += C->current.proj.central_meridian;
	*lat = d_asin ((phi + s) / (1.0 + M_PI_2)) * R2D;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

double GMT_left_eckert6 (struct GMT_CTRL *C, double y)
{
	double x, phi;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	phi = 0.5 * y * C->current.proj.k6_ir;
	x = C->current.proj.k6_r * D2R * (C->common.R.wesn[XLO] - C->current.proj.central_meridian) * (1.0 + cos (phi));
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_right_eckert6 (struct GMT_CTRL *C, double y)
{
	double x, phi;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	phi = 0.5 * y * C->current.proj.k6_ir;
	x = C->current.proj.k6_r * D2R * (C->common.R.wesn[XHI] - C->current.proj.central_meridian) * (1.0 + cos (phi));
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

/* -JN ROBINSON PSEUDOCYLINDRICAL PROJECTION */

void GMT_vrobinson (struct GMT_CTRL *C, double lon0)
{	/* Set up Robinson projection */
	GMT_LONG err_flag;

	if (C->current.setting.interpolant == 0) {	/* Must reset and warn */
		GMT_message (C, "Warning: -JN requires Akima or Cubic spline interpolant, set to Akima\n");
		C->current.setting.interpolant = 1;
	}

	gmt_check_R_J (C, &lon0);
	C->current.proj.n_cx = 0.8487 * C->current.proj.EQ_RAD * D2R;
	C->current.proj.n_cy = 1.3523 * C->current.proj.EQ_RAD;
	C->current.proj.n_i_cy = 1.0 / C->current.proj.n_cy;
	C->current.proj.central_meridian = lon0;

	C->current.proj.n_phi[0] = 0;	C->current.proj.n_X[0] = 1.0000;	C->current.proj.n_Y[0] = 0.0000;
	C->current.proj.n_phi[1] = 5;	C->current.proj.n_X[1] = 0.9986;	C->current.proj.n_Y[1] = 0.0620;
	C->current.proj.n_phi[2] = 10;	C->current.proj.n_X[2] = 0.9954;	C->current.proj.n_Y[2] = 0.1240;
	C->current.proj.n_phi[3] = 15;	C->current.proj.n_X[3] = 0.9900;	C->current.proj.n_Y[3] = 0.1860;
	C->current.proj.n_phi[4] = 20;	C->current.proj.n_X[4] = 0.9822;	C->current.proj.n_Y[4] = 0.2480;
	C->current.proj.n_phi[5] = 25;	C->current.proj.n_X[5] = 0.9730;	C->current.proj.n_Y[5] = 0.3100;
	C->current.proj.n_phi[6] = 30;	C->current.proj.n_X[6] = 0.9600;	C->current.proj.n_Y[6] = 0.3720;
	C->current.proj.n_phi[7] = 35;	C->current.proj.n_X[7] = 0.9427;	C->current.proj.n_Y[7] = 0.4340;
	C->current.proj.n_phi[8] = 40;	C->current.proj.n_X[8] = 0.9216;	C->current.proj.n_Y[8] = 0.4958;
	C->current.proj.n_phi[9] = 45;	C->current.proj.n_X[9] = 0.8962;	C->current.proj.n_Y[9] = 0.5571;
	C->current.proj.n_phi[10] = 50;	C->current.proj.n_X[10] = 0.8679;	C->current.proj.n_Y[10] = 0.6176;
	C->current.proj.n_phi[11] = 55;	C->current.proj.n_X[11] = 0.8350;	C->current.proj.n_Y[11] = 0.6769;
	C->current.proj.n_phi[12] = 60;	C->current.proj.n_X[12] = 0.7986;	C->current.proj.n_Y[12] = 0.7346;
	C->current.proj.n_phi[13] = 65;	C->current.proj.n_X[13] = 0.7597;	C->current.proj.n_Y[13] = 0.7903;
	C->current.proj.n_phi[14] = 70;	C->current.proj.n_X[14] = 0.7186;	C->current.proj.n_Y[14] = 0.8435;
	C->current.proj.n_phi[15] = 75;	C->current.proj.n_X[15] = 0.6732;	C->current.proj.n_Y[15] = 0.8936;
	C->current.proj.n_phi[16] = 80;	C->current.proj.n_X[16] = 0.6213;	C->current.proj.n_Y[16] = 0.9394;
	C->current.proj.n_phi[17] = 85;	C->current.proj.n_X[17] = 0.5722;	C->current.proj.n_Y[17] = 0.9761;
	C->current.proj.n_phi[18] = 90;	C->current.proj.n_X[18] = 0.5322;	C->current.proj.n_Y[18] = 1.0000;
	if (C->current.setting.interpolant == 2) {	/* Natural cubic spline */
		err_flag = GMT_cspline (C, C->current.proj.n_phi, C->current.proj.n_X, GMT_N_ROBINSON, C->current.proj.n_x_coeff);
		err_flag = GMT_cspline (C, C->current.proj.n_phi, C->current.proj.n_Y, GMT_N_ROBINSON, C->current.proj.n_y_coeff);
		err_flag = GMT_cspline (C, C->current.proj.n_Y, C->current.proj.n_phi, GMT_N_ROBINSON, C->current.proj.n_iy_coeff);
	}
	else {	/* Akimas spline */
		err_flag = GMT_akima (C, C->current.proj.n_phi, C->current.proj.n_X, GMT_N_ROBINSON, C->current.proj.n_x_coeff);
		err_flag = GMT_akima (C, C->current.proj.n_phi, C->current.proj.n_Y, GMT_N_ROBINSON, C->current.proj.n_y_coeff);
		err_flag = GMT_akima (C, C->current.proj.n_Y, C->current.proj.n_phi, GMT_N_ROBINSON, C->current.proj.n_iy_coeff);
	}
}

double gmt_robinson_spline (struct GMT_CTRL *C, double xp, double *x, double *y, double *c)
{	/* Compute the interpolated values from the Robinson coefficients */

	GMT_LONG j = 0, j1;
	double yp, a, b, h, ih, dx;

	if (xp < x[0] || xp > x[GMT_N_ROBINSON-1])	/* Desired point outside data range */
		return (C->session.d_NaN);

	while (j < GMT_N_ROBINSON && x[j] <= xp) j++;
	if (j == GMT_N_ROBINSON) j--;
	if (j > 0) j--;

	dx = xp - x[j];
	switch (C->current.setting.interpolant) {	/* GMT_vrobinson would not allow case 0 so only 1 | 2 is possible */
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
		default:
			yp = 0;
	}
	return (yp);
}

void GMT_robinson (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Robinson x/y */
	double phi, X, Y;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	phi = fabs (lat);

	X = gmt_robinson_spline (C, phi, C->current.proj.n_phi, C->current.proj.n_X, C->current.proj.n_x_coeff);
	Y = gmt_robinson_spline (C, phi, C->current.proj.n_phi, C->current.proj.n_Y, C->current.proj.n_y_coeff);
	*x = C->current.proj.n_cx * X * lon;	/* D2R is in n_cx already */
	*y = C->current.proj.n_cy * copysign (Y, lat);
}

void GMT_irobinson (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Robinson x/y to lon/lat */
	double X, Y;

	Y = fabs (y * C->current.proj.n_i_cy);
	*lat = gmt_robinson_spline (C, Y, C->current.proj.n_Y, C->current.proj.n_phi, C->current.proj.n_iy_coeff);
	X = gmt_robinson_spline (C, *lat, C->current.proj.n_phi, C->current.proj.n_X, C->current.proj.n_x_coeff);
	*lon = x / (C->current.proj.n_cx * X);
	if ((fabs (*lon) - GMT_CONV_LIMIT) > 180.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	*lon += C->current.proj.central_meridian;
	if (y < 0.0) *lat = -(*lat);
}

double GMT_left_robinson (struct GMT_CTRL *C, double y)
{
	double x, X, Y;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	Y = fabs (y * C->current.proj.n_i_cy);
	if (GMT_intpol (C, C->current.proj.n_Y, C->current.proj.n_X, GMT_N_ROBINSON, 1, &Y, &X, C->current.setting.interpolant)) {
		GMT_message (C, "GMT Internal error in GMT_left_robinson!\n");
		GMT_exit (EXIT_FAILURE);
	}

	x = C->current.proj.n_cx * X * (C->common.R.wesn[XLO] - C->current.proj.central_meridian);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_right_robinson (struct GMT_CTRL *C, double y)
{
	double x, X, Y;

	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	Y = fabs (y * C->current.proj.n_i_cy);
	if (GMT_intpol (C, C->current.proj.n_Y, C->current.proj.n_X, GMT_N_ROBINSON, 1, &Y, &X, C->current.setting.interpolant)) {
		GMT_message (C, "GMT Internal error in GMT_right_robinson!\n");
		GMT_exit (EXIT_FAILURE);
	}

	x = C->current.proj.n_cx * X * (C->common.R.wesn[XHI] - C->current.proj.central_meridian);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

/* -JI SINUSOIDAL EQUAL AREA PROJECTION */

void GMT_vsinusoidal (struct GMT_CTRL *C, double lon0)
{	/* Set up Sinusoidal projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
}

void GMT_sinusoidal (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Sinusoidal Equal-Area x/y */

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);

	lat *= D2R;

	*x = C->current.proj.EQ_RAD * lon * D2R * cos (lat);
	*y = C->current.proj.EQ_RAD * lat;
}

void GMT_isinusoidal (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Sinusoidal Equal-Area x/y to lon/lat */

	*lat = y * C->current.proj.i_EQ_RAD;
	*lon = (doubleAlmostEqual (fabs (*lat), M_PI)) ? 0.0 : R2D * x / (C->current.proj.EQ_RAD * cos (*lat));
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = C->session.d_NaN;
		return;
	}
	*lon += C->current.proj.central_meridian;
	*lat *= R2D;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

double GMT_left_sinusoidal (struct GMT_CTRL *C, double y)
{
	double x, lat;
	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	lat = y * C->current.proj.i_EQ_RAD;
	x = C->current.proj.EQ_RAD * (C->common.R.wesn[XLO] - C->current.proj.central_meridian) * D2R * cos (lat);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_right_sinusoidal (struct GMT_CTRL *C, double y)
{
	double x, lat;
	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	lat = y * C->current.proj.i_EQ_RAD;
	x = C->current.proj.EQ_RAD * (C->common.R.wesn[XHI] - C->current.proj.central_meridian) * D2R * cos (lat);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

/* -JC CASSINI PROJECTION */

void GMT_vcassini (struct GMT_CTRL *C, double lon0, double lat0)
{	/* Set up Cassini projection */

	double e1, s2, c2;

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;
	C->current.proj.c_p = lat0 * D2R;
	sincos (2.0 * C->current.proj.c_p, &s2, &c2);

	e1 = (1.0 - d_sqrt (C->current.proj.one_m_ECC2)) / (1.0 + d_sqrt (C->current.proj.one_m_ECC2));

	C->current.proj.c_c1 = 1.0 - (1.0/4.0) * C->current.proj.ECC2 - (3.0/64.0) * C->current.proj.ECC4 - (5.0/256.0) * C->current.proj.ECC6;
	C->current.proj.c_c2 = -((3.0/8.0) * C->current.proj.ECC2 + (3.0/32.0) * C->current.proj.ECC4 + (25.0/768.0) * C->current.proj.ECC6);
	C->current.proj.c_c3 = (15.0/128.0) * C->current.proj.ECC4 + (45.0/512.0) * C->current.proj.ECC6;
	C->current.proj.c_c4 = -(35.0/768.0) * C->current.proj.ECC6;
	C->current.proj.c_M0 = C->current.proj.EQ_RAD * (C->current.proj.c_c1 * C->current.proj.c_p + s2 * (C->current.proj.c_c2 + c2 * (C->current.proj.c_c3 + c2 * C->current.proj.c_c4)));
	C->current.proj.c_i1 = 1.0 / (C->current.proj.EQ_RAD * C->current.proj.c_c1);
	C->current.proj.c_i2 = (3.0/2.0) * e1 - (29.0/12.0) * pow (e1, 3.0);
	C->current.proj.c_i3 = (21.0/8.0) * e1 * e1 - (1537.0/128.0) * pow (e1, 4.0);
	C->current.proj.c_i4 = (151.0/24.0) * pow (e1, 3.0);
	C->current.proj.c_i5 = (1097.0/64.0) * pow (e1, 4.0);
}

void GMT_cassini (struct GMT_CTRL *P, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Cassini x/y */

	double tany, N, T, A, C, M, s, c, s2, c2, A2, A3;

	GMT_WIND_LON (P, lon)	/* Remove central meridian and place lon in -180/+180 range */
	lon *= D2R;

	if (GMT_IS_ZERO (lat)) {	/* Quick when lat is zero */
		*x = P->current.proj.EQ_RAD * lon;
		*y = -P->current.proj.c_M0;
		return;
	}

	lat *= D2R;
	sincos (lat, &s, &c);
	sincos (2.0 * lat, &s2, &c2);
	tany = s / c;
	N = P->current.proj.EQ_RAD / sqrt (1.0 - P->current.proj.ECC2 * s * s);
	T = tany * tany;
	A = lon * c;
	A2 = A * A;
	A3 = A2 * A;
	C = P->current.proj.ECC2 * c * c * P->current.proj.i_one_m_ECC2;
	M = P->current.proj.EQ_RAD * (P->current.proj.c_c1 * lat + s2 * (P->current.proj.c_c2 + c2 * (P->current.proj.c_c3 + c2 * P->current.proj.c_c4)));
	*x = N * (A - T * A3 / 6.0 - (8.0 - T + 8 * C) * T * A3 * A2 / 120.0);
	*y = M - P->current.proj.c_M0 + N * tany * (0.5 * A2 + (5.0 - T + 6.0 * C) * A2 * A2 / 24.0);
}

void GMT_icassini (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Cassini x/y to lon/lat */

	double M1, u1, u2, s, c, phi1, tany, T1, N1, R_1, D, S2, D2, D3;

	M1 = C->current.proj.c_M0 + y;
	u1 = M1 * C->current.proj.c_i1;
	u2 = 2.0 * u1;
	sincos (u2, &s, &c);
	phi1 = u1 + s * (C->current.proj.c_i2 + c * (C->current.proj.c_i3 + c * (C->current.proj.c_i4 + c * C->current.proj.c_i5)));
	if (doubleAlmostEqual (fabs (phi1), M_PI_2)) {
		*lat = copysign (M_PI_2, phi1);
		*lon = C->current.proj.central_meridian;
	}
	else {
		sincos (phi1, &s, &c);
		tany = s / c;
		T1 = tany * tany;
		S2 = 1.0 - C->current.proj.ECC2 * s * s;
		N1 = C->current.proj.EQ_RAD / sqrt (S2);
		R_1 = C->current.proj.EQ_RAD * C->current.proj.one_m_ECC2 / pow (S2, 1.5);
		D = x / N1;
		D2 = D * D;
		D3 = D2 * D;
		*lat = R2D * (phi1 - (N1 * tany / R_1) * (0.5 * D2 - (1.0 + 3.0 * T1) * D2 * D2 / 24.0));
		*lon = C->current.proj.central_meridian + R2D * (D - T1 * D3 / 3.0 + (1.0 + 3.0 * T1) * T1 * D3 * D2 / 15.0) / c;
	}
}

/* Cassini functions for the Sphere */

void GMT_cassini_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Cassini x/y */
	double slon, clon, clat, tlat, slat;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lon, &slon, &clon);
	sincosd (lat, &slat, &clat);
	tlat = slat / clat;
	*x = C->current.proj.EQ_RAD * d_asin (clat * slon);
	*y = C->current.proj.EQ_RAD * (atan (tlat / clon) - C->current.proj.c_p);
}

void GMT_icassini_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Cassini x/y to lon/lat */

	double D, sD, cD, cx, tx, sx;

	x *= C->current.proj.i_EQ_RAD;
	D = y * C->current.proj.i_EQ_RAD + C->current.proj.c_p;
	sincos (D, &sD, &cD);
	sincos (x, &sx, &cx);
	tx = sx / cx;
	*lat = d_asind (sD * cx);
	*lon = C->current.proj.central_meridian + atand (tx / cD);
}

/* -JB ALBERS EQUAL-AREA CONIC PROJECTION */

void GMT_valbers (struct GMT_CTRL *C, double lon0, double lat0, double ph1, double ph2)
{	/* Set up Albers projection */

	double s0, s1, s2, c1, c2, q0, q1, q2, m1, m2;

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.north_pole = (C->common.R.wesn[YHI] > 0.0 && (C->common.R.wesn[YLO] >= 0.0 || (-C->common.R.wesn[YLO]) < C->common.R.wesn[YHI]));
	C->current.proj.pole = (C->current.proj.north_pole) ? 90.0 : -90.0;

	s0 = sind (lat0);
	sincosd (ph1, &s1, &c1);
	sincosd (ph2, &s2, &c2);

	m1 = c1 * c1 / (1.0 - C->current.proj.ECC2 * s1 * s1);	/* Actually m1 and m2 squared */
	m2 = c2 * c2 / (1.0 - C->current.proj.ECC2 * s2 * s2);
	q0 = (GMT_IS_ZERO (C->current.proj.ECC)) ? 2.0 * s0 : C->current.proj.one_m_ECC2 * (s0 / (1.0 - C->current.proj.ECC2 * s0 * s0) - C->current.proj.i_half_ECC * log ((1.0 - C->current.proj.ECC * s0) / (1.0 + C->current.proj.ECC * s0)));
	q1 = (GMT_IS_ZERO (C->current.proj.ECC)) ? 2.0 * s1 : C->current.proj.one_m_ECC2 * (s1 / (1.0 - C->current.proj.ECC2 * s1 * s1) - C->current.proj.i_half_ECC * log ((1.0 - C->current.proj.ECC * s1) / (1.0 + C->current.proj.ECC * s1)));
	q2 = (GMT_IS_ZERO (C->current.proj.ECC)) ? 2.0 * s2 : C->current.proj.one_m_ECC2 * (s2 / (1.0 - C->current.proj.ECC2 * s2 * s2) - C->current.proj.i_half_ECC * log ((1.0 - C->current.proj.ECC * s2) / (1.0 + C->current.proj.ECC * s2)));

	C->current.proj.a_n = (doubleAlmostEqualZero (ph1, ph2)) ? s1 : (m1 - m2) / (q2 - q1);
	C->current.proj.a_i_n = 1.0 / C->current.proj.a_n;
	C->current.proj.a_C = m1 + C->current.proj.a_n * q1;
	C->current.proj.a_rho0 = C->current.proj.EQ_RAD * sqrt (C->current.proj.a_C - C->current.proj.a_n * q0) * C->current.proj.a_i_n;
	C->current.proj.a_n2ir2 = (C->current.proj.a_n * C->current.proj.a_n) / (C->current.proj.EQ_RAD * C->current.proj.EQ_RAD);
	C->current.proj.a_test = 1.0 - (C->current.proj.i_half_ECC * C->current.proj.one_m_ECC2) * log ((1.0 - C->current.proj.ECC) / (1.0 + C->current.proj.ECC));

}

void GMT_valbers_sph (struct GMT_CTRL *C, double lon0, double lat0, double ph1, double ph2)
{	/* Set up Spherical Albers projection (Snyder, page 100) */

	double s1, c1;

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.north_pole = (C->common.R.wesn[YHI] > 0.0 && (C->common.R.wesn[YLO] >= 0.0 || (-C->common.R.wesn[YLO]) < C->common.R.wesn[YHI]));
	C->current.proj.pole = (C->current.proj.north_pole) ? 90.0 : -90.0;

	sincosd (ph1, &s1, &c1);

	C->current.proj.a_n = 0.5 * (s1 + sind (ph2));
	C->current.proj.a_i_n = 1.0 / C->current.proj.a_n;
	C->current.proj.a_C = c1 * c1 + 2.0 * C->current.proj.a_n * s1;
	C->current.proj.a_rho0 = C->current.proj.EQ_RAD * sqrt (C->current.proj.a_C - 2.0 * C->current.proj.a_n * sind (lat0)) * C->current.proj.a_i_n;
	C->current.proj.a_n2ir2 = 0.5 * C->current.proj.a_n / (C->current.proj.EQ_RAD * C->current.proj.EQ_RAD);
	C->current.proj.a_Cin = 0.5 * C->current.proj.a_C / C->current.proj.a_n;
}

void GMT_albers (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Albers x/y */

	double s, c, q, theta, rho, r;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	s = sind (lat);
	if (GMT_IS_ZERO (C->current.proj.ECC))
		q = 2.0 * s;
	else {
		r = C->current.proj.ECC * s;
		q = C->current.proj.one_m_ECC2 * (s / (1.0 - C->current.proj.ECC2 * s * s) - C->current.proj.i_half_ECC * log ((1.0 - r) / (1.0 + r)));
	}
	theta = C->current.proj.a_n * lon * D2R;
	rho = C->current.proj.EQ_RAD * sqrt (C->current.proj.a_C - C->current.proj.a_n * q) * C->current.proj.a_i_n;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = C->current.proj.a_rho0 - rho * c;
}

void GMT_ialbers (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Albers x/y to lon/lat */

	GMT_LONG n_iter;
	double theta, rho, q, phi, phi0, s, c, s2, ex_1, delta, r;

	theta = (C->current.proj.a_n < 0.0) ? d_atan2 (-x, y - C->current.proj.a_rho0) : d_atan2 (x, C->current.proj.a_rho0 - y);
	rho = hypot (x, C->current.proj.a_rho0 - y);
	q = (C->current.proj.a_C - rho * rho * C->current.proj.a_n2ir2) * C->current.proj.a_i_n;

	if (doubleAlmostEqualZero (fabs (q), C->current.proj.a_test))
		*lat = copysign (90.0, q);
	else {
		phi = d_asin (0.5 * q);
		n_iter = 0;
		do {
			phi0 = phi;
			sincos (phi0, &s, &c);
			r = C->current.proj.ECC * s;
			s2 = s * s;
			ex_1 = 1.0 - C->current.proj.ECC2 * s2;
			phi = phi0 + 0.5 * ex_1 * ex_1 * ((q * C->current.proj.i_one_m_ECC2) - s / ex_1
				+ C->current.proj.i_half_ECC * log ((1 - r) / (1.0 + r))) / c;
			delta = fabs (phi - phi0);
			n_iter++;
		}
		while (delta > GMT_CONV_LIMIT && n_iter < 100);
		*lat = R2D * phi;
	}
	*lon = C->current.proj.central_meridian + R2D * theta * C->current.proj.a_i_n;
}

/* Spherical versions of Albers */

void GMT_albers_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Spherical Albers x/y */

	double s, c, theta, rho;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (C->current.proj.GMT_convert_latitudes) lat = GMT_latg_to_lata (C, lat);

	theta = C->current.proj.a_n * lon * D2R;
	rho = C->current.proj.EQ_RAD * sqrt (C->current.proj.a_C - 2.0 * C->current.proj.a_n * sind (lat)) * C->current.proj.a_i_n;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = C->current.proj.a_rho0 - rho * c;
}

void GMT_ialbers_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Spherical Albers x/y to lon/lat */

	double theta, A, dy;

	theta = (C->current.proj.a_n < 0.0) ? d_atan2 (-x, y - C->current.proj.a_rho0) : d_atan2 (x, C->current.proj.a_rho0 - y);
	dy = C->current.proj.a_rho0 - y;
	A = (x * x + dy * dy) * C->current.proj.a_n2ir2;

	*lat = d_asind (C->current.proj.a_Cin - A);
	*lon = C->current.proj.central_meridian + R2D * theta * C->current.proj.a_i_n;
	if (C->current.proj.GMT_convert_latitudes) *lat = GMT_lata_to_latg (C, *lat);
}

/* -JD EQUIDISTANT CONIC PROJECTION */

void GMT_veconic (struct GMT_CTRL *C, double lon0, double lat0, double lat1, double lat2)
{	/* Set up Equidistant Conic projection */
	double c1;

	gmt_check_R_J (C, &lon0);
	C->current.proj.north_pole = (C->common.R.wesn[YHI] > 0.0 && (C->common.R.wesn[YLO] >= 0.0 || (-C->common.R.wesn[YLO]) < C->common.R.wesn[YHI]));
	c1 = cosd (lat1);
	C->current.proj.d_n = (doubleAlmostEqualZero (lat1, lat2)) ? sind (lat1) : (c1 - cosd (lat2)) / (D2R * (lat2 - lat1));
	C->current.proj.d_i_n = R2D / C->current.proj.d_n;	/* R2D put here instead of in lon for ieconic */
	C->current.proj.d_G = (c1 / C->current.proj.d_n) + lat1 * D2R;
	C->current.proj.d_rho0 = C->current.proj.EQ_RAD * (C->current.proj.d_G - lat0 * D2R);
	C->current.proj.central_meridian = lon0;
}

void GMT_econic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Equidistant Conic x/y */
	double rho, theta, s, c;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	rho = C->current.proj.EQ_RAD * (C->current.proj.d_G - lat * D2R);
	theta = C->current.proj.d_n * lon * D2R;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = C->current.proj.d_rho0 - rho * c;
}

void GMT_ieconic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Equidistant Conic x/y to lon/lat */

	double rho, theta;

	rho = hypot (x, C->current.proj.d_rho0 - y);
	if (C->current.proj.d_n < 0.0) rho = -rho;
	theta = (C->current.proj.d_n < 0.0) ? d_atan2 (-x, y - C->current.proj.d_rho0) : d_atan2 (x, C->current.proj.d_rho0 - y);
	*lat = (C->current.proj.d_G - rho * C->current.proj.i_EQ_RAD) * R2D;
	*lon = C->current.proj.central_meridian + theta * C->current.proj.d_i_n;
}

/* -JPoly POLYCONIC PROJECTION */

void GMT_vpolyconic (struct GMT_CTRL *C, double lon0, double lat0)
{	/* Set up Polyconic projection */

	gmt_check_R_J (C, &lon0);
	C->current.proj.central_meridian = lon0;
	C->current.proj.pole = lat0;
}

void GMT_polyconic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y)
{	/* Convert lon/lat to Polyconic x/y */
	double sE, cE, sp, cp;

	GMT_WIND_LON (C, lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (GMT_IS_ZERO(lat)) {
		*x = C->current.proj.EQ_RAD * lon * D2R;
		*y = C->current.proj.EQ_RAD * (lat - C->current.proj.pole) * D2R;
	}
	else {
		sincosd(lat, &sp, &cp);
		sincosd(lon * sp, &sE, &cE);
		cp /= sp;	/* = cot(phi) */
		*x = C->current.proj.EQ_RAD * cp * sE;
		*y = C->current.proj.EQ_RAD * ((lat - C->current.proj.pole) * D2R + cp * (1.0 - cE));
	}
}

void GMT_ipolyconic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y)
{	/* Convert Polyconic x/y to lon/lat */

	double B, phi, phi0, tanp, delta;
	GMT_LONG n_iter = 0;

	x *= C->current.proj.i_EQ_RAD;
	y *= C->current.proj.i_EQ_RAD;
	y += C->current.proj.pole * D2R;
	if (GMT_IS_ZERO (y)) {
		*lat = y * R2D + C->current.proj.pole;
		*lon = x * R2D + C->current.proj.central_meridian;
	}
	else {
		B = x * x + y * y;
		phi = y;
		do {
			phi0 = phi;
			tanp = tan(phi);
			phi -= (y * (phi * tanp + 1.0) - phi - 0.5 * (phi * phi + B) * tanp) /
				((phi - y) / tanp - 1.0);
			delta = fabs (phi - phi0);
			n_iter++;
		}
		while (delta > GMT_CONV_LIMIT && n_iter < 100);
		*lat = R2D * phi;
		*lon = C->current.proj.central_meridian + asind(x * tanp) / sin(phi);
	}
}

void gmt_ipolyconic_sub (struct GMT_CTRL *C, double y, double lon, double *x)
{	/* Used in left/right_polyconic only */
	double E, sp, cp, phi0, phi, delta;
	GMT_LONG n_iter = 0;

	*x = lon;
	GMT_WIND_LON (C, *x);
	y *= C->current.proj.i_EQ_RAD;
	y += C->current.proj.pole * D2R;
	if (GMT_IS_ZERO (y))
		*x *= C->current.proj.EQ_RAD * D2R;
	else {
		phi = y;
		do {
			phi0 = phi;
			sincos (phi, &sp, &cp);
			E = (*x) * sp;
			cp /= sp;	/* = cot(phi) */
			phi = y - cp * (1.0 - cosd(E));
			delta = fabs (phi - phi0);
			n_iter++;
		}
		while (delta > GMT_CONV_LIMIT && n_iter < 100);
		*x = C->current.proj.EQ_RAD * cp * sin(E);
	}
}

double GMT_left_polyconic (struct GMT_CTRL *C, double y)
{
	double x;
	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	gmt_ipolyconic_sub (C, y, C->common.R.wesn[XLO], &x);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}

double GMT_right_polyconic (struct GMT_CTRL *C, double y)
{
	double x;
	y -= C->current.proj.origin[GMT_Y];
	y *= C->current.proj.i_scale[GMT_Y];
	gmt_ipolyconic_sub (C, y, C->common.R.wesn[XHI], &x);
	return (x * C->current.proj.scale[GMT_X] + C->current.proj.origin[GMT_X]);
}
