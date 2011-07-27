/*	$Id$
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

#include "gmt.h"
#include "meca.h"

#define PIDEG 180
#define PI2DEG 90
#define PIIDEG 360

void rot_axis (struct AXIS A, struct nodal_plane PREF, struct AXIS *Ar)
{
	/*
	 * Change coordinates of axis from
	 * north,east,down
	 * to
	 * x1 = steepest descent upwards
	 * x2 = strike direction of reference plane
	 * x3 = x1^x2
	 *
	 * new strike is angle counted from x1 (0 <= strike < 360)
	 * new dip is counted from strike in x3 direction (0 <= dip <= 90)
	 * 19 April 1999
	 *
	 */

	double xn, xe, xz, x1, x2, x3, zero_360();

	xn = cosd (A.dip) * cosd (A.str);
	xe = cosd (A.dip) * sind (A.str);
	xz = sind (A.dip);

	x1 = xn * sind (PREF.str) * cosd (PREF.dip) - xe * cosd (PREF.str) * cosd (PREF.dip) - xz * sind (PREF.dip);
	x2 = xn * cosd (PREF.str) + xe * sind (PREF.str);
	x3 = xn * sind (PREF.str) * sind (PREF.dip) - xe * cosd (PREF.str) * sind (PREF.dip) + xz * cosd (PREF.dip);

	Ar->dip = asind (x3);
	Ar->str = atan2d(x2, x1);
	if (Ar->dip < 0.0) {
		Ar->dip += 180.0;
		Ar->str = zero_360 ((Ar->str += 180));
	}
}

void rot_tensor (struct M_TENSOR mt, struct nodal_plane PREF, struct M_TENSOR *mtr)
{
	/*
	 *
	 * Change coordinates from
	 * (r,t,f) (upwards, south, east)
	 * to
	 * x1 = x2^x3
	 * x2 = steepest descent downwards
	 * x3 = strike direction of reference plane
	 *
	 * 19 April 1999
	 *
	 */
	double a = PREF.str * D2R, d =  PREF.dip * D2R;
	double sa, ca, s2a, c2a, sa2, ca2, sd, cd, s2d, c2d, sd2, cd2;

	sincos (a, &sa, &ca);
	sincos (2.0 * a, &s2a, &c2a);
 	sincos (d, &sd, &cd);
 	sincos (2.0 * d, &s2d, &c2d);
	sa2 = sa * sa;	ca2 = ca * ca;
	sd2 = sd * sd; cd2 = cd * cd;

	mtr->f[0] = cd2*mt.f[0] + sa2*sd2*mt.f[1] + ca2*sd2*mt.f[2] +
		sa*s2d*mt.f[3] + ca*s2d*mt.f[4] + s2a*sd2*mt.f[5];
	mtr->f[1] = sd2*mt.f[0] + sa2*cd2*mt.f[1] + ca2*cd2*mt.f[2] -
		sa*s2d*mt.f[3] - ca*s2d*mt.f[4] + s2a*cd2*mt.f[5];
	mtr->f[2] = ca2*mt.f[1] + sa2*mt.f[2] - s2a*mt.f[5];
	mtr->f[3] = s2d*(- mt.f[0] + sa2*mt.f[1] + ca2*mt.f[2])/2. +
		c2d*(sa*mt.f[3] + ca*mt.f[4]) + s2a*s2d*mt.f[5]/2.;
	mtr->f[4] = s2a*sd*(- mt.f[1] + mt.f[2])/2. - ca*cd*mt.f[3] +
		sa*cd*mt.f[4] - c2a*sd*mt.f[5];
	mtr->f[5] = s2a*cd*(- mt.f[1] + mt.f[2])/2. + ca*sd*mt.f[3] -
		sa*sd*mt.f[4] - c2a*cd*mt.f[5];
}

void rot_nodal_plane (struct nodal_plane PLAN,struct nodal_plane PREF,struct nodal_plane *PLANR)
/*
   Calcule l'azimut, le pendage, le glissement relatifs d'un
   mecanisme par rapport a un plan de reference PREF
   defini par son azimut et son pendage.
   On regarde la demi-sphere derriere le plan.
   Les angles sont en degres.

   Genevieve Patau, 8 septembre 1992.
*/
      
{
	double dfi = PLAN.str - PREF.str;
	double sd, cd, sdfi, cdfi, srd, crd;
	double sir, cor, cdr, sdr, sr, cr;

	sincosd (PLAN.dip, &sd, &cd);
	sincosd (dfi, &sdfi, &cdfi);
	sincosd (PREF.dip, &srd, &crd);
	sincosd (PLAN.rake, &sir, &cor);

	cdr = cd * crd + cdfi * sd * srd;
	sdr = sqrt(1. - cdr * cdr);

	cr = - sd * sdfi / sdr;
	sr = (sd * crd * cdfi - cd * srd) / sdr;
	PLANR->str = d_atan2d(sr, cr);
	if (cdr < 0.)  PLANR->str += PIDEG;
	PLANR->str = zero_360(PLANR->str);
	PLANR->dip = acosd (fabs (cdr));
	cr = cr * (sir * (cd * crd * cdfi + sd * srd) - cor * crd * sdfi) + sr * ( cor * cdfi + sir * cd * sdfi);
	sr = (cor * srd * sdfi + sir * (sd * crd - cd * srd * cdfi)) / sdr;
	PLANR->rake = d_atan2d(sr, cr);
	if (cdr < 0.) {
		PLANR->rake +=  PIDEG;
		if (PLANR->rake > PIDEG) PLANR->rake -= PIIDEG;
	}
}

void rot_meca (st_me meca,struct nodal_plane PREF,st_me *mecar)
/*
   Projection d'un mecanisme sur un plan donne PREF.
   C'est la demi-sphere derriere le plan qui est representee.
   Les angles sont en degres.

   Genevieve Patau, 7 septembre 1992.
*/

{
	struct nodal_plane NP1, NP2;
	struct nodal_plane NPR1, NPR2;

	if (fabs (meca.NP1.str - PREF.str) < EPSIL && fabs (meca.NP1.dip - PREF.dip) < EPSIL) {
		mecar->NP1.str = 0.;
		mecar->NP1.dip = 0.;
		mecar->NP1.rake = zero_360(270. - meca.NP1.rake);
	}
	else {
		NP1.str = meca.NP1.str;
		NP1.dip = meca.NP1.dip;
		NP1.rake = meca.NP1.rake;
		rot_nodal_plane(NP1, PREF, &NPR1);
		mecar->NP1.str = NPR1.str;
		mecar->NP1.dip = NPR1.dip;
		mecar->NP1.rake = NPR1.rake;
	}

	if (fabs (meca.NP2.str - PREF.str) < EPSIL && fabs (meca.NP2.dip - PREF.dip) < EPSIL) {
		mecar->NP2.str = 0.;
		mecar->NP2.dip = 0.;
		mecar->NP2.rake = zero_360(270. - meca.NP2.rake);
	}
	else {
		NP2.str = meca.NP2.str;
		NP2.dip = meca.NP2.dip;
		NP2.rake = meca.NP2.rake;
		rot_nodal_plane(NP2, PREF, &NPR2);
		mecar->NP2.str = NPR2.str;
		mecar->NP2.dip = NPR2.dip;
		mecar->NP2.rake = NPR2.rake;
	}

	if (cosd (mecar->NP2.dip) < EPSIL && mecar->NP1.rake * mecar->NP2.rake < 0.) {
		mecar->NP2.str += PIDEG;
		mecar->NP2.rake += PIDEG;
		mecar->NP2.str = zero_360(mecar->NP2.str);
		if (mecar->NP2.rake > PIDEG) mecar->NP2.rake -= PIIDEG; 
	}

	mecar->magms = meca.magms;
	mecar->moment.mant = meca.moment.mant;
	mecar->moment.exponent = meca.moment.exponent;
}


GMT_LONG gutm(double lon , double lat , double *xutm , double *yutm, GMT_LONG fuseau)
{
	double ccc = 6400057.7, eprim = 0.08276528;
	double alfe = 0.00507613, bete = 0.429451e-4;
	double game = 0.1696e-6, pirad = 0.0174533;
	double aj2, aj4, aj6, amo, al, arcme;
	double co, ecoxi, eta, gn, uuu, vvv, xi;

	if (fuseau == 0) fuseau = (GMT_LONG)((lon + 186.) / 6.);

	/* calcul des coordonnees utm */
	amo = ((double)fuseau * 6. - 183.);
	al = lat * pirad;
	co = cos(al);
	xi = co * sin((lon - amo) * pirad);
	xi = 0.5 * log((1. + xi) / (1. - xi));
	eta = atan(sin(al) / (co * cos((lon - amo) * pirad))) - al;
	gn = ccc / sqrt(1. + (eprim * co) * (eprim * co));
	ecoxi = (eprim * co * xi) * (eprim * co * xi);
	*xutm = gn * xi * (1. + ecoxi / 6.);
	*yutm = gn * eta * (1. + ecoxi / 2.);

	/* calcul de arcme (longueur de l'arc de meridien) */
	vvv = cos(al);
	uuu = vvv * sin(al);
	vvv = vvv * vvv;
	aj2 = al + uuu;
	aj4 = (3. * aj2 + 2. * uuu * vvv) / 4.;
	aj6 = (5. * aj4 + 2. * uuu * vvv * vvv) / 3.;
	arcme = ccc * (al - alfe * aj2 + bete * aj4 - game * aj6);
	*xutm = (500000. + 0.9996 * *xutm) * 0.001;
	*yutm = (0.9996 * (*yutm + arcme)) * 0.001;
	return(fuseau);
}

GMT_LONG dans_coupe (double lon, double lat, double depth, double xlonref, double ylatref, GMT_LONG fuseau, double str, double dip, double p_length, double p_width, double *distance, double *n_dep)

/* if fuseau < 0, cartesian coordinates */

{
     double xlon, ylat, largeur, sd, cd, ss, cs;
     GMT_LONG test;

	if (fuseau >= 0)
		gutm(lon, lat, &xlon, &ylat, fuseau);
	else {
		xlon = lon;
		ylat = lat;
	}
	sincosd (dip, &sd, &cd);
	sincosd (str, &ss, &cs);
	largeur = (xlon - xlonref) * cs - (ylat - ylatref) * ss;
	*n_dep = depth * sd + largeur * cosd (dip);
	largeur = depth * cosd (dip) - largeur * sd;
	*distance = (ylat - ylatref) * cs + (xlon - xlonref) * ss;
	if (*distance >= 0. && *distance <= p_length && fabs (largeur) <= p_width)
		test = 1;
	else
		test = 0;
	return (test);
}
