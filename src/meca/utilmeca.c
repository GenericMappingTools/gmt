/*	$Id$
 *    Copyright (c) 1996-2012 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "pslib.h"	/* to have pslib environment */
#include "gmt.h"	/* to have gmt environment */
#include "meca.h"
#include "utilmeca.h"
#include "nrutil.h"

#define squared(x) ((x) * (x))

/************************************************************************/
void get_trans (struct GMT_CTRL *GMT, double slon, double slat, double *t11, double *t12, double *t21, double *t22)
{
	/* determine local transformation between (lon,lat) and (x,y) */
	/* return this in the 2 x 2 matrix t */
	/* this is useful for drawing velocity vectors in X,Y coordinates */
	/* even on a map which is not a Cartesian projection */

 	/* Kurt Feigl, from code by T. Herring */

	/* INPUT */
	/*   slat        - latitude, in degrees  */
	/*   slon        - longitude in degrees  */

	/* OUTPUT (returned) */
	/*   t11,t12,t21,t22 transformation matrix */

	/* LOCAL VARIABLES */
	double su, sv, udlat, vdlat, udlon, vdlon, dudlat, dvdlat, dudlon, dvdlon, dl;

	/* how much does x,y change for a 1 degree change in lon,lon ? */
	GMT_geo_to_xy (GMT, slon,     slat,     &su,    &sv );
	GMT_geo_to_xy (GMT, slon,     slat+1.0, &udlat, &vdlat);
	GMT_geo_to_xy (GMT, slon+1.0, slat    , &udlon, &vdlon);

	/* Compute dudlat, dudlon, dvdlat, dvdlon */
	dudlat = udlat - su;
	dvdlat = vdlat - sv;
	dudlon = udlon - su;
	dvdlon = vdlon - sv;

	/* Make unit vectors for the long (e/x) and lat (n/y) */
	/* to construct local transformation matrix */

	dl = sqrt (dudlon*dudlon + dvdlon*dvdlon);
	*t11 = dudlon/dl ;
	*t21 = dvdlon/dl ;

	dl = sqrt (dudlat*dudlat + dvdlat*dvdlat);
	*t12 = dudlat/dl ;
	*t22 = dvdlat/dl ;
}

double null_axis_dip (double str1, double dip1, double str2, double dip2)
{
	/*
	   compute null axis dip when strike and dip are given
	   for each nodal plane.  Angles are in degrees.

	   Genevieve Patau
	*/

	double den;

	den = asind (sind (dip1) * sind (dip2) * sind (str1 - str2));
	if (den < 0.) den = -den;
	return (den);
}

double null_axis_strike (double str1, double dip1, double str2, double dip2)
{
	/*
	   Compute null axis strike when strike and dip are given
	   for each nodal plane.   Angles are in degrees.

	   Genevieve Patau
	*/

	double phn, cosphn, sinphn, sd1, cd1, sd2, cd2, ss1, cs1, ss2, cs2;

	sincosd (dip1, &sd1, &cd1);
	sincosd (dip2, &sd2, &cd2);
	sincosd (str1, &ss1, &cs1);
	sincosd (str2, &ss2, &cs2);

	cosphn = sd1 * cs1 * cd2 - sd2 * cs2 * cd1;
	sinphn = sd1 * ss1 * cd2 - sd2 * ss2 * cd1;
	if (sind(str1 - str2) < 0.0) {
		cosphn = -cosphn;
		sinphn = -sinphn;
	}
	phn = d_atan2d(sinphn, cosphn);
	if (phn < 0.0) phn += 360.0;
	return (phn);
}

double proj_radius(double str1, double dip1, double str)
{
	/*
	   Compute the vector radius for a given strike,
	   equal area projection, inferior sphere.
	   Strike and dip of the plane are given.

	   Genevieve Patau
	*/
	double dip, r;

	dip = atan (tand (dip1) * sind (str - str1));
	r = sqrt (2.) * sin (M_PI_4 - dip / 2.);
	return (r);
}

/***********************************************************************************************************/
double ps_mechanism (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, struct GMT_FILL *F, struct GMT_FILL *E, GMT_LONG outline)
{	/* By Genevieve Patau */

	double x[1000], y[1000];
	double pos_NP1_NP2 = sind (meca.NP1.str - meca.NP2.str);
	int fault = (meca.NP1.rake > 0 ? 1 : -1);
	double radius_size, str, radius, increment, si, co, ssize[1];

	GMT_LONG i;

	struct AXIS N_axis;

	/* compute null axis strike and dip */
	N_axis.dip = null_axis_dip (meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip);
	N_axis.str = null_axis_strike (meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip);

	/* compute radius size of the bubble */
	radius_size = size * 0.5;

	/*  argument is DIAMETER!!*/
	ssize[0] = size;
	GMT_setfill (GMT, E, outline);
	PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);

	GMT_setfill (GMT, F, outline);
	if (fabs (N_axis.dip) < EPSIL) {
		/* pure normal or inverse fault (null axis strike is determined
		   with + or - 180 degrees. */
 		/* first nodal plane part */
		i = 0;
		increment = 1.0;
		str = meca.NP1.str;
		while (str <= meca.NP1.str + 180. + EPSIL) {
			radius = proj_radius (meca.NP1.str, meca.NP1.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}
		if (fault == -1) {
			/* normal fault, close first compressing part */
			str = meca.NP1.str + 180.;
			while (str >= meca.NP1.str - EPSIL) {
				sincosd (str, &si, &co);
				x[i] = x0 + si * radius_size;
				y[i] = y0 + co * radius_size;
				str -= increment;
				i++;
			}
			PSL_plotpolygon (PSL, x, y, i);
			i = 0;
		}
		/* second nodal plane part */
		str = meca.NP2.str;
		while (str <= meca.NP2.str + 180. + EPSIL) {
			radius = proj_radius (meca.NP2.str, meca.NP2.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}
		if (fault == -1) {
			/* normal fault, close second compressing part */
			str = meca.NP2.str + 180.;
			while (str >= meca.NP2.str - EPSIL) {
				sincosd (str, &si, &co);
				x[i] = x0 + si * radius_size;
				y[i] = y0 + co * radius_size;
				str -= increment;
				i++;
			}
		}
		PSL_plotpolygon (PSL, x, y, i);
	}
	/* pure strike-slip */
	else if (fabs (90. - N_axis.dip) < EPSIL) {

		increment = (fabs(meca.NP1.rake) < EPSIL) ? -1.0 : 1.0;
		/* first compressing part */
		for (i = 0; i <= 90; i++) {
			str = meca.NP1.str - 90.0 + i * increment;
			sincosd (str, &si, &co);
			x[i] = x0 + si * radius_size;
			y[i] = y0 + co * radius_size;
		}
		x[i] = x0;
		y[i] = y0;
		i++;
		PSL_plotpolygon (PSL, x, y, i);
		/* second compressing part */
		for (i = 0; i <= 90; i++) {
			str = meca.NP1.str + 90.0 + i * increment;
			sincosd (str, &si, &co);
			x[i] = x0 + si * radius_size;
			y[i] = y0 + co * radius_size;
		}
		x[i] = x0;
		y[i] = y0;
		i++;
		PSL_plotpolygon (PSL, x, y, i);
	}
	else {
		/* other cases */
		/* first nodal plane till null axis */
		i = 0;
		increment = 1.;
		if (meca.NP1.str > N_axis.str) meca.NP1.str -= 360.;
		str = meca.NP1.str;
		while (fabs (90. - meca.NP1.dip) < EPSIL ? str <= meca.NP1.str + EPSIL : str <= N_axis.str + EPSIL) {
			radius = proj_radius (meca.NP1.str, meca.NP1.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}

		/* second nodal plane from null axis */
		meca.NP2.str += (1 + fault) * 90.;
		if (meca.NP2.str >= 360.) meca.NP2.str -= 360.;
		increment = fault;
		if (fault * (meca.NP2.str - N_axis.str) < -EPSIL) meca.NP2.str += fault * 360.;
		str = fabs (90. - meca.NP2.dip) < EPSIL ? meca.NP2.str : N_axis.str;
		while (increment > 0. ? str <= meca.NP2.str + EPSIL : str >= meca.NP2.str - EPSIL) {
			radius = proj_radius (meca.NP2.str - (1 + fault) * 90., meca.NP2.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}

		/* close the first compressing part */
		meca.NP1.str = zero_360(meca.NP1.str);
		meca.NP2.str = zero_360(meca.NP2.str);
		increment = pos_NP1_NP2 >= 0. ? -fault : fault;
		if (increment * (meca.NP1.str - meca.NP2.str) < - EPSIL) meca.NP1.str += increment * 360.;
		str = meca.NP2.str;
		while (increment > 0. ? str <= meca.NP1.str + EPSIL : str >= meca.NP1.str - EPSIL) {
			sincosd (str, &si, &co);
			x[i] = x0 + si * radius_size;
			y[i] = y0 + co * radius_size;
			str += increment;
			i++;
		}

		PSL_plotpolygon (PSL, x, y, i);

		/* first nodal plane till null axis */
		i = 0;
		meca.NP1.str = zero_360 (meca.NP1.str + 180.);
		if (meca.NP1.str - N_axis.str < - EPSIL) meca.NP1.str += 360.;
		increment = -1.;
		str = meca.NP1.str;
		while (fabs (90. - meca.NP1.dip) < EPSIL ? str >= meca.NP1.str -EPSIL : str >= N_axis.str - EPSIL) {
			radius = proj_radius (meca.NP1.str - 180., meca.NP1.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}

		/* second nodal plane from null axis */
		meca.NP2.str = zero_360(meca.NP2.str + 180.);
		increment = -fault;
		if (fault * (N_axis.str - meca.NP2.str) < - EPSIL) meca.NP2.str -= fault * 360.;
		str = fabs (90. - meca.NP2.dip) < EPSIL ? meca.NP2.str : N_axis.str;
		while (increment > 0. ? str <= meca.NP2.str + EPSIL : str >= meca.NP2.str - EPSIL) {
			radius = proj_radius (meca.NP2.str - (1 - fault) * 90., meca.NP2.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
			str += increment;
			i++;
		}

		/* close the second compressing part */
		meca.NP1.str = zero_360(meca.NP1.str);
		meca.NP2.str = zero_360(meca.NP2.str);
		increment = pos_NP1_NP2 >= 0. ? -fault : fault;
		if (increment * (meca.NP1.str - meca.NP2.str) < - EPSIL) meca.NP1.str += increment * 360.;
		str = meca.NP2.str;
		while (increment > 0. ? str <= meca.NP1.str + EPSIL : str >= meca.NP1.str - EPSIL) {
			sincosd (str, &si, &co);
			x[i] = x0 + si * radius_size;
			y[i] = y0 + co * radius_size;
			str += increment;
			i++;
		}

		PSL_plotpolygon (PSL, x, y, i);
	}
	return (size);
}

/*********************************************************************/
double ps_plan (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, GMT_LONG num_of_plane)
{	/* By Genevieve Patau */

	GMT_LONG i;

	double x[1000], y[1000], ssize[1];
	double radius_size, str, radius, si, co;

	/* compute radius size of the bubble */
	radius_size = size * 0.5;

	/*  argument is DIAMETER!!*/
	ssize[0] = size;
	PSL_setfill (PSL, GMT->session.no_rgb, TRUE);
	PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);

	if (num_of_plane != 2) {
		for (i = 0; i <= 180; i++) {
			str = meca.NP1.str + i;
			radius = proj_radius (meca.NP1.str, meca.NP1.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
		}
		PSL_plotline (PSL, x, y, i, PSL_MOVE + PSL_STROKE);
	}
	if (num_of_plane != 1) {
		for (i = 0; i <= 180; i++) {
			str = meca.NP2.str + i;
			radius = proj_radius(meca.NP2.str, meca.NP2.dip, str) * radius_size;
			sincosd (str, &si, &co);
			x[i] = x0 + radius * si;
			y[i] = y0 + radius * co;
		}
		PSL_plotline (PSL, x, y, i, PSL_MOVE + PSL_STROKE);
	}
	return (size);
}

/*********************************************************************/
double zero_360 (double str)
{	/* By Genevieve Patau: put an angle between 0 and 360 degrees */
	if (str >= 360.0)
		str -= 360.0;
	else if (str < 0.0)
		str += 360.0;
	return (str);
}

/**********************************************************************/
double computed_mw (struct MOMENT moment, double ms)
{
	/* Compute mw-magnitude from seismic moment or MS magnitude. */

	/* Genevieve Patau from
	   Thorne Lay, Terry C. Wallace
	   Modern Global Seismology
	   Academic Press, p. 384
	 */

	double mw;

	if (moment.exponent == 0)
		mw = ms;
	else
		mw = (log10 (moment.mant) + (double)moment.exponent - 16.1) * 2. / 3.;

	return (mw);
}

/*********************************************************************/
double computed_strike1 (struct nodal_plane NP1)
{
	/*
	   Compute the strike of the decond nodal plane when are given
	   strike, dip and rake for the first nodal plane with AKI & RICHARD's
	   convention.  Angles are in degrees.
	   Genevieve Patau
	*/

	double str2, temp, cp2, sp2, ss, cs, sr, cr;
	double cd1 = cosd (NP1.dip);
	double am = (GMT_IS_ZERO (NP1.rake) ? 1. : NP1.rake /fabs (NP1.rake));

	sincosd (NP1.rake, &sr, &cr);
	sincosd (NP1.str, &ss, &cs);
	if (cd1 < EPSIL && fabs (cr) < EPSIL) {
#if 0
		GMT_report (GMT, GMT_MSG_DEBUG, "\nThe second plane is horizontal;");
		GMT_report (GMT, GMT_MSG_DEBUG, "\nStrike is undetermined.");
		GMT_report (GMT, GMT_MSG_DEBUG, "\nstr2 = NP1.str + 180. is taken to define");
		GMT_report (GMT, GMT_MSG_DEBUG, "\nrake in the second plane.\n");
#endif
		str2 = NP1.str + 180.0;
	}
	else {
		temp = cr * cs;
		temp += sr * ss * cd1;
		sp2 = -am * temp;
		temp = ss * cr;
		temp -= sr *  cs * cd1;
		cp2 = am * temp;
		str2 = d_atan2d(sp2, cp2);
		str2 = zero_360(str2);
	}
	return (str2);
}

/*********************************************************************/
double computed_dip1 (struct nodal_plane NP1)
{
	/*
	   Compute second nodal plane dip when are given strike,
	   dip and rake for the first nodal plane with AKI & RICHARD's
	   convention.  Angles are in degrees.
	   Genevieve Patau
	*/

	double am = (GMT_IS_ZERO (NP1.rake) ? 1.0 : NP1.rake / fabs (NP1.rake));
	double dip2;

	dip2 = acosd (am * sind (NP1.rake) * sind (NP1.dip));

	return (dip2);
}

/*********************************************************************/
double computed_rake1 (struct nodal_plane NP1)
{
	/*
	   Compute rake in the second nodal plane when strike ,dip
	   and rake are given for the first nodal plane with AKI &
	   RICHARD's convention.  Angles are in degrees.

	   Genevieve Patau
	*/

	double computed_strike1(), computed_dip1();
	double rake2, sinrake2;
	double str2 = computed_strike1(NP1);
	double dip2 = computed_dip1(NP1);
	double am = (GMT_IS_ZERO (NP1.rake) ? 1.0 : NP1.rake / fabs (NP1.rake));
	double sd, cd, ss, cs;
	sincosd (NP1.dip, &sd, &cd);
	sincosd (NP1.str - str2, &ss, &cs);

	if (fabs (dip2 - 90.0) < EPSIL)
		sinrake2 = am * cd;
	else
		sinrake2 = -am * sd * cs / cd;

		rake2 = d_atan2d (sinrake2, -am * sd * ss);

	return (rake2);
}

/*********************************************************************/
double computed_dip2 (double str1, double dip1, double str2)
{
	/*
	   Compute second nodal plane dip when are given
	   strike and dip for the first plane and strike for
	   the second plane.  Angles are in degrees.
	   Warning : if dip1 == 90 and cos(str1 - str2) == 0
				 the second plane dip is undetermined
				 and the only first plane will be plotted.

	   Genevieve Patau */

	double dip2, cosdp12 = cosd(str1 - str2);

	if (fabs (dip1 - 90.) < EPSIL && fabs (cosdp12) < EPSIL)
		dip2 = 1000.0; /* (only first plane will be plotted) */
	else
		dip2 = d_atan2d (cosd (dip1), -sind (dip1) * cosdp12);

	return (dip2);
}

/*********************************************************************/
double computed_rake2 (double str1, double dip1, double str2, double dip2, double fault)
{
	/*
	   Compute rake in the second nodal plane when strike and dip
	   for first and second nodal plane are given with a double
	   characterizing the fault :
						  +1. inverse fault
						  -1. normal fault.
	   Angles are in degrees.

	   Genevieve Patau */

	double rake2, sinrake2, sd, cd, ss, cs;

	sincosd (str1 - str2, &ss, &cs);

	sd = sind(dip1);        cd = cosd(dip2);
	if (fabs (dip2 - 90.0) < EPSIL)
		sinrake2 = fault * cd;
	else
		sinrake2 = -fault * sd * cs / cd;

	rake2 = d_atan2d (sinrake2, - fault * sd * ss);

	return (rake2);
}

/*********************************************************************/
void define_second_plane (struct nodal_plane NP1, struct nodal_plane *NP2)
{
	/*
	    Compute strike, dip, slip for the second nodal plane
	    when are given strike, dip and rake for the first one.
	    Genevieve Patau
	*/

	NP2->str = computed_strike1 (NP1);
	NP2->dip = computed_dip1 (NP1);
	NP2->rake  = computed_rake1 (NP1);
}

/***************************************************************************************/
void moment2axe (struct GMT_CTRL *GMT, struct M_TENSOR mt, struct AXIS *T, struct AXIS *N, struct AXIS *P)
{
	/* This version uses GMT_jacobi and does not suffer from the convert_matrix bug */
	GMT_LONG j, nrots, np = 3;
	double *a, *d, *b, *z, *v;
	double az[3], pl[3];

	a = GMT_memory (GMT, NULL, np*np, double);
	d = GMT_memory (GMT, NULL, np, double);
	b = GMT_memory (GMT, NULL, np, double);
	z = GMT_memory (GMT, NULL, np, double);
	v = GMT_memory (GMT, NULL, np*np, double);

	a[0]=mt.f[0];	a[1]=mt.f[3];	a[2]=mt.f[4];
	a[3]=mt.f[3];	a[4]=mt.f[1];	a[5]=mt.f[5];
	a[6]=mt.f[4];	a[7]=mt.f[5];	a[8]=mt.f[2];

	if (GMT_jacobi (GMT, a, &np, &np, d, v, b, z, &nrots))
		fprintf(GMT->session.std[GMT_ERR],"%s: Eigenvalue routine failed to converge in 50 sweeps.\n", GMT->init.progname);

	for (j = 0; j < np; j++) {
		pl[j] = asin(-v[j*np]);
		az[j] = atan2(v[j*np+2], -v[j*np+1]);
		if (pl[j] <= 0.) {
			pl[j] = -pl[j];
			az[j] += M_PI;
		}
		if (az[j] < 0)
			az[j] += TWO_PI;
		else if (az[j] > TWO_PI)
			az[j] -= TWO_PI;
		pl[j] *= R2D;
		az[j] *= R2D;
	}
	T->val = d[0];	T->e = mt.expo;	T->str = az[0]; T->dip = pl[0];
	N->val = d[1];	N->e = mt.expo; N->str = az[1]; N->dip = pl[1];
	P->val = d[2];	P->e = mt.expo; P->str = az[2]; P->dip = pl[2];

	GMT_free (GMT, a);	GMT_free (GMT, d);
	GMT_free (GMT, b);	GMT_free (GMT, z);
	GMT_free (GMT, v);
}

/***************************************************************************************/
double ps_tensor (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double size, struct AXIS T, struct AXIS N, struct AXIS P, struct GMT_FILL *C, struct GMT_FILL *E, GMT_LONG outline, GMT_LONG plot_zerotrace)
{
	GMT_LONG d, b = 1, m, i, ii, n = 0, j = 1, j2 = 0, j3 = 0;
	GMT_LONG big_iso = 0;
	GMT_LONG djp, mjp, jp_flag;

	double a[3], p[3], v[3], azi[3][2];
	double vi, iso, f, fir, s2alphan, alphan;
	double cfi, sfi, can, san, xz, xn, xe;
	double cpd, spd, cpb, spb, cpm, spm;
	double cad, sad, cab, sab, cam, sam;
	double az = 0., azp = 0., takeoff, r;
	double x[400], y[400], x2[400], y2[400], x3[400], y3[400];
	double xp1[800], yp1[800], xp2[400], yp2[400];
	double radius_size, si, co, ssize[1];

	struct GMT_FILL *F1 = NULL, *F2 = NULL;

	a[0] = T.str; a[1] = N.str; a[2] = P.str;
	p[0] = T.dip; p[1] = N.dip; p[2] = P.dip;
	v[0] = T.val; v[1] = N.val; v[2] = P.val;

	vi = (v[0] + v[1] + v[2]) / 3.;
	for (i=0; i<=2; i++) v[i] = v[i] - vi;

	ssize[0] = size;
	radius_size = size * 0.5;

	if (fabs (squared(v[0]) + squared(v[1]) + squared(v[2])) < EPSIL) {
 		/* pure implosion-explosion */
		ssize[0] = radius_size*2.0;
		if (vi > 0.) {
			ssize[0] = radius_size*2.0;
			GMT_setfill (GMT, C, TRUE);
			PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		}
		if (vi < 0.) {
			ssize[0] = radius_size*2.0;
			GMT_setfill (GMT, E, TRUE);
			PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		}
		return (radius_size*2.);
	}

	if (fabs (v[0]) >= fabs (v[2])) {
		d = 0;
		m = 2;
	}
	else {
		d = 2;
		m = 0;
	}

	if (plot_zerotrace) vi = 0.;

	f = - v[1] / v[d];
	iso = vi / v[d];
	jp_flag = 0;
	djp = -1;
	mjp = -1;

	/* Cliff Frohlich, Seismological Research letters,
 	 * Vol 7, Number 1, January-February, 1996
 	 * Unless the isotropic parameter lies in the range
 	 * between -1 and 1 - f there will be no nodes whatsoever */

	if (iso < -1) {
		GMT_setfill (GMT, E, TRUE);
		PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		return (size);
	}
	else if (iso > 1 - f) {
		GMT_setfill (GMT, C, TRUE);
		PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		return (size);
	}

	sincosd (p[d], &spd, &cpd);
	sincosd (p[b], &spb, &cpb);
	sincosd (p[m], &spm, &cpm);
	sincosd (a[d], &sad, &cad);
	sincosd (a[b], &sab, &cab);
	sincosd (a[m], &sam, &cam);

	for (i = 0; i < 360; i++) {
		fir = (double) i * D2R;
		s2alphan = (2. + 2. * iso) / (3. + (1. - 2. * f) * cos(2. * fir));
		if (s2alphan > 1.) {
			big_iso++;
/* below pieces added as patch to fix big_iso case plotting problems. Not well done, but works
   Jeremy Pesicek Nov. 2010.

   second case added Dec. 2010 */

			if (d == 0 && m == 2) {
				jp_flag = 1;
				djp = 2;
				mjp = 0;
			}
			if (d == 2 && m == 0) {
				jp_flag = 2;
				djp = 0;
				mjp = 2;
			}

			d = djp;
			m = mjp;

			f = - v[1] / v[d];
			iso = vi / v[d];
			s2alphan = (2. + 2. * iso) / (3. + (1. - 2. * f) * cos(2. * fir));
			sincosd (p[d], &spd, &cpd);
			sincosd (p[b], &spb, &cpb);
			sincosd (p[m], &spm, &cpm);
			sincosd (a[d], &sad, &cad);
			sincosd (a[b], &sab, &cab);
			sincosd (a[m], &sam, &cam);

			alphan = asin(sqrt(s2alphan));
			sincos (fir, &sfi, &cfi);
			sincos (alphan, &san, &can);

			xz = can * spd + san * sfi * spb + san * cfi * spm;
			xn = can * cpd * cad + san * sfi * cpb * cab + san * cfi * cpm * cam;
			xe = can * cpd * sad + san * sfi * cpb * sab + san * cfi * cpm * sam;

			if (fabs(xn) < EPSIL && fabs(xe) < EPSIL) {
				takeoff = 0.;
				az = 0.;
			}
			else {
				az = atan2(xe, xn);
				if (az < 0.) az += M_PI * 2.;
				takeoff = acos(xz / sqrt(xz * xz + xn * xn + xe * xe));
			}
			if (takeoff > M_PI_2) {
				takeoff = M_PI - takeoff;
				az += M_PI;
				if (az > M_PI * 2.) az -= M_PI * 2.;
			}
			r = M_SQRT2 * sin(takeoff / 2.);
			sincos (az, &si, &co);
			if (i == 0) {
				azi[i][0] = az;
				x[i] = x0 + radius_size * r * si;
				y[i] = y0 + radius_size * r * co;
				azp = az;
			}
			else {
				if (fabs(fabs(az - azp) - M_PI) < D2R * 10.) {
					azi[n][1] = azp;
					azi[++n][0] = az;
				}
				if (fabs(fabs(az -azp) - M_PI * 2.) < D2R * 2.) {
					if (azp < az) azi[n][0] += M_PI * 2.;
					else azi[n][0] -= M_PI * 2.;
				}
				switch (n) {
					case 0 :
						x[j] = x0 + radius_size * r * si;
						y[j] = y0 + radius_size * r * co;
						j++;
						break;
					case 1 :
						x2[j2] = x0 + radius_size * r * si;
						y2[j2] = y0 + radius_size * r * co;
						j2++;
						break;
					case 2 :
						x3[j3] = x0 + radius_size * r * si;
						y3[j3] = y0 + radius_size * r * co;
						j3++;
						break;
				}
				azp = az;
			}
		}
/* end patch to fix big_iso case plotting problems. Jeremy Pesicek, Nov., Dec. 2010 */
		else {
			alphan = asin(sqrt(s2alphan));
			sincos (fir, &sfi, &cfi);
			sincos (alphan, &san, &can);

			xz = can * spd + san * sfi * spb + san * cfi * spm;
			xn = can * cpd * cad + san * sfi * cpb * cab + san * cfi * cpm * cam;
			xe = can * cpd * sad + san * sfi * cpb * sab + san * cfi * cpm * sam;

			if (fabs (xn) < EPSIL && fabs (xe) < EPSIL) {
				takeoff = 0.;
				az = 0.;
			}
			else {
				az = atan2(xe, xn);
				if (az < 0.) az += M_PI * 2.;
				takeoff = acos(xz / sqrt(xz * xz + xn * xn + xe * xe));
			}
			if (takeoff > M_PI_2) {
				takeoff = M_PI - takeoff;
				az += M_PI;
				if (az > M_PI * 2.) az -= M_PI * 2.;
			}
			r = M_SQRT2 * sin(takeoff / 2.);
			sincos (az, &si, &co);
			if (i == 0) {
				azi[i][0] = az;
				x[i] = x0 + radius_size * r * si;
				y[i] = y0 + radius_size * r * co;
				azp = az;
			}
			else {
				if (fabs (fabs (az - azp) - M_PI) < D2R * 10.) {
					azi[n][1] = azp;
					azi[++n][0] = az;
				}
				if (fabs (fabs (az -azp) - M_PI * 2.) < D2R * 2.) {
					if (azp < az) azi[n][0] += M_PI * 2.;
					else azi[n][0] -= M_PI * 2.;
				}
				switch (n) {
					case 0 :
						x[j] = x0 + radius_size * r * si;
						y[j] = y0 + radius_size * r * co;
						j++;
						break;
					case 1 :
						x2[j2] = x0 + radius_size * r * si;
						y2[j2] = y0 + radius_size * r * co;
						j2++;
						break;
					case 2 :
						x3[j3] = x0 + radius_size * r * si;
						y3[j3] = y0 + radius_size * r * co;
						j3++;
						break;
				}
				azp = az;
			}
		}
	}
	azi[n][1] = az;

	if (v[1] < 0.)
		F1 = C,	F2 = E;
	else
		F1 = E,	F2 = C;

	if (!big_iso) {
		GMT_setfill (GMT, F2, TRUE);
		PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
	}
	else if (jp_flag == 1) {
		fprintf (stderr, "Warning: big isotropic component, case not fully tested! \n");
		GMT_setfill (GMT, F1, TRUE);
		PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		F1 = E, F2 = C;
	}
	else if (jp_flag == 2) {
		fprintf (stderr, "Warning: big isotropic component, case not fully tested! \n");
		GMT_setfill (GMT, F1, TRUE);
		PSL_plotsymbol (PSL, x0, y0, ssize, GMT_SYMBOL_CIRCLE);
		F2 = E, F1 = C;
	}

	GMT_setfill (GMT, F1, FALSE);
	switch (n) {
		case 0 :
			for (i = 0; i < 360; i++) {
				xp1[i] = x[i]; yp1[i] = y[i];
			}
			PSL_plotpolygon (PSL, xp1, yp1, i);
			break;
		case 1 :
			for (i = 0; i < j; i++) {
				xp1[i] = x[i]; yp1[i] = y[i];
			}
			if (azi[0][0] - azi[0][1] > M_PI) azi[0][0] -= M_PI * 2.;
			else if (azi[0][1] - azi[0][0] > M_PI) azi[0][0] += M_PI * 2.;
			if (azi[0][0] < azi[0][1]) for (az = azi[0][1] - D2R; az > azi[0][0]; az -= D2R) {
				sincos (az, &si, &co);
				xp1[i] = x0 + radius_size * si;
				yp1[i++] = y0 + radius_size * co;
			}
			else for (az = azi[0][1] + D2R; az < azi[0][0]; az += D2R) {
				sincos (az, &si, &co);
				xp1[i] = x0 + radius_size * si;
				yp1[i++] = y0 + radius_size * co;
			}
			PSL_plotpolygon (PSL, xp1, yp1, i);
			for (i=0; i<j2; i++) {
				xp2[i] = x2[i]; yp2[i] = y2[i];
			}
			if (azi[1][0] - azi[1][1] > M_PI) azi[1][0] -= M_PI * 2.;
			else if (azi[1][1] - azi[1][0] > M_PI) azi[1][0] += M_PI * 2.;
			if (azi[1][0] < azi[1][1]) for (az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
				sincos (az, &si, &co);
				xp2[i] = x0 + radius_size * si;
				yp2[i++] = y0 + radius_size * co;
			}
			else for (az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
				sincos (az, &si, &co);
				xp2[i] = x0 + radius_size * si;
				yp2[i++] = y0 + radius_size * co;
			}
			PSL_plotpolygon (PSL, xp2, yp2, i);
			break;
		case 2 :
			for (i = 0; i < j3; i++) {
				xp1[i] = x3[i]; yp1[i] = y3[i];
			}
			for (ii = 0; ii < j; ii++) {
				xp1[i] = x[ii]; yp1[i++] = y[ii];
			}

			if (azi[2][0] - azi[0][1] > M_PI)
				azi[2][0] -= M_PI * 2.;
			else if (azi[0][1] - azi[2][0] > M_PI)
				azi[2][0] += M_PI * 2.;
			if (azi[2][0] < azi[0][1]) {
				for (az = azi[0][1] - D2R; az > azi[2][0]; az -= D2R) {
					sincos (az, &si, &co);
					xp1[i] = x0 + radius_size * si;
					yp1[i++] = y0 + radius_size * co;
				}
			}
			else {
				for (az = azi[0][1] + D2R; az < azi[2][0]; az += D2R) {
					sincos (az, &si, &co);
					xp1[i] = x0 + radius_size * si;
					yp1[i++] = y0 + radius_size * co;
				}
			}
			PSL_plotpolygon (PSL, xp1, yp1, i);
			for (i = 0; i < j2; i++) {
				xp2[i] = x2[i]; yp2[i] = y2[i];
			}
			if (azi[1][0] - azi[1][1] > M_PI)
				azi[1][0] -= M_PI * 2.;
			else if (azi[1][1] - azi[1][0] > M_PI)
				azi[1][0] += M_PI * 2.;
			if (azi[1][0] < azi[1][1]) {
				for (az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
					sincos (az, &si, &co);
					xp2[i] = x0 + radius_size * si;
					yp2[i++] = y0 + radius_size * co;
				}
			}
			else {
				for (az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
					sincos (az, &si, &co);
					xp2[i] = x0 + radius_size * si;
					yp2[i++] = y0 + radius_size * co;
				}
			}
			PSL_plotpolygon (PSL, xp2, yp2, i);
			break;
	}
	return (size);
}

void axe2dc (struct AXIS T, struct AXIS P, struct nodal_plane *NP1, struct nodal_plane *NP2)
{
	/*
	  Calculate double couple from principal axes.
	  Angles are in degrees.

	  Genevieve Patau, 16 juin 1997
	*/

	double p1, d1, p2, d2;
	double cdp, sdp, cdt, sdt, cpt, spt, cpp, spp;
	double amz, amy, amx, im;

	sincosd (P.dip, &sdp, &cdp);
	sincosd (P.str, &spp, &cpp);
	sincosd (T.dip, &sdt, &cdt);
	sincosd (T.str, &spt, &cpt);

	cpt *= cdt; spt *= cdt;
	cpp *= cdp; spp *= cdp;

	amz = sdt + sdp; amx = spt + spp; amy = cpt + cpp;
	d1 = atan2d (hypot(amx, amy), amz);
	p1 = atan2d (amy, -amx);
	if (d1 > 90.0) {
		d1 = 180.0 - d1;
		p1 -= 180.0;
	}
	if (p1 < 0.0) p1 += 360.0;

	amz = sdt - sdp; amx = spt - spp; amy = cpt - cpp;
	d2 = atan2d (hypot(amx, amy), amz);
	p2 = atan2d (amy, -amx);
	if (d2 > 90.0) {
		d2 = 180.0 - d2;
		p2 -= 180.0;
	}
	if (p2 < 0.0) p2 += 360.0;

	NP1->dip = d1; NP1->str = p1;
	NP2->dip = d2; NP2->str = p2;

	im = 1;
	if (P.dip > T.dip) im = -1;
	NP1->rake = computed_rake2 (NP2->str, NP2->dip, NP1->str, NP1->dip, im);
	NP2->rake = computed_rake2 (NP1->str, NP1->dip, NP2->str, NP2->dip, im);
}

void dc2axe (st_me meca, struct AXIS *T, struct AXIS *N, struct AXIS *P)
{
	/*
	From FORTRAN routines of Anne Deschamps :
	compute azimuth and plungement of P-T axis
	from nodal plane strikes, dips and rakes.
	*/

	double cd1, sd1, cd2, sd2, cp1, sp1, cp2, sp2;
	double amz, amx, amy, dx, px, dy, py;

	cd1 = cosd (meca.NP1.dip) * M_SQRT2;
	sd1 = sind (meca.NP1.dip) * M_SQRT2;
	cd2 = cosd (meca.NP2.dip) * M_SQRT2;
	sd2 = sind (meca.NP2.dip) * M_SQRT2;
	cp1 = - cosd (meca.NP1.str) * sd1;
	sp1 = sind (meca.NP1.str) * sd1;
	cp2 = - cosd (meca.NP2.str) * sd2;
	sp2 = sind (meca.NP2.str) * sd2;

	amz = - (cd1 + cd2);
	amx = - (sp1 + sp2);
	amy = cp1 + cp2;
	dx = atan2d (hypot(amx, amy), amz) - 90.0;
	px = atan2d (amy, -amx);
	if (px < 0.0) px += 360.0;
	if (dx < EPSIL) {
		if (px > 90.0 && px < 180.0) px += 180.0;
		if (px >= 180.0 && px < 270.0) px -= 180.0;
	}

	amz = cd1 - cd2;
	amx = sp1 - sp2;
	amy = - cp1 + cp2;
	dy = atan2d (hypot(amx, amy), -fabs(amz)) - 90.0;
	py = atan2d (amy, -amx);
	if (amz > 0.0) py -= 180.0;
	if (py < 0.0) py += 360.0;
	if (dy < EPSIL) {
		if (py > 90.0 && py < 180.0) py += 180.0;
		if (py >= 180.0 && py < 270.0) py -= 180.0;
	}

	if (meca.NP1.rake > 0.0) {
		P->dip = dy; P->str = py;
		T->dip = dx; T->str = px;
	}
	else {
		P->dip = dx; P->str = px;
		T->dip = dy; T->str = py;
	}

	N->str = null_axis_strike (T->str, T->dip, P->str, P->dip);
	N->dip = null_axis_dip (T->str, T->dip, P->str, P->dip);
}

void axis2xy (double x0, double y0, double size, double pp, double dp, double pt, double dt, double *xp,double *yp, double *xt, double *yt)
{
	/* angles are in degrees */
	double radius, spp, cpp, spt, cpt;

	sincosd (pp, &spp, &cpp);
	sincosd (pt, &spt, &cpt);

	size *= 0.5;
	radius = sqrt(1. - sind(dp));
	if (radius >= 0.97) radius = 0.97;
	*xp = radius * spp * size + x0;
	*yp = radius * cpp * size + y0;
	radius = sqrt(1. - sind(dt));
	if (radius >= 0.97) radius = 0.97;
	*xt = radius * spt * size + x0;
	*yt = radius * cpt * size + y0;
}

void transform_local (double x0, double y0, double dxp, double dyp, double scale, double t11, double t12, double t21, double t22, double *x1, double *y1)
{
	/* perform local transformation on offsets (dxp,dyp) from */
	/* "origin point" x0,y0 given transformation matrix T */

	/* Kurt Feigl, from code by T. Herring */

	/* INPUT */
	/*   x0,y0       - dxp,dyp with respect to this point */
	/*   dxp         - x component of arrow */
	/*   dyp         - y component of arrow */
	/*   scale       - scaling for arrow    */
	/*   t11,t12,t21,t22 transformation matrix */

	/* OUTPUT (returned) */
	/*   x1,y1       - paper coordinates of arrow tail */

	/* LOCAL VARIABLES */
	double du, dv;

	/* perform local transformation */
	du = scale * (t11*dxp + t12*dyp);
	dv = scale * (t21*dxp + t22*dyp);

	/*  Now add to origin  and return values */
	*x1 = x0 + du;
	*y1 = y0 + dv;

}

void trace_arrow (struct GMT_CTRL *GMT, double slon, double slat, double dxp, double dyp, double scale, double *x1, double *y1, double *x2, double *y2)
{
	/* convert a vector arrow (delx,dely) arrow from (lat,lon) */

	/* Kurt Feigl, from code by T. Herring */

	/* INPUT */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   dxp         - x component of arrow */
	/*   dyp         - y component of arrow */
	/*   scale       - scaling for arrow    */

	/* OUTPUT (returned) */
	/*   x1,y1       - paper coordinates of arrow tail */
	/*   x2,y2       - paper coordinates of arrow head */

	/* local */
	double t11, t12, t21, t22, xt, yt;

	/* determine local transformation between (lon, lat) and (x, y) */
	/* return this in the 2 x 2 matrix t */
	get_trans (GMT, slon, slat, &t11, &t12, &t21, &t22);

	/* map start of arrow from lat, lon to x, y */
	GMT_geo_to_xy (GMT, slon, slat, &xt, &yt);

	/* perform the transformation */
	transform_local (xt, yt, dxp, dyp, scale, t11, t12, t21, t22, x2, y2);

	/* return values */

	*x1 = xt;
	*y1 = yt;
}

void trace_ellipse (double angle, double major, double minor, GMT_LONG npoints, double *x, double *y)
{	/* Given specs for an ellipse, return it in x,y */
	double phi = 0.0, sd, cd, s, c;
	GMT_LONG i;

	sincosd (angle, &sd, &cd);

	for (i = 0; i < 360; i++) {
		sincos (phi, &s, &c);
		*x++ = major * c * cd - minor * s * sd;
		*y++ = major * c * sd + minor * s * cd;
		phi += M_PI*2.0/(npoints-2);
	}
}

void ellipse_convert (double sigx, double sigy, double rho, double conrad, double *eigen1, double *eigen2, double *ang)
{
	/* convert from one parameterization of an ellipse to another

	 * Kurt Feigl, from code by T. Herring

	 * INPUT
	 *   sigx, sigy  - Sigmas in the x and y directions.
	 *   rho         - Correlation coefficient between x and y

	 * OUTPUT (returned)
	 *   eigen1      - the smaller eigenvalue
	 *   eigen2      - the larger eigenvalue
	 *   ang         - Orientation of ellipse relative to X axis in radians
	 *               - should be counter-clockwise from X axis

	 * LOCAL VARIABLES

	 *   a,b,c,d,e   - Constants used in getting eigenvalues
	 *   conrad      - Radius for the confidence interval
	 */

	double a, b, c, d, e;

	/* confidence scaling */
	/*   confid      - Confidence interval wanted (0-1) */
	/* conrad = sqrt( -2.0 * log(1.0 - confid)); */

	/* the formulas for this part may be found in Bomford, p. 719 */

	a = squared (sigy*sigy - sigx*sigx);
	b = 4. * squared (rho*sigx*sigy);
	c = squared (sigx) + squared (sigy);

	/* minimum eigenvector (semi-minor axis) */
	*eigen1 = conrad * sqrt ((c - sqrt(a + b))/2.0);

	/* maximu eigenvector (semi-major axis) */
	*eigen2 = conrad * sqrt ((c + sqrt(a + b))/2.0);

	d = 2. * rho * sigx * sigy;
	e = squared (sigx) - squared (sigy);

	*ang = atan2 (d, e)/2.0;

	/*    that is all */
}

void paint_ellipse (struct PSL_CTRL *PSL, double x0, double y0, double angle, double major, double minor, double scale, double t11, double t12, double t21, double t22, GMT_LONG polygon, double *rgb, GMT_LONG outline)
{	/* Make an ellipse at center x0,y0  */
#define NPOINTS_ELLIPSE 362

	GMT_LONG npoints = NPOINTS_ELLIPSE, i;
	/* relative to center of ellipse */
	double dxe[NPOINTS_ELLIPSE],dye[NPOINTS_ELLIPSE];
	/* absolute paper coordinates */
	double axe[NPOINTS_ELLIPSE],aye[NPOINTS_ELLIPSE];

	trace_ellipse (angle, major, minor, npoints, dxe, dye);

	for (i = 0; i < npoints - 2; i++) transform_local (x0, y0, dxe[i], dye[i], scale, t11, t12, t21, t22, &axe[i], &aye[i]);
	if (polygon) {
		PSL_setfill (PSL, rgb, TRUE);
		PSL_plotpolygon (PSL, axe, aye, npoints - 2);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints - 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
}

/************************************************************************/
GMT_LONG trace_cross (struct GMT_CTRL *GMT, double slon, double slat, double eps1, double eps2, double theta, double sscale, double v_width, double h_length, double h_width, double vector_shape,GMT_LONG outline,struct GMT_PEN pen)
{
	/* make a Strain rate cross at(slat,slon) */

	/* Kurt Feigl, from code by D. Dong */

	/*   INPUT VARIABLES: */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   sscale      : scaling factor for size of cloverleaf */
	/*   theta       : azimuth of more compressive eigenvector (deg) */
	/*   eps1,eps2   : eigenvalues of strain rate (1/yr) */
	/*   v_width, h_length,h_width,vector_shape: arrow characteristics */

	/* local */
	double dx, dy, x1, x2, y1, y2, hl, hw, vw, s, c, dim[7];

	sincosd (theta, &s, &c);

	/*  extension component */
	dx =  eps1 * c;
	dy = -eps1 * s;

	/* arrow is outward from slat,slon */
	trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

	if (eps1 < 0.0) {
		d_swap (x1, x2);
		d_swap (y1, y2);
	}

	if (hypot (x1-x2,y1-y2) <= 1.5 * h_length) {
		hl = hypot (x1-x2, y1-y2) * 0.6;
		hw = hl * h_width / h_length;
		vw = hl * v_width / h_length;
		if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;
	}
	else {
		hw = h_width;
		hl = h_length;
		vw = v_width;
	}

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	dim[5] = vector_shape, dim[6] = GMT_VEC_END | GMT_VEC_FILL;
	PSL_setcolor (GMT->PSL, pen.rgb, PSL_IS_STROKE);
	PSL_plotsymbol (GMT->PSL, x1, x2, dim, PSL_VECTOR);

	/* second, extensional arrow in opposite direction */

	trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

	if (eps1 < 0.0) {
		d_swap (x1, x2);
		d_swap (y1, y2);
	}

	if (hypot (x1-x2,y1-y2) <= 1.5 * h_length) {
		hl = hypot (x1-x2,y1-y2) * 0.6;
		hw = hl * h_width / h_length;
		vw = hl * v_width / h_length;
		if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;
	}
	else {
		hw = h_width;
		hl = h_length;
		vw = v_width;
	}

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	PSL_setcolor (GMT->PSL, pen.rgb, PSL_IS_STROKE);
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* compression component */
	dx = eps2 * s;
	dy = eps2 * c;

	trace_arrow (GMT, slon, slat, dx, dy, sscale, &x1, &y1, &x2, &y2);

	if (eps2 > 0.0) {
		d_swap (x1, x2);
		d_swap (y1, y2);
	}

	/* arrow should go toward slat, slon */
	if (hypot (x1-x2,y1-y2) <= 1.5 * h_length) {
		hl = hypot (x1-x2,y1-y2) * 0.6;
		hw = hl * h_width / h_length;
		vw = hl * v_width / h_length;
		if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;
	}
	else {
		hw = h_width;
		hl = h_length;
		vw = v_width;
	}

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	PSL_setcolor (GMT->PSL, pen.rgb, PSL_IS_STROKE);
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	/* second, compressional arrow in opposite direction */

	trace_arrow (GMT, slon, slat, -dx, -dy, sscale, &x1, &y1, &x2, &y2);

	if (eps2 > 0.0) {
		d_swap (x1, x2);
		d_swap (y1, y2);
	}

	/* arrow should go toward slat, slon */

	if (hypot (x1-x2,y1-y2) <= 1.5 * h_length) {
		hl = hypot (x1-x2,y1-y2) * 0.6;
		hw = hl * h_width / h_length;
		vw = hl * v_width / h_length;
		if (vw < 2.0/PSL_DOTS_PER_INCH) vw = 2.0/PSL_DOTS_PER_INCH;
	}
	else {
		hw = h_width;
		hl = h_length;
		vw = v_width;
	}

	dim[0] = x2, dim[1] = y2;
	dim[2] = vw, dim[3] = hl, dim[4] = hw;
	PSL_setcolor (GMT->PSL, pen.rgb, PSL_IS_STROKE);
	PSL_plotsymbol (GMT->PSL, x1, y1, dim, PSL_VECTOR);

	return 0;
}

GMT_LONG trace_wedge (double spin, double sscale, double wedge_amp, GMT_LONG lines, double *x, double *y)
{
	/* make a rotation rate wedge and return in x,y */

	/* Kurt Feigl, from code by D. Dong */

	/*   INPUT VARIABLES: */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   sscale      : scaling factor for size (radius) of wedge */
	/*   wedge_amp   : scaling factor for angular size of wedge */
	/*   spin        : CW spin rate in rad/yr */
	/*   line        : if true, draw lines                  */

	GMT_LONG nstep, i1, i, nump;
	double th, x0, y0, spin10, th0, x1, y1, s, c;

	/*     How far would we spin */
	spin10 = wedge_amp * spin;

	/*     set origin */
	th0 = x0 = y0 = 0.0;

	/*     go to zero */
	nump = 1;
	*x++ = x0;
	*y++ = y0;
	nstep = 100;

	/*     make a wedge as wide as the rotation in 10 Myr, */
	/*     with a line for every 0.2 microrad/yr */

	i1 = nstep;
	for (i = 0; i <= i1 ; ++i) {
		th = i * spin10 / nstep;
		sincos (th, &s, &c);
		x1 = x0 + s * sscale;
		y1 = y0 + c * sscale;
		++nump;
		*x++ = x1;
		*y++ = y1;
		if (lines && fabs (th-th0) >= 0.2) {
			/*          draw a line to the middle */
			/*           go to zero and come back */
			++nump;
			*x++ = x0;
			*y++ = y0;
			++nump;
			*x++ = x1;
			*y++ = y1;
			th0 = th;
		}
	}

	/*     go to zero */
	++nump;
	*x++ = x0;
	*y++ = y0;

	return nump;
}

GMT_LONG trace_sigwedge (double spin, double spinsig, double sscale, double wedge_amp, double *x, double *y)
{
	/* make a rotation rate uncertainty wedge and return in x,y */

	/* Kurt Feigl, from code by D. Dong */

	/*   INPUT VARIABLES: */
	/*   slat        - latitude, in degrees of arrow tail */
	/*   slon        - longitude in degrees of arrow tail */
	/*   sscale      : scaling factor for size (radius) of wedge */
	/*   wedge_amp   : scaling factor for angular size of wedge */
	/*   spin,spinsig:CW rotation rate and sigma in rad/yr */

	GMT_LONG nstep, i, nump;
	double th, x0, y0, spin10, sig10, th0, x1, y1, s, c;

	/*     How far would we spin */
	spin10 = wedge_amp * spin;
	sig10  = wedge_amp * spinsig;

	/*     set origin */
	x0 = y0 = th0 = 0.0;

	/*     go to zero */
	nump = 1;
	*x++ = x0;
	*y++ = y0;

	/*     make a dense wedge to show the uncertainty */
	nstep = 30;
	for (i = -nstep; i <= nstep; ++i) {
		th = spin10 + i * sig10 / nstep;
		sincos (th, &s, &c);
		x1 = x0 + s * sscale * .67;
		y1 = y0 + c * sscale * .67;
		++nump;
		*x++ = x1;
		*y++ = y1;
	}

	/* return to zero */

	++nump;
	*x++ = x0;
	*y++ = y0;
	return nump;
}

void paint_wedge (struct PSL_CTRL *PSL, double x0, double y0, double spin, double spinsig, double sscale, double wedge_amp, double t11, double t12, double t21, double t22,
	GMT_LONG polygon, double *rgb,
	GMT_LONG epolygon, double *ergb,
	GMT_LONG outline)
{

	/* Make a wedge at center x0,y0  */

#define NPOINTS 1000

	GMT_LONG npoints = NPOINTS, i;

	/* relative to center of ellipse */
	double dxe[NPOINTS], dye[NPOINTS];
	/* absolute paper coordinates */
	double axe[NPOINTS], aye[NPOINTS];

	/* draw wedge */

	npoints = trace_wedge (spin, 1.0, wedge_amp, TRUE, dxe, dye);

	for (i = 0; i <= npoints - 1; i++) transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (polygon) {
		PSL_setfill (PSL, rgb, TRUE);
		PSL_plotpolygon (PSL, axe, aye, npoints);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints, PSL_MOVE + PSL_STROKE);

	/* draw uncertainty wedge */

	npoints = trace_sigwedge (spin, spinsig, 1.0,wedge_amp, dxe, dye);

	for (i = 0; i < npoints - 1; i++) transform_local (x0, y0, dxe[i], dye[i], sscale, t11, t12, t21, t22, &axe[i], &aye[i]);

	if (epolygon) {
		PSL_setfill (PSL, ergb, TRUE);
		PSL_plotpolygon (PSL, axe, aye, npoints - 1);
	}
	else
		PSL_plotline (PSL, axe, aye, npoints - 1, PSL_MOVE + PSL_STROKE);
}
