/*--------------------------------------------------------------------
 *
 *    Copyright (c) 1996-2012 by G. Patau
 *    Copyright (c) 2013-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    Donated to the GMT project by G. Patau upon her retirement from IGPG
 *    Distributed under the Lesser GNU Public Licence
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*

pscoupe will read focal mechanisms from input file and plot beachballs on a cross-section. 
Focal mechanisms are specified in double couple, moment tensor, or principal axis.
PostScript code is written to stdout.

 Author:	Genevieve Patau
 Date:		9 September 1992
 Version:	5
 Roots:		based on psxy.c version 3.0,   Ported to GMT 5 by P. Wessel

 */

#include "gmt_dev.h"
#include "meca.h"
#include "utilmeca.h"

#define THIS_MODULE_CLASSIC_NAME	"pscoupe"
#define THIS_MODULE_MODERN_NAME	"coupe"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Plot cross-sections of focal mechanisms"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYdehipt" GMT_OPT("Hc")

#define DEFAULT_FONTSIZE		9.0	/* In points */
#define DEFAULT_OFFSET			3.0	/* In points */
#define DEFAULT_SYMBOL_SIZE		6.0	/* In points */

#define READ_CMT	0
#define READ_AKI	1
#define READ_PLANES	2
#define READ_AXIS	4
#define READ_TENSOR	8

#define PLOT_DC		1
#define PLOT_AXIS	2
#define PLOT_TRACE	4
#define PLOT_TENSOR	8

/* Control structure for pscoupe */

struct PSCOUPE_CTRL {
	struct A {	/* -A[<params>] */
		bool active, frame, polygon;
		int fuseau;
		char proj_type;
		double p_width, p_length, dmin, dmax;
		double xlonref, ylatref;
		struct GMT_PEN pen;
		struct nodal_plane PREF;
		char newfile[PATH_MAX], extfile[PATH_MAX];
	} A;
 	struct E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
	struct F {	/* Repeatable -F<mode>[<args>] */
		bool active;
	} F;
 	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S<format><scale>[+f<font>][+j<justify>][+o<dx>[/<dy>]] and -Fs */
		bool active;
		bool zerotrace;
		bool no_label;
		unsigned int readmode;
		unsigned int plotmode;
		unsigned int n_cols;
		int justify;
		int symbol;
		char P_symbol, T_symbol;
		double scale;
		double offset[2];
		struct GMT_FILL fill;
		struct GMT_FONT font;
	} S;
	struct T {	/* -Tnplane[/<pen>] */
		bool active;
		unsigned int n_plane;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<cpt> */
		bool active;
		char *file;
	} Z;
	struct A2 {	/* -Fa[size[/Psymbol[Tsymbol]]] */
		bool active;
		char P_symbol, T_symbol;
		double size;
	} A2;
	struct E2 {	/* -Fe<fill> */
		bool active;
		struct GMT_FILL fill;
	} E2;
 	struct G2 {	/* -Fg<fill> */
		bool active;
		struct GMT_FILL fill;
	} G2;
 	struct P2 {	/* -Fp[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} P2;
	struct R2 {	/* -Fr[<fill>] */
		bool active;
		struct GMT_FILL fill;
	} R2;
 	struct T2 {	/* -Ft[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} T2;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCOUPE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSCOUPE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->L.pen = C->T.pen = C->P2.pen = C->T2.pen = C->W.pen = GMT->current.setting.map_default_pen;
	/* Set width temporarily to -1. This will indicate later that we need to replace by W.pen */
	C->L.pen.width = C->T.pen.width = C->P2.pen.width = C->T2.pen.width = -1.0;
	C->L.active = true;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = DEFAULT_FONTSIZE;
	C->S.justify = PSL_TC;
	C->A2.size = DEFAULT_SYMBOL_SIZE * GMT->session.u2u[GMT_PT][GMT_INCH];
	C->A2.P_symbol = C->A2.T_symbol = PSL_CIRCLE;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL void rot_axis (struct AXIS A, struct nodal_plane PREF, struct AXIS *Ar) {
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

	double xn, xe, xz, x1, x2, x3, meca_zero_360();

	xn = cosd (A.dip) * cosd (A.str);
	xe = cosd (A.dip) * sind (A.str);
	xz = sind (A.dip);

	x1 = xn * sind (PREF.str) * cosd (PREF.dip) - xe * cosd (PREF.str) * cosd (PREF.dip) - xz * sind (PREF.dip);
	x2 = xn * cosd (PREF.str) + xe * sind (PREF.str);
	x3 = xn * sind (PREF.str) * sind (PREF.dip) - xe * cosd (PREF.str) * sind (PREF.dip) + xz * cosd (PREF.dip);

	Ar->dip = asind (x3);
	Ar->str = atan2d (x2, x1);
	if (Ar->dip < 0.0) {
		Ar->dip += 180.0;
		Ar->str = meca_zero_360 ((Ar->str += 180.0));
	}
}

GMT_LOCAL void rot_tensor (struct M_TENSOR mt, struct nodal_plane PREF, struct M_TENSOR *mtr) {
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

GMT_LOCAL void rot_nodal_plane (struct nodal_plane PLAN, struct nodal_plane PREF, struct nodal_plane *PLANR) {
/*
   Calcule l'azimut, le pendage, le glissement relatifs d'un
   mecanisme par rapport a un plan de reference PREF
   defini par son azimut et son pendage.
   On regarde la demi-sphere derriere le plan.
   Les angles sont en degres.

   Genevieve Patau, 8 septembre 1992.
*/

	double dfi = PLAN.str - PREF.str;
	double sd, cd, sdfi, cdfi, srd, crd;
	double sir, cor, cdr, sr, cr;

	sincosd (PLAN.dip, &sd, &cd);
	sincosd (dfi, &sdfi, &cdfi);
	sincosd (PREF.dip, &srd, &crd);
	sincosd (PLAN.rake, &sir, &cor);

	cdr = cd * crd + cdfi * sd * srd;

	cr = - sd * sdfi;
	sr = (sd * crd * cdfi - cd * srd);
	PLANR->str = d_atan2d (sr, cr);
	if (cdr < 0.)  PLANR->str += 180.0;
	PLANR->str = meca_zero_360 (PLANR->str);
	PLANR->dip = acosd (fabs (cdr));
	cr = cr * (sir * (cd * crd * cdfi + sd * srd) - cor * crd * sdfi) + sr * ( cor * cdfi + sir * cd * sdfi);
	sr = (cor * srd * sdfi + sir * (sd * crd - cd * srd * cdfi));
	PLANR->rake = d_atan2d (sr, cr);
	if (cdr < 0.) {
		PLANR->rake +=  180.0;
		if (PLANR->rake > 180.0) PLANR->rake -= 360.0;
	}
}

GMT_LOCAL void rot_meca (st_me meca, struct nodal_plane PREF, st_me *mecar) {
/*
   Projection d'un mecanisme sur un plan donne PREF.
   C'est la demi-sphere derriere le plan qui est representee.
   Les angles sont en degres.

   Genevieve Patau, 7 septembre 1992.
*/

	if (fabs (meca.NP1.str - PREF.str) < EPSIL && fabs (meca.NP1.dip - PREF.dip) < EPSIL) {
		mecar->NP1.str = 0.;
		mecar->NP1.dip = 0.;
		mecar->NP1.rake = meca_zero_360 (270. - meca.NP1.rake);
	}
	else
		rot_nodal_plane (meca.NP1, PREF, &mecar->NP1);

	if (fabs (meca.NP2.str - PREF.str) < EPSIL && fabs (meca.NP2.dip - PREF.dip) < EPSIL) {
		mecar->NP2.str = 0.;
		mecar->NP2.dip = 0.;
		mecar->NP2.rake = meca_zero_360 (270. - meca.NP2.rake);
	}
	else
		rot_nodal_plane (meca.NP2, PREF, &mecar->NP2);

	if (cosd (mecar->NP2.dip) < EPSIL && fabs (mecar->NP1.rake - mecar->NP2.rake) < 90.0) {
		mecar->NP1.str += 180.0;
		mecar->NP1.rake += 180.0;
		mecar->NP1.str = meca_zero_360 (mecar->NP1.str);
		if (mecar->NP1.rake > 180.0) mecar->NP1.rake -= 360.0;
	}

	mecar->magms = meca.magms;
	mecar->moment.mant = meca.moment.mant;
	mecar->moment.exponent = meca.moment.exponent;
}

GMT_LOCAL int gutm (double lon, double lat, double *xutm, double *yutm, int fuseau) {
	double ccc = 6400057.7, eprim = 0.08276528;
	double alfe = 0.00507613, bete = 0.429451e-4;
	double game = 0.1696e-6;
	double aj2, aj4, aj6, amo, al, arcme;
	double si, co, ecoxi, eta, gn, uuu, vvv, xi;

	if (fuseau == 0) fuseau = irint (floor ((lon + 186.) / 6.));

	/* calcul des coordonnees utm */
	amo = ((double)fuseau * 6. - 183.);
	al = lat * D2R;
	sincos (al, &si, &co);
	xi = co * sind (lon - amo);
	xi = 0.5 * log((1. + xi) / (1. - xi));
	eta = atan2 (si, co * cosd (lon - amo)) - al;
	gn = ccc / sqrt(1. + (eprim * co) * (eprim * co));
	ecoxi = (eprim * co * xi) * (eprim * co * xi);
	*xutm = gn * xi * (1. + ecoxi / 6.);
	*yutm = gn * eta * (1. + ecoxi / 2.);

	/* calcul de arcme (longueur de l'arc de meridien) */
	uuu = co * si;
	vvv = co * co;
	aj2 = al + uuu;
	aj4 = (3. * aj2 + 2. * uuu * vvv) / 4.;
	aj6 = (5. * aj4 + 2. * uuu * vvv * vvv) / 3.;
	arcme = ccc * (al - alfe * aj2 + bete * aj4 - game * aj6);
	*xutm = (500000. + 0.9996 * *xutm) * 0.001;
	*yutm = (0.9996 * (*yutm + arcme)) * 0.001;
	return (fuseau);
}

GMT_LOCAL int dans_coupe (double lon, double lat, double depth, double xlonref, double ylatref, int fuseau, double str, double dip, double p_length, double p_width, double *distance, double *n_dep) {
/* if fuseau < 0, cartesian coordinates */

	double xlon, ylat, largeur, sd, cd, ss, cs;

	if (fuseau >= 0)
		gutm (lon, lat, &xlon, &ylat, fuseau);
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
	return (*distance >= 0. && *distance <= p_length && fabs (largeur) <= p_width);
}

#define CONSTANTE2 0.9931177
#define RAYON 6371.0
#define COORD_DEG 0
#define COORD_RAD 1
#define COORD_KM 2

GMT_LOCAL void distaz (double lat1, double lon1, double lat2, double lon2, double *distkm, double *azdeg, int syscoord) {
 /*
  Coordinates in degrees  : syscoord = 0
  Coordinates in radians : syscoord = 1
  Cartesian coordinates in km : syscoord = 2
*/

	double slat1, clat1, slon1, clon1, slat2, clat2, slon2, clon2;
	double a1, b1, g1, h1, a2, b2, c1, c3, c4, distrad;

	if (syscoord == COORD_KM) {
		*distkm = hypot (lon2 - lon1, lat2 - lat1);
		*azdeg = atan2d (lon2 - lon1, lat2 - lat1);
	}
	else {
		if (syscoord == COORD_DEG) {
			lat1 *= D2R;
			lon1 *= D2R;
			lat2 *= D2R;
			lon2 *= D2R;
			if ((M_PI_2 - fabs(lat1)) > EPSIL) lat1 = atan(CONSTANTE2 * tan(lat1));
			if ((M_PI_2 - fabs(lat2)) > EPSIL) lat2 = atan(CONSTANTE2 * tan(lat2));
		}
		sincos (lat1, &slat1, &clat1);
		sincos (lon1, &slon1, &clon1);
		sincos (lat2, &slat2, &clat2);
		sincos (lon2, &slon2, &clon2);

		a1 = clat1 * clon1;
		b1 = clat1 * slon1;
		g1 = slat1 * clon1;
		h1 = slat1 * slon1;

		a2 = clat2 * clon2;
		b2 = clat2 * slon2;

		c1 = a1 * a2 + b1 * b2 + slat1 * slat2;
		if (fabs(c1) < 0.94)
			distrad = acos(c1);
		else if (c1 > 0.)
			distrad = asin(sqrt((a1 - a2) * (a1 - a2) + (b1 - b2) * (b1 - b2) + (slat1 - slat2) * (slat1 - slat2)) / 2.) * 2.;
		else
			distrad = acos(sqrt((a1 + a2) * (a1 + a2) + (b1 + b2) * (b1 + b2) + (slat1 + slat2) * (slat1 + slat2)) / 2.) * 2.;
		*distkm = distrad * RAYON;

		c3 = (a2 - slon1) * (a2 - slon1) + (b2 + clon1) * (b2 + clon1) + slat2 * slat2 - 2.;
		c4 = (a2 - g1) * (a2 - g1) + (b2 - h1) * (b2 - h1) + (slat2 + clat1) * (slat2 + clat1) - 2.;
		*azdeg = atan2d (c3, c4);
	}

	if (*azdeg < 0.) *azdeg += 360.0;

	return;
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pscoupe synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -A<params> %s %s -S<format><scale>[+f<font>][+j<justify>][+o<dx>[/<dy>]]\n", name, GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-E<fill>] [-Fa[<size>][/<Psymbol>[<Tsymbol>]] [-Fe<fill>] [-Fg<fill>] [-Fr<fill>] [-Fp[<pen>]] [-Ft[<pen>]]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fs<symbol><size>+f<font>+o<dx>/<dy>+j<justify>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<fill>] %s[-L<pen>] [-M] [-N] %s%s\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<nplane>[/<pen>]] [%s] [%s] [-W<pen>] \n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<cpt>]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[%s] [%s] [%s]\n\t[%s]\n\t[%s]\n\t[%s] [%s] [%s]\n\n", API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify cross-section parameters. Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Aa<lon1/lat1/lon2/lat2/dip/p_width/dmin/dmax>[+f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ab<lon1/lat1/strike/p_length/dip/p_width/dmin/dmax>[+f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ac<x1/y1/x2/y2/dip/p_width/dmin/dmax>[+f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ad<x1/y1/strike/p_length/dip/p_width/dmin/max>[+f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add +f to get the frame from the cross-section parameters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select format type and symbol size.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append the format code for your input file:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a  Focal mechanism in Aki & Richard's convention:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike dip rake mag [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c  Focal mechanism in Global CMT convention\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike1 dip1 rake1 strike2 dip2 rake2 moment [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      with moment in 2 columns : mantissa and exponent corresponding to seismic moment in dynes-cm\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d  Closed double couple defined from seismic moment tensor (zero trace and zero determinant):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p  Focal mechanism defined with:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth strike1 dip1 strike2 fault mag [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      fault = -1/+1 for a normal/inverse fault\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m  Seismic (full) moment tensor:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t  Zero trace moment tensor defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x  Principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   y  Best double couple defined from principal axis:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z  Deviatoric part of the moment tensor (zero trace):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally add +f<font>+j<justify>+o<dx>[/<dy>] to change the label font, location and offset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   fontsize < 0 : no label written; offset is from the limit of the beach ball.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default label is above the beach ball. Append +u to plot it under.\n");
	GMT_Option (API, "J-,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	gmt_fill_syntax (API->GMT, 'E', NULL, "Set color used for extensive parts [Default is white]");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets various attributes of symbols depending on <mode>:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Plot axis. Default symbols are circles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Set color used for T_symbol [default as set by -E].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g Set color used for P_symbol [default as set by -G].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p Draw P_symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r Draw box behind labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s Select symbol type and symbol size (in %s). Choose between:\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t     st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle, (s)quare, (t)riangle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t Draw T_symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Set color used for compressive parts [Default is black]");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Draw line or symbol outline using the current pen (see -W) or sets pen attribute for outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set same size for any magnitude. Size is given with -S.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Do not print cross-section information to files\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Tn[/<pen>] draw nodal planes and circumference only to provide a transparent beach ball\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   using the current pen (see -W) or sets pen attribute.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 1 the only first nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 2 the only second nodal plane is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = 0 both nodal planes are plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If moment tensor is required, nodal planes overlay moment tensor.\n");
	GMT_Option (API, "U,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set pen attributes [%s]\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Use CPT to assign colors based on depth-value in 3rd column.\n");

	GMT_Option (API, "X,c,di,e,h,p,i,t,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pscoupe and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	char txt[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *p = NULL;
	struct GMT_OPTION *opt = NULL;
	double lon1, lat1, lon2, lat2;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Cross-section definition */
				Ctrl->A.active = true;
				Ctrl->A.proj_type = opt->arg[0];
				if ((p = strstr (opt->arg, "+f"))) {	/* Get the frame from the cross-section parameters */
					Ctrl->A.frame = true;
					p[0] = '\0';	/* Chop off modifier */
				}
				else if (opt->arg[strlen(opt->arg)-1] == 'f') Ctrl->A.frame = true;
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'c') {
					sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
						&lon1, &lat1, &lon2, &lat2, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
					distaz (lat1, lon1, lat2, lon2, &Ctrl->A.p_length, &Ctrl->A.PREF.str, Ctrl->A.proj_type == 'a' ?  COORD_DEG : COORD_KM);
					sprintf (Ctrl->A.newfile, "A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, lon1, lat1, lon2, lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (Ctrl->A.extfile, "A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, lon1, lat1, lon2, lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				}
				else {
					sscanf (&opt->arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
						&lon1, &lat1, &Ctrl->A.PREF.str, &Ctrl->A.p_length, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
					sprintf (Ctrl->A.newfile, "A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, lon1, lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (Ctrl->A.extfile, "A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, lon1, lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				}
				Ctrl->A.PREF.rake = 0.;
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'b')
					Ctrl->A.fuseau = gutm (lon1, lat1, &Ctrl->A.xlonref, &Ctrl->A.ylatref, 0);
				else {
					Ctrl->A.fuseau = -1;
					Ctrl->A.xlonref = lon1;
					Ctrl->A.ylatref = lat1;
				}
				Ctrl->A.polygon = true;
				if (p) p[0] = '+';	/* Restore modifier */
				break;

			case 'E':	/* Set color for extensive parts  */
				Ctrl->E.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->E.fill))) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				Ctrl->A.polygon = true;
				break;
			case 'F':	/* Set various symbol parameters  */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'a':	/* plot axis */
						Ctrl->A2.active = true;
						strncpy (txt, &opt->arg[1], GMT_LEN256-1);
						if ((p = strchr (txt, '/')) != NULL) p[0] = '\0';
						if (txt[0]) Ctrl->A2.size = gmt_M_to_inch (GMT, txt);
						if (p) {
							p++;
							switch (strlen (p)) {
								case 1:
									Ctrl->A2.P_symbol = Ctrl->A2.T_symbol = p[0];
									break;
								case 2:
									Ctrl->A2.P_symbol = p[0], Ctrl->A2.T_symbol = p[1];
									break;
							}
						}
						break;
					case 'e':	/* Set color for T axis symbol */
						Ctrl->E2.active = true;
						if (gmt_getfill (GMT, &opt->arg[1], &Ctrl->E2.fill)) {
							gmt_fill_syntax (GMT, ' ', "Fe", " ");
							n_errors++;
						}
						break;
					case 'g':	/* Set color for P axis symbol */
						Ctrl->G2.active = true;
						if (gmt_getfill (GMT, &opt->arg[1], &Ctrl->G2.fill)) {
							gmt_fill_syntax (GMT, ' ', "Fg", " ");
							n_errors++;
						}
						break;
					case 'p':	/* Draw outline of P axis symbol [set outline attributes] */
						Ctrl->P2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->P2.pen)) {
							gmt_pen_syntax (GMT, ' ', "Fp", " ", 0);
							n_errors++;
						}
						break;
					case 'r':	/* draw box around text */
						Ctrl->R2.active = true;
						if (opt->arg[1] && gmt_getfill (GMT, &opt->arg[1], &Ctrl->R2.fill)) {
							gmt_fill_syntax (GMT, ' ', "Fr", " ");
							n_errors++;
						}
						break;
					case 's':	/* Only points : get symbol [and size] */
						Ctrl->S.active = true;
						Ctrl->S.symbol = opt->arg[1];
						if ((strstr (opt->arg, "+f")) || strstr (opt->arg, "+o") || strstr (opt->arg, "+j")) { 
							/* New syntax: -Fs<symbol><size>+f<font>+o<dx>/<dy>+j<justify> */
							char word[GMT_LEN256] = {""}, *c = NULL;

							/* parse beachball size */
							if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
							Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[2]);
							if (c) c[0] = '+';	/* Restore modifiers */

							if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
								Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
							if (gmt_get_modifier (opt->arg, 'f', word)) 
								n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
							if (gmt_get_modifier (opt->arg, 'o', word)) {
								if (gmt_get_pair (GMT, word, GMT_PAIR_DIM_DUP, Ctrl->S.offset) < 0) n_errors++;
							} else {	/* Set default offset */
								if (Ctrl->S.justify%4 != 2) /* Not center aligned */
									Ctrl->S.offset[0] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
								if (Ctrl->S.justify/4 != 1) /* Not middle aligned */
									Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
							}
							if (Ctrl->S.font.size <= 0.0) Ctrl->S.no_label = true;
						} else {	/* Old syntax: -Fs<symbol>[<size>[/fontsize[/offset[+u]]]] */	
							Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Set default offset */
							if ((p = strstr (opt->arg, "+u"))) {	/* Plot label under symbol [over] */
								Ctrl->S.justify = PSL_BC;
								p[0] = '\0';	/* Chop off modifier */
							}
							else if (opt->arg[strlen(opt->arg)-1] == 'u') {
								Ctrl->S.justify = PSL_BC; 
								opt->arg[strlen(opt->arg)-1] = '\0';
							}
							txt[0] = txt_b[0] = txt_c[0] = '\0';
							sscanf (&opt->arg[2], "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
							if (txt[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
							if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
							if (txt_c[0]) Ctrl->S.offset[1] = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
							if (Ctrl->S.font.size < 0.0) Ctrl->S.no_label = true;
							if (p) p[0] = '+';	/* Restore modifier */
						}
						if (gmt_M_is_zero (Ctrl->S.scale))
							Ctrl->S.n_cols = 4;
						else
							Ctrl->S.n_cols = 3;
						break;
					case 't':	/* Draw outline of T axis symbol [set outline attributes] */
						Ctrl->T2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->T2.pen)) {
							gmt_pen_syntax (GMT, ' ', "Ft", " ", 0);
							n_errors++;
						}
						break;
				}
				break;
			case 'G':	/* Set color for compressive parts */
				Ctrl->G.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				Ctrl->A.polygon = true;
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				Ctrl->L.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					gmt_pen_syntax (GMT, 'L', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not skip points outside border */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Switch of production of mechanism files */
				Ctrl->Q.active = true;
				break;
			case 'S':	/* Mechanisms : get format [and size] */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {	/* parse format */
					case 'c':
						Ctrl->S.readmode = READ_CMT;	Ctrl->S.n_cols = 11;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 'a':
						Ctrl->S.readmode = READ_AKI;	Ctrl->S.n_cols = 7;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 'p':
						Ctrl->S.readmode = READ_PLANES;	Ctrl->S.n_cols = 8;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 'x':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						Ctrl->S.plotmode = PLOT_TENSOR;
						break;
					case 'y':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						Ctrl->S.plotmode = PLOT_DC;
						break;
					case 't':
						Ctrl->S.readmode = READ_AXIS;	Ctrl->S.n_cols = 13;
						Ctrl->S.plotmode = PLOT_TRACE;
						break;
					case 'm':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						Ctrl->S.plotmode = PLOT_TENSOR;
						Ctrl->S.zerotrace = true;
						break;
					case 'd':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						Ctrl->S.plotmode = PLOT_DC;
						Ctrl->S.zerotrace = true;
						break;
					case 'z':
						Ctrl->S.readmode = READ_TENSOR;	Ctrl->S.n_cols = 10;
						Ctrl->S.plotmode = PLOT_TRACE;
						Ctrl->S.zerotrace = true;
						break;
					default:
						n_errors++;
						break;
				}

				if ((strstr (opt->arg, "+f")) || strstr (opt->arg, "+o") || strstr (opt->arg, "+j")) { 
					/* New syntax: -S<format><scale>+f<font>+o<dx>/<dy>+j<justify> */
					char word[GMT_LEN256] = {""}, *c = NULL;

					/* parse beachball size */
					if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
					Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
					if (c) c[0] = '+';	/* Restore modifiers */

					if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
						Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
					if (gmt_get_modifier (opt->arg, 'f', word)) 
						n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
					if (gmt_get_modifier (opt->arg, 'o', word)) {
						if (gmt_get_pair (GMT, word, GMT_PAIR_DIM_DUP, Ctrl->S.offset) < 0) n_errors++;
					} else {	/* Set default offset */
						if (Ctrl->S.justify%4 != 2) /* Not center aligned */
							Ctrl->S.offset[0] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
						if (Ctrl->S.justify/4 != 1) /* Not middle aligned */
							Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
					}
					if (Ctrl->S.font.size <= 0.0) Ctrl->S.no_label = true;
				} else {	/* Old syntax: -S<format><scale>[/fontsize[/offset]][+u] */
					Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Set default offset */
					if ((p = strstr (opt->arg, "+u"))) {	/* Plot label under symbol [over] */
						Ctrl->S.justify = PSL_BC;
						p[0] = '\0';	/* Chop off modifier */
					}
					else if (opt->arg[strlen(opt->arg)-1] == 'u') {
						Ctrl->S.justify = PSL_BC;
						opt->arg[strlen(opt->arg)-1] = '\0';
					}
					txt[0] = txt_b[0] = txt_c[0] = '\0';
					sscanf (&opt->arg[1], "%[^/]/%[^/]/%s", txt, txt_b, txt_c);
					if (txt[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt);
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
					if (txt_c[0]) Ctrl->S.offset[1] = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
					if (Ctrl->S.font.size < 0.0) Ctrl->S.no_label = true;
					if (p) p[0] = '+';	/* Restore modifier */
				}
				break;

			case 'T':
				Ctrl->T.active = true;
				sscanf (opt->arg, "%d", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && gmt_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					gmt_pen_syntax (GMT, 'T', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'Z':	/* Vary symbol color with z */
				Ctrl->Z.active = true;
				if (opt->arg[0]) Ctrl->Z.file = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (GMT->parent, &Ctrl->Z.active, &(Ctrl->Z.file));

	/* Check that the options selected are mutually consistent */

	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active, "Syntax error: Must specify -A option\n");
	//n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->S.active, "Syntax error: Cannot specify both -F and -S\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active, "Syntax error: Must specify -S option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.scale < 0.0, "Syntax error: -S must specify scale\n");

	/* Set to default pen where needed */

	if (Ctrl->L.pen.width  < 0.0) Ctrl->L.pen  = Ctrl->W.pen;
	if (Ctrl->T.pen.width  < 0.0) Ctrl->T.pen  = Ctrl->W.pen;
	if (Ctrl->T2.pen.width < 0.0) Ctrl->T2.pen = Ctrl->W.pen;
	if (Ctrl->P2.pen.width < 0.0) Ctrl->P2.pen = Ctrl->W.pen;

	/* Default -e<fill> and -g<fill> to -E<fill> and -G<fill> */

	if (!Ctrl->E2.active) Ctrl->E2.fill = Ctrl->E.fill;
	if (!Ctrl->G2.active) Ctrl->G2.fill = Ctrl->G.fill;

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_coupe (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: coupe\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pscoupe (V_API, mode, args);
}

int GMT_pscoupe (void *V_API, int mode, void *args) {
	int n_rec = 0, n_plane_old = 0, form = 0, error;
	int i, transparence_old = 0, not_defined = 0;
	int n_scanned = 0;
	FILE *pnew = NULL, *pext = NULL;

	double size, xy[2], xynew[2] = {0.0}, plot_x, plot_y, angle = 0.0, n_dep, distance, fault, depth;
	double P_x, P_y, T_x, T_y, *in = NULL;

	char event_title[GMT_BUFSIZ] = {""}, Xstring[GMT_BUFSIZ] = {""}, Ystring[GMT_BUFSIZ] = {""};

	st_me meca, mecar;
	struct MOMENT moment;
	struct M_TENSOR mt, mtr;
	struct AXIS T, N, P, Tr, Nr, Pr;
	struct GMT_PALETTE *CPT = NULL;
	struct GMT_RECORD *In = NULL;

	struct PSCOUPE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pscoupe main code ----------------------------*/

	gmt_M_memset (&meca, 1, meca);
	gmt_M_memset (&moment, 1, moment);

	if (Ctrl->Z.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, NULL)) == NULL) {
			Return (API->error);
		}
	}

	if (Ctrl->A.frame) {
		GMT->common.R.wesn[XLO] = 0.0;
		GMT->common.R.wesn[XHI] = Ctrl->A.p_length;
		GMT->common.R.wesn[YLO] = Ctrl->A.dmin;
		GMT->common.R.wesn[YHI] = Ctrl->A.dmax;
		if (gmt_M_is_zero (Ctrl->A.PREF.dip)) Ctrl->A.PREF.dip = 1.0;
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);

	gmt_setpen (GMT, &Ctrl->W.pen);
	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (!Ctrl->Q.active) {
		pnew = fopen (Ctrl->A.newfile, "w");
		pext = fopen (Ctrl->A.extfile, "w");
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_MIXED, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				fclose (pnew);
				fclose (pext);
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

		in = In->data;
		n_rec++;
		size = Ctrl->S.scale;

		/* Must examine the trailing text for optional columns: newX, newY and title */
		/* newX and newY are not used in pscoupe, but we promised psmeca and pscoupe can use the same input file */
		if (In->text) {
			n_scanned = sscanf (In->text, "%s %s %[^\n]s\n", Xstring, Ystring, event_title);
			if (n_scanned >= 2) { /* Got new x,y coordinates and possibly event title */
				unsigned int type;
				if (GMT->current.setting.io_lonlat_toggle[GMT_IN]) {	/* Expect lat lon but watch for junk */
					if ((type = gmt_scanf_arg (GMT, Ystring, GMT_IS_LON, false, &xynew[GMT_X])) == GMT_IS_NAN) xynew[GMT_X] = GMT->session.d_NaN;
					if ((type = gmt_scanf_arg (GMT, Xstring, GMT_IS_LAT, false, &xynew[GMT_Y])) == GMT_IS_NAN) xynew[GMT_Y] = GMT->session.d_NaN;
				}
				else {	/* Expect lon lat but watch for junk */
					if ((type = gmt_scanf_arg (GMT, Xstring, GMT_IS_LON, false, &xynew[GMT_X])) == GMT_IS_NAN) xynew[GMT_X] = GMT->session.d_NaN;
					if ((type = gmt_scanf_arg (GMT, Ystring, GMT_IS_LAT, false, &xynew[GMT_Y])) == GMT_IS_NAN) xynew[GMT_Y] = GMT->session.d_NaN;
				}
				if (gmt_M_is_dnan (xynew[GMT_X]) || gmt_M_is_dnan (xynew[GMT_Y])) {	/* Got part of a title, presumably */
					xynew[GMT_X] = 0.0;	 /* revert to 0 if newX and newY are not given */
					xynew[GMT_Y] = 0.0;
					if (!(strchr ("XY", Xstring[0]) && strchr ("XY", Ystring[0])))	/* Old meca format with X Y placeholders */
			strncpy (event_title, In->text, GMT_BUFSIZ-1);
				}
				else if (n_scanned == 2)	/* Got no title */
					event_title[0] = '\0';
			}
			else if (n_scanned == 1)	/* Only got event title */
				strncpy (event_title, In->text, GMT_BUFSIZ-1);
			else	/* Got no title */
				event_title[0] = '\0';
		}

		depth = in[2];

		if (!dans_coupe (in[GMT_X], in[GMT_Y], depth, Ctrl->A.xlonref, Ctrl->A.ylatref, Ctrl->A.fuseau, Ctrl->A.PREF.str,
			Ctrl->A.PREF.dip, Ctrl->A.p_length, Ctrl->A.p_width, &distance, &n_dep) && !Ctrl->N.active)
			continue;

		xy[GMT_X] = distance;
		xy[GMT_Y] = n_dep;

		if (!Ctrl->N.active) {
			gmt_map_outside (GMT, xy[GMT_X], xy[GMT_Y]);
			if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
		}

		if (Ctrl->Z.active) gmt_get_fill_from_z (GMT, CPT, depth, &Ctrl->G.fill);

		gmt_geo_to_xy (GMT, xy[GMT_X], xy[GMT_Y], &plot_x, &plot_y);

		if (Ctrl->S.symbol) {
			if (!Ctrl->Q.active) {
				fprintf (pnew, "%f %f %f %f\n", distance, n_dep, depth, in[3]);
				fprintf (pext, "%s\n", In->text);
			}
			gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->L.active);
			PSL_plotsymbol (PSL, plot_x, plot_y, &size, Ctrl->S.symbol);
		}
		else if (Ctrl->S.readmode == READ_CMT) {
			meca.NP1.str    = in[3];
			meca.NP1.dip    = in[4];
			meca.NP1.rake   = in[5];
			meca.NP2.str    = in[6];
			meca.NP2.dip    = in[7];
			meca.NP2.rake   = in[8];
			moment.mant     = in[9];
			moment.exponent = irint (in[10]);
			if (moment.exponent == 0) meca.magms = in[9];
			rot_meca (meca, Ctrl->A.PREF, &mecar);
		}
		else if (Ctrl->S.readmode == READ_AKI) {
			meca.NP1.str    = in[3];
			meca.NP1.dip    = in[4];
			meca.NP1.rake   = in[5];
			meca.magms      = in[6];
			moment.mant     = meca.magms;
			moment.exponent = 0;
			meca_define_second_plane (meca.NP1, &meca.NP2);
			rot_meca (meca, Ctrl->A.PREF, &mecar);
		}
		else if (Ctrl->S.readmode == READ_PLANES) {
			meca.NP1.str = in[3];
			meca.NP1.dip = in[4];
			meca.NP2.str = in[5];
			fault        = in[6];
			meca.magms   = in[7];

			moment.exponent = 0;
			moment.mant = meca.magms;
			meca.NP2.dip = meca_computed_dip2 (meca.NP1.str, meca.NP1.dip, meca.NP2.str);
			if (meca.NP2.dip == 1000.0) {
				not_defined = true;
				transparence_old = Ctrl->T.active;
				n_plane_old = Ctrl->T.n_plane;
				Ctrl->T.active = true;
				Ctrl->T.n_plane = 1;
				meca.NP1.rake = 1000.0;
				GMT_Report (API, GMT_MSG_VERBOSE, "Second plane is not defined for event %s only first plane is plotted.\n", In->text);
			}
			else
				meca.NP1.rake = meca_computed_rake2 (meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
			meca.NP2.rake = meca_computed_rake2 (meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip, fault);
			rot_meca (meca, Ctrl->A.PREF, &mecar);

		}
		else if (Ctrl->S.readmode == READ_AXIS) {
			T.val = in[3];
			T.str = in[4];
			T.dip = in[5];
			T.e = irint (in[12]);

			N.val = in[6];
			N.str = in[7];
			N.dip = in[8];
			N.e = irint (in[12]);

			P.val = in[9];
			P.str = in[10];
			P.dip = in[12];
			P.e = irint (in[12]);

/*
F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
Definition of scalar moment.
*/
			meca.moment.exponent = T.e;
			meca.moment.mant = sqrt (squared(T.val) + squared (N.val) + squared (P.val)) / M_SQRT2;
			meca.magms = 0.;

/* normalization by M0 */
			T.val /= meca.moment.mant;
			N.val /= meca.moment.mant;
			P.val /= meca.moment.mant;

			rot_axis (T, Ctrl->A.PREF, &Tr);
			rot_axis (N, Ctrl->A.PREF, &Nr);
			rot_axis (P, Ctrl->A.PREF, &Pr);
			Tr.val = T.val;
			Nr.val = N.val;
			Pr.val = P.val;
			Tr.e = T.e;
			Nr.e = N.e;
			Pr.e = P.e;

			if (Ctrl->S.plotmode == PLOT_DC || Ctrl->T.active) meca_axe2dc (Tr, Pr, &meca.NP1, &meca.NP2);
		}
		else if (Ctrl->S.readmode == READ_TENSOR) {
			for (i = 3; i < 9; i++) mt.f[i-3] = in[i];
			mt.expo = irint (in[i]);

			moment.exponent = mt.expo;
/*
F. A. Dahlen and Jeroen Tromp, Theoretical Seismology, Princeton, 1998, p.167.
Definition of scalar moment.
*/
			moment.mant = sqrt (squared (mt.f[0]) + squared (mt.f[1]) + squared (mt.f[2]) + 2.0 * (squared (mt.f[3]) + squared (mt.f[4]) + squared (mt.f[5]))) / M_SQRT2;
			meca.magms = 0.0;

/* normalization by M0 */
			for (i = 0; i <= 5; i++) mt.f[i] /= moment.mant;

			rot_tensor (mt, Ctrl->A.PREF, &mtr);
			meca_moment2axe (GMT, mtr, &T, &N, &P);

			if (Ctrl->S.plotmode == PLOT_DC || Ctrl->T.active) meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
		}

		if (!Ctrl->S.symbol) {
			if (Ctrl->M.active) {
				moment.mant = 4.0;
				moment.exponent = 23;
			}

			size = (meca_computed_mw (moment, meca.magms) / 5.0) * Ctrl->S.scale;

			if (!Ctrl->Q.active) fprintf (pext, "%s\n", In->text);
			if (Ctrl->S.readmode == READ_AXIS) {
				if (!Ctrl->Q.active)
					fprintf (pnew, "%f %f %f %f %f %f %f %f %f %f %f %f %d 0 0 %s\n",
					xy[0], xy[1], depth, Tr.val, Tr.str, Tr.dip, Nr.val, Nr.str, Nr.dip,
					Pr.val, Pr.str, Pr.dip, moment.exponent, event_title);
				T = Tr;
				N = Nr;
				P = Pr;
			}
			else if (Ctrl->S.readmode == READ_TENSOR) {
				if (!Ctrl->Q.active)
					fprintf (pnew, "%f %f %f %f %f %f %f %f %f %d 0 0 %s\n",
					xy[0], xy[1], depth, mtr.f[0], mtr.f[1], mtr.f[2], mtr.f[3], mtr.f[4], mtr.f[5],
					moment.exponent, event_title);
				mt = mtr;
			}
			else {
				if (!Ctrl->Q.active)
					fprintf (pnew, "%f %f %f %f %f %f %f %f %f %f %d 0 0 %s\n",
					xy[0], xy[1], depth, mecar.NP1.str, mecar.NP1.dip, mecar.NP1.rake,
					mecar.NP2.str, mecar.NP2.dip, mecar.NP2.rake,
					moment.mant, moment.exponent, event_title);
				meca = mecar;
			}

			if (Ctrl->S.plotmode == PLOT_TENSOR) {
				gmt_setpen (GMT, &Ctrl->L.pen);
				meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.zerotrace, n_rec);
			}

			if (Ctrl->S.zerotrace) {
				gmt_setpen (GMT, &Ctrl->W.pen);
				meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, NULL, NULL, true, true, n_rec);
			}

			if (Ctrl->T.active) {
				gmt_setpen (GMT, &Ctrl->T.pen);
				meca_ps_plan (GMT, PSL, plot_x, plot_y, meca, size, Ctrl->T.n_plane);
				if (not_defined) {
					not_defined = false;
					Ctrl->T.active = transparence_old;
					Ctrl->T.n_plane = n_plane_old;
				}
			}
			else if (Ctrl->S.plotmode == PLOT_DC) {
				gmt_setpen (GMT, &Ctrl->L.pen);
				meca_ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
			}

			if (Ctrl->A2.active) {
				if (Ctrl->S.readmode != READ_TENSOR && Ctrl->S.readmode != READ_AXIS) meca_dc2axe (meca, &T, &N, &P);
				meca_axis2xy (plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
				gmt_setpen (GMT, &Ctrl->P2.pen);
				gmt_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active);
				PSL_plotsymbol (PSL, P_x, P_y, &Ctrl->A2.size, Ctrl->A2.P_symbol);
				gmt_setpen (GMT, &Ctrl->T2.pen);
				gmt_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active);
				PSL_plotsymbol (PSL, T_x, T_y, &Ctrl->A2.size, Ctrl->A2.T_symbol);
			}
		}

		if (!Ctrl->S.no_label) {
			int label_justify = 0;
			double label_x, label_y;
			double label_offset[2];

			label_justify = gmt_flip_justify(GMT, Ctrl->S.justify);
			label_offset[0] = label_offset[1] = GMT_TEXT_CLEARANCE * 0.01 * Ctrl->S.font.size / PSL_POINTS_PER_INCH;

			label_x = plot_x + 0.5 * (Ctrl->S.justify%4 - label_justify%4) * size * 0.5;
			label_y = plot_y + 0.5 * (Ctrl->S.justify/4 - label_justify/4) * size * 0.5;

			/* Also deal with any justified offsets if given */
			if (Ctrl->S.justify%4 == 1) /* Left aligned */
				label_x -= Ctrl->S.offset[0];
			else /* Right or center aligned */
				label_x += Ctrl->S.offset[0];
			if (Ctrl->S.justify/4 == 0) /* Bottom aligned */
				label_y -= Ctrl->S.offset[1];
			else /* Top or middle aligned */
				label_y += Ctrl->S.offset[1];

			gmt_setpen (GMT, &Ctrl->W.pen);
			PSL_setfill (PSL, Ctrl->R2.fill.rgb, false);
			if (Ctrl->R2.active) PSL_plottextbox (PSL, label_x, label_y, Ctrl->S.font.size, event_title, angle, label_justify, label_offset, 0);
			form = gmt_setfont(GMT, &Ctrl->S.font);
			PSL_plottext (PSL, label_x, label_y, Ctrl->S.font.size, event_title, angle, label_justify, form);
		}
	} while (true);

	if (!Ctrl->Q.active) {
		fclose (pnew);
		fclose (pext);
	}

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of records read: %li\n", n_rec);

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setdash (PSL, NULL, 0);
	gmt_map_basemap (GMT);
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Deal with the fact that -R was set implicitly via -A */
		char r_code[GMT_LEN256] = {""};
		int id = gmt_get_option_id (0, "R");		/* The -RP history item */
		if (GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);	/*  Free previous -R */
		sprintf (r_code, "%.16g/%.16g/%.16g/%.16g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		GMT->init.history[id] = strdup (r_code);
	}

	Return (GMT_NOERROR);
}
