/*--------------------------------------------------------------------
 *	$Id: gmt_proj.c,v 1.55 2011-03-03 21:02:50 guru Exp $
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
 * Date:	22-MAR-2006
 * Version:	4.2.2
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_proj.h"

void GMT_iwinkel_sub (double y, double *phi);			/* Used by GMT_{left,right}_winkel */
void GMT_ipolyconic_sub (double y, double lon, double *x);	/* Used by GMT_{left,right}_polyconic */
void GMT_check_R_J(double *clon);

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

/* LINEAR TRANSFORMATIONS */

void GMT_translin (double forw, double *inv)	/* Linear forward */
{
	*inv = forw;
}

void GMT_translind (double forw, double *inv)	/* Linear forward, but with degrees*/
{
	GMT_WIND_LON(forw)	/* Make sure forw is in -180/+180 range after removing central meridian */
	*inv = forw ;
}

void GMT_itranslind (double *forw, double inv)	/* Linear inverse, but with degrees*/
{
	while (inv < -GMT_180) inv += 360.0;
	while (inv > +GMT_180) inv -= 360.0;
	*forw = inv + project_info.central_meridian;
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

/* -JP POLAR (r-theta) PROJECTION */

void GMT_vpolar (double lon0)
{
	/* Set up a Polar (theta,r) transformation */

	project_info.p_base_angle = lon0;
	project_info.central_meridian = 0.5 * (project_info.e + project_info.w);

	/* Plus pretend that it is kind of a geographic polar projection */

	project_info.north_pole = project_info.got_elevations;
	project_info.pole = (project_info.got_elevations) ? 90.0 : 0.0;
}

void GMT_polar (double x, double y, double *x_i, double *y_i)
{	/* Transform x and y to polar(cylindrical) coordinates */
	if (project_info.got_azimuths) x = 90.0 - x;		/* azimuths, not directions */
	if (project_info.got_elevations) y = 90.0 - y;		/* elevations */
	sincosd (x - project_info.p_base_angle, y_i, x_i);	/* Change base line angle */
	(*x_i) *= y;
	(*y_i) *= y;
}

void GMT_ipolar (double *x, double *y, double x_i, double y_i)
{	/* Inversely transform both x and y from polar(cylindrical) coordinates */
	*x = d_atan2d (y_i, x_i) + project_info.p_base_angle;
	if (project_info.got_azimuths) *x = 90.0 - (*x);		/* azimuths, not directions */
	*y = hypot (x_i, y_i);
	if (project_info.got_elevations) *y = 90.0 - (*y);    /* elevations, presumably */
}

/* -JM MERCATOR PROJECTION */

void GMT_vmerc (double lon0, double slat)
{
	/* Set up a Mercator transformation */

	project_info.central_meridian = lon0;
	project_info.j_x = cosd (slat) / d_sqrt (1.0 - project_info.ECC2 * sind (slat) * sind (slat)) * project_info.EQ_RAD;
	project_info.j_ix = 1.0 / project_info.j_x;
}

/* Mercator projection for the sphere */

void GMT_merc_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Mercator x/y (project_info.EQ_RAD in project_info.j_x) */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	*x = project_info.j_x * D2R * lon;
	*y = (fabs (lat) < 90.0) ? project_info.j_x * d_log (tand (45.0 + 0.5 * lat)) : copysign (DBL_MAX, lat);
}

void GMT_imerc_sph (double *lon, double *lat, double x, double y)
{
	/* Convert Mercator x/y to lon/lat  (project_info.EQ_RAD in project_info.j_ix) */

	*lon = x * project_info.j_ix * R2D + project_info.central_meridian;
	*lat = atand (sinh (y * project_info.j_ix));
	if (project_info.GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/* -JY CYLINDRICAL EQUAL-AREA PROJECTION */

void GMT_vcyleq (double lon0, double slat)
{
	/* Set up a Cylindrical equal-area transformation */

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.j_x = project_info.EQ_RAD * D2R * cosd (slat);
	project_info.j_y = project_info.EQ_RAD / cosd (slat);
	project_info.j_ix = 1.0 / project_info.j_x;
	project_info.j_iy = 1.0 / project_info.j_y;
}

void GMT_cyleq (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cylindrical equal-area x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);

	*x = lon * project_info.j_x;
	*y = project_info.j_y * sind (lat);
	if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= project_info.Dx;
		(*y) *= project_info.Dy;
	}
}

void GMT_icyleq (double *lon, double *lat, double x, double y)
{
	/* Convert Cylindrical equal-area x/y to lon/lat */

	if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}
	*lon = x * project_info.j_ix + project_info.central_meridian;
	*lat = d_asind (y * project_info.j_iy);
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/* -JQ CYLINDRICAL EQUIDISTANT PROJECTION */

void GMT_vcyleqdist (double lon0, double slat)
{
	/* Set up a Cylindrical equidistant transformation */

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.j_x = D2R * project_info.EQ_RAD * cosd (slat);
	project_info.j_y = D2R * project_info.EQ_RAD;
	project_info.j_ix = 1.0 / project_info.j_x;
	project_info.j_iy = 1.0 / project_info.j_y;
}

void GMT_cyleqdist (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cylindrical equidistant x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * project_info.j_x;
	*y = lat * project_info.j_y;
}

void GMT_icyleqdist (double *lon, double *lat, double x, double y)
{

	/* Convert Cylindrical equal-area x/y to lon/lat */

	*lon = x * project_info.j_ix + project_info.central_meridian;
	*lat = y * project_info.j_iy;
}

/* -JJ MILLER CYLINDRICAL PROJECTION */

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

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * project_info.j_x;
	*y = project_info.j_y * d_log (tand (45.0 + 0.4 * lat));
}

void GMT_imiller (double *lon, double *lat, double x, double y)
{

	/* Convert Miller Cylindrical x/y to lon/lat */

	*lon = x * project_info.j_ix + project_info.central_meridian;
	*lat = 2.5 * atand (exp (y * project_info.j_iy)) - 112.5;
}

/* -JCyl_stere CYLINDRICAL STEREOGRAPHIC PROJECTION */

void GMT_vcylstereo (double lon0, double slat)
{
	/* Set up a Cylindrical Stereographic transformation */

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.j_x = project_info.EQ_RAD * D2R * cosd (slat);
	project_info.j_y = project_info.EQ_RAD * (1.0 + cosd (slat));
	project_info.j_ix = 1.0 / project_info.j_x;
	project_info.j_iy = 1.0 / project_info.j_y;
}

void GMT_cylstereo (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cylindrical Stereographic x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	*x = lon * project_info.j_x;
	*y = project_info.j_y * tand (0.5 * lat);
}

void GMT_icylstereo (double *lon, double *lat, double x, double y)
{

	/* Convert Cylindrical Stereographic x/y to lon/lat */

	*lon = x * project_info.j_ix + project_info.central_meridian;
	*lat = 2.0 * atand (y * project_info.j_iy);
}


/* -JS POLAR STEREOGRAPHIC PROJECTION */

void GMT_vstereo (double lon0, double lat0, double horizon)
{
	/* Set up a Stereographic transformation */

	double clat;
	/* GMT_check_R_J (&lon0); */
	if (project_info.GMT_convert_latitudes) {	/* Set Conformal radius and pole latitude */
		GMT_scale_eqrad ();
		clat = GMT_latg_to_latc (lat0);
	}
	else
		clat = lat0;

	project_info.central_meridian = lon0;
	project_info.pole = lat0;		/* This is always geodetic */
	sincosd (clat, &(project_info.sinp), &(project_info.cosp));	/* These may be conformal */
	project_info.north_pole = (lat0 > 0.0);
	project_info.s_c = 2.0 * project_info.EQ_RAD * gmtdefs.map_scale_factor;
	project_info.s_ic = 1.0 / project_info.s_c;
	project_info.f_horizon = horizon;
	project_info.rho_max = tand (0.5 * horizon) * project_info.s_c;
}

void GMT_plrs_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to x/y using Spherical polar projection */
	double rho, slon, clon;

	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lon, &slon, &clon);
	if (project_info.north_pole) {
		rho = project_info.s_c * tand (45.0 - 0.5 * lat);
		*y = -rho * clon;
		*x =  rho * slon;
	}
	else {
		rho = project_info.s_c * tand (45.0 + 0.5 * lat);
		*y = rho * clon;
		*x = rho * slon;
	}
	if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
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

	if (project_info.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}
	c = 2.0 * atan (hypot (x, y) * project_info.s_ic);

	if (project_info.north_pole) {
		*lon = project_info.central_meridian + d_atan2d (x, -y);
		*lat = d_asind (cos (c));
	}
	else {
		*lon = project_info.central_meridian + d_atan2d (x, y);
		*lat = d_asind (-cos (c));
	}
	if (project_info.GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);

}

void GMT_stereo1_sph (double lon, double lat, double *x, double *y)
{
	double sin_dlon, cos_dlon, s, c, cc, A;

	/* Convert lon/lat to x/y using Spherical stereographic projection, oblique view */

	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
	sincosd (lon - project_info.central_meridian, &sin_dlon, &cos_dlon);
	sincosd (lat, &s, &c);
	cc = c * cos_dlon;
	A = project_info.s_c / (1.0 + project_info.sinp * s + project_info.cosp * cc);
	*x = A * c * sin_dlon;
	*y = A * (project_info.cosp * s - project_info.sinp * cc);
	if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
		(*x) *= project_info.Dx;
		(*y) *= project_info.Dy;
	}
}

void GMT_istereo_sph (double *lon, double *lat, double x, double y)
{
	double rho, c, sin_c, cos_c;

	if (project_info.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}
	rho = hypot (x, y);
	if (rho > project_info.rho_max) {
		*lat = *lon = GMT_d_NaN;
		return;
	}
	c = 2.0 * atan (rho * project_info.s_ic);
	sincos (c, &sin_c, &cos_c);
	if (rho != 0.0) sin_c /= rho;
	*lat = asind (cos_c * project_info.sinp + y * sin_c * project_info.cosp);
	*lon = d_atan2d (x * sin_c, cos_c * project_info.cosp - y * sin_c * project_info.sinp) + project_info.central_meridian;
	if (project_info.GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/* Spherical equatorial view */

void GMT_stereo2_sph (double lon, double lat, double *x, double *y)
{
	double dlon, s, c, clon, slon, A;

	/* Convert lon/lat to x/y using stereographic projection, equatorial view */

	dlon = lon - project_info.central_meridian;
	if (GMT_IS_ZERO (dlon - 180.0)) {
		*x = *y = 0.0;
	}
	else {
		if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);
		sincosd (lat, &s, &c);
		sincosd (dlon, &slon, &clon);
		A = project_info.s_c / (1.0 + c * clon);
		*x = A * c * slon;
		*y = A * s;
		if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= project_info.Dx;
			(*y) *= project_info.Dy;
		}
	}
}

/* -JL LAMBERT CONFORMAL CONIC PROJECTION */

void GMT_vlamb (double rlong0, double rlat0, double pha, double phb)
{
	/* Set up a Lambert Conformal Conic projection (Spherical when e = 0) */

	double sin_pha, cos_pha, sin_phb, cos_phb, t_pha, m_pha, t_phb, m_phb, t_rlat0;

	GMT_check_R_J (&rlong0);
	project_info.north_pole = (rlat0 > 0.0);
	project_info.pole = (project_info.north_pole) ? 90.0 : -90.0;
	sincosd (pha, &sin_pha, &cos_pha);
	sincosd (phb, &sin_phb, &cos_phb);
	
	t_pha = tand (45.0 - 0.5 * pha) / pow ((1.0 - project_info.ECC *
		sin_pha) / (1.0 + project_info.ECC * sin_pha), project_info.half_ECC);
	m_pha = cos_pha / d_sqrt (1.0 - project_info.ECC2 * sin_pha * sin_pha);
	t_phb = tand (45.0 - 0.5 * phb) / pow ((1.0 - project_info.ECC *
		sin_phb) / (1.0 + project_info.ECC * sin_phb), project_info.half_ECC);
	m_phb = cos_phb / d_sqrt (1.0 - project_info.ECC2 * sin_phb * sin_phb);
	t_rlat0 = tand (45.0 - 0.5 * rlat0) /
		pow ((1.0 - project_info.ECC * sind (rlat0)) /
		(1.0 + project_info.ECC * sind (rlat0)), project_info.half_ECC);

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

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	es = project_info.ECC * sind (lat);
	hold2 = pow ((1.0 - es) / (1.0 + es), project_info.half_ECC);
	hold3 = tand (45.0 - 0.5 * lat);
	hold1 = pow (hold3 / hold2, project_info.l_N);
	rho = project_info.l_rF * hold1;
	theta = project_info.l_Nr * lon;
	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.l_rho0 - rho * c;
}

void GMT_ilamb (double *lon, double *lat, double x, double y)
{
	GMT_LONG i;
	double theta, rho, t, tphi, phi, dy, r;

	dy = project_info.l_rho0 - y;
	theta = (project_info.l_N < 0.0) ? d_atan2 (-x, -dy) : d_atan2 (x, dy);
	*lon = theta * project_info.l_i_Nr + project_info.central_meridian;

	rho = copysign (hypot (x, dy), project_info.l_N);
	t = pow (rho * project_info.l_i_rF, project_info.l_i_N);
	phi = 0.0; tphi = 999.0;	/* Initialize phi = 0 */
	for (i = 0; i < 100 && fabs(tphi - phi) > GMT_CONV_LIMIT; i++) {
		tphi = phi;
		r = project_info.ECC * sin (phi);
		phi = M_PI_2 - 2.0 * atan (t * pow ((1.0 - r) / (1.0 + r), project_info.half_ECC));
	}
	*lat = phi * R2D;
}

/* Spherical cases of Lambert */

void GMT_lamb_sph (double lon, double lat, double *x, double *y)
{
	double rho, theta, t, s, c;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	t = MAX (0.0, tand (45.0 - 0.5 * lat));	/* Guard against negative t */
	rho = project_info.l_rF * pow (t, project_info.l_N);
	theta = project_info.l_Nr * lon;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.l_rho0 - rho * c;
}

void GMT_ilamb_sph (double *lon, double *lat, double x, double y)
{
	double theta, rho, t, dy;

	dy = project_info.l_rho0 - y;
	theta = (project_info.l_N < 0.0) ? d_atan2 (-x, -dy) : d_atan2 (x, dy);
	*lon = theta * project_info.l_i_Nr + project_info.central_meridian;

	rho = copysign (hypot (x, dy), project_info.l_N);
	t = pow (rho * project_info.l_i_rF, project_info.l_i_N);
	*lat = 90.0 - 2.0 * atand (t);
	if (project_info.GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/* -JO OBLIQUE MERCATOR PROJECTION */

void GMT_oblmrc (double lon, double lat, double *x, double *y)
{
	/* Convert a longitude/latitude point to Oblique Mercator coordinates
	 * by way of rotation coordinates and then using regular Mercator */
	double tlon, tlat;

	GMT_obl (lon * D2R, lat * D2R, &tlon, &tlat);

	*x = project_info.j_x * tlon;
	*y = (fabs (tlat) < M_PI_2) ? project_info.j_x * d_log (tan (M_PI_4 + 0.5 * tlat)) : copysign (DBL_MAX, tlat);
}

void GMT_ioblmrc (double *lon, double *lat, double x, double y)
{
	/* Convert a longitude/latitude point from Oblique Mercator coordinates
	 * by way of regular Mercator and then rotate coordinates */
	double tlon, tlat;

	tlon = x * project_info.j_ix;
	tlat = atan (sinh (y * project_info.j_ix));

	GMT_iobl (lon, lat, tlon, tlat);

	(*lon) *= R2D;
	(*lat) *= R2D;
}

void GMT_obl (double lon, double lat, double *olon, double *olat)
{
	/* Convert a longitude/latitude point to Oblique lon/lat (all in rads) */
	double p_cross_x[3], X[3];

	GMT_geo_to_cart (lat, lon, X, FALSE);

	*olat = d_asin (GMT_dot3v (X, project_info.o_FP));

	GMT_cross3v (project_info.o_FP, X, p_cross_x);
	GMT_normalize3v (p_cross_x);

	*olon = copysign (d_acos (GMT_dot3v (p_cross_x, project_info.o_FC)), GMT_dot3v (X, project_info.o_FC));
}

void GMT_iobl (double *lon, double *lat, double olon, double olat)
{
	/* Convert a longitude/latitude point from Oblique lon/lat  (all in rads) */
	double p_cross_x[3], X[3];

	GMT_geo_to_cart (olat, olon, X, FALSE);
	*lat = d_asin (GMT_dot3v (X, project_info.o_IP));

	GMT_cross3v (project_info.o_IP, X, p_cross_x);
	GMT_normalize3v (p_cross_x);

	*lon = copysign (d_acos (GMT_dot3v (p_cross_x, project_info.o_IC)), GMT_dot3v (X, project_info.o_IC));

	while ((*lon) < 0.0) (*lon) += TWO_PI;
	while ((*lon) >= TWO_PI) (*lon) -= TWO_PI;
}

/* -JT TRANSVERSE MERCATOR PROJECTION */

void GMT_vtm (double lon0, double lat0)
{
	/* Set up an TM projection */
	double e1, s2, c2;

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
	project_info.t_lat0 = lat0 * D2R;	/* In radians */
	sincos (2.0 * project_info.t_lat0, &s2, &c2);
	project_info.t_M0 = project_info.EQ_RAD * (project_info.t_c1 * project_info.t_lat0 + s2 * (project_info.t_c2 + c2 * (project_info.t_c3 + c2 * project_info.t_c4)));
	project_info.t_r = project_info.EQ_RAD * gmtdefs.map_scale_factor;
	project_info.t_ir = 1.0 / project_info.t_r;
}

/* Ellipsoidal TM functions */

void GMT_tm (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to TM x/y */
	double N, T, T2, C, A, M, dlon, tan_lat, A2, A3, A5, s, c, s2, c2;

	if (GMT_IS_ZERO (fabs (lat) - 90.0)) {
		M = project_info.EQ_RAD * project_info.t_c1 * M_PI_2;
		*x = 0.0;
		*y = gmtdefs.map_scale_factor * M;
	}
	else {
		lat *= D2R;
		sincos (lat, &s, &c);
		sincos (2.0 * lat, &s2, &c2);
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

	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_latc (lat);

	sincosd (lat, &slat, &clat);
	sincosd (dlon, &slon, &clon);
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
	phi = asind (sind / coshxx);
	*lat = phi;

	lambda = atan2d (sinhxx, cosd);
	*lon = lambda + project_info.central_meridian;
	if (project_info.GMT_convert_latitudes) *lat = GMT_latc_to_latg (*lat);
}

/* -JU UNIVERSAL TRANSVERSE MERCATOR PROJECTION */

/* Ellipsoidal UTM */

void GMT_utm (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm (lon, lat, x, y);
	(*x) += GMT_FALSE_EASTING;
	if (!project_info.north_pole) (*y) += GMT_FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm (double *lon, double *lat, double x, double y)
{
	/* Convert UTM x/y to lon/lat */

	x -= GMT_FALSE_EASTING;
	if (!project_info.north_pole) y -= GMT_FALSE_NORTHING;
	GMT_itm (lon, lat, x, y);
}

/* Spherical UTM */

void GMT_utm_sph (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to UTM x/y */

	if (lon < 0.0) lon += 360.0;
	GMT_tm_sph (lon, lat, x, y);
	(*x) += GMT_FALSE_EASTING;
	if (!project_info.north_pole) (*y) += GMT_FALSE_NORTHING;	/* For S hemisphere, add 10^7 m */
}

void GMT_iutm_sph (double *lon, double *lat, double x, double y)
{
	/* Convert UTM x/y to lon/lat */

	x -= GMT_FALSE_EASTING;
	if (!project_info.north_pole) y -= GMT_FALSE_NORTHING;
	GMT_itm_sph (lon, lat, x, y);
}

/* -JA LAMBERT AZIMUTHAL EQUAL AREA PROJECTION */

void GMT_vlambeq (double lon0, double lat0, double horizon)
{
	/* Set up Spherical Lambert Azimuthal Equal-Area projection */

	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	if (project_info.GMT_convert_latitudes) lat0 = GMT_latg_to_lata (lat0);
	sincosd (lat0, &(project_info.sinp), &(project_info.cosp));
	project_info.f_horizon = horizon;
	project_info.rho_max = 2.0 * sind (0.5 * horizon) * project_info.EQ_RAD;
}

void GMT_lambeq (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Spherical Lambert Azimuthal Equal-Area x/y */
	double k, tmp, sin_lat, cos_lat, sin_lon, cos_lon, c;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);
	c = cos_lat * cos_lon;
	tmp = 1.0 + project_info.sinp * sin_lat + project_info.cosp * c;
	if (tmp > 0.0) {
		k = project_info.EQ_RAD * d_sqrt (2.0 / tmp);
		*x = k * cos_lat * sin_lon;
		*y = k * (project_info.cosp * sin_lat - project_info.sinp * c);
		if (project_info.GMT_convert_latitudes) {	/* Gotta fudge abit */
			(*x) *= project_info.Dx;
			(*y) *= project_info.Dy;
		}
	}
	else
		*x = *y = -DBL_MAX;
}

void GMT_ilambeq (double *lon, double *lat, double x, double y)
{
	/* Convert Lambert Azimuthal Equal-Area x/y to lon/lat */
	double rho, a, sin_c, cos_c;

	if (project_info.GMT_convert_latitudes) {	/* Undo effect of fudge factors */
		x *= project_info.iDx;
		y *= project_info.iDy;
	}

	rho = hypot (x, y);
	if (rho > project_info.rho_max) {			/* Horizon		*/
		*lat = *lon = GMT_d_NaN;
		return;
	}
	a = 0.5 * rho * project_info.i_EQ_RAD;			/* a = sin(c/2)		*/
	a *= a;							/* a = sin(c/2)**2	*/
	cos_c = 1.0 - 2.0 * a;					/* cos_c = cos(c)	*/
	sin_c = sqrt (1.0 - a) * project_info.i_EQ_RAD;		/* sin_c = sin(c)/rho	*/
	*lat = d_asind (cos_c * project_info.sinp + y * sin_c * project_info.cosp);
	*lon = d_atan2d (x * sin_c, project_info.cosp * cos_c - y * project_info.sinp * sin_c) + project_info.central_meridian;
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/* -JG GENERAL PERSPECTIVE PROJECTION */

/* Set up General Perspective projection */

void genper_to_xtyt (double angle, double x, double y, double offset, double *xt, double *yt)
{
	double A, theta, xp, yp;

	theta = project_info.g_azimuth - angle;

	A = (y * project_info.g_cos_azimuth + x * project_info.g_sin_azimuth) * project_info.g_sin_tilt / project_info.g_H + project_info.g_cos_tilt;

	if (A > 0.0) {
		xp = (x * project_info.g_cos_azimuth - y * project_info.g_sin_azimuth) * project_info.g_cos_tilt / A;
		yp = (y * project_info.g_cos_azimuth + x * project_info.g_sin_azimuth) / A;
		if (fabs(yp) > fabs(project_info.g_max_yt)) {
			yp = -project_info.g_max_yt;
			xp = -yp * tand(theta);
		}
	}
	else {
		yp = -project_info.g_max_yt;
		xp = -yp * tand(theta);
	}

	yp -= offset;

	*xt = xp * project_info.g_cos_twist - yp * project_info.g_sin_twist;
	*yt = yp * project_info.g_cos_twist + xp * project_info.g_sin_twist;

	return;
}

/* conversion from geodetic latitude to geocentric latitude */

double genper_getgeocentric (double phi, double h)
{
	double phig, sphi, cphi, N1;

	sincosd (phi, &sphi, &cphi);

	N1 = project_info.EQ_RAD/sqrt(1.0 - (project_info.ECC2*sphi*sphi));

	phig = phi - asind(N1*project_info.ECC2*sphi*cphi/((h/project_info.EQ_RAD+1.0)*project_info.EQ_RAD));

	return (phig);
}

void genper_toxy (double lat, double lon, double h, double *x, double *y)
{
	double angle;
	double xp, yp, rp;
	double N, C, S, K;
	double sphi, cphi;
	double sdphi, cdphi;
	double sphi1, cphi1;
	double sdlon, cdlon;

	cdphi = project_info.g_cdphi;
	sdphi = project_info.g_sdphi;

	cphi1 = project_info.g_cphi1;
	sphi1 = project_info.g_sphi1;
	h *= 1e3;

	sincosd (lat, &sphi, &cphi);

	N = project_info.g_R/sqrt(1.0 - (project_info.ECC2*sphi*sphi));
	C = ((N+h)/project_info.g_R)*cphi;
	S = ((N*project_info.one_m_ECC2 + h)/project_info.g_R)*sphi;

	sincosd (lon - project_info.g_lon0, &sdlon, &cdlon);

	K = project_info.g_H / (project_info.g_P*cdphi - S*sphi1 - C*cphi1*cdlon);

	xp = K*C*sdlon;
	yp = K*(project_info.g_P*sdphi + S*cphi1 - C*sphi1*cdlon);

	rp = sqrt(xp*xp + yp*yp);

	if (rp > project_info.g_rmax) {
		angle = atan2(xp, yp);
		sincos (angle, &xp, &yp);
		xp *= project_info.g_rmax;
		yp *= project_info.g_rmax;
		angle *= R2D;
	}
	*x = xp;
	*y = yp;

	if (project_info.g_debug > 1) {
		fprintf (stderr, "\n");
		fprintf (stderr, "lat  %12.3f\n", lat);
		fprintf (stderr, "lon  %12.3f\n", lon);
		fprintf (stderr, "h    %12.3f\n", h);
		fprintf (stderr, "N    %12.1f\n", N);
		fprintf (stderr, "C    %12.7f\n", C);
		fprintf (stderr, "S    %12.7f\n", S);
		fprintf (stderr, "K    %12.1f\n", K);
		fprintf (stderr, "x    %12.1f\n", *x);
		fprintf (stderr, "y    %12.1f\n", *y);
	}
}

void genper_tolatlong (double x, double y, double h, double *lat, double *lon)
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

	H = project_info.g_H;
	P = project_info.g_P;
	R = project_info.g_R;

	one_m_e2 = project_info.one_m_ECC2;
	e2 = project_info.ECC2;

	cphig = project_info.g_cphig;
	cphi1 = project_info.g_cphi1;
	sphi1 = project_info.g_sphi1;

	B = project_info.g_B;
	D = project_info.g_D;

	u = project_info.g_BLH - project_info.g_DG*y + project_info.g_BJ*y + project_info.g_DHJ;
	v = project_info.g_LH2 + project_info.g_G*y*y - project_info.g_HJ*y + one_m_e2*x*x;

	if (project_info.g_debug > 1) {
		fprintf(stderr, "\n");
		fprintf(stderr, "genper_tolatlong - 1 \n");
		fprintf(stderr, "x    %12.1f\n", x);
		fprintf(stderr, "y    %12.1f\n", y);
		fprintf(stderr, "\n");
		fprintf(stderr, "P    %12.7f\n", P);
		fprintf(stderr, "phig %12.7f\n", project_info.g_phig);
		fprintf(stderr, "\n");
		fprintf(stderr, "B    %12.7f\n", B);
		fprintf(stderr, "D    %12.7f\n", D);
		fprintf(stderr, "u    %12.1f\n", u);
		fprintf(stderr, "v    %12.6e\n", v);
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
		fprintf (stderr, "\n");
		fprintf (stderr, "genper_tolatlong - 2\n");
		fprintf (stderr, "x    %12.1f\n", x);
		fprintf (stderr, "y    %12.1f\n", y);
		fprintf (stderr, "\n");
		fprintf (stderr, "P    %12.7f\n", P);
		fprintf (stderr, "phig %12.7f\n", project_info.g_phig);
		fprintf (stderr, "\n");
		fprintf (stderr, "B    %12.7f\n", B);
		fprintf (stderr, "D    %12.7f\n", D);
		fprintf (stderr, "u    %12.1f\n", u);
		fprintf (stderr, "v    %12.6e\n", v);
	}
	if (set_exit || project_info.g_debug > 1) {
		fprintf (stderr, "t    %12.7f\n", t);
		fprintf (stderr, "Kp   %12.1f\n", Kp);
		fprintf (stderr, "Kp2  %12.1f\n", Kp2);
		fprintf (stderr, "X    %12.1f\n", X);
		fprintf (stderr, "Y    %12.1f\n", Y);
		fprintf (stderr, "S    %12.7f\n", S);
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

		if (set_exit == 1) fprintf (stderr, "genper_tolatlong - 3\n");
		if (project_info.g_debug > 1 || set_exit) {
			fprintf (stderr, "asinS %12.7f\n", asind(S));
			fprintf (stderr, "phi   %12.7f\n", R2D*phi);
			fprintf (stderr, "E     %12.7f\n", E);
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
			if (set_exit == 1) fprintf (stderr, "genper_tolatlong - 4 \n");
			if (set_exit || project_info.g_debug > 1) {
				fprintf (stderr, "\niter %ld\n", niter);
				fprintf (stderr, "t    %12.7f\n", t);
				fprintf (stderr, "Kp   %12.1f\n", Kp);
				fprintf (stderr, "X    %12.1f\n", X);
				fprintf (stderr, "Y    %12.1f\n", Y);
				fprintf (stderr, "S    %12.7f\n", S);
				fprintf (stderr, "phi  %12.7f\n", phi*R2D);
				fprintf (stderr, "E    %12.7f\n", E);
			}
		}
		while (fabs(phi - phi_last) > 1e-7);
	}
	if (set_exit == 1) fprintf(stderr, "genper_tolatlong - 5\n");
	if (set_exit || project_info.g_debug > 1) {
		fprintf (stderr, "phi    %12.7f\n", phi*R2D);
		GMT_exit(1);
	}
	*lat = phi * R2D;
	*lon = atan2d (Y, X) + project_info.g_lon0;
	return;
}

void genper_setup (double h0, double altitude, double lat, double lon0)
{
/* if ellipsoid lat0 is geodetic latitude and must convert to geocentric latitude */

	double N1, phig_last;
	double R, H, P;
	double sphi1, cphi1, sphig, cphig;
	double a, e2, phig;

	GMT_LONG niter;

	a = project_info.EQ_RAD;
	e2 = project_info.ECC2;

	h0 *= 1e3;

	sincosd (lat, &sphi1, &cphi1);
	sphig = sphi1; cphig = cphi1;

	N1 = a/sqrt(1.0 - (e2*sphi1*sphi1));
	niter = 0;

	if (project_info.g_radius || altitude < -10.0) {
		/* use altitude as the radial distance from the center of the earth */
		H = fabs(altitude*1e3) - a;
		P = H/a + 1.0;
		phig = lat;
	}
	else if (altitude <= 0) {
		/* setup altitude of geosynchronous viewpoint n */
		double temp = 86164.1/TWO_PI;
		H = pow(3.98603e14*temp*temp, 0.3333) - a;
		P = H/a + 1.0;
		phig = lat - asind(N1*e2*sphi1*cphi1/(P*a));
		sincosd (phig, &sphig, &cphig);
		if (cphi1 != 0.0)
			H = P*a*cphig/cphi1 - N1 - h0;
		else
			H = P*a - N1 - h0;
	}
	else if (altitude < 10) {
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
		/* fprintf (stderr, "altitude %f\n", altitude); */
		H = altitude*1e3;
		/* need to setup P from iterating phig */
		phig = lat;
		do {
				niter++;
				sincosd (phig, &sphig, &cphig);
				P = (cphi1/cphig) * (H + N1 + h0)/a;
				phig_last = phig;
				phig = lat - asind(N1*e2*sphi1*cphi1/(P*a));
				/* fprintf (stderr, "%2d P %12.7f phig %12.7f\n", niter, P, phig); */
		}
		while (fabs(phig - phig_last) > 1e-9);
		sincosd (phig, &sphig, &cphig);
		P = (cphi1/cphig)*(H + N1 + h0)/a;
	}

	/* R = H/(P-1.0); */
	R = a;
	/* XXX Which one is it ? */

	project_info.g_H = H;
	project_info.g_P = P;
	project_info.g_R = R;
	project_info.g_lon0 = lon0;
	project_info.g_sphi1 = sphi1;
	project_info.g_cphi1 = cphi1;

	project_info.g_phig = phig;
	project_info.g_sphig = sphig;
	project_info.g_cphig = cphig;

	sincosd (lat-phig, &(project_info.g_sdphi), &(project_info.g_cdphi));

	project_info.g_L = 1.0 - e2*cphi1*cphi1;
	project_info.g_G = 1.0 - e2*sphi1*sphi1;
	project_info.g_J = 2.0*e2*sphi1*cphi1;
	project_info.g_B = P*project_info.g_cdphi;
	project_info.g_D = P*project_info.g_sdphi;
	project_info.g_BLH = -2.0*project_info.g_B*project_info.g_L*H;
	project_info.g_DG = 2.0*project_info.g_D*project_info.g_G;
	project_info.g_BJ = project_info.g_B*project_info.g_J;
	project_info.g_HJ = H*project_info.g_J;
	project_info.g_DHJ = project_info.g_D*project_info.g_HJ;
	project_info.g_LH2 = project_info.g_L*H*H;

	if (project_info.g_debug > 0) {
		fprintf (stderr, "a    %12.4f\n", a);
		fprintf (stderr, "R    %12.4f\n", R);
		fprintf (stderr, "e^2  %12.7f\n", e2);
		fprintf (stderr, "H    %12.4f\n", H);
		fprintf (stderr, "phi1 %12.4f\n", lat);
		fprintf (stderr, "lon0 %12.4f\n", lon0);
		fprintf (stderr, "h0   %12.4f\n", h0);
		fprintf (stderr, "N1   %12.1f\n", N1);
		fprintf (stderr, "P    %12.7f\n", P);
		fprintf (stderr, "phig %12.7f\n", phig);
	}

	return;
}

void GMT_vgenper (double lon0, double lat0, double altitude, double azimuth, double tilt, double twist, double width, double height)
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

	project_info.central_meridian = lon0;

	lat0_save = lat0;

	Req = R = project_info.EQ_RAD;
	Rpolar = Req * sqrt(project_info.one_m_ECC2);

	sincosd (lat0, &t2, &t1);

	t1 *= R;
	t12 = R*t1;
	t2 *= Rpolar;
	t22 = Rpolar*t2;

	Rlat0 = sqrt((t12*t12 + t22*t22)/(t1*t1 + t2*t2));

	lat0 = genper_getgeocentric (lat0, 0.0);
	sincosd (lat0, &(project_info.sinp), &(project_info.cosp));

	if (project_info.ECC2 != 0.0) {
		genper_setup (0.0, altitude, lat0_save, lon0);
		project_info.central_meridian = lon0;
		project_info.pole = project_info.g_phig;
	}
	else {
		project_info.pole = lat0;

		if (project_info.g_radius || (altitude < -10.0)) {
			/* use altitude as the radial distance from the center of the earth*/
			H = fabs(altitude*1e3) - R;
			P = H/R + 1.0;
		}
		else if (altitude <= 0) {
			/* compute altitude of geosynchronous viewpoint n*/
			double temp = 86164.1/TWO_PI;
			H = pow(3.98603e14*temp*temp, 0.3333) - R;
			P = H/R + 1.0;
		}
		else if (altitude < 10) {
			P = altitude;
			H = R * (P - 1.0);
		}
		else {
			H = altitude*1e3;
			P = H/R + 1.0;
		}
		project_info.g_R = R;
		project_info.g_H = H;
		project_info.g_P = P;
	}

	H = project_info.g_H;
	P = project_info.g_P;
	R = project_info.g_R;

	project_info.g_P_inverse = P > 0.0 ? 1.0/P : 1.0;

	if (project_info.g_longlat_set) {
		double norm_long = lon0;

		vp_lat = tilt;
		vp_long = azimuth;

		if (vp_lat == lat0_save && vp_long == lon0)
			tilt = azimuth = 0.0;
		else {
			if (project_info.g_debug > 0) {
				fprintf (stderr, " sensor point long %7.4f lat  %7.4f\n", lon0, lat0);
				fprintf (stderr, " input view point long %7.4f lat %7.4f\n", vp_long, vp_lat);
				fprintf (stderr, " input twist %7.4f\n", twist);
				fprintf (stderr, " altitude %f H %f R %f P %7.4f\n", altitude, H/1000.0, R/1000.0,P);
			}

			sincosd (90.0 - lat0, &sin_lat0, &cos_lat0);
			sincosd (90.0 - vp_lat, &sin_lat1, &cos_lat1);

			while (vp_long < 0) vp_long += 360.0;
			while (norm_long < 0) norm_long += 360.0;

			dlong  = vp_long - norm_long;
			if (dlong < -GMT_180) dlong += 360.0;

			cos_eca = cos_lat0*cos_lat1 + sin_lat0*sin_lat1*cosd(dlong);
			eca = d_acos(cos_eca);
			sin_eca = sin(eca);

			rho2 = P*P + 1.0 - 2.0*P*cos_eca;
			rho = sqrt(rho2);

			tilt = d_acosd((rho2 + P*P - 1.0)/(2.0*rho*P));

			azimuth = d_acosd((cos_lat1 - cos_lat0*cos_eca)/(sin_lat0*sin_eca));

			if (dlong < 0) azimuth = 360.0 - azimuth;

		}
		if (project_info.g_debug > 0) fprintf (stderr, "vgenper: pointing at longitude %10.4f latitude %10.4f\n           with computed tilt %5.2f azimuth %6.2f\n", vp_long, vp_lat, tilt, azimuth);

	}
	else if (project_info.g_debug > 1) {
		fprintf (stderr, " sensor point long %6.3f lat  %6.3f\n", lon0, lat0);
		fprintf (stderr, " input azimuth   %6.3f tilt %6.3f\n", azimuth, tilt);
		fprintf (stderr, " input twist %6.3f\n", twist);
	}

	if (tilt < 0.0) tilt = d_asind(project_info.g_P_inverse);

	sincosd (tilt, &(project_info.g_sin_tilt), &(project_info.g_cos_tilt));
	sincosd (twist, &(project_info.g_sin_twist), &(project_info.g_cos_twist));

	project_info.g_box = !(fabs(width) < GMT_SMALL);

	if (width != 0.0 && height == 0) height = width;
	if (height != 0.0 && width == 0) width = height;
	project_info.g_width = width/2.0;

	project_info.g_azimuth = azimuth;
	sincosd (azimuth, &(project_info.g_sin_azimuth), &(project_info.g_cos_azimuth));

	PP = sqrt((P - 1.0)/(P + 1.0));
	rmax = R*PP;
	rmax_min = Rpolar*PP;
	rmax_max = Req*PP;
	rmax_at_lat0 = Rlat0*PP;

	if (project_info.ECC2 != 0.0) rmax = rmax_at_lat0;

	kp = R*(P - 1.0) / (P - project_info.g_P_inverse);

	omega_max = d_acosd(project_info.g_P_inverse);
	project_info.g_rmax = rmax;

	/* project_info.f_horizon = project_info.g_P_inverse; */
	project_info.f_horizon = omega_max;
	/* XXX Which one is it ? */

	max_yt = 2.0*R*sind(0.5*omega_max);

	eccen = sind(tilt)/sqrt(1.0 - 1.0/(P*P));

	gamma = 180.0 - d_asind(project_info.g_sin_tilt * P);
	Omega = 180.0 - tilt - gamma;

	if (project_info.g_debug > 0) fprintf (stderr, "vgenper: tilt %6.3f sin_tilt %10.6f P %6.4f gamma %6.4f\n   Omega %6.4f eccen %10.4f\n", tilt, project_info.g_sin_tilt, P, gamma, Omega, eccen);

	if (eccen == 1.0) {
		max_yt = MIN (max_yt, rmax * 2.0);
		if (project_info.g_debug > 1) fprintf (stderr, "vgenper: Projected map is a parabola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n Plot truncated for projected distances > rmax %8.2f\n", tilt, omega_max, rmax/1000.0);
	}
	else if (eccen > 1.0) {
		if (width != 0.0) {
			if (project_info.g_debug > 1) fprintf (stderr, "vgenper: Projected map is a hyperbola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n", tilt, omega_max);
		}
		else {
			max_yt = MIN (max_yt, rmax * 2.0);
			if (project_info.g_debug > 1) fprintf (stderr, "vgenper: Projected map is a hyperbola with requested tilt %6.3f\n max ECA is %6.3f degrees.\n Plot truncated for projected distances > rmax %8.2f\n", tilt, omega_max, rmax/1000.0);
		}
	}
	else if (eccen > 0.5) {
		if (width != 0.0) {
			double t = sind(tilt), Pecc, maxecc = 0.5;
			Pecc = sqrt(1.0/(1.0 - (t*t/maxecc)));
			max_yt = R*sqrt((Pecc-1.0)/(Pecc+1.0));
			if (project_info.g_debug > 1) fprintf (stderr, "vgenper: Projected map is an enlongated ellipse (eccentricity of %6.4f) with requested tilt %6.3f\nwill truncate plot at rmax %8.2f\n", eccen, tilt, max_yt);
		}
		else {
			if (max_yt > rmax *2.0) max_yt = rmax * 2.0;
			if (project_info.g_debug > 1) fprintf (stderr, "vgenper: Projected map is an enlongated ellipse with requested tilt %6.3f\n eccentricity %6.3f\n Plot truncated for projected distances > rmax %8.2f\n", tilt, eccen, rmax/1000.0);
		}
	}

	project_info.g_max_yt = max_yt;

	sincosd (Omega, &sinOmega, &cosOmega);

	rho2 = P*P + 1.0 - 2.0*P*cosOmega;
	rho = sqrt(rho2);

	sinlatvp = project_info.sinp*cosOmega + project_info.cosp*sinOmega*project_info.g_cos_azimuth;
	latvp = d_asind(sinlatvp);
	coslatvp = sqrt (1.0 - sinlatvp*sinlatvp);

	lonvp = acosd((cosOmega - sinlatvp*project_info.sinp)/(project_info.cosp*coslatvp));
	if (azimuth > 180.0) lonvp = -lonvp;
	lonvp += lon0;

	if (gmtdefs.verbose) fprintf (stderr, "vgenper: pointing at longitude %10.4f latitude %10.4f\n          with tilt %5.2f azimuth %6.2f at distance %6.4f\n         with width %6.3f height %6.3f twist %6.2f\n", lonvp, latvp, tilt, azimuth, rho, width, height, twist);

	project_info.g_yoffset = 0.0;

	if (height != 0.0) {
		project_info.g_yoffset = project_info.g_sin_tilt * H ;
		xt_max = R * rho * sind(0.5*width);
		xt_min = -xt_max;
		yt_max = R * rho * sind(0.5*height);
		yt_min = -yt_max;
	}
	else {
		FILE *fp = NULL;
		xt_min = 1e20;
		xt_max = -xt_min;

		yt_min = 1e20;
		yt_max = -yt_min;

		if (project_info.g_debug > 2) {
			fp = fopen("g_border.txt", "w");

			fprintf (stderr, "tilt %10.4f sin_tilt %10.4f cos_tilt %10.4f\n", tilt, project_info.g_sin_tilt, project_info.g_cos_tilt);
			fprintf (stderr, "azimuth %10.4f sin_azimuth %10.4f cos_azimuth %10.4f\n", azimuth, project_info.g_sin_azimuth, project_info.g_cos_azimuth);
		}

		for (az = 0 ; az < 360 ; az++) {
			sincosd (az, &x, &y);
			x *= rmax;
			y *= rmax;
			genper_to_xtyt ((double)az, x, y, project_info.g_yoffset, &xt, &yt);

			if (project_info.g_debug > 2) fprintf (fp,"%3ld x %10.2f y %10.2f xt %10.3f yt %10.3f\n", az, x/1000, y/1000, xt/1000, yt/1000);

			xt_min = MIN (xt, xt_min);
			xt_max = MAX (xt, xt_max);
			yt_min = MIN (yt, yt_min);
			yt_max = MAX (yt, yt_max);
		}
		if (project_info.g_debug > 2) fclose(fp);
		if (eccen > 0.5) {
			project_info.g_width = atand(2.0*rmax/H);
			height = width = 2.0 * project_info.g_width;
			xt_max = yt_max = R * rho * sind(project_info.g_width);
			xt_min = yt_min = -xt_max;
		}
	}
	if (project_info.g_debug > 1) {
		fprintf (stderr, "vgenper: xt max %7.1f km\n", xt_max/1000.0);
		fprintf (stderr, "vgenper: xt min %7.1f km\n", xt_min/1000.0);
		fprintf (stderr, "vgenper: yt max %7.1f km\n", yt_max/1000.0);
		fprintf (stderr, "vgenper: yt min %7.1f km\n", yt_min/1000.0);
	}

	project_info.g_xmin = xt_min;
	project_info.g_xmax = xt_max;
	project_info.g_ymin = yt_min;
	project_info.g_ymax = yt_max;

	if (width != 0.0) project_info.y_scale = project_info.x_scale/width*height;

	if (project_info.g_debug > 0) {
		GMT_genper (lonvp, latvp, &xt_vp, &yt_vp);
		fprintf (stderr, "\nvgenper: polar %ld north %ld\n", project_info.polar, project_info.north_pole);
		fprintf (stderr, "vgenper: altitude H %7.1f km P %7.4f\n", H/1000.0, P);
		fprintf (stderr, "vgenper: azimuth %5.1f tilt %5.1f\n", azimuth, tilt);
		fprintf (stderr, "vgenper: viewpoint width %5.1f height %5.1f degrees\n", width, height);
		fprintf (stderr, "vgenper: radius max %7.1f km\n", project_info.g_rmax/1000.0);
		fprintf (stderr, "vgenper: eccentricity %7.4f km\n", eccen);
		fprintf (stderr, "vgenper: eq radius max %7.1f km\n", rmax_max/1000.0);
		fprintf (stderr, "vgenper: polar radius max %7.1f km\n", rmax_min/1000.0);
		fprintf (stderr, "vgenper: lat0 radius max %7.1f km\n", rmax_at_lat0/1000.0);
		fprintf (stderr, "vgenper: kp %7.1f \n", kp/1000.0);
		fprintf (stderr, "vgenper: y offset %7.1f km\n", project_info.g_yoffset/1000.0);
		fprintf (stderr, "vgenper: yt max %7.1f km\n", yt_max/1000.0);
		fprintf (stderr, "vgenper: yt min %7.1f km\n", yt_min/1000.0);
		fprintf (stderr, "vgenper: y max %7.1f km\n", project_info.g_ymax/1000.0);
		fprintf (stderr, "vgenper: y min %7.1f km\n", project_info.g_ymin/1000.0);
		fprintf (stderr, "vgenper: x max %7.1f km\n", project_info.g_xmax/1000.0);
		fprintf (stderr, "vgenper: x min %7.1f km\n", project_info.g_xmin/1000.0);
		fprintf (stderr, "vgenper: omega max %6.2f degrees\n", omega_max);
		fprintf (stderr, "vgenper: gamma %6.3f Omega %6.3f \n", gamma, Omega);
		fprintf (stderr, "vgenper: viewpoint lon %6.3f lat %6.3f \n", lonvp, latvp);
		fprintf (stderr, "vgenper: viewpoint xt %6.3f yt %6.3f \n", xt_vp/1000.0, yt_vp/1000.0);
		fprintf (stderr, "vgenper: user viewpoint %ld\n", project_info.g_box);
	}

}

/* Convert lon/lat to General Perspective x/y */

void GMT_genper (double lon, double lat, double *xt, double *yt)
{
	double dlon, sin_lat, cos_lat, sin_dlon, cos_dlon;
	double cosc, sinc;
	double x, y, kp;
	double angle;

	dlon = lon - project_info.central_meridian;
	while (dlon < -GMT_180) dlon += 360.0;
	while (dlon > 180.0) dlon -= 360.0;
	dlon *= D2R;

	lat = genper_getgeocentric (lat, 0.0);

	sincosd (lat, &sin_lat, &cos_lat);
	sincos (dlon, &sin_dlon, &cos_dlon);

	cosc = project_info.sinp * sin_lat + project_info.cosp * cos_lat * cos_dlon;
	sinc = d_sqrt(1.0 - cosc * cosc);

	project_info.g_outside = FALSE;

	angle = M_PI - dlon;
	if (cosc < project_info.g_P_inverse) { /* over the horizon */
		project_info.g_outside = TRUE;

		if (project_info.polar)
			angle = M_PI - dlon;
		else if (project_info.cosp*sinc != 0.0) {
			angle = d_acos((sin_lat - project_info.sinp*cosc)/(project_info.cosp*sinc));
			if (dlon < 0.0) angle = -angle;
		}
		else
			angle = 0.0;

		sincos (angle, &x, &y);
		x *= project_info.g_rmax;
		y *= project_info.g_rmax;
		angle *= R2D;
	}
	else if (project_info.ECC2 != 0.0) { /* within field of view, ellipsoidal earth */
		genper_toxy (lat, lon, 0.0, &x, &y);
		/* angle = project_info.g_azimuth; */
		angle = atan2d(x, y);
		/* XXX Which one is it? Forgotten R2D. Switched x and y. */
	}
	else { /* within field of view, spherical earth */
		kp = project_info.g_R * (project_info.g_P - 1.0) / (project_info.g_P - cosc);
		x = kp * cos_lat * sin_dlon;
		y = kp * (project_info.cosp * sin_lat - project_info.sinp * cos_lat * cos_dlon);
		/* angle = project_info.g_azimuth; */
		angle = atan2d(x, y);
		/* XXX Which one is it? Forgotten R2D. Switched x and y. */
	}

	genper_to_xtyt (angle, x, y, project_info.g_yoffset, xt, yt);

	if (GMT_is_dnan(*yt) || GMT_is_dnan(*xt)) {
		fprintf (stderr, "genper: yt or xt nan\n");
		fprintf (stderr, "genper: lon %6.3f lat %6.3f\n", lon, lat);
		fprintf (stderr, "genper: xt %10.3e yt %10.3e\n", *xt, *yt);
		GMT_exit(1);
	}
}

/* Convert General Perspective x/y to lon/lat */

void GMT_igenper (double *lon, double *lat, double xt, double yt)
{
	double H, P, R;
	double sin_c, cos_c;
	double x, y;
	double M, Q;
	double con, com, rho;

	H = project_info.g_H;
	R = project_info.g_R;
	P = project_info.g_P;

	x = xt;
	y = yt;

	xt = (x * project_info.g_cos_twist + y * project_info.g_sin_twist);
	yt = (y * project_info.g_cos_twist - x * project_info.g_sin_twist);
	yt += project_info.g_yoffset;

	M = H * xt / (H - yt * project_info.g_sin_tilt);
	Q = H * yt * project_info.g_cos_tilt /(H - yt * project_info.g_sin_tilt);
	x = M * project_info.g_cos_azimuth + Q * project_info.g_sin_azimuth;
	y = Q * project_info.g_cos_azimuth - M * project_info.g_sin_azimuth;

	rho = hypot(x, y);

	project_info.g_outside = FALSE;

	if (rho < GMT_SMALL) {
		*lat = project_info.pole;
		*lon = project_info.central_meridian;
		return;
	}
	if (rho > project_info.g_rmax) {
		x *= project_info.g_rmax/rho;
		y *= project_info.g_rmax/rho;
		rho = project_info.g_rmax;
		project_info.g_outside = TRUE;
	}

	con = P - 1.0;
	com = P + 1.0;

	if (project_info.ECC2 != 0.0)
		genper_tolatlong (x, y, 0.0, lat, lon);
	else {
		sin_c = (P - d_sqrt(1.0 - (rho * rho * com)/(R*R*con))) / (R * con / rho + rho/(R*con));
		cos_c = d_sqrt(1.0 - sin_c * sin_c);
		sin_c /= rho;
		*lat = d_asind (cos_c * project_info.sinp + y * sin_c * project_info.cosp);
		*lon = d_atan2d (x * sin_c, cos_c * project_info.cosp - y * sin_c * project_info.sinp) + project_info.central_meridian;
	}
	return;
}

GMT_LONG GMT_genper_overlap (double lon0, double lat0, double lon1, double lat1)
{
/* Dummy routine */
	if (project_info.g_debug > 0) fprintf (stderr, "genper_overlap: overlap called\n");
	return (TRUE);
}

GMT_LONG GMT_genper_map_clip_path (GMT_LONG np, double *work_x, double *work_y)
{
	GMT_LONG i;
	double da, angle;
	double x, y, xt, yt;

	if (project_info.g_debug > 0) {
		fprintf (stderr, "\n\ngenper_map_clip_path: np %ld\n", np);
		fprintf (stderr, " x_scale %e y_scale %e, x0 %e y0 %e\n", project_info.x_scale, project_info.y_scale, project_info.x0, project_info.y0);
	}

	da = TWO_PI/(np-1);

	for (i = 0; i < np; i++) {
		angle = i * da;
		sincos (angle, &x, &y);
		x *= project_info.g_rmax;
		y *= project_info.g_rmax;

		/* XXX forgotten R2D */
		genper_to_xtyt (angle*R2D, x, y, project_info.g_yoffset, &xt, &yt);

		if (project_info.g_width != 0.0) {
			xt = MAX (project_info.g_xmin, MIN (xt, project_info.g_xmax));
			yt = MAX (project_info.g_ymin, MIN (yt, project_info.g_ymax));
		}
		work_x[i] = xt * project_info.x_scale + project_info.x0;
		work_y[i] = yt * project_info.y_scale + project_info.y0;
	}
	return 0;
}

/* -JG ORTHOGRAPHIC PROJECTION */

void GMT_vortho (double lon0, double lat0, double horizon)
{
	/* Set up Orthographic projection */

	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	sincosd (lat0, &(project_info.sinp), &(project_info.cosp));
	project_info.f_horizon = horizon;
	project_info.rho_max = sind (horizon);
}

void GMT_ortho (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Orthographic x/y */
	double sin_lat, cos_lat, sin_lon, cos_lon;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);
	*x = project_info.EQ_RAD * cos_lat * sin_lon;
	*y = project_info.EQ_RAD * (project_info.cosp * sin_lat - project_info.sinp * cos_lat * cos_lon);
}

void GMT_iortho (double *lon, double *lat, double x, double y)
{
	/* Convert Orthographic x/y to lon/lat */
	double rho, cos_c;

	x *= project_info.i_EQ_RAD;
	y *= project_info.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > project_info.rho_max) {
		*lat = *lon = GMT_d_NaN;
		return;
	}
	cos_c = sqrt (1.0 - rho * rho);	/* Produces NaN for rho > 1: beyond horizon */
	*lat = d_asind (cos_c * project_info.sinp + y * project_info.cosp);
	*lon = d_atan2d (x, cos_c * project_info.cosp - y * project_info.sinp) + project_info.central_meridian;
}

/* -JF GNOMONIC PROJECTION */

void GMT_vgnomonic (double lon0, double lat0, double horizon)
{
	/* Set up Gnomonic projection */

	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.f_horizon = horizon;
	project_info.rho_max = tand (project_info.f_horizon);
	project_info.pole = lat0;
	project_info.north_pole = (lat0 > 0.0);
	sincosd (lat0, &(project_info.sinp), &(project_info.cosp));
}

void GMT_gnomonic (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Gnomonic x/y */
	double k, sin_lat, cos_lat, sin_lon, cos_lon, cc;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	sincosd (lat, &sin_lat, &cos_lat);
	sincosd (lon, &sin_lon, &cos_lon);

	cc = cos_lat * cos_lon;
	k =  project_info.EQ_RAD / (project_info.sinp * sin_lat + project_info.cosp * cc);
	*x = k * cos_lat * sin_lon;
	*y = k * (project_info.cosp * sin_lat - project_info.sinp * cc);
}

void GMT_ignomonic (double *lon, double *lat, double x, double y)
{
	/* Convert Gnomonic x/y to lon/lat */
	double rho, c;

	x *= project_info.i_EQ_RAD;
	y *= project_info.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > project_info.rho_max) {
		*lat = *lon = GMT_d_NaN;
		return;
	}
	c = atan (rho);
	*lat = d_asind (cos(c) * (project_info.sinp + y * project_info.cosp));
	*lon = d_atan2d (x, project_info.cosp - y * project_info.sinp) + project_info.central_meridian;
}

/* -JE AZIMUTHAL EQUIDISTANT PROJECTION */

void GMT_vazeqdist (double lon0, double lat0, double horizon)
{
	/* Set up azimuthal equidistant projection */

	/* GMT_check_R_J (&lon0); */
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	sincosd (lat0, &(project_info.sinp), &(project_info.cosp));
	project_info.f_horizon = horizon;
	project_info.rho_max = horizon * D2R;
}

void GMT_azeqdist (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to azimuthal equidistant x/y */
	double k, cc, c, clat, slon, clon, slat, t;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lat, &slat, &clat);
	sincosd (lon, &slon, &clon);

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
	/* Convert azimuthal equidistant x/y to lon/lat */
	double rho, sin_c, cos_c;

	x *= project_info.i_EQ_RAD;
	y *= project_info.i_EQ_RAD;
	rho = hypot (x, y);
	if (rho > project_info.f_horizon * D2R) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	sincos (rho, &sin_c, &cos_c);
	if (rho != 0.0) sin_c /= rho;
	/* Since in the rest we only have sin_c in combination with x or y, it is not necessary to set
	   sin_c = 1 when rho == 0. In that case x*sin_c and y*sin_c or 0 anyhow. */
	*lat = d_asind (cos_c * project_info.sinp + y * sin_c * project_info.cosp);
	*lon = d_atan2d (x * sin_c, cos_c * project_info.cosp - y * sin_c * project_info.sinp) + project_info.central_meridian;
}

/* -JW MOLLWEIDE EQUAL AREA PROJECTION */

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
	GMT_LONG i;
	double phi, delta, psin_lat, c, s;

	if (GMT_IS_ZERO (fabs (lat) - 90.0)) {	/* Special case */
		*x = 0.0;
		*y = copysign (project_info.w_y, lat);
		return;
	}

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
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

	phi = asin (y * project_info.w_iy);
	*lon = x / (project_info.w_x * cos(phi));
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	phi2 = 2.0 * phi;
	*lat = asind ((phi2 + sin (phi2)) / M_PI);
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/* -JH HAMMER-AITOFF EQUAL AREA PROJECTION */

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

	if (GMT_IS_ZERO (fabs (lat) - 90.0)) {	/* Save time */
		*x = 0.0;
		*y = M_SQRT2 * copysign (project_info.EQ_RAD, lat);
		return;
	}

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	sincosd (lat, &slat, &clat);
	sincosd (0.5 * lon, &slon, &clon);

	D = project_info.EQ_RAD * sqrt (2.0 / (1.0 + clat * clon));
	*x = 2.0 * D * clat * slon;
	*y = D * slat;
}

void GMT_ihammer (double *lon, double *lat, double x, double y)
{
	/* Convert Hammer_Aitoff Equal-Area x/y to lon/lat */
	double rho, a, sin_c, cos_c;

	x *= 0.5;
	rho = hypot (x, y);
	a = 0.5 * rho * project_info.i_EQ_RAD;			/* a = sin(c/2)		*/
	a *= a;							/* a = sin(c/2)**2	*/
	cos_c = 1.0 - 2.0 * a;					/* cos_c = cos(c)	*/
	if (cos_c < 0.0) {					/* Horizon		*/
		*lat = *lon = GMT_d_NaN;
		return;
	}
	sin_c = sqrt (1.0 - a) * project_info.i_EQ_RAD;		/* sin_c = sin(c)/rho	*/
	*lat = asind (y * sin_c);
	*lon = 2.0 * d_atan2d (x * sin_c, cos_c) + project_info.central_meridian;
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/* -JV VAN DER GRINTEN PROJECTION */

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
	if (GMT_IS_ZERO (lon - project_info.central_meridian)) {	/* Save time */
		theta = d_asin (2.0 * fabs (lat) / 180.0);
		*x = 0.0;
		*y = M_PI * copysign (project_info.EQ_RAD, lat) * tan (0.5 * theta);
		return;
	}

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (GMT_IS_ZERO (flat)) {	/* Save time */
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
	if (x2y2 > 1.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
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
	*lon = project_info.central_meridian;
	if (x != 0.0) *lon += 90.0 * (x2y2 - 1.0 + sqrt (1.0 + 2 * (x2 - y2) + p)) / x;
}

/* -JR WINKEL-TRIPEL MODIFIED GMT_IS_AZIMUTHAL PROJECTION */

void GMT_vwinkel (double lon0, double scale)
{
	/* Set up Winkel Tripel projection */

	GMT_check_R_J (&lon0);
	project_info.r_cosphi1 = cosd (50.0+(28.0/60.0));
	project_info.central_meridian = lon0;
	project_info.w_r = 0.25 * (scale * project_info.M_PR_DEG * 360.0);	/* = Half the minor axis */
}

void GMT_winkel (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Winkel Tripel x/y */
	double C, D, x1, x2, y1, c, s;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	lat *= D2R;
	lon *= (0.5 * D2R);

	/* Fist find Aitoff x/y */

	sincos (lat, &s, &c);
	D = d_acos (c * cos (lon));
	if (GMT_IS_ZERO (D))
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
	/* Based on iterative solution published by:
	 * Ipbuker, 2002, Cartography & Geographical Information Science, 20, 1, 37-42.
	 */

	GMT_LONG n_iter = 0;
	double phi0, lambda0, sp, cp, s2p, sl, cl, sl2, cl2, C, D, sq_C, C_32;
	double f1, f2, df1dp, df1dl, df2dp, df2dl, denom, delta;

	x *= project_info.i_EQ_RAD;
	y *= project_info.i_EQ_RAD;
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
		f1 = 0.5 * ((2.0 * D * cp * sl2) / sq_C + lambda0 * project_info.r_cosphi1) - x;
		f2 = 0.5 * (D * sp / sq_C + phi0) - y;
		df1dp = (0.25 * (sl * s2p) / C) - ((D * sp * sl2) / C_32);
		df1dl = 0.5 * ((cp * cp * sl2 * sl2) / C + (D * cp * cl2 * sp * sp) / C_32 + project_info.r_cosphi1);
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
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	*lat *= R2D;
}

void GMT_iwinkel_sub (double y, double *phi)
{	/* Valid only along meridian 180 degree from central meridian.  Used in left/right_winkel only */
	GMT_LONG n_iter = 0;
	double c, phi0, delta, sp, cp;

	c = 2.0 * y * project_info.i_EQ_RAD;
	*phi = y * project_info.i_EQ_RAD;
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

double GMT_left_winkel (double y)
{
	double c, phi, x;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	GMT_iwinkel_sub (y, &phi);
	GMT_geo_to_xy (project_info.central_meridian - 180.0, phi, &x, &c);
	return (x);
}

double GMT_right_winkel (double y)
{
	double c, phi, x;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	GMT_iwinkel_sub (y, &phi);
	GMT_geo_to_xy (project_info.central_meridian + 180.0, phi, &x, &c);
	return (x);
}

/* -JKf ECKERT IV PROJECTION */

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
	GMT_LONG n_iter = 0;
	double phi, delta, s_lat, s, c;
	/* Convert lon/lat to Eckert IV x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
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
	*x = project_info.k4_x * lon * D2R * (1.0 + c);
	*y = project_info.k4_y * s;
}

void GMT_ieckert4 (double *lon, double *lat, double x, double y)
{
	/* Convert Eckert IV x/y to lon/lat */

	double phi, s, c;

	s = y * project_info.k4_iy;
	phi = d_asin (s);
	c = cos (phi);
	*lon = R2D * x * project_info.k4_ix / (1.0 + c);
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	*lat = d_asind ((phi + s * c + 2.0 * s) / (2.0 + M_PI_2));
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

double GMT_right_eckert4 (double y)
{
	double x, phi;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = d_asin (y * project_info.k4_iy);
	x = project_info.k4_x * D2R * (project_info.e - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

/* -JKs ECKERT VI PROJECTION */

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
	GMT_LONG n_iter = 0;
	double phi, delta, s_lat, s, c;
	/* Convert lon/lat to Eckert VI x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);
	phi = lat * D2R;
	s_lat = sin (phi);
	do {
		sincos (phi, &s, &c);
		delta = -(phi + s - (1.0 + M_PI_2) * s_lat) / (1.0 + c);
		phi += delta;
	}
	while (fabs(delta) > GMT_CONV_LIMIT && n_iter < 100);

	*x = project_info.k6_r * lon * D2R * (1.0 + cos (phi));
	*y = 2.0 * project_info.k6_r * phi;
}

void GMT_ieckert6 (double *lon, double *lat, double x, double y)
{
	/* Convert Eckert VI x/y to lon/lat */

	double phi, c, s;

	phi = 0.5 * y * project_info.k6_ir;
	sincos (phi, &s, &c);
	*lon = R2D * x * project_info.k6_ir / (1.0 + c);
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	*lat = d_asin ((phi + s) / (1.0 + M_PI_2)) * R2D;
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
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

double GMT_right_eckert6 (double y)
{
	double x, phi;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	phi = 0.5 * y * project_info.k6_ir;
	x = project_info.k6_r * D2R * (project_info.e - project_info.central_meridian) * (1.0 + cos (phi));
	return (x * project_info.x_scale + project_info.x0);
}

/* -JN ROBINSON PSEUDOCYLINDRICAL PROJECTION */

void GMT_vrobinson (double lon0)
{
	GMT_LONG err_flag;

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
	project_info.n_x_coeff  = (double *) GMT_memory (VNULL, (size_t)(3 * GMT_N_ROBINSON), sizeof (double), GMT_program);
	project_info.n_y_coeff  = (double *) GMT_memory (VNULL, (size_t)(3 * GMT_N_ROBINSON), sizeof (double), GMT_program);
	project_info.n_iy_coeff = (double *) GMT_memory (VNULL, (size_t)(3 * GMT_N_ROBINSON), sizeof (double), GMT_program);
	if (gmtdefs.interpolant == 2) {	/* Natural cubic spline */
		err_flag = GMT_cspline (project_info.n_phi, project_info.n_X, (GMT_LONG)GMT_N_ROBINSON, project_info.n_x_coeff);
		err_flag = GMT_cspline (project_info.n_phi, project_info.n_Y, (GMT_LONG)GMT_N_ROBINSON, project_info.n_y_coeff);
		err_flag = GMT_cspline (project_info.n_Y, project_info.n_phi, (GMT_LONG)GMT_N_ROBINSON, project_info.n_iy_coeff);
	}
	else {	/* Akimas spline */
		err_flag = GMT_akima (project_info.n_phi, project_info.n_X, (GMT_LONG)GMT_N_ROBINSON, project_info.n_x_coeff);
		err_flag = GMT_akima (project_info.n_phi, project_info.n_Y, (GMT_LONG)GMT_N_ROBINSON, project_info.n_y_coeff);
		err_flag = GMT_akima (project_info.n_Y, project_info.n_phi, (GMT_LONG)GMT_N_ROBINSON, project_info.n_iy_coeff);
	}
}

void GMT_robinson (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Robinson x/y */
	double phi, X, Y;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	phi = fabs (lat);

	X = GMT_robinson_spline (phi, project_info.n_phi, project_info.n_X, project_info.n_x_coeff);
	Y = GMT_robinson_spline (phi, project_info.n_phi, project_info.n_Y, project_info.n_y_coeff);
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
	*lon = x / (project_info.n_cx * X);
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	if (y < 0.0) *lat = -(*lat);
}

double GMT_robinson_spline (double xp, double *x, double *y, double *c) {
	/* Compute the interpolated values from the Robinson coefficients */

	GMT_LONG j = 0, j1;
	double yp, a, b, h, ih, dx;

	if (xp < x[0] || xp > x[GMT_N_ROBINSON-1])	/* Desired point outside data range */
		return (GMT_d_NaN);

	while (j < GMT_N_ROBINSON && x[j] <= xp) j++;
	if (j == GMT_N_ROBINSON) j--;
	if (j > 0) j--;

	dx = xp - x[j];
	switch (gmtdefs.interpolant) {	/* GMT_vrobinson would not allow case 0 so only 1 | 2 is possible */
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

double GMT_left_robinson (double y)
{
	double x, X, Y;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	Y = fabs (y * project_info.n_i_cy);
	if (GMT_intpol (project_info.n_Y, project_info.n_X, (GMT_LONG)19, (GMT_LONG)1, &Y, &X, gmtdefs.interpolant)) {
		fprintf (stderr, "GMT Internal error in GMT_left_robinson!\n");
		GMT_exit (EXIT_FAILURE);
	}

	x = project_info.n_cx * X * (project_info.w - project_info.central_meridian);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_robinson (double y)
{
	double x, X, Y;

	y -= project_info.y0;
	y *= project_info.i_y_scale;
	Y = fabs (y * project_info.n_i_cy);
	if (GMT_intpol (project_info.n_Y, project_info.n_X, (GMT_LONG)19, (GMT_LONG)1, &Y, &X, gmtdefs.interpolant)) {
		fprintf (stderr, "GMT Internal error in GMT_right_robinson!\n");
		GMT_exit (EXIT_FAILURE);
	}

	x = project_info.n_cx * X * (project_info.e - project_info.central_meridian);
	return (x * project_info.x_scale + project_info.x0);
}

/* -JI SINUSOIDAL EQUAL AREA PROJECTION */

void GMT_vsinusoidal (double lon0)
{
	/* Set up Sinusoidal projection */

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
}

void GMT_sinusoidal (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Sinusoidal Equal-Area x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);

	lat *= D2R;

	*x = project_info.EQ_RAD * lon * D2R * cos (lat);
	*y = project_info.EQ_RAD * lat;
}

void GMT_isinusoidal (double *lon, double *lat, double x, double y)
{
	/* Convert Sinusoidal Equal-Area x/y to lon/lat */

	*lat = y * project_info.i_EQ_RAD;
	*lon = (GMT_IS_ZERO (fabs (*lat) - M_PI)) ? 0.0 : R2D * x / (project_info.EQ_RAD * cos (*lat));
	if (fabs (*lon) > 180.0) {	/* Horizon */
		*lat = *lon = GMT_d_NaN;
		return;
	}
	*lon += project_info.central_meridian;
	*lat *= R2D;
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
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

double GMT_right_sinusoidal (double y)
{
	double x, lat;
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	lat = y * project_info.i_EQ_RAD;
	x = project_info.EQ_RAD * (project_info.e - project_info.central_meridian) * D2R * cos (lat);
	return (x * project_info.x_scale + project_info.x0);
}

/* -JC CASSINI PROJECTION */

void GMT_vcassini (double lon0, double lat0)
{
	/* Set up Cassini projection */

	double e1, s2, c2;

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
	project_info.c_p = lat0 * D2R;
	sincos (2.0 * project_info.c_p, &s2, &c2);

	e1 = (1.0 - d_sqrt (project_info.one_m_ECC2)) / (1.0 + d_sqrt (project_info.one_m_ECC2));

	project_info.c_c1 = 1.0 - (1.0/4.0) * project_info.ECC2 - (3.0/64.0) * project_info.ECC4 - (5.0/256.0) * project_info.ECC6;
	project_info.c_c2 = -((3.0/8.0) * project_info.ECC2 + (3.0/32.0) * project_info.ECC4 + (25.0/768.0) * project_info.ECC6);
	project_info.c_c3 = (15.0/128.0) * project_info.ECC4 + (45.0/512.0) * project_info.ECC6;
	project_info.c_c4 = -(35.0/768.0) * project_info.ECC6;
	project_info.c_M0 = project_info.EQ_RAD * (project_info.c_c1 * project_info.c_p + s2 * (project_info.c_c2 + c2 * (project_info.c_c3 + c2 * project_info.c_c4)));
	project_info.c_i1 = 1.0 / (project_info.EQ_RAD * project_info.c_c1);
	project_info.c_i2 = (3.0/2.0) * e1 - (29.0/12.0) * pow (e1, 3.0);
	project_info.c_i3 = (21.0/8.0) * e1 * e1 - (1537.0/128.0) * pow (e1, 4.0);
	project_info.c_i4 = (151.0/24.0) * pow (e1, 3.0);
	project_info.c_i5 = (1097.0/64.0) * pow (e1, 4.0);
}

void GMT_cassini (double lon, double lat, double *x, double *y)
{
	/* Convert lon/lat to Cassini x/y */

	double tany, N, T, A, C, M, s, c, s2, c2, A2, A3;

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */
	lon *= D2R;

	if (GMT_IS_ZERO (lat)) {	/* Quick when lat is zero */
		*x = project_info.EQ_RAD * lon;
		*y = -project_info.c_M0;
		return;
	}

	lat *= D2R;
	sincos (lat, &s, &c);
	sincos (2.0 * lat, &s2, &c2);
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
	if (GMT_IS_ZERO (fabs (phi1) - M_PI_2)) {
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

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	sincosd (lon, &slon, &clon);
	sincosd (lat, &slat, &clat);
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
	*lat = d_asind (sD * cx);
	*lon = project_info.central_meridian + atand (tx / cD);
}

/* -JB ALBERS EQUAL-AREA CONIC PROJECTION */

void GMT_valbers (double lon0, double lat0, double ph1, double ph2)
{
	/* Set up Albers projection */

	double s0, s1, s2, c1, c2, q0, q1, q2, m1, m2;

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.north_pole = (lat0 > 0.0);
	project_info.pole = (project_info.north_pole) ? 90.0 : -90.0;

	s0 = sind (lat0);
	sincosd (ph1, &s1, &c1);
	sincosd (ph2, &s2, &c2);

	m1 = c1 * c1 / (1.0 - project_info.ECC2 * s1 * s1);	/* Actually m1 and m2 squared */
	m2 = c2 * c2 / (1.0 - project_info.ECC2 * s2 * s2);
	q0 = (GMT_IS_ZERO (project_info.ECC)) ? 2.0 * s0 : project_info.one_m_ECC2 * (s0 / (1.0 - project_info.ECC2 * s0 * s0) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s0) / (1.0 + project_info.ECC * s0)));
	q1 = (GMT_IS_ZERO (project_info.ECC)) ? 2.0 * s1 : project_info.one_m_ECC2 * (s1 / (1.0 - project_info.ECC2 * s1 * s1) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s1) / (1.0 + project_info.ECC * s1)));
	q2 = (GMT_IS_ZERO (project_info.ECC)) ? 2.0 * s2 : project_info.one_m_ECC2 * (s2 / (1.0 - project_info.ECC2 * s2 * s2) - project_info.i_half_ECC * log ((1.0 - project_info.ECC * s2) / (1.0 + project_info.ECC * s2)));

	project_info.a_n = (GMT_IS_ZERO (ph1 - ph2)) ? s1 : (m1 - m2) / (q2 - q1);
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

	sincosd (ph1, &s1, &c1);

	project_info.a_n = 0.5 * (s1 + sind (ph2));
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

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	s = sind (lat);
	if (GMT_IS_ZERO (project_info.ECC))
		q = 2.0 * s;
	else {
		r = project_info.ECC * s;
		q = project_info.one_m_ECC2 * (s / (1.0 - project_info.ECC2 * s * s) - project_info.i_half_ECC * log ((1.0 - r) / (1.0 + r)));
	}
	theta = project_info.a_n * lon * D2R;
	rho = project_info.EQ_RAD * sqrt (project_info.a_C - project_info.a_n * q) * project_info.a_i_n;

	sincos (theta, &s, &c);
	*x = rho * s;
	*y = project_info.a_rho0 - rho * c;
}

void GMT_ialbers (double *lon, double *lat, double x, double y)
{
	/* Convert Albers x/y to lon/lat */

	GMT_LONG n_iter;
	double theta, rho, q, phi, phi0, s, c, s2, ex_1, delta, r;

	theta = (project_info.a_n < 0.0) ? d_atan2 (-x, y - project_info.a_rho0) : d_atan2 (x, project_info.a_rho0 - y);
	rho = hypot (x, project_info.a_rho0 - y);
	q = (project_info.a_C - rho * rho * project_info.a_n2ir2) * project_info.a_i_n;

	if (GMT_IS_ZERO (fabs (q) - project_info.a_test))
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

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (project_info.GMT_convert_latitudes) lat = GMT_latg_to_lata (lat);

	theta = project_info.a_n * lon * D2R;
	rho = project_info.EQ_RAD * sqrt (project_info.a_C - 2.0 * project_info.a_n * sind (lat)) * project_info.a_i_n;

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

	*lat = d_asind (project_info.a_Cin - A);
	*lon = project_info.central_meridian + R2D * theta * project_info.a_i_n;
	if (project_info.GMT_convert_latitudes) *lat = GMT_lata_to_latg (*lat);
}

/* -JD EQUIDISTANT CONIC PROJECTION */

void GMT_veconic (double lon0, double lat0, double lat1, double lat2)
{
	double c1;

	/* Set up Equidistant Conic projection */

	GMT_check_R_J (&lon0);
	project_info.north_pole = (lat0 > 0.0);
	c1 = cosd (lat1);
	project_info.d_n = (GMT_IS_ZERO (lat1 - lat2)) ? sind (lat1) : (c1 - cosd (lat2)) / (D2R * (lat2 - lat1));
	project_info.d_i_n = R2D / project_info.d_n;	/* R2D put here instead of in lon for ieconic */
	project_info.d_G = (c1 / project_info.d_n) + lat1 * D2R;
	project_info.d_rho0 = project_info.EQ_RAD * (project_info.d_G - lat0 * D2R);
	project_info.central_meridian = lon0;
}

void GMT_econic (double lon, double lat, double *x, double *y)
{
	double rho, theta, s, c;
	/* Convert lon/lat to Equidistant Conic x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	rho = project_info.EQ_RAD * (project_info.d_G - lat * D2R);
	theta = project_info.d_n * lon * D2R;

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

/* -JPoly POLYCONIC PROJECTION */

void GMT_vpolyconic (double lon0, double lat0)
{
	/* Set up Polyconic projection */

	GMT_check_R_J (&lon0);
	project_info.central_meridian = lon0;
	project_info.pole = lat0;
}

void GMT_polyconic (double lon, double lat, double *x, double *y)
{
	double sE, cE, sp, cp;
	/* Convert lon/lat to Polyconic x/y */

	GMT_WIND_LON(lon)	/* Remove central meridian and place lon in -180/+180 range */

	if (GMT_IS_ZERO(lat)) {
		*x = project_info.EQ_RAD * lon * D2R;
		*y = project_info.EQ_RAD * (lat - project_info.pole) * D2R;
	}
	else {
		sincosd(lat, &sp, &cp);
		sincosd(lon * sp, &sE, &cE);
		cp /= sp;	/* = cot(phi) */
		*x = project_info.EQ_RAD * cp * sE;
		*y = project_info.EQ_RAD * ((lat - project_info.pole) * D2R + cp * (1.0 - cE));
	}
}

void GMT_ipolyconic (double *lon, double *lat, double x, double y)
{
	/* Convert Polyconic x/y to lon/lat */

	double B, phi, phi0, tanp, delta;
	GMT_LONG n_iter = 0;

	x *= project_info.i_EQ_RAD;
	y *= project_info.i_EQ_RAD;
	y += project_info.pole * D2R;
	if (GMT_IS_ZERO(y)) {
		*lat = y * R2D + project_info.pole;
		*lon = x * R2D + project_info.central_meridian;
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
		*lon = project_info.central_meridian + asind(x * tanp) / sin(phi);
	}
}

void GMT_ipolyconic_sub (double y, double lon, double *x)
{	/* Used in left/right_polyconic only */
	double E, sp, cp, phi0, phi, delta;
	GMT_LONG n_iter = 0;

	*x = lon;
	GMT_WIND_LON(*x);
	y *= project_info.i_EQ_RAD;
	y += project_info.pole * D2R;
	if (GMT_IS_ZERO(y))
		*x *= project_info.EQ_RAD * D2R;
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
		*x = project_info.EQ_RAD * cp * sin(E);
	}
}

double GMT_left_polyconic (double y)
{
	double x;
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	GMT_ipolyconic_sub (y, project_info.w, &x);
	return (x * project_info.x_scale + project_info.x0);
}

double GMT_right_polyconic (double y)
{
	double x;
	y -= project_info.y0;
	y *= project_info.i_y_scale;
	GMT_ipolyconic_sub (y, project_info.e, &x);
	return (x * project_info.x_scale + project_info.x0);
}
