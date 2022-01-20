/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2013-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Copyright (c) 1996-2012 by G. Patau
 *	Donated to the GMT project by G. Patau upon her retirement from IGPG
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
#define THIS_MODULE_KEYS	"<D{,>?}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYdehipqt" GMT_OPT("c")

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
	struct PSCOUPE_A {	/* -Aa|b|c|d<params>[+c[t|n]][+d<dip>][+r[a|e|<dx>]][+w<width>][+z[s]a|e|<dz>|<min>/<max>] */
		bool active, frame, polygon, force, exact[2];
		int fuseau;
		int report;	/* GMT_IS_FLOAT: print w e s n, GMT_IS_TEXT: print "-Rw/e/s/n" based on -A+r */
		char proj_type;
		double p_width, p_length, dmin, dmax, dz, dx;
		double lon1, lat1, lon2, lat2;
		double xlonref, ylatref;
		struct GMT_PEN pen;
		struct nodal_plane PREF;
		char newfile[PATH_MAX], extfile[PATH_MAX];
	} A;
	struct PSCOUPE_C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
 	struct PSCOUPE_E {	/* -E<fill> */
		bool active;
		struct GMT_FILL fill;
	} E;
	struct PSCOUPE_F {	/* Repeatable -F<mode>[<args>] */
		bool active;
	} F;
 	struct PSCOUPE_G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct PSCOUPE_H {	/* -H read overall scaling factor for symbol size and pen width */
		bool active;
		unsigned int mode;
		double value;
	} H;
	struct PSCOUPE_I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file */
		double value;
	} I;
	struct PSCOUPE_L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct PSCOUPE_N {	/* -N */
		bool active;
	} N;
	struct PSCOUPE_Q {	/* -Q */
		bool active;
	} Q;
	struct PSCOUPE_S {	/* -S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>]  and -Fs */
		#include "meca_symbol.h"
	/* Extra parameters for coupe */
		bool zerotrace;
		int symbol;
	} S;
	struct PSCOUPE_T {	/* -T<nplane>[/<pen>] */
		bool active;
		unsigned int n_plane;
		struct GMT_PEN pen;
	} T;
	struct PSCOUPE_W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSCOUPE_A2 {	/* -Fa[<size>[/<Psymbol>[<Tsymbol>]]] */
		bool active;
		char P_symbol, T_symbol;
		double size;
	} A2;
	struct PSCOUPE_E2 {	/* -Fe<fill> */
		bool active;
		struct GMT_FILL fill;
	} E2;
 	struct PSCOUPE_G2 {	/* -Fg<fill> */
		bool active;
		struct GMT_FILL fill;
	} G2;
 	struct PSCOUPE_P2 {	/* -Fp[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} P2;
	struct PSCOUPE_R2 {	/* -Fr[<fill>] */
		bool active;
		struct GMT_FILL fill;
	} R2;
 	struct PSCOUPE_T2 {	/* -Ft[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} T2;
};

enum Pscoupe_scaletype {
	PSCOUPE_READ_SCALE	= 0,
	PSCOUPE_CONST_SCALE	= 1};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCOUPE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSCOUPE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.PREF.dip = 90.0;	/* Vertical is the default dip */
	C->A.p_width = 20000;	/* Infinity, basically */
	C->A.exact[GMT_X] = true;	/* We want exact distance range by default if +r is given */
	C->A.exact[GMT_Y] = true;	/* We want approximate depth range by default if +r is given */
	C->L.pen = C->T.pen = C->P2.pen = C->T2.pen = C->W.pen = GMT->current.setting.map_default_pen;
	/* Set width temporarily to -1. This will indicate later that we need to replace by W.pen */
	C->L.pen.width = C->T.pen.width = C->P2.pen.width = C->T2.pen.width = -1.0;
	//C->L.active = true;
	gmt_init_fill (GMT, &C->E.fill, 1.0, 1.0, 1.0);
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);
	C->S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	C->S.font.size = DEFAULT_FONTSIZE;
	C->S.justify = PSL_TC;
	C->S.reference = SEIS_MAG_REFERENCE;
	C->A2.size = DEFAULT_SYMBOL_SIZE * GMT->session.u2u[GMT_PT][GMT_INCH];
	C->A2.P_symbol = C->A2.T_symbol = PSL_CIRCLE;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL void pscoupe_rot_axis (struct AXIS A, struct nodal_plane PREF, struct AXIS *Ar) {
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

GMT_LOCAL void pscoupe_rot_tensor (struct M_TENSOR mt, struct nodal_plane PREF, struct M_TENSOR *mtr) {
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

GMT_LOCAL void pscoupe_rot_nodal_plane (struct nodal_plane PLAN, struct nodal_plane PREF, struct nodal_plane *PLANR) {
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

GMT_LOCAL void pscoupe_rot_meca (st_me meca, struct nodal_plane PREF, st_me *mecar) {
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
		pscoupe_rot_nodal_plane (meca.NP1, PREF, &mecar->NP1);

	if (fabs (meca.NP2.str - PREF.str) < EPSIL && fabs (meca.NP2.dip - PREF.dip) < EPSIL) {
		mecar->NP2.str = 0.;
		mecar->NP2.dip = 0.;
		mecar->NP2.rake = meca_zero_360 (270. - meca.NP2.rake);
	}
	else
		pscoupe_rot_nodal_plane (meca.NP2, PREF, &mecar->NP2);

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

GMT_LOCAL int pscoupe_gutm (double lon, double lat, double *xutm, double *yutm, int fuseau) {
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

GMT_LOCAL int pscoupe_dans_coupe (double lon, double lat, double depth, double xlonref, double ylatref, int fuseau, double str, double dip, double p_length, double p_width, double *distance, double *n_dep) {
/* if fuseau < 0, cartesian coordinates */

	double xlon, ylat, largeur, sd, cd, ss, cs;

	if (fuseau >= 0)
		pscoupe_gutm (lon, lat, &xlon, &ylat, fuseau);
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

GMT_LOCAL void pscoupe_distaz (double lat1, double lon1, double lat2, double lon2, double *distkm, double *azdeg, int syscoord) {
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

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pscoupe synopsis and optionally full usage information */

	struct GMT_FONT font;
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -Aa|b|c|d<params>[+c[n|t]][+d<dip>][+r[a|e|<dx>]][+w<width>][+z[s]a|e|<dz>|<min>/<max>] "
		"%s %s -S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>] "
		"[%s] [-C<cpt>] [-E<fill>] [-Fa[<size>[/<Psymbol>[<Tsymbol>]]]] [-Fe<fill>] [-Fg<fill>] [-Fr<fill>] [-Fp[<pen>]] [-Ft[<pen>]] "
		"[-Fs<symbol><size>] [-G<fill>] [-H[<scale>]] [-I[<intens>]] %s[-L<pen>] [-N] %s%s "
		"[-Q] [-T<nplane>[/<pen>]] [%s] [%s] [-W<pen>] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT,
		API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_tv_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	font = API->GMT->current.setting.font_annot[GMT_PRIMARY];
	font.size = DEFAULT_FONTSIZE;

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, 1, "\n-Aa|b|c|d<params>[+c[n|t]][+d<dip>][+r[a|e|<dx>]][+w<width>][+z[s]a|e|<dz>|<min>/<max>]");
	GMT_Usage (API, -2, "Specify cross-section parameters. Choose directive and append parameters:");
	GMT_Usage (API, 3, "a: Geographic start and end points, append <lon1>/<lat1>/<lon2>/<lat2>.");
	GMT_Usage (API, 3, "b: Geographic start point and strike, length: Append <lon1>/<lat1>/<strike>/<length>.");
	GMT_Usage (API, 3, "c: Cartesian start and end points, append <x1>/<y1>/<x2>/<y2>.");
	GMT_Usage (API, 3, "d: Cartesian start point and strike, length, Append <x1>/<y1>/<strike>/<length>.");
	GMT_Usage (API, -2, "Several optional modifiers are available:");
	GMT_Usage (API, 3, "+c No plotting; print the region as a -Rw/e/s/n string (+ct) or numbers (+c[n] or Default).");
	GMT_Usage (API, 3, "+d Set the <dip> of the plane [90].");
	GMT_Usage (API, 3, "+r Determine and set plot domain (-R) from the cross-section parameters [Use -R as given]. "
		"Optionally, append a to adjust domain to suitable multiples of dx/dz, e for the exact domain, or <dx> to quantize distances.");
	GMT_Usage (API, 3, "+w Set the <width> of cross-section on each side of a vertical plane or above and under an oblique plane [infinity].");
	GMT_Usage (API, 3, "+z Adjust the z-range.  Append a for a sensible rounding, e for the exact range, <dz> to quantize depths, or "
		"distance min/max from horizontal plane in km, along steepest descent direction [no limit]. Optionally prepend s to clamp minimum depth at surface (0).");
	GMT_Usage (API, -2, "Note: <width>, <length>, <dx>, <dz>, <min> and <max> must all be given in km.");
	GMT_Usage (API, -2, "Use CPT to assign colors based on depth-value in 3rd column.");
	GMT_Option (API, "J-,R");
	GMT_Usage (API, 1, "\n-S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>]");
	GMT_Usage (API, -2, "Select format directive and optional symbol modifiers:");
	GMT_Usage (API, 3, "a: Focal mechanism in Aki & Richard's convention:");
	GMT_Usage (API, 4, "X Y depth strike dip rake mag [newX newY] [event_title].");
	GMT_Usage (API, 3, "c: Focal mechanism in Global CMT convention");
	GMT_Usage (API, 4, "X Y depth strike1 dip1 rake1 strike2 dip2 rake2 moment [newX newY] [event_title], "
		"with moment in 2 columns : mantissa and exponent corresponding to seismic moment in dynes-cm.");
	GMT_Usage (API, 3, "d: Closest double couple defined from seismic moment tensor (zero trace and zero determinant):");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "p: Focal mechanism defined with:");
	GMT_Usage (API, 4, "X Y depth strike1 dip1 strike2 fault mag [newX newY] [event_title]. "
		"fault = -1/+1 for a normal/inverse fault.");
	GMT_Usage (API, 3, "m: Seismic (full) moment tensor:");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "t: Zero trace moment tensor defined from principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "x: Principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "y: Best double couple defined from principal axis:");
	GMT_Usage (API, 4, "X Y depth T_value T_azim T_plunge N_value N_azim N_plunge P_value P_azim P_plunge exp [newX newY] [event_title].");
	GMT_Usage (API, 3, "z: Deviatoric part of the moment tensor (zero trace):");
	GMT_Usage (API, 4, "X Y depth mrr mtt mff mrt mrf mtf exp [newX newY] [event_title].");
	GMT_Usage (API, -2, "If <scale> is not given then it is read from the first column after the required columns. Optional modifiers for the label:");
	GMT_Usage (API, 3, "+a Set the label angle [0].");
	GMT_Usage (API, 3, "+f Set font attributes for the label [%s].", gmt_putfont (API->GMT, &font));
	GMT_Usage (API, 3, "+j Set the label <justification> [TC].");
	GMT_Usage (API, 3, "+l Use linear symbol scaling based on moment [magnitude].");
	GMT_Usage (API, 3, "+m Use <scale> as fixed size for any magnitude or moment.");
	GMT_Usage (API, 3, "+o Set the label offset <dx>[/<dy>] [0/0].");
	GMT_Usage (API, 3, "+s Set reference magnitude [%g] or moment [%ge%d] (if +l) for symbol size.", SEIS_MAG_REFERENCE, SEIS_MOMENT_MANT_REFERENCE, SEIS_MOMENT_EXP_REFERENCE);
	GMT_Usage (API, -2, "Note: If fontsize = 0 (+f0) then no label written; offset is from the limit of the beach ball.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<cpt>");
	gmt_fill_syntax (API->GMT, 'E', NULL, "Set color used for extensive parts [Default is white].");
	GMT_Usage (API, 1, "\n-F<directive><parameters> (repeatable)");
	GMT_Usage (API, -2, "Set various attributes of symbols depending on directive:");
	GMT_Usage (API, 3, "a: Plot axis. Optionally append <size>[/<Psymbol>[<Tsymbol>] [Default symbols are circles].");
	GMT_Usage (API, 3, "e: Append color used for <Tsymbol> [Default as set by -E].");
	GMT_Usage (API, 3, "g: Append color used for <Psymbol> [default as set by -G].");
	GMT_Usage (API, 3, "p: Draw <Psymbol> outline using the current pen (see -W; or append alternative pen).");
	GMT_Usage (API, 3, "r: Fill box behind labels with appended color.");
	GMT_Usage (API, 3, "s: Select symbol type and symbol size (in %s). Choose between "
		"st(a)r, (c)ircle, (d)iamond, (h)exagon, (i)nvtriangle, (s)quare, (t)riangle.",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Usage (API, 3, "t: Draw <Tsymbol> outline using the current pen (see -W; or append alternative pen).");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Set color used for compressive parts [Default is black].");
	GMT_Usage (API, 1, "\n-H[<scale>]");
	GMT_Usage (API, -2, "Scale symbol sizes (set via -S or input column) and pen attributes by factors read from scale column. "
		"The scale column follows the symbol size column.  Alternatively, append a fixed <scale>.");
	GMT_Usage (API, 1, "\n-I[<intens>]");
	GMT_Usage (API, -2, "Use the intensity to modulate the compressive fill color (requires -C or -G). "
		"If no intensity is given we expect it to follow the required columns in the data record.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-L<pen>");
	GMT_Usage (API, -2, "Draw line or symbol outline using the current pen (see -W; or append alternative pen).");
	GMT_Usage (API, 1, "\n-N Do Not skip/clip symbols that fall outside map border [Default will ignore those outside].");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q Do not print cross-section information to files.");
	GMT_Usage (API, 1, "\n-T<plane>[/<pen>]");
	GMT_Usage (API, -2, "Draw specified nodal <plane>(s) and circumference only to provide a transparent beach ball "
		"using the current pen (see -W; or append alternative pen):");
	GMT_Usage (API, 3, "1: Only the first nodal plane is plotted.");
	GMT_Usage (API, 3, "2: Only the second nodal plane is plotted.");
	GMT_Usage (API, 3, "0: Both nodal planes are plotted.");
	GMT_Usage (API, -2, "Note: If moment tensor is required, nodal planes overlay moment tensor.");
	GMT_Option (API, "U,V");
	GMT_Usage (API, 1, "\n-W<pen>");
	GMT_Usage (API, -2, "Set pen attributes [%s].", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X,c,di,e,h,i,p,qi,T,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int pscoupe_parse_old_A (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *Ctrl, char *arg) {
	int n;
	char *p = NULL;
	gmt_M_unused (GMT);
	if ((p = strstr (arg, "+f"))) {	/* Get the frame from the cross-section parameters */
		Ctrl->A.frame = true;
		p[0] = '\0';	/* Chop off modifier */
	}
	else if (arg[strlen(arg)-1] == 'f')	/* Very deprecated GMT3-4 syntax */
		Ctrl->A.frame = true;
	if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'c') {
		n = sscanf (&arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
			&Ctrl->A.lon1, &Ctrl->A.lat1, &Ctrl->A.lon2, &Ctrl->A.lat2, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
	}
	else {
		n = sscanf (&arg[1], "%lf/%lf/%lf/%lf/%lf/%lf/%lf/%lf",
			&Ctrl->A.lon1, &Ctrl->A.lat1, &Ctrl->A.PREF.str, &Ctrl->A.p_length, &Ctrl->A.PREF.dip, &Ctrl->A.p_width, &Ctrl->A.dmin, &Ctrl->A.dmax);
	}
	if (n != 8) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Parsing of old format %s only recovered %d items but 8 was expected - formatting/unexpected unit problem?\n", &arg[1], n);
		return GMT_PARSE_ERROR;
	}

	return GMT_NOERROR;
}

static int parse (struct GMT_CTRL *GMT, struct PSCOUPE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pscoupe and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""}, *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Cross-section definition */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				Ctrl->A.proj_type = opt->arg[0];
				if (strstr (opt->arg, "+f") || gmt_count_char (GMT, opt->arg, '/') == 7)	/* Old deprecated syntax */
					n_errors += pscoupe_parse_old_A (GMT, Ctrl, opt->arg);
				else {	/* New, modifier-equipped syntax */
					if ((p = gmt_first_modifier (GMT, opt->arg, "drwz"))) {	/* Process any modifiers */
						if (gmt_get_modifier (p, 'd', txt_a))
							Ctrl->A.PREF.dip = atof (txt_a);
						if (gmt_get_modifier (p, 'r', txt_a)) {	/* +r[a|e|<dx>] */
							Ctrl->A.frame = true;
							switch (txt_a[0]) {
								case 'a':	/* Auto for both axes */
									Ctrl->A.exact[GMT_X] = Ctrl->A.exact[GMT_Y] = false;
									break;
								case 'e':	/* Exact data range for both axes [Default] */
								case '\0':
									Ctrl->A.exact[GMT_X] = Ctrl->A.exact[GMT_Y] = true;
									break;
								default:	/* Got a <dx> argument in km so exact in x, auto in */
									Ctrl->A.dx = atof (txt_a);
									Ctrl->A.exact[GMT_X] = true;
									break;
							}
						}
						if (gmt_get_modifier (p, 'c', txt_a)) /* Check if +c, +cn [Default], or +ct */
							Ctrl->A.report = (txt_a[0] == 't') ? GMT_IS_TEXT : GMT_IS_FLOAT;
						if (gmt_get_modifier (p, 'w', txt_a))
							Ctrl->A.p_width = atof (txt_a);
						if (gmt_get_modifier (p, 'z', txt_a)) {	/* +z[s]a|e|<dz>|<z0/z1> */
							unsigned k = 0;
							if (txt_a[0] == 's') {	/* Force zmin to be at surface (0) */
								Ctrl->A.force = true;
								k++;
							}
							switch (txt_a[k]) {
								case 'a':	/* Auto-depths */
								case '\0':	/* Which is also the default */
									Ctrl->A.exact[GMT_Y] = false;	/* Determine good depth range */
									break;
								case 'e':	/* Want exact depth range */
									Ctrl->A.exact[GMT_Y] = true;	/* Determine exact depth range */
									break;
								default:
									if (strchr (txt_a, '/'))	/* Gave min/max */
										sscanf (&txt_a[k], "%lf/%lf", &Ctrl->A.dmin, &Ctrl->A.dmax);
									else	/* Gave dz increment */
										Ctrl->A.dz = atof (&txt_a[k]);
									Ctrl->A.exact[GMT_Y] = true;	/* Determine exact rounded depth range */
									break;
							}
						}
						p[0] = '\0';	/* Chop off modifiers */
					}
					/* Process the first 4 args */
					if (sscanf (&opt->arg[1], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d) != 4) {
						GMT_Report (API, GMT_MSG_ERROR, "-A requires 4 arguments before modifiers.\n");
						n_errors++;
					}
					switch (Ctrl->A.proj_type) {
						case 'a':
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &Ctrl->A.lon1), txt_a);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &Ctrl->A.lat1), txt_b);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_c, GMT_IS_LON, &Ctrl->A.lon2), txt_c);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_d, GMT_IS_LAT, &Ctrl->A.lat2), txt_d);
							break;
						case 'b':
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &Ctrl->A.lon1), txt_a);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &Ctrl->A.lat1), txt_b);
							Ctrl->A.PREF.str = atof (txt_c);
							Ctrl->A.p_length = atof (txt_d);
							break;
						case 'c':
							n_errors += gmt_verify_expectations (GMT, GMT_IS_FLOAT, gmt_scanf (GMT, txt_a, GMT_IS_FLOAT, &Ctrl->A.lon1), txt_a);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_FLOAT, gmt_scanf (GMT, txt_b, GMT_IS_FLOAT, &Ctrl->A.lat1), txt_b);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_FLOAT, gmt_scanf (GMT, txt_c, GMT_IS_FLOAT, &Ctrl->A.lon2), txt_c);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_FLOAT, gmt_scanf (GMT, txt_d, GMT_IS_FLOAT, &Ctrl->A.lat2), txt_d);
							break;
						case 'd':
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf (GMT, txt_a, GMT_IS_LON, &Ctrl->A.lon1), txt_a);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf (GMT, txt_b, GMT_IS_LAT, &Ctrl->A.lat1), txt_b);
							Ctrl->A.PREF.str = atof (txt_c);
							Ctrl->A.p_length = atof (txt_d);
							break;
						default:
							GMT_Report (API, GMT_MSG_ERROR, "Option -A: Unrecognized mode %c.\n", Ctrl->A.proj_type);
							n_errors++;
							break;
					}
				}
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'c') {
					pscoupe_distaz (Ctrl->A.lat1, Ctrl->A.lon1, Ctrl->A.lat2, Ctrl->A.lon2, &Ctrl->A.p_length, &Ctrl->A.PREF.str, Ctrl->A.proj_type == 'a' ?  COORD_DEG : COORD_KM);
					sprintf (Ctrl->A.newfile, "A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, Ctrl->A.lon1, Ctrl->A.lat1, Ctrl->A.lon2, Ctrl->A.lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (Ctrl->A.extfile, "A%c%.1f_%.1f_%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, Ctrl->A.lon1, Ctrl->A.lat1, Ctrl->A.lon2, Ctrl->A.lat2, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				}
				else {
					sprintf (Ctrl->A.newfile, "A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f",
						Ctrl->A.proj_type, Ctrl->A.lon1, Ctrl->A.lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
					sprintf (Ctrl->A.extfile, "A%c%.1f_%.1f_%.0f_%.0f_%.0f_%.0f_%.0f_%.0f_map",
						Ctrl->A.proj_type, Ctrl->A.lon1, Ctrl->A.lat1, Ctrl->A.PREF.str, Ctrl->A.p_length, Ctrl->A.PREF.dip, Ctrl->A.p_width, Ctrl->A.dmin, Ctrl->A.dmax);
				}
				if (Ctrl->A.proj_type == 'a' || Ctrl->A.proj_type == 'b')
					Ctrl->A.fuseau = pscoupe_gutm (Ctrl->A.lon1, Ctrl->A.lat1, &Ctrl->A.xlonref, &Ctrl->A.ylatref, 0);
				else {
					Ctrl->A.fuseau = -1;
					Ctrl->A.xlonref = Ctrl->A.lon1;
					Ctrl->A.ylatref = Ctrl->A.lat1;
				}
				Ctrl->A.polygon = true;
				break;

			case 'Z':	/* Backwards compatibility */
				if (gmt_M_compat_check (GMT, 6))
					GMT_Report (API, GMT_MSG_COMPAT, "-Z<cpt> is deprecated; use -C<cpt> instead.\n");
				else {	/* Hard error */
					n_errors += gmt_default_error (GMT, opt->option);
					continue;
				}
				/* Deliberate fall-through here (no break) */
			case 'C':	/* Vary symbol color with z */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;

			case 'E':	/* Set color for extensive parts  */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->E.fill))) {
					gmt_fill_syntax (GMT, 'E', NULL, " ");
					n_errors++;
				}
				Ctrl->A.polygon = true;
				break;
			case 'F':	/* Set various symbol parameters  */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'a':	/* plot axis */
						Ctrl->A2.active = true;
						strncpy (txt_a, &opt->arg[1], GMT_LEN256-1);
						if ((p = strchr (txt_a, '/')) != NULL) p[0] = '\0';
						if (txt_a[0]) Ctrl->A2.size = gmt_M_to_inch (GMT, txt_a);
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
							gmt_pen_syntax (GMT, ' ', "Fp", " ", NULL, 0);
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
						if (gmt_found_modifier (GMT, opt->arg, "fjo")) {
							/* New syntax: -Fs<symbol>[<size>]+f<font>+o<dx>/<dy>+j<justify> */
							char word[GMT_LEN256] = {""}, *c = NULL;

							/* parse beachball size */
							if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
							if (opt->arg[2]) Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[2]);
							if (c) c[0] = '+';	/* Restore modifiers */

							if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
								Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
							if (gmt_get_modifier (opt->arg, 'f', word)) {
								if (word[0] == '-' || (word[0] == '0' && (word[1] == '\0' || word[1] == 'p')))
									Ctrl->S.font.size = 0.0;
								else
									n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
							}
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
							txt_a[0] = txt_b[0] = txt_c[0] = '\0';
							sscanf (&opt->arg[2], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
							if (txt_a[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt_a);
							if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
							if (txt_c[0]) Ctrl->S.offset[1] = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
							if (Ctrl->S.font.size < 0.0) Ctrl->S.no_label = true;
							if (p) p[0] = '+';	/* Restore modifier */
						}
						if (gmt_M_is_zero (Ctrl->S.scale)) Ctrl->S.read = true;	/* Must get size from input file */
						break;
					case 't':	/* Draw outline of T axis symbol [set outline attributes] */
						Ctrl->T2.active = true;
						if (opt->arg[1] && gmt_getpen (GMT, &opt->arg[1], &Ctrl->T2.pen)) {
							gmt_pen_syntax (GMT, ' ', "Ft", " ", NULL, 0);
							n_errors++;
						}
						break;
				}
				break;
			case 'G':	/* Set color for compressive parts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (!opt->arg[0] || (opt->arg[0] && gmt_getfill (GMT, opt->arg, &Ctrl->G.fill))) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				Ctrl->A.polygon = true;
				break;
			case 'H':		/* Overall symbol/pen scale column provided */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				Ctrl->H.active = true;
				if (opt->arg[0]) {	/* Gave a fixed scale - no reading from file */
					Ctrl->H.value = atof (opt->arg);
					Ctrl->H.mode = PSCOUPE_CONST_SCALE;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (opt->arg[0])
					Ctrl->I.value = atof (opt->arg);
				else
					Ctrl->I.mode = 1;
				break;
			case 'L':	/* Draw outline [set outline attributes] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					gmt_pen_syntax (GMT, 'L', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'M':	/* Same size for any magnitude [Deprecated 8/14/2021 6.3.0 - use -S+m instead] */
				if (gmt_M_compat_check (GMT, 6)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-M is deprecated from 6.3.0; use -S modifier +m instead.\n");
					Ctrl->S.fixed = true;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'N':	/* Do not skip points outside border */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Switch of production of mechanism files */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				break;
			case 'S':	/* Mechanisms : get format [and size] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
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

				if (gmt_found_modifier (GMT, opt->arg, "afjlmos")) {
					/* New syntax: -S<format>[<scale>][+a<angle>][+f<font>][+j<justify>][+l][+m][+o<dx>[/<dy>]][+s<ref>] */
					char word[GMT_LEN256] = {""}, *c = NULL;

					/* Parse beachball size */
					if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
					Ctrl->S.scale = gmt_M_to_inch (GMT, &opt->arg[1]);
					if (c) c[0] = '+';	/* Restore modifiers */

					if (gmt_get_modifier (opt->arg, 'a', word))
						Ctrl->S.angle = atof(word);
					if (gmt_get_modifier (opt->arg, 'j', word) && strchr ("LCRBMT", word[0]) && strchr ("LCRBMT", word[1]))
						Ctrl->S.justify = gmt_just_decode (GMT, word, Ctrl->S.justify);
					if (gmt_get_modifier (opt->arg, 'f', word)) {
						if (word[0] == '-' || (word[0] == '0' && (word[1] == '\0' || word[1] == 'p')))
							Ctrl->S.font.size = 0.0;
						else
							n_errors += gmt_getfont (GMT, word, &(Ctrl->S.font));
					}
					if (gmt_get_modifier (opt->arg, 'o', word)) {
						if (gmt_get_pair (GMT, word, GMT_PAIR_DIM_DUP, Ctrl->S.offset) < 0) n_errors++;
					} else {	/* Set default offset */
						if (Ctrl->S.justify%4 != 2) /* Not center aligned */
							Ctrl->S.offset[0] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
						if (Ctrl->S.justify/4 != 1) /* Not middle aligned */
							Ctrl->S.offset[1] = DEFAULT_OFFSET * GMT->session.u2u[GMT_PT][GMT_INCH];
					}
					if (Ctrl->S.font.size <= 0.0) Ctrl->S.no_label = true;
					if (gmt_get_modifier (opt->arg, 'l', word)) {
						Ctrl->S.linear = true;
						Ctrl->S.reference = SEIS_MOMENT_MANT_REFERENCE * pow (10.0, SEIS_MOMENT_EXP_REFERENCE);	/* May change if +s is given */
					}
					if (gmt_get_modifier (opt->arg, 'm', word))
						Ctrl->S.fixed = true;
					if (gmt_get_modifier (opt->arg, 's', word))
						Ctrl->S.reference = atof (word);
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
					txt_a[0] = txt_b[0] = txt_c[0] = '\0';
					sscanf (&opt->arg[1], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
					if (txt_a[0]) Ctrl->S.scale = gmt_M_to_inch (GMT, txt_a);
					if (txt_b[0]) Ctrl->S.font.size = gmt_convert_units (GMT, txt_b, GMT_PT, GMT_PT);
					if (txt_c[0]) Ctrl->S.offset[1] = gmt_convert_units (GMT, txt_c, GMT_PT, GMT_INCH);
					if (Ctrl->S.font.size < 0.0) Ctrl->S.no_label = true;
					if (p) p[0] = '+';	/* Restore modifier */
				}
				if (gmt_M_is_zero (Ctrl->S.scale)) Ctrl->S.read = true;	/* Must get size from input file */
				break;

			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				sscanf (opt->arg, "%d", &Ctrl->T.n_plane);
				if (strlen (opt->arg) > 2 && gmt_getpen (GMT, &opt->arg[2], &Ctrl->T.pen)) {	/* Set transparent attributes */
					gmt_pen_syntax (GMT, 'T', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'W':	/* Set line attributes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	/* Check that the options selected are mutually consistent */

	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active, "Must specify -A option\n");
	//n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->S.active, "Cannot specify both -F and -S\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET] && !Ctrl->A.frame, "Must specify -R option or -A...+r modifier\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active, "Must specify -S option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.scale < 0.0, "Option -S: must specify scale\n");

	/* Set to default pen where needed */
	if (Ctrl->W.active && !Ctrl->L.active) Ctrl->L.active = true;

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

EXTERN_MSC int GMT_pscoupe (void *V_API, int mode, void *args) {
	bool detect_range = false, has_text;

	int n_rec = 0, n_plane_old = 0, form = 0, error;
	int i, transparence_old = 0, not_defined = 0;
	int n_scanned = 0;
	unsigned int xcol = 0, scol = 0, icol = 0, tcol_f = 0, tcol_s = 0;
	uint64_t tbl, seg, row, col;

	FILE *pnew = NULL, *pext = NULL;

	double size, xy[2], xynew[2] = {0.0}, plot_x, plot_y, n_dep, distance, fault, depth;
	double scale, P_x, P_y, T_x, T_y, in[GMT_LEN16];

	char event_title[GMT_BUFSIZ] = {""}, Xstring[GMT_BUFSIZ] = {""}, Ystring[GMT_BUFSIZ] = {""};
	char *no_name = "<unnamed>", *event_name = NULL;

	st_me meca, mecar;
	struct MOMENT moment;
	struct M_TENSOR mt, mtr;
	struct AXIS T, N, P, Tr, Nr, Pr;
	struct GMT_PALETTE *CPT = NULL;
	struct GMT_PEN current_pen;
	struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment input tables */
	struct GMT_DATASEGMENT *S = NULL;

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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pscoupe main code ----------------------------*/

	gmt_M_memset (&meca, 1, meca);
	gmt_M_memset (&moment, 1, moment);
	gmt_M_memset (in, GMT_LEN16, double);

	if (Ctrl->C.active) {
		if ((CPT = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
	}
	else if (Ctrl->I.active && Ctrl->I.mode == 0) {	/* No CPT and fixed intensity means we can do the constant change once */
		gmt_illuminate (GMT, Ctrl->I.value, Ctrl->G.fill.rgb);
		Ctrl->I.active = false;	/* So we don't do this again */
	}

	if (Ctrl->S.read) {	/* Read symbol size from file */
		scol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;
		gmt_set_column_type (GMT, GMT_IN, scol, GMT_IS_DIMENSION);
	}
	else	/* Fixed scale */
		scale = Ctrl->S.scale;
	if (Ctrl->H.active && Ctrl->H.mode == PSCOUPE_READ_SCALE) {
		xcol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;	/* Read scaling from data file */
		gmt_set_column_type (GMT, GMT_IN, xcol, GMT_IS_FLOAT);
	}
	if (Ctrl->I.mode) {	/* Read intensity from data file */
		icol = Ctrl->S.n_cols;
		Ctrl->S.n_cols++;
		gmt_set_column_type (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	if (GMT->common.t.variable) {	/* Need one or two transparencies from file */
		if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {
			tcol_f = Ctrl->S.n_cols;
			Ctrl->S.n_cols++;	/* Read fill transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_f, GMT_IS_FLOAT);
		}
		if (GMT->common.t.mode & GMT_SET_PEN_TRANSP) {
			tcol_s = Ctrl->S.n_cols;
			Ctrl->S.n_cols++;	/* Read stroke transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_s, GMT_IS_FLOAT);
		}
	}

	if (Ctrl->S.linear)
		GMT_Report (API, GMT_MSG_INFORMATION, "Linear moment scaling selected, normalizing by %e.\n", Ctrl->S.reference);
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Linear magnitude scaling selected, normalizing by %g.\n", Ctrl->S.reference);

	GMT_Set_Columns (API, GMT_IN, Ctrl->S.n_cols, GMT_COL_FIX);	/* Set the required numerical columns */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}

	/* Read the entire input data set */
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	if (D->n_records == 0)
		GMT_Report (API, GMT_MSG_WARNING, "No data records provided\n");

	if (doubleAlmostEqualZero (Ctrl->A.dmin, Ctrl->A.dmax)) {	/* Must find depth range and max symbol size */
		detect_range = true;
		Ctrl->A.dmin = DBL_MAX;	Ctrl->A.dmax = -DBL_MAX;
		size = Ctrl->S.scale;	/* Default size, if given */
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];	/* Shorthand */
				for (row = 0; row < S->n_rows; row++) {
					if (Ctrl->S.read && S->data[scol][row] > size) size = S->data[scol][row];	/* Largest size so far */
					if (Ctrl->H.active) {	/* Variable scaling of symbol size */
						double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? S->data[xcol][row] : Ctrl->H.value;
						size *= scl;
					}
					if (S->data[GMT_Z][row] < Ctrl->A.dmin) Ctrl->A.dmin = S->data[GMT_Z][row];
					if (S->data[GMT_Z][row] > Ctrl->A.dmax) Ctrl->A.dmax = S->data[GMT_Z][row];
				}
			}
		}
		if (!Ctrl->S.fixed) size *= 1.6;	/* Since we are not computing sizes we assume a max Magnitude 8 symbol size instead of 5 */
		size /= 2.0;	/* Since we want radius below */
		GMT_Report (API, GMT_MSG_INFORMATION, "Observed frame range is 0/%lg/%lg/%lg\n", Ctrl->A.p_length, Ctrl->A.dmin, Ctrl->A.dmax);
	}

	if (Ctrl->A.frame) {	/* Do initial or final map setup */
		/* If dx or dz were given then we impose those conditions here.  We also impose the +zs clamp at zero depth here */
		GMT->common.R.wesn[XLO] = 0.0;
		GMT->common.R.wesn[XHI] = (Ctrl->A.dx > 0.0) ? ceil (Ctrl->A.p_length / Ctrl->A.dx) * Ctrl->A.dx : Ctrl->A.p_length;
		GMT->common.R.wesn[YLO] = (Ctrl->A.dz > 0.0) ? floor (Ctrl->A.dmin / Ctrl->A.dz) * Ctrl->A.dz : ((Ctrl->A.force) ? 0.0 : Ctrl->A.dmin);
		GMT->common.R.wesn[YHI] = (Ctrl->A.dz > 0.0) ? ceil (Ctrl->A.dmax / Ctrl->A.dz) * Ctrl->A.dz : Ctrl->A.dmax;
		GMT_Report (API, GMT_MSG_INFORMATION, "Rounded frame range is %lg/%lg/%lg/%lg\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		if (gmt_M_is_zero (Ctrl->A.PREF.dip)) Ctrl->A.PREF.dip = 1.0;
	}
	gmt_set_cartesian (GMT, GMT_IN);	/* Since that is what it is */

	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	if (Ctrl->A.frame && detect_range) {	/* Extend y-range by largest symbol size and maybe label space*/
		double z_range_1 = (Ctrl->A.dmax - Ctrl->A.dmin);
		double z_inc_scl, dz, label_space[2] = {0.0, 0.0};	/* Note: label_space may remain at 0,0 if justification is on the side of the symbol */
		if (!Ctrl->S.no_label) {	/* Deal with space needed by label */
			int label_justify = gmt_flip_justify(GMT, Ctrl->S.justify);
			double H = (GMT_TEXT_CLEARANCE * 0.01 + GMT_LET_HEIGHT) * Ctrl->S.font.size / PSL_POINTS_PER_INCH;	/* Combined label and offset dimension */
			if (label_justify < 4)	/* Label goes below symbol */
				label_space[1] = H + Ctrl->S.offset[1];
			else if (label_justify > 8)	/* Label goes above symbol */
				label_space[0] = H + Ctrl->S.offset[1];
		}
		/* Need GMT->current.map.height hence the initial gmt_map_setup call.  */
		z_inc_scl = z_range_1 / (GMT->current.map.height - 2.0*size - MAX(label_space[0], label_space[1]));
		dz = (size + label_space[1]) * z_inc_scl;	/* km to extend low range outward */
		GMT->common.R.wesn[YLO] -= dz;
		dz = (size + label_space[0]) * z_inc_scl;	/* km to extend high range outward */
		GMT->common.R.wesn[YHI] += dz;
		GMT_Report (API, GMT_MSG_INFORMATION, "Symbol-adjusted depth range is %lg/%lg/%lg/%lg\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		if (!(Ctrl->A.exact[GMT_X] && Ctrl->A.exact[GMT_Y])) {
			double wesn[4];
			gmt_M_memcpy (wesn, GMT->common.R.wesn, 4U, double);	/* Make a copy */
			gmt_round_wesn (GMT->common.R.wesn, false);	/* Use data range to round to nearest reasonable multiples */
			if (Ctrl->A.exact[GMT_X]) gmt_M_memcpy (GMT->common.R.wesn, wesn, 2U, double);	/* Wanted to keep the original range */
			if (Ctrl->A.exact[GMT_Y]) gmt_M_memcpy (&GMT->common.R.wesn[YLO], &wesn[YLO], 2U, double);	/* Wanted to keep the original range */
			GMT_Report (API, GMT_MSG_INFORMATION, "Final auto-adjusted depth range is %lg/%lg/%lg/%lg\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
		}
		if (Ctrl->A.force) GMT->common.R.wesn[YLO] = 0.0;	/* Just as a precaution */

		if (Ctrl->A.report) {	/* No plotting, just report the region */
			char txt[GMT_LEN256] = {""};
			struct GMT_RECORD *Out = NULL;
			GMT_Report (API, GMT_MSG_INFORMATION, "Report region as %lg/%lg/%lg/%lg\n", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
				Return (API->error);
			}
			if (Ctrl->A.report == GMT_IS_TEXT) {
				Out = gmt_new_record (GMT, NULL, txt);	/* Since we only need to worry about trailing text */
				sprintf (txt, "-R%.16lg/%.16lg/%.16lg/%.16lg", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
				if ((error = GMT_Set_Columns (API, GMT_OUT, 0, GMT_COL_FIX)) != GMT_NOERROR) {
					Return (error);
				}
			}
			else {	/* Want numerical record */
				Out = gmt_new_record (GMT, GMT->common.R.wesn, NULL);	/* Since we only need to worry about numerics */
				if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
					Return (error);
				}
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
				Return (API->error);
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
				Return (API->error);
			}
			gmt_M_free (GMT, Out);
			Return (GMT_NOERROR);
		}

		/* Now must do the set up again since wesn has changed */
		if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);
	}

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, Ctrl->N.active ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_BEFORE);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);

	PSL_setfont (PSL, GMT->current.setting.font_annot[GMT_PRIMARY].id);

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);


	if (!Ctrl->Q.active) {
		pnew = fopen (Ctrl->A.newfile, "w");
		pext = fopen (Ctrl->A.extfile, "w");
	}

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];	/* Shorthand */
			for (row = 0; row < S->n_rows; row++) {
				for (col = 0; col < S->n_columns; col++) in[col] = S->data[col][row];	/* Make a local copy */
				if (gmt_M_is_dnan (in[GMT_X]) || gmt_M_is_dnan (in[GMT_Y]))	/* Probably a non-recognized header since we got NaNs */
					continue;

				/* Data record to process */

				has_text = S->text && S->text[row];
				n_rec++;
				if (Ctrl->S.read) scale = in[scol];
				size = scale;
				if (Ctrl->H.active) {	/* Variable scaling of symbol size and pen width */
					double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
					size *= scl;
				}
				/* Must examine the trailing text for optional columns: newX, newY and title */
				/* newX and newY are not used in pscoupe, but we promised psmeca and pscoupe can use the same input file */
				if (has_text) {
					n_scanned = sscanf (S->text[row], "%s %s %[^\n]s\n", Xstring, Ystring, event_title);
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
								strncpy (event_title, S->text[row], GMT_BUFSIZ-1);
						}
						else if (n_scanned == 2)	/* Got no title */
							event_title[0] = '\0';
					}
					else if (n_scanned == 1)	/* Only got event title */
						strncpy (event_title, S->text[row], GMT_BUFSIZ-1);
					else	/* Got no title */
						event_title[0] = '\0';
				}

				depth = in[GMT_Z];

				if (!pscoupe_dans_coupe (in[GMT_X], in[GMT_Y], depth, Ctrl->A.xlonref, Ctrl->A.ylatref, Ctrl->A.fuseau, Ctrl->A.PREF.str,
					Ctrl->A.PREF.dip, Ctrl->A.p_length, Ctrl->A.p_width, &distance, &n_dep) && !Ctrl->N.active)
					continue;

				xy[GMT_X] = distance;
				xy[GMT_Y] = n_dep;

				if (!Ctrl->N.active) {
					gmt_map_outside (GMT, xy[GMT_X], xy[GMT_Y]);
					if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
				}

				if (Ctrl->C.active)	/* Update color based on depth */
					gmt_get_fill_from_z (GMT, CPT, depth, &Ctrl->G.fill);
				if (Ctrl->I.active) {	/* Modify color based on intensity */
					if (Ctrl->I.mode == 0)
						gmt_illuminate (GMT, Ctrl->I.value, Ctrl->G.fill.rgb);
					else
						gmt_illuminate (GMT, in[icol], Ctrl->G.fill.rgb);
				}
				if (GMT->common.t.variable) {	/* Update the transparency for current symbol (or -t was given) */
					double transp[2] = {0.0, 0.0};	/* None selected */
					if (GMT->common.t.n_transparencies == 2) {	/* Requested two separate values to be read from file */
						transp[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
						transp[GMT_PEN_TRANSP]  = 0.01 * in[tcol_s];
					}
					else if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {	/* Gave fill transparency */
						transp[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
						if (GMT->common.t.n_transparencies == 0) transp[GMT_PEN_TRANSP] = transp[GMT_FILL_TRANSP];	/* Implied to be used for stroke also */
					}
					else {	/* Gave stroke transparency */
						transp[GMT_PEN_TRANSP] = 0.01 * in[tcol_s];
						if (GMT->common.t.n_transparencies == 0) transp[GMT_FILL_TRANSP] = transp[GMT_PEN_TRANSP];	/* Implied to be used for fill also */
					}
					PSL_settransparencies (PSL, transp);
				}

				gmt_geo_to_xy (GMT, xy[GMT_X], xy[GMT_Y], &plot_x, &plot_y);

				if (Ctrl->S.symbol) {
					if (!Ctrl->Q.active) {
						fprintf (pnew, "%f %f %f %f\n", distance, n_dep, depth, in[3]);
						fprintf (pext, "%s\n", S->text ? S->text[row] : "");
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
					pscoupe_rot_meca (meca, Ctrl->A.PREF, &mecar);
				}
				else if (Ctrl->S.readmode == READ_AKI) {
					meca.NP1.str    = in[3];
					meca.NP1.dip    = in[4];
					meca.NP1.rake   = in[5];
					meca.magms      = in[6];
					moment.mant     = meca.magms;
					moment.exponent = 0;
					meca_define_second_plane (meca.NP1, &meca.NP2);
					pscoupe_rot_meca (meca, Ctrl->A.PREF, &mecar);
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
						event_name = (has_text) ? S->text[row] : no_name;
						GMT_Report (API, GMT_MSG_WARNING, "Second plane is not defined for event %s only first plane is plotted.\n", event_name);
					}
					else
						meca.NP1.rake = meca_computed_rake2 (meca.NP2.str, meca.NP2.dip, meca.NP1.str, meca.NP1.dip, fault);
					meca.NP2.rake = meca_computed_rake2 (meca.NP1.str, meca.NP1.dip, meca.NP2.str, meca.NP2.dip, fault);
					pscoupe_rot_meca (meca, Ctrl->A.PREF, &mecar);

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

					pscoupe_rot_axis (T, Ctrl->A.PREF, &Tr);
					pscoupe_rot_axis (N, Ctrl->A.PREF, &Nr);
					pscoupe_rot_axis (P, Ctrl->A.PREF, &Pr);
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

					pscoupe_rot_tensor (mt, Ctrl->A.PREF, &mtr);
					meca_moment2axe (GMT, mtr, &T, &N, &P);

					if (Ctrl->S.plotmode == PLOT_DC || Ctrl->T.active) meca_axe2dc (T, P, &meca.NP1, &meca.NP2);
				}

				if (!Ctrl->S.symbol) {
					if (Ctrl->S.fixed) {
						moment.mant     = SEIS_MOMENT_MANT_REFERENCE;
						moment.exponent = SEIS_MOMENT_EXP_REFERENCE;
					}

					size = (scale / Ctrl->S.reference) * ((Ctrl->S.linear) ? moment.mant * pow (10.0, moment.exponent) : meca_computed_mw (moment, meca.magms));

					if (Ctrl->H.active) {	/* Variable scaling of symbol size and pen width */
						double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						size *= scl;
					}

					if (!Ctrl->Q.active) fprintf (pext, "%s\n", S->text ? S->text[row] : "");
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
						current_pen = Ctrl->L.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
						meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active, Ctrl->S.zerotrace, n_rec);
					}

					if (Ctrl->S.zerotrace) {
						current_pen = Ctrl->W.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
						meca_ps_tensor (GMT, PSL, plot_x, plot_y, size, T, N, P, NULL, NULL, true, true, n_rec);
					}

					if (Ctrl->T.active) {
						current_pen = Ctrl->T.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
						meca_ps_plan (GMT, PSL, plot_x, plot_y, meca, size, Ctrl->T.n_plane);
						if (not_defined) {
							not_defined = false;
							Ctrl->T.active = transparence_old;
							Ctrl->T.n_plane = n_plane_old;
						}
					}
					else if (Ctrl->S.plotmode == PLOT_DC) {
						current_pen = Ctrl->L.pen;
						if (Ctrl->H.active) {
							double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
							gmt_scale_pen (GMT, &current_pen, scl);
						}
						gmt_setpen (GMT, &current_pen);
						meca_ps_mechanism (GMT, PSL, plot_x, plot_y, meca, size, &Ctrl->G.fill, &Ctrl->E.fill, Ctrl->L.active);
					}

					if (Ctrl->A2.active) {	/* Plot axis symbols */
						double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						double asize = (Ctrl->H.active) ? Ctrl->A2.size * scl : Ctrl->A2.size;
						if (Ctrl->S.readmode != READ_TENSOR && Ctrl->S.readmode != READ_AXIS) meca_dc2axe (meca, &T, &N, &P);
						meca_axis2xy (plot_x, plot_y, size, P.str, P.dip, T.str, T.dip, &P_x, &P_y, &T_x, &T_y);
						current_pen = Ctrl->P2.pen;
						if (Ctrl->H.active)
							gmt_scale_pen (GMT, &current_pen, scl);
						gmt_setpen (GMT, &current_pen);
						gmt_setfill (GMT, &Ctrl->G2.fill, Ctrl->P2.active);
						PSL_plotsymbol (PSL, P_x, P_y, &asize, Ctrl->A2.P_symbol);
						current_pen = Ctrl->T2.pen;
						if (Ctrl->H.active)
							gmt_scale_pen (GMT, &current_pen, scl);
						gmt_setpen (GMT, &current_pen);
						gmt_setfill (GMT, &Ctrl->E2.fill, Ctrl->T2.active);
						PSL_plotsymbol (PSL, T_x, T_y, &asize, Ctrl->A2.T_symbol);
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

					current_pen = Ctrl->W.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSCOUPE_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					gmt_setpen (GMT, &current_pen);
					PSL_setfill (PSL, Ctrl->R2.fill.rgb, 0);
					if (Ctrl->R2.active) PSL_plottextbox (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, label_offset, 0);
					form = gmt_setfont(GMT, &Ctrl->S.font);
					PSL_plottext (PSL, label_x, label_y, Ctrl->S.font.size, event_title, Ctrl->S.angle, label_justify, form);
				}
			}
		}
	}

	if (GMT->common.t.variable) {	/* Reset the transparencies */
		double transp[2] = {0.0, 0.0};	/* None selected */
		PSL_settransparencies (PSL, transp);
	}

	if (!Ctrl->Q.active) {
		fclose (pnew);
		fclose (pext);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of records read: %li\n", n_rec);

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

EXTERN_MSC int GMT_coupe (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: coupe\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pscoupe (V_API, mode, args);
}
